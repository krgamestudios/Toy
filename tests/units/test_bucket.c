#include "toy_bucket.h"
#include "toy_console_colors.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

int test_buckets(void) {
	Toy_Bucket* bucket = Toy_allocateBucket(128);
	unsigned char* pointers[100];

	for (unsigned int i = 0; i < 100; i++) {
		pointers[i] = Toy_partitionBucket(&bucket, i + 1);
		if ((uintptr_t)pointers[i] % _Alignof(max_align_t) != 0) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: unexpected bucket allocation state\n" TOY_CC_RESET);
			Toy_freeBucket(&bucket);
			return -1;
		}
		memset(pointers[i], i, i + 1);
	}

	for (unsigned int i = 0; i < 100; i++) {
		for (unsigned int j = 0; j <= i; j++) {
			if (pointers[i][j] != i) {
				fprintf(stderr, TOY_CC_ERROR "ERROR: unexpected bucket contents\n" TOY_CC_RESET);
				Toy_freeBucket(&bucket);
				return -1;
			}
		}
		if (i != 50) {
			Toy_releaseBucketPartition(pointers[i]);
		}
	}

	Toy_collectBucketGarbage(&bucket);
	if (pointers[50][0] != 50 || pointers[50][50] != 50) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: unexpected bucket contents\n" TOY_CC_RESET);
		Toy_freeBucket(&bucket);
		return -1;
	}
	Toy_releaseBucketPartition(pointers[50]);

	Toy_collectBucketGarbage(&bucket);
	if (bucket->next != NULL) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: unexpected bucket allocation state\n" TOY_CC_RESET);
		Toy_freeBucket(&bucket);
		return -1;
	}

	//test an allocation larger than the initial bucket
	unsigned char* large = Toy_partitionBucket(&bucket, 1024 * 1024);
	memset(large, 42, 1024 * 1024);
	if ((uintptr_t)large % _Alignof(max_align_t) != 0) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: unexpected bucket allocation state\n" TOY_CC_RESET);
		Toy_freeBucket(&bucket);
		return -1;
	}
	Toy_releaseBucketPartition(large);

	Toy_collectBucketGarbage(&bucket);
	Toy_freeBucket(&bucket);
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
