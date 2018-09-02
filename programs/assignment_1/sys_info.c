#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int main(int argc, char* argv[] ){
	int pipefd[2];
	int p = pipe(pipefd);
	int f = fork();
	if (f < 0){
		write(2, "Error while creating process", 100);
	}
	// Parent process
	else if (f > 0){
		printf("Parent PID = %d \n",getpid());
		
		// Close read end
		close(pipefd[0]);
		// Write to pipe
		int i;
		for(i=1 ; i<argc ; i++){
			char c[50];
			strcpy(c,argv[i]);
			//c = "asdf";
			write(pipefd[1],c,strlen(c)+1);
		}
		close(pipefd[1]);
	}
	// Child process
	else {
		printf("Child PID = %d \n",getpid());
		// Close write end
		close(pipefd[1]);
		char output[100];
		// Read from pipe
		read(pipefd[0],output,100);
		close(pipefd[0]);
		// Echo command
		if(strstr(output,"echo")!=NULL){
			char *earg[] = {"echo", "Hello World!", NULL};
			execv(output,earg);	
		}
		// Execute the passed command
		execl(output,output,(char*) NULL);
	}
	return(0);

}

