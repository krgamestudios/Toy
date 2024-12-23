
These tests compared two memory solutions for `Toy_Array`, under different conditions. 'Pushes' is the number of iterations used in the following stress function:

```c
//defined in toy_value.h
#define TOY_VALUE_FROM_INTEGER(value) ((Toy_Value){{ .integer = value }, TOY_VALUE_INTEGER})

//util macros
#define TOY_ARRAY_EXPAND(array) ((array) = ((array) != NULL && (array)->count + 1 > (array)->capacity ? Toy_resizeArray((array), (array)->capacity * TOY_ARRAY_EXPANSION_RATE) : (array)))
#define TOY_ARRAY_PUSHBACK(array, value) (TOY_ARRAY_EXPAND(array), (array)->data[(array)->count++] = (value))

void stress_fillArray(Toy_Array** array) {
	//Toy_Value is either 8 or 16 bytes
	for (int i = 0; i < 10 * 1000 * 1000; i++) {
		TOY_ARRAY_PUSHBACK(*array, TOY_VALUE_FROM_INTEGER(i));
	}
}
```

'Memory' is the capacity of the `Toy_Bucket` when used. In the first set of results, the stress function was called once, while the second set called it 100 times, clearing the memory entirely each time. 'malloc' and 'bucket' shows the measured time taken for each situation.

```
1x run

pushes: 1000 * 1000
memory: 1024 * 1024 * 20
malloc: 0.01 0:00.01
bucket: 0.00 0:00.02

pushes: 10 * 1000 * 1000
memory: 1024 * 1024 * 200
malloc: 0.08 0:00.14
bucket: 0.13 0:00.29
```

```
100x looped runs

pushes: 1000 * 1000
memory: 1024 * 1024 * 20
malloc: 0.94 0:01.47
bucket: 1.02 0:02.60

pushes: 10 * 1000 * 1000
memory: 1024 * 1024 * 200
malloc: 8.28 0:15.77
bucket: 11.81 0:30.06
```