#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <time.h>
#include "number.h"
#include "flexint.h"
#include "worker.h"
#include "longint.h"

#define NUMBERS "Numbers"

static void truncated() {
    printf("Truncated file, needs repair\n");
    exit(0);
}

static void help() {
    printf("First argument\n");
    printf("\tn\tuse .dat files\n");
    printf("\tr\tuse .red files\n");
    printf("Second argument (optional)\n");
    printf("\t<dir>\toutput directory\n");
    exit(0);
}

static int seldat(const struct dirent *dir) {
    return (strncmp(dir->d_name, NUMBERS, strlen(NUMBERS)) == 0) &&
        (strstr(dir->d_name, ".dat") != NULL);
}

static int selred(const struct dirent *dir) {
    return (strncmp(dir->d_name, NUMBERS, strlen(NUMBERS)) == 0) &&
        (strstr(dir->d_name, ".red") != NULL);
}

typedef struct span_s {
    long first, last;
    char filename[64];
    long filesize;
} span_t;
static span_t span[100];
static int spans = 0;

static int compspan(const void *a, const void *b) {
    span_t *span_a = (span_t*)a;
    span_t *span_b = (span_t*)b;

    if (span_a->first < span_b->first) return -1;
    else if (span_a->first > span_b->first) return 1;
    else if (span_b->last < span_a->last) return -1;
    else return 1;
}

int main(int argc, char **argv) {
    struct dirent **p_dirlist;
    struct statvfs dirstat;
    long totalsize = 0;
    long maxsize = 0;
    int (*selector)(const struct dirent*);
    char *outdir = ".";

    if (argc == 1) help();
    else {
	if (argc > 2) outdir = argv[2];
        if (argv[1][0] == 'n') selector = seldat;
        else if (argv[1][0] == 'r') selector = selred;
        else help();
    }

    int num_files = scandir(".", &p_dirlist, selector, NULL);
    if (num_files <= 1) {
        printf("Nothing to merge\n");
        exit(0);
    }
    
    if (num_files > 100) {
        printf("Too many files\n");
        exit(0);
    }

    for (int i = 0; i < num_files; i++) {
        char *filename = p_dirlist[i]->d_name;
        long first, last;
        struct stat filestat;

        sscanl(strchr(filename, '.') + 1, &first);
        sscanl(strchr(filename, '-') + 1, &last);
        span[spans].first = first;
        span[spans].last = last;
        strcpy(span[spans].filename, filename);
        stat(filename, &filestat);
        span[spans].filesize = filestat.st_size;
        totalsize += filestat.st_size;
        if (filestat.st_size > maxsize) maxsize = filestat.st_size;
        spans++;
    }

    qsort(span, spans, sizeof(span_t), compspan);
    for (int i = 1; i < spans; i++) {
        if (span[i].first <= span[i-1].last) 
            span[i-1].last = span[i].first - 1;
        else if (span[i].first > span[i-1].last + 1) {
            printf("Disjoint files\n");
            exit(0);
        }
    }

    statvfs(".", &dirstat);
    if (dirstat.f_bsize * dirstat.f_bfree <= maxsize * 2) {
        printf("File size ");
        fprintp(stdout, maxsize);
        printf(" too large for remaining ");
        fprintp(stdout, dirstat.f_bsize * dirstat.f_bfree);
        printf("\n");
        exit(0);
    }

    long first = span[0].first;
    long last = span[spans - 1].last;
    
    if (span[0].last >= span[spans - 1].last) {
        printf("%s has superset\n", span[0].filename);
        for (int i = 1; i < spans; i++) unlink(span[i].filename);
        exit(0);
    }
    
    char filename[128];
    sprintf(filename, "%s/%s.", outdir, NUMBERS);
    sprintlf(filename, "%-%.", first, last);
    if (selector == selred) strcat(filename, "red");
    else strcat(filename, "dat");
    printf("Output %s\n", filename);
    
    if (argc > 1 && argv[1][0] == '?') exit(0);
    
    FILE *output = fopen(filename, "wb");

    time_t start = time(NULL);
    unsigned char bytes[256];
    for (int i = 0; i < spans; i++) {
        FILE *input = fopen(span[i].filename, "rb");

        if (selector == selred) {
            printf("Copying %s : ", span[i].filename);
            fprintlf(stdout, " % - %\n", span[i].first, span[i].last);

            void *buffer = malloc(1<<20);
            int copy, actual, size = span[i].last - span[i].first + 1;
            do {
                if (size < (1<<20)) copy = size;
                else copy = 1<<20;
                if ((actual = fread(buffer, 1, copy, input)) < copy) {
                    printf("File too small, please repair\n");
                    exit(0);
                }
                fwrite(buffer, 1, copy, output);
                size -= copy;
            } while (size > 0);
	    free(buffer);
            
        } else {
            printf("Analyzing %s : ", span[i].filename);
            fprintlf(stdout, " % - %\n", span[i].first, span[i].last);

            long number = span[i].first;
            while (number <= span[i].last) {
                if ((i < spans - 1) && (number >= span[i + 1].first)) break;

                if (fread(bytes, 1, 1, input) < 1) truncated();
                fwrite(bytes, 1, 1, output);
            
                int divisors = bytes[0];
                for (int i = 0; i < divisors; i++) {
                    int size = flex_read(input, NULL, bytes);
                    if (size < 0) truncated();
                    if (fread(bytes + size, 1, 1, input) != 1) truncated();
                    fwrite(bytes, 1, size + 1, output);
                }
                number++;

                if ((number % 1000000) == 0) {
                    fprintt(stdout, time(NULL) - start);
                    fprintlf(stdout, "  %\r", number);
                    fflush(stdout);
                }
            }
        }
        
        fclose(input);
        fflush(output);
        unlink(span[i].filename);
    }
    fclose(output);
    printf("Output %s : ", filename);
    fprintlf(stdout, " % - %\n", first, last);

    for (int i = 0; i < spans; i++) unlink(span[i].filename);
}
