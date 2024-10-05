// Object-like structure to store prime number sequences

#ifndef _PRIME_H_
#define _PRIME_H_

// Allocate
void primes_init(int threads, int is_init);

// Add a number to the sequence
void primes_add(
    long prime   // prime number to add
    );

// Returns the number of primes so far
long primes_count();

// Returns the highest prime after init
long primes_last();

// Returns RAM occupation
long primes_size();

// Allocate a sequence of primes
void *seq_alloc(
    long span
    );

// Add a prime to sequence
void seq_add(
    void *arg,
    long prime
    );

// Return the sequence to the master list
void primes_add_seq(
    void *arg
    );

// Write to file
void primes_write(
    long upto,
    int do_list;
    );

// Allocate an iterator
void *prime_new(
    );                   // returns an iterator object

// Iterate
long prime_next(
    void *iterator,
    long *step           // step from previous prime
    );                   // returns the next prime or 0 if no more

// Return the index of this prime
long prime_index(void *iterator);

// Return index of number, or 0 if not found
long prime_find(void *arg, long number);

// Release the iterator
void prime_end(
    void *iterator
    );

#endif
