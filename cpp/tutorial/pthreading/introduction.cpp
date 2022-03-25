#include <pthread.h>
#include <iostream>

using namespace std;

#define NUM_THREADS 5

struct thread_data {
    int thread_id;
    char *message;
};

void *PrintHello(void *threadid) {
    long tid;
    tid = (long)threadid;
    cout << "Hello World! Thread ID " << tid << endl;
    pthread_exit(NULL);
}

int main(int argc, char const *argv[]) {
    pthread_t thread[NUM_THREADS];
    int rc;
    int i;

    for (i = 0; i < NUM_THREADS; i++) {
        cout << "main(): creating thread, " << i << endl;
        rc = pthread_create(&thread[i], NULL, PrintHello, (void *)i);

        if (rc) {
            cout << "Error: unable to create thread," << rc << endl;
            exit(-1);
        }
    }
    
    pthread_exit(NULL);
}
