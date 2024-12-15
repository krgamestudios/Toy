#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//utilities
#define APPEND(dest, src) \
	strncpy((dest) + (strlen(dest)), (src), strlen((src)) + 1);

#if defined(_WIN32) || defined(_WIN64)
	#define FLIPSLASH(str) for (int i = 0; str[i]; i++) str[i] = str[i] == '/' ? '\\' : str[i];
#else
	#define FLIPSLASH(str) for (int i = 0; str[i]; i++) str[i] = str[i] == '\\' ? '/' : str[i];
#endif

unsigned char* readFile(char* path, int* size) {
	//BUGFIX: fix the path based on platform - it might be slower, but it's better than dealing with platform crap
	int pathLength = strlen(path);
	char realPath[pathLength + 1];
	strncpy(realPath, path, pathLength);
	realPath[pathLength] = '\0';
	FLIPSLASH(realPath);

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
		*size = -2; //singal a read error
		return NULL;
	}

	buffer[(*size)++] = '\0';

	//clean up and return
	fclose(file);
	return buffer;
}

int getFilePath(char* dest, const char* src) {
	char* p = NULL;

	//find the last slash, regardless of platform
	p = strrchr(src, '\\');
	if (p == NULL) {
		p = strrchr(src, '/');
	}
	if (p == NULL) {
		int len = strlen(src);
		strncpy(dest, src, len);
		return len;
	}

	//determine length of the path
	int len = p - src + 1;

	//copy to the dest
	strncpy(dest, src, len);
	dest[len] = '\0';

	return len;
}

int getFileName(char* dest, const char* src) {
	char* p = NULL;

	//find the last slash, regardless of platform
	p = strrchr(src, '\\');
	if (p == NULL) {
		p = strrchr(src, '/');
	}
	if (p == NULL) {
		int len = strlen(src);
		strncpy(dest, src, len);
		return len;
	}

	p++; //skip the slash

	//determine length of the file name
	int len = strlen(p);

	//copy to the dest
	strncpy(dest, p, len);
	dest[len] = '\0';

	return len;
}

int main(void) {
	//check the platform
	printf("Platform: ");
#if defined(__linux__)
	printf("Linux");
#elif defined(_WIN64)
	printf("Win64");
#elif defined(_WIN32)
	printf("Win32");
#elif defined(__APPLE__)
	printf("macOS");
#else
	printf("Unknown");
#endif

	printf("\n");

	//run each test
    {
        char src[256] = "../folder/file.txt";
        char dest[256];
        getFilePath(dest, src);
        printf("Path: %s\n", dest);
    }

    {
        char src[256] = "../folder/file.txt";
        char dest[256];
        getFileName(dest, src);
        printf("Name: %s\n", dest);
    }

    {
        char src[256] = "../folder/file.txt";
        char dest[256];
        getFilePath(dest, src);
        APPEND(dest, "target.txt");
        printf("Target: %s\n", dest);
    }

    return 0;
}
