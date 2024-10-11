#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include "worker.h"
#include "prime.h"
#include "number.h"
#include "longint.h"

static long upto = 1e6;
static long from = 2;
static long span = 1e5;
static char numbers_data[64] = "Numbers.";

static int cores = 4;
static int do_numbers = FORMAT_NONE;
static int dont_run = 0;
static int is_init = 0;

int main (int argc, char **argv) {
    long int memory = 0, filesize = 0;
    char memory_s[20], filesize_s[20];
    void *workers[16];
    void *sequence[16];
    long next;

    FILE *cpuinfo = fopen("/proc/cpuinfo", "r");
    char key[80];
    while (!feof(cpuinfo)) {
        int processor;
        if (fscanf(cpuinfo, "%s", key) == 0) continue;
        if (strcmp(key, "processor") == 0) {
            if (fscanf(cpuinfo, " : %d", &processor) == 0) continue;
            cores = processor + 1;
        }
    }
    fclose(cpuinfo);
    
    argv++;
    argc--;
    dont_run = argc == 0;
    
    for (int i = 0; i < argc; i++) {
        if (argv[i][0] == 'i') {
            is_init = 1;
            do_numbers = FORMAT_NONE;
        }
        else if (argv[i][0] == 't') sscanf(&argv[i][1], "%d", &cores);
        else if (argv[i][0] == 'n') do_numbers = FORMAT_PLAIN;
        else if (argv[i][0] == 'r') do_numbers = FORMAT_DEGREE;
        else if (argv[i][0] == 's') sscanf(&argv[i][1], "%lu", &span);
        else if (argv[i][0] == '+') {
            sscanf(&argv[i][1], "%lu", &from);
            if (argv[i][strlen(argv[i]) - 1] == 'g') from *= 1e9;
            if (argv[i][strlen(argv[i]) - 1] == 'm') from *= 1e6;
        }
        else if (argv[i][0] >= '1' && argv[i][0] <= '9') {
            sscanf(argv[i], "%lu", &upto);
            if (argv[i][strlen(argv[i]) - 1] == 'g') upto *= 1e9;
            if (argv[i][strlen(argv[i]) - 1] == 'm') upto *= 1e6;
        }
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
        printf("\t<num>\tup to\n");
        printf("\t+<num>\tstart from <num> (do numbers only)\n");
        printf("\tt<num>\tthreads\n");
        printf("\tn\twrite numbers.dat\n");
        printf("\tr\twrite numbers.red\n");
        printf("\ts<num>\tspan of computation\n");
        printf("\t?\tdon't run, show parameters\n");
    }

    next = primes_init(cores, is_init, upto, 1) + 1;
    if (do_numbers) {
        if (from > next) {
            fprintlf(stdout, "Can't start from  % > %  known\n", from, next - 1);
            primes_close(1);
            exit(0);
        }
        next = from;
    } else {
        if (upto < next) {
            fprintlf(stdout, "Already computed  % >= %\n", next - 1, upto);
            primes_close(1);
            exit(0);
        }
    }

    fprintlf(stdout, "Calculate  %  to  %  in spans of  % ", next, upto, span);
    printf(" on %d threads", cores);
    if (do_numbers) {
        sprintlf(numbers_data, "%-%", next, upto);
        switch (do_numbers) {
        case FORMAT_PLAIN: strcat(numbers_data, ".dat"); break;
        case FORMAT_INDEX: strcat(numbers_data, ".dxt"); break;
        case FORMAT_DEGREE: strcat(numbers_data, ".red"); break;
        default: exit(0);
        }
        unlink(numbers_data);
        printf(" >%s", numbers_data);
    }
    else printf(" no numbers");
    if (is_init) printf(" INIT");
    printf("\n");
    if (dont_run) {
        primes_close(1);
        exit(0);
    }

    time_t start = time(NULL);
    if (is_init) {
        primes_add(2);
        if (do_numbers) numbers_init(2, span);
        
        for (next = 3; next <= span; next++)
            decomp(next, NULL, do_numbers);
    
        fprintlf(stdout, "Init 2 - %  : %  primes\n", span, primes_count());
        
        if (do_numbers) {
            numbers_write(numbers_data, do_numbers);
            numbers_close();
        }
    }

    from = next;
    long total_steps = (upto - from) / (span * cores);
    
    struct timeval t_this;
    float d_this, d_prev = 0;
    void *d_base = d_new(), *d_step = d_new();
    int decile = 0;

    while (next <= upto) {
        long sofar = primes_count();
        long first = next;
        int threads;
        
        gettimeofday(&t_this, NULL);
        
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
        
        if (do_numbers) {
            numbers_write(numbers_data, do_numbers);
            numbers_close();
        }
        primes_write();

        long latest = primes_count();
        long remaining_steps = (upto - next) / (span * cores);
        float percent = 100.0 - 100.0 * remaining_steps / total_steps;
        if (percent > 10.0 * decile) {
            decile++;
            d_reset(d_base);
            d_reset(d_step);
        }

        long eta = 0;
        d_this = d_since(&t_this);
        d_add(d_base, d_this);
        d_add(d_step, (d_this - d_prev) / d_avg(d_base));
        eta = (long) (
            d_avg(d_base) * remaining_steps *
            (1.0 + d_avg(d_step) * remaining_steps / 2)
            );
        d_prev = d_this;
        
        printf(" [");
        fprintt(stdout, time(NULL) - start);
        printf("]%5.1f%%", percent);
        fprintlf(stdout, " .. %  :  %  primes. ETA ", first, latest);
        fprintt(stdout, eta);
        printf("          \r");
        fflush(stdout);
    }

    fprintlf(stdout, "\nTotal  %  primes last =  %.  RAM ", primes_count(), primes_last());
    fprintp(stdout, primes_size());
    printf(".  Time ");
    fprintt(stdout, time(NULL) - start);
    printf("\n");
    primes_write();
    primes_close(0);

    
    return 0;
}

