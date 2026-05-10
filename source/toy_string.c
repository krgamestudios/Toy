#include "toy_string.h"
#include "toy_console_colors.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//utils
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

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
		//the parent of this node triggered a decrement across the whole tree
		decrementRefCount(str->node.left);
		decrementRefCount(str->node.right);
	}
	if (str->info.refCount == 0) {
		if (str->info.type == TOY_STRING_NODE) {
			//THIS node has triggered the decrement, so run this again
			decrementRefCount(str->node.left);
			decrementRefCount(str->node.right);
		}

		//mark this memory as unused
		Toy_releaseBucketPartition((void*)str);
	}
}

//exposed functions
Toy_String* Toy_toString(Toy_Bucket** bucketHandle, const char* cstring) {
	return Toy_toStringLength(bucketHandle, cstring, strlen(cstring));
}

Toy_String* Toy_toStringLength(Toy_Bucket** bucketHandle, const char* cstring, unsigned int length) {
	Toy_String* ret = (Toy_String*)Toy_partitionBucket(bucketHandle, sizeof(Toy_String));

	ret->info.type = TOY_STRING_LEAF;
	ret->info.length = length;
	ret->info.refCount = 1;
	ret->info.cachedHash = 0; //don't calc until needed
	ret->leaf.data = cstring; //don't make a local copy

	return ret;
}

Toy_String* Toy_createStringLength(Toy_Bucket** bucketHandle, const char* cstring, unsigned int length) {
	Toy_String* ret = (Toy_String*)Toy_partitionBucket(bucketHandle, sizeof(Toy_String) + length + 1);

	if (length > 0) {
		ret->leaf.data = (char*)(ret + 1); //increments by 1 'string', to the length +1
		strncpy((char*)(ret->leaf.data), cstring, length);
		((char*)(ret->leaf.data))[length] = '\0'; //don't forget the null
		ret->info.length = length;
	}
	else {
		ret->leaf.data = "";
		ret->info.length = length;
	}

	ret->info.type = TOY_STRING_LEAF;
	ret->info.refCount = 1;
	ret->info.cachedHash = 0; //don't calc until needed

	return ret;
}

Toy_String* Toy_copyString(Toy_String* str) {
	assert(str->info.refCount != 0 && "Can't copy a string with refcount of zero");
	incrementRefCount(str);
	return str;
}

Toy_String* Toy_concatStrings(Toy_Bucket** bucketHandle, Toy_String* left, Toy_String* right) {
	assert(left->info.refCount != 0 && right->info.refCount != 0 && "Can't concatenate a string with a refcount of zero");

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
	assert(str->info.refCount > 0 && "Can't free a string with refcount 0");
	//memory is freed when the bucket is
	decrementRefCount(str);
}

unsigned int Toy_getStringLength(Toy_String* str) {
	return str->info.length;
}

unsigned int Toy_getStringRefCount(Toy_String* str) {
	return str->info.refCount;
}

static void getStringRawUtil(char* dest, Toy_String* str) {
	//sometimes, "clever" can be a bad thing...
	if (str->info.type == TOY_STRING_NODE) {
		getStringRawUtil(dest, str->node.left);
		getStringRawUtil(dest + str->node.left->info.length, str->node.right);
	}

	else {
		memcpy(dest, str->leaf.data, str->info.length);
	}
}

char* Toy_getStringRaw(Toy_String* str) {
	assert(str->info.refCount != 0 && "Can't build a raw string from a string with refcount of zero");

	//BUGFIX: Make sure it's aligned, and there's space for the null
	unsigned int len = (str->info.length + 3) & ~3;
	if (len == str->info.length) { //nulls aren't counted in a string's length
		len += 4;
	}

	char* buffer = malloc(len);

	getStringRawUtil(buffer, str);
	buffer[str->info.length] = '\0';

	return buffer;
}

static int deepCompareUtil(Toy_String* left, Toy_String* right, const char** leftHead, const char** rightHead) {
	//NOTE: this function can't handle strings of zero length
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

		//if there's a difference, and neither is null, than a result has (probably) been found
		if ((**leftHead != '\0' && **rightHead != '\0' && (**leftHead != **rightHead))) {
			result = *(const unsigned char*)(*leftHead) - *(const unsigned char*)(*rightHead);
		}
	}

	//returning 0 means no difference found yet
	return result;
}

int Toy_compareStrings(Toy_String* left, Toy_String* right) {
	//BUGFIX: since deepCompareUtil() can't handle strings of length zero, insert a check here
	if (left->info.length == 0 || right->info.length == 0) {
		return left->info.length - right->info.length;
	}

	//BUGFIX: If both args are leaves, and one is a substring of the other, then deepCompareUtil() will return a wrong result
	if (left->info.type == TOY_STRING_LEAF && right->info.type == TOY_STRING_LEAF) {
		unsigned int maxLength = left->info.length > right->info.length ? left->info.length : right->info.length;
		return strncmp(left->leaf.data, right->leaf.data, maxLength);
	}

	//util pointers
	const char* leftHead = NULL;
	const char* rightHead = NULL;

	int result = deepCompareUtil(left, right, &leftHead, &rightHead);

	//BUGFIX: deepCompareUtil() doesn't handle substrings correctly
	if (result == 0 && leftHead != NULL && rightHead != NULL) {
		return (int)(*leftHead - *rightHead);
	}
	else {
		return result;
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

unsigned int Toy_hashString(Toy_String* str) {
	if (str->info.cachedHash != 0) {
		return str->info.cachedHash;
	}
	else if (str->info.type == TOY_STRING_NODE) {
		char* buffer = Toy_getStringRaw(str);
		str->info.cachedHash = hashCString(buffer);
		free(buffer);
	}
	else {
		str->info.cachedHash = hashCString(str->leaf.data);
	}

	return str->info.cachedHash;
}
