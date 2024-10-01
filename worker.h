#ifndef _WORKER_H_
#define _WORKER_H_

typedef struct worker_s {
    // inputs
    unsigned int begin;
    unsigned int end;
    // outputs
    unsigned int count;
    unsigned int *primes;   // must be packed
    number_t *numbers;
} worker_t;

#endif
