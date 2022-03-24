/*  Mutex = MUTual EXclusion

    Race condition
    - A race condition is a situation where two or more threads/process happen to change a commomn data at the same time.
    - If there is a race condition then we have to project it and the protected section is called critical section/region.

    Mutex
    - Mutex is used to avoid race condition.
    - We use lock(), unlock() on mutex to avoid race condition.
*/

#include <iostream>
#include <thread>
#include <mutex>

int myAMount = 0;
std::mutex m;

void addMoney() // this is the critical region
{
    m.lock();
    ++ myAMount; // this is the critical section
    m.unlock();
}

int main(int argc, char const *argv[])
{
    /*  Threads start running together..
        The first that reaches the CPU locks it (there is always one thread faster).
        Example:
            t1 arrives first. It locks the CPU and t2 can't reach the CPU because it's already locked.
            t1 runs all code after lock() and t2 waits for the t1's unlock().
            Once the t1 executed all instructions, it unlocks the mutex and allow t2 to reach the CPU.
            t2 executes its code.
    */
    std::thread t1(addMoney); // addMoney is the function
    std::thread t2(addMoney);

    t1.join();
    t2.join();

    std::cout << myAMount << '\n';

    return 0;
}
