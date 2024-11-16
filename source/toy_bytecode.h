#pragma once

#include "toy_common.h"
#include "toy_ast.h"

typedef struct Toy_Bytecode {
	unsigned char* ptr;
	unsigned int capacity;
	unsigned int count;

	unsigned int moduleCount;
} Toy_Bytecode;

TOY_API Toy_Bytecode Toy_compileBytecode(Toy_Ast* ast);
TOY_API void Toy_freeBytecode(Toy_Bytecode bc);
