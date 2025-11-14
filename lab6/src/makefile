CC=gcc
CFLAGS= -Wall -Wextra -pthread

all: server client

calc.o: calc.c calc.h
	$(CC) $(CFLAGS) -c calc.c -o calc.o

server : server.c calc.o
	$(CC) $(CFLAGS) server.c calc.o -o server

client: client.c calc.o
	$(CC) $(CFLAGS) client.c calc.o -o client

clean :
	rm -f *.o server client