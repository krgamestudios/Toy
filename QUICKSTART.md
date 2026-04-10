# Toy v2 Quick-Start Guide

To help you start using Toy as fast as possible, here are the most useful elements of the language. Not everything available is listed, but this should let you start coding right away.

## Keyword 'print'

The `print` keyword takes one value as a parameter, which is sent to stdout by default, or can be redirected elsewhere using C.

```
print "Hello World!";
```

## Keyword 'assert'

The `assert` keyword takes two values as parameters, separated by a comma. If the first value is falsy or `null`, the optional second parameter is sent to stderr by default, or can be redirected elsewhere using C. If no second parameter is provided, a generic error message is used instead.

```
//nothing happens
assert 1 < 2;

//this assert will fail, and output the second parameter
assert null, "Hello world!";
```

## Variables and Types

Variables can be declared with the `var` keyword, and can be given an optional type from the list below. If no type is specified, `any` is used by default.

```
var answer = 42;

var question: string = "How many roads must a man walk down?";
```

To make a variable immutable, use the `const` keyword after the type declaration. In this case, it must be assigend a value.

```
var quote: string const = "War. War never changes.";
```

The types available in Toy are:

| type | name | description |
| --- | --- | --- |
| `bool` | boolean | Either `true` or `false`. |
| `int` | integer | Any whole number (32-bits). |
| `float` | float | A decimal number (32-bits), using floating-point arithmetic. |
| `string` | string | A series of characters used for text processing. |
| `array` | array | A series of values stored sequentially in memory. |
| `table` | table | A series key-value pairs stored in such a way that allows for fast lookups. Booleans, functions, opaques and `null` can't be used as keys. |
| `function` | function | A chunk of reusable code that takes zero or more parameters, and returns zero or more results. Functions are declared with the `fn` keyword. |
| `opaque` | opaque | This value is unusable in Toy, but allows you to pass data between C functions. |
| `any` | any | The default type when nothing is specified. Theis can hold any value. |

## Control Flow

Choosing an option, or repeating a chunk of code multiple times, is essential for any general purpose language.

Choosing between two options can be done with the `if-then-else` else statement. If the condition is truthy, the 'then-branch' will be executed. Otherwise, the optional 'else-branch' is executed instead.

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

To repeat a certain action, use the `while-then` loop, which repeats the body as long as the condition is true at the beginning of each loop.

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

*Note: The `for` loop is coming, eventually, but isn't vital right now.*

## Arrays and Tables

Arrays are defined with a pair of brackets, and can contain a list of comma-separated values.

```
//'array' is a reserved keyword, so it can't be used as a name
var a = [1,2,3];

//instead, it's used as a type
var b: array = [4,5,6];

//define an empty array like this
var c: array = [];

//arrays are zero-indexed
print a[0]; //'1'
```

Tables are also defined with brackets, and contain a comma-separated list of key-value pairs defined by colons:

```
//most types can be used as keys
var t = ["alpha": 1, "beta": 2, "gamma": 3];

//the 'table' keyword can define the type, and an empty table still has a colon
var u: table = [:];

//printing a table does NOT guarantee internal order, but the elements can be accessed with the keys.
print t["beta"];
```

## Functions

Watch this space.

## External Libraries and Extending Toy

Watch this space.

