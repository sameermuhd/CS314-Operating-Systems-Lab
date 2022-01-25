#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
void printString(char *string, int lenOfString, int iterator){
	if(iterator < lenOfString){
		printf("%c %d\n", string[iterator], (int)getpid());
		sleep(rand()%4 + 1);
		int ID = fork();
		if(ID > 0){
			int cpid = wait(NULL);
			exit(0);
		}
		else{ printString(string, lenOfString, iterator + 1); }
	}
	exit(0);
}
int main(){
	char *string = "Hello World";
	int lenOfString = 11;
	printString(string, lenOfString, 0);
	return 0;
}
