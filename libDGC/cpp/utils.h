#pragma once

#include "CommonDefs.h"
#include <cstdint>

uint32_t FBGetTempPath(uint32_t nBufferLength, char* lpBuffer);
void* MyAlloc(size_t size);
void MyFree(void* ptr);

#ifdef IMPLEMENT_WINDOWS_APIS

int MulDiv(int number, int numerator, int denominator);

#endif

#ifndef _WIN32
int _stricmp(const char* str1, const char* str2);
#endif
