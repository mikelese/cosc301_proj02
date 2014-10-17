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
#include "list.h"

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

int parseConfig(FILE *input_file, node **PATH) {
	// Ripped from proj01 main.c
	// Initialize variables
    size_t size = 0;
    char *line = NULL;
	//PATH = NULL;
	int path_length = 0;

	//Go through all lines of input
    while(getline(&line,&size,input_file) != -1) {
    	
    	for(int i=0;i<strlen(line);i++) {
    		if(line[i]=='\n') {
    			line[i]='\0';
    			break;
    		}
    	}

    	struct stat statresult;
		int rv = stat(line, &statresult);
		if (rv < 0) {
    		printf("file '%s' is invalid\n", line);	
		}
		else {
			path_length++;
			//if(PATH != NULL){
			//	printf("%s\n", PATH -> val);
			//}
			//printf("(parse) Line: %s\n", line);
			listadd(PATH,line);
			//printf("(parse) Head: %s\n", PATH->val);
		}		
    }
    free(line);
    return path_length;
}

//this function handles each token
int parseToken(char **arguments, int mode, int tempMode, node *PATH) {

	if(strcmp(arguments[0],"exit")==0) {
		free_tokens(arguments);
		return -1;
	}

	if(strcasecmp(arguments[0],"mode")==0) {
		int i;
		for(i=0;arguments[i]!=NULL;i++) ;

		if(i>1) {

			if(!strncmp(arguments[1],"paralell",strlen(arguments[1])) && tempMode == 0) {
				free_tokens(arguments);
				return 1;
			}
			if(!strncmp(arguments[1],"sequential",strlen(arguments[1])) && tempMode == 1){
				free_tokens(arguments);
				return 0;
			}

		}
		free_tokens(arguments);
		return tempMode;

	}

	node *runner = PATH;

	while(runner != NULL) {
		struct stat statresult;
		char* concated = malloc((strlen(runner->val)+strlen(arguments[0])+2)*sizeof(char));
		strcpy(concated,runner->val);
		concated = strcat(concated,"/");
		concated = strcat(concated,arguments[0]);
		printf("%s\n", concated);
		int rv = stat(concated, &statresult);
		if (rv >= 0) {
			arguments[0] = concated;
			break;
		}		
		runner = runner->next;
		free(concated);
	}

	if(runner==NULL) {
		printf("shell error: command '%s' not found in shell-config directories\n", arguments[0]);
		free_tokens(arguments);
		return tempMode;
	}

	//Fork current process
	pid_t pid = fork();
	
	if (pid<0) {
		printf("shell error: %s (%s)\n", strerror(errno),*arguments);
		free_tokens(arguments);
		return tempMode;
	}
	
	if(pid==0) {
		if(execv(arguments[0],arguments)<0) {
			printf("shell error (execv): %s\n", strerror(errno));
			free_tokens(arguments);
			exit(0);
		}
	}
	
	else {
		if(mode == 0){
			wait(NULL);
		}
	}
	
	free_tokens(arguments);
	return tempMode;
}

int main(int argc, char **argv) {
	node *PATH = NULL;

	FILE *datafile = NULL;
    datafile = fopen("shell-config", "r");
	if (datafile == NULL) {
    	printf("Unable to open file %s: %s\n", "shell-config", strerror(errno));
   	 	exit(-1);
	}

	int num_paths = parseConfig(datafile,&PATH);

	listprint(PATH);

	if(!num_paths) {
		printf("Your shell-config contains no valid files.\n");
		printf("Please provide one: ");
		//TODO input
	}

	fclose(datafile);

	size_t size = 0;
	char *line = NULL;
	int mode = 0;
	
	printf("ca$hmoneyballer$ (sequential): ");
	while(getline(&line,&size,stdin) != -1) {
        for (int i=0;i<strlen(line);i++) {
            if (line[i] == '#') {
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
        //char *head = *tokens;
        int tempMode = mode;
		int didexit = 0;
		for(int i = 0; tokens[i] != NULL; i++){
			if(tokens[i][0]=='\n') {
				break;
			}
			int numToks = 0;
			char *token = tokens[i];
			//count the number of spaces in the line
		    for (int j=0;j<strlen(token);j++) {
		        if(isspace(token[j]) != 0) {
		            numToks++;
		        }
		    }

			char **arguments = tokenify(token," \t\n",numToks);
			
			tempMode = parseToken(arguments,mode,tempMode, PATH);

			//free_tokens(arguments);

			if(tempMode == -1){
				didexit = 1;
				tempMode = mode;
			}
			
        }

        
		if(mode){
			//printf("REACH PAR MODE, NUM TOKS: %d\n", numToks);
			for(int i = 0; i <= numToks; i++){
				//printf("REACH WAIT\n");
				wait(NULL);
			}
		}
		
		free_tokens(tokens);
		
		if(didexit){
			printf("Today was a good day...\n");
			free(line);
			listdestroy(PATH);
			exit(0);
		}
		
		if(!tempMode) {
			printf("ca$hmoneyballer$ (sequential): ");
		}
		else {
			printf("ca$hmoneyballer$ (paralell): ");
		}
		
		mode = tempMode;
	}
	
	free(line);

	listdestroy(PATH);

    return 0;
}

