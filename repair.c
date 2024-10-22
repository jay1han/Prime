#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include "number.h"
#include "flexint.h"
#include "longint.h"

#define NUMBERS   1
#define REDUCED   2
#define PRIMES    3

int main(int argc, char **argv) {
    if (argc == 1 || argv[1][0] == '?') {
        printf("Repairs Numbers..dat, Numbers..red and Primes..dat files\n");
        
    } else {
        struct stat filestat;
        char outfile[64] = "";
        int filetype = 0;
        
        if (strstr(argv[1], ".dat") != NULL) {
            if (strncmp(argv[1], "Primes", 6) == 0) filetype = PRIMES;
            else filetype = NUMBERS;
        } else if (strstr(argv[1], ".red") != NULL) filetype = REDUCED;
        else {
            printf("Unknown file type %s\n", argv[1]);
            exit(0);
        }

        long first, last, number, pos;
	
        switch (filetype) {
        case NUMBERS:
        case REDUCED:
            sscanl(strchr(argv[1], '.') + 1, &first);
            sscanl(strchr(argv[1], '-') + 1, &last);
            break;

        case PRIMES:
            first = 0;
            sscanl(strchr(argv[1], '.') + 1, &last);
            break;
        }

	
	if (filetype == REDUCED) {
	    stat(argv[1], &filestat);
	    number = first + filestat.st_size - 1;
            pos = number - first + 1;
            
	} else {
	    FILE *file = fopen(argv[1], "rb");
	    if (file == NULL) {
		printf("File not found\n");
		exit(0);
	    }
        
	    time_t start = time(NULL);
	    fprintf(stdout, "Start\r");

	    unsigned char bytes[10];
	    int stop = 0, size, flexsize;
	    
	    for (number = first; number <= last; number++) {
		long step;
            
		switch (filetype) {
		case PRIMES:
		    if ((flexsize = flex_read(file, &step, NULL)) < 0) stop = 1;
		    else {
			number += step - 1;
			pos += flexsize;
		    }
		    break;
                
		case NUMBERS:
		    if (fread(bytes, 1, 1, file) != 1) stop = 1;
		    else {
			size = 1;
			int divisors = bytes[0];
			for (int i = 0; i < divisors; i++) {
			    if ((flexsize = flex_read(file, NULL, NULL)) < 0) stop = 1;
			    else {
				if (fread(bytes, 1, 1, file) != 1) stop = 1;
				else size += flexsize + 1;
			    }
			}
			if (!stop) pos += size;
		    }
		}
		if (stop) break;
            
		if ((number % 1000000) == 0) {
		    printf("  ");
		    fprintt(stdout, time(NULL) - start);
		    fprintlf(stdout, "  %\r", number);
		    fflush(stdout);
		}
		pos = ftell(file);
	    }
	    
	    number--;
	    fclose(file);
	}
        
        if (number >= last) {
            fprintlf(stdout, "\nFound all of  %\n", last);
            strcpy(outfile, argv[1]);
            
        } else {
            switch(filetype) {
            case PRIMES:
                sprintlf(outfile, "Primes.%.dat", number);
                break;

            case REDUCED:
                sprintlf(outfile, "Numbers.%-%.red", first, number);
                break;

            case NUMBERS:
                sprintlf(outfile, "Numbers.%-%.dat", first, number);
                break;
            }
            
            fprintlf(stdout, "\nFound up to  %  <  %.  Renaming to :  ", number, last);
            printf("%s\n", outfile);
            rename(argv[1], outfile);
        }
        
        stat(outfile, &filestat);
        if (filestat.st_size > pos) {
            int x = truncate(outfile, pos);
            fprintlf(stdout, "Truncated size from  %  to  %\n", (long)filestat.st_size, pos);
        }
    }
}
