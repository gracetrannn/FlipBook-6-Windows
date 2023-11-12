// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//
#ifndef __STDAFX_H
#define __STDAFX_H
#if _MSC_VER >= 1400
	#define _VC2005
	#define _CRT_SECURE_NO_WARNINGS
	#ifndef WINVER
		#define WINVER 0x503
	#endif
#else
	#define strcpy_s(a,b,c) strcpy((a),(c))
	#define sprintf_s snprintf
#endif
#include "myver.h"
#define FBTPC
#define WINDOWS_SEP_CHAR '\\'
#define WINDOWS_SEP_STRING "\\"
#define MAC_SEP_CHAR '/'
#define MAC_SEP_STRING "/"
#if MAC
#define NATIVE_SEP_CHAR MAC_SEP_CHAR
#define NATIVE_SEP_STRING MAC_SEP_STRING
#else
#define NATIVE_SEP_CHAR WINDOWS_SEP_CHAR
#define NATIVE_SEP_STRING WINDOWS_SEP_STRING
#endif

#ifdef _XCODE
//#define _HASH_NO_WINDOWS_INCLUDES
//#define FBMAC
//typedef	int BIGINT;
//#include <HashDefines.h>
//#define DOUBLE double
//#define _NEEDSWAP
#else
typedef	_int64  BIGINT;
#endif

//#ifndef _OSX
//#define _OSX
//#endif

#ifdef _DEBUG
#define DEBUG
#define DOMYBUG
#endif


//#define DOCAMTOOL
//#define DOMYBUG // to enable message in release mode

int MyError(int id, int flags = 0);//MB_OK);
int MyError(int id, const char * txt, int flags = 0);//MB_OK);
#ifdef _NEEDSWAP
	#define SWAPV(v) SwapV(v)
	#define SWAPDBL(v) SwapDbl(v)
	#define DOSWAP(a) SwapEm(&a, sizeof(a))
	#define DOSWAPC(a,b) SwapEm(a,b)
	#define DOSWAPX(a,b,c,d) SwapEm(a,b);SwapEm(a+b+c,(d - b - c))
	void SwapEm(void * pData, unsigned int bytes);
	unsigned int SwapV(unsigned int v);
	double SwapDbl(double v);
#else
	#define SWAPV(v) v
	#define SWAPDBL(v) v
	#define DOSWAP(a) /* */
	#define DOSWAPC(a,b) /* */
	#define DOSWAPX(a,b,c,d) /* */
#endif

#ifdef DOMYBUG
	#define NEEDDPF
	#define DPZ dpf
	#ifndef NOBUG
		#define MYBUG
	#endif
#endif
#include "resource.h"
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxcview.h>
//#include <afxdb.h>			// MFC ODBC database classes
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#ifdef _XCODE
#include <afxpriv.h>
#else
#include <atlbase.h>
extern CComModule _Module;
#endif

#define NEGONE 0xffffffff
#define NEGTWO 0xfffffffe

typedef	unsigned char *HPBYTE;
//typedef	unsigned short *HPWORD;

#ifndef HDIB
DECLARE_HANDLE(HDIB);
#endif
#define PLUS
#define PRINTING
#define FARCDECL _cdecl
#ifdef MYBUG
	#define NEEDDPF
	#define LASTERR LastError()
	#define DPF dpf
	#define DPF2 /##/
	#define DPR dpr
	#define DPZ dpf
	void dpr(LPSTR sztext, LPRECT lpRect);
	void LastError();
#else
	#ifdef _OSX
	#define LASTERR LastError()
	#define DPF dpf
	#define DPF2 dpf
	#define DPR dpr
	void inline dpr(LPCSTR txt, LPRECT lpRect) {};
	void inline LastError() {};
	#else
	#define LASTERR /##/
	#define DPR /##/
	#define DPF /##/
	#define DPF2 /##/
	#endif
#endif

#ifdef NEEDDPF
	extern "C" void FARCDECL dpf(LPSTR szFormat, ...);
#else
	#ifdef _OSX
	#define DPZ dpf
	void inline dpf(const void * txt) {};
	void inline dpf(const void * txt, UINT a) {};
	void inline dpf(const void * txt, UINT a, UINT b) {};
	void inline dpf(const void * txt, UINT a, UINT b, UINT c) {};
	void inline dpf(const void * txt, UINT a, UINT b, UINT c, UINT d) {};
//	void inline dpf(const void * txt, UINT a, UINT b, UINT c, const void * d) {};
	void inline dpf(const void * txt, const void * a) {};
	void inline dpf(const void * txt, UINT a, const void * b) {};
	void inline dpf(const void * txt, const void * a, const void * b) {};
	void inline dpf(const void * txt, const void * a, UINT b) {};
	void inline dpf(const void * txt, UINT a, UINT b, UINT c, UINT d,
					UINT e,const void * z) {};
	void inline dpf(const void * txt, UINT a, UINT b, UINT c, UINT d,
					UINT e,UINT f) {};
	void inline dpf(const void * txt,UINT a, UINT b, UINT c, UINT d, UINT e) {};
	#else
	#define DPZ /##/
	#endif
#endif

	int OEMMessageBox(UINT idd, UINT nType = 0);
#ifdef DO_OEM
	#define OEM_TEXT OEM_Text
	#define OEM_DLG OEM_Dlg
	#define OEM_BUF OEM_Buf
	void OEM_Text(CWnd * pWnd);
	void OEM_Dlg(CWnd * pWnd, int iid);
	BOOL OEM_Buf(LPSTR buf);
#else
	#ifdef _OSX
		#define OEM_TEXT OEM_Text
		#define OEM_DLG OEM_Dlg
		void inline OEM_Text(CWnd * pWnd){};
		void inline OEM_Dlg(CWnd * pWnd, int iid) {};
		void inline OEM_Buf(char * pBuf) {};
		#define OEM_BUF OEM_Buf
	#else
		#define OEM_TEXT /##/
		#define OEM_DLG /##/
		#define OEM_BUF /##/
	#endif
#endif

#endif
