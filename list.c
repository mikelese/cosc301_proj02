#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

/* your list function definitions */

void listadd(node **head, char *i, char* command) {
	//create new node for entry
	node *n = malloc(sizeof(node));
	char *s = strdup(i);
	if(command != NULL){
		char *c = strdup(command);
		n->command = c;
	}
	else{
		n->command = NULL;
	}
	n->val = s;
	n->next = NULL;

	//if the list is empty make the new node the head
	if(*head == NULL){
		*head = n;
		return;
	}
	
	//initialize variables for running through the list
	node *runner = *head;
		
	while(runner->next != NULL) {
		runner = runner->next;
	}
	runner->next = n;
	//printf("Prior: %s Added: %s\n",runner->val,runner->next->val);
}

int listdelete(const char *name, struct node **head) {
	struct node *previous = *head;
	struct node *current = previous->next;
	
	if(!strcasecmp(name,previous->val)){
		if(previous->command !=NULL){
			free(previous->command);
		}
		free(previous->val);
		free(previous);
		*head = current;
		return 1;
	}
	
	while(current != NULL){
		if(!strcasecmp(name,current->val)){
			previous->next = current->next;
			if(current->command !=NULL){
				free(current->command);
			}
			free(current->val);
			free(current);
			return 1;
		}
		previous = current;
		current = current->next;
	}
	return 0;
}

void listdestroy(node *list) {
	// Iterate through list and free all nodes
    while (list != NULL) {
        node *tmp = list;
        list = list->next;
		if(tmp->command !=NULL){
			free(tmp->command);
		}
		free(tmp->val);
        free(tmp);
    }
}

void listprint(node *list) {
	printf("*** List Contents Begin ***\n");
	// Iterate through list and print all nodes
    while (list != NULL) {
		//printf("here");
        printf("%s\n", list->val);
        list = list->next;
    }
	printf("*** List Contents End ***\n");
}















