#include "toy_string.h"
#include "toy_console_colors.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//utils
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

static void deepCopyUtil(char* dest, Toy_String* str) {
	//sometimes, "clever" can be a bad thing...
	if (str->info.type == TOY_STRING_NODE) {
		deepCopyUtil(dest, str->node.left);
		deepCopyUtil(dest + str->node.left->info.length, str->node.right);
	}

	else {
		memcpy(dest, str->leaf.data, str->info.length);
	}
}

static void incrementRefCount(Toy_String* str) {
	str->info.refCount++;
	if (str->info.type == TOY_STRING_NODE) {
		incrementRefCount(str->node.left);
		incrementRefCount(str->node.right);
	}
}

static void decrementRefCount(Toy_String* str) {
	str->info.refCount--;
	if (str->info.type == TOY_STRING_NODE) {
		decrementRefCount(str->node.left);
		decrementRefCount(str->node.right);
	}
}

static unsigned int hashCString(const char* string) {
	unsigned int hash = 2166136261u;

	for (unsigned int i = 0; string[i]; i++) {
		hash *= string[i];
		hash ^= 16777619;
	}

	return hash;
}

static Toy_String* partitionStringLength(Toy_Bucket** bucketHandle, const char* cstring, unsigned int length) {
	Toy_String* ret = (Toy_String*)Toy_partitionBucket(bucketHandle, sizeof(Toy_String) + length + 1);

	ret->info.type = TOY_STRING_LEAF;
	ret->info.length = length;
	ret->info.refCount = 1;
	ret->info.cachedHash = 0; //don't calc until needed
	memcpy(ret->leaf.data, cstring, length + 1);
	ret->leaf.data[length] = '\0';

	return ret;
}

//exposed functions
Toy_String* Toy_createString(Toy_Bucket** bucketHandle, const char* cstring) {
	unsigned int length = strlen(cstring);

	return Toy_createStringLength(bucketHandle, cstring, length);
}

Toy_String* Toy_createStringLength(Toy_Bucket** bucketHandle, const char* cstring, unsigned int length) {
	//normal behaviour
	if (length < (*bucketHandle)->capacity - sizeof(Toy_String) - 1) {
		return partitionStringLength(bucketHandle, cstring, length);
	}

	//break the string up if it's too long
	Toy_String* result = NULL;

	for (unsigned int i = 0; i < length; i += (*bucketHandle)->capacity - sizeof(Toy_String) - 1) { //increment by the amount actually used by the cstring
		unsigned int amount = MIN((length - i), (*bucketHandle)->capacity - sizeof(Toy_String) - 1);
		Toy_String* fragment = partitionStringLength(bucketHandle, cstring + i, amount);

		result = result == NULL ? fragment : Toy_concatStrings(bucketHandle, result, fragment);
	}

	return result;
}

Toy_String* Toy_createNameStringLength(Toy_Bucket** bucketHandle, const char* cname, unsigned int length, Toy_ValueType varType, bool constant) {
	assert(varType != TOY_VALUE_NULL && "Can't declare a name string with variable type 'null'");

	Toy_String* ret = (Toy_String*)Toy_partitionBucket(bucketHandle, sizeof(Toy_String) + length + 1);

	ret->info.type = TOY_STRING_NAME;
	ret->info.length = length;
	ret->info.refCount = 1;
	ret->info.cachedHash = 0; //don't calc until needed
	memcpy(ret->name.data, cname, length + 1);
	ret->name.data[length] = '\0';
	ret->name.varType = varType;
	ret->name.varConstant = constant;

	return ret;
}

Toy_String* Toy_copyString(Toy_String* str) {
	assert(str->info.refCount != 0 && "Can't copy a string with refcount of zero");
	incrementRefCount(str);
	return str;
}

Toy_String* Toy_deepCopyString(Toy_Bucket** bucketHandle, Toy_String* str) {
	assert(str->info.refCount != 0 && "Can't deep copy a string with refcount of zero");

	//handle deep copies of strings that are too long for the bucket capacity
	if (sizeof(Toy_String) + str->info.length + 1 > (*bucketHandle)->capacity) {
		char* buffer = Toy_getStringRawBuffer(str);
		Toy_String* result = Toy_createStringLength(bucketHandle, buffer, str->info.length); //handles the fragmenting
		free(buffer);
		return result;
	}

	Toy_String* ret = (Toy_String*)Toy_partitionBucket(bucketHandle, sizeof(Toy_String) + str->info.length + 1);

	if (str->info.type == TOY_STRING_NODE || str->info.type == TOY_STRING_LEAF) {
		ret->info.type = TOY_STRING_LEAF;
		ret->info.length = str->info.length;
		ret->info.refCount = 1;
		ret->info.cachedHash = str->info.cachedHash;
		deepCopyUtil(ret->leaf.data, str); //copy each leaf into the buffer
		ret->leaf.data[ret->info.length] = '\0';
	}
	else {
		ret->info.type = TOY_STRING_NAME;
		ret->info.length = str->info.length;
		ret->info.refCount = 1;
		ret->info.cachedHash = str->info.cachedHash;
		memcpy(ret->name.data, str->name.data, str->info.length + 1);
		ret->name.data[ret->info.length] = '\0';
	}

	return ret;
}

Toy_String* Toy_concatStrings(Toy_Bucket** bucketHandle, Toy_String* left, Toy_String* right) {
	assert(left->info.refCount != 0 && right->info.refCount != 0 && "Can't concatenate a string with a refcount of zero");
	assert(left->info.type != TOY_STRING_NAME && right->info.type != TOY_STRING_NAME && "Can't concatenate a name string");

	Toy_String* ret = (Toy_String*)Toy_partitionBucket(bucketHandle, sizeof(Toy_String));

	ret->info.type = TOY_STRING_NODE;
	ret->info.length = left->info.length + right->info.length;
	ret->info.refCount = 1;
	ret->info.cachedHash = 0; //don't calc until needed
	ret->node.left = left;
	ret->node.right = right;

	incrementRefCount(left);
	incrementRefCount(right);

	return ret;
}

void Toy_freeString(Toy_String* str) {
	//memory is freed when the bucket is
	decrementRefCount(str);
}

unsigned int Toy_getStringLength(Toy_String* str) {
	return str->info.length;
}

unsigned int Toy_getStringRefCount(Toy_String* str) {
	return str->info.refCount;
}

Toy_ValueType Toy_getNameStringVarType(Toy_String* str) {
	assert(str->info.type == TOY_STRING_NAME && "Can't get the variable type of a non-name string");

	return str->name.varType;
}

bool Toy_getNameStringVarConstant(Toy_String* str) {
	assert(str->info.type == TOY_STRING_NAME && "Can't get the variable constness of a non-name string");

	return str->name.varConstant;
}

char* Toy_getStringRawBuffer(Toy_String* str) {
	assert(str->info.type != TOY_STRING_NAME && "Can't get raw string buffer of a name string");
	assert(str->info.refCount != 0 && "Can't get raw string buffer of a string with refcount of zero");

	//BUGFIX: Make sure it's aligned, and there's space for the null
	unsigned int len = (str->info.length + 3) & ~3;
	if (len == str->info.length) { //nulls aren't counted in a string's length
		len += 4;
	}

	char* buffer = malloc(len);

	deepCopyUtil(buffer, str);
	buffer[str->info.length] = '\0';

	return buffer;
}

static int deepCompareUtil(Toy_String* left, Toy_String* right, const char** leftHead, const char** rightHead) {
	//WARNING: this function can't handle strings of zero length
	int result = 0;

	//if it's the same object, of course they match
	if (left == right) {
		return result;
	}

	//BUGFIX: if we're not currently iterating through the left leaf (and leftHead is not null), skip out
	if (left->info.type == TOY_STRING_LEAF && (*leftHead) != NULL && (**leftHead) != '\0' && ((*leftHead) < left->leaf.data || (*leftHead) > (left->leaf.data + left->info.length)) ) {
		return result;
	}

	//BUGFIX: if we're not currently iterating through the right leaf (and rightHead is not null), skip out
	if (right->info.type == TOY_STRING_LEAF && (*rightHead) != NULL && (**rightHead) != '\0' && ((*rightHead) < right->leaf.data || (*rightHead) > (right->leaf.data + right->info.length)) ) {
		return result;
	}

	//dig into left
	if (left->info.type == TOY_STRING_NODE) {
		if ((result = deepCompareUtil(left->node.left, right, leftHead, rightHead)) != 0) {
			return result;
		}
		if ((result = deepCompareUtil(left->node.right, right, leftHead, rightHead)) != 0) {
			return result;
		}

		//return zero to keep going
		return result;
	}

	//dig into right
	if (right->info.type == TOY_STRING_NODE) {
		if ((result = deepCompareUtil(left, right->node.left, leftHead, rightHead)) != 0) {
			return result;
		}

		if ((result = deepCompareUtil(left, right->node.right, leftHead, rightHead)) != 0) {
			return result;
		}

		//return zero to keep going
		return result;
	}

	//keep comparing the leaves
	if (left->info.type == TOY_STRING_LEAF && right->info.type == TOY_STRING_LEAF) {
		//initial head states can be null, or null characters
		if ((*leftHead) == NULL || (**leftHead) == '\0') {
			(*leftHead) = left->leaf.data;
		}

		if ((*rightHead) == NULL || (**rightHead) == '\0') {
			(*rightHead) = right->leaf.data;
		}

		//compare and increment
		while (**leftHead && (**leftHead == **rightHead)) {
			(*leftHead)++;
			(*rightHead)++;
		}

		//if both are not null, then it's a real result
		if ( (**leftHead == '\0' || **rightHead == '\0') == false) {
			result = *(const unsigned char*)(*leftHead) - *(const unsigned char*)(*rightHead);
		}
	}

	//if either are a null character, return 0 to check the next node
	return result;
}

int Toy_compareStrings(Toy_String* left, Toy_String* right) {
	//BUGFIX: since deepCompareUtil() can't handle strings of length zero, insert a check here
	if (left->info.length == 0 || right->info.length == 0) {
		return left->info.length - right->info.length;
	}

	if (left->info.type == TOY_STRING_NAME || right->info.type == TOY_STRING_NAME) {
		assert (left->info.type == right->info.type && "Can't compare a name string to a non-name string");
		return strncmp(left->name.data, right->name.data, left->info.length);
	}

	//util pointers
	const char* leftHead = NULL;
	const char* rightHead = NULL;

	return deepCompareUtil(left, right, &leftHead, &rightHead);
}

unsigned int Toy_hashString(Toy_String* str) {
	if (str->info.cachedHash != 0) {
		return str->info.cachedHash;
	}
	else if (str->info.type == TOY_STRING_NODE) {
		//TODO: I wonder if it would be possible to discretely swap the composite node string with a new leaf string here? Would that speed up other parts of the code by not having to walk the tree in future? - needs to be benchmarked
		char* buffer = Toy_getStringRawBuffer(str);
		str->info.cachedHash = hashCString(buffer);
		free(buffer);
	}
	else if (str->info.type == TOY_STRING_LEAF) {
		str->info.cachedHash = hashCString(str->leaf.data);
	}
	else if (str->info.type == TOY_STRING_NAME) {
		str->info.cachedHash = hashCString(str->name.data);
	}

	return str->info.cachedHash;
}
