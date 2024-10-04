#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "number.h"
#include "prime.h"

typedef struct __attribute__((packed)) factor_s {
    unsigned int factor;
    unsigned char exponent;
} factor_t;

typedef struct __attribute__((packed)) number_s {
    unsigned char divisors;
    factor_t factors[10];
} number_t;

static struct {
    number_t *numbers;
    unsigned int first, last;
} self;

void numbers_init(unsigned int first, unsigned int last) {
    self.first   = first;
    self.last    = last;
    self.numbers = calloc(sizeof(number_t), last - first + 1);
}

void numbers_write(char *filename) {
    unsigned int first = self.first;
    FILE *file = fopen(filename, "ab");

    for (unsigned int index = 0; index <= self.last - self.first; index++) {
        fwrite(&self.numbers[index],
               // TODO more compact
               sizeof(factor_t) * self.numbers[index].divisors + 1,
               1, file);
    }
    
    fclose(file);
}

void numbers_print(char *filename) {
    unsigned int first = self.first;
    FILE *file = fopen(filename, "a");

    for (unsigned int index = 0; index <= self.last - self.first; index++) {
        if (self.numbers[index].divisors == 0)
            fprintf(file, "%uP\n", self.first + index);
        else {
            fprintf(file, "%u:", self.first + index);
            for (int divisor = 0; divisor < self.numbers[index].divisors; divisor++) {
                fprintf(file, " %u^%d",
                        self.numbers[index].factors[divisor].factor,
                        self.numbers[index].factors[divisor].exponent);
            }
            fprintf(file, "\n");
        }
    }
    
    fclose(file);
}

void numbers_close() {
    free(self.numbers);
    self.numbers = NULL;
}

void *number_new(unsigned int number) {
    if (self.last == 0 || number < self.first || number > self.last) return NULL;
    number_t *this = &self.numbers[number - self.first];
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
}
