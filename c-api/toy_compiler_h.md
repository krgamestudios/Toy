
# toy_compiler.h

This header defines the compiler structure, which is used to transform abstract syntax trees into usable intermediate bytecode. There are two steps to generating bytecode - the writing step, and the collation step.

During the writing step, the core of the program is generated, along with a series of literals representing the values within the program; these values are compressed and flattened into semi-unrecognizable forms. If the same literal is used multiple times in a program, such as a variable name, the name itself is replaced by a reference to the flattened literals within the cache.

During the collation step, everything from the core program’s execution instructions, the flattened literals, the functions (which have their own sections and protocols within the bytecode) and version information (such as the macros defined in toy_common.h) are all combined into a single buffer of bytes, known as bytecode. This bytecode can then be safely saved to a file or immediately executed.

## Define Functions

Executing the following functions out-of-order causes undefined behaviour.

### void Toy_initCompiler(Toy_Compiler* compiler)

This function initializes the given compiler.

### void Toy_writeCompiler(Toy_Compiler* compiler, Toy_ASTNode* node)

This function writes the given `node` argument to the compiler. During the writing step, this function may be called repeatedly, with a stream of results from `Toy_scanParser()`, until `Toy_scanParser()` returns `NULL`.

### unsigned char* Toy_collateCompiler(Toy_Compiler* compiler, size_t* size)

This function returns a buffer of bytes, known as "bytecode", created from the given compiler; it also stores the size of the bytecode in the variable pointed to by `size`.

Calling `Toy_collateCompiler()` multiple times on the same compiler will produce undefined behaviour.

### void Toy_freeCompiler(Toy_Compiler* compiler)

This function frees a compiler. Calling this on a compiler which has not been collated will free that compiler as expected - anything written to it will be lost.
