#include "stdafx.h"
#include "mymci.h"
#include "vfw.h"

CMyMCI::CMyMCI()
{
	m_bOpened = 0;
	m_Video = 0;
}

CMyMCI::~CMyMCI()
{
	Close();
}



int CMyMCI::Create(HWND hWnd, LPCSTR name)
{
	ASSERT(m_Video == NULL);
	MCIWndRegisterClass();
	if(m_Video == NULL)
		{
		m_Video = MCIWndCreate(hWnd, AfxGetInstanceHandle(),
				MCIWNDF_NOTIFYPOS |
//				MCIWNDF_NOERRORDLG |
//		MCIWNDF_NOAUTOSIZEWINDOW |
				MCIWNDF_NOAUTOSIZEMOVIE |
				MCIWNDF_NOOPEN|MCIWNDF_NOPLAYBAR |MCIWNDF_NOMENU,
				name);
		
		}
	return m_Video == NULL ? 1 : 0;
}

int CMyMCI::Open(LPCSTR name)
{
	ASSERT(m_bOpened == 0);
	UINT f = 0;//MCIWNDF_NOTIFYPOS;
	int res = MCIWndOpen(m_Video,name,f);
	if (!res)
		{
		m_bOpened = 1;
		MCIWndUseFrames(m_Video);
		}
	return res ? 1 : 0;
}

int CMyMCI::SetSize(int width, int height)
{
	RECT rcDest;
	int x,y,w, h;
	if (width * 3 > 4 * height)
		{
		h = height;
		w = 4 * height / 3;
		x = (width - w) / 2;
		y = 0;
		}
	else
		{
		w = width;
		h =  3 * width / 4;
		x = 0;
		y = (height - h) / 2;
		}
	rcDest.left = x;
	rcDest.top = y;
	rcDest.right = x + w;//640;//width;
	rcDest.bottom = y + h;//480;//height;
	int v = MCIWndPutDest(m_Video, &rcDest);
	::SetWindowPos(m_Video,           // window to resize 
               NULL,                          // z-order: don't care 
               x,y,w,h,                          // position: don't care
              // width,//rcDest.right - rcDest.left,      // width 
			  // height,//rcDest.bottom - rcDest.top, 
               SWP_NOZORDER | SWP_NOACTIVATE); 
	return 0;
}

int CMyMCI::Close()
{
	m_bOpened = 0;
	if (m_Video != NULL)
		return MCIWndClose(m_Video);
	else
		return 1;
}

int CMyMCI::Destroy()
{
	if(m_Video != NULL)
		{
		MCIWndStop(m_Video);
		MCIWndClose(m_Video);
		MCIWndDestroy(m_Video);
		m_Video = 0;
		}
	return 0;
}

int CMyMCI::Play()	{ return MCIWndPlay(m_Video);}

int CMyMCI::Pause()	{ return MCIWndPause(m_Video);}

int CMyMCI::Resume(){return MCIWndResume(m_Video);}

int CMyMCI::Repeat(BOOL bLoop)
{
//	::SendMessage(m_Video, MCIWNDM_SETREPEAT, 0, (LPARAM)bLoop);
//	::SendMessage(m_Video, WM_COMMAND, MAKELONG(2054, 0),0); 
//	MCIWndSetRepeat(m_Video, bLoop);
	MCIWndSetRepeat(m_Video, TRUE);
	::SendMessage(m_Video, WM_COMMAND, MAKELONG(2054, 0), 0);

	return 0;
}

int CMyMCI::Stop()	{ return MCIWndStop(m_Video);}

int CMyMCI::Seek(UINT pos){ MCIWndSeek(m_Video,pos);return 0;}

UINT CMyMCI::Where() { return MCIWndGetPosition(m_Video);}

UINT CMyMCI::Length()	// gets pnbr of frames
{
	return MCIWndGetEnd(m_Video);
}

int CMyMCI::CanPlay()
{
	RECT rcSrc;
	int bCanPlay = 1;
	int v = MCIWndGetDest(m_Video, &rcSrc);
	if (v || (rcSrc.right == rcSrc.left))  // no file
		{
		bCanPlay = 0;
		}
	return  bCanPlay;
}



