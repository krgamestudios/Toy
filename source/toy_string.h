#pragma once

#include "toy_common.h"

#include "toy_bucket.h"
#include "toy_value.h"

//forward declare
union Toy_String_t;

//rope pattern, conforming to the C spec - see #158
typedef enum Toy_StringType {
	TOY_STRING_NODE,
	TOY_STRING_LEAF,
	TOY_STRING_NAME,
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
	char data[];
} Toy_StringLeaf;

typedef struct Toy_StringName {
	Toy_StringInfo _padding;
	Toy_ValueType varType;
	bool varConstant;
	char data[];
} Toy_StringName;

typedef union Toy_String_t {
	Toy_StringInfo info;
	Toy_StringNode node;
	Toy_StringLeaf leaf;
	Toy_StringName name;
} Toy_String;

//
TOY_API Toy_String* Toy_createString(Toy_Bucket** bucketHandle, const char* cstring);
TOY_API Toy_String* Toy_createStringLength(Toy_Bucket** bucketHandle, const char* cstring, unsigned int length);

TOY_API Toy_String* Toy_createNameStringLength(Toy_Bucket** bucketHandle, const char* cname, unsigned int length, Toy_ValueType varType, bool constant); //for variable names

TOY_API Toy_String* Toy_copyString(Toy_String* str);
TOY_API Toy_String* Toy_deepCopyString(Toy_Bucket** bucketHandle, Toy_String* str);

TOY_API Toy_String* Toy_concatStrings(Toy_Bucket** bucketHandle, Toy_String* left, Toy_String* right);

TOY_API void Toy_freeString(Toy_String* str);

TOY_API unsigned int Toy_getStringLength(Toy_String* str);
TOY_API unsigned int Toy_getStringRefCount(Toy_String* str);
TOY_API Toy_ValueType Toy_getNameStringVarType(Toy_String* str);
TOY_API bool Toy_getNameStringVarConstant(Toy_String* str);

TOY_API char* Toy_getStringRawBuffer(Toy_String* str); //allocates the buffer on the heap, needs to be freed

TOY_API int Toy_compareStrings(Toy_String* left, Toy_String* right); //return value mimics strcmp()

TOY_API unsigned int Toy_hashString(Toy_String* string);
