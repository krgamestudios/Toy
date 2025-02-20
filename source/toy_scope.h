#pragma once

#include "toy_common.h"

#include "toy_bucket.h"
#include "toy_value.h"
#include "toy_string.h"
#include "toy_table.h"

//wraps Toy_Table, restricting keys to name strings, and handles scopes as a linked list
typedef struct Toy_Scope {
	struct Toy_Scope* next;
	Toy_Table* table;
	unsigned int refCount;
} Toy_Scope;

//handle deep scopes - the scope is stored in the bucket, not the table
TOY_API Toy_Scope* Toy_pushScope(Toy_Bucket** bucketHandle, Toy_Scope* scope);
TOY_API Toy_Scope* Toy_popScope(Toy_Scope* scope);
TOY_API Toy_Scope* Toy_private_pushDummyScope(Toy_Bucket** bucketHandle, Toy_Scope* scope); //doesn't delcare a table for storage

//manage the contents
TOY_API void Toy_declareScope(Toy_Scope* scope, Toy_String* key, Toy_Value value);
TOY_API void Toy_assignScope(Toy_Scope* scope, Toy_String* key, Toy_Value value);
TOY_API Toy_Value* Toy_accessScopeAsPointer(Toy_Scope* scope, Toy_String* key);

TOY_API bool Toy_isDeclaredScope(Toy_Scope* scope, Toy_String* key);

//TODO: delcare with a custom table (game engine entities)