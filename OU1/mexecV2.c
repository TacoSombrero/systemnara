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
void kill_lines(char **lines);
void runCommands(char **lines, int lineCount, int i);
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
	//int pipe2[2];

	createPipe(pipe1);

	int i;
	pid_t pid = 0;
	for (i = 0; i < lineCount && pid == 0; i++)
	{
		pid = makeFork();
	}


	if(pid != 0){
		i--;
	}

	if (getpid() != ppid)
	{
		if(getppid() != ppid){
			changeFileDescriptors(pipe1[1], STDOUT_FILENO);	
		}
		changeFileDescriptors(pipe1[0], STDIN_FILENO);
		

		
	}
	
		int status;
		wait(&status);
		runCommands(lines, lineCount, i);
	kill_lines(lines);
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
			fprintf(stderr, "Could not open the file: %s\n", argv[1]);
			return 1;
		}
		return 0;
	}
	else if (argc > 2)
	{
		fprintf(stderr, "Too many arguments!\n");
		fprintf(stderr, "Usage: %s OR %s <input_file>\n", argv[0], argv[0]);
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
		perror("Memory allocation failed");
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
		perror("ChangeFD");
		exit(1);
	}
	close(fd);
}

void createPipe(int newPipe[2])
{
	if (pipe(newPipe) == -1)
	{
		perror("MAIN PIPE");
		exit(1);
	}
}

int makeFork(void)
{
	pid_t pid = fork();
	if (pid == -1)
	{
		perror("Error on fork\n");
		exit(1);
	}
	return pid;
}

/* 	Every process should run the command at line |lineCount - i|.
	Because then the last child process will run the first command in the line.
	After that the second child runs the command after that.
	until the "parent process of them all" eventually runs the last inputed command at the end.
*/
void runCommands(char **lines, int lineCount, int i)
{
	char *args[1024];
	split_line(lines[lineCount - i], args);
	if(execvp(args[0], args) == -1){
		perror("runCommands ");
		exit(1);
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

void kill_lines(char **lines)
{
	for (int i = 0; lines[i] != NULL; i++)
	{
		free(lines[i]);
	}
	free(lines);
}
