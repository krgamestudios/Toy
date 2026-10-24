## Variable Names And Reserved Words

In Toy, a variable is a named piece of data that holds a value of some kind, which is declared with the `var` keyword:

```toy
var counter: Int = 0;
```

Despite being an extremely small snippet of code, there's actually a lot going on here, even without diving into details like scoping and reference counting. So, let's start with the name itself.

A valid name begins with an uppercase or lowercase letter or an underscore[^1], and consists of only uppercase or lowercase letters, underscores or digits from 0 to 9[^2]. In addition, there's a small set of case-sensitive "reserved words" that can't be used as names, as they're either keywords, types or they've been set aside for potential later use with no plans to use them at this time. These are:

[^1]: As far as the lexer cares, an underscore `_` is a letter with no special meaning.

[^2]: Toy was written with the assumption that users would only use the English alphabet (sorry!), and support for non-English characters, umlauts, etc. is currently undefined.

| Keywords | Reserved | Types | Values
| --- | --- | --- | ---
| as | class | Bool | true
| assert | do | Int | false
| break | export | Float | null
| const | yield | String |
| continue || Array |
| else || Table |
| for || Function |
| foreach* || Opaque |
| fn || Any |
| if |||
| import |||
| in* |||
| of* |||
| pass** |||
| print |||
| return |||
| var |||
| while |||

*\*Will be implemented very soon*  
*\*\*May be removed*  

## Variable Types And The Optional Type System

After specifying a variable's name, you may attach a 'type' to that name, so as to restrict the kinds of variables that can be stored within. If no type is specified, then the variable is given the `Any` type, which has no restrictions on the contents.

```toy
//Omitting the type
var variable = "value";

//specifying the type - don't forget the colon!
var variable: String = "value";
```

When declaring or assigning to a variable, it's not strictly necessary to give it a value, in which case the value `null` is used as a placeholder, regardless of any specified type. `null` is a special value that lacks a type of its own, is only equal to itself, and isn't considered "truthy" or "falsy" so can't be used to evaluate conditional statements.

```toy
var total: Int = null;

if (total) { Error: 'null' is neither true nor false
	//
}

if (total > 40) { Error: Can't compare value types 'null' and 'Int'
	//
}
```

Finally, you may follow a type specification with the `const` keyword to ensure the name can't be reassigned after it's initial declaration.

```toy
//'name' can't be changed
var name: String const = "Kayne";

//'job' also can't be changed, but without a value at declaration, it will always be 'null'
var job: String const;

//'passions' will always contain this array, but the array's contents *can* change over time
var passions: Array const = ["gamedev", "langdev", "Pokemon"];
passions.pushBack("gardening"); //OK!
```

## Values

Each type listed above will narrow the potential values that can be assigned to a name, in addition to `null`. These are the most commonly used types in programming, and the number of types are deliberately limited, but they should be (mostly) self-explanatory. The more complex "compound" types often have additional features, listed below and on their own pages.

`Bool`, or "booleans, are either `true` or `false`. All comparison operators return one of these values.

`Int` and `Float` are integers and floats repsectfully, both using 32-bit representations under the hood. All arithmetic operations work on these, namely addition `+`, subtraction `-`, multiplication `*`, division `/`, and integers also support modulo `%`[^3].

[^3]: Division and modulo with a zero denominator will result in an error. 

`String` is a compound type, meaning it has additional components within it, either pre-computed or generated at runtime. Namely, indexing a string will give a new string created from its contents. When indexing, using only a single number will return a new string of length 1, starting from that index. If a second value is also provided after a colon, it becomes the length of the new string[^4].

[^4]: If you use a colon but omit the second value, then the remainder of the string is used, like so: `print string[0:];`.

```toy
//define a string
var string: String = "Hello World";

print string[0]; //H
print string[6:5]; //World
```

The length of a string can be found with the length attribute, and two strings can be combined with the dedicated concatenate `..` operator.

```toy
//you can't imagine the joy I feel from seeing this work
print (string[0:5] .. string[6:]).length; //10
```

`Array` and `Table` are two more compound types, which hold other values within, either sequentially or in key-value pairs. These will be detailed more on their own pages, but to declare and start using an array, use a pair of square brackets `[` `]` with any initial values or variables between, or key-value pairs divided with a colon to declare a table.

```toy
var humanFood: Array = ["bread", "butter", "meat", "lettuce", "tomato"];

var animalFood: Table = [
	"dog": "meat roll",
	"cat": "tuna",
	"bird": ["seeds", "pellets", "honey sticks"]
];
```

`Function` is the type of all functions within Toy[^5], while `Opaque` is intended for user-specified values. These two types are the main way for Toy to be expanded upon and utilized within a host program, as some functions can be written in C and injected into Toy's VM at runtime. They can then provide values that Toy can't interact with directly, but *can* be operated on by other functions from the host, and can even be given attributes of their own[^6].

[^5]: There's actually two kinds of functions, but this is supposed to be invisible from within Toy itself.

[^6]: The Opaque attributes system works well, though it does require some heavy lifting on the developer's part.

```toy
import "procedural_generation.toy" as procgen;
import "structure_generation.toy" as towngen;

var map: Opaque = createBlankMap(50,50);

procgen(map);

while (map.population < 1000) { //make sure there's enough NPCs
	towngen(map);
}

saveGameMap(map);
```

