#pragma once

#include "toy_common.h"
#include "toy_scope.h"

//runtime module info
typedef struct Toy_Module {
	//closure support - points to parent scope
	Toy_Scope* parentScope;

	unsigned char* code;

	//extracted metadata
	// unsigned int codeCount; //NOTE: not used
	unsigned int jumpsCount;
	unsigned int paramCount;
	unsigned int dataCount;
	unsigned int subsCount;

	unsigned int codeAddr;
	unsigned int jumpsAddr;
	unsigned int paramAddr;
	unsigned int dataAddr;
	unsigned int subsAddr;
} Toy_Module;

TOY_API Toy_Module Toy_parseModule(unsigned char* ptr);
