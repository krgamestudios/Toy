#include "toy_lexer.h"
#include "toy_console_colors.h"

#include <stdio.h>
#include <string.h>

int main(void) {
	{
		//source code sample to operate on
		char* source = "print null;";

		//test the lexer
		Toy_Lexer lexer;
		Toy_bindLexer(&lexer, source);

		//get each token
		Toy_Token print = Toy_private_scanLexer(&lexer);
		Toy_Token null = Toy_private_scanLexer(&lexer);
		Toy_Token semi = Toy_private_scanLexer(&lexer);
		Toy_Token eof = Toy_private_scanLexer(&lexer);

		//test each token is correct
		if (strncmp(print.lexeme, "print", print.length)) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: print lexeme is wrong: %s" TOY_CC_RESET, print.lexeme);
			return -1;
		}


		if (strncmp(null.lexeme, "null", null.length)) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: null lexeme is wrong: %s" TOY_CC_RESET, null.lexeme);
			return -1;
		}

		if (strncmp(semi.lexeme, ";", semi.length)) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: semicolon lexeme is wrong: %s" TOY_CC_RESET, semi.lexeme);
			return -1;
		}

		if (eof.type != TOY_TOKEN_EOF) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to find EOF token" TOY_CC_RESET);
			return -1;
		}
	}

	printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
	return 0;
}
