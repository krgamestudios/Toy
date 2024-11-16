#include "toy_bytecode.h"
#include "toy_console_colors.h"

#include "toy_routine.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//utils
static void expand(Toy_Bytecode* bc, unsigned int amount) {
	if (bc->count + amount > bc->capacity) {

		while (bc->count + amount > bc->capacity) { //expand as much as needed
			bc->capacity = bc->capacity < 8 ? 8 : bc->capacity * 2;
		}

		bc->ptr = realloc(bc->ptr, bc->capacity);

		if (bc->ptr == NULL) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to allocate a 'Toy_Bytecode' of %d capacity\n" TOY_CC_RESET, (int)(bc->capacity));
			exit(1);
		}
	}
}

static void emitByte(Toy_Bytecode* bc, unsigned char byte) {
	expand(bc, 1);
	bc->ptr[bc->count++] = byte;
}

//bytecode
static void writeBytecodeHeader(Toy_Bytecode* bc) {
	emitByte(bc, TOY_VERSION_MAJOR);
	emitByte(bc, TOY_VERSION_MINOR);
	emitByte(bc, TOY_VERSION_PATCH);

	//check strlen for the build string
	const char* build = Toy_private_version_build();
	size_t len = strlen(build) + 1;

	//BUGFIX: ensure the end of the header has 4-byte alignment
	if (len % 4 != 1) { //1 to fill the 4th byte above
		len += 4 - (len % 4) +1; //ceil
	}

	expand(bc, len);
	memcpy(bc->ptr + bc->count, build, len);
	bc->count += len;

	bc->ptr[bc->count] = '\0';
}

static void writeBytecodeBody(Toy_Bytecode* bc, Toy_Ast* ast) {
	//a 'module' is a routine that runs at the root-level of a file
	//since routines can be recursive, this distinction is important
	//eventually, the bytecode may support multiple modules packed into one file
	void* module = Toy_compileRoutine(ast);

	//don't try writing an empty module
	if (module == NULL) {
		return;
	}

	size_t len = (size_t)(((int*)module)[0]);

	expand(bc, len);
	memcpy(bc->ptr + bc->count, module, len);
	bc->count += len;
	bc->moduleCount++;
}

//exposed functions
Toy_Bytecode Toy_compileBytecode(Toy_Ast* ast) {
	//setup
	Toy_Bytecode bc;

	bc.ptr = NULL;
	bc.capacity = 0;
	bc.count = 0;

	bc.moduleCount = 0;

	//build
	writeBytecodeHeader(&bc);
	writeBytecodeBody(&bc, ast); //TODO: implement module packing

	return bc;
}

void Toy_freeBytecode(Toy_Bytecode bc) {
	free(bc.ptr);
}
