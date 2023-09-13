#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>


int main(void){
	int fp = open("./file.txt", O_WRONLY);
	if(fp < 0)
        	printf("Error opening the file\n");
	dup2(fp, STDOUT_FILENO);

	printf("HELLO WORLD\n");
}