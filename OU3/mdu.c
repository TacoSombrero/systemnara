#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>


typedef struct toThread{
	int size;
	char **paths;
	int number_of_threads;
	DIR **ds; 
	int j;
}toThread;

int get_options(int argc, char *argv[]);
int find_files(void *items);
void open_dir(char *s, toThread *toThread);
int get_size_of_file(char *name_of_file);
char *get_new_path(char *current_dir, char *name, bool is_directory);
char *check_string(char *s);

int main(int argc, char *argv[]){
	
	int number_of_threads = 0;
	number_of_threads = get_options(argc, argv);
	struct toThread *toThread = malloc(sizeof(struct toThread));
	//int **sizes = malloc(sizeof(int*));
	char **paths = malloc(sizeof(char *));
	toThread->ds = malloc(sizeof(DIR**));
	toThread->j = 0;
	toThread->paths = paths;
	toThread->size = 0;
	toThread->number_of_threads = number_of_threads;
	
	bool use_cwd = true;
	for (; optind < argc; optind++)
	{
		use_cwd = false;
		open_dir(argv[optind], toThread);
		printf("%d\t%s\n", toThread->size, argv[optind]);
	}

	if(use_cwd){
		open_dir("./", toThread);
		printf("%d\t%s\n", toThread->size, "./");
	}

	for(int i = 0; i < toThread->j; i++){
		closedir(toThread->ds[i]);
	}

	return 0;
}


/**
 * get_options() - Retrives all the flags sent to the program.
 * 
 * @argc: Number of commandline arguments.
 * @argv: The commandline arguments.
 * @return: A string for the name of the makefile that shall be used.
*/
int get_options(int argc, char *argv[]){
	int opt;
	int number_of_threads = 1;
	
	while ((opt = getopt(argc, argv, "j:")) != -1){
		switch (opt)
		{
		case 'j':
			number_of_threads = atoi(optarg);
			break;
		case ':':
			printf("option needs a value\n");
			break;
		}
	}
	return number_of_threads;
}

/**
 * open_dir() - opens a directory and checks if it worked.
 * @s: Directory to open.
 * @return: A DIR pointer.
*/
void open_dir(char *str, toThread *toThread){
	DIR *dir = malloc(sizeof(DIR*));
	dir = opendir(str);
	toThread->ds[toThread->j] = dir;
	if(dir != NULL){

		int len = strlen(str);
		if(str[len - 1] != '/'){
			strcat(str, "/");
		}

		int i = 0;
		struct dirent *pDirent;
		
		for(i = 0; (pDirent = readdir(dir)) != NULL; i++){
			if((strncmp(pDirent->d_name, ".", 2) == 0) || (strncmp(pDirent->d_name, "..", 3) == 0)){
				continue;
			}
			if(pDirent->d_type == 4){
				
				char *new_dir_path = get_new_path(str, pDirent->d_name, true);
				toThread->size += get_size_of_file(new_dir_path);
				//fprintf(stderr,"%s is dir\n", pDirent->d_name);
				open_dir(new_dir_path, toThread);
			}else{
				char *new_file_path = get_new_path(str, pDirent->d_name, false);
				toThread->size += get_size_of_file(new_file_path);
				//fprintf(stderr, "%s is file\n", pDirent->d_name);
			}
		}
	}else{
		//fprintf(stderr, "DIR == NULL\n");
		toThread->size = get_size_of_file(str);
	}
	// s is a file
	// Get size of the file.
}

char *check_string(char *s){
	int len = strlen(s);
	char *str = malloc(sizeof(len+1));
	int i = 0;
	if (s[len - 1] == '\n'){
		s[len - 1] = '\0';
	}
	for(i = 0;s[i] != '\0'; i++){
		str[i] = s[i];
	}

	if(s[len-1] != '/'){

		int i = 0;
		for(i = 0; i < len-1; i++){
			str[i] = s[i];
		}
		str[i] = '/';
		str[i+1] = '\0';

		return str;
	}
	return s;
}

/**
 * get_size_of_file() - Uses lstat to get the size of a file.
 * @path: The path to the file.
 * @return: The size of the file.
*/
int get_size_of_file(char *name_of_file){
/*
	FILE *fp = fopen(name_of_file, "r");
	if(fp == NULL){
		DIR *dir = opendir(name_of_file);
		if(dir == NULL){
			fprintf(stderr, "Could not open file: %s\n", name_of_file);
			return 0;
		}
		closedir(dir);
	}
	fclose(fp);*/

	struct stat info;
	if(lstat(name_of_file, &info)){
		perror(name_of_file);
		//exit(EXIT_FAILURE);
		return 0;
	}

	int size = info.st_blocks;

	return size;
}

char *get_new_path(char *current_dir, char *name, bool is_directory){

	
	int x = strlen(name);
	int y = strlen(current_dir);

	char *new_dir = malloc(x+y+2);
	int i = 0;
	while(current_dir[i] != '\0'){
		new_dir[i] = current_dir[i];
		i++;
	}
	int j = 0;
	while (name[j] != '\0'){
		new_dir[i] = name[j];
		i++;
		j++;
	}

	if(is_directory){
		new_dir[i] = '/';
	}

	return new_dir;
	
}