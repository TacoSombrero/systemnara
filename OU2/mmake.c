#include <stdio.h> 
#include <unistd.h> 
#include "/usr/include/getopt.h"
#include "parser.h"
#include <stdbool.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

FILE *open_file(const char *file);
pid_t make_fork(void);
void compile_program(bool suppress, char **cmd);
int build_target(makefile *make, const char *target, bool force_compile, bool suppress);
bool is_modified_more_recently(const char *prereq, const char* target);

int main(int argc, char *argv[]){
	int opt;
	char *file = "mmakefile";
	bool suppress = false;
	bool force_compile = false;
	
	while((opt = getopt(argc, argv, "Bsf:")) != -1){
		switch (opt)
		{
		case 'f':
			//printf("filename: %s\n", optarg); 
			//Use the optarg file instead of mmakefile
			file = optarg;
			break;
		case 's':
			//printf("I should be very quiet...\n");
			suppress = true; 
			break;
		case 'B':
			//printf("Do what must be done lord vader.\n");
			force_compile = true;
			break;
		case ':': 
			printf("option needs a value\n"); 
			break;
		}
	}

	FILE *fp = open_file(file);
	
	if(fp == NULL){
		fprintf(stderr, "%s: No such file or directory\n", file);
		exit(EXIT_FAILURE);
	}
	struct makefile *make = parse_makefile(fp);
	fclose(fp);
	if(make == NULL){
		perror(file);
		exit(EXIT_FAILURE);
	}

	bool build_default_target = true;
	
	for(; optind < argc; optind++){
		build_default_target = false;
		
		//Build argv[optind] targets
		build_target(make, argv[optind], force_compile, suppress);	
	}

	if(build_default_target){
		//Build default target.
		build_target(make, makefile_default_target(make), force_compile, suppress);
		
	}

	return 0;
}


int build_target(makefile *make, const char *target, bool force_compile, bool suppress){
	//Check prereq and compile them if needed.
	rule *rule = makefile_rule(make, target);
	if(rule == NULL){
		//There are no rule for this prerequirement.
		return 0;
	}
	const char **prereq = rule_prereq(rule);
	//Compile prereq. If prereq is newer than the target.
	for(int i = 0; prereq[i] != NULL; i++){
		build_target(make, prereq[i], force_compile, suppress);
		int status;
		wait(&status);
		if(force_compile || is_modified_more_recently(prereq[i], target)){
			//Compile the prereq.
			char **prereq_cmd = rule_cmd(makefile_rule(make, target));
			compile_program(suppress, prereq_cmd);
			continue;
		}
	}
	return 0;
}

/**
 * is_modified_more_recently() - Checks is prereq is modified more recently then target.
 * @prereq: The pre-requirement.
 * @target: The target.
 * @return: True if @prereq: is modified more recently then @target:. False if not.
*/
bool is_modified_more_recently(const char *prereq, const char* target){

	FILE *fp = fopen(prereq, "r");
	FILE *fp2 = fopen(target, "r");
	if(fp == NULL ){
		return false;
	}	
	if(fp2 == NULL){
		return true;
	}
	fclose(fp);
	fclose(fp2);

	struct stat prereq_info;
	if(lstat(prereq, &prereq_info)){
		perror(prereq);
		exit(EXIT_FAILURE);
	}

	struct stat target_info;
	if(lstat(target, &target_info)){
		perror(target);
		exit(EXIT_FAILURE);
	}

	return (prereq_info.st_mtime > target_info.st_mtime);
}

/**
 * open_file() - Opens a file and checks if it succeded.
 * This function exits the program if the file doesn't exist.
 * @file: The name of the file to open.
 * @return: A file pointer to the newly opened file. Or NULL if file doesn't exist.
*/
FILE *open_file(const char *file){
	FILE *fp = NULL;
	fp = fopen(file, "r");
	return fp;
}

/**
 * make_fork() - Creates a new fork.
 * @return: The pid of the newly made child process.
*/
pid_t make_fork(void){
	pid_t pid = fork();
	if (pid == -1){
		perror("Failed to make a new fork!");
		exit(EXIT_FAILURE);
	}
	return pid;
}

void compile_program(bool suppress, char **cmd){
	pid_t pid = make_fork();
	if(pid == 0){
		if(!suppress){
			for(int i = 0; cmd[i] != NULL; i++){
				fprintf(stdout, "%s ", cmd[i]);
			}
			fprintf(stdout, "\n");
			//This might lead to error. I have no idea. Just remember that it might cause problems when running.
			//close(STDOUT_FILENO);
		}
		if(execvp(cmd[0], cmd) == -1){
			perror(cmd[0]);
			exit(EXIT_FAILURE);
		}
	}
}