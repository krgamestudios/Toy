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

//some useful sizes, could be swapped out as needed
#ifndef TOY_ARRAY_INITIAL_CAPACITY
#define TOY_ARRAY_INITIAL_CAPACITY 8
#endif

#ifndef TOY_ARRAY_EXPANSION_RATE
#define TOY_ARRAY_EXPANSION_RATE 2
#endif

//quick allocate
#ifndef TOY_ARRAY_ALLOCATE
#define TOY_ARRAY_ALLOCATE() Toy_resizeArray(NULL, TOY_ARRAY_INITIAL_CAPACITY)
#endif

//quick free
#ifndef TOY_ARRAY_FREE
#define TOY_ARRAY_FREE(array) Toy_resizeArray(array, 0)
#endif

//one line to expand the array
#ifndef TOY_ARRAY_EXPAND
#define TOY_ARRAY_EXPAND(array) (array = (array != NULL && (array)->count + 1 > (array)->capacity ? Toy_resizeArray(array, (array)->capacity * TOY_ARRAY_EXPANSION_RATE) : array))
#endif

//quick push back
#ifndef TOY_ARRAY_PUSHBACK
#define TOY_ARRAY_PUSHBACK(array, value) (TOY_ARRAY_EXPAND(array),(array)->data[(array)->count++] = (value))
#endif

//URGENT: get array length in scripts (dot operator?)
//URGENT: array as a type