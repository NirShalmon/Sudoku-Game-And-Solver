/*This module handles user interface, i.e. reading commands from the user
 * and converting it into data usable by the other modules*/

/*An enum for all the possible commands recieved by the user*/
typedef enum func_name
{set, hint, validate, ex, undo, redo, reset,m_errors,p_board,autofill,num_solutions, invalid,solve_command,save,edit_command,generate} func_name;

/*A struct that holds all the relevant information from a command given by the user
 * func_name is the name of the commands, args is for its numerical arguments, no more than 3 are ever needed
 * rawInput saves the command as it was read from the user
 * tokens saves the arguments as strings, and converts them to numbers if necessary*/
typedef struct commandInfo
{
	char* rawInput;
	func_name commandName;
	char *tokens[3]; /*Arguments received as strings*/
	int args[3]; /*Arguments converted to int if necessary*/
	int prev_value; /*When a set command is undone, this is the previous value in the cell*/
	int **autofill_values; /*Values for the autofill command*/

} commandInfo;
commandInfo* readCommand(); /*Reads the command given by the user*/
void print_undo_redo_prompt(int x, int y, int old_val, int new_val, char set); /*Prints a fitting prompt for the undo/redo commands*/
void free_command(commandInfo *com); /*Frees the memory allocated for a command*/


