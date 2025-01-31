#include "toy_vm.h"
#include "toy_console_colors.h"

#include "toy_print.h"
#include "toy_opcodes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//utilities
#define READ_BYTE(vm) \
	vm->code[vm->programCounter++]

#define READ_UNSIGNED_INT(vm) \
	*((unsigned int*)(vm->code + readPostfixUtil(&(vm->programCounter), 4)))

#define READ_INT(vm) \
	*((int*)(vm->code + readPostfixUtil(&(vm->programCounter), 4)))

#define READ_FLOAT(vm) \
	*((float*)(vm->code + readPostfixUtil(&(vm->programCounter), 4)))

static inline int readPostfixUtil(unsigned int* ptr, int amount) {
	int ret = *ptr;
	*ptr += amount;
	return ret;
}

static inline void fixAlignment(Toy_VM* vm) {
	//NOTE: It's a tilde, not a negative sign
	vm->programCounter = (vm->programCounter + 3) & ~3;
}

//instruction handlers
static void processRead(Toy_VM* vm) {
	Toy_ValueType type = READ_BYTE(vm);

	Toy_Value value = TOY_VALUE_FROM_NULL();

	switch(type) {
		case TOY_VALUE_NULL: {
			//No-op
			break;
		}

		case TOY_VALUE_BOOLEAN: {
			value = TOY_VALUE_FROM_BOOLEAN((bool)READ_BYTE(vm));
			break;
		}

		case TOY_VALUE_INTEGER: {
			fixAlignment(vm);
			value = TOY_VALUE_FROM_INTEGER(READ_INT(vm));
			break;
		}

		case TOY_VALUE_FLOAT: {
			fixAlignment(vm);
			value = TOY_VALUE_FROM_FLOAT(READ_FLOAT(vm));
			break;
		}

		case TOY_VALUE_STRING: {
			enum Toy_StringType stringType = READ_BYTE(vm);
			int len = (int)READ_BYTE(vm); //only needed for name strings

			//grab the jump as an integer
			unsigned int jump = *((int*)(vm->code + vm->jumpsAddr + READ_INT(vm)));

			//jumps are relative to the data address
			char* cstring = (char*)(vm->code + vm->dataAddr + jump);

			//build a string from the data section
			if (stringType == TOY_STRING_LEAF) {
				value = TOY_VALUE_FROM_STRING(Toy_createString(&vm->stringBucket, cstring));
			}
			else if (stringType == TOY_STRING_NAME) {
				Toy_ValueType valueType = TOY_VALUE_UNKNOWN;

				value = TOY_VALUE_FROM_STRING(Toy_createNameStringLength(&vm->stringBucket, cstring, len, valueType, false));
			}
			else {
				Toy_error("Invalid string type found in opcode read");
			}

			break;
		}

		case TOY_VALUE_ARRAY: {
			fixAlignment(vm);

			//the number of values to read from the stack
			unsigned int count = (unsigned int)READ_INT(vm);
			unsigned int capacity = count > TOY_ARRAY_INITIAL_CAPACITY ? count : TOY_ARRAY_INITIAL_CAPACITY;

			//neat trick to find the next power of two, inclusive (restriction of the array system)
			capacity--;
			capacity |= capacity >> 1;
			capacity |= capacity >> 2;
			capacity |= capacity >> 4;
			capacity |= capacity >> 8;
			capacity |= capacity >> 16;
			capacity++;

			//create the array and read in the values
			Toy_Array* array = Toy_resizeArray(NULL, capacity);
			array->capacity = capacity;
			array->count = count;

			for (int i = count - 1; i >= 0; i--) { //read in backwards from the stack
				array->data[i] = Toy_popStack(&vm->stack);
			}

			//finished
			value = TOY_VALUE_FROM_ARRAY(array);

			break;
		}

		case TOY_VALUE_TABLE: {
			fixAlignment(vm);

			//the number of values to read from the stack
			unsigned int count = (unsigned int)READ_INT(vm);

			//capacity covers keys AND values
			unsigned int capacity = count / 2;
			capacity = capacity > TOY_TABLE_INITIAL_CAPACITY ? capacity : TOY_TABLE_INITIAL_CAPACITY;

			//neat trick to find the next power of two, inclusive (restriction of the table system)
			capacity--;
			capacity |= capacity >> 1;
			capacity |= capacity >> 2;
			capacity |= capacity >> 4;
			capacity |= capacity >> 8;
			capacity |= capacity >> 16;
			capacity++;

			//create the table and read in the key-values
			Toy_Table* table = Toy_private_adjustTableCapacity(NULL, capacity);

			//read in backwards from the stack
			for (unsigned int i = 0; i < count / 2; i++) {
				Toy_Value v = Toy_popStack(&vm->stack);
				Toy_Value k = Toy_popStack(&vm->stack);

				Toy_insertTable(&table, k, v);
			}

			//finished
			value = TOY_VALUE_FROM_TABLE(table);

			break;
		}

		case TOY_VALUE_FUNCTION: {
			//
			// break;
		}

		case TOY_VALUE_OPAQUE: {
			//
			// break;
		}

		case TOY_VALUE_ANY: {
			//
			// break;
		}

		case TOY_VALUE_UNKNOWN: {
			//
			// break;
		}

		default:
			fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid value type %d found, exiting\n" TOY_CC_RESET, type);
			exit(-1);
	}

	//push onto the stack
	Toy_pushStack(&vm->stack, value);

	//leave the counter in a good spot
	fixAlignment(vm);
}

static void processDeclare(Toy_VM* vm) {
	Toy_ValueType type = READ_BYTE(vm); //variable type
	unsigned int len = READ_BYTE(vm); //name length
	bool constant = READ_BYTE(vm); //constness

	//grab the jump
	unsigned int jump = *(unsigned int*)(vm->code + vm->jumpsAddr + READ_INT(vm));

	//grab the data
	char* cstring = (char*)(vm->code + vm->dataAddr + jump);

	//build the name string
	Toy_String* name = Toy_createNameStringLength(&vm->stringBucket, cstring, len, type, constant);

	//get the value
	Toy_Value value = Toy_popStack(&vm->stack);

	//declare it
	Toy_declareScope(vm->scope, name, value);

	//cleanup
	Toy_freeString(name);
}

static void processAssign(Toy_VM* vm) {
	//get the value & name
	Toy_Value value = Toy_popStack(&vm->stack);
	Toy_Value name = Toy_popStack(&vm->stack);

	//check name string type
	if (!TOY_VALUE_IS_STRING(name) || TOY_VALUE_AS_STRING(name)->info.type != TOY_STRING_NAME) {
		Toy_error("Invalid assignment target");
		Toy_freeValue(name);
		Toy_freeValue(value);
		return;
	}

	//assign it
	Toy_assignScope(vm->scope, TOY_VALUE_AS_STRING(name), value); //scope now owns the value, doesn't need to be freed

	//in case of chaining, leave a copy on the stack
	bool chainedAssignment = READ_BYTE(vm);
	if (chainedAssignment) {
		Toy_pushStack(&vm->stack, Toy_copyValue(value));
	}

	//cleanup
	Toy_freeValue(name);
}

static void processAssignCompound(Toy_VM* vm) {
	//get the value, key, target
	Toy_Value value = Toy_popStack(&vm->stack);
	Toy_Value key = Toy_popStack(&vm->stack);
	Toy_Value target = Toy_popStack(&vm->stack);

	//shake out variable names
	if (TOY_VALUE_IS_STRING(target) && TOY_VALUE_AS_STRING(target)->info.type == TOY_STRING_NAME) {
		Toy_Value* valuePtr = Toy_accessScopeAsPointer(vm->scope, TOY_VALUE_AS_STRING(target));
		Toy_freeValue(target);
		if (valuePtr == NULL) {
			return;
		}
		target = TOY_REFERENCE_FROM_POINTER(valuePtr);
	}

	//assign based on target's type
	if (TOY_VALUE_IS_ARRAY(target)) {
		if (TOY_VALUE_IS_INTEGER(key) != true) {
			Toy_error("Bad key type for assignment target");
			Toy_freeValue(target);
			Toy_freeValue(key);
			Toy_freeValue(value);
			return;
		}

		Toy_Array* array = TOY_VALUE_AS_ARRAY(target);
		int index = TOY_VALUE_AS_INTEGER(key);

		//bounds check
		if (index < 0 || (unsigned int)index >= array->count) {
			Toy_error("Index of assignment target out of bounds");
			Toy_freeValue(target);
			Toy_freeValue(key);
			Toy_freeValue(value);
			return;
		}

		//set the value
		array->data[index] = Toy_copyValue(Toy_unwrapValue(value));

		//in case of chaining, leave a copy on the stack
		bool chainedAssignment = READ_BYTE(vm);
		if (chainedAssignment) {
			Toy_pushStack(&vm->stack, Toy_copyValue(value));
		}

		//cleanup
		Toy_freeValue(value);
	}

	else if (TOY_VALUE_IS_TABLE(target)) {
		Toy_Table* table = TOY_VALUE_AS_TABLE(target);

		//set the value
		Toy_insertTable(&table, Toy_copyValue(Toy_unwrapValue(key)), Toy_copyValue(Toy_unwrapValue(value)));

		//in case of chaining, leave a copy on the stack
		bool chainedAssignment = READ_BYTE(vm);
		if (chainedAssignment) {
			Toy_pushStack(&vm->stack, Toy_copyValue(value));
		}

		//cleanup
		Toy_freeValue(value);
	}

	else {
		Toy_error("Invalid assignment target");
		Toy_freeValue(target);
		Toy_freeValue(key);
		Toy_freeValue(value);
		return;
	}
}

static void processAccess(Toy_VM* vm) {
	Toy_Value name = Toy_popStack(&vm->stack);

	//check name string type
	if (!TOY_VALUE_IS_STRING(name) || TOY_VALUE_AS_STRING(name)->info.type != TOY_STRING_NAME) {
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
		Toy_error("Invalid access target");
		return;
	}

	//find the value
	Toy_Value* valuePtr = Toy_accessScopeAsPointer(vm->scope, TOY_VALUE_AS_STRING(name));

	if (valuePtr == NULL) {
		Toy_freeValue(name);
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
		return;
	}

	//in the event of a certain subset of types, create references instead (these should only exist on the stack)
	if (TOY_VALUE_IS_REFERENCE(*valuePtr) || TOY_VALUE_IS_ARRAY(*valuePtr) || TOY_VALUE_IS_TABLE(*valuePtr)) {
		Toy_Value ref = TOY_REFERENCE_FROM_POINTER(valuePtr);
		Toy_pushStack(&vm->stack, ref);
	}

	else {
		Toy_pushStack(&vm->stack, Toy_copyValue(*valuePtr));
	}

	//cleanup
	Toy_freeValue(name);
}

static void processDuplicate(Toy_VM* vm) {
	Toy_Value value = Toy_copyValue(Toy_peekStack(&vm->stack));
	Toy_pushStack(&vm->stack, value);

	//check for compound assignments
	Toy_OpcodeType squeezed = READ_BYTE(vm);
	if (squeezed == TOY_OPCODE_ACCESS) {
		processAccess(vm);
	}
}

static void processEliminate(Toy_VM* vm) {
	//discard the stack top
	Toy_Value value = Toy_popStack(&vm->stack);
	Toy_freeValue(value);
}

static void processArithmetic(Toy_VM* vm, Toy_OpcodeType opcode) {
	Toy_Value right = Toy_popStack(&vm->stack);
	Toy_Value left = Toy_popStack(&vm->stack);

	//check types
	if ((!TOY_VALUE_IS_INTEGER(left) && !TOY_VALUE_IS_FLOAT(left)) || (!TOY_VALUE_IS_INTEGER(right) && !TOY_VALUE_IS_FLOAT(right))) {
		char buffer[256];
		snprintf(buffer, 256, "Invalid types '%s' and '%s' passed in arithmetic", Toy_private_getValueTypeAsCString(left.type), Toy_private_getValueTypeAsCString(right.type));
		Toy_error(buffer);

		Toy_freeValue(left);
		Toy_freeValue(right);
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
		return;
	}

	//check for divide by zero
	if (opcode == TOY_OPCODE_DIVIDE || opcode == TOY_OPCODE_MODULO) {
		if ((TOY_VALUE_IS_INTEGER(right) && TOY_VALUE_AS_INTEGER(right) == 0) || (TOY_VALUE_IS_FLOAT(right) && TOY_VALUE_AS_FLOAT(right) == 0)) {
			Toy_error("Can't divide or modulo by zero");
			Toy_freeValue(left);
			Toy_freeValue(right);
			Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
			return;
		}
	}

	//check for modulo by a float
	if (opcode == TOY_OPCODE_MODULO && TOY_VALUE_IS_FLOAT(right)) {
		Toy_error("Can't modulo by a float");
		Toy_freeValue(left);
		Toy_freeValue(right);
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
		return;
	}

	//coerce ints into floats if needed
	if (TOY_VALUE_IS_INTEGER(left) && TOY_VALUE_IS_FLOAT(right)) {
		left = TOY_VALUE_FROM_FLOAT( (float)TOY_VALUE_AS_INTEGER(left) );
	}
	else
	if (TOY_VALUE_IS_FLOAT(left) && TOY_VALUE_IS_INTEGER(right)) {
		right = TOY_VALUE_FROM_FLOAT( (float)TOY_VALUE_AS_INTEGER(right) );
	}

	//apply operation
	Toy_Value result = TOY_VALUE_FROM_NULL();

	if (opcode == TOY_OPCODE_ADD) {
		result = TOY_VALUE_IS_FLOAT(left) ? TOY_VALUE_FROM_FLOAT( TOY_VALUE_AS_FLOAT(left) + TOY_VALUE_AS_FLOAT(right)) : TOY_VALUE_FROM_INTEGER( TOY_VALUE_AS_INTEGER(left) + TOY_VALUE_AS_INTEGER(right) );
	}
	else if (opcode == TOY_OPCODE_SUBTRACT) {
		result = TOY_VALUE_IS_FLOAT(left) ? TOY_VALUE_FROM_FLOAT( TOY_VALUE_AS_FLOAT(left) - TOY_VALUE_AS_FLOAT(right)) : TOY_VALUE_FROM_INTEGER( TOY_VALUE_AS_INTEGER(left) - TOY_VALUE_AS_INTEGER(right) );
	}
	else if (opcode == TOY_OPCODE_MULTIPLY) {
		result = TOY_VALUE_IS_FLOAT(left) ? TOY_VALUE_FROM_FLOAT( TOY_VALUE_AS_FLOAT(left) * TOY_VALUE_AS_FLOAT(right)) : TOY_VALUE_FROM_INTEGER( TOY_VALUE_AS_INTEGER(left) * TOY_VALUE_AS_INTEGER(right) );
	}
	else if (opcode == TOY_OPCODE_DIVIDE) {
		result = TOY_VALUE_IS_FLOAT(left) ? TOY_VALUE_FROM_FLOAT( TOY_VALUE_AS_FLOAT(left) / TOY_VALUE_AS_FLOAT(right)) : TOY_VALUE_FROM_INTEGER( TOY_VALUE_AS_INTEGER(left) / TOY_VALUE_AS_INTEGER(right) );
	}
	else if (opcode == TOY_OPCODE_MODULO) {
		result = TOY_VALUE_FROM_INTEGER( TOY_VALUE_AS_INTEGER(left) % TOY_VALUE_AS_INTEGER(right) );
	}
	else {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid opcode %d passed to processArithmetic, exiting\n" TOY_CC_RESET, opcode);
		exit(-1);
	}

	//finally
	Toy_pushStack(&vm->stack, result);

	//check for compound assignments
	Toy_OpcodeType squeezed = READ_BYTE(vm);
	if (squeezed == TOY_OPCODE_ASSIGN) {
		processAssign(vm);
	}
}

static void processComparison(Toy_VM* vm, Toy_OpcodeType opcode) {
	Toy_Value right = Toy_popStack(&vm->stack);
	Toy_Value left = Toy_popStack(&vm->stack);

	//most things can be equal, so handle it separately
	if (opcode == TOY_OPCODE_COMPARE_EQUAL) {
		bool equal = Toy_checkValuesAreEqual(left, right);

		//equality has an optional "negate" opcode within it's word
		if (READ_BYTE(vm) != TOY_OPCODE_NEGATE) {
			Toy_pushStack(&vm->stack, TOY_VALUE_FROM_BOOLEAN(equal) );
		}
		else {
			Toy_pushStack(&vm->stack, TOY_VALUE_FROM_BOOLEAN(!equal) );
		}

		Toy_freeValue(left);
		Toy_freeValue(right);
		return;
	}

	if (Toy_checkValuesAreComparable(left, right) != true) {
		char buffer[256];
		snprintf(buffer, 256, "Can't compare value types '%s' and '%s'", Toy_private_getValueTypeAsCString(left.type), Toy_private_getValueTypeAsCString(right.type));
		Toy_error(buffer);

		Toy_freeValue(left);
		Toy_freeValue(right);
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
		return;
	}

	//get the comparison
	int comparison = Toy_compareValues(left, right);

	//push the result of the comparison as a boolean, based on the opcode
	if (opcode == TOY_OPCODE_COMPARE_LESS && comparison < 0) {
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_BOOLEAN(true));
	}
	else if (opcode == TOY_OPCODE_COMPARE_LESS_EQUAL && (comparison < 0 || comparison == 0)) {
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_BOOLEAN(true));
	}
	else if (opcode == TOY_OPCODE_COMPARE_GREATER && comparison > 0) {
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_BOOLEAN(true));
	}
	else if (opcode == TOY_OPCODE_COMPARE_GREATER_EQUAL && (comparison > 0 || comparison == 0)) {
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_BOOLEAN(true));
	}

	//if all else failed, then it's not true
	else {
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_BOOLEAN(false));
	}

	Toy_freeValue(left);
	Toy_freeValue(right);
}

static void processLogical(Toy_VM* vm, Toy_OpcodeType opcode) {
	if (opcode == TOY_OPCODE_AND) {
		Toy_Value right = Toy_popStack(&vm->stack);
		Toy_Value left = Toy_popStack(&vm->stack);

		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_BOOLEAN( Toy_checkValueIsTruthy(left) && Toy_checkValueIsTruthy(right) ));
	}
	else if (opcode == TOY_OPCODE_OR) {
		Toy_Value right = Toy_popStack(&vm->stack);
		Toy_Value left = Toy_popStack(&vm->stack);

		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_BOOLEAN( Toy_checkValueIsTruthy(left) || Toy_checkValueIsTruthy(right) ));
	}
	else if (opcode == TOY_OPCODE_TRUTHY) {
		Toy_Value top = Toy_popStack(&vm->stack);

		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_BOOLEAN( Toy_checkValueIsTruthy(top) ));
	}
	else if (opcode == TOY_OPCODE_NEGATE) {
		Toy_Value top = Toy_popStack(&vm->stack); //bad values are filtered by the parser

		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_BOOLEAN( !Toy_checkValueIsTruthy(top) ));
	}
	else {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid opcode %d passed to processLogical, exiting\n" TOY_CC_RESET, opcode);
		exit(-1);
	}
}

static void processJump(Toy_VM* vm) {
	Toy_OpParamJumpType type = READ_BYTE(vm);
	Toy_OpParamJumpConditional cond = READ_BYTE(vm);
	fixAlignment(vm);

	//assume the param is a signed integer
	int param = READ_INT(vm);

	//should we jump?
	switch(cond) {
		case TOY_OP_PARAM_JUMP_ALWAYS:
			break;

		case TOY_OP_PARAM_JUMP_IF_TRUE: {
			Toy_Value value = Toy_popStack(&vm->stack);
			if (Toy_checkValueIsTruthy(value) == true) {
				Toy_freeValue(value);
				break;
			}

			Toy_freeValue(value);
			return;
		}

		case TOY_OP_PARAM_JUMP_IF_FALSE: {
			Toy_Value value = Toy_popStack(&vm->stack);
			if (Toy_checkValueIsTruthy(value) != true) {
				Toy_freeValue(value);
				break;
			}

			Toy_freeValue(value);
			return;
		}
	}

	//do the jump
	switch(type) {
		case TOY_OP_PARAM_JUMP_ABSOLUTE:
			vm->programCounter = vm->codeAddr + param;
			return;

		case TOY_OP_PARAM_JUMP_RELATIVE:
			vm->programCounter += param;
			return;
	}
}

static void processEscape(Toy_VM* vm) {
	fixAlignment(vm);

	int addr = READ_INT(vm); //where to go
	int diff = READ_INT(vm); //what to do

	vm->programCounter += addr;

	while (diff > 0 && vm->scope != NULL) {
		vm->scope = Toy_popScope(vm->scope);
		diff--;
	}
}

static void processAssert(Toy_VM* vm) {
	unsigned int count = READ_BYTE(vm);

	Toy_Value value = TOY_VALUE_FROM_NULL();
	Toy_Value message = TOY_VALUE_FROM_NULL();

	//determine the args
	if (count == 1) {
		message = TOY_VALUE_FROM_STRING(Toy_createString(&vm->stringBucket, "assertion failed")); //TODO: needs a better default message
		value = Toy_popStack(&vm->stack);
	}
	else if (count == 2) {
		message = Toy_popStack(&vm->stack);
		value = Toy_popStack(&vm->stack);
	}
	else {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid assert argument count %d found, exiting\n" TOY_CC_RESET, (int)count);
		exit(-1);
	}

	//do the check
	if (TOY_VALUE_IS_NULL(value) || Toy_checkValueIsTruthy(value) != true) {
		//on a failure, print the message
		Toy_String* string = Toy_stringifyValue(&vm->stringBucket, message);
		char* buffer = Toy_getStringRawBuffer(string);

		Toy_assertFailure(buffer);

		free(buffer);
		Toy_freeString(string);
		return;
	}

	//cleanup
	Toy_freeValue(value);
	Toy_freeValue(message);
}

static void processPrint(Toy_VM* vm) {
	//print the value on top of the stack, popping it
	Toy_Value value = Toy_popStack(&vm->stack);
	Toy_String* string = Toy_stringifyValue(&vm->stringBucket, value);
	char* buffer = Toy_getStringRawBuffer(string); //TODO: check string type to skip this call

	Toy_print(buffer);

	free(buffer);
	Toy_freeString(string);
	Toy_freeValue(value);
}

static void processConcat(Toy_VM* vm) {
	Toy_Value right = Toy_popStack(&vm->stack);
	Toy_Value left = Toy_popStack(&vm->stack);

	if (!TOY_VALUE_IS_STRING(left) || !TOY_VALUE_IS_STRING(right)) {
		Toy_error("Failed to concatenate a value that is not a string");
		Toy_freeValue(left);
		Toy_freeValue(right);
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
		return;
	}

	//all good
	Toy_String* result = Toy_concatStrings(&vm->stringBucket, TOY_VALUE_AS_STRING(left), TOY_VALUE_AS_STRING(right));
	Toy_pushStack(&vm->stack, TOY_VALUE_FROM_STRING(result));
}

static void processIndex(Toy_VM* vm) {
	unsigned char count = READ_BYTE(vm); //value[index, length] ; 1[2, 3]

	Toy_Value value = TOY_VALUE_FROM_NULL();
	Toy_Value index = TOY_VALUE_FROM_NULL();
	Toy_Value length = TOY_VALUE_FROM_NULL();

	if (count == 3) {
		length = Toy_popStack(&vm->stack);
		index = Toy_popStack(&vm->stack);
		value = Toy_popStack(&vm->stack);
	}
	else if (count == 2) {
		index = Toy_popStack(&vm->stack);
		value = Toy_popStack(&vm->stack);
	}
	else {
		Toy_error("Incorrect number of elements found in index");
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
		return;
	}

	//process based on value's type
	if (TOY_VALUE_IS_STRING(value)) {
		//type checks
		if (!TOY_VALUE_IS_INTEGER(index)) {
			Toy_error("Failed to index a string");
			Toy_freeValue(value);
			Toy_freeValue(index);
			Toy_freeValue(length);
			Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
			return;
		}

		if (!(TOY_VALUE_IS_NULL(length) || TOY_VALUE_IS_INTEGER(length))) {
			Toy_error("Failed to index-length a string");
			Toy_freeValue(value);
			Toy_freeValue(index);
			Toy_freeValue(length);
			Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
			return;
		}

		//extract values
		int i = TOY_VALUE_AS_INTEGER(index);
		int l = TOY_VALUE_IS_INTEGER(length) ? TOY_VALUE_AS_INTEGER(length) : 1;
		Toy_String* str = TOY_VALUE_AS_STRING(value);

		//check indexing is within bounds
		if ( (i < 0 || (unsigned int)i >= str->info.length) || (i+l <= 0 || (unsigned int)(i+l) > str->info.length)) {
			Toy_error("String index is out of bounds");
			Toy_freeValue(value);
			Toy_freeValue(index);
			Toy_freeValue(length);
			Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
			return;
		}

		//extract string
		Toy_String* result = NULL;

		//extract cstring, based on type
		if (str->info.type == TOY_STRING_LEAF) {
			const char* cstr = str->leaf.data;
			result = Toy_createStringLength(&vm->stringBucket, cstr + i, l);
		}
		else if (str->info.type == TOY_STRING_NODE) {
			char* cstr = Toy_getStringRawBuffer(str);
			result = Toy_createStringLength(&vm->stringBucket, cstr + i, l);
			free(cstr);
		}
		else {
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unknown string type found in processIndex, exiting\n" TOY_CC_RESET);
			exit(-1);
		}

		//finally
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_STRING(result));
	}

	else if (TOY_VALUE_IS_ARRAY(value)) {
		//type checks
		if (!TOY_VALUE_IS_INTEGER(index)) {
			Toy_error("Failed to index an array");
			Toy_freeValue(value);
			Toy_freeValue(index);
			Toy_freeValue(length);
			Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
			return;
		}

		if (!(TOY_VALUE_IS_NULL(length) || TOY_VALUE_IS_INTEGER(length))) {
			Toy_error("Failed to index-length an array");
			Toy_freeValue(value);
			Toy_freeValue(index);
			Toy_freeValue(length);
			Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
			return;
		}

		//extract values
		int i = TOY_VALUE_AS_INTEGER(index);
		int l = TOY_VALUE_IS_INTEGER(length) ? TOY_VALUE_AS_INTEGER(length) : 1;
		Toy_Array* array = TOY_VALUE_AS_ARRAY(value);

		//check indexing is within bounds
		if ( (i < 0 || (unsigned int)i >= array->count) || (i+l <= 0 || (unsigned int)(i+l) > array->count)) {
			Toy_error("Array index is out of bounds");
			Toy_freeValue(value);
			Toy_freeValue(index);
			Toy_freeValue(length);
			Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
			return;
		}

		//in the event of a certain subset of types, create references instead (these should only exist on the stack)
		if (TOY_VALUE_IS_REFERENCE(array->data[i]) || TOY_VALUE_IS_ARRAY(array->data[i]) || TOY_VALUE_IS_TABLE(array->data[i])) {
			Toy_Value ref = TOY_REFERENCE_FROM_POINTER(&(array->data[i]));
			Toy_pushStack(&vm->stack, ref);
		}

		else {
			Toy_pushStack(&vm->stack, Toy_copyValue(array->data[i]));
		}
	}

	else if (TOY_VALUE_IS_TABLE(value)) {
		if (TOY_VALUE_IS_NULL(length) != true) {
			Toy_error("Can't index-length a table");
			Toy_freeValue(value);
			Toy_freeValue(index);
			Toy_freeValue(length);
			Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
			return;
		}

		//get the table & element value
		Toy_Table* table = TOY_VALUE_AS_TABLE(value);
		Toy_TableEntry* entry = Toy_private_lookupTableEntryPtr(&table, index);

		if (entry == NULL) {
			Toy_error("Table key not found");
			Toy_freeValue(value);
			Toy_freeValue(index);
			Toy_freeValue(length);
			Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
			return;
		}

		//in the event of a certain subset of types, create references instead (these should only exist on the stack)
		if (TOY_VALUE_IS_REFERENCE(entry->value) || TOY_VALUE_IS_ARRAY(entry->value) || TOY_VALUE_IS_TABLE(entry->value)) {
			Toy_Value ref = TOY_REFERENCE_FROM_POINTER(&(entry->value));
			Toy_pushStack(&vm->stack, ref);
		}

		else {
			Toy_pushStack(&vm->stack, Toy_copyValue(entry->value));
		}
	}

	else {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Unknown value type '%s' found in processIndex, exiting\n" TOY_CC_RESET, Toy_private_getValueTypeAsCString(value.type));
		exit(-1);
	}

	Toy_freeValue(value);
	Toy_freeValue(index);
	Toy_freeValue(length);
}

static void process(Toy_VM* vm) {
	while(true) {
		//prep by aligning to the 4-byte word
		fixAlignment(vm);

		Toy_OpcodeType opcode = READ_BYTE(vm);

		switch(opcode) {
			//variable instructions
			case TOY_OPCODE_READ:
				processRead(vm);
				break;

			case TOY_OPCODE_DECLARE:
				processDeclare(vm);
				break;

			case TOY_OPCODE_ASSIGN:
				processAssign(vm);
				break;

			case TOY_OPCODE_ASSIGN_COMPOUND:
				processAssignCompound(vm);
				break;

			case TOY_OPCODE_ACCESS:
				processAccess(vm);
				break;

			case TOY_OPCODE_DUPLICATE:
				processDuplicate(vm);
				break;

			case TOY_OPCODE_ELIMINATE:
				processEliminate(vm);
				break;

			//arithmetic instructions
			case TOY_OPCODE_ADD:
			case TOY_OPCODE_SUBTRACT:
			case TOY_OPCODE_MULTIPLY:
			case TOY_OPCODE_DIVIDE:
			case TOY_OPCODE_MODULO:
				processArithmetic(vm, opcode);
				break;

			//comparison instructions
			case TOY_OPCODE_COMPARE_EQUAL:
			case TOY_OPCODE_COMPARE_LESS:
			case TOY_OPCODE_COMPARE_LESS_EQUAL:
			case TOY_OPCODE_COMPARE_GREATER:
			case TOY_OPCODE_COMPARE_GREATER_EQUAL:
				processComparison(vm, opcode);
				break;

			//logical instructions
			case TOY_OPCODE_AND:
			case TOY_OPCODE_OR:
			case TOY_OPCODE_TRUTHY:
			case TOY_OPCODE_NEGATE:
				processLogical(vm, opcode);
				break;

			//control instructions
			case TOY_OPCODE_RETURN:
				//temp terminator
				return;

			case TOY_OPCODE_JUMP:
				processJump(vm);
				break;

			case TOY_OPCODE_ESCAPE:
				processEscape(vm);
				break;

			case TOY_OPCODE_SCOPE_PUSH:
				vm->scope = Toy_pushScope(&vm->scopeBucket, vm->scope);
				break;

			case TOY_OPCODE_SCOPE_POP:
				vm->scope = Toy_popScope(vm->scope);
				break;

			//various action instructions
			case TOY_OPCODE_ASSERT:
				processAssert(vm);
				break;

			case TOY_OPCODE_PRINT:
				processPrint(vm);
				break;

			case TOY_OPCODE_CONCAT:
				processConcat(vm);
				break;

			case TOY_OPCODE_INDEX:
				processIndex(vm);
				break;

			case TOY_OPCODE_UNUSED:
			case TOY_OPCODE_PASS:
			case TOY_OPCODE_ERROR:
			case TOY_OPCODE_EOF:
				fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid opcode %d found, exiting\n" TOY_CC_RESET, opcode);
				exit(-1);
		}
	}
}

//exposed functions
void Toy_resetVM(Toy_VM* vm, bool preserveScope) {
	vm->code = NULL;

	vm->jumpsCount = 0;
	vm->paramCount = 0;
	vm->dataCount = 0;
	vm->subsCount = 0;

	vm->codeAddr = 0;
	vm->jumpsAddr = 0;
	vm->paramAddr = 0;
	vm->dataAddr = 0;
	vm->subsAddr = 0;

	vm->programCounter = 0;

	Toy_resetStack(&vm->stack);

	if (preserveScope == false) {
		vm->scope = Toy_popScope(vm->scope);
	}

	//NOTE: buckets are not altered during resets
}

void Toy_initVM(Toy_VM* vm) {
	//create persistent memory
	vm->scope = NULL;
	vm->stack = Toy_allocateStack();
	vm->stringBucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);
	vm->scopeBucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);

	Toy_resetVM(vm, true);
}

void Toy_inheritVM(Toy_VM* vm, Toy_VM* parent) {
	//inherent persistent memory
	vm->scope = NULL;
	vm->stack = Toy_allocateStack();
	vm->stringBucket = parent->stringBucket;
	vm->scopeBucket = parent->scopeBucket;

	//TODO: parent bucket pointers are updated after function calls

	Toy_resetVM(vm, true);
}

void Toy_bindVM(Toy_VM* vm, Toy_Module* module, bool preserveScope) {
	vm->code = module->code;

	vm->jumpsCount = module->jumpsCount;
	vm->paramCount = module->paramCount;
	vm->dataCount = module->dataCount;
	vm->subsCount = module->subsCount;

	vm->codeAddr = module->codeAddr;
	vm->jumpsAddr = module->jumpsAddr;
	vm->paramAddr = module->paramAddr;
	vm->dataAddr = module->dataAddr;
	vm->subsAddr = module->subsAddr;

	if (preserveScope == false) {
		vm->scope = Toy_pushScope(&vm->scopeBucket, module->scopePtr);
	}
}

void Toy_runVM(Toy_VM* vm) {
	if (vm->codeAddr == 0) {
		//ignore uninitialized VMs or empty modules
		return;
	}

	//TODO: read params into scope

	//prep the program counter for execution
	vm->programCounter = vm->codeAddr;

	//begin
	process(vm);

	//TODO: add return value extraction
}

void Toy_freeVM(Toy_VM* vm) {
	Toy_resetVM(vm, false);

	//clear the persistent memory
	Toy_freeStack(vm->stack);
	Toy_freeBucket(&vm->stringBucket);
	Toy_freeBucket(&vm->scopeBucket);
}
