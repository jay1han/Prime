// V5: for larger sizes

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "worker.h"

unsigned int upto = 2000;
int count = 0;
char primes_file[40] = "primes";
char numbers_file[40] = "numbers";

pthread_t threads[16];
worker_t workers[16];

int cores = 8;
int quiet = 0;
int do_write = 1;

char *pretty(long int size, char *text) {
    if (size < 10e3) sprintf(text, "%ld", size);
    else if (size < 10e6) sprintf(text, "%.1lfK", (double)size / 1e3);
    else if (size < 10e9L) sprintf(text, "%.1lfM", (double)size / 1e6);
    else sprintf(text, "%.1lfG", (double)size / 1e9);
    return text;
}

void parse(int argc, char **argv) {
    for (int i = 0; i < argc; i++) {
        if (argv[i][0] == '-') strcpy(primes_file, &argv[i][1]);
        else if (argv[i][0] == '+') strcpy(numbers_file, &argv[i][1]);
        else if (argv[i][0] == 't') sscanf(&argv[i][1], "%d", &cores);
        else if (argv[i][0] == 'n') do_write = 0;
        else if (argv[i][0] == 'q') quiet = 1;
        else sscanf(argv[i], "%u", &upto);
    }
    
    if (cores > 16) cores = 16;
    if (upto >= 1e9) upto = 1e9 - 1;
    if (upto < 100000) upto = 100000;
}

int main (int argc, char **argv) {
    long int memory = 0, filesize = 0;
    char memory_s[20], filesize_s[20];

    parse(argc - 1, argv + 1);
    
    printf("Up to %u on %d threads\n", upto, cores);

    workers[0].begin = 2;
    workers[0].end = 100000;
    work(&workers[0]);
    
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
            filesize += 8 + table[i].divisors * sizeof(factor_t);
            if ((i % step) == 0) {
                if (!quiet) printf("%d\n", i);
                fflush(file);
            }
        }
        fclose(file);
        printf("\n");
    }
    
    printf("Found %d primes up to %d in steps of %d on %d threads using %s RAM written %s bytes to \"%s\"\n",
           count, upto, step, cores, pretty(memory, memory_s), pretty(filesize, filesize_s), filename);

    return 0;
}
