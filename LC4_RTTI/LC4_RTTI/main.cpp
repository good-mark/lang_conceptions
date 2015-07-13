#include <stdio.h>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <iostream>

typedef struct typeInfo {
	std::string name;
	std::map<std::string, size_t> parents;
} typeInfo;

typedef std::map<std::string, typeInfo> objectTable; 
objectTable table; // (имя, хэш, возможность сравнения)

//#define TYPEID( objectName ) objectName.info

/*
	Добавляет класс в таблицу классов table.
*/
typeInfo* addToClassTable( std::string ClassName ) {
	if ( table.find( ClassName ) == table.end() ) {		
		typeInfo info;										
		info.name = ClassName;								
		table.insert( std::make_pair( ClassName, info ) );	
	}
	return &(table.find( ClassName )->second);
}

typeInfo* getClassInfo( std::string ClassName ) {
	return &(table.find( ClassName )->second);
}

/*
	Добавляет связь между ребенком и родителем в информацию о классе typeInfo, хранящуюся в table.
	Возвращает true в случае успешного добавления, false - при ошибке.
*/
bool addParent( std::string ClassName, std::string ParentName, size_t moved ) {
	objectTable::iterator itClass = table.find( ClassName );
	objectTable::iterator itParent = table.find( ParentName );
	if ( itClass == table.end() || itParent == table.end() )
		return false;
	itClass->second.parents.insert( std::make_pair( ParentName, moved ) );
	return true;
}

/*
	Делает обход в глубину по связям из таблицы table.
*/
bool checkHasParent( std::string child, std::string parent, size_t* moved ) {
	
	std::map<std::string, size_t>::iterator it = getClassInfo( child )->parents.begin();
	for ( it; it != getClassInfo( child )->parents.end(); ++it )
		if ( it->first == parent ) {
			*moved += it->second;
			return true;
		}

	it = getClassInfo( child )->parents.begin();
	for ( it; it != getClassInfo( child )->parents.end(); ++it )
		if ( checkHasParent( it->first, parent, moved ) == true ) {
			*moved += it->second;
			return true;
		}
	return false; 
}

/*
	Указываем текущий тип указателя OldClass, чтобы знать тип аргумента функции.
	Возвращает true в случае успеха, false - если приведение типов выполнить не удалось.
*/
template<class OldClass, class NewClass>
NewClass* cast( OldClass* ptr ) {
	NewClass a;
	std::string NewClassName = a.info->name;
	size_t moved = 0;
	if ( checkHasParent( ptr->getInfo().name, NewClassName, &moved ) == true ) {
		NewClass* res = reinterpret_cast<NewClass*>( (void *) ((size_t) ptr + moved ) );
		res->info = getClassInfo( NewClassName );
		return res;
	}
	else 
		return nullptr;
}

/*
	Переопределение конструктора с параметрами у каждого класса, 
	чтобы можно было добавлять информацию в таблицу классов table
	посредством создания экземпляра класса в INIT_CLASS и ADD_PARENT.
*/
#define LABEL_CLASS( ClassName )	public:															\
										typeInfo* info;												\
										virtual typeInfo getInfo() {									\
											info = getClassInfo( #ClassName );				\
											return *info;										\
										}							\
										ClassName() { info = addToClassTable( #ClassName ); }		\
										ClassName( int a, std::string ParentName ) {				\
											info = addToClassTable( #ClassName );					\
											if ( a != 0 ) addParent( #ClassName, ParentName, a ); 	\
										}															\
									private:													

#define INIT_CLASS( ClassName ) ClassName ClassName##a( 0, "any string" ); 
#define ADD_PARENT( ClassName, ParentName )	ClassName ClassName##ParentName##child;											\
											size_t ClassName##ParentName##moved =											\
												(size_t) ( static_cast<ParentName*>( &ClassName##ParentName##child ) ) -	\
																			( size_t ) ( &ClassName##ParentName##child );	\
											ClassName ClassName##ParentName##b( ClassName##ParentName##moved, #ParentName );

#define DYNAMIC_CAST( ptr, OldClassName, NewClassName ) cast<OldClassName, NewClassName>( ptr );

/*
	На этом часть, которая должна быть в .h, заканчивается, 
	и начинается пример, показывающий, как все работает.
*/

/*
	В каждом классе обязательно должен быть реализован конструктор по умолчанию.
	Кроме того, перед объявлением полей и методов класса необходимо "пометить" класс 
	LABEL_CLASS( ClassName ), а после проинициализировать класс INIT_CLASS 
	и добавить связь с классом-родителем с помощью ADD_PARENT.
*/
class FirstBase {

     LABEL_CLASS( FirstBase );

public:

     int firstbase;

     void Print() { std::cout << "FirstBase: " << firstbase << std::endl; }

};

INIT_CLASS( FirstBase )

 

class SecondBase {

     LABEL_CLASS( SecondBase );

public:

     int secondbase;

     void Print() { std::cout << "SecondBase: " << secondbase << std::endl; }

};

INIT_CLASS( SecondBase )

 

class TestClass : public FirstBase, public SecondBase {

     LABEL_CLASS( TestClass );

public:

     int a;

     void Print() { std::cout << "TestClass: " << a << std::endl; }

};

INIT_CLASS( TestClass )

ADD_PARENT( TestClass, FirstBase )

ADD_PARENT( TestClass, SecondBase )

int main() {

     TestClass d;

     d.firstbase = 11;

     d.secondbase = 13;

     d.a = 17;

     FirstBase* a = &d;

     SecondBase* b = DYNAMIC_CAST( a, FirstBase, SecondBase );

     b->Print(); // ожидаю строку "SecondBase: 13"

     return 0;

}