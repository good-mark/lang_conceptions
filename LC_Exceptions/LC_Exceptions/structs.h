#include <stdlib.h>
#include <csetjmp>

struct BaseStruct {
    BaseStruct();
    virtual ~BaseStruct();
    void unreg();
    BaseStruct* prev;  
};

struct TryStruct {    
    TryStruct();
    ~TryStruct();    
    jmp_buf buf;    
    TryStruct* prev;    
    BaseStruct* objs;  
};

static int PopException();

int ThrowException( const char *str, bool popstate );
