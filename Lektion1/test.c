#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>
//#include <sys/wait.h>
//#include <unistd.h>
#include <fcntl.h>

void main(void){
	char *str1 = "ls al as akjsnd";
	char **str = &str1;
	
	fprintf(stderr, "%c\n", str[0][1]);
}