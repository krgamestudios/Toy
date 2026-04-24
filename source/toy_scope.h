#pragma once

#include "toy_common.h"

#include "toy_bucket.h"
#include "toy_string.h"
#include "toy_value.h"

//keys are leaf-only strings
typedef struct Toy_ScopeEntry {
	Toy_String key;
	Toy_Value value;
	Toy_ValueType type;
	unsigned int psl; //psl '0' means empty
	bool constant;
} Toy_ScopeEntry;

//holds a table-like collection of variables TODO: check bitness
typedef struct Toy_Scope {
	struct Toy_Scope* next;
	unsigned int refCount;
	Toy_ScopeEntry* data;
	unsigned int capacity;
	unsigned int count;
	unsigned int maxPsl;
} Toy_Scope;

//handle deep scopes - the scope is stored in the bucket, not the table
TOY_API Toy_Scope* Toy_pushScope(Toy_Bucket** bucketHandle, Toy_Scope* scope);
TOY_API Toy_Scope* Toy_popScope(Toy_Scope* scope);

//manage the contents
TOY_API void Toy_declareScope(Toy_Scope* scope, Toy_String* key, Toy_ValueType type, Toy_Value value, bool constant);
TOY_API void Toy_assignScope(Toy_Scope* scope, Toy_String* key, Toy_Value value);
TOY_API Toy_Value* Toy_accessScopeAsPointer(Toy_Scope* scope, Toy_String* key);

TOY_API bool Toy_isDeclaredScope(Toy_Scope* scope, Toy_String* key);

//manage refcounting
TOY_API void Toy_private_incrementScopeRefCount(Toy_Scope* scope);
TOY_API void Toy_private_decrementScopeRefCount(Toy_Scope* scope);

//some useful sizes, could be swapped out as needed
#ifndef TOY_SCOPE_INITIAL_CAPACITY
#define TOY_SCOPE_INITIAL_CAPACITY 8
#endif

//NOTE: The DOOM hack needs a power of 2
#ifndef TOY_SCOPE_EXPANSION_RATE
#define TOY_SCOPE_EXPANSION_RATE 2
#endif

//expand when the contents passes a certain percentage (80%) of the capacity
#ifndef TOY_SCOPE_EXPANSION_THRESHOLD
#define TOY_SCOPE_EXPANSION_THRESHOLD 0.7f
#endif
