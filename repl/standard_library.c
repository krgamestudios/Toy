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
static void answer(Toy_VM* vm, Toy_FunctionNative* self) {
	(void)vm;
	(void)self;
	Toy_print(TOY_CC_DEBUG "This function returns the integer '42' to the calling scope." TOY_CC_RESET);
	Toy_pushStack(&vm->stack, TOY_VALUE_FROM_INTEGER(42));
}

static void identity(Toy_VM* vm, Toy_FunctionNative* self) {
	//does nothing, but any arguements are left on the stack as results
	(void)vm;
	(void)self;
}

static void echo(Toy_VM* vm, Toy_FunctionNative* self) {
	(void)self;
	//pops one argument, and prints it
	Toy_Value value = Toy_popStack(&vm->stack);
	Toy_String* string = Toy_stringifyValue(&vm->memoryBucket, value);
	char* cstr = Toy_getStringRaw(string);

	Toy_print(cstr);

	free(cstr);
	Toy_freeString(string);
	Toy_freeValue(value);
}

static void next(Toy_VM* vm, Toy_FunctionNative* self) {
	//used by 'range'
	if (self->meta2 < self->meta1) {
		Toy_Value result = TOY_VALUE_FROM_INTEGER(self->meta2);
		Toy_pushStack(&vm->stack, result);
		self->meta2++;
	}
	else {
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
	}
}

static void range(Toy_VM* vm, Toy_FunctionNative* self) {
	(void)self;

	//one arg to represent the number of iterations
	Toy_Value value = Toy_popStack(&vm->stack);

	//check types
	if (!TOY_VALUE_IS_INTEGER(value)) {
		char buffer[256];
		snprintf(buffer, 256, "Expected Integer argument in 'range', found '%s'", Toy_getValueTypeAsCString(value.type));
		Toy_error(buffer);
		Toy_freeValue(value);
		return;
	}

	//make the callback
	Toy_Function* fn = Toy_createFunctionFromCallback(&vm->memoryBucket, next);
	fn->native.meta1 = TOY_VALUE_AS_INTEGER(value); //fake a closure
	fn->native.meta2 = 0; //counter

	Toy_Value result = TOY_VALUE_FROM_FUNCTION(fn);

	Toy_pushStack(&vm->stack, result);
}


CallbackPairs callbackPairs[] = {
	{"dbg_answer", answer},
	{"dbg_identity", identity},
	{"dbg_echo", echo},
	{"range", range},

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
		Toy_String* key = Toy_createStringLength(&vm->memoryBucket, callbackPairs[i].name, strlen(callbackPairs[i].name));
		Toy_Function* fn = Toy_createFunctionFromCallback(&(vm->memoryBucket), callbackPairs[i].callback);

		Toy_declareScope(vm->scope, key, TOY_VALUE_FUNCTION, TOY_VALUE_FROM_FUNCTION(fn), true);
	}
}
