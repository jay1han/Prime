CC := gcc -c -O3 -ggdb -MMD -MF
LD := gcc
COMMON := prime.o worker.o number.o flexint.o
ALLOBJECTS := main.o print.o merge.o $(COMMON)

all: primes print merge

primes: $(COMMON) main.o
	$(LD) $^ -o $@

print: $(COMMON) print.o
	$(LD) $^ -o $@

merge: $(COMMON) merge.o
	$(LD) $^ -o $@

%.o: %.c
	$(CC) -MMD -MF $(basename $@).d -c $< -o $@

clean:
	-rm *.o *.d primes print

-include $(ALLOBJECTS:.o=.d)
