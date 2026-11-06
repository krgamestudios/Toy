#pragma once

#include "toy_common.h"

#include "toy_lexer.h"
#include "toy_ast.h"

typedef struct Toy_Parser {
	Toy_Lexer* lexer;

	//last two outputs
	Toy_Token current;
	Toy_Token previous;

	const char* workingDir;

	bool error;
	bool panic; //currently processing an error
} Toy_Parser;

TOY_API void Toy_bindParser(Toy_Parser* parser, Toy_Lexer* lexer);
TOY_API Toy_Ast* Toy_scanParser(Toy_Bucket** bucketHandle, Toy_Parser* parser);
TOY_API void Toy_resetParser(Toy_Parser* parser);

//NOTE: must be a working directory, ending with a slash
TOY_API void Toy_adjustParserWorkingDirectory(Toy_Parser* parser, const char* dir);

//The resolver returns an allocated path; the parser frees it. NULL is an error.
//Like the print callbacks, this hook is process-wide. NULL restores defaults.
typedef char* (*Toy_importResolver)(const char* name, const char* workingDir);
TOY_API void Toy_setImportResolver(Toy_importResolver resolver);

//URGENT: write a test for import resolvers