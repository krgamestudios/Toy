#pragma once

#include "toy_common.h"
#include "toy_ast.h"

//the 'escapes' are lists of data used for processing the 'break' and 'continue' keywords
typedef struct Toy_private_EscapeEntry_t {
	unsigned int addr; //the address to write *to*
	unsigned int depth; //the current depth
} Toy_private_EscapeEntry_t;

typedef struct Toy_private_EscapeStack {
	struct Toy_private_EscapeStack* next;
	unsigned int capacity;
	unsigned int count;
	Toy_private_EscapeEntry_t data[];
} Toy_private_EscapeStack;

//structure for holding the bytecode during compilation
typedef struct Toy_Bytecode {
    //store the submodules as a list, identified by a string (and index, for actual lookups)
	Toy_String* identity; //root compiler has null id
    unsigned int lookupIndex;
    struct Toy_Bytecode* next;
	unsigned char* buffer; //holds the results while the rest of the program is compiled
	char* sourceHook; //BUGFIX: god help me

	unsigned char* code; //the instruction set
	unsigned int codeCapacity;
	unsigned int codeCount;

	unsigned char* jumps; //each 'jump' is the starting address of a c-string within 'data'
	unsigned int jumpsCapacity;
	unsigned int jumpsCount; //WONTFIX: The jump table probably isn't needed, but removing it is risky

	unsigned char* param; //each 'param' is the starting address of a c-string within 'data'
	unsigned int paramCapacity;
	unsigned int paramCount;

	unsigned char* data; //a block of read-only data
	unsigned int dataCapacity;
	unsigned int dataCount;

	//tools for handling the build process
	unsigned int currentScopeDepth;
	Toy_private_EscapeStack* breakEscapes;
	Toy_private_EscapeStack* continueEscapes;

	//compilation errors
	bool panic;
} Toy_Bytecode;

TOY_API unsigned char* Toy_compileToBytecode(Toy_Ast* ast);

//URGENT: Semantic optimisations
// [ ] Extract Subroutines & Imports
// [ ] Folding Constant Literals
// [ ] Extra Block Reduction
// [ ] Dead Code Elimination



//URGENT: subroutines, such as functions and imported files, are held as a cached list with a key-value system
//URGENT: They are then referenced from the "root" of the bytecode

//TODO: rename 'Toy_Bytecode' to something better