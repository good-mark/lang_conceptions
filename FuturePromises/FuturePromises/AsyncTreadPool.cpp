#include "FuturePromises.h"

/*
	Реализация методов класса AsyncThreadPool
*/
AsyncThreadPool::AsyncThreadPool() {
	threadsNum = std::thread::hardware_concurrency();
	buzyThreads = 0;
	mutex = CreateMutex( NULL, FALSE, NULL );
}

AsyncThreadPool::~AsyncThreadPool() {
	CloseHandle( mutex );
	for ( std::map<unsigned, void*>::iterator it = promises.begin(); it != promises.end(); ++it ) {
		free( it->second );
		//pool.promises.erase( GetCurrentThreadId() );
	}
}

bool AsyncThreadPool::enough() {
	return buzyThreads >= threadsNum;
}

/*
	Increases the number of working threads.
*/
void AsyncThreadPool::incBuzyThreads() {
	WaitForSingleObject( mutex, INFINITE ); 
	++buzyThreads;
	ReleaseMutex( mutex );
}

/*
	Decreases the number of working threads.
*/
void AsyncThreadPool::decBuzyThreads() {
	WaitForSingleObject( mutex, INFINITE ); // 1
	--buzyThreads;
	ReleaseMutex( mutex );
}