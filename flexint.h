#ifndef _FLEXINT_H_
#define _FLEXINT_H_

// Fold a long into bytes and return number of bytes written
int flex_fold(long number, unsigned char *bytes);

// Unfold a number of bytes and return number of bytes read
int flex_open(unsigned char *bytes, long *value);

// Same but from a file
int flex_read(FILE *file, long *value, unsigned char *buffer);

#endif
