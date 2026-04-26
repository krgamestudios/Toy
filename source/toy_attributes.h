#pragma once

#include "toy_value.h"
#include "toy_vm.h"

Toy_Value handleStringAttributes(Toy_VM* vm, Toy_Value compound, Toy_Value attribute);
Toy_Value handleArrayAttributes(Toy_VM* vm, Toy_Value compound, Toy_Value attribute);
Toy_Value handleTableAttributes(Toy_VM* vm, Toy_Value compound, Toy_Value attribute);

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
