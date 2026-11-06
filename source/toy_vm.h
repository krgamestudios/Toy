#pragma once

#include "toy_common.h"

#include "toy_bucket.h"
#include "toy_scope.h"
#include "toy_stack.h"
#include "toy_attributes.h"

#include "toy_value.h"
#include "toy_string.h"
#include "toy_array.h"
#include "toy_table.h"
#include "toy_function.h"

typedef struct Toy_VM {
	//raw instructions to be executed
	unsigned char* bytecode;
	unsigned char* module;

	//module info
	unsigned int lookupCount;
	unsigned int lookupAddr;

	//metadata
	unsigned int jumpsCount;
	unsigned int paramCount;
	unsigned int dataCount;

	unsigned int codeAddr;
	unsigned int jumpsAddr;
	unsigned int paramAddr;
	unsigned int dataAddr;

	//Host context and instruction budget are shared with nested calls.
	void* userData;
	uint64_t* instructionBudget;
	unsigned int nativeArgumentCount;
	unsigned int callDepth;
	unsigned int maxCallDepth;

	//execution utils
	unsigned int programCounter;

	//scope - block-level key/value pairs
	Toy_Scope* scope;
	Toy_Scope* scopeBase;

	//stack - immediate-level values only
	Toy_Stack* stack;

	//easy access to memory
	Toy_Bucket* memoryBucket;
	Toy_Bucket** parentBucketHandle;
} Toy_VM;

TOY_API void Toy_resetVM(Toy_VM* vm, bool preserveScope, bool preserveStack);

TOY_API void Toy_initVM(Toy_VM* vm); //creates memory
TOY_API void Toy_inheritVM(Toy_VM* parentVM, Toy_VM* subVM); //inherits scope bucket

void Toy_bindVM(Toy_VM* vm, unsigned char* bytecode, Toy_Scope* parentScope);
TOY_API unsigned int Toy_runVM(Toy_VM* vm);
TOY_API void Toy_freeVM(Toy_VM* vm);

TOY_API Toy_Value Toy_getReturnValueFromVM(Toy_VM* parentVM, Toy_VM* subVM);

//Arguments are borrowed; the returned value belongs to the caller.
union Toy_Function_t;
TOY_API Toy_Value Toy_callFunction(Toy_VM* vm, union Toy_Function_t* fn, unsigned int argc, Toy_Value* argv);
