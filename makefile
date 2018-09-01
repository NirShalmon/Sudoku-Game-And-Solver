CC = gcc
OBJS = main.o error_handler.o board.o parser.o exhaustive_solver.o stack_tools.o file_operations.o executer.o ILPsolver.o
EXEC = sudoku-console
COMP_FLAG = -ansi -Wall -Wextra \
-Werror -pedantic-errors
GUROBI_COMP = -I/usr/local/lib/gurobi563/include

GUROBI_LIB = -L/usr/local/lib/gurobi563/lib -lgurobi56



$(EXEC): $(OBJS)
	$(CC) $(OBJS) $(GUROBI_LIB) -o $@
all: sudoku-console
main.o: main.c board.h parser.h stack_tools.h executer.h error_handler.h
	$(CC) $(COMP_FLAG) -c $*.c
board.o: board.c board.h error_handler.h stack_tools.h
	$(CC) $(COMP_FLAG) -c $*.c
error_handler.o: error_handler.c error_handler.h
	$(CC) $(COMP_FLAG) -c $*.c
parser.o: parser.c parser.h error_handler.h
	$(CC) $(COMP_FLAG) -c $*.c
exhaustive_solver.o: exhaustive_solver.c exhaustive_solver.h board.h error_handler.h stack_tools.h
	$(CC) $(COMP_FLAG) -c $*.c
executer.o: executer.c executer.h parser.h stack_tools.h exhaustive_solver.h error_handler.h file_operations.h board.h ILPsolver.h
	$(CC) $(COMP_FLAG) -c $*.c
file_operations.o: file_operations.c file_operations.h board.h error_handler.h
	$(CC) $(COMP_FLAG) -c $*.c
stack_tools.o: stack_tools.c stack_tools.h parser.h error_handler.h
	$(CC) $(COMP_FLAG) -c $*.c
ILPsolver.o: ILPsolver.c ILPsolver.h board.h error_handler.h
	$(CC) $(COMP_FLAG) $(GUROBI_COMP) -c $*.c
clean:
	rm -f $(OBJS) $(EXEC)
