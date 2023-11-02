


#ifndef __STACK_H
#define __STACK_H

#include <stdbool.h>
typedef struct stack stack;

/**
 * stack_create() - Creates a new empty stack.
 * @param return The new stack.
*/
stack *stack_create(void);

/**
 * stack_empty() - Checks if the stack is empty or not.
 * @param s The stack.
 * @param return True if stack is empty, otherwise false.
*/
bool stack_empty(stack *s);

/**
 * stack_push() - Adding a new string to the stack.
 * @param s The stack we are adding str to.
 * @param return The new stack.
*/
stack *stack_push(stack *s, char *str);

/**
 * stack_pop() - Removes the top cell of the stack.
 * @param s The stack to remove the top of.
 * @param return The new stack.
*/
stack *stack_pop(stack *s);

/**
 * stack_top() - Returns the top of the stack.
 * @param s The stack to look at.
 * @param return The string at the top of the stack.
*/
char *stack_top(stack *s);

/**
 * stack_kill() - Deallocates all used memory by the stack.
 * @param s The stack to kill.
*/
void stack_kill(stack *s);

#endif