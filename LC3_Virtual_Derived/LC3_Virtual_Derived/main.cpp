#include <stdio.h>
#include <vector>
#include <string>
#include <map>
#include <iostream>

typedef void ( *func_pointer )();
typedef std::map<std::string, func_pointer> vtable; 

typedef std::vector<vtable*> virtual_table;
vtable bvtable;
vtable dvtable;

//virtual_table bvtable;
//virtual_table dvtable;

void BNamefunc() {}
void DNamefunc() {}

void BaseBoth() { std::cout << "Base::Both a=0" << std::endl; }
void DerivedBoth() { std::cout << "Derived::Both b=1" << std::endl; }
void BaseOnlyBase() { std::cout << "Base::OnlyBase" << std::endl; }
void DerivedOnlyDerived() { std::cout << "OnlyDerived" << std::endl; }

#define concatenate( a, b ) a##b
#define call( BName, func ) BName( func )

#define VIRTUAL_CLASS( BName )	virtual_table concatenate( BName, BName );								\
								typedef struct	BName {													\
													virtual_table* table;								\
													BName() {											\
														table = &concatenate( BName, BName );			\
														if ( !((*table).size()) )						\
															(*table).push_back( &bvtable );				\
													}													\
													BName( std::string func, func_pointer funcPtr ) {							\
														table = &concatenate( BName, BName );			\
														if ( table->size() == 0 )						\
															(*table).push_back( &bvtable );				\
														vtable* temp = (*table)[0];						\
														( *temp ).insert( std::make_pair( func, funcPtr ) ); }

#define END( BName )							} BName;	BName test;		

#define VIRTUAL_CLASS_DERIVED( DName, Parent )	virtual_table concatenate( DName, DName );								\
												typedef struct DName {													\
																	virtual_table* table;								\
																	DName() {											\
																		table = &concatenate( DName, DName );			\
																		if ( !((*table).size()) )						\
																			(*table).push_back( &dvtable );				\
																		(*table).push_back( &bvtable );					\
																	}													\
																	DName( std::string func, func_pointer funcPtr ) {							\
																		table = &concatenate( DName, DName );			\
																		if ( !((*table).size()) )						\
																			(*table).push_back( &dvtable );				\
																		(*table).push_back( &bvtable );					\
																		vtable* temp = (*table)[0];						\
																		( *temp ).insert( std::make_pair( func, funcPtr ) ); }
#define END_DERIVE( DName, Parent )								} DName; DName d;							

#define DECLARE_METHOD( BName, func )	BName b##BName##func( #func, concatenate( BName, func ) );

#define VIRTUAL_CALL( Name, func )		virtual_table Name##func##table = (*( ( *Name ).table ));					\
										for ( unsigned int i = 0; i < Name##func##table.size(); ++i ) {				\
											vtable::iterator it = ( *( Name##func##table[i] ) ).find( #func );		\
											if ( it != ( *( Name##func##table[i] )  ).end() ) 						\
												(*it).second(); break;	}											
										
VIRTUAL_CLASS( Base )
	int a;
END( Base )

DECLARE_METHOD( Base, Both ) 
DECLARE_METHOD( Base, OnlyBase ) 

VIRTUAL_CLASS_DERIVED( Derived, Base )
	int b;
END_DERIVE( Derived, Base )

DECLARE_METHOD( Derived, Both )
DECLARE_METHOD( Derived, OnlyDerived )


int main() {

	Base base;
	base.a = 0;
	Base* s = &base;
	Derived derived;

	// полиморфизм
	Base* reallyDerived = reinterpret_cast<Base*>(&derived);

	VIRTUAL_CALL( s, Both ); //печатает“Base::Both a=0”

	VIRTUAL_CALL( reallyDerived, Both ); //печатает“Derived::Both b=1”

	VIRTUAL_CALL( reallyDerived, OnlyBase ); //печатает“Base::OnlyBase”

	VIRTUAL_CALL( reallyDerived, OnlyDerived ); 
}
