#include "ast_inspector.h"
#include "toy_console_colors.h"
#include "toy_bucket.h"
#include "toy_string.h"
#include "toy_value.h"

#include <stdio.h>
#include <stdlib.h>

void inspect_by_type(Toy_Ast* ast, int depth);

void inspect_block(Toy_Ast* ast, int depth);
void inspect_value(Toy_Ast* ast, int depth);
void inspect_print(Toy_Ast* ast, int depth);

#define PRINTSTR(x) printf("%*s%s", depth*4, "", x);

static Toy_Bucket* bucket = NULL; //lazy

void inspect_ast(Toy_Ast* ast) {
	bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);
	inspect_by_type(ast, 0);
	Toy_freeBucket(&bucket);
}

void inspect_by_type(Toy_Ast* ast, int depth) {
	switch(ast->type) {
		case TOY_AST_BLOCK:
			inspect_block(ast, depth);
			return;

		case TOY_AST_VALUE:
			inspect_value(ast, depth);
			return;
		// case TOY_AST_UNARY:
		// case TOY_AST_BINARY:
		// case TOY_AST_BINARY_SHORT_CIRCUIT:
		// case TOY_AST_COMPARE:
		// case TOY_AST_GROUP:
		// case TOY_AST_COMPOUND:
		// case TOY_AST_AGGREGATE:

		// case TOY_AST_ASSERT:
		// case TOY_AST_IF_THEN_ELSE:
		// case TOY_AST_WHILE_THEN:
		// case TOY_AST_BREAK:
		// case TOY_AST_CONTINUE:
		// case TOY_AST_RETURN:
		case TOY_AST_PRINT:
			inspect_print(ast, depth);
			return;

		// case TOY_AST_VAR_DECLARE:
		// case TOY_AST_VAR_ASSIGN:
		// case TOY_AST_VAR_ACCESS:

		// case TOY_AST_FN_DECLARE:
		// case TOY_AST_FN_INVOKE:

		// case TOY_AST_STACK_POP:

		default:
			printf(TOY_CC_WARN "%*sAST %s (unhandled by inspector)" TOY_CC_RESET "\n", depth*4, "", Toy_private_getAstTypeAsCString(ast->type));
	}
}

void inspect_block(Toy_Ast* ast, int depth) {
	//show the block braces
	PRINTSTR("{\n");

	if (ast->block.child) {
		inspect_by_type(ast->block.child, depth + 1);

		if (ast->block.next) {
			inspect_block(ast->block.next, depth + 0);
		}
	}

	PRINTSTR("}\n");
}

void inspect_value(Toy_Ast* ast, int depth) {
	(void)depth;
	Toy_String* str = Toy_stringifyValue(&bucket, ast->value.value);

	char* buffer = Toy_getStringRaw(str); //SLOW
	printf("%s '%s'", Toy_getValueTypeAsCString(ast->value.value.type), buffer);
	free(buffer);

	Toy_freeString(str);
}

void inspect_print(Toy_Ast* ast, int depth) {
	(void)depth;
	PRINTSTR("PRINT ");

	inspect_by_type(ast->print.child, depth);

	printf(";\n");
}