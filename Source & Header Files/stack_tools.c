/*This module implements the stack data structure and handles
 * all of its operations*/

#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "error_handler.h"
#include "stack_tools.h"

/*Initializes an empty stack and returns it*/
Stack create_stack()
{
	Stack stk;
	stk.size=0;
	stk.top=NULL;
	return stk;
}
/*Pushes info on top of the stack stk*/
void push(Stack *stk, void *info)
{
	stackNode *new=(stackNode*)malloc(sizeof(stackNode));
	if(new==NULL)
		function_error(f_malloc);

	new->info=info;
	new->next=stk->top;

	stk->top=new; /*Setting the new node to be the top of the stack*/
	stk->size++; /*Incrementing the stack size*/
}

/*Returns the element on top of the stack
  If the stack is empty, returns NULL*/
void *pop(Stack *stk)
{
	void *res;
	stackNode *ctop;

	if(!stk->size)
	{
		return NULL;
	}
	ctop=stk->top;
	res=ctop->info; /*Get the info*/
	stk->top=ctop->next; /*Set the next element to be the top one*/
	free(ctop); /*Free the stack node of the popped element*/
	stk->size--; /*Decrement stack size*/
	return res;
}

/*Used to empty ONLY stacks containing commandInfo structs (i.e, undo/redo stacks)*/
void empty_stack(Stack *stk)
{
	commandInfo *com;
	while(stk->size){
			com=(commandInfo*)pop(stk);
			free_command(com);
		}
}
