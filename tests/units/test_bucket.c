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
		if (bucket == NULL || bucket->count != 4 * sizeof(int)) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to partition 'Toy_Bucket' correctly: count is %d, expected %d\n" TOY_CC_RESET, (int)(bucket->count), (int)(4*sizeof(int)));
			return -1;
		}

		//cleanup
		Toy_freeBucket(&bucket);
	}

	//test partitioning a bucket, several times, with an internal expansion
	{
		//init
		Toy_Bucket* bucket = Toy_allocateBucket(sizeof(int) * 4);

		//grab some memory
		Toy_partitionBucket(&bucket, sizeof(int));
		Toy_partitionBucket(&bucket, sizeof(int));
		Toy_partitionBucket(&bucket, sizeof(int));
		Toy_partitionBucket(&bucket, sizeof(int));
		Toy_partitionBucket(&bucket, sizeof(int));
		Toy_partitionBucket(&bucket, sizeof(int));

		//checks - please note that the top-most bucket is what is being filled - older buckets are further along
		if (
			bucket->capacity != 4 * sizeof(int) ||
			bucket->count != 2 * sizeof(int) ||
			bucket->next == NULL ||
			bucket->next->capacity != 4 * sizeof(int) ||
			bucket->next->count != 4 * sizeof(int))
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to expand 'Toy_Bucket' correctly\n" TOY_CC_RESET);
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

	return total;
}
