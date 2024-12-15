#pragma once

#include "toy_common.h"
#include "toy_value.h"

//key-value entry, and probe sequence length - https://programming.guide/robin-hood-hashing.html
typedef struct Toy_TableEntry { //32 | 64 BITNESS
	Toy_Value key;              //8  | 8
	Toy_Value value;            //8  | 8
	unsigned int psl;			//4  | 4
} Toy_TableEntry;               //20 | 20

//key-value table (contains = count + tombstones)
typedef struct Toy_Table { //32 | 64 BITNESS
	unsigned int capacity; //4  | 4
	unsigned int count;    //4  | 4
	unsigned int minPsl;   //4  | 4
	unsigned int maxPsl;   //4  | 4
	Toy_TableEntry data[]; //-  | -
} Toy_Table;               //16 | 16

TOY_API Toy_Table* Toy_allocateTable(void);
TOY_API void Toy_freeTable(Toy_Table* table);
TOY_API void Toy_insertTable(Toy_Table** tableHandle, Toy_Value key, Toy_Value value);
TOY_API Toy_Value Toy_lookupTable(Toy_Table** tableHandle, Toy_Value key);
TOY_API void Toy_removeTable(Toy_Table** tableHandle, Toy_Value key);

//NOTE: exposed to skip unnecessary allocations within Toy_Scope
TOY_API Toy_Table* Toy_private_adjustTableCapacity(Toy_Table* oldTable, unsigned int newCapacity);

//some useful sizes, could be swapped out as needed
#ifndef TOY_TABLE_INITIAL_CAPACITY
#define TOY_TABLE_INITIAL_CAPACITY 8
#endif

//NOTE: The DOOM hack needs a power of 2
#ifndef TOY_TABLE_EXPANSION_RATE
#define TOY_TABLE_EXPANSION_RATE 2
#endif

//expand when the contents passes a certain percentage of the capacity
#ifndef TOY_TABLE_EXPANSION_THRESHOLD
#define TOY_TABLE_EXPANSION_THRESHOLD 0.8
#endif
