#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *print_hello(void *s);

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int last_thread_completed = 0;

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s <some word>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	pthread_t th[5];

	char *s = malloc(sizeof(char *));
	s = argv[1];

	for (int i = 0; i < 5; i++)
	{
		int *x = malloc(sizeof(int *));
		*x = i;
		pthread_create(&th[i], NULL, print_hello, x);
	}

	// Let the last thread run first
	pthread_mutex_lock(&lock);
	last_thread_completed = 1;
	pthread_mutex_unlock(&lock);
	pthread_cond_signal(&cond); // Signal the first thread to start

	for (int i = 0; i < 5; i++)
	{
		pthread_join(th[i], NULL);
	}

	pthread_mutex_destroy(&lock);
	pthread_cond_destroy(&cond);

	return 0;
}

void *print_hello(void *s)
{

	pthread_mutex_lock(&lock);
	if(!last_thread_completed){
		pthread_cond_wait(&cond, &lock); // Wait for a signal.
	}
	pthread_mutex_unlock(&lock);
	

	// DO THE JOB.

	pthread_mutex_lock(&lock);
	
	last_thread_completed = 1;
	int *str = (int *)s;
	printf("HELLO %d!\n", *str);
	
	pthread_mutex_unlock(&lock);
	pthread_cond_signal(&cond);
	return NULL;
}