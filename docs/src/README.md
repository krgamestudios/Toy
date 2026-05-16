<p align="center">
	<image src="img/toylogo.png" alt="The Toy Logo" />
</p>

The Toy Programming Language is an imperative, bytecode-interpreted, embeddable scripting language. Rather than functioning independently, it serves as part of another program, the "host". This design allows for straightforward customization by both the host's developers and end users, achieved by exposing program logic through external scripts.

## Nifty Features

* Simple C-like syntax
* Intermediate AST and bytecode representations
* Strong, but optional type system
* First-class functions and closures
* Extensible with native C-bindings
* Can re-direct output, error and assertion messages
* Open-Source under the zlib license

## Syntax

```toy
fn makeCounter() {
	var counter: Int = 0;

	fn increment() {
		return ++counter;
	}

	return increment;
}

var tally = makeCounter();

while (true) {
	var result = tally();

	print result; //prints 1 to 10

	if (result >= 10) {
		break;
	}
}
```

## Further Reading

This website is a work in progress - for further info, see the official repository: [https://gitea.krgamestudios.com/krgamestudios/Toy](https://gitea.krgamestudios.com/krgamestudios/Toy), or the GitHub mirror: [https://github.com/krgamestudios/Toy](https://github.com/krgamestudios/Toy).

An example of Toy in action: [Vampire Toyvivors](https://gitea.krgamestudios.com/krgamestudios/VampireToyvivors) (a simple "game" used for testing).