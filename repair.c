#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "number.h"
#include "flexint.h"

int main(int argc, char **argv) {
    if (argc == 1 || argv[1][0] == '?') {
        
    } else {
        char outfile[64];
        
        long first, last;
        sscanl(strchr(argv[1], '.') + 1, &first);
        sscanl(strchr(argv[1], '-') + 1, &last);

        FILE *file = fopen(argv[1], "rb");
        unsigned char bytes[10];
        long pos = 0, number;
        for (number = first; number <= last; number++) {
            if (fread(bytes, 1, 1, file) != 1) break;
            int divisors = bytes[0];
            for (int i = 0; i < divisors; i++) {
                int divisor = fread(bytes, 1, 10, file);
                long factor;
                int size = flex_open(bytes, &factor);
                if (size + 1 > divisor) break;
                fseek(file, size + 1 - divisor, SEEK_CUR);
            }

            if ((number % 1000000) == 0) fspin(stdout, number);
            pos = ftell(file);
        }
        fclose(file);
        
        if (number > last) {
            printlf("Found all of  %\n", last);
            strcpy(outfile, argv[1]);
            
        } else {
            number--;
            printlf("Found up to  %  <  %. Renaming to : Numbers.%-%.dat\n",
                    number, last, first, number);
            
            sprintlf(outfile, "Numbers.%-%.dat", first, number);
            rename(argv[1], outfile);
        }

        struct stat filestat;
        stat(outfile, &filestat);
        if (filestat.st_size > pos) {
            int x = truncate(outfile, pos);
            printlf("Truncated from % to %\n", (long)filestat.st_size, pos);
        }
    }
}
