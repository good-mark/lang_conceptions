#include "structs.h"
#include <assert.h>

static TryStruct* rootSlice = nullptr;  
//extern int ThrowException( const char *str );

BaseStruct::BaseStruct() {
	if ( rootSlice != nullptr ) {
		prev = rootSlice->objs;
		rootSlice->objs  = this;
	}
}

BaseStruct::~BaseStruct() {
	unreg();
}

void BaseStruct::unreg() {
	BaseStruct* temp = reinterpret_cast<BaseStruct*>( ~0 );
	if ( rootSlice != nullptr && prev != temp ) {
		rootSlice->objs = prev;
		prev = reinterpret_cast<BaseStruct*>( ~0 );
	}
}

TryStruct::TryStruct() {
  objs = NULL;
  prev = rootSlice;
  rootSlice = this;
}

TryStruct::~TryStruct() {
  rootSlice = prev ;
}

static int PopException() {
  TryStruct* ts = rootSlice;
  assert( ts != NULL );
  rootSlice = ts->prev ;
  return 0;
}

int ThrowException( const char *str, bool popstate ) {
	if ( str == NULL )
		return -1;
	TryStruct *sl = rootSlice;
	BaseStruct *obj = rootSlice->objs ;
	while ( NULL != obj ) {
		BaseStruct *tmp = obj;
		obj = obj->prev ;
		tmp->~BaseStruct();
	}
	if ( popstate )
		PopException();
	longjmp( sl->buf , int( str ) );	
	return 0;
}
