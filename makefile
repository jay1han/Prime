CC := gcc -c -O3 -ggdb -MMD -MF
LD := gcc
COMMON := prime.o worker.o number.o flexint.o longint.o
ALLOBJECTS := main.o print.o merge.o repair.o $(COMMON)
ALLEXECS := primes.exe print.exe merge.exe repair.exe

all: $(ALLEXECS)

primes.exe: $(COMMON) main.o
	$(LD) $^ -o $@

print.exe: $(COMMON) print.o
	$(LD) $^ -o $@

merge.exe: $(COMMON) merge.o
	$(LD) $^ -o $@

repair.exe: $(COMMON) repair.o
	$(LD) $^ -o $@

%.o: %.c
	$(CC) -MMD -MF $(basename $@).d -c $< -o $@

clean:
	-rm *.o *.d $(ALLEXECS)

-include $(ALLOBJECTS:.o=.d)
