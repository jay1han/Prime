#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "number.h"
#include "flexint.h"
#include "longint.h"

int main(int argc, char **argv) {
    if (argc == 1 || argv[1][0] == '?') {
        
    } else {
        char outfile[64] = "";
        int filetype = 0;
        
        long first, last;
        sscanl(strchr(argv[1], '.') + 1, &first);
        sscanl(strchr(argv[1], '-') + 1, &last);

        if (strstr(argv[1], ".dat") != NULL) filetype = 1;
        else if (strstr(argv[1], ".red") != NULL) filetype = 2;
        else {
            printf("Unknown file type %s\n", argv[1]);
            exit(0);
        }

        FILE *file = fopen(argv[1], "rb");
        unsigned char bytes[10];
        long pos = 0, number;
        for (number = first; number <= last; number++) {
            if (fread(bytes, 1, 1, file) != 1) break;
            
            if (filetype == 1) {
                int divisors = bytes[0];
                int divisor;
                for (divisor = 0; divisor < divisors; divisor++) {
                    if (flex_read(file, NULL, NULL) < 0) break;
                    if (fread(bytes, 1, 1, file) != 1) break;
                }
                if (divisor < divisors) break;
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

            outfile[0] = 0;
            sprintlf(outfile, "Numbers.%-%.", first, number);
            if (filetype == 1) strcat(outfile, "dat");
            else if (filetype == 2) strcat(outfile, "red");
            rename(argv[1], outfile);
        }

        if (filetype == 1) {
            struct stat filestat;
            stat(outfile, &filestat);
            if (filestat.st_size > pos) {
                int x = truncate(outfile, pos);
                printlf("Truncated from % to %\n", (long)filestat.st_size, pos);
            }
        }
    }
}
