#include "macros.h"
#include <stdio.h>
#include <iostream>
//#include "structs.h"

class A : public BaseStruct {
public:
	A(): val( ++cnt ) { printf ( "A::A(%d)\n",val ); }
    A( int i ): val( i ) { printf ( "A::A(%d)\n",val ); }
    virtual ~A() {printf ( "A::~A(%d)\n",val ); }
	static int cnt;
	int val;
};

int A::cnt = 0;

int main() {
	A a(1);
	TRY_BLOCK {
		A b(2);
		TRY_BLOCK {
			A c(3);
			THROW_IN_BLOCK("error\n");
			std::cerr << "notreached\n";
		}
		CATCH_BLOCK_FIN {
			std::cerr << "." << exception;
			RETHROW_IN_BLOCK;
		}
		FIN_BLOCK;
		std::cerr << "notreached\n";
    }
	CATCH_BLOCK_FIN {
		std::cerr << ".." << exception;
    }
	FIN_BLOCK;

	return 0;
}
