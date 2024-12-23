#include "toy.h"

#include <stdio.h>
#include <stdlib.h>

//generate an immense series of Toy_String instances to fill the buckets, thrn compare the time taken for each possible vale of TOY_BUCKET_IDEAL

static unsigned int hash(unsigned int x) {
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = ((x >> 16) ^ x) * 0x45d9f3b;
	x = ((x >> 16) ^ x);
	return x;
}

static unsigned int seed = 42;

static unsigned int rng() {
	return seed = hash(seed);
}

#define MAX 9
const char* samples[] = { //9 entries
	"the",
	"quick",
	"brown",
	"fox",
	"jumped",
	"over",
	"the",
	"lazy",
	"dog",
};

void stress_fillBucket(Toy_Bucket** bucketHandle) {
	for (unsigned int i = 0; i < 10000000; i++) {
		//create some leaf and node strings
		Toy_String* a = Toy_createString(bucketHandle, samples[rng() % MAX]);
		Toy_String* b = Toy_createString(bucketHandle, samples[rng() % MAX]);
		Toy_String* c = Toy_createString(bucketHandle, samples[rng() % MAX]);
		Toy_String* d = Toy_createString(bucketHandle, samples[rng() % MAX]);

		Toy_String* l = Toy_concatStrings(bucketHandle, a, b);
		Toy_String* r = Toy_concatStrings(bucketHandle, c, d);
		Toy_concatStrings(bucketHandle, l, r);

//		char* buffer = Toy_getStringRawBuffer(s);
//		printf("%s\n", buffer);
//		free(buffer);
	}
}

static unsigned long long int measureDepth(Toy_Bucket* bucket) {
	return bucket == NULL ? 0 : 1 + measureDepth(bucket->next);
}

static unsigned long long int measureCapacity(Toy_Bucket* bucket) {
	return bucket == NULL ? 0 : bucket->capacity + measureCapacity(bucket->next);
}

static unsigned long long int measureCount(Toy_Bucket* bucket) {
	return bucket == NULL ? 0 : bucket->count + measureCount(bucket->next);
}

int main() {
	Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);

	stress_fillBucket(&bucket);

	unsigned long long int depth = measureDepth(bucket);
	unsigned long long int capacity = measureCapacity(bucket);
	unsigned long long int count = measureCount(bucket);

	printf(TOY_CC_FONT_RED TOY_CC_BACK_YELLOW "Result: %u: %llu, %llu, %llu" TOY_CC_RESET "\n", TOY_BUCKET_IDEAL, depth, capacity, count);

	Toy_freeBucket(&bucket);

	return 0;
}
