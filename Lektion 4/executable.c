#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int value = 0;
pthread_mutex_t l1;

void write_value(int v){
	usleep(250000);
	value = v;
}

int read_value(){
	usleep(250000);
	return value;
}

void* update_value(void *x){
	pthread_mutex_lock(&l1);
	int v = read_value();
	write_value(v + *(int *)x);
	pthread_mutex_unlock(&l1);
	
	return NULL;
}

int main(){

	if (pthread_mutex_init(&l1, NULL) != 0) {
		printf("\n mutex init has failed\n");
		return 1;
	}

	printf("Before: %d\n", read_value());
	int v1 = 100;
	int v2 = 800;
	pthread_t ptid1;
	pthread_t ptid2;
	pthread_create(&ptid1, NULL, update_value, (void *) &v1);
	pthread_create(&ptid2, NULL, update_value, (void *) &v2);

	pthread_join(ptid1, NULL);
	pthread_join(ptid2, NULL);

	pthread_mutex_destroy(&l1);

	printf("After: %d\n", read_value());

}
