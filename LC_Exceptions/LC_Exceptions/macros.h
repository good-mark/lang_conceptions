#include "structs.h"

#define	TRY_BLOCK		{																	\
							TryStruct ts;													\
							const char* exception = ( const char * ) setjmp( ts.buf );		\
							if ( exception == NULL ) {

#define CATCH_BLOCK_FIN		} else { 

#define FIN_BLOCK			}																\
						}
 
#define THROW_IN_BLOCK( exc ) ThrowException( exc, 0 ); 

#define RETHROW_IN_BLOCK ThrowException( exception, 1 );

