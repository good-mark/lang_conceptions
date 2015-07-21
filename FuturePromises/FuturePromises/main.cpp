#include "FuturePromises.h"
#include <process.h>
#include <iostream>

int inc( const int a ) {
	return a + 1;
}

unsigned Counter; 
unsigned __stdcall SecondThreadFunc( void* pArguments )
{
    printf( "In second thread...\n" );
	Future<int>* f = ( Future<int>* ) pArguments;
	std::cout << "In SecondThreadFunc " << f->Get() << std::endl;
    while ( Counter < 100000 )
        Counter++;
	CloseHandle( GetCurrentThread() );
    _endthreadex( 0 );
    return 0;
} 

int main() {
	// simple one-thread example
	/*Promise<int> pr;
	Future<int> check = pr.GetFuture();
	pr.SetValue( inc( 10 ) );
	int res = check.Get();
	if( res )
        std::cout << "Success\n" << res;
    else
        std::cout << "Error\n";*/

	// 2-threads example
	Promise<int> pr;
	Future<int> check = pr.GetFuture();
	unsigned threadID;
	HANDLE hThread = ( HANDLE ) _beginthreadex( NULL, 0, &SecondThreadFunc, &check, 0, &threadID );
	pr.SetValue( 7 ); // If this string is commented, setted value won't be printed
	WaitForSingleObject( hThread, INFINITE );
	CloseHandle( hThread );

	// asynch example
	std::function<int(void)> f11 = std::bind( inc, 11 );
	std::function<int(void)> f13 = std::bind( inc, 13 );
	std::function<int(void)> f19 = std::bind( inc, 19 );
	Future<int> fut11 = async<int>( &f11, 0 );
	Future<int> fut13 = async<int>( &f13, 0 );
	Future<int> fut19 = async<int>( &f19, 1 );
	std::cout << "####### Yeap, I've done it! " << fut11.Get() << ". And again: " << fut13.Get() << std::endl;
	std::cout << "####### Synch: " << fut19.Get() << std::endl;
	/*
	// callAfter example
	Promise<int> pr1;
	Promise<int> pr2;
	Promise<int> pr3;
	Future<int> f1 = pr1.GetFuture();
	Future<int> f2 = pr2.GetFuture();
	Future<int> f3 = pr3.GetFuture();
	pr1.SetValue( 55 );
	pr2.SetValue( 65 );
	pr3.SetValue( 75 );
	f3.callAfter( f2.callAfter( f1 ) );
	std::cout << "####### callAfter values: " << f1.Get() << " " << f2.Get() << " " << f3.Get() << std::endl;*/
	return 0;
}
