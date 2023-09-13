#include <stdio.h>
#include <stdlib.h>
#include<unistd.h>

void main(void){
	if(execlp("ls", NULL) == -1){
		perror("ERROR ");
		exit(1);
	}

}