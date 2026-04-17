#pragma once

#include "toy_common.h"
#include "toy_bucket.h"
#include "toy_scope.h"
#include "toy_vm.h"

//forward declare
struct Toy_VM;
typedef void (*Toy_nativeCallback)(struct Toy_VM*);

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
	Toy_nativeCallback callback;
} Toy_FunctionNative;

typedef union Toy_Function_t {
	Toy_FunctionType type;
	Toy_FunctionBytecode bytecode;
	Toy_FunctionNative native;
} Toy_Function;

TOY_API Toy_Function* Toy_createFunctionFromBytecode(Toy_Bucket** bucketHandle, unsigned char* bytecode, Toy_Scope* parentScope);
TOY_API Toy_Function* Toy_createFunctionFromCallback(Toy_Bucket** bucketHandle, Toy_nativeCallback callback);

TOY_API Toy_Function* Toy_copyFunction(Toy_Bucket** bucketHandle, Toy_Function* fn);
TOY_API void Toy_freeFunction(Toy_Function* fn);