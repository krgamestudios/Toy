#include "toy_compiler.h"
#include "toy_console_colors.h"

#include "toy_opcodes.h"
#include "toy_value.h"
#include "toy_string.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//misc. utils
static bool checkForChainedAssign(Toy_Ast* ptr) {
	if (ptr == NULL) {
		return false;
	}

	if (ptr->type == TOY_AST_VAR_ASSIGN) {
		return true;
	}

	return false;
}

static bool checkForChainedInvoke(Toy_Ast* ptr) {
	if (ptr == NULL) {
		return false;
	}

	if (ptr->type == TOY_AST_FN_INVOKE) {
		return true;
	}

	return false;
}

//escapes
Toy_private_EscapeArray* Toy_private_resizeEscapeArray(Toy_private_EscapeArray* ptr, unsigned int capacity) {
	//if you're freeing everything, just return
	if (capacity == 0) {
		free(ptr);
		return NULL;
	}

	unsigned int originalCapacity = ptr == NULL ? 0 : ptr->capacity;
	unsigned int orignalCount = ptr == NULL ? 0 : ptr->count;

	ptr = (Toy_private_EscapeArray*)realloc(ptr, capacity * sizeof(Toy_private_EscapeEntry_t) + sizeof(Toy_private_EscapeArray));

	if (ptr == NULL) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to resize an escape array within 'Toy_Bytecode' from %d to %d capacity\n" TOY_CC_RESET, (int)originalCapacity, (int)capacity);
		exit(-1);
	}

	ptr->capacity = capacity;
	ptr->count = orignalCount;

	return ptr;
}

//writing utils
static void expand(unsigned char** handle, unsigned int* capacity, unsigned int* count, unsigned int amount) {
	if ((*count) + amount > (*capacity)) {
		while ((*count) + amount > (*capacity)) {
			(*capacity) = (*capacity) < 8 ? 8 : (*capacity) * 2;
		}
		(*handle) = realloc((*handle), (*capacity));

		if ((*handle) == NULL) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to allocate %d space for a part of 'Toy_Bytecode'\n" TOY_CC_RESET, (int)(*capacity));
			exit(1);
		}
	}
}

static void emitByte(unsigned char** handle, unsigned int* capacity, unsigned int* count, unsigned char byte) {
	expand(handle, capacity, count, 1);
	((unsigned char*)(*handle))[(*count)++] = byte;
}

//BUG: There might be issues here when compiled on big-endian platforms
static void emitInt(unsigned char** handle, unsigned int* capacity, unsigned int* count, unsigned int bytes) {
	char* ptr = (char*)&bytes;
	emitByte(handle, capacity, count, *(ptr++));
	emitByte(handle, capacity, count, *(ptr++));
	emitByte(handle, capacity, count, *(ptr++));
	emitByte(handle, capacity, count, *(ptr++));
}

static void emitFloat(unsigned char** handle, unsigned int* capacity, unsigned int* count, float bytes) {
	char* ptr = (char*)&bytes;
	emitByte(handle, capacity, count, *(ptr++));
	emitByte(handle, capacity, count, *(ptr++));
	emitByte(handle, capacity, count, *(ptr++));
	emitByte(handle, capacity, count, *(ptr++));
}

static void emitBuffer(unsigned char** handle, unsigned int* capacity, unsigned int* count, unsigned char* buffer, unsigned int bufferSize) {
	expand(handle, capacity, count, bufferSize); //4-byte aligned
	memcpy((*handle) + *count, buffer, bufferSize);
	*count += bufferSize;
}

//curry writing utils
#define EMIT_BYTE(mb, part, byte) \
	emitByte((&((*mb)->part)), &((*mb)->part##Capacity), &((*mb)->part##Count), byte)
#define EMIT_INT(mb, part, bytes) \
	emitInt((&((*mb)->part)), &((*mb)->part##Capacity), &((*mb)->part##Count), bytes)
#define EMIT_FLOAT(mb, part, bytes) \
	emitFloat((&((*mb)->part)), &((*mb)->part##Capacity), &((*mb)->part##Count), bytes)

//skip bytes, but return the address
#define SKIP_BYTE(mb, part) (EMIT_BYTE(mb, part, 0), ((*mb)->part##Count - 1))
#define SKIP_INT(mb, part) (EMIT_INT(mb, part, 0), ((*mb)->part##Count - 4))

//overwrite a pre-existing position
#define OVERWRITE_INT(mb, part, addr, bytes) \
	emitInt((&((*mb)->part)), &((*mb)->part##Capacity), &(addr), bytes);

//simply get the address (always an integer)
#define CURRENT_ADDRESS(mb, part) ((*mb)->part##Count)

//Cached write to data, enabling string reuse, see #168
static unsigned int emitCStringToData(unsigned char** dataHandle, unsigned int* capacity, unsigned int* count, const char* cstr) {
	const unsigned int slen = (unsigned int)strlen(cstr) + 1; //+1 for null

	//See if the string already exists in the data NOTE: assumes data only ever holds c-strings
	unsigned int pos = 0;
	while (pos < *count) {
		const char* entry = ((char*)(*dataHandle)) + pos;
		unsigned int elen = strlen(entry) + 1; //+1 for null

		//compare
		if (slen == elen && strncmp(cstr, entry, slen) == 0) {
			return pos;
		}

		//next
		pos += (elen + 3) & ~3;
	}

	//default, append the new entry
	unsigned int addr = *count; //save the target address
	expand(dataHandle, capacity, count, (slen + 3) & ~3); //4-byte aligned
	memcpy((*dataHandle) + addr, cstr, slen);
	*count += (slen + 3) & ~3;

	return addr; //return the address of the string in the data section
}

static unsigned int emitString(Toy_Bytecode** mb, Toy_String* str) {
	//the address within the data section
	unsigned int dataAddr = 0;

	//move the string into the data section
	if (str->info.type == TOY_STRING_NODE) {
		char* buffer = Toy_getStringRaw(str);

		dataAddr = emitCStringToData(&(*mb)->data, &(*mb)->dataCapacity, &(*mb)->dataCount, buffer);

		free(buffer);
	}
	else if (str->info.type == TOY_STRING_LEAF) {
		char buffer[str->info.length + 1]; //make sure its a null-terminated c-string
		memcpy(buffer, str->leaf.data, str->info.length);
		buffer[str->info.length] = '\0';

		dataAddr = emitCStringToData(&(*mb)->data, &(*mb)->dataCapacity, &(*mb)->dataCount, buffer);
	}

	//mark the position within the jump index, reusing an existing entry if it exists
	for (unsigned int i = 0; i < (*mb)->jumpsCount; i += 4) {
		if (*(unsigned int*)((*mb)->jumps + i) == dataAddr) {
			//reuse, and finish
			EMIT_INT(mb, code, i);
			return 1;
		}
	}

	EMIT_INT(mb, code, (*mb)->jumpsCount); //mark the new jump index in the code
	EMIT_INT(mb, jumps, dataAddr); //append to the jump table

	return 1;
}

static unsigned int emitParameters(Toy_Bytecode* mb, Toy_Ast* ast) {
	//recursive checks
	if (ast == NULL) {
		return 0;
	}
	else if (ast->type == TOY_AST_AGGREGATE) {
		unsigned int total = 0;
		total += emitParameters(mb, ast->aggregate.left);
		total += emitParameters(mb, ast->aggregate.right);
		return total;
	}
	else if (ast->type != TOY_AST_VAR_DECLARE) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Unknown AST type passed to 'emitParameters()'\n" TOY_CC_RESET);
		exit(-1);
		return 0;
	}

	//the address within the data section
	char buffer[256];
	snprintf(buffer, 256, "%.*s", ast->varDeclare.name->info.length, ast->varDeclare.name->leaf.data);
	unsigned int dataAddr = emitCStringToData(&(mb->data), &(mb->dataCapacity), &(mb->dataCount), buffer);

	//check the param index for that entry i.e. don't reuse parameter names
	for (unsigned int i = 0; i < mb->paramCount; i+=4) {
		if (*(unsigned int*)(mb->param + i) == dataAddr) {
			//not allowed
			fprintf(stderr, TOY_CC_ERROR "COMPILER ERROR: Function parameters must have unique names\n" TOY_CC_RESET);
			mb->panic = true;
			return 0;
		}
	}

	//emit to the param index
	EMIT_INT(&mb, param, dataAddr);
	EMIT_INT(&mb, param, ast->varDeclare.valueType); //'constant' is lost, but that's fine for params

	//this returns the number of written parameters
	return 1;
}

static unsigned int writeBytecodeFromAst(Toy_Bytecode** mb, Toy_Ast* ast); //forward declare for recursion
static void writeBytecodeBody(Toy_Bytecode* mb, Toy_Ast* ast);
static unsigned char* collateBytecodeBody(Toy_Bytecode* mb);
static unsigned int writeInstructionAssign(Toy_Bytecode** mb, Toy_AstVarAssign ast, bool chainedAssignment); //forward declare for chaining of var declarations
static unsigned int writeInstructionAccess(Toy_Bytecode** mb, Toy_AstVarAccess ast);
static unsigned int writeInstructionFnInvoke(Toy_Bytecode** mb, Toy_AstFnInvoke ast, bool chainedInvoke);

static unsigned int writeInstructionValue(Toy_Bytecode** mb, Toy_AstValue ast) {
	EMIT_BYTE(mb, code, TOY_OPCODE_READ);
	EMIT_BYTE(mb, code, ast.value.type);

	//emit the raw value based on the type
	if (TOY_VALUE_IS_NULL(ast.value)) {
		//NOTHING - null's type data is enough

		//4-byte alignment
		EMIT_BYTE(mb, code, 0);
		EMIT_BYTE(mb, code, 0);
	}
	else if (TOY_VALUE_IS_BOOLEAN(ast.value)) {
		EMIT_BYTE(mb, code, TOY_VALUE_AS_BOOLEAN(ast.value));

		//4-byte alignment
		EMIT_BYTE(mb, code, 0);
	}
	else if (TOY_VALUE_IS_INTEGER(ast.value)) {
		//4-byte alignment
		EMIT_BYTE(mb, code, 0);
		EMIT_BYTE(mb, code, 0);

		EMIT_INT(mb, code, TOY_VALUE_AS_INTEGER(ast.value));
	}
	else if (TOY_VALUE_IS_FLOAT(ast.value)) {
		//4-byte alignment
		EMIT_BYTE(mb, code, 0);
		EMIT_BYTE(mb, code, 0);

		EMIT_FLOAT(mb, code, TOY_VALUE_AS_FLOAT(ast.value));
	}
	else if (TOY_VALUE_IS_STRING(ast.value)) {
		//4-byte alignment
		EMIT_BYTE(mb, code, TOY_STRING_LEAF); //normal string
		EMIT_BYTE(mb, code, 0); //can't store the length

		return emitString(mb, TOY_VALUE_AS_STRING(ast.value));
	}
	else {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid AST type found: Unknown value type\n" TOY_CC_RESET);
		exit(-1);
	}

	return 1;
}

static unsigned int writeInstructionUnary(Toy_Bytecode** mb, Toy_AstUnary ast) {
	unsigned int result = 0;

	if (ast.flag == TOY_AST_FLAG_NEGATE) {
		result = writeBytecodeFromAst(mb, ast.child);

		EMIT_BYTE(mb, code, TOY_OPCODE_NEGATE);

		//4-byte alignment
		EMIT_BYTE(mb, code, 0);
		EMIT_BYTE(mb, code, 0);
		EMIT_BYTE(mb, code, 0);
	}

	else if (ast.flag == TOY_AST_FLAG_PREFIX_INCREMENT || ast.flag == TOY_AST_FLAG_PREFIX_DECREMENT) { //NOTE: tightly coupled to the parser's logic, and somewhat duplicates ACCESS
		//read the var name onto the stack
		Toy_String* name = TOY_VALUE_AS_STRING(ast.child->value.value);

		EMIT_BYTE(mb, code, TOY_OPCODE_READ);
		EMIT_BYTE(mb, code, TOY_VALUE_STRING);
		EMIT_BYTE(mb, code, TOY_STRING_LEAF);
		EMIT_BYTE(mb, code, name->info.length); //store the length (max 255)

		emitString(mb, name);

		//duplicate the var name, then get the value
		EMIT_BYTE(mb, code,TOY_OPCODE_DUPLICATE);
		EMIT_BYTE(mb, code, TOY_OPCODE_ACCESS); //squeezed
		EMIT_BYTE(mb, code, 0);
		EMIT_BYTE(mb, code, 0);

		//read the integer '1'
		EMIT_BYTE(mb, code, TOY_OPCODE_READ);
		EMIT_BYTE(mb, code, TOY_VALUE_INTEGER);
		EMIT_BYTE(mb, code, 0);
		EMIT_BYTE(mb, code, 0);

		EMIT_INT(mb, code, 1);

		//add (or subtract) the two values, then assign (pops the second duplicate, and leaves value on the stack)
		EMIT_BYTE(mb, code, ast.flag == TOY_AST_FLAG_PREFIX_INCREMENT ? TOY_OPCODE_ADD : TOY_OPCODE_SUBTRACT);
		EMIT_BYTE(mb, code,TOY_OPCODE_ASSIGN); //squeezed
		EMIT_BYTE(mb, code,1);
		EMIT_BYTE(mb, code,0);

		//leaves one value on the stack
		result = 1;
	}

	else if (ast.flag == TOY_AST_FLAG_POSTFIX_INCREMENT || ast.flag == TOY_AST_FLAG_POSTFIX_DECREMENT) { //NOTE: ditto
		//read the var name onto the stack
		Toy_String* name = TOY_VALUE_AS_STRING(ast.child->value.value);

		EMIT_BYTE(mb, code, TOY_OPCODE_READ);
		EMIT_BYTE(mb, code, TOY_VALUE_STRING);
		EMIT_BYTE(mb, code, TOY_STRING_LEAF);
		EMIT_BYTE(mb, code, name->info.length); //store the length (max 255)

		emitString(mb, name);

		//access the value (postfix++ and postfix--)
		EMIT_BYTE(mb, code, TOY_OPCODE_ACCESS);
		EMIT_BYTE(mb, code,0);
		EMIT_BYTE(mb, code,0);
		EMIT_BYTE(mb, code,0);

		//read the var name onto the stack (again)
		name = TOY_VALUE_AS_STRING(ast.child->value.value);

		EMIT_BYTE(mb, code, TOY_OPCODE_READ);
		EMIT_BYTE(mb, code, TOY_VALUE_STRING);
		EMIT_BYTE(mb, code, TOY_STRING_LEAF);
		EMIT_BYTE(mb, code, name->info.length); //store the length (max 255)

		emitString(mb, name);

		//duplicate the var name, then get the value
		EMIT_BYTE(mb, code,TOY_OPCODE_DUPLICATE);
		EMIT_BYTE(mb, code, TOY_OPCODE_ACCESS); //squeezed
		EMIT_BYTE(mb, code,0);
		EMIT_BYTE(mb, code,0);

		//read the integer '1'
		EMIT_BYTE(mb, code, TOY_OPCODE_READ);
		EMIT_BYTE(mb, code, TOY_VALUE_INTEGER);
		EMIT_BYTE(mb, code, 0);
		EMIT_BYTE(mb, code, 0);

		EMIT_INT(mb, code, 1);

		//add (or subtract) the two values, then assign (pops the second duplicate)
		EMIT_BYTE(mb, code, ast.flag == TOY_AST_FLAG_POSTFIX_INCREMENT ? TOY_OPCODE_ADD : TOY_OPCODE_SUBTRACT);
		EMIT_BYTE(mb, code,TOY_OPCODE_ASSIGN); //squeezed
		EMIT_BYTE(mb, code,0);
		EMIT_BYTE(mb, code,0);

		//leaves one value on the stack
		result = 1;
	}

	else if (ast.flag == TOY_AST_FLAG_INVERT) {
		result = writeBytecodeFromAst(mb, ast.child);

		EMIT_BYTE(mb, code, TOY_OPCODE_INVERT);

		//4-byte alignment
		EMIT_BYTE(mb, code, 0);
		EMIT_BYTE(mb, code, 0);
		EMIT_BYTE(mb, code, 0);
	}

	else {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid AST unary flag found\n" TOY_CC_RESET);
		exit(-1);
	}

	return result;
}

static unsigned int writeInstructionBinary(Toy_Bytecode** mb, Toy_AstBinary ast) {
	//left, then right, then the binary's operation
	writeBytecodeFromAst(mb, ast.left);
	writeBytecodeFromAst(mb, ast.right);

	if (ast.flag == TOY_AST_FLAG_ADD) {
		EMIT_BYTE(mb, code,TOY_OPCODE_ADD);
	}
	else if (ast.flag == TOY_AST_FLAG_SUBTRACT) {
		EMIT_BYTE(mb, code,TOY_OPCODE_SUBTRACT);
	}
	else if (ast.flag == TOY_AST_FLAG_MULTIPLY) {
		EMIT_BYTE(mb, code,TOY_OPCODE_MULTIPLY);
	}
	else if (ast.flag == TOY_AST_FLAG_DIVIDE) {
		EMIT_BYTE(mb, code,TOY_OPCODE_DIVIDE);
	}
	else if (ast.flag == TOY_AST_FLAG_MODULO) {
		EMIT_BYTE(mb, code,TOY_OPCODE_MODULO);
	}

	else if (ast.flag == TOY_AST_FLAG_CONCAT) {
		EMIT_BYTE(mb, code, TOY_OPCODE_CONCAT);
	}
	else {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid AST binary flag found\n" TOY_CC_RESET);
		exit(-1);
	}

	//4-byte alignment
	EMIT_BYTE(mb, code,TOY_OPCODE_PASS); //checked in combined assignments
	EMIT_BYTE(mb, code,0);
	EMIT_BYTE(mb, code,0);

	return 1; //leaves only 1 value on the stack
}

static unsigned int writeInstructionBinaryShortCircuit(Toy_Bytecode** mb, Toy_AstBinaryShortCircuit ast) {
	//lhs
	writeBytecodeFromAst(mb, ast.left);

	//duplicate the top (so the lhs can be 'returned' by this expression, if needed)
	EMIT_BYTE(mb, code,TOY_OPCODE_DUPLICATE);
	EMIT_BYTE(mb, code, 0);
	EMIT_BYTE(mb, code, 0);
	EMIT_BYTE(mb, code, 0);

	// && return the first falsy operand, or the last operand
	if (ast.flag == TOY_AST_FLAG_AND) {
		EMIT_BYTE(mb, code, TOY_OPCODE_JUMP);
		EMIT_BYTE(mb, code, TOY_OP_PARAM_JUMP_RELATIVE);
		EMIT_BYTE(mb, code, TOY_OP_PARAM_JUMP_IF_FALSE);
		EMIT_BYTE(mb, code, 0);
	}

	// || return the first truthy operand, or the last operand
	else if (ast.flag == TOY_AST_FLAG_OR) {
		EMIT_BYTE(mb, code, TOY_OPCODE_JUMP);
		EMIT_BYTE(mb, code, TOY_OP_PARAM_JUMP_RELATIVE);
		EMIT_BYTE(mb, code, TOY_OP_PARAM_JUMP_IF_TRUE);
		EMIT_BYTE(mb, code, 0);
	}

	else {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid AST binary short circuit flag found\n" TOY_CC_RESET);
		exit(-1);
	}

	//parameter address
	unsigned int paramAddr = SKIP_INT(mb, code); //parameter to be written later

	//if the lhs value isn't needed, pop it
	EMIT_BYTE(mb, code,TOY_OPCODE_ELIMINATE);
	EMIT_BYTE(mb, code, 1);
	EMIT_BYTE(mb, code, 0);
	EMIT_BYTE(mb, code, 0);

	//rhs
	writeBytecodeFromAst(mb, ast.right);

	//set the parameter
	OVERWRITE_INT(mb, code, paramAddr, CURRENT_ADDRESS(mb, code) - (paramAddr + 4));

	return 1; //leaves only 1 value on the stack
}

static unsigned int writeInstructionCompare(Toy_Bytecode** mb, Toy_AstCompare ast) {
	//left, then right, then the compare's operation
	writeBytecodeFromAst(mb, ast.left);
	writeBytecodeFromAst(mb, ast.right);

	if (ast.flag == TOY_AST_FLAG_COMPARE_EQUAL) {
		EMIT_BYTE(mb, code,TOY_OPCODE_COMPARE_EQUAL);
	}
	else if (ast.flag == TOY_AST_FLAG_COMPARE_NOT) {
		EMIT_BYTE(mb, code,TOY_OPCODE_COMPARE_EQUAL);
		EMIT_BYTE(mb, code,TOY_OPCODE_NEGATE); //squeezed
		EMIT_BYTE(mb, code,0);
		EMIT_BYTE(mb, code,0);

		return 1;
	}
	else if (ast.flag == TOY_AST_FLAG_COMPARE_LESS) {
		EMIT_BYTE(mb, code,TOY_OPCODE_COMPARE_LESS);
	}
	else if (ast.flag == TOY_AST_FLAG_COMPARE_LESS_EQUAL) {
		EMIT_BYTE(mb, code,TOY_OPCODE_COMPARE_LESS_EQUAL);
	}
	else if (ast.flag == TOY_AST_FLAG_COMPARE_GREATER) {
		EMIT_BYTE(mb, code,TOY_OPCODE_COMPARE_GREATER);
	}
	else if (ast.flag == TOY_AST_FLAG_COMPARE_GREATER_EQUAL) {
		EMIT_BYTE(mb, code,TOY_OPCODE_COMPARE_GREATER_EQUAL);
	}

	else {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid AST compare flag found\n" TOY_CC_RESET);
		exit(-1);
	}

	//4-byte alignment (covers most cases)
	EMIT_BYTE(mb, code,0);
	EMIT_BYTE(mb, code,0);
	EMIT_BYTE(mb, code,0);

	return 1; //leaves only 1 value on the stack
}

static unsigned int writeInstructionGroup(Toy_Bytecode** mb, Toy_AstGroup ast) {
	//not certain what this leaves
	return writeBytecodeFromAst(mb, ast.child);
}

static unsigned int writeInstructionCompound(Toy_Bytecode** mb, Toy_AstCompound ast) {
	unsigned int result = writeBytecodeFromAst(mb, ast.child);

	if (ast.flag == TOY_AST_FLAG_COMPOUND_ARRAY) {
		//signal how many values to read in as array elements
		EMIT_BYTE(mb, code, TOY_OPCODE_READ);
		EMIT_BYTE(mb, code, TOY_VALUE_ARRAY);

		//4-byte alignment
		EMIT_BYTE(mb, code,0);
		EMIT_BYTE(mb, code,0);

		//how many elements
		EMIT_INT(mb, code, result);

		return 1; //leaves only 1 value on the stack
	}
	if (ast.flag == TOY_AST_FLAG_COMPOUND_TABLE) {
		//signal how many values to read in as table elements
		EMIT_BYTE(mb, code, TOY_OPCODE_READ);
		EMIT_BYTE(mb, code, TOY_VALUE_TABLE);

		//4-byte alignment
		EMIT_BYTE(mb, code,0);
		EMIT_BYTE(mb, code,0);

		//how many elements
		EMIT_INT(mb, code, result);

		return 1; //leaves only 1 value on the stack
	}
	else {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid AST compound flag found\n" TOY_CC_RESET);
		exit(-1);
		return 0;
	}
}

static unsigned int writeInstructionAggregate(Toy_Bytecode** mb, Toy_AstAggregate ast) {
	unsigned int result = 0;

	//left, then right
	result += writeBytecodeFromAst(mb, ast.left);
	result += writeBytecodeFromAst(mb, ast.right);

	if (ast.flag == TOY_AST_FLAG_COLLECTION) {
		//collections are handled above
		return result;
	}
	else if (ast.flag == TOY_AST_FLAG_PAIR) {
		//pairs are handled above
		return result;
	}
	else if (ast.flag == TOY_AST_FLAG_INDEX) {
		//value[index, length]
		EMIT_BYTE(mb, code, TOY_OPCODE_INDEX);
		EMIT_BYTE(mb, code, result);

		//4-byte alignment
		EMIT_BYTE(mb, code,0);
		EMIT_BYTE(mb, code,0);

		return 1; //leaves only 1 value on the stack
	}
	else if (ast.flag == TOY_AST_FLAG_FN_ARGUMENTS) {
		//BUGFIX: invoking a function as an argument gets messy, so re-count the aggregate elements and discard 'result'
		int count = ast.left != NULL ? 2 : 1;
		Toy_Ast* iter = ast.right;

		while (iter != NULL && iter->type == TOY_AST_AGGREGATE) {
			iter = iter->aggregate.right;
			count++;
		}

		return count;
	}
	else {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid AST aggregate flag found\n" TOY_CC_RESET);
		exit(-1);
		return 0;
	}
}

static unsigned int writeInstructionAssert(Toy_Bytecode** mb, Toy_AstAssert ast) {
	//the thing to print
	writeBytecodeFromAst(mb, ast.child);
	writeBytecodeFromAst(mb, ast.message);

	//output the print opcode
	EMIT_BYTE(mb, code, TOY_OPCODE_ASSERT);

	//4-byte alignment
	EMIT_BYTE(mb, code, ast.message != NULL ? 2 : 1); //arg count
	EMIT_BYTE(mb, code,0);
	EMIT_BYTE(mb, code,0);

	return 0;
}

static unsigned int writeInstructionIfThenElse(Toy_Bytecode** mb, Toy_AstIfThenElse ast) {
	//cond-branch
	writeBytecodeFromAst(mb, ast.condBranch);

	//leave the assigned value on the stack when inside a condition
	if (checkForChainedAssign(ast.condBranch)) {
		Toy_AstVarAccess access = { .type = TOY_AST_VAR_ACCESS, .child = ast.condBranch->varAssign.target };
		writeInstructionAccess(mb, access);
	}

	//emit the jump word (opcode, type, condition, padding)
	EMIT_BYTE(mb, code, TOY_OPCODE_JUMP);
	EMIT_BYTE(mb, code, TOY_OP_PARAM_JUMP_RELATIVE);
	EMIT_BYTE(mb, code, TOY_OP_PARAM_JUMP_IF_FALSE);
	EMIT_BYTE(mb, code, 0);

	unsigned int thenParamAddr = SKIP_INT(mb, code); //parameter to be written later

	//emit then-branch
	writeBytecodeFromAst(mb, ast.thenBranch);

	if (ast.elseBranch != NULL) {
		//emit the jump-to-end (opcode, type, condition, padding)
		EMIT_BYTE(mb, code, TOY_OPCODE_JUMP);
		EMIT_BYTE(mb, code, TOY_OP_PARAM_JUMP_RELATIVE);
		EMIT_BYTE(mb, code, TOY_OP_PARAM_JUMP_ALWAYS);
		EMIT_BYTE(mb, code, 0);

		unsigned int elseParamAddr = SKIP_INT(mb, code); //parameter to be written later

		//specify the starting position for the else branch
		OVERWRITE_INT(mb, code, thenParamAddr, CURRENT_ADDRESS(mb, code) - (thenParamAddr + 4));

		//emit the else branch
		writeBytecodeFromAst(mb, ast.elseBranch);

		//specify the ending position for the else branch
		OVERWRITE_INT(mb, code, elseParamAddr, CURRENT_ADDRESS(mb, code) - (elseParamAddr + 4));
	}

	else {
		//without an else branch, set the jump destination and move on
		OVERWRITE_INT(mb, code, thenParamAddr, CURRENT_ADDRESS(mb, code) - (thenParamAddr + 4));
	}

	return 0;
}

static unsigned int writeInstructionWhileThen(Toy_Bytecode** mb, Toy_AstWhileThen ast) {
	//begin
	unsigned int beginAddr = CURRENT_ADDRESS(mb, code);

	//cond-branch
	writeBytecodeFromAst(mb, ast.condBranch);

	//leave the assigned value on the stack when inside a condition
	if (checkForChainedAssign(ast.condBranch)) {
		Toy_AstVarAccess access = { .type = TOY_AST_VAR_ACCESS, .child = ast.condBranch->varAssign.target };
		writeInstructionAccess(mb, access);
	}

	//emit the jump word (opcode, type, condition, padding)
	EMIT_BYTE(mb, code, TOY_OPCODE_JUMP);
	EMIT_BYTE(mb, code, TOY_OP_PARAM_JUMP_RELATIVE);
	EMIT_BYTE(mb, code, TOY_OP_PARAM_JUMP_IF_FALSE);
	EMIT_BYTE(mb, code, 0);

	unsigned int paramAddr = SKIP_INT(mb, code); //parameter to be written later

	//emit then-branch
	writeBytecodeFromAst(mb, ast.thenBranch);

	//jump to begin to repeat the conditional test
	EMIT_BYTE(mb, code, TOY_OPCODE_JUMP);
	EMIT_BYTE(mb, code, TOY_OP_PARAM_JUMP_RELATIVE);
	EMIT_BYTE(mb, code, TOY_OP_PARAM_JUMP_ALWAYS);
	EMIT_BYTE(mb, code, 0);

	EMIT_INT(mb, code, beginAddr - (CURRENT_ADDRESS(mb, code) + 4)); //this sets a negative value

	//set the exit parameter for the cond
	OVERWRITE_INT(mb, code, paramAddr, CURRENT_ADDRESS(mb, code) - (paramAddr + 4));

	//set the break & continue data
	while ((*mb)->breakEscapes->count > 0) {
		//extract
		unsigned int addr = (*mb)->breakEscapes->data[(*mb)->breakEscapes->count - 1].addr;
		unsigned int depth = (*mb)->breakEscapes->data[(*mb)->breakEscapes->count - 1].depth;

		unsigned int diff = depth - (*mb)->currentScopeDepth;

		OVERWRITE_INT(mb, code, addr, CURRENT_ADDRESS(mb, code) - (addr + 8)); //tell break to come here AFTER reading the instruction
		OVERWRITE_INT(mb, code, addr, diff);

		//tick down
		(*mb)->breakEscapes->count--;
	}

	while ((*mb)->continueEscapes->count > 0) {
		//extract
		unsigned int addr = (*mb)->continueEscapes->data[(*mb)->continueEscapes->count - 1].addr;
		unsigned int depth = (*mb)->continueEscapes->data[(*mb)->continueEscapes->count - 1].depth;

		unsigned int diff = depth - (*mb)->currentScopeDepth;

		OVERWRITE_INT(mb, code, addr, beginAddr - (addr + 8)); //tell continue to return to the start AFTER reading the instruction
		OVERWRITE_INT(mb, code, addr, diff);

		//tick down
		(*mb)->continueEscapes->count--;
	}

	return 0;
}

static unsigned int writeInstructionForThen(Toy_Bytecode** mb, Toy_AstForThen ast) {
	//check the operands
	if (ast.condBranch->type != TOY_AST_ITERABLE || ast.condBranch->iterable.left->type != TOY_AST_VAR_DECLARE || ast.condBranch->iterable.right->type != TOY_AST_VAR_ACCESS) {
		fprintf(stderr, TOY_CC_ERROR "COMPILER ERROR: Invalid conditional found in a 'for' loop\n" TOY_CC_RESET);
		(*mb)->panic = true;
		return 0;
	}

	//set up the iterable, and the counter
	writeInstructionAccess(mb, ast.condBranch->iterable.right->varAccess);

	EMIT_BYTE(mb, code, TOY_OPCODE_READ);
	EMIT_BYTE(mb, code, TOY_VALUE_INTEGER);
	EMIT_BYTE(mb, code, 0);
	EMIT_BYTE(mb, code, 0);

	EMIT_INT(mb, code, 0); //start from zero

	unsigned int beginAddr = CURRENT_ADDRESS(mb, code);

	//access [-1] from [-2], incrementing [-1] afterwards
	//then delegate to JUMP
	EMIT_BYTE(mb, code, TOY_OPCODE_ITERATE);
	EMIT_BYTE(mb, code, TOY_OP_PARAM_JUMP_RELATIVE);
	EMIT_BYTE(mb, code, TOY_OP_PARAM_JUMP_IF_NULL);
	EMIT_BYTE(mb, code, 0);

	unsigned int thenParamAddr = SKIP_INT(mb, code); //parameter to be written later

	//push scope (built-into the keyword)
	EMIT_BYTE(mb, code, TOY_OPCODE_SCOPE_PUSH);
	EMIT_BYTE(mb, code, 0);
	EMIT_BYTE(mb, code, 0);
	EMIT_BYTE(mb, code, 0);
	(*mb)->currentScopeDepth++;

	//delcare the iterator with the given string
	EMIT_BYTE(mb, code, TOY_OPCODE_DECLARE);
	EMIT_BYTE(mb, code, ast.condBranch->iterable.left->varDeclare.valueType);
	EMIT_BYTE(mb, code, ast.condBranch->iterable.left->varDeclare.name->info.length); //quick optimisation to skip a 'strlen()' call
	EMIT_BYTE(mb, code, ast.condBranch->iterable.left->varDeclare.constant); //check for constness

	emitString(mb, ast.condBranch->iterable.left->varDeclare.name);

	//TODO: use a different approach
	//BUGFIX: shadow the iterable's name
	EMIT_BYTE(mb, code, TOY_OPCODE_READ);
	EMIT_BYTE(mb, code, TOY_VALUE_NULL);
	EMIT_BYTE(mb, code, 0);
	EMIT_BYTE(mb, code, 0);

	EMIT_BYTE(mb, code, TOY_OPCODE_DECLARE);
	EMIT_BYTE(mb, code, TOY_VALUE_NULL);
	EMIT_BYTE(mb, code, TOY_VALUE_AS_STRING(ast.condBranch->iterable.right->varAccess.child->value.value)->info.length);
	EMIT_BYTE(mb, code, true); //check for constness

	emitString(mb, TOY_VALUE_AS_STRING(ast.condBranch->iterable.right->varAccess.child->value.value));

	//write the body
	writeBytecodeFromAst(mb, ast.thenBranch);

	//pop scope after each iteration
	EMIT_BYTE(mb, code, TOY_OPCODE_SCOPE_POP);
	EMIT_BYTE(mb, code, 0);
	EMIT_BYTE(mb, code, 0);
	EMIT_BYTE(mb, code, 0);
	(*mb)->currentScopeDepth--;

	//jump to begin to repeat the conditional test
	EMIT_BYTE(mb, code, TOY_OPCODE_JUMP);
	EMIT_BYTE(mb, code, TOY_OP_PARAM_JUMP_RELATIVE);
	EMIT_BYTE(mb, code, TOY_OP_PARAM_JUMP_ALWAYS);
	EMIT_BYTE(mb, code, 0);

	EMIT_INT(mb, code, beginAddr - (CURRENT_ADDRESS(mb, code) + 4)); //this sets a negative value

	//end of the loop, overwrite the parameter
	OVERWRITE_INT(mb, code, thenParamAddr, CURRENT_ADDRESS(mb, code) - (thenParamAddr + 4));

	//set the break & continue data
	while ((*mb)->breakEscapes->count > 0) {
		//extract
		unsigned int addr = (*mb)->breakEscapes->data[(*mb)->breakEscapes->count - 1].addr;
		unsigned int depth = (*mb)->breakEscapes->data[(*mb)->breakEscapes->count - 1].depth;

		unsigned int diff = depth - (*mb)->currentScopeDepth;

		OVERWRITE_INT(mb, code, addr, CURRENT_ADDRESS(mb, code) - (addr + 8)); //tell break to come here AFTER reading the instruction
		OVERWRITE_INT(mb, code, addr, diff);

		//tick down
		(*mb)->breakEscapes->count--;
	}

	while ((*mb)->continueEscapes->count > 0) {
		//extract
		unsigned int addr = (*mb)->continueEscapes->data[(*mb)->continueEscapes->count - 1].addr;
		unsigned int depth = (*mb)->continueEscapes->data[(*mb)->continueEscapes->count - 1].depth;

		unsigned int diff = depth - (*mb)->currentScopeDepth;

		OVERWRITE_INT(mb, code, addr, beginAddr - (addr + 8)); //tell continue to return to the start AFTER reading the instruction
		OVERWRITE_INT(mb, code, addr, diff);

		//tick down
		(*mb)->continueEscapes->count--;
	}

	//eliminate the value & counter from within the bytecode, so they're cleaned up after breaks
	EMIT_BYTE(mb, code,TOY_OPCODE_ELIMINATE);
	EMIT_BYTE(mb, code, 2);
	EMIT_BYTE(mb, code, 0);
	EMIT_BYTE(mb, code, 0);

	return 0;
}

static unsigned int writeInstructionBreak(Toy_Bytecode** mb, Toy_AstBreak ast) {
	//unused
	(void)ast;

	//escapes are always relative
	EMIT_BYTE(mb, code, TOY_OPCODE_ESCAPE);
	EMIT_BYTE(mb, code, 0);
	EMIT_BYTE(mb, code, 0);
	EMIT_BYTE(mb, code, 0);

	unsigned int addr = SKIP_INT(mb, code);
	(void)SKIP_INT(mb, code); //empty space for depth

	//expand the escape array if needed
	if ((*mb)->breakEscapes->capacity <= (*mb)->breakEscapes->count) {
		(*mb)->breakEscapes = Toy_private_resizeEscapeArray((*mb)->breakEscapes, (*mb)->breakEscapes->capacity * TOY_ESCAPE_EXPANSION_RATE);
	}

	//store for later
	(*mb)->breakEscapes->data[(*mb)->breakEscapes->count++] = (Toy_private_EscapeEntry_t){ .addr = addr, .depth = (*mb)->currentScopeDepth };

	return 0;
}

static unsigned int writeInstructionContinue(Toy_Bytecode** mb, Toy_AstContinue ast) {
	//unused
	(void)ast;

	//escapes are always relative
	EMIT_BYTE(mb, code, TOY_OPCODE_ESCAPE);
	EMIT_BYTE(mb, code, 0);
	EMIT_BYTE(mb, code, 0);
	EMIT_BYTE(mb, code, 0);

	unsigned int addr = SKIP_INT(mb, code);
	(void)SKIP_INT(mb, code); //empty space for depth

	//expand the escape array if needed
	if ((*mb)->continueEscapes->capacity <= (*mb)->continueEscapes->count) {
		(*mb)->continueEscapes = Toy_private_resizeEscapeArray((*mb)->continueEscapes, (*mb)->continueEscapes->capacity * TOY_ESCAPE_EXPANSION_RATE);
	}

	//store for later
	(*mb)->continueEscapes->data[(*mb)->continueEscapes->count++] = (Toy_private_EscapeEntry_t){ .addr = addr, .depth = (*mb)->currentScopeDepth };

	return 0;
}

static unsigned int writeInstructionReturn(Toy_Bytecode** mb, Toy_AstReturn ast) {
	unsigned int retCount = 0;

	//what is returned
	if (checkForChainedAssign(ast.child)) {
		retCount = writeInstructionAssign(mb, ast.child->varAssign, true);
	}
	else if (checkForChainedInvoke(ast.child)) {
		retCount = writeInstructionFnInvoke(mb, ast.child->fnInvoke, true);
	}
	else {
		retCount = writeBytecodeFromAst(mb, ast.child); //default value
	}

	//output the print opcode
	EMIT_BYTE(mb, code,TOY_OPCODE_RETURN);

	//4-byte alignment
	EMIT_BYTE(mb, code,(unsigned char)retCount);
	EMIT_BYTE(mb, code,0);
	EMIT_BYTE(mb, code,0);

	return retCount;
}

static unsigned int writeInstructionPrint(Toy_Bytecode** mb, Toy_AstPrint ast) {
	//the thing to print
	writeBytecodeFromAst(mb, ast.child);

	//output the print opcode
	EMIT_BYTE(mb, code,TOY_OPCODE_PRINT);

	//4-byte alignment
	EMIT_BYTE(mb, code,0);
	EMIT_BYTE(mb, code,0);
	EMIT_BYTE(mb, code,0);

	return 0;
}

static unsigned int writeInstructionVarDeclare(Toy_Bytecode** mb, Toy_AstVarDeclare ast) {
	//if we're dealing with chained assignments, hijack the next assignment with 'chainedAssignment' set to true
	if (checkForChainedAssign(ast.expr)) {
		writeInstructionAssign(mb, ast.expr->varAssign, true);
	}
	else if (checkForChainedInvoke(ast.expr)) {
		writeInstructionFnInvoke(mb, ast.expr->fnInvoke, true);
	}
	else {
		writeBytecodeFromAst(mb, ast.expr); //default value
	}

	//delcare with the given name string
	EMIT_BYTE(mb, code, TOY_OPCODE_DECLARE);
	EMIT_BYTE(mb, code, ast.valueType);
	EMIT_BYTE(mb, code, ast.name->info.length); //quick optimisation to skip a 'strlen()' call
	EMIT_BYTE(mb, code, ast.constant); //check for constness

	emitString(mb, ast.name);

	return 0;
}

static unsigned int writeInstructionAssign(Toy_Bytecode** mb, Toy_AstVarAssign ast, bool chainedAssignment) {
	unsigned int result = 0;

	//target is a variable name
	if (ast.target->type == TOY_AST_VALUE && TOY_VALUE_IS_STRING(ast.target->value.value)) {
		//name string
		Toy_String* target = TOY_VALUE_AS_STRING(ast.target->value.value);

		//emit the name string
		EMIT_BYTE(mb, code, TOY_OPCODE_READ);
		EMIT_BYTE(mb, code, TOY_VALUE_STRING);
		EMIT_BYTE(mb, code, TOY_STRING_LEAF);
		EMIT_BYTE(mb, code, target->info.length); //store the length (max 255)

		emitString(mb, target);
	}

	//target is an indexing of some compound value
	else if (ast.target->type == TOY_AST_AGGREGATE && ast.target->aggregate.flag == TOY_AST_FLAG_INDEX) {
		writeBytecodeFromAst(mb, ast.target->aggregate.left); //any deeper indexing will just work, using reference values
		writeBytecodeFromAst(mb, ast.target->aggregate.right); //key

		//if we're dealing with chained assignments, hijack the next assignment with 'chainedAssignment' set to true
		if (checkForChainedAssign(ast.expr)) {
			result += writeInstructionAssign(mb, ast.expr->varAssign, true);
		}
		else if (checkForChainedInvoke(ast.expr)) {
			result += writeInstructionFnInvoke(mb, ast.expr->fnInvoke, true);
		}
		else {
			result += writeBytecodeFromAst(mb, ast.expr); //default value
		}

		EMIT_BYTE(mb, code, TOY_OPCODE_ASSIGN_COMPOUND); //uses the top three values on the stack
		EMIT_BYTE(mb, code, chainedAssignment);
		EMIT_BYTE(mb, code,0);
		EMIT_BYTE(mb, code,0);

		return result + (chainedAssignment ? 1 : 0);
	}

	else {
		//unknown target
		fprintf(stderr, TOY_CC_ERROR "COMPILER ERROR: Invalid AST type found: Malformed assignment target\n" TOY_CC_RESET);
		(*mb)->panic = true;
		return 0;
	}

	//determine RHS, include duplication if needed
	if (ast.flag == TOY_AST_FLAG_ASSIGN) {
		//if we're dealing with chained assignments, hijack the next assignment with 'chainedAssignment' set to true
		if (checkForChainedAssign(ast.expr)) {
			result += writeInstructionAssign(mb, ast.expr->varAssign, true);
		}
		else if (checkForChainedInvoke(ast.expr)) {
			result += writeInstructionFnInvoke(mb, ast.expr->fnInvoke, true);
		}
		else {
			result += writeBytecodeFromAst(mb, ast.expr); //default value
		}

		EMIT_BYTE(mb, code, TOY_OPCODE_ASSIGN);
		EMIT_BYTE(mb, code, chainedAssignment);
		EMIT_BYTE(mb, code,0);
		EMIT_BYTE(mb, code,0);
	}
	else if (ast.flag == TOY_AST_FLAG_ADD_ASSIGN) {
		EMIT_BYTE(mb, code,TOY_OPCODE_DUPLICATE);
		EMIT_BYTE(mb, code,TOY_OPCODE_ACCESS); //squeezed
		EMIT_BYTE(mb, code,0);
		EMIT_BYTE(mb, code,0);

		//if we're dealing with chained assignments, hijack the next assignment with 'chainedAssignment' set to true
		if (checkForChainedAssign(ast.expr)) {
			result += writeInstructionAssign(mb, ast.expr->varAssign, true);
		}
		else if (checkForChainedInvoke(ast.expr)) {
			result += writeInstructionFnInvoke(mb, ast.expr->fnInvoke, true);
		}
		else {
			result += writeBytecodeFromAst(mb, ast.expr); //default value
		}

		EMIT_BYTE(mb, code,TOY_OPCODE_ADD);
		EMIT_BYTE(mb, code,TOY_OPCODE_ASSIGN); //squeezed
		EMIT_BYTE(mb, code, chainedAssignment);
		EMIT_BYTE(mb, code,0);
	}
	else if (ast.flag == TOY_AST_FLAG_SUBTRACT_ASSIGN) {
		EMIT_BYTE(mb, code,TOY_OPCODE_DUPLICATE);
		EMIT_BYTE(mb, code,TOY_OPCODE_ACCESS); //squeezed
		EMIT_BYTE(mb, code,0);
		EMIT_BYTE(mb, code,0);

		//if we're dealing with chained assignments, hijack the next assignment with 'chainedAssignment' set to true
		if (checkForChainedAssign(ast.expr)) {
			result += writeInstructionAssign(mb, ast.expr->varAssign, true);
		}
		else if (checkForChainedInvoke(ast.expr)) {
			result += writeInstructionFnInvoke(mb, ast.expr->fnInvoke, true);
		}
		else {
			result += writeBytecodeFromAst(mb, ast.expr); //default value
		}

		EMIT_BYTE(mb, code,TOY_OPCODE_SUBTRACT);
		EMIT_BYTE(mb, code,TOY_OPCODE_ASSIGN); //squeezed
		EMIT_BYTE(mb, code, chainedAssignment);
		EMIT_BYTE(mb, code,0);
	}
	else if (ast.flag == TOY_AST_FLAG_MULTIPLY_ASSIGN) {
		EMIT_BYTE(mb, code,TOY_OPCODE_DUPLICATE);
		EMIT_BYTE(mb, code,TOY_OPCODE_ACCESS); //squeezed
		EMIT_BYTE(mb, code,0);
		EMIT_BYTE(mb, code,0);

		//if we're dealing with chained assignments, hijack the next assignment with 'chainedAssignment' set to true
		if (checkForChainedAssign(ast.expr)) {
			result += writeInstructionAssign(mb, ast.expr->varAssign, true);
		}
		else if (checkForChainedInvoke(ast.expr)) {
			result += writeInstructionFnInvoke(mb, ast.expr->fnInvoke, true);
		}
		else {
			result += writeBytecodeFromAst(mb, ast.expr); //default value
		}

		EMIT_BYTE(mb, code,TOY_OPCODE_MULTIPLY);
		EMIT_BYTE(mb, code,TOY_OPCODE_ASSIGN); //squeezed
		EMIT_BYTE(mb, code, chainedAssignment);
		EMIT_BYTE(mb, code,0);
	}
	else if (ast.flag == TOY_AST_FLAG_DIVIDE_ASSIGN) {
		EMIT_BYTE(mb, code,TOY_OPCODE_DUPLICATE);
		EMIT_BYTE(mb, code,TOY_OPCODE_ACCESS); //squeezed
		EMIT_BYTE(mb, code,0);
		EMIT_BYTE(mb, code,0);

		//if we're dealing with chained assignments, hijack the next assignment with 'chainedAssignment' set to true
		if (checkForChainedAssign(ast.expr)) {
			result += writeInstructionAssign(mb, ast.expr->varAssign, true);
		}
		else if (checkForChainedInvoke(ast.expr)) {
			result += writeInstructionFnInvoke(mb, ast.expr->fnInvoke, true);
		}
		else {
			result += writeBytecodeFromAst(mb, ast.expr); //default value
		}

		EMIT_BYTE(mb, code,TOY_OPCODE_DIVIDE);
		EMIT_BYTE(mb, code,TOY_OPCODE_ASSIGN); //squeezed
		EMIT_BYTE(mb, code, chainedAssignment);
		EMIT_BYTE(mb, code,0);
	}
	else if (ast.flag == TOY_AST_FLAG_MODULO_ASSIGN) {
		EMIT_BYTE(mb, code,TOY_OPCODE_DUPLICATE);
		EMIT_BYTE(mb, code,TOY_OPCODE_ACCESS); //squeezed
		EMIT_BYTE(mb, code,0);
		EMIT_BYTE(mb, code,0);

		//if we're dealing with chained assignments, hijack the next assignment with 'chainedAssignment' set to true
		if (checkForChainedAssign(ast.expr)) {
			result += writeInstructionAssign(mb, ast.expr->varAssign, true);
		}
		else if (checkForChainedInvoke(ast.expr)) {
			result += writeInstructionFnInvoke(mb, ast.expr->fnInvoke, true);
		}
		else {
			result += writeBytecodeFromAst(mb, ast.expr); //default value
		}

		EMIT_BYTE(mb, code,TOY_OPCODE_MODULO);
		EMIT_BYTE(mb, code,TOY_OPCODE_ASSIGN); //squeezed
		EMIT_BYTE(mb, code, chainedAssignment);
		EMIT_BYTE(mb, code,0);
	}

	else {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid AST assign flag found\n" TOY_CC_RESET);
		exit(-1);
	}

	return result;
}

static unsigned int writeInstructionAccess(Toy_Bytecode** mb, Toy_AstVarAccess ast) {
	if (!(ast.child->type == TOY_AST_VALUE && TOY_VALUE_IS_STRING(ast.child->value.value))) {
		fprintf(stderr, TOY_CC_ERROR "COMPILER ERROR: Found a non-name-string in a value node when trying to write access\n" TOY_CC_RESET);
		(*mb)->panic = true;
		return 0;
	}

	Toy_String* name = TOY_VALUE_AS_STRING(ast.child->value.value);

	//push the name
	EMIT_BYTE(mb, code, TOY_OPCODE_READ);
	EMIT_BYTE(mb, code, TOY_VALUE_STRING);
	EMIT_BYTE(mb, code, TOY_STRING_LEAF);
	EMIT_BYTE(mb, code, name->info.length); //store the length (max 255)

	emitString(mb, name);

	//convert name to value
	EMIT_BYTE(mb, code, TOY_OPCODE_ACCESS);
	EMIT_BYTE(mb, code,0);
	EMIT_BYTE(mb, code,0);
	EMIT_BYTE(mb, code,0);

	return 1;
}

static unsigned int writeInstructionFnDeclare(Toy_Bytecode** mb, Toy_AstFnDeclare ast) {
	/*
	FnDeclare: name, params, body

	fn name(param1: int, param2: float, param3: string, param4) {
		//
	}

	ast->params->aggregate
		.left->aggregate
			.left->aggregate
				.left->aggregate
					.left = (NULL)
					.right->value.value.as.string.name (param1: int)
				.right->value.value.as.string.name (param2: float)
			.right->value.value.as.string.name (param3: string)
		.right->value.value.as.string.name (param4: any)
	*/

	//generate the subroutine
	Toy_Bytecode compiler = { 0 };

	compiler.breakEscapes = Toy_private_resizeEscapeArray(NULL, TOY_ESCAPE_INITIAL_CAPACITY);
	compiler.continueEscapes = Toy_private_resizeEscapeArray(NULL, TOY_ESCAPE_INITIAL_CAPACITY);

	//compile the ast to memory
	unsigned int paramCount = emitParameters(&compiler, ast.params);
	writeBytecodeBody(&compiler, ast.body);
	unsigned char* subroutine = collateBytecodeBody(&compiler);

	//cleanup the compiler
	Toy_private_resizeEscapeArray(compiler.breakEscapes, 0);
	Toy_private_resizeEscapeArray(compiler.continueEscapes, 0);

	free(compiler.param);
	free(compiler.code);
	free(compiler.jumps);
	free(compiler.data);
	free(compiler.subs);

	//write the subroutine to the subs section
	unsigned int subsAddr = (*mb)->subsCount;
	emitBuffer(&((*mb)->subs), &((*mb)->subsCapacity), &((*mb)->subsCount), subroutine, *((unsigned int*)subroutine));
	free(subroutine);

	//read the function as a value, with the address as a parameter
	EMIT_BYTE(mb, code, TOY_OPCODE_READ);
	EMIT_BYTE(mb, code, TOY_VALUE_FUNCTION);
	EMIT_BYTE(mb, code, (unsigned char)paramCount);
	EMIT_BYTE(mb, code, 0);

	EMIT_INT(mb, code, subsAddr);

	//delcare the function
	EMIT_BYTE(mb, code, TOY_OPCODE_DECLARE);
	EMIT_BYTE(mb, code, TOY_VALUE_FUNCTION);
	EMIT_BYTE(mb, code, ast.name->info.length); //quick optimisation to skip a 'strlen()' call
	EMIT_BYTE(mb, code, true); //functions are const for now

	//time to write to the actual function name
	emitString(mb, ast.name);

	return 0;
}

static unsigned int writeInstructionFnInvoke(Toy_Bytecode** mb, Toy_AstFnInvoke ast, bool chainedInvoke) {
	unsigned int argCount = writeBytecodeFromAst(mb, ast.args);

	if (argCount > 255) {
		fprintf(stderr, TOY_CC_ERROR "COMPILER ERROR: Invalid function invokation with %d functions arguments (maximum 255)\n" TOY_CC_RESET, (int)argCount);
		(*mb)->panic = true;
		return 0;
	}

	unsigned int fnCount = writeBytecodeFromAst(mb, ast.function);

	if (fnCount != 1) {
		fprintf(stderr, TOY_CC_ERROR "COMPILER ERROR: Invalid function invokation with %d function AST nodes (expected 1)\n" TOY_CC_RESET, (int)fnCount);
		(*mb)->panic = true;
		return 0;
	}

	//call the function
	EMIT_BYTE(mb, code, TOY_OPCODE_INVOKE);
	EMIT_BYTE(mb, code, TOY_VALUE_FUNCTION);
	EMIT_BYTE(mb, code, (unsigned char)argCount);
	EMIT_BYTE(mb, code, 0); //BUG: IDK how many returns

	return chainedInvoke ? 1 : 0;
}

static unsigned int writeInstructionAttribute(Toy_Bytecode** mb, Toy_AstAttribute ast) {
	//write the lhs normally
	unsigned int result = writeBytecodeFromAst(mb, ast.left);

	//write the attribute's identifier
	result += writeBytecodeFromAst(mb, ast.right);

	//exactly what this results in is type-dependant
	EMIT_BYTE(mb, code,TOY_OPCODE_ATTRIBUTE);
	EMIT_BYTE(mb, code, 0);
	EMIT_BYTE(mb, code, 0);
	EMIT_BYTE(mb, code, 0);

	(void)result;

	return 1;
}

static unsigned int writeInstructionStackPop(Toy_Bytecode** mb, Toy_AstStackPop ast) {
	unsigned int result = writeBytecodeFromAst(mb, ast.child);

	//dead simple
	EMIT_BYTE(mb, code,TOY_OPCODE_ELIMINATE);
	EMIT_BYTE(mb, code, 1);
	EMIT_BYTE(mb, code, 0);
	EMIT_BYTE(mb, code, 0);

	return result - 1;
}


static unsigned int writeBytecodeFromAst(Toy_Bytecode** mb, Toy_Ast* ast) {
	if (ast == NULL) {
		return 0;
	}

	//if an error occured, just exit
	if (mb == NULL || (*mb) == NULL || (*mb)->panic) {
		return 0;
	}

	//NOTE: 'result' is used to in 'writeInstructionAggregate()'
	unsigned int result = 0;

	//determine how to write each instruction based on the Ast
	switch(ast->type) {
		case TOY_AST_BLOCK:
			if (ast->block.innerScope) {
				EMIT_BYTE(mb, code, TOY_OPCODE_SCOPE_PUSH);
				EMIT_BYTE(mb, code, 0);
				EMIT_BYTE(mb, code, 0);
				EMIT_BYTE(mb, code, 0);

				(*mb)->currentScopeDepth++;
			}

			result += writeBytecodeFromAst(mb, ast->block.child);
			result += writeBytecodeFromAst(mb, ast->block.next);

			if (ast->block.innerScope) {
				EMIT_BYTE(mb, code, TOY_OPCODE_SCOPE_POP);
				EMIT_BYTE(mb, code, 0);
				EMIT_BYTE(mb, code, 0);
				EMIT_BYTE(mb, code, 0);

				(*mb)->currentScopeDepth--;
			}
			break;

		case TOY_AST_VALUE:
			result += writeInstructionValue(mb, ast->value);
			break;

		case TOY_AST_UNARY:
			result += writeInstructionUnary(mb, ast->unary);
			break;

		case TOY_AST_BINARY:
			result += writeInstructionBinary(mb, ast->binary);
			break;

		case TOY_AST_BINARY_SHORT_CIRCUIT:
			result += writeInstructionBinaryShortCircuit(mb, ast->binaryShortCircuit);
			break;

		case TOY_AST_COMPARE:
			result += writeInstructionCompare(mb, ast->compare);
			break;

		case TOY_AST_GROUP:
			result += writeInstructionGroup(mb, ast->group);
			break;

		case TOY_AST_COMPOUND:
			result += writeInstructionCompound(mb, ast->compound);
			break;

		case TOY_AST_AGGREGATE:
			result += writeInstructionAggregate(mb, ast->aggregate);
			break;

		case TOY_AST_ASSERT:
			result += writeInstructionAssert(mb, ast->assert);
			break;

		case TOY_AST_IF_THEN_ELSE:
			result += writeInstructionIfThenElse(mb, ast->ifThenElse);
			break;

		case TOY_AST_WHILE_THEN:
			result += writeInstructionWhileThen(mb, ast->whileThen);
			break;

		case TOY_AST_FOR_THEN:
			result += writeInstructionForThen(mb, ast->forThen);
			break;

		case TOY_AST_BREAK:
			result += writeInstructionBreak(mb, ast->breakPoint);
			break;

		case TOY_AST_CONTINUE:
			result += writeInstructionContinue(mb, ast->continuePoint);
			break;

		case TOY_AST_RETURN:
			result += writeInstructionReturn(mb, ast->fnReturn);
			break;

		case TOY_AST_PRINT:
			result += writeInstructionPrint(mb, ast->print);
			break;

		case TOY_AST_VAR_DECLARE:
			result += writeInstructionVarDeclare(mb, ast->varDeclare);
			break;

		case TOY_AST_VAR_ASSIGN:
			result += writeInstructionAssign(mb, ast->varAssign, false);
			break;

		case TOY_AST_VAR_ACCESS:
			result += writeInstructionAccess(mb, ast->varAccess);
			break;

		case TOY_AST_FN_DECLARE:
			result += writeInstructionFnDeclare(mb, ast->fnDeclare);
			break;

		case TOY_AST_FN_INVOKE:
			result += writeInstructionFnInvoke(mb, ast->fnInvoke, false);
			break;

		case TOY_AST_ATTRIBUTE:
			result += writeInstructionAttribute(mb, ast->attribute);
			break;

		case TOY_AST_ITERABLE:
			//the 'in' keyword is only valid within a for-loop's condition
			fprintf(stderr, TOY_CC_ERROR "COMPILER ERROR: the 'in' keyword is only valid within a for-loop's condition\n" TOY_CC_RESET);
			(*mb)->panic = true;
			break;

		case TOY_AST_STACK_POP:
			result += writeInstructionStackPop(mb, ast->stackPop);
			break;

		case TOY_AST_PASS:
			//NO-OP
			break;

		//meta instructions are disallowed
		case TOY_AST_ERROR:
			fprintf(stderr, TOY_CC_ERROR "COMPILER ERROR: Invalid AST type found: Unknown 'error'\n" TOY_CC_RESET);
			(*mb)->panic = true;
			break;

		case TOY_AST_END:
			//NO-OP
			break;
	}

	return result;
}

static void writeBytecodeBody(Toy_Bytecode* mb, Toy_Ast* ast) {
	//this is separated from 'collateBytecodeBody', to separate the concerns for bytecode & functions

	writeBytecodeFromAst(&mb, ast);

	//append an extra return if needed
	if (mb->codeCount < 4 || mb->code[mb->codeCount - 4] != TOY_OPCODE_RETURN) { //if empty or no return statement
		EMIT_BYTE(&mb, code, TOY_OPCODE_RETURN); //end terminator
		EMIT_BYTE(&mb, code, 0); //4-byte alignment
		EMIT_BYTE(&mb, code, 0);
		EMIT_BYTE(&mb, code, 0);
	}
}

static unsigned char* collateBytecodeBody(Toy_Bytecode* mb) {
	//if an error occurred, just exit
	if (mb->panic) {
		return NULL;
	}

	//write the header and combine the parts
	unsigned char* buffer = NULL;
	unsigned int capacity = 0, count = 0;
	int codeAddr = 0;
	int jumpsAddr = 0;
	int paramAddr = 0;
	int dataAddr = 0;
	int subsAddr = 0;

	emitInt(&buffer, &capacity, &count, 0); //total size (overwritten later)
	emitInt(&buffer, &capacity, &count, mb->jumpsCount); //jumps size
	emitInt(&buffer, &capacity, &count, mb->paramCount); //param size
	emitInt(&buffer, &capacity, &count, mb->dataCount); //data size
	emitInt(&buffer, &capacity, &count, mb->subsCount); //routine size

	//generate blank spaces, cache their positions in the *Addr variables for later writes
	if (mb->codeCount > 0) {
		codeAddr = count;
		emitInt(&buffer, &capacity, &count, 0); //code
	}
	if (mb->jumpsCount > 0) {
		jumpsAddr = count;
		emitInt(&buffer, &capacity, &count, 0); //jumps
	}
	if (mb->paramCount > 0) {
		paramAddr = count;
		emitInt(&buffer, &capacity, &count, 0); //params
	}
	if (mb->dataCount > 0) {
		dataAddr = count;
		emitInt(&buffer, &capacity, &count, 0); //data
	}
	if (mb->subsCount > 0) {
		subsAddr = count;
		emitInt(&buffer, &capacity, &count, 0); //subs
	}

	//append various parts to the buffer
	if (mb->codeCount > 0) {
		expand(&buffer, &capacity, &count, mb->codeCount);
		memcpy((buffer + count), mb->code, mb->codeCount);

		*((int*)(buffer + codeAddr)) = count;
		count += mb->codeCount;
	}

	if (mb->jumpsCount > 0) {
		expand(&buffer, &capacity, &count, mb->jumpsCount);
		memcpy((buffer + count), mb->jumps, mb->jumpsCount);

		*((int*)(buffer + jumpsAddr)) = count;
		count += mb->jumpsCount;
	}

	if (mb->paramCount > 0) {
		expand(&buffer, &capacity, &count, mb->paramCount);
		memcpy((buffer + count), mb->param, mb->paramCount);

		*((int*)(buffer + paramAddr)) = count;
		count += mb->paramCount;
	}

	if (mb->dataCount > 0) {
		expand(&buffer, &capacity, &count, mb->dataCount);
		memcpy((buffer + count), mb->data, mb->dataCount);

		*((int*)(buffer + dataAddr)) = count;
		count += mb->dataCount;
	}

	if (mb->subsCount > 0) {
		expand(&buffer, &capacity, &count, mb->subsCount);
		memcpy((buffer + count), mb->subs, mb->subsCount);

		*((int*)(buffer + subsAddr)) = count;
		count += mb->subsCount;
	}

	//finally, record the total size within the header, and return the result
	((int*)buffer)[0] = count;

	return buffer;
}

//exposed functions
unsigned char* Toy_compileToBytecode(Toy_Ast* ast) {
	//setup
	Toy_Bytecode compiler = { 0 };

	compiler.breakEscapes = Toy_private_resizeEscapeArray(NULL, TOY_ESCAPE_INITIAL_CAPACITY);
	compiler.continueEscapes = Toy_private_resizeEscapeArray(NULL, TOY_ESCAPE_INITIAL_CAPACITY);

	//compile the ast to memory
	writeBytecodeBody(&compiler, ast);
	unsigned char* buffer = collateBytecodeBody(&compiler);

	//cleanup
	Toy_private_resizeEscapeArray(compiler.breakEscapes, 0);
	Toy_private_resizeEscapeArray(compiler.continueEscapes, 0);

	free(compiler.param);
	free(compiler.code);
	free(compiler.jumps);
	free(compiler.data);
	free(compiler.subs);

	return buffer;
}
