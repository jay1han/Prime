#include "number.h"

typedef struct __attribute__((packed)) factor_s {
    unsigned int factor;
    unsigned char exponent;
} factor_t;

typedef struct __attribute__((packed)) number_s {
    unsigned char divisors;
    factor_t *factors;
} number_t;

void numbers_init(unsigned int begin, unsigned int span) {
}

void numbers_output(char *filename) {
}

void numbers_close() {
}

void *number_new(unsigned int number) {
}

void number_isprime(void *number) {
}

void number_addfactor(void *number, unsigned int factor, int exponent) {
}

void number_done(void *number) {
}
