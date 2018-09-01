/*This module implements the exhaustive backtracking algorithm
 * for the num_solutions command*/

#include <stdlib.h>
#include <stdio.h>

#include "board.h"
#include "stack_tools.h"
#include "error_handler.h"

/*A struct required for the exhaustive backtracking algorithm
 * Holds all the info required to execute a recursion step*/
typedef struct recursion_info{
	int x,y;
	int *valid_values;
	int count;
}recursion_info;

/*Calculates the x coordinate of the next cell in the defined order*/
int next_x(game_board* board,int x){
	if(x == board_len(board)-1){
		return 0;
	}
	return x+1;
}

/*Calculates the y coordinate of the next cell in the defined order*/
int next_y(game_board *board,int x,int y){
	if(x == board_len(board)-1){
		return y+1;
	}
	return y;
}

int optionsCnt; /*Stores the number of possible solutions*/

/*
 * The exhaustive backtracking algorithm itself
 * Each step is explained inside the code
 */
void exhaustive_solve(game_board *board){
	int *valid_values=NULL; /*An array for the valid values of a cell*/
	int i=0; /*Currently examined valid value's index*/
	int cur_x=0,cur_y=0; /*Currently examined cell coordinates*/
	Stack rec_stack; /*A stack with the info of the recursion steps*/
	recursion_info *rec_info; /*Each step's info is kept here*/

	rec_stack=create_stack();

	rec_start:
	if(cur_y==board_len(board)){ /*We reached a cell out of the board, thus all previous cells are filled with legal values*/
		++optionsCnt; /*Increment option count*/

		/*Extract info about the previous recursion step and copy it to the current step info*/
		rec_info=(recursion_info*)pop(&rec_stack);
		if(rec_info==NULL)
			return;
		cur_x=rec_info->x;
		cur_y=rec_info->y;
		valid_values=rec_info->valid_values;
		i=rec_info->count;

		/*Free memory and start where the step we extracted left off*/
		free(rec_info);
		goto rec_start;
	}

	if(board->cells[cur_x][cur_y].is_fixed)
	{
		/*Skip fixed cells, no need to handle them or push into stack*/
		cur_x=next_x(board,cur_x);
		cur_y=next_y(board,cur_x,cur_y);
		i=0;
		valid_values=NULL;
		goto rec_start;
	}
	if(valid_values==NULL){
	/*Find valid values only if we're dealing with a new recursion step
	 * A new step is indicated by valid_values==NULL*/
	 valid_values=get_valid_values(board,cur_x,cur_y);
	 i=0;
	}
	if(valid_values[i])
	{
		/*Set cell to the i'th valid value, increment i*/
		set_cell(board,cur_x,cur_y,valid_values[i]);
		++i;

		/*Save the recursion step info into the stack*/
		rec_info=(recursion_info*)malloc(sizeof(recursion_info));
		if(rec_info==NULL)
			function_error(f_malloc);
		rec_info->count=i;
		rec_info->valid_values=valid_values;
		rec_info->x=cur_x;
		rec_info->y=cur_y;
		push(&rec_stack,rec_info);

		/*Move on to the next cell*/
		cur_x=next_x(board,cur_x);
		cur_y=next_y(board,cur_x,cur_y);
		i=0;
		valid_values=NULL;
		goto rec_start;

	}
	else
	{
		/*Valid values exhausted, free memory and set cell back to empty*/
		free(valid_values);
		set_cell(board,cur_x,cur_y,0);

		/*Go back to the previous recursion step, if there are none we are done*/
		rec_info=(recursion_info*)pop(&rec_stack);
		if(rec_info==NULL)
			return;
		cur_x=rec_info->x;
		cur_y=rec_info->y;
		i=rec_info->count;
		valid_values=rec_info->valid_values;
		free(rec_info);
		goto rec_start;
	}


}

/*Runs the exhaustive backtracking algorithm on the board and
 * returns the number of solutions*/
int count_solutions(game_board *board){
	optionsCnt = 0;
	exhaustive_solve(board);
	return optionsCnt;
}


