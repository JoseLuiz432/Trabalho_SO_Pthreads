###################################MAKEFILE######################################

EXEC=Formigopolis

all:	Formigopolis
Formigopolis:	Formigopolis.o
	gcc -o Formigopolis Formigopolis.o -pthread

Formigopolis.o:	Formigopolis.c
	gcc -o Formigopolis.o -c Formigopolis.c

clean: 
	rm -rf *.o

mrproper: clean
	rm -rf Formigopolis