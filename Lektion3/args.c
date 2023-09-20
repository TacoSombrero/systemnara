// Program to illustrate the getopt()
// function in C
  
#include <stdio.h> 
#include <unistd.h> 
#include "/usr/include/getopt.h"
  
int main(int argc, char *argv[]) 
{
	int opt;
	
	// put ':' in the starting of the
	// string so that program can 
	//distinguish between '?' and ':' 
	while((opt = getopt(argc, argv, "Bsf:")) != -1) 
	{ 
		switch(opt) 
		{ 
			case 'i': 
			case 'l': 
			case 'r': 
				printf("option: %c\n", opt); 
				break; 
			case 'f': 
				printf("filename: %s\n", optarg); 
				break; 
			case ':': 
				printf("option needs a value\n"); 
				break; 
		} 
	}
	
	return 0;
}