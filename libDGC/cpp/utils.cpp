#include "utils.h"
#include <filesystem>

uint32_t FBGetTempPath(uint32_t nBufferLength, char* lpBuffer) {
    
#ifdef _WIN32
	try {
		auto path = std::filesystem::temp_directory_path().string();
		auto length = path.length();
		if (nBufferLength >= length) {
			strcpy(lpBuffer, path.c_str());
		}
		return length;
	}
	catch (...)
	{
		return 0;
	}
#else
    char* tmp = std::getenv("TMPDIR");
    strcpy(lpBuffer, tmp);
    return (uint32_t)strlen(tmp);
#endif
}

void* MyAlloc(size_t size)
{
	void* ptr = (void*)(new char[size]);
	memset(ptr, 0, size);
	return ptr;
}

void MyFree(void* ptr)
{
	delete[] ptr;
}

#ifdef IMPLEMENT_WINDOWS_APIS

int MulDiv(int number, int numerator, int denominator)
{
	bool negative = (number ^ numerator ^ denominator) < 0;
	int64_t mul = number;
	mul *= numerator;
	mul += negative ? -denominator / 2 : denominator / 2;
	return mul / denominator;
}

#endif

#ifndef _WIN32

#include <strings.h>

int _stricmp(const char* str1, const char* str2) {
    return strcasecmp(str1, str2);
}

#endif
