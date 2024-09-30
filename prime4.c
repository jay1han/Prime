// V4: with profiling

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>

typedef struct factor_s {
    int factor;
    int exponent;
} factor_t;
factor_t factors[100];

typedef struct number_s {
    int divisors;
    factor_t *factors;
} number_t;
number_t *table;

sem_t *semaphores;
long int total = 0;

int step = 1000;
int upto = 2000;
int init;
int count = 0;
int cores = 8;
char primes[] = "primes";
int quiet = 0;

int decomp(int original) {
    int divisor;
    int divisors = 0;
    int exponent;
    int remainder;

    if (!quiet && (original % (upto / 1000)) == 0) printf("%d\n", original);
    
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
        
    for (divisor = 3; divisor * divisor <= remainder; divisor += 2) {
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
        return 1;
    }
    
    if (remainder > 1) {
        factors[divisors].factor = remainder;
        factors[divisors].exponent = 1;
        divisors++;
    }
    
    table[original].divisors = divisors;
    table[original].factors = (factor_t*) malloc(divisors * sizeof(factor_t));
    memcpy(table[original].factors, factors, divisors * sizeof(factor_t));

    total += divisors * sizeof(factor_t);
    return 0;
}

void *start(void *arg) {
    int i, j;
    int count = 0;
    
    for (i = init; i < upto; i += step) {
        if (sem_trywait(&semaphores[i / step]) == 0) {
            for (j = i; j < (i + step) && j < upto; j++) {
                if (decomp(j)) count++;
            }
        }
        else i += step;
    }

    *(int*)arg = count;
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

pthread_t threads[16];
int counts[16];

char *pretty(long int size, char *text) {
    if (size < 10e3) sprintf(text, "%ld", size);
    else if (size < 10e6) sprintf(text, "%.1lfK", (double)size / 1e3);
    else if (size < 10e9L) sprintf(text, "%.1lfM", (double)size / 1e6);
    else sprintf(text, "%.1lfG", (double)size / 1e9);
    return text;
}

int main (int argc, char **argv) {
    int i;
    int s;
    long int size = 0;
    char *filename = primes;
    char total_s[20], size_s[20];
    int do_write = 1;

    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-') filename = &argv[i][1];
        else if (argv[i][0] == 'c') sscanf(&argv[i][1], "%d", &cores);
        else if (argv[i][0] == 's') sscanf(&argv[i][1], "%d", &step);
        else if (argv[i][0] == 't') do_write = 0;
        else if (argv[i][0] == 'q') quiet = 1;
        else sscanf(argv[i], "%d", &upto);
    }
    if (cores > 16) cores = 16;
    if (upto > 1e9) upto = 1e9;
    if (upto < 2000) upto = 20000;
    if (step < 1000) step = 1000;
    if (upto % step) upto = (upto / step) * step;

    printf("Up to %d in steps of %d on %d threads\n", upto, step, cores);

    table = (number_t*) malloc(upto * sizeof(number_t));
    memset(table, 0, upto * sizeof(number_t));
    total = upto * sizeof(number_t);
    
    s = upto / step;
    semaphores = (sem_t*) malloc(s * sizeof(sem_t));
    memset(semaphores, 0, s * sizeof(sem_t));
    total += s * sizeof(sem_t);
    
    for (i = 0; i < s; i++) {
        sem_init(&semaphores[i], 0, 1);
    }

    count = 2;
    for (init = 4; init < step; init++) {
        if (decomp(init)) count++;
    }

    memset(counts, 0, 16 * sizeof(int));
    for (i = 0; i < cores; i++) {
        pthread_create(&threads[i], NULL, start, &counts[i]);
    }
    
    for (i = 0; i < cores; i++) {
        pthread_join(threads[i], NULL);
    }

    for (i = 0; i < cores; i++) {
        count += counts[i];
    }

    if (do_write) {
        printf("Writing to \"%s\"\n", filename);
        FILE *file = fopen("primes", "wb");
        for (i = 2; i < upto; i++) {
            fwrite(&i, 4, 1, file);
            fwrite(&table[i].divisors, 4, 1, file);
            fwrite(table[i].factors, sizeof(factor_t), table[i].divisors, file);
            size += 8 + table[i].divisors * sizeof(factor_t);
            if ((i % step) == 0) {
                if (!quiet) printf("%d\n", i);
                fflush(file);
            }
        }
        fclose(file);
        printf("\n");
    }
    
    printf("Found %d primes up to %d in steps of %d on %d threads using %s RAM written %s bytes to \"%s\"\n",
           count, upto, step, cores, pretty(total, total_s), pretty(size, size_s), filename);

    return 0;
}
