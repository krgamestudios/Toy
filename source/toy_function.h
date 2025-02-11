#pragma once

#include "toy_common.h"

#include "toy_module.h"

typedef enum Toy_FunctionType {
	TOY_FUNCTION_MODULE,
	TOY_FUNCTION_NATIVE,
} Toy_FunctionType;

typedef union Toy_FunctionModule {
	Toy_FunctionType type;
	Toy_Module module;
} Toy_FunctionModule;

typedef union Toy_FunctionNative {
	Toy_FunctionType type;
	void* native; //TODO: replace with the native function pointer
} Toy_FunctionNative;

typedef union Toy_Function_t {
	Toy_FunctionType type;
	Toy_FunctionModule module;
	Toy_FunctionNative native;
} Toy_Function;

TOY_API Toy_Function* Toy_createModuleFunction(Toy_Bucket** bucketHandle, Toy_Module module);
