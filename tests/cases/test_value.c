#include "toy_value.h"
#include "toy_console_colors.h"

#include "toy_bucket.h"
#include "toy_string.h"
#include "toy_array.h"

#include <stdio.h>
#include <stdlib.h>
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

		//cleanup
		TOY_ARRAY_FREE(array);
	}

	return 0;
}

int test_value_copying() {
	//test simple integer copy
	{
		Toy_Value original = TOY_VALUE_FROM_INTEGER(42);
		Toy_Value result = Toy_copyValue(original);

		if (!TOY_VALUE_IS_INTEGER(result) ||
			TOY_VALUE_AS_INTEGER(result) != 42
		) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: copy an integer value failed\n" TOY_CC_RESET);
			return -1;
		}
	}

	//test string copy
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_SMALL);

		Toy_Value original = TOY_VALUE_FROM_STRING(Toy_createString(&bucket, "Hello world!"));
		Toy_Value result = Toy_copyValue(original);

		if (TOY_VALUE_IS_STRING(result) == false ||
			TOY_VALUE_AS_STRING(result)->type != TOY_STRING_LEAF ||
			strcmp(TOY_VALUE_AS_STRING(result)->as.leaf.data, "Hello world!") != 0 ||
			TOY_VALUE_AS_STRING(result)->refCount != 2
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: copy a string value failed\n" TOY_CC_RESET);
			Toy_freeValue(original);
			Toy_freeValue(result);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_freeValue(original);
		Toy_freeValue(result);
		Toy_freeBucket(&bucket);
	}

	//test copy arrays
	{
		//setup
		Toy_Array* array = TOY_ARRAY_ALLOCATE();
		TOY_ARRAY_PUSHBACK(array, TOY_VALUE_FROM_INTEGER(42));
		TOY_ARRAY_PUSHBACK(array, TOY_VALUE_FROM_INTEGER(69));
		TOY_ARRAY_PUSHBACK(array, TOY_VALUE_FROM_INTEGER(8891));

		Toy_Value original = TOY_VALUE_FROM_ARRAY(array);

		Toy_Value result = Toy_copyValue(original);

		if (TOY_VALUE_AS_ARRAY(result) == false ||
			TOY_VALUE_AS_ARRAY(result)->capacity != 8 ||
			TOY_VALUE_AS_ARRAY(result)->count != 3 ||
			TOY_VALUE_IS_INTEGER(TOY_VALUE_AS_ARRAY(result)->data[0]) != true || TOY_VALUE_AS_INTEGER(TOY_VALUE_AS_ARRAY(result)->data[0]) != 42 ||
			TOY_VALUE_IS_INTEGER(TOY_VALUE_AS_ARRAY(result)->data[1]) != true || TOY_VALUE_AS_INTEGER(TOY_VALUE_AS_ARRAY(result)->data[1]) != 69 ||
			TOY_VALUE_IS_INTEGER(TOY_VALUE_AS_ARRAY(result)->data[2]) != true || TOY_VALUE_AS_INTEGER(TOY_VALUE_AS_ARRAY(result)->data[2]) != 8891
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: copy an array value failed\n" TOY_CC_RESET);
			Toy_freeValue(original);
			Toy_freeValue(result);
			return -1;
		}

		//cleanup
		Toy_freeValue(original);
		Toy_freeValue(result);
	}

	//arrays can't be compared

	return 0;
}

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

	//again with arrays
	{
		//setup
		Toy_Array* array1 = TOY_ARRAY_ALLOCATE();
		TOY_ARRAY_PUSHBACK(array1, TOY_VALUE_FROM_INTEGER(42));
		TOY_ARRAY_PUSHBACK(array1, TOY_VALUE_FROM_INTEGER(69));
		TOY_ARRAY_PUSHBACK(array1, TOY_VALUE_FROM_INTEGER(8891));

		Toy_Value value1 = TOY_VALUE_FROM_ARRAY(array1);

		Toy_Array* array2 = TOY_ARRAY_ALLOCATE();
		TOY_ARRAY_PUSHBACK(array2, TOY_VALUE_FROM_INTEGER(42));
		TOY_ARRAY_PUSHBACK(array2, TOY_VALUE_FROM_INTEGER(69));
		TOY_ARRAY_PUSHBACK(array2, TOY_VALUE_FROM_INTEGER(8891));

		Toy_Value value2 = TOY_VALUE_FROM_ARRAY(array2);


		if (Toy_checkValuesAreEqual(value1, value2) != true)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: array values are not equal\n" TOY_CC_RESET);
			Toy_freeValue(value1);
			Toy_freeValue(value2);
			return -1;
		}

		//cleanup
		Toy_freeValue(value1);
		Toy_freeValue(value2);
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

	//arrays can't be compared

	return 0;
}

int test_value_stringify() {
	//stringify null
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_SMALL);
		Toy_Value value = TOY_VALUE_FROM_NULL();

		//run
		Toy_String* string = Toy_stringifyValue(&bucket, value);

		//check
		if (string->type != TOY_STRING_LEAF ||
			strcmp(string->as.leaf.data, "null") != 0)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: stringify 'null' failed\n" TOY_CC_RESET);
			Toy_freeString(string);
			Toy_freeValue(value);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_freeString(string);
		Toy_freeValue(value);
		Toy_freeBucket(&bucket);
	}

	//stringify boolean (true)
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_SMALL);
		Toy_Value value = TOY_VALUE_FROM_BOOLEAN(true);

		//run
		Toy_String* string = Toy_stringifyValue(&bucket, value);

		//check
		if (string->type != TOY_STRING_LEAF ||
			strcmp(string->as.leaf.data, "true") != 0)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: stringify boolean 'true' failed\n" TOY_CC_RESET);
			Toy_freeString(string);
			Toy_freeValue(value);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_freeString(string);
		Toy_freeValue(value);
		Toy_freeBucket(&bucket);
	}

	//stringify boolean (false)
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_SMALL);
		Toy_Value value = TOY_VALUE_FROM_BOOLEAN(false);

		//run
		Toy_String* string = Toy_stringifyValue(&bucket, value);

		//check
		if (string->type != TOY_STRING_LEAF ||
			strcmp(string->as.leaf.data, "false") != 0)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: stringify boolean 'false' failed\n" TOY_CC_RESET);
			Toy_freeString(string);
			Toy_freeValue(value);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_freeString(string);
		Toy_freeValue(value);
		Toy_freeBucket(&bucket);
	}

	//stringify integer
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_SMALL);
		Toy_Value value = TOY_VALUE_FROM_INTEGER(42);

		//run
		Toy_String* string = Toy_stringifyValue(&bucket, value);

		//check
		if (string->type != TOY_STRING_LEAF ||
			strcmp(string->as.leaf.data, "42") != 0)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: stringify integer '42' failed\n" TOY_CC_RESET);
			Toy_freeString(string);
			Toy_freeValue(value);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_freeString(string);
		Toy_freeValue(value);
		Toy_freeBucket(&bucket);
	}

	//stringify float
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_SMALL);
		Toy_Value value = TOY_VALUE_FROM_FLOAT(3.1415f);

		//run
		Toy_String* string = Toy_stringifyValue(&bucket, value);

		//check
		if (string->type != TOY_STRING_LEAF ||
			strcmp(string->as.leaf.data, "3.1415") != 0)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: stringify float '3.1415' failed\n" TOY_CC_RESET);
			Toy_freeString(string);
			Toy_freeValue(value);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_freeString(string);
		Toy_freeValue(value);
		Toy_freeBucket(&bucket);
	}

	//stringify strings
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_SMALL);

		Toy_Value value = TOY_VALUE_FROM_STRING(Toy_createString(&bucket, "Hello world!"));
		Toy_String* string = Toy_stringifyValue(&bucket, value);
		char* buffer = Toy_getStringRawBuffer(string);

		if (buffer == NULL || strcmp(buffer, "Hello world!") != 0)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: stringify string 'Hello world!' failed\n" TOY_CC_RESET);
			free(buffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		free(buffer);
		Toy_freeBucket(&bucket);
	}

	//stringify array
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_SMALL);

		//setup
		Toy_Array* array = TOY_ARRAY_ALLOCATE();
		TOY_ARRAY_PUSHBACK(array, TOY_VALUE_FROM_INTEGER(42));
		TOY_ARRAY_PUSHBACK(array, TOY_VALUE_FROM_INTEGER(69));
		TOY_ARRAY_PUSHBACK(array, TOY_VALUE_FROM_INTEGER(8891));

		Toy_Value value = TOY_VALUE_FROM_ARRAY(array);
		Toy_String* string = Toy_stringifyValue(&bucket, value);
		char* buffer = Toy_getStringRawBuffer(string);

		if (buffer == NULL || strcmp(buffer, "[42,69,8891]") != 0)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: stringify array '[42,69,8891]' failed\n" TOY_CC_RESET);
			free(buffer);
			TOY_ARRAY_FREE(array);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		free(buffer);
		TOY_ARRAY_FREE(array);
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

	{
		res = test_value_copying();
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

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

	{
		res = test_value_stringify();
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	//TODO: references

	return total;
}
