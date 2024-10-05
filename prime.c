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
    long span, offset;
    long *primes;
} seq_t;

static struct {
    int part, offset;
    long *primes[1000];
    seq_t *sequences;
    int threads;
    long last_init;
} self;

#define PART 1000000

static int selprime(const struct dirent *dir) {
    return strncmp(dir->d_name, "PrimeData", 9) == 0;
}

static int namesort(const struct dirent **p_dir1, const struct dirent **p_dir2) {
    long num1, num2;

    sscanf(&(*p_dir1)->d_name[10], "%lu", &num1);
    sscanf(&(*p_dir2)->d_name[10], "%lu", &num2);
    if (num1 == num2) return 0;
    else if (num1 < num2) return -1;
    else return 1;
}

typedef struct init_s {
    long first, last;
    char filename[64];
} init_t;

static long ingest(init_t *init) {
    FILE *file;
    void *sequence;
    long prime, count, size;
    
    printf("Ingest %s from %lu to %lu\n", init->filename, init->first, init->last);

    file = fopen(init->filename, "rb");
    
    size = init->last - init->first + 1;
    if (size > PART) size = PART;
    sequence = seq_alloc(size);
    
    count = 0;
    while (fread(&prime, sizeof(long), 1, file) == 1) {
        seq_add(sequence, prime);
        if (++count == PART) {
            primes_add_seq(sequence);
            count = 0;
            sequence = seq_alloc(size);
        }
    }
    fclose(file);
    
    if (count > 0) primes_add_seq(sequence);
    return prime;
}

void primes_init(int threads, int is_init) {
    int num_files;
    struct dirent **p_dirlist, *p_dir;
    init_t *init;

    self.part = 0;
    self.offset = 0;
    self.primes[0] = calloc(sizeof(long), PART);
    self.threads = threads;
    self.sequences = calloc(sizeof(seq_t), threads);

    if (!is_init) {
        num_files = scandir(".", &p_dirlist, selprime, namesort);
        
        if (num_files > 0) {
            init = calloc(sizeof(init_t), num_files);
            
            for (int i = 0; i < num_files; i++) {
                struct dirent *p_dir = p_dirlist[i];
                strcpy(init[i].filename, p_dir->d_name);
                sscanf(strchr(init[i].filename, '_') + 1, "%lu", &init[i].first);
                sscanf(strchr(init[i].filename, '-') + 1, "%lu", &init[i].last);
            }
            
            for (int i = 0; i < num_files; i++) {
                self.last_init = ingest(&init[i]);
            }
            
            free(init);
            free(p_dirlist);
        }
    }
}

void primes_add(long prime) {
    if (prime <= self.last_init) return;
    
    self.primes[self.part][self.offset] = prime;
    if (self.offset == PART - 1) {
        self.primes[++self.part] = calloc(sizeof(long), PART);
        self.offset = 0;
    } else self.offset++;
}

long primes_count() {
    return self.part * PART + self.offset;
}

long prime_number(long prime_i) {
    int part = prime_i / PART;
    int offset = prime_i - (part * PART);
    return self.primes[part][offset];
}

void primes_write(char *filename, long from, long upto) {
    FILE *file = fopen(filename, "wb");
    void *iterator = prime_new();
    long prime = prime_next(iterator);

    while (prime < from) prime = prime_next(iterator);
    
    while (prime != 0 && prime <= upto) {
        fwrite(&prime, sizeof(long), 1, file);
        prime = prime_next(iterator);
    }
    fclose(file);
}

void primes_print(char *filename, long from, long upto) {
    FILE *file = fopen(filename, "w");
    void *iterator = prime_new();
    long prime = prime_next(iterator);

    while (prime < from) prime = prime_next(iterator);
    
    while (prime != 0 && prime <= upto) {
        fprintf(file, "%lu\n", prime);
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

long prime_next(void *arg) {
    iterator_t *this = (iterator_t*)arg;
    if (this->part == self.part && this->offset == self.offset) return 0;
    
    long prime = self.primes[this->part][this->offset];
    if (this->offset == PART - 1) {
        this->part++;
        this->offset = 0;
    } else this->offset++;
    return prime;
}

long prime_index(void *arg) {
    iterator_t *this = (iterator_t*)arg;
    return this->part * PART + this->offset - 1;
}

void prime_end(void *arg) {
    free(arg);
}

void *seq_alloc(long span) {
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
    sequence->primes = malloc(sizeof(long) * span / 2);
    return (void*)sequence;
}

void seq_add(void *arg, long prime) {
    seq_t *sequence = (seq_t*)arg;

    sequence->primes[sequence->offset] = prime;
    sequence->offset++;
}

void primes_add_seq(void *arg) {
    seq_t *sequence = (seq_t*)arg;

    if (sequence->offset == 0) return;
    
    if (sequence->primes[0] < self.last_init) {
        // Must copy one by one
        
    } else {

        if (self.offset + sequence->offset < PART) {
            memcpy(
                self.primes[self.part] + self.offset,
                sequence->primes,
                sizeof(long) * sequence->offset
                );
            self.offset += sequence->offset;
            
        } else {
            long part = PART - self.offset;
            memcpy(
                self.primes[self.part] + self.offset,
                sequence->primes,
                sizeof(long) * part
                );
            self.part++;
            self.primes[self.part] = calloc(sizeof(long), PART);
            self.offset = sequence->offset - part;
            memcpy(
                self.primes[self.part],
                &sequence->primes[part],
                sizeof(long) * self.offset
                );
        }
    }
    
    sequence->primes = NULL;
}
