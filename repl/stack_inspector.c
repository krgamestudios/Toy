#include "stack_inspector.h"
#include "toy_console_colors.h"
#include "toy_string.h"

#include <stdio.h>
#include <stdlib.h>

void inspect_stack(Toy_Stack* stack) {
	printf(TOY_CC_NOTICE "Stack State: %u / %u\n" TOY_CC_RESET, stack->count, stack->capacity);

	if (stack->count == 0) {
		printf(TOY_CC_NOTICE "\n-- Empty Stack --\n\n" TOY_CC_RESET);
		return;
	}

	Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);

	printf(TOY_CC_NOTICE "-- Beginning Stack Dump --\n\n%-10s%-20s%-20s\n\n" TOY_CC_RESET, "Index", "Type", "Value");

	for (unsigned int i = 0; i < stack->count; i++) {
		Toy_Value v = ((Toy_Value*)(stack + 1))[i]; //'stack + 1' is a naughty trick

		//print type
		printf("%-10u%-20s", i, Toy_getValueTypeAsCString(v.type));

		//print value
		Toy_String* string = Toy_stringifyValue(&bucket, Toy_unwrapValue(v));
		char* buffer = Toy_getStringRaw(string); //lazy, but it works
		printf("%-20s", buffer);
		free(buffer);
		Toy_freeString(string);

		printf("\n");
	}

	printf(TOY_CC_NOTICE "\n-- End Stack Dump --\n\n" TOY_CC_RESET);

	Toy_freeBucket(&bucket);
}