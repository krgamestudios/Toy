#include "toy_scope.h"
#include "toy_console_colors.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "toy_print.h"

//utils
static void incrementRefCount(Toy_Scope* scope) {
	for (Toy_Scope* iter = scope; iter; iter = iter->next) {
		iter->refCount++;
	}
}

static void decrementRefCount(Toy_Scope* scope) {
	for (Toy_Scope* iter = scope; iter; iter = iter->next) {	
		iter->refCount--;
		if (iter->refCount == 0 && iter->table != NULL) {
			Toy_freeTable(iter->table);
			iter->table = NULL;
		}
	}
}

static Toy_TableEntry* lookupScope(Toy_Scope* scope, Toy_String* key, unsigned int hash, bool recursive) {
	//terminate
	if (scope == NULL) {
		return NULL;
	}

	//copy and modify the code from Toy_lookupTable, so it can behave slightly differently
	unsigned int probe = hash % scope->table->capacity;

	while (true) {
		//found the entry
		if (TOY_VALUE_IS_STRING(scope->table->data[probe].key) && Toy_compareStrings(TOY_VALUE_AS_STRING(scope->table->data[probe].key), key) == 0) {
			return &(scope->table->data[probe]);
		}

		//if its an empty slot (didn't find it here)
		if (TOY_VALUE_IS_NULL(scope->table->data[probe].key)) {
			return recursive ? lookupScope(scope->next, key, hash, recursive) : NULL;
		}

		//adjust and continue
		probe = (probe + 1) % scope->table->capacity;
	}
}

//exposed functions
Toy_Scope* Toy_pushScope(Toy_Bucket** bucketHandle, Toy_Scope* scope) {
	Toy_Scope* newScope = Toy_partitionBucket(bucketHandle, sizeof(Toy_Scope));

	newScope->next = scope;
	newScope->table = Toy_allocateTable();
	newScope->refCount = 0;

	incrementRefCount(newScope);

	return newScope;
}

Toy_Scope* Toy_popScope(Toy_Scope* scope) {
	if (scope == NULL) {
		return NULL;
	}

	decrementRefCount(scope);
	return scope->next;
}

Toy_Scope* Toy_deepCopyScope(Toy_Bucket** bucketHandle, Toy_Scope* scope) {
	//copy/pasted from pushScope, so I can allocate the table manually
	Toy_Scope* newScope = Toy_partitionBucket(bucketHandle, sizeof(Toy_Scope));

	newScope->next = scope->next;
	newScope->table = Toy_private_adjustTableCapacity(NULL, scope->table->capacity);
	newScope->refCount = 0;

	incrementRefCount(newScope);

	//forcibly copy the contents
	for (int i = 0; i < scope->table->capacity; i++) {
		if (!TOY_VALUE_IS_NULL(scope->table->data[i].key)) {
			Toy_insertTable(&newScope->table, Toy_copyValue(scope->table->data[i].key), Toy_copyValue(scope->table->data[i].value));
		}
	}

	return newScope;
}

void Toy_declareScope(Toy_Scope* scope, Toy_String* key, Toy_Value value) {
	if (key->type != TOY_STRING_NAME) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Toy_Scope only allows name strings as keys\n" TOY_CC_RESET);
		exit(-1);
	}

	Toy_TableEntry* entryPtr = lookupScope(scope, key, Toy_hashString(key), false);

	if (entryPtr != NULL) {
		char buffer[key->length + 256];
		sprintf(buffer, "Can't redefine a variable: %s", key->as.name.data);
		Toy_error(buffer);
		return;
	}

	//type check
	Toy_ValueType kt = Toy_getNameStringType(key);
	if (kt != TOY_VALUE_ANY && value.type != TOY_VALUE_NULL && kt != value.type) {
		char buffer[key->length + 256];
		sprintf(buffer, "Incorrect value type assigned to in variable declaration '%s' (expected %d, got %d)", key->as.name.data, (int)kt, (int)value.type);
		Toy_error(buffer);
		return;
	}

	//constness check
	if (Toy_getNameStringConstant(key) && value.type == TOY_VALUE_NULL) {
		char buffer[key->length + 256];
		sprintf(buffer, "Can't declare %s as const with value 'null'", key->as.name.data);
		Toy_error(buffer);
		return;
	}

	Toy_insertTable(&scope->table, TOY_VALUE_FROM_STRING(Toy_copyString(key)), value);
}

void Toy_assignScope(Toy_Scope* scope, Toy_String* key, Toy_Value value) {
	if (key->type != TOY_STRING_NAME) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Toy_Scope only allows name strings as keys\n" TOY_CC_RESET);
		exit(-1);
	}

	Toy_TableEntry* entryPtr = lookupScope(scope, key, Toy_hashString(key), true);

	if (entryPtr == NULL) {
		char buffer[key->length + 256];
		sprintf(buffer, "Undefined variable: %s", key->as.name.data);
		Toy_error(buffer);
		return;
	}

	//type check
	Toy_ValueType kt = Toy_getNameStringType( TOY_VALUE_AS_STRING(entryPtr->key) );
	if (kt != TOY_VALUE_ANY && value.type != TOY_VALUE_NULL && kt != value.type) {
		char buffer[key->length + 256];
		sprintf(buffer, "Incorrect value type assigned to in variable assignment '%s' (expected %d, got %d)", key->as.name.data, (int)kt, (int)value.type);
		Toy_error(buffer);
		return;
	}

	//constness check
	if (Toy_getNameStringConstant( TOY_VALUE_AS_STRING(entryPtr->key) )) {
		char buffer[key->length + 256];
		sprintf(buffer, "Can't assign to const %s", key->as.name.data);
		Toy_error(buffer);
		return;
	}

	entryPtr->value = value;
}

Toy_Value Toy_accessScope(Toy_Scope* scope, Toy_String* key) {
	if (key->type != TOY_STRING_NAME) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Toy_Scope only allows name strings as keys\n" TOY_CC_RESET);
		exit(-1);
	}

	Toy_TableEntry* entryPtr = lookupScope(scope, key, Toy_hashString(key), true);

	if (entryPtr == NULL) {
		char buffer[key->length + 256];
		sprintf(buffer, "Undefined variable: %s\n", key->as.name.data);
		Toy_error(buffer);
		return TOY_VALUE_FROM_NULL();
	}

	return entryPtr->value;
}

bool Toy_isDeclaredScope(Toy_Scope* scope, Toy_String* key) {
	if (key->type != TOY_STRING_NAME) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Toy_Scope only allows name strings as keys\n" TOY_CC_RESET);
		exit(-1);
	}

	Toy_TableEntry* entryPtr = lookupScope(scope, key, Toy_hashString(key), true);

	return entryPtr != NULL;
}
