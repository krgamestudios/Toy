#include "toy_vm.h"
#include "toy_console_colors.h"

#include "toy_print.h"
#include "toy_opcodes.h"
#include "toy_value.h"
#include "toy_string.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//utilities
#define READ_BYTE(vm) \
	vm->module[vm->programCounter++]

#define READ_UNSIGNED_INT(vm) \
	*((unsigned int*)(vm->module + readPostfixUtil(&(vm->programCounter), 4)))

#define READ_INT(vm) \
	*((int*)(vm->module + readPostfixUtil(&(vm->programCounter), 4)))

#define READ_FLOAT(vm) \
	*((float*)(vm->module + readPostfixUtil(&(vm->programCounter), 4)))

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
			int len = (int)READ_BYTE(vm);

			//grab the jump as an integer
			unsigned int jump = *((int*)(vm->module + vm->jumpsAddr + READ_INT(vm)));

			//jumps are relative to the data address
			char* cstring = (char*)(vm->module + vm->dataAddr + jump);

			//build a string from the data section
			if (stringType == TOY_STRING_LEAF) {
				value = TOY_VALUE_FROM_STRING(Toy_createString(&vm->stringBucket, cstring));
			}
			else if (stringType == TOY_STRING_NAME) {
				Toy_ValueType valueType = TOY_VALUE_UNKNOWN;

				value = TOY_VALUE_FROM_STRING(Toy_createNameStringLength(&vm->stringBucket, cstring, len, valueType, false));
			}
			else {
				Toy_error("Invalid string type found");
			}

			break;
		}

		case TOY_VALUE_ARRAY: {
			//
			// break;
		}

		case TOY_VALUE_TABLE: {
			//
			// break;
		}

		case TOY_VALUE_FUNCTION: {
			//
			// break;
		}

		case TOY_VALUE_OPAQUE: {
			//
			// break;
		}

		case TOY_VALUE_TYPE: {
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
	unsigned int jump = *(unsigned int*)(vm->module + vm->jumpsAddr + READ_INT(vm));

	//grab the data
	char* cstring = (char*)(vm->module + vm->dataAddr + jump);

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
	if (!TOY_VALUE_IS_STRING(name) || TOY_VALUE_AS_STRING(name)->type != TOY_STRING_NAME) {
		Toy_error("Invalid assignment target");
		return;
	}

	//assign it
	Toy_assignScope(vm->scope, TOY_VALUE_AS_STRING(name), value);

	//cleanup
	Toy_freeValue(name);
}

static void processAccess(Toy_VM* vm) {
	Toy_Value name = Toy_popStack(&vm->stack);

	//check name string type
	if (!TOY_VALUE_IS_STRING(name) && TOY_VALUE_AS_STRING(name)->type != TOY_STRING_NAME) {
		Toy_error("Invalid access target");
		return;
	}

	//find and push the value
	Toy_Value value = Toy_accessScope(vm->scope, TOY_VALUE_AS_STRING(name));
	Toy_pushStack(&vm->stack, Toy_copyValue(value));

	//cleanup
	Toy_freeValue(name);
}

static void processDuplicate(Toy_VM* vm) {
	Toy_Value value = Toy_copyValue(Toy_peekStack(&vm->stack));
	Toy_pushStack(&vm->stack, value);
	Toy_freeValue(value);

	//check for compound assignments
	Toy_OpcodeType squeezed = READ_BYTE(vm);
	if (squeezed == TOY_OPCODE_ACCESS) {
		processAccess(vm);
	}
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
		return;
	}

	//check for divide by zero
	if (opcode == TOY_OPCODE_DIVIDE || opcode == TOY_OPCODE_MODULO) {
		if ((TOY_VALUE_IS_INTEGER(right) && TOY_VALUE_AS_INTEGER(right) == 0) || (TOY_VALUE_IS_FLOAT(right) && TOY_VALUE_AS_FLOAT(right) == 0)) {
			Toy_error("Can't divide or modulo by zero");
			Toy_freeValue(left);
			Toy_freeValue(right);
			return;
		}
	}

	//check for modulo by a float
	if (opcode == TOY_OPCODE_MODULO && TOY_VALUE_IS_FLOAT(right)) {
		Toy_error("Can't modulo by a float");
		Toy_freeValue(left);
		Toy_freeValue(right);
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

	if (Toy_checkValuesAreComparable(left, right) == false) {
		char buffer[256];
		snprintf(buffer, 256, "Can't compare value types '%s' and '%s'", Toy_private_getValueTypeAsCString(left.type), Toy_private_getValueTypeAsCString(right.type));
		Toy_error(buffer);
		Toy_freeValue(left);
		Toy_freeValue(right);
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
	Toy_OpJumpType type = READ_BYTE(vm);
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

static void processAssert(Toy_VM* vm) {
	unsigned int count = READ_BYTE(vm);

	Toy_Value value = TOY_VALUE_FROM_NULL();
	Toy_Value message = TOY_VALUE_FROM_NULL();

	//determine the args
	if (count == 1) {
		message = TOY_VALUE_FROM_STRING(Toy_createString(&vm->stringBucket, "assertion failed"));
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
	if (TOY_VALUE_IS_NULL(value) || Toy_checkValueIsTruthy(value) == false) {
		//on a failure, print the message
		Toy_stringifyValue(message, Toy_assertFailure);
	}

	//cleanup
	Toy_freeValue(value);
	Toy_freeValue(message);
}

static void processPrint(Toy_VM* vm) {
	//print the value on top of the stack, popping it
	Toy_Value value = Toy_popStack(&vm->stack);
	Toy_stringifyValue(value, Toy_print);
	Toy_freeValue(value);
}

static void processConcat(Toy_VM* vm) {
	Toy_Value right = Toy_popStack(&vm->stack);
	Toy_Value left = Toy_popStack(&vm->stack);

	if (!TOY_VALUE_IS_STRING(left) || !TOY_VALUE_IS_STRING(right)) {
		Toy_error("Failed to concatenate a value that is not a string");
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
		//TODO: clear stack
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
			return;
		}

		if (!(TOY_VALUE_IS_NULL(length) || TOY_VALUE_IS_INTEGER(length))) {
			Toy_error("Failed to index-length a string");
			Toy_freeValue(value);
			Toy_freeValue(index);
			Toy_freeValue(length);
			return;
		}

		//extract values
		int i = TOY_VALUE_AS_INTEGER(index);
		int l = TOY_VALUE_IS_INTEGER(length) ? TOY_VALUE_AS_INTEGER(length) : 1;

		//extract string
		Toy_String* str = TOY_VALUE_AS_STRING(value);
		Toy_String* result = NULL;

		//extract cstring, based on type
		if (str->type == TOY_STRING_LEAF) {
			const char* cstr = str->as.leaf.data;
			result = Toy_createStringLength(&vm->stringBucket, cstr + i, l);
		}
		else if (str->type == TOY_STRING_NODE) {
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

			case TOY_OPCODE_ACCESS:
				processAccess(vm);
				break;

			case TOY_OPCODE_DUPLICATE:
				processDuplicate(vm);
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

			case TOY_OPCODE_PASS:
			case TOY_OPCODE_ERROR:
			case TOY_OPCODE_EOF:
				fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid opcode %d found, exiting\n" TOY_CC_RESET, opcode);
				exit(-1);
		}
	}
}

//exposed functions
void Toy_initVM(Toy_VM* vm) {
	//clear the stack, scope and memory
	vm->stringBucket = NULL;
	vm->scopeBucket = NULL;
	vm->stack = NULL;
	vm->scope = NULL;

	Toy_resetVM(vm);
}

void Toy_bindVM(Toy_VM* vm, struct Toy_Bytecode* bc) {
	if (bc->ptr[0] != TOY_VERSION_MAJOR || bc->ptr[1] > TOY_VERSION_MINOR) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Wrong bytecode version found: expected %d.%d.%d found %d.%d.%d, exiting\n" TOY_CC_RESET, TOY_VERSION_MAJOR, TOY_VERSION_MINOR, TOY_VERSION_PATCH, bc->ptr[0], bc->ptr[1], bc->ptr[2]);
		exit(-1);
	}

	if (bc->ptr[2] != TOY_VERSION_PATCH) {
		fprintf(stderr, TOY_CC_WARN "WARNING: Wrong bytecode version found: expected %d.%d.%d found %d.%d.%d, continuing\n" TOY_CC_RESET, TOY_VERSION_MAJOR, TOY_VERSION_MINOR, TOY_VERSION_PATCH, bc->ptr[0], bc->ptr[1], bc->ptr[2]);
	}

	if (strcmp((char*)(bc->ptr + 3), TOY_VERSION_BUILD) != 0) {
		fprintf(stderr, TOY_CC_WARN "WARNING: Wrong bytecode build info found: expected '%s' found '%s', continuing\n" TOY_CC_RESET, TOY_VERSION_BUILD, (char*)(bc->ptr + 3));
	}

	//offset by the header size
	int offset = 3 + strlen(TOY_VERSION_BUILD) + 1;
	if (offset % 4 != 0) {
		offset += 4 - (offset % 4); //ceil
	}

	if (bc->moduleCount != 0) { //tmp check, just in case the bytecode is empty; will rework this when module packing works
		//delegate to a more specialized function
		Toy_bindVMToModule(vm, bc->ptr + offset);
	}
}

void Toy_bindVMToModule(Toy_VM* vm, unsigned char* module) {
	vm->module = module;

	//read the header metadata
	vm->moduleSize = READ_UNSIGNED_INT(vm);
	vm->paramSize = READ_UNSIGNED_INT(vm);
	vm->jumpsSize = READ_UNSIGNED_INT(vm);
	vm->dataSize = READ_UNSIGNED_INT(vm);
	vm->subsSize = READ_UNSIGNED_INT(vm);

	//read the header addresses
	if (vm->paramSize > 0) {
		vm->paramAddr = READ_UNSIGNED_INT(vm);
	}

	vm->codeAddr = READ_UNSIGNED_INT(vm); //required

	if (vm->jumpsSize > 0) {
		vm->jumpsAddr = READ_UNSIGNED_INT(vm);
	}

	if (vm->dataSize > 0) {
		vm->dataAddr = READ_UNSIGNED_INT(vm);
	}

	if (vm->subsSize > 0) {
		vm->subsAddr = READ_UNSIGNED_INT(vm);
	}

	//allocate the stack, scope, and memory (skip if already in use)
	if (vm->stringBucket == NULL) {
		vm->stringBucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);
	}
	if (vm->scopeBucket == NULL) {
		vm->scopeBucket = Toy_allocateBucket(TOY_BUCKET_SMALL);
	}
	if (vm->stack == NULL) {
		vm->stack = Toy_allocateStack();
	}
	if (vm->scope == NULL) {
		vm->scope = Toy_pushScope(&vm->scopeBucket, NULL);
	}
}

void Toy_runVM(Toy_VM* vm) {
	//NO-OP on empty VMs
	if (vm->module == NULL) {
		return;
	}

	//TODO: read params into scope

	//prep the program counter for execution
	vm->programCounter = vm->codeAddr;

	//begin
	process(vm);
}

void Toy_freeVM(Toy_VM* vm) {
	//clear the stack, scope and memory
	Toy_freeStack(vm->stack);
	Toy_popScope(vm->scope);
	Toy_freeBucket(&vm->stringBucket);
	Toy_freeBucket(&vm->scopeBucket);

	Toy_resetVM(vm);
}

void Toy_resetVM(Toy_VM* vm) {
	vm->module = NULL;
	vm->moduleSize = 0;

	vm->paramSize = 0;
	vm->jumpsSize = 0;
	vm->dataSize = 0;
	vm->subsSize = 0;

	vm->paramAddr = 0;
	vm->codeAddr = 0;
	vm->jumpsAddr = 0;
	vm->dataAddr = 0;
	vm->subsAddr = 0;

	vm->programCounter = 0;

	//NOTE: stack, scope and memory are not altered during resets
}
