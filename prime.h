// Object-like structure to store prime number sequences

#ifndef _PRIME_H_
#define _PRIME_H_

// Allocate a structure to build a new sequence
void primes_init(
    unsigned int step,   // size of thread of computation
    );

// Add a number to the sequence
void primes_add(
    unsigned int prime   // prime number to add
    );

// Allocate an iterator
void *prime_new(
    );                   // returns an iterator object

// Iterate
unsigned int prime_next(
    void *iterator
    );                   // returns the next prime or 0 if no more
                         // release the iterator automatically

// Release the iterator
void prime_end(
    void *iterator
    );

#endif
