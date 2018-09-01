/*This module handles everything regarding the game board
 * i.e. setting cells, verifying number of values in each row/column/block etc.*/

#include "board.h"

#include <stdio.h>
#include <stdlib.h>
#include "error_handler.h"
#include "stack_tools.h"

/*Retunrs the length of the board, i.e the size of a block/row/column*/
int board_len(game_board *board){
	return board->block_columns * board->block_rows;
}

/*Returns the length of the separating line, as described in the project file*/
int get_sep_line_len(game_board *board){
	return board_len(board) * 4 + board->block_rows+1;
}

/*Calculates the index of the block in which a cell is located
  Blocks are indexed left to right, up to down, starting at 1*/
int get_block_index(game_board *board,int x,int y){
	int block_y,block_x;
	block_y =  y/board->block_rows;
	block_x = x/board->block_columns;
	return block_y * board->blocks_per_row + block_x;
}

/*puts the coordinates of the cell_index cell of block block_index in (cell_x,cell_y)*/
void get_cell_in_block(game_board *board,int block_index, int cell_index,int *cell_x, int *cell_y){
	int block_x,block_y; /*coordinates of block*/
	int x_in_block,y_in_block;
	block_x = block_index % board->blocks_per_row;
	block_y = block_index / board->blocks_per_row;
	x_in_block = cell_index % board->block_columns;
	y_in_block = cell_index / board->block_columns;
	*cell_x = block_x * board->block_columns + x_in_block;
	*cell_y = block_y * board->block_rows + y_in_block;
}

/*Prints a separating line between rows of blocks*/
void print_sep_line(int len){
	int i;
	for(i = 0; i < len; i++){
		printf("-");
	}
	printf("\n");
}

/*Returns 1 if the given cell is erroneous, otherwise returns 0*/
char is_erronous(game_board *board, int x, int y){
	int cell_value;
	cell_value = board->cells[x][y].value;
	return board->values_in_row[y][cell_value] >= 2 ||
			board->values_in_column[x][cell_value] >= 2 ||
			board->values_in_block[get_block_index(board,x,y)][cell_value] >= 2;
}

/*Prints the string representation of the cell in position x,y of the board*/
void print_cell(game_board *board,char mark_fixed,char mark_errors, int x,int y){
	if(board->cells[x][y].value == 0){
		/*cell is empty*/
		printf("    ");
		return;
	}

	printf(" %2d",board->cells[x][y].value);
	if(board->cells[x][y].is_fixed && mark_fixed){
		printf(".");
	}else if(is_erronous(board,x,y) && mark_errors){
		printf("*");
	}else{
		printf(" ");
	}
}

/*Prints the string representation of the game board
  Goes row by row, block by block, left to right, up to down,
  prints separating characters where needed*/

void print_board(game_board *board,char mark_fixed,char mark_errors){
	int i,j,k,t;
	print_sep_line(get_sep_line_len(board));
	for(i = 0; i < board->blocks_per_column; i++){
		/*print line of blocks*/
		for(j = 0; j < board->block_rows; j++){
			/*print line*/
			for(k = 0; k < board->blocks_per_row; k++){
				printf("|"); /*End of row in a block*/
				for(t = 0; t < board->block_columns; t++){
					print_cell(board,mark_fixed,mark_errors,k * board->block_columns + t,i * board->block_rows + j);
				}
			}
			printf("|\n"); /*End of row in a block and end of line*/
		}
		print_sep_line(get_sep_line_len(board)); /*End of a line of blocks, prints separator line*/
	}
	fflush(stdout);
}

void update_value_var(game_board *board, char *value, int delta){
	if(delta == 1 && *value == 1){
		board->errors++;
	}else if(delta == -1 && *value == 2){
		board->errors--;
	}
	*value += delta;
}

/*
 * Updates the arrays that keep existent values in row, column and block
 * (See game_board struct documentation in header file)
 */
void set_board_values(game_board *board,int x,int y, int block_index,int value, char set){
	int delta;
	if(set) delta = 1;

	else delta = -1;
	update_value_var(board,&board->values_in_block[block_index][value],delta);
	update_value_var(board,&board->values_in_row[y][value],delta);
	update_value_var(board,&board->values_in_column[x][value],delta);
}

/*
 * Sets the corresponding cell on the board to value
 * If the new value is erroneous, changes it and returns 2
 * Otherwise changes the value and returns 0
 * Assumes x and y are valid and 0<=value<=BOARD_LENGTH.
 * NOTE - this will change a fixed cell
 * when setting a fixed cell to 0 it will become not fixed
 */
char set_cell(game_board *board, int x, int y,int value){
	int block_index,cur_val;
	cur_val = board->cells[x][y].value;
	block_index = get_block_index(board,x,y);
	if(value == board->cells[x][y].value){ /* if there is no change */
		return 0;
	}
	if(value == 0){
		/*clear the current value*/
		board->empty_cells++;
		set_board_values(board,x,y,block_index,cur_val,0);
		board->cells[x][y].value = 0;
		board->cells[x][y].is_fixed = 0;
		return 0;
	}
	/*clear the current value*/
	set_board_values(board,x,y,block_index,cur_val,0);
	/*set the new value*/
	if(!board->cells[x][y].value)
		board->empty_cells--;
	board->cells[x][y].value = value;
	set_board_values(board,x,y,block_index,value,1);
	if(board->values_in_block[block_index][value] >= 2 ||
		board->values_in_row[y][value] >= 2 ||
		board->values_in_column[x][value]>= 2){
		return 2;
	}
	return 0;
}

/*Clears a cell, i.e. setting it to be empty and not fixed*/
void clear_cell(game_board *board,int x, int y){
	set_cell(board,x,y,0);
	board->cells[x][y].is_fixed = 0;
}

/*Clears the entire board*/
void clear_board(game_board *board){
	int i,j;
	for(i = 0; i < board_len(board); i++){
		for(j = 0; j < board_len(board); j++){
			clear_cell(board,i,j);
		}
	}
}

/*Creates and empty game_board struct, initialising a matrix for the cells and
 * the arrays that keep track of each value in rows, columns and blocks, as well
 * as a variable that keeps the number of empty cells*/
game_board create_board(int block_rows, int block_columns){
    int i;
	game_board board;
	board.block_columns = block_columns;
	board.block_rows = block_rows;
	board.errors = 0;
	board.empty_cells= board_len(&board)*board_len(&board);
    board.cells = (game_cell**)calloc(board_len(&board),sizeof(game_cell*));
    board.values_in_block = (char**)calloc(board_len(&board),sizeof(char*));
    board.values_in_column = (char**)calloc(board_len(&board),sizeof(char*));
    board.values_in_row = (char**)calloc(board_len(&board),sizeof(char*));
    if(board.cells == NULL || board.values_in_block == NULL || board.values_in_column == NULL || board.values_in_row == NULL){
        function_error(f_calloc);
    }
    for(i = 0; i < board_len(&board); i++){
    	board.values_in_block[i] = (char*)calloc(board_len(&board)+1,sizeof(char));
    	board.values_in_column[i] = (char*)calloc(board_len(&board)+1,sizeof(char));
    	board.values_in_row[i] = (char*)calloc(board_len(&board)+1,sizeof(char));
    	board.cells[i] = (game_cell*)calloc(board_len(&board),sizeof(game_cell));
    	if(board.cells[i] == NULL || board.values_in_block[i] == NULL || board.values_in_column[i] == NULL || board.values_in_row[i] == NULL){
    		function_error(f_calloc);
    	}
    }
    return board;
}

/*Returns an array of all the valid values that can be assigned to a cell
 * End of array is the first index to contain 0*/
int* get_valid_values(game_board *board, int x, int y){
	int i, cnt,block_index;
	int *valid_values;
	valid_values = (int*)calloc(board_len(board)+1,sizeof(int));
	block_index = get_block_index(board,x,y);
	if(valid_values == NULL){
		function_error(f_calloc);
	}
	cnt = 0; /*Counts the amount of valid values*/
	for(i = 1; i <= board_len(board); i++){
		if(!board->values_in_block[block_index][i] &&
			!board->values_in_column[x][i]	&&
			!board->values_in_row[y][i]){ /*Check if the value already exists in the row/block/column*/
			valid_values[cnt++] = i; /*If not, add to array*/
		}
	}
	valid_values[cnt] = 0;
	return valid_values;
}

/*Set all non-empty cells of the board to be fixed*/
void fix_all_cells(game_board *board){
	int i,j;
	for(i = 0; i < board_len(board); i++){
		for(j = 0; j < board_len(board); j++){
			if(board->cells[i][j].value != 0){
				board->cells[i][j].is_fixed = 1;
			}
		}
	}
}

/*Free a dynamically allocated 2d array of game_cell*/
void free_2d_cell_array(game_cell **arr, int len){
	int i;
	for(i = 0; i < len; i++){
		free(arr[i]);
	}
	free(arr);
}

/*Free a dynamically allocated 2d array of chars*/
void free_2d_char_array(char **arr, int len){
	int i;
	for(i = 0; i < len; i++){
		free(arr[i]);
	}
	free(arr);
}



/*Frees a board struct*/
void free_board(game_board *board){
	free_2d_cell_array(board->cells,board_len(board));
	free_2d_char_array(board->values_in_block,board_len(board));
	free_2d_char_array(board->values_in_column,board_len(board));
	free_2d_char_array(board->values_in_row,board_len(board));
}

/*Copy all the relevant data from the source board to the target board
 * At the end of the operation, source and target are completely identical
 * (Except for their address in memory,of course)
 * assumes boards are of the same dimensions*/
void copy_board(game_board *source, game_board *target)
{
	int i,j;
	target->empty_cells=source->empty_cells;
	target->errors=source->errors;
	for(i=0;i<board_len(source);i++){
		for(j=0;j<board_len(source);j++){
			target->cells[i][j].value=source->cells[i][j].value;
			target->cells[i][j].is_fixed=source->cells[i][j].is_fixed;
			target->values_in_block[i][j+1]=source->values_in_block[i][j+1];
			target->values_in_row[i][j+1]=source->values_in_row[i][j+1];
			target->values_in_column[i][j+1]=source->values_in_column[i][j+1];
		}
	}
}

/*Randomly selects the given amount of cells and sets them to be fixed*/
void fix_random_cells(game_board *board,int cell_num)
{
	int cnt,x,y;
	for(cnt=0;cnt<cell_num;)
	{
		x=rand()%board_len(board);
		y=rand()%board_len(board);
		if(!board->cells[x][y].is_fixed)
		{
			board->cells[x][y].is_fixed=1;
			cnt++;
		}
	}
}

/*Clears all the cells that are not fixed
 * Used when giving the user a board to solve*/
void clear_non_fixed(game_board *board)
{
	int x,y;
	for(x=0;x<board_len(board);x++){
		for(y=0;y<board_len(board);y++){
			if(!board->cells[x][y].is_fixed)
			{
				clear_cell(board,x,y);
			}
		}
	}
}


