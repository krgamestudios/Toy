#include "toy_scope.h"
#include "toy_console_colors.h"

#include "toy_bucket.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int test_scope_allocation() {
	//allocate and free a scope
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);

		Toy_Scope* scope = Toy_pushScope(&bucket, NULL);

		//check
		if (scope == NULL ||
			scope->next != NULL ||
			scope->table == NULL ||
			scope->table->capacity != 8 ||
			scope->refCount != 1 ||

			false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to allocate a Toy_Scope\n" TOY_CC_RESET);
			Toy_popScope(scope);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_popScope(scope);
		Toy_freeBucket(&bucket);
	}

	//allocate and free a list of scopes
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);

		//run
		Toy_Scope* scope = NULL;

		for (int i = 0; i < 5; i++) {
			scope = Toy_pushScope(&bucket, scope);
		}

		//check
		if (
			scope == NULL ||
			scope->next == NULL ||
			scope->table == NULL ||
			scope->table->capacity != 8 ||
			scope->refCount != 1 ||

			scope->next->next == NULL ||
			scope->next->table == NULL ||
			scope->next->table->capacity != 8 ||
			scope->next->refCount != 2 ||

			scope->next->next->next == NULL ||
			scope->next->next->table == NULL ||
			scope->next->next->table->capacity != 8 ||
			scope->next->next->refCount != 3 ||

			scope->next->next->next->next == NULL ||
			scope->next->next->next->table == NULL ||
			scope->next->next->next->table->capacity != 8 ||
			scope->next->next->next->refCount != 4 ||

			scope->next->next->next->next->next != NULL ||
			scope->next->next->next->next->table == NULL ||
			scope->next->next->next->next->table->capacity != 8 ||
			scope->next->next->next->next->refCount != 5 || //refCount includes all ancestors

			false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to allocate a list of Toy_Scope\n" TOY_CC_RESET);
			while (scope) {
				scope = Toy_popScope(scope);
			}
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		while (scope) {
			scope = Toy_popScope(scope);
		}
		Toy_freeBucket(&bucket);
	}

	//ensure pops work correctly
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);

		//run
		Toy_Scope* scope = NULL;

		for (int i = 0; i < 5; i++) {
			scope = Toy_pushScope(&bucket, scope);
		}

		for (int i = 0; i < 2; i++) {
			scope = Toy_popScope(scope);
		}

		//check
		if (
			scope == NULL ||
			scope->next == NULL ||
			scope->table == NULL ||
			scope->table->capacity != 8 ||
			scope->refCount != 1 ||

			scope->next->next == NULL ||
			scope->next->table == NULL ||
			scope->next->table->capacity != 8 ||
			scope->next->refCount != 2 ||

			scope->next->next->next != NULL ||
			scope->next->next->table == NULL ||
			scope->next->next->table->capacity != 8 ||
			scope->next->next->refCount != 3 ||

			false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to allocate and free a list of Toy_Scope\n" TOY_CC_RESET);
			while (scope) {
				scope = Toy_popScope(scope);
			}
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		while (scope) {
			scope = Toy_popScope(scope);
		}
		Toy_freeBucket(&bucket);
	}

	//branched scope references
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);

		//run
		Toy_Scope* scopeBase = Toy_pushScope(&bucket, NULL);
		Toy_Scope* scopeA = Toy_pushScope(&bucket, scopeBase);
		Toy_Scope* scopeB = Toy_pushScope(&bucket, scopeBase);

		//check
		if (
			scopeBase == NULL ||
			scopeBase->next != NULL ||
			scopeBase->table == NULL ||
			scopeBase->table->capacity != 8 ||
			scopeBase->refCount != 3 ||

			scopeA == NULL ||
			scopeA->next != scopeBase ||
			scopeA->table == NULL ||
			scopeA->table->capacity != 8 ||
			scopeA->refCount != 1 ||

			scopeB == NULL ||
			scopeB->next != scopeBase ||
			scopeB->table == NULL ||
			scopeB->table->capacity != 8 ||
			scopeB->refCount != 1 ||

			scopeA->next != scopeB->next || //double check

			false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to allocate branched scopes\n" TOY_CC_RESET);
			Toy_popScope(scopeB);
			Toy_popScope(scopeA);
			Toy_popScope(scopeBase);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_popScope(scopeB);
		Toy_popScope(scopeA);
		Toy_popScope(scopeBase);

		Toy_freeBucket(&bucket);
	}

	//deallocate ancestors correctly
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);

		//run
		Toy_Scope* scopeA = Toy_pushScope(&bucket, NULL);
		Toy_Scope* scopeB = Toy_pushScope(&bucket, scopeA);
		Toy_Scope* scopeC = Toy_pushScope(&bucket, scopeB);

		Toy_popScope(scopeB);

		//check
		if (
			scopeA == NULL ||
			scopeA->next != NULL ||
			scopeA->table == NULL ||
			scopeA->table->capacity != 8 ||
			scopeA->refCount != 2 ||

			//scopeB still exists in memory until scopeC is popped
			scopeB == NULL ||
			scopeB->next != scopeA ||
			scopeB->table == NULL ||
			scopeB->table->capacity != 8 ||
			scopeB->refCount != 1 ||

			scopeC == NULL ||
			scopeC->next != scopeB ||
			scopeC->table == NULL ||
			scopeC->table->capacity != 8 ||
			scopeC->refCount != 1 ||

			false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to deallocate ancestors scopes correctly\n" TOY_CC_RESET);
			Toy_popScope(scopeC);
			Toy_popScope(scopeA);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_popScope(scopeC);
		Toy_popScope(scopeA);

		Toy_freeBucket(&bucket);
	}

	//deep copy
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);

		//run
		Toy_Scope* scopeA = Toy_pushScope(&bucket, NULL);
		Toy_Scope* scopeB = Toy_pushScope(&bucket, scopeA);
		Toy_Scope* scopeCopy = Toy_deepCopyScope(&bucket, scopeB);

		//check
		if (
			scopeA == NULL ||
			scopeA->next != NULL ||
			scopeA->table == NULL ||
			scopeA->table->capacity != 8 ||
			scopeA->refCount != 3 ||

			scopeB == NULL ||
			scopeB->next != scopeA ||
			scopeB->table == NULL ||
			scopeB->table->capacity != 8 ||
			scopeB->refCount != 1 ||

			scopeB == NULL ||
			scopeB->next != scopeA ||
			scopeB->table == NULL ||
			scopeB->table->capacity != 8 ||
			scopeB->refCount != 1 ||

			scopeB == scopeCopy ||

			false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to deep copy a scope\n" TOY_CC_RESET);
			Toy_popScope(scopeCopy);
			Toy_popScope(scopeB);
			Toy_popScope(scopeA);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_popScope(scopeCopy);
		Toy_popScope(scopeB);
		Toy_popScope(scopeA);

		Toy_freeBucket(&bucket);
	}

	return 0;
}

int test_scope_elements() {
	//allocate, access and assign an element
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);
		Toy_Scope* scope = Toy_pushScope(&bucket, NULL);

		Toy_String* hello1 = Toy_createNameStringLength(&bucket, "hello", 5, TOY_VALUE_ANY, false);
		Toy_String* hello2 = Toy_createNameStringLength(&bucket, "hello", 5, TOY_VALUE_ANY, false);

		//check nothing is here
		if (Toy_isDeclaredScope(scope, hello2)) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected entry found in Toy_Scope\n" TOY_CC_RESET);
			Toy_freeString(hello2);
			Toy_freeString(hello1);
			Toy_popScope(scope);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//declare and access values
		Toy_declareScope(scope, hello1, TOY_VALUE_FROM_INTEGER(42));

		if (!Toy_isDeclaredScope(scope, hello2)) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected missing entry in Toy_Scope\n" TOY_CC_RESET);
			Toy_freeString(hello2);
			Toy_freeString(hello1);
			Toy_popScope(scope);
			Toy_freeBucket(&bucket);
			return -1;
		}

		Toy_Value* result = Toy_accessScopeAsPointer(scope, hello2);

		//check integer
		if (scope == NULL ||
			scope->next != NULL ||
			scope->table == NULL ||
			scope->table->capacity != 8 ||
			scope->refCount != 1 ||

			TOY_VALUE_IS_INTEGER(*result) != true ||
			TOY_VALUE_AS_INTEGER(*result) != 42 ||

			false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to declare in Toy_Scope\n" TOY_CC_RESET);
			Toy_freeString(hello2);
			Toy_freeString(hello1);
			Toy_popScope(scope);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//assign values
		Toy_assignScope(scope, hello1, TOY_VALUE_FROM_FLOAT(3.1415f));

		Toy_Value* resultTwo = Toy_accessScopeAsPointer(scope, hello2);

		//check float
		if (scope == NULL ||
			scope->next != NULL ||
			scope->table == NULL ||
			scope->table->capacity != 8 ||
			scope->refCount != 1 ||

			TOY_VALUE_IS_FLOAT(*resultTwo) != true ||
			TOY_VALUE_AS_FLOAT(*resultTwo) != 3.1415f ||

			false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to assign in Toy_Scope\n" TOY_CC_RESET);
			Toy_freeString(hello2);
			Toy_freeString(hello1);
			Toy_popScope(scope);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_freeString(hello2);
		Toy_freeString(hello1);
		Toy_popScope(scope);
		Toy_freeBucket(&bucket);
	}

	//find an entry in an ancestor scope
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);
		Toy_Scope* scope = Toy_pushScope(&bucket, NULL);

		Toy_String* hello = Toy_createNameStringLength(&bucket, "hello", 5, TOY_VALUE_ANY, false);

		//declare and push
		Toy_declareScope(scope, hello, TOY_VALUE_FROM_INTEGER(42));

		scope = Toy_pushScope(&bucket, scope);
		scope = Toy_pushScope(&bucket, scope);

		{
			//check it's accessible
			Toy_Value* result1 = Toy_accessScopeAsPointer(scope, hello);

			if (TOY_VALUE_IS_INTEGER(*result1) != true ||
				TOY_VALUE_AS_INTEGER(*result1) != 42)
			{
				fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to access from an ancestor Toy_Scope\n" TOY_CC_RESET);
				Toy_freeString(hello);
				while ((scope = Toy_popScope(scope)) != NULL) /* */;
				Toy_freeBucket(&bucket);
				return -1;
			}
		}

		Toy_declareScope(scope, hello, TOY_VALUE_FROM_FLOAT(3.1415f));

		{
			//check it's shadowed correctly
			Toy_Value* result2 = Toy_accessScopeAsPointer(scope, hello);

			if (TOY_VALUE_IS_FLOAT(*result2) != true ||
				TOY_VALUE_AS_FLOAT(*result2) != 3.1415f)
			{
				fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to shadow an entry in Toy_Scope\n" TOY_CC_RESET);
				Toy_freeString(hello);
				while ((scope = Toy_popScope(scope)) != NULL) /* */;
				Toy_freeBucket(&bucket);
				return -1;
			}
		}

		scope = Toy_popScope(scope);

		{
			//check it's recovered correctly
			Toy_Value* result3 = Toy_accessScopeAsPointer(scope, hello);

			if (TOY_VALUE_IS_INTEGER(*result3) != true ||
				TOY_VALUE_AS_INTEGER(*result3) != 42)
			{
				fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to recover an entry in Toy_Scope\n" TOY_CC_RESET);
				Toy_freeString(hello);
				while ((scope = Toy_popScope(scope)) != NULL) /* */;
				Toy_freeBucket(&bucket);
				return -1;
			}
		}

		Toy_assignScope(scope, hello, TOY_VALUE_FROM_INTEGER(8891));

		{
			//check it's assigned correctly
			Toy_Value* result4 = Toy_accessScopeAsPointer(scope, hello);

			if (TOY_VALUE_IS_INTEGER(*result4) != true ||
				TOY_VALUE_AS_INTEGER(*result4) != 8891)
			{
				fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to assign to an ancestor in Toy_Scope\n" TOY_CC_RESET);
				Toy_freeString(hello);
				while ((scope = Toy_popScope(scope)) != NULL) /* */;
				Toy_freeBucket(&bucket);
				return -1;
			}
		}

		scope = Toy_popScope(scope);

		{
			//check it's in the correct state
			Toy_Value* result5 = Toy_accessScopeAsPointer(scope, hello);

			if (TOY_VALUE_IS_INTEGER(*result5) != true ||
				TOY_VALUE_AS_INTEGER(*result5) != 8891)
			{
				fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to access an altered entry of an ancestor in Toy_Scope\n" TOY_CC_RESET);
				Toy_freeString(hello);
				while ((scope = Toy_popScope(scope)) != NULL) /* */;
				Toy_freeBucket(&bucket);
				return -1;
			}
		}

		//cleanup
		Toy_freeString(hello);
		while ((scope = Toy_popScope(scope)) != NULL) /* */;
		Toy_freeBucket(&bucket);
	}

	return 0;
}

int main() {
	//run each test set, returning the total errors given
	int total = 0, res = 0;

	{
		res = test_scope_allocation();
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		res = test_scope_elements();
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	return total;
}
