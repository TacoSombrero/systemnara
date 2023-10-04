#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include <dirent.h>
#include <string.h>

char *get_options(int argc, char *argv[], int *thread_number);
void find_files(char *dir_name, char *current_path);
DIR *open_dir(char *s);


int main(int argc, char *argv[]){

	int thread_number = 1;
	char *dir = get_options(argc, argv, &thread_number);
	
	find_files(dir, dir);


	return 0;
}


/**
 * get_options() - Retrives all the flags sent to the program.
 * 
 * @argc: Number of commandline arguments.
 * @argv: The commandline arguments.
 * @return: A string for the name of the makefile that shall be used.
*/
char *get_options(int argc, char *argv[], int *thread_number){
	int opt;
	char *file = "./\0";
	
	while ((opt = getopt(argc, argv, "j:f:")) != -1){
		switch (opt)
		{
		case 'f':
			file = optarg;
			break;
		case 'j':
			*thread_number = atoi(optarg);
			break;
		case ':':
			printf("option needs a value\n");
			break;
		}
	}
	return file;
}

void find_files(char *dir_name, char *current_path){
	struct dirent *pDirent;
	DIR *dir = open_dir(dir_name);
	


	while ((pDirent = readdir(dir)) != NULL)
	{
		if(pDirent->d_type == 4){ // If it is a directory...
			strcat(current_path, dir_name);
			find_files(pDirent->d_name, current_path);
		}
		//get_size_of_file(pDirent->d_name);
		printf("[%s]\n", pDirent->d_name);
	}
	

}

/**
 * open_dir() - opens a directory and checks if it worked.
 * @s: Directory to open.
 * @return: A DIR pointer.
*/
DIR *open_dir(char *s){
	DIR *dir = opendir(s);
	if(dir == NULL){
		fprintf(stderr, "Failed to open directory: %s\n", s);
		exit(EXIT_FAILURE);
	}
	return dir;
}

/*int get_size_of_file(char *path){

}
*/