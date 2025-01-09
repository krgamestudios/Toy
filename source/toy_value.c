#include "toy_value.h"
#include "toy_console_colors.h"

#include "toy_bucket.h"
#include "toy_string.h"
#include "toy_array.h"
#include "toy_table.h"

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

		case TOY_VALUE_TABLE: {
			//since table internals can change, recalc the hash each time it's needed
			Toy_Table* ptr = value.as.table;
			unsigned int hash = 0;

			for (unsigned int i = 0; i < ptr->capacity; i++) {
				if (TOY_VALUE_IS_NULL(ptr->data[i].key) != true) {
					hash ^= Toy_hashValue(ptr->data[i].key);
					hash ^= Toy_hashValue(ptr->data[i].value);
				}
			}

			return hash;
		}

		case TOY_VALUE_FUNCTION:
		case TOY_VALUE_OPAQUE:
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

		case TOY_VALUE_TABLE: {
			//tables probably won't get copied much
			Toy_Table* ptr = value.as.table;
			Toy_Table* result = Toy_private_adjustTableCapacity(NULL, ptr->capacity);

			for (unsigned int i = 0; i < ptr->capacity; i++) {
				if (TOY_VALUE_IS_NULL(ptr->data[i].key) != true) {
					result->data[i].key = Toy_copyValue(ptr->data[i].key);
					result->data[i].value = Toy_copyValue(ptr->data[i].value);
				}
			}

			result->capacity = ptr->capacity;
			result->count = ptr->count;

			return TOY_VALUE_FROM_TABLE(result);
		}
		case TOY_VALUE_FUNCTION:
		case TOY_VALUE_OPAQUE:
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

		case TOY_VALUE_ARRAY:
			Toy_resizeArray(value.as.array, 0);
			break;

		case TOY_VALUE_TABLE:
			Toy_freeTable(value.as.table);
			break;

		case TOY_VALUE_REFERENCE:
			//don't free references
			return;

		case TOY_VALUE_FUNCTION:
		case TOY_VALUE_OPAQUE:
		case TOY_VALUE_ANY:
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
			return right.type == TOY_VALUE_BOOLEAN && left.as.boolean == right.as.boolean;

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

		case TOY_VALUE_TABLE: {
			if (right.type == TOY_VALUE_TABLE) {
				Toy_Table* leftTable = left.as.table;
				Toy_Table* rightTable = right.as.table;

				//different counts
				if (leftTable->count != rightTable->count) {
					return false;
				}

				for (unsigned int i = 0; i < leftTable->capacity; i++) {
					Toy_TableEntry* entry = leftTable->data + i;

					if (TOY_VALUE_IS_NULL(entry->key) != true) {
						//any mismatch is an easy difference
						Toy_Value rightValue = Toy_lookupTable(&rightTable, entry->key);

						if (TOY_VALUE_IS_NULL(rightValue) || Toy_checkValuesAreEqual(entry->value, rightValue) != true) {
							return false;
						}
					}
				}
			}
			else {
				break;
			}

			//finally
			return true;
		}

		case TOY_VALUE_FUNCTION:
		case TOY_VALUE_OPAQUE:
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
			//nothing is comparable with a table
			return false;

		case TOY_VALUE_FUNCTION:
		case TOY_VALUE_OPAQUE:
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
			break;

		case TOY_VALUE_FUNCTION:
		case TOY_VALUE_OPAQUE:
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

	//TODO: could have "constant" strings that can be referenced, instead of null, true, false, etc. - new string type of 'permanent'

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

			//if array is empty, skip below
			if (ptr->count == 0) {
				Toy_String* empty = Toy_createString(bucketHandle, "[]");
				return empty;
			}

			Toy_String* open = Toy_createStringLength(bucketHandle, "[", 1);
			Toy_String* close = Toy_createStringLength(bucketHandle, "]", 1);
			Toy_String* comma = Toy_createStringLength(bucketHandle, ",", 1); //reusable
			Toy_String* quote = Toy_createStringLength(bucketHandle, "\"", 1); //reusable
			bool needsComma = false;

			Toy_String* string = open;

			for (unsigned int i = 0; i < ptr->count; i++) {
				if (needsComma) {
					Toy_String* tmp = Toy_concatStrings(bucketHandle, string, comma); //increment ref
					Toy_freeString(string); //decrement ref
					string = tmp;
				}

				//get the element
				Toy_String* element = Toy_stringifyValue(bucketHandle, ptr->data[i]);

				//put quotemarks around internal string elements
				if (TOY_VALUE_IS_STRING(ptr->data[i])) {
					Toy_String* tmpA = Toy_concatStrings(bucketHandle, quote, element);
					Toy_String* tmpB = Toy_concatStrings(bucketHandle, tmpA, quote);

					Toy_freeString(element);
					Toy_freeString(tmpA);
					element = tmpB;
				}

				//append each element
				Toy_String* final = Toy_concatStrings(bucketHandle, string, element);

				Toy_freeString(element);
				Toy_freeString(string);

				string = final;

				needsComma = true;
			}

			//closing bracket
			Toy_String* tmp = Toy_concatStrings(bucketHandle, string, close);
			Toy_freeString(string);
			string = tmp;

			//clean up
			Toy_freeString(open);
			Toy_freeString(close);
			Toy_freeString(comma); //TODO: reusable global, or string type "permanent"
			Toy_freeString(quote); //TODO: reusable global, or string type "permanent"

			return string;
		}

		case TOY_VALUE_TABLE: {
			//TODO: concat + free is definitely a performance nightmare, could make an append function?
			Toy_Table* ptr = value.as.table;

			//if table is empty, skip below
			if (ptr->count == 0) {
				Toy_String* empty = Toy_createString(bucketHandle, "[:]");
				return empty;
			}

			Toy_String* open = Toy_createStringLength(bucketHandle, "[", 1);
			Toy_String* close = Toy_createStringLength(bucketHandle, "]", 1);
			Toy_String* colon = Toy_createStringLength(bucketHandle, ":", 1); //reusable
			Toy_String* comma = Toy_createStringLength(bucketHandle, ",", 1); //reusable
			Toy_String* quote = Toy_createStringLength(bucketHandle, "\"", 1); //reusable
			bool needsComma = false;

			Toy_String* string = open;

			for (unsigned int i = 0; i < ptr->capacity; i++) {
				if (TOY_VALUE_IS_NULL(ptr->data[i].key)) {
					continue;
				}

				if (needsComma) {
					Toy_String* tmp = Toy_concatStrings(bucketHandle, string, comma); //increment ref
					Toy_freeString(string); //decrement ref
					string = tmp;
				}

				//make the element pair
				Toy_String* k = Toy_stringifyValue(bucketHandle, ptr->data[i].key);
				Toy_String* v = Toy_stringifyValue(bucketHandle, ptr->data[i].value);

				//put quotemarks around internal string elements (key)
				if (TOY_VALUE_IS_STRING(ptr->data[i].key)) {
					Toy_String* tmpA = Toy_concatStrings(bucketHandle, quote, k);
					Toy_String* tmpB = Toy_concatStrings(bucketHandle, tmpA, quote);

					Toy_freeString(k);
					Toy_freeString(tmpA);
					k = tmpB;
				}

				//put quotemarks around internal string elements (value)
				if (TOY_VALUE_IS_STRING(ptr->data[i].value)) {
					Toy_String* tmpA = Toy_concatStrings(bucketHandle, quote, v);
					Toy_String* tmpB = Toy_concatStrings(bucketHandle, tmpA, quote);

					Toy_freeString(v);
					Toy_freeString(tmpA);
					v = tmpB;
				}

				//stick the colon between, make the pair
				Toy_String* c = Toy_concatStrings(bucketHandle, k, colon);
				Toy_String* pair = Toy_concatStrings(bucketHandle, c, v);

				//append the element pair
				Toy_String* final = Toy_concatStrings(bucketHandle, string, pair);

				//do a bunch of freeing so the internal refCounts stay balanced
				Toy_freeString(k);
				Toy_freeString(v);
				Toy_freeString(c);
				Toy_freeString(pair);
				Toy_freeString(string);

				//finally
				string = final;

				//TODO: would a simple buffer be faster here?

				//if there's more elements
				needsComma = true;
			}

			//closing bracket
			Toy_String* tmp = Toy_concatStrings(bucketHandle, string, close);
			Toy_freeString(string);
			string = tmp;

			//clean up
			Toy_freeString(open);
			Toy_freeString(close);
			Toy_freeString(colon); //TODO: reusable global, or string type "permanent"
			Toy_freeString(comma); //TODO: reusable global, or string type "permanent"
			Toy_freeString(quote); //TODO: reusable global, or string type "permanent"

			return string;
		}

		case TOY_VALUE_FUNCTION:
		case TOY_VALUE_OPAQUE:
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
		case TOY_VALUE_ANY: return "any";
		case TOY_VALUE_REFERENCE: return "reference";
		case TOY_VALUE_UNKNOWN: return "unknown";
	}

	return NULL;
}

