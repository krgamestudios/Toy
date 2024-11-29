#include "toy_value.h"
#include "toy_console_colors.h"

#include "toy_bucket.h"
#include "toy_string.h"
#include "toy_array.h"

#include <stdio.h>
#include <stdlib.h>

//utils
static unsigned int hashUInt(unsigned int x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

//exposed functions
unsigned int Toy_hashValue(Toy_Value value) {
	switch(value.type) {
		case TOY_VALUE_NULL:
			return 0;

		case TOY_VALUE_BOOLEAN:
			return TOY_VALUE_AS_BOOLEAN(value) ? 1 : 0;

		case TOY_VALUE_INTEGER:
			return hashUInt(TOY_VALUE_AS_INTEGER(value));

		case TOY_VALUE_FLOAT:
			return hashUInt( *((int*)(&TOY_VALUE_AS_FLOAT(value))) );

		case TOY_VALUE_STRING:
			return Toy_hashString(TOY_VALUE_AS_STRING(value));

		case TOY_VALUE_ARRAY: {
			//since array internals can change, recalc the hash each time it's needed
			Toy_Array* array = TOY_VALUE_AS_ARRAY(value);
			unsigned int hash = 0;

			for (unsigned int i = 0; i < array->count; i++) {
				hash ^= Toy_hashValue(array->data[i]);
			}

			return hash;
		}

		case TOY_VALUE_TABLE:
		case TOY_VALUE_FUNCTION:
		case TOY_VALUE_OPAQUE:
		case TOY_VALUE_TYPE:
		case TOY_VALUE_ANY:
		case TOY_VALUE_UNKNOWN:
			fprintf(stderr, TOY_CC_ERROR "ERROR: Can't hash an unknown value type, exiting\n" TOY_CC_RESET);
			exit(-1);
	}

	return 0;
}

Toy_Value Toy_copyValue(Toy_Value value) {
	switch(value.type) {
		case TOY_VALUE_NULL:
		case TOY_VALUE_BOOLEAN:
		case TOY_VALUE_INTEGER:
		case TOY_VALUE_FLOAT:
			return value;

		case TOY_VALUE_STRING: {
			Toy_String* string = TOY_VALUE_AS_STRING(value);
			return TOY_VALUE_FROM_STRING(Toy_copyString(string));
		}

		case TOY_VALUE_ARRAY: {
			Toy_Array* array = TOY_VALUE_AS_ARRAY(value);
			Toy_Array* result = Toy_resizeArray(NULL, array->capacity);

			for (unsigned int i = 0; i < array->count; i++) {
				result->data[i] = Toy_copyValue(array->data[i]);
			}

			return TOY_VALUE_FROM_ARRAY(result);
		}

		case TOY_VALUE_TABLE:
		case TOY_VALUE_FUNCTION:
		case TOY_VALUE_OPAQUE:
		case TOY_VALUE_TYPE:
		case TOY_VALUE_ANY:
		case TOY_VALUE_UNKNOWN:
			fprintf(stderr, TOY_CC_ERROR "ERROR: Can't copy an unknown value type, exiting\n" TOY_CC_RESET);
			exit(-1);
	}

	//dummy return
	return TOY_VALUE_FROM_NULL();
}

void Toy_freeValue(Toy_Value value) {
	switch(value.type) {
		case TOY_VALUE_NULL:
		case TOY_VALUE_BOOLEAN:
		case TOY_VALUE_INTEGER:
		case TOY_VALUE_FLOAT:
			break;

		case TOY_VALUE_STRING: {
			Toy_String* string = TOY_VALUE_AS_STRING(value);
			Toy_freeString(string);
			break;
		}

		case TOY_VALUE_ARRAY: {
			Toy_Array* array = TOY_VALUE_AS_ARRAY(value);

			for (unsigned int i = 0; i < array->count; i++) {
				Toy_freeValue(array->data[i]);
			}

			TOY_ARRAY_FREE(array);
		}

		case TOY_VALUE_TABLE:
		case TOY_VALUE_FUNCTION:
		case TOY_VALUE_OPAQUE:
		case TOY_VALUE_TYPE:
		case TOY_VALUE_ANY:
		case TOY_VALUE_UNKNOWN:
			fprintf(stderr, TOY_CC_ERROR "ERROR: Can't free an unknown value type, exiting\n" TOY_CC_RESET);
			exit(-1);
	}
}

bool Toy_checkValueIsTruthy(Toy_Value value) {
	//null is an error
	if (TOY_VALUE_IS_NULL(value)) {
		Toy_error("'null' is neither true nor false");
		return false;
	}

	//only 'false' is falsy
	if (TOY_VALUE_IS_BOOLEAN(value)) {
		return TOY_VALUE_AS_BOOLEAN(value);
	}

	//anything else is truthy
	return true;
}

bool Toy_checkValuesAreEqual(Toy_Value left, Toy_Value right) {
	switch(left.type) {
		case TOY_VALUE_NULL:
			return TOY_VALUE_IS_NULL(right);

		case TOY_VALUE_BOOLEAN:
			return TOY_VALUE_IS_BOOLEAN(right) && TOY_VALUE_AS_BOOLEAN(left) == TOY_VALUE_AS_BOOLEAN(right);

		case TOY_VALUE_INTEGER:
			if (TOY_VALUE_IS_INTEGER(right)) {
				return TOY_VALUE_AS_INTEGER(left) == TOY_VALUE_AS_INTEGER(right);
			}
			else if (TOY_VALUE_IS_FLOAT(right)) {
				return TOY_VALUE_AS_INTEGER(left) == TOY_VALUE_AS_FLOAT(right);
			}
			else {
				break;
			}

		case TOY_VALUE_FLOAT:
			if (TOY_VALUE_IS_INTEGER(right)) {
				return TOY_VALUE_AS_FLOAT(left) == TOY_VALUE_AS_INTEGER(right);
			}
			else if (TOY_VALUE_IS_FLOAT(right)) {
				return TOY_VALUE_AS_FLOAT(left) == TOY_VALUE_AS_FLOAT(right);
			}
			else {
				break;
			}

		case TOY_VALUE_STRING:
			if (TOY_VALUE_IS_STRING(right)) {
				return Toy_compareStrings(TOY_VALUE_AS_STRING(left), TOY_VALUE_AS_STRING(right)) == 0;
			}
			else {
				break;
			}

		case TOY_VALUE_ARRAY: {
			Toy_Array* leftArray = TOY_VALUE_AS_ARRAY(left);
			Toy_Array* rightArray = TOY_VALUE_AS_ARRAY(right);

			//different lengths is an easy way to check
			if (leftArray->count != rightArray->count) {
				return false;
			}

			for (unsigned int i = 0; i < leftArray->count; i++) {
				//any mismatch is an easy difference
				if (Toy_checkValuesAreEqual(leftArray->data[i], rightArray->data[i])) {
					return false;
				}
			}

			//finally
			return true;
		}

		case TOY_VALUE_TABLE:
		case TOY_VALUE_FUNCTION:
		case TOY_VALUE_OPAQUE:
		case TOY_VALUE_TYPE:
		case TOY_VALUE_ANY:
		case TOY_VALUE_UNKNOWN:
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unknown types in value equality, exiting\n" TOY_CC_RESET);
			exit(-1);
	}

	return false;
}

bool Toy_checkValuesAreComparable(Toy_Value left, Toy_Value right) {
	//NOTE: "equal" and "comparable" are different - equal means they're identical, comparable is only possible for certain types

	switch(left.type) {
		case TOY_VALUE_NULL:
			return false;

		case TOY_VALUE_BOOLEAN:
			return TOY_VALUE_IS_BOOLEAN(right);

		case TOY_VALUE_INTEGER:
		case TOY_VALUE_FLOAT:
			return TOY_VALUE_IS_INTEGER(right) || TOY_VALUE_IS_FLOAT(right);

		case TOY_VALUE_STRING:
			return TOY_VALUE_IS_STRING(right);

		case TOY_VALUE_ARRAY:
			//nothing is comparable with an array
			return false;

		case TOY_VALUE_TABLE:
		case TOY_VALUE_FUNCTION:
		case TOY_VALUE_OPAQUE:
		case TOY_VALUE_TYPE:
		case TOY_VALUE_ANY:
		case TOY_VALUE_UNKNOWN:
			fprintf(stderr, TOY_CC_ERROR "Unknown types in value comparison check, exiting\n" TOY_CC_RESET);
			exit(-1);
	}

	return false;
}

int Toy_compareValues(Toy_Value left, Toy_Value right) {
	//comparison means there's a difference in value, with some kind of quantity - so null, bool, etc. aren't comparable
	switch(left.type) {
		case TOY_VALUE_NULL:
		case TOY_VALUE_BOOLEAN:
			break;

		case TOY_VALUE_INTEGER:
			if (TOY_VALUE_IS_INTEGER(right)) {
				return TOY_VALUE_AS_INTEGER(left) - TOY_VALUE_AS_INTEGER(right);
			}
			else if (TOY_VALUE_IS_FLOAT(right)) {
				return TOY_VALUE_AS_INTEGER(left) - TOY_VALUE_AS_FLOAT(right);
			}
			else {
				break;
			}

		case TOY_VALUE_FLOAT:
			if (TOY_VALUE_IS_INTEGER(right)) {
				return TOY_VALUE_AS_FLOAT(left) - TOY_VALUE_AS_INTEGER(right);
			}
			else if (TOY_VALUE_IS_FLOAT(right)) {
				return TOY_VALUE_AS_FLOAT(left) - TOY_VALUE_AS_FLOAT(right);
			}
			else {
				break;
			}

		case TOY_VALUE_STRING:
			if (TOY_VALUE_IS_STRING(right)) {
				return Toy_compareStrings(TOY_VALUE_AS_STRING(left), TOY_VALUE_AS_STRING(right));
			}

		case TOY_VALUE_ARRAY:
			break;

		case TOY_VALUE_TABLE:
		case TOY_VALUE_FUNCTION:
		case TOY_VALUE_OPAQUE:
		case TOY_VALUE_TYPE:
		case TOY_VALUE_ANY:
		case TOY_VALUE_UNKNOWN:
			fprintf(stderr, TOY_CC_ERROR "Unknown types in value comparison, exiting\n" TOY_CC_RESET);
			exit(-1);
	}

	return -1;
}

Toy_String* Toy_stringifyValue(Toy_Bucket** bucketHandle, Toy_Value value) {
	//TODO: could have "constant" strings that can be referenced, instead of null, true, false, etc.

	switch(value.type) {
		case TOY_VALUE_NULL:
			return Toy_createString(bucketHandle, "null");

		case TOY_VALUE_BOOLEAN:
			return Toy_createString(bucketHandle, TOY_VALUE_AS_BOOLEAN(value) ? "true" : "false");

		case TOY_VALUE_INTEGER: {
			char buffer[16];
			sprintf(buffer, "%d", TOY_VALUE_AS_INTEGER(value));
			return Toy_createString(bucketHandle, buffer);
		}

		case TOY_VALUE_FLOAT: {
			char buffer[16];
			sprintf(buffer, "%f", TOY_VALUE_AS_FLOAT(value));
			return Toy_createString(bucketHandle, buffer);
		}

		case TOY_VALUE_STRING:
			return Toy_copyString(TOY_VALUE_AS_STRING(value));

		case TOY_VALUE_ARRAY: {
			//TODO: concat + free is definitely a performance nightmare
			Toy_Array* array = TOY_VALUE_AS_ARRAY(value);
			Toy_String* string = Toy_createStringLength(bucketHandle, "[", 1);
			Toy_String* comma = Toy_createStringLength(bucketHandle, ",", 1); //reusable

			for (unsigned int i = 0; i < array->count; i++) {
				//append each element
				Toy_String* tmp = Toy_concatStrings(bucketHandle, string, Toy_stringifyValue(bucketHandle, array->data[i])); //increment ref
				Toy_freeString(string); //decrement ref
				string = tmp;

				//if we need a comma
				if (i + 1 < array->count) {
					Toy_String* tmp = Toy_concatStrings(bucketHandle, string, comma); //increment ref
					Toy_freeString(string); //decrement ref
					string = tmp;
				}
			}

			//clean up
			Toy_freeString(comma); //TODO: reusable global, or string type "permanent"

			return string;
		}

		case TOY_VALUE_TABLE:
		case TOY_VALUE_FUNCTION:
		case TOY_VALUE_OPAQUE:
		case TOY_VALUE_TYPE:
		case TOY_VALUE_ANY:
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
		case TOY_VALUE_UNKNOWN: return "unknown";
	}

	return NULL;
}