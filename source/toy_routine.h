#pragma once

#include "toy_common.h"
#include "toy_ast.h"

//internal structure that holds the individual parts of a compiled routine
typedef struct Toy_Routine {
	unsigned char* param; //c-string params in sequence (could be moved below the jump table?)
	unsigned int paramCapacity;
	unsigned int paramCount;

	unsigned char* code; //the instruction set
	unsigned int codeCapacity;
	unsigned int codeCount;

	unsigned int* jumps; //each 'jump' is the starting address of an element within 'data'
	unsigned int jumpsCapacity;
	unsigned int jumpsCount;

	unsigned char* data; //data for longer stuff
	unsigned int dataCapacity;
	unsigned int dataCount;

	unsigned char* subs; //subroutines, recursively
	unsigned int subsCapacity;
	unsigned int subsCount;

	bool panic; //any issues found at this point are compilation errors
} Toy_Routine;

TOY_API void* Toy_compileRoutine(Toy_Ast* ast);

