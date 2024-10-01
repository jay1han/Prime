#include "prime.h"
#include "prime5.h"

// Store decomposition in *number_p and return 1 only if prime
int decomp(unsigned int original, number_t *number_p) {
    int divisors = 0;
    factor_t factors[20];
    unsigned int remainder = original;

    void *prime = prime_new();
    unsigned int divisor = prime_next(prime);
    while (divisor * divisor <= remainder) {
        if ((remainder % divisor) == 0) {
            int exponent = 0;
            do {
                exponent++;
                remainder /= divisor;
            } while ((remainder % divisor) == 0);
            factors[divisors].factor = divisor;
            factors[divisors].exponent = exponent;
            divisors++;
        }
        divisor = prime_next(prime);
        if (divisor == 0) break;
    }
    prime_end(prime);
    
    if (remainder == original) {
        number_p->divisors = 0;
        number_p->factors = NULL;
        return 1;
    }
    
    if (remainder > 1) {
        factors[divisors].factor = remainder;
        factors[divisors].exponent = 1;
        divisors++;
    }
    
    number_p->divisors = divisors;
    number_p->factors = (factor_t*) malloc(divisors * sizeof(factor_t));
    memcpy(number_p->factors, factors, divisors * sizeof(factor_t));

    memory += divisors * sizeof(factor_t);
    return 0;
}
