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

static int pack(unsigned int number, unsigned char *bytes);
static int unpack(unsigned char *bytes, unsigned int *value);

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

static int pack(unsigned int number, unsigned char *bytes) {
    int count = 0;

    if (number < ((unsigned int)1 << 8)) {
        // 1 to 7 bits
        count = 1;
        bytes[0] = number & 0x7F;
    } else if (number < ((unsigned int)1 << 15)) {
        // 8 to 15 bits
        count = 2;
        bytes[0] = (number >> 8) & 0x80;
        bytes[1] = number & 0xFF;
    } else if (number < ((unsigned int)1 << 15)) {
        // 16 to 22 bits
        count = 3;
        bytes[0] = (number >> 16) & 0xC0;
        bytes[1] = (number >> 8) & 0xFF;
        bytes[2] = number & 0xFF;
    } else if (number < ((unsigned int)1 << 15)) {
        // 23 to 29 bits
        count = 4;
        bytes[0] = (number >> 24) & 0xE0;
        bytes[1] = (number >> 16) & 0xFF;
        bytes[2] = (number >> 8) & 0xFF;
        bytes[3] = number & 0xFF;
    } else {
        // 32 bits
        count = 5;
        bytes[0] = 0xF0;
        bytes[1] = (number >> 24) & 0xFF;
        bytes[2] = (number >> 16) & 0xFF;
        bytes[3] = (number >> 8) & 0xFF;
        bytes[4] = number & 0xFF;
    }

    return count;
}

static int unpack(unsigned char *bytes, unsigned int *value) {
    unsigned int number;
    int count = 0;

    if (bytes[0] & 0x80 == 0) {
        // One byte
        number = bytes[0];
        count = 1;
    } else if (bytes[0] & 0x40 == 0) {
        // Two bytes, 7 + 8 = 15 bits
        number = bytes[1] | ((bytes[0] & 0x7F) << 8);
        count = 2;
    } else if (bytes[0] & 0x20 == 0) {
        // Three bytes, 6 + 8 + 8 = 22 bits
        number = bytes[2] | (bytes[1] << 8) | ((bytes[0] & 0x3F) << 16);
        count = 3;
    } else if (bytes[0] & 0x10 == 0) {
        // Four bytes, 5 + 8 + 8 + 8 = 29 bits
        number = bytes[3] | (bytes[2] << 8) | (bytes[1] << 16) | ((bytes[0] & 0x1F) << 24);
        count = 4;
    } else {
        // Five bytes, 32 bits
        number = bytes[4] | (bytes[3] << 8) | (bytes[2] << 16) | (bytes[1] << 24);
        count = 5;
    }
    
    *value = number;
    return count;
}
