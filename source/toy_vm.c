#include "toy_vm.h"
#include "toy_console_colors.h"

#include "toy_print.h"
#include "toy_opcodes.h"
#include "toy_attributes.h"

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

//forward declarations for delegations
static void processJump(Toy_VM* vm);

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
			Toy_StringType stringType = READ_BYTE(vm);
			int len = (int)READ_BYTE(vm); //WARN: only used for name strings
			(void)len;

			//grab the jump as an integer
			unsigned int jump = *((int*)(vm->code + vm->jumpsAddr + READ_INT(vm)));

			//BUG: the jump parameter could cause an out of bounds read if it's malformed

			//jumps are relative to the data address
			char* cstring = (char*)(vm->code + vm->dataAddr + jump);

			//build a string from the data section
			if (stringType == TOY_STRING_LEAF) {
				value = TOY_VALUE_FROM_STRING(Toy_toString(&vm->memoryBucket, cstring));
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

			//create the table (count covers keys AND values)
			Toy_Table* table = Toy_allocateTable(count / 2);

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
			unsigned int paramCount = (unsigned int)READ_BYTE(vm); //unused
			(void)paramCount;

			fixAlignment(vm);

			unsigned int addr = (unsigned int)READ_INT(vm);

			//create and push the function value
			Toy_Function* function = Toy_createFunctionFromBytecode(&vm->memoryBucket, vm->code + vm->subsAddr + addr, vm->scope); //BUG: functions don't have the jumpTable indirection?
			value = TOY_VALUE_FROM_FUNCTION(function);

			break;
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
	Toy_String* name = Toy_toStringLength(&vm->memoryBucket, cstring, len);

	//get the value
	Toy_Value value = Toy_popStack(&vm->stack);

	//declare it
	Toy_declareScope(vm->scope, name, type, value, constant);

	//cleanup
	Toy_freeString(name);
}

static void processAssign(Toy_VM* vm) {
	//get the value & name
	Toy_Value value = Toy_popStack(&vm->stack);
	Toy_Value name = Toy_popStack(&vm->stack);

	//assign it
	Toy_assignScope(vm->scope, TOY_VALUE_AS_STRING(name), Toy_copyValue(&vm->memoryBucket, value));

	//in case of chaining, leave a copy on the stack
	bool chainedAssignment = READ_BYTE(vm);
	if (chainedAssignment) {
		Toy_pushStack(&vm->stack, Toy_copyValue(&vm->memoryBucket, value));
	}

	//cleanup
	Toy_freeValue(name);
	Toy_freeValue(value);
}

static void processAssignCompound(Toy_VM* vm) {
	//get the value, key, target
	Toy_Value value = Toy_popStack(&vm->stack);
	Toy_Value key = Toy_popStack(&vm->stack);
	Toy_Value target = Toy_popStack(&vm->stack);

	//shake out variable names
	if (TOY_VALUE_IS_STRING(target)) {
		Toy_Value* valuePtr = Toy_accessScopeAsPointer(vm->scope, TOY_VALUE_AS_STRING(target));
		Toy_freeValue(target);
		if (valuePtr == NULL) {
			return;
		}

		//in the event of a certain subset of types, create references instead (these should only exist on the stack)
		if (TOY_VALUE_IS_REFERENCE(*valuePtr) || TOY_VALUE_IS_ARRAY(*valuePtr) || TOY_VALUE_IS_TABLE(*valuePtr) || TOY_VALUE_IS_FUNCTION(*valuePtr)) {
			target = TOY_REFERENCE_FROM_POINTER(valuePtr);
		}
		else {
			target = Toy_copyValue(&vm->memoryBucket, *valuePtr);
		}
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
		array->data[index] = Toy_copyValue(&vm->memoryBucket, value);

		//in case of chaining, leave a copy on the stack
		bool chainedAssignment = READ_BYTE(vm);
		if (chainedAssignment) {
			Toy_pushStack(&vm->stack, Toy_copyValue(&vm->memoryBucket, value));
		}

		//cleanup
		Toy_freeValue(value);
	}

	else if (TOY_VALUE_IS_TABLE(target)) {
		Toy_Table* table = TOY_VALUE_AS_TABLE(target);

		//set the value
		Toy_insertTable(&table, Toy_copyValue(&vm->memoryBucket, key), Toy_copyValue(&vm->memoryBucket, value));

		//in case of chaining, leave a copy on the stack
		bool chainedAssignment = READ_BYTE(vm);
		if (chainedAssignment) {
			Toy_pushStack(&vm->stack, Toy_copyValue(&vm->memoryBucket, value));
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

	//find the value
	Toy_Value* valuePtr = Toy_accessScopeAsPointer(vm->scope, TOY_VALUE_AS_STRING(name));

	if (valuePtr == NULL) {
		Toy_freeValue(name);
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
		return;
	}

	//in the event of a certain subset of types, create references instead (these should only exist on the stack)
	if (TOY_VALUE_IS_REFERENCE(*valuePtr) || TOY_VALUE_IS_ARRAY(*valuePtr) || TOY_VALUE_IS_TABLE(*valuePtr) || TOY_VALUE_IS_FUNCTION(*valuePtr)) {
		Toy_Value ref = TOY_REFERENCE_FROM_POINTER(valuePtr);
		Toy_pushStack(&vm->stack, ref);
	}
	else {
		Toy_pushStack(&vm->stack, Toy_copyValue(&vm->memoryBucket, *valuePtr));
	}

	//cleanup
	Toy_freeValue(name);
}

static void processInvoke(Toy_VM* vm) {
	Toy_ValueType valueType = READ_BYTE(vm);
	unsigned int argCount = (unsigned int)READ_BYTE(vm);
	fixAlignment(vm);

	//check for invoking bad values
	if (valueType != TOY_VALUE_FUNCTION) {
		Toy_error("Unrecognized invoke on a non-function value");
		return;
	}

	//function to call
	Toy_Value value = Toy_popStack(&vm->stack);
	if (TOY_VALUE_IS_FUNCTION(value) != true) {
		Toy_error("Can't call a non-function value");
		return;
	}

	//process based on the function type
	Toy_Function* fn = TOY_VALUE_AS_FUNCTION(value);

	switch(fn->type) {
		case TOY_FUNCTION_CUSTOM: {
			//spin up a new sub-vm
			Toy_VM subVM;
			Toy_inheritVM(vm, &subVM);
			Toy_bindVM(&subVM, fn->bytecode.code, fn->bytecode.parentScope);

			//check args count
			if (argCount * 8 != subVM.paramCount) {
				Toy_error("Incorrect number of parameters specified for function call");
				break;
			}

			if (argCount > vm->stack->count) {
				Toy_error("Incorrect number of parameters on the stack for function call");
				break;
			}

			//inject params, backwards from the stack
			for (unsigned int i = argCount; i > 0; i--) {
				Toy_Value argValue = Toy_popStack(&vm->stack);

				//paramAddr is relative to the data section, and is followed by the param type
				unsigned int paramAddr = ((unsigned int*)(subVM.code + subVM.paramAddr))[(i-1)*2];
				Toy_ValueType paramType = (Toy_ValueType)(((unsigned int*)(subVM.code + subVM.paramAddr))[(i-1)*2 + 1]);

				//c-string of the param's name
				const char* cstr = ((char*)(subVM.code + subVM.dataAddr)) + paramAddr;

				//as a name string
				Toy_String* name = Toy_toStringLength(&subVM.memoryBucket, cstr, strlen(cstr));

				Toy_declareScope(subVM.scope, name, paramType, argValue, true);

				Toy_freeString(name);
			}

			//run
			unsigned int resultCount = Toy_runVM(&subVM);

			//extract and store any results
			if (resultCount > 0) {
				Toy_Value result = Toy_getReturnValueFromVM(vm, &subVM);
				Toy_pushStack(&vm->stack, result);
			}

			//cleanup
			Toy_freeVM(&subVM);
		}
		break;

		case TOY_FUNCTION_NATIVE: {
			//NOTE: arguments are on the stack, leave results on the stack
			fn->native.callback(vm, &fn->native);
		}
		break;

		default:
			Toy_error("Can't call an unknown function type");
			break;
	}

	Toy_freeValue(value);
}

static void processAttribute(Toy_VM* vm) {
	//get the compound & attribute
	Toy_Value attribute = Toy_popStack(&vm->stack);
	Toy_Value compound = Toy_popStack(&vm->stack);

	Toy_Value result = TOY_VALUE_FROM_NULL();

	//type-based attributes
	if (TOY_VALUE_IS_STRING(compound)) {
		result = Toy_private_handleStringAttributes(vm, compound, attribute);
	}
	else if (TOY_VALUE_IS_ARRAY(compound)) {
		result = Toy_private_handleArrayAttributes(vm, compound, attribute);
	}
	else if (TOY_VALUE_IS_TABLE(compound)) {
		result = Toy_private_handleTableAttributes(vm, compound, attribute);
	}
	else if (TOY_VALUE_IS_OPAQUE(compound)) {
		result = Toy_private_handleOpaqueAttributes(vm, compound, attribute);
	}
	else {
		char buffer[256];
		snprintf(buffer, 256, "Can't access an attribute of type '%s'", Toy_getValueTypeAsCString(Toy_unwrapValue(compound).type));
		Toy_error(buffer);
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
		return;
	}

	//BUGFIX: check for callable functions, so they can access the compound too
	if (TOY_VALUE_IS_FUNCTION(result)) {
		//WONTFIX: `print array.pushBack;' will leave the compound's reference on the stack
		Toy_pushStack(&vm->stack, compound);
	}

	//leave the result on the stack
	Toy_pushStack(&vm->stack, result);

	//cleanup
	Toy_freeValue(attribute);
}

static void processDuplicate(Toy_VM* vm) {
	Toy_Value value = Toy_copyValue(&vm->memoryBucket, Toy_peekStack(&vm->stack));
	Toy_pushStack(&vm->stack, value);

	//check for compound assignments
	Toy_OpcodeType squeezed = READ_BYTE(vm);
	if (squeezed == TOY_OPCODE_ACCESS) {
		processAccess(vm);
	}
}

static void processEliminate(Toy_VM* vm) {
	//discard the stack top, X times
	unsigned int x = (unsigned int)READ_BYTE(vm);
	for (unsigned int i = 0; i < x; i++) {
		Toy_Value value = Toy_popStack(&vm->stack);
		Toy_freeValue(value);
	}
}

static void processIterate(Toy_VM* vm) {
	//ITERATE on [-2] based on type, with [-1] as counter
	//then delegate to processJump

	Toy_Value counter = Toy_popStack(&vm->stack);
	Toy_Value compound = Toy_popStack(&vm->stack);

	if (!TOY_VALUE_IS_INTEGER(counter)) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Unknown counter type '%s' found in for loop, exiting\n" TOY_CC_RESET, Toy_getValueTypeAsCString(Toy_unwrapValue(counter).type));
		exit(-1);
	}

	if (TOY_VALUE_IS_ARRAY(compound)) {
		Toy_Array* array = TOY_VALUE_AS_ARRAY(compound);
		unsigned int index = (unsigned int)TOY_VALUE_AS_INTEGER(counter);

		//check out-of-bounds
		if (index >= array->count) {
			//DON'T free the iterable & counter, that's embedded in the bytecode
			Toy_pushStack(&vm->stack, compound);
			Toy_pushStack(&vm->stack, counter);
			//force a jump then exit
			Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
			processJump(vm);
			return;
		}

		//get the desired element
		Toy_Value value = Toy_copyValue(&vm->memoryBucket, array->data[index]);

		//push everything back onto the stack (iterating the counter)
		Toy_pushStack(&vm->stack, compound);
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_INTEGER(index + 1));
		Toy_pushStack(&vm->stack, value);
	}
	else if (TOY_VALUE_IS_TABLE(compound)) {
		Toy_Table* table = TOY_VALUE_AS_TABLE(compound);
		unsigned int index = (unsigned int)TOY_VALUE_AS_INTEGER(counter);

		//loop to the next element
		while(index < table->capacity) {
			//is this a useable element
			if (!TOY_VALUE_IS_NULL(table->data[index].key)) {
				break;
			}

			index++;
		}

		//return the compound & counter to the stack
		Toy_pushStack(&vm->stack, compound);
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_INTEGER(index + 1));

		//if something was found, push it and return
		if (index < table->capacity) {
			Toy_pushStack(&vm->stack, Toy_copyValue(&vm->memoryBucket, table->data[index].value));
		}
		else {
			//otherwise force a jump then exit
			Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
			processJump(vm);
		}
	}
	//TODO: support closures in for-loops
	else {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Unknown iterable type '%s' found in for loop, exiting\n" TOY_CC_RESET, Toy_getValueTypeAsCString(Toy_unwrapValue(compound).type));
		exit(-1);
	}
}

static void processArithmetic(Toy_VM* vm, Toy_OpcodeType opcode) {
	//BUGFIX: handle negative variables
	if (opcode == TOY_OPCODE_INVERT) {
		Toy_Value value = Toy_popStack(&vm->stack);
		if (TOY_VALUE_IS_INTEGER(value)) {
			int intermediate = TOY_VALUE_AS_INTEGER(value);
			Toy_pushStack(&vm->stack, TOY_VALUE_FROM_INTEGER(-intermediate));
		}
		else if (TOY_VALUE_IS_FLOAT(value)) {
			float intermediate = TOY_VALUE_AS_FLOAT(value);
			Toy_pushStack(&vm->stack, TOY_VALUE_FROM_FLOAT(-intermediate));
		}
		else {
			Toy_error("Can't negate a non-number");
			Toy_freeValue(value);
			Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
		}
		return;
	}

	Toy_Value right = Toy_popStack(&vm->stack);
	Toy_Value left = Toy_popStack(&vm->stack);

	//check types
	if ((!TOY_VALUE_IS_INTEGER(left) && !TOY_VALUE_IS_FLOAT(left)) || (!TOY_VALUE_IS_INTEGER(right) && !TOY_VALUE_IS_FLOAT(right))) {
		char buffer[256];
		snprintf(buffer, 256, "Invalid types '%s' and '%s' passed in arithmetic", Toy_getValueTypeAsCString(Toy_unwrapValue(left).type), Toy_getValueTypeAsCString(Toy_unwrapValue(right).type));
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
		snprintf(buffer, 256, "Can't compare value types '%s' and '%s'", Toy_getValueTypeAsCString(Toy_unwrapValue(left).type), Toy_getValueTypeAsCString(Toy_unwrapValue(right).type));
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

static unsigned int processReturn(Toy_VM* vm) {
	//the values to be returned are waiting on the stack
	unsigned int resultCount = (unsigned int)READ_BYTE(vm);
	fixAlignment(vm);

	return resultCount;
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

		case TOY_OP_PARAM_JUMP_IF_NULL: {
			Toy_Value value = Toy_popStack(&vm->stack);
			if (TOY_VALUE_IS_NULL(value)) {
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
		message = TOY_VALUE_FROM_STRING(Toy_toString(&vm->memoryBucket, "assertion failed")); //TODO: better default assert message
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
		Toy_String* string = Toy_stringifyValue(&vm->memoryBucket, message);
		char* buffer = Toy_getStringRaw(string);

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
	Toy_String* string = Toy_stringifyValue(&vm->memoryBucket, value);

	//check string type to skip a potential malloc
	if (string->info.type == TOY_STRING_LEAF) {
		Toy_print(string->leaf.data);
	}
	else {
		char* buffer = Toy_getStringRaw(string);
		Toy_print(buffer);
		free(buffer);
	}

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
	Toy_String* result = Toy_concatStrings(&vm->memoryBucket, TOY_VALUE_AS_STRING(left), TOY_VALUE_AS_STRING(right));
	Toy_pushStack(&vm->stack, TOY_VALUE_FROM_STRING(result));
}

static void processIndex(Toy_VM* vm) {
	unsigned char count = READ_BYTE(vm); //value[index, length] ; 1[2, 3]

	//TODO: slicing from the end of a string/array, value[0:-1]

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
			result = Toy_createStringLength(&vm->memoryBucket, cstr + i, l);
		}
		else if (str->info.type == TOY_STRING_NODE) {
			char* cstr = Toy_getStringRaw(str);
			result = Toy_createStringLength(&vm->memoryBucket, cstr + i, l);
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
		if (TOY_VALUE_IS_REFERENCE(array->data[i]) || TOY_VALUE_IS_ARRAY(array->data[i]) || TOY_VALUE_IS_TABLE(array->data[i]) || TOY_VALUE_IS_FUNCTION(array->data[i])) {
			Toy_Value ref = TOY_REFERENCE_FROM_POINTER(&(array->data[i]));
			Toy_pushStack(&vm->stack, ref);
		}
		else {
			Toy_pushStack(&vm->stack, Toy_copyValue(&vm->memoryBucket, array->data[i]));
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
		if (TOY_VALUE_IS_REFERENCE(entry->value) || TOY_VALUE_IS_ARRAY(entry->value) || TOY_VALUE_IS_TABLE(entry->value) || TOY_VALUE_IS_FUNCTION(entry->value)) {
			Toy_Value ref = TOY_REFERENCE_FROM_POINTER(&(entry->value));
			Toy_pushStack(&vm->stack, ref);
		}
		else {
			Toy_pushStack(&vm->stack, Toy_copyValue(&vm->memoryBucket, entry->value));
		}
	}

	else {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Unknown value type '%s' found in processIndex, exiting\n" TOY_CC_RESET, Toy_getValueTypeAsCString(Toy_unwrapValue(value).type));
		exit(-1);
	}

	Toy_freeValue(value);
	Toy_freeValue(index);
	Toy_freeValue(length);
}

static unsigned int process(Toy_VM* vm) {
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

			case TOY_OPCODE_INVOKE:
				processInvoke(vm);
				break;

			case TOY_OPCODE_ATTRIBUTE:
				processAttribute(vm);
				break;

			case TOY_OPCODE_DUPLICATE:
				processDuplicate(vm);
				break;

			case TOY_OPCODE_ELIMINATE:
				processEliminate(vm);
				break;

			case TOY_OPCODE_ITERATE:
				processIterate(vm);
				break;

			//arithmetic instructions
			case TOY_OPCODE_ADD:
			case TOY_OPCODE_SUBTRACT:
			case TOY_OPCODE_MULTIPLY:
			case TOY_OPCODE_DIVIDE:
			case TOY_OPCODE_MODULO:
			case TOY_OPCODE_INVERT:
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
				return processReturn(vm); //the only return statement, which signals the number of values for extraction

			case TOY_OPCODE_JUMP:
				processJump(vm);
				break;

			case TOY_OPCODE_ESCAPE:
				processEscape(vm);
				break;

			case TOY_OPCODE_SCOPE_PUSH:
				vm->scope = Toy_pushScope(&vm->memoryBucket, vm->scope);
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
void Toy_resetVM(Toy_VM* vm, bool preserveScope, bool preserveStack) {
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

	if (!preserveScope) {
		vm->scope = Toy_popScope(vm->scope);
	}

	if (!preserveStack) {
		Toy_resetStack(&vm->stack); //WARN: has a realloc()
	}

	//not sure how often to call teh GC
	if (vm->memoryBucket) {
		Toy_collectBucketGarbage(&vm->memoryBucket); //WONTFIX: call GC after a certain number of bucket links allocated
	}
}

void Toy_initVM(Toy_VM* vm) {
	//create persistent memory
	vm->scope = NULL;
	vm->stack = Toy_allocateStack();
	vm->memoryBucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);

	vm->parentBucketHandle = NULL;

	Toy_resetVM(vm, true, true);
}

void Toy_inheritVM(Toy_VM* parentVM, Toy_VM* subVM) {
	//inherent persistent memory
	subVM->scope = NULL;
	subVM->stack = Toy_allocateStack();
	subVM->memoryBucket = parentVM->memoryBucket;

	subVM->parentBucketHandle = &parentVM->memoryBucket; //track this to update it later

	Toy_resetVM(subVM, true, true);
}

void Toy_bindVM(Toy_VM* vm, unsigned char* bytecode, Toy_Scope* parentScope) {
	vm->code = bytecode; //set code, so it can be read

	(void)READ_UNSIGNED_INT(vm); //global header

	//section headers
	vm->jumpsCount = READ_UNSIGNED_INT(vm);
	vm->paramCount = READ_UNSIGNED_INT(vm);
	vm->dataCount = READ_UNSIGNED_INT(vm);
	vm->subsCount = READ_UNSIGNED_INT(vm);

	//section locations
	vm->codeAddr = READ_UNSIGNED_INT(vm);
	if (vm->jumpsCount) {
		vm->jumpsAddr = READ_UNSIGNED_INT(vm);
	}
	if (vm->paramCount) {
		vm->paramAddr = READ_UNSIGNED_INT(vm);
	}
	if (vm->dataCount) {
		vm->dataAddr = READ_UNSIGNED_INT(vm);
	}
	if (vm->subsCount) {
		vm->subsAddr = READ_UNSIGNED_INT(vm);
	}

	//scopes
	if (vm->scope == NULL) {
		vm->scope = Toy_pushScope(&vm->memoryBucket, parentScope);
	}
}

unsigned int Toy_runVM(Toy_VM* vm) {
	if (vm->codeAddr == 0) {
		//ignore uninitialized VMs or empty bytecode
		return 0;
	}

	//prep the program counter for execution
	vm->programCounter = vm->codeAddr;

	//begin
	return process(vm);
}

void Toy_freeVM(Toy_VM* vm) {
	Toy_resetVM(vm, false, true);

	//clear the persistent memory
	Toy_freeStack(vm->stack);

	if (vm->parentBucketHandle != NULL) {
		*(vm->parentBucketHandle) = vm->memoryBucket; //update the outer VM, if there is one
	}
	else {
		Toy_freeBucket(&vm->memoryBucket);
	}
}

Toy_Value Toy_getReturnValueFromVM(Toy_VM* parentVM, Toy_VM* subVM) {
	if (subVM->stack->count > 0) {
		return Toy_copyValue(&parentVM->memoryBucket, subVM->stack->data[subVM->stack->count-1]);
	}
	else {
		return TOY_VALUE_FROM_NULL();
	}
}
