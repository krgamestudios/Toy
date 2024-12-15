#include "toy_print.h"
#include "toy_console_colors.h"

#include <stdio.h>

int counter = 0;

void count(const char* msg) {
	(void)msg;
	counter++;
}

int test_callbacks(void) {
	//set a custom print callback, invoke it, and reset
	{
		//setup
		Toy_setPrintCallback(count);

		//invoke
		Toy_print("");

		//check
		if (counter != 1) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to set print callback\n" TOY_CC_RESET);
			return -1;
		}

		//reset and retry
		Toy_resetPrintCallback();
		Toy_print("");

		if (counter != 1) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to reset print callback\n" TOY_CC_RESET);
			return -1;
		}

		//cleanup
		counter = 0;
	}

	//set a custom error callback, invoke it, and reset
	{
		//setup
		Toy_setErrorCallback(count);

		//invoke
		Toy_error("");

		//check
		if (counter != 1) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to set error callback\n" TOY_CC_RESET);
			return -1;
		}

		//reset and retry
		Toy_resetErrorCallback();
		Toy_error("");

		if (counter != 1) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to reset error callback\n" TOY_CC_RESET);
			return -1;
		}

		//cleanup
		counter = 0;
	}

	//set a custom assert failure callback, invoke it, and reset
	{
		//setup
		Toy_setAssertFailureCallback(count);

		//invoke
		Toy_assertFailure("");

		//check
		if (counter != 1) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to set assert failure callback\n" TOY_CC_RESET);
			return -1;
		}

		//reset and retry
		Toy_resetAssertFailureCallback();
		Toy_assertFailure("");

		if (counter != 1) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to reset assert failure callback\n" TOY_CC_RESET);
			return -1;
		}

		//cleanup
		counter = 0;
	}

	return 0;
}

int main(void) {
	//run each test set, returning the total errors given
	int total = 0, res = 0;

	{
		res = test_callbacks();
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	return total;
}

