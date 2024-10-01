#include <stdlib.h>
#include "prime5.h"
#include "worker.h"
#include "decomp.h"

void *work(void *arg) {
    worker_t *worker_p = (worker_t*)arg;
    unsigned int number;
    unsigned int *primes;
    number_t *numbers;

    worker_p->count = 0;
    numbers = calloc((worker_p->end - worker_p->begin), sizeof(number_t));
    primes = calloc((worker_p->end - worker_p->begin), sizeof(unsigned int));
    
    for (unsigned int step = 0; step < (worker_p->end - worker_p->begin); step++) {
        if (decomp(worker_p->begin + step, &numbers[step]))
            primes[worker_p->count++] = worker_p->begin + step;
    }

    worker_p->numbers = numbers;
    worker_p->primes = primes;
    
    return arg;
}

