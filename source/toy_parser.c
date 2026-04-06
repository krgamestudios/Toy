#include "toy_parser.h"
#include "toy_console_colors.h"

#include "toy_value.h"
#include "toy_string.h"

#include <stdio.h>

//utilities
static void printError(Toy_Parser* parser, Toy_Token token, const char* errorMsg) {
	//keep going while panicking
	if (parser->panic) {
		return;
	}

	fprintf(stderr, TOY_CC_ERROR "[Line %d] Error ", (int)token.line);

	//check type
	if (token.type == TOY_TOKEN_EOF) {
		fprintf(stderr, "at end");
	}
	else {
		fprintf(stderr, "at '%.*s'", (int)token.length, token.lexeme);
	}

	//finally
	fprintf(stderr, ": %s\n" TOY_CC_RESET, errorMsg);
	parser->error = true;
	parser->panic = true;
}

static void advance(Toy_Parser* parser) {
	parser->previous = parser->current;
	parser->current = Toy_private_scanLexer(parser->lexer);

	if (parser->current.type == TOY_TOKEN_ERROR) {
		printError(parser, parser->current, "Can't read the source code");
	}
}

static bool match(Toy_Parser* parser, Toy_TokenType tokenType) {
	if (parser->current.type == tokenType) {
		advance(parser);
		return true;
	}
	return false;
}

static void consume(Toy_Parser* parser, Toy_TokenType tokenType, const char* msg) {
	if (parser->current.type != tokenType) {
		printError(parser, parser->current, msg);
		return;
	}

	advance(parser);
}

static void synchronize(Toy_Parser* parser) {
	while (parser->current.type != TOY_TOKEN_EOF) {
		switch(parser->current.type) {
			//these tokens can start a statement
			case TOY_TOKEN_KEYWORD_ASSERT:
			case TOY_TOKEN_KEYWORD_BREAK:
			case TOY_TOKEN_KEYWORD_CLASS:
			case TOY_TOKEN_KEYWORD_CONTINUE:
			case TOY_TOKEN_KEYWORD_DO:
			case TOY_TOKEN_KEYWORD_EXPORT:
			case TOY_TOKEN_KEYWORD_FOR:
			case TOY_TOKEN_KEYWORD_FOREACH:
			case TOY_TOKEN_KEYWORD_FUNCTION:
			case TOY_TOKEN_KEYWORD_IF:
			case TOY_TOKEN_KEYWORD_IMPORT:
			case TOY_TOKEN_KEYWORD_PRINT:
			case TOY_TOKEN_KEYWORD_RETURN:
			case TOY_TOKEN_KEYWORD_VAR:
			case TOY_TOKEN_KEYWORD_WHILE:
			case TOY_TOKEN_KEYWORD_YIELD:
				parser->error = true;
				parser->panic = false;
				return;

			default:
				advance(parser);
		}
	}
}

//precedence declarations
typedef enum ParsingPrecedence {
	PREC_NONE,
	PREC_ASSIGNMENT,
	PREC_GROUP,
	PREC_TERNARY,
	PREC_NEGATE,
	PREC_OR,
	PREC_AND,
	PREC_COMPARISON,
	PREC_TERM,
	PREC_FACTOR,
	PREC_UNARY,
	PREC_CALL,
	PREC_PRIMARY,
} ParsingPrecedence;

typedef Toy_AstFlag (*ParsingRule)(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle);

typedef struct ParsingTuple {
	ParsingPrecedence precedence;
	ParsingRule prefix;
	ParsingRule infix;
} ParsingTuple;

static void parsePrecedence(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle, ParsingPrecedence precRule);

static Toy_AstFlag identifier(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle);
static Toy_AstFlag literal(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle);
static Toy_AstFlag unary(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle);
static Toy_AstFlag binary(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle);
static Toy_AstFlag group(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle);
static Toy_AstFlag compound(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle);
static Toy_AstFlag aggregate(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle);
static Toy_AstFlag unaryPostfix(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle);
static Toy_AstFlag invoke(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle);

//precedence definitions
static ParsingTuple parsingRulesetTable[] = {
	{PREC_PRIMARY,literal,NULL},// TOY_TOKEN_NULL,

	//variable names (initially handled as a string)
	{PREC_PRIMARY,identifier,NULL},// TOY_TOKEN_NAME,

	//types
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_TYPE_BOOLEAN,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_TYPE_INTEGER,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_TYPE_FLOAT,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_TYPE_STRING,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_TYPE_ARRAY,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_TYPE_TABLE,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_TYPE_FUNCTION,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_TYPE_OPAQUE,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_TYPE_ANY,

	//keywords and reserved words
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_AS,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_ASSERT,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_BREAK,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_CLASS,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_CONST,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_CONTINUE,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_DO,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_ELSE,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_EXPORT,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_FOR,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_FOREACH,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_FUNCTION,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_IF,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_IMPORT,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_IN,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_OF,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_PASS,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_PRINT,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_RETURN,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_VAR,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_WHILE,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_KEYWORD_YIELD,

	//literal values
	{PREC_PRIMARY,literal,NULL},// TOY_TOKEN_LITERAL_TRUE,
	{PREC_PRIMARY,literal,NULL},// TOY_TOKEN_LITERAL_FALSE,
	{PREC_PRIMARY,literal,NULL},// TOY_TOKEN_LITERAL_INTEGER,
	{PREC_PRIMARY,literal,NULL},// TOY_TOKEN_LITERAL_FLOAT,
	{PREC_PRIMARY,literal,NULL},// TOY_TOKEN_LITERAL_STRING,

	//math operators
	{PREC_TERM,NULL,binary},// TOY_TOKEN_OPERATOR_ADD,
	{PREC_TERM,unary,binary},// TOY_TOKEN_OPERATOR_SUBTRACT,
	{PREC_FACTOR,NULL,binary},// TOY_TOKEN_OPERATOR_MULTIPLY,
	{PREC_FACTOR,NULL,binary},// TOY_TOKEN_OPERATOR_DIVIDE,
	{PREC_FACTOR,NULL,binary},// TOY_TOKEN_OPERATOR_MODULO,
	{PREC_ASSIGNMENT,NULL,binary},// TOY_TOKEN_OPERATOR_ADD_ASSIGN,
	{PREC_ASSIGNMENT,NULL,binary},// TOY_TOKEN_OPERATOR_SUBTRACT_ASSIGN,
	{PREC_ASSIGNMENT,NULL,binary},// TOY_TOKEN_OPERATOR_MULTIPLY_ASSIGN,
	{PREC_ASSIGNMENT,NULL,binary},// TOY_TOKEN_OPERATOR_DIVIDE_ASSIGN,
	{PREC_ASSIGNMENT,NULL,binary},// TOY_TOKEN_OPERATOR_MODULO_ASSIGN,
	{PREC_UNARY,unary,unaryPostfix},// TOY_TOKEN_OPERATOR_INCREMENT,
	{PREC_UNARY,unary,unaryPostfix},// TOY_TOKEN_OPERATOR_DECREMENT,
	{PREC_ASSIGNMENT,NULL,binary},// TOY_TOKEN_OPERATOR_ASSIGN,

	//comparator operators
	{PREC_COMPARISON,NULL,binary},// TOY_TOKEN_OPERATOR_COMPARE_EQUAL,
	{PREC_COMPARISON,NULL,binary},// TOY_TOKEN_OPERATOR_COMPARE_NOT,
	{PREC_COMPARISON,NULL,binary},// TOY_TOKEN_OPERATOR_COMPARE_LESS,
	{PREC_COMPARISON,NULL,binary},// TOY_TOKEN_OPERATOR_COMPARE_LESS_EQUAL,
	{PREC_COMPARISON,NULL,binary},// TOY_TOKEN_OPERATOR_COMPARE_GREATER,
	{PREC_COMPARISON,NULL,binary},// TOY_TOKEN_OPERATOR_COMPARE_GREATER_EQUAL,

	//structural operators
	{PREC_CALL,group,invoke},// TOY_TOKEN_OPERATOR_PAREN_LEFT,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_OPERATOR_PAREN_RIGHT,
	{PREC_GROUP,compound,aggregate},// TOY_TOKEN_OPERATOR_BRACKET_LEFT,
	{PREC_NONE,compound,aggregate},// TOY_TOKEN_OPERATOR_BRACKET_RIGHT,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_OPERATOR_BRACE_LEFT,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_OPERATOR_BRACE_RIGHT,

	//other operators
	{PREC_AND,NULL,binary},// TOY_TOKEN_OPERATOR_AND,
	{PREC_OR,NULL,binary},// TOY_TOKEN_OPERATOR_OR,
	{PREC_NEGATE,unary,NULL},// TOY_TOKEN_OPERATOR_NEGATE,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_OPERATOR_QUESTION,
	{PREC_GROUP,compound,aggregate},// TOY_TOKEN_OPERATOR_COLON,

	{PREC_NONE,NULL,NULL},// TOY_TOKEN_OPERATOR_SEMICOLON,
	{PREC_GROUP,NULL,aggregate},// TOY_TOKEN_OPERATOR_COMMA,

	{PREC_NONE,NULL,NULL},// TOY_TOKEN_OPERATOR_DOT,
	{PREC_UNARY,NULL,binary},// TOY_TOKEN_OPERATOR_CONCAT,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_OPERATOR_REST,

	//unused operators
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_OPERATOR_AMPERSAND,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_OPERATOR_PIPE,

	//meta tokens
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_ERROR,
	{PREC_NONE,NULL,NULL},// TOY_TOKEN_EOF,
};

static ParsingTuple* getParsingRule(Toy_TokenType type) {
	return &parsingRulesetTable[type];
}

static Toy_ValueType readType(Toy_Parser* parser) {
	advance(parser);

	switch(parser->previous.type) {
		case TOY_TOKEN_TYPE_BOOLEAN:
			return TOY_VALUE_BOOLEAN;

		case TOY_TOKEN_TYPE_INTEGER:
			return TOY_VALUE_INTEGER;

		case TOY_TOKEN_TYPE_FLOAT:
			return TOY_VALUE_FLOAT;

		case TOY_TOKEN_TYPE_STRING:
			return TOY_VALUE_STRING;

		case TOY_TOKEN_TYPE_ARRAY:
			return TOY_VALUE_ARRAY;

		case TOY_TOKEN_TYPE_TABLE:
			return TOY_VALUE_TABLE;

		case TOY_TOKEN_TYPE_FUNCTION:
			return TOY_VALUE_FUNCTION;

		case TOY_TOKEN_TYPE_OPAQUE:
			return TOY_VALUE_OPAQUE;

		case TOY_TOKEN_TYPE_ANY:
			return TOY_VALUE_ANY;

		default:
			printError(parser, parser->previous, "Expected type identifier");
			return TOY_VALUE_UNKNOWN;
	}
}

static Toy_AstFlag identifier(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle) {
	//emit the name string
	Toy_String* name = Toy_toStringLength(bucketHandle, parser->previous.lexeme, parser->previous.length);
	Toy_Value value = TOY_VALUE_FROM_STRING(name);
	Toy_private_emitAstValue(bucketHandle, rootHandle, value);

	//check for an assignment
	Toy_AstFlag flag = TOY_AST_FLAG_NONE;

	if (match(parser, TOY_TOKEN_OPERATOR_ASSIGN)) {
		flag = TOY_AST_FLAG_ASSIGN;
	}
	else if (match(parser, TOY_TOKEN_OPERATOR_ADD_ASSIGN)) {
		flag = TOY_AST_FLAG_ADD_ASSIGN;
	}
	else if (match(parser, TOY_TOKEN_OPERATOR_SUBTRACT_ASSIGN)) {
		flag = TOY_AST_FLAG_SUBTRACT_ASSIGN;
	}
	else if (match(parser, TOY_TOKEN_OPERATOR_MULTIPLY_ASSIGN)) {
		flag = TOY_AST_FLAG_MULTIPLY_ASSIGN;
	}
	else if (match(parser, TOY_TOKEN_OPERATOR_DIVIDE_ASSIGN)) {
		flag = TOY_AST_FLAG_DIVIDE_ASSIGN;
	}
	else if (match(parser, TOY_TOKEN_OPERATOR_MODULO_ASSIGN)) {
		flag = TOY_AST_FLAG_MODULO_ASSIGN;
	}

	//emit the assignment if found
	if (flag != TOY_AST_FLAG_NONE) {
		Toy_Ast* expr = NULL;
		parsePrecedence(bucketHandle, parser, &expr, PREC_ASSIGNMENT); //this makes chained assignment possible, I think
		Toy_private_emitAstVariableAssignment(bucketHandle, rootHandle, flag, expr);
		return TOY_AST_FLAG_NONE;
	}

	//assume it's an access instead
	Toy_private_emitAstVariableAccess(bucketHandle, rootHandle);
	return TOY_AST_FLAG_NONE;
}

static Toy_AstFlag literal(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle) {
	switch(parser->previous.type) {
		case TOY_TOKEN_NULL:
			Toy_private_emitAstValue(bucketHandle, rootHandle, TOY_VALUE_FROM_NULL());
			return TOY_AST_FLAG_NONE;

		case TOY_TOKEN_LITERAL_TRUE:
			Toy_private_emitAstValue(bucketHandle, rootHandle, TOY_VALUE_FROM_BOOLEAN(true));
			return TOY_AST_FLAG_NONE;

		case TOY_TOKEN_LITERAL_FALSE:
			Toy_private_emitAstValue(bucketHandle, rootHandle, TOY_VALUE_FROM_BOOLEAN(false));
			return TOY_AST_FLAG_NONE;

		case TOY_TOKEN_LITERAL_INTEGER: {
			//filter the '_' character
			char buffer[parser->previous.length];

			unsigned int i = 0, o = 0;
			do {
				buffer[i] = parser->previous.lexeme[o];
				if (buffer[i] != '_') i++;
			} while (parser->previous.lexeme[o++] && i < parser->previous.length);
			buffer[i] = '\0'; //BUGFIX

			int value = 0;
			sscanf(buffer, "%d", &value);
			Toy_private_emitAstValue(bucketHandle, rootHandle, TOY_VALUE_FROM_INTEGER(value));
			return TOY_AST_FLAG_NONE;
		}

		case TOY_TOKEN_LITERAL_FLOAT: {
			//filter the '_' character
			char buffer[parser->previous.length];

			unsigned int i = 0, o = 0;
			do {
				buffer[i] = parser->previous.lexeme[o];
				if (buffer[i] != '_') i++;
			} while (parser->previous.lexeme[o++] && i < parser->previous.length);
			buffer[i] = '\0'; //BUGFIX

			float value = 0;
			sscanf(buffer, "%f", &value);
			Toy_private_emitAstValue(bucketHandle, rootHandle, TOY_VALUE_FROM_FLOAT(value));
			return TOY_AST_FLAG_NONE;
		}

		case TOY_TOKEN_LITERAL_STRING: {
			char buffer[parser->previous.length + 1];
			unsigned int escapeCounter = 0;
			unsigned int i = 0, o = 0;

			if (parser->previous.length > 0) { //BUGFIX: compensate for zero-length strings
				do {
					buffer[i] = parser->previous.lexeme[o];
					if (buffer[i] == '\\' && parser->previous.lexeme[++o]) {
						escapeCounter++;

						//also handle escape characters
						switch(parser->previous.lexeme[o]) {
							case 'n':
								buffer[i] = '\n';
								break;
							case 't':
								buffer[i] = '\t';
								break;
							case '\\':
								buffer[i] = '\\';
								break;
							case '"':
								buffer[i] = '"';
								break;
						}
					}
					i++;
				} while (parser->previous.lexeme[o++] && i < parser->previous.length);
			}

			buffer[i] = '\0';
			unsigned int len = i - escapeCounter; //NOTE: len is ONLY the string length
			Toy_private_emitAstValue(bucketHandle, rootHandle, TOY_VALUE_FROM_STRING(Toy_createStringLength(bucketHandle, buffer, len))); //BUGFIX: create the string to avoid losing local var 'buffer'

			return TOY_AST_FLAG_NONE;
		}

		default:
			printError(parser, parser->previous, "Unexpected token passed to literal precedence rule");
			Toy_private_emitAstError(bucketHandle, rootHandle);
			return TOY_AST_FLAG_NONE;
	}
}

static Toy_AstFlag unary(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle) {
	//'subtract' can only be applied to numbers and groups, while 'negate' can only be applied to booleans and groups

	if (parser->previous.type == TOY_TOKEN_OPERATOR_SUBTRACT) {
		bool connectedDigit = parser->previous.lexeme[1] >= '0' && parser->previous.lexeme[1] <= '9'; //BUGFIX: '- 1' should not be optimised into a negative
		parsePrecedence(bucketHandle, parser, rootHandle, PREC_UNARY);

		//negative numbers
		if ((*rootHandle)->type == TOY_AST_VALUE && TOY_VALUE_IS_INTEGER((*rootHandle)->value.value) && connectedDigit) {
			(*rootHandle)->value.value = TOY_VALUE_FROM_INTEGER( -TOY_VALUE_AS_INTEGER((*rootHandle)->value.value) );
		}
		else if ((*rootHandle)->type == TOY_AST_VALUE && TOY_VALUE_IS_FLOAT((*rootHandle)->value.value) && connectedDigit) {
			(*rootHandle)->value.value = TOY_VALUE_FROM_FLOAT( -TOY_VALUE_AS_FLOAT((*rootHandle)->value.value) );
		}
		else {
			//actually emit the negation node
			Toy_private_emitAstUnary(bucketHandle, rootHandle, TOY_AST_FLAG_NEGATE);
		}
	}

	else if (parser->previous.type == TOY_TOKEN_OPERATOR_NEGATE) {
		parsePrecedence(bucketHandle, parser, rootHandle, PREC_UNARY);
		Toy_private_emitAstUnary(bucketHandle, rootHandle, TOY_AST_FLAG_NEGATE);
	}

	else if (parser->previous.type == TOY_TOKEN_OPERATOR_INCREMENT || parser->previous.type == TOY_TOKEN_OPERATOR_DECREMENT) {
		Toy_AstFlag flag = parser->previous.type == TOY_TOKEN_OPERATOR_INCREMENT ? TOY_AST_FLAG_PREFIX_INCREMENT : TOY_AST_FLAG_PREFIX_DECREMENT;

		//grab the info below
		Toy_Ast* primary = NULL;

		parsePrecedence(bucketHandle, parser, &primary, PREC_PRIMARY);

		//double check it's a name string within an access NOTE: doing some fiddling with the existing AST here
		if (primary->type != TOY_AST_VAR_ACCESS || primary->varAccess.child->type != TOY_AST_VALUE || TOY_VALUE_IS_STRING(primary->varAccess.child->value.value) != true) {
			printError(parser, parser->previous, "Unexpected non-name-string token in unary-prefix operator precedence rule");
			Toy_private_emitAstError(bucketHandle, rootHandle);
		}
		else {
			//swap the varAccess for a unary prefix, as the latter leaves the value on the stack
			*rootHandle = primary->varAccess.child;
			Toy_private_emitAstUnary(bucketHandle, rootHandle, flag);
		}
	}

	else {
		printError(parser, parser->previous, "Unexpected token passed to unary precedence rule");
		Toy_private_emitAstError(bucketHandle, rootHandle);
	}

	return TOY_AST_FLAG_NONE;
}

static Toy_AstFlag binary(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle) {
	//infix must advance
	advance(parser);

	switch(parser->previous.type) {
		//arithmetic
		case TOY_TOKEN_OPERATOR_ADD: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_TERM + 1);
			return TOY_AST_FLAG_ADD;
		}

		case TOY_TOKEN_OPERATOR_SUBTRACT: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_TERM + 1);
			return TOY_AST_FLAG_SUBTRACT;
		}

		case TOY_TOKEN_OPERATOR_MULTIPLY: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_FACTOR + 1);
			return TOY_AST_FLAG_MULTIPLY;
		}

		case TOY_TOKEN_OPERATOR_DIVIDE: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_FACTOR + 1);
			return TOY_AST_FLAG_DIVIDE;
		}

		case TOY_TOKEN_OPERATOR_MODULO: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_FACTOR + 1);
			return TOY_AST_FLAG_MODULO;
		}

		//assignment
		case TOY_TOKEN_OPERATOR_ASSIGN: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_ASSIGNMENT + 1);
			return TOY_AST_FLAG_ASSIGN;
		}

		case TOY_TOKEN_OPERATOR_ADD_ASSIGN: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_ASSIGNMENT + 1);
			return TOY_AST_FLAG_ADD_ASSIGN;
		}

		case TOY_TOKEN_OPERATOR_SUBTRACT_ASSIGN: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_ASSIGNMENT + 1);
			return TOY_AST_FLAG_SUBTRACT_ASSIGN;
		}

		case TOY_TOKEN_OPERATOR_MULTIPLY_ASSIGN: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_ASSIGNMENT + 1);
			return TOY_AST_FLAG_MULTIPLY_ASSIGN;
		}

		case TOY_TOKEN_OPERATOR_DIVIDE_ASSIGN: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_ASSIGNMENT + 1);
			return TOY_AST_FLAG_DIVIDE_ASSIGN;
		}

		case TOY_TOKEN_OPERATOR_MODULO_ASSIGN: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_ASSIGNMENT + 1);
			return TOY_AST_FLAG_MODULO_ASSIGN;
		}

		//comparison
		case TOY_TOKEN_OPERATOR_COMPARE_EQUAL: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_COMPARISON + 1);
			return TOY_AST_FLAG_COMPARE_EQUAL;
		}

		case TOY_TOKEN_OPERATOR_COMPARE_NOT: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_COMPARISON + 1);
			return TOY_AST_FLAG_COMPARE_NOT;
		}

		case TOY_TOKEN_OPERATOR_COMPARE_LESS: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_COMPARISON + 1);
			return TOY_AST_FLAG_COMPARE_LESS;
		}

		case TOY_TOKEN_OPERATOR_COMPARE_LESS_EQUAL: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_COMPARISON + 1);
			return TOY_AST_FLAG_COMPARE_LESS_EQUAL;
		}

		case TOY_TOKEN_OPERATOR_COMPARE_GREATER: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_COMPARISON + 1);
			return TOY_AST_FLAG_COMPARE_GREATER;
		}

		case TOY_TOKEN_OPERATOR_COMPARE_GREATER_EQUAL: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_COMPARISON + 1);
			return TOY_AST_FLAG_COMPARE_GREATER_EQUAL;
		}

		//logical
		case TOY_TOKEN_OPERATOR_AND: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_AND + 1);
			return TOY_AST_FLAG_AND;
		}

		case TOY_TOKEN_OPERATOR_OR: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_OR + 1);
			return TOY_AST_FLAG_OR;
		}

		case TOY_TOKEN_OPERATOR_CONCAT: {
			parsePrecedence(bucketHandle, parser, rootHandle, PREC_UNARY + 1);
			return TOY_AST_FLAG_CONCAT;
		}

		default:
			printError(parser, parser->previous, "Unexpected token passed to binary precedence rule");
			Toy_private_emitAstError(bucketHandle, rootHandle);
			return TOY_AST_FLAG_NONE;
	}
}

static Toy_AstFlag group(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle) {
	//groups are ()
	if (parser->previous.type == TOY_TOKEN_OPERATOR_PAREN_LEFT) {
		parsePrecedence(bucketHandle, parser, rootHandle, PREC_GROUP);
		consume(parser, TOY_TOKEN_OPERATOR_PAREN_RIGHT, "Expected ')' at end of group");

		//Toy_AstGroup could be omitted, but is kept for now
		Toy_private_emitAstGroup(bucketHandle, rootHandle);
	}

	else {
		printError(parser, parser->previous, "Unexpected token passed to grouping precedence rule");
		Toy_private_emitAstError(bucketHandle, rootHandle);
	}

	return TOY_AST_FLAG_NONE;
}

static Toy_AstFlag compound(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle) {
	//read in an array or table compound definition
	if (parser->previous.type == TOY_TOKEN_OPERATOR_BRACKET_LEFT) {
		//BUGFIX: special case for empty arrays
		if (match(parser, TOY_TOKEN_OPERATOR_BRACKET_RIGHT)) {
			Toy_private_emitAstPass(bucketHandle, rootHandle);
			Toy_private_emitAstCompound(bucketHandle, rootHandle, TOY_AST_FLAG_COMPOUND_ARRAY);
			return TOY_AST_FLAG_NONE;
		}

		//BUGFIX: special case for empty tables
		if (match(parser, TOY_TOKEN_OPERATOR_COLON)) {
			consume(parser, TOY_TOKEN_OPERATOR_BRACKET_RIGHT, "Expected ']' at the end of empty table");
			Toy_private_emitAstPass(bucketHandle, rootHandle);
			Toy_private_emitAstAggregate(bucketHandle, rootHandle, TOY_AST_FLAG_PAIR, *rootHandle);
			Toy_private_emitAstCompound(bucketHandle, rootHandle, TOY_AST_FLAG_COMPOUND_TABLE);
			return TOY_AST_FLAG_NONE;
		}

		parsePrecedence(bucketHandle, parser, rootHandle, PREC_GROUP);

		//peek inside to see what you have
		Toy_AstFlag flag = TOY_AST_FLAG_NONE;
		if ((*rootHandle)->type == TOY_AST_AGGREGATE) {
			//1 element in a table will mean the top-level node is a collection
			flag = (*rootHandle)->aggregate.flag;

			//more than 1 element in a table will mean you need to check the last element in the collection to find a pair
			if (flag == TOY_AST_FLAG_COLLECTION && (*rootHandle)->aggregate.right->type == TOY_AST_AGGREGATE) {
				flag = (*rootHandle)->aggregate.right->aggregate.flag; //yes, this is hacky
			}
		}

		//BUGFIX: special case for trailing commas
		if (parser->previous.type == TOY_TOKEN_OPERATOR_BRACKET_RIGHT && parser->current.type != TOY_TOKEN_OPERATOR_BRACKET_RIGHT) {
			Toy_private_emitAstCompound(bucketHandle, rootHandle, flag == TOY_AST_FLAG_PAIR ? TOY_AST_FLAG_COMPOUND_TABLE : TOY_AST_FLAG_COMPOUND_ARRAY);
			return TOY_AST_FLAG_NONE;
		}

		consume(parser, TOY_TOKEN_OPERATOR_BRACKET_RIGHT, "Expected ']' at the end of compound expression");
		Toy_private_emitAstCompound(bucketHandle, rootHandle, flag == TOY_AST_FLAG_PAIR ? TOY_AST_FLAG_COMPOUND_TABLE : TOY_AST_FLAG_COMPOUND_ARRAY);

		return TOY_AST_FLAG_NONE;
	}
	else if (parser->previous.type == TOY_TOKEN_OPERATOR_BRACKET_RIGHT) {
		//allows for trailing commas
		Toy_private_emitAstPass(bucketHandle, rootHandle);
		return TOY_AST_FLAG_NONE;
	}
	else {
		printError(parser, parser->previous, "Unexpected token passed to compound precedence rule");
		Toy_private_emitAstError(bucketHandle, rootHandle);
		return TOY_AST_FLAG_NONE;
	}
}

static Toy_AstFlag aggregate(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle) {
	//infix must advance
	advance(parser);

	//aggregates are a collection of parts, expressing a larger idea
	if (parser->previous.type == TOY_TOKEN_OPERATOR_COMMA) {
		parsePrecedence(bucketHandle, parser, rootHandle, PREC_GROUP); //NOT +1, as compounds are right-recursive
		return TOY_AST_FLAG_COLLECTION;
	}
	if (parser->previous.type == TOY_TOKEN_OPERATOR_COLON) {
		parsePrecedence(bucketHandle, parser, rootHandle, PREC_GROUP); //NOT +1, as compounds are right-recursive
		return TOY_AST_FLAG_PAIR;
	}
	else if (parser->previous.type == TOY_TOKEN_OPERATOR_BRACKET_LEFT) {
		parsePrecedence(bucketHandle, parser, rootHandle, PREC_GROUP);
		consume(parser, TOY_TOKEN_OPERATOR_BRACKET_RIGHT, "Expected ']' at the end of index expression");
		return TOY_AST_FLAG_INDEX;
	}
	else if (parser->previous.type == TOY_TOKEN_OPERATOR_BRACKET_RIGHT) {
		Toy_private_emitAstPass(bucketHandle, rootHandle);
		return TOY_AST_FLAG_NONE;
	}
	else {
		printError(parser, parser->previous, "Unexpected token passed to aggregate precedence rule");
		Toy_private_emitAstError(bucketHandle, rootHandle);
		return TOY_AST_FLAG_NONE;
	}
}

static Toy_AstFlag unaryPostfix(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle) {
	//grab the var name, rejecting any other token types
	if (parser->previous.type != TOY_TOKEN_NAME) {
		printError(parser, parser->previous, "Unexpected parameter passed to unary-postfix precedence rule");
		Toy_private_emitAstError(bucketHandle, rootHandle);
		return TOY_AST_FLAG_NONE;
	}

	Toy_Ast* primary = NULL;
	ParsingRule nameRule = getParsingRule(parser->previous.type)->prefix;
	nameRule(bucketHandle, parser, &primary); //this is to skip the call to advance() at the beginning of parsePrecedence()

	//double check it's a name string within an access NOTE: doing some fiddling with the existing AST here
	if (primary->type != TOY_AST_VAR_ACCESS || primary->varAccess.child->type != TOY_AST_VALUE || TOY_VALUE_IS_STRING(primary->varAccess.child->value.value) != true) {
		printError(parser, parser->previous, "Unexpected non-name-string token in unary-postfix operator precedence rule");
		Toy_private_emitAstError(bucketHandle, rootHandle);
		return TOY_AST_FLAG_NONE;
	}

	(*rootHandle) = primary->varAccess.child;

	//output the postfix AST
	if (match(parser, TOY_TOKEN_OPERATOR_INCREMENT)) {
		Toy_private_emitAstUnary(bucketHandle, rootHandle, TOY_AST_FLAG_POSTFIX_INCREMENT);
		return TOY_AST_FLAG_POSTFIX_INCREMENT;
	}
	else if (match(parser, TOY_TOKEN_OPERATOR_DECREMENT)) {
		Toy_private_emitAstUnary(bucketHandle, rootHandle, TOY_AST_FLAG_POSTFIX_DECREMENT);
		return TOY_AST_FLAG_POSTFIX_DECREMENT;
	}
	else {
		printError(parser, parser->previous, "Unexpected token passed to unary-postfix precedence rule");
		Toy_private_emitAstError(bucketHandle, rootHandle);
		return TOY_AST_FLAG_NONE;
	}
}

static Toy_AstFlag invoke(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle) {
	//infix must advance
	advance(parser);

	//read in args
	Toy_Ast* args = NULL;
	unsigned int paramIterations = 0;

	while (parser->current.type != TOY_TOKEN_OPERATOR_PAREN_RIGHT && (paramIterations++ == 0 || match(parser, TOY_TOKEN_OPERATOR_COMMA))) {
		//get the next arg
		Toy_Ast* ast = NULL;
		parsePrecedence(bucketHandle, parser, &ast, PREC_GROUP);

		//add to the args aggregate (is added backwards, because weird)
		Toy_private_emitAstAggregate(bucketHandle, &args, TOY_AST_FLAG_COLLECTION, ast);
	}

	consume(parser, TOY_TOKEN_OPERATOR_PAREN_RIGHT, "Expected ')' at the end of argument list");

	//finally, emit the call as an Ast
	Toy_private_emitAstFunctionInvokation(bucketHandle, rootHandle, args);

	return TOY_AST_FLAG_NONE;
}

//grammar rules
static void parsePrecedence(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle, ParsingPrecedence precRule) {
	//'step over' the token to parse
	advance(parser);

	//every valid expression has a prefix rule
	ParsingRule prefix = getParsingRule(parser->previous.type)->prefix;

	if (prefix == NULL) {
		//make a nice error message
		if (Toy_private_findKeywordByType(parser->previous.type)) {
			printError(parser, parser->previous, "Found reserved keyword instead");
		}
		else {
			printError(parser, parser->previous, "Expected expression");
		}

		Toy_private_emitAstError(bucketHandle, rootHandle);
		return;
	}

	prefix(bucketHandle, parser, rootHandle);

	//infix rules are left-recursive
	while (precRule <= getParsingRule(parser->current.type)->precedence) {
		ParsingRule infix = getParsingRule(parser->current.type)->infix;

		if (infix == NULL) {
			printError(parser, parser->previous, "Expected operator");
			Toy_private_emitAstError(bucketHandle, rootHandle);
			return;
		}

		Toy_Ast* ptr = (*rootHandle); //NOTE: infix functions will need to be careful not to damage the pre-existing tree, if they can avoid it
		Toy_AstFlag flag = infix(bucketHandle, parser, &ptr);

		//finished
		if (flag == TOY_AST_FLAG_NONE) {
			(*rootHandle) = ptr;
			return;
		}
		//eww, gross
		else if (flag >= 10 && flag <= 19) {
			Toy_private_emitAstVariableAssignment(bucketHandle, rootHandle, flag, ptr);
		}
		else if (flag >= 20 && flag <= 29) {
			Toy_private_emitAstCompare(bucketHandle, rootHandle, flag, ptr);
		}
		else if (flag >= 30 && flag <= 39) {
			Toy_private_emitAstAggregate(bucketHandle, rootHandle, flag, ptr);
		}
		else if (flag >= 40 && flag <= 49) {
			(*rootHandle) = ptr;
			continue;
		}
		else {
			//BUGFIX: '&&' and '||' are special cases, with short-circuit logic
			if (flag == TOY_AST_FLAG_AND || flag == TOY_AST_FLAG_OR) {
				Toy_private_emitAstBinaryShortCircuit(bucketHandle, rootHandle, flag, ptr);
			}
			else {
				Toy_private_emitAstBinary(bucketHandle, rootHandle, flag, ptr);
			}
		}
	}

	//can't assign below a certain precedence
	if (precRule <= PREC_ASSIGNMENT && match(parser, TOY_TOKEN_OPERATOR_ASSIGN)) {
		printError(parser, parser->current, "Invalid assignment target");
	}
}

static void makeExpr(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle) {
	parsePrecedence(bucketHandle, parser, rootHandle, PREC_ASSIGNMENT);
}

//forward declarations
static void makeBlockStmt(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle);
static void makeDeclarationStmt(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle, bool errorOnEmpty);

static void makeAssertStmt(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle) {
	Toy_Ast* ast = NULL; //assert's emit function is a bit different
	makeExpr(bucketHandle, parser, &ast);

	//if assert is disabled, don't emit the assert
	if (parser->removeAssert) {
		Toy_private_emitAstPass(bucketHandle, rootHandle);
	}
	else {
		//NOTE: if it's an aggregate node, then it's got a second arg
		if (ast->type == TOY_AST_AGGREGATE) {
			Toy_private_emitAstAssert(bucketHandle, rootHandle, ast->aggregate.left, ast->aggregate.right);
		}
		else {
			Toy_private_emitAstAssert(bucketHandle, rootHandle, ast, NULL);
		}
	}

	consume(parser, TOY_TOKEN_OPERATOR_SEMICOLON, "Expected ';' at the end of assert statement");
}

static void makeIfThenElseStmt(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle) {
	Toy_Ast* condBranch = NULL;
	Toy_Ast* thenBranch = NULL;
	Toy_Ast* elseBranch = NULL;

	//if (condBranch)
	consume(parser, TOY_TOKEN_OPERATOR_PAREN_LEFT, "Expected '(' after 'if' keyword");
	makeExpr(bucketHandle, parser, &condBranch);
	consume(parser, TOY_TOKEN_OPERATOR_PAREN_RIGHT, "Expected ')' after 'if' condition");

	// { thenBranch }
	makeDeclarationStmt(bucketHandle, parser, &thenBranch, true);

	//else { elseBranch }
	if (match(parser, TOY_TOKEN_KEYWORD_ELSE)) {
		makeDeclarationStmt(bucketHandle, parser, &elseBranch, true);
	}

	Toy_private_emitAstIfThenElse(bucketHandle, rootHandle, condBranch, thenBranch, elseBranch);
}

static void makeWhileStmt(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle) {
	Toy_Ast* condBranch = NULL;
	Toy_Ast* thenBranch = NULL;

	//while (condBranch)
	consume(parser, TOY_TOKEN_OPERATOR_PAREN_LEFT, "Expected '(' after 'while' keyword");
	makeExpr(bucketHandle, parser, &condBranch);
	consume(parser, TOY_TOKEN_OPERATOR_PAREN_RIGHT, "Expected ')' after 'while' condition");

	// { thenBranch }
	makeDeclarationStmt(bucketHandle, parser, &thenBranch, true);

	Toy_private_emitAstWhileThen(bucketHandle, rootHandle, condBranch, thenBranch);
}

static void makeBreakStmt(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle) {
	Toy_private_emitAstBreak(bucketHandle, rootHandle);
	consume(parser, TOY_TOKEN_OPERATOR_SEMICOLON, "Expected ';' at the end of break statement");
}

static void makeContinueStmt(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle) {
	Toy_private_emitAstContinue(bucketHandle, rootHandle);
	consume(parser, TOY_TOKEN_OPERATOR_SEMICOLON, "Expected ';' at the end of continue statement");
}

static void makeReturnStmt(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle) {
	parsePrecedence(bucketHandle, parser, rootHandle, PREC_GROUP); //expect an aggregate
	Toy_private_emitAstReturn(bucketHandle, rootHandle);

	consume(parser, TOY_TOKEN_OPERATOR_SEMICOLON, "Expected ';' at the end of return statement");
}

static void makePrintStmt(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle) {
	makeExpr(bucketHandle, parser, rootHandle);
	Toy_private_emitAstPrint(bucketHandle, rootHandle);

	consume(parser, TOY_TOKEN_OPERATOR_SEMICOLON, "Expected ';' at the end of print statement");
}

static void makeExprStmt(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle) {
	makeExpr(bucketHandle, parser, rootHandle);
	consume(parser, TOY_TOKEN_OPERATOR_SEMICOLON, "Expected ';' at the end of expression statement");
}

static void makeVariableDeclarationStmt(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle) {
	consume(parser, TOY_TOKEN_NAME, "Expected variable name after 'var' keyword");

	if (parser->previous.length > 255) {
		printError(parser, parser->previous, "Can't have a variable name longer than 255 characters");
		Toy_private_emitAstError(bucketHandle, rootHandle);
		return;
	}

	Toy_Token nameToken = parser->previous;

	//read the type specifier if present
	Toy_ValueType varType = TOY_VALUE_ANY;
	bool constant = false;

	if (match(parser, TOY_TOKEN_OPERATOR_COLON)) {
		varType = readType(parser);

		if (match(parser, TOY_TOKEN_KEYWORD_CONST)) {
			constant = true;
		}
	}

	//build the name string
	Toy_String* nameStr = Toy_toStringLength(bucketHandle, nameToken.lexeme, nameToken.length);

	//if there's an assignment, read it, or default to null
	Toy_Ast* expr = NULL;
	if (match(parser, TOY_TOKEN_OPERATOR_ASSIGN)) {
		makeExpr(bucketHandle, parser, &expr);
	}
	else {
		Toy_private_emitAstValue(bucketHandle, &expr, TOY_VALUE_FROM_NULL());
	}

	//finally, emit the declaration as an Ast
	Toy_private_emitAstVariableDeclaration(bucketHandle, rootHandle, nameStr, varType, constant, expr);

	consume(parser, TOY_TOKEN_OPERATOR_SEMICOLON, "Expected ';' at the end of var statement");
}

static void makeFunctionDeclarationStmt(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle) {
	consume(parser, TOY_TOKEN_NAME, "Expected function name after 'fn' keyword");

	if (parser->previous.length > 255) {
		printError(parser, parser->previous, "Can't have a function name longer than 255 characters");
		Toy_private_emitAstError(bucketHandle, rootHandle);
		return;
	}

	//build the name string
	Toy_Token nameToken = parser->previous;
	Toy_String* nameStr = Toy_toStringLength(bucketHandle, nameToken.lexeme, nameToken.length);

	//read the function parameters (done manually to avoid other syntax issues)
	Toy_Ast* params = NULL;

	if (!match(parser, TOY_TOKEN_OPERATOR_PAREN_LEFT)) {
		printError(parser, parser->previous, "Expected '(' at the beginning of parameter list");
		Toy_private_emitAstError(bucketHandle, rootHandle);
		return;
	}

	unsigned int paramIterations = 0;

	while (parser->current.type != TOY_TOKEN_OPERATOR_PAREN_RIGHT && (paramIterations++ == 0 || match(parser, TOY_TOKEN_OPERATOR_COMMA))) {
		//grab the name token
		advance(parser);
		Toy_Token nameToken = parser->previous;

		//TODO: fix this with param type info
		//read the type specifier if present
		// Toy_ValueType varType = TOY_VALUE_ANY;
		// bool constant = true; //parameters are immutable

		if (match(parser, TOY_TOKEN_OPERATOR_COLON)) {
			// varType = readType(parser);
			readType(parser);

			if (match(parser, TOY_TOKEN_KEYWORD_CONST)) {
				// constant = true;
			}
		}

		//emit the parameter as a name string
		Toy_String* name = Toy_toStringLength(bucketHandle, nameToken.lexeme, nameToken.length);
		Toy_Value value = TOY_VALUE_FROM_STRING(name);
		Toy_Ast* ast = NULL;
		Toy_private_emitAstValue(bucketHandle, &ast, value); //TODO: params with type info

		//add to the params aggregate (is added backwards, because weird)
		Toy_private_emitAstAggregate(bucketHandle, &params, TOY_AST_FLAG_COLLECTION, ast);
	}

	consume(parser, TOY_TOKEN_OPERATOR_PAREN_RIGHT, "Expected ')' at the end of parameter list");

	//read the body
	consume(parser, TOY_TOKEN_OPERATOR_BRACE_LEFT, "Expected '{' at the beginning of function body");

	Toy_Ast* body = NULL;
	makeBlockStmt(bucketHandle, parser, &body);

	consume(parser, TOY_TOKEN_OPERATOR_BRACE_RIGHT, "Expected '}' at the end of function body");

	//finally, emit the declaration as an Ast
	Toy_private_emitAstFunctionDeclaration(bucketHandle, rootHandle, nameStr, params, body);
}

static void makeStmt(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle) {
	//inner scope
	if (match(parser, TOY_TOKEN_OPERATOR_BRACE_LEFT)) {
		makeBlockStmt(bucketHandle, parser, rootHandle);
		consume(parser, TOY_TOKEN_OPERATOR_BRACE_RIGHT, "Expected '}' at the end of block scope");
		(*rootHandle)->block.innerScope = true;
		return;
	}

	//assert
	else if (match(parser, TOY_TOKEN_KEYWORD_ASSERT)) {
		makeAssertStmt(bucketHandle, parser, rootHandle);
		return;
	}

	//if-then-else
	else if (match(parser, TOY_TOKEN_KEYWORD_IF)) {
		makeIfThenElseStmt(bucketHandle, parser, rootHandle);
		return;
	}

	//while-then
	else if (match(parser, TOY_TOKEN_KEYWORD_WHILE)) {
		makeWhileStmt(bucketHandle, parser, rootHandle);
		return;
	}

	//TODO: for-pre-clause-post-then

	//break
	else if (match(parser, TOY_TOKEN_KEYWORD_BREAK)) {
		makeBreakStmt(bucketHandle, parser, rootHandle);
		return;
	}

	//continue
	else if (match(parser, TOY_TOKEN_KEYWORD_CONTINUE)) {
		makeContinueStmt(bucketHandle, parser, rootHandle);
		return;
	}

	//return
	else if (match(parser, TOY_TOKEN_KEYWORD_RETURN)) {
		makeReturnStmt(bucketHandle, parser, rootHandle);
		return;
	}

	//TODO: import

	//print
	else if (match(parser, TOY_TOKEN_KEYWORD_PRINT)) {
		makePrintStmt(bucketHandle, parser, rootHandle);
		return;
	}

	//empty lines, or pass statements
	else if (match(parser, TOY_TOKEN_OPERATOR_SEMICOLON) || match(parser, TOY_TOKEN_KEYWORD_PASS)) {
		Toy_private_emitAstPass(bucketHandle, rootHandle);
		return;
	}

	//expressions
	else {
		//default
		makeExprStmt(bucketHandle, parser, rootHandle);
		return;
	}
}

static void makeDeclarationStmt(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle, bool errorOnEmpty) {
	//disallow empty control flow bodies
	if (errorOnEmpty && match(parser, TOY_TOKEN_OPERATOR_SEMICOLON)) {
		printError(parser, parser->previous, "Empty control flow bodies are disallowed, use the 'pass' keyword");
		Toy_private_emitAstError(bucketHandle, rootHandle);
		return;
	}

	//variable declarations
	else if (match(parser, TOY_TOKEN_KEYWORD_VAR)) {
		makeVariableDeclarationStmt(bucketHandle, parser, rootHandle);
	}

	//function declarations
	else if (match(parser, TOY_TOKEN_KEYWORD_FUNCTION)) {
		makeFunctionDeclarationStmt(bucketHandle, parser, rootHandle);
	}

	//otherwise
	else {
		makeStmt(bucketHandle, parser, rootHandle);
	}
}

static void makeBlockStmt(Toy_Bucket** bucketHandle, Toy_Parser* parser, Toy_Ast** rootHandle) {
	//begin the block
	Toy_private_initAstBlock(bucketHandle, rootHandle);

	//read a series of statements into the block
	while (parser->current.type != TOY_TOKEN_OPERATOR_BRACE_RIGHT && !match(parser, TOY_TOKEN_EOF)) {
		//process the grammar rules
		Toy_Ast* stmt = NULL;
		makeDeclarationStmt(bucketHandle, parser, &stmt, false);

		//if something went wrong
		if (parser->panic) {
			synchronize(parser);

			Toy_Ast* err = NULL;
			Toy_private_emitAstError(bucketHandle, &err);
			Toy_private_appendAstBlock(bucketHandle, *rootHandle, err);

			continue;
		}
		Toy_private_appendAstBlock(bucketHandle, *rootHandle, stmt);
	}
}

//exposed functions
void Toy_bindParser(Toy_Parser* parser, Toy_Lexer* lexer) {
	Toy_resetParser(parser);
	parser->lexer = lexer;
	advance(parser);
}

Toy_Ast* Toy_scanParser(Toy_Bucket** bucketHandle, Toy_Parser* parser) {
	Toy_Ast* rootHandle = NULL;

	//check for EOF
	if (match(parser, TOY_TOKEN_EOF)) {
		Toy_private_emitAstEnd(bucketHandle, &rootHandle);
		return rootHandle;
	}

	makeBlockStmt(bucketHandle, parser, &rootHandle);

	//don't emit this error if we're already panicking
	if (parser->panic != true && parser->previous.type != TOY_TOKEN_EOF) {
		printError(parser, parser->previous, "Expected 'EOF' and the end of the parser scan (possibly an extra '}' was found)");
	}

	return rootHandle;
}

void Toy_resetParser(Toy_Parser* parser) {
	parser->lexer = NULL;

	parser->current = ((Toy_Token){TOY_TOKEN_NULL, 0, 0, NULL});
	parser->previous = ((Toy_Token){TOY_TOKEN_NULL, 0, 0, NULL});

	parser->error = false;
	parser->panic = false;

	parser->removeAssert = false;
}

void Toy_configureParser(Toy_Parser* parser, bool removeAssert) {
	parser->removeAssert = removeAssert;
}
