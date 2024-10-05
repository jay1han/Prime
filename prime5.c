// V5: for larger sizes

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "worker.h"
#include "prime.h"
#include "number.h"

long from = 2;
long upto = 1000000;
long span = 10000;
char primes_data[64];
char primes_list[64];
char numbers_data[64];
char numbers_list[64];

int cores = 8;
long show = 0;
int print_primes = 1;
int write_numbers = 0;
int print_numbers = 0;
int do_numbers = 0;
int dont_run = 0;
int is_init = 0;

int parse(int argc, char **argv) {
    dont_run = argc == 0;
    
    for (int i = 0; i < argc; i++) {
        if (argv[i][0] == 'i') is_init = 1;
        else if (argv[i][0] == '+') sscanf(&argv[i][1], "%lu", &upto);
        else if (argv[i][0] == '-') sscanf(&argv[i][1], "%lu", &from);
        else if (argv[i][0] == 't') sscanf(&argv[i][1], "%d", &cores);
        else if (argv[i][0] == 'a') write_numbers = 1;
        else if (argv[i][0] == 'h') print_numbers = 1;
        else if (argv[i][0] == 'n') print_primes = 0;
        else if (argv[i][0] == 'p') sscanf(&argv[i][1], "%d", &show);
        else if (argv[i][0] == 's') sscanf(&argv[i][1], "%lu", &span);
        else dont_run = 1;
    }
    
    if (cores > 16) cores = 16;
    if (upto < 1000) upto = 1000;
    if (span > (upto / cores)) span = upto / (cores + 1);
    if (((upto - span) % span) != 0) {
        long turns = upto / (cores * span) + 1;
        span = upto / (turns * cores);
    }
    if (write_numbers || print_numbers) do_numbers = 1;

    sprintf(primes_data, "PrimeData_%lu-%lu.dat", from, upto);
    sprintf(primes_list, "PrimeList_%lu-%lu.lst", from, upto);
    sprintf(numbers_data, "NumberData_%lu-%lu.dat", from, upto);
    unlink(numbers_data);
    sprintf(numbers_list, "NumberList_%lu-%lu.lst", from, upto);
    unlink(numbers_list);

    if (dont_run) {
        printf("Options\n");
        printf("\ti\tinitialise data\n");
        printf("\t-#\tstart from #\n");
        printf("\t+#\tup to and including #\n");
        printf("\tt#\tthreads\n");
        printf("\ta\twrite numbers.dat\n");
        printf("\th\tprint numbers.lst\n");
        printf("\tn\tdo not print primes.lst\n");
        printf("\tp#\tshow progress every #\n");
        printf("\ts#\tspan of computation\n");
        printf("\t?\tdon't run, show parameters\n");
    }
    
    printf("From %lu to %lu in spans of %lu on %d threads", from, upto, span, cores);
    if (show == 0) printf(" quietly");
    else printf(" show %lu's", show);
    if (!do_numbers) printf(" no numbers");
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
    long next;

    if(parse(argc - 1, argv + 1)) return 0;

    primes_init(cores, is_init);

    if (is_init) {
        primes_add(2);
        numbers_init(2, span);
        
        for (next = 3; next <= span; next++)
            decomp(next, NULL, do_numbers);
    
        printf("Init %lu..%lu : %lu primes\n",
               2, span, primes_count());
        
        if (write_numbers) numbers_write(numbers_data);
        if (print_numbers) numbers_print(numbers_list);
        numbers_close();
    } else next = from;
    
    while (next <= upto) {
        long first = next;
        long sofar = primes_count();
        int threads;

        if (next + span * cores - 1 > upto) numbers_init(next, upto);
        else numbers_init(next, next + span * cores - 1);
        
        for (threads = 0; threads < cores; threads++) {
            if (next + span > upto) {
                workers[threads++] = worker_start(next, upto, show, do_numbers);
                next = upto + 1;
                break;
            }
            workers[threads] = worker_start(next, next + span - 1, show, do_numbers);
            next += span;
        }
    
        for (int thread = 0; thread < threads; thread++)
            sequence[thread] = worker_join(workers[thread]);

        for (int thread = 0; thread < threads; thread++)
            primes_add_seq(sequence[thread]);
        
        long latest = primes_count();
        printf("Span %lu..%lu on %d threads: %lu primes, total %lu\n",
               first, next - 1, threads, latest - sofar, latest);
        if (write_numbers) numbers_write(numbers_data);
        if (print_numbers) numbers_print(numbers_list);
        numbers_close();
    }

    printf("Total %lu primes\n", primes_count());
    primes_write(primes_data, from, upto);
    if (print_primes) primes_print(primes_list, from, upto);
    
    return 0;
}
