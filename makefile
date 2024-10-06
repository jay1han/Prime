CC := gcc -c -O3 -ggdb -MMD -MF
LD := gcc
ALLOBJECTS := prime.o worker.o number.o flexint.o

all: primes print

primes: $(ALLOBJECTS) main.o
	$(LD) $^ -o $@

print: ${ALLOBJECTS} print.o
	$(LD) $^ -o $@

%.o: %.c
	$(CC) -MMD -MF $(basename $@).d -c $< -o $@

clean:
	-rm *.o *.d primes

-include $(ALLOBJECTS:.o=.d)
