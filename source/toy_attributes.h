#pragma once

#include "toy_value.h"
#include "toy_vm.h"

Toy_Value handleStringAttributes(Toy_VM* vm, Toy_Value compound, Toy_Value attribute);
Toy_Value handleArrayAttributes(Toy_VM* vm, Toy_Value compound, Toy_Value attribute);
Toy_Value handleTableAttributes(Toy_VM* vm, Toy_Value compound, Toy_Value attribute);

// string.length
// string.asUpper
// string.asLower
// array.length
// array.pushBack(x)
// array.popBack()
// array.forEach(fn) // fn(x) -> void
// array.sort(fn)    // fn(a,b) -> int
// table.length
// table.insert(x)
// table.hasKey(x)
// table.remove(x)
// table.forEach(fn) // fn(x) -> void
