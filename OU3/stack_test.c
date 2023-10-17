#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stack.h"


// Integers are stored via int pointers stored as void pointers.
// Convert the given pointer and print the dereferenced value.
static void print_ints(const void *data)
{
	const int *v = data;
	printf("[%d]", *v);
}

int main(void)
{
	// Create the stack. Make the stack responsible for
	// deallocation pushed values.
	stack *s = stack_create();
	char *a = "0";
	char *b = "1";
	char *c = "2";
	char *d = "3";
	char *e = "4";
	char *f = "5";

	for(int i = 0; i < 6; i++){
		char *str = malloc(sizeof(char) * 2);
		strcpy(str, a);
		s = stack_push(s, str);
	}

	while(!stack_empty(s)){
		fprintf(stderr, "%s\n", stack_top(s));
		s = stack_pop(s);
	}

	stack_kill(s);

	return 0;
}