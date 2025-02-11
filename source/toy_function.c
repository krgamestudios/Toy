#include "toy_function.h"

Toy_Function* Toy_createModuleFunction(Toy_Bucket** bucketHandle, Toy_Module module) {
	Toy_Function* fn = (Toy_Function*)Toy_partitionBucket(bucketHandle, sizeof(Toy_Function));

	fn->type = TOY_FUNCTION_MODULE;
	fn->module.module = module;

	return fn;
}