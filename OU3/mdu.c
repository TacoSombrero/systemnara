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
	int exit_id;
}threadItems;

void start_threads(stack *s, pthread_t *thread_ids, threadItems *items);
char *allocate_for_string(char *str);
bool securely_check_stack_empty(stack *s);
void get_options(int argc, char *argv[], threadItems *items);
void *run(void *temp);
void open_dir(char *str, threadItems *items);
int get_size_of_file(char *name_of_file, threadItems *items);
char *get_new_path(char *path, char *filename, int type);
void check_str_ending(char *str);
void append_str(char *dest, char *src);

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sizelock = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[]){

	char *default_folder = "./";
	struct threadItems *items = malloc(sizeof(struct threadItems));
	if(items == NULL){
		fprintf(stderr, "Failed to allocate space for items\n");
		exit(EXIT_FAILURE);
	}
	stack *s = stack_create();
	items->directories = s;
	items->total_size_of_files = 0;
	items->number_of_threads = 1;
	items->exit_id = 0;

	get_options(argc, argv, items);

	for (; optind < argc; optind++)
	{
		
		char *new_path = allocate_for_string(argv[optind]);
		//fprintf(stderr, "%s\n", new_path);
		items->directories = stack_push(items->directories, new_path);
		pthread_t *thread_ids = calloc(items->number_of_threads, sizeof(pthread_t));
	
	
		start_threads(s, thread_ids, items);

		free(thread_ids);

		printf("%d\t%s\n", items->total_size_of_files, argv[optind]);

		items->total_size_of_files = 0;
		
		if(optind == (argc - 1)){
			//fprintf(stderr, "GOR HTERE\n");
			int e = items->exit_id;
			stack_kill(s);
			free(items);
			return e;
		}
	}


	char *new_path = allocate_for_string(default_folder);
	items->directories = stack_push(items->directories, new_path);

	pthread_t *thread_ids = calloc(items->number_of_threads, sizeof(pthread_t));
	
	start_threads(s, thread_ids, items);

	free(thread_ids);

	printf("%d\t%s\n", items->total_size_of_files, default_folder);
	
	int e = items->exit_id;

	stack_kill(s);
	free(items);

	return e;
}

void start_threads(stack *s, pthread_t *thread_ids, threadItems *items){
	while(!securely_check_stack_empty(s)){
		int i = 0;
		while (!securely_check_stack_empty(s) && i < items->number_of_threads){
			//conditional variable check here. Checking if number_of_threads is greater than 0.
			pthread_create(&thread_ids[i], NULL, run, (void*)items);				
			i++;
		}
		
		for(i = 0; i < items->number_of_threads; i++){
			//
			pthread_join(thread_ids[i], NULL);
			
		}
	}
}

char *allocate_for_string(char *str){
	size_t string_size = strlen(str) + 1;
	char *temp = (char *)malloc(string_size);
	if(temp == NULL){
		fprintf(stderr, "allocate_for_string: Malloc failed\n");
		exit(EXIT_FAILURE);
	}
	temp = strcpy(temp, str);
	return temp;
}

bool securely_check_stack_empty(stack *s){
	pthread_mutex_lock(&lock);
	bool b = stack_empty(s);
	pthread_mutex_unlock(&lock);
	return b;
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

	pthread_mutex_lock(&lock);
	char *str = stack_top(items->directories);
	//fprintf(stderr, "Stack_top: %s\n", str);
	items->directories = stack_pop(items->directories);
	pthread_mutex_unlock(&lock);
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
		items->total_size_of_files += get_size_of_file(str, items);
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
				pthread_mutex_lock(&lock);
				items->directories = stack_push(items->directories, new_path);
				pthread_mutex_unlock(&lock);
				//items->total_size_of_files += get_size_of_file(new_path);
				//fprintf(stderr, "Directory: %s\n", new_path);
			}else{
				// Need to get path to dir then push to stack.
				pthread_mutex_lock(&sizelock);
				items->total_size_of_files += get_size_of_file(new_path, items);
				pthread_mutex_unlock(&sizelock);
				//fprintf(stderr, "FILE: %s\n", new_path);
				free(new_path);
			}
		}
		closedir(dir);
	}else{
		pthread_mutex_lock(&sizelock);
		items->total_size_of_files += get_size_of_file(str, items);
		pthread_mutex_unlock(&sizelock);
		//fprintf(stderr, "FILE: %s\n", str);
		
	}
	free(str);
}


/**
 * get_size_of_file() - Uses lstat to get the size of a file.
 * @path: The path to the file.
 * @return: The size of the file.
*/
int get_size_of_file(char *name_of_file, threadItems *items){

	int read = access(name_of_file, R_OK);
	if(read != 0){
		perror(name_of_file);
		items->exit_id = 1;
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
	int x = 3;
	
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

