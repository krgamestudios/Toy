#include "toy_attributes.h"
#include "toy_console_colors.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

Toy_Value handleStringAttributes(Toy_VM* vm, Toy_Value compound, Toy_Value attribute) {
	if (strncmp(TOY_VALUE_AS_STRING(attribute)->leaf.data, "length", 6)  == 0) {
		return TOY_VALUE_FROM_INTEGER(TOY_VALUE_AS_STRING(compound)->info.length);
	}
	else if (strncmp(TOY_VALUE_AS_STRING(attribute)->leaf.data, "asUpper", 7)  == 0) {
		char* buffer = Toy_getStringRaw(TOY_VALUE_AS_STRING(compound));
		for (int i = 0; buffer[i] != '\0'; i++) {
			buffer[i] = toupper(buffer[i]);
		}
		Toy_String* str = Toy_createStringLength(&vm->memoryBucket, buffer, strlen(buffer));
		free(buffer);
		return TOY_VALUE_FROM_STRING(str);
	}
	else if (strncmp(TOY_VALUE_AS_STRING(attribute)->leaf.data, "asLower", 7) == 0) {
		char* buffer = Toy_getStringRaw(TOY_VALUE_AS_STRING(compound));
		for (int i = 0; buffer[i] != '\0'; i++) {
			buffer[i] = tolower(buffer[i]);
		}
		Toy_String* str = Toy_createStringLength(&vm->memoryBucket, buffer, strlen(buffer));
		free(buffer);
		return TOY_VALUE_FROM_STRING(str);
	}
	else {
		char buffer[256];
		snprintf(buffer, 256, "Unknown attribute '%s' of type '%s'", TOY_VALUE_AS_STRING(attribute)->leaf.data, Toy_private_getValueTypeAsCString(compound.type));
		Toy_error(buffer);
		return TOY_VALUE_FROM_NULL();
	}
}

static void attr_arrayPushBack(Toy_VM* vm) {
	Toy_Value compound = Toy_popStack(&vm->stack);
	Toy_Value element = Toy_popStack(&vm->stack);

	Toy_Array* array = TOY_VALUE_AS_ARRAY(compound);

	//check the capacity limit
	if (array->count == array->capacity) {
		//correct the source value's pointer
		array = Toy_resizeArray(array, array->capacity * TOY_ARRAY_EXPANSION_RATE);
		if (TOY_VALUE_IS_REFERENCE(compound) && compound.as.reference->type == TOY_VALUE_ARRAY) {
			compound.as.reference->as.array = array;
		}
		else {
			char buffer[256];
			snprintf(buffer, 256, "Unknown error at %s %d", __FILE__, __LINE__);
			Toy_error(buffer);
		}
	}

	array->data[array->count] = element;
	array->count++;
}

static void attr_arrayPopBack(Toy_VM* vm) {
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

static void attr_arrayForEach(Toy_VM* vm) {
	(void)vm;
	//URGENT: attr_arrayForEach
}

static void attr_arraySort(Toy_VM* vm) {
	(void)vm;
	//URGENT: attr_arraySort
}

Toy_Value handleArrayAttributes(Toy_VM* vm, Toy_Value compound, Toy_Value attribute) {
	if (strncmp(TOY_VALUE_AS_STRING(attribute)->leaf.data, "length", 6)  == 0) {
		return TOY_VALUE_FROM_INTEGER(TOY_VALUE_AS_ARRAY(compound)->count);
	}
	else if (strncmp(TOY_VALUE_AS_STRING(attribute)->leaf.data, "pushBack", 8)  == 0) {
		Toy_Function* fn = Toy_createFunctionFromCallback(&vm->memoryBucket, attr_arrayPushBack);
		return TOY_VALUE_FROM_FUNCTION(fn);
	}
	else if (strncmp(TOY_VALUE_AS_STRING(attribute)->leaf.data, "popBack", 7)  == 0) {
		Toy_Function* fn = Toy_createFunctionFromCallback(&vm->memoryBucket, attr_arrayPopBack);
		return TOY_VALUE_FROM_FUNCTION(fn);
	}
	else if (strncmp(TOY_VALUE_AS_STRING(attribute)->leaf.data, "forEach", 7)  == 0) {
		Toy_Function* fn = Toy_createFunctionFromCallback(&vm->memoryBucket, attr_arrayForEach);
		return TOY_VALUE_FROM_FUNCTION(fn);
	}
	else if (strncmp(TOY_VALUE_AS_STRING(attribute)->leaf.data, "sort", 4)  == 0) {
		Toy_Function* fn = Toy_createFunctionFromCallback(&vm->memoryBucket, attr_arraySort);
		return TOY_VALUE_FROM_FUNCTION(fn);
	}
	else {
		char buffer[256];
		snprintf(buffer, 256, "Unknown attribute '%s' of type '%s'", TOY_VALUE_AS_STRING(attribute)->leaf.data, Toy_private_getValueTypeAsCString(compound.type));
		Toy_error(buffer);
		return TOY_VALUE_FROM_NULL();
	}

	return TOY_VALUE_FROM_NULL();
}

Toy_Value handleTableAttributes(Toy_VM* vm, Toy_Value compound, Toy_Value attribute) {
	//URGENT: handleTableAttributes
	(void)vm;
	(void)compound;
	(void)attribute;
	return TOY_VALUE_FROM_NULL();
}