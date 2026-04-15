#include "toy_bucket.h"
#include "toy_console_colors.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

//buckets of fun
Toy_Bucket* Toy_allocateBucket(unsigned int capacity) {
	assert(capacity != 0 && "Cannot allocate a 'Toy_Bucket' with zero capacity");

	Toy_Bucket* bucket = malloc(sizeof(Toy_Bucket) + capacity);

	if (bucket == NULL) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to allocate a 'Toy_Bucket' of %d capacity\n" TOY_CC_RESET, (int)capacity);
		exit(1);
	}

	//initialize the bucket
	bucket->next = NULL;
	bucket->capacity = capacity;
	bucket->count = 0;

	return bucket;
}

unsigned char* Toy_partitionBucket(Toy_Bucket** bucketHandle, unsigned int amount) {
	//the endpoint must be aligned to the word size, otherwise you'll get a bus error from moving pointers
	amount = (amount + 3) & ~3;

	assert((*bucketHandle) != NULL && "Expected a 'Toy_Bucket', received NULL");
	assert((*bucketHandle)->capacity >= amount && "ERROR: Failed to partition a 'Toy_Bucket', requested amount is too high");

	//if you're out of space in this bucket, allocate another one
	if ((*bucketHandle)->capacity < (*bucketHandle)->count + amount) {
		Toy_Bucket* tmp = Toy_allocateBucket((*bucketHandle)->capacity);
		tmp->next = (*bucketHandle); //it's buckets all the way down
		(*bucketHandle) = tmp;
	}

	//track the new count, and return the specified memory space
	(*bucketHandle)->count += amount;
	return ((*bucketHandle)->data + (*bucketHandle)->count - amount);
}

void Toy_freeBucket(Toy_Bucket** bucketHandle) {
	Toy_Bucket* iter = (*bucketHandle);

	while (iter != NULL) {
		//run down the chain
		Toy_Bucket* last = iter;
		iter = iter->next;

		//clear the previous bucket from memory
		free(last);
	}

	//for safety
	(*bucketHandle) = NULL;
}
