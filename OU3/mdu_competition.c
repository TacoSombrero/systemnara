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
#include <time.h>

typedef struct threadItems
{
	stack *directories;
	int total_size_of_files;
	int number_of_threads;
	int last_thread_completed;
	int number_of_working_threads;
	bool first_thread;
	int exit_id;
} threadItems;

void start_threads(pthread_t *thread_ids, threadItems *items);
char *allocate_for_string(char *str);
void get_options(int argc, char *argv[], threadItems *items);
void *run(void *temp);
int open_dir(char *str, threadItems *items);
int get_size_of_file(char *path);
char *get_new_path(char *path, char *filename, int type);

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sizelock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t working_threads_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t exit_id_lock = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
time_t starttime, endtime;

int main(int argc, char *argv[])
{

	char *default_folder = "./";
	struct threadItems *items = malloc(sizeof(struct threadItems));
	stack *s = stack_create();
	items->directories = s;
	items->total_size_of_files = 0;
	items->number_of_threads = 0;
	items->exit_id = 0;
	items->last_thread_completed = 0;
	items->number_of_working_threads = 0;
	items->first_thread = true;

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

		// If on the last argument, clean up and exit.
		if (optind == (argc - 1))
		{
			int e = items->exit_id;
			stack_kill(s);
			free(items);
			return e;
		}
	}

	char *new_path = allocate_for_string(default_folder);
	items->directories = stack_push(items->directories, new_path);
	pthread_t *thread_ids = calloc(items->number_of_threads, sizeof(pthread_t));

	start_threads(thread_ids, items);
	run(items);

	free(thread_ids);

	printf("%d\t%s\n", items->total_size_of_files, default_folder);

	items->total_size_of_files = 0;

	int e = items->exit_id;

	stack_kill(s);
	free(items);

	return e;
}

/**
 * start_threads() - Creates the requested number of threads.
 * @thread_ids: Array of pthread_t positions.
 * @items: A struct of type threadItems.
 */
void start_threads(pthread_t *thread_ids, threadItems *items)
{
	for (int i = 0; i < (items->number_of_threads - 1); i++)
	{
		pthread_create(&thread_ids[i], NULL, run, (void *)items);
	}

	for (int i = 0; i < (items->number_of_threads - 1); i++)
	{
		pthread_join(thread_ids[i], NULL);
	}
}

/**
 * allocate_for_string)() - Allocates memory for a string.
 * @str: The string to allocate memory for.
 * @return: A char pointer to the newly allocated string.
 */
char *allocate_for_string(char *str)
{
	size_t string_size = strlen(str) + 1;
	char *temp = (char *)malloc(string_size);
	temp = strcpy(temp, str);
	return temp;
	
}

/**
 * get_options() - Retrives all the flags sent to the program.
 *
 * @argc: Number of commandline arguments.
 * @argv: The commandline arguments.
 * @items: A struct of type threadItems.
 * @return: A string for the name of the makefile that shall be used.
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
 * @temp: A struct of type threadItems.
 * @return: A NULL pointer.
 */
void *run(void *temp)
{
	struct threadItems *items = (threadItems *)temp;
	int local_size = 0;
	while (1)
	{
		pthread_mutex_lock(&lock);
		while (stack_empty(items->directories))
		{
			pthread_mutex_lock(&working_threads_lock);
			if (!items->first_thread && items->number_of_working_threads == 0)
			{
				pthread_mutex_unlock(&working_threads_lock);
				items->last_thread_completed = 1;
				pthread_mutex_unlock(&lock);
				pthread_mutex_lock(&sizelock);
				items->total_size_of_files += local_size;
				pthread_mutex_unlock(&sizelock);
				pthread_cond_broadcast(&cond);
				return NULL;
			}
			else if (items->last_thread_completed)
			{
				pthread_mutex_unlock(&working_threads_lock);
				pthread_mutex_unlock(&lock);
				pthread_mutex_lock(&sizelock);
				items->total_size_of_files += local_size;
				pthread_mutex_unlock(&sizelock);
				return NULL;
			}
			else
			{
				pthread_mutex_unlock(&working_threads_lock);
				pthread_cond_wait(&cond, &lock);
			}
		}
		// Get a new directory path to work with
		char *str = stack_top(items->directories);
		items->directories = stack_pop(items->directories);
		pthread_mutex_unlock(&lock);

		// Change the number of working threads
		pthread_mutex_lock(&working_threads_lock);
		items->first_thread = false;
		items->number_of_working_threads++;
		pthread_mutex_unlock(&working_threads_lock);

		local_size += open_dir(str, items);

		pthread_mutex_lock(&working_threads_lock);
		items->number_of_working_threads--;
		pthread_mutex_unlock(&working_threads_lock);
	}
}

/**
 * open_dir() - opens a directory and checks if it worked.
 * @str: Directory to open.
 * @items: A struct of type threadItems.
 * @return: A DIR pointer.
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
				pthread_mutex_lock(&lock);
				items->directories = stack_push(items->directories, new_path);
				pthread_cond_broadcast(&cond);
				pthread_mutex_unlock(&lock);
			}
			else
			{
				size += get_size_of_file(new_path);
				free(new_path);
			}
		}
		closedir(dir);
	}
	else
	{
		size += get_size_of_file(str);
	}
	free(str);
	return size;
}

/**
 * get_size_of_file() - Uses lstat to get the size of a file.
 * @path: The path to the file.
 * @items: A struct of type threadItems.
 * @return: The size of the file.
 */
int get_size_of_file(char *path)
{
	struct stat info;
	lstat(path, &info);
	return info.st_blocks;
}

/**
 * get_new_path() - Creates a new path for a directory or a file.
 * @path: The current path.
 * @filename: The name of the directory or file.
 * @type: If type is equal to 4 a / will be appended to the end of the path. Otherwise not.
 * @return: The new path.
 */
char *get_new_path(char *path, char *filename, int type)
{
	int x = 3;

	size_t string_size = strlen(path) + strlen(filename) + x;
	char *temp = (char *)malloc(string_size);
	temp = strcpy(temp, path);
	// Check if last char of str is /
	int len = strlen(temp);
	if (temp[len - 1] != '/')
	{
		temp = strcat(temp, "/");
	}
	// Append
	temp = strcat(temp, filename);

	if (type == 4)
	{
		temp = strcat(temp, "/");
	}
	return temp;
}
