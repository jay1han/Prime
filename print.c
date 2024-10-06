#include <stdio.h>
#include <string.h>
#include "prime.h"
#include "number.h"
#include "flexint.h"
#include "worker.h"

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
        printlf("Step %\n", maxstep);
        
    } else if (argc > 1) {
        
        if (argv[1][0] >= '1' && argv[1][0] <= '9') {
            // Decompose the number
            if (strlen(argv[1]) > 18) printf("Too long\n");
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
            long number = 2;
            unsigned char bytes[256];
            int divisors;
            FILE *file = fopen(argv[1], "rb");

            if (file == NULL) printf("Can't open file\n");
            else {
                while (fread(bytes, 1, 1, file) == 1) {
                    printlf("%", number);
                    divisors = bytes[0];
                    if (divisors == 0) printf(" P\n");
                    else {
                        printf("=");
                        for (int i = 0; i < divisors; i++) {
                            int divisor = fread(bytes, 1, 10, file);
                            long factor;
                            int size = flex_open(bytes, &factor);
                            int exponent = bytes[size];
                            fseek(file, size + 1 - divisor, SEEK_CUR);

                            printlf(" %", factor);
                            printf("^%d", exponent);
                        }
                        printf("\n");
                    }
                    number++;
                }
                fclose(file);
            }
        }
    }
    
    primes_close(1);
    return 0;
}
