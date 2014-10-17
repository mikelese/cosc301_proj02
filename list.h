#ifndef __LIST_H__
#define __LIST_H__

// Define node
typedef struct node {
	char *val;
	char *command;
	struct node *next;
} node;

// Our node class has 4 functions, an add that places an item in
// the correct location to keep list sorted, a list destroy, 
// a list delete, and a list print
void listadd(node **head, char *i, char *command);
int listdelete(const char *name, struct node **head);
void listdestroy(node *head);
void listprint(node *head);


#endif // __LIST_H__
