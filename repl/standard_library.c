#include "standard_library.h"
#include "toy_console_colors.h"

#include "toy_print.h"
#include "toy_scope.h"
#include "toy_stack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct CallbackPairs {
	const char* name;
	Toy_nativeCallback callback;
} CallbackPairs;

//example callbacks
void debug(Toy_VM* vm) {
	(void)vm;
	Toy_print("This function returns the integer '42' to the calling scope.");
	Toy_pushStack(&vm->stack, TOY_VALUE_FROM_INTEGER(42));
}

void identity(Toy_VM* vm) {
	//does nothing, but any arguements are left on the stack as results
	(void)vm;
}

void echo(Toy_VM* vm) {
	//pops one argument, and prints it
	Toy_Value value = Toy_popStack(&vm->stack);
	Toy_String* string = Toy_stringifyValue(&vm->memoryBucket, value);
	char* cstr = Toy_getStringRaw(string);

	Toy_print(cstr);

	free(cstr);
	Toy_freeString(string);
	Toy_freeValue(value);
}

CallbackPairs callbackPairs[] = {
	{"debug", debug},
	{"identity", identity},
	{"echo", echo},

	{NULL, NULL},
};

//exposed functions
void initStandardLibrary(Toy_VM* vm) {
	if (vm == NULL || vm->scope == NULL || vm->memoryBucket == NULL) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Can't initialize standard library, exiting\n" TOY_CC_RESET);
		exit(-1);
	}

	//declare each pair
	for (int i = 0; callbackPairs[i].name; i++) {
		//cheat
		Toy_String key = (Toy_String){
			.leaf = { ._padding = { .type = TOY_STRING_LEAF, .length = strlen(callbackPairs[i].name), .refCount = 1, .cachedHash = 0 }, .data = callbackPairs[i].name }
		};

		Toy_Function* fn = Toy_createFunctionFromCallback(&(vm->memoryBucket), callbackPairs[i].callback);

		Toy_declareScope(vm->scope, &key, TOY_VALUE_FUNCTION, TOY_VALUE_FROM_FUNCTION(fn), true);
	}
}
