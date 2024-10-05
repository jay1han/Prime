CC := gcc -c -O -ggdb -MMD -MF
LD := gcc
ALLOBJECTS := prime.o worker.o prime5.o number.o flexint.o

prime5: $(ALLOBJECTS)
	$(LD) $^ -o $@

%.o: %.c
	$(CC) -MMD -MF $(basename $@).d -c $< -o $@

clean:
	-@rm *.o *.d prime5

prime4: prime4.c
	gcc -O prime4.c -o prime4

-include $(ALLOBJECTS:.o=.d)
