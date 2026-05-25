# Toy v2 Quick-Start Guide

To help you start using Toy as fast as possible, here are the most useful elements of the language. Not everything available is listed, but this should let you start coding right away.

## Keyword 'print'

The `print` keyword prints a given value to stdout (or elsewhere if configured with the API).

```
print "Hello World!";
```

## Keyword 'assert'

The `assert` keyword takes two values separated by a comma. If the first value is falsy or `null` the optional second parameter is printed to stderr (or elsewhere if configured with the API). If no second parameter is provided a generic error message is used instead.

```
//nothing happens
assert 1 < 2;

//this assert will fail, and output the second parameter
assert null, "Hello world!";
```

## Variables and Types

Variables are declared with the `var` keyword with and an optional type from the list below. If no type is specified `Any` is used instead.

```
var answer = 42;

var question: String = "How many roads must a man walk down?";
```

To make a variable immutable put the `const` keyword after the type. If you do, it must be assigned a value.

```
var quote: String const = "War. War never changes.";
```

Toy's types are:

| type | name | description |
| --- | --- | --- |
| `Bool` | Boolean | Either `true` or `false`. |
| `Int` | Integer | Any signed whole number (32-bits). |
| `Float` | Float | Any signed decimal number (32-bits), using floating point arithmatic. |
| `String` | String | Normal text, effectively utf-8. |
| `Array` | Array | A series of values stored sequentially in memory. |
| `Table` | Table | A series key-value pairs stored in a hash table. Booleans, functions, opaques and `null` can't be used as keys. |
| `Function` | Function | A chunk of reusable code that takes zero or more parameters, and may return a result. Functions are declared with the `fn` keyword, or in the API. |
| `Opaque` | Opaque | This value is unusable in Toy, but allows you to pass data between C bindings provided with the API. |
| `Any` | Any | The default type when nothing is specified. It can hold any value. |

## Control Flow

Making a decision, or repeating a chunk of code multiple times, is essential for any language. Choosing between multiple options can be done with the `if-then-else` statement - if the condition is truthy, the 'then-branch' will be executed. Otherwise, the optional 'else-branch' is executed instead.

```
var answer = 42;

if (answer < 56) {
    print "Cod dang it!";
}
else {
    print "Something's fishy here...";
}
```

```
var challenge = "hard";

if (challenge == "hard") {
    print "I choose to build a scripting language, not because it's easy, but because it's hard!";
}

//the else-branch is optional
```

To repeat a certain action, use the `while-then` loop, which repeats the body as long as the given condition remains true on each loop.

```
var loops = 0;

while (loops++ < 8) {
    print "These episodes are endless.";
}
```

To break out of a loop, you can use the `break` keyword. Alternatively, to restart the loop early, use the `continue` keyword.

```
var loops = 0;

while (true) {
    if (++loops < 15532) {
        continue;
    }

    break; //poor yuki ;_;
}
```

Alternatively, when iterating over an array or table, you can use the `for` keyword like this. While inside a for-loop, the iterable is inaccessible to prevent run-time modification (it's an ad-hoc bugfix, and will likely be improved later).

```
//Iterate on an array with a for-loop
var array = ["foo", "bar", "buzz", "fizz"];

for (var i in array) {
    if (i == "buzz") break; //supports break and continue
    print i;
}
```

```
//Also works when printing the values in a table
//However, the order of the values is undefined
var table = [
	"Alpha": 1,
	"Bravo": 2,
	"Charlie": 3,
	"Delta": 4,
	"Echo": 5,
];

for (var i in table) {
	print i; //this will print a number
    print table; //this will print null
}
```

## Arrays and Tables

Arrays are defined with a pair of brackets, and can contain a list of comma-separated values.

```
//define an array
var array = [1,2,3];

//specify the type
var bray: Array = [4,5,6];

//define an empty array
var craycray: Array = [];

//arrays are zero-indexed
print array[0]; //'1'
```

Tables are also defined with brackets, and contain a comma-separated list of key-value pairs defined by colons:

```
//most types can be used as keys
var table = ["alpha": 1, "beta": 2, "gamma": 3];

//the 'Table' keyword can define the type, and an empty table still has a colon
var under: Table = [:];

//printing the whole table does NOT guarantee internal order
print table["beta"];
```

## Attributes

Some values, including Strings, Arrays and Tables, have "attributes" which are accessible with the dot `.` operator. These can expose internal values or components for manipulating said values.

```
var string = "Hello World";
print string.length; //11
print string.asUpper; //HELLO WORLD
print string.asLower; //hello world

var array = [1,2,3];
array.pushBack(4); //array = [1,2,3,4]
var element = array.popBack(); //element = 4
var emptyArray = [];

var table = ["alpha": 1, "beta":2];
print table.length; //2
table.insert("key",element); //table["key"] = 4
print table.hasKey("alpha"); //true
table.remove("alpha"); //table = ["beta":2,"key":4]
var emptyTable = [:];
```

Opaques can also be given attributes, but this requires some in-depth understanding of the API, so won't be covered here.

## Functions

Functions are defined with the `fn` keyword, and follow a c-like syntax, with optional types on each parameter:

```toy
fn fib(n: Int) {
	if (n < 2) return n;
	return fib(n-1) + fib(n-2);
}

print fib(12); //144
```

```toy
fn isLeapYear(n: Int) {
	if (n % 400 == 0) return true;
	if (n % 100 == 0) return false;
	return n % 4 == 0;
}
```

## External API and Extending Toy

*Note: Watch this space, docs for the C API are coming soon. For now, the [Cheat Sheet](/cheatsheet) can get you started.*

