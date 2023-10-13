#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>
#include "stack.h"
#include <sys/time.h>

typedef struct threadItems{
	stack *directories;
	int total_size_of_files;
	int number_of_threads;
	int last_thread_completed;
	int number_of_working_threads;
	bool first_thread;
	int exit_id;
}threadItems;

void items_init(threadItems *items);
void start_threads(pthread_t *thread_ids, threadItems *items);
char *allocate_for_string(char *str);
bool securely_check_stack_empty(stack *s);
void get_options(int argc, char *argv[], threadItems *items);
void *run(void *temp);
void open_dir(char *str, threadItems *items);
int get_size_of_file(char *path, threadItems *items);
char *get_new_path(char *path, char *filename, int type);
void check_str_ending(char *str);
void append_str(char *dest, char *src);

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sizelock = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;


int main(int argc, char *argv[]){

	char *default_folder = "./";
	struct threadItems *items = malloc(sizeof(struct threadItems));
	if(items == NULL){
		fprintf(stderr, "Failed to allocate space for items\n");
		exit(EXIT_FAILURE);
	}
	stack *s = stack_create();
	items->directories = s;
	items_init(items);

	get_options(argc, argv, items);

	for (; optind < argc; optind++)
	{
		
		char *new_path = allocate_for_string(argv[optind]);
		items->directories = stack_push(items->directories, new_path);
		pthread_t *thread_ids = calloc(items->number_of_threads, sizeof(pthread_t));
	
		start_threads(thread_ids, items);
		run(items);

		free(thread_ids);

		printf("%d\t%s\n", items->total_size_of_files, argv[optind]);

		items->total_size_of_files = 0;
		
		if(optind == (argc - 1)){
			int e = items->exit_id;
			stack_kill(s);
			free(items);
			return e;
		}
	}

	char *new_path = allocate_for_string(default_folder);
	items->directories = stack_push(items->directories, new_path);

	pthread_t *thread_ids = calloc(items->number_of_threads + 1, sizeof(pthread_t));
	
	start_threads(thread_ids, items);
	run(items);

	free(thread_ids);

	printf("%d\t%s\n", items->total_size_of_files, default_folder);
	
	int e = items->exit_id;

	stack_kill(s);
	free(items);

	return e;
}

/**
 * items_init() - Initializer for the struct of type threadItems.
 * @items: the 
*/
void items_init(threadItems *items){
	items->total_size_of_files = 0;
	items->number_of_threads = 0;
	items->exit_id = 0;
	items->last_thread_completed = 0;
	items->number_of_working_threads = 0;
	items->first_thread = true;
}

/**
 * start_threads() - 
 * @thread_ids: Array of pthread_t positions.
 * @items: A struct of type threadItems.
*/
void start_threads(pthread_t *thread_ids, threadItems *items){
	for (int i = 0; i < (items->number_of_threads - 1); i++){
		pthread_create(&thread_ids[i], NULL, run, (void*)items);			
	}
	
	for(int i = 0; i < (items->number_of_threads - 1); i++){
		pthread_join(thread_ids[i], NULL);
	}
}

/**
 * allocate_for_string)() - Allocates memory for a string.
 * @str: The string to allocate memory for.
 * @return: A char pointer to the newly allocated string.
*/
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

/**
 * securely_check_stack_empty() - Securely checks if the stack is empty using a mutex lock.
 * @s: The name of the stack to check.
 * @return: True if stack is empty, false if not.
*/
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
 * @items: A struct of type threadItems.
 * @return: A string for the name of the makefile that shall be used.
*/
void get_options(int argc, char *argv[], threadItems *items){
	int opt;
	
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

/**
 * run() - Takes one item from the stack and traverses that directory.
 * If another directory is found within the first one it to gets put into the stack
 * @temp: A struct of type threadItems.
 * @return: A NULL pointer.
*/
void *run(void *temp){
	struct threadItems *items = (threadItems*)temp;
	while(1){
		pthread_mutex_lock(&lock);
		while(stack_empty(items->directories)){
			if(!items->first_thread && items->number_of_working_threads == 0){
				items->last_thread_completed = 1;
				pthread_cond_broadcast(&cond);
				pthread_mutex_unlock(&lock);
				return NULL;
			}else if(items->last_thread_completed){
				pthread_mutex_unlock(&lock);
				return NULL;
			}else{
				pthread_cond_wait(&cond, &lock);
			}
		}
		items->first_thread = false;
		items->number_of_working_threads++;
		char *str = stack_top(items->directories);
		items->directories = stack_pop(items->directories);
		pthread_mutex_unlock(&lock);
		open_dir(str, items);
		pthread_mutex_lock(&lock);
		items->number_of_working_threads--;
		pthread_mutex_unlock(&lock);
	}
}

/**
 * open_dir() - opens a directory and checks if it worked.
 * @str: Directory to open.
 * @items: A struct of type threadItems.
 * @return: A DIR pointer.
*/
void open_dir(char *str, threadItems *items){
	DIR *dir = opendir(str);
	if(dir != NULL){
		struct dirent *pDirent;
		pthread_mutex_lock(&sizelock);
		items->total_size_of_files += get_size_of_file(str, items);
		pthread_mutex_unlock(&sizelock);
		for(int i = 0; (pDirent = readdir(dir)) != NULL; i++){
			if((strncmp(pDirent->d_name, ".", 2) == 0) || (strncmp(pDirent->d_name, "..", 3) == 0)){
				continue;
			}
			char *new_path = get_new_path(str, pDirent->d_name, pDirent->d_type);
			if(pDirent->d_type == 4){
				pthread_mutex_lock(&lock);
				items->directories = stack_push(items->directories, new_path);
				pthread_mutex_unlock(&lock);
			}else{
				pthread_mutex_lock(&sizelock);
				items->total_size_of_files += get_size_of_file(new_path, items);
				pthread_mutex_unlock(&sizelock);
				free(new_path);
			}
		}
		closedir(dir);
	}else{
		pthread_mutex_lock(&sizelock);
		items->total_size_of_files += get_size_of_file(str, items);
		pthread_mutex_unlock(&sizelock);		
	}
	free(str);
}


/**
 * get_size_of_file() - Uses lstat to get the size of a file.
 * @path: The path to the file.
 * @items: A struct of type threadItems.
 * @return: The size of the file.
*/
int get_size_of_file(char *path, threadItems *items){

	
	/*int read = access(path, R_OK);
	if(read != 0){
		fprintf(stderr, "NAME: %s\n", path);
		perror(path);
		items->exit_id = 1;

	}else{
		
	}*/

	struct stat info;
	if(lstat(path, &info)){
		perror(path);
		fprintf(stderr, "%s does not exist\n", path);
		return 0;
	}

	if(!(info.st_mode % S_IRUSR)){
		fprintf(stderr, "%s: Permission Denied\n", path);
		items->exit_id = 1;
	}

	int size = info.st_blocks;

	return size;
}

/**
 * get_new_path() - Creates a new path for a directory or a file.
 * @path: The current path.
 * @filename: The name of the directory or file.
 * @type: If type is equal to 4 a / will be appended to the end of the path. Otherwise not.
 * @return: The new path.
*/
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

/**
 * check_str_ending() - Checks if the string is ending with a /.
 * @str: The string to check.
*/
void check_str_ending(char *str){
	int len = strlen(str);
	if(str[len - 1] != '/'){
		str = strcat(str, "/");
	}
}

/**
 * append_str() - appands a string with another string
 * @dest: The string to append.
 * @src: The string to append with.
*/
void append_str(char *dest, char *src){
	dest = strcat(dest, src);
}

