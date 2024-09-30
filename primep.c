// Third version: with self-divisors

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
    int divisors;
    factor_t *factors;
} number_t;
number_t *table;

sem_t *semaphores;

#define STEP  1000
#define CORES 8

int bound = STEP * 2;
int init;

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
        
    for (divisor = 3; divisor * divisor < remainder; divisor += 2) {
        if ((table[divisor].factors == NULL) && ((remainder % divisor) == 0)) {
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
    
    for (i = init; i < bound; i += STEP) {
        if (sem_trywait(&semaphores[i / STEP]) == 0) {
            for (j = i; j < (i + STEP) && j < bound; j++) decomp(j);
        }
        else i += STEP;
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

pthread_t threads[CORES];

int main (int argc, char **argv) {
    int i;
    int s;
    int do_print = 0;

    for (i = 1; i < argc; i++) {
        if (argv[i][0] == 'p') do_print = 1;
        else sscanf(argv[i], "%d", &bound);
    }

    if (bound % STEP) {
        printf("Only steps of %d\n", STEP);
        exit(0);
    }

    table = (number_t*) malloc(bound * sizeof(number_t));
    memset(table, 0, bound * sizeof(number_t));
    s = bound / STEP;
    semaphores = (sem_t*) malloc(s * sizeof(sem_t));
    memset(semaphores, 0, s * sizeof(sem_t));
    
    for (i = 0; i < s; i++) {
        sem_init(&semaphores[i], 0, 1);
    }
    
    factors = (factor_t*) malloc(bound * sizeof(factor_t));

    for (init = 4; init * init < bound; init++) {
        decomp(init);
    }

    for (i = 0; i < CORES; i++) {
        pthread_create(&threads[i], NULL, start, NULL);
    }
    
    for (i = 0; i < CORES; i++) {
        pthread_join(threads[i], NULL);
    }

    if (do_print) {
        for (i = 2; i < bound; i++) print(i);
    }
    
    return 0;
}
