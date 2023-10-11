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
void get_new_path(char *current_dir, char *name, bool is_directory, char *new_path);
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
		/*int len = strlen(argv[optind]);
		if (argv[optind][len - 1] == '\n'){
			argv[optind][len - 1] = '\0';
		}*/
		int len = strlen(argv[optind]);
		char s[len];
		strcpy(s, argv[optind]);
		open_dir(argv[optind], toThread);
		toThread->size += get_size_of_file(argv[optind]);
		printf("%d\t%s\n", toThread->size, s);
	}

	if(use_cwd){
		open_dir("./", toThread);
		toThread->size += get_size_of_file(".");
		printf("%d\t%s\n", toThread->size, ".");
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
	//char *new_file_path;
	//char *new_dir_path;
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
			int len = strlen(pDirent->d_name) + strlen(str) + 2;
			char *new_path = malloc(len * sizeof(char));
			if(pDirent->d_type == 4){
				
				get_new_path(str, pDirent->d_name, true, new_path);
				toThread->size += get_size_of_file(new_path);
				open_dir(new_path, toThread);
			}else{
				get_new_path(str, pDirent->d_name, false, new_path);
				toThread->size += get_size_of_file(new_path);
			}
		}	
	}else{
		toThread->size = get_size_of_file(str);
	}
}

/**
 * get_size_of_file() - Uses lstat to get the size of a file.
 * @path: The path to the file.
 * @return: The size of the file.
*/
int get_size_of_file(char *name_of_file){

	struct stat info;
	if(lstat(name_of_file, &info)){
		perror(name_of_file);
		return 0;
	}

	int size = info.st_blocks;

	return size;
}

void get_new_path(char *current_dir, char *name, bool is_directory, char *new_path){
	strcpy(new_path, current_dir);
	strcat(new_path, name);

	if(is_directory){
		strcat(new_path, "/");
	}
}