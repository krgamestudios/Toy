#include "toy_routine.h"
#include "toy_console_colors.h"

#include "toy_opcodes.h"
#include "toy_lexer.h"
#include "toy_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//tests
int test_routine_expressions(Toy_Bucket** bucketHandle) {
	//simple test to ensure the header looks right with an empty ast
	{
		//setup
		Toy_Ast* ast = NULL;
		Toy_private_emitAstPass(bucketHandle, &ast);

		//run
		void* buffer = Toy_compileRoutine(ast);

		//check header
		int* ptr = (int*)buffer;

		if ((ptr++)[0] != 28 || //total size
			(ptr++)[0] != 0 || //param count
			(ptr++)[0] != 0 || //jump count
			(ptr++)[0] != 0 || //data count
			(ptr++)[0] != 0) //subs count
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine header, ast: PASS\n" TOY_CC_RESET);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//check code
		if (*((unsigned char*)(buffer + 24)) != TOY_OPCODE_RETURN ||
			*((unsigned char*)(buffer + 25)) != 0 ||
			*((unsigned char*)(buffer + 26)) != 0 ||
			*((unsigned char*)(buffer + 27)) != 0
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine code, ast: PASS\n" TOY_CC_RESET);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//cleanup
		free(buffer);
	}

	//rerun the test with a more complex ast, derived from a snippet of source
	{
		//setup
		const char* source = ";"; //interestingly, different ASTs will produce the same output
		Toy_Lexer lexer;
		Toy_Parser parser;

		Toy_bindLexer(&lexer, source);
		Toy_bindParser(&parser, &lexer);
		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		//run
		void* buffer = Toy_compileRoutine(ast);

		//check header
		int* ptr = (int*)buffer;

		if ((ptr++)[0] != 28 || //total size
			(ptr++)[0] != 0 || //param count
			(ptr++)[0] != 0 || //jump count
			(ptr++)[0] != 0 || //data count
			(ptr++)[0] != 0) //subs count
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine header, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//check code
		if (*((unsigned char*)(buffer + 24)) != TOY_OPCODE_RETURN ||
			*((unsigned char*)(buffer + 25)) != 0 ||
			*((unsigned char*)(buffer + 26)) != 0 ||
			*((unsigned char*)(buffer + 27)) != 0
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine code, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//cleanup
		free(buffer);
	}

	//produce a null value
	{
		//setup
		const char* source = "null;";
		Toy_Lexer lexer;
		Toy_Parser parser;

		Toy_bindLexer(&lexer, source);
		Toy_bindParser(&parser, &lexer);
		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		//run
		void* buffer = Toy_compileRoutine(ast);

		//check header
		int* ptr = (int*)buffer;

		if ((ptr++)[0] != 32 || //total size
			(ptr++)[0] != 0 || //param count
			(ptr++)[0] != 0 || //jump count
			(ptr++)[0] != 0 || //data count
			(ptr++)[0] != 0) //subs count
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine header, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//check code
		if (*((unsigned char*)(buffer + 24)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 25)) != TOY_VALUE_NULL ||
			*((unsigned char*)(buffer + 26)) != 0 ||
			*((unsigned char*)(buffer + 27)) != 0 ||
			*((unsigned char*)(buffer + 28)) != TOY_OPCODE_RETURN ||
			*((unsigned char*)(buffer + 29)) != 0 ||
			*((unsigned char*)(buffer + 30)) != 0 ||
			*((unsigned char*)(buffer + 31)) != 0
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine code, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//cleanup
		free(buffer);
	}

	//produce a boolean value
	{
		//setup
		const char* source = "true;";
		Toy_Lexer lexer;
		Toy_Parser parser;

		Toy_bindLexer(&lexer, source);
		Toy_bindParser(&parser, &lexer);
		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		//run
		void* buffer = Toy_compileRoutine(ast);

		//check header
		int* ptr = (int*)buffer;

		if ((ptr++)[0] != 32 || //total size
			(ptr++)[0] != 0 || //param count
			(ptr++)[0] != 0 || //jump count
			(ptr++)[0] != 0 || //data count
			(ptr++)[0] != 0) //subs count
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine header, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//check code
		if (*((unsigned char*)(buffer + 24)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 25)) != TOY_VALUE_BOOLEAN ||
			*((unsigned char*)(buffer + 26)) != 1 ||
			*((unsigned char*)(buffer + 27)) != 0 ||
			*((unsigned char*)(buffer + 28)) != TOY_OPCODE_RETURN ||
			*((unsigned char*)(buffer + 29)) != 0 ||
			*((unsigned char*)(buffer + 30)) != 0 ||
			*((unsigned char*)(buffer + 31)) != 0
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine code, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//cleanup
		free(buffer);
	}

	//produce an integer value
	{
		//setup
		const char* source = "42;";
		Toy_Lexer lexer;
		Toy_Parser parser;

		Toy_bindLexer(&lexer, source);
		Toy_bindParser(&parser, &lexer);
		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		//run
		void* buffer = Toy_compileRoutine(ast);

		//check header
		int* ptr = (int*)buffer;

		if ((ptr++)[0] != 36 || //total size
			(ptr++)[0] != 0 || //param count
			(ptr++)[0] != 0 || //jump count
			(ptr++)[0] != 0 || //data count
			(ptr++)[0] != 0) //subs count
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine header, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//check code
		if (*((unsigned char*)(buffer + 24)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 25)) != TOY_VALUE_INTEGER ||
			*((unsigned char*)(buffer + 26)) != 0 ||
			*((unsigned char*)(buffer + 27)) != 0 ||
			*(int*)(buffer + 28) != 42 ||
			*((unsigned char*)(buffer + 32)) != TOY_OPCODE_RETURN ||
			*((unsigned char*)(buffer + 33)) != 0 ||
			*((unsigned char*)(buffer + 34)) != 0 ||
			*((unsigned char*)(buffer + 35)) != 0
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine code, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//cleanup
		free(buffer);
	}

	//produce a float value
	{
		//setup
		const char* source = "3.1415;";
		Toy_Lexer lexer;
		Toy_Parser parser;

		Toy_bindLexer(&lexer, source);
		Toy_bindParser(&parser, &lexer);
		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		//run
		void* buffer = Toy_compileRoutine(ast);

		//check header
		int* ptr = (int*)buffer;

		if ((ptr++)[0] != 36 || //total size
			(ptr++)[0] != 0 || //param count
			(ptr++)[0] != 0 || //jump count
			(ptr++)[0] != 0 || //data count
			(ptr++)[0] != 0) //subs count
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine header, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//check code
		if (*((unsigned char*)(buffer + 24)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 25)) != TOY_VALUE_FLOAT ||
			*((unsigned char*)(buffer + 26)) != 0 ||
			*((unsigned char*)(buffer + 27)) != 0 ||
			*(float*)(buffer + 28) != 3.1415f ||
			*((unsigned char*)(buffer + 32)) != TOY_OPCODE_RETURN ||
			*((unsigned char*)(buffer + 33)) != 0 ||
			*((unsigned char*)(buffer + 34)) != 0 ||
			*((unsigned char*)(buffer + 35)) != 0
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine code, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//cleanup
		free(buffer);
	}

	//produce a string value
	{
		//setup
		const char* source = "\"Hello world!\";";
		Toy_Lexer lexer;
		Toy_Parser parser;

		Toy_bindLexer(&lexer, source);
		Toy_bindParser(&parser, &lexer);
		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		//run
		void* buffer = Toy_compileRoutine(ast);

		//check header
		int* header = (int*)buffer;

		if (header[0] != 64 || //total size
			header[1] != 0 || //param size
			header[2] != 4 || //jumps size
			header[3] != 16 || //data size
			header[4] != 0 || //subs size

			// header[??] != ?? || //params address
			header[5] != 32 || //code address
			header[6] != 44 || //jumps address
			header[7] != 48 || //data address
			// header[??] != ?? || //subs address

			false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine header, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		void* code = buffer + 32; //8 values in the header, each 4 bytes

		//check code
		if (
			//code start
			*((unsigned char*)(code + 0)) != TOY_OPCODE_READ ||
			*((unsigned char*)(code + 1)) != TOY_VALUE_STRING ||
			*((unsigned char*)(code + 2)) != TOY_STRING_LEAF ||
			*((unsigned char*)(code + 3)) != 0 ||
			*(unsigned int*)(code + 4) != 0 || //the jump index
			*((unsigned char*)(code + 8)) != TOY_OPCODE_RETURN ||
			*((unsigned char*)(code + 9)) != 0 ||
			*((unsigned char*)(code + 10)) != 0 ||
			*((unsigned char*)(code + 11)) != 0 ||

			false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine code, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		void* jumps = code + 12;

		//check jumps
		if (
			//code start
			*(unsigned int*)(jumps + 0) != 0 || //the address relative to the start of the data section

			false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine jumps, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		void* data = jumps + 4;

		//check data
		if (
			//data start (the end of the data is padded to the nearest multiple of 4)
			strcmp( ((char*)data) + ((unsigned int*)jumps)[0], "Hello world!" ) != 0 ||

			false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine data, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//cleanup
		free(buffer);
	}

	return 0;
}

// int test_routine_unary(Toy_Bucket** bucketHandle) {
// 	//Nothing produces a unary instruction yet
// }

int test_routine_binary(Toy_Bucket** bucketHandle) {
	//produce a simple algorithm
	{
		//setup
		const char* source = "3 + 5;";
		Toy_Lexer lexer;
		Toy_Parser parser;

		Toy_bindLexer(&lexer, source);
		Toy_bindParser(&parser, &lexer);
		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		//run
		void* buffer = Toy_compileRoutine(ast);

		//check header
		int* ptr = (int*)buffer;

		if ((ptr++)[0] != 48 || //total size
			(ptr++)[0] != 0 || //param count
			(ptr++)[0] != 0 || //jump count
			(ptr++)[0] != 0 || //data count
			(ptr++)[0] != 0) //subs count
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine header, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//check code
		if (*((unsigned char*)(buffer + 24)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 25)) != TOY_VALUE_INTEGER ||
			*((unsigned char*)(buffer + 26)) != 0 ||
			*((unsigned char*)(buffer + 27)) != 0 ||
			*(int*)(buffer + 28) != 3 ||

			*((unsigned char*)(buffer + 32)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 33)) != TOY_VALUE_INTEGER ||
			*((unsigned char*)(buffer + 34)) != 0 ||
			*((unsigned char*)(buffer + 35)) != 0 ||
			*(int*)(buffer + 36) != 5 ||

			*((unsigned char*)(buffer + 40)) != TOY_OPCODE_ADD ||
			*((unsigned char*)(buffer + 41)) != TOY_OPCODE_PASS ||
			*((unsigned char*)(buffer + 42)) != 0 ||
			*((unsigned char*)(buffer + 43)) != 0 ||

			*((unsigned char*)(buffer + 44)) != TOY_OPCODE_RETURN ||
			*((unsigned char*)(buffer + 45)) != 0 ||
			*((unsigned char*)(buffer + 46)) != 0 ||
			*((unsigned char*)(buffer + 47)) != 0
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine code, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//cleanup
		free(buffer);
	}

	//produce a simple comparison
	{
		//setup
		const char* source = "3 == 5;";
		Toy_Lexer lexer;
		Toy_Parser parser;

		Toy_bindLexer(&lexer, source);
		Toy_bindParser(&parser, &lexer);
		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		//run
		void* buffer = Toy_compileRoutine(ast);

		//check header
		int* ptr = (int*)buffer;

		if ((ptr++)[0] != 48 || //total size
			(ptr++)[0] != 0 || //param count
			(ptr++)[0] != 0 || //jump count
			(ptr++)[0] != 0 || //data count
			(ptr++)[0] != 0) //subs count
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine header, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//check code
		if (*((unsigned char*)(buffer + 24)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 25)) != TOY_VALUE_INTEGER ||
			*((unsigned char*)(buffer + 26)) != 0 ||
			*((unsigned char*)(buffer + 27)) != 0 ||
			*(int*)(buffer + 28) != 3 ||

			*((unsigned char*)(buffer + 32)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 33)) != TOY_VALUE_INTEGER ||
			*((unsigned char*)(buffer + 34)) != 0 ||
			*((unsigned char*)(buffer + 35)) != 0 ||
			*(int*)(buffer + 36) != 5 ||

			*((unsigned char*)(buffer + 40)) != TOY_OPCODE_COMPARE_EQUAL ||
			*((unsigned char*)(buffer + 41)) != 0 ||
			*((unsigned char*)(buffer + 42)) != 0 ||
			*((unsigned char*)(buffer + 43)) != 0 ||

			*((unsigned char*)(buffer + 44)) != TOY_OPCODE_RETURN ||
			*((unsigned char*)(buffer + 45)) != 0 ||
			*((unsigned char*)(buffer + 46)) != 0 ||
			*((unsigned char*)(buffer + 47)) != 0
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine code, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//cleanup
		free(buffer);
	}

	//produce a simple comparison
	{
		//setup
		const char* source = "3 != 5;";
		Toy_Lexer lexer;
		Toy_Parser parser;

		Toy_bindLexer(&lexer, source);
		Toy_bindParser(&parser, &lexer);
		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		//run
		void* buffer = Toy_compileRoutine(ast);

		//check header
		int* ptr = (int*)buffer;

		if ((ptr++)[0] != 48 || //total size
			(ptr++)[0] != 0 || //param count
			(ptr++)[0] != 0 || //jump count
			(ptr++)[0] != 0 || //data count
			(ptr++)[0] != 0) //subs count
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine header, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//check code
		if (*((unsigned char*)(buffer + 24)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 25)) != TOY_VALUE_INTEGER ||
			*((unsigned char*)(buffer + 26)) != 0 ||
			*((unsigned char*)(buffer + 27)) != 0 ||
			*(int*)(buffer + 28) != 3 ||

			*((unsigned char*)(buffer + 32)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 33)) != TOY_VALUE_INTEGER ||
			*((unsigned char*)(buffer + 34)) != 0 ||
			*((unsigned char*)(buffer + 35)) != 0 ||
			*(int*)(buffer + 36) != 5 ||

			*((unsigned char*)(buffer + 40)) != TOY_OPCODE_COMPARE_EQUAL ||
			*((unsigned char*)(buffer + 41)) != TOY_OPCODE_NEGATE ||
			*((unsigned char*)(buffer + 42)) != 0 ||
			*((unsigned char*)(buffer + 43)) != 0 ||

			*((unsigned char*)(buffer + 44)) != TOY_OPCODE_RETURN ||
			*((unsigned char*)(buffer + 45)) != 0 ||
			*((unsigned char*)(buffer + 46)) != 0 ||
			*((unsigned char*)(buffer + 47)) != 0
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine code, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//cleanup
		free(buffer);
	}

	//produce a more complex algorithm
	{
		//setup
		const char* source = "(1 + 2) * (3 + 4);";
		Toy_Lexer lexer;
		Toy_Parser parser;

		Toy_bindLexer(&lexer, source);
		Toy_bindParser(&parser, &lexer);
		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		//run
		void* buffer = Toy_compileRoutine(ast);

		//check header
		int* ptr = (int*)buffer;

		if ((ptr++)[0] != 72 || //total size
			(ptr++)[0] != 0 || //param count
			(ptr++)[0] != 0 || //jump count
			(ptr++)[0] != 0 || //data count
			(ptr++)[0] != 0) //subs count
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine header, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//check code
		if (
			//left hand side
			*((unsigned char*)(buffer + 24)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 25)) != TOY_VALUE_INTEGER ||
			*((unsigned char*)(buffer + 26)) != 0 ||
			*((unsigned char*)(buffer + 27)) != 0 ||
			*(int*)(buffer + 28) != 1 ||

			*((unsigned char*)(buffer + 32)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 33)) != TOY_VALUE_INTEGER ||
			*((unsigned char*)(buffer + 34)) != 0 ||
			*((unsigned char*)(buffer + 35)) != 0 ||
			*(int*)(buffer + 36) != 2 ||

			*((unsigned char*)(buffer + 40)) != TOY_OPCODE_ADD ||
			*((unsigned char*)(buffer + 41)) != TOY_OPCODE_PASS ||
			*((unsigned char*)(buffer + 42)) != 0 ||
			*((unsigned char*)(buffer + 43)) != 0 ||

			//right hand side
			*((unsigned char*)(buffer + 44)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 45)) != TOY_VALUE_INTEGER ||
			*((unsigned char*)(buffer + 46)) != 0 ||
			*((unsigned char*)(buffer + 47)) != 0 ||
			*(int*)(buffer + 48) != 3 ||

			*((unsigned char*)(buffer + 52)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 53)) != TOY_VALUE_INTEGER ||
			*((unsigned char*)(buffer + 54)) != 0 ||
			*((unsigned char*)(buffer + 55)) != 0 ||
			*(int*)(buffer + 56) != 4 ||

			*((unsigned char*)(buffer + 60)) != TOY_OPCODE_ADD ||
			*((unsigned char*)(buffer + 61)) != TOY_OPCODE_PASS ||
			*((unsigned char*)(buffer + 62)) != 0 ||
			*((unsigned char*)(buffer + 63)) != 0 ||

			//multiply the two values
			*((unsigned char*)(buffer + 64)) != TOY_OPCODE_MULTIPLY ||
			*((unsigned char*)(buffer + 65)) != TOY_OPCODE_PASS ||
			*((unsigned char*)(buffer + 66)) != 0 ||
			*((unsigned char*)(buffer + 67)) != 0 ||

			*((unsigned char*)(buffer + 68)) != TOY_OPCODE_RETURN ||
			*((unsigned char*)(buffer + 69)) != 0 ||
			*((unsigned char*)(buffer + 70)) != 0 ||
			*((unsigned char*)(buffer + 71)) != 0
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine code, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//cleanup
		free(buffer);
	}

	return 0;
}

int test_routine_keywords(Toy_Bucket** bucketHandle) {
	//assert
	{
		//setup
		const char* source = "assert true;";
		Toy_Lexer lexer;
		Toy_Parser parser;

		Toy_bindLexer(&lexer, source);
		Toy_bindParser(&parser, &lexer);
		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		//run
		void* buffer = Toy_compileRoutine(ast);

		//check header
		int* ptr = (int*)buffer;

		if ((ptr++)[0] != 36 || //total size
			(ptr++)[0] != 0 || //param count
			(ptr++)[0] != 0 || //jump count
			(ptr++)[0] != 0 || //data count
			(ptr++)[0] != 0) //subs count
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine header, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//check code
		if (*((unsigned char*)(buffer + 24)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 25)) != TOY_VALUE_BOOLEAN ||
			*((bool*)(buffer + 26)) != true || //bools are packed
			*((unsigned char*)(buffer + 27)) != 0 ||

			*((unsigned char*)(buffer + 28)) != TOY_OPCODE_ASSERT ||
			*((unsigned char*)(buffer + 29)) != 1 || //one parameter
			*((unsigned char*)(buffer + 30)) != 0 ||
			*((unsigned char*)(buffer + 31)) != 0 ||

			*((unsigned char*)(buffer + 32)) != TOY_OPCODE_RETURN ||
			*((unsigned char*)(buffer + 33)) != 0 ||
			*((unsigned char*)(buffer + 34)) != 0 ||
			*((unsigned char*)(buffer + 35)) != 0
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine code, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//cleanup
		free(buffer);
	}

	//assert (with message)
	{
		//setup
		const char* source = "assert true, false;";
		Toy_Lexer lexer;
		Toy_Parser parser;

		Toy_bindLexer(&lexer, source);
		Toy_bindParser(&parser, &lexer);
		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		//run
		void* buffer = Toy_compileRoutine(ast);

		//check header
		int* ptr = (int*)buffer;

		if ((ptr++)[0] != 40 || //total size
			(ptr++)[0] != 0 || //param count
			(ptr++)[0] != 0 || //jump count
			(ptr++)[0] != 0 || //data count
			(ptr++)[0] != 0) //subs count
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine header, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//check code
		if (*((unsigned char*)(buffer + 24)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 25)) != TOY_VALUE_BOOLEAN ||
			*((bool*)(buffer + 26)) != true || //bools are packed
			*((unsigned char*)(buffer + 27)) != 0 ||

			*((unsigned char*)(buffer + 28)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 29)) != TOY_VALUE_BOOLEAN ||
			*((bool*)(buffer + 30)) != false || //bools are packed
			*((unsigned char*)(buffer + 31)) != 0 ||

			*((unsigned char*)(buffer + 32)) != TOY_OPCODE_ASSERT ||
			*((unsigned char*)(buffer + 33)) != 2 || //two parameters
			*((unsigned char*)(buffer + 34)) != 0 ||
			*((unsigned char*)(buffer + 35)) != 0 ||

			*((unsigned char*)(buffer + 36)) != TOY_OPCODE_RETURN ||
			*((unsigned char*)(buffer + 37)) != 0 ||
			*((unsigned char*)(buffer + 38)) != 0 ||
			*((unsigned char*)(buffer + 39)) != 0
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine code, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//cleanup
		free(buffer);
	}

	//if-then
	{
		//setup
		const char* source = "if (true) print \"hello world\";";
		Toy_Lexer lexer;
		Toy_Parser parser;

		Toy_bindLexer(&lexer, source);
		Toy_bindParser(&parser, &lexer);
		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		//run
		void* buffer = Toy_compileRoutine(ast);

		//check header
		int* ptr = (int*)buffer;

		if ((ptr++)[0] != 76 || //total size
			(ptr++)[0] != 0 || //param count
			(ptr++)[0] != 4 || //jump count
			(ptr++)[0] != 12 || //data count
			(ptr++)[0] != 0 || //subs count

			(ptr++)[0] != 32 || //code addr
			(ptr++)[0] != 60 || //jump addr
			(ptr++)[0] != 64 || //data addr

			//header size: 32

			false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine header, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//check code
		if (//cond
			*((unsigned char*)(buffer + 32)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 33)) != TOY_VALUE_BOOLEAN ||
			*((bool*)(buffer + 34)) != true || //bools are packed
			*((unsigned char*)(buffer + 35)) != 0 ||

			*((unsigned char*)(buffer + 36)) != TOY_OPCODE_JUMP ||
			*((unsigned char*)(buffer + 37)) != TOY_OP_PARAM_JUMP_RELATIVE ||
			*((unsigned char*)(buffer + 38)) != TOY_OP_PARAM_JUMP_IF_FALSE ||
			*((unsigned char*)(buffer + 39)) != 0 ||

			*((int*)(buffer + 40)) != 12 || //param: jump distance (relative)

			//then branch
			*((unsigned char*)(buffer + 44)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 45)) != TOY_VALUE_STRING ||
			*((unsigned char*)(buffer + 46)) != TOY_STRING_LEAF ||
			*((unsigned char*)(buffer + 47)) != 0 ||

			*((int*)(buffer + 48)) != 0 || //first indirection

			*((unsigned char*)(buffer + 52)) != TOY_OPCODE_PRINT ||
			*((unsigned char*)(buffer + 53)) != 0 ||
			*((unsigned char*)(buffer + 54)) != 0 ||
			*((unsigned char*)(buffer + 55)) != 0 ||

			//EOF
			*((unsigned char*)(buffer + 56)) != TOY_OPCODE_RETURN ||
			*((unsigned char*)(buffer + 57)) != 0 ||
			*((unsigned char*)(buffer + 58)) != 0 ||
			*((unsigned char*)(buffer + 59)) != 0 ||

			//jump region
			*((int*)(buffer + 60)) != 0 ||

			//data region (strings begin at 4-byte words)
			strcmp((char*)(buffer + 64), "hello world") != 0 ||

			false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine code, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//cleanup
		free(buffer);
	}

	//if-then-else
	{
		//setup
		const char* source = "if (true) print \"hello world\"; else print \"goodbye world\";";
		Toy_Lexer lexer;
		Toy_Parser parser;

		Toy_bindLexer(&lexer, source);
		Toy_bindParser(&parser, &lexer);
		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		//run
		void* buffer = Toy_compileRoutine(ast);

		//check header
		int* ptr = (int*)buffer;

		if ((ptr++)[0] != 116 || //total size
			(ptr++)[0] != 0 || //param count
			(ptr++)[0] != 8 || //jump count
			(ptr++)[0] != 28 || //data count
			(ptr++)[0] != 0 || //subs count

			(ptr++)[0] != 32 || //code addr
			(ptr++)[0] != 80 || //jump addr
			(ptr++)[0] != 88 || //data addr

			//header size: 32

			false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine header, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//check code
		if (//cond
			*((unsigned char*)(buffer + 32)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 33)) != TOY_VALUE_BOOLEAN ||
			*((bool*)(buffer + 34)) != true || //bools are packed
			*((unsigned char*)(buffer + 35)) != 0 ||

			*((unsigned char*)(buffer + 36)) != TOY_OPCODE_JUMP ||
			*((unsigned char*)(buffer + 37)) != TOY_OP_PARAM_JUMP_RELATIVE ||
			*((unsigned char*)(buffer + 38)) != TOY_OP_PARAM_JUMP_IF_FALSE ||
			*((unsigned char*)(buffer + 39)) != 0 ||

			*((int*)(buffer + 40)) != 20 || //param: jump distance (relative)

			//then branch
			*((unsigned char*)(buffer + 44)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 45)) != TOY_VALUE_STRING ||
			*((unsigned char*)(buffer + 46)) != TOY_STRING_LEAF ||
			*((unsigned char*)(buffer + 47)) != 0 ||

			*((int*)(buffer + 48)) != 0 || //first indirection

			*((unsigned char*)(buffer + 52)) != TOY_OPCODE_PRINT ||
			*((unsigned char*)(buffer + 53)) != 0 ||
			*((unsigned char*)(buffer + 54)) != 0 ||
			*((unsigned char*)(buffer + 55)) != 0 ||

			//jump to the end
			*((unsigned char*)(buffer + 56)) != TOY_OPCODE_JUMP ||
			*((unsigned char*)(buffer + 57)) != TOY_OP_PARAM_JUMP_RELATIVE ||
			*((unsigned char*)(buffer + 58)) != TOY_OP_PARAM_JUMP_ALWAYS ||
			*((unsigned char*)(buffer + 59)) != 0 ||

			*((int*)(buffer + 60)) != 12 || //param: jump distance (relative)

			//else branch
			*((unsigned char*)(buffer + 64)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 65)) != TOY_VALUE_STRING ||
			*((unsigned char*)(buffer + 66)) != TOY_STRING_LEAF ||
			*((unsigned char*)(buffer + 67)) != 0 ||

			*((int*)(buffer + 68)) != 4 || //second indirection

			*((unsigned char*)(buffer + 72)) != TOY_OPCODE_PRINT ||
			*((unsigned char*)(buffer + 73)) != 0 ||
			*((unsigned char*)(buffer + 74)) != 0 ||
			*((unsigned char*)(buffer + 75)) != 0 ||

			//EOF
			*((unsigned char*)(buffer + 76)) != TOY_OPCODE_RETURN ||
			*((unsigned char*)(buffer + 77)) != 0 ||
			*((unsigned char*)(buffer + 78)) != 0 ||
			*((unsigned char*)(buffer + 79)) != 0 ||

			//jump region
			*((int*)(buffer + 80)) != 0 ||
			*((int*)(buffer + 84)) != 12 ||

			//data region (strings begin at 4-byte words)
			strcmp((char*)(buffer + 88), "hello world") != 0 ||

			strcmp((char*)(buffer + 100), "goodbye world") != 0 ||

			false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine code, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//cleanup
		free(buffer);
	}

	//print
	{
		//setup
		const char* source = "print 42;";
		Toy_Lexer lexer;
		Toy_Parser parser;

		Toy_bindLexer(&lexer, source);
		Toy_bindParser(&parser, &lexer);
		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		//run
		void* buffer = Toy_compileRoutine(ast);

		//check header
		int* ptr = (int*)buffer;

		if ((ptr++)[0] != 40 || //total size
			(ptr++)[0] != 0 || //param count
			(ptr++)[0] != 0 || //jump count
			(ptr++)[0] != 0 || //data count
			(ptr++)[0] != 0) //subs count
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine header, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//check code
		if (*((unsigned char*)(buffer + 24)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 25)) != TOY_VALUE_INTEGER ||
			*((unsigned char*)(buffer + 26)) != 0 ||
			*((unsigned char*)(buffer + 27)) != 0 ||
			*(int*)(buffer + 28) != 42 ||
			*((unsigned char*)(buffer + 32)) != TOY_OPCODE_PRINT ||
			*((unsigned char*)(buffer + 33)) != 0 ||
			*((unsigned char*)(buffer + 34)) != 0 ||
			*((unsigned char*)(buffer + 35)) != 0 ||
			*((unsigned char*)(buffer + 36)) != TOY_OPCODE_RETURN ||
			*((unsigned char*)(buffer + 37)) != 0 ||
			*((unsigned char*)(buffer + 38)) != 0 ||
			*((unsigned char*)(buffer + 39)) != 0
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine code, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//cleanup
		free(buffer);
	}

	//var declare
	{
		//setup
		const char* source = "var foobar = 42;";
		Toy_Lexer lexer;
		Toy_Parser parser;

		Toy_bindLexer(&lexer, source);
		Toy_bindParser(&parser, &lexer);
		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		//run
		void* buffer = Toy_compileRoutine(ast);

		//check header
		int* header = (int*)buffer;

		if (header[0] != 64 || //total size
			header[1] != 0 || //param size
			header[2] != 4 || //jumps size
			header[3] != 8 || //data size
			header[4] != 0 || //subs size

			// header[??] != ?? || //params address
			header[5] != 32 || //code address
			header[6] != 52 || //jumps address
			header[7] != 56 || //data address
			// header[??] != ?? || //subs address

			false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine header, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		void* code = buffer + 32; //8 values in the header, each 4 bytes

		//check code
		if (
			//code start
			*((unsigned char*)(code + 0)) != TOY_OPCODE_READ ||
			*((unsigned char*)(code + 1)) != TOY_VALUE_INTEGER ||
			*((unsigned char*)(code + 2)) != 0 ||
			*((unsigned char*)(code + 3)) != 0 ||

			*(int*)(code + 4) != 42 ||

			*((unsigned char*)(code + 8)) != TOY_OPCODE_DECLARE ||
			*((unsigned char*)(code + 9)) != TOY_VALUE_ANY ||
			*((unsigned char*)(code + 10)) != 6 || //strlen
			*((unsigned char*)(code + 11)) != 0 ||

			*(unsigned int*)(code + 12) != 0 || //the jump index

			*((unsigned char*)(code + 16)) != TOY_OPCODE_RETURN ||
			*((unsigned char*)(code + 17)) != 0 ||
			*((unsigned char*)(code + 18)) != 0 ||
			*((unsigned char*)(code + 19)) != 0 ||

			false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine code, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		void* jumps = code + 20;

		//check jumps
		if (
			//code start
			*(unsigned int*)(jumps + 0) != 0 || //the address relative to the start of the data section

			false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine jumps, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		void* data = jumps + 4;

		//check data
		if (
			//data start (the end of the data is padded to the nearest multiple of 4)
			strcmp( ((char*)data) + ((unsigned int*)jumps)[0], "foobar" ) != 0 ||

			false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine data, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//cleanup
		free(buffer);
	}

	//var declare (with type)
	{
		//setup
		const char* source = "var foobar: int = 42;";
		Toy_Lexer lexer;
		Toy_Parser parser;

		Toy_bindLexer(&lexer, source);
		Toy_bindParser(&parser, &lexer);
		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		//run
		void* buffer = Toy_compileRoutine(ast);

		//check header
		int* header = (int*)buffer;

		if (header[0] != 64 || //total size
			header[1] != 0 || //param size
			header[2] != 4 || //jumps size
			header[3] != 8 || //data size
			header[4] != 0 || //subs size

			// header[??] != ?? || //params address
			header[5] != 32 || //code address
			header[6] != 52 || //jumps address
			header[7] != 56 || //data address
			// header[??] != ?? || //subs address

			false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine header, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		void* code = buffer + 32; //8 values in the header, each 4 bytes

		//check code
		if (
			//code start
			*((unsigned char*)(code + 0)) != TOY_OPCODE_READ ||
			*((unsigned char*)(code + 1)) != TOY_VALUE_INTEGER ||
			*((unsigned char*)(code + 2)) != 0 ||
			*((unsigned char*)(code + 3)) != 0 ||

			*(int*)(code + 4) != 42 ||

			*((unsigned char*)(code + 8)) != TOY_OPCODE_DECLARE ||
			*((unsigned char*)(code + 9)) != TOY_VALUE_INTEGER ||
			*((unsigned char*)(code + 10)) != 6 || //strlen
			*((unsigned char*)(code + 11)) != 0 ||

			*(unsigned int*)(code + 12) != 0 || //the jump index

			*((unsigned char*)(code + 16)) != TOY_OPCODE_RETURN ||
			*((unsigned char*)(code + 17)) != 0 ||
			*((unsigned char*)(code + 18)) != 0 ||
			*((unsigned char*)(code + 19)) != 0 ||

			false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine code, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		void* jumps = code + 20;

		//check jumps
		if (
			//code start
			*(unsigned int*)(jumps + 0) != 0 || //the address relative to the start of the data section

			false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine jumps, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		void* data = jumps + 4;

		//check data
		if (
			//data start (the end of the data is padded to the nearest multiple of 4)
			strcmp( ((char*)data) + ((unsigned int*)jumps)[0], "foobar" ) != 0 ||

			false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine data, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//cleanup
		free(buffer);
	}

	return 0;
}

int main() {
	//run each test set, returning the total errors given
	int total = 0, res = 0;

	{
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);
		res = test_routine_expressions(&bucket);
		Toy_freeBucket(&bucket);
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);
		res = test_routine_binary(&bucket);
		Toy_freeBucket(&bucket);
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);
		res = test_routine_keywords(&bucket);
		Toy_freeBucket(&bucket);
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	return total;
}
