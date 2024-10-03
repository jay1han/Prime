#include <stdlib.h>
#include <stdio.h>
#include "prime.h"

// Object-like structure to retrieve contiguous list of primes
// The list is the data, and callers use an iterator to go through the list

typedef struct iterator_s {
    int part, offset;
} iterator_t;

static struct {
    int part, offset;
    unsigned int *primes[1000];
} self;

#define PART 1000000

void primes_init() {
    self.part = 0;
    self.offset = 0;
    self.primes[0] = calloc(sizeof(unsigned int), PART);
}

void primes_add(unsigned int prime) {
    self.primes[self.part][self.offset] = prime;
    if (self.offset == PART - 1) {
        self.primes[++self.part] = calloc(sizeof(unsigned int), PART);
        self.offset = 0;
    } else self.offset++;
}

unsigned int primes_count() {
    return self.part * PART + self.offset;
}

void primes_output(char *filename) {
    FILE *file = fopen(filename, "w");
    void *iterator = prime_new();
    unsigned int prime = prime_next(iterator);
    
    while (prime != 0) {
        fprintf(file, "%u\n", prime);
        prime = prime_next(iterator);
    }
    fclose(file);
}

void *prime_new() {
    iterator_t *iterator = (iterator_t*)malloc(sizeof(iterator_t));
    iterator->offset = 0;
    iterator->part = 0;
    return (void*)iterator;
}

unsigned int prime_next(void *arg) {
    iterator_t *this = (iterator_t*)arg;
    if (this->part == self.part && this->offset == self.offset) return 0;
    
    unsigned int prime = self.primes[this->part][this->offset];
    if (this->offset == PART - 1) {
        this->part++;
        this->offset = 0;
    } else this->offset++;
    return prime;
}

void prime_end(void *arg) {
    free(arg);
}
