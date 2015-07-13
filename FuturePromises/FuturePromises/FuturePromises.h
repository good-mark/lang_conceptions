#include "windows.h"
#include <thread> // thread::hardware_concurrency()
#include <functional> // std::bind, std::function
#include <map>

template <typename objectType>
class SharedState {
public:
	SharedState();
	SharedState( const SharedState<objectType>& other ); 
	~SharedState();
    void WaitResult();
    objectType Get();
	bool TryGet( objectType& outValue );

    void setValue( const objectType nvalue );
    void setException( const std::exception& exc );

	HANDLE gotValue;
private:
    objectType value;
    std::exception* error;
	bool ready;
	bool exceptionThrown;

	HANDLE ev;
	HANDLE mutex;
};

template<class objectType>
class Future {
private:
	SharedState<objectType>* sstate;

public:
	Future( SharedState<objectType>* sstate ) : sstate( sstate ) {}
	objectType Get(); // блокирует поток и ожидает возвращение результата
	bool TryGet( objectType& outValue ); // не блокирующая попытка получения

	template<class argType>
	Future& callAfter( Future<argType>& arg );
};

template<class objectType>
class Promise {
public:
	SharedState<objectType>* sstate;

	Promise() : sstate( new SharedState<objectType>() ) {}
	Future<objectType> GetFuture(); 
	void SetValue( objectType const& value ); // установить успешный результат в promise
	void SetException( std::exception* exc ); // установить, что в процессе вычисления отложенного результата произошло исключение
};
 
//	Realization of async

class AsyncThreadPool {
private:
	//std::pair<HANDLE, bool> threadpool[THREADS_NUM]; 
	int buzyThreads;
	int threadsNum;

public:
	AsyncThreadPool();
	~AsyncThreadPool();
	bool enough();
	void incBuzyThreads();
	void decBuzyThreads();
	std::map<unsigned, void*> promises; 
	HANDLE mutex;
};

template<typename objectType> 
unsigned __stdcall startProcess( void* params );

extern AsyncThreadPool pool;

template<typename objectType> // шаблон - тип возвращаемого значения func
Future<objectType> async( std::function<objectType(void)>* func, bool synch = false );

#include "FuturePromises.hpp"

