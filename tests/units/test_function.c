#include "toy_vm.h"
#include "toy_console_colors.h"

#include "toy_lexer.h"
#include "toy_parser.h"
#include "toy_compiler.h"
#include "toy_print.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//define a function in one bytecode, invoke it in another
const char* sourceAlpha = "\n\
//using the classic closure approach\n\
fn makeCounter() {\n\
	var counter: Int = 0;\n\
\
	fn increment() {\n\
		return ++counter;\n\
	}\n\
\n\
	return increment;\n\
}\n\
";

const char* sourceBeta = "\n\
var tally = makeCounter();\n\
\n\
while (true) {\n\
	var result = tally();\n\
\n\
	print result;\n\
\n\
	if (result >= 10) {\n\
		return result;\n\
	}\n\
}\n\
";

//utils
unsigned char* makeCodeFromSource(Toy_Bucket** bucketHandle, const char* source) {
	Toy_Lexer lexer;
	Toy_bindLexer(&lexer, source);

	Toy_Parser parser;
	Toy_bindParser(&parser, &lexer);

	Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);
	return Toy_compileToBytecode(ast);
}

//tests
int test_functions_from_bytecodes(void) {
	//do the thing
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);
		Toy_String* makeCounterString = Toy_createStringLength(&bucket, "makeCounter", 11);

		Toy_VM vm;
		Toy_initVM(&vm);

		//run alpha
		unsigned char* alpha = makeCodeFromSource(&bucket, sourceAlpha);
		Toy_bindVM(&vm, alpha, NULL);
		Toy_runVM(&vm);

		//check for the function was declared
		if (Toy_isDeclaredScope(vm.scope, makeCounterString) != true) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to create the function '%s' from source\n" TOY_CC_RESET, makeCounterString->leaf.data);
			Toy_freeString(makeCounterString);
			Toy_freeVM(&vm);
			Toy_freeBucket(&bucket);
			free(alpha);
			return -1;
		}

		//get the function and clean up
		Toy_Value fnValue = Toy_copyValue(&bucket, *Toy_accessScopeAsPointer(vm.scope, makeCounterString));
		Toy_resetVM(&vm, false, true);

		//run beta
		unsigned char* beta = makeCodeFromSource(&bucket, sourceBeta);
		Toy_bindVM(&vm, beta, NULL);
		Toy_declareScope(vm.scope, makeCounterString, TOY_VALUE_ANY, fnValue, true);
		Toy_runVM(&vm);

		//examine the results
		if (vm.stack->count != 1 ||
			TOY_VALUE_IS_INTEGER(vm.stack->data[0]) != true ||
			TOY_VALUE_AS_INTEGER(vm.stack->data[0]) != 10
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected result found in function results\n" TOY_CC_RESET);
			Toy_freeString(makeCounterString);
			Toy_freeVM(&vm);
			Toy_freeBucket(&bucket);
			free(alpha);
			free(beta);
			return -1;
		}

		Toy_resetVM(&vm, false, true);

		//cleanup
		Toy_freeValue(fnValue);
		Toy_freeString(makeCounterString);
		Toy_freeVM(&vm);
		Toy_freeBucket(&bucket);
		free(alpha);
		free(beta);
	}

	//all good
	return 0;
}

int test_functions_from_callbacks(void) {
	//WARN: Test not yet implemented
	printf(TOY_CC_WARN "WIP test not yet implemented: %s\n" TOY_CC_RESET, __FILE__);
	return 0;
}

int main(void) {
	//run each test set, returning the total errors given
	int total = 0, res = 0;

	{
		res = test_functions_from_bytecodes();
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		res = test_functions_from_callbacks();
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	return total;
}
