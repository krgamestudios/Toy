#include "scope_inspector.h"
#include "toy_console_colors.h"
#include "toy_string.h"

#include <stdio.h>
#include <stdlib.h>

void inspect_scope(Toy_Scope* scope, int depth) {
	printf(TOY_CC_NOTICE "Scope State: %u / %u\n\n" TOY_CC_RESET, scope->count, scope->capacity);

	if (scope->count == 0) {
		printf(TOY_CC_NOTICE "\n-- Empty Scope --\n\n" TOY_CC_RESET);
		if (scope->next != NULL) inspect_scope(scope->next, depth + 1);
		return;
	}

	Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);

	printf(TOY_CC_NOTICE "-- Beginning Scope Dump [%d] --\n\n%-20s%-20s%-20s\n" TOY_CC_RESET, depth, "Name", "Type", "Value");

	for (unsigned int i = 0; i < scope->capacity; i++) {
		if (scope->data[i].key == NULL || scope->data[i].key->info.length == 0) {
			continue;
		}

		Toy_String* k = scope->data[i].key;
		Toy_Value v = scope->data[i].value;

		//print name & type
		printf("%-20s%-10s%-10s", k != NULL ? k->leaf.data : "", Toy_getValueTypeAsCString(scope->data[i].type), scope->data[i].constant ? "const" : "");

		//print value
		Toy_String* string = Toy_stringifyValue(&bucket, Toy_unwrapValue(v));
		char* buffer = Toy_getStringRaw(string);
		printf("%-20s\n", buffer);

		//clean up & continue
		free(buffer);
		Toy_freeString(string);
	}

	Toy_freeBucket(&bucket);

	printf(TOY_CC_NOTICE "\n-- End Scope Dump [%d] --\n\n" TOY_CC_RESET, depth);

	if (scope->next != NULL) {
		inspect_scope(scope->next, depth + 1);
	}
}