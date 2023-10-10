#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <pthread.h>

char *get_options(int argc, char *argv[], pthread_t *number_of_threads);
void *find_files(void **items);
struct items **open_dir(char *s);
int get_size_of_file(char *name_of_file);
char *get_new_path(char *dir, char *current_dir, bool is_directory);

struct items{
	struct dirent *item;
	char *starting_directory;
	int j;
}items;

int main(int argc, char *argv[]){

	pthread_t number_of_threads = 1;
	char *directory = get_options(argc, argv, &number_of_threads);

	pthread_t threads[number_of_threads];

	struct items **items = open_dir(directory);

	for (pthread_t i = 0; i < number_of_threads; i++)
	{
		pthread_create(&threads[i],NULL,find_files,(void *)items);
	}
	
	for (pthread_t i = 0; i < number_of_threads; i++)
	{
		pthread_join(threads[i],NULL);
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
char *get_options(int argc, char *argv[], pthread_t *number_of_threads){
	int opt;
	char *dir = "./";
	
	while ((opt = getopt(argc, argv, "j:f:")) != -1){
		switch (opt)
		{
		case 'f':
			dir = optarg;
			break;
		case 'j':
			*number_of_threads = atoi(optarg);
			break;
		case ':':
			printf("option needs a value\n");
			break;
		}
	}
	return dir;
}


void *find_files(void **items){
	struct items **files = (struct items **)items;
	//DIR *dir = open_dir(dir_name);
	files[0]->item->d_name;

	int i = 0;
	while(files[i] != NULL){
		if((strncmp(files[i]->item->d_name, ".", 2) == 0) || (strncmp(files[i]->item->d_name, "..", 3) == 0)){
			continue;
		}
		if(files[i]->item->d_name == 4){
			char *new_dir = files[i]->item->d_name;
			char *new_path = get_new_path(new_dir, "./", true);
			struct dirent **new_dirent = open_dir(new_path);
			find_files(new_path);
		}

		char *path = get_new_path(files[i]->item->d_name, "./", false);
		int size = get_size_of_file(path);
		
		printf("%s size: %d\n", path, size);

		i++;
	}

	return NULL;
}

/**
 * open_dir() - opens a directory and checks if it worked.
 * @s: Directory to open.
 * @return: A DIR pointer.
*/
struct items **open_dir(char *s){
	DIR *dir = opendir(s);
	if(dir == NULL){
		fprintf(stderr, "Failed to open dir: %s", s);
		return NULL;
	}

	struct dirent *pDirent;
	struct items **items = malloc(sizeof(struct items*));

	for(int i = 0; (pDirent = readdir(dir)) != NULL ; i++){
		struct items *pdir = malloc(sizeof(struct items));
		pdir->item = pDirent;
		items[i] = pdir;
		items[i]->starting_directory = s;
	}

	return items;
}


/**
 * get_size_of_file() - Uses lstat to get the size of a file.
 * @path: The path to the file.
 * @return: The size of the file.
*/
int get_size_of_file(char *name_of_file){

	FILE *fp = fopen(name_of_file, "r");
	if(fp == NULL){
		fprintf(stderr, "Could not open file: %s\n", name_of_file);
		return -1;
	}
	fclose(fp);

	struct stat info;
	if(lstat(name_of_file, &info)){
		perror(name_of_file);
		exit(EXIT_FAILURE);
	}

	int size = info.st_size;

	return size;
}


char *get_new_path(char *name, char *current_dir, bool is_directory){
	char *new_dir = calloc(strlen(name) + strlen(current_dir) + 5, 1);
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