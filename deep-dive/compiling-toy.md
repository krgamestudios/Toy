
# Compiling Toy

This tutorial is a subsection of [Embedding Toy](deep-dive/embedding-toy) that has been spun off into its own page for the sake of brevity/sanity. It's recommended that you read the main article first.

The exact phases outlined here are entirely implementation-dependent - that is, they aren't required, and are simply how the canonical implementation of Toy works.

## How the Compilation works

There are four main phases to running a Toy source file. These are:

```
lexing -> parsing -> compiling -> interpreting
```

Each phase has a dedicated set of functions and structures, as well as intermediate structures between these that carry information.

```
source   -> lexer       -> token
token    -> parser      -> AST
AST      -> compiler    -> bytecode
bytecode -> interpreter -> result
```

## Lexer

Exactly how the source code is loaded into a C-string is left up to the user, however once it's loaded, it can be bound to a `Toy_Lexer` structure.

```c
Toy_Lexer lexer;
Toy_initLexer(&lexer, source);
```

The lexer, when invoked, will break down the string of characters into individual `Tokens`.

The lexer does not need to be freed after use, however the source code does.

## Parser

The `Toy_Parser` structure takes a `Toy_Lexer` as an argument when initialized.

```c
Toy_Parser parser; 
Toy_initParser(&parser, &lexer);

Toy_ASTNode* node = Toy_scanParser(&parser);

Toy_freeParser(&parser);
```

The parser pumps the lexer for tokens, one at a time, and converts them into structures called Abstract Syntax Trees (or ASTs for short). Each AST represents a single top-level statement within the Toy script. You'll know when the parser is finished with the lexer's source when `Toy_scanParser()` begins returning `NULL` pointers.

The AST Nodes produced by `Toy_scanParser()` must be freed manually, and the parser itself should not be used again.

## Compiler

The actual compilation phase has two steps - instruction writing and collation.

```c
size_t size;
Toy_Compiler compiler;

Toy_initCompiler(&compiler);
Toy_writeCompiler(&compiler, node); //node is an Toy_ASTNode

unsigned char* tb = Toy_collateCompiler(&compiler, &size);

Toy_freeCompiler(&compiler);
```

The writing step is the process in which AST nodes are compressed into bytecode instructions, while literal values are extracted and placed aside in a cache (usually in a compressed, intermediate state).

The collation phase, however, is when the bytecode instructions, along with the now flattened intermediate literals and function bodies are combined. The bytecode header specified in [Developing Toy](deep-dive/developing-toy) is placed at the beginning of this blob of bytes during this step.

The Toy bytecode (abbreviated to `tb`), along with the `size` variable indicating the size of the bytecode, are the result of the compilation. This bytecode can be saved into a file for later consumption by the host at runtime - you must ensure that any bytecode files have the `.tb` extension.

Alternatively, the bytecode in memory can be passed directly to the interpreter.

## Interpreter

The interpreter acts based on the contents of the bytecode given to it.

```c
Toy_Interpreter interpreter;
Toy_initInterpreter(&interpreter);
Toy_runInterpreter(&interpreter, tb, size);
Toy_freeInterpreter(&interpreter);
```

Exactly how it accomplishes this task is implementation dependant - as long as the results match expectations.

## REPL

An example program, called `toyrepl`, is provided alongside Toy's core. This program can handle many things, such as loading, compiling and executing Toy scripts; it's capable of compiling any valid Toy program for later use, even those that rely on non-standard libraries. It also has a number of commonly needed libraries provided.

To get a list of options, run `toyrepl -h`.

