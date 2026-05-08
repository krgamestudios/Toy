#include "toy_function.h"

Toy_Function* Toy_createFunctionFromBytecode(Toy_Bucket** bucketHandle, unsigned char* bytecode, Toy_Scope* parentScope) {
	Toy_Function* fn = (Toy_Function*)Toy_partitionBucket(bucketHandle, sizeof(Toy_Function));

	fn->type = TOY_FUNCTION_CUSTOM;
	fn->bytecode.code = bytecode;
	fn->bytecode.parentScope = parentScope;
	Toy_private_incrementScopeRefCount(fn->bytecode.parentScope);

	return fn;
}

Toy_Function* Toy_createFunctionFromCallback(Toy_Bucket** bucketHandle, Toy_nativeCallback callback) {
	Toy_Function* fn = (Toy_Function*)Toy_partitionBucket(bucketHandle, sizeof(Toy_Function));

	fn->type = TOY_FUNCTION_NATIVE;
	fn->native.callback = callback;

	return fn;
}

Toy_Function* Toy_copyFunction(Toy_Bucket** bucketHandle, Toy_Function* original) {
	Toy_Function* fn = (Toy_Function*)Toy_partitionBucket(bucketHandle, sizeof(Toy_Function));

	if (original->type == TOY_FUNCTION_CUSTOM) {
		fn->type = original->type;
		fn->bytecode.code = original->bytecode.code;
		fn->bytecode.parentScope = original->bytecode.parentScope;
		Toy_private_incrementScopeRefCount(fn->bytecode.parentScope);
	}
	else if (fn->type == TOY_FUNCTION_NATIVE) {
		fn->type = original->type;
		fn->native.callback = original->native.callback;
	}

	return fn;
}

TOY_API void Toy_freeFunction(Toy_Function* fn) {
	if (fn->type == TOY_FUNCTION_CUSTOM) {
		Toy_private_decrementScopeRefCount(fn->bytecode.parentScope);
	}
	else if (fn->type == TOY_FUNCTION_NATIVE) {
		fn->native.callback = NULL;
	}

	Toy_releaseBucketPartition((void*)fn);
}