#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "number.h"
#include "prime.h"
#include "flexint.h"
#include "longint.h"

typedef struct factor_s {
    long prime_i;
    long factor;
    unsigned char exponent;
} factor_t;

typedef struct number_s {
    int divisors;
    factor_t factors[10];
} number_t;

static struct {
    number_t *numbers;
    long first, last;
} self;

void numbers_init(long first, long last) {
    self.first   = first;
    self.last    = last;
    self.numbers = calloc(sizeof(number_t), last - first + 1);
}

void numbers_write(char *filename, int format) {
    long first = self.first;
    FILE *file = fopen(filename, "ab");
    unsigned char bytes[10];

    for (long index = 0; index <= self.last - self.first; index++) {
        if (format == FORMAT_DEGREE) {
            unsigned char degree = 0;
            for (int divisor = 0; divisor < self.numbers[index].divisors; divisor++)
                degree += self.numbers[index].factors[divisor].exponent;
            fwrite(&degree, 1, 1, file);
            
        } else {    // FORMAT_INDEX
            fwrite(&self.numbers[index].divisors, 1, 1, file);
            for (int divisor = 0; divisor < self.numbers[index].divisors; divisor++) {
                if (format == FORMAT_PLAIN) {
                    fwrite(bytes, 1,
                           flex_fold(self.numbers[index].factors[divisor].factor, bytes),
                           file);
                } else {
                    if (self.numbers[index].factors[divisor].prime_i > 0) {
                        fwrite(bytes, 1,
                               flex_fold(self.numbers[index].factors[divisor].prime_i, bytes),
                               file);
                    } else {
                        bytes[0] = 0xFF;
                        fwrite(bytes, 1,
                               flex_fold(self.numbers[index].factors[divisor].factor, bytes + 1) + 1,
                               file);
                    }
                }
                fwrite(&self.numbers[index].factors[divisor].exponent, 1, 1, file);
            }
        }
    }
    
    fclose(file);
}

void numbers_close() {
    free(self.numbers);
    self.numbers = NULL;
}

void *number_new(long number) {
    if (self.last == 0 || number < self.first || number > self.last) return NULL;
    number_t *this = &self.numbers[number - self.first];
    this->divisors = 0;
    return (void*)this;
}

inline void number_addprime(void *arg, void *prime, unsigned char exponent) {
    number_t *this = (number_t*)arg;
    this->factors[this->divisors].prime_i  = prime_index(prime);
    this->factors[this->divisors].factor   = prime_value(prime);
    this->factors[this->divisors].exponent = exponent;
    this->divisors++;
}

inline void number_addfactor(void *arg, long factor, unsigned char exponent) {
    number_t *this = (number_t*)arg;
    long prime_i;
    
    this->factors[this->divisors].prime_i  = 0;
    this->factors[this->divisors].factor   = factor;
    this->factors[this->divisors].exponent = exponent;
    this->divisors++;
}

void number_done(void *arg) {
}

void number_print(long number) {
    if (self.last == 0 || number < self.first || number > self.last) {
        fprintlf(stdout, "% is not in the list\n", number);
        return;
    }
    
    number_t *this = &self.numbers[number - self.first];

    if (this->divisors == 0) fprintlf(stdout, "% P\n", number);
    else {
        fprintlf(stdout, "%=", number);
        for (int i = 0; i < this->divisors; i++) {
            fprintlf(stdout, " %", this->factors[i].factor);
            printf("^%d", this->factors[i].exponent);
        }
        printf("\n");
    }
}
