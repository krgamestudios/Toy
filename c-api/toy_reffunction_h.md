

# toy_reffunction.h

This header defines the Toy_RefFunction structure, as well as all the related utilities.

See [Toy_RefString](toy_refstring_h.md) for more information about the reference pattern.

This module reserves the right to instead preform a deep copy when it sees fit (this is for future debugging purposes).

## Defined Interfaces

### typedef void* (*Toy_RefFunctionAllocatorFn)(void* pointer, size_t oldSize, size_t newSize)

This interface conforms to Toy's memory API, and generally shouldn't be used without a good reason.

## Defined Functions

### void Toy_setRefFunctionAllocatorFn(Toy_RefFunctionAllocatorFn)

This function conforms to and is invoked by Toy's memory API, and generally shouldn't be used without a good reason.

### Toy_RefFunction* Toy_createRefFunction(const void* data, size_t length)

This function returns a new `Toy_RefFunction`, containing a copy of `data`, or `NULL` on error.

This function also sets the returned `refFunction`'s reference counter to 1.

### void Toy_deleteRefFunction(Toy_RefFunction* refFunction)

This function reduces the `refFunction`'s reference counter by 1 and, if it reaches 0, frees the memory.

### int Toy_countRefFunction(Toy_RefFunction* refFunction)

This function returns the total number of references to `refFunction`, for debugging.

### size_t Toy_lengthRefFunction(Toy_RefFunction* refFunction)

This function returns the length of the underlying bytecode of `refFunction`.

### Toy_RefFunction* Toy_copyRefFunction(Toy_RefFunction* refFunction)

This function increases the reference counter of `refFunction` by 1, before returning the given pointer.

This function reserves the right to create a deep copy where needed.

### Toy_RefFunction* Toy_deepCopyRefFunction(Toy_RefFunction* refFunction)

This function behaves identically to `Toy_copyRefFunction`, except that it explicitly forces a deep copy of the internal memory. Using this function should be done carefully, as it incurs a performance penalty that negates the benefit of this module.
