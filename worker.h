#ifndef _WORKER_H_
#define _WORKER_H_

// Start a worker on a span of numbers
void *worker_start(
    unsigned int begin,
    unsigned int last,
    unsigned int show
    );                      // returns a worker object

// Wait for a worker to finish
void worker_join(
    void *arg
    );

#endif
