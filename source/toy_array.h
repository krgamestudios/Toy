#pragma once

#include "toy_common.h"
#include "toy_value.h"

//standard generic array
typedef struct Toy_Array { //32 | 64 BITNESS
	unsigned int capacity; //4  | 4
	unsigned int count;    //4  | 4
	Toy_Value data[];      //-  | -
} Toy_Array;               //8  | 8

TOY_API Toy_Array* Toy_resizeArray(Toy_Array* array, unsigned int capacity);

TOY_API Toy_Array* Toy_createArray(unsigned int capacity);

//some useful sizes, could be swapped out as needed
#ifndef TOY_ARRAY_INITIAL_CAPACITY
#define TOY_ARRAY_INITIAL_CAPACITY 8
#endif

#ifndef TOY_ARRAY_EXPANSION_RATE
#define TOY_ARRAY_EXPANSION_RATE 2
#endif
