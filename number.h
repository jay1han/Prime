// Object-like structure to store numbers and their decomposition

#ifndef _NUMBER_H_
#define _NUMBER_H_

// Inialize the numbers list
void numbers_init(
    unsigned int begin,  // the first number in this list
    unsigned int span    // the number of numbers in this list
    );

// After all numbers in this iteration have been computed,
// Store the current list in binary form to file {fd}
void numbers_output(
    char *filename
    );

// Print the current list in text form to file {fd}
void numbers_print(
    char *filename
    );

// Close the numbers list
void numbers_close();

// Allocate a number object
void *number_new(
    unsigned int number  // the number
    );                   // returns a number object

// With this number object,
// Set the number as prime and add in the list
void number_isprime(
    void *number
    );

// Add a factor to the number
void number_addfactor(
    void *number,
    unsigned int factor,
    int exponent
    );

// Add the number in the list
void number_done(
    void *number
    );

#endif
