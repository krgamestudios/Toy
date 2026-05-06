#include "toy_common.h"

//defined separately, as compilation can take several seconds, invalidating the comparisons of the given macros
static const char* build = __DATE__ " " __TIME__ ", incomplete Toy v2.x";

const char* Toy_private_versionBuild(void) {
	return build;
}
