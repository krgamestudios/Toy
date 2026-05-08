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
	amount = (amount + 3) & ~3; //NOTE: this also leaves the lowest two bits as zero

	assert((*bucketHandle) != NULL && "Expected a 'Toy_Bucket', received NULL");
	assert((*bucketHandle)->capacity >= (amount + 4) && "ERROR: Failed to partition a 'Toy_Bucket', requested amount is too high");

	//if you're out of space in this bucket, allocate another one
	if ((*bucketHandle)->capacity < (*bucketHandle)->count + amount + 4) { //+4 for the metadata header
		Toy_Bucket* tmp = Toy_allocateBucket((*bucketHandle)->capacity);
		tmp->next = (*bucketHandle); //it's buckets all the way down
		(*bucketHandle) = tmp;
	}

	//use a 4-byte metadata header to hold the size of this partition, for GC
	*((unsigned int*)((*bucketHandle)->data + (*bucketHandle)->count)) = amount;

	//track the new metadata, and return the requested memory space
	(*bucketHandle)->count += amount + 4;
	return ((*bucketHandle)->data + (*bucketHandle)->count - amount); //metadata is before the pointer's address
}

void Toy_releaseBucketPartition(unsigned char* ptr) {
	*((int*)(ptr-4)) |= 1; //flips the low-bit within the header
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

TOY_API void Toy_collectBucketGarbage(Toy_Bucket** bucketHandle) {
	//clear whatever this handle is pointing to
	if ((*bucketHandle) == NULL) {
		return;
	}

	Toy_Bucket* link = *bucketHandle;
	while (link) {
		//find non-free partitions
		unsigned char* ptr = link->data;

		bool gc = true;

		while (ptr - link->data < link->count) { //for each partition
			if ( (*((int*)ptr) & 1) == 0) { //is this partition still in use?
				gc = false;
				break;
			}
			ptr += (*((int*)(ptr)) ^ 1) + 4; //XOR to remove the 'free' flag from the size
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