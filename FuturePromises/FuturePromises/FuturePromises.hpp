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
		//ready = false;
		//exceptionThrown = false;
		SetEvent( gotValue );
        return res;
	}
    if ( exceptionThrown ) {
		//ready = false;
		//exceptionThrown = false;
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
		//ready = false;
		//exceptionThrown = false;
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
	Blocks the thread and waits for return. 
*/
template<typename objectType>
objectType Future<objectType>::Get() { 
	std::cout << "I am in Get\n";
	sstate->WaitResult();
	objectType res = sstate->Get();
	return res;
}

/*
	Non-blocking attempt to get the result.
	If the result is not ready yet, return NULL, else return result.
*/
template<class objectType>
bool Future<objectType>::TryGet( objectType& outValue ) { 
	std::cout << "I am in TryGet\n";
	return sstate->TryGet( outValue );
}

template<typename objectType>
template<typename argType>
Future<objectType>& Future<objectType>::callAfter(  Future<argType>& args ) {
	argType arg;
	args.TryGet( arg );
	//std::function<objectType(void)> func = std::bind( *f, arg );
	//*this = async( &func, 0 );
	return *this;
}
 
// Realization of Promise's methods

/*
	Sets success result in promise.
*/
template<class objectType>
void Promise<objectType>::SetValue( objectType const& value ) { 
	sstate->setValue( value );
}

/*
	Sets the flag if an exception occured during the counting the result.
*/
template<class objectType>
void Promise<objectType>::SetException( std::exception* exc ) { 
	sstate->setException( exc );
}

template<class objectType>
Future<objectType> Promise<objectType>::GetFuture() {
	return Future<objectType>( sstate );
}

template<typename objectType>
unsigned __stdcall startProcess( void* params ) {
	std::function<objectType()>* func = ( std::function<objectType()>* ) params;
	Promise<objectType>* pr = ( Promise<objectType>* ) pool.promises[GetCurrentThreadId()];
	std::cout << "I am ready to set value\n";
	objectType res = ( *func )();
	pr->SetValue( res );
	std::cout << "I have set value\n";
	WaitForSingleObject( pr->sstate->gotValue, INFINITE );
	pool.decBuzyThreads(); 
	//WaitForSingleObject( pool.mutex, INFINITE );
	//free( pool.promises[GetCurrentThreadId()] );
	//pool.promises.erase( GetCurrentThreadId() );
	//ReleaseMutex( pool.mutex );
	CloseHandle( GetCurrentThread() );
	_endthreadex( 0 );
	return 0;
}

/*
	objectType is a type of func's return value.
*/
template<typename objectType> 
Future<objectType> async( std::function<objectType(void)>* func, bool synch ) {
	
	if ( synch || pool.enough() ) {
		Promise<objectType> pr;
		Future<objectType> result = pr.GetFuture();
		pr.SetValue( ( *func )() );
		return result;
	} else {
		pool.incBuzyThreads();
		unsigned threadID;
		HANDLE hThread = ( HANDLE ) _beginthreadex( NULL, 0, &startProcess<objectType>, ( void* ) func, 0, &threadID );
		WaitForSingleObject( pool.mutex, INFINITE );
		pool.promises[threadID] = new Promise<objectType>();
		ReleaseMutex( pool.mutex );Promise<objectType>* pr = ( Promise<objectType>* ) pool.promises[threadID];
		return pr->GetFuture();
	}
	
}