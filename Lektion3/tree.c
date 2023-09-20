/**
 * @brief Contains code to help solve recursion exercises
 *        related to a tree of connected letters
 */
#include <stdlib.h>
#include <stdio.h>

/**
 * @brief Represents a letter connection in a tree, ex 'A->B'
 */
typedef struct Connection {
    char from;
    char to;
} Connection;

char rek(char ch, Connection *c, int i, int count);

/**
 * @brief Read a list of letter connections from file
 * 
 * @param filename file to read from
 * @param count returns the amount of connections
 * @return Connection* array of all connections
 */
Connection* read_connections_file(char* filename, int* count) {
    FILE* f;
    if ((f = fopen(filename, "r")) == NULL) {
        perror("file open");
        exit(EXIT_FAILURE);
    };

    char buffer[256];
    int connections_max_length = 1;
    Connection* connections = calloc(1, sizeof(Connection));

    int i = 0;
    // Loop over every line
    while(fgets(buffer, 256, f)) {
        // Every line is in format 'A->B'
        connections[i].from = buffer[0];
        connections[i].to = buffer[3];
        i++;
        // Reallocate connections array if the allocated memory is exceeded
        if (connections_max_length > i) {
            connections_max_length = connections_max_length*2;
            connections = realloc(connections, sizeof(Connection)*connections_max_length);
        }
    }
    *count = i;
    return connections;
}


int main(void){

	char ch;
	scanf("%c", &ch);

	int count;
	int i = 0;
	Connection *c = read_connections_file("tree1.txt", &count);

	rek(ch, c, i, count +1);
	return 0;

}

char rek(char ch, Connection *c, int i, int count){
	if(i < count && c[i].from == ch){
		printf("%c%c", c[i].from, c[i].to);
		rek(c[i].to, c, 0, count);
	}else if(i < count){
		rek(ch, c, i + 1, count);
	}else{
		printf("%c\n", c[i].to);
		return 0;
	}
}
