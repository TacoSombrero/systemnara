#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void *thread_func(void *arg);
pthread_mutex_t lock;
int main (){
	
			

	if (pthread_mutex_init(&lock, NULL) != 0) {
		printf("\n mutex init has failed\n");
		return 1;
	}
	pthread_t tid[10];
	for(int i = 0; i < 10; i++){
		pthread_create(&tid[i], NULL, thread_func, (void*)&tid);
	}
	for(int i = 0; i < 10; i++){
		pthread_join(tid[i], NULL);
	}
	

	pthread_mutex_destroy(&lock);
}




void *thread_func(void *arg) {

	pthread_mutex_lock(&lock);
	char* buf = "Multithreading är kul!\n";
	for(int i = 0; i < strlen(buf); i++){
		write(STDOUT_FILENO, &buf[i], 1);
		usleep(1);
	}
	pthread_mutex_unlock(&lock);
	return NULL;
}
