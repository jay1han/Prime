// V5: for larger sizes

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "worker.h"
#include "prime.h"
#include "number.h"

static long upto = 1e6;
static long from = 2;
static long span = 1e5;
static char numbers_data[64] = "Numbers.";

static int cores = 8;
static int do_numbers = 0;
static int dont_run = 0;
static int is_init = 0;

static char spinner[] = "|/-\\";

int main (int argc, char **argv) {
    long int memory = 0, filesize = 0;
    char memory_s[20], filesize_s[20];
    void *workers[16];
    void *sequence[16];
    long next;

    argv++;
    argc--;
    dont_run = argc == 0;
    
    for (int i = 0; i < argc; i++) {
        if (argv[i][0] == 'i') is_init = 1;
        else if (argv[i][0] == 't') sscanf(&argv[i][1], "%d", &cores);
        else if (argv[i][0] == 'n') do_numbers = 1;
        else if (argv[i][0] == 's') sscanf(&argv[i][1], "%lu", &span);
        else if (argv[i][0] == '+') sscanf(&argv[i][1], "%lu", &from);
        else if (argv[i][0] >= '1' && argv[i][0] <= '9') sscanf(argv[i], "%lu", &upto);
        else dont_run = 1;
    }
    
    if (cores > 16) cores = 16;
    if (upto < 1e6) upto = 1e6;
    if (span < 1e4) span = 1e4;
    if (span > (upto / cores)) span = upto / (cores + 1);
    if (((upto - span) % span) != 0) {
        long turns = upto / (cores * span) + 1;
        span = upto / (turns * cores);
    }

    if (dont_run) {
        printf("Options\n");
        printf("\ti\tinitialise data\n");
        printf("\t#\tup to\n");
        printf("\t+#\tstart from # (numbers only)\n");
        printf("\tt#\tthreads\n");
        printf("\tn\twrite numbers.dat\n");
        printf("\ts#\tspan of computation\n");
        printf("\t?\tdon't run, show parameters\n");
    }

    next = primes_init(cores, is_init) + 1;
    if (is_init) next = 2;
    if (do_numbers) {
        if (from > next) {
            printlf("Can't start from  % > %  last known prime\n", from, next - 1);
            exit(0);
        }
        next = from;
    }
                          
    printlf("From  %  to  %  in spans of  % ", next, upto, span);
    printf(" on %d threads", cores);
    if (do_numbers) {
        sprintlf(numbers_data, "%-%.dat", next, upto);
        unlink(numbers_data);
        printf(" >%s", numbers_data);
    }
    else printf(" no numbers");
    if (is_init) printf(" INIT");
    printf("\n");
    if (dont_run) return 0;
    
    if (is_init) {
        primes_add(2);
        if (do_numbers) numbers_init(2, span);
        
        for (next = 3; next <= span; next++)
            decomp(next, NULL, do_numbers);
    
        printlf("Init 2 - %  : %  primes\n", span, primes_count());
        
        if (do_numbers) {
            numbers_write(numbers_data, do_numbers);
            numbers_close();
        }
        
    } else {
        if ((do_numbers == 0) && (upto < primes_last())) {
            printlf("Already computed  % > %\n", primes_last(), upto);
            exit(0);
        }
    }

    int spin = 0;
    while (next <= upto) {
        long first = next;
        long sofar = primes_count();
        int threads;

        if (do_numbers) {
            if (next + span * cores - 1 > upto) numbers_init(next, upto);
            else numbers_init(next, next + span * cores - 1);
        }
        
        for (threads = 0; threads < cores; threads++) {
            if (next + span > upto) {
                workers[threads++] = worker_start(next, upto, do_numbers);
                next = upto + 1;
                break;
            }
            workers[threads] = worker_start(next, next + span - 1, do_numbers);
            next += span;
        }
    
        for (int thread = 0; thread < threads; thread++)
            sequence[thread] = worker_join(workers[thread]);

        for (int thread = 0; thread < threads; thread++)
            primes_add_seq(sequence[thread]);
        
        long latest = primes_count();
        printf("%c ", spinner[spin]);
        printlf("Span  % - %  :  %  primes, total  %\r", first, next - 1, latest - sofar, latest);
        fflush(stdout);
        if (do_numbers) {
            numbers_write(numbers_data, do_numbers);
            numbers_close();
        }
        if (++spin == 4) spin = 0;
    }
    printf("\n");

    printlf("Total  % primes last = % ", primes_count(), primes_last());
    printpf(" RAM usage %\n", primes_size());
    primes_write(upto);
    
    return 0;
}
