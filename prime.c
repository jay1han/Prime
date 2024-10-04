#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include "prime.h"
#include "pack.h"

// Object-like structure to retrieve contiguous list of primes
// The list is the data, and callers use an iterator to go through the list

typedef struct iterator_s {
    int part, offset;
} iterator_t;

static struct {
    int part, offset;
    unsigned int *primes[1000];
} self;

#define PART 1000000

static int selprime(const struct dirent *dir) {
    return strncmp(dir->d_name, "PrimeData", 9) == 0;
}

static int namesort(const struct dirent **p_dir1, const struct dirent **p_dir2) {
    unsigned int num1, num2;

    sscanf(&(*p_dir1)->d_name[9], "%u", &num1);
    sscanf(&(*p_dir2)->d_name[9], "%u", &num2);
    if (num1 == num2) return 0;
    else if (num1 < num2) return -1;
    else return 1;
}

typedef struct init_s {
    unsigned int first, last;
    char filename[64];
} init_t;

static void ingest(char *filename, init_t *init) {
    // TODO
    printf("Ingest %s\n", filename);
}

void primes_init(int is_init) {
    int num_files;
    struct dirent **p_dirlist, *p_dir;
    init_t *init;

    self.part = 0;
    self.offset = 0;
    self.primes[0] = calloc(sizeof(unsigned int), PART);

    if (!is_init) {
        num_files = scandir(".", &p_dirlist, selprime, namesort);
        
        if (num_files > 0) {
            init = calloc(sizeof(init_t), num_files);
            for (int i = 0; i < num_files; i++) {
                struct dirent *p_dir = &(*p_dirlist)[i];
                ingest(p_dir->d_name, &init[i]);
            }
            free(init);
            free(p_dirlist);
        }
    }
}

void primes_add(unsigned int prime) {
    self.primes[self.part][self.offset] = prime;
    if (self.offset == PART - 1) {
        self.primes[++self.part] = calloc(sizeof(unsigned int), PART);
        self.offset = 0;
    } else self.offset++;
}

unsigned int primes_count() {
    return self.part * PART + self.offset;
}

void primes_write(char *filename) {
    FILE *file = fopen(filename, "wb");
    void *iterator = prime_new();
    unsigned int prime = prime_next(iterator);

    // TODO write binary
    while (prime != 0) {
        fprintf(file, "%u\n", prime);
        prime = prime_next(iterator);
    }
    fclose(file);
}

void primes_print(char *filename) {
    FILE *file = fopen(filename, "w");
    void *iterator = prime_new();
    unsigned int prime = prime_next(iterator);

    while (prime != 0) {
        fprintf(file, "%u\n", prime);
        prime = prime_next(iterator);
    }
    fclose(file);
}

void *prime_new() {
    iterator_t *iterator = (iterator_t*)malloc(sizeof(iterator_t));
    iterator->offset = 0;
    iterator->part = 0;
    return (void*)iterator;
}

unsigned int prime_next(void *arg) {
    iterator_t *this = (iterator_t*)arg;
    if (this->part == self.part && this->offset == self.offset) return 0;
    
    unsigned int prime = self.primes[this->part][this->offset];
    if (this->offset == PART - 1) {
        this->part++;
        this->offset = 0;
    } else this->offset++;
    return prime;
}

unsigned int prime_index(void *arg) {
    iterator_t *this = (iterator_t*)arg;
    return this->part * PART + this->offset - 1;
}

void prime_end(void *arg) {
    free(arg);
}

typedef struct seq_s {
    unsigned int span, offset;
    unsigned int *primes;
} seq_t;

void *seq_alloc(unsigned int span) {
    seq_t *sequence = malloc(sizeof(seq_t));

    sequence->span   = span / 2;
    sequence->offset = 0;
    sequence->primes = malloc(sizeof(unsigned int) * span / 2);
    return (void*)sequence;
}

void seq_add(void *arg, unsigned int prime) {
    seq_t *sequence = (seq_t*)arg;

    sequence->primes[sequence->offset] = prime;
    sequence->offset++;
}

void primes_add_seq(void *arg) {
    seq_t *sequence = (seq_t*)arg;

    if (self.offset + sequence->offset < PART) {
        memcpy(
            self.primes[self.part] + self.offset,
            sequence->primes,
            sizeof(unsigned int) * sequence->offset
            );
        self.offset += sequence->offset;
    } else {
        unsigned int part = PART - self.offset;
        memcpy(
            self.primes[self.part] + self.offset,
            sequence->primes,
            sizeof(unsigned int) * part
            );
        self.part++;
        self.primes[self.part] = calloc(sizeof(unsigned int), PART);
        self.offset = sequence->offset - part;
        memcpy(
            self.primes[self.part],
            &sequence->primes[part],
            sizeof(unsigned int) * self.offset
            );
    }
    
    free(arg);
}
