#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include "prime.h"
#include "number.h"
#include "flexint.h"
#include "longint.h"

// Object-like structure to retrieve contiguous list of primes
// The list is the data, and callers use an iterator to go through the list

typedef struct iterator_s {
    int part, offset;             // Pointer to next self.bytes[part][offset]
    long index;                    // Index of current prime (start with index 1 = number 2)
    long cumul;                   // Current prime
} iterator_t;

typedef struct seq_s {
    int offset;                   // Current pointer in sequence
    long *primes;                 // Array of primes
} seq_t;

static struct {
    int part, offset;             // Indexes to bytes[part][offset]
    unsigned char *bytes[3000];
    seq_t *sequences;             // Pool of sequences
    int threads;                  // Max number of threads (sequences)
    long count;                   // Number of primes
    long new;                     // Number of new primes (determines whether to write)
    long last;                    // Largest known prime
    long upto;
    char filename[64];
    FILE *file;
    struct {
        int part, offset;
    } written;
} self;

#define PART (1<<20)
#define DATA "Primes"

// Selects files starting with "Data"
static int seldata(const struct dirent *dir) {
    return strncmp(dir->d_name, DATA, strlen(DATA)) == 0;
}

// Reverse compare so we get the largest file
static int compdata(const struct dirent **pf1,
                    const struct dirent **pf2) {
    long last1, last2;

    sscanl(strchr((*pf1)->d_name, '.') + 1, &last1);
    sscanl(strchr((*pf2)->d_name, '.') + 1, &last2);
    if (last1 < last2) return 1;
    if (last1 > last2) return -1;
    return 0;
}

static void primes_scan(unsigned char *bytes, int size) {
    long step;

    for (int offset = 0; offset < size; ) {
        offset += flex_open(&bytes[offset], &step);
        if (step == 0) return;
        self.last += step;
        self.count++;
    }
}

// Find file starting with "Data" and ingest it, return last known prime
long primes_init(int threads, int is_init, long upto, int do_print) {
    self.part = 0;
    self.offset = 0;
    self.count = 0;
    self.new = 0;
    self.last = 0;
    self.upto = upto;
    self.bytes[0] = malloc(PART);
    self.threads = threads;
    self.sequences = calloc(sizeof(seq_t), threads);

    sprintf(self.filename, "%s.", DATA);
    sprintlf(self.filename, "%.dat", self.upto);
    self.file = NULL;
    
    long previous;
    if (is_init) previous = 1;
    else {
        int num_files;
        struct dirent **p_dirlist;
        
        num_files = scandir(".", &p_dirlist, seldata, compdata);
        if (num_files == 0) {
            fprintf(stderr, "No Primes file\n");
            exit(0);
        }

        for (int i = 1; i < num_files; i++) {
            fprintf(stderr, "Remove unused %s\n", p_dirlist[i]->d_name);
            unlink(p_dirlist[i]->d_name);
        }
        
        char *filename = p_dirlist[0]->d_name;
        if (strcmp(filename, self.filename) == 0) self.filename[0] = 0;
        
        FILE *file;
        sscanl(strchr(filename, '.') + 1, &previous);

        if (do_print) {
            fprintf(stderr, "%s  up to ", filename);
            fprintlf(stderr, " % ", previous);
            fflush(stderr);
        }
        
        file = fopen(filename, "rb");

        while ((self.offset = fread(self.bytes[self.part], 1, PART, file)) == PART) {
            self.bytes[++self.part] = malloc(PART);
        }
        fclose(file);
        
        free(p_dirlist);

        for (int part = 0; part < self.part; part++)
            primes_scan(self.bytes[part], PART);
        primes_scan(self.bytes[self.part], self.offset);
        
        if (do_print) {
            fprintlf(stderr, " :  %  primes, last  %,  RAM ", self.count, self.last);
            fprintp(stderr, primes_size());
            fprintf(stderr, "\n");
        }
    }

    if (self.filename[0]) self.file = fopen(self.filename, "wb");
    self.written.part = 0;
    self.written.offset = 0;
        
    return previous;
}

// Add one prime
void primes_add(long prime) {
    if (prime <= self.last) return;

    self.new++;
    self.offset += flex_fold(prime - self.last, &self.bytes[self.part][self.offset]);
    self.count++;
    if (self.offset > PART - 10) {
        self.bytes[self.part][self.offset] = 0;
        self.bytes[++self.part] = malloc(PART);
        self.offset = 0;
    }
    
    self.last = prime;
}

// Create a new iterator
void *prime_new() {
    iterator_t *iterator = (iterator_t*)malloc(sizeof(iterator_t));
    iterator->offset = 0;
    iterator->part   = 0;
    iterator->index  = 0;
    iterator->cumul  = 0;
    return (void*)iterator;
}

// Return the next prime in list, 0 if none
inline long prime_next(void *arg, long *step) {
    iterator_t *this = (iterator_t*)arg;
    if (this->index >= self.count) return 0;

    long flex;
    this->offset += flex_open(&self.bytes[this->part][this->offset], &flex);
    this->cumul += flex;
    if (this->offset > PART - 10) {
        this->part++;
        this->offset = 0;
    }
    this->index++;

    if (step != NULL) *step = flex;
    return this->cumul;
}

// Return the byte index of current prime
inline long prime_index(void *arg) {
    iterator_t *this = (iterator_t*)arg;
    return this->index;
}

// Return the current value
inline long prime_value(void *arg) {
    iterator_t *this = (iterator_t*)arg;
    return this->cumul;
}

// Return the index of number, or 0 if not found
inline long prime_find(void *arg, long number) {
    iterator_t *this = (iterator_t*)arg;

    if (number > self.last) return 0;
    while (this->cumul < number) {
        if (this->index >= self.count) return 0;
        
        long flex;
        this->offset += flex_open(&self.bytes[this->part][this->offset], &flex);
        this->cumul += flex;
        if (this->offset > PART - 10) {
            this->part++;
            this->offset = 0;
        }
        this->index++;
    }

    if (this->cumul == number) return this->index;
    return 0;
}

// Release iterator
void prime_end(void *arg) {
    free(arg);
}

inline long prime_get(long index) {
    void *prime = prime_new();
    long found;

    for (long i = 0; i < index; i++) found = prime_next(prime, NULL);
    prime_end(prime);

    return found;
}

inline long primes_count() {
    return self.count;
}

inline long primes_last() {
    return self.last;
}

inline long primes_size() {
    return (long)PART * (self.part + 1);
}

// Write a list of primes
void primes_write() {
    if (!self.file) return;
    if (!self.new) return;
    
    if (self.written.part == self.part)
        fwrite(self.bytes[self.written.part] + self.written.offset, 1,
               self.offset - self.written.offset, self.file);
    else {
        fwrite(self.bytes[self.written.part] + self.written.offset, 1,
               PART - self.written.offset, self.file);
        for (int part = self.written.part + 1; part < self.part; part++)
            fwrite(self.bytes[part], 1, PART, self.file);
        fwrite(self.bytes[self.part], 1, self.offset, self.file);
    }

    self.written.part   = self.part;
    self.written.offset = self.offset;
}

void primes_print() {
    void *iterator = prime_new();
    long prime;

    while ((prime = prime_next(iterator, NULL)) != 0)
        fprintlf(stdout, "%\n", prime);
    prime_end(iterator);
}

// Erase previous files
void primes_close(int cancel) {
    if (!self.file) return;
    
    fclose(self.file);
    if (cancel > 0) unlink(self.filename);
    else if (self.filename[0]) {
        struct dirent **p_dirlist, *p_dir;
        int num_files = scandir(".", &p_dirlist, seldata, NULL);
        for (int i = 0; i < num_files; i++) {
            struct dirent *p_dir = p_dirlist[i];
            if (strcmp(p_dir->d_name, self.filename))
                unlink(p_dir->d_name);
        }
        if (num_files > 0) free(p_dirlist);
    }
}

// Return a sequence
void *seq_alloc(long size) {
    seq_t *sequence = NULL;
    int i;

    for (i = 0; i < self.threads; i++) {
        if (self.sequences[i].primes == NULL) {
            sequence = &self.sequences[i];
            break;
        }
    }
    if (sequence == NULL) {
        fprintf(stderr, "Panic: sequences\n");
        exit(0);
    }
    
    sequence->offset = 0;
    sequence->primes = calloc(sizeof(long), size);
    return (void*)sequence;
}

// Add a prime to sequence
inline void seq_add(void *arg, long prime) {
    seq_t *sequence = (seq_t*)arg;

    sequence->primes[sequence->offset] = prime;
    sequence->offset++;
}

// Merge sequence into main list
void primes_add_seq(void *arg) {
    seq_t *sequence = (seq_t*)arg;

    for (int count = 0; count < sequence->offset; count++)
        primes_add(sequence->primes[count]);

    free(sequence->primes);
    sequence->primes = NULL;
}

