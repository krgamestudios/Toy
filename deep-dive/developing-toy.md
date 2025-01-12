# Developing Toy

Here you'll find some of the implementation details.

# Bytecode

The output of Toy's compiler, and the input of the interpreter, is known as "bytecode". Here, I've attempted to fully document the layout of the canonical bytecode's structure, but since this was written after most of this was implemented, there may be small discrepancies present.

There are four main sections of the bytecode:

* Header
* Literal Cache
* Function Definitions
* Program Definition

## Bytecode Header Format

Note: The bytecode header format must not change.

This section is used to define what version of Toy is currently running, as well as to prevent any version/fork clashes.

The header consists of four values:

* TOY_VERSION_MAJOR
* TOY_VERSION_MINOR
* TOY_VERSION_PATCH
* TOY_VERSION_BUILD

The first three are single unsigned bytes, embedded at the beginning of the bytecode in sequence. These represent the major, minor and patch versions of the language. The fourth value is a null-terminated c-string of unspecified data, which is *intended* but not required to specify the time that the language's compiler was itself compiled. The build string can hold arbitrary data, such as the current maintainer's name, current fork of the language, or other versioning info.

There are some strict rules when interpreting these values (mimicking, but not conforming to [semver.org](https://semver.org/)):

* Under no circumstance, should you ever run bytecode whose major version is different - there are definitely broken APIs involved.
* Under no circumstance, should you ever run bytecode whose minor version is above the interpreter's minor version - the bytecode could potentially use unimplemented features.
* You may, at your own risk, attempt to run bytecode whose patch version is different.
* You may, at your own risk, attempt to run bytecode whose build version is different.

All interpreter implementations retain the right to reject any bytecode whose header data does not conform to the above specification.

The latest version information can be found in [toy_common.h](https://github.com/Ratstail91/Toy/blob/main/source/toy_common.h)

## Literal Cache

In Toy, a "Literal" is a value of some kind, be it an integer, or a dictionary, or even a variable name. Rather than embedding the same literal (potentially) many times within the bytecode, the "Literal Cache" was devised to act as an immutable, indexable repository of any literals needed. When bytecode is first loaded into the interpreter, the first thing that happens (after the header is parsed) is the reconstruction of the literal cache. The internal function `readInterpreterSections()` is responsible for this step.

The first `unsigned short` to be read from this section is `literalCount`, which defines the number of literals which are to be read. Once all literals have been read out of this section, the opcode `TOY_OP_SECTION_END` is expected to be consumed. Some preprocessor macros can also enable or disable debug printing functionality within the REPL.

The list of valid literal types are:


### TOY_LITERAL_NULL

This literal is simply inserted into the literal cache when encountered.

### TOY_LITERAL_BOOLEAN

This literal specifies that the next byte is its value, either true or false.

### TOY_LITERAL_INTEGER

This literal specifies that the next 4 bytes are its value, interpreted as a 32-bit integer.

### TOY_LITERAL_FLOAT

This literal specifies that the next 4 bytes are its value, interpreted as a 32-bit floating point integer.

### TOY_LITERAL_STRING

This literal specifies that the next collection of null terminated bytes are its value, interpreted as a null-terminated string.

### TOY_LITERAL_ARRAY_INTERMEDIATE

`TOY_LITERAL_ARRAY_INTERMEDIATE` specifies that the literal to be read is a flattened `LiteralArray`. A "flattened" compound literal does not actually store its contents, only references to its contents' positions within the literal cache.

To read this array, you must first read an `unsigned short` which specifies the size, then read that many additional `unsigned shorts`, which are indices. Finally, the original `LiteralArray` can be reconstructed using those indices, in order.

As the final step, the newly reconstructed `LiteralArray` is added to the literal cache.

### TOY_LITERAL_DICTIONARY_INTERMEDIATE

`TOY_LITERAL_DICTIONARY_INTERMEDIATE` specifies that the literal to be read is a flattened `LiteralDictionary`. A "flattened" compound literal does not actually store its contents, only references to its contents' positions within the literal cache.

To read this dictionary, you must first read an `unsigned short` which specifies the size (both keys and values), then read that many additional `unsigned shorts`, which are indices of keys and values. Finally, the original `LiteralDictionary` can be reconstructed using those key and value indices.

As the final step, the newly reconstructed `LiteralDictionary` is added to the literal cache.

### TOY_LITERAL_FUNCTION

When a `TOY_LITERAL_FUNCTION` is encountered, the next `unsigned short` to be read (the function index) should be converted into an integer literal, before having its type manually changed to `TOY_LITERAL_FUNCTION_INTERMEDIATE` for storage within the literal cache.

Functions will be processed properly in a later step - so this literal is added to the cache as a placeholder until that point.

### TOY_LITERAL_IDENTIFIER

This literal specifies that the next collection of null terminated bytes are its value, interpreted as a null-terminated string.

### TOY_LITERAL_TYPE

This literal specifies that the next byte is the type of a literal, and the following byte is a boolean specifying const-ness.

(This literal type may be integrated with `TOY_LITERAL_TYPE_INTERMEDIATE` at some point.)

### TOY_LITERAL_TYPE_INTERMEDIATE

This literal specifies that the next byte is the type of a literal, and the following byte is a boolean specifying const-ness.

Then, if the type is `TOY_LITERAL_ARRAY`, the following `unsigned short` is an index within the cache, representing the type of the contents.

Otherwise, if the type is `TOY_LITERAL_DICTIONARY`, the following two `unsigned short`s are indices within the cache, representing the types of the keys and values.

### TOY_LITERAL_INDEX_BLANK

This literal is simply inserted into the literal cache when encountered.

## Function Definitions

The second stage of `readInterpreterSections()` is used to read the third section of the given bytecode - the function definitions.

The first `unsigned short` is the number of functions present within this section. The second `unsigned short` is the length of this entire section (this one is not necessarily needed, and may be removed at some point).

For each `TOY_LITERAL_FUNCTION_INTERMEDIATE` within the cache, you must read an `unsigned short` as the size. Then, the following `size` block of bytecode is to be copied, wholesale, into the specified cached literal, before setting that literal's type to `TOY_LITERAL_FUNCTION`. While the function is not operational yet, it will be further processed when needed.

Once all function literals have been read out of this section, the opcode `TOY_OP_SECTION_END` is expected to be consumed.

## Program Definition

TODO

### Opcodes

TODO: finish these

|Toy_Opcode|Value|Description|
|---|---|---|
|TOY_OP_EOF|0|End of File signal. Used internally as an OK signal.|
|TOY_OP_PASS|1|Does nothing - a no-op.|
|TOY_OP_ASSERT|2|Pops `rhs`, `lhs`\* from the stack. `rhs` must be a string, identifiers are not accepted. If `lhs` is `null` or `false`, prints `rhs` to the assert output, and cancels the script.|
|TOY_OP_PRINT|3|Pops one\* literal from the stack, and prints it to the print output.|
|TOY_OP_LITERAL|4|Read one byte as an index; retrieve the given index's literal from the cache, and push it to the stack.|
|TOY_OP_LITERAL_LONG|5|Same as `TOY_OP_LITERAL`, except you read two bytes at a time.|
|TOY_OP_LITERAL_RAW|6|Pops one literal from the stack. If it's an identifier, explicitly replace it with it's value. Push the literal to the stack.|
|TOY_OP_NEGATE|7|Pops one\* literal from the stack. If it's an integer or float\*\*, push the negative value to the stack.|
|TOY_OP_ADDITION|8|Pops `rhs`\*, `lhs`\* from the stack. If both are strings, attempts to concatenate them, and push the result to the stack\*\*. Otherwise, will attempt\*\* to add `lhs` to `rhs` (coercing integers to floats if needed), and push the result to the stack.|
|TOY_OP_SUBTRACTION|9|Pops `rhs`\*, `lhs`\* from the stack. Attempts\*\* to subtract `rhs` from `lhs` (coercing integers to floats if needed), and push the result to the stack.|
|TOY_OP_MULTIPLICATION|10|Pops `rhs`\*, `lhs`\* from the stack. Attempts\*\* to multiply `lhs` by `rhs` (coercing integers to floats if needed), and push the result to the stack.|
|TOY_OP_DIVISION|11|Pops `rhs`\*, `lhs`\* from the stack. Attempts\*\* to divide `lhs` by `rhs` (coercing integers to floats if needed), and push the result to the stack.|
|TOY_OP_MODULO|12|Pops `rhs`\*, `lhs`\* from the stack. Attempts\*\* to modulo `lhs` by `rhs`, and push the result to the stack.|
|TOY_OP_GROUPING_BEGIN|13|Begin operating on a group; analagous to the `(` operator.|
|TOY_OP_GROUPING_END|14|Finish operating on a group; analagous to the `)` operator.|
|TOY_OP_SCOPE_BEGIN|15|Push an inner-scope onto the stack; analogous to the `{` operator.|
|TOY_OP_SCOPE_END|16|Push an inner-scope off of the stack; analogous to the `}` operator.|
|TOY_OP_TYPE_DECL|17|Not used.|
|TOY_OP_TYPE_DECL_LONG|18|Not used.|
|TOY_OP_VAR_DECL|19|Read one byte, retrieving that literal from the cache as `identifier`, and then read the next byte, retrieving that literal from the cache as `type`. Attempt\*\* to declare a variable with `identifier` as its name and `type` as its type, then pop the top of the stack\* as it's value (cooerce integer values to floats if `type` specifies a float).|
|TOY_OP_VAR_DECL_LONG|20|Same as `TOY_OP_VAR_DECL`, except you read two bytes at a time.|
|TOY_OP_FN_DECL|21|Read one byte, retrieving that literal from the cache as `identifier`, and then read the next byte, retrieving that literal from the cache as `function`. Attempt\*\* to declare a variable with `identifier` as its name and `fn const` as it's type, then set `function` as its value. The function's scope has the current scope as its ancestor.|
|TOY_OP_FN_DECL_LONG|22|Same as `TOY_OP_FN_DECL`, except you read two bytes at a time.|
|TOY_OP_VAR_ASSIGN|23|Pops `rhs`*, `lhs` from the stack. `lhs` must be an identifier. Attempts\*\* to store the value of `rhs`, with `lhs` as it's identifier.|
|TOY_OP_VAR_ADDITION_ASSIGN|24|Equivilent of `TOY_OP_ADDITION` followed by `TOY_OP_VAR_ASSIGN`.|
|TOY_OP_VAR_SUBTRACTION_ASSIGN|25|Equivilent of `TOY_OP_SUBTRACTION` followed by `TOY_OP_VAR_ASSIGN`.|
|TOY_OP_VAR_MULTIPLICATION_ASSIGN|26|Equivilent of `TOY_OP_MULTIPLICATION` followed by `TOY_OP_VAR_ASSIGN`.|
|TOY_OP_VAR_DIVISION_ASSIGN|27|Equivilent of `TOY_OP_DIVISION` followed by `TOY_OP_VAR_ASSIGN`.|
|TOY_OP_VAR_MODULO_ASSIGN|28|Equivilent of `TOY_OP_MODULO` followed by `TOY_OP_VAR_ASSIGN`.|
|TOY_OP_TYPE_CAST|29|Pops `val`\*, `type`\* from the stack. Attempts** to change the type of `val` to `type`, and push the result to the stack. Only works on `bool`, `int`, `float` and `string` types, and only if the value is in the correct format.|
|TOY_OP_TYPE_OF|30|Pops `rhs` from the stack. If `rhs` is an identifier, it determines the the type of the variable specified, otherwise it determines the type of `rhs` directly. The result is pushed to the stack.|
|TOY_OP_IMPORT|31|Pops `alias` and `identifier` from the stack. Attempts\*\* to invoke a hook specified by `identifier`, with the given `alias`.|
|TOY_OP_EXPORT_removed|32|Removed.|
|TOY_OP_INDEX|33|Pops `third`\*, `second`\*, `first`\* and `compound`\* from the stack. Pushes the value specified by `compound[first : second : third]` onto the stack.
|TOY_OP_INDEX_ASSIGN|34|Pops `assign`\*, `third`\*, `second`\*, `first`\* and `compound`\* from the stack. Sets the value specified by `compound[first : second : third]` to be `assign`.|
|TOY_OP_INDEX_ASSIGN_INTERMEDIATE|35|Due to the unfortunately bonkers nature of indexing, this is needed as part of `TOY_OP_INDEX_ASSIGN`. It acts the same as `TOY_OP_INDEX`, but leaves an additional copy of some variables on the stack for `TOY_OP_INDEX_ASSIGN` to process.
|TOY_OP_DOT|36|
|TOY_OP_COMPARE_EQUAL|37|
|TOY_OP_COMPARE_NOT_EQUAL|38|
|TOY_OP_COMPARE_LESS|39|
|TOY_OP_COMPARE_LESS_EQUAL|40|
|TOY_OP_COMPARE_GREATER|41|
|TOY_OP_COMPARE_GREATER_EQUAL|42|
|TOY_OP_INVERT|43|
|TOY_OP_AND|44|
|TOY_OP_OR|45|
|TOY_OP_JUMP|46|
|TOY_OP_IF_FALSE_JUMP|47|
|TOY_OP_FN_CALL|48|
|TOY_OP_FN_RETURN|49|
|TOY_OP_POP_STACK|50|
|TOY_OP_TERNARY|51|
|TOY_OP_FN_END|52|
|TOY_OP_SECTION_END|255|
|TOY_OP_PREFIX|256|Used internally.|
|TOY_OP_POSTFIX|257|Used internally.|


\*If this literal is an identifier, it is instead replaced with the correct given value from the current scope.  
\*\*On failure, the script will print an error message to the error output and exit.  

## Function Internal Structure

TODO: loose first argument, args & returns counters in the program space

# Parser Structure and Operations

TODO

# Compiler Structure and Operations

TODO

# Interpreter Structure and Operations

The Toy interpreter is, at it's core, just a big loop that reads bytes from memory and acts on them. Here, I'll break down exactly how it works, from a top-down perspective.

## Running the Interpreter

There are four main functions for running the interpreter:

* `Toy_initInterpreter`
* `Toy_runInterpreter`
* `Toy_resetInterpreter`
* `Toy_freeInterpreter`

First, `init` zeroes out the interpreter, sets up the printing functions, and delegates to `reset`, which in turn sets up the program's scope (and injects the default global functions). The initialization function is split into two this way so that `reset` can be used independently on a "dirty" interpreter to ready it for another script (or another run of the same script). `reset` is usually not needed and may be removed in future.

`free` simply frees the interpreter after execution.

Interestingly, `run` doesn't jump straight into execution. Instead, it first does its own bit of setup, before reading out the bytecode's header. If the header indicates an incompatible version, then the interpreter will refuse to run, to prevent mistakes from ruining the program.

`run` will also delegate to a function called `readInterpreterSections()`, which reads and reconstructs the "literalCache" - a collection of all values within the program (variable identifiers, variable values, function bytecode, etc.)

Next, `run` will pass to a function called `execInterpreter()`, which contains the program's loop.

Finally, `run` will automatically free the bytecode and associated literalCache (this may change at some point).

## Executing the Interpreter

Opcodes within the bytecode are 1 byte in length, and specify a single action to take. Each possible action is defined within the interpreter in a function that begins with `exec`, and are called from within a big looping switch statement. If any of these `exec` functions encounters an error, they can simply return false to break the loop.

The interpreter is stack-based; most, if not all, the actions are preformed on literals within a specially designated array called `stack`. For example:

```c
	case TOY_OP_PRINT:
		if (!execPrint(interpreter)) {
			return;
		}
	break;
```

When the opcode `TOY_OP_PRINT` is encountered, the top literal within the stack is popped off, and printed (more info on literals below).

```c
static bool execPrint(Toy_Interpreter* interpreter) {
	//get the top literal
	Toy_Literal lit = Toy_popLiteralArray(&interpreter->stack);

	//if the top literal is an identifier, get it's value
	Toy_Literal idn = lit;
	if (TOY_IS_IDENTIFIER(lit) && Toy_parseIdentifierToValue(interpreter, &lit)) {
		Toy_freeLiteral(idn);
	}

	//print as a string to the current print method
	Toy_printLiteralCustom(lit, interpreter->printOutput);

	//free the literal
	Toy_freeLiteral(lit);

	//continue the loop
	return true;
}
```

## Identity Crisis

As in most programming languages, variables can be represented by names specified by the programmer; in Toy, these are called "identifiers". These identifiers can be passed around in place of their actual values, but can't be used directly. To retrieve a value, you must first "parse" it, like so:

```c
Toy_Literal idn = literal; //cache the literal, just in case it's an identifier
if (TOY_IS_IDENTIFIER(literal) && Toy_parseIdentifierToValue(interpreter, &literal)) { //if it is an identifier, parse it...
	Toy_freeLiteral(idn); //always remember to free the original identifier, otherwise you'll have a memory leak!
}
```

You will often see this pattern throughout the codebase.

## Other Utility Functions

Other functions are available at the top of the interpreter source file:

* printing utilities
* injection utilities
* parsing utilities
* bytecode utilities
* function utilities (these are at the very bottom of the source file)

# Literals

TODO

# Arrays & Dictionaries

TODO