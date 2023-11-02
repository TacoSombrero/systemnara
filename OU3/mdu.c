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
#include <errno.h>

typedef struct threadItems{
	stack *directories;
	int total_size_of_files;
	int number_of_threads;
	int last_thread_completed;
	int number_of_working_threads;
	bool first_thread;
	int exit_id;

	pthread_mutex_t *lock;
	pthread_mutex_t *sizelock;
	pthread_mutex_t *working_threads_lock;
	pthread_mutex_t *exit_id_lock;
	pthread_cond_t *cond;
} threadItems;

void start(threadItems *items, char *str);
void items_init(threadItems *items);
void start_threads(pthread_t *thread_ids, threadItems *items);
char *allocate_for_string(char *str);
void get_options(int argc, char *argv[], threadItems *items);
void *run(void *temp);
void thread_exit(threadItems *items, int *local_size);
int open_dir(char *str, threadItems *items);
int get_size_of_file(char *path);
char *get_new_path(char *path, char *filename, int type);
void check_str_ending(char *str);
void append_str(char *dest, char *src);


int main(int argc, char *argv[])
{

	char *default_folder = "./";
	struct threadItems *items = malloc(sizeof(struct threadItems));
	if (items == NULL){
		perror("Items");
		exit(EXIT_FAILURE);
	}
	stack *s = stack_create();
	items->directories = s;
	items_init(items);

	get_options(argc, argv, items);

	for (; optind < argc; optind++)
	{
		start(items, argv[optind]);

		// If on the last argument, clean up and exit.
		if (optind == (argc - 1))
		{
			int e = items->exit_id;
			stack_kill(s);
			free(items);
			return e;
		}
	}

	start(items, default_folder);

	int e = items->exit_id;

	stack_kill(s);
	free(items);

	return e;
}

/**
 * start() - Creates necessary allocations and runs the program.
 * @param items A struct of type threadItems.
 * @param str The path to the file or directory.
 */
void start(threadItems *items, char *str)
{
	char *new_path = allocate_for_string(str);
	items->directories = stack_push(items->directories, new_path);
	pthread_t *thread_ids = calloc(items->number_of_threads, sizeof(pthread_t));
	if(thread_ids == NULL){
		perror("Thread IDs");
		exit(EXIT_FAILURE);
	}
	pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t sizelock = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t working_threads_lock = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t exit_id_lock = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

	items->lock = &lock;
	items->sizelock = &sizelock;
	items->working_threads_lock = &working_threads_lock;
	items->exit_id_lock = &exit_id_lock;
	items->cond = &cond;

	start_threads(thread_ids, items);

	free(thread_ids);

	printf("%d\t%s\n", items->total_size_of_files, str);

	items->total_size_of_files = 0;
}

/**
 * items_init() - Initializer for the struct of type threadItems.
 * @param items A struct of type threadItems.
 */
void items_init(threadItems *items)
{
	items->total_size_of_files = 0;
	items->number_of_threads = 0;
	items->exit_id = 0;
	items->last_thread_completed = 0;
	items->number_of_working_threads = 0;
	items->first_thread = true;
}

/**
 * start_threads() - Creates the requested number of threads.
 * @param thread_ids Array of pthread_t positions.
 * @param items A struct of type threadItems.
 */
void start_threads(pthread_t *thread_ids, threadItems *items)
{
	for (int i = 0; i < (items->number_of_threads - 1); i++)
	{
		if(pthread_create(&thread_ids[i], NULL, run, (void *)items) != 0){
			fprintf(stderr, "pthread_create: Failed to create thread\n");
			exit(EXIT_FAILURE);
		}
	}

	run(items);

	for (int i = 0; i < (items->number_of_threads - 1); i++)
	{
		if(pthread_join(thread_ids[i], NULL) != 0){
			fprintf(stderr, "pthread_join: Failed to join thread\n");
			exit(EXIT_FAILURE);
		}
	}
}

/**
 * allocate_for_string)() - Allocates memory for a string.
 * @param str The string to allocate memory for.
 * @param return A char pointer to the newly allocated string.
 */
char *allocate_for_string(char *str)
{
	size_t string_size = strlen(str) + 1;
	char *temp = (char *)malloc(string_size);
	if (temp == NULL)
	{
		perror(str);
		exit(EXIT_FAILURE);
	}
	temp = strcpy(temp, str);
	return temp;
}

/**
 * get_options() - Retrives all the flags sent to the program.
 *
 * @param argc Number of commandline arguments.
 * @param argv The commandline arguments.
 * @param items A struct of type threadItems.
 * @param return A string for the name of the makefile that shall be used.
 */
void get_options(int argc, char *argv[], threadItems *items)
{
	int opt;

	while ((opt = getopt(argc, argv, "j:")) != -1)
	{
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
 * @param temp A struct of type threadItems.
 * @param return A NULL pointer.
 */
void *run(void *temp)
{
	struct threadItems *items = (threadItems *)temp;
	int local_size = 0;
	while (1)
	{
		pthread_mutex_lock(items->lock);
		while (stack_empty(items->directories))
		{
			pthread_mutex_lock(items->working_threads_lock);
			if (!items->first_thread && items->number_of_working_threads == 0)
			{
				pthread_mutex_unlock(items->working_threads_lock);
				items->last_thread_completed = 1;
				pthread_cond_broadcast(items->cond);
				thread_exit(items, &local_size);
				return NULL;
			}
			else if (items->last_thread_completed)
			{
				pthread_mutex_unlock(items->working_threads_lock);
				thread_exit(items, &local_size);
				return NULL;
			}
			else
			{
				pthread_mutex_unlock(items->working_threads_lock);
				pthread_cond_wait(items->cond, items->lock);
			}
		}
		// Get a new directory path to work with
		char *str = stack_top(items->directories);
		items->directories = stack_pop(items->directories);
		pthread_mutex_unlock(items->lock);

		// Change the number of working threads
		pthread_mutex_lock(items->working_threads_lock);
		items->first_thread = false;
		items->number_of_working_threads++;
		pthread_mutex_unlock(items->working_threads_lock);

		local_size += open_dir(str, items);

		pthread_mutex_lock(items->working_threads_lock);
		items->number_of_working_threads--;
		pthread_mutex_unlock(items->working_threads_lock);
	}
}

/**
 * thread_exit() - Do the last steps before exiting the loop of the run funciton.
 * @param items A struct of type threadItems.
 * @param local_size The size of all the files and directories that the thread has found.
*/
void thread_exit(threadItems *items, int *local_size){
	pthread_mutex_unlock(items->lock);
	pthread_mutex_lock(items->sizelock);
	items->total_size_of_files += *local_size;
	pthread_mutex_unlock(items->sizelock);
}

/**
 * open_dir() - opens a directory and checks if it worked.
 * @param str Directory to open.
 * @param items A struct of type threadItems.
 * @param return A DIR pointer.
 */
int open_dir(char *str, threadItems *items)
{
	DIR *dir = opendir(str);
	int size = 0;
	if (dir != NULL)
	{
		struct dirent *pDirent;
		size += get_size_of_file(str);
		for (int i = 0; (pDirent = readdir(dir)) != NULL; i++)
		{
			if ((strncmp(pDirent->d_name, ".", 2) == 0) || (strncmp(pDirent->d_name, "..", 3) == 0))
			{
				continue;
			}
			char *new_path = get_new_path(str, pDirent->d_name, pDirent->d_type);
			if (pDirent->d_type == 4)
			{
				pthread_mutex_lock(items->lock);
				items->directories = stack_push(items->directories, new_path);
				pthread_cond_broadcast(items->cond);
				pthread_mutex_unlock(items->lock);
			}
			else
			{
				size += get_size_of_file(new_path);
				free(new_path);
			}
		}
		if(closedir(dir) != 0){
			perror(str);
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		FILE *fp = fopen(str, "r");
		if(fp == NULL){
			size += get_size_of_file(str);
			fprintf(stderr, "%s, Permission denied\n", str);
			pthread_mutex_lock(items->exit_id_lock);
			items->exit_id = 1;
			pthread_mutex_unlock(items->exit_id_lock);
		}else{
			fclose(fp);
			size += get_size_of_file(str);
		}
	}
	free(str);
	return size;
}

/**
 * get_size_of_file() - Uses lstat to get the size of a file.
 * @param path The path to the file.
 * @param items A struct of type threadItems.
 * @param return The size of the file.
 */
int get_size_of_file(char *path)
{

	struct stat info;
	if (lstat(path, &info) < 0)
	{
		perror(path);
		exit(EXIT_FAILURE);
	}

	return info.st_blocks;
}

/**
 * get_new_path() - Creates a new path for a directory or a file.
 * @param path The current path.
 * @param filename The name of the directory or file.
 * @param type If type is equal to 4 a / will be appended to the end of the path. Otherwise not.
 * @param return The new path.
 */
char *get_new_path(char *path, char *filename, int type)
{
	int x = 3;

	size_t string_size = strlen(path) + strlen(filename) + x;
	char *temp = (char *)malloc(string_size);
	if (temp == NULL)
	{
		perror(path);
		exit(EXIT_FAILURE);
	}
	temp = strcpy(temp, path);
	// Check if last char of str is /
	check_str_ending(temp);
	// Append
	append_str(temp, filename);

	if (type == 4)
	{
		append_str(temp, "/");
	}
	return temp;
}

/**
 * check_str_ending() - Checks if the string is ending with a /.
 * @param str The string to check.
 */
void check_str_ending(char *str)
{
	int len = strlen(str);
	if (str[len - 1] != '/')
	{
		str = strcat(str, "/");
	}
}

/**
 * append_str() - appands a string with another string
 * @param dest The string to append.
 * @param src The string to append with.
 */
void append_str(char *dest, char *src)
{
	dest = strcat(dest, src);
}
