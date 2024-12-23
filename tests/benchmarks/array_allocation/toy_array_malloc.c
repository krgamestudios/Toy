#include "toy_array.h"
#include "toy_console_colors.h"

#include <stdio.h>
#include <stdlib.h>

Toy_Array* Toy_resizeArray(Toy_Array* paramArray, unsigned int capacity) {
	if (capacity == 0) {
		free(paramArray);
		return NULL;
	}

	//if some values will be removed, free them first
	if (paramArray != NULL && paramArray->count > capacity) {
		for (unsigned int i = capacity; i < paramArray->count; i++) {
			Toy_freeValue(paramArray->data[i]);
		}
	}

	unsigned int originalCapacity = paramArray == NULL ? 0 : paramArray->capacity;

	Toy_Array* array = realloc(paramArray, capacity * sizeof(Toy_Value) + sizeof(Toy_Array));

	if (array == NULL) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to resize a 'Toy_Array' from %d to %d capacity\n" TOY_CC_RESET, (int)originalCapacity, (int)capacity);
		exit(-1);
	}

	array->capacity = capacity;
	array->count = paramArray == NULL ? 0 :
		(array->count > capacity ? capacity : array->count); //truncate lost data

	return array;
}