#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#define MY_ERROR_CODE 1


int main ( int argc, char *argv[] ){
	if ( argc != 3 ) {
		/* Fel antal argument, skriv ut ett felmeddelande. */
		fprintf(stderr,"Usage: %s filename\n", argv[0]);
		return MY_ERROR_CODE;
	}
	struct stat info;
	struct stat info2;
	char *target = "fact";
	char *prereq = "fact.c";
	if(lstat(argv[1] ,&info)){
		perror(argv[1]);
		exit(EXIT_FAILURE);
	}
	
	if(lstat(argv[2] ,&info2)){
		perror(argv[2]);
		exit(EXIT_FAILURE);
	}

	fprintf(stderr, "%s: %d \n%s: %d\n", argv[1], info.st_mtime, argv[2], info2.st_mtime);
	return 0;
}