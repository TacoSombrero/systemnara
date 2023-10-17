#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define LOC_MAXLEN 13

void check_str_ending(char *str);

int main(void)
{
	char *str1 = "path/to/directory/";
	size_t requiredSize = strlen(str1) + 1;
	char *str = (char*) malloc(requiredSize);
	str = strcpy(str, str1);
	check_str_ending(str);
	fprintf(stderr, "%s\n", str);
	/*char dest[LOC_MAXLEN];
	snprintf(dest, LOC_MAXLEN, "%s%s", "abc", "def");

	printf("%s\n", dest);

	/* append new string using length of previously added string
	snprintf(dest + strlen(dest), LOC_MAXLEN - strlen(dest), "%s", "ghi");
	printf("%s\n", dest);

	/* repeat that
	snprintf(dest + strlen(dest), LOC_MAXLEN - strlen(dest), "%s", "jkl");
	printf("%s\n", dest);*/

	return 0;
}

void check_str_ending(char *str)
{
	int len = strlen(str);
	if(str[len - 1] != '/'){
		fprintf(stderr, "did it\n");
		str = strcat(str, "/");
	}
	
}
