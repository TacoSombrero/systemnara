CC = gcc
CFLAGS = -g -std=gnu11 -Werror -Wall -Wextra -Wpedantic -Wmissing-declarations -Wmissing-prototypes -Wold-style-definition -lpthread 

mdu: mdu.o queue.o
	$(CC) $(CFLAGS) mdu.o queue.o -o mdu

mdu.o: mdu.c queue.h
	$(CC) $(CFLAGS) -c mdu.c

queue.o : queue.c queue.h
	$(CC) $(CFLAGS) -c queue.c