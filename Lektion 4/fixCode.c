#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>

struct resource
{
	pthread_mutex_t lock1;
	pthread_mutex_t lock2;
};

typedef struct resource Resource;

void *thread_func1(void *arg)
{
	Resource *res = (Resource *)arg;
	int counter = 0;
	printf("Thread %d started\n", 1);
	while (1)
	{
		pthread_mutex_lock(&res->lock2);
		pthread_mutex_lock(&res->lock1);
		
		printf("Thread %d counter: %d\n", 1, counter);
		counter++;
		pthread_mutex_unlock(&res->lock1);
		pthread_mutex_unlock(&res->lock2);
	}
	return NULL;
}

void *thread_func2(void *arg)
{
	Resource *res = (Resource *)arg;
	int counter = 0;
	printf("Thread %d started\n", 2);
	while (1)
	{
		pthread_mutex_lock(&res->lock2);
		pthread_mutex_lock(&res->lock1);
		
		printf("Thread %d counter: %d\n", 2, counter);
		counter++;
		pthread_mutex_unlock(&res->lock2);
		pthread_mutex_unlock(&res->lock1);
	}
	return NULL;
}

Resource *resource_create()
{
	Resource *res = malloc(sizeof(Resource));
	pthread_mutex_init(&res->lock1, NULL);
	pthread_mutex_init(&res->lock2, NULL);
	return res;
}

int main(void)
{

	pthread_t thread1[1];
	pthread_t thread2[1];

	Resource *res1 = resource_create();

	pthread_create(thread1, NULL, thread_func1, res1);
	pthread_create(thread2, NULL, thread_func2, res1);

	pthread_join(thread1[0], NULL);
	pthread_join(thread2[0], NULL);

	return 0;
}
