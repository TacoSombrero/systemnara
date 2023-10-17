#ifndef	QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// ===========INTERNAL DATA TYPES============

typedef struct cell cell;

typedef struct queue queue;

// ==========INTERNAL FUNCTION IMPLEMENTATION==========

/**
 * queue_empty() - Create an empty queue.
 *
 * Returns: A pointer to the new queue or NULL
 */
queue *create_queue(void);

/**
 * queue_is_empty() - Check if a queue is empty.
 * @get_queue: Queue to check.
 *
 * Returns: True if queue is empty, otherwise false.
 */
bool queue_is_empty(queue *get_queue);

/**
 * queue_enqueue() - Put a value at the end of the queue.
 * @get_queue: Queue to manipulate.
 * @value: Pointer to the value that needs to be inserted.
 *
 * Returns: The modified queue or NULL
 */
queue *queue_enqueue(queue *get_queue, char *path);

/**
 * queue_dequeue() - Remove the element at the front of a queue.
 * 
 * @get_queue: Queue to manipulate.
 *
 * Returns: The modified queue.
 */
queue *queue_dequeue(queue *get_queue);

/**
 * queue_front() - Inspect the value at the front of the queue.
 * 
 * @get_queue: Queue to inspect.
 *
 * Returns: The value at the top of the queue.
 *	    NOTE: The return value is undefined for an empty queue.
 */
void *queue_front(const queue *get_queue);

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
void queue_kill(queue *get_queue);

#endif