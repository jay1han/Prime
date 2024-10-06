#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "prime.h"
#include "number.h"
#include "flexint.h"
#include "worker.h"

static char spinner[4] = "|/-\\";

static int run(char *filename, int chunked) {
    long number = 2;
    unsigned char bytes[256];
    int divisors;
    int numsize, maxsize = 0;
    FILE *file = fopen(filename, "rb");
    FILE *out;
    int spin = 0;
    unsigned char *chunk = NULL;
    int chunkp;

    if (chunked > 0) {
        chunk = malloc(chunked);
        
        char outname[64];
        strcpy(outname, filename);
        char *ext = strstr(outname, ".dat");
        if (ext == NULL) {
            fprintf(stderr, "Noncanonical file name\n");
            exit(0);
        }
        sprintf(ext, ".%d", chunked);
        out = fopen(outname, "wb");
        if (out == NULL) {
            fprintf(stderr, "Can't open %s\n", outname);
            exit(0);
        } else fprintf(stderr, "Write : %s\n", outname);
    }
    
    if (file == NULL) {
        fprintf(stderr, "Can't open file\n");
        exit(0);
        
    } else {
        if (chunked < 0) fprintf(stderr, "Analyzing : %s\n", filename);
        
        while (fread(bytes, 1, 1, file) == 1) {
            if (!chunked) printlf("%", number);
            else if (chunked > 0) {
                memset(chunk, 0, chunked);
                chunkp = 1;
            }
                    
            divisors = bytes[0];
            if (chunked > 0) chunk[0] = bytes[0];
                    
            if (divisors == 0) {
                if (!chunked) printf(" P\n");
                        
            } else {
                numsize = 1;
                if (!chunked) printf("=");
                        
                for (int i = 0; i < divisors; i++) {
                    int divisor = fread(bytes, 1, 10, file);
                    long factor;
                    int size = flex_open(bytes, &factor);
                    int exponent = bytes[size];
                    if (chunked > 0) {
                        memcpy(chunk + chunkp, bytes, size + 1);
                        chunkp += size + 1;
                    }
                    fseek(file, size + 1 - divisor, SEEK_CUR);

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
            number++;

            if ((number % 1000000) == 0) {
                char temp[20];
                sprintl(temp, number);
                fprintf(stderr, "%c %s\r", spinner[spin], temp);
                if (++spin == 4) spin = 0;
            }
        }
        fclose(file);

        if (chunked <= 0) fprintf(stderr, "Max %d bytes per number\n", maxsize);
    }

    return maxsize;
}

int main (int argc, char **argv) {
    long max = primes_init(1, 0, 2, 0);

    if (argc == 1 || argv[1][0] == 'p') {
        // Print all primes
        void *prime = prime_new();
        long step, maxstep = 0;
        long factor = prime_next(prime, &step);

        while (factor != 0) {
            if (step > maxstep) maxstep = step;
            printlf("%\n", factor);
            factor = prime_next(prime, &step);
        }
        prime_end(prime);
        printlf("Max step %\n", maxstep);
        
    } else if (argc > 1) {
        
        if (argv[1][0] >= '1' && argv[1][0] <= '9') {
            // Decompose the number
            if (strlen(argv[1]) > 18) fprintf(stderr, "Too long\n");
            else {
                long number;
                sscanl(argv[1], &number);

                if (number > max)
                    printlf("% is too big, max %\n", number, max);
                else {
                    numbers_init(number, number);
                    decomp(number, NULL, 1);
                    number_print(number);
                    numbers_close();
                }
            }
            
        } else {
            // Print Numbers.dat file
            int chunked = 0;
            if (argc > 2) {
                if (argv[2][0] == 'c') chunked = run(argv[1], -1);
                else sscanf(argv[2], "%d", &chunked);
                
                fprintf(stderr, "Chunk to %d bytes. ", chunked);
                run(argv[1], chunked);
            } else run(argv[1], 0);
        }
    }
    
    primes_close(1);
    return 0;
}
