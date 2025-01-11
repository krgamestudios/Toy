
# Embedding Toy

This tutorial assumes that you've managed to embed Toy into your program by following the tutorial [Building Toy](deep-dive/building-toy).

Here, we'll look at some ways in which you can utilize Toy's C API within your host program.

Be aware that when you create a new Literal object, you must call `Toy_freeLiteral()` on it afterwards! If you don't, your program will leak memory as Toy has no internal tracker for such things.

## Embedded API Macros

The functions intended for usage by the API are prepended with the C macro `TOY_API`. The exact value of this macro can vary by platform, or even be empty. In addition, the macros defined in [literal.h](https://github.com/Ratstail91/Toy/blob/main/source/toy_literal.h) are available for use when manipulating literals. These include:

* `TOY_IS_*` - check if a literal is a specific type
* `TOY_AS_*` - cast the literal to a specific type
* `TOY_TO_*_LITERAL` - create a literal of a specific type
* `TOY_IS_TRUTHY` - check if a literal is truthy
* `TOY_MAX_STRING_LENGTH` - the maximum length of a string in Toy (can be altered if needed)

## Structures Used Throughout Toy

The main unit of data within Toy's internals is `Toy_Literal`, which can contain any value that can exist within the Toy language - even identifiers. The exact implementation of `Toy_Literal` may change or evolve as time goes on, so it's recommended that you only interact with literals directly by using the macros and functions outlined [above](#embedded-api-macros). See the [types](getting-started/types) page for information on exactly what data types exist in Toy.

There are two main "compound structures" used within Toy's internals - the `Toy_LiteralArray` and `Toy_LiteralDictionary`. The former is an array of `Toy_Literal` instances stored sequentially in memory for fast lookups, while the latter is a key-value hashmap designed for efficient lookups based on a `Toy_Literal` key. These are both accessible via the language as well.

These compound structures hold **copies** of literals given to them, rather than taking ownership of existing literals.

## Compiling Toy Scripts

Please see [Compiling Toy](deep-dive/compiling-toy) for more information on the process of turning scripts into bytecode.

## Interpreting Toy

The `Toy_Interpreter` structure is the beating heart of Toy - You'll usually only need one interpreter, as it can be reset as needed.

The four basic functions are used as follows:

```c
//assume "tb" and "size" are the results of compilation
Toy_Interpreter interpreter;

Toy_initInterpreter(&interpreter);
Toy_runInterpreter(&interpreter, tb, size);
Toy_resetInterpreter(&interpreter); //You usually want to reset between runs with the same interpreter
Toy_freeInterpreter(&interpreter);
```

In addition to this, you might also wish to "inject" a series of usable libraries into the interpreter, which can be `import`-ed within the language itself. This process only needs to be done once, after initialization, but before the first run.

```c
Toy_injectNativeHook(&interpreter, "standard", Toy_hookStandard);
```

A "hook" is a callback function which is invoked when the given library is imported. `standard` is the most commonly used library available.

```
import standard;
```

Hooks can simply inject native functions into the current scope, or they can do other, more esoteric things (though this is not recommended).

```c
//a utility structure for storing the native C functions
typedef struct Natives {
    char* name;
    Toy_NativeFn fn;
} Natives;

int Toy_hookStandard(Toy_Interpreter* interpreter, Toy_Literal identifier, Toy_Literal alias) {
    //the list of available native C functions that can be called from Toy
    Natives natives[] = {
        {"clock", nativeClock},
        {NULL, NULL}
    };

    //inject each native C functions into the current scope
    for (int i = 0; natives[i].name; i++) {
        Toy_injectNativeFn(interpreter, natives[i].name, natives[i].fn);
    }

    return 0;
}
```

## Calling Toy from C

In some situations, you may find it convenient to call a function written in Toy from the host program. For this, a pair of utility functions have been provided:

```c
TOY_API bool Toy_callLiteralFn(Toy_Interpreter* interpreter, Toy_Literal func, Toy_LiteralArray* arguments, Toy_LiteralArray* returns);
TOY_API bool Toy_callFn       (Toy_Interpreter* interpreter, char* name,       Toy_LiteralArray* arguments, Toy_LiteralArray* returns);
```

The first argument must be an interpreter. The third argument is a pointer to a `Toy_LiteralArray` containing a list of arguments to pass to the function, and the fourth is a pointer to a `Toy_LiteralArray` where the return values can be stored (an array is used here for a potential future feature). The contents of the argument array are consumed and left in an indeterminate state (but is safe to free), while the returns array always has one value - if the function did not return a value, then it contains a `null` literal.

The second arguments to these functions are either the function to be called as a `Toy_Literal`, or the name of the function within the interpreter's scope. The latter API simply finds the specified `Toy_Literal` if it exists and calls the former. As with most APIs, these return `false` if something went wrong.

## Memory Allocation

Depending on your platform of choice, you may want to alter how the memory is allocated within Toy. You can do this with the simple memory API:

```c
//signature returns the new pointer to be used
typedef void* (*Toy_MemoryAllocatorFn)(void* pointer, size_t oldSize, size_t newSize);
TOY_API void Toy_setMemoryAllocator(Toy_MemoryAllocatorFn);
```

Pass it a function which matches the above signature, and it'll be callable via the following macros:

* `TOY_ALLOCATE(type, count)`
* `TOY_FREE(type, pointer)`
* `TOY_GROW_ARRAY(type, pointer, oldCount, newCount)`
* `TOY_SHRINK_ARRAY(type, pointer, oldCount, newCount)`
* `TOY_FREE_ARRAY(type, pointer, oldCount)`

Also, the following macros are provided to calculate the ideal array capacities (the latter of which is for rapidly growing structures):

* `TOY_GROW_CAPACITY(capacity)`
* `TOY_GROW_CAPACITY_FAST(capacity)`

