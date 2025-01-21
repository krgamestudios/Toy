#pragma once

#include "toy_common.h"
#include "toy_ast.h"
#include "toy_module.h"

typedef struct Toy_ModuleBundle {
	unsigned char* ptr;
	unsigned int capacity;
	unsigned int count;
} Toy_ModuleBundle;

//create a bundle
TOY_API void Toy_initModuleBundle(Toy_ModuleBundle* bundle);
TOY_API void Toy_appendModuleBundle(Toy_ModuleBundle* bundle, Toy_Ast* ast);
TOY_API void Toy_freeModuleBundle(Toy_ModuleBundle* bundle);

//load module bundle with external data (makes an internal copy)
TOY_API void Toy_bindModuleBundle(Toy_ModuleBundle* bundle, unsigned char* ptr, unsigned int size);
TOY_API Toy_Module Toy_extractModuleFromBundle(Toy_ModuleBundle* bundle, unsigned char index);
