CC = gcc
CFLAGS = -g -std=gnu11 -Werror -Wall -Wextra -Wpedantic -Wmissing-declarations -Wmissing-prototypes -Wold-style-definition

mexec: mexec.o
	$(CC)  mexec.o -o mexec

mexec.o: mexec.c
	$(CC)  -c mexec.c
