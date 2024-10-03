#include <stdlib.h>
#include <string.h>
#include "number.h"

typedef struct __attribute__((packed)) factor_s {
    unsigned int factor;
    unsigned char exponent;
} factor_t;

typedef struct __attribute__((packed)) packed_s {
    unsigned char divisors;
    factor_t *factors;
} packed_t;

typedef struct number_s {
    unsigned int number;
    unsigned int divisors;
    factor_t factors[10];
} number_t;

static struct {
    packed_t *numbers;
    unsigned int first, last;
} self;

void numbers_init(unsigned int first, unsigned int last) {
    self.first   = first;
    self.last    = last;
    self.numbers = calloc(sizeof(number_t), last - first + 1);
}

void numbers_output(char *filename) {
    // TODO
}

void numbers_close() {
    free(self.numbers);
    self.numbers = NULL;
}

void *number_new(unsigned int number) {
    if (self.last == 0 || number < self.first || number > self.last) return NULL;
    number_t *this = (number_t*)malloc(sizeof(number_t));
    this->number   = number;
    this->divisors = 0;
    return (void*)this;
}

void number_addfactor(void *arg, unsigned int factor, int exponent) {
    number_t *this = (number_t*)arg;
    this->factors[this->divisors].factor   = factor;
    this->factors[this->divisors].exponent = exponent;
    this->divisors++;
}

void number_done(void *arg) {
    number_t *this = (number_t*)arg;
    if (this->divisors == 0) return;

    unsigned int offset = this->number - self.first;
    self.numbers[offset].divisors = this->divisors;
    self.numbers[offset].factors = malloc(sizeof(factor_t) * this->divisors);
    memcpy(self.numbers[offset].factors, this->factors, sizeof(factor_t) * this->divisors);
    free(this);
}
