
# Runner Library

The runner library is used to execute one script from inside another. It also has functions that allow you to retrieve variables from the other script.

The runner library has a concept called a "dirty" script - dirty scripts are those which have already been run, and whose variables can be accessed. Dirty scripts must be reset before it is run again.

The runner library can usually be accessed with the `import` keyword:

```toy
import runner;
```

## Defined Functions

### loadScript(path: string): opaque

This is used to load an external script into an opaque variable.

This function does a lot of work:

* It validates the file path using the drive syntax
* It reads in the source code of the script file
* It compiles the source script into bytecode
* It constructs and initializes an Interpreter
* It packages it all into an opaque variable and returns it

### loadScriptBytecode(path: string): opaque

This is used to load an external bytecode file into an opaque variable.

This function does a lot of work:

* It validates the file path using the drive syntax
* It constructs and initializes an Interpreter
* It packages it all into an opaque variable and returns it

Note: This function resembles `loadScript()`, but skips the compilation step.

### runScript(self: opaque)

This function executes an external script, which must first be loaded into an opaque variable with either `loadScript()` or `loadScriptBytecode()`.

### getScriptVar(self: opaque, name: string): any

This function retrieves a variable from the top level of a script's environment.

## callScriptFn(self: opaque, name: string, ...rest)

This function retrieves a function from the top level of a script's environment, and calls it with `rest` as the argument list.

### resetScript(self: opaque)

This function resets the script so that it is no longer in a "dirty" state, and can be re-run using `runScript()`.

### freeScript(self: opaque)

This function frees a script's resources, cleaning up any memory that is no longer needed. Failing to call this will result in a memory leak.

### checkScriptDirty(self: opaque): bool

This function returns true if the script is "dirty", otherwise it returns false.
