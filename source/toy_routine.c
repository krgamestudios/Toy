#include "toy_routine.h"
#include "toy_console_colors.h"

#include "toy_opcodes.h"
#include "toy_value.h"
#include "toy_string.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//escapes
void* Toy_private_resizeEscapeArray(Toy_private_EscapeArray* ptr, unsigned int capacity) {
	//if you're freeing everything, just return
	if (capacity == 0) {
		free(ptr);
		return NULL;
	}

	unsigned int originalCapacity = ptr == NULL ? 0 : ptr->capacity;
	unsigned int orignalCount = ptr == NULL ? 0 : ptr->count;

	ptr = (Toy_private_EscapeArray*)realloc(ptr, capacity * sizeof(Toy_private_EscapeEntry_t) + sizeof(Toy_private_EscapeArray));

	if (ptr == NULL) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to resize an escape array within 'Toy_Routine' from %d to %d capacity\n" TOY_CC_RESET, (int)originalCapacity, (int)capacity);
		exit(-1);
	}

	ptr->capacity = capacity;
	ptr->count = orignalCount;

	return ptr;
}

//utils
static void expand(unsigned char** handle, unsigned int* capacity, unsigned int* count, unsigned int amount) {
	if ((*count) + amount > (*capacity)) {
		while ((*count) + amount > (*capacity)) {
			(*capacity) = (*capacity) < 8 ? 8 : (*capacity) * 2;
		}
		(*handle) = realloc((*handle), (*capacity));

		if ((*handle) == NULL) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to allocate %d space for a part of 'Toy_Routine'\n" TOY_CC_RESET, (int)(*capacity));
			exit(1);
		}
	}
}

static void emitByte(unsigned char** handle, unsigned int* capacity, unsigned int* count, unsigned char byte) {
	expand(handle, capacity, count, 1);
	((unsigned char*)(*handle))[(*count)++] = byte;
}

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

static bool checkForChaining(Toy_Ast* ptr) {
	//BUGFIX
	if (ptr == NULL) {
		return false;
	}

	if (ptr->type == TOY_AST_VAR_ASSIGN) {
		return true;
	}

	if (ptr->type == TOY_AST_UNARY) {
		if (ptr->unary.flag >= TOY_AST_FLAG_PREFIX_INCREMENT && ptr->unary.flag <= TOY_AST_FLAG_POSTFIX_DECREMENT) {
			return true;
		}
	}

	return false;
}

//write instructions based on the AST types
#define EMIT_BYTE(rt, part, byte) \
	emitByte((&((*rt)->part)), &((*rt)->part##Capacity), &((*rt)->part##Count), byte)
#define EMIT_INT(rt, part, bytes) \
	emitInt((&((*rt)->part)), &((*rt)->part##Capacity), &((*rt)->part##Count), bytes)
#define EMIT_FLOAT(rt, part, bytes) \
	emitFloat((&((*rt)->part)), &((*rt)->part##Capacity), &((*rt)->part##Count), bytes)

//skip bytes, but return the address
#define SKIP_BYTE(rt, part) (EMIT_BYTE(rt, part, 0), ((*rt)->part##Count - 1))
#define SKIP_INT(rt, part) (EMIT_INT(rt, part, 0), ((*rt)->part##Count - 4))

//overwrite a pre-existing position
#define OVERWRITE_INT(rt, part, addr, bytes) \
	emitInt((&((*rt)->part)), &((*rt)->part##Capacity), &(addr), bytes);

//simply get the address (always an integer)
#define CURRENT_ADDRESS(rt, part) ((*rt)->part##Count)

static void emitToJumpTable(Toy_Routine** rt, unsigned int startAddr) {
	EMIT_INT(rt, code, (*rt)->jumpsCount); //mark the jump index in the code
	EMIT_INT(rt, jumps, startAddr); //save address at the jump index
}

static unsigned int emitString(Toy_Routine** rt, Toy_String* str) {
	//4-byte alignment
	unsigned int length = str->info.length + 1;
	if (length % 4 != 0) {
		length += 4 - (length % 4); //ceil
	}

	//grab the current start address
	unsigned int startAddr = (*rt)->dataCount;

	//move the string into the data section
	expand((&((*rt)->data)), &((*rt)->dataCapacity), &((*rt)->dataCount), length);

	if (str->info.type == TOY_STRING_NODE) {
		char* buffer = Toy_getStringRawBuffer(str);
		memcpy((*rt)->data + (*rt)->dataCount, buffer, str->info.length + 1);
		free(buffer);
	}
	else if (str->info.type == TOY_STRING_LEAF) {
		memcpy((*rt)->data + (*rt)->dataCount, str->leaf.data, str->info.length + 1);
	}
	else if (str->info.type == TOY_STRING_NAME) {
		memcpy((*rt)->data + (*rt)->dataCount, str->name.data, str->info.length + 1);
	}

	(*rt)->dataCount += length;

	//mark the jump position
	emitToJumpTable(rt, startAddr);

	return 1;
}

static unsigned int writeRoutineCode(Toy_Routine** rt, Toy_Ast* ast); //forward declare for recursion
static unsigned int writeInstructionAssign(Toy_Routine** rt, Toy_AstVarAssign ast, bool chainedAssignment); //forward declare for chaining of var declarations

static unsigned int writeInstructionValue(Toy_Routine** rt, Toy_AstValue ast) {
	EMIT_BYTE(rt, code, TOY_OPCODE_READ);
	EMIT_BYTE(rt, code, ast.value.type);

	//emit the raw value based on the type
	if (TOY_VALUE_IS_NULL(ast.value)) {
		//NOTHING - null's type data is enough

		//4-byte alignment
		EMIT_BYTE(rt, code, 0);
		EMIT_BYTE(rt, code, 0);
	}
	else if (TOY_VALUE_IS_BOOLEAN(ast.value)) {
		EMIT_BYTE(rt, code, TOY_VALUE_AS_BOOLEAN(ast.value));

		//4-byte alignment
		EMIT_BYTE(rt, code, 0);
	}
	else if (TOY_VALUE_IS_INTEGER(ast.value)) {
		//4-byte alignment
		EMIT_BYTE(rt, code, 0);
		EMIT_BYTE(rt, code, 0);

		EMIT_INT(rt, code, TOY_VALUE_AS_INTEGER(ast.value));
	}
	else if (TOY_VALUE_IS_FLOAT(ast.value)) {
		//4-byte alignment
		EMIT_BYTE(rt, code, 0);
		EMIT_BYTE(rt, code, 0);

		EMIT_FLOAT(rt, code, TOY_VALUE_AS_FLOAT(ast.value));
	}
	else if (TOY_VALUE_IS_STRING(ast.value)) {
		//4-byte alignment
		EMIT_BYTE(rt, code, TOY_STRING_LEAF); //normal string
		EMIT_BYTE(rt, code, 0); //can't store the length

		return emitString(rt, TOY_VALUE_AS_STRING(ast.value));
	}
	else {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid AST type found: Unknown value type\n" TOY_CC_RESET);
		exit(-1);
	}

	return 1;
}

static unsigned int writeInstructionUnary(Toy_Routine** rt, Toy_AstUnary ast) {
	unsigned int result = 0;

	if (ast.flag == TOY_AST_FLAG_NEGATE) {
		result = writeRoutineCode(rt, ast.child);

		EMIT_BYTE(rt, code, TOY_OPCODE_NEGATE);

		//4-byte alignment
		EMIT_BYTE(rt, code, 0);
		EMIT_BYTE(rt, code, 0);
		EMIT_BYTE(rt, code, 0);
	}

	else if (ast.flag == TOY_AST_FLAG_PREFIX_INCREMENT || ast.flag == TOY_AST_FLAG_PREFIX_DECREMENT) { //NOTE: tightly coupled to the parser's logic, and somewhat duplicates ACCESS
		//read the var name onto the stack
		Toy_String* name = TOY_VALUE_AS_STRING(ast.child->value.value);

		EMIT_BYTE(rt, code, TOY_OPCODE_READ);
		EMIT_BYTE(rt, code, TOY_VALUE_STRING);
		EMIT_BYTE(rt, code, TOY_STRING_NAME);
		EMIT_BYTE(rt, code, name->info.length); //store the length (max 255)

		emitString(rt, name);

		//duplicate the var name, then get the value
		EMIT_BYTE(rt, code,TOY_OPCODE_DUPLICATE);
		EMIT_BYTE(rt, code, TOY_OPCODE_ACCESS); //squeezed
		EMIT_BYTE(rt, code, 0);
		EMIT_BYTE(rt, code, 0);

		//read the integer '1'
		EMIT_BYTE(rt, code, TOY_OPCODE_READ);
		EMIT_BYTE(rt, code, TOY_VALUE_INTEGER);
		EMIT_BYTE(rt, code, 0);
		EMIT_BYTE(rt, code, 0);

		EMIT_INT(rt, code, 1);

		//add (or subtract) the two values, then assign (pops the second duplicate, and leaves value on the stack)
		EMIT_BYTE(rt, code, ast.flag == TOY_AST_FLAG_PREFIX_INCREMENT ? TOY_OPCODE_ADD : TOY_OPCODE_SUBTRACT);
		EMIT_BYTE(rt, code,TOY_OPCODE_ASSIGN); //squeezed
		EMIT_BYTE(rt, code,1);
		EMIT_BYTE(rt, code,0);

		//leaves one value on the stack
		result = 1;
	}

	else if (ast.flag == TOY_AST_FLAG_POSTFIX_INCREMENT || ast.flag == TOY_AST_FLAG_POSTFIX_DECREMENT) { //NOTE: ditto
		//read the var name onto the stack
		Toy_String* name = TOY_VALUE_AS_STRING(ast.child->value.value);

		EMIT_BYTE(rt, code, TOY_OPCODE_READ);
		EMIT_BYTE(rt, code, TOY_VALUE_STRING);
		EMIT_BYTE(rt, code, TOY_STRING_NAME);
		EMIT_BYTE(rt, code, name->info.length); //store the length (max 255)

		emitString(rt, name);

		//access the value (postfix++ and postfix--)
		EMIT_BYTE(rt, code, TOY_OPCODE_ACCESS);
		EMIT_BYTE(rt, code,0);
		EMIT_BYTE(rt, code,0);
		EMIT_BYTE(rt, code,0);

		//read the var name onto the stack (again)
		name = TOY_VALUE_AS_STRING(ast.child->value.value);

		EMIT_BYTE(rt, code, TOY_OPCODE_READ);
		EMIT_BYTE(rt, code, TOY_VALUE_STRING);
		EMIT_BYTE(rt, code, TOY_STRING_NAME);
		EMIT_BYTE(rt, code, name->info.length); //store the length (max 255)

		emitString(rt, name);

		//duplicate the var name, then get the value
		EMIT_BYTE(rt, code,TOY_OPCODE_DUPLICATE);
		EMIT_BYTE(rt, code, TOY_OPCODE_ACCESS); //squeezed
		EMIT_BYTE(rt, code,0);
		EMIT_BYTE(rt, code,0);

		//read the integer '1'
		EMIT_BYTE(rt, code, TOY_OPCODE_READ);
		EMIT_BYTE(rt, code, TOY_VALUE_INTEGER);
		EMIT_BYTE(rt, code, 0);
		EMIT_BYTE(rt, code, 0);

		EMIT_INT(rt, code, 1);

		//add (or subtract) the two values, then assign (pops the second duplicate)
		EMIT_BYTE(rt, code, ast.flag == TOY_AST_FLAG_POSTFIX_INCREMENT ? TOY_OPCODE_ADD : TOY_OPCODE_SUBTRACT);
		EMIT_BYTE(rt, code,TOY_OPCODE_ASSIGN); //squeezed
		EMIT_BYTE(rt, code,0);
		EMIT_BYTE(rt, code,0);

		//leaves one value on the stack
		result = 1;
	}

	else {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid AST unary flag found\n" TOY_CC_RESET);
		exit(-1);
	}

	return result;
}

static unsigned int writeInstructionBinary(Toy_Routine** rt, Toy_AstBinary ast) {
	//left, then right, then the binary's operation
	writeRoutineCode(rt, ast.left);
	writeRoutineCode(rt, ast.right);

	if (ast.flag == TOY_AST_FLAG_ADD) {
		EMIT_BYTE(rt, code,TOY_OPCODE_ADD);
	}
	else if (ast.flag == TOY_AST_FLAG_SUBTRACT) {
		EMIT_BYTE(rt, code,TOY_OPCODE_SUBTRACT);
	}
	else if (ast.flag == TOY_AST_FLAG_MULTIPLY) {
		EMIT_BYTE(rt, code,TOY_OPCODE_MULTIPLY);
	}
	else if (ast.flag == TOY_AST_FLAG_DIVIDE) {
		EMIT_BYTE(rt, code,TOY_OPCODE_DIVIDE);
	}
	else if (ast.flag == TOY_AST_FLAG_MODULO) {
		EMIT_BYTE(rt, code,TOY_OPCODE_MODULO);
	}

	else if (ast.flag == TOY_AST_FLAG_CONCAT) {
		EMIT_BYTE(rt, code, TOY_OPCODE_CONCAT);
	}
	else {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid AST binary flag found\n" TOY_CC_RESET);
		exit(-1);
	}

	//4-byte alignment
	EMIT_BYTE(rt, code,TOY_OPCODE_PASS); //checked in combined assignments
	EMIT_BYTE(rt, code,0);
	EMIT_BYTE(rt, code,0);

	return 1; //leaves only 1 value on the stack
}

static unsigned int writeInstructionBinaryShortCircuit(Toy_Routine** rt, Toy_AstBinaryShortCircuit ast) {
	//lhs
	writeRoutineCode(rt, ast.left);

	//duplicate the top (so the lhs can be 'returned' by this expression, if needed)
	EMIT_BYTE(rt, code,TOY_OPCODE_DUPLICATE);
	EMIT_BYTE(rt, code, 0);
	EMIT_BYTE(rt, code, 0);
	EMIT_BYTE(rt, code, 0);

	// && return the first falsy operand, or the last operand
	if (ast.flag == TOY_AST_FLAG_AND) {
		EMIT_BYTE(rt, code, TOY_OPCODE_JUMP);
		EMIT_BYTE(rt, code, TOY_OP_PARAM_JUMP_RELATIVE);
		EMIT_BYTE(rt, code, TOY_OP_PARAM_JUMP_IF_FALSE);
		EMIT_BYTE(rt, code, 0);
	}

	// || return the first truthy operand, or the last operand
	else if (ast.flag == TOY_AST_FLAG_OR) {
		EMIT_BYTE(rt, code, TOY_OPCODE_JUMP);
		EMIT_BYTE(rt, code, TOY_OP_PARAM_JUMP_RELATIVE);
		EMIT_BYTE(rt, code, TOY_OP_PARAM_JUMP_IF_TRUE);
		EMIT_BYTE(rt, code, 0);
	}

	else {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid AST binary short circuit flag found\n" TOY_CC_RESET);
		exit(-1);
	}

	//parameter address
	unsigned int paramAddr = SKIP_INT(rt, code); //parameter to be written later

	//if the lhs value isn't needed, pop it
	EMIT_BYTE(rt, code,TOY_OPCODE_ELIMINATE);
	EMIT_BYTE(rt, code, 0);
	EMIT_BYTE(rt, code, 0);
	EMIT_BYTE(rt, code, 0);

	//rhs
	writeRoutineCode(rt, ast.right);

	//set the parameter
	OVERWRITE_INT(rt, code, paramAddr, CURRENT_ADDRESS(rt, code) - (paramAddr + 4));

	return 1; //leaves only 1 value on the stack
}

static unsigned int writeInstructionCompare(Toy_Routine** rt, Toy_AstCompare ast) {
	//left, then right, then the compare's operation
	writeRoutineCode(rt, ast.left);
	writeRoutineCode(rt, ast.right);

	if (ast.flag == TOY_AST_FLAG_COMPARE_EQUAL) {
		EMIT_BYTE(rt, code,TOY_OPCODE_COMPARE_EQUAL);
	}
	else if (ast.flag == TOY_AST_FLAG_COMPARE_NOT) {
		EMIT_BYTE(rt, code,TOY_OPCODE_COMPARE_EQUAL);
		EMIT_BYTE(rt, code,TOY_OPCODE_NEGATE); //squeezed
		EMIT_BYTE(rt, code,0);
		EMIT_BYTE(rt, code,0);

		return 1;
	}
	else if (ast.flag == TOY_AST_FLAG_COMPARE_LESS) {
		EMIT_BYTE(rt, code,TOY_OPCODE_COMPARE_LESS);
	}
	else if (ast.flag == TOY_AST_FLAG_COMPARE_LESS_EQUAL) {
		EMIT_BYTE(rt, code,TOY_OPCODE_COMPARE_LESS_EQUAL);
	}
	else if (ast.flag == TOY_AST_FLAG_COMPARE_GREATER) {
		EMIT_BYTE(rt, code,TOY_OPCODE_COMPARE_GREATER);
	}
	else if (ast.flag == TOY_AST_FLAG_COMPARE_GREATER_EQUAL) {
		EMIT_BYTE(rt, code,TOY_OPCODE_COMPARE_GREATER_EQUAL);
	}

	else {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid AST compare flag found\n" TOY_CC_RESET);
		exit(-1);
	}

	//4-byte alignment (covers most cases)
	EMIT_BYTE(rt, code,0);
	EMIT_BYTE(rt, code,0);
	EMIT_BYTE(rt, code,0);

	return 1; //leaves only 1 value on the stack
}

static unsigned int writeInstructionGroup(Toy_Routine** rt, Toy_AstGroup ast) {
	//not certain what this leaves
	return writeRoutineCode(rt, ast.child);
}

static unsigned int writeInstructionCompound(Toy_Routine** rt, Toy_AstCompound ast) {
	unsigned int result = writeRoutineCode(rt, ast.child);

	if (ast.flag == TOY_AST_FLAG_COMPOUND_ARRAY) {
		//signal how many values to read in as array elements
		EMIT_BYTE(rt, code, TOY_OPCODE_READ);
		EMIT_BYTE(rt, code, TOY_VALUE_ARRAY);

		//4-byte alignment
		EMIT_BYTE(rt, code,0);
		EMIT_BYTE(rt, code,0);

		//how many elements
		EMIT_INT(rt, code, result);

		return 1; //leaves only 1 value on the stack
	}
	if (ast.flag == TOY_AST_FLAG_COMPOUND_TABLE) {
		//signal how many values to read in as table elements
		EMIT_BYTE(rt, code, TOY_OPCODE_READ);
		EMIT_BYTE(rt, code, TOY_VALUE_TABLE);

		//4-byte alignment
		EMIT_BYTE(rt, code,0);
		EMIT_BYTE(rt, code,0);

		//how many elements
		EMIT_INT(rt, code, result);

		return 1; //leaves only 1 value on the stack
	}
	else {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid AST compound flag found\n" TOY_CC_RESET);
		exit(-1);
		return 0;
	}
}

static unsigned int writeInstructionAggregate(Toy_Routine** rt, Toy_AstAggregate ast) {
	unsigned int result = 0;

	//left, then right
	result += writeRoutineCode(rt, ast.left);
	result += writeRoutineCode(rt, ast.right);

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
		EMIT_BYTE(rt, code, TOY_OPCODE_INDEX);
		EMIT_BYTE(rt, code, result);

		//4-byte alignment
		EMIT_BYTE(rt, code,0);
		EMIT_BYTE(rt, code,0);

		return 1; //leaves only 1 value on the stack
	}
	else {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid AST aggregate flag found\n" TOY_CC_RESET);
		exit(-1);
		return 0;
	}
}

static unsigned int writeInstructionAssert(Toy_Routine** rt, Toy_AstAssert ast) {
	//the thing to print
	writeRoutineCode(rt, ast.child);
	writeRoutineCode(rt, ast.message);

	//output the print opcode
	EMIT_BYTE(rt, code, TOY_OPCODE_ASSERT);

	//4-byte alignment
	EMIT_BYTE(rt, code, ast.message != NULL ? 2 : 1); //arg count
	EMIT_BYTE(rt, code,0);
	EMIT_BYTE(rt, code,0);

	return 0;
}

static unsigned int writeInstructionIfThenElse(Toy_Routine** rt, Toy_AstIfThenElse ast) {
	//cond-branch
	writeRoutineCode(rt, ast.condBranch);

	//emit the jump word (opcode, type, condition, padding)
	EMIT_BYTE(rt, code, TOY_OPCODE_JUMP);
	EMIT_BYTE(rt, code, TOY_OP_PARAM_JUMP_RELATIVE);
	EMIT_BYTE(rt, code, TOY_OP_PARAM_JUMP_IF_FALSE);
	EMIT_BYTE(rt, code, 0);

	unsigned int thenParamAddr = SKIP_INT(rt, code); //parameter to be written later

	//emit then-branch
	writeRoutineCode(rt, ast.thenBranch);

	if (ast.elseBranch != NULL) {
		//emit the jump-to-end (opcode, type, condition, padding)
		EMIT_BYTE(rt, code, TOY_OPCODE_JUMP);
		EMIT_BYTE(rt, code, TOY_OP_PARAM_JUMP_RELATIVE);
		EMIT_BYTE(rt, code, TOY_OP_PARAM_JUMP_ALWAYS);
		EMIT_BYTE(rt, code, 0);

		unsigned int elseParamAddr = SKIP_INT(rt, code); //parameter to be written later

		//specify the starting position for the else branch
		OVERWRITE_INT(rt, code, thenParamAddr, CURRENT_ADDRESS(rt, code) - (thenParamAddr + 4));

		//emit the else branch
		writeRoutineCode(rt, ast.elseBranch);

		//specify the ending position for the else branch
		OVERWRITE_INT(rt, code, elseParamAddr, CURRENT_ADDRESS(rt, code) - (elseParamAddr + 4));
	}

	else {
		//without an else branch, set the jump destination and move on
		OVERWRITE_INT(rt, code, thenParamAddr, CURRENT_ADDRESS(rt, code) - (thenParamAddr + 4));
	}

	return 0;
}

static unsigned int writeInstructionWhileThen(Toy_Routine** rt, Toy_AstWhileThen ast) {
	//begin
	unsigned int beginAddr = CURRENT_ADDRESS(rt, code);

	//cond-branch
	writeRoutineCode(rt, ast.condBranch);

	//emit the jump word (opcode, type, condition, padding)
	EMIT_BYTE(rt, code, TOY_OPCODE_JUMP);
	EMIT_BYTE(rt, code, TOY_OP_PARAM_JUMP_RELATIVE);
	EMIT_BYTE(rt, code, TOY_OP_PARAM_JUMP_IF_FALSE);
	EMIT_BYTE(rt, code, 0);

	unsigned int paramAddr = SKIP_INT(rt, code); //parameter to be written later

	//emit then-branch
	writeRoutineCode(rt, ast.thenBranch);

	//jump to begin to repeat the conditional test
	EMIT_BYTE(rt, code, TOY_OPCODE_JUMP);
	EMIT_BYTE(rt, code, TOY_OP_PARAM_JUMP_RELATIVE);
	EMIT_BYTE(rt, code, TOY_OP_PARAM_JUMP_ALWAYS);
	EMIT_BYTE(rt, code, 0);

	EMIT_INT(rt, code, beginAddr - (CURRENT_ADDRESS(rt, code) + 4)); //this sets a negative value

	//set the exit parameter for the cond
	OVERWRITE_INT(rt, code, paramAddr, CURRENT_ADDRESS(rt, code) - (paramAddr + 4));

	//set the break & continue data
	while ((*rt)->breakEscapes->count > 0) {
		//extract
		unsigned int addr = (*rt)->breakEscapes->data[(*rt)->breakEscapes->count - 1].addr;
		unsigned int depth = (*rt)->breakEscapes->data[(*rt)->breakEscapes->count - 1].depth;

		unsigned int diff = depth - (*rt)->currentScopeDepth;

		OVERWRITE_INT(rt, code, addr, CURRENT_ADDRESS(rt, code) - (addr + 8)); //tell break to come here AFTER reading the instruction
		OVERWRITE_INT(rt, code, addr, diff);

		//tick down
		(*rt)->breakEscapes->count--;
	}

	while ((*rt)->continueEscapes->count > 0) {
		//extract
		unsigned int addr = (*rt)->continueEscapes->data[(*rt)->continueEscapes->count - 1].addr;
		unsigned int depth = (*rt)->continueEscapes->data[(*rt)->continueEscapes->count - 1].depth;

		unsigned int diff = depth - (*rt)->currentScopeDepth;

		OVERWRITE_INT(rt, code, addr, CURRENT_ADDRESS(rt, code) - (addr + 8)); //tell continue to return to the start AFTER reading the instruction
		OVERWRITE_INT(rt, code, addr, diff);

		//tick down
		(*rt)->continueEscapes->count--;
	}

	return 0;
}

static unsigned int writeInstructionBreak(Toy_Routine** rt, Toy_AstBreak ast) {
	//unused
	(void)ast;

	//escapes are always relative
	EMIT_BYTE(rt, code, TOY_OPCODE_ESCAPE);
	EMIT_BYTE(rt, code, 0);
	EMIT_BYTE(rt, code, 0);
	EMIT_BYTE(rt, code, 0);

	unsigned int addr = SKIP_INT(rt, code);
	(void)SKIP_INT(rt, code); //empty space for depth

	//expand the escape array if needed
	if ((*rt)->breakEscapes->capacity <= (*rt)->breakEscapes->count) {
		(*rt)->breakEscapes = Toy_private_resizeEscapeArray((*rt)->breakEscapes, (*rt)->breakEscapes->capacity * TOY_ESCAPE_EXPANSION_RATE);
	}

	//store for later
	(*rt)->breakEscapes->data[(*rt)->breakEscapes->count++] = (Toy_private_EscapeEntry_t){ .addr = addr, .depth = (*rt)->currentScopeDepth };

	return 0;
}

static unsigned int writeInstructionContinue(Toy_Routine** rt, Toy_AstContinue ast) {
	//unused
	(void)ast;

	//escapes are always relative
	EMIT_BYTE(rt, code, TOY_OPCODE_ESCAPE);
	EMIT_BYTE(rt, code, 0);
	EMIT_BYTE(rt, code, 0);
	EMIT_BYTE(rt, code, 0);

	unsigned int addr = SKIP_INT(rt, code);
	(void)SKIP_INT(rt, code); //empty space for depth

	//expand the escape array if needed
	if ((*rt)->continueEscapes->capacity <= (*rt)->continueEscapes->count) {
		(*rt)->continueEscapes = Toy_private_resizeEscapeArray((*rt)->continueEscapes, (*rt)->continueEscapes->capacity * TOY_ESCAPE_EXPANSION_RATE);
	}

	//store for later
	(*rt)->continueEscapes->data[(*rt)->continueEscapes->count++] = (Toy_private_EscapeEntry_t){ .addr = addr, .depth = (*rt)->currentScopeDepth };

	return 0;
}

static unsigned int writeInstructionPrint(Toy_Routine** rt, Toy_AstPrint ast) {
	//the thing to print
	writeRoutineCode(rt, ast.child);

	//output the print opcode
	EMIT_BYTE(rt, code,TOY_OPCODE_PRINT);

	//4-byte alignment
	EMIT_BYTE(rt, code,0);
	EMIT_BYTE(rt, code,0);
	EMIT_BYTE(rt, code,0);

	return 0;
}

static unsigned int writeInstructionVarDeclare(Toy_Routine** rt, Toy_AstVarDeclare ast) {
	//if we're dealing with chained assignments, hijack the next assignment with 'chainedAssignment' set to true
	if (checkForChaining(ast.expr)) {
		writeInstructionAssign(rt, ast.expr->varAssign, true);
	}
	else {
		writeRoutineCode(rt, ast.expr); //default value
	}

	//delcare with the given name string
	EMIT_BYTE(rt, code, TOY_OPCODE_DECLARE);
	EMIT_BYTE(rt, code, Toy_getNameStringVarType(ast.name));
	EMIT_BYTE(rt, code, ast.name->info.length); //quick optimisation to skip a 'strlen()' call
	EMIT_BYTE(rt, code, Toy_getNameStringVarConstant(ast.name) ? 1 : 0); //check for constness

	emitString(rt, ast.name);

	return 0;
}

static unsigned int writeInstructionAssign(Toy_Routine** rt, Toy_AstVarAssign ast, bool chainedAssignment) {
	unsigned int result = 0;

	//target is a name string
	if (ast.target->type == TOY_AST_VALUE && TOY_VALUE_IS_STRING(ast.target->value.value) && TOY_VALUE_AS_STRING(ast.target->value.value)->info.type == TOY_STRING_NAME) {
		//name string
		Toy_String* target = TOY_VALUE_AS_STRING(ast.target->value.value);

		//emit the name string
		EMIT_BYTE(rt, code, TOY_OPCODE_READ);
		EMIT_BYTE(rt, code, TOY_VALUE_STRING);
		EMIT_BYTE(rt, code, TOY_STRING_NAME);
		EMIT_BYTE(rt, code, target->info.length); //store the length (max 255)

		emitString(rt, target);
	}

	//target is an indexing of some compound value
	else if (ast.target->type == TOY_AST_AGGREGATE && ast.target->aggregate.flag == TOY_AST_FLAG_INDEX) {
		writeRoutineCode(rt, ast.target->aggregate.left); //any deeper indexing will just work, using reference values
		writeRoutineCode(rt, ast.target->aggregate.right); //key

		//if we're dealing with chained assignments, hijack the next assignment with 'chainedAssignment' set to true
		if (checkForChaining(ast.expr)) {
			result += writeInstructionAssign(rt, ast.expr->varAssign, true);
		}
		else {
			result += writeRoutineCode(rt, ast.expr); //default value
		}

		EMIT_BYTE(rt, code, TOY_OPCODE_ASSIGN_COMPOUND); //uses the top three values on the stack
		EMIT_BYTE(rt, code, chainedAssignment);
		EMIT_BYTE(rt, code,0);
		EMIT_BYTE(rt, code,0);

		return result + (chainedAssignment ? 1 : 0);
	}

	else {
		//unknown target
		fprintf(stderr, TOY_CC_ERROR "COMPILER ERROR: Invalid AST type found: Malformed assignment target\n" TOY_CC_RESET);
		(*rt)->panic = true;
		return 0;
	}

	//determine RHS, include duplication if needed
	if (ast.flag == TOY_AST_FLAG_ASSIGN) {
		//if we're dealing with chained assignments, hijack the next assignment with 'chainedAssignment' set to true
		if (checkForChaining(ast.expr)) {
			result += writeInstructionAssign(rt, ast.expr->varAssign, true);
		}
		else {
			result += writeRoutineCode(rt, ast.expr); //default value
		}

		EMIT_BYTE(rt, code, TOY_OPCODE_ASSIGN);
		EMIT_BYTE(rt, code, chainedAssignment);
		EMIT_BYTE(rt, code,0);
		EMIT_BYTE(rt, code,0);
	}
	else if (ast.flag == TOY_AST_FLAG_ADD_ASSIGN) {
		EMIT_BYTE(rt, code,TOY_OPCODE_DUPLICATE);
		EMIT_BYTE(rt, code,TOY_OPCODE_ACCESS); //squeezed
		EMIT_BYTE(rt, code,0);
		EMIT_BYTE(rt, code,0);

		//if we're dealing with chained assignments, hijack the next assignment with 'chainedAssignment' set to true
		if (checkForChaining(ast.expr)) {
			result += writeInstructionAssign(rt, ast.expr->varAssign, true);
		}
		else {
			result += writeRoutineCode(rt, ast.expr); //default value
		}

		EMIT_BYTE(rt, code,TOY_OPCODE_ADD);
		EMIT_BYTE(rt, code,TOY_OPCODE_ASSIGN); //squeezed
		EMIT_BYTE(rt, code, chainedAssignment);
		EMIT_BYTE(rt, code,0);
	}
	else if (ast.flag == TOY_AST_FLAG_SUBTRACT_ASSIGN) {
		EMIT_BYTE(rt, code,TOY_OPCODE_DUPLICATE);
		EMIT_BYTE(rt, code,TOY_OPCODE_ACCESS); //squeezed
		EMIT_BYTE(rt, code,0);
		EMIT_BYTE(rt, code,0);

		//if we're dealing with chained assignments, hijack the next assignment with 'chainedAssignment' set to true
		if (checkForChaining(ast.expr)) {
			result += writeInstructionAssign(rt, ast.expr->varAssign, true);
		}
		else {
			result += writeRoutineCode(rt, ast.expr); //default value
		}

		EMIT_BYTE(rt, code,TOY_OPCODE_SUBTRACT);
		EMIT_BYTE(rt, code,TOY_OPCODE_ASSIGN); //squeezed
		EMIT_BYTE(rt, code, chainedAssignment);
		EMIT_BYTE(rt, code,0);
	}
	else if (ast.flag == TOY_AST_FLAG_MULTIPLY_ASSIGN) {
		EMIT_BYTE(rt, code,TOY_OPCODE_DUPLICATE);
		EMIT_BYTE(rt, code,TOY_OPCODE_ACCESS); //squeezed
		EMIT_BYTE(rt, code,0);
		EMIT_BYTE(rt, code,0);

		//if we're dealing with chained assignments, hijack the next assignment with 'chainedAssignment' set to true
		if (checkForChaining(ast.expr)) {
			result += writeInstructionAssign(rt, ast.expr->varAssign, true);
		}
		else {
			result += writeRoutineCode(rt, ast.expr); //default value
		}

		EMIT_BYTE(rt, code,TOY_OPCODE_MULTIPLY);
		EMIT_BYTE(rt, code,TOY_OPCODE_ASSIGN); //squeezed
		EMIT_BYTE(rt, code, chainedAssignment);
		EMIT_BYTE(rt, code,0);
	}
	else if (ast.flag == TOY_AST_FLAG_DIVIDE_ASSIGN) {
		EMIT_BYTE(rt, code,TOY_OPCODE_DUPLICATE);
		EMIT_BYTE(rt, code,TOY_OPCODE_ACCESS); //squeezed
		EMIT_BYTE(rt, code,0);
		EMIT_BYTE(rt, code,0);

		//if we're dealing with chained assignments, hijack the next assignment with 'chainedAssignment' set to true
		if (checkForChaining(ast.expr)) {
			result += writeInstructionAssign(rt, ast.expr->varAssign, true);
		}
		else {
			result += writeRoutineCode(rt, ast.expr); //default value
		}

		EMIT_BYTE(rt, code,TOY_OPCODE_DIVIDE);
		EMIT_BYTE(rt, code,TOY_OPCODE_ASSIGN); //squeezed
		EMIT_BYTE(rt, code, chainedAssignment);
		EMIT_BYTE(rt, code,0);
	}
	else if (ast.flag == TOY_AST_FLAG_MODULO_ASSIGN) {
		EMIT_BYTE(rt, code,TOY_OPCODE_DUPLICATE);
		EMIT_BYTE(rt, code,TOY_OPCODE_ACCESS); //squeezed
		EMIT_BYTE(rt, code,0);
		EMIT_BYTE(rt, code,0);

		//if we're dealing with chained assignments, hijack the next assignment with 'chainedAssignment' set to true
		if (checkForChaining(ast.expr)) {
			result += writeInstructionAssign(rt, ast.expr->varAssign, true);
		}
		else {
			result += writeRoutineCode(rt, ast.expr); //default value
		}

		EMIT_BYTE(rt, code,TOY_OPCODE_MODULO);
		EMIT_BYTE(rt, code,TOY_OPCODE_ASSIGN); //squeezed
		EMIT_BYTE(rt, code, chainedAssignment);
		EMIT_BYTE(rt, code,0);
	}

	else {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid AST assign flag found\n" TOY_CC_RESET);
		exit(-1);
	}

	return result + (chainedAssignment ? 1 : 0);
}

static unsigned int writeInstructionAccess(Toy_Routine** rt, Toy_AstVarAccess ast) {
	if (!(ast.child->type == TOY_AST_VALUE && TOY_VALUE_IS_STRING(ast.child->value.value) && TOY_VALUE_AS_STRING(ast.child->value.value)->info.type == TOY_STRING_NAME)) {
		fprintf(stderr, TOY_CC_ERROR "COMPILER ERROR: Found a non-name-string in a value node when trying to write access\n" TOY_CC_RESET);
		exit(-1);
	}

	Toy_String* name = TOY_VALUE_AS_STRING(ast.child->value.value);

	//push the name
	EMIT_BYTE(rt, code, TOY_OPCODE_READ);
	EMIT_BYTE(rt, code, TOY_VALUE_STRING);
	EMIT_BYTE(rt, code, TOY_STRING_NAME);
	EMIT_BYTE(rt, code, name->info.length); //store the length (max 255)

	emitString(rt, name);

	//convert name to value
	EMIT_BYTE(rt, code, TOY_OPCODE_ACCESS);
	EMIT_BYTE(rt, code,0);
	EMIT_BYTE(rt, code,0);
	EMIT_BYTE(rt, code,0);

	return 1;
}

//routine structure
// static void writeRoutineParam(Toy_Routine* rt) {
// 	//
// }

static unsigned int writeRoutineCode(Toy_Routine** rt, Toy_Ast* ast) {
	if (ast == NULL) {
		return 0;
	}

	//if an error occured, just exit
	if (rt == NULL || (*rt) == NULL || (*rt)->panic) {
		return 0;
	}

	//NOTE: 'result' is used to in 'writeInstructionAggregate()'
	unsigned int result = 0;

	//determine how to write each instruction based on the Ast
	switch(ast->type) {
		case TOY_AST_BLOCK:
			if (ast->block.innerScope) {
				EMIT_BYTE(rt, code, TOY_OPCODE_SCOPE_PUSH);
				EMIT_BYTE(rt, code, 0);
				EMIT_BYTE(rt, code, 0);
				EMIT_BYTE(rt, code, 0);

				(*rt)->currentScopeDepth++;
			}

			result += writeRoutineCode(rt, ast->block.child);
			result += writeRoutineCode(rt, ast->block.next);

			if (ast->block.innerScope) {
				EMIT_BYTE(rt, code, TOY_OPCODE_SCOPE_POP);
				EMIT_BYTE(rt, code, 0);
				EMIT_BYTE(rt, code, 0);
				EMIT_BYTE(rt, code, 0);

				(*rt)->currentScopeDepth--;
			}
			break;

		case TOY_AST_VALUE:
			result += writeInstructionValue(rt, ast->value);
			break;

		case TOY_AST_UNARY:
			result += writeInstructionUnary(rt, ast->unary);
			break;

		case TOY_AST_BINARY:
			result += writeInstructionBinary(rt, ast->binary);
			break;

		case TOY_AST_BINARY_SHORT_CIRCUIT:
			result += writeInstructionBinaryShortCircuit(rt, ast->binaryShortCircuit);
			break;

		case TOY_AST_COMPARE:
			result += writeInstructionCompare(rt, ast->compare);
			break;

		case TOY_AST_GROUP:
			result += writeInstructionGroup(rt, ast->group);
			break;

		case TOY_AST_COMPOUND:
			result += writeInstructionCompound(rt, ast->compound);
			break;

		case TOY_AST_AGGREGATE:
			result += writeInstructionAggregate(rt, ast->aggregate);
			break;

		case TOY_AST_ASSERT:
			result += writeInstructionAssert(rt, ast->assert);
			break;

		case TOY_AST_IF_THEN_ELSE:
			result += writeInstructionIfThenElse(rt, ast->ifThenElse);
			break;

		case TOY_AST_WHILE_THEN:
			result += writeInstructionWhileThen(rt, ast->whileThen);
			break;

		case TOY_AST_BREAK:
			result += writeInstructionBreak(rt, ast->breakPoint);
			break;

		case TOY_AST_CONTINUE:
			result += writeInstructionContinue(rt, ast->continuePoint);
			break;

		case TOY_AST_PRINT:
			result += writeInstructionPrint(rt, ast->print);
			break;

		case TOY_AST_VAR_DECLARE:
			result += writeInstructionVarDeclare(rt, ast->varDeclare);
			break;

		case TOY_AST_VAR_ASSIGN:
			result += writeInstructionAssign(rt, ast->varAssign, false);
			break;

		case TOY_AST_VAR_ACCESS:
			result += writeInstructionAccess(rt, ast->varAccess);
			break;

		case TOY_AST_PASS:
			//NO-OP
			break;

		//meta instructions are disallowed
		case TOY_AST_ERROR:
			fprintf(stderr, TOY_CC_ERROR "COMPILER ERROR: Invalid AST type found: Unknown 'error'\n" TOY_CC_RESET);
			(*rt)->panic = true;
			break;

		case TOY_AST_END:
			fprintf(stderr, TOY_CC_ERROR "COMPILER ERROR: Invalid AST type found: Unknown 'end'\n" TOY_CC_RESET);
			(*rt)->panic = true;
			break;
	}

	return result;
}

static void* writeRoutine(Toy_Routine* rt, Toy_Ast* ast) {
	//code
	writeRoutineCode(&rt, ast);
	EMIT_BYTE(&rt, code, TOY_OPCODE_RETURN); //temp terminator
	EMIT_BYTE(&rt, code, 0); //4-byte alignment
	EMIT_BYTE(&rt, code, 0);
	EMIT_BYTE(&rt, code, 0);

	//if an error occurred, just exit
	if (rt->panic) {
		return NULL;
	}

	//write the header and combine the parts
	unsigned char* buffer = NULL;
	unsigned int capacity = 0, count = 0;
	// int paramAddr = 0, subsAddr = 0;
	int codeAddr = 0;
	int jumpsAddr = 0;
	int dataAddr = 0;

	emitInt(&buffer, &capacity, &count, 0); //total size (overwritten later)
	emitInt(&buffer, &capacity, &count, rt->paramCount); //param size
	emitInt(&buffer, &capacity, &count, rt->jumpsCount); //jumps size
	emitInt(&buffer, &capacity, &count, rt->dataCount); //data size
	emitInt(&buffer, &capacity, &count, rt->subsCount); //routine size

	//generate blank spaces, cache their positions in the *Addr variables (for storing the start positions)
	if (rt->paramCount > 0) {
		// paramAddr = count;
		emitInt(&buffer, &capacity, &count, 0); //params
	}
	if (rt->codeCount > 0) {
		codeAddr = count;
		emitInt(&buffer, &capacity, &count, 0); //code
	}
	if (rt->jumpsCount > 0) {
		jumpsAddr = count;
		emitInt(&buffer, &capacity, &count, 0); //jumps
	}
	if (rt->dataCount > 0) {
		dataAddr = count;
		emitInt(&buffer, &capacity, &count, 0); //data
	}
	if (rt->subsCount > 0) {
		// subsAddr = count;
		emitInt(&buffer, &capacity, &count, 0); //subs
	}

	//append various parts to the buffer
	//TODO: param region

	if (rt->codeCount > 0) {
		expand(&buffer, &capacity, &count, rt->codeCount);
		memcpy((buffer + count), rt->code, rt->codeCount);

		*((int*)(buffer + codeAddr)) = count;
		count += rt->codeCount;
	}

	if (rt->jumpsCount > 0) {
		expand(&buffer, &capacity, &count, rt->jumpsCount);
		memcpy((buffer + count), rt->jumps, rt->jumpsCount);

		*((int*)(buffer + jumpsAddr)) = count;
		count += rt->jumpsCount;
	}

	if (rt->dataCount > 0) {
		expand(&buffer, &capacity, &count, rt->dataCount);
		memcpy((buffer + count), rt->data, rt->dataCount);

		*((int*)(buffer + dataAddr)) = count;
		count += rt->dataCount;
	}

	//TODO: subroutine region

	//finally, record the total size within the header, and return the result
	*((int*)buffer) = count;

	return buffer;
}

//exposed functions
void* Toy_compileRoutine(Toy_Ast* ast) {
	//setup
	Toy_Routine rt;

	rt.param = NULL;
	rt.paramCapacity = 0;
	rt.paramCount = 0;

	rt.code = NULL;
	rt.codeCapacity = 0;
	rt.codeCount = 0;

	rt.jumps = NULL;
	rt.jumpsCapacity = 0;
	rt.jumpsCount = 0;

	rt.data = NULL;
	rt.dataCapacity = 0;
	rt.dataCount = 0;

	rt.subs = NULL;
	rt.subsCapacity = 0;
	rt.subsCount = 0;

	rt.currentScopeDepth = 0;
	rt.breakEscapes = Toy_private_resizeEscapeArray(NULL, TOY_ESCAPE_INITIAL_CAPACITY);
	rt.continueEscapes = Toy_private_resizeEscapeArray(NULL, TOY_ESCAPE_INITIAL_CAPACITY);

	rt.panic = false;

	//build
	void * buffer = writeRoutine(&rt, ast);

	//cleanup
	Toy_private_resizeEscapeArray(rt.breakEscapes, 0);
	Toy_private_resizeEscapeArray(rt.continueEscapes, 0);

	free(rt.param);
	free(rt.code);
	free(rt.jumps);
	free(rt.data);
	free(rt.subs);

	return buffer;
}
