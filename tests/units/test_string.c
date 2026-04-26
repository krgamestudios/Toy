#include "toy_string.h"
#include "toy_console_colors.h"

#include "toy_bucket.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_sizeof_string_64bit(void) {
	//test for the correct size
	{
		if (sizeof(Toy_String) != 32) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: 'Toy_String' is an unexpected size in memory: expected 32, found %d \n" TOY_CC_RESET, (int)sizeof(Toy_String));
			return -1;
		}
	}

	return 0;
}

int test_sizeof_string_32bit(void) {
	//test for the correct size
	{
		if (sizeof(Toy_String) != 24) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: 'Toy_String' is an unexpected size in memory: expected 24, found %d \n" TOY_CC_RESET, (int)sizeof(Toy_String));
			return -1;
		}
	}

	return 0;
}

int test_string_allocation(void) {
	//allocate a string within a bucket
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(1024);

		const char* cstring = "Hello world";
		Toy_String* str = Toy_createStringLength(&bucket, cstring, strlen(cstring));

		//check
		if (str->info.type != TOY_STRING_LEAF ||
			str->info.length != 11 ||
			str->info.refCount != 1 ||
			strcmp(str->leaf.data, "Hello world") != 0)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: 'Toy_createStringLength' failed\n" TOY_CC_RESET);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//free the string
		Toy_freeString(str);

		//inspect the bucket
		if (bucket->capacity != 1024 ||
			bucket->count != sizeof(Toy_String) + 12 ||
			bucket->next != NULL)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected bucket state after 'Toy_createStringLength'\n" TOY_CC_RESET);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//inspect the (now freed) string's memory
		if (Toy_getStringRefCount(str) != 0) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected string state after it was freed\n" TOY_CC_RESET);
			Toy_freeBucket(&bucket);
			return -1;
		}

		Toy_freeBucket(&bucket);
	}

	//copy a string
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(1024);

		const char* cstring = "Hello world";
		Toy_String* str = Toy_createStringLength(&bucket, cstring, strlen(cstring));

		//shallow and deep
		Toy_String* copy = Toy_copyString(str);

		if (str != copy ||
			str->info.refCount != 2)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to copy a string\n" TOY_CC_RESET);
			Toy_freeBucket(&bucket);
			return -1;
		}

		Toy_freeBucket(&bucket);
	}

	//allocate a zero-length string
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(1024);

		const char* cstring = "";
		Toy_String* str = Toy_createStringLength(&bucket, cstring, 0);

		//check
		if (str->info.type != TOY_STRING_LEAF ||
			str->info.length != 0 ||
			str->info.refCount != 1 ||
			strcmp(str->leaf.data, "") != 0)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to allocate a Toy_String with zero length\n" TOY_CC_RESET);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//free the string
		Toy_freeString(str);

		Toy_freeBucket(&bucket);
	}

	return 0;
}

int test_string_concatenation(void) {
	//concatenate two strings, and check the refcounts
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(1024);

		Toy_String* first = Toy_createStringLength(&bucket, "Hello ", 6);
		Toy_String* second = Toy_createStringLength(&bucket, "world", 5);

		//concatenate
		Toy_String* result = Toy_concatStrings(&bucket, first, second);

		//check the refcounts
		if (first->info.refCount != 2 ||
			second->info.refCount != 2 ||
			result->info.refCount != 1 ||
			result->info.length != 11)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected string states after concatenation\n" TOY_CC_RESET);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//clean up the separate strings
		Toy_freeString(first);
		Toy_freeString(second);

		//check the refcounts again
		if (first->info.refCount != 1 ||
			second->info.refCount != 1 ||
			result->info.refCount != 1 ||
			result->info.length != 11)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected string states after concatenation and free\n" TOY_CC_RESET);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//clean up
		Toy_freeString(result);
		Toy_freeBucket(&bucket);
	}

	//concatenate two strings, and check the resulting buffer
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(1024);

		Toy_String* first = Toy_createStringLength(&bucket, "Hello ", 6);
		Toy_String* second = Toy_createStringLength(&bucket, "world", 5);

		//concatenate
		Toy_String* result = Toy_concatStrings(&bucket, first, second);

		char* buffer = Toy_getStringRaw(result);

		//check the refcounts
		if (strlen(buffer) != 11 ||
			strncmp(buffer, "Hello world", 11) != 0)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected value of a concatenated string\n" TOY_CC_RESET);
			free(buffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		free(buffer);
		Toy_freeString(result);
		Toy_freeString(first);
		Toy_freeString(second);
		Toy_freeBucket(&bucket);
	}

	return 0;
}

int test_string_with_stressed_bucket(void) {
	//how much is that dog in the window?
	{
		//test data: 36 characters total, 44 with spaces
		char* testData[] = {
			"the",
			"quick",
			"brown",
			"fox",
			"jumped", //longest word: 6 characters
			"over",
			"the",
			"lazy",
			"dog", //9 entries long
			NULL,
		};

		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(128);//deliberately too much data for one bucket

		//stress
		Toy_String* str = Toy_createStringLength(&bucket, testData[0], strlen(testData[0]));
		Toy_String* ptr = str;
		for (int i = 1; testData[i]; i++) {
			str = Toy_concatStrings(&bucket, str, Toy_createStringLength(&bucket, testData[i], strlen(testData[i])));
		}

		//check
		if (ptr->info.refCount != 9 ||
			str->info.length != 36)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected state of the string after stress test\n" TOY_CC_RESET);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//grab the buffer
		char* buffer = Toy_getStringRaw(str);

		if (strncmp(buffer, "thequickbrownfoxjumpedoverthelazydog", 36) != 0 ||
			strlen(buffer) != 36)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected state of the string value after stress test: '%s'\n" TOY_CC_RESET, buffer);
			free(buffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		if (bucket->next == NULL) //just to make sure
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected state of the string bucket after stress test\n" TOY_CC_RESET);
			free(buffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//clean up
		free(buffer);
		Toy_freeBucket(&bucket);
	}

	//
	return 0;
}

int test_string_equality(void) {
	//simple string equality (no concats)
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(1024);
		Toy_String* helloWorldOne = Toy_createStringLength(&bucket, "Hello world", 11);
		Toy_String* helloWorldTwo = Toy_createStringLength(&bucket, "Hello world", 11);
		Toy_String* helloEveryone = Toy_createStringLength(&bucket, "Hello everyone", 14);

		int result = 0; //for print the errors

		//check match
		if ((result = Toy_compareStrings(helloWorldOne, helloWorldTwo)) != 0)
		{
			char* leftBuffer = Toy_getStringRaw(helloWorldOne);
			char* rightBuffer = Toy_getStringRaw(helloWorldTwo);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String equality '%s' == '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//check mismatch
		if ((result = Toy_compareStrings(helloWorldOne, helloEveryone)) == 0)
		{
			char* leftBuffer = Toy_getStringRaw(helloWorldOne);
			char* rightBuffer = Toy_getStringRaw(helloEveryone);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String equality '%s' != '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//check match (same object)
		if ((result = Toy_compareStrings(helloWorldOne, helloWorldOne)) != 0)
		{
			char* leftBuffer = Toy_getStringRaw(helloWorldOne);
			char* rightBuffer = Toy_getStringRaw(helloWorldOne);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String equality '%s' == '%s' is incorrect, found %s (these are the same object)\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_freeBucket(&bucket);
	}

	//string equality (with concat left)
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(1024);
		Toy_String* helloWorldOne = Toy_concatStrings(&bucket, Toy_createStringLength(&bucket, "Hello ", 6), Toy_createStringLength(&bucket, "world", 5));
		Toy_String* helloWorldTwo = Toy_createStringLength(&bucket, "Hello world", 11);
		Toy_String* helloEveryone = Toy_createStringLength(&bucket, "Hello everyone", 14);

		int result = 0; //for print the errors

		//check match
		if ((result = Toy_compareStrings(helloWorldOne, helloWorldTwo)) != 0)
		{
			char* leftBuffer = Toy_getStringRaw(helloWorldOne);
			char* rightBuffer = Toy_getStringRaw(helloWorldTwo);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String concat left equality '%s' == '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//check mismatch
		if ((result = Toy_compareStrings(helloWorldOne, helloEveryone)) == 0)
		{
			char* leftBuffer = Toy_getStringRaw(helloWorldOne);
			char* rightBuffer = Toy_getStringRaw(helloEveryone);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String concat left equality '%s' != '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//check match (same object)
		if ((result = Toy_compareStrings(helloWorldOne, helloWorldOne)) != 0)
		{
			char* leftBuffer = Toy_getStringRaw(helloWorldOne);
			char* rightBuffer = Toy_getStringRaw(helloWorldOne);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String concat left equality '%s' == '%s' is incorrect, found %s (these are the same object)\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_freeBucket(&bucket);
	}

	//string equality (with concat right)
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(1024);
		Toy_String* helloWorldOne = Toy_createStringLength(&bucket, "Hello world", 11);
		Toy_String* helloWorldTwo = Toy_concatStrings(&bucket, Toy_createStringLength(&bucket, "Hello ", 6), Toy_createStringLength(&bucket, "world", 5));

		int result = 0; //for print the errors

		//check match
		if ((result = Toy_compareStrings(helloWorldOne, helloWorldTwo)) != 0)
		{
			char* leftBuffer = Toy_getStringRaw(helloWorldOne);
			char* rightBuffer = Toy_getStringRaw(helloWorldTwo);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String concat right equality '%s' == '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_freeBucket(&bucket);
	}

	//string equality (with concat both)
	{
		//setup - these concat points are deliberately different
		Toy_Bucket* bucket = Toy_allocateBucket(1024);
		Toy_String* helloWorldOne = Toy_concatStrings(&bucket, Toy_createStringLength(&bucket, "Hello ", 6), Toy_createStringLength(&bucket, "world", 5));
		Toy_String* helloWorldTwo = Toy_concatStrings(&bucket, Toy_createStringLength(&bucket, "Hello", 5), Toy_createStringLength(&bucket, " world", 6));
		Toy_String* helloEveryone = Toy_concatStrings(&bucket, Toy_createStringLength(&bucket, "Hell", 4), Toy_createStringLength(&bucket, "world", 5));

		int result = 0; //for print the errors

		//check match
		if ((result = Toy_compareStrings(helloWorldOne, helloWorldTwo)) != 0)
		{
			char* leftBuffer = Toy_getStringRaw(helloWorldOne);
			char* rightBuffer = Toy_getStringRaw(helloWorldTwo);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String concat both equality '%s' == '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//check mismatch
		if ((result = Toy_compareStrings(helloWorldOne, helloEveryone)) == 0)
		{
			char* leftBuffer = Toy_getStringRaw(helloWorldOne);
			char* rightBuffer = Toy_getStringRaw(helloEveryone);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String concat both equality '%s' != '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_freeBucket(&bucket);
	}

	//string equality (with concat arbitrary)
	{
		//setup - The quick brown fox jumps over the lazy dog.
		Toy_Bucket* bucket = Toy_allocateBucket(1024);
		Toy_String* helloWorldOne = Toy_concatStrings(&bucket,
			Toy_createStringLength(&bucket, "The quick brown ", 16),
			Toy_concatStrings(&bucket,
				Toy_createStringLength(&bucket, "fox jumps o", 11),
				Toy_createStringLength(&bucket, "ver the lazy dog.", 17)
			)
		);

		Toy_String* helloWorldTwo = Toy_concatStrings(&bucket,
			Toy_concatStrings(&bucket,
				Toy_createStringLength(&bucket, "The quick brown fox", 19),
				Toy_concatStrings(&bucket,
					Toy_createStringLength(&bucket, " jumps ove", 10),
					Toy_createStringLength(&bucket, "r th", 4)
				)
			),
			Toy_concatStrings(&bucket,
				Toy_createStringLength(&bucket, "e lazy dog", 10),
				Toy_createStringLength(&bucket, ".", 1)
			)
		);

		Toy_String* helloEveryone = Toy_concatStrings(&bucket,
			Toy_createStringLength(&bucket, "The quick brown fox jumps over ", 31),
			Toy_concatStrings(&bucket,
				Toy_createStringLength(&bucket, "the lazy ", 9),
				Toy_createStringLength(&bucket, "reddit mod.", 11)
			)
		);

		int result = 0; //for print the errors

		//check match
		if ((result = Toy_compareStrings(helloWorldOne, helloWorldTwo)) != 0)
		{
			char* leftBuffer = Toy_getStringRaw(helloWorldOne);
			char* rightBuffer = Toy_getStringRaw(helloWorldTwo);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String concat arbitrary equality '%s' == '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//check mismatch
		if ((result = Toy_compareStrings(helloWorldOne, helloEveryone)) == 0)
		{
			char* leftBuffer = Toy_getStringRaw(helloWorldOne);
			char* rightBuffer = Toy_getStringRaw(helloEveryone);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String concat arbitrary equality '%s' != '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_freeBucket(&bucket);
	}

	//string equality (empty strings, no concats)
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(1024);
		Toy_String* helloWorldOne = Toy_createStringLength(&bucket, "", 0);
		Toy_String* helloWorldTwo = Toy_createStringLength(&bucket, "", 0);
		Toy_String* helloEveryone = Toy_createStringLength(&bucket, "Hello everyone", 14);

		int result = 0; //for print the errors

		//check match
		if ((result = Toy_compareStrings(helloWorldOne, helloWorldTwo)) != 0)
		{
			char* leftBuffer = Toy_getStringRaw(helloWorldOne);
			char* rightBuffer = Toy_getStringRaw(helloWorldTwo);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String equality empty '%s' == '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//check mismatch
		if ((result = Toy_compareStrings(helloWorldOne, helloEveryone)) == 0)
		{
			char* leftBuffer = Toy_getStringRaw(helloWorldOne);
			char* rightBuffer = Toy_getStringRaw(helloEveryone);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String equality empty '%s' != '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//check mismatch (same object)
		if ((result = Toy_compareStrings(helloWorldOne, helloWorldOne)) != 0)
		{
			char* leftBuffer = Toy_getStringRaw(helloWorldOne);
			char* rightBuffer = Toy_getStringRaw(helloWorldOne);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String equality empty '%s' == '%s' is incorrect, found %s (these are the same object)\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_freeBucket(&bucket);
	}

	//string equality (empty strings, deep concats)
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(1024);
		Toy_String* helloWorldOne = Toy_concatStrings(&bucket,
			Toy_createStringLength(&bucket, "", 0),
			Toy_concatStrings(&bucket,
				Toy_createStringLength(&bucket, "", 0),
				Toy_createStringLength(&bucket, "", 0)
			)
		);

		Toy_String* helloWorldTwo = Toy_concatStrings(&bucket,
			Toy_concatStrings(&bucket,
				Toy_createStringLength(&bucket, "", 0),
				Toy_concatStrings(&bucket,
					Toy_createStringLength(&bucket, "", 0),
					Toy_createStringLength(&bucket, "", 0)
				)
			),
			Toy_concatStrings(&bucket,
				Toy_createStringLength(&bucket, "", 0),
				Toy_createStringLength(&bucket, "", 0)
			)
		);

		Toy_String* helloEveryone = Toy_concatStrings(&bucket,
			Toy_createStringLength(&bucket, "The quick brown fox jumps over ", 31),
			Toy_concatStrings(&bucket,
				Toy_createStringLength(&bucket, "the lazy ", 9),
				Toy_createStringLength(&bucket, "reddit mod.", 11)
			)
		);

		int result = 0; //for print the errors

		//check match
		if ((result = Toy_compareStrings(helloWorldOne, helloWorldTwo)) != 0)
		{
			char* leftBuffer = Toy_getStringRaw(helloWorldOne);
			char* rightBuffer = Toy_getStringRaw(helloWorldTwo);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String equality empty with concats '%s' == '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//check mismatch
		if ((result = Toy_compareStrings(helloWorldOne, helloEveryone)) == 0)
		{
			char* leftBuffer = Toy_getStringRaw(helloWorldOne);
			char* rightBuffer = Toy_getStringRaw(helloEveryone);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String equality empty with concats '%s' != '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//check match (same object)
		if ((result = Toy_compareStrings(helloWorldOne, helloWorldOne)) != 0)
		{
			char* leftBuffer = Toy_getStringRaw(helloWorldOne);
			char* rightBuffer = Toy_getStringRaw(helloWorldOne);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String equality empty with concats '%s' == '%s' is incorrect, found %s (these are the same object)\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_freeBucket(&bucket);
	}

	//substring non-equality (no concats)
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(1024);
		Toy_String* left = Toy_createStringLength(&bucket, "identity", 8);
		Toy_String* right = Toy_createStringLength(&bucket, "ident", 5);

		int result = 0; //for print the errors

		//check mismatch
		if ((result = Toy_compareStrings(left, right)) == 0)
		{
			char* leftBuffer = Toy_getStringRaw(left);
			char* rightBuffer = Toy_getStringRaw(right);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String equality '%s' != '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_freeBucket(&bucket);
	}

	//substring non-equality (with matching and non-matching concats)
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(1024);
		Toy_String* identity = Toy_createStringLength(&bucket, "identity", 8);
		Toy_String* ident = Toy_createStringLength(&bucket, "ident", 5);

		Toy_String* matchingIdentity = Toy_concatStrings(&bucket,
				Toy_createStringLength(&bucket, "ident", 5),
				Toy_createStringLength(&bucket, "ity", 3)
			);

		Toy_String* stolenIdentity = Toy_concatStrings(&bucket,
				Toy_createStringLength(&bucket, "id", 2),
				Toy_createStringLength(&bucket, "entity", 6)
			);

		int result = 0; //for print the errors

		//ensure the concats match the base
		if ((result = Toy_compareStrings(identity, matchingIdentity)) != 0)
		{
			char* leftBuffer = Toy_getStringRaw(identity);
			char* rightBuffer = Toy_getStringRaw(matchingIdentity);
			fprintf(stderr, TOY_CC_ERROR "ERROR: Substring concat non-equality failed early on line %d with '%s' and '%s' is incorrect, found %s\n" TOY_CC_RESET, __LINE__, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		if ((result = Toy_compareStrings(identity, stolenIdentity)) != 0)
		{
			char* leftBuffer = Toy_getStringRaw(identity);
			char* rightBuffer = Toy_getStringRaw(stolenIdentity);
			fprintf(stderr, TOY_CC_ERROR "ERROR: Substring concat non-equality failed early on line %d with '%s' and '%s' is incorrect, found %s\n" TOY_CC_RESET, __LINE__, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//ensure both concats are a mismatch for 'ident'
		if ((result = Toy_compareStrings(ident, matchingIdentity)) == 0)
		{
			char* leftBuffer = Toy_getStringRaw(ident);
			char* rightBuffer = Toy_getStringRaw(matchingIdentity);
			fprintf(stderr, TOY_CC_ERROR "ERROR: Substring concat non-equality failed on line %d with '%s' and '%s' is incorrect, found %s\n" TOY_CC_RESET, __LINE__, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		if ((result = Toy_compareStrings(ident, stolenIdentity)) == 0)
		{
			char* leftBuffer = Toy_getStringRaw(ident);
			char* rightBuffer = Toy_getStringRaw(stolenIdentity);
			fprintf(stderr, TOY_CC_ERROR "ERROR: Substring concat non-equality failed  on line %d with '%s' and '%s' is incorrect, found %s\n" TOY_CC_RESET, __LINE__, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//repeat these tests with the parameters swapped, just to be safe
		if ((result = Toy_compareStrings(matchingIdentity, identity)) != 0)
		{
			char* leftBuffer = Toy_getStringRaw(matchingIdentity);
			char* rightBuffer = Toy_getStringRaw(identity);
			fprintf(stderr, TOY_CC_ERROR "ERROR: Substring concat non-equality failed early on line %d with '%s' and '%s' is incorrect, found %s\n" TOY_CC_RESET, __LINE__, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		if ((result = Toy_compareStrings(stolenIdentity, identity)) != 0)
		{
			char* leftBuffer = Toy_getStringRaw(stolenIdentity);
			char* rightBuffer = Toy_getStringRaw(identity);
			fprintf(stderr, TOY_CC_ERROR "ERROR: Substring concat non-equality failed early on line %d with '%s' and '%s' is incorrect, found %s\n" TOY_CC_RESET, __LINE__, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		if ((result = Toy_compareStrings(matchingIdentity, ident)) == 0)
		{
			char* leftBuffer = Toy_getStringRaw(matchingIdentity);
			char* rightBuffer = Toy_getStringRaw(ident);
			fprintf(stderr, TOY_CC_ERROR "ERROR: Substring concat non-equality failed on line %d with '%s' and '%s' is incorrect, found %s\n" TOY_CC_RESET, __LINE__, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		if ((result = Toy_compareStrings(stolenIdentity, ident)) == 0)
		{
			char* leftBuffer = Toy_getStringRaw(stolenIdentity);
			char* rightBuffer = Toy_getStringRaw(ident);
			fprintf(stderr, TOY_CC_ERROR "ERROR: Substring concat non-equality failed  on line %d with '%s' and '%s' is incorrect, found %s\n" TOY_CC_RESET, __LINE__, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}



		//cleanup
		Toy_freeBucket(&bucket);
	}


	return 0;
}

int test_string_diffs(void) {
	//simple string diffs (no concats)
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(1024);
		Toy_String* helloEveryone = Toy_createStringLength(&bucket, "Hello everyone", 14);
		Toy_String* helloUniverse = Toy_createStringLength(&bucket, "Hello universe", 14);

		int result = 0; //for print the errors

		//check diff
		if (((result = Toy_compareStrings(helloEveryone, helloUniverse)) < 0) == false)
		{
			char* leftBuffer = Toy_getStringRaw(helloEveryone);
			char* rightBuffer = Toy_getStringRaw(helloUniverse);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String diff '%s' == '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//check diff (reversed)
		if (((result = Toy_compareStrings(helloUniverse, helloEveryone)) > 0) == false)
		{
			char* leftBuffer = Toy_getStringRaw(helloUniverse);
			char* rightBuffer = Toy_getStringRaw(helloEveryone);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String diff '%s' == '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_freeBucket(&bucket);
	}

	//string diffs (with concat arbitrary)
	{
		//setup - The quick brown fox jumps over the lazy dog.
		Toy_Bucket* bucket = Toy_allocateBucket(1024);
		Toy_String* pangram = Toy_concatStrings(&bucket,
			Toy_createStringLength(&bucket, "The quick brown ", 16),
			Toy_concatStrings(&bucket,
				Toy_createStringLength(&bucket, "fox jumps o", 11),
				Toy_createStringLength(&bucket, "ver the lazy dog.", 17)
			)
		);

		Toy_String* neckbeard = Toy_concatStrings(&bucket,
			Toy_createStringLength(&bucket, "The quick brown fox jumps over ", 31),
			Toy_concatStrings(&bucket,
				Toy_createStringLength(&bucket, "the lazy ", 9),
				Toy_createStringLength(&bucket, "reddit mod.", 11)
			)
		);

		int result = 0; //for print the errors

		//check diff
		if (((result = Toy_compareStrings(pangram, neckbeard)) < 0) == false)
		{
			char* leftBuffer = Toy_getStringRaw(pangram);
			char* rightBuffer = Toy_getStringRaw(neckbeard);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String diff '%s' == '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//check diff (reversed)
		if (((result = Toy_compareStrings(neckbeard, pangram)) > 0) == false)
		{
			char* leftBuffer = Toy_getStringRaw(neckbeard);
			char* rightBuffer = Toy_getStringRaw(pangram);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String diff '%s' == '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
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
#if TOY_BITNESS == 64
		res = test_sizeof_string_64bit();
#else
		res = test_sizeof_string_32bit();
#endif
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		res = test_string_allocation();
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		res = test_string_concatenation();
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		res = test_string_with_stressed_bucket();
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		res = test_string_equality();
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		res = test_string_diffs();
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	//NOTE: string fragmentation is no longer supported

	return total;
}