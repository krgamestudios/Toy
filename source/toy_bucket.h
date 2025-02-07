#pragma once

#include "toy_common.h"

//NOTE: this is an 'arena allocator', and has restrictions on it's usage:
// - It can only expand until it is freed
// - It cannot be copied or moved around in memory
// - It cannot allocate more memory than it has 'capacity'
// If each of these rules are followed, this is actually more efficient than other options

//a custom allocator
typedef struct Toy_Bucket {  //32 | 64 BITNESS
	struct Toy_Bucket* next; //4  | 8
	unsigned int capacity;   //4  | 4
	unsigned int count;      //4  | 4
	unsigned char data[];    //-  | -
} Toy_Bucket;                //12 | 16

TOY_API Toy_Bucket* Toy_allocateBucket(unsigned int capacity);
TOY_API unsigned char* Toy_partitionBucket(Toy_Bucket** bucketHandle, unsigned int amount);
TOY_API void Toy_freeBucket(Toy_Bucket** bucketHandle);

//standard capacity sizes
#ifndef TOY_BUCKET_1KB
#define TOY_BUCKET_1KB (1 << 10)
#endif

#ifndef TOY_BUCKET_2KB
#define TOY_BUCKET_2KB (1 << 11)
#endif

#ifndef TOY_BUCKET_4KB
#define TOY_BUCKET_4KB (1 << 12)
#endif

#ifndef TOY_BUCKET_8KB
#define TOY_BUCKET_8KB (1 << 13)
#endif

#ifndef TOY_BUCKET_16KB
#define TOY_BUCKET_16KB (1 << 14)
#endif

#ifndef TOY_BUCKET_32KB
#define TOY_BUCKET_32KB (1 << 15)
#endif

#ifndef TOY_BUCKET_64KB
#define TOY_BUCKET_64KB (1 << 16)
#endif

//CPU L1 caches tend to be 64kb, but that's far from guaranteed
#ifndef TOY_BUCKET_IDEAL
#define TOY_BUCKET_IDEAL (TOY_BUCKET_64KB - sizeof(Toy_Bucket))
#endif

