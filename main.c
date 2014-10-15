#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <poll.h>
#include <signal.h>


char** tokenify(const char *s) {
	//duplicate string for parsing
    char *str = strdup(s);
    int numSemiColon = 0;
	
	//count the number of spaces in the line
    for (int i=0;i<strlen(str);i++) {
        if(str[i] == ';') {
            numSemiColon++;
        }
    }
	
	//make space for the returned array of tokens
    char **ret = malloc((sizeof (char*))* (numSemiColon+2));
    char *tok = strtok(str,";");
	
	//loop through tokens and place each one in
	//the array to be returned
    int index = 0;
    while (tok != NULL) {
        ret[index] = strdup(tok);
        tok = strtok(NULL,";");
        index++;
    }

	//null terminate the array, free the passed in str
	//and return the array
    ret[index] = NULL;
    free(str);
    return ret;
}

void parseToken(char *token) {
	//this function handles each token
	printf("%s\n",token);
}

void free_tokens(char **tokens) {
	// free each string
	for(int i = 0; tokens[i] != NULL; i++){
        free(tokens[i]);
    }
	// then free the array
    free(tokens);
}

int main(int argc, char **argv) {

    // i'm going to worry about structure later
	// for now this code just handles input in
	// a reasonable manner.
	
	size_t size = 0;
	char *line = NULL;
	
	while(getline(&line,&size,stdin) != -1) {
        for (int i=0;i<strlen(line);i++) {
            if (line[i] == '#') {
                line[i] = '\0';
				break;
            }
		}
		
        char **tokens = tokenify(line);
        char *head = *tokens;
		for(int i = 0; tokens[i] != NULL; i++){
			parseToken(tokens[i]);
        }
		
		free_tokens(tokens);
	}
	
	free(line);

    return 0;
}

