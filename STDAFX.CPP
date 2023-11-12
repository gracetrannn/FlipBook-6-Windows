// stdafx.cpp : source file that includes just the standard includes
//	test2.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include "afxpriv.h"
#ifdef _QVC2005
#pragma comment(linker,"/manifestdependency:\"type='win32' \
   name='Microsoft.Windows.Common-Controls' \
	version='6.0.0.0' \
	processorArchitecture='x86' \
	publicKeyToken='6595b64144ccf1df' \
	language='*'\"")
#endif
#define COMPILE_MULTIMON_STUBS

#include "MULTIMON.H"

int MyError(int id, int flags /* = 0 */)
{
	return AfxMessageBox(id, flags);
}

int MyError(int id, LPCSTR txt, int flags /* = MBOK */)
{
	CString prompt;
	AfxFormatString1(prompt, id, txt);
	return AfxMessageBox(prompt, flags);//MB_YESNOCANCEL, AFX_IDP_ASK_TO_SAVE))
}

int OEMMessageBox(UINT idd, UINT nType)
{
	#ifdef DO_OEM
		char buf[300];
		AfxLoadString(idd, buf,299);
		OEM_Buf(buf);
		return AfxMessageBox(buf, nType);
	#else
		return AfxMessageBox(idd, nType);
	#endif
}

#ifdef DO_OEM


BOOL OEM_Replace(LPSTR buf, LPCSTR str1, LPCSTR str2)
{
	int i, j;
	for (i = 0; buf[i]; i++)
		{
		for (j = 0; str1[j] && (str1[j] == buf[i+j]);j++);
		if (!str1[j])
			break;
		}
	if (!buf[i])
		return 0; // not found
	int c1 = strlen(str1);
	int c2 = strlen(str2);
	if (c1 > c2)
		{			// new string smaller, move left
		for (j = i+c1; buf[j]; j++)
			buf[j-c1+c2] = buf[j];
		buf[j-c1+c2] = 0;
		}
	else if (c1 < c2)
		{
		int c3 = strlen(buf);
		for (j = c3+c2-c1;j > i+c1 ; j--)
			buf[j] = buf[j+c1-c2];
		}
	for (j = 0; j < c2; j++)
		buf[i+j] = str2[j];
	return TRUE;
}

BOOL OEM_Buf(LPSTR buf)
{
	BOOL v = 0;
	v |= OEM_Replace(buf, "DigiCel ", OEM_COMPANY);
	v |= OEM_Replace(buf, "FlipBook", DO_OEM);
ASSERT(v != 0);
	return v;
}

void OEM_Text(CWnd * pWnd)
{
	char buf[300];
	pWnd->GetWindowText(buf,299);
	if (OEM_Buf(buf))
		pWnd->SetWindowText(buf);
}

void OEM_Dlg(CWnd * pWnd, int iid)
{
	OEM_Text(pWnd->GetDlgItem(iid));
}

#endif
#ifdef MYBUG

void LastError()
{
LPVOID lpMsgBuf;
FormatMessage( 
    FORMAT_MESSAGE_ALLOCATE_BUFFER | 
    FORMAT_MESSAGE_FROM_SYSTEM | 
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    GetLastError(),
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
    (LPTSTR) &lpMsgBuf,
    0,
    NULL 
);
// Process any inserts in lpMsgBuf.
// ...
// Display the string.
	DPF("last error:%s",(LPCSTR)lpMsgBuf);
//MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
// Free the buffer.
LocalFree( lpMsgBuf );
}


static HWND hwnd = 0;
static int dpflag = -1;
static int counter = 1;

void dpr(LPSTR sztext, LPRECT lpR)
{
	dpf("%s,%d,%d,%d,%d",sztext,
			lpR->left,lpR->top,lpR->right-lpR->left,lpR->bottom-lpR->top);
}

void FARCDECL dpf(LPSTR szFormat, ...)
{
	char amsg[280];
	if (dpflag <= 0)
		{
		char szDBWinClass[] = "DBWin";
        hwnd = FindWindow(szDBWinClass, NULL);
		dpflag = 1;
//		if (hwnd)
//			counter = 1;
		}
	if (szFormat[0] == '$' && hwnd)
		{
		if (szFormat[1] == '+' )
			counter++;
		else if (szFormat[1] == '1')
			{
			counter = 1;
			return;
			}
		else if (szFormat[1] == '0')
			{
			counter = 0;
			return;
			}
		}
	if (!counter)
		return;
	wvsprintf(amsg, szFormat,(LPSTR)(&szFormat+1));
	if (!strlen(amsg) || (amsg[strlen(amsg)-1] != ':'))
		strcat(amsg, "\n");
#ifdef _DEBUG
//	OutputDebugString(amsg);
	TRACE(amsg);
#else
	#ifdef _OSX
	printf(amsg);
	#else
	OutputDebugString(amsg);
//	PostMessage(hwnd, WM_COMMAND, 1400, GlobalAddAtom(amsg));
	#endif
	if (szFormat[0] == '$')
		{
		if (szFormat[1] == '-')
			counter--;
		else if (szFormat[1] == '0')
			counter = 0;
		return;
		}
#endif
}
#endif

#ifdef _NEEDSWAP
UINT SwapV(UINT v)
{
	return (v << 24) + ((v & 0xff00) << 8) + ((v >> 8)& 0xff00) + (v >> 24);
}

void SwapEm(void * pp, UINT c)
{
	BYTE *p = (BYTE *)pp;
	BYTE t;
	for (;c>3;c-=4,p+=4)
		{
		t = p[0];
		p[0] = p[3];
		p[3] = t;
		t = p[1];
		p[1] = p[2];
		p[2] = t;
		}
	if( c > 1)
		{
		t = p[0];
		p[0] = p[1];
		p[1] = t;
		}
}

double SwapDbl(double v)
{
	double t = v;
	BYTE *p = (BYTE *)&t;
	int i;
	for (i = 0; i < 4; i++)
		{
		BYTE q = p[i];
		p[i] = p[7-i];
		p[7-i] = q;
		}
	return t;
}

void SwapEm(BYTE * pDst, BYTE * pSrc, UINT c)
{
	for (;c>3;c-=4,pSrc+=4)
		{
		*pDst++ = ((BYTE *)pSrc)[3];
		*pDst++ = ((BYTE *)pSrc)[2];
		*pDst++ = ((BYTE *)pSrc)[1];
		*pDst++ = ((BYTE *)pSrc)[0];
		}
}
#endif
