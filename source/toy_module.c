#include "toy_module.h"
#include "toy_console_colors.h"

static inline unsigned int readUnsignedInt(unsigned char** handle) {
	unsigned int i = *((unsigned int*)(*handle));
	(*handle) += 4;
	return i;
}

Toy_Module Toy_parseModule(unsigned char* ptr) {
	if (ptr == NULL) {
		return (Toy_Module){ 0 };
	}

	Toy_Module module;

	module.scopePtr = NULL;

	module.code = ptr;

	//header
	readUnsignedInt(&ptr);

	module.jumpsCount = readUnsignedInt(&ptr);
	module.paramCount = readUnsignedInt(&ptr);
	module.dataCount = readUnsignedInt(&ptr);
	module.subsCount = readUnsignedInt(&ptr);

	module.codeAddr = readUnsignedInt(&ptr);
	if (module.jumpsCount) {
		module.jumpsAddr = readUnsignedInt(&ptr);
	}
	if (module.paramCount) {
		module.paramAddr = readUnsignedInt(&ptr);
	}
	if (module.dataCount) {
		module.dataAddr = readUnsignedInt(&ptr);
	}
	if (module.subsCount) {
		module.subsAddr = readUnsignedInt(&ptr);
	}

	return module;
}