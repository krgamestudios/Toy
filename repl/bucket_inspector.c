#include "bucket_inspector.h"
// #include <toy_string.h>

#include <stdio.h>

int inspect_bucket(Toy_Bucket** bucketHandle) {
	int depth = 0;

	//for each bucket
	for (Toy_Bucket* iter = (*bucketHandle); iter != NULL; iter = iter->next) {
		int occupied = 0;
		int released = 0;
		unsigned char* ptr = iter->data;


		while ((ptr - iter->data < iter->count) && *((int*)ptr) != 0) { //for each partition
			if ( ( *((int*)ptr) & 1) == 0) { //is this partition still in use?
				occupied++;
			}
			else {
				released++;
			}

			//jump distance:   ((*((int*)ptr) | 1) ^ 1) + 4
			// printf(" jump %d, ", ((*((int*)ptr) | 1) ^ 1) + 4);
			ptr += ((*((int*)ptr) | 1) ^ 1) + 4; //OR + XOR to remove the 'free' flag from the size
		}

		printf("Bucket link %d: count %u, %d occupied, %d released\n", depth, iter->count, occupied, released);

		depth++;
	}

	printf("\n");

	return depth;
}

/*
int inspect_bucket_for_strings(Toy_Bucket** bucketHandle) {
	int depth = 0;

	//for each bucket
	for (Toy_Bucket* iter = (*bucketHandle); iter != NULL; iter = iter->next) {
		int occupied = 0;
		int released = 0;
		unsigned char* ptr = iter->data;


		while ((ptr - iter->data < iter->count) && *((int*)ptr) != 0) { //for each partition
			if ( ( *((int*)ptr) & 1) == 0) { //is this partition still in use?
				occupied++;

				//try to print as a string if possible
				Toy_String* str = (void*)(ptr + 4);

				if (str->info.type == TOY_STRING_LEAF && str->info.length < 255) {
					printf("String Leaf (%d bytes, %d refCount): %.*s\n", *((int*)ptr), str->info.refCount, str->info.length, str->leaf.data);
				}
				else if (str->info.type == TOY_STRING_NODE) {
					printf("String Node (%d bytes, %d refCount): ...\n", *((int*)ptr), str->info.refCount);
				}
			}
			else {
				released++;
			}

			//jump distance:   ((*((int*)ptr) | 1) ^ 1) + 4
			// printf(" jump %d, ", ((*((int*)ptr) | 1) ^ 1) + 4);
			ptr += ((*((int*)ptr) | 1) ^ 1) + 4; //OR + XOR to remove the 'free' flag from the size
		}

		printf("Bucket link %d: count %u, %d occupied, %d released\n", depth, iter->count, occupied, released);

		depth++;
	}

	printf("\n");

	return depth;
}
*/