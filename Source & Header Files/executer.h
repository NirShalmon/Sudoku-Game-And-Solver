/*This module handles the execution of user commands*/


#include "board.h"

/*An enum for the game states*/
typedef enum game_state
{	init,edit,solve
} game_state;


/*A struct that keeps all the relevant data for a game session*/
typedef struct game_data{
	game_board board;
	Stack undo_stack,redo_stack;
	char mark_errors;
	game_state state;

}game_data;

/*Executes the given command on the given game data*/
void execute(game_data *game, commandInfo *com);
