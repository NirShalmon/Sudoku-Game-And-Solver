/*This module handles function error prompting*/

#define MAX_INPUT_LEN 256

/*An enum representing all the functions that can cause a failure*/
typedef enum failable_function{
    f_malloc,f_scanf,f_calloc,f_fgets,f_fopen,f_fprintf,f_fseek,f_ftell,f_fread
} failable_function;


/*Prints an error fuction prompt for a failed function and exits the program*/
void function_error(failable_function func);
