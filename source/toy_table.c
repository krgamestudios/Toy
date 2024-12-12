#include "toy_table.h"
#include "toy_console_colors.h"
#include "toy_print.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//utils
static void probeAndInsert(Toy_Table** tableHandle, Toy_Value key, Toy_Value value) {
	//make the entry
	unsigned int probe = Toy_hashValue(key) % (*tableHandle)->capacity;
	Toy_TableEntry entry = (Toy_TableEntry){ .key = key, .value = value, .psl = 0 };

	//probe
	while (true) {
		//if we're overriding an existing value
		if (Toy_checkValuesAreEqual((*tableHandle)->data[probe].key, key)) {
			(*tableHandle)->data[probe] = entry;

			//TODO: benchmark the psl optimisation
			(*tableHandle)->minPsl = entry.psl < (*tableHandle)->minPsl ? entry.psl : (*tableHandle)->minPsl;
			(*tableHandle)->maxPsl = entry.psl > (*tableHandle)->maxPsl ? entry.psl : (*tableHandle)->maxPsl;

			return;
		}

		//if this spot is free, insert and return
		if (TOY_VALUE_IS_NULL((*tableHandle)->data[probe].key)) {
			(*tableHandle)->data[probe] = entry;

			(*tableHandle)->count++;

			//TODO: benchmark the psl optimisation
			(*tableHandle)->minPsl = entry.psl < (*tableHandle)->minPsl ? entry.psl : (*tableHandle)->minPsl;
			(*tableHandle)->maxPsl = entry.psl > (*tableHandle)->maxPsl ? entry.psl : (*tableHandle)->maxPsl;

			return;
		}

		//if the new entry is "poorer", insert it and shift the old one
		if ((*tableHandle)->data[probe].psl < entry.psl) {
			Toy_TableEntry tmp = (*tableHandle)->data[probe];
			(*tableHandle)->data[probe] = entry;
			entry = tmp;
		}

		//adjust and continue
		probe++;
		probe &= (*tableHandle)->capacity - 1; //DOOM hack
		entry.psl++;
	}
}

//exposed functions
Toy_Table* Toy_private_adjustTableCapacity(Toy_Table* oldTable, unsigned int newCapacity) {
	//allocate and zero a new table in memory
	Toy_Table* newTable = malloc(newCapacity * sizeof(Toy_TableEntry) + sizeof(Toy_Table));

	if (newTable == NULL) {
		Toy_error(TOY_CC_ERROR "ERROR: Failed to allocate a 'Toy_Table'\n" TOY_CC_RESET);
	}

	newTable->capacity = newCapacity;
	newTable->count = 0;
	newTable->minPsl = 0;
	newTable->maxPsl = 0;

	//unlike other structures, the empty space in a table needs to be null
	memset(newTable + 1, 0, newTable->capacity * sizeof(Toy_TableEntry));

	if (oldTable == NULL) { //for initial allocations
		return newTable;
	}

	//for each entry in the old table, copy it into the new table
	for (unsigned int i = 0; i < oldTable->capacity; i++) {
		if (!TOY_VALUE_IS_NULL(oldTable->data[i].key)) {
			probeAndInsert(&newTable, oldTable->data[i].key, oldTable->data[i].value);
		}
	}

	//clean up and return
	free(oldTable);
	return newTable;
}

Toy_Table* Toy_allocateTable() {
	return Toy_private_adjustTableCapacity(NULL, TOY_TABLE_INITIAL_CAPACITY);
}

void Toy_freeTable(Toy_Table* table) {
	if (table != NULL) {
		//if some values will be removed, free them first
		for (unsigned int i = 0; i < table->capacity; i++) {
			Toy_freeValue(table->data[i].key);
			Toy_freeValue(table->data[i].value);
		}

		free(table);
	}
}

void Toy_insertTable(Toy_Table** tableHandle, Toy_Value key, Toy_Value value) {
	if (TOY_VALUE_IS_NULL(key) || TOY_VALUE_IS_BOOLEAN(key)) { //TODO: disallow functions and opaques
		Toy_error(TOY_CC_ERROR "ERROR: Bad table key\n" TOY_CC_RESET);
	}

	//expand the capacity
	if ((*tableHandle)->count > (*tableHandle)->capacity * TOY_TABLE_EXPANSION_THRESHOLD) {
		(*tableHandle) = Toy_private_adjustTableCapacity((*tableHandle), (*tableHandle)->capacity * TOY_TABLE_EXPANSION_RATE);
	}

	probeAndInsert(tableHandle, key, value);
}

Toy_Value Toy_lookupTable(Toy_Table** tableHandle, Toy_Value key) {
	if (TOY_VALUE_IS_NULL(key) || TOY_VALUE_IS_BOOLEAN(key)) { //TODO: disallow functions and opaques
		Toy_error(TOY_CC_ERROR "ERROR: Bad table key\n" TOY_CC_RESET);
	}

	//lookup
	unsigned int probe = Toy_hashValue(key) % (*tableHandle)->capacity;

	while (true) {
		//found the entry
		if (Toy_checkValuesAreEqual((*tableHandle)->data[probe].key, key)) {
			return (*tableHandle)->data[probe].value;
		}

		//if its an empty slot
		if (TOY_VALUE_IS_NULL((*tableHandle)->data[probe].key)) {
			return TOY_VALUE_FROM_NULL();
		}

		//adjust and continue
		probe++;
		probe &= (*tableHandle)->capacity - 1; //DOOM hack
	}
}

void Toy_removeTable(Toy_Table** tableHandle, Toy_Value key) {
	if (TOY_VALUE_IS_NULL(key) || TOY_VALUE_IS_BOOLEAN(key)) { //TODO: disallow functions and opaques
		Toy_error(TOY_CC_ERROR "ERROR: Bad table key\n" TOY_CC_RESET);
	}

	//lookup
	unsigned int probe = Toy_hashValue(key) % (*tableHandle)->capacity;
	unsigned int wipe = probe; //wiped at the end

	while (true) {
		//found the entry
		if (Toy_checkValuesAreEqual((*tableHandle)->data[probe].key, key)) {
			break;
		}

		//if its an empty slot
		if (TOY_VALUE_IS_NULL((*tableHandle)->data[probe].key)) {
			return;
		}

		//adjust and continue
		probe++;
		probe &= (*tableHandle)->capacity - 1; //DOOM hack
	}

	//shift along the later entries
	for (unsigned int i = (*tableHandle)->minPsl; i < (*tableHandle)->maxPsl; i++) {
		//DOOM hack used twice
		unsigned int p = (probe + i + 0) & ((*tableHandle)->capacity-1); //prev
		unsigned int u = (probe + i + 1) & ((*tableHandle)->capacity-1); //current

		(*tableHandle)->data[p] = (*tableHandle)->data[u];
		(*tableHandle)->data[p].psl--;

		//if you hit something where it should be, or nothing at all, stop
		if (TOY_VALUE_IS_NULL((*tableHandle)->data[u].key) || (*tableHandle)->data[p].psl == 0) {
			wipe = u;
			break;
		}
	}

	//finally, wipe the removed entry
	(*tableHandle)->data[wipe] = (Toy_TableEntry){ .key = TOY_VALUE_FROM_NULL(), .value = TOY_VALUE_FROM_NULL(), .psl = 0 };
	(*tableHandle)->count--;
}
