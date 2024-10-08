#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include "number.h"
#include "flexint.h"
#include "worker.h"
#include "longint.h"

#define NUMBERS "Numbers"

static void help() {
    printf("Options\n");
    printf("\tn\tuse .dat files\n");
    printf("\tr\tuse .red files\n");
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

    if (argc == 1) help();
    else {
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
    for (int i = 0; i < spans; i++) {
        printf("%s :", span[i].filename);
        printlf(" % - % ", span[i].first, span[i].last);
        printpf(": %\n", span[i].filesize);
    }

    statvfs(".", &dirstat);
    if (dirstat.f_bsize * dirstat.f_bfree <= maxsize * 2) {
        printpf("File size % too large for remaining %\n",
                maxsize, dirstat.f_bsize * dirstat.f_bfree);
        exit(0);
    }

    for (int i = 1; i < spans; i++) {
        if (span[i].last < span[i + 1].first -1) {
            printf("Disjoint files\n");
            exit(0);
        }
    }

    long first = span[0].first;
    long last = span[spans - 1].last;
    
    if (span[0].last >= span[spans - 1].last) {
        printf("%s has superset\n", span[0].filename);
        for (int i = 1; i < spans; i++) unlink(span[i].filename);
        exit(0);
    }
    
    char filename[64];
    sprintf(filename, "%s.", NUMBERS);
    sprintlf(filename, "%-%.", first, last);
    if (selector == selred) strcat(filename, "red");
    else strcat(filename, "dat");
    printf("Output %s\n", filename);
    
    if (argc > 1 && argv[1][0] == '?') exit(0);
    
    FILE *output = fopen(filename, "wb");

    unsigned char bytes[256];
    for (int i = 0; i < spans; i++) {
        FILE *input = fopen(span[i].filename, "rb");

        if (selector == selred) {
            printf("Copying %s : ", span[i].filename);
            printlf(" % - %\n", span[i].first, span[i].last);

            void *buffer = malloc(1<<20);
            int size;
            do {
                size = fread(buffer, 1, 1<<20, input);
                fwrite(buffer, 1, size, output);
            } while (size == 1<<20);
            
        } else {
            printf("Analyzing %s : ", span[i].filename);
            printlf(" % - %\n", span[i].first, span[i].last);

            long number = span[i].first;
            while (number <= span[i].last) {
                if ((i < spans - 1) && (number >= span[i + 1].first)) break;

                if (fread(bytes, 1, 1, input) < 1) {
                    printf("File too short, needs repair\n");
                    exit(0);
                }
                fwrite(bytes, 1, 1, output);
            
                int divisors = bytes[0];
                for (int divisor = 0; divisor < divisors; divisor++) {
                    int content = fread(bytes, 1, 10, input);
                    long factor;
                    int size = flex_open(bytes, &factor);
                    fseek(input, size + 1 - content, SEEK_CUR);
                    fwrite(bytes, 1, size + 1, output);
                }
                number++;

                if ((number % 1000000) == 0) fspin(stdout, number);
            }
        }
        
        fclose(input);
        fflush(output);
        unlink(span[i].filename);
    }
    fclose(output);
    printf("Output %s : ", filename);
    printlf(" % - %\n", first, last);

    for (int i = 0; i < spans; i++) unlink(span[i].filename);
}
