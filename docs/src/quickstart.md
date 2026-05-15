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

Variables can be declared with the `var` keyword, and can be given an optional type from the list below. If no type is specified, `Any` is used by default.

```
var answer = 42;

var question: String = "How many roads must a man walk down?";
```

To make a variable immutable, use the `const` keyword after the type declaration. In this case, it must be assigend a value.

```
var quote: string const = "War. War never changes.";
```

The types available in Toy are:

| type | name | description |
| --- | --- | --- |
| `Bool` | Boolean | Either `true` or `false`. |
| `Int` | Integer | Any whole number (32-bits). |
| `Float` | Float | A decimal number (32-bits), using floating-point arithmetic. |
| `String` | String | A series of characters used for text processing. |
| `Array` | Array | A series of values stored sequentially in memory. |
| `Table` | Table | A series key-value pairs stored in such a way that allows for fast lookups. Booleans, functions, opaques and `null` can't be used as keys. |
| `Function` | Function | A chunk of reusable code that takes zero or more parameters, and may return a result. Functions are declared with the `fn` keyword. |
| `Opaque` | Opaque | This value is unusable in Toy, but allows you to pass data between C bindings. |
| `Any` | Any | The default type when nothing is specified. Theis can hold any value. |

## Control Flow

Making a decision, or repeating a chunk of code multiple times, is essential for any general purpose language.

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

//printing a table does NOT guarantee internal order, but the elements can be accessed with the keys.
print table["beta"];
```

## Attributes

Some value types, including Strings, Arrays and Tables, have "attributes" which are accessible with the dot `.` operator. These can expose internal values or components for manipulating said values.

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


## Functions

Functions are defined with the `fn` keyword, and follow a c-like syntax, with optional types on the parameters:

```
fn fib(n: Int) {
	if (n < 2) return n;
	return fib(n-1) + fib(n-2);
}

print fib(12); //144
```

## External Libraries and Extending Toy

Watch this space, docs for the C API are coming soon.

