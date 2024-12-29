#pragma once

#include "toy_common.h"
#include "toy_ast.h"

//the 'escapes' are lists of data used for processing the 'break' and 'continue' keywords, and can be safely ignored
typedef struct Toy_private_EscapeEntry_t {
	unsigned int addr; //the address to write *to*
	unsigned int depth; //the current depth
} Toy_private_EscapeEntry_t;

typedef struct Toy_private_EscapeArray {
	unsigned int capacity;
	unsigned int count;
	Toy_private_EscapeEntry_t data[];
} Toy_private_EscapeArray;

//not needed at runtime, so they can be bigger
#ifndef TOY_ESCAPE_INITIAL_CAPACITY
#define TOY_ESCAPE_INITIAL_CAPACITY 32
#endif

#ifndef TOY_ESCAPE_EXPANSION_RATE
#define TOY_ESCAPE_EXPANSION_RATE 4
#endif

TOY_API void* Toy_private_resizeEscapeArray(Toy_private_EscapeArray* ptr, unsigned int capacity);

//internal structure that holds the individual parts of a compiled routine
typedef struct Toy_Routine {
	unsigned char* param; //c-string params in sequence (could be moved below the jump table?)
	unsigned int paramCapacity;
	unsigned int paramCount;

	unsigned char* code; //the instruction set
	unsigned int codeCapacity;
	unsigned int codeCount;

	unsigned char* jumps; //each 'jump' is the starting address of an element within 'data'
	unsigned int jumpsCapacity;
	unsigned int jumpsCount;

	unsigned char* data; //data for longer stuff
	unsigned int dataCapacity;
	unsigned int dataCount;

	unsigned char* subs; //subroutines, recursively
	unsigned int subsCapacity;
	unsigned int subsCount;

	unsigned int currentScopeDepth;
	Toy_private_EscapeArray* breakEscapes;
	Toy_private_EscapeArray* continueEscapes;

	bool panic; //any issues found at this point are compilation errors
} Toy_Routine;

TOY_API void* Toy_compileRoutine(Toy_Ast* ast);
