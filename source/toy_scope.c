#include "toy_scope.h"
#include "toy_console_colors.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "toy_print.h"
#include "toy_function.h"
#include "toy_array.h"
#include "toy_table.h"

//utils
static Toy_ScopeEntry* lookupScopeEntryPtr(Toy_Scope* scope, Toy_String* key, unsigned int hash, bool recursive) {
	//terminate
	if (scope == NULL || scope->data == NULL) {
		return NULL;
	}

	//probe for the correct location
	unsigned int probe = hash % scope->capacity;

	while (true) {
		//found the entry
		if (scope->data[probe].key != NULL && Toy_compareStrings(scope->data[probe].key, key) == 0) {
			return &(scope->data[probe]);
		}

		//if its an empty slot (didn't find it here)
		if (scope->data[probe].key == NULL) {
			return recursive ? lookupScopeEntryPtr(scope->next, key, hash, recursive) : NULL;
		}

		//adjust and continue
		probe = (probe + 1) % scope->capacity;
	}
}

static void probeAndInsert(Toy_Scope* scope, Toy_String* key, Toy_Value value, Toy_ValueType type, bool constant) {
	//make the entry
	unsigned int probe = Toy_hashString(key) % scope->capacity;
	Toy_ScopeEntry entry = (Toy_ScopeEntry){ .key = key, .value = value, .type = type, .constant = constant, .psl = 1 };

	//probe
	while (true) {
		//if we're overriding an existing value
		if (scope->data[probe].key != NULL && Toy_compareStrings(scope->data[probe].key, entry.key) == 0) {
			scope->data[probe] = entry;
			scope->maxPsl = entry.psl > scope->maxPsl ? entry.psl : scope->maxPsl;
			return;
		}

		//if this spot is free, insert and return
		if (scope->data[probe].key == NULL) { //fuck my life
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

static Toy_ScopeEntry* adjustScopeEntries(Toy_Bucket** bucketHandle, Toy_Scope* scope, unsigned int newCapacity) {
	//WARN: doesn't delete existing scope entries
	//allocate and zero a new Toy_ScopeEntry array in the bucket
	Toy_ScopeEntry* newEntries = (Toy_ScopeEntry*)Toy_partitionBucket(bucketHandle, newCapacity * sizeof(Toy_ScopeEntry));

	if (newEntries == NULL) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to allocate space for 'Toy_Scope' entries\n" TOY_CC_RESET);
		exit(1);
	}

	//wipe the memory
	memset(newEntries, 0, newCapacity * sizeof(Toy_ScopeEntry));

	if (scope == NULL) { //for initial allocations
		return newEntries;
	}

	//move the old data into the new block of memory
	unsigned int oldCapacity = scope->capacity;
	Toy_ScopeEntry* oldEntries = scope->data;
	scope->capacity = newCapacity;
	scope->data = newEntries;
	scope->count = 0;

	//for each existing entry in the old array, copy it into the new array
	for (unsigned int i = 0; i < oldCapacity; i++) {
		if (oldEntries[i].key != NULL && oldEntries[i].key->info.length > 0) {
			probeAndInsert(scope, oldEntries[i].key, oldEntries[i].value, oldEntries[i].type, oldEntries[i].constant);
		}
	}

	//clean up and return
	Toy_releaseBucketPartition((unsigned char*)oldEntries);
	return newEntries;
}

Toy_Value coerceValueTypesIfAble(Toy_ValueType type, Toy_Value value) {
	//integer to float
	if (type == TOY_VALUE_FLOAT && value.type == TOY_VALUE_INTEGER) {
		value = TOY_VALUE_FROM_FLOAT( (float)TOY_VALUE_AS_INTEGER(value) );
	}
	return value;
}

//exposed functions
Toy_Scope* Toy_pushScope(Toy_Bucket** bucketHandle, Toy_Scope* scope) {
	//scopes are declared separately from their entries so the pointer to scopes don't become invalid
	Toy_Scope* newScope = (Toy_Scope*)Toy_partitionBucket(bucketHandle, sizeof(Toy_Scope));

	//WARN: You've tried lazily allocating scope memory in the past, it doesn't work and breaks a bunch of tests, so stop trying.
	newScope->next = scope;
	newScope->refCount = 0;
	newScope->scopes = NULL;
	newScope->owner = scope != NULL ? scope->owner : newScope;
	newScope->heapNext = newScope->owner->scopes;
	newScope->owner->scopes = newScope;
	newScope->externalRefs = 0;
	newScope->marked = false;
	newScope->data = adjustScopeEntries(bucketHandle, NULL, TOY_SCOPE_INITIAL_CAPACITY);
	newScope->capacity = TOY_SCOPE_INITIAL_CAPACITY;
	newScope->count = 0;
	newScope->maxPsl = 0;

	Toy_private_incrementScopeRefCount(newScope);

	return newScope;
}

static void freeScopeValues(Toy_Scope* scope) {
	if (scope->data == NULL) {
		return;
	}

	for (unsigned int i = 0; i < scope->capacity; i++) {
		if (scope->data[i].key != NULL) {
			Toy_freeString(scope->data[i].key);
			Toy_freeValue(scope->data[i].value);
		}
	}

	Toy_releaseBucketPartition((unsigned char*)scope->data);
	scope->data = NULL;
}

Toy_Scope* Toy_popScope(Toy_Scope* scope) {
	if (scope == NULL) {
		return NULL;
	}

	Toy_Scope* next = scope->next;
	Toy_private_decrementScopeRefCount(scope);
	if (scope->refCount == 0) {
		freeScopeValues(scope);
		//Keep the heap owner until the VM releases its arena.
		if (scope != scope->owner) {
			Toy_Scope** link = &scope->owner->scopes;
			while (*link != scope) {
				link = &(*link)->heapNext;
			}

			*link = scope->heapNext;
			Toy_releaseBucketPartition((void*)scope);
		}
	}

	return next;
}

static void visitValue(Toy_Value value, Toy_Scope* owner,
	void (*visit)(Toy_Scope*, Toy_Scope*)) {
	if (value.type == TOY_VALUE_FUNCTION &&
		value.as.function->type == TOY_FUNCTION_CUSTOM) {
		visit(value.as.function->bytecode.parentScope, owner);
	}
	else if (value.type == TOY_VALUE_OBJECT) {
		visitValue(value.as.object->methods, owner, visit);
		visitValue(value.as.object->getter, owner, visit);
		visitValue(value.as.object->setter, owner, visit);
	}
	else if (value.type == TOY_VALUE_ARRAY) {
		for (unsigned int i = 0; i < value.as.array->count; i++) {
			visitValue(value.as.array->data[i], owner, visit);
		}
	}
	else if (value.type == TOY_VALUE_TABLE) {
		for (unsigned int i = 0; i < value.as.table->capacity; i++) {
			if (value.as.table->data[i].key.type != TOY_VALUE_NULL) {
				visitValue(value.as.table->data[i].value, owner, visit);
			}

		}
	}
}

static void visitScope(Toy_Scope* scope, Toy_Scope* owner,
	void (*visit)(Toy_Scope*, Toy_Scope*)) {
	if (scope->data == NULL) {
		return;
	}

	for (unsigned int i = 0; i < scope->capacity; i++) {
		if (scope->data[i].key != NULL) {
			visitValue(scope->data[i].value, owner, visit);
		}
	}
}

static void subtractInternal(Toy_Scope* scope, Toy_Scope* owner) {
	for (; scope != NULL && scope->owner == owner; scope = scope->next) {
		if (scope->externalRefs > 0) {
			scope->externalRefs--;
		}
	}
}

static void markScope(Toy_Scope* scope, Toy_Scope* owner) {
	if (scope == NULL || scope->owner != owner || scope->marked) {
		return;
	}

	scope->marked = true;
	markScope(scope->next, owner);
	visitScope(scope, owner, markScope);
}

void Toy_collectScopes(Toy_Scope* owner) {
	if (owner == NULL) {
		return;
	}

	owner = owner->owner;
	for (Toy_Scope* scope = owner->scopes; scope; scope = scope->heapNext) {
		scope->externalRefs = scope->refCount;
		scope->marked = false;
	}

	//References held inside this heap are not roots. VM stacks and host
	//callbacks retain their counts, including the captured ancestor chain.
	for (Toy_Scope* scope = owner->scopes; scope; scope = scope->heapNext) {
		visitScope(scope, owner, subtractInternal);
	}

	for (Toy_Scope* scope = owner->scopes; scope; scope = scope->heapNext) {
		if (scope->externalRefs > 0) {
			markScope(scope, owner);
		}
	}

	//Destroy values first: their functions can still refer to other scopes.
	for (Toy_Scope* scope = owner->scopes; scope; scope = scope->heapNext) {
		if (!scope->marked) {
			freeScopeValues(scope);
		}
	}

	Toy_Scope** link = &owner->scopes;
	while (*link != NULL) {
		Toy_Scope* scope = *link;
		if (!scope->marked && scope != owner) {
			*link = scope->heapNext;
			Toy_releaseBucketPartition((void*)scope);
		}

		else {
			link = &scope->heapNext;
		}
	}
}

void Toy_freeScopeHeap(Toy_Scope* owner) {
	if (owner == NULL) {
		return;
	}

	owner = owner->owner;
	for (Toy_Scope* scope = owner->scopes; scope; scope = scope->heapNext) {
		freeScopeValues(scope);
	}

	Toy_Scope* scope = owner->scopes;
	while (scope != NULL) {
		Toy_Scope* next = scope->heapNext;
		Toy_releaseBucketPartition((void*)scope);
		scope = next;
	}
}

void Toy_declareScope(Toy_Bucket** bucketHandle, Toy_Scope* scope, Toy_String* key, Toy_ValueType type, Toy_Value value, bool constant) {
	Toy_ScopeEntry* entryPtr = lookupScopeEntryPtr(scope, key, Toy_hashString(key), false);

	if (entryPtr != NULL) {
		char buffer[key->info.length + 256];
		sprintf(buffer, "Can't redefine a variable: %s", key->leaf.data);
		Toy_error(buffer);
		return;
	}

	//expand the table capacity if needed
	if (scope->count >= scope->capacity * TOY_SCOPE_EXPANSION_THRESHOLD) {
		scope->data = adjustScopeEntries(bucketHandle, scope, scope->capacity * TOY_SCOPE_EXPANSION_RATE);
	}

	value = coerceValueTypesIfAble(type, value);

	//type check
	if (type != TOY_VALUE_ANY && value.type != TOY_VALUE_NULL && type != value.type && value.type != TOY_VALUE_REFERENCE) {
		char buffer[key->info.length + 256];
		sprintf(buffer, "Incorrect value type in declaration of '%.*s' (expected %s, got %s)", key->info.length, key->leaf.data, Toy_getValueTypeAsCString(type), Toy_getValueTypeAsCString(value.type));
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

	value = coerceValueTypesIfAble(entryPtr->type, value);

	//type check
	if (entryPtr->type != TOY_VALUE_ANY && value.type != TOY_VALUE_NULL && entryPtr->type != value.type && value.type != TOY_VALUE_REFERENCE) {
		char buffer[key->info.length + 256];
		sprintf(buffer, "Incorrect value type in assignment of '%.*s' (expected %s, got %s)", key->info.length, key->leaf.data, Toy_getValueTypeAsCString(entryPtr->type), Toy_getValueTypeAsCString(value.type));
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

	Toy_freeValue(entryPtr->value);

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

void Toy_private_incrementScopeRefCount(Toy_Scope* scope) {
	for (Toy_Scope* iter = scope; iter; iter = iter->next) {
		//check for issues
		if (iter->next != NULL && iter->next->refCount <= 0) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: Toy_Scope's ancestor has a refcount of 0 or below\n" TOY_CC_RESET);
			exit(-1);
		}

		iter->refCount++;
	}
}

void Toy_private_decrementScopeRefCount(Toy_Scope* scope) {
	for (Toy_Scope* iter = scope; iter != NULL; iter = iter->next) {
		iter->refCount--;

		//check for issues
		if (iter->next != NULL && iter->next->refCount <= 0) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: Toy_Scope's ancestor has a refcount of 0 or below\n" TOY_CC_RESET);
			exit(-1);
		}
	}
}