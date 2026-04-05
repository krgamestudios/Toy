#pragma once

#include "toy_common.h"

#include "toy_bucket.h"

//forward declare
union Toy_String_t;

//rope pattern, conforming to the C spec - see #158
typedef enum Toy_StringType {
	TOY_STRING_NODE,
	TOY_STRING_LEAF,
} Toy_StringType;

typedef struct Toy_StringInfo {
	Toy_StringType type;
	unsigned int length;
	unsigned int refCount;
	unsigned int cachedHash;
} Toy_StringInfo;

typedef struct Toy_StringNode {
	Toy_StringInfo _padding;
	union Toy_String_t* left;
	union Toy_String_t* right;
} Toy_StringNode;

typedef struct Toy_StringLeaf {
	Toy_StringInfo _padding;
	const char* data;
} Toy_StringLeaf;

typedef union Toy_String_t {
	Toy_StringInfo info;
	Toy_StringNode node;
	Toy_StringLeaf leaf;
} Toy_String;

//
TOY_API Toy_String* Toy_toString(Toy_Bucket** bucketHandle, const char* cstring);
TOY_API Toy_String* Toy_toStringLength(Toy_Bucket** bucketHandle, const char* cstring, unsigned int length);

TOY_API Toy_String* Toy_copyString(Toy_String* str);
TOY_API Toy_String* Toy_concatStrings(Toy_Bucket** bucketHandle, Toy_String* left, Toy_String* right);

TOY_API void Toy_freeString(Toy_String* str);

TOY_API unsigned int Toy_getStringLength(Toy_String* str);
TOY_API unsigned int Toy_getStringRefCount(Toy_String* str);

TOY_API char* Toy_getStringRaw(Toy_String* str); //allocates the buffer on the heap, needs to be freed
TOY_API int Toy_compareStrings(Toy_String* left, Toy_String* right); //return value mimics strcmp()

TOY_API unsigned int Toy_hashString(Toy_String* string);
