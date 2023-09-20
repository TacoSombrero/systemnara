#include <stdio.h>
int fib(int n);

int main(void){
	int *x;
	scanf("%d", x);
	printf("Fib of is = %d\n", fib(*x));
	return 0;
}

int fib(int n){
	if(n == 0){
		return 0;
	}else if (n == 1){
		return 1;
	}else{
		return (fib(n-1) + fib(n-2));
	}
}
