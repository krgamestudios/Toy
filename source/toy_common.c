#include "toy_common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "toy_lexer.h"
#include "toy_parser.h"
#include "toy_compiler.h"

//util macros
#define MIN(a, b) ((a) < (b) ? (a) : (b))

//defined separately, as compilation can take several seconds, invalidating the comparisons of the given macros
static const char* build = __DATE__ " " __TIME__ ", v2-beta-1";

const char* Toy_private_versionBuild(void) {
	return build;
}

int Toy_private_getWorkingDir(char* dest, const char* src, size_t destLength) {
	char* p = NULL;

	//find the last slash, regardless of platform
	p = strrchr(src, '\\');
	if (p == NULL) {
		p = strrchr(src, '/');
	}
	if (p == NULL) {
		dest[0] = '\0';
		return 0;
	}

	p++; //skip the slash

	//determine length of the file path (including the slash)

	int len = MIN((size_t)(p - src), destLength);

	//copy to the dest
	strncpy(dest, src, len);
	dest[len] = '\0';

	return len;
}

int Toy_private_getFileName(char* dest, const char* src, size_t destLength) {
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
}

unsigned char* Toy_private_readFile(const char* path, int* size) {
	//open the file
	FILE* file = fopen(path, "rb");
	if (file == NULL) {
		*size = -1; //missing file error
		return NULL;
	}

	//determine the file's length
	fseek(file, 0L, SEEK_END);
	*size = ftell(file);
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

unsigned char* Toy_private_makeCodeFromSource(const char* source, const char* workingDir) {
	Toy_Lexer lexer;
	Toy_bindLexer(&lexer, source);

	Toy_Parser parser;
	Toy_bindParser(&parser, &lexer);
	Toy_adjustParserWorkingDirectory(&parser, workingDir);

	Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);

	Toy_Ast* ast = Toy_scanParser(&bucket, &parser);
	unsigned char* code = Toy_compileToBytecode(ast);

	Toy_freeBucket(&bucket);

	return code;
}
