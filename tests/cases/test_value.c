#include "toy_value.h"
#include "toy_console_colors.h"

#include "toy_bucket.h"
#include "toy_string.h"
#include "toy_array.h"

#include <stdio.h>
#include <string.h>

int test_value_creation() {
	//test for the correct size
	{
#if TOY_BITNESS == 64
		if (sizeof(Toy_Value) != 16)
#else
		if (sizeof(Toy_Value) != 8)
#endif
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: 'Toy_Value' is an unexpected size in memory, expected %d found %d\n" TOY_CC_RESET, TOY_BITNESS, (int)sizeof(Toy_Value));
			return -1;
		}
	}

	//test creating a null
	{
		Toy_Value v = TOY_VALUE_FROM_NULL();

		if (!TOY_VALUE_IS_NULL(v)) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: creating a 'null' value failed\n" TOY_CC_RESET);
			return -1;
		}
	}

	//test creating booleans
	{
		Toy_Value t = TOY_VALUE_FROM_BOOLEAN(true);
		Toy_Value f = TOY_VALUE_FROM_BOOLEAN(false);

		if (!Toy_checkValueIsTruthy(t) || Toy_checkValueIsTruthy(f)) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: 'boolean' value failed\n" TOY_CC_RESET);
			return -1;
		}
	}

	//test creating strings
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_SMALL);

		Toy_Value greeting = TOY_VALUE_FROM_STRING(Toy_createString(&bucket, "Hello world!"));

		if (TOY_VALUE_IS_STRING(greeting) == false ||
			TOY_VALUE_AS_STRING(greeting)->type != TOY_STRING_LEAF ||
			strcmp(TOY_VALUE_AS_STRING(greeting)->as.leaf.data, "Hello world!") != 0
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: 'string' value failed\n" TOY_CC_RESET);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_freeBucket(&bucket);
	}

	//test creating arrays
	{
		//setup
		Toy_Array* array = TOY_ARRAY_ALLOCATE();
		TOY_ARRAY_PUSHBACK(array, TOY_VALUE_FROM_INTEGER(42));
		TOY_ARRAY_PUSHBACK(array, TOY_VALUE_FROM_INTEGER(69));
		TOY_ARRAY_PUSHBACK(array, TOY_VALUE_FROM_INTEGER(8891));

		Toy_Value v = TOY_VALUE_FROM_ARRAY(array);

		if (TOY_VALUE_AS_ARRAY(v) == false ||
			TOY_VALUE_AS_ARRAY(v)->capacity != 8 ||
			TOY_VALUE_AS_ARRAY(v)->count != 3 ||
			TOY_VALUE_IS_INTEGER(TOY_VALUE_AS_ARRAY(v)->data[0]) != true || TOY_VALUE_AS_INTEGER(TOY_VALUE_AS_ARRAY(v)->data[0]) != 42 ||
			TOY_VALUE_IS_INTEGER(TOY_VALUE_AS_ARRAY(v)->data[1]) != true || TOY_VALUE_AS_INTEGER(TOY_VALUE_AS_ARRAY(v)->data[1]) != 69 ||
			TOY_VALUE_IS_INTEGER(TOY_VALUE_AS_ARRAY(v)->data[2]) != true || TOY_VALUE_AS_INTEGER(TOY_VALUE_AS_ARRAY(v)->data[2]) != 8891
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: 'array' value failed\n" TOY_CC_RESET);
			TOY_ARRAY_FREE(array);
			return -1;
		}
	}

	return 0;
}

//URGENT: copy values

int test_value_hashing() {
	//test value hashing
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);

		//values
		Toy_Value n = TOY_VALUE_FROM_NULL();
		Toy_Value t = TOY_VALUE_FROM_BOOLEAN(true);
		Toy_Value f = TOY_VALUE_FROM_BOOLEAN(false);
		Toy_Value i = TOY_VALUE_FROM_INTEGER(42);
		//skip float
		Toy_Value s = TOY_VALUE_FROM_STRING(Toy_createString(&bucket, "Hello world"));

		Toy_Array* array = TOY_ARRAY_ALLOCATE();
		TOY_ARRAY_PUSHBACK(array, TOY_VALUE_FROM_INTEGER(42));
		TOY_ARRAY_PUSHBACK(array, TOY_VALUE_FROM_INTEGER(69));
		TOY_ARRAY_PUSHBACK(array, TOY_VALUE_FROM_INTEGER(8891));
		Toy_Value a = TOY_VALUE_FROM_ARRAY(array);

		if (Toy_hashValue(n) != 0 ||
			Toy_hashValue(t) != 1 ||
			Toy_hashValue(f) != 0 ||
			Toy_hashValue(i) != 4147366645 ||
			Toy_hashValue(s) != 994097935 ||
			TOY_VALUE_AS_STRING(s)->cachedHash == 0 ||
			Toy_hashValue(a) != 2544446955
			)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected hash of a value\n" TOY_CC_RESET);
			TOY_ARRAY_FREE(array);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		TOY_ARRAY_FREE(array);
		Toy_freeBucket(&bucket);
	}

	//URGENT: string hashing, use godbolt

	return 0;
}

int test_value_equality() {
	//test value equality
	{
		Toy_Value answer = TOY_VALUE_FROM_INTEGER(42);
		Toy_Value question = TOY_VALUE_FROM_INTEGER(42);
		Toy_Value nice = TOY_VALUE_FROM_INTEGER(69);

		if (Toy_checkValuesAreEqual(answer, question) != true) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: value equality check failed, expected true\n" TOY_CC_RESET);
			return -1;
		}

		if (Toy_checkValuesAreEqual(answer, nice) != false) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: value equality check failed, expected false\n" TOY_CC_RESET);
			return -1;
		}
	}

	//again with strings
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_SMALL);

		Toy_Value answer = TOY_VALUE_FROM_STRING(Toy_createString(&bucket, "poe wrote on both"));
		Toy_Value question = TOY_VALUE_FROM_STRING(Toy_createString(&bucket, "why is a raven like a writing desk?"));
		Toy_Value duplicate = TOY_VALUE_FROM_STRING(Toy_createString(&bucket, "poe wrote on both"));

		if (Toy_checkValuesAreEqual(answer, duplicate) != true) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: string value equality check failed, expected true\n" TOY_CC_RESET);
			return -1;
		}

		if (Toy_checkValuesAreEqual(answer, question) != false) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: string value equality check failed, expected false\n" TOY_CC_RESET);
			return -1;
		}

		//cleanup
		Toy_freeBucket(&bucket);
	}

	return 0;
}

int test_value_comparison() {
	//test value comparable
	{
		Toy_Value answer = TOY_VALUE_FROM_INTEGER(42);
		Toy_Value question = TOY_VALUE_FROM_INTEGER(42);
		Toy_Value nope = TOY_VALUE_FROM_NULL();

		if (Toy_checkValuesAreComparable(answer, question) != true) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: value comparison check failed, expected true\n" TOY_CC_RESET);
			return -1;
		}

		if (Toy_checkValuesAreComparable(answer, nope) != false) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: value comparison check failed, expected false\n" TOY_CC_RESET);
			return -1;
		}
	}

	//test comparison
	{
		Toy_Value answer = TOY_VALUE_FROM_INTEGER(42);
		Toy_Value question = TOY_VALUE_FROM_INTEGER(42);
		Toy_Value nice = TOY_VALUE_FROM_INTEGER(69);

		if (Toy_compareValues(answer, question) != 0) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: value comparison failed, expected 0\n" TOY_CC_RESET);
			return -1;
		}

		if (Toy_compareValues(answer, nice) == 0) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: value comparison failed, expected not 0\n" TOY_CC_RESET);
			return -1;
		}
	}

	//again with strings
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_SMALL);

		Toy_Value answer = TOY_VALUE_FROM_STRING(Toy_createString(&bucket, "poe wrote on both"));
		Toy_Value question = TOY_VALUE_FROM_STRING(Toy_createString(&bucket, "why is a raven like a writing desk?"));
		Toy_Value duplicate = TOY_VALUE_FROM_STRING(Toy_createString(&bucket, "poe wrote on both"));

		if (Toy_compareValues(answer, duplicate) != 0) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: string value comparison failed, expected 0\n" TOY_CC_RESET);
			return -1;
		}

		if (Toy_compareValues(answer, question) == 0) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: string value comparison failed, expected not 0\n" TOY_CC_RESET);
			return -1;
		}

		//cleanup
		Toy_freeBucket(&bucket);
	}

	return 0;
}

int main() {
	//run each test set, returning the total errors given
	int total = 0, res = 0;

	{
		res = test_value_creation();
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	// { //URGENT: test copying
	// 	res = test_value_copying();
	// 	if (res == 0) {
	// 		printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
	// 	}
	// 	total += res;
	// }

	{
		res = test_value_hashing();
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		res = test_value_equality();
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		res = test_value_comparison();
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	// { //URGENT: test stringify
	// 	res = test_value_stringify();
	// 	if (res == 0) {
	// 		printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
	// 	}
	// 	total += res;
	// }

	//URGENT: arrays

	return total;
}
