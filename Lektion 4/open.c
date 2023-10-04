#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
bool str_ending(char *str);
int main(){
	DIR *dir = opendir("./");
	struct dirent *pDirent;
	if(dir == NULL){
		exit(EXIT_FAILURE);
	}

	while((pDirent = readdir(dir)) != NULL){
		if(str_ending(pDirent->d_name)){
			if(pDirent->d_type == 4){
				printf("DIRECTORY: [%s]\n", pDirent->d_name);
			}else{
				printf("FILE: [%s]\n", pDirent->d_name);
			}
			
		}
	}

}


bool str_ending(char *str){
	int len = strlen(str);
	if(str[len - 1] == 'c' && str[len -2] == '.'){
		return true;
	}
	return false;
}