# Theorizing Toy

Sooner or later, every coder will try to create their own programming language. In my case, it took me over a decade and a half to realize that was even an option, but once I did, I read through a fantastic book called [Crafting Interpreters](https://craftinginterpreters.com/). This sent me down the rabbit hole, so to speak.

The main driving idea behind the Toy programming language has remained the same from the very beginning - I wanted a scripting language that could be embedded into a larger host program to allow for easy modification by the end user. Specifically, I wanted to enable easy modding of video games made in an imaginary game engine.

At the time of writing, I've started working on said engine, building it around Toy, and adjusting Toy to fit the engine as needed. I've also begun working on a game within that engine, as I believe the best way to build an engine is to build a game with it first. The engine has been dubbed "Box", and the game is called "Skylands".

But this post isn't about the engine; it's about Toy - I want to explain, in some detail, my thought processes when developing it. Let's start at the beginning.

```toy
print "Hello world";
```

I've drawn the `print` keyword from Crafting Interpreter's Lox language, for much the same reason as explained in the book - it's a simple and easy way to debug issues. You'll be able to print out any kind of value or variable from this statement - but it loses some context, like function implementations and the values of `opaque` literals.

Let's touch on variables quickly. There's about a dozen variable types that can be used, depending on how you count them. They include `bool`, `int`, `float`, `string` and a couple of compound types - but strict typing in Toy is completely optional (`any` is used by default). There are also functions, which are reusable chunks of code, and a pretty standard set of operators with their traditional precedences.

One way in which Toy stands out is the bytecode compilation step. Before execution, the source code must be compiled into an intermediate bytecode format (a trait also inherited from Lox) before it can be executed by the interpreter. The exact specifications of the bytecode formatting are not currently documented (yet). The intermediate bytecode stage and the independence of the interpreter from the compiler also allow unique features, such as the possibility of operating on a microcontroller.

One major native feature that is missing from Toy is an input system, such as from stdin. Instead, Toy is intended to receive its instructions from the host program, including any input needed. One such example would be a game controller library - something that takes in button presses and calls certain Toy functions to move a character around the game world. Toy is almost infinitely extensible via the C API's hook injection system.

I would like to keep the core language nice and simple, as much as possible - something you can explain with just the quick-start page. However, feedback and criticism are always welcome.
