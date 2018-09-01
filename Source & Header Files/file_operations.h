/*This module handles all operations involving files, i.e. loading and saving board
 * to and from files*/


/*Saves the board to the given path
 * Return 1 on a successful save, otherwise returns 0*/
char save_board(game_board *board,char* path,char fix_cells);

/*Loads a board from the given path
 * Returns a 0x0 board if the loading has failed*/
game_board load_board(char* path);
