#include "CommonDefs.h"

#include <cstdio>
#include <cstdarg>

#ifdef _WIN32
//#include <windows.h>
#elif __APPLE__
//#include <Foundation/Foundation.h>
#endif

void FBMessageBox(const char* message)
{
}

void DebugPrintFormatted(const char* format, ...)
{
#ifndef NDEBUG
    char* s = new char[1024]; // hope this is enough space
    va_list list;
    va_start(list, format);
    vsprintf(s, format, list);
    va_end(list);

#ifdef _WIN32
    //OutputDebugString(s);
#elif __APPLE__
    //NSLog("%s", s);
#endif

    delete[] s;
#endif  
}
