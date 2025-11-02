CC=gcc
LAB3_DIR=../../lab3/src
CFLAGS=-I$(LAB3_DIR)
SOURCES=$(LAB3_DIR)/utils.c $(LAB3_DIR)/find_min_max.c

all : process_memory parallel_min_max

process_memory :
	$(CC) process_memory.c -o process_memory

parallel_min_max : utils.o find_min_max.o
	$(CC) -o parallel_min_max utils.o find_min_max.o $(LAB3_DIR)/parallel_min_max.c $(CFLAGS)

utils.o : $(LAB3_DIR)/utils.c
	$(CC) $(CFLAGS) -c $(LAB3_DIR)/utils.c -o utils.o

find_min_max.o : $(LAB3_DIR)/find_min_max.c
	$(CC) $(CFLAGS) -c $(LAB3_DIR)/find_min_max.c -o find_min_max.o

clean:
	rm -f *.o parallel_min_max process_memory