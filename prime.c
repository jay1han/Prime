#include <stdlib.h>
#include "prime.h"

// Object-like structure to retrieve contiguous list of primes

unsigned int begin[100000], length[100000];
unsigned int *list[100000];

typedef struct prime_s {
    unsigned int number;
    int part, offset;
} prime_t;

static int parts = 0;

// Add a list of "count" primes to our total list
void prime_add(unsigned int *primes, int count) {
    begin[parts] = primes[0];
    length[parts] = count;
    list[parts] = primes;
    parts++;
}

// Create a new object to iterate the list
void *prime_new() {
    prime_t *p = (prime_t*)malloc(sizeof(prime_t));
    p->number = 1;
    p->part = -1;
    p->offset = -1;
    return (void*)p;
}

// Get the next prime or 0 if none
unsigned int prime_next(void *arg) {
    prime_t *p = (prime_t*)arg;
    if (++(p->offset) >= length[p->part]) {
        if (++(p->part) >= parts) return 0;
        else p->offset = 0;
    }
    p->number = list[p->part][p->offset];
    return p->number;
}

// Destroy the objet
void prime_end(void *p) {
    free(p);
}
