// Object-like structure to store numbers and their decomposition

#ifndef _NUMBER_H_
#define _NUMBER_H_

#include <stdarg.h>

// Inialize the numbers list
void numbers_init(
    long first,  // the first number in this list
    long last    // the last number in this list
    );

// After all numbers in this iteration have been computed,
// Store the current list in binary form to file {fd}
void numbers_write(
    char *filename,
    int format
    );

// Close the numbers list
void numbers_close();

// Allocate a number object
void *number_new(
    long number  // the number
    );                   // returns a number object

// With this number object,
// Set the number as prime and add in the list
void number_isprime(
    void *number
    );

// Add a known prime to the number
void number_addprime(
    void *number,
    void *prime,
    unsigned char exponent
    );

// Add an arbitrary factor to the number
void number_addfactor(
    void *number,
    long factor,
    unsigned char exponent
    );

// Save the number
void number_done(
    void *number
    );

void number_print(long number);

// Print longs
void printl(long num);
void printlf(char *fmt, ...);
void printpf(char *fmr, ...);
int sprintl(char *output, long num);
void sprintlf(char *output, char *fmt, ...);

// Scan a punctuated long
void sscanl(char *input, long *value);

#endif
