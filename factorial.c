#include <stdio.h>
#include <stdlib.h>

unsigned long factorial(int input) 
{
	if(input == 1 || input == 0)
		return 1 ;
	return input*factorial(input - 1) ;
}


int main(int argc, char *argv []) 
{
	unsigned long  res ;
	int input ;
	scanf("%d", &input ) ;
	res = factorial(input) ;

	printf("%lu\n",res) ;	

}
