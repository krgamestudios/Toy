

# repl_tools.h

This header provides a number of tools for compiling and running Toy, and is used primarily by the REPL. However, it can also be modified and used by any host program with a little effort.

This is not a core part of Toy or a library, and as such `repl_tools.h` and `repl_tools.c` can both be found in the `repl/` folder.

## Defined Functions

### const char* Toy_readFile(const char* path, size_t* fileSize)

This function reads in a file, and returns it as a constant buffer. It also sets the variable pointed to by `fileSize` to the size of the given buffer.

On error, this function returns `NULL`.

### int Toy_writeFile(const char* path, const unsigned char* bytes, size_t size)

This function writes the buffer pointed to by `bytes` to a file specified by `path`. The buffer's size should be specified by `size`.

On error, this function returns a non-zero value.

### const unsigned char* Toy_compileString(const char* source, size_t* size)

This function takes a cstring of Toy source code, and returns a compiled buffer based on that source code. The variable pointed to by `size` is set to the size of the bytecode.

On error, this function returns `NULL`.

### void Toy_runBinary(const unsigned char* tb, size_t size)

This function takes a bytecode array of `size` size, and executes it. The libraries available to the code are currently:

* lib_about
* lib_standard
* lib_random
* lib_runner

### void Toy_runBinaryFile(const char* fname)

This function loads in the binary file specified by `fname`, and passes it to `Toy_runBinary()`.

### void Toy_runSource(const char* source)

This function compiles the source with `Toy_compileString()`, and passes it to `Toy_runBinary()`.

### void Toy_runSourceFile(const char* fname)

This function loads in the file specified by `fname`, compiles it, and passes it to `Toy_runBinary()`.

### void Toy_parseBinaryFileHeader(const char* fname)

This function parses the header information stored within the bytecode file `fname`.

This is only used for debugging and validation purposes.
