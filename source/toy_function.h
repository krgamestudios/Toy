#pragma once

#include "toy_common.h"
#include "toy_bucket.h"
#include "toy_scope.h"

typedef enum Toy_FunctionType {
	TOY_FUNCTION_CUSTOM,
	TOY_FUNCTION_NATIVE,
} Toy_FunctionType;

typedef struct Toy_FunctionBytecode {
	Toy_FunctionType type;
	unsigned char* code;
	Toy_Scope* parentScope;
} Toy_FunctionBytecode;

typedef struct Toy_FunctionNative {
	Toy_FunctionType type;
	void* native; //TODO: replace with the native function pointer
} Toy_FunctionNative;

typedef union Toy_Function_t {
	Toy_FunctionType type;
	Toy_FunctionBytecode bytecode;
	Toy_FunctionNative native;
} Toy_Function;

TOY_API Toy_Function* Toy_createFunctionFromBytecode(Toy_Bucket** bucketHandle, unsigned char* bytecode, Toy_Scope* parentScope);

TOY_API void Toy_freeFunction(Toy_Function* fn);