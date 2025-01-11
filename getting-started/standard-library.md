
# Standard Library

The standard library offers a number of miscellaneous utility functions, which can be used for various purposes. These are the most commonly used functions, so the standard library is almost certain to be included in the host program.

The standard library can usually be accessed with the `import` keyword:

```
import standard;
```

## Defined Misc. Functions

### clock(): string

This function returns a string representation of the current timestamp.

### hash(self: any): int

This function returns a hashed value of `self`.

This function uses the internal literal hashing algorithms. As such, the following can't be hashed:

* functions
* types
* opaques
* `null`

Any attempt to hash these will return -1, except `null` which returns 0.

## Defined Maths Functions

### abs(self): any

This function returns the absolute value of any integer or float passed in.

### ceil(self): int

This function returns the value of any integer or float passed in, rounded up.

### floor(self): int

This function returns the value of any integer or float passed in, rounded down.

### max(...): any

This function returns the value of the highest integer or float passed in. It can take any number of arguments.

### min(...): any

This function returns the value of the lowest integer or float passed in. It can take any number of arguments.

### round(self): int

This function returns the value of any integer or float passed in, rounded to the nearest whole number.

### sign(self): int

This function expects an integer or float as the value for `self`.

If `self` is below 0, this function returns -1. Otherwise, it returns 1.

### normalize(self): int

This function expects an integer or float as the value for `self`.

If `self` is below 0, this function returns -1. Otherwise, if `self` is above 0, this function returns 1. Otherwise it returns 0.

### clamp(value, min, max): any

This function expects integers or floats as the values for `value`, `min`, and `max`.

If `value` is smaller than `min`, this function will return `min`. Otherwise, if `value` larger than `max`, it will return `max`. Otherwise, it will return `value`.

### lerp(start, end, amount): any

This function expects integers or floats as the values for `value`, `min`, and `max`.

This function will return the value of `start` adjusted towards the value of `end` by `amount`, by interpreting `amount` as a fraction.

## Defined Compound Functions

### concat(self, other): any

This function only works when self and others are matching compounds (both arrays, dictionaries or strings). It returns a new compound of that kind, with the content of `other` appended to the content of `self`.

### containsKey(self: dictionary, key): bool

This function returns `true` if `self` contains the given `key`; otherwise, it returns false.

### containsValue(self, value): bool

This function returns `true` if `self` contains the given `value`; otherwise, it returns false.

### every(self, func: fn): bool

This function takes either an array or a dictionary as the `self` argument and a function as `func`. The argument `func` must take two arguments - the first is the index/key of the array/dictionary, and the second is the value. The contents of `self` are passed into `func`, one element at a time, until `func` returns `false`, at which point this function returns `false`. Otherwise, this function returns `true`.

### forEach(self, func: fn)

This function takes either an array or a dictionary as the `self` argument and a function as `func`. The argument `func` must take two arguments - the first is the index/key of the array/dictionary, and the second is the value. The contents of `self` are passed into `func` one element at a time.

```
import standard;

fn p(i, x) {
    print x;
}

var a = [1, 3, 5];

a.forEach(p); //prints 1, 3, and 5 to stdout
```

### filter(self, func: fn): any

This function takes either an array or a dictionary as the `self` argument and a function as `func`. The argument `func` must take two arguments - the first is the index/key of the array/dictionary, and the second is the value. The contents of `self` are passed into `func`, one element at a time, and the function returns a new compound for every element that `func` returned a truthy value for.

### getKeys(self: dictionary): [any]

This function returns an array of all non-null keys stored within the dictionary. The order is undefined.

### getValues(self: dictionary): [any]

This function returns an array of all values with non-null keys stored within the dictionary. The order is undefined.

### indexOf(self: array, value): int

This function returns the first index within `self` that is equal to `value`, or `null` if none are found.

### map(self, func: fn): any

This function takes either an array or a dictionary as the `self` argument, and a function as `func`. The argument `func` must take two arguments - the first is the index/key of the array/dictionary, and the second is the value. It returns an array with the results of each call - the order of the results when called on a dictionary is undefined.

```
import standard;

fn increment(k, v) {
    return v + 1;
}

var a = [1, 2, 3];

print a.map(increment); //prints [2,3,4];
```

### reduce(self, default: any, func: fn): any

This function takes either an array or a dictionary as the `self` argument, a default value, and a function as `func`. The argument `func` takes three arguments - the first is the accumulator, the second is the index/key, and the third is the value. It applies the given function to every element of the array/dictionary, passing the result of each call as the accumulator to the next (the default value is used for the first call). Finally, the final value of the accumulator is returned to the caller.

```
import standard;

fn f(acc, k, v) {
    return acc + v;
}

var a = [1, 2, 3, 4];

print a.reduce(0, f); //prints "10"
```

### some(self, func: fn): bool

This function takes either an array or a dictionary as the `self` argument, and a function as `func`. The argument `func` must take two arguments - the first is the index/key of the array/dictionary, and the second is the value. The contents of `self` are passed into `func`, one element at a time, until `func` returns `true`, at which point this function returns `true`. Otherwise, this function returns `false`.

### sort(self: array, func: fn)

This function takes an array as the `self` argument, and a comparison function as `func`. The argument `func` must take two arguments and return a truthy or falsy value. The contents of the array in `self` are sorted based on the results of `func`, as though the function was the less comparator function.

```
import standard;

fn less(a, b) {
    return a < b;
}

var a = [4, 1, 3, 2];

print a.sort(less); //prints "[1, 2, 3, 4]"
```

### toLower(self: string): string

This function returns a new string which is identical to the string `self`, except any uppercase letters are replaced with the corresponding lowercase letters.

### toString(self): string

This function returns a string representation of `self`. This is intended for arrays and dictionaries, but can theoretically work on any printable value.

If the resulting string is longer than `TOY_MAX_STRING_LENGTH` - 1, then it is truncated.

### toUpper(self: string):string

This function returns a new string which is identical to the string `self`, except any lowercase letters are replaced with the corresponding uppercase letters.

### trim(self: string, trimChars: string = " \t\n\r"): string

This function returns a new string which is identical to the string `self`, except any characters at the beginning or end of `self` which are present in the argument `trimChars` are removed. The argument `trimChars` is optional, and has the following characters as the default value:

* The space character
* The horizontal tab character
* The newline character
* The carriage return character

These characters used because they are the only control characters currently supported by Toy.

### trimBegin(self: string, trimChars: string = " \t\n\r"): string

This function is identical to `trim(self, trimChars)`, except it is only applied to the beginning of the first argument.

### trimEnd(self: string, trimChars: string = " \t\n\r"): string

This function is identical to `trim(self, trimChars)`, except it is only applied to the end of the first argument.


