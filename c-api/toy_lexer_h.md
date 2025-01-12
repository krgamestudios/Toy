
# toy_lexer.h

This header defines the lexer and token structures, which can be bound to a piece of source code, and used to tokenize it within a parser.

## Defined Functions

### void Toy_initLexer(Toy_Lexer* lexer, const char* source)

This function initializes a lexer, binding it to the `source` parameter; the lexer is now ready to be passed to the parser.

### Toy_Token Toy_private_scanLexer(Toy_Lexer* lexer)

This function "scans" the lexer, returning a token to the parser.

Private functions are not intended for general use.

### void Toy_private_printToken(Toy_Token* token)

This function prints a given token to stdout.

Private functions are not intended for general use.

### void Toy_private_setComments(Toy_Lexer* lexer, bool enabled)

This function sets whether comments are allowed within source code. By default, comments are allowed, and are only disabled in the REPL.

Private functions are not intended for general use.
