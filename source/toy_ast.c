#include "toy_ast.h"

void Toy_private_initAstBlock(Toy_Bucket** bucketHandle, Toy_Ast** astHandle) {
	Toy_Ast* tmp = (Toy_Ast*)Toy_partitionBucket(bucketHandle, sizeof(Toy_Ast));

	tmp->type = TOY_AST_BLOCK;
	tmp->block.innerScope = false;
	tmp->block.child = NULL;
	tmp->block.next = NULL;
	tmp->block.tail = NULL;

	(*astHandle) = tmp;
}

void Toy_private_appendAstBlock(Toy_Bucket** bucketHandle, Toy_Ast* block, Toy_Ast* child) {
	//first, check if we're an empty head
	if (block->block.child == NULL) {
		block->block.child = child;
		return; //NOTE: first call on an empty head skips any memory allocations
	}

	//run (or jump) until we hit the current tail
	Toy_Ast* iter = block->block.tail ? block->block.tail : block;

	while(iter->block.next != NULL) {
		iter = iter->block.next;
	}

	//append a new link to the chain
	Toy_private_initAstBlock(bucketHandle, &(iter->block.next));

	//store the child in the new link, prep the tail pointer
	iter->block.next->block.child = child;
	block->block.tail = iter->block.next;
}

void Toy_private_emitAstValue(Toy_Bucket** bucketHandle, Toy_Ast** astHandle, Toy_Value value) {
	Toy_Ast* tmp = (Toy_Ast*)Toy_partitionBucket(bucketHandle, sizeof(Toy_Ast));

	tmp->type = TOY_AST_VALUE;
	tmp->value.value = value;

	(*astHandle) = tmp;
}

void Toy_private_emitAstUnary(Toy_Bucket** bucketHandle, Toy_Ast** astHandle, Toy_AstFlag flag) {
	Toy_Ast* tmp = (Toy_Ast*)Toy_partitionBucket(bucketHandle, sizeof(Toy_Ast));

	tmp->type = TOY_AST_UNARY;
	tmp->unary.flag = flag;
	tmp->unary.child = *astHandle;

	(*astHandle) = tmp;
}

void Toy_private_emitAstBinary(Toy_Bucket** bucketHandle, Toy_Ast** astHandle, Toy_AstFlag flag, Toy_Ast* right) {
	Toy_Ast* tmp = (Toy_Ast*)Toy_partitionBucket(bucketHandle, sizeof(Toy_Ast));

	tmp->type = TOY_AST_BINARY;
	tmp->binary.flag = flag;
	tmp->binary.left = *astHandle; //left-recursive
	tmp->binary.right = right;

	(*astHandle) = tmp;
}

void Toy_private_emitAstCompare(Toy_Bucket** bucketHandle, Toy_Ast** astHandle, Toy_AstFlag flag, Toy_Ast* right) {
	Toy_Ast* tmp = (Toy_Ast*)Toy_partitionBucket(bucketHandle, sizeof(Toy_Ast));

	tmp->type = TOY_AST_COMPARE;
	tmp->compare.flag = flag;
	tmp->compare.left = *astHandle; //left-recursive
	tmp->compare.right = right;

	(*astHandle) = tmp;
}

void Toy_private_emitAstGroup(Toy_Bucket** bucketHandle, Toy_Ast** astHandle) {
	Toy_Ast* tmp = (Toy_Ast*)Toy_partitionBucket(bucketHandle, sizeof(Toy_Ast));

	tmp->type = TOY_AST_GROUP;
	tmp->group.child = (*astHandle);

	(*astHandle) = tmp;
}

void Toy_private_emitAstCompound(Toy_Bucket** bucketHandle, Toy_Ast** astHandle, Toy_AstFlag flag, Toy_Ast* right) {
	Toy_Ast* tmp = (Toy_Ast*)Toy_partitionBucket(bucketHandle, sizeof(Toy_Ast));

	tmp->type = TOY_AST_COMPOUND;
	tmp->compound.flag = flag;
	tmp->compound.left = *astHandle; //left-recursive
	tmp->compound.right = right;

	(*astHandle) = tmp;
}

void Toy_private_emitAstAssert(Toy_Bucket** bucketHandle, Toy_Ast** astHandle, Toy_Ast* child, Toy_Ast* msg) {
	Toy_Ast* tmp = (Toy_Ast*)Toy_partitionBucket(bucketHandle, sizeof(Toy_Ast));

	tmp->type = TOY_AST_ASSERT;
	tmp->assert.child = child;
	tmp->assert.message = msg;

	(*astHandle) = tmp;
}

void Toy_private_emitAstIfThenElse(Toy_Bucket** bucketHandle, Toy_Ast** astHandle, Toy_Ast* condBranch, Toy_Ast* thenBranch, Toy_Ast* elseBranch) {
	Toy_Ast* tmp = (Toy_Ast*)Toy_partitionBucket(bucketHandle, sizeof(Toy_Ast));

	tmp->type = TOY_AST_IF_THEN_ELSE;
	tmp->ifThenElse.condBranch = condBranch;
	tmp->ifThenElse.thenBranch = thenBranch;
	tmp->ifThenElse.elseBranch = elseBranch;

	(*astHandle) = tmp;
}

void Toy_private_emitAstPrint(Toy_Bucket** bucketHandle, Toy_Ast** astHandle) {
	Toy_Ast* tmp = (Toy_Ast*)Toy_partitionBucket(bucketHandle, sizeof(Toy_Ast));

	tmp->type = TOY_AST_PRINT;
	tmp->print.child = (*astHandle);

	(*astHandle) = tmp;
}

void Toy_private_emitAstVariableDeclaration(Toy_Bucket** bucketHandle, Toy_Ast** astHandle, Toy_String* name, Toy_Ast* expr) {
	Toy_Ast* tmp = (Toy_Ast*)Toy_partitionBucket(bucketHandle, sizeof(Toy_Ast));

	tmp->type = TOY_AST_VAR_DECLARE;
	tmp->varDeclare.name = name;
	tmp->varDeclare.expr = expr;

	(*astHandle) = tmp;
}

void Toy_private_emitAstVariableAssignment(Toy_Bucket** bucketHandle, Toy_Ast** astHandle, Toy_String* name, Toy_AstFlag flag, Toy_Ast* expr) {
	Toy_Ast* tmp = (Toy_Ast*)Toy_partitionBucket(bucketHandle, sizeof(Toy_Ast));

	tmp->type = TOY_AST_VAR_ASSIGN;
	tmp->varAssign.flag = flag;
	tmp->varAssign.name = name;
	tmp->varAssign.expr = expr;

	(*astHandle) = tmp;
}

void Toy_private_emitAstVariableAccess(Toy_Bucket** bucketHandle, Toy_Ast** astHandle, Toy_String* name) {
	Toy_Ast* tmp = (Toy_Ast*)Toy_partitionBucket(bucketHandle, sizeof(Toy_Ast));

	tmp->type = TOY_AST_VAR_ACCESS;
	tmp->varAccess.name = name;

	(*astHandle) = tmp;
}

void Toy_private_emitAstPass(Toy_Bucket** bucketHandle, Toy_Ast** astHandle) {
	Toy_Ast* tmp = (Toy_Ast*)Toy_partitionBucket(bucketHandle, sizeof(Toy_Ast));

	tmp->type = TOY_AST_PASS;

	(*astHandle) = tmp;
}

void Toy_private_emitAstError(Toy_Bucket** bucketHandle, Toy_Ast** astHandle) {
	Toy_Ast* tmp = (Toy_Ast*)Toy_partitionBucket(bucketHandle, sizeof(Toy_Ast));

	tmp->type = TOY_AST_ERROR;

	(*astHandle) = tmp;
}

void Toy_private_emitAstEnd(Toy_Bucket** bucketHandle, Toy_Ast** astHandle) {
	Toy_Ast* tmp = (Toy_Ast*)Toy_partitionBucket(bucketHandle, sizeof(Toy_Ast));

	tmp->type = TOY_AST_END;

	(*astHandle) = tmp;
}
