#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>

int main(){
	pid_t ppid = getpid();
	pid_t cpid = fork();
	if(cpid == -1){
		exit(EXIT_FAILURE);
	}

	if(cpid == 0){
		//child
		sleep(10);
		exit(0);
	}

	fprintf(stderr, "\nParent PID: %d\n", ppid);
	fprintf(stderr, "Child PID: %d\n\n", cpid);

	fprintf(stderr, "getpid from parent: %d\n", getpid());
	return 0;
}