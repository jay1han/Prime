#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "number.h"
#include "prime.h"

typedef struct factor_s {
    int prime_i;
    long factor;
    int exponent;
} factor_t;

typedef struct number_s {
    int divisors;
    factor_t factors[10];
} number_t;

static struct {
    number_t *numbers;
    long first, last;
} self;

static int pack(long number, unsigned char *bytes);
static int unpack(unsigned char *bytes, long *value);

void numbers_init(long first, long last) {
    self.first   = first;
    self.last    = last;
    self.numbers = calloc(sizeof(number_t), last - first + 1);
}

void numbers_write(char *filename) {
    long first = self.first;
    FILE *file = fopen(filename, "ab");
    unsigned char bytes[10];

    for (long index = 0; index <= self.last - self.first; index++) {
        fwrite(&self.numbers[index].divisors, 1, 1, file);
        for (int divisor = 0; divisor < self.numbers[index].divisors; divisor++) {
            if (self.numbers[index].factors[divisor].prime_i == -1) {
                bytes[0] = 0xFF;
                fwrite(bytes, 1, pack(self.numbers[index].factors[divisor].factor, bytes + 1) + 1, file);
            }
            else
                fwrite(bytes, 1, pack(self.numbers[index].factors[divisor].prime_i, bytes), file);
            fwrite(&self.numbers[index].factors[divisor].exponent, 1, 1, file);
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

inline void number_addprime(void *arg, int prime_i, int exponent) {
    number_t *this = (number_t*)arg;
    this->factors[this->divisors].prime_i  = prime_i;
    this->factors[this->divisors].exponent = exponent;
    this->divisors++;
}

inline void number_addfactor(void *arg, long factor, int exponent) {
    number_t *this = (number_t*)arg;
    this->factors[this->divisors].prime_i  = -1;
    this->factors[this->divisors].factor   = factor;
    this->factors[this->divisors].exponent = exponent;
    this->divisors++;
}

void number_done(void *arg) {
}

inline static int pack(long number, unsigned char *bytes) {
    int count = 0;

    if (number < ((long)1 << 7)) {
        // 1 to 7 bits
        count = 1;
        bytes[0] = number;
    } else if (number < ((long)1 << 14)) {
        // 8 to 14 bits
        count = 2;
        bytes[0] = (number >> 8) | 0x80;
        bytes[1] = number & 0xFF;
    } else if (number < ((long)1 << 21)) {
        // 15 to 21 bits
        count = 3;
        bytes[0] = (number >> 16) | 0xC0;
        bytes[1] = (number >> 8) & 0xFF;
        bytes[2] = number & 0xFF;
    } else if (number < ((long)1 << 28)) {
        // 22 to 28 bits
        count = 4;
        bytes[0] = (number >> 24) | 0xE0;
        bytes[1] = (number >> 16) & 0xFF;
        bytes[2] = (number >> 8) & 0xFF;
        bytes[3] = number & 0xFF;
    } else if (number < ((long)1 << 35)) {
        // 29 to 35 bits
        count = 5;
        bytes[0] = (number >> 32) | 0xF0;
        bytes[1] = (number >> 24) & 0xFF;
        bytes[2] = (number >> 16) & 0xFF;
        bytes[3] = (number >> 8) & 0xFF;
        bytes[4] = number & 0xFF;
    } else if (number < ((long)1 << 42)) {
        // 36 to 42 bits
        count = 6;
        bytes[0] = (number >> 40) | 0xF8;
        bytes[1] = (number >> 32) & 0xFF;
        bytes[2] = (number >> 24) & 0xFF;
        bytes[3] = (number >> 16) & 0xFF;
        bytes[4] = (number >> 8) & 0xFF;
        bytes[5] = number & 0xFF;
    } else if (number < ((long)1 << 49)) {
        // 43 to 49 bits
        count = 7;
        bytes[0] = (number >> 48) | 0xFC;
        bytes[1] = (number >> 40) & 0xFF;
        bytes[2] = (number >> 32) & 0xFF;
        bytes[3] = (number >> 24) & 0xFF;
        bytes[4] = (number >> 16) & 0xFF;
        bytes[5] = (number >> 8) & 0xFF;
        bytes[6] = number & 0xFF;
    } else {
        // 50 to 64 bits
        count = 9;
        bytes[0] = 0xFE;
        bytes[1] = (number >> 56) & 0xFF;
        bytes[2] = (number >> 48) & 0xFF;
        bytes[3] = (number >> 40) & 0xFF;
        bytes[4] = (number >> 32) & 0xFF;
        bytes[5] = (number >> 24) & 0xFF;
        bytes[6] = (number >> 16) & 0xFF;
        bytes[7] = (number >> 8) & 0xFF;
        bytes[8] = number & 0xFF;
    }

    return count;
}

static int unpack(unsigned char *bytes, long *value) {
    long number;
    int count = 0;

    if (bytes[0] & 0x80 == 0) {
        // One byte, 7 bits
        number = bytes[0] & 0x7F;
        count = 1;
    } else if (bytes[0] & 0x40 == 0) {
        // Two bytes, 6 + 8 = 14 bits
        number = bytes[1] | ((long)(bytes[0] & 0x3F) << 8);
        count = 2;
    } else if (bytes[0] & 0x20 == 0) {
        // Three bytes, 5 + 8 + 8 = 21 bits
        number = bytes[2] | ((long)bytes[1] << 8) | ((long)(bytes[0] & 0x1F) << 16);
        count = 3;
    } else if (bytes[0] & 0x10 == 0) {
        // Four bytes, 4 + 8 + 8 + 8 = 28 bits
        number = bytes[3] | ((long)bytes[2] << 8) | ((long)bytes[1] << 16) | (((long)bytes[0] & 0x0F) << 24);
        count = 4;
    } else if (bytes[0] & 0x08 == 0) {
        // 5 bytes, 3 + 4 * 8 = 35 bits
        number = bytes[4] | ((long)bytes[3] << 8) | ((long)bytes[2] << 16) | ((long)bytes[1] << 24) | (((long)bytes[0] & 0x07) << 32);
        count = 5;
    } else if (bytes[0] & 0x04 == 0) {
        // 6 bytes, 2 + 5 * 8 = 42 bits
        number = bytes[5] | ((long)bytes[4] << 8) | ((long)bytes[3] << 16) | ((long)bytes[2] << 24) | ((long)bytes[1] << 32) | (((long)bytes[0] & 0x03) << 40);
        count = 6;
    } else if (bytes[0] & 0x04 == 0) {
        // 7 bytes, 1 + 6 * 8 = 49 bits
        number = bytes[6] | ((long)bytes[5] << 8) | ((long)bytes[4] << 16) | ((long)bytes[3] << 24) | ((long)bytes[2] << 32) | ((long)bytes[1] << 40) | (((long)bytes[0] & 0x01) << 48);
        count = 7;
    } else {
        // 9 bytes, 64 bits
        number = bytes[8] | ((long)bytes[7] << 8) | ((long)bytes[6] << 16) | ((long)bytes[5] << 24) | ((long)bytes[4] << 32) | ((long)bytes[3] << 40) | ((long)bytes[2] << 48) | ((long)bytes[1] << 56);
        count = 9;
    }
    
    *value = number;
    return count;
}

int sprintl(char *output, long num) {
    char temp[32];
    int length, spaces;
  
    sprintf(temp, "%lu", num);
    length = strlen(temp);
    spaces = (length - 1) / 3;

    temp[length + spaces] = 0;
    for (int i = 1; i <= length; i++) {
        temp[length + spaces - i] = temp[length - i];
        if (i % 3 == 0) {
            spaces--;
            temp[length + spaces - i] = '_';
        }
    }
    
    strcpy(output, temp);
    return strlen(temp);
}

void printl(long num) {
    char temp[32];

    sprintl(temp, num);
    printf("%s", temp);
}

void sprintlf(char *output, char *fmt, ...) {
    va_list(args);
    char *target = output + strlen(output);
    char *source;

    va_start(args, fmt);
    for (source = fmt; *source != 0; source++) {
        if (*source == '%') {
            target += sprintl(target, va_arg(args, long));
        } else *(target++) = *source;
    }
    va_end(args);
}

void printlf(char *fmt, ...) {
    va_list(args);
    char *source;

    va_start(args, fmt);
    for (source = fmt; *source != 0; source++) {
        if (*source == '%') {
            printl(va_arg(args, long));
        } else printf("%c", *source);
    }
    va_end(args);
}

void sscanl(char *input, long *value) {
    char temp[32];
    char *source, *target = temp;
    long number = 0;

    for (source = input; *source != 0; source++) {
        if (*source >= '0' && *source <= '9') break;
    }

    for (; (*source >= '0' && *source <= '9') || *source == '_'; source++) {
        if (*source == '_') continue;
        number = number * 10L + (*source - '0');
    }

    *value = number;
}
