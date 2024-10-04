#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include "worker.h"
#include "decomp.h"
#include "prime.h"

typedef struct worker_s {
    unsigned int first;
    unsigned int last;
    unsigned int show;
    pthread_t thread;
    void *sequence;
} worker_t;

static void *work(void *arg) {
    worker_t *worker = (worker_t*)arg;
    unsigned int number;
    
    for (number = worker->first; number <= worker->last; number++) {
        if (worker->show != 0 && (number % worker->show) == 0)
            printf("%u\n", number);
        decomp(number, worker->sequence);
    }
    return arg;
}

void *worker_start(unsigned int first, unsigned int last, unsigned int show) {
    worker_t *worker;

    worker = malloc(sizeof(worker_t));
    worker->first = first;
    worker->last  = last;
    worker->show  = show;
    worker->sequence = seq_alloc(last - first);

    pthread_create(&worker->thread, NULL, work, worker);
    return (void*)worker;
}

void *worker_join(void *arg) {
    worker_t *worker = (worker_t*)arg;
    void *sequence = worker->sequence;
    pthread_join(worker->thread, NULL);
    free(worker);
    return sequence;
}
