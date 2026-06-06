# Cheat Sheet

## Compile and Run A Snippet Of Code

```c
#include "toy_lexer.h"
#include "toy_parser.h"
#include "toy_compiler.h"
#include "toy_vm.h"
#include <stdlib.h>

int main() {
	//example code
	const char* source = "print \"Hello world!\";";

	//buckets use the arena pattern for memory allocation
	Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);

	//compile the code
	Toy_Lexer lexer;
	Toy_bindLexer(&lexer, (char*)source);

	Toy_Parser parser;
	Toy_bindParser(&parser, &lexer);

	Toy_Ast* ast = Toy_scanParser(&bucket, &parser);
	unsigned char* bytecode = Toy_compileToBytecode(ast);

	//the ast, which is stored in this bucket, is no longer needed
	Toy_freeBucket(&bucket);

	//the virtual machine used at runtime
	Toy_VM vm;
	Toy_initVM(&vm);
	Toy_bindVM(&vm, bytecode, NULL);

	//execute the given code
	Toy_runVM(&vm);

	//cleanup after ourselves
	Toy_freeVM(&vm);
	free(bytecode);
}
```

## Quick and Dirty Compilation

```c
unsigned char* compileSource(const char* source) {
	Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);

	Toy_Lexer lexer;
	Toy_bindLexer(&lexer, source);

	Toy_Parser parser;
	Toy_bindParser(&parser, &lexer);

	Toy_Ast* ast = Toy_scanParser(&bucket, &parser);
	unsigned char* bytecode = Toy_compileToBytecode(ast);

	Toy_freeBucket(&bucket);
	return bytecode;
}
```

## API Functions

This is a rough outline of all API functions declared in Toy's headers. As a rule, functions that begin with `TOY_API` are useable and begin with `Toy_`, while functions that begin with `Toy_private_` are generally not intended for use, and only exposed for technical reasons.

*Note: This list is updated manually, if something is outdated let me know.*

```c
TOY_API Toy_Array* Toy_resizeArray(Toy_Array* array, unsigned int capacity);
TOY_API void Toy_setOpaqueAttributeHandler(Toy_OpaqueAttributeHandler cb);
TOY_API Toy_Bucket* Toy_allocateBucket(unsigned int capacity);
TOY_API unsigned char* Toy_partitionBucket(Toy_Bucket** bucketHandle, unsigned int amount);
TOY_API void Toy_releaseBucketPartition(unsigned char* ptr);
TOY_API void Toy_freeBucket(Toy_Bucket** bucketHandle);
TOY_API void Toy_collectBucketGarbage(Toy_Bucket** bucketHandle);
TOY_API unsigned char* Toy_compileToBytecode(Toy_Ast* ast);
TOY_API Toy_Function* Toy_createFunctionFromBytecode(Toy_Bucket** bucketHandle, unsigned char* bytecode, Toy_Scope* parentScope);
TOY_API Toy_Function* Toy_createFunctionFromCallback(Toy_Bucket** bucketHandle, Toy_nativeCallback callback);
TOY_API Toy_Function* Toy_copyFunction(Toy_Bucket** bucketHandle, Toy_Function* fn);
TOY_API void Toy_freeFunction(Toy_Function* fn);
TOY_API void Toy_bindLexer(Toy_Lexer* lexer, const char* source);
TOY_API void Toy_bindParser(Toy_Parser* parser, Toy_Lexer* lexer);
TOY_API Toy_Ast* Toy_scanParser(Toy_Bucket** bucketHandle, Toy_Parser* parser);
TOY_API void Toy_resetParser(Toy_Parser* parser);
TOY_API void Toy_print(const char* msg);
TOY_API void Toy_error(const char* msg);
TOY_API void Toy_assertFailure(const char* msg);
TOY_API void Toy_setPrintCallback(Toy_callbackType cb);
TOY_API void Toy_setErrorCallback(Toy_callbackType cb);
TOY_API void Toy_setAssertFailureCallback(Toy_callbackType cb);
TOY_API void Toy_resetPrintCallback(void);
TOY_API void Toy_resetErrorCallback(void);
TOY_API void Toy_resetAssertFailureCallback(void);
TOY_API Toy_Scope* Toy_pushScope(Toy_Bucket** bucketHandle, Toy_Scope* scope);
TOY_API Toy_Scope* Toy_popScope(Toy_Scope* scope);
TOY_API void Toy_declareScope(Toy_Scope* scope, Toy_String* key, Toy_ValueType type, Toy_Value value, bool constant);
TOY_API void Toy_assignScope(Toy_Scope* scope, Toy_String* key, Toy_Value value);
TOY_API Toy_Value* Toy_accessScopeAsPointer(Toy_Scope* scope, Toy_String* key);
TOY_API bool Toy_isDeclaredScope(Toy_Scope* scope, Toy_String* key);
TOY_API Toy_Stack* Toy_allocateStack(void);
TOY_API void Toy_freeStack(Toy_Stack* stack);
TOY_API void Toy_resetStack(Toy_Stack** stackHandle);
TOY_API void Toy_pushStack(Toy_Stack** stackHandle, Toy_Value value);
TOY_API Toy_Value Toy_peekStack(Toy_Stack** stackHandle);
TOY_API Toy_Value Toy_popStack(Toy_Stack** stackHandle);
TOY_API Toy_String* Toy_toString(Toy_Bucket** bucketHandle, const char* cstring);
TOY_API Toy_String* Toy_toStringLength(Toy_Bucket** bucketHandle, const char* cstring, unsigned int length);
TOY_API Toy_String* Toy_createStringLength(Toy_Bucket** bucketHandle, const char* cstring, unsigned int length);
TOY_API Toy_String* Toy_copyString(Toy_String* str);
TOY_API Toy_String* Toy_concatStrings(Toy_Bucket** bucketHandle, Toy_String* left, Toy_String* right);
TOY_API void Toy_freeString(Toy_String* str);
TOY_API unsigned int Toy_getStringLength(Toy_String* str);
TOY_API unsigned int Toy_getStringRefCount(Toy_String* str);
TOY_API char* Toy_getStringRaw(Toy_String* str);
TOY_API int Toy_compareStrings(Toy_String* left, Toy_String* right);
TOY_API unsigned int Toy_hashString(Toy_String* string);
TOY_API Toy_Table* Toy_allocateTable(unsigned int minCapacity);
TOY_API void Toy_freeTable(Toy_Table* table);
TOY_API void Toy_insertTable(Toy_Table** tableHandle, Toy_Value key, Toy_Value value);
TOY_API Toy_Value Toy_lookupTable(Toy_Table** tableHandle, Toy_Value key);
TOY_API void Toy_removeTable(Toy_Table** tableHandle, Toy_Value key);
TOY_API Toy_Value Toy_unwrapValue(Toy_Value value);
TOY_API unsigned int Toy_hashValue(Toy_Value value);
TOY_API Toy_Value Toy_copyValue(struct Toy_Bucket** bucketHandle, Toy_Value value);
TOY_API void Toy_freeValue(Toy_Value value);
TOY_API bool Toy_checkValueIsTruthy(Toy_Value value);
TOY_API bool Toy_checkValuesAreEqual(Toy_Value left, Toy_Value right);
TOY_API bool Toy_checkValuesAreComparable(Toy_Value left, Toy_Value right);
TOY_API int Toy_compareValues(Toy_Value left, Toy_Value right);
TOY_API union Toy_String_t* Toy_stringifyValue(struct Toy_Bucket** bucketHandle, Toy_Value value);
TOY_API const char* Toy_getValueTypeAsCString(Toy_ValueType type);
TOY_API void Toy_resetVM(Toy_VM* vm, bool preserveScope, bool preserveStack);
TOY_API void Toy_initVM(Toy_VM* vm);
TOY_API void Toy_inheritVM(Toy_VM* parentVM, Toy_VM* subVM);
TOY_API unsigned int Toy_runVM(Toy_VM* vm);
TOY_API void Toy_freeVM(Toy_VM* vm);
TOY_API Toy_Value Toy_getReturnValueFromVM(Toy_VM* parentVM, Toy_VM* subVM);
```

## Standard Library

The tools in `repl/` includes a standard library, which can be added to the root VM by calling `initStandardLibrary` just before `Toy_runVM`. It provides the following general purpose functions:

* `min(x, y)`
* `max(x, y)`
* `floor(x)`
* `ceil(x)`
* `sqrt(x)`
* `range(x)`

It also offers an example of how to write an API.

*Note: `range()` returns a closure intended for use in the `for` keyword, but the that keyword isn't done yet.*

## Inspector Tools

Other tools within `repl/` include:

* ast_inspector
* bucket_inspector
* bytecode_inspector
* scope_inspector
* stack_inspector

While these are useful tools and are expanded as needed, there may be some components of each that aren't complete yet. These are only intended for use when developing Toy itself, but may prove useful for others looking to debug their code.

## Creating and Using Functions

See `repl/standard_library.c` for examples.

## Creating And Using Opaques

This is a quick rundown of how I'm using opaques to create the API in [ToyBox](https://gitea.krgamestudios.com/krgamestudios/ToyBox):

```c
//an enum of all opaques
typedef enum OpaqueType {
	OPAQUE_KEYBOARD,
	//mouse, etc.
} OpaqueType;

//'type' being the first member allows for pointer punning
typedef struct KeyboardData {
	OpaqueType type;
} KeyboardData;

//'keyboardData' holds information about the keyboard state
KeyboardData keyboardData = {
	.type = OPAQUE_KEYBOARD,
};

//inject 'keyboardData' into the current scope with the name 'Keyboard'
Toy_String* name = Toy_toString(&vm->memoryBucket, "Keyboard");
Toy_declareScope(vm->scope, name, TOY_VALUE_OPAQUE, TOY_OPAQUE_FROM_POINTER(&keyboardData), true);
Toy_freeString(name); //don't forget to clean up after yourself!

//each opaque has its own handler, which is called from a central dispatch callback
Toy_Value dispatchOpaqueAttributes(Toy_VM* vm, Toy_Value compound, Toy_Value attribute) {
	//pointer punning to determine the opaque's type
	OpaqueType* type = (OpaqueType*)TOY_VALUE_AS_OPAQUE(compound);

	switch(*type) {
		case OPAQUE_KEYBOARD:
			return handleKeyboardAttributes(vm, compound, attribute);
	}
}

//the dispatch is usually set early on
Toy_setOpaqueAttributeHandler(dispatchOpaqueAttributes);

//from here, each opaque does what it needs to
Toy_Value handleKeyboardAttributes(Toy_VM* vm, Toy_Value compound, Toy_Value attribute) {
	KeyboardData* keyboard = (KeyboardData*)TOY_VALUE_AS_OPAQUE(compound);
	Toy_String* string = TOY_VALUE_AS_STRING(attribute);
	const char* cstr = string->leaf.data;

	if (strlen(cstr) == 5 && strcmp(cstr, "ENTER") == 0) {
		//TODO: check if enter was pressed
	}

	//the return value is left on the stack
	return TOY_VALUE_FROM_NULL();
}
```
