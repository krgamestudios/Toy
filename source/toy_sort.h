#pragma once

#include "toy_value.h"

typedef int (*Toy_Sort_Comparator)(const Toy_Value*, const Toy_Value*);

void dual_pivot_quick_sort(Toy_Value* arr, int low, int high, Toy_Sort_Comparator comp_ptr);

void insertion_sort(Toy_Value *arr, int low, int high, Toy_Sort_Comparator comp_ptr);

int partition(Toy_Value* arr, int low, int high, int* lp, Toy_Sort_Comparator comp_ptr);

int toy_value_default_compare(const Toy_Value *a, const Toy_Value *b);

static void swap_ptr(Toy_Value* first, Toy_Value* second);
