#include <stdlib.h>
#include "prime.h"

// Object-like structure to retrieve contiguous list of primes
// The list is the data, and callers use an iterator to go through the list

typedef struct prime_s {
    unsigned int number;
    int part, offset;
} prime_t;

static int parts = 0;
unsigned int **primes;

void primes_init(unsigned int span) {
}

void primes_add(unsigned int prime) {
}

void primes_defrag() {
}

unsigned int primes_count() {
}

void primes_output(char *filename) {
}

void *prime_new() {
}

unsigned int prime_next(void *iterator) {
}

void prime_end(void *iterator) {
}
