#include "toy_attributes.h"
#include "toy_console_colors.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//if set, used for delegating to user-defined code
static Toy_OpaqueAttributeHandler opaqueAttributeCallback = NULL;

//utils
#define MATCH_VALUE_AND_CSTRING(value, cstring) \
	((TOY_VALUE_AS_STRING(value)->info.length == strlen(cstring)) && \
	(strncmp(cstring, TOY_VALUE_AS_STRING(value)->leaf.data, TOY_VALUE_AS_STRING(value)->info.length) == 0))

#define MAYBE_UNWRAP(value) if (TOY_VALUE_IS_REFERENCE(value)) { value = Toy_unwrapValue(value); }

//NOTE: there is no need to call 'Toy_freeValue' on the arguments, as the VM assumes you don't
Toy_Value Toy_private_handleStringAttributes(Toy_VM* vm, Toy_Value compound, Toy_Value attribute) {
	if (MATCH_VALUE_AND_CSTRING(attribute, "length")) {
		return TOY_VALUE_FROM_INTEGER(TOY_VALUE_AS_STRING(compound)->info.length);
	}
	else if (MATCH_VALUE_AND_CSTRING(attribute, "asUpper")) {
		char* buffer = Toy_getStringRaw(TOY_VALUE_AS_STRING(compound));
		for (int i = 0; buffer[i] != '\0'; i++) {
			buffer[i] = toupper(buffer[i]);
		}
		Toy_String* str = Toy_createStringLength(&vm->memoryBucket, buffer, strlen(buffer));
		free(buffer);
		return TOY_VALUE_FROM_STRING(str);
	}
	else if (MATCH_VALUE_AND_CSTRING(attribute, "asLower")) {
		char* buffer = Toy_getStringRaw(TOY_VALUE_AS_STRING(compound));
		for (int i = 0; buffer[i] != '\0'; i++) {
			buffer[i] = tolower(buffer[i]);
		}
		Toy_String* str = Toy_createStringLength(&vm->memoryBucket, buffer, strlen(buffer));
		free(buffer);
		return TOY_VALUE_FROM_STRING(str);
	}
	else if (MATCH_VALUE_AND_CSTRING(attribute, "asCapitalized")) {
		char* buffer = Toy_getStringRaw(TOY_VALUE_AS_STRING(compound));
		buffer[0] = toupper(buffer[0]); //only applies toteh first character
		Toy_String* str = Toy_createStringLength(&vm->memoryBucket, buffer, strlen(buffer));
		free(buffer);
		return TOY_VALUE_FROM_STRING(str);
	}
	else {
		char buffer[256];
		snprintf(buffer, 256, "Unknown attribute '%s' of type '%s'", TOY_VALUE_AS_STRING(attribute)->leaf.data, Toy_getValueTypeAsCString(compound.type));
		Toy_error(buffer);
		return TOY_VALUE_FROM_NULL();
	}
}

static void attr_arrayPushBack(Toy_VM* vm, Toy_FunctionNative* self) {
	(void)self;

	Toy_Value compound = Toy_popStack(&vm->stack);
	Toy_Value element = Toy_popStack(&vm->stack);

	Toy_Array* array = TOY_VALUE_AS_ARRAY(compound);

	//BUGFIX: check the capacity limit
	if (array->count == array->capacity) {
		//correct the source value's pointer
		array = Toy_resizeArray(array, array->capacity * TOY_ARRAY_EXPANSION_RATE);
		if (TOY_VALUE_IS_REFERENCE(compound) && compound.as.reference->type == TOY_VALUE_ARRAY) {
			compound.as.reference->as.array = array;
		}
		else {
			char buffer[256];
			snprintf(buffer, 256, "Unknown error after expanding array size at %s %d", __FILE__, __LINE__);
			Toy_error(buffer);
		}
	}

	array->data[array->count] = element;
	array->count++;
}

static void attr_arrayPopBack(Toy_VM* vm, Toy_FunctionNative* self) {
	(void)self;

	Toy_Value compound = Toy_popStack(&vm->stack);

	Toy_Array* array = TOY_VALUE_AS_ARRAY(compound);

	//empty returns nothing
	if (array->count == 0) {
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
		return;
	}

	Toy_Value element = array->data[array->count-1];
	array->count--;

	Toy_pushStack(&vm->stack, element);
}

static void attr_arrayFillWith(Toy_VM* vm, Toy_FunctionNative* self) {
	(void)self;

	//check parameter count
	if (vm->stack->count < 3) {
		char buffer[256];
		snprintf(buffer, 256, "Not enough parameters found in 'Array.fillWith(amount, value)'");
		Toy_error(buffer);
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
		return;
	}

	Toy_Value compound = Toy_popStack(&vm->stack);
	Toy_Value value = Toy_popStack(&vm->stack); //NOTE: the args are still backwards, except compound
	Toy_Value length = Toy_popStack(&vm->stack);

	//check types
	if (!TOY_VALUE_IS_INTEGER(length) || TOY_VALUE_AS_INTEGER(length) <= 0) {
		char buffer[256];
		snprintf(buffer, 256, "Bad parameter found in 'Array.fillWith(amount, value)'");
		Toy_error(buffer);
		Toy_freeValue(value);
		Toy_freeValue(length);
		Toy_freeValue(compound);
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
		return;
	}

	Toy_Array* array = TOY_VALUE_AS_ARRAY(compound);

	if (array->count != 0) {
		char buffer[256];
		snprintf(buffer, 256, "Can't fill a non-empty array, as it would override %u values with %d copies of the given default", array->count, TOY_VALUE_AS_INTEGER(length));
		Toy_error(buffer);
		Toy_freeValue(value);
		Toy_freeValue(length);
		Toy_freeValue(compound);
		Toy_pushStack(&vm->stack, TOY_VALUE_FROM_NULL());
		return;
	}

	//The DOOM hack, neat trick to find the next power of two, inclusive (restriction of the array system)
	unsigned int capacity = TOY_VALUE_AS_INTEGER(length);
	capacity--;
	capacity |= capacity >> 1;
	capacity |= capacity >> 2;
	capacity |= capacity >> 4;
	capacity |= capacity >> 8;
	capacity |= capacity >> 16;
	capacity++;

	//correct the source value's pointer
	array = Toy_resizeArray(array, capacity);
	if (TOY_VALUE_IS_REFERENCE(compound) && compound.as.reference->type == TOY_VALUE_ARRAY) {
		compound.as.reference->as.array = array;
	}

	//finally, actually fill the array with the desired values
	for (int i = 0; i < TOY_VALUE_AS_INTEGER(length); i++) {
		array->data[array->count++] = Toy_copyValue(&vm->memoryBucket, value);
	}

	Toy_pushStack(&vm->stack, TOY_VALUE_FROM_ARRAY(array));
}

static int quicksortRunCompare(Toy_VM* vm, int* out) {
	unsigned int resultCount = Toy_runVM(vm);

	if (resultCount != 1) {
		return -1;
	}

	Toy_Value result = Toy_popStack(&vm->stack);

	if (!TOY_VALUE_IS_INTEGER(result)) {
		Toy_freeValue(result);
		return -2;
	}

	*out = TOY_VALUE_AS_INTEGER(result);
	return 0;
}

static int quicksortRawData(Toy_Value* data, unsigned int count, unsigned int pivot, Toy_VM* vm, Toy_Value* lhs, Toy_Value* rhs) {
	// //DEBUG
	// printf("(%u, %u, at %d):\t", count, pivot, TOY_VALUE_AS_INTEGER(data[pivot]));
	// for (unsigned int i = 0; i < count; i++) {
	// 	if (TOY_VALUE_AS_INTEGER(data[i]) == 96) {
	// 		printf(TOY_CC_WARN "\t%d" TOY_CC_RESET, TOY_VALUE_AS_INTEGER(data[i]));
	// 	}
	// 	else {
	// 		printf("\t%d", TOY_VALUE_AS_INTEGER(data[i]));
	// 	}
	// }
	// printf("\n");

	//base case
	if (count <= 1) return 0;
	if (count == 2) {
		//a one-line check is all thats needed
		int out = 0;

		*lhs = TOY_REFERENCE_FROM_POINTER(&data[0]);
		*rhs = TOY_REFERENCE_FROM_POINTER(&data[1]);

		if (quicksortRunCompare(vm, &out) != 0) {
			char buffer[256];
			snprintf(buffer, 256, "Bad results given by 'compare' in 'Array.sort(compare)', unknown error");
			Toy_error(buffer);
			return -1; //failed, exit
		}

		if (out > 1) {
			Toy_Value tmp = data[0];
			data[0] = data[1];
			data[1] = tmp;
		}

		return 0; //all good, terminate here
	}

	//iterators, following Tony Hoare's partition scheme
	unsigned int left = 0;
	unsigned int right = count - 1;

	for (;;) {
		int out = -1; //out param

		//find the leftmost element not less than the pivot
		while (out <= 0) {
			*lhs = TOY_REFERENCE_FROM_POINTER(&data[left]);
			*rhs = TOY_REFERENCE_FROM_POINTER(&data[pivot]);

			if (quicksortRunCompare(vm, &out) != 0) {
				char buffer[256];
				snprintf(buffer, 256, "Bad results given by 'compare' in 'Array.sort(compare)', unknown error");
				Toy_error(buffer);
				return -1; //failed, exit
			}

			if (out <= 0) {
				left++;
			}

			//BUGFIX
			if (left >= pivot) break;
		}

		out = -1; //reset, since the checks are now pivot < right

		//find the rightmost element not greater than the pivot
		while (out <= 0) {
			*lhs = TOY_REFERENCE_FROM_POINTER(&data[pivot]);
			*rhs = TOY_REFERENCE_FROM_POINTER(&data[right]);

			if (quicksortRunCompare(vm, &out) != 0) {
				char buffer[256];
				snprintf(buffer, 256, "Bad results given by 'compare' in 'Array.sort(compare)', unknown error");
				Toy_error(buffer);
				return -1; //failed, exit
			}

			if (out <= 0) {
				right--;
			}

			//BUGFIX
			if (right <= pivot) break;
		}

		//check for iterator crossing
		if (left >= right) break;

		//swap the left & right element positions (and the pivot, if needed)
		Toy_Value tmp = data[left];
		data[left] = data[right];
		data[right] = tmp;

		//NOTE: the pivot's value has moved, so move the index too
		if (left == pivot) {
			pivot = right;
		}
		else if (right == pivot) {
			pivot = left;
		}

		//BUGFIX: the pivot has exhausted all values on one side, reset it
		if (pivot == left || pivot == right) {
			left = 0;
			right = count - 1;
			pivot = count / 2;
		}
	}

	//recursion
	return quicksortRawData(data, pivot+1, (pivot+1)/2, vm, lhs, rhs) || quicksortRawData(data + pivot, count - pivot - 1, (count - pivot - 1)/2, vm, lhs, rhs);
}

static void attr_arraySort(Toy_VM* vm, Toy_FunctionNative* self) {
	(void)self;

	//check parameter count
	if (vm->stack->count < 2) {
		char buffer[256];
		snprintf(buffer, 256, "Not enough parameters found in 'Array.sort(compare)'");
		Toy_error(buffer);
		return;
	}

	Toy_Value compound = Toy_popStack(&vm->stack);
	Toy_Value compare = Toy_popStack(&vm->stack);

	//check types
	if (!TOY_VALUE_IS_FUNCTION(compare) || TOY_VALUE_AS_FUNCTION(compare)->type != TOY_FUNCTION_CUSTOM) {
		char buffer[256];
		snprintf(buffer, 256, "Bad parameter found in 'Array.sort(compare)'");
		Toy_error(buffer);
		Toy_freeValue(compare);
		Toy_freeValue(compound);
		return;
	}

	//the standard types
	Toy_Array* array = TOY_VALUE_AS_ARRAY(compound);
	Toy_Function* fn = TOY_VALUE_AS_FUNCTION(compare);

	//TODO: edge-case and degenerate checks (2 or less elements)

	//build the VM that will run the comparisons
	Toy_VM subVM;
	Toy_inheritVM(vm, &subVM);
	Toy_bindVM(&subVM, fn->bytecode.code, fn->bytecode.parentScope);

	//check the functor's parameter count is 2
	if (2 * 8 != subVM.paramCount) {
		char buffer[256];
		snprintf(buffer, 256, "Incorrect number of parameters specified for 'compare' in 'Array.sort(compare)' (expected 2, found %d)", subVM.paramCount / 8);
		Toy_error(buffer);
		Toy_freeVM(&subVM);
		Toy_freeValue(compare);
		Toy_freeValue(compound);
		return;
	}

	//get the two parameters and declare them as references to values (which we can mess with for speed & efficiency)
	Toy_Value lhs = TOY_VALUE_FROM_NULL(); //NOTE: yes, this means this quicksort can't be parallelized, but idc
	{
		const char* cstr = ((char*)(subVM.code + subVM.dataAddr)) + ((unsigned int*)(subVM.code + subVM.paramAddr))[0];
		Toy_ValueType paramType = (Toy_ValueType)(((unsigned int*)(subVM.code + subVM.paramAddr))[1]);
		Toy_String* name = Toy_toStringLength(&subVM.memoryBucket, cstr, strlen(cstr));
		Toy_declareScope(&vm->memoryBucket, subVM.scope, name, paramType, TOY_REFERENCE_FROM_POINTER(&lhs), true);
		Toy_freeString(name);
	}
	Toy_Value rhs = TOY_VALUE_FROM_NULL();
	{
		const char* cstr = ((char*)(subVM.code + subVM.dataAddr)) + ((unsigned int*)(subVM.code + subVM.paramAddr))[2];
		Toy_ValueType paramType = (Toy_ValueType)(((unsigned int*)(subVM.code + subVM.paramAddr))[3]);
		Toy_String* name = Toy_toStringLength(&subVM.memoryBucket, cstr, strlen(cstr));
		Toy_declareScope(&vm->memoryBucket, subVM.scope, name, paramType, TOY_REFERENCE_FROM_POINTER(&rhs), true);
		Toy_freeString(name);
	}

	//run the sorting algorithm
	quicksortRawData(array->data, array->count, array->count / 2, &subVM, &lhs, &rhs);

	//clean up
	Toy_freeVM(&subVM);
	Toy_freeValue(compare);
	Toy_freeValue(compound);
}

static unsigned int arrayFlattenCounter(Toy_Array* array) {
	unsigned int total = 0;
	for (unsigned int i = 0; i < array->count; i++) {
		if (TOY_VALUE_IS_ARRAY(array->data[i])) {
			total += arrayFlattenCounter(TOY_VALUE_AS_ARRAY(array->data[i]));
		}
		else {
			total++; //not an array to be removed
		}
	}
	return total;
}

static void arrayFlattenMover(Toy_Array* oldArray, Toy_Array* newArray) {
	for (unsigned int i = 0; i < oldArray->count; i++) {
		if (TOY_VALUE_IS_ARRAY(oldArray->data[i])) {
			arrayFlattenMover(TOY_VALUE_AS_ARRAY(oldArray->data[i]), newArray);

			//also free the now-empty element
			Toy_resizeArray(TOY_VALUE_AS_ARRAY(oldArray->data[i]), 0);
			oldArray->data[i] = TOY_VALUE_FROM_NULL();
		}
		else {
			//move from one array to another, and remove it from the former
			newArray->data[newArray->count++] = oldArray->data[i];
			oldArray->data[i] = TOY_VALUE_FROM_NULL();
		}
	}
}

static void attr_arrayFlatten(Toy_VM* vm, Toy_FunctionNative* self) {
	(void)self;

	Toy_Value compound = Toy_popStack(&vm->stack);

	if (!TOY_VALUE_IS_REFERENCE(compound) || compound.as.reference->type != TOY_VALUE_ARRAY) {
		char buffer[256];
		snprintf(buffer, 256, "'Array.flatten()' does not work on non-variable values");
		Toy_error(buffer);
		return;
	}

	Toy_Array* array = TOY_VALUE_AS_ARRAY(compound);

	//count how many top-level elements will end up in the flattened array
	unsigned int capacity = arrayFlattenCounter(array);

	//"WHAT DO YOU MEAN DOOM??" -Takanashi Kiara, final words
	capacity--;
	capacity |= capacity >> 1;
	capacity |= capacity >> 2;
	capacity |= capacity >> 4;
	capacity |= capacity >> 8;
	capacity |= capacity >> 16;
	capacity++;

	Toy_Array* newArray = Toy_resizeArray(NULL, capacity);

	arrayFlattenMover(array, newArray);

	Toy_resizeArray(array, 0);

	compound.as.reference->as.array = newArray;
}

Toy_Value Toy_private_handleArrayAttributes(Toy_VM* vm, Toy_Value compound, Toy_Value attribute) {
	if (MATCH_VALUE_AND_CSTRING(attribute, "length")) {
		return TOY_VALUE_FROM_INTEGER(TOY_VALUE_AS_ARRAY(compound)->count);
	}
	else if (MATCH_VALUE_AND_CSTRING(attribute, "pushBack")) {
		Toy_Function* fn = Toy_createFunctionFromCallback(&vm->memoryBucket, attr_arrayPushBack);
		return TOY_VALUE_FROM_FUNCTION(fn);
	}
	else if (MATCH_VALUE_AND_CSTRING(attribute, "popBack")) {
		Toy_Function* fn = Toy_createFunctionFromCallback(&vm->memoryBucket, attr_arrayPopBack);
		return TOY_VALUE_FROM_FUNCTION(fn);
	}
	else if (MATCH_VALUE_AND_CSTRING(attribute, "fillWith")) {
		Toy_Function* fn = Toy_createFunctionFromCallback(&vm->memoryBucket, attr_arrayFillWith);
		return TOY_VALUE_FROM_FUNCTION(fn);
	}
	else if (MATCH_VALUE_AND_CSTRING(attribute, "sort")) {
		Toy_Function* fn = Toy_createFunctionFromCallback(&vm->memoryBucket, attr_arraySort);
		return TOY_VALUE_FROM_FUNCTION(fn);
	}
	else if (MATCH_VALUE_AND_CSTRING(attribute, "flatten")) {
		Toy_Function* fn = Toy_createFunctionFromCallback(&vm->memoryBucket, attr_arrayFlatten);
		return TOY_VALUE_FROM_FUNCTION(fn);
	}
	else {
		char buffer[256];
		snprintf(buffer, 256, "Unknown attribute '%s' of type '%s'", TOY_VALUE_AS_STRING(attribute)->leaf.data, Toy_getValueTypeAsCString(compound.type));
		Toy_error(buffer);
		return TOY_VALUE_FROM_NULL();
	}
}

static void attr_tableInsert(Toy_VM* vm, Toy_FunctionNative* self) {
	(void)self;

	Toy_Value compound = Toy_popStack(&vm->stack);
	Toy_Value value = Toy_popStack(&vm->stack); //NOTE: the args are still backwards, except compound
	Toy_Value key = Toy_popStack(&vm->stack);

	Toy_Table* table = TOY_VALUE_AS_TABLE(compound);
	Toy_insertTable(&table, key, value);

	//BUGFIX: check the capacity limit (Toy_insertTable automatically alters the pointer value)
	if (TOY_VALUE_AS_TABLE(compound) != table) {
		//correct the source value's pointer
		if (TOY_VALUE_IS_REFERENCE(compound) && compound.as.reference->type == TOY_VALUE_TABLE) {
			compound.as.reference->as.table = table;
		}
		else {
			char buffer[256];
			snprintf(buffer, 256, "Unknown error after expanding table size at %s %d", __FILE__, __LINE__);
			Toy_error(buffer);
		}
	}
}

static void attr_tableHasKey(Toy_VM* vm, Toy_FunctionNative* self) {
	(void)self;

	Toy_Value compound = Toy_popStack(&vm->stack);
	Toy_Value key = Toy_popStack(&vm->stack);

	Toy_Table* table = TOY_VALUE_AS_TABLE(compound);

	Toy_TableEntry* entry = Toy_private_lookupTableEntryPtr(&table, key);
	Toy_Value result = TOY_VALUE_FROM_BOOLEAN(entry != NULL);

	Toy_pushStack(&vm->stack, result);
}

static void attr_tableRemove(Toy_VM* vm, Toy_FunctionNative* self) {
	(void)self;

	Toy_Value compound = Toy_popStack(&vm->stack);
	Toy_Value key = Toy_popStack(&vm->stack);

	Toy_Table* table = TOY_VALUE_AS_TABLE(compound);

	Toy_removeTable(&table, key);
}

Toy_Value Toy_private_handleTableAttributes(Toy_VM* vm, Toy_Value compound, Toy_Value attribute) {
	if (MATCH_VALUE_AND_CSTRING(attribute, "length")) {
		return TOY_VALUE_FROM_INTEGER(TOY_VALUE_AS_TABLE(compound)->count);
	}
	else if (MATCH_VALUE_AND_CSTRING(attribute, "insert")) {
		Toy_Function* fn = Toy_createFunctionFromCallback(&vm->memoryBucket, attr_tableInsert);
		return TOY_VALUE_FROM_FUNCTION(fn);
	}
	else if (MATCH_VALUE_AND_CSTRING(attribute, "hasKey")) {
		Toy_Function* fn = Toy_createFunctionFromCallback(&vm->memoryBucket, attr_tableHasKey);
		return TOY_VALUE_FROM_FUNCTION(fn);
	}
	else if (MATCH_VALUE_AND_CSTRING(attribute, "remove")) {
		Toy_Function* fn = Toy_createFunctionFromCallback(&vm->memoryBucket, attr_tableRemove);
		return TOY_VALUE_FROM_FUNCTION(fn);
	}
	else {
		char buffer[256];
		snprintf(buffer, 256, "Unknown attribute '%s' of type '%s'", TOY_VALUE_AS_STRING(attribute)->leaf.data, Toy_getValueTypeAsCString(compound.type));
		Toy_error(buffer);
		return TOY_VALUE_FROM_NULL();
	}
}

Toy_Value Toy_private_handleOpaqueAttributes(Toy_VM* vm, Toy_Value compound, Toy_Value attribute) {
	if (opaqueAttributeCallback == NULL) {
		char buffer[256];
		snprintf(buffer, 256, "Unknown attribute '%s' of type '%s' (did you set the opaque callbacks?)", TOY_VALUE_AS_STRING(attribute)->leaf.data, Toy_getValueTypeAsCString(compound.type));
		Toy_error(buffer);
		return TOY_VALUE_FROM_NULL();
	}

	return opaqueAttributeCallback(vm, compound, attribute);
}

void Toy_setOpaqueAttributeHandler(Toy_OpaqueAttributeHandler cb) {
	opaqueAttributeCallback = cb;
}

int Toy_private_handleGlobalAttributes(Toy_VM* vm, Toy_Value compound, Toy_Value attribute, Toy_Value* result) {
	MAYBE_UNWRAP(compound);

	if (MATCH_VALUE_AND_CSTRING(attribute, "asString")) {
		Toy_String* str = Toy_stringifyValue(&vm->memoryBucket, compound);
		*result = TOY_VALUE_FROM_STRING(str);
		return 1;
	}

	//no matches
	return 0;
}