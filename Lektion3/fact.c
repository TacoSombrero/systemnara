#include <stdio.h>
int fact(int n);

int main(void){
	int *x;
	scanf("%d", x);
	printf("n! = %d\n", fact(*x));
	return 0;
}

int fact(int n){
	if(n > 0){
		return (n * fact(n-1));
	}else{
		return 1;
	}
}