#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void){
	int pid = fork();
	if(pid == 0){
		if(execv("./cisterrible", NULL) == -1){
			perror("");
			exit(1);
		}
	}
	int status;
	wait(&status);
	printf("BYeeeee!\n");
}