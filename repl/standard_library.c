#include "standard_library.h"
#include "toy_console_colors.h"

#include "toy_scope.h"
#include "toy_stack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct CallbackPairs {
	const char* name;
	Toy_nativeCallback callback;
} CallbackPairs;

//example of how to write and use C bindings
static void std_min(Toy_VM* vm, Toy_FunctionNative* self) {
	(void)self;

	//return the lesser of two values, or null on error
	Toy_Value first = Toy_popStack(&vm->stack);
	Toy_Value second = Toy_popStack(&vm->stack);

	//check types
	if ((!TOY_VALUE_IS_INTEGER(first) && !TOY_VALUE_IS_FLOAT(first)) || (!TOY_VALUE_IS_INTEGER(second) && !TOY_VALUE_IS_FLOAT(second))) {
		char buffer[256];
		snprintf(buffer, 256, "Invalid types '%s' and '%s' found in 'min()'", Toy_getValueTypeAsCString(Toy_unwrapValue(first).type), Toy_getValueTypeAsCString(Toy_unwrapValue(second).type));
		Toy_error(buffer);

		Toy_freeValue(first);
		Toy_freeValue(second);
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
		return;
	}

	//compare ints, or coerce ints into floats if needed
	if (TOY_VALUE_IS_INTEGER(first) && TOY_VALUE_IS_INTEGER(second)) {
		Toy_Value result = TOY_VALUE_FROM_INTEGER(TOY_VALUE_AS_INTEGER(first) < TOY_VALUE_AS_INTEGER(second) ? TOY_VALUE_AS_INTEGER(first) : TOY_VALUE_AS_INTEGER(second));
		Toy_pushStack(&vm->stack, result);
		return;
	}
	else if (TOY_VALUE_IS_INTEGER(first) && TOY_VALUE_IS_FLOAT(second)) {
		first = TOY_VALUE_FROM_FLOAT( (float)TOY_VALUE_AS_INTEGER(first) );
	}
	else if (TOY_VALUE_IS_FLOAT(first) && TOY_VALUE_IS_INTEGER(second)) {
		second = TOY_VALUE_FROM_FLOAT( (float)TOY_VALUE_AS_INTEGER(second) );
	}

	//finally, do the comparison on floats
	Toy_Value result = TOY_VALUE_FROM_FLOAT(TOY_VALUE_AS_FLOAT(first) < TOY_VALUE_AS_FLOAT(second) ? TOY_VALUE_AS_FLOAT(first) : TOY_VALUE_AS_FLOAT(second));
	Toy_pushStack(&vm->stack, result);
	//NOTE: not freeing scalar values does work, but only in narrow cases
}

static void std_max(Toy_VM* vm, Toy_FunctionNative* self) {
	(void)self;

	//return the lesser of two values, or null on error
	Toy_Value first = Toy_popStack(&vm->stack);
	Toy_Value second = Toy_popStack(&vm->stack);

	//check types
	if ((!TOY_VALUE_IS_INTEGER(first) && !TOY_VALUE_IS_FLOAT(first)) || (!TOY_VALUE_IS_INTEGER(second) && !TOY_VALUE_IS_FLOAT(second))) {
		char buffer[256];
		snprintf(buffer, 256, "Invalid types '%s' and '%s' found in 'min()'", Toy_getValueTypeAsCString(Toy_unwrapValue(first).type), Toy_getValueTypeAsCString(Toy_unwrapValue(second).type));
		Toy_error(buffer);

		Toy_freeValue(first);
		Toy_freeValue(second);
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
		return;
	}

	//compare ints, or coerce ints into floats if needed
	if (TOY_VALUE_IS_INTEGER(first) && TOY_VALUE_IS_INTEGER(second)) {
		Toy_Value result = TOY_VALUE_FROM_INTEGER(TOY_VALUE_AS_INTEGER(first) > TOY_VALUE_AS_INTEGER(second) ? TOY_VALUE_AS_INTEGER(first) : TOY_VALUE_AS_INTEGER(second));
		Toy_pushStack(&vm->stack, result);
		return;
	}
	else if (TOY_VALUE_IS_INTEGER(first) && TOY_VALUE_IS_FLOAT(second)) {
		first = TOY_VALUE_FROM_FLOAT( (float)TOY_VALUE_AS_INTEGER(first) );
	}
	else if (TOY_VALUE_IS_FLOAT(first) && TOY_VALUE_IS_INTEGER(second)) {
		second = TOY_VALUE_FROM_FLOAT( (float)TOY_VALUE_AS_INTEGER(second) );
	}

	//finally, do the comparison on floats
	Toy_Value result = TOY_VALUE_FROM_FLOAT(TOY_VALUE_AS_FLOAT(first) > TOY_VALUE_AS_FLOAT(second) ? TOY_VALUE_AS_FLOAT(first) : TOY_VALUE_AS_FLOAT(second));
	Toy_pushStack(&vm->stack, result);
	//NOTE: not freeing scalar values does work, but only in narrow cases
}

static void next(Toy_VM* vm, Toy_FunctionNative* self) {
	//used by 'std_range'
	if (self->meta2 < self->meta1) {
		Toy_Value result = TOY_VALUE_FROM_INTEGER(self->meta2);
		Toy_pushStack(&vm->stack, result);
		self->meta2++;
	}
	else {
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
	}
}

static void std_range(Toy_VM* vm, Toy_FunctionNative* self) {
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
	{"min", std_min},
	{"max", std_max},
	{"range", std_range},

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
