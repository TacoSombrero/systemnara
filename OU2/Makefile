CC = gcc
CFLAGS = -g -std=gnu11 -Werror -Wall -Wextra -Wpedantic -Wmissing-declarations -Wmissing-prototypes -Wold-style-definition

mmake: mmake.o parser.o
	$(CC) $(CFLAGS) mmake.o parser.o -o mmake

mmake.o: mmake.c
	$(CC) $(CFLAGS) -c mmake.c

parser.o: parser.c
	$(CC) $(CFLAGS) -c parser.c