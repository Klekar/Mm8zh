CC=gcc

all: testovac.c
	$(CC) -lpthread testovac.c