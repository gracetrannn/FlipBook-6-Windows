#pragma once

#include <cstdint>
#include <cassert>
#include <cstddef>

// TODO: change to ifndef
#ifndef _WIN32
	#define IMPLEMENT_WINDOWS_APIS
#endif

#ifdef IMPLEMENT_WINDOWS_APIS
	#include "WindowsTypes.h"
	#include "CFile.h"
	#include "CString.h"
	#include "CArchive.h"
	#include "CTime.h"
	#include "geometry.h"
#else
	#include <afxwin.h>
	//#include <windows.h>
#endif

#ifndef __STDAFX_H
	#ifndef ASSERT
		//#define ASSERT assert
        #define ASSERT
	#endif

	#define NEGONE 0xffffffff
	#define NEGTWO 0xfffffffe

	#define ZERO_CLASS memset(this,0,sizeof(*this))

	typedef	unsigned char* HPBYTE;

	void FBMessageBox(const char* message);
	void DebugPrintFormatted(const char* format, ...);

	#define DPF		DebugPrintFormatted
	#define DPF2	DebugPrintFormatted
	#define DPZ		DebugPrintFormatted
	#define dpf		DebugPrintFormatted
    #define DPL     DebugPrintFormatted
    #define dpl     DebugPrintFormatted
    #define dpc     DebugPrintFormatted

	#define SWAPV(v) v
	#define SWAPDBL(v) v
	#define DOSWAP(a)
	#define DOSWAPC(a,b)
	#define DOSWAPX(a,b,c,d)

	#ifdef _WIN32
		#define NATIVE_SEP_CHAR		'\\'
		#define NATIVE_SEP_STRING	"\\"
	#else
		#define NATIVE_SEP_CHAR		'/'
		#define NATIVE_SEP_STRING	"/"
	#endif

    #define NATIVE_SEP_CHAR_WIN32       '\\'
    #define NATIVE_SEP_CHAR_NWIN32      '/'
#endif

#ifndef _WIN32
#define ATTRIBUTE_PACKED    __attribute__((packed))
#else
#define ATTRIBUTE_PACKED
#endif
