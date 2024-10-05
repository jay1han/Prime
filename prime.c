#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include "prime.h"
#include "number.h"
#include "flexint.h"

// Object-like structure to retrieve contiguous list of primes
// The list is the data, and callers use an iterator to go through the list

typedef struct iterator_s {
    int part, offset;             // Index to self.bytes[part][offset]
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
    int count;                    // Number of primes
    long last;                    // Largest known prime
} self;

#define PART (1<<20)
#define DATA "Data"
#define LIST "List"

// Selects files starting with "Data"
static int seldata(const struct dirent *dir) {
    return strncmp(dir->d_name, DATA, 4) == 0;
}

// Selects files starting with "Data" or "List"
static int sellist(const struct dirent *dir) {
    return strncmp(dir->d_name, LIST, 4) == 0;
}

// Compare filenames according to first number after "Data"
static int namesort(const struct dirent **p_dir1, const struct dirent **p_dir2) {
    long num1, num2;

    sscanl((char*)&(*p_dir1)->d_name[5], &num1);
    sscanl((char*)&(*p_dir2)->d_name[5], &num2);
    if (num1 == num2) return 0;
    else if (num1 < num2) return -1;
    else return 1;
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

// Find file starting with "Data" and ingest it
void primes_init(int threads, int is_init) {
    self.part = 0;
    self.offset = 0;
    self.count = 0;
    self.last = 0;
    self.bytes[0] = malloc(PART);
    self.threads = threads;
    self.sequences = calloc(sizeof(seq_t), threads);

    if (!is_init) {
        int num_files;
        struct dirent **p_dirlist;
        
        num_files = scandir(".", &p_dirlist, seldata, namesort);
        if (num_files > 1) {
            printf("Too many Data files\n");
            exit(0);
        }

        if (num_files == 0) {
            printf("No Data file\n");
            exit(0);
        }
        
        char *filename = p_dirlist[0]->d_name;
        FILE *file;
        long first, last;
        sscanl(strchr(filename, '.') + 1, &first);
        sscanl(strchr(filename, '-') + 1, &last);

        printf("Ingest %s from ", filename);
        printlf("% to %\n", first, last);

        file = fopen(filename, "rb");

        while ((self.offset = fread(self.bytes[self.part], 1, PART, file)) == PART) {
            self.bytes[++self.part] = malloc(PART);
        }
        fclose(file);
        
        free(p_dirlist);

        for (int part = 0; part < self.part; part++)
            primes_scan(self.bytes[part], PART);
        primes_scan(self.bytes[self.part], self.offset);

        printlf("Ingested % primes, last=%", self.count, self.last);
        printpf(" RAM usage %\n", primes_size());
    }
}

// Add one prime
void primes_add(long prime) {
    if (prime <= self.last) return;

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
    iterator->part = 0;
    iterator->cumul = 0;
    return (void*)iterator;
}

// Return the next prime in list, 0 if none
inline long prime_next(void *arg, long *step) {
    iterator_t *this = (iterator_t*)arg;
    if (this->cumul > 1 && this->part == self.part && this->offset == self.offset)
        return 0;

    long flex;
    this->offset += flex_open(&self.bytes[this->part][this->offset], &flex);
    this->cumul += flex;
    if (this->offset > PART - 10) {
        this->part++;
        this->offset = 0;
    }

    if (step != NULL) *step = flex;
    return this->cumul;
}

// Return the absolute index of current prime
inline long prime_index(void *arg) {
    iterator_t *this = (iterator_t*)arg;
    return this->part * PART + this->offset - 1;
}

// Release iterator
void prime_end(void *arg) {
    free(arg);
}

long primes_count() {
    return self.count;
}

long primes_last() {
    return self.last;
}

long primes_size() {
    return (long)PART * (self.part + 1);
}

// Write the full list of primes, erasing the previous files
void primes_write(long upto, int do_list) {
    struct dirent **p_dirlist, *p_dir;
    int num_files;

    num_files = scandir(".", &p_dirlist, seldata, NULL);
    for (int i = 0; i < num_files; i++) {
        struct dirent *p_dir = p_dirlist[i];
        unlink(p_dir->d_name);
    }
    if (num_files > 0) free(p_dirlist);
    
    char datafile[64];
    sprintf(datafile, "%s.", DATA);
    sprintlf(datafile, "2-%.dat", upto);
    FILE *data = fopen(datafile, "wb");
        
    for (int part = 0; part < self.part; part++)
        fwrite(self.bytes[part], 1, PART, data);
    fwrite(self.bytes[self.part], 1, self.offset, data);
    fclose(data);
    
    if (do_list) {
        
        num_files = scandir(".", &p_dirlist, sellist, NULL);
        for (int i = 0; i < num_files; i++) {
            struct dirent *p_dir = p_dirlist[i];
            unlink(p_dir->d_name);
        }
        if (num_files > 0) free(p_dirlist);
        
        char listfile[64];
        sprintf(listfile, "%s.", LIST);
        sprintlf(listfile, "2-%.lst", upto);
        FILE *list = fopen(listfile, "w");

        void *iterator = prime_new();
        long prime = prime_next(iterator, NULL);
        unsigned char bytes[10];
        
        while (prime != 0) {
            fprintf(list, "%lu\n", prime);
            prime = prime_next(iterator, NULL);
        }
        
        fclose(list);
    }
}

// Return a sequence
void *seq_alloc(long size) {
    seq_t *sequence;
    int i;

    for (i = 0; i < self.threads; i++) {
        if (self.sequences[i].primes == NULL) {
            sequence = &self.sequences[i];
            break;
        }
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
    
    sequence->primes = NULL;
}
