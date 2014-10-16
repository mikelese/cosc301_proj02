#include <stdio.h>
#include <stdlib.h>
#include "list.h"

/* your list function definitions */

void listadd(node **head, char *i) {
	//create new node for entry
	node *n = malloc(sizeof(node));
	n->val = i;
	n->next = NULL;

	//if the list is empty make the new node the head
	if(*head == NULL){
		printf("%s\n", "no head exists");
		printf("Adding %s to head\n",i);
		*head = n;
		return;
	}
	
	//initialize variables for running through the list
	node *runner = *head;
		
	while(runner->next != NULL) {
		runner = runner->next;
	}
	printf("Adding %s to list after %s\n",i,runner->val);
	runner->next = n;
}

void listdestroy(node *list) {
	// Iterate through list and free all nodes
    while (list != NULL) {
        node *tmp = list;
        list = list->next;
        free(tmp);
    }
}

void listprint(node *list) {
	printf("*** List Contents Begin ***\n");
	// Iterate through list and print all nodes
    while (list != NULL) {
        printf("%s\n", list->val);
        list = list->next;
    }
	printf("*** List Contents End ***\n");
}















