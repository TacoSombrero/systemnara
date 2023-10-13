CC = gcc
CFLAGS = -g -std=gnu11 -Werror -Wall -Wextra -Wpedantic -Wmissing-declarations -Wmissing-prototypes -Wold-style-definition -lpthread 

mdu: mdu.o stack.o
	$(CC) $(CFLAGS) mdu.o stack.o -o mdu

mdu.o: mdu.c
	$(CC) $(CFLAGS) -c mdu.c

stack.o: stack.c stack.h
	$(CC) $(CFLAGS) -c stack.c