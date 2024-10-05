// V5: for larger sizes

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "worker.h"
#include "prime.h"
#include "number.h"

static long from = 2;
static long upto = 1e6;
static long span = 1e4;
static char numbers_data[64] = "Numbers.";

static int cores = 8;
static long show = 0;
static int do_list = 1;
static int do_numbers = 0;
static int dont_run = 0;
static int is_init = 0;

static int parse(int argc, char **argv) {
    dont_run = argc == 0;
    
    for (int i = 0; i < argc; i++) {
        if (argv[i][0] == 'i') is_init = 1;
        else if (argv[i][0] == '+') sscanf(&argv[i][1], "%lu", &upto);
        else if (argv[i][0] == '-') sscanf(&argv[i][1], "%lu", &from);
        else if (argv[i][0] == 't') sscanf(&argv[i][1], "%d", &cores);
        else if (argv[i][0] == 'a') do_numbers = 1;
        else if (argv[i][0] == 'n') do_list = 0;
        else if (argv[i][0] == 'p') sscanf(&argv[i][1], "%lu", &show);
        else if (argv[i][0] == 's') sscanf(&argv[i][1], "%lu", &span);
        else dont_run = 1;
    }
    
    if (cores > 16) cores = 16;
    if (upto < 1e6) upto = 1e6;
    if (from < 2) from = 2;
    if (span < 1e4) span = 1e4;
    if (span > (upto / cores)) span = upto / (cores + 1);
    if (((upto - span) % span) != 0) {
        long turns = upto / (cores * span) + 1;
        span = upto / (turns * cores);
    }

    sprintlf(numbers_data, "%-%.dat", from, upto);
    unlink(numbers_data);

    if (dont_run) {
        printf("Options\n");
        printf("\ti\tinitialise data\n");
        printf("\t-#\tstart from #\n");
        printf("\t+#\tup to and including #\n");
        printf("\tt#\tthreads\n");
        printf("\ta\twrite numbers.dat\n");
        printf("\tn\tdo not print primes.lst\n");
        printf("\tp#\tshow progress every #\n");
        printf("\ts#\tspan of computation\n");
        printf("\t?\tdon't run, show parameters\n");
    }

    printlf("From % to % in spans of %", from, upto, span);
    printf(" on %d threads", cores);
    if (show == 0) printf(" quietly");
    else printlf(" show %'s", show);
    printf(" >Data");
    if (do_list) printf(" >List");
    if (do_numbers) printf(" >%s", numbers_data);
    else printf(" no numbers");
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
        if (do_numbers) numbers_init(2, span);
        
        for (next = 3; next <= span; next++)
            decomp(next, NULL, do_numbers);
    
        printlf("Init 2 .. % : % primes\n", span, primes_count());
        
        if (do_numbers) {
            numbers_write(numbers_data);
            numbers_close();
        }
        
    } else {
        if (upto < primes_last()) {
            printlf("Already computed % > %\n", primes_last(), upto);
            exit(0);
        }
        next = from;
    }
    
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
        printlf("Span % .. %", first, next - 1);
        printf(" on %d threads: ", threads);
        printlf("% primes, total %\n", latest - sofar, latest);
        if (do_numbers) {
            numbers_write(numbers_data);
            numbers_close();
        }
    }

    printlf("Total % primes last=%", primes_count(), primes_last());
    printpf(" RAM usage %\n", primes_size());
    primes_write(upto, do_list);
    
    return 0;
}

