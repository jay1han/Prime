CC := gcc -c -O3 -ggdb -MMD -MF
LD := gcc
ALLOBJECTS := prime.o worker.o main.o number.o flexint.o

primes: $(ALLOBJECTS)
	$(LD) $^ -o $@

%.o: %.c
	$(CC) -MMD -MF $(basename $@).d -c $< -o $@

clean:
	-rm *.o *.d primes

-include $(ALLOBJECTS:.o=.d)
