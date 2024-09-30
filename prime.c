// First version: naive

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct factor_s {
    int factor;
    int exponent;
} factor_t;
factor_t *factors;

typedef struct number_s {
    int divisors;
    factor_t *factors;
} number_t;
number_t *table;

void decomp(int original) {
    int divisor;
    int divisors = 0;
    int exponent;
    int remainder;

    printf("%8d = ", original);
    
    remainder = original;

    // Cheat for 2
    exponent = 0;
    while ((remainder & 1) == 0) {
        exponent++;
        remainder >>= 1;
    }
    if (exponent > 0) {
        factors[divisors].factor = 2;
        factors[divisors].exponent = exponent;
        printf("2^%d ", exponent);
        divisors++;
    }
        
    for (divisor = 3; divisor < remainder; divisor += 2) {
        if ((table[divisor].divisors == 0) && ((remainder % divisor) == 0)) {
            exponent = 0;
            do {
                exponent++;
                remainder /= divisor;
            } while ((remainder % divisor) == 0);
            factors[divisors].factor = divisor;
            factors[divisors].exponent = exponent;
            printf("%d^%d ", divisor, exponent);
            divisors++;
        }
    }
    
    if (remainder == original) {
        printf("Prime\n");
        return;
    }
    printf("\n");
    
    if (remainder > 1) {
        factors[divisors].factor = remainder;
        factors[divisors].exponent = 1;
        divisors++;
    }

    table[original].divisors = divisors;
    table[original].factors = (factor_t*) malloc(divisors * sizeof(factor_t));
    memcpy(table[original].factors, factors, divisors * sizeof(factor_t));
}

int main (int argc, char **argv) {
    number_t *p, *n;
    int i;
    int bound = 100;

    if (argc > 1) sscanf(argv[1], "%d", &bound);

    table = (number_t*) malloc(bound * sizeof(number_t));
    memset(table, 0, bound * sizeof(number_t));
    factors = (factor_t*) malloc(bound * sizeof(factor_t));

    for (i = 3; i < bound; i ++) decomp(i);
    
    return 0;
}

