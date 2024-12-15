#pragma once

#include "toy_common.h"
#include "toy_value.h"

typedef struct Toy_Stack { //32 | 64 BITNESS
	unsigned int capacity; //4  | 4
	unsigned int count;    //4  | 4
	Toy_Value data[];      //-  | -
} Toy_Stack;               //8  | 8

TOY_API Toy_Stack* Toy_allocateStack(void);
TOY_API void Toy_freeStack(Toy_Stack* stack);

TOY_API void Toy_pushStack(Toy_Stack** stackHandle, Toy_Value value);
TOY_API Toy_Value Toy_peekStack(Toy_Stack** stackHandle);
TOY_API Toy_Value Toy_popStack(Toy_Stack** stackHandle);

//some useful sizes, could be swapped out as needed
#ifndef TOY_STACK_INITIAL_CAPACITY
#define TOY_STACK_INITIAL_CAPACITY 8
#endif

#ifndef TOY_STACK_EXPANSION_RATE
#define TOY_STACK_EXPANSION_RATE 2
#endif

#ifndef TOY_STACK_CONTRACTION_THRESHOLD
#define TOY_STACK_CONTRACTION_THRESHOLD (1 / 4)
#endif

//prevent an infinite expansion, limited to 1MB
#ifndef TOY_STACK_OVERFLOW
#define TOY_STACK_OVERFLOW (1024 * 1024 / sizeof(Toy_Value))
#endif
