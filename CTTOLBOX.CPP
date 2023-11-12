
// ctoolbox.cpp : implementation file
//
#include "stdafx.h"
#include "sketch.h"
#include "ctoolbox.h"
#include "dialogs.h"
#include "tooldlg.h"
#include "mydoc.h"
#include "myview.h"
#include "mainfrm.h"
#include "nscaler.h"

#define HOTCOLD
#define UNGRAY
//#define WRAPTOOL 14
//#define SHRINK


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define TIMER_ID 13
#define TIMER_DELAY 150
/////////////////////////////////////////////////////////////////////////////

ctoolbox::ctoolbox()
{
	DPF("toolbox construct");
	m_pDlg = 0;
	m_nSlots = 0;
	m_bReqDlg = 0;
	m_bTiming = 0;
	m_w = 0;
	m_id = NEGONE;
}

ctoolbox::~ctoolbox()
{
	DPF("toolbox destruct");
//	CloseDialog();
}

void ctoolbox::WeMoved(int x, int y)
{
DPF("tool box move, x:%d,y:%d",x,y);
	m_y = x;
	m_x = y;
	if (!m_pDlg)
		return;
	RECT rc;
	RECT rc1;
	GetWindowRect(&rc);
	int a = GetSystemMetrics(SM_CYSMCAPTION);
//	int ddx = GetSystemMetrics(SM_CXFIXEDFRAME);
//	int ddy = GetSystemMetrics(SM_CYFIXEDFRAME);
	int xx, yy;
	if (!m_pDlg->m_nDocked)
		{
		m_pDlg->GetWindowRect(&rc1);
		x = rc1.left;
		y = rc1.top;
		xx = rc.left;
		yy = rc.bottom;
		int dx = x - xx;
		int dy = y - yy;
		int dist = dx * dx + dy * dy;
//DPF("tool box move, xx:%d,yy:%d,dist:%d",xx,yy,dist);
		if (dist && (dist < 515))
			m_pDlg->m_nDocked = 1;
		}
	if (!m_pDlg->m_nDocked)
		{
		m_pDlg->GetWindowRect(&rc1);
		x = rc1.left;
		y = rc1.top;
		xx = rc.right;
		yy = rc.top - a - 2;
		int dx = x - xx;
		int dy = y - yy;
		int dist = dx * dx + dy * dy;
DPF("tool box move, xx:%d,yy:%d,dist:%d",xx,yy,dist);
		if (dist && (dist < 515))
			m_pDlg->m_nDocked = 2;
		}
	if (m_pDlg->m_nDocked == 1)
		m_pDlg->SetWindowPos(NULL, rc.left-2,
			rc.bottom+2, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
	else if (m_pDlg->m_nDocked == 2)
		m_pDlg->SetWindowPos(NULL,rc.right+2,
				rc.top-a-2, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
	
}


BEGIN_MESSAGE_MAP(ctoolbox, CToolBar)
	//{{AFX_MSG_MAP(ctoolbox)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ctoolbox message handlers

BOOL ctoolbox::ToolMessage(int msg)
{
	if (!m_pDlg)
		return 0;
	m_pDlg->PostMessage(WM_COMMAND,msg,0);
	return 1;
}

void ctoolbox::CloseDialog(BOOL bMe /* = 0 */)
{
	if (m_pDlg && !bMe)
		m_pDlg->DestroyWindow();
	m_pDlg = 0;
}

void ctoolbox::DrawSelect(int slot, int on)
{
	UINT idd,style;
	int image;
	UINT index = CommandToIndex(m_ids[slot]);
	GetButtonInfo(index,idd,style,image);
	DPF("idd:%d,style:%x,idx:%d",idd,style,image);
	if (on)
		image = slot + m_nSlots;
	else
		image = slot;
	SetButtonInfo(index,idd,style,image);
}

void ctoolbox::SetSlot(int bmid, int toolid, BOOL bDlg /* = 0 */)
{
	m_bmids[m_nSlots] = bmid;
	m_bdlgs[m_nSlots] = bDlg;
	m_ids[m_nSlots++] = toolid;
}

void ctoolbox::FocusFrame()
{
#ifndef FLIPBOOK_MAC
	m_pFrame->SetFocus();
#endif
}

void ctoolbox::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (m_id != -1)
		{
		m_pFrame->PostMessage(WM_COMMAND, m_ids[m_id], 0);
		}
	CToolBar::OnRButtonUp(nFlags, point);
}

BOOL ctoolbox::HasDialog(UINT id)
{
	return m_bdlgs[id];
}

void ctoolbox::SetupDialog (UINT inSlotIndex)
{
	m_id = inSlotIndex;
	m_bReqDlg = TRUE;
}

void ctoolbox::OnRButtonDown(UINT nFlags, CPoint point)
{
	UINT i = PointToSlot(point);
	if ((i < m_nSlots) && m_bdlgs[i])
		{
		m_id = i;
		m_bReqDlg = TRUE;
		}
	CToolBar::OnRButtonDown(nFlags, point);
}

void ctoolbox::DoDialog()
{
	m_bReqDlg = 0;
	UINT kind = 256 * m_bToneMatte + (m_tool & (15+128));
	if (!m_pDlg || (m_pDlg->m_kind != kind))
		{
		CloseDialog();
//		delete m_pDlg;
		m_pDlg = new CToolDlg;
		m_pDlg->m_hWnd = 0;
		m_pDlg->m_kind = kind;
		if (!m_pDlg->MyCreate(this,m_pDoc))
			TRACE("Failed to create tool dlg");
		RECT rc;
		GetWindowRect(&rc);
		int x = rc.left;
		int y = rc.top;
		int a = GetSystemMetrics(SM_CYCAPTION);
		int dx = GetSystemMetrics(SM_CXSIZEFRAME);
		int dy = GetSystemMetrics(SM_CYSIZEFRAME);
		m_pDlg->SetWindowPos(NULL, x-2,
				rc.bottom+2, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
		m_pDlg->m_nDocked = 1;
		m_pDlg->Invalidate();
		}
	FocusFrame();
}


int  ctoolbox::PointToSlot(CPoint point)
{
	TOOLINFO TI;
	int z = OnToolHitTest(point,& TI);
	int i;
	for (i = 0; i < (int)m_nSlots; i++)
		{
		if (z == (int)m_ids[i])
			break;
		}
	return i;
}

void ctoolbox::OnLButtonUp(UINT nFlags, CPoint point)
{
	DPF("Lbut up");
	m_ticks = 0;
	if (m_bDown)
		{
		m_bDown = 0;
//		OnRButtonUp(0,CPoint(0,0));
		}
	CToolBar::OnLButtonUp( nFlags,  point);
}

void ctoolbox::OnLButtonDown(UINT nFlags, CPoint point)
{
	int idd = PointToSlot(point);
	if (idd < (int)m_nSlots)
		{
		m_ticks = 4;
		}
	CToolBar::OnLButtonDown(nFlags, point);
}

void ctoolbox::OnTimer(UINT nTimerID)
{
	if (nTimerID != TIMER_ID)
		return;
	if (!(GetStyle() & WS_VISIBLE))
		{
		m_ticks = 0;
		CloseDialog();
		}
	else if (!IsFloating())
		{
	//	m_pDlg = 0;
	//	CloseDialog();
		}
	else
		{
		RECT rc;
		GetWindowRect(&rc);
		if (IsFloating() && m_pDlg)
			{
			if ((rc.top != m_y) || (rc.left != m_x))
				WeMoved(rc.top,rc.left);
				{
				m_y = rc.top;
				m_x = rc.left;
				}
			}
		}
	if (m_ticks)
		{
		m_ticks--;
		if (!m_ticks && !m_pDlg && HasDialog(m_id))
			{
//			m_bReqDlg = 1;
			DoDialog();
			}
		}
	CToolBar::OnTimer(nTimerID);

}

HBITMAP zMyMake(LPBITMAPINFOHEADER lpBitmap)//, BYTE * lpBits)
{
#ifdef FLIPBOOK_MAC
	// there's some graphics complexity here that is not supported, so we short-circuit on Mac
	FBBitmap* bm = [[FBBitmap alloc] initWithBytes:lpBitmap];
	HBITMAP hbm = new CBitmap (bm);
	[bm autorelease];
	return hbm;
#else
	int nWidth = (int)lpBitmap->biWidth;
	int nHeight = (int)lpBitmap->biHeight;
	HDC hDCScreen = ::GetDC(NULL);
	HBITMAP hbm = ::CreateCompatibleBitmap(hDCScreen, nWidth, nHeight);

	if (hbm != NULL)
		{
		HDC hDCGlyphs = ::CreateCompatibleDC(hDCScreen);
		HBITMAP hbmOld = (HBITMAP)::SelectObject(hDCGlyphs, hbm);

		LPBYTE lpBits;
		lpBits = (LPBYTE)(lpBitmap + 1);
//		lpBits += (1 << (lpBitmapInfo->biBitCount)) * sizeof(RGBQUAD);

		StretchDIBits(hDCGlyphs, 0, 0, nWidth, nHeight, 0, 0, nWidth, nHeight,
			lpBits, (LPBITMAPINFO)lpBitmap, DIB_RGB_COLORS, SRCCOPY);
		SelectObject(hDCGlyphs, hbmOld);
		::DeleteDC(hDCGlyphs);
		}
	::ReleaseDC(NULL, hDCScreen);

	return hbm;
#endif
}


BOOL ctoolbox::MakeImageList(CImageList & images, UINT offset)
{
	UINT c, p;
	UINT sw = 48;
	UINT sh = 48;
	p = 4 * ((2 * 3 * m_w * m_nSlots + 3) / 4);
	BYTE * pBMP = new BYTE[40+m_h *p]; 
	UINT sp = 4 * ((4*3*sw+3)/ 4);
	BYTE * pTemp = new BYTE[sh*sp]; 
	BYTE * pData = pBMP;
	LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)pData;
	lpBI->biSize			= sizeof(BITMAPINFOHEADER);
	lpBI->biWidth			= 2 * m_w * m_nSlots;// twice selected
	lpBI->biHeight			= m_h;
	lpBI->biPlanes			= 1;
	lpBI->biBitCount		= 24;
	lpBI->biCompression		= BI_RGB;
	lpBI->biSizeImage		= p * m_h;
	lpBI->biXPelsPerMeter	= 0;
	lpBI->biYPelsPerMeter	= 0;
	lpBI->biClrUsed			= 0;
	lpBI->biClrImportant	= 0;
	pData += lpBI->biSize;
 	const COLORREF		buttonColor (::GetSysColor (COLOR_BTNFACE));
	UINT rr = GetRValue (buttonColor);
	UINT gg = GetGValue (buttonColor);
	UINT bb = GetBValue (buttonColor);
	for (c = 0; c < m_nSlots; c++, pData += 3*m_w)
		{
//		DPF("c:%d",c);
		UINT idd;
		idd = m_bmids[c];
		HRSRC hRes = FindResource(AfxGetApp()->m_hInstance,
					MAKEINTRESOURCE(idd),RT_BITMAP);
//		LASTERR;
		if (!hRes)
			continue;
		HGLOBAL hand = LoadResource(NULL , hRes);    
//		LASTERR;
		if (!hand)
			continue;
		BYTE * pSrc = (BYTE *) LockResource(hand);       
		if (!pSrc)
			{
			continue;
			}
		LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)pSrc;
		UINT ww = lpBI->biWidth;
		UINT hh = lpBI->biHeight;
		UINT dd = lpBI->biBitCount;
//		DPF("c:%d,src %d,%d,%d", c,ww,hh,dd);
		if (ww != sw * 4)
			continue;
		if (hh != sh)
			continue;
		if (dd != 24)
			continue;
		pSrc +=  40;	// to bitmap data
		memmove(pTemp, pSrc, sh * sp);
		UnlockResource(hand);
//	pData is dest ptr with pitch of p
		BYTE * pDst = pData;
		for (int zz = 0; zz < 2; zz++, pDst += m_nSlots * 3 * m_w)
			{
			UINT x,y;
			BYTE * pSrc;
			if (!zz)
				pSrc = pTemp + 3 * sw * offset; // to specified image
			else
				pSrc = pTemp + 3 * 3 * sw;		// to selected image
			for (y = 0; y < sh; y++)
			for (x = 0; x < sw; x++)
				{
				if ((pSrc[sp*y+3*x+0] == 200) && 
						(pSrc[sp*y+3*x+1] == 200) &&
						(pSrc[sp*y+3*x+2] == 200))
					{
					pSrc[sp*y+3*x+0] = bb;
					pSrc[sp*y+3*x+1] = gg;
					pSrc[sp*y+3*x+2] = rr;
					}
			}
			CGScaler scale;
			if (scale.Init(ww/4,hh,24,m_w,m_h))
				{
DPF("scale failure");
				break;
				}
			int q = scale.Custom(pDst, pSrc,p,sp);
DPF("after custom:%d",q);
			int z = scale.Copy();
			}
		}
//
//	here we should have correct bitmap
//
//
//	here we should have correct bitmap
//
	CBitmap bm;
	HBITMAP hBitmap = zMyMake(lpBI);
	bm.Attach(hBitmap);
	delete [] pBMP;
	delete [] pTemp;

 //ReplaceBackgroundColor (bm);

	VERIFY (images.Create (m_w,m_h,ILC_COLOR24,2*m_nSlots,0));
	VERIFY (images.Add (&bm, RGB (0, 0, 0)) != -1);
	bm.Detach();
	return 0;
}


BOOL ctoolbox::MakeToolBar()
{
	DPF("Making toolbar");
	m_nSlots = 0;
	if (m_bToneMatte)
		SetSlot(IDB_MAT_FILL_TOOL, ID_TOOL_PENCIL,1);
	else
		SetSlot(IDB_PENCILTOOL, ID_TOOL_PENCIL,1);
		SetSlot(IDB_FILLTOOL, ID_TOOL_FILL,1);
	if (m_bToneMatte)
		SetSlot(IDB_MAT_BRSH_TOOL, ID_TOOL_TRACE,1);
	else	
		SetSlot(IDB_TRACETOOL, ID_TOOL_TRACE,1);
	SetSlot(IDB_BRUSHTOOL, ID_TOOL_BRUSH,1);
	SetSlot(IDB_ERASERTOOL, ID_TOOL_ERASER);
	SetSlot(IDB_EYEDROPTOOL, ID_TOOL_EYEDROP);
	SetSlot(IDB_ZOOMTOOL, ID_TOOL_ZOOM);
	SetSlot(IDB_HANDTOOL, ID_TOOL_HAND);
	SetSlot(IDB_SELECTTOOL, ID_TOOL_SELECT,1);
	UINT ww = m_w * m_nSlots;
	SetSizes(CSize(m_w+7,m_h+6),CSize(m_w,m_h));
	CImageList images;
 	CToolBarCtrl&	barCtrl = GetToolBarCtrl();
	MakeImageList(images,0);
	barCtrl.SetImageList (&images);
	images.Detach();

	MakeImageList(images,1);
	barCtrl.SetHotImageList (&images);
	images.Detach();

	MakeImageList(images,2);
	barCtrl.SetDisabledImageList (&images);
	images.Detach();

	if (!SetButtons( m_ids,m_nSlots))
		{
		TRACE("Failed to create toolbar\n");
		return -1;      // fail to create
		}
#ifdef HOTCOLD
	ModifyStyle(0, TBSTYLE_FLAT |  TBSTYLE_TRANSPARENT );
//#else
//	m_pToolBar->ModifyStyleEx(0, TBSTYLE_WRAPABLE | TBSTYLE_FLAT);
#endif
//	m_pToolBar->GetItemRect(LAY_BUTTON, &rect);

DPF("a");
	RECT rc;
	GetWindowRect(&rc);
	m_x = rc.left;
	m_y = rc.top;
	return 0;
}

int  ctoolbox::ToolToSlot(int tool)
{
	int i;
//	if (m_bCamera)
//		return tool;
	tool += ID_TOOL_PENCIL;
/*
	if (m_rows == 2)
		i = 7;
	else
		i = 4;
*/
	i = m_nSlots-1;
	for (; i && ((UINT)tool != m_ids[i]); i--);
//DPF("tool to slot,tool:%d,slot:%d",tool-ID_TOOL_PENCIL, i);
	return i;
}

void ctoolbox::ChangeTool()
{
	UINT tool;
	tool = m_pDoc->SelectTool();
	if (m_pDoc->ToolInfo(3) && m_pDoc->ToolInfo(0))
		tool |= 128;
//	DPF("toolbox change tool:%d,%d",tool,m_tool);
	DrawSelect(ToolToSlot(ID_TOOL_ERASER-ID_TOOL_PENCIL),tool & 128);
	DrawSelect(ToolToSlot(m_tool & 127),0);
	DrawSelect(ToolToSlot(tool & 127),1);
	m_tool = tool;
	m_id = ToolToSlot(m_tool & 127);
	if (!HasDialog(m_id))
		CloseDialog();
	else if (m_bReqDlg || m_pDlg)
		DoDialog();

#ifdef FLIPBOOK_MAC
	Invalidate();
#endif
}


BOOL ctoolbox::Setup(CMainFrame * pFrame,UINT w, UINT h,
			CSketchDoc * pDoc, BOOL bToneMatte, BOOL bForce)
{
	SetWindowText("Tools");
	m_pFrame = pFrame;
	m_pDoc = pDoc;
	BOOL bColor = pDoc ? m_pDoc->m_pScene->ColorMode() : 0;
	if ((bColor != m_bColor) || (m_bToneMatte != bToneMatte) ||
			(m_w != w) || (m_h != h))
		{
		m_w = w;
		m_h = h;
		m_bToneMatte = bToneMatte;
		m_bColor = bColor;
		if (MakeToolBar())
			return 1;
		}
	VERIFY(SetTimer(TIMER_ID, TIMER_DELAY, NULL) == TIMER_ID);
	m_bTiming = 1;
	if (bForce)
		{
		BOOL bHaveDlg = m_pDlg ? 1 : 0;
		if (bHaveDlg)
			{
			m_pDlg->DestroyWindow();
			m_pDlg = 0;
			DoDialog();
			}
		}
	FocusFrame();
	return 0;
}

