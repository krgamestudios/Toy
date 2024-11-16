#pragma once

#include "toy_common.h"

#include "toy_bytecode.h"
#include "toy_bucket.h"
#include "toy_stack.h"
#include "toy_scope.h"

typedef struct Toy_VM {
	//raw instructions to be executed
	unsigned char* module;
	unsigned int moduleSize;

	unsigned int paramSize;
	unsigned int jumpsSize;
	unsigned int dataSize;
	unsigned int subsSize;

	unsigned int paramAddr;
	unsigned int codeAddr;
	unsigned int jumpsAddr;
	unsigned int dataAddr;
	unsigned int subsAddr;

	unsigned int programCounter;

	//stack - immediate-level values only
	Toy_Stack* stack;

	//scope - block-level key/value pairs
	Toy_Scope* scope;

	//easy access to memory
	Toy_Bucket* stringBucket; //stores the string literals
	Toy_Bucket* scopeBucket; //stores the scopes

	//TODO: panic flag
} Toy_VM;

TOY_API void Toy_initVM(Toy_VM* vm);
TOY_API void Toy_bindVM(Toy_VM* vm, struct Toy_Bytecode* bc); //process the version data
TOY_API void Toy_bindVMToModule(Toy_VM* vm, unsigned char* module); //process the module only

TOY_API void Toy_runVM(Toy_VM* vm);
TOY_API void Toy_freeVM(Toy_VM* vm);

TOY_API void Toy_resetVM(Toy_VM* vm); //prepares for another run without deleting stack, scope and memory

//TODO: inject extra data (hook system for external libraries)
