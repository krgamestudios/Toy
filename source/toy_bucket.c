#include "toy_bucket.h"
#include "toy_console_colors.h"

#include <stdio.h>
#include <stdlib.h>

//buckets of fun
Toy_Bucket* Toy_allocateBucket(unsigned int capacity) {
	if (capacity == 0) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Cannot allocate a 'Toy_Bucket' with zero capacity\n" TOY_CC_RESET);
		exit(1);
	}

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

void* Toy_partitionBucket(Toy_Bucket** bucketHandle, unsigned int amount) {
	if ((*bucketHandle) == NULL) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Expected a 'Toy_Bucket', received NULL\n" TOY_CC_RESET);
		exit(1);
	}

	//BUGFIX: the endpoint must be aligned to the word size, otherwise you'll get a bus error from moving pointers
	amount = (amount + 3) & ~3;

	//if you try to allocate too much space
	if ((*bucketHandle)->capacity < amount) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to partition a 'Toy_Bucket': requested %d from a bucket of %d capacity\n" TOY_CC_RESET, (int)amount, (int)((*bucketHandle)->capacity));
		exit(1);
	}

	//if you're out of space in this bucket
	if ((*bucketHandle)->capacity < (*bucketHandle)->count + amount) {
		//move to the next bucket
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
