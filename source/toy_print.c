#include "toy_print.h"

#include <stdio.h>

static void outDefault(const char* msg) {
	fprintf(stdout, "%s", msg);
}

static void errDefault(const char* msg) {
	fprintf(stderr, "%s", msg);
}

static void assertDefault(const char* msg) {
	fprintf(stderr, "%s", msg);
}

static Toy_callbackType printCallback = outDefault;
static Toy_callbackType errorCallback = errDefault;
static Toy_callbackType assertCallback = assertDefault;

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

void Toy_resetPrintCallback() {
	printCallback = outDefault;
}

void Toy_resetErrorCallback() {
	errorCallback = errDefault;
}

void Toy_resetAssertFailureCallback() {
	assertCallback = assertDefault;
}
