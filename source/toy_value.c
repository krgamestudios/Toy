#include "toy_value.h"
#include "toy_console_colors.h"

#include "toy_bucket.h"
#include "toy_string.h"
#include "toy_array.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//utils
static unsigned int hashUInt(unsigned int x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

//exposed functions
Toy_Value Toy_unwrapValue(Toy_Value value) {
	//turns out C doesn't have actual references
	if (value.type == TOY_VALUE_REFERENCE) {
		return Toy_unwrapValue(*(value.as.reference));
	}
	else {
		return value;
	}
}

unsigned int Toy_hashValue(Toy_Value value) {
	value = Toy_unwrapValue(value);

	switch(value.type) {
		case TOY_VALUE_NULL:
			return 0;

		case TOY_VALUE_BOOLEAN:
			return value.as.boolean ? 1 : 0;

		case TOY_VALUE_INTEGER:
			return hashUInt((unsigned int)value.as.integer);

		case TOY_VALUE_FLOAT:
			return hashUInt( *((unsigned int*)(&value.as.number)) );

		case TOY_VALUE_STRING:
			return Toy_hashString(value.as.string);

		case TOY_VALUE_ARRAY: {
			//since array internals can change, recalc the hash each time it's needed
			Toy_Array* ptr = value.as.array;
			unsigned int hash = 0;

			for (unsigned int i = 0; i < ptr->count; i++) {
				hash ^= Toy_hashValue(ptr->data[i]);
			}

			return hash;
		}

		case TOY_VALUE_TABLE:
		case TOY_VALUE_FUNCTION:
		case TOY_VALUE_OPAQUE:
		case TOY_VALUE_TYPE:
		case TOY_VALUE_ANY:
		case TOY_VALUE_REFERENCE:
		case TOY_VALUE_UNKNOWN:
			fprintf(stderr, TOY_CC_ERROR "ERROR: Can't hash an unknown value type, exiting\n" TOY_CC_RESET);
			exit(-1);
	}

	return 0;
}

Toy_Value Toy_copyValue(Toy_Value value) {
	value = Toy_unwrapValue(value);

	switch(value.type) {
		case TOY_VALUE_NULL:
		case TOY_VALUE_BOOLEAN:
		case TOY_VALUE_INTEGER:
		case TOY_VALUE_FLOAT:
			return value;

		case TOY_VALUE_STRING: {
			return TOY_VALUE_FROM_STRING(Toy_copyString(value.as.string));
		}

		case TOY_VALUE_ARRAY: {
			//arrays probably won't get copied much
			Toy_Array* ptr = value.as.array;
			Toy_Array* result = Toy_resizeArray(NULL, ptr->capacity);

			for (unsigned int i = 0; i < ptr->count; i++) {
				result->data[i] = Toy_copyValue(ptr->data[i]);
			}

			result->capacity = ptr->capacity;
			result->count = ptr->count;

			return TOY_VALUE_FROM_ARRAY(result);
		}

		case TOY_VALUE_TABLE:
		case TOY_VALUE_FUNCTION:
		case TOY_VALUE_OPAQUE:
		case TOY_VALUE_TYPE:
		case TOY_VALUE_ANY:
		case TOY_VALUE_REFERENCE:
		case TOY_VALUE_UNKNOWN:
			fprintf(stderr, TOY_CC_ERROR "ERROR: Can't copy an unknown value type, exiting\n" TOY_CC_RESET);
			exit(-1);
	}

	//dummy return
	return TOY_VALUE_FROM_NULL();
}

void Toy_freeValue(Toy_Value value) {
	//NOTE: do not unwrap this value, as references shouldn't be freed

	switch(value.type) {
		case TOY_VALUE_NULL:
		case TOY_VALUE_BOOLEAN:
		case TOY_VALUE_INTEGER:
		case TOY_VALUE_FLOAT:
			break;

		case TOY_VALUE_STRING: {
			Toy_freeString(value.as.string);
			break;
		}

		case TOY_VALUE_ARRAY: {
			Toy_Array* ptr = value.as.array;

			for (unsigned int i = 0; i < ptr->count; i++) {
				Toy_freeValue(ptr->data[i]);
			}

			TOY_ARRAY_FREE(ptr);
			break;
		}

		case TOY_VALUE_TABLE:
		case TOY_VALUE_FUNCTION:
		case TOY_VALUE_OPAQUE:
		case TOY_VALUE_TYPE:
		case TOY_VALUE_ANY:
		case TOY_VALUE_REFERENCE:
		case TOY_VALUE_UNKNOWN:
			fprintf(stderr, TOY_CC_ERROR "ERROR: Can't free an unknown value type, exiting\n" TOY_CC_RESET);
			exit(-1);
	}
}

bool Toy_checkValueIsTruthy(Toy_Value value) {
	value = Toy_unwrapValue(value);

	//null is an error
	if (value.type == TOY_VALUE_NULL) {
		Toy_error("'null' is neither true nor false");
		return false;
	}

	//only 'false' is falsy
	if (value.type == TOY_VALUE_BOOLEAN) {
		return value.as.boolean;
	}

	//anything else is truthy
	return true;
}

bool Toy_checkValuesAreEqual(Toy_Value left, Toy_Value right) {
	left = Toy_unwrapValue(left);
	right = Toy_unwrapValue(right);

	switch(left.type) {
		case TOY_VALUE_NULL:
			return right.type == TOY_VALUE_NULL;

		case TOY_VALUE_BOOLEAN:
			return right.type == TOY_VALUE_NULL && left.as.boolean == right.as.boolean;

		case TOY_VALUE_INTEGER:
			if (right.type == TOY_VALUE_INTEGER) {
				return left.as.integer == right.as.integer;
			}
			else if (right.type == TOY_VALUE_FLOAT) {
				return left.as.integer == right.as.number;
			}
			else {
				break;
			}

		case TOY_VALUE_FLOAT:
			if (right.type == TOY_VALUE_INTEGER) {
				return left.as.number == right.as.integer;
			}
			else if (right.type == TOY_VALUE_FLOAT) {
				return left.as.number == right.as.number;
			}
			else {
				break;
			}

		case TOY_VALUE_STRING:
			if (right.type == TOY_VALUE_STRING) {
				return Toy_compareStrings(left.as.string, right.as.string) == 0;
			}
			else {
				break;
			}

		case TOY_VALUE_ARRAY: {
			if (right.type == TOY_VALUE_ARRAY) {
				Toy_Array* leftArray = left.as.array;
				Toy_Array* rightArray = right.as.array;

				//different lengths is an easy way to check
				if (leftArray->count != rightArray->count) {
					return false;
				}

				for (unsigned int i = 0; i < leftArray->count; i++) {
					//any mismatch is an easy difference
					if (Toy_checkValuesAreEqual(leftArray->data[i], rightArray->data[i]) != true) {
						return false;
					}
				}
			}
			else {
				break;
			}

			//finally
			return true;
		}

		case TOY_VALUE_TABLE:
		case TOY_VALUE_FUNCTION:
		case TOY_VALUE_OPAQUE:
		case TOY_VALUE_TYPE:
		case TOY_VALUE_ANY:
		case TOY_VALUE_REFERENCE:
		case TOY_VALUE_UNKNOWN:
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unknown types in value equality, exiting\n" TOY_CC_RESET);
			exit(-1);
	}

	return false;
}

bool Toy_checkValuesAreComparable(Toy_Value left, Toy_Value right) {
	left = Toy_unwrapValue(left);
	right = Toy_unwrapValue(right);

	//NOTE: "equal" and "comparable" are different - equal means they're identical, comparable is only possible for certain types
	switch(left.type) {
		case TOY_VALUE_NULL:
			return false;

		case TOY_VALUE_BOOLEAN:
			return right.type == TOY_VALUE_BOOLEAN;

		case TOY_VALUE_INTEGER:
		case TOY_VALUE_FLOAT:
			return right.type == TOY_VALUE_INTEGER || right.type == TOY_VALUE_FLOAT;

		case TOY_VALUE_STRING:
			return right.type == TOY_VALUE_STRING;

		case TOY_VALUE_ARRAY:
			//nothing is comparable with an array
			return false;

		case TOY_VALUE_TABLE:
		case TOY_VALUE_FUNCTION:
		case TOY_VALUE_OPAQUE:
		case TOY_VALUE_TYPE:
		case TOY_VALUE_ANY:
		case TOY_VALUE_REFERENCE:
		case TOY_VALUE_UNKNOWN:
			fprintf(stderr, TOY_CC_ERROR "Unknown types in value comparison check, exiting\n" TOY_CC_RESET);
			exit(-1);
	}

	return false;
}

int Toy_compareValues(Toy_Value left, Toy_Value right) {
	left = Toy_unwrapValue(left);
	right = Toy_unwrapValue(right);

	//comparison means there's a difference in value, with some kind of quantity - so null, bool, etc. aren't comparable
	switch(left.type) {
		case TOY_VALUE_NULL:
		case TOY_VALUE_BOOLEAN:
			break;

		case TOY_VALUE_INTEGER:
			if (right.type == TOY_VALUE_INTEGER) {
				return left.as.integer - right.as.integer;
			}
			else if (right.type == TOY_VALUE_FLOAT) {
				return left.as.integer - right.as.number;
			}
			else {
				break;
			}

		case TOY_VALUE_FLOAT:
			if (right.type == TOY_VALUE_INTEGER) {
				return left.as.number - right.as.integer;
			}
			else if (right.type == TOY_VALUE_FLOAT) {
				return left.as.number - right.as.number;
			}
			else {
				break;
			}

		case TOY_VALUE_STRING:
			if (right.type == TOY_VALUE_STRING) {
				return Toy_compareStrings(left.as.string, right.as.string);
			}

		case TOY_VALUE_ARRAY:
			break;

		case TOY_VALUE_TABLE:
		case TOY_VALUE_FUNCTION:
		case TOY_VALUE_OPAQUE:
		case TOY_VALUE_TYPE:
		case TOY_VALUE_ANY:
		case TOY_VALUE_REFERENCE:
		case TOY_VALUE_UNKNOWN:
			break;
	}

	fprintf(stderr, TOY_CC_ERROR "Unknown types in value comparison, exiting\n" TOY_CC_RESET);
	exit(-1);

	return ~0;
}

Toy_String* Toy_stringifyValue(Toy_Bucket** bucketHandle, Toy_Value value) {
	value = Toy_unwrapValue(value);

	//TODO: could have "constant" strings that can be referenced, instead of null, true, false, etc.

	switch(value.type) {
		case TOY_VALUE_NULL:
			return Toy_createString(bucketHandle, "null");

		case TOY_VALUE_BOOLEAN:
			return Toy_createString(bucketHandle, value.as.boolean ? "true" : "false");

		case TOY_VALUE_INTEGER: {
			char buffer[16];
			sprintf(buffer, "%d", value.as.integer);
			return Toy_createString(bucketHandle, buffer);
		}

		case TOY_VALUE_FLOAT: {
			//using printf
			char buffer[16];
			sprintf(buffer, "%f", value.as.number);

			//BUGFIX: printf format specificer '%f' will set the precision to 6 decimal places, which means there's trailing zeroes
			unsigned int length = strlen(buffer);

			//find the decimal, if it exists
			unsigned int decimal = 0;
			while (decimal != length && buffer[decimal] != '.' && buffer[decimal] != ',') decimal++; //'.' and ',' supports more locales

			//locales are hard, sorry!
			if (decimal != length && buffer[decimal] == ',') buffer[decimal] = '.';

			//wipe the trailing zeros
			while(decimal != length && buffer[length-1] == '0') buffer[--length] = '\0';

			return Toy_createStringLength(bucketHandle, buffer, length);
		}

		case TOY_VALUE_STRING:
			return Toy_copyString(value.as.string);

		case TOY_VALUE_ARRAY: {
			//TODO: concat + free is definitely a performance nightmare, could make an append function?
			Toy_Array* ptr = value.as.array;
			Toy_String* string = Toy_createStringLength(bucketHandle, "[", 1);
			Toy_String* comma = Toy_createStringLength(bucketHandle, ",", 1); //reusable

			for (unsigned int i = 0; i < ptr->count; i++) {
				//append each element
				Toy_String* tmp = Toy_concatStrings(bucketHandle, string, Toy_stringifyValue(bucketHandle, ptr->data[i])); //increment ref
				Toy_freeString(string); //decrement ref
				string = tmp;

				//if we need a comma
				if (i + 1 < ptr->count) {
					Toy_String* tmp = Toy_concatStrings(bucketHandle, string, comma); //increment ref
					Toy_freeString(string); //decrement ref
					string = tmp;
				}
			}

			//closing bracket
			Toy_String* tmp = Toy_concatStrings(bucketHandle, string, Toy_createStringLength(bucketHandle, "]", 1));
			Toy_freeString(string);
			string = tmp;

			//clean up
			Toy_freeString(comma); //TODO: reusable global, or string type "permanent"

			return string;
		}

		case TOY_VALUE_TABLE:
		case TOY_VALUE_FUNCTION:
		case TOY_VALUE_OPAQUE:
		case TOY_VALUE_TYPE:
		case TOY_VALUE_ANY:
		case TOY_VALUE_REFERENCE:
		case TOY_VALUE_UNKNOWN:
			fprintf(stderr, TOY_CC_ERROR "Unknown types in value stringify, exiting\n" TOY_CC_RESET);
			exit(-1);
	}

	return NULL;
}

const char* Toy_private_getValueTypeAsCString(Toy_ValueType type) {
	switch (type) {
		case TOY_VALUE_NULL: return "null";
		case TOY_VALUE_BOOLEAN: return "bool";
		case TOY_VALUE_INTEGER: return "int";
		case TOY_VALUE_FLOAT: return "float";
		case TOY_VALUE_STRING: return "string";
		case TOY_VALUE_ARRAY: return "array";
		case TOY_VALUE_TABLE: return "table";
		case TOY_VALUE_FUNCTION: return "function";
		case TOY_VALUE_OPAQUE: return "opaque";
		case TOY_VALUE_TYPE: return "type";
		case TOY_VALUE_ANY: return "any";
		case TOY_VALUE_REFERENCE: return "reference";
		case TOY_VALUE_UNKNOWN: return "unknown";
	}

	return NULL;
}