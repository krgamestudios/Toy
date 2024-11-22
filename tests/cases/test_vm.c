#include "toy_vm.h"
#include "toy_console_colors.h"

#include "toy_lexer.h"
#include "toy_parser.h"
#include "toy_bytecode.h"
#include "toy_print.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//utils
Toy_Bytecode makeBytecodeFromSource(Toy_Bucket** bucketHandle, const char* source) { //did I forget this?
	Toy_Lexer lexer;
	Toy_bindLexer(&lexer, source);

	Toy_Parser parser;
	Toy_bindParser(&parser, &lexer);

	Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);
	Toy_Bytecode bc = Toy_compileBytecode(ast);

	return bc;
}

//tests
int test_setup_and_teardown(Toy_Bucket** bucketHandle) {
	//basic init & quit
	{
		//generate bytecode for testing
		const char* source = "(1 + 2) * (3 + 4);";

		Toy_Lexer lexer;
		Toy_bindLexer(&lexer, source);

		Toy_Parser parser;
		Toy_bindParser(&parser, &lexer);

		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		Toy_Bytecode bc = Toy_compileBytecode(ast);

		//run the setup
		Toy_VM vm;
		Toy_initVM(&vm);
		Toy_bindVM(&vm, &bc);

		//check the header size
		int headerSize = 3 + strlen(TOY_VERSION_BUILD) + 1;
		if (headerSize % 4 != 0) {
			headerSize += 4 - (headerSize % 4); //ceil
		}

		//check the module was loaded correctly
		if (
			vm.module - bc.ptr != headerSize ||
			vm.moduleSize != 72 ||
			vm.paramSize != 0 ||
			vm.jumpsSize != 0 ||
			vm.dataSize != 0 ||
			vm.subsSize != 0
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to setup and teadown Toy_VM, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			Toy_freeVM(&vm);
			Toy_freeBytecode(bc);
			return -1;
		}

		//don't run it this time, simply teadown
		Toy_freeVM(&vm);
		Toy_freeBytecode(bc);
	}

	return 0;
}

int test_simple_execution(Toy_Bucket** bucketHandle) {
	//test execution
	{
		//generate bytecode for testing
		const char* source = "(1 + 2) * (3 + 4);";

		Toy_Lexer lexer;
		Toy_bindLexer(&lexer, source);

		Toy_Parser parser;
		Toy_bindParser(&parser, &lexer);

		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		Toy_Bytecode bc = Toy_compileBytecode(ast);

		//run the setup
		Toy_VM vm;
		Toy_initVM(&vm);
		Toy_bindVM(&vm, &bc);

		//run
		Toy_runVM(&vm);

		//check the final state of the stack
		if (vm.stack == NULL ||
			vm.stack->count != 1 ||
			TOY_VALUE_IS_INTEGER( Toy_peekStack(&vm.stack) ) != true ||
			TOY_VALUE_AS_INTEGER( Toy_peekStack(&vm.stack) ) != 21
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected result in 'Toy_VM', source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			Toy_freeVM(&vm);
			Toy_freeBytecode(bc);
			return -1;
		}

		//teadown
		Toy_freeVM(&vm);
		Toy_freeBytecode(bc);
	}

	return 0;
}

int test_opcode_not_equal(Toy_Bucket** bucketHandle) {
	//testing a specific opcode; '!=' is compressed into a single word, so lets check it works
	{
		//generate bytecode for testing
		const char* source = "3 != 5;";

		Toy_Lexer lexer;
		Toy_bindLexer(&lexer, source);

		Toy_Parser parser;
		Toy_bindParser(&parser, &lexer);

		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		Toy_Bytecode bc = Toy_compileBytecode(ast);

		//run the setup
		Toy_VM vm;
		Toy_initVM(&vm);
		Toy_bindVM(&vm, &bc);

		//run
		Toy_runVM(&vm);

		//check the final state of the stack
		if (vm.stack == NULL ||
			vm.stack->count != 1 ||
			TOY_VALUE_IS_BOOLEAN( Toy_peekStack(&vm.stack) ) != true ||
			TOY_VALUE_AS_BOOLEAN( Toy_peekStack(&vm.stack) ) != true
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected result in 'Toy_VM', source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			Toy_freeVM(&vm);
			Toy_freeBytecode(bc);
			return -1;
		}

		//teadown
		Toy_freeVM(&vm);
		Toy_freeBytecode(bc);
	}

	return 0;
}

static char* callbackUtilReceived = NULL;
static void callbackUtil(const char* msg) {
	if (msg != NULL) {
		free(callbackUtilReceived);
		callbackUtilReceived = (char*)malloc(strlen(msg) + 1);
		strcpy(callbackUtilReceived, msg);
	}
}

int test_keyword_assert(Toy_Bucket** bucketHandle) {
	//test assert true
	{
		//setup
		Toy_setAssertFailureCallback(callbackUtil);
		const char* source = "assert true;";

		Toy_Lexer lexer;
		Toy_bindLexer(&lexer, source);

		Toy_Parser parser;
		Toy_bindParser(&parser, &lexer);

		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);
		Toy_Bytecode bc = Toy_compileBytecode(ast);

		Toy_VM vm;
		Toy_initVM(&vm);
		Toy_bindVM(&vm, &bc);

		//run
		Toy_runVM(&vm);

		//check
		if (callbackUtilReceived != NULL)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected assert message '%s' found, source: %s\n" TOY_CC_RESET, callbackUtilReceived != NULL ? callbackUtilReceived : "NULL", source);

			//cleanup and return
			Toy_resetAssertFailureCallback();
			free(callbackUtilReceived);
			Toy_freeVM(&vm);
			Toy_freeBytecode(bc);
			return -1;
		}

		//teadown
		Toy_resetAssertFailureCallback();
		free(callbackUtilReceived);
		callbackUtilReceived = NULL;
		Toy_freeVM(&vm);
		Toy_freeBytecode(bc);
	}

	//test assert false
	{
		//setup
		Toy_setAssertFailureCallback(callbackUtil);
		const char* source = "assert false;";

		Toy_Lexer lexer;
		Toy_bindLexer(&lexer, source);

		Toy_Parser parser;
		Toy_bindParser(&parser, &lexer);

		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);
		Toy_Bytecode bc = Toy_compileBytecode(ast);

		Toy_VM vm;
		Toy_initVM(&vm);
		Toy_bindVM(&vm, &bc);

		//run
		Toy_runVM(&vm);

		//check
		if (callbackUtilReceived == NULL ||
			strcmp(callbackUtilReceived, "assertion failed") != 0)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected assert failure message '%s' found, source: %s\n" TOY_CC_RESET, callbackUtilReceived != NULL ? callbackUtilReceived : "NULL", source);

			//cleanup and return
			Toy_resetAssertFailureCallback();
			free(callbackUtilReceived);
			Toy_freeVM(&vm);
			Toy_freeBytecode(bc);
			return -1;
		}

		//teadown
		Toy_resetAssertFailureCallback();
		free(callbackUtilReceived);
		callbackUtilReceived = NULL;
		Toy_freeVM(&vm);
		Toy_freeBytecode(bc);
	}

	//test assert false with message
	{
		//setup
		Toy_setAssertFailureCallback(callbackUtil);
		const char* source = "assert false, \"You passed a false to assert\";";

		Toy_Lexer lexer;
		Toy_bindLexer(&lexer, source);

		Toy_Parser parser;
		Toy_bindParser(&parser, &lexer);

		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);
		Toy_Bytecode bc = Toy_compileBytecode(ast);

		Toy_VM vm;
		Toy_initVM(&vm);
		Toy_bindVM(&vm, &bc);

		//run
		Toy_runVM(&vm);

		//check
		if (callbackUtilReceived == NULL ||
			strcmp(callbackUtilReceived, "You passed a false to assert") != 0)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected assert failure message '%s' found, source: %s\n" TOY_CC_RESET, callbackUtilReceived != NULL ? callbackUtilReceived : "NULL", source);

			//cleanup and return
			Toy_resetAssertFailureCallback();
			free(callbackUtilReceived);
			Toy_freeVM(&vm);
			Toy_freeBytecode(bc);
			return -1;
		}

		//teadown
		Toy_resetAssertFailureCallback();
		free(callbackUtilReceived);
		callbackUtilReceived = NULL;
		Toy_freeVM(&vm);
		Toy_freeBytecode(bc);
	}

	return 0;
}

int test_keyword_print(Toy_Bucket** bucketHandle) {
	//test print
	{
		//setup
		Toy_setPrintCallback(callbackUtil);
		const char* source = "print 42;";

		Toy_Lexer lexer;
		Toy_bindLexer(&lexer, source);

		Toy_Parser parser;
		Toy_bindParser(&parser, &lexer);

		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);
		Toy_Bytecode bc = Toy_compileBytecode(ast);

		Toy_VM vm;
		Toy_initVM(&vm);
		Toy_bindVM(&vm, &bc);

		//run
		Toy_runVM(&vm);

		//check
		if (callbackUtilReceived == NULL ||
			strcmp(callbackUtilReceived, "42") != 0)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected value '%s' passed to print keyword, source: %s\n" TOY_CC_RESET, callbackUtilReceived != NULL ? callbackUtilReceived : "NULL", source);

			//cleanup and return
			Toy_resetPrintCallback();
			free(callbackUtilReceived);
			Toy_freeVM(&vm);
			Toy_freeBytecode(bc);
			return -1;
		}

		//teadown
		Toy_resetPrintCallback();
		free(callbackUtilReceived);
		callbackUtilReceived = NULL;
		Toy_freeVM(&vm);
		Toy_freeBytecode(bc);
	}

	//test print with a string
	{
		//setup
		Toy_setPrintCallback(callbackUtil);
		const char* source = "print \"Hello world!\";";

		Toy_Lexer lexer;
		Toy_bindLexer(&lexer, source);

		Toy_Parser parser;
		Toy_bindParser(&parser, &lexer);

		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);
		Toy_Bytecode bc = Toy_compileBytecode(ast);

		Toy_VM vm;
		Toy_initVM(&vm);
		Toy_bindVM(&vm, &bc);

		//run
		Toy_runVM(&vm);

		//check
		if (callbackUtilReceived == NULL ||
			strcmp(callbackUtilReceived, "Hello world!") != 0)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected value '%s' passed to print keyword, source: %s\n" TOY_CC_RESET, callbackUtilReceived != NULL ? callbackUtilReceived : "NULL", source);

			//cleanup and return
			Toy_resetPrintCallback();
			free(callbackUtilReceived);
			Toy_freeVM(&vm);
			Toy_freeBytecode(bc);
			return -1;
		}

		//teadown
		Toy_resetPrintCallback();
		free(callbackUtilReceived);
		callbackUtilReceived = NULL;
		Toy_freeVM(&vm);
		Toy_freeBytecode(bc);
	}

	//test print with a string concat
	{
		//setup
		Toy_setPrintCallback(callbackUtil);
		const char* source = "print \"Hello\" .. \"world!\";";

		Toy_Lexer lexer;
		Toy_bindLexer(&lexer, source);

		Toy_Parser parser;
		Toy_bindParser(&parser, &lexer);

		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);
		Toy_Bytecode bc = Toy_compileBytecode(ast);

		Toy_VM vm;
		Toy_initVM(&vm);
		Toy_bindVM(&vm, &bc);

		//run
		Toy_runVM(&vm);

		//check
		if (callbackUtilReceived == NULL ||
			strcmp(callbackUtilReceived, "Helloworld!") != 0)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected value '%s' passed to print keyword, source: %s\n" TOY_CC_RESET, callbackUtilReceived != NULL ? callbackUtilReceived : "NULL", source);

			//cleanup and return
			Toy_resetPrintCallback();
			free(callbackUtilReceived);
			Toy_freeVM(&vm);
			Toy_freeBytecode(bc);
			return -1;
		}

		//teadown
		Toy_resetPrintCallback();
		free(callbackUtilReceived);
		callbackUtilReceived = NULL;
		Toy_freeVM(&vm);
		Toy_freeBytecode(bc);
	}

	return 0;
}

int test_scope(Toy_Bucket** bucketHandle) {
	//test declaration with initial value
	{
		//generate bytecode for testing
		const char* source = "var foobar = 42;";

		Toy_Lexer lexer;
		Toy_bindLexer(&lexer, source);

		Toy_Parser parser;
		Toy_bindParser(&parser, &lexer);

		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		Toy_Bytecode bc = Toy_compileBytecode(ast);

		//run the setup
		Toy_VM vm;
		Toy_initVM(&vm);
		Toy_bindVM(&vm, &bc);

		//run
		Toy_runVM(&vm);

		//check
		Toy_String* key = Toy_createNameStringLength(bucketHandle, "foobar", 6, TOY_VALUE_ANY, false);

		if (vm.stack == NULL ||
			vm.stack->count != 0 ||

			vm.scope == NULL ||
			Toy_isDeclaredScope(vm.scope, key) == false ||
			TOY_VALUE_IS_INTEGER(Toy_accessScope(vm.scope, key)) != true ||
			TOY_VALUE_AS_INTEGER(Toy_accessScope(vm.scope, key)) != 42

		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected result in 'Toy_VM' when testing scope, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			Toy_freeVM(&vm);
			Toy_freeBytecode(bc);
			return -1;
		}

		//teadown
		Toy_freeVM(&vm);
		Toy_freeBytecode(bc);
	}

	//test declaration with absent value
	{
		//generate bytecode for testing
		const char* source = "var foobar;";

		Toy_Lexer lexer;
		Toy_bindLexer(&lexer, source);

		Toy_Parser parser;
		Toy_bindParser(&parser, &lexer);

		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		Toy_Bytecode bc = Toy_compileBytecode(ast);

		//run the setup
		Toy_VM vm;
		Toy_initVM(&vm);
		Toy_bindVM(&vm, &bc);

		//run
		Toy_runVM(&vm);

		//check
		Toy_String* key = Toy_createNameStringLength(bucketHandle, "foobar", 6, TOY_VALUE_UNKNOWN, false);

		if (vm.stack == NULL ||
			vm.stack->count != 0 ||

			vm.scope == NULL ||
			Toy_isDeclaredScope(vm.scope, key) == false ||
			TOY_VALUE_IS_NULL(Toy_accessScope(vm.scope, key)) != true
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected result in 'Toy_VM' when testing scope, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			Toy_freeVM(&vm);
			Toy_freeBytecode(bc);
			return -1;
		}

		//teadown
		Toy_freeVM(&vm);
		Toy_freeBytecode(bc);
	}

	return 0;
}

int test_vm_reuse(Toy_Bucket** bucketHandle) {
	//run code in the same vm multiple times
	{
		Toy_setPrintCallback(callbackUtil);

		Toy_VM vm;
		Toy_initVM(&vm);

		//run 1
		Toy_Bytecode bc1 = makeBytecodeFromSource(bucketHandle, "print \"Hello world!\";");
		Toy_bindVM(&vm, &bc1);
		Toy_runVM(&vm);
		Toy_resetVM(&vm);

		if (callbackUtilReceived == NULL || strcmp(callbackUtilReceived, "Hello world!") != 0) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected value '%s' found in VM reuse run 1\n" TOY_CC_RESET, callbackUtilReceived != NULL ? callbackUtilReceived : "NULL");

			//cleanup and return
			free(callbackUtilReceived);
			callbackUtilReceived = NULL;
			Toy_freeBytecode(bc1);
			Toy_freeVM(&vm);
			Toy_resetPrintCallback();
			return -1;
		}
		Toy_freeBytecode(bc1);

		//run 2
		Toy_Bytecode bc2 = makeBytecodeFromSource(bucketHandle, "print \"Hello world!\";");
		Toy_bindVM(&vm, &bc2);
		Toy_runVM(&vm);
		Toy_resetVM(&vm);

		if (callbackUtilReceived == NULL || strcmp(callbackUtilReceived, "Hello world!") != 0) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected value '%s' found in VM reuse run 2\n" TOY_CC_RESET, callbackUtilReceived != NULL ? callbackUtilReceived : "NULL");

			//cleanup and return
			free(callbackUtilReceived);
			callbackUtilReceived = NULL;
			Toy_freeBytecode(bc2);
			Toy_freeVM(&vm);
			Toy_resetPrintCallback();
			return -1;
		}
		Toy_freeBytecode(bc2);

		//run 3
		Toy_Bytecode bc3 = makeBytecodeFromSource(bucketHandle, "print \"Hello world!\";");
		Toy_bindVM(&vm, &bc3);
		Toy_runVM(&vm);
		Toy_resetVM(&vm);

		if (callbackUtilReceived == NULL || strcmp(callbackUtilReceived, "Hello world!") != 0) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected value '%s' found in VM reuse run 3\n" TOY_CC_RESET, callbackUtilReceived != NULL ? callbackUtilReceived : "NULL");

			//cleanup and return
			free(callbackUtilReceived);
			callbackUtilReceived = NULL;
			Toy_freeBytecode(bc3);
			Toy_freeVM(&vm);
			Toy_resetPrintCallback();
			return -1;
		}
		Toy_freeBytecode(bc3);

		//cleanup
		Toy_freeVM(&vm);
		free(callbackUtilReceived);
		callbackUtilReceived = NULL;
		Toy_resetPrintCallback();
	}

	return 0;
}

int main() {
	//run each test set, returning the total errors given
	int total = 0, res = 0;

	{
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);
		res = test_setup_and_teardown(&bucket);
		Toy_freeBucket(&bucket);
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);
		res = test_simple_execution(&bucket);
		Toy_freeBucket(&bucket);
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);
		res = test_opcode_not_equal(&bucket);
		Toy_freeBucket(&bucket);
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);
		res = test_keyword_print(&bucket);
		Toy_freeBucket(&bucket);
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);
		res = test_keyword_assert(&bucket);
		Toy_freeBucket(&bucket);
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	//TODO: test_ifThenElse()

	{
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);
		res = test_scope(&bucket);
		Toy_freeBucket(&bucket);
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);
		res = test_vm_reuse(&bucket);
		Toy_freeBucket(&bucket);
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	return total;
}
