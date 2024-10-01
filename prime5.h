#ifndef _PRIME5_H_
#define _PRIME5_H_

typedef struct __attribute__((packed)) factor_s {
    unsigned int factor;
    unsigned char exponent;
} factor_t;

typedef struct __attribute__((packed)) number_s {
    unsigned char divisors;
    factor_t *factors;
} number_t;

#endif

