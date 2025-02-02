---
layout: page
title: Toy v2 Quick-Start Guide
order: 1
---

# Toy v2 Quick-Start Guide

To help you start using Toy as fast as possible, here are the most useful elements of the language. Not everything available is listed, but this should let you start coding right away.

## Keyword 'print'

The `print` keyword takes one value as a parameter, which is sent to stdout by default.

The value can be redirected elsewhere using the [debug C API](c_api/debug).

```toy
print "Hello World!";
```

## Keyword 'assert'

The `assert` keyword takes two values as parameters, separated by a comma. If the first value is `null` or `false`, the second parameter is sent to stderr by default. Otherwise, this statement is ignored.

The second value, which is replaced by a default error message if omitted, can be redirected elsewhere using the [debug C API](c_api/debug).

The generation of assert statements can be disabled using the [parser C API](c_api/parser).

```toy
assert 1 < 2;

assert null, "Hello world!";
```

## Variables and Types

Values can be stored in variables, by specifying a name with the `var` keyword. The name can also be declared with a type, which restricts what kind of value can be stored in the name. Types are optional, and defaults to `any` when not used.

```toy
var answer = 42;

var question: string = "How many roads must a man walk down?";
```

To make a variable unchangeable after declaration, you can add the `const` keyword after the type.

```toy
var quote: string const = "War. War never changes.";
```

The usable types are as follows:

| type | name | description |
| --- | --- | --- |
| `bool` | boolean | Either `true` or `false`. |
| `int` | integer | Any whole number, representable by a signed 32-bit integer. |
| `float` | float | A decimal number, represented by 32-bits for floating-point arithmetic. |
| `string` | string | A piece of text, supports UTF-8. |
| `array` | array | A series of values stored in sequential memory. |
| `table` | table | A series key-value pairs stored in such a way that allows for fast lookups. Booleans, functions and opaques can't be used as keys. |
| `function` | function | A chunk of code to be called. It can take multiple parameters, and provide multiple return values. Unlike other variables, functions are declared with the `fn` keyword. |
| `opaque` | opaque | The contents of this value is unusable in the script, but can be passed from one API to another. |
| `any` | any | The default type used when none is specified. This type can hold any value. |

*Note: Functions and opaques are not yet implemented at the time of writing.*

## Control Flow

Performing different actions, or repeating an action multiple times, is essential for any general purpose language.

Choosing between two options can be done with the `if-then-else` else statement. If the condition is 'truthy', the 'then-branch' will be executed. If the condition is not truthy, the optional 'else-branch' is executed instead.

```toy
var answer = 42;

if (answer < 56) {
    print "Success";
}
else {
    print "Something's fishy here...";
}
```

```toy
var challenge = "hard";

if (challenge == "hard") {
    print "I choose to build a scripting language, not because it's easy, but because it's hard!";
}

//the else-branch is optional
```

To repeat a certain action, use the `while-then` loop, which repeats the body as long as the condition is true at the beginning of each loop.

```toy
var loops = 0;

while (loops++ < 8) {
    print "These episodes are endless.";
}
```

To break out of a loop, you can use the `break` keyword. Alternatively, to restart the loop early, use the `continue` keyword.

```toy
var loops = 0;

while (true) {
    if (++loops < 15532) {
        continue;
    }

    break; //poor yuki ;_;
}
```

## Functions

Watch this space.

## External Libraries and Extending Toy

Watch this space.

## Reserved Keywords & Operators

Watch this space.

