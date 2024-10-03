#include "prime.h"
#include "prime5.h"
#include "number.h"

// Store decomposition in *number_p and return 1 only if prime
void decomp(unsigned int original) {
    unsigned int remainder = original;
    void *number = number_new(original);
    void *prime = prime_new();
    unsigned int factor = prime_next(prime);
    
    while (factor * factor <= remainder) {
        if ((remainder % factor) == 0) {
            int exponent = 0;
            do {
                exponent++;
                remainder /= factor;
            } while ((remainder % factor) == 0);
            number_addfactor(number, factor, exponent);
        }
        factor = prime_next(prime);
        if (factor == 0) break;
    }
    prime_end(prime);
    
    if (remainder == original) number_isprime(number);
    else if (remainder > 1)    number_addfactor(number, remainder, 1);
    
    number_done(number);
}
