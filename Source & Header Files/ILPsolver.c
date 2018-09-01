/*This module implements the ILP solution algorithm
 * using Gurobi Optimizer*/

#include <stdlib.h>
#include <stdio.h>
#include "gurobi_c.h"
#include "board.h"
#include "error_handler.h"

#define CONST_ROW_AND_COL 0
#define CONST_ROW_AND_VAL 1
#define CONST_COL_AND_VAL 2
#define CONST_BLOCK_AND_VAL 3
#define CONSTRAINT_MODE_TYPE_COUNT 4

/*Creates a variable index map, where map[i][j][k] is the index of the
 * variable that indicates the value k in the cell <i,j>
 * If the variable is not part of any constraint, map[i][j][k] will contain -1
 * Also updates the int pointed to by var_count to contain the number of used variables*/
int ***get_var_index_map(game_board *board, int *var_count){
	int ***map,i,j,k,len,*valid_values,valid_values_index;
	len = board_len(board);
	*var_count = 0;

	/*Memory allocation*/
	map = (int***)malloc(sizeof(int**) * len);
	if(map == NULL) function_error(f_malloc);
	for(i = 0; i < len; ++i){
		map[i] = (int**)malloc(sizeof(int*)*len);
		if(map[i] == NULL) function_error(f_malloc);
		for(j = 0; j < len; ++j){
			map[i][j] = (int*) malloc(sizeof(int) * len);
			if(map[i][j] == NULL) function_error(f_malloc);
		}
	}
	for(i = 0; i < len; ++i){
		for(j = 0; j < len; ++j){
			/*Get the valid values for the each cell (in order)*/
			if(board->cells[i][j].value == 0){
				valid_values = get_valid_values(board,i,j);
			}
			valid_values_index = 0;
			for(k = 0; k < len; ++k){
				/*if cell is empty and k+1 is a valid value*/
				if(board->cells[i][j].value == 0 && valid_values[valid_values_index] == k+1){
					map[i][j][k] = *var_count; /*Set the index of the variable ijk to the last*/
					*var_count+=1; /*Increment variable counter*/
					valid_values_index++; /*Go to the next valid value*/
				}else{
					/*k is not a valid value for cell i,j , so we don't use the variable ijk*/
					map[i][j][k] = -1;
				}
			}
			/*Free the valid values array only if we checked an empty cell (i.e it was
			 * in fact dynamically allocated)*/
			if(board->cells[i][j].value == 0){
				free(valid_values);
			}
		}
	}
	return map;
}

/*Frees the index map created in the function above*/
void free_var_index_map(int b_len, int ***map){
	int i,j;
	for(i = 0; i < b_len; ++i){
		for(j = 0; j < b_len; ++j){
			free(map[i][j]);
		}
		free(map[i]);
	}
	free(map);
}
/*Returns a board with the solution array sol applied to it*/
game_board sol_to_board(game_board *source, double *sol, int ***var_index_map){
	game_board solution;
	int i,j,k,len;
	solution = create_board(source->block_rows,source->block_columns);
	len = board_len(source);
	for(i = 0; i < len; ++i){
		for(j = 0; j < len; ++j){
			if(source->cells[i][j].value != 0){ /*if source has cell set - keep the same value*/
				set_cell(&solution,i,j,source->cells[i][j].value);
			}else{ /*else - search for correct value according to the solution*/
				for(k = 0; k < len; ++k){
					if(var_index_map[i][j][k] != -1 && sol[var_index_map[i][j][k]] > 0.5){ /*if (i,j,k) is a var and is 1 in the solution*/
						set_cell(&solution,i,j,k+1);
						break;
					}
				}
			}
		}
	}
	return solution;
}

/*Converts a 2d variable array to a constraint gurobi can read and
 * returns the constraint length
 */
int get_gurobi_constraint(game_board *board, int **vars, int ***var_index_map,int *ind){
	int i,real_var_count,len;
	len = board_len(board);
	real_var_count = 0; /*the number of vars in the gorubi constraint*/
	for(i = 0; i < len; ++i){
		if(var_index_map[vars[i][0]][vars[i][1]][vars[i][2]] == -1){/*if this is a constant*/
			if(board->cells[vars[i][0]][vars[i][1]].value == vars[i][2] + 1){ /*constant == 1*/
				return 0;
			}
		}else{
			ind[real_var_count++] = var_index_map[vars[i][0]][vars[i][1]][vars[i][2]];
		}
	}
	return real_var_count;
}

/*Fills the constraint 2d array with the required constrained according to mode:
 * mode == 0: constant row and col(one value per cell constraint)
 * mode == 1: constant row and val(each value once per row)
 * mode == 2: constant col and val(each value once per col)
 * mode == 3: constant block and val(each value once per block)
 */
void get_constraint(game_board *board, int **constraint, char mode, int const1, int const2){
	int k,len,x,y;
	len = board_len(board);
	for(k = 0; k < len; ++k){
		switch (mode){
		case CONST_ROW_AND_COL: /*cell[const2][const1] == k*/
			constraint[k][0] = const2; /*row*/
			constraint[k][1] = const1; /*col*/
			constraint[k][2] = k;
			break;
		case CONST_ROW_AND_VAL: /*cell[k][const1] == const2*/
			constraint[k][0] = k;
			constraint[k][1] = const1;
			constraint[k][2] = const2;
			break;
		case CONST_COL_AND_VAL: /*cell[const1][k] == const2*/
			constraint[k][0] = const1;
			constraint[k][1] = k;
			constraint[k][2] = const2;
			break;
		case CONST_BLOCK_AND_VAL: /* cell k of block const1 == const2*/
			get_cell_in_block(board,const1,k,&x,&y);
			constraint[k][0] = x;
			constraint[k][1] = y;
			constraint[k][2] = const2;
			break;
		}
	}
}

/*setup the gurobi environment and model*/
void gurobi_setup( GRBenv** env, GRBmodel** model) {
	int error;
	error = GRBloadenv(env, "mip1.log");
	if (error) {
		printf("Error: GRBloadenv has failed\n");
	}
	error = GRBnewmodel(*env, model, "mip1", 0, NULL, NULL, NULL, NULL, NULL);
	if (error) {
		printf("Error: GRBnewmodel has failed\n");
	}
	error = GRBsetintparam(GRBgetenv(*model), "OutputFlag", 0);
	if (error) {
		printf("Error: GRBsetintparam has failed\n");
	}
}

/*solve the gorubi model*/
void gurobi_solve_model(GRBmodel* model, int* optimstatus) {
	int error;
	error = GRBupdatemodel(model);
	if (error) {
		printf("Error: GRBupdatemodel has failed\n");
	}
	error = GRBoptimize(model);
	if (error) {
		printf("Error: GRBoptimize has failed\n");
	}
	error = GRBwrite(model, "mip1.lp");
	if (error) {
		printf("Error: GRBwrite has failed\n");
	}
	error = GRBgetintattr(model, GRB_INT_ATTR_STATUS, &*optimstatus);
	if (error) {
		printf("Error: GRBgetintattr has failed\n");
	}
}

/*add the needed contraints to the model*/
void add_soduko_contraints(game_board* source, int*** var_index_map,int** ind, GRBmodel** model, double** val) {
	int i,j,mode,len,error,constraint_size;
	int **constraint;
	len = board_len(source);
	constraint = (int**)calloc(len,sizeof(int*));
	if(constraint == NULL)function_error(f_calloc);
	for(i = 0; i < len; ++i){
		constraint[i] =(int*)calloc(3,sizeof(int));
		if(constraint[i] == NULL) function_error(f_calloc);
	}
	for (i = 0; i < len; ++i) {
		for (j = 0; j < len; ++j) {
			for (mode = 0; mode < CONSTRAINT_MODE_TYPE_COUNT; ++mode) {
				get_constraint(source, constraint, mode, i, j);
				constraint_size = get_gurobi_constraint(source, constraint,
						var_index_map, *ind);
				if (constraint_size){
					error = GRBaddconstr(*model, constraint_size, *ind, *val,
							GRB_EQUAL, 1.0, NULL);
					if (error) {
						printf("Error: GRBaddconstr has failed\n");
					}
				}
				
			}
		}
	}
	for(i = 0; i < len; ++i){
		free(constraint[i]);
	}
	free(constraint);
}

/*add the varibales to the model*/
void gurobi_add_vars(int var_count,GRBmodel** model,double* val) {
	int i,error;
	char *vtype;
	vtype = (char*)malloc(sizeof(char) * var_count);
	if(vtype == NULL)function_error(f_malloc);
	for (i = 0; i < var_count; ++i) {
		vtype[i] = GRB_BINARY;
		val[i] = 1.0;
	}
	error = GRBaddvars(*model, var_count, 0, NULL, NULL, NULL, NULL, NULL, NULL,vtype, NULL); /*objective function is 0*/
	if (error) {
		printf("Error: GRBaddvars has failed\n");
	}
	error = GRBsetintattr(*model, GRB_INT_ATTR_MODELSENSE, GRB_MAXIMIZE); /*set to maximize*/
	if (error) {
		printf("Error: GRBsetintattr has failed\n");
	}
	error = GRBupdatemodel(*model); /* update the model - to integrate new variables */
	if (error) {
		printf("Error: GRBupdatemodel has failed\n");
	}
	free(vtype);
}

/* Returns a solved board that begins in the same state as source
 * if no solution is found - returns a 0x0 board
 */
game_board find_solution(game_board *source){
	game_board solution;
	GRBenv   *env   = NULL;
	GRBmodel *model = NULL;
	int       error = 0;
	double    *sol,*val;
	int       *ind;
	int       optimstatus;
	int ***var_index_map;
	int var_count;


	gurobi_setup( &env, &model);

	var_index_map = get_var_index_map(source,&var_count);

	val = (double*)malloc(sizeof(double) * var_count);
	if(val == NULL)function_error(f_malloc);
	sol = (double*)malloc(sizeof(double) * var_count);
	if(sol == NULL)function_error(f_malloc);
	ind = (int*)malloc(sizeof(int) * var_count);
	if(ind == NULL)function_error(f_malloc);

	gurobi_add_vars(var_count,&model,val);
	add_soduko_contraints(source, var_index_map, &ind, &model, &val);
	gurobi_solve_model(model, &optimstatus);
	if(optimstatus == GRB_INFEASIBLE){
		solution.block_columns = 0;
		solution.block_rows = 0;

	}else{
		error = GRBgetdblattrarray(model,GRB_DBL_ATTR_X,0,var_count,sol);
		if(error){
			printf("Error: GRBgetdblattrarray has failed\n");
		}
		solution = sol_to_board(source,sol,var_index_map);
	}
	free(sol);
	free(ind);
	free(val);
	free_var_index_map(board_len(source),var_index_map);
	GRBfreemodel(model);
	GRBfreeenv(env);
	return solution;
}

/*Returns 1 if the board is solvable, else returns 0*/

char is_solvable(game_board* board){
	char solvable;
	game_board temp = find_solution(board);
	solvable = (temp.block_columns != 0);
	if(solvable){
		free_board(&temp);
	}
	return solvable;
}
