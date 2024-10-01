prime4: prime4.c
	gcc -O prime4.c -o prime4

prime5: prime.o decomp.o worker.o prime5.o
	ld prime5.o prime.o decomp.o -o prime5

prime5.o: prime.h prime5.c
	gcc -c -O prime5.c -o prime5.o

prime.o: prime.h prime.c
	gcc -c -O prime.c -o prime.o

decomp.o: prime.h decomp.h decomp.c
	gcc -c -O decomp.c -o decomp.o

worker.o: prime.h worker.h worker.c
	gcc -c -O worker.c -o worker.o
