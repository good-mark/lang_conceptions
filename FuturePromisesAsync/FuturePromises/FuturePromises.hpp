// Realization of SharedState's methods

template<typename objectType>
SharedState<objectType>::SharedState() {
    mutex = CreateMutex( NULL, FALSE, NULL );
	ready = false;
	exceptionThrown = false;
	ev = CreateEvent( NULL, FALSE, FALSE, NULL );
	gotValue = CreateEvent( NULL, FALSE, FALSE, NULL );
}

template<typename objectType>
SharedState<objectType>::SharedState( const SharedState<objectType>& other ) {
	*this = other; 
	mutex = CreateMutex( NULL, FALSE, NULL );
	gotValue = CreateEvent( NULL, FALSE, FALSE, NULL );
}

template<typename objectType>
SharedState<objectType>::~SharedState() {
    CloseHandle( mutex );
	CloseHandle( gotValue );
	CloseHandle( ev );
}

template<typename objectType>
void SharedState<objectType>::WaitResult() {
	std::cout << "I am waiting result\n";
	WaitForSingleObject( ev, INFINITE );
	std::cout << "I've just got the result\n";
}

template<typename objectType>
objectType SharedState<objectType>::Get() {
	WaitForSingleObject( mutex, INFINITE );
    if ( ready ) {
		ReleaseMutex( mutex );
		objectType res = value;
		ready = false;
		exceptionThrown = false;
		SetEvent( gotValue );
        return res;
	}
    if ( exceptionThrown ) {
		ready = false;
		exceptionThrown = false;
		ReleaseMutex( mutex );
		SetEvent( gotValue );
        throw *error;
	}
	ReleaseMutex( mutex );
    throw std::runtime_error( "Something went wrong" );
}

template<typename objectType>
bool SharedState<objectType>::TryGet( objectType& outValue ) {
	WaitForSingleObject( mutex, 0 );
	if ( ready ) {
		outValue = value;
		ready = false;
		exceptionThrown = false;
		ReleaseMutex( mutex );
		SetEvent( gotValue );
        return true;
	}
	else {
		ReleaseMutex( mutex );
		return false;
	}
}

template<typename objectType>
void SharedState<objectType>::setValue( const objectType nvalue ) {
	DWORD res = WaitForSingleObject( mutex, INFINITE );
    if ( res != WAIT_OBJECT_0 ) {
		throw std::runtime_error( "Error was occured" );
	} else {
        value = nvalue;
		ready = true;
		ReleaseMutex( mutex );
		SetEvent( ev );
    }
}

template<typename objectType>
void SharedState<objectType>::setException( const std::exception& exc ) {
    if ( WaitForSingleObject( mutex, 0 ) != WAIT_OBJECT_0 ) {
		throw std::runtime_error( "Error was occured" );
	} else {
        error = exc;
		exceptionThrown = true;
		ReleaseMutex( mutex );		
		SetEvent( ev );
    }
}

// Realization of Future's methods

/*
	Блокирует поток и ожидает возвращение результата.
*/
template<typename objectType>
objectType Future<objectType>::Get() { 
	std::cout << "I am in Get\n";
	sstate->WaitResult();
	objectType res = sstate->Get();
	std::cout << "I've got\n";
	return res;
}

/*
	Возвращает NULL, если результат еще не получен, и сам результат иначе.
*/
template<class objectType>
bool Future<objectType>::TryGet( objectType& outValue ) { // неблокирующая попытка получения
	std::cout << "I am in TryGet\n";
	return sstate.TryGet( outValue );
}
 
// Realization of Promise's methods

/*
	Устанавливает успешный результат в promise.
*/
template<class objectType>
void Promise<objectType>::SetValue( objectType const& value ) { 
	sstate->setValue( value );
}

/*
	Устанавливает, что в процессе вычисления результата произошло исключение.
*/
template<class objectType>
void Promise<objectType>::SetException( std::exception* exc ) { 
	sstate->setException( exc );
}

template<class objectType>
Future<objectType> Promise<objectType>::GetFuture() {
	return Future<objectType>( sstate );
}

AsyncThreadPool::AsyncThreadPool() {
	threadsNum = std::thread::hardware_concurrency();
	buzyThreads = 0;
	mutex = CreateMutex( NULL, FALSE, NULL );
}

AsyncThreadPool::~AsyncThreadPool() {
	CloseHandle( mutex );
}

template<typename objectType>
unsigned __stdcall startProcess( void* params ) {
	std::function<objectType()>* func = ( std::function<objectType()>* ) params;
	//result = pr.GetFuture();
	//std::cout << "I am in startProcess\n";
	Promise<objectType>* pr = ( Promise<objectType>* ) pool.promises[GetCurrentThreadId()];
	std::cout << "I am ready to set value\n";
	objectType res = ( *func )();
	pr->SetValue( res );
	std::cout << "I have set value\n";
	WaitForSingleObject( pr->sstate->gotValue, INFINITE );
	pool.decBuzyThreads(); 
	WaitForSingleObject( pool.mutex, INFINITE );
	free( pool.promises[GetCurrentThreadId()] );
	pool.promises.erase( GetCurrentThreadId() );
	ReleaseMutex( pool.mutex );
	CloseHandle( GetCurrentThread() );
	_endthreadex( 0 );
	return 0;
}

bool AsyncThreadPool::enough() {
	return buzyThreads >= threadsNum;
}

void AsyncThreadPool::incBuzyThreads() {
	WaitForSingleObject( mutex, INFINITE ); // 1
	++buzyThreads;
	ReleaseMutex( mutex );
}

void AsyncThreadPool::decBuzyThreads() {
	WaitForSingleObject( mutex, INFINITE ); // 1
	--buzyThreads;
	ReleaseMutex( mutex );
}

template<typename objectType> // шаблон - тип возвращаемого значения func
Future<objectType> async( std::function<objectType(void)>* func, bool synch ) {
	
	if ( synch || pool.enough() ) {
		Promise<objectType> pr;
		Future<objectType> result = pr.GetFuture();
		pr.SetValue( ( *func )() );
		return result;
	} else {
		pool.incBuzyThreads();
		unsigned threadID;
		//std::function<objectType(void)> task = std::bind( &startProcess, func );
		//std::pair<std::function<objectType()>, Promise<objectType> > res = std::make_pair( func, pr );
		HANDLE hThread = ( HANDLE ) _beginthreadex( NULL, 0, &startProcess<objectType>, ( void* ) func, 0, &threadID );
		WaitForSingleObject( pool.mutex, INFINITE );
		pool.promises[threadID] = new Promise<objectType>();
		ReleaseMutex( pool.mutex );Promise<objectType>* pr = ( Promise<objectType>* ) pool.promises[threadID];
		return pr->GetFuture();
	}
	
}