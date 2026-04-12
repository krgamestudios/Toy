#include "toy_function.h"

Toy_Function* Toy_createFunctionFromBytecode(Toy_Bucket** bucketHandle, unsigned char* bytecode, Toy_Scope* parentScope) {
	Toy_Function* fn = (Toy_Function*)Toy_partitionBucket(bucketHandle, sizeof(Toy_Function));

	fn->type = TOY_FUNCTION_CUSTOM;
	fn->bytecode.code = bytecode;
	fn->bytecode.parentScope = parentScope;
	Toy_private_incrementScopeRefCount(fn->bytecode.parentScope);

	return fn;
}

TOY_API void Toy_freeFunction(Toy_Function* fn) {
	if (fn->type == TOY_FUNCTION_CUSTOM) {
		Toy_private_decrementScopeRefCount(fn->bytecode.parentScope);
	}
}