#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "error_handler.h"
#include "parser.h"
#include "stack_tools.h"
#include "executer.h"
#include "board.h"



int main(){
	/*Game setup phase*/
	game_data game;
	commandInfo *com; /*Commands read from the user will be kept here*/
	game.undo_stack=create_stack();
	game.redo_stack=create_stack();
	srand(time(NULL));

	game.state = init;
	game.mark_errors = 1;
	printf("Sudoku\n------\n");

	/*Game phase - constanly read commands and execute them*/
	while(1){
		printf("Enter your command:\n");
		fflush(stdout);
		com = readCommand();
		execute(&game,com);
	}
	return 0;
}
