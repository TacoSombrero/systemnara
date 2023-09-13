#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>
// #include <sys/wait.h>
// #include <unistd.h>
#include <fcntl.h>

#define MAXLENGHT 1024

int check_prog_params(int argc, const char *argv[], FILE **fp);
char **readline(int *lineCount, FILE *fp);
void kill_lines(char **lines, const int lineCount);
void runCommands(char **lines, const int lineCount, const int i);
void split_line(char *line, char **args);
void changeFileDescriptors(int, int);
int makeFork(void);
void createPipe(int newPipe[2]);

int main(int argc, const char *argv[])
{

	FILE *fp;
	int lineCount = 0;
	pid_t ppid = getpid();

	if (check_prog_params(argc, argv, &fp) == 1)
	{
		return (EXIT_FAILURE);
	}

	char **lines = NULL;

	if (argc < 2)
	{
		// Read from stdin
		lines = readline(&lineCount, NULL);
	}
	else
	{
		// Read from file
		lines = readline(&lineCount, fp);
		fclose(fp);
	}

	int pipe1[2];
	int pipe2[2];

	int i;
	pid_t pid = 0;
	for (i = 0; i < lineCount && pid == 0; i++)
	{
		if (i % 2 == 1 && getpid() != ppid)
		{
			createPipe(pipe1);
		}
		else if(i % 2 == 0 && getpid() != ppid)
		{
			createPipe(pipe2);
		}
		pid = makeFork();
	}
	

	if(pid != 0){
		i--;
	}

	int *inputPipe;
	int *outputPipe;
	if (i % 2 == 1){
		outputPipe = pipe2;
		inputPipe = pipe1;
	}
	else{
		outputPipe = pipe1;
		inputPipe = pipe2;
	}

	if (getpid() != ppid && lineCount != 1)
	{
		if(pid != 0){
			// Change STDIN to pipe for every child.
			changeFileDescriptors(inputPipe[0], STDIN_FILENO);
			close(inputPipe[1]);
		}

		if (getppid() != ppid)
		{
			// Change STDOUT to pipe for every child except the first child.
			changeFileDescriptors(outputPipe[1], STDOUT_FILENO);
			close(outputPipe[0]);
		}
	}

	if (getpid() != ppid)
	{
		runCommands(lines, lineCount, i);
	}
	
	int status;
	wait(&status);
	if (!WIFSTOPPED(status)){
		kill_lines(lines, lineCount);
		exit(EXIT_FAILURE);
	}
	kill_lines(lines, lineCount);
	return 0;
}

int check_prog_params(int argc, const char *argv[], FILE **fp)
{
	if (argc == 2)
	{
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
	else if (argc > 2)
	{
		fprintf(stderr, "usage: %s [FILE]\n", argv[0]);
		return 1; // Return 1 for failure.
	}
	return 0;
}

char **readline(int *lineCount, FILE *fp)
{
	int buffsize = MAXLENGHT;
	char **lines = (char **)malloc(MAXLENGHT * sizeof(char *));
	if (lines == NULL)
	{
		perror("Memory allocation failed!");
		exit(EXIT_FAILURE);
	}
	*lineCount = 0;
	while (1)
	{

		char *line = (char *)malloc(buffsize * sizeof(char));
		if (line == NULL)
		{
			perror("Memory allocation failed!");
			exit(EXIT_FAILURE);
		}

		if (fp == NULL)
		{
			if (fgets(line, MAXLENGHT, stdin) == NULL)
			{
				free(line);
				break;
			}
		}
		else
		{
			if (fgets(line, MAXLENGHT, fp) == NULL)
			{
				free(line);
				break;
			}
		}

		// Remove \n at the end of the line and replace with \0
		int len = strlen(line);
		if (line[len - 1] == '\n')
		{
			line[len - 1] = '\0';
		}

		lines[*lineCount] = line;
		*lineCount += 1;

		if (*lineCount >= buffsize)
		{
			buffsize *= 2;
			lines = realloc(lines, buffsize * sizeof(char *));
			if (lines == NULL)
			{
				perror("Memory allocation failed!");
				exit(EXIT_FAILURE);
			}
		}
	}

	if (*lineCount != 0)
	{
		// Reallocate memory to actual number of lines read.
		lines = realloc(lines, *lineCount * sizeof(char *));
		if (lines == NULL)
		{
			perror("Memory allocation failed!");
			exit(EXIT_FAILURE);
		}
	}

	return lines;
}

void changeFileDescriptors(int fd, int fd2)
{
	if (dup2(fd, fd2) < 0)
	{
		perror("Failed on file descriptor redirect!");
		exit(EXIT_FAILURE);
	}
	close(fd);
}

void createPipe(int newPipe[2])
{
	if (pipe(newPipe) == -1)
	{
		perror("Failed to create a new pipe!");
		exit(EXIT_FAILURE);
	}
}

int makeFork(void)
{
	pid_t pid = fork();
	if (pid == -1)
	{
		perror("Failed to make a new fork!");
		exit(EXIT_FAILURE);
	}
	return pid;
}

/* 	Every process should run the command at line |lineCount - i|.
	Because then the last child process will run the first command in the line.
	After that the second child runs the command after that.
	until the "parent process of them all" eventually runs the last inputed command at the end.
*/
void runCommands(char **lines, const int lineCount, const int i)
{
	char *args[1024];
	split_line(lines[lineCount - i], args);
	if(execvp(args[0], args) == -1){
		perror(args[0]);
		exit(EXIT_FAILURE);
	}
}

// split the line into arguments.
void split_line(char *line, char **args)
{
	char *token = strtok(line, " ");
	int i = 0;
	while (token != NULL)
	{
		args[i] = token;
		token = strtok(NULL, " ");
		i++;
	}
	args[i] = '\0';
}

void kill_lines(char **lines, const int lineCount)
{
	for (int i = 0;i < lineCount; i++)
	{
		free(lines[i]);
	}
	free(lines);
}
