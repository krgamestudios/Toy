#include "toy_function.h"

Toy_Function* Toy_createFunctionFromBytecode(Toy_Bucket** bucketHandle, unsigned char* bytecode) {
	Toy_Function* fn = (Toy_Function*)Toy_partitionBucket(bucketHandle, sizeof(Toy_Function));

	fn->type = TOY_FUNCTION_CUSTOM;
	fn->bytecode.code = bytecode;

	return fn;
}