#include <iostream>

using namespace std;
void bar() {
int y;
int* z = &y;
std::cout<<z<<endl;
for (int i = 0; i < 56; ++i) {
	++z;
}

printf("%d\n", (*z));
}

void foo() {
int x = 8;
std::cout<<&x<<endl;
bar();
}

int main()
{
foo();
cout << "Hello World!" << endl;
return 0;
}