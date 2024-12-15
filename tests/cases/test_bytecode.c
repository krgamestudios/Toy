#include "toy_bytecode.h"
#include "toy_console_colors.h"

#include "toy_opcodes.h"
#include "toy_lexer.h"
#include "toy_parser.h"

#include <stdio.h>
#include <string.h>

//tests
int test_bytecode_header(Toy_Bucket** bucketHandle) {
	//simple test to ensure the header looks right
	{
		//setup
		Toy_Ast* ast = NULL;
		Toy_private_emitAstPass(bucketHandle, &ast);

		//run
		Toy_Bytecode bc = Toy_compileBytecode(ast);

		//check
		if (bc.ptr[0] != TOY_VERSION_MAJOR ||
			bc.ptr[1] != TOY_VERSION_MINOR ||
			bc.ptr[2] != TOY_VERSION_PATCH ||
			strcmp((char*)(bc.ptr + 3), TOY_VERSION_BUILD) != 0)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to write the bytecode header correctly:\n" TOY_CC_RESET);
			fprintf(stderr, TOY_CC_ERROR "\t%d.%d.%d.%s\n" TOY_CC_RESET, (int)(bc.ptr[0]), (int)(bc.ptr[1]), (int)(bc.ptr[2]), (char*)(bc.ptr + 3));
			fprintf(stderr, TOY_CC_ERROR "\t%d.%d.%d.%s\n" TOY_CC_RESET, TOY_VERSION_MAJOR, TOY_VERSION_MINOR, TOY_VERSION_PATCH, TOY_VERSION_BUILD);

			//cleanup and return
			Toy_freeBytecode(bc);
			return -1;
		}

		if (bc.count % 4 != 0) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: bytecode size is not a multiple of 4, size is: %d\n" TOY_CC_RESET, (int)bc.count);

			//cleanup and return
			Toy_freeBytecode(bc);
			return -1;
		}

		//cleanup
		Toy_freeBytecode(bc);
	}

	return 0;
}

int test_bytecode_from_source(Toy_Bucket** bucketHandle) {
	{
		//setup
		const char* source = "(1 + 2) * (3 + 4);";
		Toy_Lexer lexer;
		Toy_Parser parser;

		Toy_bindLexer(&lexer, source);
		Toy_bindParser(&parser, &lexer);
		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		//run
		Toy_Bytecode bc = Toy_compileBytecode(ast);

		//check bytecode alignment
		if (bc.count % 4 != 0) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: bytecode alignment is not a multiple of 4 (size is %d), source: %s\n" TOY_CC_RESET, (int)bc.count, source);

			//cleanup and return
			Toy_freeBytecode(bc);
			return -1;
		}

		//check bytecode header
		if (bc.ptr[0] != TOY_VERSION_MAJOR ||
			bc.ptr[1] != TOY_VERSION_MINOR ||
			bc.ptr[2] != TOY_VERSION_PATCH ||
			strcmp((char*)(bc.ptr + 3), TOY_VERSION_BUILD) != 0)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to write the bytecode header, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			Toy_freeBytecode(bc);
			return -1;
		}

		//check contents of the routine (this is copy/pasted from test_routine.c, and tweaked with the offset)
		int offset = 3 + strlen(TOY_VERSION_BUILD) + 1;
		if (offset % 4 != 0) {
			offset += 4 - (offset % 4); //ceil
		}

		int* ptr = (int*)(bc.ptr + offset);

		if ((ptr++)[0] != 72 || //total size
			(ptr++)[0] != 0 || //param count
			(ptr++)[0] != 0 || //jump count
			(ptr++)[0] != 0 || //data count
			(ptr++)[0] != 0) //subs count
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine header within bytecode, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			Toy_freeBytecode(bc);
			return -1;
		}

		//check code
		if (
			//left hand side
			*((unsigned char*)(offset + bc.ptr + 24)) != TOY_OPCODE_READ ||
			*((unsigned char*)(offset + bc.ptr + 25)) != TOY_VALUE_INTEGER ||
			*((unsigned char*)(offset + bc.ptr + 26)) != 0 ||
			*((unsigned char*)(offset + bc.ptr + 27)) != 0 ||
			*(int*)(offset + bc.ptr + 28) != 1 ||

			*((unsigned char*)(offset + bc.ptr + 32)) != TOY_OPCODE_READ ||
			*((unsigned char*)(offset + bc.ptr + 33)) != TOY_VALUE_INTEGER ||
			*((unsigned char*)(offset + bc.ptr + 34)) != 0 ||
			*((unsigned char*)(offset + bc.ptr + 35)) != 0 ||
			*(int*)(offset + bc.ptr + 36) != 2 ||

			*((unsigned char*)(offset + bc.ptr + 40)) != TOY_OPCODE_ADD ||
			*((unsigned char*)(offset + bc.ptr + 41)) != TOY_OPCODE_PASS ||
			*((unsigned char*)(offset + bc.ptr + 42)) != 0 ||
			*((unsigned char*)(offset + bc.ptr + 43)) != 0 ||

			//right hand side
			*((unsigned char*)(offset + bc.ptr + 44)) != TOY_OPCODE_READ ||
			*((unsigned char*)(offset + bc.ptr + 45)) != TOY_VALUE_INTEGER ||
			*((unsigned char*)(offset + bc.ptr + 46)) != 0 ||
			*((unsigned char*)(offset + bc.ptr + 47)) != 0 ||
			*(int*)(offset + bc.ptr + 48) != 3 ||

			*((unsigned char*)(offset + bc.ptr + 52)) != TOY_OPCODE_READ ||
			*((unsigned char*)(offset + bc.ptr + 53)) != TOY_VALUE_INTEGER ||
			*((unsigned char*)(offset + bc.ptr + 54)) != 0 ||
			*((unsigned char*)(offset + bc.ptr + 55)) != 0 ||
			*(int*)(offset + bc.ptr + 56) != 4 ||

			*((unsigned char*)(offset + bc.ptr + 60)) != TOY_OPCODE_ADD ||
			*((unsigned char*)(offset + bc.ptr + 61)) != TOY_OPCODE_PASS ||
			*((unsigned char*)(offset + bc.ptr + 62)) != 0 ||
			*((unsigned char*)(offset + bc.ptr + 63)) != 0 ||

			//multiply the two values
			*((unsigned char*)(offset + bc.ptr + 64)) != TOY_OPCODE_MULTIPLY ||
			*((unsigned char*)(offset + bc.ptr + 65)) != TOY_OPCODE_PASS ||
			*((unsigned char*)(offset + bc.ptr + 66)) != 0 ||
			*((unsigned char*)(offset + bc.ptr + 67)) != 0 ||

			*((unsigned char*)(offset + bc.ptr + 68)) != TOY_OPCODE_RETURN ||
			*((unsigned char*)(offset + bc.ptr + 69)) != 0 ||
			*((unsigned char*)(offset + bc.ptr + 70)) != 0 ||
			*((unsigned char*)(offset + bc.ptr + 71)) != 0
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine code within bytecode, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			Toy_freeBytecode(bc);
			return -1;
		}

		//cleanup
		Toy_freeBytecode(bc);
	}

	return 0;
}

int main(void) {
	//run each test set, returning the total errors given
	int total = 0, res = 0;

	{
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);
		res = test_bytecode_header(&bucket);
		Toy_freeBucket(&bucket);
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);
		res = test_bytecode_from_source(&bucket);
		Toy_freeBucket(&bucket);
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	return total;
}
