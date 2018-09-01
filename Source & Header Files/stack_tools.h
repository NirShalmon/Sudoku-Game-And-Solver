/*This module implements the stack data structure and handles
 * all of its operations*/

/*A sturct for a stack element, contains info and next element*/
typedef struct stackNode{
	void *info;
	struct stackNode *next;
} stackNode;

 /*A stack struct, holds the top element and the number of elements*/
typedef struct Stack{
	stackNode *top;
	int size;
} Stack;






Stack create_stack(); /*Initalizes an empty stack and returns it*/
void push(Stack *stk, void *info); /*Pushes info on top of the stack stk*/
void *pop(Stack *stk);/*Returns the element on top of the stack. If the stack is empty, returns NULL*/
void empty_stack(Stack *stk); /*Used to empty ONLY stacks containing commandInfo structs (i.e, undo/redo stacks)*/
