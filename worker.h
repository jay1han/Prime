#ifndef _WORKER_H_
#define _WORKER_H_

// Store decomposition in *number_p and return 1 only if prime
void decomp(unsigned int number, void *sequence, int do_numbers);

// Start a worker on a span of numbers
void *worker_start(
    unsigned int begin,
    unsigned int last,
    unsigned int show,
    int do_numbers
    );                      // returns a worker object

// Wait for a worker to finish and return the sequence
void *worker_join(
    void *arg
    );

#endif
