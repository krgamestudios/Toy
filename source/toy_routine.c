#include "toy_routine.h"
#include "toy_console_colors.h"

#include "toy_opcodes.h"
#include "toy_value.h"
#include "toy_string.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//utils
static void expand(void** handle, unsigned int* capacity, unsigned int* count, unsigned int amount) {
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

static void emitByte(void** handle, unsigned int* capacity, unsigned int* count, unsigned char byte) {
	expand(handle, capacity, count, 1);
	((unsigned char*)(*handle))[(*count)++] = byte;
}

static void emitInt(void** handle, unsigned int* capacity, unsigned int* count, unsigned int bytes) {
	char* ptr = (char*)&bytes;
	emitByte(handle, capacity, count, *(ptr++));
	emitByte(handle, capacity, count, *(ptr++));
	emitByte(handle, capacity, count, *(ptr++));
	emitByte(handle, capacity, count, *(ptr++));
}

static void emitFloat(void** handle, unsigned int* capacity, unsigned int* count, float bytes) {
	char* ptr = (char*)&bytes;
	emitByte(handle, capacity, count, *(ptr++));
	emitByte(handle, capacity, count, *(ptr++));
	emitByte(handle, capacity, count, *(ptr++));
	emitByte(handle, capacity, count, *(ptr++));
}

//write instructions based on the AST types
#define EMIT_BYTE(rt, part, byte) \
	emitByte((void**)(&((*rt)->part)), &((*rt)->part##Capacity), &((*rt)->part##Count), byte);
#define EMIT_INT(rt, part, bytes) \
	emitInt((void**)(&((*rt)->part)), &((*rt)->part##Capacity), &((*rt)->part##Count), bytes);
#define EMIT_FLOAT(rt, part, bytes) \
	emitFloat((void**)(&((*rt)->part)), &((*rt)->part##Capacity), &((*rt)->part##Count), bytes);

static void emitToJumpTable(Toy_Routine** rt, unsigned int startAddr) {
	EMIT_INT(rt, code, (*rt)->jumpsCount); //mark the jump index in the code
	EMIT_INT(rt, jumps, startAddr); //save address at the jump index
}

static unsigned int emitString(Toy_Routine** rt, Toy_String* str) {
	//4-byte alignment
	unsigned int length = str->length + 1;
	if (length % 4 != 0) {
		length += 4 - (length % 4); //ceil
	}

	//grab the current start address
	unsigned int startAddr = (*rt)->dataCount;

	//move the string into the data section
	expand((void**)(&((*rt)->data)), &((*rt)->dataCapacity), &((*rt)->dataCount), length);

	if (str->type == TOY_STRING_NODE) {
		char* buffer = Toy_getStringRawBuffer(str);
		memcpy((*rt)->data + (*rt)->dataCount, buffer, str->length + 1);
		free(buffer);
	}
	else if (str->type == TOY_STRING_LEAF) {
		memcpy((*rt)->data + (*rt)->dataCount, str->as.leaf.data, str->length + 1);
	}
	else if (str->type == TOY_STRING_NAME) {
		memcpy((*rt)->data + (*rt)->dataCount, str->as.name.data, str->length + 1);
	}

	(*rt)->dataCount += length;

	//mark the jump position
	emitToJumpTable(rt, startAddr);

	return 1;
}

static unsigned int writeRoutineCode(Toy_Routine** rt, Toy_Ast* ast); //forward declare for recursion

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
	//working with a stack means the child gets placed first
	unsigned int result = writeRoutineCode(rt, ast.child);

	if (ast.flag == TOY_AST_FLAG_NEGATE) {
		EMIT_BYTE(rt, code, TOY_OPCODE_NEGATE);

		//4-byte alignment
		EMIT_BYTE(rt, code, 0);
		EMIT_BYTE(rt, code, 0);
		EMIT_BYTE(rt, code, 0);
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

	//nowhere to really put these for now
	else if (ast.flag == TOY_AST_FLAG_AND) {
		EMIT_BYTE(rt, code,TOY_OPCODE_AND);
	}
	else if (ast.flag == TOY_AST_FLAG_OR) {
		EMIT_BYTE(rt, code,TOY_OPCODE_OR);
	}
	else if (ast.flag == TOY_AST_FLAG_CONCAT) {
		EMIT_BYTE(rt, code, TOY_OPCODE_CONCAT);
	}
	else {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid AST binary flag found\n" TOY_CC_RESET);
		exit(-1);
	}

	//4-byte alignment
	EMIT_BYTE(rt, code,TOY_OPCODE_PASS); //checked in compound assignments
	EMIT_BYTE(rt, code,0);
	EMIT_BYTE(rt, code,0);

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
	unsigned int result = 0;

	//left, then right
	result += writeRoutineCode(rt, ast.left);
	result += writeRoutineCode(rt, ast.right);

	if (ast.flag == TOY_AST_FLAG_COMPOUND_COLLECTION) {
		//collections are handled above
		return result;
	}
	else if (ast.flag == TOY_AST_FLAG_COMPOUND_INDEX) {
		//value[index, length]
		EMIT_BYTE(rt, code, TOY_OPCODE_INDEX);
		EMIT_BYTE(rt, code, result);

		//4-byte alignment
		EMIT_BYTE(rt, code,0);
		EMIT_BYTE(rt, code,0);

		return 1; //leaves only 1 value on the stack
	}
	else {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid AST compound flag found\n" TOY_CC_RESET);
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
	//initial value
	writeRoutineCode(rt, ast.expr);

	//delcare with the given name string
	EMIT_BYTE(rt, code, TOY_OPCODE_DECLARE);
	EMIT_BYTE(rt, code, Toy_getNameStringType(ast.name));
	EMIT_BYTE(rt, code, ast.name->length); //quick optimisation to skip a 'strlen()' call
	EMIT_BYTE(rt, code, Toy_getNameStringConstant(ast.name) ? 1 : 0); //check for constness

	emitString(rt, ast.name);

	return 0;
}

static unsigned int writeInstructionAssign(Toy_Routine** rt, Toy_AstVarAssign ast) {
	unsigned int result = 0;

	//don't treat these as valid values
	switch (ast.expr->type) {
		case TOY_AST_BLOCK:
		case TOY_AST_COMPOUND:
		case TOY_AST_ASSERT:
		case TOY_AST_PRINT:
		case TOY_AST_VAR_DECLARE:
			//emit a compiler error, set the panic flag and skip out
			fprintf(stderr, TOY_CC_ERROR "COMPILER ERROR: Invalid AST type found: Malformed assignment\n" TOY_CC_RESET);
			(*rt)->panic = true;
			return 0;

		default:
			break;
	}

	//name, duplicate, right, opcode
	if (ast.flag == TOY_AST_FLAG_ASSIGN) {
		EMIT_BYTE(rt, code, TOY_OPCODE_READ);
		EMIT_BYTE(rt, code, TOY_VALUE_STRING);
		EMIT_BYTE(rt, code, TOY_STRING_NAME);
		EMIT_BYTE(rt, code, ast.name->length); //store the length (max 255)

		emitString(rt, ast.name);
		result += writeRoutineCode(rt, ast.expr);

		EMIT_BYTE(rt, code, TOY_OPCODE_ASSIGN);
		EMIT_BYTE(rt, code, 0);
	}
	else if (ast.flag == TOY_AST_FLAG_ADD_ASSIGN) {
		EMIT_BYTE(rt, code, TOY_OPCODE_READ);
		EMIT_BYTE(rt, code, TOY_VALUE_STRING);
		EMIT_BYTE(rt, code, TOY_STRING_NAME);
		EMIT_BYTE(rt, code, ast.name->length); //store the length (max 255)

		emitString(rt, ast.name);

		EMIT_BYTE(rt, code,TOY_OPCODE_DUPLICATE);
		EMIT_BYTE(rt, code,TOY_OPCODE_ACCESS); //squeezed
		EMIT_BYTE(rt, code,0);
		EMIT_BYTE(rt, code,0);

		result += writeRoutineCode(rt, ast.expr);

		EMIT_BYTE(rt, code,TOY_OPCODE_ADD);
		EMIT_BYTE(rt, code,TOY_OPCODE_ASSIGN); //squeezed
	}
	else if (ast.flag == TOY_AST_FLAG_SUBTRACT_ASSIGN) {
		EMIT_BYTE(rt, code, TOY_OPCODE_READ);
		EMIT_BYTE(rt, code, TOY_VALUE_STRING);
		EMIT_BYTE(rt, code, TOY_STRING_NAME);
		EMIT_BYTE(rt, code, ast.name->length); //store the length (max 255)

		emitString(rt, ast.name);

		EMIT_BYTE(rt, code,TOY_OPCODE_DUPLICATE);
		EMIT_BYTE(rt, code,TOY_OPCODE_ACCESS); //squeezed
		EMIT_BYTE(rt, code,0);
		EMIT_BYTE(rt, code,0);

		result += writeRoutineCode(rt, ast.expr);

		EMIT_BYTE(rt, code,TOY_OPCODE_SUBTRACT);
		EMIT_BYTE(rt, code,TOY_OPCODE_ASSIGN); //squeezed
	}
	else if (ast.flag == TOY_AST_FLAG_MULTIPLY_ASSIGN) {
		EMIT_BYTE(rt, code, TOY_OPCODE_READ);
		EMIT_BYTE(rt, code, TOY_VALUE_STRING);
		EMIT_BYTE(rt, code, TOY_STRING_NAME);
		EMIT_BYTE(rt, code, ast.name->length); //store the length (max 255)

		emitString(rt, ast.name);

		EMIT_BYTE(rt, code,TOY_OPCODE_DUPLICATE);
		EMIT_BYTE(rt, code,TOY_OPCODE_ACCESS); //squeezed
		EMIT_BYTE(rt, code,0);
		EMIT_BYTE(rt, code,0);

		result += writeRoutineCode(rt, ast.expr);

		EMIT_BYTE(rt, code,TOY_OPCODE_MULTIPLY);
		EMIT_BYTE(rt, code,TOY_OPCODE_ASSIGN); //squeezed
	}
	else if (ast.flag == TOY_AST_FLAG_DIVIDE_ASSIGN) {
		EMIT_BYTE(rt, code, TOY_OPCODE_READ);
		EMIT_BYTE(rt, code, TOY_VALUE_STRING);
		EMIT_BYTE(rt, code, TOY_STRING_NAME);
		EMIT_BYTE(rt, code, ast.name->length); //store the length (max 255)

		emitString(rt, ast.name);

		EMIT_BYTE(rt, code,TOY_OPCODE_DUPLICATE);
		EMIT_BYTE(rt, code,TOY_OPCODE_ACCESS); //squeezed
		EMIT_BYTE(rt, code,0);
		EMIT_BYTE(rt, code,0);

		result += writeRoutineCode(rt, ast.expr);

		EMIT_BYTE(rt, code,TOY_OPCODE_DIVIDE);
		EMIT_BYTE(rt, code,TOY_OPCODE_ASSIGN); //squeezed
	}
	else if (ast.flag == TOY_AST_FLAG_MODULO_ASSIGN) {
		EMIT_BYTE(rt, code, TOY_OPCODE_READ);
		EMIT_BYTE(rt, code, TOY_VALUE_STRING);
		EMIT_BYTE(rt, code, TOY_STRING_NAME);
		EMIT_BYTE(rt, code, ast.name->length); //store the length (max 255)

		emitString(rt, ast.name);

		EMIT_BYTE(rt, code,TOY_OPCODE_DUPLICATE);
		EMIT_BYTE(rt, code,TOY_OPCODE_ACCESS); //squeezed
		EMIT_BYTE(rt, code,0);
		EMIT_BYTE(rt, code,0);

		result += writeRoutineCode(rt, ast.expr);

		EMIT_BYTE(rt, code,TOY_OPCODE_MODULO);
		EMIT_BYTE(rt, code,TOY_OPCODE_ASSIGN); //squeezed
	}

	else {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Invalid AST assign flag found\n" TOY_CC_RESET);
		exit(-1);
	}

	//4-byte alignment
	EMIT_BYTE(rt, code,0);
	EMIT_BYTE(rt, code,0);

	return result;
}

static unsigned int writeInstructionAccess(Toy_Routine** rt, Toy_AstVarAccess ast) {
	//push the name
	EMIT_BYTE(rt, code, TOY_OPCODE_READ);
	EMIT_BYTE(rt, code, TOY_VALUE_STRING);
	EMIT_BYTE(rt, code, TOY_STRING_NAME);
	EMIT_BYTE(rt, code, ast.name->length); //store the length (max 255)

	emitString(rt, ast.name);

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

	unsigned int result = 0;

	//determine how to write each instruction based on the Ast
	switch(ast->type) {
		case TOY_AST_BLOCK:
			if (ast->block.innerScope) {
				EMIT_BYTE(rt, code, TOY_OPCODE_SCOPE_PUSH);
				EMIT_BYTE(rt, code, 0);
				EMIT_BYTE(rt, code, 0);
				EMIT_BYTE(rt, code, 0);
			}

			result += writeRoutineCode(rt, ast->block.child);
			result += writeRoutineCode(rt, ast->block.next);

			if (ast->block.innerScope) {
				EMIT_BYTE(rt, code, TOY_OPCODE_SCOPE_POP);
				EMIT_BYTE(rt, code, 0);
				EMIT_BYTE(rt, code, 0);
				EMIT_BYTE(rt, code, 0);
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

		case TOY_AST_COMPARE:
			result += writeInstructionCompare(rt, ast->compare);
			break;

		case TOY_AST_GROUP:
			result += writeInstructionGroup(rt, ast->group);
			break;

		case TOY_AST_COMPOUND:
			result += writeInstructionCompound(rt, ast->compound);
			break;

		case TOY_AST_ASSERT:
			result += writeInstructionAssert(rt, ast->assert);
			break;

		case TOY_AST_PRINT:
			result += writeInstructionPrint(rt, ast->print);
			break;

		case TOY_AST_VAR_DECLARE:
			result += writeInstructionVarDeclare(rt, ast->varDeclare);
			break;

		case TOY_AST_VAR_ASSIGN:
			result += writeInstructionAssign(rt, ast->varAssign);
			break;

		case TOY_AST_VAR_ACCESS:
			result += writeInstructionAccess(rt, ast->varAccess);
			break;

		//meta instructions are disallowed
		case TOY_AST_PASS:
			//NOTE: this *should* be disallowed, but for now it's required for testing
			break;

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
	void* buffer = NULL;
	unsigned int capacity = 0, count = 0;
	// int paramAddr = 0, codeAddr = 0, subsAddr = 0;
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
		emitInt((void**)&buffer, &capacity, &count, 0); //params
	}
	if (rt->codeCount > 0) {
		codeAddr = count;
		emitInt((void**)&buffer, &capacity, &count, 0); //code
	}
	if (rt->jumpsCount > 0) {
		jumpsAddr = count;
		emitInt((void**)&buffer, &capacity, &count, 0); //jumps
	}
	if (rt->dataCount > 0) {
		dataAddr = count;
		emitInt((void**)&buffer, &capacity, &count, 0); //data
	}
	if (rt->subsCount > 0) {
		// subsAddr = count;
		emitInt((void**)&buffer, &capacity, &count, 0); //subs
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

	//TODO: subs region

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

	rt.panic = false;

	//build
	void * buffer = writeRoutine(&rt, ast);

	//cleanup the temp object
	free(rt.param);
	free(rt.code);
	free(rt.jumps);
	free(rt.data);
	free(rt.subs);

	return buffer;
}
