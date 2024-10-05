#include "flexint.h"

typedef struct flexint_s {
    long number;
    unsigned char flex[9];
    int size;
} flexint_t;

inline int flex_fold(long number, unsigned char *bytes) {
    return 0;
}

inline int flex_open(unsigned char *bytes, long *number) {
    return 0;
}
