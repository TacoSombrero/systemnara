#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include "stack.h"

typedef struct threadItems{
	stack *directories;
	int total_size_of_files;
	int number_of_threads;
}threadItems;

void get_options(int argc, char *argv[], threadItems *items);
void *run(void *temp);
void open_dir(char *str, threadItems *items);
int get_size_of_file(char *name_of_file);
char *get_new_path(char *path, char *filename, int type);
void check_str_ending(char *str);
void append_str(char *dest, char *src);

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[]){

	struct threadItems *items = malloc(sizeof(struct threadItems));
	if(items == NULL){
		fprintf(stderr, "Failed to allocate space for items\n");
		exit(EXIT_FAILURE);
	}
	stack *s = stack_create();
	items->directories = s;

	get_options(argc, argv, items);
	

	bool use_cwd = true;
	for (; optind < argc; optind++)
	{
		use_cwd = false;
		char *new_path = get_new_path(argv[optind], "", 8);
		fprintf(stderr, "%s\n", new_path);
		items->directories = stack_push(items->directories, new_path);
	}

	if(use_cwd){
		char *new_path = get_new_path("./", "", 4);
		items->directories = stack_push(items->directories, new_path);
	}

	char *str_to_print = get_new_path(stack_top(s), "", 8);


	pthread_t *thread_ids = calloc(items->number_of_threads, sizeof(pthread_t));

	while(!stack_empty(s)){
		int i = items->number_of_threads;
		while (!stack_empty(s) && i > 0){
			
			//conditional variable check here. Checking if number_of_threads is greater than 0.
			pthread_create(&thread_ids[i], NULL, run, (void*)items);				
			i--;
		}
		
		for(int i = items->number_of_threads; i > 0 ; i--){
			//
			pthread_join(thread_ids[i], NULL);
			
		}
	}
	

	//run(items);
	printf("%d\t%s\n", items->total_size_of_files, str_to_print);
	stack_kill(s);
	free(items);
}

/**
 * get_options() - Retrives all the flags sent to the program.
 * 
 * @argc: Number of commandline arguments.
 * @argv: The commandline arguments.
 * @return: A string for the name of the makefile that shall be used.
*/
void get_options(int argc, char *argv[], threadItems *items){
	int opt;
	items->number_of_threads = 1;
	
	while ((opt = getopt(argc, argv, "j:")) != -1){
		switch (opt)
		{
		case 'j':
			items->number_of_threads = atoi(optarg);
			break;
		case ':':
			printf("option needs a value\n");
			break;
		}
	}
}

void *run(void *temp){
	struct threadItems *items = (threadItems*)temp;

	char *str = stack_top(items->directories);
	//fprintf(stderr, "Stack_top: %s\n", str);
	items->directories = stack_pop(items->directories);
	open_dir(str, items);
	
	
	return NULL;
}

/**
 * open_dir() - opens a directory and checks if it worked.
 * @s: Directory to open.
 * @return: A DIR pointer.
*/
void open_dir(char *str, threadItems *items){
	DIR *dir = opendir(str);
	if(dir != NULL){
		struct dirent *pDirent;
		items->total_size_of_files += get_size_of_file(str);
		//fprintf(stderr, "FILE: %s\n", str);
		for(int i = 0; (pDirent = readdir(dir)) != NULL; i++){
			if((strncmp(pDirent->d_name, ".", 2) == 0) || (strncmp(pDirent->d_name, "..", 3) == 0)){
				continue;
			}
			char *new_path = get_new_path(str, pDirent->d_name, pDirent->d_type);
			if(pDirent->d_type == 4){
				// Need to get path to dir then push to stack.
				//-------MIGHT GET PROBLEMS HERE--------
				// MIGHT have to allocate space for each thing sent to stack.

				// Allocate space for string.
				items->directories = stack_push(items->directories, new_path);
				//items->total_size_of_files += get_size_of_file(new_path);
				//fprintf(stderr, "Directory: %s\n", new_path);
			}else{
				// Need to get path to dir then push to stack.
				items->total_size_of_files += get_size_of_file(new_path);
				//fprintf(stderr, "FILE: %s\n", new_path);
			}
		}
	}else{
		items->total_size_of_files += get_size_of_file(str);
		//fprintf(stderr, "FILE: %s\n", str);
	}
	free(str);
}


/**
 * get_size_of_file() - Uses lstat to get the size of a file.
 * @path: The path to the file.
 * @return: The size of the file.
*/
int get_size_of_file(char *name_of_file){

	int read = access(name_of_file, R_OK);
	if(read != 0){
		fprintf(stderr, "Cannot read directory '%s': Permission denied\n", name_of_file);
		return 0;
	}

	struct stat info;
	if(lstat(name_of_file, &info)){
		perror(name_of_file);
		fprintf(stderr, "%s does not exist\n", name_of_file);
		return 0;
	}

	int size = info.st_blocks;

	return size;
}

char *get_new_path(char *path, char *filename, int type){
	int x = 1;
	if(type == 4){
		x = 2;
	}
	size_t string_size = strlen(path) + strlen(filename) + x;
	char *temp = (char *)malloc(string_size);
	if(temp == NULL){
		fprintf(stderr, "get_new_path: Malloc failed\n");
		exit(EXIT_FAILURE);
	}
	temp = strcpy(temp, path);
	// Check if last char of str is / 
	check_str_ending(temp);
	// Append
	append_str(temp, filename);

	if(type == 4){
		append_str(temp, "/");
	}
	return temp;
}

void check_str_ending(char *str){
	int len = strlen(str);
	if(str[len - 1] != '/'){
		str = strcat(str, "/");
	}
}

void append_str(char *dest, char *src){
	dest = strcat(dest, src);
}








#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include "stack.h"

typedef struct threadItems{
	stack *directories;
	int total_size_of_files;
	int number_of_threads;
}threadItems;

void get_options(int argc, char *argv[], threadItems *items);
void *run(void *temp);
void open_dir(char *str, threadItems *items);
int get_size_of_file(char *name_of_file);
char *get_new_path(char *path, char *filename, int type);
void check_str_ending(char *str);
void append_str(char *dest, char *src);

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[]){

	struct threadItems *items = malloc(sizeof(struct threadItems));
	if(items == NULL){
		fprintf(stderr, "Failed to allocate space for items\n");
		exit(EXIT_FAILURE);
	}
	stack *s = stack_create();
	items->directories = s;

	get_options(argc, argv, items);

	int optind2 = optind;

	bool use_cwd = true;
	for (; optind < argc; optind++)
	{
		use_cwd = false;
		char *new_path = get_new_path(argv[optind], "", 4);
		fprintf(stderr, "%s\n", new_path);
		items->directories = stack_push(items->directories, new_path);
	}

	if(use_cwd){
		char *new_path = get_new_path("./", "", 4);
		items->directories = stack_push(items->directories, new_path);
	}

	char *str_to_print = get_new_path(stack_top(s), "", 8);


	pthread_t *thread_ids = calloc(items->number_of_threads, sizeof(pthread_t));

	while(!stack_empty(s)){
		int i = items->number_of_threads;
		while (!stack_empty(s) && i > 0){
			
			//conditional variable check here. Checking if number_of_threads is greater than 0.
			pthread_create(&thread_ids[i], NULL, run, (void*)items);				
			i--;
		}
		
		for(int i = items->number_of_threads; i > 0 ; i--){
			//
			pthread_join(thread_ids[i], NULL);
			
		}
	}

	for(; optind2 < argc; optind2++){
		printf("%d\t%s\n", items->total_size_of_files, argv[optind2]);
	}	
	if(use_cwd){
		printf("%d\t%s\n", items->total_size_of_files, str_to_print);
	}

	//run(items);
	
	stack_kill(s);
	free(items);
}

/**
 * get_options() - Retrives all the flags sent to the program.
 * 
 * @argc: Number of commandline arguments.
 * @argv: The commandline arguments.
 * @return: A string for the name of the makefile that shall be used.
*/
void get_options(int argc, char *argv[], threadItems *items){
	int opt;
	items->number_of_threads = 1;
	
	while ((opt = getopt(argc, argv, "j:")) != -1){
		switch (opt)
		{
		case 'j':
			items->number_of_threads = atoi(optarg);
			break;
		case ':':
			printf("option needs a value\n");
			break;
		}
	}
}

void *run(void *temp){
	struct threadItems *items = (threadItems*)temp;

	char *str = stack_top(items->directories);
	//fprintf(stderr, "Stack_top: %s\n", str);
	items->directories = stack_pop(items->directories);
	open_dir(str, items);
	
	
	return NULL;
}

/**
 * open_dir() - opens a directory and checks if it worked.
 * @s: Directory to open.
 * @return: A DIR pointer.
*/
void open_dir(char *str, threadItems *items){
	DIR *dir = opendir(str);
	if(dir != NULL){
		struct dirent *pDirent;
		items->total_size_of_files += get_size_of_file(str);
		//fprintf(stderr, "FILE: %s\n", str);
		for(int i = 0; (pDirent = readdir(dir)) != NULL; i++){
			if((strncmp(pDirent->d_name, ".", 2) == 0) || (strncmp(pDirent->d_name, "..", 3) == 0)){
				continue;
			}
			char *new_path = get_new_path(str, pDirent->d_name, pDirent->d_type);
			if(pDirent->d_type == 4){
				// Need to get path to dir then push to stack.
				//-------MIGHT GET PROBLEMS HERE--------
				// MIGHT have to allocate space for each thing sent to stack.

				// Allocate space for string.
				items->directories = stack_push(items->directories, new_path);
				//items->total_size_of_files += get_size_of_file(new_path);
				//fprintf(stderr, "Directory: %s\n", new_path);
			}else{
				// Need to get path to dir then push to stack.
				items->total_size_of_files += get_size_of_file(new_path);
				//fprintf(stderr, "FILE: %s\n", new_path);
				
			}
		}
	}else{
		items->total_size_of_files += get_size_of_file(str);
		//fprintf(stderr, "FILE: %s\n", str);
	}
	free(str);
}


/**
 * get_size_of_file() - Uses lstat to get the size of a file.
 * @path: The path to the file.
 * @return: The size of the file.
*/
int get_size_of_file(char *name_of_file){

	int read = access(name_of_file, R_OK);
	if(read != 0){
		fprintf(stderr, "Cannot read directory '%s': Permission denied\n", name_of_file);
		return 0;
	}

	struct stat info;
	if(lstat(name_of_file, &info)){
		perror(name_of_file);
		fprintf(stderr, "%s does not exist\n", name_of_file);
		return 0;
	}

	int size = info.st_blocks;

	return size;
}

char *get_new_path(char *path, char *filename, int type){
	int x = 1;
	if(type == 4){
		x = 2;
	}
	size_t string_size = strlen(path) + strlen(filename) + x;
	char *temp = (char *)malloc(string_size);
	if(temp == NULL){
		fprintf(stderr, "get_new_path: Malloc failed\n");
		exit(EXIT_FAILURE);
	}
	temp = strcpy(temp, path);
	// Check if last char of str is / 
	check_str_ending(temp);
	// Append
	append_str(temp, filename);

	if(type == 4){
		append_str(temp, "/");
	}
	return temp;
}

void check_str_ending(char *str){
	int len = strlen(str);
	if(str[len - 1] != '/'){
		str = strcat(str, "/");
	}
}

void append_str(char *dest, char *src){
	dest = strcat(dest, src);
}

