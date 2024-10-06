CC := gcc -c -O3 -ggdb -MMD -MF
LD := gcc
COMMON := prime.o worker.o number.o flexint.o
ALLOBJECTS := main.o print.o $(COMMON)

all: primes print

primes: $(COMMON) main.o
	$(LD) $^ -o $@

print: $(COMMON) print.o
	$(LD) $^ -o $@

%.o: %.c
	$(CC) -MMD -MF $(basename $@).d -c $< -o $@

clean:
	-rm *.o *.d primes print

-include $(ALLOBJECTS:.o=.d)
