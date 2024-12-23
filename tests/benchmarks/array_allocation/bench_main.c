#include "toy.h"

//util macros
#define TOY_ARRAY_EXPAND(array) ((array) = ((array) != NULL && (array)->count + 1 > (array)->capacity ? Toy_resizeArray((array), (array)->capacity * TOY_ARRAY_EXPANSION_RATE) : (array)))
#define TOY_ARRAY_PUSHBACK(array, value) (TOY_ARRAY_EXPAND(array), (array)->data[(array)->count++] = (value))

void stress_fillArray(Toy_Array** array) {
	//Toy_Value is either 8 or 16 bytes
	for (int i = 0; i < 10 * 1000 * 1000; i++) {
		TOY_ARRAY_PUSHBACK(*array, TOY_VALUE_FROM_INTEGER(i));
	}
}

int main() {
	//Compare different memory strategies for Toy_Array

	for (int i = 0; i < 100; i++) {
		/*

		//malloc
		Toy_Array* array = Toy_resizeArray(NULL, TOY_ARRAY_INITIAL_CAPACITY);
		stress_fillArray(&array);
		Toy_resizeArray(array, 0);

		/*/

		//Toy_Bucket
		benchBucket = Toy_allocateBucket(1024 * 1024 * 200); //200MB

		Toy_Array* array = Toy_resizeArray(NULL, TOY_ARRAY_INITIAL_CAPACITY);
		stress_fillArray(&array);
		Toy_resizeArray(array, 0);

		Toy_freeBucket(&benchBucket);

		//*/
	}

	return 0;
}
