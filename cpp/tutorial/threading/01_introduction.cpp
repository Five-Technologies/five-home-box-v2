/*  In every application there is a default thread which is main(), in sid ethis we create other threads.
    A thread is also known as a lightweight process. Idea is achieve parallelism by dividing a process into multiple threads.
*/

#include <iostream>
#include <thread>
#include <chrono>
#include <algorithm>

using namespace std;
using namespace chrono;

typedef unsigned long long ull;
ull evenSum{ 0 }, oddSum{ 0 };

void findEven(ull start, ull end);
void findOdd(ull start, ull end);

int main()
{
    ull start{ 0 }, end{ 1900000000 };
    auto startTime = high_resolution_clock::now();


    // start 2 threads that run at parallely
    thread t1(findEven, start, end); // start running the function
    thread t2(findOdd, start, end);
    t1.join(); // join this thread to the parent thread to wait it before ending the program
    t2.join();

    cout << "OddSum: " << evenSum << '\n';
    cout << "EvenSum: " << oddSum << '\n';

    auto stopTime = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stopTime - startTime);

    cout << "Duration: " << duration.count()/1000000 << " seconds" << '\n';
}

void findEven(ull start, ull end)
{
    for (ull i = start; i <= end; ++i)
    {
        if ((i & 1) == 1)
            oddSum += i;
    }
}

void findOdd(ull start, ull end)
{
    for (ull i = start; i <= end; ++i)
    {
        if ((i & 1) == 0)
            evenSum += i;
    }
}
