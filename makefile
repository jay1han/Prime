prime.dbg: prime.c
	gcc -ggdb prime.c -o prime.dbg

prime.opt: prime.c
	gcc -O prime.c -o prime.opt
