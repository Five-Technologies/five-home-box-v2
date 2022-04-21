/* CELEBP44 */

#define _OPEN_THREADS
#define _OPEN_SYS // Identify __MUTEX_RECURSIVE
#define LOOP_CONSTANT 100000
#define THREADS 10

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

using namespace std;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int i, j ,k, l;
int userlock = 1;

void *threadFunc(void *param) {
    int loop = 0;
    int rc;

    for (loop=0; loop<LOOP_CONSTANT; ++loop) {
        if (uselock) {
            rc = pthread_mutex_lock(&mutex);
            cout << "phtread_mutex_lock()\n";
        }
        ++i; ++j; ++k; ++l;
        if (uselock) {
            rc = pthread_mutex_unlock(&mutex);
            cou << "pthread_mutex_unlock()\n";
        }
    }
    return NULL;
}

int main(int argc, char **argv) {
    pthread_t threadid[THREADS];
    int rc = 0;
    int loop = 0;
    pthread_attr_t pta;

    printf("Entering testcase\n");
    printf("Give any number of parameters to show data corruption\n");
    if (argc != 1)  {
        printf("A parameter was specified, no serialization is being done!\n");
        uselock = 0;
    }

    pthread_mutexattr_t attr;
    pthread_mutex_t mutex;

    if (pthread_mutexattr_init(&attr) != 0) {
        perror("pthread_mutex_attr_init() error");
        exit(1);
    }
    cout << "attr initialized\n";

    if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) != 0) {
        perror("pthread_mutexattr_setType() error");
        exit(2);
    }
    cout << "attr types set\n";

    if (pthread_mutex_init(&mutex, &attr) != 0) {
        perror("pthread_mutex_init() error");
        exit(3);
    }
    cout << "mutex initialized\n";

    if (pthread_mutexattr_destroy(&attr) != 0) {
        perror("pthread_mutex_attr_init() error");
        exit(4);
    }
    cout << "attr destroyed\n";

    return 0;
}
