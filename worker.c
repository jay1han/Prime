#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include "worker.h"
#include "decomp.h"

typedef struct worker_s {
    unsigned int begin;
    unsigned int span;
    unsigned int show;
    pthread_t thread;
} worker_t;

static void *work(void *arg) {
    worker_t *worker = (worker_t*)arg;
    unsigned int number = worker->begin;
    
    for (unsigned int step = 0; step < worker->span; step++) {
        if ((number % worker->span) == 0) printf("%u\n", number);
        decomp(number);
        number++;
    }
    return arg;
}

void *worker_start(unsigned int begin, unsigned int span, unsigned int show) {
    worker_t *worker;

    worker = malloc(sizeof(worker_t));
    worker->begin = begin;
    worker->span  = span;
    worker->show  = show;

    pthread_create(&worker->thread, NULL, work, worker);
    return (void*)worker;
}

void worker_join(void *arg) {
    worker_t *worker = (worker_t*)arg;
    pthread_join(worker->thread, NULL);
    free(worker);
}
