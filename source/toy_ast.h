#pragma once

#include "toy_common.h"

#include "toy_bucket.h"
#include "toy_value.h"
#include "toy_string.h"

//each major type
typedef enum Toy_AstType {
	TOY_AST_BLOCK,

	TOY_AST_VALUE,
	TOY_AST_UNARY,
	TOY_AST_BINARY,
	TOY_AST_BINARY_SHORT_CIRCUIT,
	TOY_AST_COMPARE,
	TOY_AST_GROUP,
	TOY_AST_COMPOUND,
	TOY_AST_AGGREGATE,

	TOY_AST_ASSERT,
	TOY_AST_IF_THEN_ELSE,
	TOY_AST_WHILE_THEN,
	TOY_AST_FOR_THEN,
	TOY_AST_BREAK,
	TOY_AST_CONTINUE,
	TOY_AST_RETURN,
	TOY_AST_PRINT,

	TOY_AST_VAR_DECLARE,
	TOY_AST_VAR_ASSIGN,
	TOY_AST_VAR_ACCESS,

	TOY_AST_FN_DECLARE,
	TOY_AST_FN_INVOKE,
	TOY_AST_ATTRIBUTE,
	TOY_AST_ITERABLE,

	TOY_AST_STACK_POP, //BUGFIX: force a single stack pop for expression statements

	TOY_AST_PASS,
	TOY_AST_ERROR,
	TOY_AST_END,
} Toy_AstType;

//flags are handled differently by different types
typedef enum Toy_AstFlag {
	TOY_AST_FLAG_NONE = 0,

	//binary flags
	TOY_AST_FLAG_ADD = 1,
	TOY_AST_FLAG_SUBTRACT = 2,
	TOY_AST_FLAG_MULTIPLY = 3,
	TOY_AST_FLAG_DIVIDE = 4,
	TOY_AST_FLAG_MODULO = 5,

	TOY_AST_FLAG_AND = 6,
	TOY_AST_FLAG_OR = 7,
	TOY_AST_FLAG_CONCAT = 8,

	TOY_AST_FLAG_ASSIGN = 10,
	TOY_AST_FLAG_ADD_ASSIGN = 11,
	TOY_AST_FLAG_SUBTRACT_ASSIGN = 12,
	TOY_AST_FLAG_MULTIPLY_ASSIGN = 13,
	TOY_AST_FLAG_DIVIDE_ASSIGN = 14,
	TOY_AST_FLAG_MODULO_ASSIGN = 15,

	TOY_AST_FLAG_COMPARE_EQUAL = 20,
	TOY_AST_FLAG_COMPARE_NOT = 21,
	TOY_AST_FLAG_COMPARE_LESS = 22,
	TOY_AST_FLAG_COMPARE_LESS_EQUAL = 23,
	TOY_AST_FLAG_COMPARE_GREATER = 24,
	TOY_AST_FLAG_COMPARE_GREATER_EQUAL = 25,

	TOY_AST_FLAG_COMPOUND_ARRAY = 30,
	TOY_AST_FLAG_COMPOUND_TABLE = 31,
	TOY_AST_FLAG_COLLECTION = 32,
	TOY_AST_FLAG_PAIR = 33,
	TOY_AST_FLAG_INDEX = 34,
	TOY_AST_FLAG_FN_ARGUMENTS = 35,

	//unary flags
	TOY_AST_FLAG_NEGATE = 40,
	TOY_AST_FLAG_PREFIX_INCREMENT = 41,
	TOY_AST_FLAG_PREFIX_DECREMENT = 42,
	TOY_AST_FLAG_POSTFIX_INCREMENT = 43,
	TOY_AST_FLAG_POSTFIX_DECREMENT = 44,

	TOY_AST_FLAG_INVOKATION = 45,
	TOY_AST_FLAG_ATTRIBUTE = 46,

	// TOY_AST_FLAG_TERNARY,
} Toy_AstFlag;

//the root AST type
typedef union Toy_Ast Toy_Ast;

typedef struct Toy_AstBlock {
	Toy_AstType type;
	bool innerScope;
	Toy_Ast* child; //begin encoding the line
	Toy_Ast* next; //'next' is either an AstBlock or null
	Toy_Ast* tail; //'tail' - either points to the tail of the current list, or null; only used as an optimisation in toy_ast.c
} Toy_AstBlock;

typedef struct Toy_AstValue {
	Toy_AstType type;
	Toy_Value value;
} Toy_AstValue;

typedef struct Toy_AstUnary {
	Toy_AstType type;
	Toy_AstFlag flag;
	Toy_Ast* child;
} Toy_AstUnary;

typedef struct Toy_AstBinary {
	Toy_AstType type;
	Toy_AstFlag flag;
	Toy_Ast* left;
	Toy_Ast* right;
} Toy_AstBinary;

typedef struct Toy_AstBinaryShortCircuit {
	Toy_AstType type;
	Toy_AstFlag flag;
	Toy_Ast* left;
	Toy_Ast* right;
} Toy_AstBinaryShortCircuit;

typedef struct Toy_AstCompare {
	Toy_AstType type;
	Toy_AstFlag flag;
	Toy_Ast* left;
	Toy_Ast* right;
} Toy_AstCompare;

typedef struct Toy_AstGroup {
	Toy_AstType type;
	Toy_Ast* child;
} Toy_AstGroup;

typedef struct Toy_AstCompound {
	Toy_AstType type;
	Toy_AstFlag flag;
	Toy_Ast* child;
} Toy_AstCompound;

typedef struct Toy_AstAggregate {
	Toy_AstType type;
	Toy_AstFlag flag;
	Toy_Ast* left;
	Toy_Ast* right;
} Toy_AstAggregate;

typedef struct Toy_AstAssert {
	Toy_AstType type;
	Toy_Ast* child;
	Toy_Ast* message;
} Toy_AstAssert;

typedef struct Toy_AstIfThenElse {
	Toy_AstType type;
	Toy_Ast* condBranch;
	Toy_Ast* thenBranch;
	Toy_Ast* elseBranch;
} Toy_AstIfThenElse;

typedef struct Toy_AstWhileThen {
	Toy_AstType type;
	Toy_Ast* condBranch;
	Toy_Ast* thenBranch;
} Toy_AstWhileThen;

typedef struct Toy_AstForThen {
	Toy_AstType type;
	Toy_Ast* condBranch;
	Toy_Ast* thenBranch;
} Toy_AstForThen;

typedef struct Toy_AstBreak {
	Toy_AstType type;
} Toy_AstBreak;

typedef struct Toy_AstContinue {
	Toy_AstType type;
} Toy_AstContinue;

typedef struct Toy_AstReturn {
	Toy_AstType type;
	Toy_Ast* child;
} Toy_AstReturn;

typedef struct Toy_AstPrint {
	Toy_AstType type;
	Toy_Ast* child;
} Toy_AstPrint;

typedef struct Toy_AstVarDeclare {
	Toy_AstType type;
	Toy_String* name;
	Toy_Ast* expr;
	Toy_ValueType valueType;
	bool constant;
} Toy_AstVarDeclare;

typedef struct Toy_AstVarAssign {
	Toy_AstType type;
	Toy_AstFlag flag;
	Toy_Ast* target;
	Toy_Ast* expr;
} Toy_AstVarAssign;

typedef struct Toy_AstVarAccess {
	Toy_AstType type;
	Toy_Ast* child;
} Toy_AstVarAccess;

typedef struct Toy_AstFnDeclare {
	Toy_AstType type;
	Toy_String* name;
	Toy_Ast* params;
	Toy_Ast* body;
} Toy_AstFnDeclare;

typedef struct Toy_AstFnInvoke {
	Toy_AstType type;
	Toy_Ast* function;
	Toy_Ast* args;
} Toy_AstFnInvoke;

typedef struct Toy_AstAttribute {
	Toy_AstType type;
	Toy_Ast* left;
	Toy_Ast* right;
} Toy_AstAttribute;

typedef struct Toy_AstIterable {
	Toy_AstType type;
	Toy_Ast* left;
	Toy_Ast* right;
} Toy_AstIterable;

typedef struct Toy_AstStackPop {
	Toy_AstType type;
	Toy_Ast* child;
} Toy_AstStackPop;

typedef struct Toy_AstPass {
	Toy_AstType type;
} Toy_AstPass;

typedef struct Toy_AstError {
	Toy_AstType type;
} Toy_AstError;

typedef struct Toy_AstEnd {
	Toy_AstType type;
} Toy_AstEnd;

union Toy_Ast { //see 'test_ast.c' for bitness tests
	Toy_AstType type;
	Toy_AstBlock block;
	Toy_AstValue value;
	Toy_AstUnary unary;
	Toy_AstBinary binary;
	Toy_AstBinaryShortCircuit binaryShortCircuit;
	Toy_AstCompare compare;
	Toy_AstGroup group;
	Toy_AstCompound compound;
	Toy_AstAggregate aggregate;
	Toy_AstAssert assert;
	Toy_AstIfThenElse ifThenElse;
	Toy_AstWhileThen whileThen;
	Toy_AstForThen forThen;
	Toy_AstBreak breakPoint;
	Toy_AstContinue continuePoint;
	Toy_AstReturn fnReturn;
	Toy_AstPrint print;
	Toy_AstVarDeclare varDeclare;
	Toy_AstVarAssign varAssign;
	Toy_AstVarAccess varAccess;
	Toy_AstFnDeclare fnDeclare;
	Toy_AstFnInvoke fnInvoke;
	Toy_AstAttribute attribute;
	Toy_AstIterable iterable;
	Toy_AstStackPop stackPop;
	Toy_AstPass pass;
	Toy_AstError error;
	Toy_AstEnd end;
};

void Toy_private_initAstBlock(Toy_Bucket** bucketHandle, Toy_Ast** astHandle);
void Toy_private_appendAstBlock(Toy_Bucket** bucketHandle, Toy_Ast* block, Toy_Ast* child);

void Toy_private_emitAstValue(Toy_Bucket** bucketHandle, Toy_Ast** astHandle, Toy_Value value);
void Toy_private_emitAstUnary(Toy_Bucket** bucketHandle, Toy_Ast** astHandle, Toy_AstFlag flag);
void Toy_private_emitAstBinary(Toy_Bucket** bucketHandle, Toy_Ast** astHandle,Toy_AstFlag flag, Toy_Ast* right);
void Toy_private_emitAstBinaryShortCircuit(Toy_Bucket** bucketHandle, Toy_Ast** astHandle,Toy_AstFlag flag, Toy_Ast* right);
void Toy_private_emitAstCompare(Toy_Bucket** bucketHandle, Toy_Ast** astHandle,Toy_AstFlag flag, Toy_Ast* right);
void Toy_private_emitAstGroup(Toy_Bucket** bucketHandle, Toy_Ast** astHandle);
void Toy_private_emitAstCompound(Toy_Bucket** bucketHandle, Toy_Ast** astHandle,Toy_AstFlag flag);
void Toy_private_emitAstAggregate(Toy_Bucket** bucketHandle, Toy_Ast** astHandle,Toy_AstFlag flag, Toy_Ast* right);

void Toy_private_emitAstAssert(Toy_Bucket** bucketHandle, Toy_Ast** astHandle, Toy_Ast* child, Toy_Ast* msg);
void Toy_private_emitAstIfThenElse(Toy_Bucket** bucketHandle, Toy_Ast** astHandle, Toy_Ast* condBranch, Toy_Ast* thenBranch, Toy_Ast* elseBranch);
void Toy_private_emitAstWhileThen(Toy_Bucket** bucketHandle, Toy_Ast** astHandle, Toy_Ast* condBranch, Toy_Ast* thenBranch);
void Toy_private_emitAstForThen(Toy_Bucket** bucketHandle, Toy_Ast** astHandle, Toy_Ast* condBranch, Toy_Ast* thenBranch);
void Toy_private_emitAstBreak(Toy_Bucket** bucketHandle, Toy_Ast** rootHandle);
void Toy_private_emitAstContinue(Toy_Bucket** bucketHandle, Toy_Ast** rootHandle);
void Toy_private_emitAstReturn(Toy_Bucket** bucketHandle, Toy_Ast** astHandle);
void Toy_private_emitAstPrint(Toy_Bucket** bucketHandle, Toy_Ast** astHandle);

void Toy_private_emitAstVariableDeclaration(Toy_Bucket** bucketHandle, Toy_Ast** astHandle, Toy_String* name, Toy_ValueType valueType, bool constant, Toy_Ast* expr);
void Toy_private_emitAstVariableAssignment(Toy_Bucket** bucketHandle, Toy_Ast** astHandle, Toy_AstFlag flag, Toy_Ast* expr);
void Toy_private_emitAstVariableAccess(Toy_Bucket** bucketHandle, Toy_Ast** astHandle);

void Toy_private_emitAstFunctionDeclaration(Toy_Bucket** bucketHandle, Toy_Ast** astHandle, Toy_String* name, Toy_Ast* params, Toy_Ast* body);
void Toy_private_emitAstFunctionInvokation(Toy_Bucket** bucketHandle, Toy_Ast** astHandle, Toy_Ast* params);
void Toy_private_emitAstAttribute(Toy_Bucket** bucketHandle, Toy_Ast** astHandle, Toy_Ast* expr);
void Toy_private_emitAstIterable(Toy_Bucket** bucketHandle, Toy_Ast** astHandle, Toy_Ast* expr);

void Toy_private_emitAstStackPop(Toy_Bucket** bucketHandle, Toy_Ast** astHandle);

void Toy_private_emitAstPass(Toy_Bucket** bucketHandle, Toy_Ast** astHandle);
void Toy_private_emitAstError(Toy_Bucket** bucketHandle, Toy_Ast** astHandle);
void Toy_private_emitAstEnd(Toy_Bucket** bucketHandle, Toy_Ast** astHandle);

const char* Toy_private_getAstTypeAsCString(Toy_AstType type);