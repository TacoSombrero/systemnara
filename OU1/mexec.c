#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 * Implementation of a pipeline
 *
 * Authors: Filip Kanon (c22fkn@cs.umu.se)
 *
 * Version information:
 *   v1.0 2023-09-17: First labres submission.
 */

#define MAXLENGHT 1024

int check_prog_params(int argc, const char *argv[], FILE **fp);
void handler(int signo);
void create_forks_and_pipes(int pipe1[2], int pipe2[2], pid_t ppid, int *i);
void redirect_file_descriptors(int *input_pipe, int *output_pipe, int pipe1[2], int pipe2[2], pid_t ppid, int *i);

char **readline(FILE *fp);
void kill_lines(void);
void run_commands(const int i, pid_t ppid);
void split_line(char *line, char **args);
void change_file_descriptors(int, int);
int make_fork(void);
void create_pipe(int new_pipe[2]);


int lineCount = 0;
char **lines = NULL;
pid_t pid = 0;

int main(int argc, const char *argv[])
{

	signal(SIGUSR1, handler);

	FILE *fp;
	pid_t ppid = getpid();

	if (check_prog_params(argc, argv, &fp) == 1){
		return (EXIT_FAILURE);
	}

	if (argc < 2){
		// Read from stdin
		lines = readline(NULL);
	}
	else{
		// Read from file
		lines = readline(fp);
		fclose(fp);
	}

	int pipe1[2];
	int pipe2[2];
	int i;

	create_forks_and_pipes(pipe1, pipe2, ppid, &i);

	int *input_pipe = NULL;
	int *output_pipe = NULL;

	if(getpid() != ppid){
		redirect_file_descriptors(input_pipe, output_pipe, pipe1, pipe2, ppid, &i);
		run_commands(i, ppid);
	}

	int status;
	wait(&status);

	if (WIFSTOPPED(status)){
		//fprintf(stderr, "EXIT FAIL\n");
		exit(EXIT_FAILURE);
	}else{
		//fprintf(stderr, "return 0\n");
		kill_lines();
		return 0;
	}	
}

/**
 * handler() - De allocates all memory if the signal "SIGUSR1" is recived.
 * @signo: Type of recieved signal.
 */
void handler(int signo){
	if(signo == SIGUSR1){
		kill_lines();
		kill(-1*pid, SIGKILL);
		exit(EXIT_FAILURE);
	}	
}

/**
 * create_forks_and_pipes() - Creates new pipes and makes forks.
 * @pipe: Pointers to the pipes.
 * @ppid: The pid of the first process (The process that started it all).
 * @i: A pointer that where the number of generated processes will be stored.
 */
void create_forks_and_pipes(int pipe1[2], int pipe2[2], pid_t ppid, int *i){
	for (*i = 0; *i < lineCount && pid == 0; *i += 1){
		if (*i % 2 == 1 && getpid() != ppid){
			create_pipe(pipe1);
		}
		else if(*i % 2 == 0 && getpid() != ppid){
			create_pipe(pipe2);
		}
		pid = make_fork();
	}

	if(pid != 0){
		*i -= 1;
	}
}

/**
 * redirect_file_descriptors() - Redirects the STDIN and STDOUT of the child processes to the pipes.
 * This function will fluctuate back and forward between making pipe1 the input pipe or the output pipe.
 * Same goes for the pipe2. This depends on what index (i) the running process has. 
 * @input_pipe: Pointers to the pipe used as input.
 * @output_pipe: Pointers to the pipe used as output.
 * @pipe: Pointers to the pipes.
 * @ppid: The pid of the first process (The process that started it all).
 * @i: A pointer that where the number of generated processes will be stored.
 */
void redirect_file_descriptors(int *input_pipe, int *output_pipe, int pipe1[2], int pipe2[2], pid_t ppid, int *i){
	if (*i % 2 == 1){
		output_pipe = pipe2;
		input_pipe = pipe1;
	}
	else{
		output_pipe = pipe1;
		input_pipe = pipe2;
	}

	if (getpid() != ppid && lineCount != 1){
		if(pid != 0){
			// Change STDIN to pipe for every child.
			change_file_descriptors(input_pipe[0], STDIN_FILENO);
			close(input_pipe[1]);
		}

		if (getppid() != ppid){
			// Change STDOUT to pipe for every child except the first child.
			change_file_descriptors(output_pipe[1], STDOUT_FILENO);
			close(output_pipe[0]);
		}
	}
}

/**
 * check_prog_params() - Checks if the right amount of command line arguments were given.
 * And if the file can be opened.
 * @argc: Number of command line arguments
 * @argv: command line arguments
 * @fp: Pointer to the file.
 *
 * Returns: 0 if everything is ok. 
 * Returns: 1 on error.
 */
int check_prog_params(int argc, const char *argv[], FILE **fp)
{
	if (argc == 2){
		// Read from file
		*fp = fopen(argv[1], "r");
		// Check if file was successfully opened
		if (*fp == NULL)
		{
			fprintf(stderr, "%s: No such file or directory\n", argv[1]);
			return 1;
		}
		return 0;
	}
	else if (argc > 2){
		fprintf(stderr, "usage: %s [FILE]\n", argv[0]);
		return 1; // Return 1 for failure.
	}
	return 0;
}

/**
 * readline() - Reads the input either from STDIN or a file.
 * @fp: Pointer to which file to read from. Set to NULL if input should be on STDIN.
 * Returns: A 2D array of lines read from the file or STDIN.
*/
char **readline(FILE *fp)
{
	int buffsize = MAXLENGHT;
	char **lines = (char **)malloc(MAXLENGHT * sizeof(char *));
	if (lines == NULL){
		perror("Memory allocation failed!");
		exit(EXIT_FAILURE);
	}
	lineCount = 0;
	while (1){

		char *line = (char *)malloc(buffsize * sizeof(char));
		if (line == NULL){
			perror("Memory allocation failed!");
			exit(EXIT_FAILURE);
		}

		if (fp == NULL){
			if (fgets(line, MAXLENGHT, stdin) == NULL){
				free(line);
				break;
			}
		}else{
			if (fgets(line, MAXLENGHT, fp) == NULL){
				free(line);
				break;
			}
		}

		// Remove \n at the end of the line and replace with \0
		int len = strlen(line);
		if (line[len - 1] == '\n'){
			line[len - 1] = '\0';
		}

		lines[lineCount] = line;
		lineCount += 1;

		if (lineCount >= buffsize){
			buffsize *= 2;
			lines = realloc(lines, buffsize * sizeof(char *));
			if (lines == NULL){
				perror("Memory allocation failed!");
				exit(EXIT_FAILURE);
			}
		}
	}

	if (lineCount != 0){
		// Reallocate memory to actual number of lines read.
		lines = realloc(lines, lineCount * sizeof(char *));
		if (lines == NULL){
			perror("Memory allocation failed!");
			exit(EXIT_FAILURE);
		}
	}

	return lines;
}

/**
 * change_file_descriptors() - Reads the input either from STDIN or a file.
 * @fp: Pointer to which file to read from. Set to NULL if input should be on STDIN.
 * Returns: A 2D array of lines read from the file or STDIN.
*/
void change_file_descriptors(int fd, int fd2)
{
	if (dup2(fd, fd2) < 0){
		perror("Failed on file descriptor redirect!");
		exit(EXIT_FAILURE);
	}
	close(fd);
}

/**
 * create_pipe() - Creates a new pipe.
 * @new_pipe: Array for the file descriptor pointers.
*/
void create_pipe(int new_pipe[2])
{
	if (pipe(new_pipe) == -1){
		perror("Failed to create a new pipe!");
		exit(EXIT_FAILURE);
	}
}

/**
 * make_fork() - Creates a new fork.
 * Returns: The pid of the newly made child process.
*/
int make_fork(void)
{
	pid_t pid = fork();
	if (pid == -1){
		perror("Failed to make a new fork!");
		exit(EXIT_FAILURE);
	}
	return pid;
}

/**
 * run_commands() - Runs the commands.
 * @i: The index for which command to be executed.
 * @ppid: The pid of the first process (The process that started it all).
*/
void run_commands(const int i, pid_t ppid)
{
	char *args[1024];
	split_line(lines[lineCount - i], args);
	if(execvp(args[0], args) == -1){
		perror(args[0]);
		kill_lines();
		kill(ppid, SIGUSR1);
		exit(EXIT_FAILURE);
	}
}

/**
 * split_line() - Splits one line inte separate words.
 * @line: Line to separate.
 * @args: A pointer to where the separated words will be stored.
*/
void split_line(char *line, char **args)
{
	char *token = strtok(line, " ");
	int i = 0;
	while (token != NULL){
		args[i] = token;
		token = strtok(NULL, " ");
		i++;
	}
	args[i] = '\0';
}

/**
 * kill_lines() - Deallocates all used memory.
*/
void kill_lines(void)
{
	for (int i = 0;i < lineCount; i++){
		free(lines[i]);
	}
	free(lines);
}
