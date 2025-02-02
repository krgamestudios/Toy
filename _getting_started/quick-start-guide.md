---
layout: page
title: Toy v2 Quick-Start Guide
order: 1
---

# Toy v2 Quick-Start Guide

To help you start using Toy as fast as possible, here are the most useful elements of the language. Not everything available is listed, but this should let you start coding right away.

## Keyword 'print'

The `print` keyword takes one value as a parameter, which is sent to stdout by default, or can be redirected elsewhere using the [output C API](/c_api/output).

```
print "Hello World!";
```

## Keyword 'assert'

The `assert` keyword takes two values as parameters, separated by a comma. If the first value is falsy or `null`, the optional second parameter is sent to stderr by default, or can be redirected elsewhere using the [output C API](/c_api/output). If no second parameter is provided, a generic message is used instead.

An option to disable the `assert` keyword during compilation is provided in the [parser C API](/c_api/parser).

```
//nothing happens
assert 1 < 2;

//this assert will fail, and output the second parameter
assert null, "Hello world!";
```

## Variables and Types

Values can be stored in variables, by specifying a name with the `var` keyword. The name can be declared with an optional type, which restricts the type of value that can be stored in the name. If no type is specified, `any` is used instead.

```
var answer = 42;

var question: string = "How many roads must a man walk down?";
```

To make a variable immutable, you can add the `const` keyword after the type when it's declared. If you do, it must be assigned a value.

```
var quote: string const = "War. War never changes.";
```

The types available in Toy are:

| type | name | description |
| --- | --- | --- |
| `bool` | boolean | Either `true` or `false`. |
| `int` | integer | Any whole number (32-bits). |
| `float` | float | A decimal number (32-bits), using floating-point arithmetic. |
| `string` | string | A piece of text, supports UTF-8, [in theory](https://github.com/Ratstail91/Toy/issues/174). |
| `array` | array | A series of values stored sequentially in memory. |
| `table` | table | A series key-value pairs stored in such a way that allows for fast lookups. Booleans, functions, opaques and `null` can't be used as keys. |
| `function` | function | A chunk of reusable code that takes zero or more parameters, and returFunctions are declared with the `fn` keyword. |
| `opaque` | opaque | This value is unusable in the script, but can be passed from one imported function to another. |
| `any` | any | The default type when nothing is specified. Theis can hold any value. |

*Note: Functions and opaques are not fully implemented at the time of writing, so details may change.*

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

## Functions

Watch this space.

## External Libraries and Extending Toy

Watch this space.

## Reserved Keywords & Operators

Watch this space.

