#include <iostream>
using namespace std;

class MyClass {
    public:
        // MyClass(){}
        void print(){}
};

int main(int argc, char const *argv[])
{
    int *ptr = nullptr; // declare and initialize
    int i = 5;
    ptr = &i;
    int j = *ptr;

    cout << "pointer address : " << &ptr << endl;
    cout << "pointer value   : " << ptr << endl;
    cout << "pointer pointer : " << *ptr << endl;

    cout << "variable address: " << &i << endl;
    cout << "variable value  : " << i << endl;

    MyClass *myClass = new MyClass();
    myClass->print();
    delete myClass; // delete object

    const char* str = "Hello World";
    const int c = 1;
    const int* pconst = &c;
    const int c2 = 2;
    pconst = &c2;

    return 0;
}
