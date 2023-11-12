#if !MAC // Mac implementation is in FBTablet.h

#ifndef _TABLET_H_
#define _TABLET_H_

#include "afxmt.h"
#include "wintab.h"


#define TABLET_MESSAGE 0x7FEF // less than wintab messages

#define MAX_BUF 64 

class CMyInkCollector;
class CMyInkEvents;
class CTablet
{
public:
	CTablet();
	~CTablet();
static BOOL HaveWinTab();
static void Unload();
/*
	returns 1 if opened using wintab
	2 if using ink collector
*/
	UINT Open(HWND hWnd, int res, BOOL bPrimary, BOOL bWintab);
	UINT Packet(WPARAM wSerial, LPARAM hCtx);
	void PrsInit(void);
//	UINT Pressure() { return m_pressure;};
//	UINT Cursor() {return m_cursor;};
	int	m_icount;
	int	m_ocount;
	POINT m_past[MAX_BUF];
	UINT  m_press[MAX_BUF];
	void ButtonDown(BOOL bDown,UINT minpress = 0, UINT maxpress = 0,
						int winx = 0,int winy = 0);
protected:
	HWND m_hWnd;
    CMyInkCollector * m_pmic;
	void Begin() { m_bFirst = 1;};
	UINT InkPacket(int x, int y, int pressure);
	UINT PrsAdjust(void * p);
	BOOL m_bDown;
	BOOL m_bFirst;
	UINT AddPoint(int x, int y, UINT pressure);
// tablet stuff
	UINT m_pressure;
	UINT m_minpress;
	UINT m_maxpress;
	UINT m_cursor;
	UINT m_act_cursor;
	CMutex *m_pWTMutex;
	HCTX hCtx;
	UINT m_wPrsBtn;
//	int m_ExtX;
//	int m_ExtY;
//	LOGCONTEXT lc;
	UINT wActiveCsr;
	UINT wOldCsr;
	BYTE wPrsBtn;
	UINT prsYesBtnOrg, prsYesBtnExt, prsNoBtnOrg, prsNoBtnExt;

	int m_factor;
	int m_xdpi;
	int m_ydpi;
	int  m_xbase;
	int  m_ybase;
friend class CMyInkEvents;
};


#endif

#endif
