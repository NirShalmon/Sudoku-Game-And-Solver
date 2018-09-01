/*This module handles all operations involving files, i.e. loading and saving board
 * to and from files*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "board.h"
#include "error_handler.h"

#define DELIM " \t\r\n"

/*Saves the board to the given path
 * Return 1 on a successful save, otherwise returns 0
 * fix_cells parameter indicates whether all non-empty cells of the board
 * should be fixed when saved, i.e. if we're saving in edit mode*/
char save_board(game_board *board,char* path,char fix_cells){
	int x,y;
	FILE* outFile;
	outFile = fopen(path,"w");
	if(outFile == NULL){
		return 0; /*Not function_error according to the project description*/
	}
	if(fix_cells){
		fix_all_cells(board);
	}
	fprintf(outFile,"%d %d\n",board->block_rows,board->block_columns);
	if(ferror(outFile)) function_error(f_fprintf);
	for(y = 0; y < board_len(board);++y){
		for(x = 0; x < board_len(board);++x){
			fprintf(outFile,"%d",board->cells[x][y].value);
			if(ferror(outFile)) function_error(f_fprintf);
			if(board->cells[x][y].is_fixed){
				fprintf(outFile,".");
				if(ferror(outFile)) function_error(f_fprintf);
			}
			fprintf(outFile," ");
			if(ferror(outFile)) function_error(f_fprintf);
		}
		fprintf(outFile,"\n");
	}
	fclose(outFile);
	return 1;
}

/*Returns the size of the file, and moves the pointer to it's start*/
long file_size(FILE* file){
	long sz;
	if(fseek(file,0,SEEK_END)) function_error(f_fseek);
	if((sz = ftell(file)) == -1L) function_error(f_ftell);
	if(fseek(file,0,SEEK_SET)) function_error(f_fseek);
	return sz;
}

/*Returns a string containing the text in the file
 * Assumes inFile is an existing file pointer open in write mode*/
char* get_contents(FILE* inFile){
	int file_sz;
	char *fileContents;
	file_sz = file_size(inFile);
	fileContents = (char*)calloc(file_sz+1,sizeof(char));
	if(fileContents == NULL) function_error(f_calloc);
	fread(fileContents,1,file_sz+1,inFile);
	if(ferror(inFile)) function_error(f_fread);
	return fileContents;
}

/*Loads a board from the given path
 * Returns a 0x0 board if the loading has failed*/
game_board load_board(char* path){
	int cell_num,block_rows,block_columns,len,x,y;
	FILE* inFile;
	char* fileContents,*tok;
	game_board board;

	inFile = fopen(path,"r");
	if(inFile == NULL){ /*Not function_error*/
		board.block_columns = 0;
		board.block_rows = 0;
		return board;
	}

	fileContents = get_contents(inFile);
	tok = strtok(fileContents,DELIM);
	/*Getting board dimensions*/
	block_rows = atoi(tok);
	tok = strtok(NULL,DELIM);
	block_columns = atoi(tok);
	board = create_board(block_rows,block_columns);
	tok = strtok(NULL,DELIM);
	cell_num = 0;

	while(tok != NULL){
		x = cell_num % board_len(&board);
		y = cell_num / board_len(&board);
		len = strlen(tok);
		set_cell(&board,x,y,atoi(tok));
		if(tok[len-1] == '.'){ /*Checks whether the cell is fixed*/
			board.cells[x][y].is_fixed = 1;
		}
		tok = strtok(NULL,DELIM);
		++cell_num;
	}
	fclose(inFile);
	free(fileContents);
	return board;
}
