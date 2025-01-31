#pragma once

#include "toy_common.h"

#include "toy_bucket.h"
#include "toy_scope.h"
#include "toy_module.h"

#include "toy_value.h"
#include "toy_string.h"
#include "toy_stack.h"
#include "toy_array.h"
#include "toy_table.h"

typedef struct Toy_VM {
	//raw instructions to be executed
	unsigned char* code;

	//metadata
	unsigned int jumpsCount;
	unsigned int paramCount;
	unsigned int dataCount;
	unsigned int subsCount;

	unsigned int codeAddr;
	unsigned int jumpsAddr;
	unsigned int paramAddr;
	unsigned int dataAddr;
	unsigned int subsAddr;

	//execution utils
	unsigned int programCounter;

	//scope - block-level key/value pairs
	Toy_Scope* scope;

	//stack - immediate-level values only
	Toy_Stack* stack;

	//easy access to memory
	Toy_Bucket* stringBucket; //stores the string literals
	Toy_Bucket* scopeBucket; //stores the scope instances TODO: is this separation needed?
} Toy_VM;

TOY_API void Toy_resetVM(Toy_VM* vm, bool preserveScope);

TOY_API void Toy_initVM(Toy_VM* vm); //creates memory
TOY_API void Toy_inheritVM(Toy_VM* vm, Toy_VM* parent); //inherits memory

TOY_API void Toy_bindVM(Toy_VM* vm, Toy_Module* module, bool preserveScope);
TOY_API void Toy_runVM(Toy_VM* vm);

TOY_API void Toy_freeVM(Toy_VM* vm);

//TODO: inject extra data (hook system for external libraries)
