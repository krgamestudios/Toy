
# toy_parser.h

This header defines the parser structure which, after being initialized with a lexer produces a series of abstract syntax trees to be passed to the compiler. The following is a utility function provided by [repl_tools.h](repl_tools_h.md), demonstrating how to use the parser.

```c
//generate bytecode from a given string
const unsigned char* Toy_compileString(const char* source, size_t* size) {
	//declare the relevant instances
	Toy_Lexer lexer;
	Toy_Parser parser;
	Toy_Compiler compiler;

	//initialize each of them
	Toy_initLexer(&lexer, source);
	Toy_initParser(&parser, &lexer);
	Toy_initCompiler(&compiler);

	//when the parser returns NULL, it is finished
	Toy_ASTNode* node = Toy_scanParser(&parser);
	while(node != NULL) {
		//if the parser returns an error node, clean up and exit gracefully
		if (node->type == TOY_AST_NODE_ERROR) {
			Toy_freeASTNode(node);
			Toy_freeCompiler(&compiler);
			Toy_freeParser(&parser);
			//no need to clean the lexer
			return NULL;
		}

		//write the node to the compiler
		Toy_writeCompiler(&compiler, node);
		Toy_freeASTNode(node);

		//grab the next node
		node = Toy_scanParser(&parser);
	}

	//get the bytecode to be returned
	const unsigned char* tb = Toy_collateCompiler(&compiler, size);

	//cleanup
	Toy_freeCompiler(&compiler);
	Toy_freeParser(&parser);
	//no need to clean the lexer

	//finally
	return tb;
}
```

## Defined Functions

### void Toy_initParser(Toy_Parser* parser, Toy_Lexer* lexer)

This function initializes a `Toy_Parser`, binding the given `Toy_Lexer` to it.

### void Toy_freeParser(Toy_Parser* parser)

This function frees a `Toy_Parser` once its task is completed.

### Toy_ASTNode* Toy_scanParser(Toy_Parser* parser)

This function returns an abstract syntax tree representing part of the program, or an error node. The abstract syntax tree must be passed to `Toy_writeCompiler()` and/or `Toy_freeASTNode()`.

This function should be called repeatedly until it returns `NULL`, indicating the end of the program.

### void Toy_freeASTNode(Toy_ASTNode* node)

This function cleans up any valid instance of `Toy_ASTNode` pointer passed to it. It is most commonly used to clean up the values returned by `Toy_scanParser`, after they have been passed to `Toy_writeCompiler`, or when the node is an error node.

Note: this function is *actually* defined in toy_ast_node.h, but documented here, because this is where it matters most.
