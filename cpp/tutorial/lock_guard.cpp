#include <iostream>
#include <thread>
#include <mutex>
#include <stdexcept>

std::mutex mtx;

void printEven(int x) {
    if (x % 2 == 0) {
        std::cout << x << " is even\n";
    } else {
        throw std::logic_error("not even");
    }
}

void printThreadId(int id) {
    try {
        std::lock_guard<std::mutex> lck(mtx);
        printEven(id);
    } catch (std::logic_error&) {
        std::cout << "[exception caught]\n";
    }
}

int main(int argc, char const *argv[])
{
    std::thread threads[10];

    for (int i=0; i<10; ++i) {
        threads[i] = std::thread(printThreadId, i+1);
    }

    for (auto& th : threads) th.join();
    return 0;
}
