#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include "prime.h"

// Object-like structure to retrieve contiguous list of primes
// The list is the data, and callers use an iterator to go through the list

typedef struct iterator_s {
    int part, offset;
} iterator_t;

typedef struct seq_s {
    unsigned int span, offset;
    unsigned int *primes;
} seq_t;

static struct {
    int part, offset;
    unsigned int *primes[1000];
    seq_t *sequences;
    int threads;
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

static void ingest(init_t *init) {
    FILE *file;
    void *sequence;
    unsigned int first, last, prime;
    
    printf("Ingest %s from %u to %u\n", init->filename, init->first, init->last);
    sequence = seq_alloc(init->last - init->first + 1);

    file = fopen(init->filename, "rb");
    while (fread(&prime, sizeof(unsigned int), 1, file) == 1)
        seq_add(sequence, prime);
    fclose(file);
    primes_add_seq(sequence);
}

void primes_init(int threads, int is_init) {
    int num_files;
    struct dirent **p_dirlist, *p_dir;
    init_t *init;

    self.part = 0;
    self.offset = 0;
    self.primes[0] = calloc(sizeof(unsigned int), PART);
    self.threads = threads;
    self.sequences = calloc(sizeof(seq_t), threads);

    if (!is_init) {
        num_files = scandir(".", &p_dirlist, selprime, namesort);
        
        if (num_files > 0) {
            init = calloc(sizeof(init_t), num_files);
            
            for (int i = 0; i < num_files; i++) {
                struct dirent *p_dir = p_dirlist[i];
                strcpy(init[i].filename, p_dir->d_name);
                sscanf(strchr(init[i].filename, '_') + 1, "%u", &init[i].first);
                sscanf(strchr(init[i].filename, '-') + 1, "%u", &init[i].last);
            }
            
            for (int i = 0; i < num_files; i++) {
                ingest(&init[i]);
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

unsigned int prime_number(unsigned int prime_i) {
    int part = prime_i / PART;
    int offset = prime_i - (part * PART);
    return self.primes[part][offset];
}

void primes_write(char *filename, unsigned int from, unsigned int upto) {
    FILE *file = fopen(filename, "wb");
    void *iterator = prime_new();
    unsigned int prime = prime_next(iterator);

    while (prime < from) prime = prime_next(iterator);
    
    while (prime != 0 && prime <= upto) {
        fwrite(&prime, sizeof(unsigned int), 1, file);
        prime = prime_next(iterator);
    }
    fclose(file);
}

void primes_print(char *filename, unsigned int from, unsigned int upto) {
    FILE *file = fopen(filename, "w");
    void *iterator = prime_new();
    unsigned int prime = prime_next(iterator);

    while (prime < from) prime = prime_next(iterator);
    
    while (prime != 0 && prime <= upto) {
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

void *seq_alloc(unsigned int span) {
    seq_t *sequence;
    int i;

    for (i = 0; i < self.threads; i++) {
        if (self.sequences[i].primes == NULL) {
            sequence = &self.sequences[i];
            break;
        }
    }
    
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
    
    sequence->primes = NULL;
}
