#pragma once

//for specified type sizes
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

//TOY_API is platform-dependant, and marks publicly usable API functions
#if defined(__linux__)
	#define TOY_API extern
#elif defined(_WIN32) || defined(_WIN64)
	#if defined(TOY_EXPORT)
		#define TOY_API __declspec(dllexport)
	#elif defined(TOY_IMPORT)
		#define TOY_API __declspec(dllimport)
	#else
		#define TOY_API extern
	#endif
#elif defined(__APPLE__)
	#define TOY_API extern
#else
	//generic solution
	#define TOY_API extern
#endif

//TOY_BITNESS is used to encourage memory-cache friendliness
#if defined(__linux__)
	#if defined(__LP64__)
		#define TOY_BITNESS 64
	#else
		#define TOY_BITNESS 32
	#endif
#elif defined(_WIN32) || defined(_WIN64)
	#if defined(_WIN64)
		#define TOY_BITNESS 64
	#else
		#define TOY_BITNESS 32
	#endif
#elif defined(__APPLE__)
	#if defined(__LP64__)
		#define TOY_BITNESS 64
	#else
		#define TOY_BITNESS 32
	#endif
#else
	//generic solution
	#define TOY_BITNESS -1
#endif

//version specifiers, embedded as the header
#define TOY_VERSION_MAJOR 2
#define TOY_VERSION_MINOR 0
#define TOY_VERSION_PATCH 0

//defined as a function, for technical reasons
#define TOY_VERSION_BUILD Toy_private_version_build()
TOY_API const char* Toy_private_version_build(void);

/*

Version validation rules:

 * Under no circumstance, should you ever run code whose major version is different from the interpreter’s major version
 * Under no circumstance, should you ever run code whose minor version is above the interpreter’s minor version
 * You may, at your own risk, attempt to run code whose patch version is different from the interpreter’s patch version
 * You may, at your own risk, attempt to run code whose build version is different from the interpreter’s build version

*/
