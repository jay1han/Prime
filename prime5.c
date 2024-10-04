// V5: for larger sizes

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "worker.h"
#include "prime.h"
#include "number.h"
#include "decomp.h"

unsigned int from = 2;
unsigned int upto = 1000000;
unsigned int span = 10000;
char primes_data[64];
char primes_list[64];
char numbers_data[64];
char numbers_list[64];

int cores = 8;
unsigned int show = 0;
int print_primes = 1;
int write_numbers = 0;
int print_numbers = 0;
int dont_run = 0;
int is_init = 0;

char *pretty(long int size, char *text) {
    if (size < 10e3) sprintf(text, "%ld", size);
    else if (size < 10e6) sprintf(text, "%.1lfK", (double)size / 1e3);
    else if (size < 10e9L) sprintf(text, "%.1lfM", (double)size / 1e6);
    else sprintf(text, "%.1lfG", (double)size / 1e9);
    return text;
}

int parse(int argc, char **argv) {
    dont_run = argc == 0;
    
    for (int i = 0; i < argc; i++) {
        if (argv[i][0] == 'i') is_init = 1;
        else if (argv[i][0] == '+') sscanf(&argv[i][1], "%u", &upto);
        else if (argv[i][0] == '-') sscanf(&argv[i][1], "%u", &from);
        else if (argv[i][0] == 't') sscanf(&argv[i][1], "%d", &cores);
        else if (argv[i][0] == 'a') write_numbers = 1;
        else if (argv[i][0] == 'h') print_numbers = 1;
        else if (argv[i][0] == 'n') print_primes = 0;
        else if (argv[i][0] == 'p') sscanf(&argv[i][1], "%d", &show);
        else if (argv[i][0] == 's') sscanf(&argv[i][1], "%u", &span);
        else dont_run = 1;
    }
    
    if (cores > 16) cores = 16;
    if (upto >= 1e9) upto = 1e9;
    if (upto < 1000) upto = 1000;
    if (span > (upto / cores)) span = upto / (cores + 1);
    if (((upto - span) % span) != 0) {
        unsigned int turns = upto / (cores * span) + 1;
        span = upto / (turns * cores);
    }

    sprintf(primes_data, "PrimeData_%u-%u.dat", from, upto);
    sprintf(primes_list, "PrimeList_%u-%u.lst", from, upto);
    sprintf(numbers_data, "NumberData_%u-%u.dat", from, upto);
    unlink(numbers_data);
    sprintf(numbers_list, "NumberList_%u-%u.lst", from, upto);
    unlink(numbers_list);

    if (dont_run) {
        printf("Options\n");
        printf("\ti\tinitialise data\n");
        printf("\t-#\tstart from #\n");
        printf("\t+#\tup to and including #\n");
        printf("\tt#\tthreads\n");
        printf("\ta\twrite numbers.dat\n");
        printf("\th\tprint numbers.lst\n");
        printf("\tn\tdo not write primes.dat\n");
        printf("\tp#\tshow progress every #\n");
        printf("\ts#\tspan of computation\n");
        printf("\t?\tdon't run, show parameters\n");
    }
    
    printf("From %u to %u in spans of %u on %d threads", from, upto, span, cores);
    if (show == 0) printf(" quietly");
    else printf(" show %u's", show);
    printf(" >%s", primes_data);
    if (print_primes) printf(" >%s", primes_list);
    if (write_numbers) printf(" >%s", numbers_data);
    if (print_numbers) printf(" >%s", numbers_list);
    if (is_init) printf(" INIT");
    printf("\n");

    return dont_run;
}

int main (int argc, char **argv) {
    long int memory = 0, filesize = 0;
    char memory_s[20], filesize_s[20];
    void *workers[16];
    void *sequence[16];
    unsigned int next;

    if(parse(argc - 1, argv + 1)) return 0;

    primes_init(cores, is_init);

    if (is_init) {
        primes_add(2);
        numbers_init(2, span);
        
        for (next = 3; next <= span; next++)
            decomp(next, NULL);
    
        printf("Init %u..%u : %u primes\n",
               2, span, primes_count());
        
        if (write_numbers) numbers_write(numbers_data);
        if (print_numbers) numbers_print(numbers_list);
        numbers_close();
    } else next = from;
    
    while (next <= upto) {
        unsigned int first = next;
        unsigned int sofar = primes_count();
        int threads;

        if (next + span * cores - 1 > upto) numbers_init(next, upto);
        else numbers_init(next, next + span * cores - 1);
        
        for (threads = 0; threads < cores; threads++) {
            if (next + span > upto) {
                workers[threads++] = worker_start(next, upto, show);
                next = upto + 1;
                break;
            }
            workers[threads] = worker_start(next, next + span - 1, show);
            next += span;
        }
    
        for (int thread = 0; thread < threads; thread++)
            sequence[thread] = worker_join(workers[thread]);

        for (int thread = 0; thread < threads; thread++)
            primes_add_seq(sequence[thread]);
        
        unsigned int latest = primes_count();
        printf("Span %u..%u on %d threads: %u primes, total %u\n",
               first, next - 1, threads, latest - sofar, latest);
        if (write_numbers) numbers_write(numbers_data);
        if (print_numbers) numbers_print(numbers_list);
        numbers_close();
    }

    printf("Total %u primes\n", primes_count());
    primes_write(primes_data, from, upto);
    if (print_primes) primes_print(primes_list, from, upto);
    
    return 0;
}
