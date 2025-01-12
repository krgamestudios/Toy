
# Preface

The game engine is incomplete and still evolving, as such this page should be considered outdated at all times...

# Game Engine

The Toy programming language was designed from the beginning as an embedded scripting language for some kind of game engine. Different iterations have existed with different implementations, some of which could charitably be said to function. The current version, and the most stable and feature-complete so far, has reached a point where it needs some kind of concrete engine to improve any further.

Currently, the engine exists within its own repository, which can be found here:

[https://github.com/Ratstail91/Box](https://github.com/Ratstail91/Box)

## Engine Structure

The engine's APIs depend heavily on Toy's drive system, so that must be initialized at the beginning of the `main()` function.

The engine proper is invoked with just three lifecycle functions`:

```c
#include "box_engine.h"

int main(int argc, char* argv[]) {
	//initialize the drive system
	Toy_initDriveSystem();
	Toy_setDrivePath("scripts", "assets/scripts");

	//invoke the engine
	Box_initEngine("scripts:/init.toy"); //passing in the specified init file
	Box_execEngine();
	Box_freeEngine();

	//clean up the drive system when you're done
	Toy_freeDriveSystem();

	return 0;
}

```

The engine proper holds the following elements:

* SDL2 video and audio elements
* Frame rate controls
* Toy interpreter
* Input keyboard mapping dictionaries
* The root node

Video and audio in the game engine are handled by SDL2 and SDL2_mixer - this allows for the game to, in theory, be built on multiple platforms. For the time being, however, we're just targeting Windows due to a lack of testing machines.

The engine is calibrated to run at 60 FPS - if the "simulation time" falls too far behind the real time, then frame rendering is skipped in favour of speeding up the game logic. Likewise, if the simulation time runs too fast, then the simulation step is skipped instead and an extra frame is drawn. This process can lead to multiple skips in a row, in both directions.

The engine's interpreter is the core of the scripting system. Other interpreters may be generated and cleared during Toy's internal processes, but this one lasts for the duration of the program.

The keyboard inputs can be remapped using API functions - these mappings are stored and accessed within a pair of Toy dictionaries that have simply been embedded directly into the engine for easy access.

The nodes form a deep tree-like structure, with the "root node" at it's base.

## Node Structure

The fundamental building block of the engine's logic is the node structure - nodes can represent anything within the game world, from entities to abstract global systems. You can think of entities as having a 1:1 mapping to Toy scripts, as each one is given bytecode on initialization that populates its internals (external libraries like the runner library are still possible).

A node holds the following elements:

* A reference to it's parent
* An array of references to its children, and bookkeeping variables for tracking them
* A dictionary of functions defined in the Toy script
* A single SDL texture reference, and controls for rendering it (including as an animated sprite sheet)
* Position, motion and scale values

The nodes are deeply integrated with Toy scripts, while Toy was written specifically for this purpose. The tree-like structure of the nodes all exist entirely within the computer's heap memory as a result of Toy's memory model - this comes with performance drawbacks and clean up requirements. Child nodes should never be referenced directly, as they may be `NULL` references that have been released, but not yet pruned.

The rules of execution for scripts and functions is as follows:

* The script is executed during node initialization
* All functions (regardless of name) are stored within the node - effectively preserving the scope of the script as a whole
* These functions can now be invoked from elsewhere in the program
* Certain function names (listed below) are invoked at specific times during the game loop throughout the entire node tree
* If the specially named functions do not exist, the node is simply skipped
* Every function, which is intended to be called through `callNodeFn()` or at specific times in the loop, must take the `opaque` node as its first argument

## Special Function Names

The following functions, which are defined within the node scripts, are invoked at specific times within the game loop. Note that if you want code to execute *during* node creation, place it within the script's root scope. Variables that you want to persist between calls should also be placed in the script's root.

* `onLoad(node: opaque)`
* `onInit(node: opaque)`
* `onFrameStart(node: opaque)`
* `onUpdate(node: opaque, delta: int)`
* `onFrameEnd(node: opaque)`
* `onStep(node: opaque)`
* `onFree(node: opaque)`
* `onDraw(node: opaque)`
* `onKeyDown(node: opaque, event: string)`
* `onKeyUp(node: opaque, event: string)`
* `onMouseMotion(node: opaque, x: int, y: int, xrel: int, yrel: int)`
* `onMouseButtonDown(node: opaque, x: int, y: int, button: string)`
* `onMouseButtonUp(node: opaque, x: int, y: int, button: string)`
* `onMouseWheel(node: opaque, xrel: int, yrel: int)`

(These may change or expand as more input devices are added, and the engine matures.)

NOTE: `onLoad()` is invoked every time a node is loaded - but `onInit()` is only invoked once by the engine. After that, `initNode()` must be called manually on any node children that are loaded later.

# Engine Libraries

A series of libraries are provided to allow Toy to interface and control the engine. In addition, the libraries stored within Toy's `repl/` directory are also available (see the main page for the list).

During startup, the script named `init.toy` in the assets/scripts directory is executed. This file can be used to configure input mappings, as well as initializing the window and node tree.

* Engine Library
* Node Library
* Input Library
* Music Library
* Sound Library


