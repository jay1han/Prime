// V5: for larger sizes

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "worker.h"
#include "prime.h"
#include "number.h"
#include "decomp.h"

unsigned int upto = 100000;
unsigned int span = 100000;
char primes_file[40] = "primes";
char numbers_file[40] = "numbers";

int cores = 8;
unsigned int show = 10000;
int do_write = 1;

char *pretty(long int size, char *text) {
    if (size < 10e3) sprintf(text, "%ld", size);
    else if (size < 10e6) sprintf(text, "%.1lfK", (double)size / 1e3);
    else if (size < 10e9L) sprintf(text, "%.1lfM", (double)size / 1e6);
    else sprintf(text, "%.1lfG", (double)size / 1e9);
    return text;
}

int parse(int argc, char **argv) {
    for (int i = 0; i < argc; i++) {
        if (argv[i][0] == '-') strcpy(primes_file, &argv[i][1]);
        else if (argv[i][0] == '+') strcpy(numbers_file, &argv[i][1]);
        else if (argv[i][0] == 't') sscanf(&argv[i][1], "%d", &cores);
        else if (argv[i][0] == 'n') do_write = 0;
        else if (argv[i][0] == 'q') show = 0;
        else if (argv[i][0] == 's') sscanf(&argv[i][1], "%d", &show);
        else sscanf(argv[i], "%u", &upto);
    }
    
    if (cores > 16) cores = 16;
    if (upto >= 1e9) upto = 1e9 - 1;
    if (upto < 100000) upto = 100000;
    if (span > upto) span = upto / (cores + 1);
    upto = (1 + upto / span) * span;

    if (argc == 0) {
        printf("Options\n");
        printf("\t-<name>\tprimes_file\n");
        printf("\t+<name>\tnumbers_file\n");
        printf("\tt#\tthreads\n");
        printf("\tn\tno output\n");
        printf("\tq\tquiet\n");
        printf("\ts#\tshow tick every #\n");
        printf("\t#\tup to\n");
        printf("\ts#\tspan of computation\n");
    }
    
    printf("Up to %u in %u spans on %d threads ", upto, span, cores);
    if (show == 0) printf("quiet\n");
    else printf("show %u's\n", show);

    return argc;
}

int main (int argc, char **argv) {
    long int memory = 0, filesize = 0;
    char memory_s[20], filesize_s[20];
    void *workers[16];
    unsigned int current;

    if(parse(argc - 1, argv + 1) == 0) return 0;

    primes_init(span * cores);
    primes_add(2);
    numbers_init(2, span - 2);
    
    for (current = 2; current < span; current++)
        decomp(current);
    if (do_write) numbers_output(numbers_file);
    numbers_close();
    primes_defrag();
    
    while (current < upto) {
        unsigned int begin = current;
        unsigned int sofar = primes_count();
        
        numbers_init(current, current + span * cores);
        
        for (int thread = 0; thread < cores; thread++) {
            workers[thread] = worker_start(current, span, show);
            current += span;
        }
    
        for (int thread = 0; thread < cores; thread++)
            worker_join(workers[thread]);

        primes_defrag();
        
        if (do_write) {
            unsigned int latest = primes_count();
            printf("Span %u..%u : %u primes, total %u\n",
                   begin, current - 1, latest - sofar, latest);
            numbers_output(numbers_file);
        }
        numbers_close();
    }

    printf("Total %u primes\n", primes_count());
    primes_output(primes_file);
    
    return 0;
}
