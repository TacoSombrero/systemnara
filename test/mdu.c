#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "queue.h"
#include <pthread.h>

/**
 * Create a struct that holds all the data (because a thread can only take one argument)
 */
typedef struct info {
	int size;
	int num_pthread;
	int currently_in_use;
	bool last_completed;
	int return_code;
	queue *get_queue;
} info;

pthread_mutex_t LOCK = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ADD_LOCK = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t SIZE_LOCK = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t CON = PTHREAD_COND_INITIALIZER;

info *set_up_info(int argc, char **argv);
void select_directory(int argc, char **argv, info *get_info);
char *allocate_string(char *str);
void start_search(info *get_info);
void check_queue_no_threads(info *get_info);
void *check_queue_with_threads(void *arg);
void traverse_directory(char *dir_name, info *get_info);
void check_type(struct dirent *entity, info *get_info, char *dir_name);
void get_new_path(char *old_name, char *new_name, bool is_directory, char *new_path);
int get_file_size(const char *file_name, info *get_info);

/**
 * main() - Main program of the code
 * 
 * @argc: A integer of number argument provided.
 * @argv: A string pointer to the argument.
 */
int main(int argc, char *argv[]) {	
	//Get all flags
	info *get_info = set_up_info(argc, argv);
	
	//Select the directorys
	select_directory(argc, argv, get_info);

	int return_code = get_info->return_code;
	free(get_info);

	return return_code;
}

info *set_up_info(int argc, char **argv) {
	//Get all the flags
	info *get_info = malloc(sizeof(info));
	if (get_info == NULL) {
		fprintf(stderr, "Failure to allocate memory\n");
		exit(EXIT_FAILURE);
	}

	get_info->get_queue = create_queue();
	get_info->num_pthread = 0;
	get_info->size = 0;
	get_info->return_code = 0;
	get_info->last_completed = false;

	int opt;
	while((opt = getopt(argc, argv, "j:")) != -1) { 
		switch(opt) {
			case 'j':
				get_info->num_pthread = atoi(optarg);
				get_info->currently_in_use = 0;
				break;
			case ':':
				printf("Usage: -j <integer value>\n");
				break;
		}
	}

	return get_info;
}

void select_directory(int argc, char *argv[], info *get_info) {
	bool more_args = false;

	for(; optind < argc; optind++) {
		more_args = true;

		char *new_path = allocate_string(argv[optind]);
		
		//Add information we recieved and into the queue
		get_info->get_queue = queue_enqueue(get_info->get_queue, new_path);

		//Start the search
		start_search(get_info);

		//Print out result
		printf("%d\t%s\n", get_info->size, argv[optind]);

		//Reset get_info for next iteration
		get_info->size = 0;
		get_info->last_completed = false;

	}

	if (!more_args) {
		char *default_dir = allocate_string(".");

		//Add information we recieved
		get_info->get_queue = queue_enqueue(get_info->get_queue, default_dir);

		start_search(get_info);

		printf("%d\t%s\n", get_info->size, default_dir);
	}

	queue_kill(get_info->get_queue);
}

char *allocate_string(char *str) {
	//Get the string length and allocate memory for it
	int length = strlen(str);

	char *new_str = calloc(length+1, sizeof(char));
	if (new_str == NULL) {
		fprintf(stderr, "Failure to allocate memory\n");
		exit(EXIT_FAILURE);
	}

	//Add the old string to the new string
	strcat(new_str, str);

	return new_str;
}

void start_search(info *get_info) {

	//if (get_info->num_pthread < 1) {
		check_queue_no_threads(get_info);
		return;
	//}

	//Create the number threads
	/*pthread_t *threads_id = malloc((get_info->num_pthread) * sizeof(pthread_t));
	
	if (get_info->num_pthread != 0) {
		for (int i = 0; i < get_info->num_pthread; i++) {
			pthread_create(&threads_id[i], NULL, check_queue_with_threads, (void *) get_info);
		}
	}

	for (int i = 0; i < get_info->num_pthread; i++) {
		pthread_join(threads_id[i], NULL);
	}

	free(threads_id);*/
}

void check_queue_no_threads(info *get_info) {
	while (true) {
		bool queue_empty = queue_is_empty(get_info->get_queue);

		if (queue_empty) {
			break;
		}

		//Take task and perform it
		char *get_task = (char *) queue_front(get_info->get_queue);
		get_info->get_queue = queue_dequeue(get_info->get_queue);
		traverse_directory(get_task, get_info);
	}
}

/*void *check_queue_with_threads(void *arg) {

	//Get the struct
	info *get_info = (info *) arg;

	//Start the algoritm
	while (true) {
		pthread_mutex_lock(&LOCK);

		//Check if queue is empty and there are no threads working
		while (queue_is_empty(get_info->get_queue)) {
			if ((get_info->currently_in_use == 0) && !(get_info->last_completed)) {
				get_info->last_completed = true;
				pthread_cond_broadcast(&CON);
				pthread_mutex_unlock(&LOCK);
				return NULL;
			} else if (get_info->last_completed) {
				pthread_mutex_unlock(&LOCK);
				return NULL;
			} else {
				pthread_cond_wait(&CON, &LOCK);
			}
		}

		//Get the char from queue and unlock
		char *get_task = (char *) queue_front(get_info->get_queue);
		get_info->get_queue = queue_dequeue(get_info->get_queue);
		pthread_mutex_unlock(&LOCK);

		//Lock to plus 
		pthread_mutex_lock(&ADD_LOCK);
		get_info->currently_in_use++;
		pthread_mutex_unlock(&ADD_LOCK);

		traverse_directory(get_task, get_info);

		pthread_mutex_lock(&ADD_LOCK);
		get_info->currently_in_use--;
		pthread_mutex_unlock(&ADD_LOCK);
	}

	return NULL;
}*/

void traverse_directory(char *dir_name, info *get_info) {
	//Open directory
	DIR *directory = opendir(dir_name);
	if (directory != NULL) {
		//Add the size of the directory
		//pthread_mutex_lock(&SIZE_LOCK);
		get_info->size += get_file_size(dir_name, get_info);
		//pthread_mutex_unlock(&SIZE_LOCK);

		//Read the directory
		struct dirent *entity = readdir(directory);
		if (entity == NULL) {
			fprintf(stderr, "ERROR: Failure to read directory %s\n", dir_name);
			//exit(EXIT_FAILURE);
		}

		//Loop through the entity
		for (int i = 0; entity != NULL; i++) {

			//Check that entity name isnt '.' or '..'
			if (strcmp(entity->d_name, ".") != 0 && strcmp(entity->d_name, "..") != 0) {

				//Check what type and get new path
				check_type(entity, get_info, dir_name);
			}

			entity = readdir(directory);
			if (entity == NULL) {
				break;
			}
		}

		if (closedir(directory) == -1) {
			fprintf(stderr, "Couldn't close directory: %s", dir_name);
			exit(EXIT_FAILURE);
		}
		
	} else {
		//pthread_mutex_lock(&SIZE_LOCK);
		get_info->size += get_file_size(dir_name, get_info);
		//pthread_mutex_unlock(&SIZE_LOCK);
	}
	free(dir_name);
}

void check_type(struct dirent *entity, info *get_info, char *dir_name) {
	/*Why we do this is because get_new_path can add two more 
	characters to the new_path*/
	int add_num = 3;

	int length = strlen(dir_name) + strlen(entity->d_name) + add_num;
	char *new_path = calloc(length, sizeof(char));
	if (new_path == NULL) {
		fprintf(stderr, "Failure to allocate memory\n");
		exit(EXIT_FAILURE);
	}

	//Check what type it is
	if (entity->d_type == 4) {
		get_new_path(dir_name, entity->d_name, true, new_path);

		//pthread_mutex_lock(&LOCK);
		get_info->get_queue = queue_enqueue(get_info->get_queue, new_path);
		//pthread_mutex_unlock(&LOCK);

		//Signal that another dir got added
		pthread_cond_signal(&CON);

	} else if (entity->d_type == 8) {
		get_new_path(dir_name, entity->d_name, false, new_path);

		//pthread_mutex_lock(&SIZE_LOCK);
		get_info->size += get_file_size(new_path, get_info);
		//pthread_mutex_unlock(&SIZE_LOCK);

		free(new_path);
	} else {
		//If it isn't a file or directory
		get_new_path(dir_name, entity->d_name, false, new_path);

		get_info->size += get_file_size(new_path, get_info);
		free(new_path);
	}
}

void get_new_path(char *old_name, char *new_name, bool is_directory, char *new_path) {
	strcat(new_path, old_name);

	//Check if old_name already has '/'
	int len_old = strlen(old_name);
	if (old_name[len_old-1] != '/') {
		strcat(new_path, "/");
	}
	
	if (new_name != NULL) {
		strcat(new_path, new_name);
	}

	if (is_directory) {
		strcat(new_path, "/");
	}
}

int get_file_size(const char *path, info *get_info) {
	struct stat path_info;
	if (lstat(path, &path_info) == -1) {
		perror(path);
		return 0;
	}

	//Check for read permission
	if (!(path_info.st_mode & S_IRUSR)) {
		fprintf(stderr, "Failure to read '%s': Permission denied\n", path);
		get_info->return_code = 1;
	}

	return path_info.st_blocks;
}