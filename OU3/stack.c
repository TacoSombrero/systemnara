#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "stack.h"

struct cell{
	char *str;
	struct cell *next;
};

struct stack{
	struct cell *top;
};

/**
 * stack_create() - Creates a new empty stack.
 * @return: The new stack.
*/
stack *stack_create(void){
	stack *s = calloc(1, sizeof(stack));
	if(s == NULL){
		fprintf(stderr, "stack_create: calloc failed\n");
		exit(EXIT_FAILURE);
	}
	s->top = NULL;
	return s;
}

/**
 * stack_empty() - Checks if the stack is empty or not.
 * @s: The stack.
 * @return: True if stack is empty, otherwise false.
*/
bool stack_empty(stack *s){
	return s->top == NULL;
}

/**
 * stack_push() - Adding a new string to the stack.
 * @s: The stack we are adding str to.
 * @return: The new stack.
*/
stack *stack_push(stack *s, char *str){
	struct cell *c = calloc(1, sizeof(struct cell));
	if(s == NULL){
		fprintf(stderr, "stack_push: calloc failed\n");
		exit(EXIT_FAILURE);
	}
	//Copy value to the stack
	c->str = str;
	//Make the new cell appear at the top of the stack and push the previous top down.
	c->next = s->top;
	s->top = c;

	return s;
}

/**
 * stack_pop() - Removes the top cell of the stack.
 * @s: The stack to remove the top of.
 * @return: The new stack.
*/
stack *stack_pop(stack *s){
	if(stack_empty(s)){
		fprintf(stderr, "stack_pop: Cannot pop on empty stack!\n");
	}else{
		struct cell *c = s->top;
		s->top = s->top->next;
		free(c);
	}
	return s;
}

/**
 * stack_top() - Returns the top of the stack.
 * @s: The stack to look at.
 * @return: The string at the top of the stack.
*/
char *stack_top(stack *s){
	if(stack_empty(s)){
		fprintf(stderr, "stack_top: Cannot do top on empty stack\n");
	}
	return s->top->str;
}

/**
 * stack_kill() - Deallocates all used memory by the stack.
 * @s: The stack to kill.
*/
void stack_kill(stack *s){
	while(!stack_empty(s)){
		s = stack_pop(s);
	}
	free(s);
}