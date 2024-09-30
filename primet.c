// Second version: threads and semaphores

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>

typedef struct factor_s {
    int factor;
    int exponent;
} factor_t;
factor_t *factors;

typedef struct number_s {
    sem_t semaphore;
    int divisors;
    factor_t *factors;
} number_t;
number_t *table;

int bound = 2000;

void decomp(int original) {
    int divisor;
    int divisors = 0;
    int exponent;
    int remainder;

    if ((original % (bound / 1000)) == 0) printf("%d\n", original);
    
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
            divisors++;
        }
    }
    
    if (remainder == original) {
        return;
    }
    
    if (remainder > 1) {
        factors[divisors].factor = remainder;
        factors[divisors].exponent = 1;
        divisors++;
    }
    
    table[original].divisors = divisors;
    table[original].factors = (factor_t*) malloc(divisors * sizeof(factor_t));
    memcpy(table[original].factors, factors, divisors * sizeof(factor_t));
}

void *start(void *arg) {
    int i, j;
    
    for (i = 1000; i < bound; i += 100) {
        if (sem_trywait(&table[i].semaphore) == 0) {
            for (j = i; j < i + 100; j++) decomp(j);
        }
        else i += 800;
    }
}

void print(int number) {
    int i;
    
    printf("%8d = ", number);

    if (table[number].divisors == 0) 
        printf("Prime\n");
    else {
        for (i = 0; i < table[number].divisors; i++) 
            printf("%d^%d ", table[number].factors[i].factor, table[number].factors[i].exponent);
        printf("\n");
    }
}

pthread_t threads[8];

int main (int argc, char **argv) {
    number_t *p, *n;
    int i;

    if (argc > 1) sscanf(argv[1], "%d", &bound);

    table = (number_t*) malloc(bound * sizeof(number_t));
    memset(table, 0, bound * sizeof(number_t));
    for (i = 0; i < bound; i++) {
        sem_init(&table[i].semaphore, 0, 1);
    }
    
    factors = (factor_t*) malloc(bound * sizeof(factor_t));

    for (i = 4; i < 1000; i++) {
        decomp(i);
    }

    for (i = 0; i < 8; i++) {
        pthread_create(&threads[i], NULL, start, NULL);
    }
    
    for (i = 0; i < 8; i++) {
        pthread_join(threads[i], NULL);
    }

//    for (i = 2; i < bound; i++) print(i);
    
    return 0;
}
