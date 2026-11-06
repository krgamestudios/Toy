#include "toy_bucket.h"
#include "toy_console_colors.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//buckets of fun
Toy_Bucket* Toy_allocateBucket(unsigned int capacity) {
	assert(capacity != 0 && "Cannot allocate a 'Toy_Bucket' with zero capacity");

	Toy_Bucket* bucket = malloc(sizeof(Toy_Bucket) + capacity);

	if (bucket == NULL) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to allocate a 'Toy_Bucket' of %d capacity\n" TOY_CC_RESET, (int)capacity);
		exit(1);
	}

	memset(bucket, 0, sizeof(Toy_Bucket) + capacity); //zero the memory, to avoid broken header metadata

	//initialize the bucket
	bucket->next = NULL;
	bucket->capacity = capacity;
	bucket->count = 0;

	return bucket;
}

unsigned char* Toy_partitionBucket(Toy_Bucket** bucketHandle, unsigned int amount) {
	assert(*bucketHandle != NULL);

	//Both the header and payload occupy aligned blocks. The size word
	//remains immediately before the payload for release and collection.
	amount = (amount + TOY_BUCKET_ALIGNMENT-1) & ~(TOY_BUCKET_ALIGNMENT-1);
	unsigned int total = TOY_BUCKET_ALIGNMENT + amount;

	if ((*bucketHandle)->capacity - (*bucketHandle)->count < total) {
		unsigned int capacity = (*bucketHandle)->capacity;
		Toy_Bucket* next = Toy_allocateBucket(capacity > total ? capacity : total);
		next->next = *bucketHandle;
		*bucketHandle = next;
	}

	unsigned char* payload = (*bucketHandle)->data + (*bucketHandle)->count + TOY_BUCKET_ALIGNMENT;
	*((unsigned int*)(payload - sizeof(unsigned int))) = amount;
	(*bucketHandle)->count += total;
	return payload;
}

void Toy_releaseBucketPartition(unsigned char* ptr) {
	*((int*)(ptr-sizeof(unsigned int))) |= 1; //flips the low-bit within the header
	//no checks here, for technical reasons
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

void Toy_collectBucketGarbage(Toy_Bucket** bucketHandle) {
	//clear whatever this handle is pointing to
	if ((*bucketHandle) == NULL) {
		return;
	}

	Toy_Bucket* link = *bucketHandle;
	while (link) {
		//find non-free partitions
		unsigned char* ptr = link->data + TOY_BUCKET_ALIGNMENT - sizeof(unsigned int);

		bool gc = true;

		while (ptr - link->data < link->count) { //for each partition
			if ( (*((int*)ptr) & 1) == 0) { //is this partition still in use?
				gc = false;
				break;
			}
			ptr += (*((unsigned int*)ptr) & ~1U) + TOY_BUCKET_ALIGNMENT; //OR + XOR to remove the 'free' flag from the size
		}

		//free this link, if its been entirely released
		if (gc) {
			//if link is the head
			if (link == (*bucketHandle)) {
				//if there's nowhere to go, don't delete the whole bucket
				if ((*bucketHandle)->next == NULL) {
					return;
				}
				else {
					(*bucketHandle) = (*bucketHandle)->next;
					free(link);
					link = (*bucketHandle);
				}
			}
			else {
				//find the prev and free this link before continuing
				Toy_Bucket* it = (*bucketHandle);
				while (it->next != link) {
					it = it->next;
				}
				it->next = link->next;
				free(link);
				link = it->next;
			}
		}
		else {
			link = link->next;
		}
	}
}