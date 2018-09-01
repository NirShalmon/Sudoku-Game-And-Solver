/*This module handles function error prompting*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "error_handler.h"



/*Prints out a message of failure for the corresponding fucntion and exits the program
 * (See header file for documentation of the failable_function enum)*/
void function_error(failable_function func){
    char *function_name;
    switch(func){
        case f_malloc:
            function_name = "malloc";
            break;
        case f_scanf:
            function_name = "scanf";
            break;
        case f_calloc:
        	function_name = "calloc";
        	break;
        case f_fgets:
        	function_name = "fgets";
        	break;
        case f_fopen:
        	function_name = "fopen";
        	break;
        case f_fprintf:
        	function_name = "fprintf";
        	break;
        case f_fseek:
        	function_name = "fseek";
        	break;
        case f_ftell:
        	function_name = "ftell";
        	break;
        case f_fread:
        	function_name = "fread";
        	break;
    }
    printf("Error: %s has failed\n",function_name);
    exit(1);
}




