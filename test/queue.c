#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "queue.h"

// ===========INTERNAL DATA TYPES============

struct cell {
	struct cell *next;
	char *path_name;
};

struct queue {
	cell *top;
	cell *bottom;
};

// ==========INTERNAL FUNCTION IMPLEMENTATION==========

/**
 * queue_empty() - Create an empty queue.
 *
 * Returns: A pointer to the new queue or NULL
 */
queue *create_queue(void) {
	//Create queue and put top at null
	queue *new_queue = calloc(1, sizeof(queue));
	if (new_queue == NULL) {
		fprintf(stderr, "Failure to create queue\n");
		return NULL;
	}

	new_queue->top = NULL;
	new_queue->bottom = NULL;

	return new_queue;
}

/**
 * queue_is_empty() - Check if a queue is empty.
 * @get_queue: Queue to check.
 *
 * Returns: True if queue is empty, otherwise false.
 */
bool queue_is_empty(queue *get_queue) {
	return get_queue->top == NULL;
}

/**
 * queue_enqueue() - Put a value at the end of the queue.
 * @get_queue: Queue to manipulate.
 * @value: Pointer to the value that needs to be inserted.
 *
 * Returns: The modified queue or NULL
 */
queue *queue_enqueue(queue *get_queue, char *path) {
	//Create new cell
	cell *new_cell = malloc(sizeof(cell));
	if (new_cell == NULL) {
		fprintf(stderr, "Failure to create new cell\n");
		return NULL;
	}

	new_cell->next = NULL;
	new_cell->path_name = path;
	
	//Get current bottom of the queue and add new bottom
	if (get_queue->bottom == NULL) {
		get_queue->top = new_cell;
		get_queue->bottom = new_cell;
	} else {
		cell *bottom_cell = get_queue->bottom;
		bottom_cell->next = new_cell;
		new_cell->next = NULL;
		get_queue->bottom = new_cell;
	}

	return get_queue;
}

/**
 * queue_dequeue() - Remove the element at the front of a queue.
 * 
 * @get_queue: Queue to manipulate.
 *
 * Returns: The modified queue.
 */
queue *queue_dequeue(queue *get_queue) {
	//Get the top cell
	cell *get_top_cell = get_queue->top;

	if (get_top_cell->next == NULL) {
		get_queue->top = NULL;
		get_queue->bottom = NULL;
	} else {
		get_queue->top = get_top_cell->next;
	}

	get_top_cell->next = NULL;

	//Free that cell
	free(get_top_cell);

	return get_queue;
}

/**
 * queue_front() - Inspect the value at the front of the queue.
 * 
 * @get_queue: Queue to inspect.
 *
 * Returns: The value at the top of the queue.
 *	    NOTE: The return value is undefined for an empty queue.
 */
void *queue_front(const queue *get_queue) {
	return get_queue->top->path_name;
}

/**
 * queue_kill() - Destroy a given queue.
 * @get_queue: Queue to destroy.
 *
 * Return all dynamic memory used by the queue and its elements. If a
 * free_func was registered at queue creation, also calls it for each
 * element to free any user-allocated memory occupied by the element values.
 *
 * Returns: Nothing.
 */
void queue_kill(queue *get_queue) {
	//Loop through the queue and remove the top one by one
	while (!queue_is_empty(get_queue)) {
		get_queue = queue_dequeue(get_queue);
	}

	free(get_queue);
}