#include "mypopen.h"


int main(){
	FILE *output = mypopen("ls -l", "r");
	if(output == NULL){
		fprintf(stderr, "incorrect parameters or too many files.\n");
		return EXIT_FAILURE;
	}

	int c;
	while((c = getc(output)) != EOF){
		putchar(c);
	}

	if(pclose(output) != 0){
		fprintf(stderr, "Could not run more or other error.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
} // end main


