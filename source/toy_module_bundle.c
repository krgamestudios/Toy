#include "toy_module_bundle.h"
#include "toy_console_colors.h"

#include "toy_module_compiler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//utils
static void expand(Toy_ModuleBundle* bundle, unsigned int amount) {
	if (bundle->count + amount > bundle->capacity) {
		bundle->capacity = 8; //DON'T bitshift zero

		while (bundle->count + amount > bundle->capacity) { //expand as much as needed
			bundle->capacity <<= 2;
		}

		bundle->ptr = realloc(bundle->ptr, bundle->capacity);

		if (bundle->ptr == NULL) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to allocate a 'Toy_ModuleBundle' of %d capacity\n" TOY_CC_RESET, (int)(bundle->capacity));
			exit(1);
		}
	}
}

static void emitByte(Toy_ModuleBundle* bundle, unsigned char byte) {
	expand(bundle, 1);
	bundle->ptr[bundle->count++] = byte;
}

static void writeModuleBundleHeader(Toy_ModuleBundle* bundle) {
	emitByte(bundle, TOY_VERSION_MAJOR);
	emitByte(bundle, TOY_VERSION_MINOR);
	emitByte(bundle, TOY_VERSION_PATCH);
	emitByte(bundle, 0); //module count

	//get the build string
	const char* build = Toy_private_version_build();
	size_t len = strlen(build) + 1; //includes null

	//emit the build string
	expand(bundle, len);
	strncpy((char*)(bundle->ptr + bundle->count), build, len);
	bundle->count += len;

	//align the count
	bundle->count = (bundle->count + 3) & ~3;
}

static int validateModuleBundleHeader(Toy_ModuleBundle* bundle) {
	if (bundle->ptr[0] != TOY_VERSION_MAJOR || bundle->ptr[1] > TOY_VERSION_MINOR) {
		return -1;
	}

	if (bundle->ptr[2] != TOY_VERSION_PATCH) {
		return 1;
	}

	if (strcmp((char*)(bundle->ptr + 4), TOY_VERSION_BUILD) != 0) {
		return 2;
	}

	return 0;
}

//exposed functions
void Toy_initModuleBundle(Toy_ModuleBundle* bundle) {
	bundle->ptr = NULL;
	bundle->capacity = 0;
	bundle->count = 0;
}

void Toy_appendModuleBundle(Toy_ModuleBundle* bundle, Toy_Ast* ast) {
	//probably some inefficincies in memory usage here
	if (bundle->capacity == 0) {
		writeModuleBundleHeader(bundle); //TODO: update the header?
	}

	//increment the module count
	if (bundle->ptr[3] < 255) {
		bundle->ptr[3]++;
	}
	else {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Too many modules in a bundle\n" TOY_CC_RESET);
		exit(-1);
	}

	void* module = Toy_compileModule(ast);

	//don't try writing an empty module
	if (module == NULL) {
		return;
	}

	//write the module to the bundle
	size_t len = (size_t)(((int*)module)[0]);

	expand(bundle, len);
	memcpy(bundle->ptr + bundle->count, module, len);
	bundle->count += len;

	free(module);
}

void Toy_freeModuleBundle(Toy_ModuleBundle* bundle) {
	free(bundle->ptr);
	Toy_initModuleBundle(bundle);
}

void Toy_bindModuleBundle(Toy_ModuleBundle* bundle, unsigned char* ptr, unsigned int size) {
	if (bundle == NULL) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Can't bind a NULL bundle\n" TOY_CC_RESET);
		exit(-1);
	}

	if (bundle->ptr != NULL || bundle->capacity != 0 || bundle->count != 0) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Can't bind a bundle with pre-existing contents\n" TOY_CC_RESET);
		exit(-1);
	}

	//copy
	expand(bundle, size);

	memcpy(bundle->ptr, ptr, size);
	bundle->count = size;

	//URGENT: test this
	int valid = validateModuleBundleHeader(bundle);

	if (valid < 0) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Wrong version info found in module header: expected %d.%d.%d.%s found %d.%d.%d.%s, exiting\n" TOY_CC_RESET, TOY_VERSION_MAJOR, TOY_VERSION_MINOR, TOY_VERSION_PATCH, TOY_VERSION_BUILD, bundle->ptr[0], bundle->ptr[1], bundle->ptr[2], (char*)(bundle->ptr + 4));
		exit(valid);
	}

	if (valid > 0) {
		fprintf(stderr, TOY_CC_WARN "WARNING: Wrong version info found in module header: expected %d.%d.%d.%s found %d.%d.%d.%s, continuing\n" TOY_CC_RESET, TOY_VERSION_MAJOR, TOY_VERSION_MINOR, TOY_VERSION_PATCH, TOY_VERSION_BUILD, bundle->ptr[0], bundle->ptr[1], bundle->ptr[2], (char*)(bundle->ptr + 4));
	}
}

Toy_Module Toy_extractModuleFromBundle(Toy_ModuleBundle* bundle, unsigned char index) {
	if (bundle == NULL) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Can't extract from a NULL bundle\n" TOY_CC_RESET);
		return (Toy_Module){ 0 };
	}

	if (bundle->ptr == NULL) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Can't extract from an empty bundle\n" TOY_CC_RESET);
		return (Toy_Module){ 0 };
	}

	//yes, it's a bit awkward
	char* buildPtr = (char*)(bundle->ptr + 4);
	int buildLen = strlen(buildPtr);
	buildLen = (buildLen + 3) & ~3;

	//first module's start position
	unsigned char* moduleHead = bundle->ptr + 4 + buildLen;

	for (unsigned char i = 0; i < index; i++) {
		unsigned int size = *((int*)(moduleHead));
		moduleHead += size;
	}

	//read in the module
	return Toy_parseModule(moduleHead);
}
