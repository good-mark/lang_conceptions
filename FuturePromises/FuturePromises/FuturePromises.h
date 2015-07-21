#include "windows.h"
#include <thread> // for thread::hardware_concurrency()
#include <functional> // for std::bind, std::function
#include <map>

/*
	A supporting class, contain the info about objects.
	Future and Promise classes use this info and methods of SharedState. 
*/
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
	objectType Get(); 
	bool TryGet( objectType& outValue ); 

	template<class argType>
	Future& callAfter( Future<argType>& arg );
};

template<class objectType>
class Promise {
public:
	SharedState<objectType>* sstate;

	Promise() : sstate( new SharedState<objectType>() ) {}
	Future<objectType> GetFuture(); 
	void SetValue( objectType const& value ); 
	void SetException( std::exception* exc ); 
};
 
//	Realization of async

class AsyncThreadPool {
private:
	int buzyThreads;  
	int threadsNum; 

public:
	AsyncThreadPool();
	~AsyncThreadPool();
	bool enough(); // 
	void incBuzyThreads(); 
	void decBuzyThreads(); 
	std::map<unsigned, void*> promises; 
	HANDLE mutex;
};

extern AsyncThreadPool pool;

template<typename objectType> 
unsigned __stdcall startProcess( void* params );

template<typename objectType> 
Future<objectType> async( std::function<objectType(void)>* func, bool synch = false );

#include "FuturePromises.hpp"



