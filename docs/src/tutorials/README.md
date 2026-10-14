<p align="center">
	<image src="img/toypreview.png" alt="The Toy Scripting Language" />
</p>

## About Toy

In 2017, I read Bob Nystrom's [Crafting Interpreters](https://craftinginterpreters.com/), while it was released piecemeal on his website, and cobbled together the beginnings of a scripting language. At the time it was little more than a variant of Nystrom's Lox treewalker, over the years I would repeatedly revisit the idea with some half-hearted attempts at developing my own scripting language. Some of them even worked, depending on how you define "worked", and there were different approaches for each - some were written in Java, C# or C++, but ultimately I would settle with C as the easiest implementation language that was also widely known.

I started a new attempt in 2022, with the express intention of finishing it - and in many ways I succeeded, though this version had some unrecoverable issues, largely stemming from memory management and architectural details[^1], and it was ultimately not fit for the purpose I had in mind for it. After almost exactly two years, I put a nice bow on v1 and called it a day, taking what I had learned into v2 to produce what you're using now.

[^1]: Even though v1 was set aside, you can still clearly see it and its subtle differences in "toypreview.png" at the top of this page.

At the time of writing, Toy v2 is still in beta, but its so close to "ready" that I'm now writing a tutorial for those who want to learn to use it properly without relying on scraps of notes and summaries of the API. My plan is to give a full explanation of every corner of the core language, as well as tutorials and references for the C API that underpins it.

The Toy Scripting Language isn't just some fun experiment for myself, I'm making it with the explicit intention that it can be used by other people. Bjarne Stroustrup once wrote "There are only two kinds of languages: the ones people complain about and the ones nobody uses." That may be bold of me, but I want to make a language that people complain about.

## Philosophy and Use Cases

I seem to have an odd fascination with game mods and modding in general, despite not actually making many mods in my time (or even playing with them very often, with the exception of Minecraft). My plan when developing Toy was to embed it into a game engine, so it could act as the scripting component and allow easy modding by players who weren't very versed with the myriad of coding languages[^2]. As such, the first philosophy should be simplicity.

[^2]: *What* game exactly, I hadn't considered yet - I'd figure it out when I got there.

How you define simplicity varies from person to person, but I've found that having only one way of doing things not only makes code more consistent and easier to read, but also reduces the number of tools the user has to memorize.

Limiting the tools available in the core language was another philosophy here, so you'll find that Toy has significant overlap with other languages that also aim for simplicity - most notably [lua](https://www.lua.org/), though the syntax is closer to other languages like JavaScript.

Toy is also extensible, in that you can inject functions implemented in C directly into the script's scopes, and they operate exactly like functions written in Toy itself, and you can pass around and operate on variables that point to data within the host program. Throw in some basic containers like arrays and tables, and Toy now has everything needed to do what's required of it.

Toy isn't limited to game engines, but the speed and flexibility required should hopefully be enough to prove it. If it doesn't meet a need, it can be tweaked in small ways, or given a sizeable overhaul until it's just right, which is the third philosophy when designing it.

This flexibility does have its drawbacks, such as incompatibility with older code, but I decided that it was worth the trade-offs of either rewriting or removing older aspects of the language that are no longer needed[^3]. The API is by definition unstable, but once I exit v2's beta the [semantic versioning](https://semver.org/) will kick in again, so it won't change without notice.

[^3]: One example is the addition of `Toy_adjustParserWorkingDirectory`, because I couldn't for the life of me maintain each script file's directory for the `import` keyword without major cross-cutting changes, so embedding the correct path at the parser stage was the least impactful solution that still irks me.

## Tutorials and References

Here's some pages that may or may not yet exist - I'll fill these out over time and begin testing them (yes, you absolutely SHOULD be testing documentation!)

* [Variables, Values and Types](variables.md) (oh my!)
