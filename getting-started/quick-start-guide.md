
# Quick Start Guide

This guide is intended to get you writing Toy code as fast as possible. As such, it's more of a reference for experienced coders to know what is available and what isn't.

Toy programs begin at the top of the file, and continue until the end, unless an error is encountered.

## Print Keyword

This keyword prints values to stdout for debugging (this can be altered by the host program).

```
print "Hello World";
```

## Names and Variables

Variables can store data of any kind, unless a type is specified; see [types](getting-started/types). Names can be up to 256 characters long; see [Reserved Keywords](#reserved-keywords) for a list of keywords that can't be used as a name.

```
var b = true;
var i = 42;
var f = 3.14;
var s = "Hello world";
```

Numbers (both integers and floats) can be delimited with underscores (`_`), to break them up visually, e.g. `100_000`.

Strings can be 4096 characters long, and the following characters can be escaped: `\n`, `\t`, `\\` and `\"`.

## Compounds

Larger containers of data are available - arrays and dictionaries. Arrays are collections of data stored sequentially, while dictionaries are hash-maps of key-value pairs:

```
var array = []; //define an array
var dict = [:]; //define a dictionary

dict["foo"] = "bar"; //you can use indexing to add to a dictionary
array.push(42); //you must use a function to push to an array
```

## Control Flow

You can control the program flow with either `if`, `while` or `for`. The only falsy value is `false`.

```
if (check()) {
	//do this
}
else {
	//otherwise do this
}

var i = 0;
while (i < 10) {
	print i++;
}

for (var i = 0; i < 10; i++) {
	print i;
}
```

`continue` and `break` both behave as you'd expect.

## Functions

Functions are defined with the `fn` keyword, and can take any number of arguments. They can return a value as well.

```
fn combine(a, b, c) {
	return [a, b, c];
}

print combine(1, 2, 3);
```

Variable number of parameters, called rest parameters, can be passed in as an array.

```
fn combine(...rest) {
	return rest;
}

print combine(1, 2, 3);
```

## UFCS and Global Functions

Functions can be called using the universal function call syntax, which is just syntactic sugar for a normal function call:

```
fn printMe(self) {
	print self;
}

array.printMe();
```

There are several globally available functions provided by default:

```
set(self, key, value)                         //array, dictionary
get(self, key)                                //array, dictionary
push(self, value)                             //array
pop(self)                                     //array
length(self)                                  //array, dictionary, string
clear(self)                                   //array, dictionary
```

## Slice Notation

When indexing a compound value, you can use slice notation to manipulate it's elements:

```
var greeting = "Hello world";

print greeting[::-1]; //dlrow olleH

greeting[0:4] = "Goodnight"; //changes greeting to equal "Goodnight world"
```

## External Libraries

The host may, at its own discretion, make external libraries available to the scripts. To access these, you can use the `import` keyword:

```
import standard;

print clock(); //made available by "standard"
```

## Assertion Tests

For testing purposes, there is the `assert` keyword. `assert` takes two arguments, separated by a comma; if the first resolves to a truthy value, then the whole statement is a no-op. Otherwise, the second argument, which MUST be a string, is displayed as an error and the script exits.

```
var answer = 42;

assert answer == 42, "This will not be seen";

//both false and null trigger assert's exit condition
assert null, "This will be seen before the script exits";
```

## Reserved Keywords

The following list cannot be used as names, due to their significance (or potential later use) in the language.

* any
* as
* astype
* assert
* bool
* break
* class (reserved)
* const
* continue
* do (reserved)
* else
* export (reserved)
* false
* float
* fn
* for
* foreach (reserved)
* if
* import
* in (reserved)
* int
* null
* of (reserved)
* opaque
* print
* return
* string
* true
* type
* typeof
* var
* while

## Full List of Operators

The following mathematical operators are available. A definition is omitted here, as they are commonly used in most programming languages.

```
+  -  *  /  %  +=  -=  *=  /=  %=  ++(prefix)  --(prefix)  (postfix)++  (postfix)--
```

Likewise, the following logical operators are available (`&&` is more tightly bound than `||` due to historical reasons):

```
(  )  [  ]  {  }  !  !=  ==  <  >  <=  >=  &&  ||  ?:
```

Other operators used throughout the language are: the assignment, colon, semicolon, comma, dot and rest operators:

```
= : ; , . ...
```
