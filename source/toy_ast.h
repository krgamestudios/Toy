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
	TOY_AST_BREAK,
	TOY_AST_CONTINUE,
	TOY_AST_PRINT,

	TOY_AST_VAR_DECLARE,
	TOY_AST_VAR_ASSIGN,
	TOY_AST_VAR_ACCESS,

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

	//unary flags
	TOY_AST_FLAG_NEGATE = 40,
	TOY_AST_FLAG_PREFIX_INCREMENT = 41,
	TOY_AST_FLAG_PREFIX_DECREMENT = 42,
	TOY_AST_FLAG_POSTFIX_INCREMENT = 43,
	TOY_AST_FLAG_POSTFIX_DECREMENT = 44,

	// TOY_AST_FLAG_TERNARY,
} Toy_AstFlag;

//the root AST type
typedef union Toy_Ast Toy_Ast;

typedef struct Toy_AstBlock {
	Toy_AstType type;
	bool innerScope;
	Toy_Ast* child; //begin encoding the line
	Toy_Ast* next; //'next' is either an AstBlock or null
	Toy_Ast* tail; //'tail' - either points to the tail of the current list, or null; only used by the head of a list as an optimisation
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

typedef struct Toy_AstBreak {
	Toy_AstType type;
} Toy_AstBreak;

typedef struct Toy_AstContinue {
	Toy_AstType type;
} Toy_AstContinue;

typedef struct Toy_AstPrint {
	Toy_AstType type;
	Toy_Ast* child;
} Toy_AstPrint;

typedef struct Toy_AstVarDeclare {
	Toy_AstType type;
	Toy_String* name;
	Toy_Ast* expr;
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

typedef struct Toy_AstPass {
	Toy_AstType type;
} Toy_AstPass;

typedef struct Toy_AstError {
	Toy_AstType type;
} Toy_AstError;

typedef struct Toy_AstEnd {
	Toy_AstType type;
} Toy_AstEnd;

union Toy_Ast {                                             //32 | 64 BITNESS
	Toy_AstType type;                                       //4  | 4
	Toy_AstBlock block;                                     //16 | 32
	Toy_AstValue value;                                     //12 | 24
	Toy_AstUnary unary;                                     //12 | 16
	Toy_AstBinary binary;                                   //16 | 24
	Toy_AstBinaryShortCircuit binaryShortCircuit;           //16 | 24
	Toy_AstCompare compare;                                 //16 | 24
	Toy_AstGroup group;                                     //8  | 16
	Toy_AstCompound compound;                               //12 | 16
	Toy_AstAggregate aggregate;                             //16 | 24
	Toy_AstAssert assert;                                   //16 | 24
	Toy_AstIfThenElse ifThenElse;                           //16 | 32
	Toy_AstWhileThen whileThen;                             //16 | 24
	Toy_AstBreak breakPoint;                                //4  | 4
	Toy_AstContinue continuePoint;                          //4  | 4
	Toy_AstPrint print;                                     //8  | 16
	Toy_AstVarDeclare varDeclare;                           //16 | 24
	Toy_AstVarAssign varAssign;                             //16 | 24
	Toy_AstVarAccess varAccess;                             //8  | 16
	Toy_AstPass pass;                                       //4  | 4
	Toy_AstError error;                                     //4  | 4
	Toy_AstEnd end;                                         //4  | 4
};                                                          //16 | 32

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
void Toy_private_emitAstBreak(Toy_Bucket** bucketHandle, Toy_Ast** rootHandle);
void Toy_private_emitAstContinue(Toy_Bucket** bucketHandle, Toy_Ast** rootHandle);
void Toy_private_emitAstPrint(Toy_Bucket** bucketHandle, Toy_Ast** astHandle);

void Toy_private_emitAstVariableDeclaration(Toy_Bucket** bucketHandle, Toy_Ast** astHandle, Toy_String* name, Toy_Ast* expr);
void Toy_private_emitAstVariableAssignment(Toy_Bucket** bucketHandle, Toy_Ast** astHandle, Toy_AstFlag flag, Toy_Ast* expr);
void Toy_private_emitAstVariableAccess(Toy_Bucket** bucketHandle, Toy_Ast** astHandle);

void Toy_private_emitAstPass(Toy_Bucket** bucketHandle, Toy_Ast** astHandle);
void Toy_private_emitAstError(Toy_Bucket** bucketHandle, Toy_Ast** astHandle);
void Toy_private_emitAstEnd(Toy_Bucket** bucketHandle, Toy_Ast** astHandle);

