#include "toy_value.h"

#include "toy_sort.h"

void insertion_sort(Toy_Value *arr, int low, int high, Toy_Sort_Comparator comp_ptr) {
    for (int i = low + 1; i <= high; i++) {
        Toy_Value key = arr[i];
        int j = i - 1;

        while (j >= low && comp_ptr(&arr[j], &key) == 1) {
            arr[j + 1] = arr[j];
            j--;
        }

        arr[j + 1] = key;
    }
}

void dual_pivot_quick_sort(Toy_Value* arr, int low, int high, Toy_Sort_Comparator comp_ptr) {
    if (high - low < 40) {
        insertion_sort(arr, low, high, comp_ptr);
        return;
    }

    if (low < high) {
        int lp, rp;
        rp = partition(arr, low, high, &lp, comp_ptr);
        dual_pivot_quick_sort(arr, low, lp - 1, comp_ptr);
        dual_pivot_quick_sort(arr, lp + 1, rp - 1, comp_ptr);
        dual_pivot_quick_sort(arr, rp + 1, high, comp_ptr);
    }
}

int partition(Toy_Value* arr, int low, int high, int* lp, Toy_Sort_Comparator comp_ptr) {
    int j = low + 1;
    int g = high - 1;
    int k = low + 1;
    Toy_Value *p = &arr[low];
    Toy_Value *q = &arr[high];

    while (k <= g) {
        if (comp_ptr(&arr[k], p) == -1) {
            swap_ptr(&arr[k], &arr[j]);
            j++;
            k++;
            continue;
        }

        int comp = comp_ptr(&arr[k], q);
        if (comp == 0 || comp == 1) {
            while (comp_ptr(&arr[g], q) == 1 && k < g)
                g--;
            swap_ptr(&arr[k], &arr[g]);
            g--;
            if (comp_ptr(&arr[k], p) == -1) {
                swap_ptr(&arr[k], &arr[j]);
                j++;
            }
        }
        k++;
    }

    j--;
    g++;

    swap_ptr(&arr[low], &arr[j]);
    swap_ptr(&arr[high], &arr[g]);

    *lp = j;
    return g;
}

int toy_value_default_compare(const Toy_Value *a, const Toy_Value *b) {
    float fa = a->type == TOY_VALUE_INTEGER ? (float)a->as.integer : a->as.number;
    float fb = b->type == TOY_VALUE_INTEGER ? (float)b->as.integer : b->as.number;
    return (fa > fb) - (fa < fb); // returns -1, 0, or 1
}

static void swap_ptr(Toy_Value* first, Toy_Value* second) {
    Toy_Value temp = *first;
    *first = *second;
    *second = temp;
}
