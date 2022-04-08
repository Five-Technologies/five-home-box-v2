#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

std::mutex mtx;

void printBlock(int n, char c) {
    // mtx.lock();
    
    for (int i = 0; i < n; ++i) {
        std::cout << c;
    }

    std::cout << '\n';
    // mtx.unlock();
}

int main(int argc, char const *argv[])
{
    std::thread th1(printBlock, 50, '*');
    std::thread th2(printBlock, 50, '$');

    th2.join();
    th1.join();

    return 0;
}
