CC := gcc -c -O -MMD -MF
LD := gcc
ALLOBJECTS := prime.o decomp.o worker.o prime5.o number.o

prime5: $(ALLOBJECTS)
	$(LD) $^ -o $@

%.o: %.c
	$(CC) -MMD -MF $(basename $@).d -c $< -o $@

clean:
	-@rm *.o *.d prime5

prime4: prime4.c
	gcc -O prime4.c -o prime4

-include $(ALLOBJECTS:.o=.d)
