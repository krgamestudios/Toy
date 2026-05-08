#include "toy_bucket.h"
#include "toy_console_colors.h"

#include <stdio.h>

int test_buckets(void) {
	//test initializing and freeing a bucket
	{
		//init
		Toy_Bucket* bucket = Toy_allocateBucket(sizeof(int) * 32);

		//check
		if (bucket == NULL || bucket->capacity != 32 * sizeof(int)) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to initialize 'Toy_Bucket'\n" TOY_CC_RESET);
			return -1;
		}

		//cleanup
		Toy_freeBucket(&bucket);
	}

	//test partitioning a bucket, several times
	{
		//init
		Toy_Bucket* bucket = Toy_allocateBucket(sizeof(int) * 32);

		//grab some memory
		Toy_partitionBucket(&bucket, sizeof(int));
		Toy_partitionBucket(&bucket, sizeof(int));
		Toy_partitionBucket(&bucket, sizeof(int));
		Toy_partitionBucket(&bucket, sizeof(int));

		//check
		if (bucket == NULL || bucket->count != 4 * (sizeof(int) +4)) { //+4 take the metadata into account
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to partition 'Toy_Bucket' correctly: count is %d, expected %d\n" TOY_CC_RESET, (int)(bucket->count), (int)(4*sizeof(int)));
			return -1;
		}

		//cleanup
		Toy_freeBucket(&bucket);
	}

	//test partitioning a bucket, several times, with an internal expansion
	{
		//init
		Toy_Bucket* bucket = Toy_allocateBucket((sizeof(int)+4) * 4); //+4 take the metadata into account

		//grab some memory
		Toy_partitionBucket(&bucket, sizeof(int));
		Toy_partitionBucket(&bucket, sizeof(int));
		Toy_partitionBucket(&bucket, sizeof(int));
		Toy_partitionBucket(&bucket, sizeof(int));
		Toy_partitionBucket(&bucket, sizeof(int));
		Toy_partitionBucket(&bucket, sizeof(int));

		//checks - please note that the top-most bucket is what is being filled - older buckets are further along
		if (
			bucket->capacity != 4 * (sizeof(int)+4) ||
			bucket->count != 2 * (sizeof(int)+4) ||
			bucket->next == NULL ||
			bucket->next->capacity != 4 * (sizeof(int)+4) ||
			bucket->next->count != 4 * (sizeof(int)+4))
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to expand 'Toy_Bucket' correctly\n" TOY_CC_RESET);
			return -1;
		}

		//cleanup
		Toy_freeBucket(&bucket);
	}

	return 0;
}

int test_garbage_collection(void) {
	//release one element in one bucket link
	{
		//init
		Toy_Bucket* bucket = Toy_allocateBucket(sizeof(int) * 32);

		//dummy data, producing 4 entries
		unsigned char* ptr1 = Toy_partitionBucket(&bucket, sizeof(int));
		unsigned char* ptr2 = Toy_partitionBucket(&bucket, sizeof(int));
		unsigned char* ptr3 = Toy_partitionBucket(&bucket, sizeof(int));
		unsigned char* ptr4 = Toy_partitionBucket(&bucket, sizeof(int));

		//release exactly one chunk of data
		(void)ptr1;
		(void)ptr2;
		Toy_releaseBucketPartition(ptr3);
		(void)ptr4;


		//check the state of the bucket's data
		if (
			bucket->capacity != 32 * sizeof(int) ||
			bucket->count != 4 * (sizeof(int)+4) ||
			bucket->next != NULL ||
			((unsigned int*)(bucket->data))[0] != 4 ||
			((unsigned int*)(bucket->data))[1] != 0 ||
			((unsigned int*)(bucket->data))[2] != 4 ||
			((unsigned int*)(bucket->data))[3] != 0 ||
			((unsigned int*)(bucket->data))[4] != 5 || //nth bit is altered here
			((unsigned int*)(bucket->data))[5] != 0 ||
			((unsigned int*)(bucket->data))[6] != 4 ||
			((unsigned int*)(bucket->data))[7] != 0
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed simple memory partition release in 'Toy_Bucket'\n" TOY_CC_RESET);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_freeBucket(&bucket);
	}

	//release one element in many bucket links
	{
		//init
		Toy_Bucket* bucket = Toy_allocateBucket(sizeof(int) * 32);

		//partition the bucket 100 times, for dummy data
		for (int i = 0; i < 50; i++) {
			Toy_partitionBucket(&bucket, sizeof(int));
		}

		unsigned char* ptr = Toy_partitionBucket(&bucket, sizeof(int)); //grab the 51st element

		for (int i = 0; i < 49; i++) {
			Toy_partitionBucket(&bucket, sizeof(int));
		}

		//16 integers to a link, check for 7 links
		if (
			bucket->next == NULL ||
			bucket->next->next == NULL ||
			bucket->next->next->next == NULL ||
			bucket->next->next->next->next == NULL ||
			bucket->next->next->next->next->next == NULL ||
			bucket->next->next->next->next->next->next == NULL ||
			bucket->next->next->next->next->next->next->next != NULL) //there is no 8th link
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to set up 'Toy_Bucket' to 'release one element in many bucket links'\n" TOY_CC_RESET);
			Toy_freeBucket(&bucket);
			return -1;
		}

		Toy_releaseBucketPartition(ptr);

		//check the 3rd element in the 4th link
		if (
			((int*)(bucket->next->next->next->data + 2 * (sizeof(int)+4) ))[0] != 5 ||
			((int*)(bucket->next->next->next->data + 2 * (sizeof(int)+4) ))[1] != 0
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to release one element in many bucket links\n" TOY_CC_RESET);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_freeBucket(&bucket);
	}

	//garbage collection on a chain
	{
		//init
		Toy_Bucket* bucket = Toy_allocateBucket(sizeof(int) * 32);

		//partition the bucket 100 times, for dummy data
		for (int i = 0; i < 100; i++) {
			Toy_partitionBucket(&bucket, sizeof(int));
		}

		//16 integers to a link, check for 7 links
		if (
			bucket->next == NULL ||
			bucket->next->next == NULL ||
			bucket->next->next->next == NULL ||
			bucket->next->next->next->next == NULL ||
			bucket->next->next->next->next->next == NULL ||
			bucket->next->next->next->next->next->next == NULL ||
			bucket->next->next->next->next->next->next->next != NULL) //there is no 8th link
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to set up 'Toy_Bucket' to test garbage collection on a link\n" TOY_CC_RESET);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//grab link pointers
		Toy_Bucket* third = bucket->next->next;
		Toy_Bucket* fourth = bucket->next->next->next;
		Toy_Bucket* fifth = bucket->next->next->next->next;

		//free all elements in this link
		for (int i = 0; i < 16; i++) {
			Toy_releaseBucketPartition((fourth->data + i*8 + 4));
		}

		//run the GC
		Toy_collectBucketGarbage(&bucket);

		//check
		if (third->next != fifth) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to remove a chain link from 'Toy_Bucket' correctly\n" TOY_CC_RESET);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_freeBucket(&bucket);
	}

	return 0;
}

int main(void) {
	//run each test set, returning the total errors given
	int total = 0, res = 0;

	{
		res = test_buckets();
		total += res;

		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
	}

	{
		res = test_garbage_collection();
		total += res;

		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
	}

	return total;
}
