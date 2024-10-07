#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "number.h"
#include "prime.h"
#include "flexint.h"

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
        fwrite(&self.numbers[index].divisors, 1, 1, file);
        for (int divisor = 0; divisor < self.numbers[index].divisors; divisor++) {
            if (format == 1) {
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
        printlf("% is not in the list\n", number);
        return;
    }
    
    number_t *this = &self.numbers[number - self.first];

    if (this->divisors == 0) printlf("% P\n", number);
    else {
        printlf("%=", number);
        for (int i = 0; i < this->divisors; i++) {
            printlf(" %", this->factors[i].factor);
            printf("^%d", this->factors[i].exponent);
        }
        printf("\n");
    }
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
            if (spaces >= 0) temp[length + spaces - i] = '_';
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

void fprintl(FILE *out, long num) {
    char temp[32];

    sprintl(temp, num);
    fprintf(out, "%s", temp);
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
    
    *target = 0;
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

void fprintlf(FILE *out, char *fmt, ...) {
    va_list(args);
    char *source;

    va_start(args, fmt);
    for (source = fmt; *source != 0; source++) {
        if (*source == '%') {
            fprintl(out, va_arg(args, long));
        } else fprintf(out, "%c", *source);
    }
    va_end(args);
}

void printpf(char *fmt, ...) {
    va_list(args);
    char *source;

    va_start(args, fmt);
    for (source = fmt; *source != 0; source++) {
        if (*source == '%') {
            long value = va_arg(args, long);
            if (value < 1e6) printf("%.3lfK", (double)value / 1e3);
            else if (value < 1e9) printf("%.3lfM", (double)value / 1e6);
            else printf("%.3lfG", (double)value / 1e9);
        } else printf("%c", *source);
    }
    va_end(args);
}

void fprintpf(FILE *out, char *fmt, ...) {
    va_list(args);
    char *source;

    va_start(args, fmt);
    for (source = fmt; *source != 0; source++) {
        if (*source == '%') {
            long value = va_arg(args, long);
            if (value < 1e6) fprintf(out, "%.3lfK", (double)value / 1e3);
            else if (value < 1e9) fprintf(out, "%.3lfM", (double)value / 1e6);
            else fprintf(out, "%.3lfG", (double)value / 1e9);
        } else fprintf(out, "%c", *source);
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

void printtf(char *fmt, ...) {
    va_list(args);
    char *source;

    va_start(args, fmt);
    for (source = fmt; *source != 0; source++) {
        if (*source == '%') {
            long sec = va_arg(args, long);
            long hours = sec / 3600;
            sec -= hours * 3600;
            long mins = sec / 60;
            sec -= mins * 60;
            if (hours > 0) printf("%ldh %02ldm %02lds", hours, mins, sec);
            else if (mins > 0) printf("%ldm %02lds", mins, sec);
            else printf("%lds", sec);
        } else printf("%c", *source);
    }
    va_end(args);
}

static char spinner[] = "|/-\\";

void fspin(FILE *out, long number) {
    static int spin = 0;

    fprintf(out, "%c", spinner[spin]);
    fprintlf(out, " %\r", number);
    fflush(out);
    if (++spin == 4) spin = 0;
}
