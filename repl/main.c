#include "bucket_inspector.h"
#include "ast_inspector.h"
#include "bytecode_inspector.h"
#include "stack_inspector.h"
#include "scope_inspector.h"

#include "toy_console_colors.h"

#include "toy_lexer.h"
#include "toy_parser.h"
#include "toy_compiler.h"
#include "toy_vm.h"

#include "standard_library.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char* readFile(const char* path, int* size) {
	//open the file
	FILE* file = fopen(path, "rb");
	if (file == NULL) {
		*size = -1; //missing file error
		return NULL;
	}

	//determine the file's length
	fseek(file, 0L, SEEK_END);
	*size = (int)ftell(file);
	rewind(file);

	//make some space
	unsigned char* buffer = malloc(*size + 1);
	if (buffer == NULL) {
		fclose(file);
		return NULL;
	}

	//read the file
	if (fread(buffer, sizeof(unsigned char), *size, file) < (unsigned int)(*size)) {
		fclose(file);
		free(buffer);
		*size = -2; //singal a read error
		return NULL;
	}

	buffer[(*size)] = '\0';

	//clean up and return
	fclose(file);
	return buffer;
}

int getFileName(char* dest, const char* src, size_t destLength) {
#define MIN(a, b) ((a) < (b) ? (a) : (b))
	char* p = NULL;

	//find the last slash, regardless of platform
	p = strrchr(src, '\\');
	if (p == NULL) {
		p = strrchr(src, '/');
	}
	if (p == NULL) {
		int len = MIN(strlen(src), destLength-1);
		strncpy(dest, src, len);
		dest[len] = '\0';
		return len;
	}

	p++; //skip the slash

	//determine length of the file name
	int len = MIN(strlen(src), destLength-1);

	//copy to the dest
	strncpy(dest, p, len);
	dest[len] = '\0';

	return len;
#undef MIN
}

//error callbacks
static int errorAndExitCallback(const char* msg) {
	fprintf(stderr, TOY_CC_ERROR "Error: %s" TOY_CC_RESET "\n", msg);
	exit(-1);
}

static int errorAndContinueCallback(const char* msg) {
	return fprintf(stderr, TOY_CC_ERROR "Error: %s" TOY_CC_RESET "\n", msg);
}

static int assertFailureAndExitCallback(const char* msg) {
	fprintf(stderr, TOY_CC_ASSERT "Assert Failure: %s" TOY_CC_RESET "\n", msg);
	exit(-1);
}

static int assertFailureAndContinueCallback(const char* msg) {
	return fprintf(stderr, TOY_CC_ASSERT "Assert Failure: %s" TOY_CC_RESET "\n", msg);
}

static int noOpCallback(const char* msg) {
	//NO-OP
	(void)msg;
	return 0;
}

static int silentExitCallback(const char* msg) {
	//NO-OP
	(void)msg;
	exit(-1);
}

//handle command line arguments
typedef struct Settings {
	bool error;
	bool help;
	bool version;
	const char* script;
	bool silentPrint;
	bool silentAssert;
	bool verbose;
} Settings;

void usageInfo(int argc, const char* argv[]) {
	(void)argc;
	printf("Usage: %s [ -h | -v | -f script.toy ] [-d]\n\n", argv[0]);
}

void helpInfo(int argc, const char* argv[]) {
	usageInfo(argc, argv);

	printf("The Toy Programming Language, leave arguments blank for an interactive REPL.\n\n");

	printf("  -h, --help\t\t\tShow this help then exit.\n");
	printf("  -v, --version\t\t\tShow version and copyright information then exit.\n");
	printf("  -f, --file script\t\tParse, compile and execute the file then exit.\n");
	printf("      --silent-print\t\tSuppress output from the print keyword.\n");
	printf("      --silent-assert\t\tSuppress output from the assert keyword.\n");
	printf("  -d, --verbose\t\tPrint debugging information about Toy's internals.\n");
}

void versionInfo(int argc, const char* argv[]) {
	(void)argc;
	(void)argv;
	printf("The Toy Programming Language, Version %d.%d.%d %s\n\n", TOY_VERSION_MAJOR, TOY_VERSION_MINOR, TOY_VERSION_PATCH, TOY_VERSION_BUILD);

	//copy/pasted from the license file - there's a way to include it directly, but it's too finnicky to bother
	const char* license = 
		"Copyright (c) 2020-2026 Kayne Ruse, KR Game Studios\n"
		"\n"
		"This software is provided 'as-is', without any express or implied\n"
		"warranty. In no event will the authors be held liable for any damages\n"
		"arising from the use of this software.\n"
		"\n"
		"Permission is granted to anyone to use this software for any purpose,\n"
		"including commercial applications, and to alter it and redistribute it\n"
		"freely, subject to the following restrictions:\n"
		"\n"
		"1. The origin of this software must not be misrepresented; you must not\n"
		"claim that you wrote the original software. If you use this software\n"
		"in a product, an acknowledgment in the product documentation would be\n"
		"appreciated but is not required.\n"
		"2. Altered source versions must be plainly marked as such, and must not be\n"
		"misrepresented as being the original software.\n"
		"3. This notice may not be removed or altered from any source distribution.\n"
		"\n"
	;

	printf("%s",license);
}

Settings parseSettings(int argc, const char* argv[]) {
	Settings settings = {0};

	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
			settings.help = true;
		}

		else if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--version")) {
			settings.version = true;
		}

		else if (!strcmp(argv[i], "-f") || !strcmp(argv[i], "--file")) {
			if (argc <= i + 1) {
				settings.error = true;
				break;
			}

			if (settings.script != NULL) {
				fprintf(stderr, TOY_CC_ERROR "ERROR: No more than one script file allowed\n" TOY_CC_RESET);
				settings.error = true;
				break;
			}

			i++;
			settings.script = argv[i];
		}

		else if (!strcmp(argv[i], "--silent-print")) {
			settings.silentPrint = true;
		}

		else if (!strcmp(argv[i], "--silent-assert")) {
			settings.silentAssert = true;
		}

		else if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--verbose")) {
			settings.verbose = true;
		}

		else {
			settings.error = true;
		}
	}

	return settings;
}

//repl function
int repl(const char* filepath, bool verbose) {
	//output options
	Toy_setPrintCallback(puts);
	Toy_setErrorCallback(errorAndContinueCallback);
	Toy_setAssertFailureCallback(assertFailureAndContinueCallback);

	//vars to use
	char prompt[256];
	getFileName(prompt, filepath, 256);
	unsigned int INPUT_BUFFER_SIZE = 4096;
	char inputBuffer[INPUT_BUFFER_SIZE];
	memset(inputBuffer, 0, INPUT_BUFFER_SIZE);

	Toy_VM vm;
	Toy_initVM(&vm);

	printf("%s> ", prompt); //shows the terminal prompt and begin

	//read from the terminal
	while(fgets(inputBuffer, INPUT_BUFFER_SIZE, stdin)) {
		//work around fgets() adding a newline
		unsigned int length = strlen(inputBuffer);
		if (inputBuffer[length - 1] == '\n') {
			inputBuffer[--length] = '\0';
		}

		if (length == 0 || inputBuffer[ strspn(inputBuffer, " \r\n\t") ] == '\0') {
			printf("%s> ", prompt); //shows the terminal prompt and restart
			continue;
		}

		//end
		if (strlen(inputBuffer) == 4 && (strncmp(inputBuffer, "exit", 4) == 0 || strncmp(inputBuffer, "quit", 4) == 0)) {
			break;
		}

		//parse the input, prep the VM for execution
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);
		Toy_Lexer lexer;
		Toy_bindLexer(&lexer, inputBuffer);
		Toy_Parser parser;
		Toy_bindParser(&parser, &lexer);
		Toy_Ast* ast = Toy_scanParser(&bucket, &parser);

		//parsing error, retry
		if (parser.error || ast == NULL) {
			Toy_freeBucket(&bucket);
			printf("%s> ", prompt); //shows the terminal prompt
			continue;
		}

		if (verbose) {
			inspect_ast(ast);
		}

		unsigned char* bytecode = Toy_compileToBytecode(ast);
		Toy_freeBucket(&bucket); //no need to for the GC here

		if (bytecode == NULL) {
			printf("%s> ", prompt);
			continue;
		}

		if (verbose) {
			inspect_bytecode(bytecode);
		}

		//WARN: Hacky debugging
		if (vm.scope == NULL) {
			Toy_bindVM(&vm, bytecode, NULL);
			initStandardLibrary(&vm);
		}
		else {
			Toy_bindVM(&vm, bytecode, NULL);
		}

		//run
		Toy_runVM(&vm);

		int depthBeforeGC = 0;
		int depthAfterGC = 0;

		//print the debug info
		if (verbose) {
			inspect_stack(vm.stack);
			inspect_scope(vm.scope, 0);

			depthBeforeGC = inspect_bucket(&vm.memoryBucket);
		}

		//free the memory, and leave the VM ready for the next loop
		Toy_resetVM(&vm, true, true);

		if (verbose) {
			depthAfterGC = inspect_bucket(&vm.memoryBucket);

			printf("GC Report: %d -> %d\n", depthBeforeGC, depthAfterGC);
		}

		//BUG: Toy references the bytecode to avoid excess copying, but this means freeing the bytecode each loop will break the repl
		//Therefore, the bytecode must persist for the duration of the program's runtime

		//free(bytecode);

		printf("%s> ", prompt); //shows the terminal prompt
	}

	//cleanup all memory
	Toy_freeVM(&vm);

	return 0;
}

//main file
int main(int argc, const char* argv[]) {
	//if there's args, process them
	Settings settings = parseSettings(argc, argv);
	if (settings.error) {
		usageInfo(argc, argv);
		return 1;
	}

	//setup the output options
	Toy_setPrintCallback(puts);
	Toy_setErrorCallback(errorAndExitCallback);
	Toy_setAssertFailureCallback(assertFailureAndExitCallback);
	if (settings.silentPrint) {
		Toy_setPrintCallback(noOpCallback);
	}
	if (settings.silentAssert) {
		Toy_setAssertFailureCallback(silentExitCallback);
	}

	//process
	if (settings.help) {
		helpInfo(argc, argv);
	}
	else if (settings.version) {
		versionInfo(argc, argv);
	}
	else if (settings.script != NULL) {
		//read the source file
		int size;
		unsigned char* source = readFile(settings.script, &size);

		//check the file
		if (source == NULL) {
			if (size == 0) {
				fprintf(stderr, TOY_CC_ERROR "ERROR: Could not parse an empty file '%s', exiting\n" TOY_CC_RESET, settings.script);
				return -1;
			}

			else if (size == -1) {
				fprintf(stderr, TOY_CC_ERROR "ERROR: File not found '%s', exiting\n" TOY_CC_RESET, settings.script);
				return -1;
			}

			else {
				fprintf(stderr, TOY_CC_ERROR "ERROR: Unknown error while reading file '%s', exiting\n" TOY_CC_RESET, settings.script);
				return -1;
			}
		}

		//compile the source code
		Toy_Lexer lexer;
		Toy_bindLexer(&lexer, (char*)source);

		Toy_Parser parser;
		Toy_bindParser(&parser, &lexer);

		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);
		Toy_Ast* ast = Toy_scanParser(&bucket, &parser);

		if (ast == NULL) {
			Toy_freeBucket(&bucket);
			free(source);
			return -1;
		}

		if (settings.verbose) {
			inspect_ast(ast);
		}

		unsigned char* bytecode = Toy_compileToBytecode(ast);
		Toy_freeBucket(&bucket);
		free(source);

		if (bytecode == NULL) {
			return -1;
		}

		if (settings.verbose) {
			inspect_bytecode(bytecode);
		}

		//run the compiled code
		Toy_VM vm;
		Toy_initVM(&vm);
		Toy_bindVM(&vm, bytecode, NULL);
		initStandardLibrary(&vm); //WARN: Hacky debugging

		Toy_runVM(&vm);

		//print the debug info
		if (settings.verbose) {
			inspect_stack(vm.stack);
			inspect_scope(vm.scope, 0);
		}

		//cleanup
		Toy_freeVM(&vm);
		free(bytecode);
	}
	else {
		repl(argv[0], settings.verbose);
	}

	return 0;
}
