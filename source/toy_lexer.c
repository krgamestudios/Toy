#include "toy_lexer.h"
#include "toy_console_colors.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

//keyword data
typedef struct {
	const Toy_TokenType type;
	const char* keyword;
} Toy_KeywordTypeTuple;

const Toy_KeywordTypeTuple keywordTuples[] = {
	//null
	{TOY_TOKEN_NULL, "null"},

	//types
	{TOY_TOKEN_TYPE_BOOLEAN, "bool"},
	{TOY_TOKEN_TYPE_INTEGER, "int"},
	{TOY_TOKEN_TYPE_FLOAT, "float"},
	{TOY_TOKEN_TYPE_STRING, "string"},
	{TOY_TOKEN_TYPE_ARRAY, "array"},
	{TOY_TOKEN_TYPE_TABLE, "table"},
	{TOY_TOKEN_TYPE_FUNCTION, "function"},
	{TOY_TOKEN_TYPE_OPAQUE, "opaque"},
	{TOY_TOKEN_TYPE_ANY, "any"},

	//keywords and reserved words
	{TOY_TOKEN_KEYWORD_AS, "as"},
	{TOY_TOKEN_KEYWORD_ASSERT, "assert"},
	{TOY_TOKEN_KEYWORD_BREAK, "break"},
	{TOY_TOKEN_KEYWORD_CLASS, "class"},
	{TOY_TOKEN_KEYWORD_CONST, "const"},
	{TOY_TOKEN_KEYWORD_CONTINUE, "continue"},
	{TOY_TOKEN_KEYWORD_DO, "do"},
	{TOY_TOKEN_KEYWORD_ELSE, "else"},
	{TOY_TOKEN_KEYWORD_EXPORT, "export"},
	{TOY_TOKEN_KEYWORD_FOR, "for"},
	{TOY_TOKEN_KEYWORD_FOREACH, "foreach"},
	{TOY_TOKEN_KEYWORD_FUNCTION, "fn"},
	{TOY_TOKEN_KEYWORD_IF, "if"},
	{TOY_TOKEN_KEYWORD_IMPORT, "import"},
	{TOY_TOKEN_KEYWORD_IN, "in"},
	{TOY_TOKEN_KEYWORD_OF, "of"},
	{TOY_TOKEN_KEYWORD_PASS, "pass"},
	{TOY_TOKEN_KEYWORD_PRINT, "print"},
	{TOY_TOKEN_KEYWORD_RETURN, "return"},
	{TOY_TOKEN_KEYWORD_VAR, "var"},
	{TOY_TOKEN_KEYWORD_WHILE, "while"},
	{TOY_TOKEN_KEYWORD_YIELD, "yield"},

	//literal values
	{TOY_TOKEN_LITERAL_TRUE, "true"},
	{TOY_TOKEN_LITERAL_FALSE, "false"},

	{TOY_TOKEN_EOF, NULL},
};

const char* Toy_private_findKeywordByType(const Toy_TokenType type) {
	if (type == TOY_TOKEN_EOF) {
		return "EOF";
	}

	for(int i = 0; keywordTuples[i].keyword; i++) {
		if (keywordTuples[i].type == type) {
			return keywordTuples[i].keyword;
		}
	}

	return NULL;
}

Toy_TokenType Toy_private_findTypeByKeyword(const char* keyword) {
	const int length = strlen(keyword);

	for (int i = 0; keywordTuples[i].keyword; i++) {
		if (!strncmp(keyword, keywordTuples[i].keyword, length)) {
			return keywordTuples[i].type;
		}
	}

	return TOY_TOKEN_EOF;
}

//static generic utility functions
static void cleanLexer(Toy_Lexer* lexer) {
	lexer->start = 0;
	lexer->current = 0;
	lexer->line = 1;
	lexer->source = NULL;
}

static bool isAtEnd(Toy_Lexer* lexer) {
	return lexer->source[lexer->current] == '\0';
}

static char peek(Toy_Lexer* lexer) {
	return lexer->source[lexer->current];
}

static char peekNext(Toy_Lexer* lexer) {
	if (isAtEnd(lexer)) return '\0';
	return lexer->source[lexer->current + 1];
}

static char advance(Toy_Lexer* lexer) {
	if (isAtEnd(lexer)) {
		return '\0';
	}

	//new line
	if (lexer->source[lexer->current] == '\n') {
		lexer->line++;
	}

	lexer->current++;
	return lexer->source[lexer->current - 1];
}

static void eatWhitespace(Toy_Lexer* lexer) {
	const char c = peek(lexer);

	switch(c) {
		case ' ':
		case '\r':
		case '\n':
		case '\t':
			advance(lexer);
			break;

		//comments
		case '/':
			//eat the line
			if (peekNext(lexer) == '/') {
				while (!isAtEnd(lexer) && advance(lexer) != '\n');
				break;
			}

			//eat the block
			if (peekNext(lexer) == '*') {
				advance(lexer);
				advance(lexer);
				while(!isAtEnd(lexer) && !(peek(lexer) == '*' && peekNext(lexer) == '/')) advance(lexer);
				advance(lexer);
				advance(lexer);
				break;
			}
		return;

		default:
			return;
	}

	//tail recursion
	eatWhitespace(lexer);
}

static bool isDigit(Toy_Lexer* lexer) {
	return peek(lexer) >= '0' && peek(lexer) <= '9';
}

static bool isAlpha(Toy_Lexer* lexer) {
	return
		(peek(lexer) >= 'A' && peek(lexer) <= 'Z') ||
		(peek(lexer) >= 'a' && peek(lexer) <= 'z') ||
		peek(lexer) == '_'
	;
}

static bool match(Toy_Lexer* lexer, char c) {
	if (peek(lexer) == c) {
		advance(lexer);
		return true;
	}

	return false;
}

//token generators
static Toy_Token makeErrorToken(Toy_Lexer* lexer, char* msg) {
	Toy_Token token;

	token.type = TOY_TOKEN_ERROR;
	token.length = strlen(msg);
	token.line = lexer->line;
	token.lexeme = msg;

	return token;
}

static Toy_Token makeToken(Toy_Lexer* lexer, Toy_TokenType type) {
	Toy_Token token;

	token.type = type;
	token.length = lexer->current - lexer->start;
	token.line = lexer->line;
	token.lexeme = &lexer->source[lexer->current - token.length];

	return token;
}

static Toy_Token makeIntegerOrFloat(Toy_Lexer* lexer) {
	Toy_TokenType type = TOY_TOKEN_LITERAL_INTEGER; //assume we're reading an integer

	//the character '_' can be inserted into numbers as a separator
	while(isDigit(lexer) || peek(lexer) == '_') advance(lexer);

	if (peek(lexer) == '.' && (peekNext(lexer) >= '0' && peekNext(lexer) <= '9')) { //peekNext(lexer) == digit
		type = TOY_TOKEN_LITERAL_FLOAT; //change the assumption to reading a float
		advance(lexer); //eat the '.'

		//'_' again
		while(isDigit(lexer) || peek(lexer) == '_') advance(lexer);
	}

	//make the token
	Toy_Token token;

	token.type = type;
	token.length = lexer->current - lexer->start;
	token.line = lexer->line;
	token.lexeme = &lexer->source[lexer->start];

	return token;
}

static bool isEscapableCharacter(char c) {
	switch (c) {
		case 'n':
		case 't':
		case '\\':
		case '"':
			return true;

		default:
			return false;
	}
}

static Toy_Token makeString(Toy_Lexer* lexer, char terminator) {
	while (!isAtEnd(lexer)) {
		//stop if you've hit the terminator
		if (peek(lexer) == terminator) {
			break;
		}

		//skip escaped control characters
		if (peek(lexer) == '\\' && isEscapableCharacter(peekNext(lexer))) {
			advance(lexer);
			advance(lexer);
			continue;
		}

		//otherwise
		advance(lexer);
	}

	if (isAtEnd(lexer)) {
		return makeErrorToken(lexer, "Unterminated string");
	}

	advance(lexer); //eat the terminator

	//make the token
	Toy_Token token;

	token.type = TOY_TOKEN_LITERAL_STRING;
	token.length = lexer->current - lexer->start - 2; //-1 to omit the quotes
	token.line = lexer->line;
	token.lexeme = &lexer->source[lexer->start + 1]; //+1 to omit the first quote

	return token;
}

static Toy_Token makeKeywordOrName(Toy_Lexer* lexer) {
	advance(lexer); //first letter can only be alpha

	while(isDigit(lexer) || isAlpha(lexer)) {
		advance(lexer);
	}

	//scan for a keyword
	for (int i = 0; keywordTuples[i].keyword; i++) {
		//WONTFIX: could squeeze miniscule performance gain from this, but ROI isn't worth it
		if (strlen(keywordTuples[i].keyword) == (lexer->current - lexer->start) && !strncmp(keywordTuples[i].keyword, &lexer->source[lexer->start], lexer->current - lexer->start)) {
			//make token (keyword)
			Toy_Token token;

			token.type = keywordTuples[i].type;
			token.length = lexer->current - lexer->start;
			token.line = lexer->line;
			token.lexeme = &lexer->source[lexer->start];

			return token;
		}
	}

	//make token (variable name)
	Toy_Token token;

	token.type = TOY_TOKEN_NAME;
	token.length = lexer->current - lexer->start;
	token.line = lexer->line;
	token.lexeme = &lexer->source[lexer->start];

	return token;
}

//exposed functions
void Toy_bindLexer(Toy_Lexer* lexer, const char* source) {
	cleanLexer(lexer);
	lexer->source = source;
}

Toy_Token Toy_private_scanLexer(Toy_Lexer* lexer) {
	if (lexer->source == NULL) {
		return makeErrorToken(lexer, "Missing source code in lexer");
	}

	eatWhitespace(lexer);

	lexer->start = lexer->current;

	if (isAtEnd(lexer)) return makeToken(lexer, TOY_TOKEN_EOF);

	if (isDigit(lexer)) return makeIntegerOrFloat(lexer);
	if (isAlpha(lexer)) return makeKeywordOrName(lexer);

	char c = advance(lexer);

	switch(c) {
		case '(': return makeToken(lexer, TOY_TOKEN_OPERATOR_PAREN_LEFT);
		case ')': return makeToken(lexer, TOY_TOKEN_OPERATOR_PAREN_RIGHT);
		case '[': return makeToken(lexer, TOY_TOKEN_OPERATOR_BRACKET_LEFT);
		case ']': return makeToken(lexer, TOY_TOKEN_OPERATOR_BRACKET_RIGHT);
		case '{': return makeToken(lexer, TOY_TOKEN_OPERATOR_BRACE_LEFT);
		case '}': return makeToken(lexer, TOY_TOKEN_OPERATOR_BRACE_RIGHT);

		case '+': return makeToken(lexer, match(lexer, '=') ? TOY_TOKEN_OPERATOR_ADD_ASSIGN : match(lexer, '+') ? TOY_TOKEN_OPERATOR_INCREMENT : TOY_TOKEN_OPERATOR_ADD);
		case '-': return makeToken(lexer, match(lexer, '=') ? TOY_TOKEN_OPERATOR_SUBTRACT_ASSIGN : match(lexer, '-') ? TOY_TOKEN_OPERATOR_DECREMENT : TOY_TOKEN_OPERATOR_SUBTRACT);
		case '*': return makeToken(lexer, match(lexer, '=') ? TOY_TOKEN_OPERATOR_MULTIPLY_ASSIGN : TOY_TOKEN_OPERATOR_MULTIPLY);
		case '/': return makeToken(lexer, match(lexer, '=') ? TOY_TOKEN_OPERATOR_DIVIDE_ASSIGN : TOY_TOKEN_OPERATOR_DIVIDE);
		case '%': return makeToken(lexer, match(lexer, '=') ? TOY_TOKEN_OPERATOR_MODULO_ASSIGN : 	TOY_TOKEN_OPERATOR_MODULO);

		case '!': return makeToken(lexer, match(lexer, '=') ? TOY_TOKEN_OPERATOR_COMPARE_NOT : TOY_TOKEN_OPERATOR_NEGATE);
		case '=': return makeToken(lexer, match(lexer, '=') ? TOY_TOKEN_OPERATOR_COMPARE_EQUAL : TOY_TOKEN_OPERATOR_ASSIGN);

		case '<': return makeToken(lexer, match(lexer, '=') ? TOY_TOKEN_OPERATOR_COMPARE_LESS_EQUAL : TOY_TOKEN_OPERATOR_COMPARE_LESS);
		case '>': return makeToken(lexer, match(lexer, '=') ? TOY_TOKEN_OPERATOR_COMPARE_GREATER_EQUAL : TOY_TOKEN_OPERATOR_COMPARE_GREATER);

		case '&': //TOY_TOKEN_OPERATOR_AMPERSAND is unused
			if (match(lexer, '&')) {
				return makeToken(lexer, TOY_TOKEN_OPERATOR_AND);
			} else {
				return makeErrorToken(lexer, "Unexpected '&'");
			}

		case '|': //TOY_TOKEN_OPERATOR_PIPE is unused
			if (match(lexer, '|')) {
				return makeToken(lexer, TOY_TOKEN_OPERATOR_OR);
			} else {
				return makeErrorToken(lexer, "Unexpected '|'");
			}

		case '?': return makeToken(lexer, TOY_TOKEN_OPERATOR_QUESTION);
		case ':': return makeToken(lexer, TOY_TOKEN_OPERATOR_COLON);
		case ';': return makeToken(lexer, TOY_TOKEN_OPERATOR_SEMICOLON);
		case ',': return makeToken(lexer, TOY_TOKEN_OPERATOR_COMMA);

		case '.':
			if (match(lexer, '.')) {
				if (match(lexer, '.')) {
					return makeToken(lexer, TOY_TOKEN_OPERATOR_REST); //three dots
				}
				else {
					return makeToken(lexer, TOY_TOKEN_OPERATOR_CONCAT); //two dots
				}
			}
			else {
				return makeToken(lexer, TOY_TOKEN_OPERATOR_DOT); //one dot
			}

		case '"':
			return makeString(lexer, c);

		default: {
			return makeErrorToken(lexer, "Unknown token value found in lexer");
		}
	}
}

static void trim(char** s, unsigned int* l) { //util
	while( isspace(( (*((unsigned char**)(s)))[(*l) - 1] )) ) (*l)--;
	while(**s && isspace( **(unsigned char**)(s)) ) { (*s)++; (*l)--; }
}

//for debugging
void Toy_private_printToken(Toy_Token* token) {
	//print errors
	if (token->type == TOY_TOKEN_ERROR) {
		printf(TOY_CC_ERROR "ERROR: \t%d\t%.*s\n" TOY_CC_RESET, (int)token->line, (int)token->length, token->lexeme);
		return;
	}

	//print the line number
	printf("\t%d\t%d\t", token->type, (int)token->line);

	//print based on type
	if (token->type == TOY_TOKEN_NAME || token->type == TOY_TOKEN_LITERAL_INTEGER || token->type == TOY_TOKEN_LITERAL_FLOAT || token->type == TOY_TOKEN_LITERAL_STRING) {
		printf("%.*s\t", (int)token->length, token->lexeme);
	} else {
		const char* keyword = Toy_private_findKeywordByType(token->type);

		if (keyword != NULL) {
			printf("%s", keyword);
		} else {
			char* str = (char*)token->lexeme; //strip const-ness for trimming
			unsigned int length = token->length;
			trim(&str, &length);
			printf("%.*s", (int)length, str);
		}
	}

	printf("\n");
}
