#include "standard_library.h"
#include "toy_console_colors.h"
#include "toy_scope.h"
#include "toy_stack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

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
		snprintf(buffer, 256, "Invalid types '%s' and '%s' found in 'max()'", Toy_getValueTypeAsCString(Toy_unwrapValue(first).type), Toy_getValueTypeAsCString(Toy_unwrapValue(second).type));
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

static void std_floor(Toy_VM* vm, Toy_FunctionNative* self) {
	(void)self;

	Toy_Value value = Toy_popStack(&vm->stack);

	if (!TOY_VALUE_IS_INTEGER(value) && !TOY_VALUE_IS_FLOAT(value)) {
		char buffer[256];
		snprintf(buffer, 256, "Invalid type '%s' found in 'floor()'", Toy_getValueTypeAsCString(Toy_unwrapValue(value).type));
		Toy_error(buffer);

		Toy_freeValue(value);
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
		return;
	}

	//this only runs on floats, but converts them to integers
	if (TOY_VALUE_IS_FLOAT(value)) {
		double d = (double)TOY_VALUE_AS_FLOAT(value);
		d = floor(d);
		value = TOY_VALUE_FROM_INTEGER((int)d);
	}

	Toy_pushStack(&vm->stack, value);
}

static void std_ceil(Toy_VM* vm, Toy_FunctionNative* self) {
	(void)self;

	Toy_Value value = Toy_popStack(&vm->stack);

	if (!TOY_VALUE_IS_INTEGER(value) && !TOY_VALUE_IS_FLOAT(value)) {
		char buffer[256];
		snprintf(buffer, 256, "Invalid type '%s' found in 'ceil()'", Toy_getValueTypeAsCString(Toy_unwrapValue(value).type));
		Toy_error(buffer);

		Toy_freeValue(value);
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
		return;
	}

	//this only runs on floats, but converts them to integers
	if (TOY_VALUE_IS_FLOAT(value)) {
		double d = (double)TOY_VALUE_AS_FLOAT(value);
		d = ceil(d);
		value = TOY_VALUE_FROM_INTEGER((int)d);
	}

	Toy_pushStack(&vm->stack, value);
}

static void std_abs(Toy_VM* vm, Toy_FunctionNative* self) {
	(void)self;

	Toy_Value value = Toy_popStack(&vm->stack);

	if (!TOY_VALUE_IS_INTEGER(value) && !TOY_VALUE_IS_FLOAT(value)) {
		char buffer[256];
		snprintf(buffer, 256, "Invalid type '%s' found in 'abs()'", Toy_getValueTypeAsCString(Toy_unwrapValue(value).type));
		Toy_error(buffer);

		Toy_freeValue(value);
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
		return;
	}

	//invert negatives if able
	if (TOY_VALUE_IS_INTEGER(value) && TOY_VALUE_AS_INTEGER(value) < 0) {
		value = TOY_VALUE_FROM_INTEGER(-TOY_VALUE_AS_INTEGER(value));
	}
	else if (TOY_VALUE_IS_FLOAT(value) && TOY_VALUE_AS_FLOAT(value) < 0) {
		value = TOY_VALUE_FROM_FLOAT(-TOY_VALUE_AS_FLOAT(value));
	}

	Toy_pushStack(&vm->stack, value);
}

static void std_sign(Toy_VM* vm, Toy_FunctionNative* self) {
	(void)self;

	Toy_Value value = Toy_popStack(&vm->stack);

	if (!TOY_VALUE_IS_INTEGER(value) && !TOY_VALUE_IS_FLOAT(value)) {
		char buffer[256];
		snprintf(buffer, 256, "Invalid type '%s' found in 'sign()'", Toy_getValueTypeAsCString(Toy_unwrapValue(value).type));
		Toy_error(buffer);

		Toy_freeValue(value);
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
		return;
	}

	int result = 1; //start with a positive

	if (TOY_VALUE_IS_INTEGER(value) && TOY_VALUE_AS_INTEGER(value) < 0) {
		result = -1;
	}
	if (TOY_VALUE_IS_FLOAT(value) && TOY_VALUE_AS_FLOAT(value) < 0) {
		result = -1;
	}

	Toy_pushStack(&vm->stack, TOY_VALUE_FROM_INTEGER(result));
}

static void std_sqrt(Toy_VM* vm, Toy_FunctionNative* self) {
	(void)self;

	Toy_Value value = Toy_popStack(&vm->stack);

	if (!TOY_VALUE_IS_INTEGER(value) && !TOY_VALUE_IS_FLOAT(value)) {
		char buffer[256];
		snprintf(buffer, 256, "Invalid type '%s' found in 'sqrt()'", Toy_getValueTypeAsCString(Toy_unwrapValue(value).type));
		Toy_error(buffer);

		Toy_freeValue(value);
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
		return;
	}

	double d = 0;

	if (TOY_VALUE_IS_INTEGER(value)) d = (double)TOY_VALUE_AS_INTEGER(value);
	else if (TOY_VALUE_IS_FLOAT(value)) d = (double)TOY_VALUE_AS_FLOAT(value);

	//only works on non-negative values
	if (d < 0) {
		char buffer[256];
		snprintf(buffer, 256, "Can't get the sqrt of a negative number (found '%g')", d);
		Toy_error(buffer);

		Toy_freeValue(value);
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
		return;
	}

	//do the thing and push the result
	d = sqrt(d);

	Toy_pushStack(&vm->stack, TOY_VALUE_FROM_FLOAT((float)d));
}

//NOTE: 'srand()' alters the 'rand()' only within its own VM environment, so ensure the key is the same
#define RAND_NAME "rand"
static void std_rand(Toy_VM* vm, Toy_FunctionNative* self) {
	//quick and dirty RNG
	if (self->meta1 == 0) {
		self->meta1 = 21795;
	}

	self->meta1 = self->meta1 * 1664525 + 1013904223;

	Toy_pushStack(&vm->stack, TOY_VALUE_FROM_INTEGER(self->meta1));
}

static void std_srand(Toy_VM* vm, Toy_FunctionNative* self) {
	(void)self;

	Toy_Value seed = Toy_popStack(&vm->stack);

	if (!TOY_VALUE_IS_INTEGER(seed)) {
		char buffer[256];
		snprintf(buffer, 256, "Invalid argument type '%s' found in 'srand()'", Toy_getValueTypeAsCString(Toy_unwrapValue(seed).type));
		Toy_error(buffer);
		Toy_freeValue(seed);
		return;
	}

	//make the key
	Toy_String* key = Toy_createStringLength(&vm->memoryBucket, RAND_NAME, strlen(RAND_NAME));
	Toy_Value* value = Toy_accessScopeAsPointer(vm->scope, key);

	if (!TOY_VALUE_IS_FUNCTION(*value) || TOY_VALUE_AS_FUNCTION(*value)->type != TOY_FUNCTION_NATIVE) {
		char buffer[256];
		snprintf(buffer, 256, "Variable name '%s' is not a native function, so the seed can't be set", RAND_NAME);
		Toy_error(buffer);
		Toy_freeString(key);
		Toy_freeValue(seed);
		return;
	}

	//get the function and set the seed
	Toy_FunctionNative* nativePtr = &(TOY_VALUE_AS_FUNCTION(*value)->native);
	nativePtr->meta1 = TOY_VALUE_AS_INTEGER(seed);

	Toy_freeString(key);
}

static void std_time(Toy_VM* vm, Toy_FunctionNative* self) {
	//TODO: Fix the datatype in about 11 years.
	(void)self;

	int seconds = (int)time(NULL);
	Toy_pushStack(&vm->stack, TOY_VALUE_FROM_INTEGER(seconds));
}

CallbackPairs callbackPairs[] = {
	{"min", std_min},
	{"max", std_max},
	{"floor", std_floor},
	{"ceil", std_ceil},
	{"abs", std_abs},
	{"sign", std_sign},
	{"sqrt", std_sqrt},
	{RAND_NAME, std_rand},
	{"srand", std_srand},
	{"time", std_time},

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

		Toy_declareScope(&vm->memoryBucket, vm->scope, key, TOY_VALUE_FUNCTION, TOY_VALUE_FROM_FUNCTION(fn), true);
	}
}
