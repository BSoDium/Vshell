
vshell: 
	gcc *.c -o vshell

handler.o: handler.c handler.h
	gcc -c handler.c -o handler.o

internals.o: internals.c internals.h
	gcc -c internals.c -o internals.o

readcmd.o: readcmd.c readcmd.h
	gcc -c readcmd.c -o readcmd.o

shellio.o: shellio.c shellio.h
	gcc -c shellio.c -o shellio.o
