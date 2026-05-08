#include "toy_print.h"
#include "toy_console_colors.h"

#include <stdio.h>

int counter = 0;

int counterCallback(const char* msg) {
	(void)msg;
	counter++;
	return 0;
}

int test_callbacks(void) {
	//set a custom print callback, invoke it, and reset
	{
		//setup
		Toy_setPrintCallback(counterCallback);

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
		Toy_setErrorCallback(counterCallback);

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
		Toy_setAssertFailureCallback(counterCallback);

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

int test_console_colors(void) {
	//test each font color
	{
#define TEST_FONT(value) printf("\033[" value "m" #value TOY_CC_RESET "\n")
		TEST_FONT(TOY_CC_FONT_BLACK);
		TEST_FONT(TOY_CC_FONT_RED);
		TEST_FONT(TOY_CC_FONT_GREEN);
		TEST_FONT(TOY_CC_FONT_YELLOW);
		TEST_FONT(TOY_CC_FONT_BLUE);
		TEST_FONT(TOY_CC_FONT_MAGENTA);
		TEST_FONT(TOY_CC_FONT_CYAN);
		TEST_FONT(TOY_CC_FONT_WHITE);
		TEST_FONT(TOY_CC_FONT_DEFAULT);
#undef TEST_FONT
	}

	//test each background color
	{
#define TEST_BACK(value) printf("\033[" value "m" #value TOY_CC_RESET "\n")
		TEST_BACK(TOY_CC_BACK_BLACK);
		TEST_BACK(TOY_CC_BACK_RED);
		TEST_BACK(TOY_CC_BACK_GREEN);
		TEST_BACK(TOY_CC_BACK_YELLOW);
		TEST_BACK(TOY_CC_BACK_BLUE);
		TEST_BACK(TOY_CC_BACK_MAGENTA);
		TEST_BACK(TOY_CC_BACK_CYAN);
		TEST_BACK(TOY_CC_BACK_WHITE);
		TEST_BACK(TOY_CC_BACK_DEFAULT);
#undef TEST_BACK
	}

	//test the commonly used shorthands
	{
#define TEST_MACRO(value) printf(value #value TOY_CC_RESET "\n")
		TEST_MACRO(TOY_CC_DEBUG);
		TEST_MACRO(TOY_CC_NOTICE);
		TEST_MACRO(TOY_CC_WARN);
		TEST_MACRO(TOY_CC_ERROR);
		TEST_MACRO(TOY_CC_ASSERT);
		TEST_MACRO(TOY_CC_RESET);
#undef TEST_MACRO
	}

	//This will always return zero, so it needs a visual check
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

	{
		res = test_console_colors();
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	return total;
}

