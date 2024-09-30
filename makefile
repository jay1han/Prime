prime.dbg: prime.c
	gcc -ggdb prime.c -o prime.dbg

prime.opt: prime.c
	gcc -O prime.c -o prime.opt

primet.dbg: primet.c
	gcc -ggdb primet.c -o primet.dbg

primet.opt: primet.c
	gcc -O primet.c -o primet.opt

primep.dbg: primep.c
	gcc -ggdb primep.c -o primep.dbg

primep.opt: primep.c
	gcc -O primep.c -o primep.opt
