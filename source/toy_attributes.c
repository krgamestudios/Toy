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

static void attr_arraySort(Toy_VM* vm, Toy_FunctionNative* self) {
	(void)vm;
	(void)self;

	//URGENT: attr_arraySort
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
	else if (MATCH_VALUE_AND_CSTRING(attribute, "sort")) {
		Toy_Function* fn = Toy_createFunctionFromCallback(&vm->memoryBucket, attr_arraySort);
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