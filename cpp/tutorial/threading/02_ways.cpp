// There are 5 different types we can create in C++ll using callable objects.
// If we create multiple threads at them same time, it doesn't guarantee which one will start first.

#include <iostream>
#include <thread>

using namespace std;

// Function pointer: this is the very basic form of thread creation.
void fun(int x)
{
    while (x --> 0) // x >= 0 and x--
    {
        cout << x;
    }
}

class Base
{
    public:
        // Functor (function object)
        void operator ()(int x)
        {
            while (x --> 0)
            {
                cout << x;
            }
        }
        // Non-static member function
        void run(int x)
        {
            while (x --> 0)
                cout << x;
        }
        // Static member function
        static void runStatic(int x)
        {
            while (x --> 0)
                cout << x;
        }

};

int main()
{
    cout << "\nFunction: ";
    thread t1(fun, 10);
    t1.join();
    
    // Lambda function: we can directly inject lambda at thread creation time.
    auto lambdaFun = [](int x)
    {
        while(x --> 0)
        {
            cout << x;
        }
    };
    
    cout << "\nLambda: ";
    thread t2(lambdaFun, 2);
    t2.join();

    cout << "\nFunctor: ";
    thread t3((Base()), 3);
    t3.join();

    cout << "\nNon-static: ";
    Base b;
    thread t4(&Base::run, &b, 4);
    t4.join();

    cout << "\nStatic: ";
    thread t5(&Base::runStatic, 5);
    t5.join();
    
    cout << endl << endl;
    return 0;
}