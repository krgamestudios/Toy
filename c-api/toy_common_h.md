

# toy_common.h

This file is generally included in most header files within Toy, as it is where the TOY_API macro is defined. It also has some utilities intended for use only by the REPL.

## Defined Macros

### TOY_API

This definition of this macro is platform-dependant, and used to enable cross-platform compilation of shared and static libraries.

### TOY_VERSION_MAJOR

The current major version of Toy. This value is embedded into the bytecode, and the interpreter will refuse to run bytecode with a major version that does not match its own version.

This value MUST fit into an unsigned char.

### TOY_VERSION_MINOR

The current minor version of Toy. This value is embedded into the bytecode, and the interpreter will refuse to run bytecode with a minor version that is greater than its own minor version.

This value MUST fit into an unsigned char.

### TOY_VERSION_PATCH

The current patch version of Toy. This value is embedded into the bytecode.

This value MUST fit into an unsigned char.

### TOY_VERSION_BUILD

The current build version of Toy. This value is embedded into the bytecode.

This evaluates to a c-string, which contains build information such as compilation date and time of the interpreter. When in verbose mode, the compiler will display a warning if the build version of the bytecode does not match the build version of the interpreter.

This macro may also be used to store additional information about forks of the Toy codebase.
