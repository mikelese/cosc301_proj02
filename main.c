#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <signal.h>


char** tokenify(const char *s,const char token) {
	//duplicate string for parsing
    char *str = strdup(s);
    int numToks = 0;
	
	//count the number of spaces in the line
    for (int i=0;i<strlen(str);i++) {
        if(str[i] == token) {
            numToks++;
        }
    }
	
	//make space for the returned array of tokens
    char **ret = malloc((sizeof (char*))* (numToks+2));
    char *tok = strtok(str,&token);
	
	//loop through tokens and place each one in
	//the array to be returned
    int index = 0;
    while (tok != NULL) {
        ret[index] = strdup(tok);
        tok = strtok(NULL,&token);
        index++;
    }

	//null terminate the array, free the passed in str
	//and return the array
    ret[index] = NULL;
    free(str);
    return ret;
}

void free_tokens(char **tokens) {
	// free each string
	for(int i = 0; tokens[i] != NULL; i++){
        free(tokens[i]);
    }
	// then free the array
    free(tokens);
}

//this function handles each token
void parseToken(char *token) {
	//Fork current process
	pid_t pid = fork();
	
	if (pid<0) {
		printf("shell error: %s (%s)\n", strerror(errno),token);
		return;
	}
	
	if(pid==0) {
		char **arguments = tokenify(token,' ');
		if(execv(arguments[0],arguments)<0) {
			printf("shell error (execv): %s\n", strerror(errno));
			free_tokens(arguments);
			exit(0);
		}

	} else {
		wait(NULL);
		printf("%s\n", "Parent process");
	}
}

int main(int argc, char **argv) {

    // i'm going to worry about structure later
	// for now this code just handles input in
	// a reasonable manner.
	
	size_t size = 0;
	char *line = NULL;
	
	printf("proj02shell$ ");
	while(getline(&line,&size,stdin) != -1) {
        for (int i=0;i<strlen(line);i++) {
            if (line[i] == '#') {
                line[i] = '\0';
				break;
            }
            if(line[i]=='\n') {
            	line[i]='\0';
            }
		}
		
        char **tokens = tokenify(line,';');
        char *head = *tokens;
		for(int i = 0; tokens[i] != NULL; i++){
			parseToken(tokens[i]);
        }
		
		free_tokens(tokens);
		printf("proj02shell$ ");
	}
	
	free(line);

    return 0;
}

