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


char** tokenify(const char *s,const char *token, int numToks) {
	//duplicate string for parsing
    char *str = strdup(s);

	//make space for the returned array of tokens
    char **ret = malloc((sizeof (char*))* (numToks+2));
    char *tok = strtok(str,token);
	
	//loop through tokens and place each one in
	//the array to be returned
    int index = 0;
    while (tok != NULL) {
        ret[index] = strdup(tok);
        tok = strtok(NULL,token);
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
int parseTokenSeq(char *token) {

	int numToks = 0;
	//count the number of spaces in the line
    for (int i=0;i<strlen(token);i++) {
        if(isspace(token[i]) != 0) {
            numToks++;
        }
    }

	char **arguments = tokenify(token," \t\n",numToks);

	if(strcmp(arguments[0],"exit")==0) {
		printf("Today was a good day...\n");
		free_tokens(arguments);
		exit(0);
	}

	if(strcmp(arguments[0],"mode")==0) {
		if(arguments[1][0]=='p') {
			return 1;
		}
		if(arguments[1][0]=='s') {
			return 0;
		}
		else {
			return 0;

		}
		free_tokens(arguments);

	}	

	//Fork current process
	pid_t pid = fork();
	
	if (pid<0) {
		printf("shell error: %s (%s)\n", strerror(errno),token);
		return 0;
	}
	
	if(pid==0) {
		if(execv(arguments[0],arguments)<0) {
			printf("shell error (execv): %s\n", strerror(errno));
			free_tokens(arguments);
			exit(0);
		}

	} 
	else {
		wait(NULL);
		printf("%s\n", "Parent process");
	}
	return 0;
}

int parseTokenPar(char *token) {

	int numToks = 0;
	//count the number of spaces in the line
    for (int i=0;i<strlen(token);i++) {
        if(isspace(token[i]) != 0) {
            numToks++;
        }
    }

	char **arguments = tokenify(token," \t\n",numToks);

	if(strcmp(arguments[0],"exit")==0) {
		printf("Today was a good day...\n");
		free_tokens(arguments);
		exit(0);
	}

	if(strcmp(arguments[0],"mode")==0) {
		if(arguments[1][0]=='p') {
			return 1;
		}
		if(arguments[1][0]=='s') {
			return 0;
		}
		else {
			return 1;
		}
		free_tokens(arguments);
	}	

	//Fork current process
	pid_t pid = fork();
	
	if (pid<0) {
		printf("shell error: %s (%s)\n", strerror(errno),token);
		return 1;
	}
	
	if(pid==0) {
		if(execv(arguments[0],arguments)<0) {
			printf("shell error (execv): %s\n", strerror(errno));
			free_tokens(arguments);
			exit(0);
		}

	} 
	else {
		printf("%s\n", "Parent process");
	}

	return 1;
}


int main(int argc, char **argv) {

    // i'm going to worry about structure later
	// for now this code just handles input in
	// a reasonable manner.
	
	size_t size = 0;
	char *line = NULL;
	int mode = 0;
	
	printf("ca$hmoneyballer$ (sequential): ");
	while(getline(&line,&size,stdin) != -1) {
        for (int i=0;i<strlen(line);i++) {
            if (line[i] == '#' /*|| line[i]=='\n'*/) {
                line[i] = '\0';
				break;
            }
		}
	    int numToks = 0;
	
		//count the number of spaces in the line
	    for (int i=0;i<strlen(line);i++) {
	        if(line[i] == ';') {
	            numToks++;
	        }
	    }
	
        char **tokens = tokenify(line,";",numToks);
        char *head = *tokens;
        int tempMode = mode;
		for(int i = 0; tokens[i] != NULL; i++){
			if(tokens[i][0]=='\n') {
				break;
			}
			if(mode==0) {
				tempMode = parseTokenSeq(tokens[i]);
			} 
			else {
				tempMode = parseTokenPar(tokens[i]);
			}
        }

        mode = tempMode;
		
		free_tokens(tokens);
		if(mode==0) {
			printf("ca$hmoneyballer$ (sequential): ");
		}
		else {
			printf("ca$hmoneyballer$ (paralell): ");
		}
	}
	
	free(line);

    return 0;
}

