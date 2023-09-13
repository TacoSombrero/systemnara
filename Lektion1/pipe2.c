#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void){
	int p[2];
	char* msg = "Hello world#1\n";
	pipe(p);
	int pid = fork();
	if(pid != 0){
		close(p[0]);
		write(p[1], msg, 15);
	}else{
		close(p[1]);
		char answer[15];
		read(p[0], answer, 15);
		printf("%d is READING:\n %s", pid, answer);
		close(p[0]);
		exit(0);
	}
	close(p[1]);
	return 0;

}