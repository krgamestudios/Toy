#include "toy_common.h"

#include <string.h>

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