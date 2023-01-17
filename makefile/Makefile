CC=gcc 
CFLAGS= -Wall -Wextra -Werror
EXEC=testClient testServeur 

all: $(EXEC)

testClient: ../sources/client.o	 ../sources/entreeSortie.o ../sources/routines.o ../sources/manipulationFichier.o ../sources/interfaceUtilisateur.o
	$(CC) ../sources/client.o  ../sources/entreeSortie.o ../sources/routines.o ../sources/manipulationFichier.o ../sources/interfaceUtilisateur.o -o ../bin/testClient

testServeur: ../sources/serveur.o ../sources/manipulationFichier.o	
	$(CC) ../sources/serveur.o ../sources/manipulationFichier.o -o ../bin/testServeur

entreeSortie.o: ../sources/entreeSortie.c
	$(CC) ../sources/entreeSortie.c $(CFLAGS)	

manipulationFichier.o: ../sources/manipulationFichier.c
	$(CC) ../sources/manipulationFichier.c $(CFLAGS)	

interfaceUtilisateur.o: ../sources/interfaceUtilisateur.c 
	$(CC) ../sources/interfaceUtilisateur.c $(CFLAGS)

routines.o: ../sources/routines.c
	$(CC) ../sources/routines.c $(CFLAGS)

client.o: ../sources/client.c 
	$(CC) ../sources/client.c $(CFLAGS)

serveur.o: ../sources/serveur.c 
	$(CC) ../sources/serveur.c $(CFLAGS)

clean:
	@echo "Effacement des fichiers objets"
	rm -rf ../sources/*.o

cleanExec:
	@echo "Effacement des executables"
	rm -f ../bin/*
