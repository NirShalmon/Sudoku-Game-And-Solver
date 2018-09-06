/*This module handles the execution of user commands*/

#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "stack_tools.h"
#include "exhaustive_solver.h"
#include "error_handler.h"
#include "executer.h"
#include "file_operations.h"
#include "board.h"
#include "ILPsolver.h"

#define INIT_C (game->state==init)
#define EDIT_C (game->state==edit)
#define SOLVE_C (game->state==solve)
#define UNDO 0
#define REDO 1
#define FIRST_TIME 2
#define ON_RESET 3
#define RANGE(x) (x>=0 && x<=board_len(&game->board)-1)
#define DEFAULT_SIZE 3
#define UNSOLVABLE (!sol.block_rows)
#define MAX_GENERATE_ATTEMPTS 1000


/*Returns 1 if the command is considered valid in the current game mode,
 * otherwise returns 0*/
char check_mode_compatibility(game_data *game,func_name com)
{
	switch(com)
	{
	case set:
	case validate:
	case reset:
	case undo:
	case p_board:
	case redo:
	case save:
	case num_solutions:return EDIT_C || SOLVE_C;
	case hint:
	case m_errors:
	case autofill:return SOLVE_C;
	case ex:
	case invalid:
	case solve_command:
	case edit_command: return 1;
	case generate: return EDIT_C;
	}


	return 0;
}
/*Returns a 2d array that represent the values to be autofilled
 * Such that, for every arr[3] in the stack, it holds that:
 * arr[0]=x coordinate
 * arr[1]=y coordinate
 * arr[2]=value to fill
 * NULL pointer indicates the end of the list*/
int **get_autofill_cells(game_board *board)
{
	Stack stk;
	int *arr;
	int *valid_values;
	int x,y,amount,i;
	int **res;

	x=board_len(board)-1;
	y=board_len(board)-1;
	stk=create_stack();
	/*The cells are inserted into the stack in reverse order so they're popped
	 * out in the right order*/
	while(y>=0)
	{
		if(!board->cells[x][y].is_fixed && board->cells[x][y].value == 0){
			valid_values=get_valid_values(board,x,y);
			if(valid_values[0] && !valid_values[1])
			{
				arr=(int*)malloc(sizeof(int)*3);
				if(arr==NULL)
					function_error(f_malloc);
				arr[0]=x;
				arr[1]=y;
				arr[2]=valid_values[0];
				push(&stk,arr);
			}
			free(valid_values);
		}
		/*Advancing to the next cell*/
		if(x==0){
			x=board_len(board)-1;
			y--;
		}
		else{
			x--;
		}
	}

	amount=stk.size;
	if(!amount)
		return NULL;
	res=(int**)malloc(sizeof(int*)*(amount+1));
	if(res==NULL)function_error(f_malloc);

	for(i=0;i<amount;i++)
		res[i]=(int*)pop(&stk);
	res[amount]=NULL; /*Indicates the end of list*/

	return res;
}

/*Fills the board with the values given by the 2d array
 * mode determines whether the autofill action is:
 * 0 - Being undone
 * 1 - Being redone
 * 2 - Being performed for the first time
 * else - Being performed as part of a reset command (shouldn't print anything)
 *
 * Returns 1 if changes were made (i.e. at least one cell was changed), otherwise returns 0*/

char autofill_board(game_board *board,int **fills, char mode)
{
	if(fills!=NULL){
	for(;*fills!=NULL;fills++){
		set_cell(board,(*fills)[0],(*fills)[1],(mode==REDO || mode==FIRST_TIME)?(*fills)[2]:0);
		switch(mode){
		case UNDO:
			print_undo_redo_prompt((*fills)[0]+1,(*fills)[1]+1,(*fills)[2],0,UNDO);
			break;
		case REDO:
			print_undo_redo_prompt((*fills)[0]+1,(*fills)[1]+1,0,(*fills)[2],REDO);
			break;
		case FIRST_TIME:
			printf("Cell <%d,%d> set to %d\n",(*fills)[0]+1,(*fills)[1]+1,(*fills)[2]);
			break;
		}
	}
	return 1;
	}
	else
		return 0;

}

/*Frees the game_data struct elements*/
void free_game_data(game_data *game){
	if(game->state == init) return;
	free_board(&game->board);
	empty_stack(&game->undo_stack);
	empty_stack(&game->redo_stack);
}

void execute_set(game_data *game,commandInfo *com){
	if(!RANGE(com->args[0]) || !RANGE(com->args[1]) || !(RANGE(com->args[2]) || com->args[2]==board_len(&game->board))){
		printf("Error: value not in range 0-%d\n",board_len(&game->board));
		free_command(com);
		return;
	}
	if(game->state == solve && game->board.cells[com->args[0]][com->args[1]].is_fixed){
		printf("Error: cell is fixed\n");
		free_command(com);
	}
	else{
		com->prev_value=game->board.cells[com->args[0]][com->args[1]].value; /*Sets the current value of the cell to be the previous, in case this command is undone*/
		set_cell(&game->board,com->args[0],com->args[1],com->args[2]);
		print_board(&game->board,game->state == solve,game->mark_errors);
		push(&game->undo_stack,com);
		empty_stack(&game->redo_stack); /*After a set command, no moves can be redone*/
		if(game->state==solve){
			if(!game->board.empty_cells && !game->board.errors){
				printf("Puzzle solved successfully\n");
				free_game_data(game);
				game->state=init;
			}
			if(!game->board.empty_cells && game->board.errors){
				printf("Puzzle solution erroneous\n");
			}
		}
	}
}

void execute_undo(game_data *game, commandInfo *com){
	commandInfo *undo_move;
	undo_move=(commandInfo*)pop(&game->undo_stack);
	if(undo_move==NULL){
		printf("Error: no moves to undo\n");
		free_command(com);
		return;
	}
	if(undo_move->commandName==set){
		set_cell(&game->board,undo_move->args[0],undo_move->args[1],undo_move->prev_value);
		print_board(&game->board,game->state == solve,game->mark_errors);
		print_undo_redo_prompt(undo_move->args[0]+1,undo_move->args[1]+1,undo_move->args[2],undo_move->prev_value,UNDO);
	}
	else if(undo_move->commandName==autofill || undo_move->commandName==generate){
		autofill_board(&game->board,undo_move->autofill_values,UNDO);
		print_board(&game->board,game->state == solve,game->mark_errors);
	}
	push(&game->redo_stack,undo_move);
	free_command(com);
}

void execute_redo(game_data *game, commandInfo *com){
	commandInfo *redo_move;
	redo_move=(commandInfo*)pop(&game->redo_stack);
	if(redo_move==NULL){
		printf("Error: no moves to redo\n");
		free_command(com);
		return;
	}
	if(redo_move->commandName==set){
		set_cell(&game->board,redo_move->args[0],redo_move->args[1],redo_move->args[2]);
		print_board(&game->board,game->state == solve,game->mark_errors);
		print_undo_redo_prompt(redo_move->args[0]+1,redo_move->args[1]+1,redo_move->prev_value,redo_move->args[2],REDO);
	}
	else if(redo_move->commandName==autofill || redo_move->commandName==generate){
		autofill_board(&game->board,redo_move->autofill_values,REDO);
		print_board(&game->board,game->state == solve,game->mark_errors);
	}
	push(&game->undo_stack,redo_move);
	free_command(com);
}

void execute_reset(game_data *game, commandInfo *com){
	commandInfo *undo_move;
	empty_stack(&game->redo_stack);
	while(game->undo_stack.size){
		undo_move=pop(&game->undo_stack);
		if(undo_move->commandName==set)
			set_cell(&game->board,undo_move->args[0],undo_move->args[1],undo_move->prev_value);
		else if(undo_move->commandName==autofill || undo_move->commandName == generate)
			autofill_board(&game->board,undo_move->autofill_values,ON_RESET);
		free_command(undo_move);

	}
	printf("Board reset\n");
	free_command(com);
}

void execute_mark_errors(game_data *game,commandInfo *com){
	if(com->args[0]!=0 && com->args[0]!=1){
		printf("Error: the value should be 0 or 1\n");
		free_command(com);
		return;
	}
	game->mark_errors=(char)com->args[0];
	free_command(com);
}

void execute_print_board(game_data *game, commandInfo *com){
	print_board(&game->board,game->state == solve,game->mark_errors);
	free_command(com);
}

void execute_autofill(game_data *game, commandInfo *com){
	char changed; /*Indicates whether the autofill has made any changes to the board*/
	if(!game->board.errors){
		com->autofill_values=get_autofill_cells(&game->board);
		changed=autofill_board(&game->board,com->autofill_values,FIRST_TIME);
		if(!changed){
			free_command(com);
			print_board(&game->board,game->state == solve,game->mark_errors);
			return;
		}
		push(&game->undo_stack,com);
		print_board(&game->board,game->state == solve,game->mark_errors);
		if(!game->board.empty_cells && !game->board.errors){
			printf("Puzzle solved successfully\n");
			free_game_data(game);
			game->state=init;
			}
		empty_stack(&game->redo_stack);
	}else{
		printf("Error: board contains erroneous values\n");
		free_command(com);
	}
}

void execute_num_solutions(game_data *game, commandInfo *com){
	int sol_num;
	game_board temp;
	if(game->board.errors){
		printf("Error: board contains erroneous values\n");
	}
	else{
		temp=create_board(game->board.block_rows,game->board.block_columns);
		copy_board(&game->board,&temp);
		fix_all_cells(&temp);
		sol_num=count_solutions(&temp);
		printf("Number of solutions: %d\n",sol_num);
		if(sol_num==1)
			printf("This is a good board!\n");
		else if(sol_num!=0)
			printf("The puzzle has more than 1 solution, try to edit it further\n");
		free_board(&temp);
	}
	free_command(com);
}

void execute_exit(game_data *game, commandInfo *com){
	printf("Exiting...\n");
	free_game_data(game);
	free_command(com);
	exit(0);
}

void execute_invalid(commandInfo *com){
	printf("ERROR: invalid command\n");
	free_command(com);
}

void execute_save(game_data *game,commandInfo *com){
	game_board sol;
	if(game->state == edit && game->board.errors){
		printf("Error: board contains erroneous values\n");
		free_command(com);
		return;
	}
	sol=find_solution(&game->board);
	if(game-> state == edit && UNSOLVABLE){
		printf("Error: board validation failed\n");
	}else{
		if(save_board(&game->board,com->tokens[0],game->state == edit)){
			printf("Saved to: %s\n",com->tokens[0]);
		}else{
			printf("Error: File cannot be created or modified\n");
		}
	}
	if(!UNSOLVABLE){
		free_board(&sol);
	}
	free_command(com);
}

void execute_solve(game_data *game,commandInfo *com){
	game_board board = load_board(com->tokens[0]);
	if(board.block_columns == 0){
		printf("Error: File doesn't exist or cannot be opened\n");
	}else{
		free_game_data(game);
		game->board = board;
		game->state = solve;
		print_board(&game->board,1,game->mark_errors);
	}
	free_command(com);
}

void execute_edit(game_data *game, commandInfo *com){
	game_board board;
	if(com->tokens[0] != NULL){
		board = load_board(com->tokens[0]);
	}else{
		board = create_board(DEFAULT_SIZE,DEFAULT_SIZE);
	}
	if(board.block_columns == 0){
		printf("Error: File cannot be opened\n");
	}else{
		free_game_data(game);
		game->board = board;
		game->state = edit;
		print_board(&game->board,0,game->mark_errors);
	}
	free_command(com);
}

void execute_hint(game_data *game, commandInfo *com)
{
	game_board sol;
	if(!RANGE(com->args[0]) || !RANGE(com->args[1]))
		printf("Error: value not in range 1-%d\n",board_len(&game->board));
	else if(game->board.errors)
		printf("Error: board contains erroneous values\n");
	else if(game->board.cells[com->args[0]][com->args[1]].is_fixed)
		printf("Error: cell is fixed\n");
	else if(game->board.cells[com->args[0]][com->args[1]].value)
		printf("Error: cell already contains a value\n");
	else
	{
		sol=find_solution(&game->board);
		if(UNSOLVABLE)
			printf("Error: board is unsolvable\n");
		else{
			printf("Hint: set cell to %d\n",sol.cells[com->args[0]][com->args[1]].value);
			free_board(&sol);
		}
	}
	free_command(com);

}



void execute_validate(game_data *game, commandInfo *com)
{
	if(game->board.errors)
		printf("Error: board contains erroneous values\n");
	else
	{
		if(!is_solvable(&game->board))
			printf("Validation failed: board is unsolvable\n");
		else
			printf("Validation passed: board is solvable\n");
	}
	free_command(com);
}

/*Performs the initial generation of X random values
 * as part of the generate function. Returns the board with the
 * generated values*/
game_board initial_generation(game_board *board, int x)
{
	game_board sol;
	int attempts=0,selected_cells,*valid_values,row,col,cnt;
			do{
				for(selected_cells=0;selected_cells<x;) /*While less than X cells have been selected*/
				{
					/*Choose a random cell*/
					row=rand()%board_len(board);
					col=rand()%board_len(board);
					if(!board->cells[col][row].value) /*If the cell is empty*/
					{
						/*Get the valid values and count how many there are*/
						valid_values=get_valid_values(board,col,row);
						cnt=0;
						while(valid_values[cnt])
							cnt++;
						if(cnt){ /*If there's at least one valid value*/
							/*Assign a random valid value, increment selected cells and continue*/
							set_cell(board,col,row,valid_values[rand()%cnt]);
							free(valid_values);
							selected_cells++;
						}
						else{ /*If there are no valid values for a cell, the board is unsolvable, so no point in continuing*/
							free(valid_values);
							break;
						}
					}
				}
				/*Try solving the board the random board*/
				sol=find_solution(board);
				if(UNSOLVABLE)
				{
					clear_non_fixed(board); /*Empties the board, since no cells are fixed at this point*/
					attempts++;
				}
				else /*If solvable, we are done*/
					break;
			}
			while(attempts<MAX_GENERATE_ATTEMPTS);
			return sol;
}

/*Second phase of the generation, gets a solved board (sol),
 * fixes Y random cells, copies it to the playing board (board) and
 * frees the solution*/
void second_generation(game_board *board, game_board *sol, int y)
{
	fix_random_cells(sol,y);
	clear_non_fixed(sol);
	copy_board(sol,board);
	free_board(sol);
}

/*Creates a 2d array of the generated fixed values so the
 * generate command can be undone/redone, similarly to the autofill command*/
int** get_generate_values(game_board *board,int y){
	int **curr,**autofill_values,i,j;
	autofill_values=(int**)malloc(sizeof(int*)*(y+1));
	curr=autofill_values;
	for(i=0;i<board_len(board);i++){
		for(j=0;j<board_len(board);j++){
			if(board->cells[i][j].is_fixed){
				(*curr)=(int*)malloc(sizeof(int)*3);
				(*curr)[0]=i;
				(*curr)[1]=j;
				(*curr)[2]=board->cells[i][j].value;
				curr++;
			}
		}
	}
	*curr=NULL;
	return autofill_values;
}

void execute_generate(game_data *game, commandInfo *com)
{
	int x,y,board_size;
	int **autofill_values;
	game_board sol;

	x=com->args[0];
	y=com->args[1];
	board_size=board_len(&game->board)*board_len(&game->board);

	if(!(x>=0 && x<=board_size) || !(y>=0 && y<=board_size))
		printf("Error: value not in range 0-%d\n",board_size);
	else if(game->board.empty_cells!=board_size)
		printf("Error: board is not empty \n");
	else
	{
		/*First phase of generation, filling X random cells*/
		sol=initial_generation(&game->board,x);
		if(UNSOLVABLE){
			printf("Error: puzzle generator failed\n");
			free_command(com);
		}
		else
		{
			/*Second phase of generation, clearing all but Y cells*/
			second_generation(&game->board,&sol,y);
			print_board(&game->board,game->state == solve,game->mark_errors);
			/*Gathering info for undo/redo and changing stacks accordingly*/
			autofill_values=get_generate_values(&game->board,y);
			com->autofill_values=autofill_values;
			push(&game->undo_stack,com);
			empty_stack(&game->redo_stack);
		}
	}
}

void execute(game_data *game, commandInfo *com)
{
	if(!check_mode_compatibility(game,com->commandName))
		com->commandName=invalid;
	switch(com->commandName){
			case set:
				execute_set(game,com);
				break;

			case hint:
				execute_hint(game,com);
				break;

			case validate:
				execute_validate(game,com);
				break;

			case undo:
				execute_undo(game,com);
				break;

			case redo:
				execute_redo(game,com);
				break;

			case reset:
				execute_reset(game,com);
				break;

			case m_errors:
				execute_mark_errors(game,com);
				break;

			case p_board:
				execute_print_board(game,com);
				break;

			case autofill:
				execute_autofill(game,com);
				break;

			case num_solutions:
				execute_num_solutions(game,com);
				break;

			case ex:
				execute_exit(game,com);
				break;

			case invalid:
				execute_invalid(com);
				break;
			case solve_command:
				execute_solve(game,com);
				break;
			case save:
				execute_save(game,com);
				break;
			case edit_command:
				execute_edit(game,com);
				break;
			case generate:
				execute_generate(game,com);
				break;
		}
	fflush(stdout);
}
