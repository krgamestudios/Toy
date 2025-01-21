
shared:
* Functions can be stored within Toy_Value & loaded by Toy_VM
* They have a list of parameters
* They can return a number of values


```toy
fn makeCounter() {
	var count = 0;

	fn next() {
		return ++count;
	}

	return next;
}

var tally = makeCounter();

print tally(); //1
print tally(); //2
print tally(); //3
```

```toy
import standard as std;

print std.clock();
```

bytecode_functions:
* They point to a raw chunk of bytecode (the module)
* Closures are supported via scopes (they have a persistent scope)

c_functions:
* Delegates to user code & APIs
* invoked via 'hooks' (which are called with the 'import' keyword)

```
typedef struct Toy_Module {
	Toy_Scope* scopePtr;

	unsigned char* code;
	unsigned int codeSize;

	unsigned int paramSize;
	unsigned int jumpsSize;
	unsigned int dataSize;
	unsigned int subsSize;

	unsigned int paramAddr;
	unsigned int codeAddr;
	unsigned int jumpsAddr;
	unsigned int dataAddr;
	unsigned int subsAddr;
}
```

flow:
	compile module from AST ->
		append module to bundle ->
			bind VM (takes module) ->
				run VM

Definitions:
	bundles have version info (MAJOR, MINOR, PATCH, BUILD) and may have more than 1 modules.
	Modules are chunks of usable memory, loadable into VMs.
	VMs run the actual code.
	Functions are unions, either pointers to modules, or pointers to user-defined C callbacks.
	Functions are packed into values.

API:
	init bundle
	verify bundle

	append bundle with module
	extract module from bundle

# Notes

* Scopes, buckets, strings, etc. will persist until the root VM is cleared
* The parameters are...