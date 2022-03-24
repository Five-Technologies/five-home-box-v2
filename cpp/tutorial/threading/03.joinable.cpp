/*  Join
    - Once a thread is started, we wait for this thread to finish by calling join() on the thread object.
    - Double join will result into program termination.
    - If needed, we should check thread is joinable before joigning by using joinable().

    Detach
    - This is used to detach the newly created thread from the parent thread.
    - Always check before detaching a thread that it is joinable otherwise we may end up double detaching and double detach() will result into program termination.
    - If we have detached thread and main function is returning then the detached thread execution is suspended.

    Either join() and detach() should be called on thread object, otherwise during thread object's destructor it will terminate the program. Because inside destructor it checks if thread is still joinable. If yes then it terminates the program. 
*/

#include <iostream>
#include <chrono>
#include <thread>
using namespace std;

void run(int x)
{
    while (x --> 0)
        cout << x << " Cpp Nuts\n";
    this_thread::sleep_for(chrono::seconds(3));
    cout << "thread finished\n";
}

void hello(int x)
{
    while (x --> 0)
        cout << x << " Hello\n";
    this_thread::sleep_for(chrono::seconds(2));
    cout << "thread finished\n";
}

int main()
{
    thread t1(run, 2);
    thread t2(hello, 3);
    cout << "main()\n";
    
    t2.detach();
    cout << "after detach\n";
    t1.join();

    if (t1.joinable())
        t1.join();
    else
        cout << "t1 already joined\n";

    cout << "after joun\n";

    return 0;
}
