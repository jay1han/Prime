// Object-like structure to store prime number sequences

#ifndef _PRIME_H_
#define _PRIME_H_

// Allocate a structure to build a new sequence
void primes_init(
    unsigned int span    // size of thread of computation
    );

// Add a number to the sequence
void primes_add(
    unsigned int prime   // prime number to add
    );

// Defragments the list
void primes_defrag();

// Returns the number of primes so far
unsigned int primes_count();

// Write to file
void primes_output(
    char *filename
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
