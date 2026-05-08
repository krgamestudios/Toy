#include "toy_print.h"

#include <stdio.h>

static Toy_callbackType printCallback = puts;
static Toy_callbackType errorCallback = puts;
static Toy_callbackType assertCallback = puts;

void Toy_print(const char* msg) {
	printCallback(msg);
}

void Toy_error(const char* msg) {
	errorCallback(msg);
}

void Toy_assertFailure(const char* msg) {
	assertCallback(msg);
}

void Toy_setPrintCallback(Toy_callbackType cb) {
	printCallback = cb;
}

void Toy_setErrorCallback(Toy_callbackType cb) {
	errorCallback = cb;
}

void Toy_setAssertFailureCallback(Toy_callbackType cb) {
	assertCallback = cb;
}

void Toy_resetPrintCallback(void) {
	printCallback = puts;
}

void Toy_resetErrorCallback(void) {
	errorCallback = puts;
}

void Toy_resetAssertFailureCallback(void) {
	assertCallback = puts;
}
