#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include "parser.h"
#include <stdbool.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

/*
 * Implementation of a makeprogram
 *
 * This program is used to automatically compile programs.
 *
 * @Authors: Filip Kanon (c22fkn@cs.umu.se)
 *
 * Version information:
 *   v1.0 2023-09-26: First labres submission.
 */

FILE *open_file(char *file);
pid_t make_fork(void);
char *get_options(int argc, char *argv[], bool *force_compile, bool *suppress);
makefile *get_parsed_makefile(FILE *fp, char *file);
void select_makefile_build_target(int argc, char *argv[], makefile *make, bool force_compile, bool suppress);
int build_target(makefile *make, const char *target, bool force_compile, bool suppress);
bool is_modified_more_recently(const char *prereq, const char *target);
void compile(bool suppress, char **cmd);
void print_commands(bool suppress, char **cmd);

int main(int argc, char *argv[])
{
	bool suppress = false;
	bool force_compile = false;

	char *file = get_options(argc, argv, &force_compile, &suppress);
	FILE *fp = open_file(file);

	struct makefile *make = get_parsed_makefile(fp, file);

	select_makefile_build_target(argc, argv, make, force_compile, suppress);
	makefile_del(make);

	return 0;
}

/**
 * get_options() - Retrives all the flags sent to the program.
 * 
 * @argc: Number of commandline arguments.
 * @argv: The commandline arguments.
 * @force_compile: If true all prerequisites will compiled. If false only the modified files will be recompiled.
 * @suppress: If true the output will be suppressed, if false the output will be shown.
 * @return: A string for the name of the makefile that shall be used.
*/
char *get_options(int argc, char *argv[], bool *force_compile, bool *suppress){
	int opt;
	char *file = "mmakefile";
	
	while ((opt = getopt(argc, argv, "Bsf:")) != -1){
		switch (opt)
		{
		case 'f':
			file = optarg;
			break;
		case 's':
			*suppress = true;
			break;
		case 'B':
			*force_compile = true;
			break;
		case ':':
			printf("option needs a value\n");
			break;
		}
	}
	return file;
}

/**
 * get_parsed_makefile() - Parses the file, closes the file pointer and checks that the parseing worked.
 * 
 * @fp: The pointer to the file.
 * @file: Pointer to the name of the file.
 * @return: A parsed makefile pointer.
*/
makefile *get_parsed_makefile(FILE *fp, char *file){
	struct makefile *make = parse_makefile(fp);
	fclose(fp);
	
	if (make == NULL)
	{
		fprintf(stderr, "%s: Could not parse makefile\n", file);
		exit(EXIT_FAILURE);
	}
	return make;
}

/**
 * select_makefile_build_target() - Selects the makefile that should be used and builds the target.
 * 
 * @argc: Number of commandline arguments.
 * @argv: The commandline arguments.
 * @make: The parsed makefile.
 * @force_compile: If true all prerequisites will compiled. If false only the modified files will be recompiled.
 * @suppress: If true the output will be suppressed, if false the output will be shown.
*/
void select_makefile_build_target(int argc, char *argv[], makefile *make, bool force_compile, bool suppress){
	bool build_default_target = true;
	for (; optind < argc; optind++)
	{
		build_default_target = false;

		rule *r = makefile_rule(make, argv[optind]);
		if (r == NULL)
		{
			fprintf(stderr, "%s: Target does not exist!\n", argv[optind]);
			exit(EXIT_FAILURE);
		}
		build_target(make, argv[optind], force_compile, suppress);
	}

	if (build_default_target)
	{
		// Build default target.
		build_target(make, makefile_default_target(make), force_compile, suppress);
	}
}

/**
 * build_target() - Selects what files needs to be compiled.
 * 
 * @make: The makefile struct.
 * @target: The target that needs to be compiled.
 * @force_compile: Boolean to force the compilation.
 * @suppress: Boolean to suppress the output.
 * @return: 0 if the function is done.
 */
int build_target(makefile *make, const char *target, bool force_compile, bool suppress)
{
	// Check prereq and compile them if needed.
	rule *rule = makefile_rule(make, target);
	if (rule == NULL)
	{
		// There are no rule for this prerequirement.
		return 0;
	}
	const char **prereq = rule_prereq(rule);

	// Compile prereq. If prereq is newer than the target.
	for (int i = 0; prereq[i] != NULL; i++)
	{
		build_target(make, prereq[i], force_compile, suppress);

		if (force_compile || is_modified_more_recently(prereq[i], target))
		{
			// Compile the prereq.
			char **prereq_cmd = rule_cmd(makefile_rule(make, target));
			compile(suppress, prereq_cmd);
			continue;
		}
	}
	return 0;
}

/**
 * is_modified_more_recently() - Checks is prereq is modified more recently then target.
 * @prereq: The pre-requirement.
 * @target: The target.
 * @return: True if prereq is modified more recently then target. False if not.
 */
bool is_modified_more_recently(const char *prereq, const char *target)
{

	FILE *fp = fopen(prereq, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "mmake: No rule to make target '%s'", prereq);
		exit(EXIT_FAILURE);
	}
	fclose(fp);

	FILE *fp2 = fopen(target, "r");
	if (fp2 == NULL)
	{
		return true;
	}
	fclose(fp2);

	struct stat prereq_info;
	if (lstat(prereq, &prereq_info))
	{
		perror(prereq);
		exit(EXIT_FAILURE);
	}

	struct stat target_info;
	if (lstat(target, &target_info))
	{
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
FILE *open_file(char *file)
{
	FILE *fp = NULL;
	fp = fopen(file, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "%s: No such file or directory\n", file);
		exit(EXIT_FAILURE);
	}
	return fp;
}

/**
 * make_fork() - Creates a new fork.
 * @return: The pid of the newly made child process.
 */
pid_t make_fork(void)
{
	pid_t pid = fork();
	if (pid == -1)
	{
		perror("Failed to make a new fork!");
		exit(EXIT_FAILURE);
	}
	return pid;
}

/**
 * compile() - runs the compile command.
 * @suppress: Boolean, if true there will be no output.
 * @cmd: The command to compile.
*/
void compile(bool suppress, char **cmd)
{
	pid_t pid = make_fork();
	if (pid == 0)
	{
		print_commands(suppress, cmd);
		execvp(cmd[0], cmd);
	}
	int status;
	wait(&status);

	if (WEXITSTATUS(status) == 1)
	{
		exit(EXIT_FAILURE);
	}
}

/**
 * print_commands() - Prints the commands that are executed if suppress is false.
 * @suppress: Boolean, if true there will be no output.
 * @cmd: The command to print.
*/
void print_commands(bool suppress, char **cmd)
{
	if (!suppress)
	{
		for (int i = 0; cmd[i] != NULL; i++)
		{
			printf("%s", cmd[i]);
			if (cmd[i + 1] == NULL){
				printf("\n");
			}
			else{
				printf(" ");
			}
		}
		fflush(stdout);
	}
}