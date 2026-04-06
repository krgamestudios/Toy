#include "toy_scope.h"
#include "toy_console_colors.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "toy_print.h"

//utils
static void incrementRefCount(Toy_Scope* scope) {
	for (Toy_Scope* iter = scope; iter; iter = iter->next) {
		//check for issues
		if (iter->next != NULL && iter->next->refCount == 0) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: Toy_Scope's ancestor has a refcount of 0'\n" TOY_CC_RESET);
			exit(-1);
		}

		iter->refCount++;
	}
}

static void decrementRefCount(Toy_Scope* scope) {
	for (Toy_Scope* iter = scope; iter; iter = iter->next) {	
		iter->refCount--;
		if (iter->refCount == 0 && iter->data != NULL) {
			//free the scope entries when this scope is no longer needed
			for (unsigned int i = 0; i < iter->capacity; i++) {
				if (iter->data[i].psl > 0) {
					Toy_freeString(&(iter->data[i].key));
					Toy_freeValue(iter->data[i].value);
				}
			}
			free(iter->data);
		}
	}
}

static Toy_ScopeEntry* lookupScopeEntryPtr(Toy_Scope* scope, Toy_String* key, unsigned int hash, bool recursive) {
	//terminate
	if (scope == NULL || scope->data == NULL) {
		return NULL;
	}

	//probe for the correct location
	unsigned int probe = hash % scope->capacity;

	while (true) {
		//found the entry
		if (Toy_compareStrings(&(scope->data[probe].key), key) == 0) {
			return &(scope->data[probe]);
		}

		//if its an empty slot (didn't find it here)
		if (scope->data[probe].key.info.length == 0) {
			return recursive ? lookupScopeEntryPtr(scope->next, key, hash, recursive) : NULL;
		}

		//adjust and continue
		probe = (probe + 1) % scope->capacity;
	}
}

void probeAndInsert(Toy_Scope* scope, Toy_String* key, Toy_Value value, Toy_ValueType type, bool constant) {
	//make the entry
	unsigned int probe = Toy_hashString(key) % scope->capacity;
	Toy_ScopeEntry entry = (Toy_ScopeEntry){ .key = *key, .value = value, .type = type, .constant = constant, .psl = 1 };

	//probe
	while (true) {
		//if we're overriding an existing value
		if (Toy_compareStrings(&(scope->data[probe].key), &(entry.key)) == 0) {
			scope->data[probe] = entry;
			scope->maxPsl = entry.psl > scope->maxPsl ? entry.psl : scope->maxPsl;
			return;
		}

		//if this spot is free, insert and return
		if (TOY_VALUE_IS_NULL(scope->data[probe].value)) {
			scope->data[probe] = entry;
			scope->count++;
			scope->maxPsl = entry.psl > scope->maxPsl ? entry.psl : scope->maxPsl;
			return;
		}

		//if the new entry is "poorer", insert it and shift the old one
		if (scope->data[probe].psl < entry.psl) {
			Toy_ScopeEntry tmp = scope->data[probe];
			scope->data[probe] = entry;
			entry = tmp;
		}

		//adjust and continue
		probe++;
		probe &= scope->capacity - 1; //DOOM hack
		entry.psl++;
	}
}

Toy_ScopeEntry* adjustScopeEntries(Toy_Scope* scope, unsigned int newCapacity) {
	//allocate and zero a new Toy_ScopeEntry array in memory
	Toy_ScopeEntry* newEntries = malloc(newCapacity * sizeof(Toy_ScopeEntry));

	if (newEntries == NULL) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to allocate space for 'Toy_Scope' entries\n" TOY_CC_RESET);
		exit(1);
	}

	//wipe the memory
	memset(newEntries, 0, newCapacity * sizeof(Toy_ScopeEntry));

	if (scope == NULL) { //for initial allocations
		return newEntries;
	}

	//movethe old data into the new block of memory
	unsigned int oldCapacity = scope->capacity;
	Toy_ScopeEntry* oldEntries = scope->data;
	scope->capacity = newCapacity;
	scope->data = newEntries;

	//for each existing entry in the old array, copy it into the new array
	for (unsigned int i = 0; i < oldCapacity; i++) {
		if (oldEntries[i].key.info.length > 0) {
			probeAndInsert(scope, &(oldEntries[i].key), oldEntries[i].value, oldEntries[i].type, oldEntries[i].constant);
		}
	}

	//clean up and return
	free(oldEntries);
	return newEntries;
}

//exposed functions
Toy_Scope* Toy_pushScope(Toy_Bucket** bucketHandle, Toy_Scope* scope) {
	Toy_Scope* newScope = (Toy_Scope*)Toy_partitionBucket(bucketHandle, sizeof(Toy_Scope));

	newScope->next = scope;
	newScope->refCount = 0;
	newScope->data = adjustScopeEntries(NULL, TOY_SCOPE_INITIAL_CAPACITY);
	newScope->capacity = TOY_SCOPE_INITIAL_CAPACITY;
	newScope->count = 0;
	newScope->maxPsl = 0;

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

void Toy_declareScope(Toy_Scope* scope, Toy_String* key, Toy_ValueType type, Toy_Value value, bool constant) {
	Toy_ScopeEntry* entryPtr = lookupScopeEntryPtr(scope, key, Toy_hashString(key), false);

	if (entryPtr != NULL) {
		char buffer[key->info.length + 256];
		sprintf(buffer, "Can't redefine a variable: %s", key->leaf.data);
		Toy_error(buffer);
		return;
	}

	//type check
	if (type != TOY_VALUE_ANY && value.type != TOY_VALUE_NULL && type != value.type && value.type != TOY_VALUE_REFERENCE) {
		char buffer[key->info.length + 256];
		sprintf(buffer, "Incorrect value type in declaration of '%s' (expected %s, got %s)", key->leaf.data, Toy_private_getValueTypeAsCString(type), Toy_private_getValueTypeAsCString(value.type));
		Toy_error(buffer);
		return;
	}

	probeAndInsert(scope, Toy_copyString(key), value, type, constant);
}

void Toy_assignScope(Toy_Scope* scope, Toy_String* key, Toy_Value value) {
	Toy_ScopeEntry* entryPtr = lookupScopeEntryPtr(scope, key, Toy_hashString(key), true);

	if (entryPtr == NULL) {
		char buffer[key->info.length + 256];
		sprintf(buffer, "Undefined variable: %s\n", key->leaf.data);
		Toy_error(buffer);
		return;
	}

	//type check
	if (entryPtr->type != TOY_VALUE_ANY && value.type != TOY_VALUE_NULL && entryPtr->type != value.type && value.type != TOY_VALUE_REFERENCE) {
		char buffer[key->info.length + 256];
		sprintf(buffer, "Incorrect value type in assignment of '%s' (expected %s, got %s)", key->leaf.data, Toy_private_getValueTypeAsCString(entryPtr->type), Toy_private_getValueTypeAsCString(value.type));
		Toy_error(buffer);
		return;
	}

	//constness check
	if (entryPtr->constant) {
		char buffer[key->info.length + 256];
		sprintf(buffer, "Can't assign to a constant variable %s", key->leaf.data);
		Toy_error(buffer);
		return;
	}

	entryPtr->value = value;
}

Toy_Value* Toy_accessScopeAsPointer(Toy_Scope* scope, Toy_String* key) {
	Toy_ScopeEntry* entryPtr = lookupScopeEntryPtr(scope, key, Toy_hashString(key), true);

	if (entryPtr == NULL) {
		char buffer[key->info.length + 256];
		sprintf(buffer, "Undefined variable: %s\n", key->leaf.data);
		Toy_error(buffer);
		return NULL;
	}

	return &(entryPtr->value);
}

bool Toy_isDeclaredScope(Toy_Scope* scope, Toy_String* key) {
	Toy_ScopeEntry* entryPtr = lookupScopeEntryPtr(scope, key, Toy_hashString(key), true);
	return entryPtr != NULL;
}
