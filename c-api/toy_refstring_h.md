

# toy_refstring.h

This header defines the structure `Toy_RefString`, as well as all the related utilities.

[refstring](https://github.com/Ratstail91/refstring) is a stand-alone utility written to reduce the amount of memory manipulation used within Toy. It was independently written and tested, before being incorporated into Toy proper. As such, it has its own memory management API, which by default is tied into Toy's [core memory API](toy_memory_h.md).

Instances of `Toy_RefString` are reference counted - that is, rather than copying an existing string in memory, a pointer to the refstring is returned, and the internal reference counter is increased by 1. When the pointer is no longer needed, `Toy_DeleteRefString` can be called; this will decrement the internal reference counter by 1, and only free it when it reaches 0. This has multiple benefits, when used correctly:

* Reduced memory usage
* Faster program execution

This module reserves the right to instead preform a deep copy when it sees fit (this is for future debugging purposes).

## Defined Interfaces

### typedef void* (*Toy_RefStringAllocatorFn)(void* pointer, size_t oldSize, size_t newSize)

This interface conforms to Toy's memory API, and generally shouldn't be used without a good reason.

## Defined Functions

### void Toy_setRefStringAllocatorFn(Toy_RefStringAllocatorFn)

This function conforms to and is invoked by Toy's memory API, and generally shouldn't be used without a good reason.

### Toy_RefString* Toy_createRefString(const char* cstring)

This function wraps `Toy_CreateRefStringLength`, by determining the length of the given `cstring` and passing it to the other function.

### Toy_RefString* Toy_createRefStringLength(const char* cstring, size_t length)

This function returns a new `Toy_RefString`, containing a copy of `cstring`, or `NULL` on error.

This function also sets the returned refstring's reference counter to 1.

### void Toy_deleteRefString(Toy_RefString* refString)

This function reduces the `refString`'s reference counter by 1 and, if it reaches 0, frees the memory.

### int Toy_countRefString(Toy_RefString* refString)

This function returns the total number of references to `refString`, for debugging.

### size_t Toy_lengthRefString(Toy_RefString* refString)

This function returns the length of the underlying cstring of `refString`.

### Toy_RefString* Toy_copyRefString(Toy_RefString* refString)

This function increases the reference counter of `refString` by 1, before returning the given pointer.

This function reserves the right to create a deep copy where needed.

### Toy_RefString* Toy_deepCopyRefString(Toy_RefString* refString)

This function behaves identically to `Toy_copyRefString`, except that it explicitly forces a deep copy of the internal memory. Using this function should be done carefully, as it incurs a performance penalty that negates the benefit of this module.

### const char* Toy_toCString(Toy_RefString* refString)

This function exposes the interal cstring of `refString`. Only use this function when dealing with external APIs.

### bool Toy_equalsRefString(Toy_RefString* lhs, Toy_RefString* rhs)

This function returns true when the two refstrings are either the same refstring, or contain the same value. Otherwise, it returns false.

### bool Toy_equalsRefStringCString(Toy_RefString* lhs, char* cstring)

This function returns true when the `refString` contains the same value as the `cstring`. Otherwise, it returns false.
