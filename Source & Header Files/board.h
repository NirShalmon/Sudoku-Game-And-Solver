/*This module handles everything regarding the game board
 * i.e. setting cells, verifying number of vales in each row/column/block etc.*/

#ifndef _GAMEH_
#define _GAMEH_

#define blocks_per_row  block_rows       /*Board dimensions*/
#define blocks_per_column block_columns        /*Board dimensions*/

/*A struct that represents a single cell.
 * Keeps the value and an is_fixed variable indicating whether the cell is fixed*/
typedef struct board_cell{

    int value; /* 0 represents an empty cell*/
    char is_fixed;

} game_cell;

/*A struct that represents a game board
 * Keeps a 2d matrix of cells for the board itself
 * and arrays that indicating whether a value exists in every
 * row, column and block. Also keeps the amount of empty cells*/
typedef struct game_board{

	int block_rows,block_columns; /*block dimensions*/
    game_cell **cells; /* cells is an array of arrays of cells*/
    char **values_in_row; /*Keeps the number of the values in each row*/
    char **values_in_column; /*Keeps the number of the values in each column*/
    char **values_in_block; /*Keeps the number of the values in each block*/
    int empty_cells; /*Keeps the number of the currently empty cells on the board*/
    int errors; /*number of values in the values_in_x arrays >= 2*/
} game_board;


/*Creates an empty board with the given block dimensions*/
game_board create_board(int block_rows,int block_columns);

/*Returns the length of the board, i.e. the number of cells in a block*/
int board_len(game_board *board);

/*Prints the board in the given format*/
void print_board(game_board *board,char mark_fixed,char mark_errors);

/*Sets all the cells on the board to be empty and non-fixed*/
void clear_board(game_board *board);

/*Set the cell <x,y> to the new value, updates all the required value_in_x arrays*/
char set_cell(game_board *board, int x, int y,int value);

/*Frees all the memory allocated for a board struct*/
void free_board(game_board *board);

/*Returns an array of the legal values for the cell <x,y>*/
int* get_valid_values(game_board *board, int x, int y);

/*Fixes all non-empty cells*/
void fix_all_cells(game_board *board);

/*Copies all the relevant values from the source board to the target board
 * Assumes the two boards are of the same dimensions*/
void copy_board(game_board *source, game_board *target);

/*Fixes random cells, as many as the hints parameter indicates*/
void fix_random_cells(game_board *board, int hints);

/*Clears all cells that are not fixed*/
void clear_non_fixed(game_board *board);

/*Puts the coordinates of the cell_index cell of block block_index in (cell_x,cell_y)*/
void get_cell_in_block(game_board *board,int block_index, int cell_index,int *cell_x, int *cell_y);

#endif
