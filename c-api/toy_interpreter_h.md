
# toy_interpreter.h

This header defines the interpreter structure, which is the beating heart of Toy.

`Toy_Interpreter` is a stack-based, bytecode-driven interpreter with a number of customisation options, including "hooks"; native C functions wrapped in `Toy_Literal` instances, injected into the interpreter in order to give the Toy scripts access to libraries via the `import` keyword. The hooks, when invoked this way, can then inject further native functions into the interpreter's current scope. Exactly which hooks are made available varies by host program, but `standard` is the most commonly included one.

Another useful customisation feature is the ability to redirect output from the `print` and `assert` keywords, as well as any internal errors that occur. This can allow you to add in a logging system, or even hook the `print` statement up to some kind of HUD.

## Defined Interfaces

Note: These interfaces are *actually* defined in [toy_literal.h](toy_literal_h.md) but are documented here, because this is where it matters most.

### typedef void (*Toy_PrintFn)(const char*)

This is the interface used by "print functions" - that is, functions used to print messages from the `print` and `assert` keywords, as well as internal interpreter errors.

### typedef int (*Toy_NativeFn)(struct Toy_Interpreter* interpreter, struct Toy_LiteralArray* arguments)

This is the interface used by "native functions" - that is, functions written in C which can be called directly by Toy scripts.

The arguments to the function are passed in as a `Toy_LiteralArray`.

### typedef int (*Toy_HookFn)(struct Toy_Interpreter* interpreter, struct Toy_Literal identifier, struct Toy_Literal alias)

This is the interface used by "hook functions" - that is, functions written in C which are invoked by using the `import` keyword, and are intended to inject other native functions into the current scope. While hook functions are capable of doing other things, this is greatly discouraged.

The identifier of the library (its name) is passed in as a `Toy_Literal`, as is any given alias; if no alias is given, then `alias` will be a null literal. Here, the identifier is `standard`, while the alias is `std`.

```
import standard as std;
```

Conventionally, when an alias is given, all the functions should instead be inserted into a `Toy_LiteralDictionary` which is then inserted into the scope with the alias as its identifier.

## Defined Functions

### void Toy_initInterpreter(Toy_Interpreter* interpreter)

This function initializes the interpreter. It allocates memory for internal systems such as the stack, and zeroes-out systems that have yet to be invoked. Internally, it also invokes `Toy_resetInterpreter` to initialize the environment.

### void Toy_runInterpreter(Toy_Interpreter* interpreter, const unsigned char* bytecode, size_t length)

This function takes a `Toy_Interpreter` and `bytecode` (as well as the `length` of the bytecode), checks its version information, parses and un-flattens the literal cache, and executes the compiled program stored in the bytecode. This function also consumes the bytecode, so the `bytecode` argument is no longer valid after calls.

If the given bytecode's embedded version is not compatible with the current interpreter, then this function will refuse to execute.

Re-using a `Toy_Interpreter` instance without first resetting it is possible (that's how the REPL works), however doing so may have unintended consequences if the scripts are not intended to be used in such a way. Any variables declared will persist.

### void Toy_resetInterpreter(Toy_Interpreter* interpreter)

This function frees any scopes that the scripts have built up, and generates a new one. It also injects several globally available functions:

* set
* get
* push
* pop
* length
* clear

### void Toy_freeInterpreter(Toy_Interpreter* interpreter)

This function frees a `Toy_Interpreter`, clearing all the memory used within. That interpreter is no longer valid for use, and must be re-initialized.

### bool Toy_injectNativeFn(Toy_Interpreter* interpreter, const char* name, Toy_NativeFn func)

This function will inject the given native function `func` into the `Toy_Interpreter`'s current scope, with the identifier as `name`. Both the name and function will be converted into literals internally before being stored. It will return true on success, otherwise it will return false.

The primary use of this function is within hooks.

### bool Toy_injectNativeHook(Toy_Interpreter* interpreter, const char* name, Toy_HookFn hook)

This function will inject the given native function `hook` into the `Toy_Interpreter`'s hook cache, with the identifier as `name`. Both the name and the function will be converted into literals internally before being stored. It will return true on success, otherwise it will return false.

Hooks are invoked with the `import` keyword within Toy's scripts.

### bool Toy_callLiteralFn(Toy_Interpreter* interpreter, Toy_Literal func, Toy_LiteralArray* arguments, Toy_LiteralArray* returns)

This function calls a `Toy_Literal` which contains a function, with the arguments to that function passed in as `arguments` and the results stored in `returns`. It returns true on success, otherwise it returns false.

The literal `func` can be either a native function or a Toy function, but it won't execute a hook.

### bool Toy_callFn(Toy_Interpreter* interpreter, const char* name, Toy_LiteralArray* arguments, Toy_LiteralArray* returns)

This utility function will find a `Toy_literal` within the `Toy_Interpreter`'s scope with an identifier that matches `name`, and will invoke it using `Toy_callLiteralFn` (passing in `arguments` and `returns` as expected).

### bool Toy_parseIdentifierToValue(Toy_Interpreter* interpreter, Toy_Literal* literalPtr)

Sometimes, native functions will receive `Toy_Literal` identifiers instead of the values - the correct values can be retrieved from the given interpreter's scope using the following pattern:

```c
Toy_Literal foobarIdn = foobar;
if (TOY_IS_IDENTIFIER(foobar) && Toy_parseIdentifierToValue(interpreter, &foobar)) {
    freeLiteral(foobarIdn); //remember to free the identifier
}
```

### void Toy_setInterpreterPrint(Toy_Interpreter* interpreter, Toy_PrintFn printOutput)

This function sets the function called by the `print` keyword. By default, the following wrapper is used:

```c
static void printWrapper(const char* output) {
    printf("%s\n", output);
}
```

Note: The above is a very minor lie - in reality, there are some preprocessor directives to allow the REPL's `-n` flag to work.

### void Toy_setInterpreterAssert(Toy_Interpreter* interpreter, Toy_PrintFn assertOutput)

This function sets the function called by the `assert` keyword on failure. By default, the following wrapper is used:

```c
static void assertWrapper(const char* output) {
    fprintf(stderr, "Assertion failure: %s\n", output);
}
```

### void Toy_setInterpreterError(Toy_Interpreter* interpreter, Toy_PrintFn errorOutput)

This function sets the function called when an error occurs within the interpreter. By default, the following wrapper is used:

```c
static void errorWrapper(const char* output) {
    fprintf(stderr, "%s", output); //no newline
}
```
