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

/*
 * Jack Sneeringer and Mike Lese
 *     COSC 301 Project 2
 *
 *
 * Stage 1: Jack poorly copied over old tokenizing code and did mode 
 * switching. Mike did pretty much everything else.
 *
 *
 * Stage 2: Jack wrote the code for handling background processes, pause, 
 * resume, and jobs while Mike wrote the PATH code, did error handling, 
 * and tracked down memory leaks.
*/

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
			listadd(PATH,line,NULL);
		}		
    }
    free(line);
    return path_length;
}

//this function handles each token
int parseToken(char **arguments, int mode, int tempMode, node *PATH, node **CHILDREN) {

	if(strcmp(arguments[0],"exit")==0) {
		return -1;
	}

	if(strcasecmp(arguments[0],"mode")==0) {
		int i;
		for(i=0;arguments[i]!=NULL;i++) ;

		if(i>1) {

			if(!strncmp(arguments[1],"paralell",strlen(arguments[1])) && tempMode == 0) {
				return 1;
			}
			if(!strncmp(arguments[1],"sequential",strlen(arguments[1])) && tempMode == 1){
				return 0;
			}
		}
		
		return tempMode;
	}	
	
	node *runner = PATH;

	while(runner != NULL) {
		struct stat statresult;
		char* concated = malloc((strlen(runner->val)+strlen(arguments[0])+2)*sizeof(char));
		strcpy(concated,runner->val);
		concated = strcat(concated,"/");
		concated = strcat(concated,arguments[0]);
		int rv = stat(concated, &statresult);
		if (rv >= 0) {
			free(arguments[0]);
			arguments[0] = concated;
			break;
		}		
		runner = runner->next;
	}

	if(runner==NULL) {
		struct stat statresult;

		int rv = stat(arguments[0], &statresult);
		if (rv < 0) {
			printf("shell error: command '%s' not found in shell-config directories\n", arguments[0]);
			return tempMode;
		}
	}

	//Fork current process
	pid_t pid = fork();
	
	if (pid<0) {
		printf("shell error: %s (%s)\n", strerror(errno),*arguments);
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

		char *str = malloc(6*sizeof(char));
		snprintf(str, 6, "%d", pid);
		
		if(mode) {
			listadd(CHILDREN,str,arguments[0]);
		}
		free(str);
		if(mode == 0){
			wait(NULL);
		}
	}
	
	return tempMode;
}

void pauseresume(char **arguments){
	
	int killstatus;
	pid_t pid = (pid_t)atoi(arguments[1]);
	
	if(!strcmp(arguments[0],"pause")){
		killstatus = kill(pid, SIGSTOP);
		if(killstatus) {
			printf("shell error: %s\n",strerror(errno));
		}
	}
	else{
		killstatus = kill(pid, SIGCONT);
		if(killstatus) {
			printf("shell error: %s\n",strerror(errno));
		}
	}
	return;
}

void jobs(node **CHILDREN){
	
	node *runner = *CHILDREN;
	char* currStatus;
	char* paused = "Paused";
	char* running = "Running";
	int status;
	while(runner != NULL){
		status = 0;
		pid_t temp = (pid_t)atoi(runner->val);
		waitpid(temp, &status, WNOHANG | WUNTRACED);
		//printf("PID: %d, Status: %d, STOPPED:%d, STOPPER: %d\n",(int)temp, status, WIFSTOPPED(status), WSTOPSIG(status));
		if(WIFSTOPPED(status)){
			currStatus = paused;
		}
		else{
			currStatus = running;
		}
		printf("PID: %d, Command: %s, Status: %s\n",(int)temp,runner->command,currStatus);
		runner = runner->next;
	}
}

int input(int mode, node *PATH, node **CHILDREN){
	
	size_t size = 0;
	char *line = NULL;

	
	if(getline(&line,&size,stdin) != -1) {
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
			
			if(!strcmp(arguments[0],"pause") || !strcmp(arguments[0],"resume")){
				if(numToks == 2){
					pauseresume(arguments);
				}
				else{
					printf("shell error: incorrect arguments to %s\n",arguments[0]);
				}
			}
			else if(!strcmp(arguments[0],"jobs")){
				if(numToks == 1){
					jobs(CHILDREN);
				}
				else{
					printf("shell error: incorrect arguments to jobs\n");
				}
			}
			else{
				tempMode = parseToken(arguments,mode,tempMode,PATH,CHILDREN);
			}
			
			free_tokens(arguments);
			
			if(tempMode == -1){
				didexit = 1;
				tempMode = mode;
			}
        }
		
		free_tokens(tokens);
		
		if(didexit && !mode){
			printf("Today was a good day...\n");
			free(line);
			listdestroy(PATH);
			exit(0);
		}
		if(didexit && mode){
			if(*CHILDREN == NULL){
				listdestroy(PATH);
				free(line);
				exit(0);
			}
			else{
				printf("processes still running, cannot exit\n");
			}
		}

		mode = tempMode;
		
		if(mode){
			printf("ca$hmoneyballer$ (parallel): ");
			fflush(stdout);
		}
	}
	
	free(line);
	
	return mode;
	
}

int main(int argc, char **argv) {
	node *PATH = NULL;
	node *CHILDREN = NULL;

	FILE *datafile = NULL;
    datafile = fopen("shell-config", "r");
	if (datafile == NULL) {
    	printf("Unable to open file %s: %s\n", "shell-config", strerror(errno));
   	 	exit(-1);
	}

	int num_paths = parseConfig(datafile,&PATH);

	if (!num_paths) {
		printf("No valid shell-config files were provided! You must use full path names!\n");
	}

	fclose(datafile);
	
	int mode = 0;
	int tempMode = 1;
	while(1){
		if(!mode){
			printf("ca$hmoneyballer$ (sequential): ");
			mode = input(mode, PATH, &CHILDREN);
			if(feof(stdin)){
				printf("\n");
				break;
			}
		}
		else{
			struct pollfd pfd[1];
			pfd[0].fd = 0; // stdin is file descriptor 0
			pfd[0].events = POLLIN;
			pfd[0].revents = 0;

			int rv = poll(&pfd[0], 1, 1000);

			if (rv == 0) {
				pid_t wait_rv = waitpid(-1,NULL,WNOHANG);
				if(wait_rv != -1 && wait_rv != 0){
					char *str = malloc(6*sizeof(char));
					snprintf(str, 6, "%d", wait_rv);

					if (!listdelete(str,&CHILDREN)) {
						listdestroy(CHILDREN);
						printf("shell error: pid %s not found. Exiting...",str);
						return -1;
					}
					free(str);
					printf("Process: %d has ended.\n", (int)wait_rv);
					if(tempMode){
						printf("ca$hmoneyballer$ (parallel): ");
						fflush(stdout);
					}
				}  
			} 
			else if (rv > 0 && tempMode) {
				tempMode = input(mode, PATH, &CHILDREN);
			}
			else {
				printf("there was some kind of error: %s\n", strerror(errno));
				return -1;
			}	
			
			if(!tempMode && CHILDREN == NULL){
				mode = 0;
				tempMode = 1;
			}
		}
		
	
	}

    return 0;
}