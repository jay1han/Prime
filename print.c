#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "prime.h"
#include "number.h"
#include "flexint.h"
#include "worker.h"
#include "longint.h"

static void truncated() {
    printf("Truncated file, needs repair\n");
    exit(0);
}

static int numbers(char *filename, int chunked) {
    long number = 2;
    unsigned char bytes[256];
    int divisors;
    int numsize, maxsize = 0, degree;
    FILE *file = fopen(filename, "rb");
    FILE *out;
    unsigned char *chunk = NULL;
    int chunkp;
    int readred = 0;

    if (chunked > 0) chunk = malloc(chunked);

    if (chunked > 0 || chunked == -2) {
        char outname[64];
        strcpy(outname, filename);

        char *ext = strstr(outname, ".dat");

        if (ext == NULL) {
            fprintf(stderr, "Noncanonical file name\n");
            exit(0);
        }
        
        if (chunked == -2) sprintf(ext, ".red");
        else sprintf(ext, ".%d", chunked);
        out = fopen(outname, "wb");
        if (out == NULL) {
            fprintf(stderr, "Can't open %s\n", outname);
            exit(0);
        } else fprintf(stderr, "Write : %s\n", outname);
    }
    
    if (strstr(filename, ".red") != NULL && chunked == 0) readred = 1;
    
    if (file == NULL) {
        fprintf(stderr, "Can't open file\n");
        exit(0);
        
    } else {
        if (chunked == -1) fprintf(stderr, "Analyzing : %s\n", filename);
        
        while (fread(bytes, 1, 1, file) == 1) {
            
            if (readred) {
                printlf("%", number);
                printf(":%d\n", (int)bytes[0]);
                
            } else {
                if (!chunked) {
                    printlf("%", number);
                } else if (chunked > 0) {
                    memset(chunk, 0, chunked);
                    chunkp = 1;
                }
                    
                divisors = bytes[0];
                if (chunked > 0) chunk[0] = bytes[0];

                degree = 0;
                if (divisors == 0) {
                    if (!chunked) printf(" P\n");
                        
                } else {
                    numsize = 1;
                    if (!chunked) printf("=");
                        
                    for (int i = 0; i < divisors; i++) {
                        long factor;
                        int size = flex_read(file, &factor, bytes);
                        if (fread(&bytes[size], 1, 1, file) != 1) truncated();
                        int exponent = bytes[size];
                        degree += exponent;
                        if (chunked > 0) {
                            memcpy(chunk + chunkp, bytes, size + 1);
                            chunkp += size + 1;
                        }

                        if (!chunked) {
                            printlf(" %", factor);
                            printf("^%d", exponent);
                        }
                        numsize += size + 1;
                    }
                    
                    if (numsize > maxsize) maxsize = numsize;
                    if (!chunked) printf("\n");
                }
                if (chunked > 0) fwrite(chunk, 1, chunked, out);
                if (chunked == -2) {
                    bytes[0] = degree;
                    fwrite(bytes, 1, 1, out);
                }
            }
            
            number++;

            if ((number % 1000000) == 0) fspin(stderr, number);
        }
        fclose(file);

        if (chunked == -1) fprintf(stderr, "Max %d bytes per number\n", maxsize);
    }

    return maxsize;
}

int main (int argc, char **argv) {
    if (argc == 1 || argv[1][0] == '?') {
        printf("Options\n");
        printf("\tp\tprint known primes\n");
        printf("\t<file>\tprint a Numbers file (.dat or .red)\n");
        printf("\tc\twrite a Chunked version\n");
        printf("\tr\twrite a Reduced version\n");
        
    } else if (argc > 1) {
        if (argv[1][0] == 'p') {
            // Print all primes
            long max_prime = primes_init(1, 0, 2, 0);
            primes_close(1);
            void *prime = prime_new();
            long step, maxstep = 0;
            long factor = prime_next(prime, &step);
            
            while (factor != 0) {
                if (step > maxstep) maxstep = step;
                printlf("%\n", factor);
                if ((factor % 1000000) == 0) fspin(stderr, factor);
                factor = prime_next(prime, &step);
            }
            prime_end(prime);
            fprintlf(stderr, "Up to  % : % primes. Max step %\n",
                     max_prime, primes_count(), maxstep);
        
        } else {
            // Print Numbers.dat file
            if (argc > 2) {
                int chunked = 0;
                if (argv[2][0] == 'c') chunked = numbers(argv[1], -1);
                else if (argv[2][0] == 'r') chunked = -2;
                else sscanf(argv[2], "%d", &chunked);
                
                if (chunked > 0) fprintf(stderr, "Chunk to %d bytes. ", chunked);
                numbers(argv[1], chunked);
            } else numbers(argv[1], 0);
        }
    }
    
    return 0;
}
