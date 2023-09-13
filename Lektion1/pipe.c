#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void){
	int args[2];
	char* msg = "Hello world#1\n";
	pipe(args);
	write(args[1], msg, 15);
	char answer[15];
	read(args[0], answer, 15);
	printf("%s", answer);
}