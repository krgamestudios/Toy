#include "toy_array.h"
#include "toy_console_colors.h"

#include "toy_bucket.h"

#include <string.h>

Toy_Bucket* benchBucket = NULL;

Toy_Array* Toy_resizeArray(Toy_Array* paramArray, unsigned int capacity) {
	//allow the array to be 'lost', and freed with the bucket
	if (capacity == 0) {
		return NULL;
	}

	//initial allocation
	if (paramArray == NULL) {
		Toy_Array* array = Toy_partitionBucket(&benchBucket, capacity * sizeof(Toy_Value) + sizeof(Toy_Array));

		array->capacity = capacity;
		array->count = 0;

		return array;
	}

	//if your array is growing, partition more space, then copy over the data
	if (paramArray->capacity < capacity) {
		Toy_Array* array = Toy_partitionBucket(&benchBucket, capacity * sizeof(Toy_Value) + sizeof(Toy_Array));

		memcpy(array, paramArray, paramArray->count * sizeof(Toy_Value) + sizeof(Toy_Array)); //doesn't copy any blank space

		array->capacity = capacity;
		array->count = paramArray->count;
		return array;
	}

	//if some values will be removed, free them first, then return the result
	if (paramArray->count > capacity) {
		for (unsigned int i = capacity; i < paramArray->count; i++) {
			Toy_freeValue(paramArray->data[i]);
		}

		paramArray->capacity = capacity; //don't worry about another allocation, this is faster
		paramArray->count = capacity;

		return paramArray;
	}

	//unreachable
	return paramArray;
}

/*

Note: This needs to be pasted in the header:

```
struct Toy_Bucket;

extern struct Toy_Bucket* benchBucket;
```

*/