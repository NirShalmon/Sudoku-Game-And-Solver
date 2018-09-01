/*This module handles user interface, i.e. reading commands from the user
 * and converting it into data usable by the other modules*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error_handler.h"
#include "parser.h"

#define DELIM " \t\r\n"


/*
 * Converts str to an integer.
 * If str is not a valid integer(I.e. it
 * contains non-digit characters), or not
 * in the range [0,10000],
 * this will return -1
 */
int string_to_int(char *str){
	int i,num = 0;
	for(i = 0; str[i]; ++i){
		num *= 10;
		if(str[i] < '0' || str[i] > '9'){
			return -1;
		}
		num += (str[i]-'0');
	}
	return num;
}

/*Fills *cmd with command data.
 * Assumes strtok was called once on cmd->rawInput,
 * and *sep is the first token
 */
void parseCommand(commandInfo *cmd,char* sep){
	char *commandName;
	int word_count=0; /*Counts the number of separate words read. We always consider only the first 4, as anything beyond that is irrelevant*/
	while(sep!=NULL && word_count<4){
		if(word_count==0){
			commandName=sep;
		}else
		{
			cmd->tokens[word_count-1]=sep;
		}
		word_count++;
		sep=strtok(NULL,DELIM);
	}
	if(!strcmp(commandName,"set") && word_count > 3) /* Make sure we have enough parameters */
	{
		cmd->commandName=set;
		cmd->args[0]=string_to_int(cmd->tokens[0])-1; /*if the token is not a valid integer, args[0] will be negative and will be treated as out of range in the executer*/
		cmd->args[1]=string_to_int(cmd->tokens[1])-1;
		cmd->args[2]=string_to_int(cmd->tokens[2]);
	}
	else if(!strcmp(commandName,"hint") && word_count > 2)/* Make sure we have enough parameters */
	{
		cmd->commandName=hint;
		cmd->args[0]=string_to_int(cmd->tokens[0])-1;
		cmd->args[1]=string_to_int(cmd->tokens[1])-1;
	}
	else if(!strcmp(commandName,"validate"))
	{
		cmd->commandName=validate;
	}
	else if(!strcmp(commandName,"exit"))
	{
		cmd->commandName=ex;
	}
	else if(!strcmp(commandName,"undo"))
	{
		cmd->commandName=undo;
	}
	else if(!strcmp(commandName,"redo"))
	{
		cmd->commandName=redo;
	}
	else if(!strcmp(commandName,"reset"))
	{
		cmd->commandName=reset;
	}
	else if(!strcmp(commandName,"mark_errors") && word_count>1)
	{
		cmd->commandName=m_errors;
		cmd->args[0]=string_to_int(cmd->tokens[0]);
	}
	else if(!strcmp(commandName,"print_board"))
	{
		cmd->commandName=p_board;
	}
	else if(!strcmp(commandName,"autofill"))
	{
			cmd->commandName=autofill;
	}
	else if(!strcmp(commandName,"num_solutions"))
	{
			cmd->commandName=num_solutions;
	}else if(!strcmp(commandName,"save") && word_count>1){
		cmd->commandName = save;
	}else if(!strcmp(commandName,"solve") && word_count>1){
		cmd->commandName = solve_command;
	}
	else if(!strcmp(commandName,"edit"))
	{
		cmd->commandName=edit_command;
		if(word_count==1)
			cmd->tokens[0]=NULL; /*If no filepath was given, indicate it by setting the argument to NULL*/
	}
	else if(!strcmp(commandName,"generate") && word_count>2)
	{
		cmd->commandName=generate;
		cmd->args[0] = string_to_int(cmd->tokens[0]);
		cmd->args[1] = string_to_int(cmd->tokens[1]);
	}
	else
	{
		cmd->commandName=invalid;
	}
	cmd->autofill_values=NULL;
}

/*Returns a commandInfo struct that contains the information needed to perform a command
 * (See commandInfo and func_name sturcts documentation in the header file)*/
commandInfo* readCommand()
{

	char *sep = NULL;
	commandInfo *cmd;
	cmd=(commandInfo*)calloc(1,sizeof(commandInfo));
	if(cmd == NULL) function_error(f_calloc);
	cmd->rawInput = (char*)calloc(MAX_INPUT_LEN+2,1);
	if(cmd->rawInput == NULL)function_error(f_calloc);
	while(sep == NULL){
		if(feof(stdin)){ /*reached EOF, exit*/
			cmd->commandName = ex;
			return cmd;
		}
		fgets(cmd->rawInput,MAX_INPUT_LEN+2,stdin);
		if(ferror(stdin)){
			function_error(f_fgets);
		}
		if(strlen(cmd->rawInput) == MAX_INPUT_LEN + 1){ /*line is too long*/
			while(strlen(cmd->rawInput) == MAX_INPUT_LEN + 1){ /*finish reading the line*/
				fgets(cmd->rawInput,MAX_INPUT_LEN+1,stdin);
				if(ferror(stdin)){
					function_error(f_fgets);
				}
			}
			cmd->commandName = invalid;
			return cmd; /*exit with invalid command*/
		}
		if(cmd->rawInput[0] != '\0'){ /*if we din't get an empty line*/
			sep=strtok(cmd->rawInput,DELIM); /*will check if there are any tokens in input*/
		}
	}
	parseCommand(cmd,sep);
	return cmd;
}

/*Prints prompt for the undo/redo commands
 * Set determines whether the action is undone (0) or redone(1)*/
void print_undo_redo_prompt(int x, int y, int old_val, int new_val, char set)
{
	if(set)
		printf("Redo ");
	else
		printf("Undo ");
	printf("%d,%d: from ",x,y);
	if(!old_val)
		printf("_ to ");
	else
		printf("%d to ",old_val);
	if(!new_val)
			printf("_\n");
		else
			printf("%d\n",new_val);
}

/*Frees the memory allocated to store a command info*/
void free_command(commandInfo *com)
{
	int **p;
	free(com->rawInput);
	p=com->autofill_values;
	if(p!=NULL)
	{
		for(;*p!=NULL;p++)
			free(*p);
		free(com->autofill_values);
	}
	free(com);
}
