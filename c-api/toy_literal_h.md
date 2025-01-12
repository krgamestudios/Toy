

# toy_literal.h

This header defines the literal structure, which is used extensively throughout Toy to represent values of some kind.

The main way of interacting with literals is to use a macro of some kind, as the exact implementation of `Toy_Literal` has and will change based on the needs of Toy.

User data can be passed around within Toy as an opaque type - use the tag value for determining what kind of opaque it is, or leave it as 0.

## Defined Enums

### Toy_LiteralType

* `TOY_LITERAL_NULL`
* `TOY_LITERAL_BOOLEAN`
* `TOY_LITERAL_INTEGER`
* `TOY_LITERAL_FLOAT`
* `TOY_LITERAL_STRING`
* `TOY_LITERAL_ARRAY`
* `TOY_LITERAL_DICTIONARY`
* `TOY_LITERAL_FUNCTION`
* `TOY_LITERAL_FUNCTION_NATIVE`
* `TOY_LITERAL_FUNCTION_HOOK`
* `TOY_LITERAL_IDENTIFIER`
* `TOY_LITERAL_TYPE`
* `TOY_LITERAL_OPAQUE`
* `TOY_LITERAL_ANY`

These are the main values of `Toy_LiteralType`, each of which represents a potential state of the `Toy_Literal` structure. Do not interact with a literal without determining its type with the `IS_*` macros first.

Other type values are possible, but are only used internally.

## Defined Macros

The following macros are used to determine if a given literal, passed in as `value`, is of a specific type. It should be noted that `TOY_IS_FUNCTION` will return false for native and hook functions.

* `TOY_IS_NULL(value)`
* `TOY_IS_BOOLEAN(value)`
* `TOY_IS_INTEGER(value)`
* `TOY_IS_FLOAT(value)`
* `TOY_IS_STRING(value)`
* `TOY_IS_ARRAY(value)`
* `TOY_IS_DICTIONARY(value)`
* `TOY_IS_FUNCTION(value)`
* `TOY_IS_FUNCTION_NATIVE(value)`
* `TOY_IS_FUNCTION_HOOK(value)`
* `TOY_IS_IDENTIFIER(value)`
* `TOY_IS_TYPE(value)`
* `TOY_IS_OPAQUE(value)`

The following macros are used to cast a literal to a specific C type to be used.

* `TOY_AS_BOOLEAN(value)`
* `TOY_AS_INTEGER(value)`
* `TOY_AS_FLOAT(value)`
* `TOY_AS_STRING(value)`
* `TOY_AS_ARRAY(value)`
* `TOY_AS_DICTIONARY(value)`
* `TOY_AS_FUNCTION(value)`
* `TOY_AS_FUNCTION_NATIVE(value)`
* `TOY_AS_FUNCTION_HOOK(value)`
* `TOY_AS_IDENTIFIER(value)`
* `TOY_AS_TYPE(value)`
* `TOY_AS_OPAQUE(value)`

The following macros are used to create a new literal, with the given `value` as it's internal value.

* `TOY_TO_NULL_LITERAL` - does not need parentheses
* `TOY_TO_BOOLEAN_LITERAL(value)`
* `TOY_TO_INTEGER_LITERAL(value)`
* `TOY_TO_FLOAT_LITERAL(value)`
* `TOY_TO_STRING_LITERAL(value)`
* `TOY_TO_ARRAY_LITERAL(value)`
* `TOY_TO_DICTIONARY_LITERAL(value)`
* `TOY_TO_FUNCTION_LITERAL(value, l)` - `l` represents the length of the bytecode passed as `value`
* `TOY_TO_FUNCTION_NATIVE_LITERAL(value)`
* `TOY_TO_FUNCTION_HOOK_LITERAL(value)`
* `TOY_TO_IDENTIFIER_LITERAL(value)`
* `TOY_TO_TYPE_LITERAL(value, c)` - `c` is the true of the type should be const
* `TOY_TO_OPAQUE_LITERAL(value, t)` - `t` is the integer tag

## More Defined Macros

The following macros are utilities used throughout Toy's internals, and are available for the user as well.

### TOY_IS_TRUTHY(x)

Returns true if the literal `x` is truthy, otherwise it returns false.

Currently, every value is considered truthy except `false`, which is falsy and `null`, which is neither true nor false.

### TOY_AS_FUNCTION_BYTECODE_LENGTH(lit)

Returns the length of a Toy function's bytecode.

This macro is only valid on `TOY_LITERAL_FUNCTION`.

### TOY_MAX_STRING_LENGTH

The maximum length of a string in Toy, which is 4096 bytes by default. This can be changed at compile time, but the results of doing so are not officially supported.

### TOY_HASH_I(lit)

Identifiers are the names of values within Toy; to speed up execution, their "hash value" is computed at compile time and stored within them. Use this to access it, if needed.

This macro is only valid on `TOY_LITERAL_IDENTIFIER`.

### TOY_TYPE_PUSH_SUBTYPE(lit, subtype)

When building a complex type, such as the type of an array or dictionary, you may need to specify inner types. Use this to push a `subtype`. Calling `Toy_freeLiteral()` on the outermost type should clean up all inner types, as expected.

This macro returns the index of the newly pushed value within its parent.

This macro is only valid on `TOY_LITERAL_TYPE`, for both `type` and `subtype`.

### TOY_GET_OPAQUE_TAG(o)

Returns the value of the opaque `o`'s tag.

This macro is only valid on `TOY_LITERAL_OPAQUE`.

## Defined Functions

### void Toy_freeLiteral(Toy_Literal literal)

This function frees the given literal's memory. Any internal pointers are now invalid.

This function should be called on EVERY literal when it is no longer needed, regardless of type.

### Toy_Literal Toy_copyLiteral(Toy_Literal original)

This function returns a copy of the given literal. Literals should never be copied without this function, as it handles a lot of internal memory allocations.

### bool Toy_literalsAreEqual(Toy_Literal lhs, Toy_Literal rhs)

This checks to see if two given literals are equal.

When an integer and a float are compared, the integer is coerced into a float for the duration of the call.

Arrays or dictionaries are equal only if their keys and values all equal. Likewise, types only equal if all subtypes are equal, in order.

Functions and opaques are never equal to anything, while values with the type `TOY_LITERAL_ANY` are always equal.

### int Toy_hashLiteral(Toy_Literal lit)

This finds the hash of a literal, for various purposes. Different hashing algorithms are used for different types, and some types can't be hashed at all.

Types that can't be hashed are

* all kinds of functions
* type
* opaque
* any

In the case of identifiers, their hashes are precomputed on creation and are stored within the literal.

### void Toy_printLiteral(Toy_Literal literal)

This wraps a call to `Toy_printLiteralCustom`, with a printf-stdout wrapper as `printFn`.

### void Toy_printLiteralCustom(Toy_Literal literal, PrintFn printFn)

This function passes the string representation of `literal` to `printFn`. 

This function is not thread safe - due to the loopy and recursive nature of printing compound values, this function uses some globally persistent variables.

### bool Toy_private_isTruthy(Toy_Literal x)

Utilized by the `TOY_IS_TRUTHY` macro.

Private functions are not intended for general use.

### bool Toy_private_toIdentifierLiteral(Toy_RefString* ptr)

Utilized by the `TOY_TO_IDENTIFIER_LITERAL` macro.

Private functions are not intended for general use.

### bool Toy_private_typePushSubtype(Toy_Literal* lit, Toy_Literal subtype)

Utilized by the `TOY_TYPE_PUSH_SUBTYPE` macro.

Private functions are not intended for general use.
