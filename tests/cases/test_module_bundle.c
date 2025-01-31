#include "toy_module_bundle.h"
#include "toy_console_colors.h"

#include "toy_opcodes.h"
#include "toy_lexer.h"
#include "toy_parser.h"

#include <stdio.h>
#include <string.h>

//tests
int test_bundle_header(Toy_Bucket** bucketHandle) {
	//simple test to ensure the header looks right
	{
		//setup
		Toy_Ast* ast = NULL;
		Toy_private_emitAstPass(bucketHandle, &ast);

		//run
		Toy_ModuleBundle bundle;
		Toy_initModuleBundle(&bundle);
		Toy_appendModuleBundle(&bundle, ast);

		if (bundle.count % 4 != 0) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: module bundle size is not a multiple of 4, size is: %d\n" TOY_CC_RESET, (int)bundle.count);

			//cleanup and return
			Toy_freeModuleBundle(&bundle);
			return -1;
		}

		//check
		if (bundle.ptr[0] != TOY_VERSION_MAJOR ||
			bundle.ptr[1] != TOY_VERSION_MINOR ||
			bundle.ptr[2] != TOY_VERSION_PATCH ||
			bundle.ptr[3] != 1 || //only one module
			strcmp((char*)(bundle.ptr + 4), TOY_VERSION_BUILD) != 0)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to write the module bundle header correctly:\n" TOY_CC_RESET);
			fprintf(stderr, TOY_CC_ERROR "\t%d.%d.%d.%s, %d modules found\n" TOY_CC_RESET, (int)(bundle.ptr[0]), (int)(bundle.ptr[1]), (int)(bundle.ptr[2]), (char*)(bundle.ptr + 4), (int)(bundle.ptr[2]));
			fprintf(stderr, TOY_CC_ERROR "\t%d.%d.%d.%s, %d modules expected\n" TOY_CC_RESET, TOY_VERSION_MAJOR, TOY_VERSION_MINOR, TOY_VERSION_PATCH, TOY_VERSION_BUILD, 1);

			//cleanup and return
			Toy_freeModuleBundle(&bundle);
			return -1;
		}

		//cleanup
		Toy_freeModuleBundle(&bundle);
	}

	return 0;
}

int test_bundle_from_source(Toy_Bucket** bucketHandle) {
	{
		//setup
		const char* source = "(1 + 2) * (3 + 4);";
		Toy_Lexer lexer;
		Toy_Parser parser;

		Toy_bindLexer(&lexer, source);
		Toy_bindParser(&parser, &lexer);
		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		//run
		Toy_ModuleBundle bundle;
		Toy_initModuleBundle(&bundle);
		Toy_appendModuleBundle(&bundle, ast);

		//check bytecode alignment
		if (bundle.count % 4 != 0) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: module bundle size is not a multiple of 4 (size is %d), source: %s\n" TOY_CC_RESET, (int)bundle.count, source);

			//cleanup and return
			Toy_freeModuleBundle(&bundle);
			return -1;
		}

		//check bytecode header
		//check
		if (bundle.ptr[0] != TOY_VERSION_MAJOR ||
			bundle.ptr[1] != TOY_VERSION_MINOR ||
			bundle.ptr[2] != TOY_VERSION_PATCH ||
			bundle.ptr[3] != 1 || //only one module
			strcmp((char*)(bundle.ptr + 4), TOY_VERSION_BUILD) != 0)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to write the module bundle header, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			Toy_freeModuleBundle(&bundle);
			return -1;
		}

		//cleanup
		Toy_freeModuleBundle(&bundle);
	}

	return 0;
}

int main(void) {
	//run each test set, returning the total errors given
	int total = 0, res = 0;

	{
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);
		res = test_bundle_header(&bucket);
		Toy_freeBucket(&bucket);
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);
		res = test_bundle_from_source(&bucket);
		Toy_freeBucket(&bucket);
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	return total;
}
