#pragma once

#include "toy_common.h"

#include "toy_value.h"
#include "toy_vm.h"

// [x] string.length
// [x] string.asUpper
// [x] string.asLower
// [x] array.length
// [x] array.pushBack(x)
// [x] array.popBack()
// [x] array.forEach(fn) // fn(x) -> void
// [ ] array.sort(fn)    // fn(a,b) -> int
// [x] table.length
// [x] table.insert(x, y)
// [x] table.hasKey(x)
// [x] table.remove(x)
// [ ] table.forEach(fn) // fn(x,y) -> void

Toy_Value Toy_private_handleStringAttributes(Toy_VM* vm, Toy_Value compound, Toy_Value attribute);
Toy_Value Toy_private_handleArrayAttributes(Toy_VM* vm, Toy_Value compound, Toy_Value attribute);
Toy_Value Toy_private_handleTableAttributes(Toy_VM* vm, Toy_Value compound, Toy_Value attribute);
Toy_Value Toy_private_handleOpaqueAttributes(Toy_VM* vm, Toy_Value compound, Toy_Value attribute);

//plug-and-play attributes for custom objects
typedef Toy_Value (*Toy_OpaqueAttributeHandler)(Toy_VM* vm, Toy_Value compound, Toy_Value attribute);
TOY_API void Toy_setOpaqueAttributeHandler(Toy_OpaqueAttributeHandler cb);