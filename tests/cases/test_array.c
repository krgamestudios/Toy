#include "toy_array.h"
#include "toy_console_colors.h"

#include <stdio.h>

int test_array(void) {
	//test allocation and free
	{
		Toy_Array* array = TOY_ARRAY_ALLOCATE();
		TOY_ARRAY_FREE(array);
	}

	//test initial data
	{
		Toy_Array* array = TOY_ARRAY_ALLOCATE();

		//check you can access the memory
		array->data[1] = TOY_VALUE_FROM_INTEGER(42);

		TOY_ARRAY_FREE(array);
	}

	//test multiple arrays (no overlaps or conflicts)
	{
		Toy_Array* array1 = TOY_ARRAY_ALLOCATE();
		Toy_Array* array2 = TOY_ARRAY_ALLOCATE();

		array1->data[1] = TOY_VALUE_FROM_INTEGER(42);
		array2->data[1] = TOY_VALUE_FROM_INTEGER(42);

		TOY_ARRAY_FREE(array1);
		TOY_ARRAY_FREE(array2);
	}

	return 0;
}

int main(void) {
	//run each test set, returning the total errors given
	int total = 0, res = 0;

	{
		res = test_array();
		total += res;

		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
	}

	return total;
}
