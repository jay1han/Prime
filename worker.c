#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include "number.h"
#include "prime.h"

void decomp(long original, void *sequence, int do_numbers) {
    long remainder = original;
    void *number;
    void *prime = prime_new();
    long factor = prime_next(prime, NULL);

    if (do_numbers) number = number_new(original);
    while (factor * factor <= remainder) {
        if ((remainder % factor) == 0) {
            if (do_numbers) {
                int exponent = 0;
                do {
                    exponent++;
                    remainder /= factor;
                } while ((remainder % factor) == 0);
                number_addprime(number, prime, exponent);
            } else {
                remainder = 0;
                break;
            }
        }
        factor = prime_next(prime, NULL);
        if (factor == 0) break;
    }
    
    if (remainder == original) {
        if (sequence == NULL) primes_add(original);
        else seq_add(sequence, original);
    }
    else if (do_numbers && (remainder > 1)) {
        number_addfactor(number, remainder, 1);
    }
    
    prime_end(prime);
    if (do_numbers) number_done(number);
}

typedef struct worker_s {
    long first;
    long last;
    pthread_t thread;
    void *sequence;
    int do_numbers;
} worker_t;

static void *work(void *arg) {
    worker_t *worker = (worker_t*)arg;
    long number;
    
    for (number = worker->first; number <= worker->last; number++) {
        decomp(number, worker->sequence, worker->do_numbers);
    }
    return arg;
}

void *worker_start(long first, long last, int do_numbers) {
    worker_t *worker;

    worker = malloc(sizeof(worker_t));
    worker->first = first;
    worker->last  = last;
    worker->do_numbers = do_numbers;
    worker->sequence   = seq_alloc(last - first);

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
