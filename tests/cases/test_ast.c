#include "toy_ast.h"
#include "toy_console_colors.h"

#include <stdio.h>
#include <string.h>

int test_sizeof_ast_64bit() {
	//NOTE: This could've covered both bitness sizes as TEST_SIZEOF(type, bit32, bit32)
#define TEST_SIZEOF(type, size) \
	if (sizeof(type) != size) { \
		fprintf(stderr, TOY_CC_ERROR "ERROR: sizeof(" #type ") is %d, expected %d\n" TOY_CC_RESET, (int)sizeof(type), size); \
		++err; \
	}

	//count errors
	int err = 0;

	//run for each type
	TEST_SIZEOF(Toy_AstType, 4);
	TEST_SIZEOF(Toy_AstBlock, 32);
	TEST_SIZEOF(Toy_AstValue, 24);
	TEST_SIZEOF(Toy_AstUnary, 16);
	TEST_SIZEOF(Toy_AstBinary, 24);
	TEST_SIZEOF(Toy_AstCompare, 24);
	TEST_SIZEOF(Toy_AstGroup, 16);
	TEST_SIZEOF(Toy_AstCompound, 16);
	TEST_SIZEOF(Toy_AstAggregate, 24);
	TEST_SIZEOF(Toy_AstAssert, 24);
	TEST_SIZEOF(Toy_AstIfThenElse, 32);
	TEST_SIZEOF(Toy_AstWhileThen, 24);
	TEST_SIZEOF(Toy_AstBreak, 4);
	TEST_SIZEOF(Toy_AstContinue, 4);
	TEST_SIZEOF(Toy_AstPrint, 16);
	TEST_SIZEOF(Toy_AstVarDeclare, 24);
	TEST_SIZEOF(Toy_AstVarAssign, 24);
	TEST_SIZEOF(Toy_AstVarAccess, 16);
	TEST_SIZEOF(Toy_AstPass, 4);
	TEST_SIZEOF(Toy_AstError, 4);
	TEST_SIZEOF(Toy_AstEnd, 4);
	TEST_SIZEOF(Toy_Ast, 32);

#undef TEST_SIZEOF

	return -err;
}

int test_sizeof_ast_32bit() {
#define TEST_SIZEOF(type, size) \
	if (sizeof(type) != size) { \
		fprintf(stderr, TOY_CC_ERROR "ERROR: sizeof(" #type ") is %d, expected %d\n" TOY_CC_RESET, (int)sizeof(type), size); \
		++err; \
	}

	//count errors
	int err = 0;

	//run for each type
	TEST_SIZEOF(Toy_AstType, 4);
	TEST_SIZEOF(Toy_AstBlock, 20);
	TEST_SIZEOF(Toy_AstValue, 12);
	TEST_SIZEOF(Toy_AstUnary, 12);
	TEST_SIZEOF(Toy_AstBinary, 16);
	TEST_SIZEOF(Toy_AstCompare, 16);
	TEST_SIZEOF(Toy_AstGroup, 8);
	TEST_SIZEOF(Toy_AstCompound, 12);
	TEST_SIZEOF(Toy_AstAggregate, 16);
	TEST_SIZEOF(Toy_AstAssert, 12);
	TEST_SIZEOF(Toy_AstIfThenElse, 16);
	TEST_SIZEOF(Toy_AstWhileThen, 12);
	TEST_SIZEOF(Toy_AstBreak, 4);
	TEST_SIZEOF(Toy_AstContinue, 4);
	TEST_SIZEOF(Toy_AstPrint, 8);
	TEST_SIZEOF(Toy_AstVarDeclare, 12);
	TEST_SIZEOF(Toy_AstVarAssign, 16);
	TEST_SIZEOF(Toy_AstVarAccess, 8);
	TEST_SIZEOF(Toy_AstPass, 4);
	TEST_SIZEOF(Toy_AstError, 4);
	TEST_SIZEOF(Toy_AstEnd, 4);
	TEST_SIZEOF(Toy_Ast, 20);

#undef TEST_SIZEOF

	return -err;
}

int test_type_emission(Toy_Bucket** bucketHandle) {
	//emit value
	{
		//emit to an AST
		Toy_Ast* ast = NULL;
		Toy_private_emitAstValue(bucketHandle, &ast, TOY_VALUE_FROM_INTEGER(42));

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_VALUE ||
			TOY_VALUE_AS_INTEGER(ast->value.value) != 42)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to emit a value as 'Toy_Ast', state unknown\n" TOY_CC_RESET);
			return -1;
		}
	}

	//emit unary
	{
		//build the AST
		Toy_Ast* ast = NULL;
		Toy_private_emitAstValue(bucketHandle, &ast, TOY_VALUE_FROM_INTEGER(42));
		Toy_private_emitAstUnary(bucketHandle, &ast, TOY_AST_FLAG_NEGATE);

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_UNARY ||
			ast->unary.flag != TOY_AST_FLAG_NEGATE ||
			ast->unary.child->type != TOY_AST_VALUE ||
			TOY_VALUE_AS_INTEGER(ast->unary.child->value.value) != 42)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to emit a unary as 'Toy_Ast', state unknown\n" TOY_CC_RESET);
			return -1;
		}
	}

	//emit binary
	{
		//build the AST
		Toy_Ast* ast = NULL;
		Toy_Ast* right = NULL;
		Toy_private_emitAstValue(bucketHandle, &ast, TOY_VALUE_FROM_INTEGER(42));
		Toy_private_emitAstValue(bucketHandle, &right, TOY_VALUE_FROM_INTEGER(69));
		Toy_private_emitAstBinary(bucketHandle, &ast, TOY_AST_FLAG_ADD, right);

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_BINARY ||
			ast->binary.flag != TOY_AST_FLAG_ADD ||
			ast->binary.left->type != TOY_AST_VALUE ||
			TOY_VALUE_AS_INTEGER(ast->binary.left->value.value) != 42 ||
			ast->binary.right->type != TOY_AST_VALUE ||
			TOY_VALUE_AS_INTEGER(ast->binary.right->value.value) != 69)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to emit a binary as 'Toy_Ast', state unknown\n" TOY_CC_RESET);
			return -1;
		}
	}

	//emit compare
	{
		//build the AST
		Toy_Ast* ast = NULL;
		Toy_Ast* right = NULL;
		Toy_private_emitAstValue(bucketHandle, &ast, TOY_VALUE_FROM_INTEGER(42)); //technically, not a valid value
		Toy_private_emitAstValue(bucketHandle, &right, TOY_VALUE_FROM_INTEGER(69));
		Toy_private_emitAstCompare(bucketHandle, &ast, TOY_AST_FLAG_ADD, right);

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_COMPARE ||
			ast->compare.flag != TOY_AST_FLAG_ADD ||
			ast->compare.left->type != TOY_AST_VALUE ||
			TOY_VALUE_AS_INTEGER(ast->compare.left->value.value) != 42 ||
			ast->compare.right->type != TOY_AST_VALUE ||
			TOY_VALUE_AS_INTEGER(ast->compare.right->value.value) != 69)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to emit a compare as 'Toy_Ast', state unknown\n" TOY_CC_RESET);
			return -1;
		}
	}

	//emit group
	{
		//build the AST
		Toy_Ast* ast = NULL;
		Toy_Ast* right = NULL;
		Toy_private_emitAstValue(bucketHandle, &ast, TOY_VALUE_FROM_INTEGER(42));
		Toy_private_emitAstValue(bucketHandle, &right, TOY_VALUE_FROM_INTEGER(69));
		Toy_private_emitAstBinary(bucketHandle, &ast, TOY_AST_FLAG_ADD, right);
		Toy_private_emitAstGroup(bucketHandle, &ast);

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_GROUP ||
			ast->group.child == NULL ||
			ast->group.child->type != TOY_AST_BINARY ||
			ast->group.child->binary.flag != TOY_AST_FLAG_ADD ||
			ast->group.child->binary.left->type != TOY_AST_VALUE ||
			TOY_VALUE_AS_INTEGER(ast->group.child->binary.left->value.value) != 42 ||
			ast->group.child->binary.right->type != TOY_AST_VALUE ||
			TOY_VALUE_AS_INTEGER(ast->group.child->binary.right->value.value) != 69)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to emit a group as 'Toy_Ast', state unknown\n" TOY_CC_RESET);
			return -1;
		}
	}

	//emit aggregate
	{
		//build the AST
		Toy_Ast* ast = NULL;
		Toy_Ast* right = NULL;
		Toy_private_emitAstValue(bucketHandle, &ast, TOY_VALUE_FROM_INTEGER(42));
		Toy_private_emitAstValue(bucketHandle, &right, TOY_VALUE_FROM_INTEGER(69));
		Toy_private_emitAstAggregate(bucketHandle, &ast, TOY_AST_FLAG_COLLECTION, right);

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_AGGREGATE ||

			ast->aggregate.left == NULL ||
			ast->aggregate.left->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_INTEGER(ast->aggregate.left->value.value) != true ||
			TOY_VALUE_AS_INTEGER(ast->aggregate.left->value.value) != 42 ||

			ast->aggregate.right == NULL ||
			ast->aggregate.right->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_INTEGER(ast->aggregate.right->value.value) != true ||
			TOY_VALUE_AS_INTEGER(ast->aggregate.right->value.value) != 69 ||

			false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to emit an aggregate as 'Toy_Ast', state unknown\n" TOY_CC_RESET);
			return -1;
		}
	}

	//emit keyword assert
	{
		//build the AST
		Toy_Ast* ast = NULL;
		Toy_Ast* child = NULL;
		Toy_Ast* msg = NULL;

		Toy_private_emitAstValue(bucketHandle, &child, TOY_VALUE_FROM_INTEGER(42));
		Toy_private_emitAstValue(bucketHandle, &msg, TOY_VALUE_FROM_INTEGER(69));

		Toy_private_emitAstAssert(bucketHandle, &ast, child, msg);

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_ASSERT ||
			ast->assert.child == NULL ||
			ast->assert.child->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_INTEGER(ast->assert.child->value.value) != true ||
			TOY_VALUE_AS_INTEGER(ast->assert.child->value.value) != 42 ||

			ast->assert.message == NULL ||
			ast->assert.message->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_INTEGER(ast->assert.message->value.value) != true ||
			TOY_VALUE_AS_INTEGER(ast->assert.message->value.value) != 69 ||

			false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to emit a keyword 'assert' as 'Toy_Ast', state unknown\n" TOY_CC_RESET);
			return -1;
		}
	}

	//emit keyword if-then
	{
		//build the AST
		Toy_Ast* ast = NULL;
		Toy_Ast* condBranch = NULL;
		Toy_Ast* thenBranch = NULL;

		Toy_private_emitAstValue(bucketHandle, &condBranch, TOY_VALUE_FROM_INTEGER(42));
		Toy_private_emitAstValue(bucketHandle, &thenBranch, TOY_VALUE_FROM_INTEGER(69));

		Toy_private_emitAstIfThenElse(bucketHandle, &ast, condBranch, thenBranch, NULL);

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_IF_THEN_ELSE ||
			ast->ifThenElse.condBranch == NULL ||
			ast->ifThenElse.condBranch->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_INTEGER(ast->ifThenElse.condBranch->value.value) != true ||
			TOY_VALUE_AS_INTEGER(ast->ifThenElse.condBranch->value.value) != 42 ||

			ast->ifThenElse.thenBranch == NULL ||
			ast->ifThenElse.thenBranch->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_INTEGER(ast->ifThenElse.thenBranch->value.value) != true ||
			TOY_VALUE_AS_INTEGER(ast->ifThenElse.thenBranch->value.value) != 69 ||

			ast->ifThenElse.elseBranch != NULL ||

			false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to emit a keyword 'if-then' as 'Toy_Ast', state unknown\n" TOY_CC_RESET);
			return -1;
		}
	}

	//emit keyword if-then-else
	{
		//build the AST
		Toy_Ast* ast = NULL;
		Toy_Ast* condBranch = NULL;
		Toy_Ast* thenBranch = NULL;
		Toy_Ast* elseBranch = NULL;

		Toy_private_emitAstValue(bucketHandle, &condBranch, TOY_VALUE_FROM_INTEGER(42));
		Toy_private_emitAstValue(bucketHandle, &thenBranch, TOY_VALUE_FROM_INTEGER(69));
		Toy_private_emitAstValue(bucketHandle, &elseBranch, TOY_VALUE_FROM_INTEGER(420));

		Toy_private_emitAstIfThenElse(bucketHandle, &ast, condBranch, thenBranch, elseBranch);

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_IF_THEN_ELSE ||
			ast->ifThenElse.condBranch == NULL ||
			ast->ifThenElse.condBranch->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_INTEGER(ast->ifThenElse.condBranch->value.value) != true ||
			TOY_VALUE_AS_INTEGER(ast->ifThenElse.condBranch->value.value) != 42 ||

			ast->ifThenElse.thenBranch == NULL ||
			ast->ifThenElse.thenBranch->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_INTEGER(ast->ifThenElse.thenBranch->value.value) != true ||
			TOY_VALUE_AS_INTEGER(ast->ifThenElse.thenBranch->value.value) != 69 ||

			ast->ifThenElse.elseBranch == NULL ||
			ast->ifThenElse.elseBranch->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_INTEGER(ast->ifThenElse.elseBranch->value.value) != true ||
			TOY_VALUE_AS_INTEGER(ast->ifThenElse.elseBranch->value.value) != 420 ||

			false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to emit a keyword 'if-then-else' as 'Toy_Ast', state unknown\n" TOY_CC_RESET);
			return -1;
		}
	}

	//emit keyword print
	{
		//build the AST
		Toy_Ast* ast = NULL;
		Toy_Ast* right = NULL;
		Toy_private_emitAstValue(bucketHandle, &ast, TOY_VALUE_FROM_INTEGER(42));
		Toy_private_emitAstValue(bucketHandle, &right, TOY_VALUE_FROM_INTEGER(69));
		Toy_private_emitAstBinary(bucketHandle, &ast, TOY_AST_FLAG_ADD, right);
		Toy_private_emitAstPrint(bucketHandle, &ast);

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_PRINT ||
			ast->print.child == NULL ||
			ast->print.child->type != TOY_AST_BINARY ||
			ast->print.child->binary.flag != TOY_AST_FLAG_ADD ||
			ast->print.child->binary.left->type != TOY_AST_VALUE ||
			TOY_VALUE_AS_INTEGER(ast->print.child->binary.left->value.value) != 42 ||
			ast->print.child->binary.right->type != TOY_AST_VALUE ||
			TOY_VALUE_AS_INTEGER(ast->print.child->binary.right->value.value) != 69)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to emit a keyword 'print' as 'Toy_Ast', state unknown\n" TOY_CC_RESET);
			return -1;
		}
	}

	//emit var declare
	{
		//build the AST
		Toy_Ast* ast = NULL;
		Toy_String* name = Toy_createNameStringLength(bucketHandle, "foobar", 6, TOY_VALUE_ANY, false);

		Toy_private_emitAstVariableDeclaration(bucketHandle, &ast, name, NULL);

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_VAR_DECLARE ||

			ast->varDeclare.name == NULL ||
			ast->varDeclare.name->type != TOY_STRING_NAME ||
			strcmp(ast->varDeclare.name->as.name.data, "foobar") != 0 ||

			ast->varDeclare.expr != NULL)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to emit a var declare as 'Toy_Ast', state unknown\n" TOY_CC_RESET);
			Toy_freeString(name);
			return -1;
		}

		//cleanup
		Toy_freeString(name);
	}

	//emit assign
	{
		//build the AST
		Toy_Ast* ast = NULL;
		Toy_Ast* right = NULL;
		Toy_String* name = Toy_createNameStringLength(bucketHandle, "foobar", 6, TOY_VALUE_INTEGER, false);
		Toy_private_emitAstValue(bucketHandle, &right, TOY_VALUE_FROM_INTEGER(69));
		Toy_private_emitAstVariableAssignment(bucketHandle, &ast, name, TOY_AST_FLAG_ASSIGN, right);

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_VAR_ASSIGN ||
			ast->varAssign.flag != TOY_AST_FLAG_ASSIGN ||
			ast->varAssign.name == NULL ||
			ast->varAssign.name->type != TOY_STRING_NAME ||
			strcmp(ast->varAssign.name->as.name.data, "foobar") != 0 ||
			ast->varAssign.name->as.name.type != TOY_VALUE_INTEGER ||
			ast->varAssign.expr->type != TOY_AST_VALUE ||
			TOY_VALUE_AS_INTEGER(ast->varAssign.expr->value.value) != 69)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to emit an assign as 'Toy_Ast', state unknown\n" TOY_CC_RESET);
			return -1;
		}
	}

	//emit access
	{
		//build the AST
		Toy_Ast* ast = NULL;
		Toy_Ast* right = NULL;
		Toy_String* name = Toy_createNameStringLength(bucketHandle, "foobar", 6, TOY_VALUE_INTEGER, false);
		Toy_private_emitAstVariableAccess(bucketHandle, &ast, name);

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_VAR_ACCESS ||
			ast->varAccess.name == NULL ||
			ast->varAccess.name->type != TOY_STRING_NAME ||
			strcmp(ast->varAccess.name->as.name.data, "foobar") != 0 ||
			ast->varAccess.name->as.name.type != TOY_VALUE_INTEGER)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to emit an access as 'Toy_Ast', state unknown\n" TOY_CC_RESET);
			return -1;
		}
	}

	//emit and append blocks of code (at the bottom of this test function, so everything else is checked first)
	{
		//initialize the root block
		Toy_Ast* block = NULL;
		Toy_private_initAstBlock(bucketHandle, &block);

		//loop over the ast emissions, appending each one as you go
		for (int i = 0; i < 5; i++) {
			//build the AST
			Toy_Ast* ast = NULL;
			Toy_Ast* right = NULL;
			Toy_private_emitAstValue(bucketHandle, &ast, TOY_VALUE_FROM_INTEGER(42));
			Toy_private_emitAstValue(bucketHandle, &right, TOY_VALUE_FROM_INTEGER(69));
			Toy_private_emitAstBinary(bucketHandle, &ast, TOY_AST_FLAG_ADD, right);
			Toy_private_emitAstGroup(bucketHandle, &ast);

			Toy_private_appendAstBlock(bucketHandle, block, ast);
		}

		//check if it worked
		Toy_Ast* iter = block;

		while(iter != NULL) {
			if (
				iter->type != TOY_AST_BLOCK ||
				iter->block.child == NULL ||
				iter->block.child->type != TOY_AST_GROUP ||
				iter->block.child->group.child == NULL ||
				iter->block.child->group.child->type != TOY_AST_BINARY ||
				iter->block.child->group.child->binary.flag != TOY_AST_FLAG_ADD ||
				iter->block.child->group.child->binary.left->type != TOY_AST_VALUE ||
				TOY_VALUE_AS_INTEGER(iter->block.child->group.child->binary.left->value.value) != 42 ||
				iter->block.child->group.child->binary.right->type != TOY_AST_VALUE ||
				TOY_VALUE_AS_INTEGER(iter->block.child->group.child->binary.right->value.value) != 69)
			{
				fprintf(stderr, TOY_CC_ERROR "ERROR: failed to emit a block as 'Toy_Ast', state unknown\n" TOY_CC_RESET);
				return -1;
			}

			iter = iter->block.next;
		}
	}

	return 0;
}

int main() {
	//run each test set, returning the total errors given
	int total = 0, res = 0;

	{
#if TOY_BITNESS == 64
		res = test_sizeof_ast_64bit();
#elif TOY_BITNESS == 32
		res = test_sizeof_ast_32bit();
#else
		res = -1;
		fprintf(stderr, TOY_CC_WARN "WARNING: Skipping test_sizeof_ast_**bit(); Can't determine the 'bitness' of this platform (seems to be %d)\n" TOY_CC_RESET, TOY_BITNESS);
#endif

		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		else if (res > 0) {
			total += res;
		}
	}

	{
		Toy_Bucket* bucketHandle = Toy_allocateBucket(TOY_BUCKET_IDEAL);
		res = test_type_emission(&bucketHandle);
		Toy_freeBucket(&bucketHandle);
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	return total;
}
