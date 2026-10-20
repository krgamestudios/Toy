#pragma once

#include "toy_common.h"

#include "toy_value.h"
#include "toy_vm.h"

//forward declarations
struct Toy_VM;

// NOTE: Standard Attributes
// [x] *.asString
// [x] string.length
// [x] string.asUpper
// [x] string.asLower
// [x] array.length
// [x] array.pushBack(x)
// [x] array.popBack()
// [x] array.fillWith(amount, value)
// [x] array.flatten()
// [x] array.sort(fn)    // fn(a,b) -> int
// [x] array.reverse()
// [x] table.length
// [x] table.insert(x, y)
// [x] table.hasKey(x)
// [x] table.remove(x)

Toy_Value Toy_private_handleStringAttributes(struct Toy_VM* vm, Toy_Value compound, Toy_Value attribute);
Toy_Value Toy_private_handleArrayAttributes(struct Toy_VM* vm, Toy_Value compound, Toy_Value attribute);
Toy_Value Toy_private_handleTableAttributes(struct Toy_VM* vm, Toy_Value compound, Toy_Value attribute);
Toy_Value Toy_private_handleOpaqueAttributes(struct Toy_VM* vm, Toy_Value compound, Toy_Value attribute);

//plug-and-play attributes for custom objects
typedef Toy_Value (*Toy_OpaqueAttributeHandler)(struct Toy_VM* vm, Toy_Value compound, Toy_Value attribute);
TOY_API void Toy_setOpaqueAttributeHandler(Toy_OpaqueAttributeHandler cb);

//operates on all built-in types
int Toy_private_handleGlobalAttributes(struct Toy_VM* vm, Toy_Value compound, Toy_Value attribute, Toy_Value* result);