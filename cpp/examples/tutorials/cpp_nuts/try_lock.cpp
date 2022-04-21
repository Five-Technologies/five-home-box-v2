#include <iostream>
#include <thread>
#include <mutex>

using namespace std;

int counter(0);
mutex m;

void increateTheCounterFor100000Time() {
    for (int i=0; i<100000; ++i) {
        if (m.try_lock()) {
            ++ counter;
            m.unlock();
        }
    }
}
int main() {
    thread t1(increateTheCounterFor100000Time);
    thread t2(increateTheCounterFor100000Time);

    t1.join();
    t2.join();

    printf("%i\n", counter);
    return 0;
}