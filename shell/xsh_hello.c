#include <xinu.h>
#include <string.h>
#include <stdio.h>

/* Say Hello World!!*/


shellcmd xsh_hello(int nargs, char *args[]){
	
	if (nargs > 2) {
                fprintf(stderr, "%s: too many arguments\n", args[0]);
                fprintf(stderr, "Try '%s --help' for more information\n",
                        args[0]);
                return 1;
        }
	
	
		printf("Hello %s, Welcome to the world of Xinu!!",args[1]);
	return 0;			
}

