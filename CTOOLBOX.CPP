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
	m_nEraserId = 0;
	m_w = 0;
	m_id = NEGONE;
	m_dlgx = -1;
}

ctoolbox::~ctoolbox()
{
	DPF("toolbox destruct");
//	CloseDialog();
}

void ctoolbox::OnBarStyleChange(DWORD dwOldStyle, DWORD dwNewStyle)
{
    // Call base class implementation.
    CToolBar::OnBarStyleChange(dwOldStyle, dwNewStyle);

    // Use exclusive-or to detect changes in style bits.
    DWORD changed = dwOldStyle ^ dwNewStyle;

    if (changed & CBRS_FLOATING) {
        if (dwNewStyle & CBRS_FLOATING) {
            // ToolBar now floating
        }
        else {
            // ToolBar now docked
			CloseDialog();
        }
    }
/*
    if (changed & CBRS_ORIENT_ANY) {
        if (dwNewStyle & CBRS_ORIENT_HORZ) {
            // ToolBar now horizontal
        }
        else if (dwNewStyle & CBRS_ORIENT_VERT) {
            // ToolBar now vertical            
        }
    }
*/
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
	ON_WM_NCPAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ctoolbox message handlers

BOOL ctoolbox::ToolMessage(int msg)
{
	if (m_pDlg)
		{	
		CWnd * pButton = m_pDlg->GetDlgItem(msg);
		if (pButton)
			{
			pButton->SendMessage(BM_CLICK,0,0);
			return 1;
			}
		}
	return 0;
}

void ctoolbox::CloseDialog(BOOL bMe /* = 0 */)
{
	if (m_pDlg)
		{
		RECT rc;
		m_pDlg->GetWindowRect(&rc);
		m_dlgx = rc.left;
		m_dlgy = rc.top;
		}
	if (m_pDlg && !bMe)
		m_pDlg->DestroyWindow();
	m_pDlg = 0;
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
//	CToolBar::OnRButtonUp(nFlags, point);
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
	if (m_ids[i] == ID_TOOL_ERASER_BOX)
		{
		if (m_bdlgs[m_id])
			m_bReqDlg = TRUE;
		}
	else if ((i < m_nSlots) && m_bdlgs[i])
		{
		m_id = i;
		m_bReqDlg = TRUE;
		}
//	CToolBar::OnRButtonDown(nFlags, point);
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
		if (m_dlgx < 0)
			{
			RECT rc;
			GetWindowRect(&rc);
			m_dlgx = rc.left - 2;
			m_dlgy = rc.bottom + 2;
			}
		m_pDlg->SetWindowPos(NULL, m_dlgx,m_dlgy,
						0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
		m_pDlg->ShowWindow(SW_SHOW);
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
//	m_ticks = 0;
	m_bDown = 0;
	if (m_ticks && m_nEraserId)
		{
		m_ticks = 0;
		int idd = m_nEraserId %1000;
		m_nEraserId = 0;
		if (idd != m_id)
			{
			m_id = idd;
			m_pFrame->PostMessage(WM_COMMAND, ID_TOOL_ERASER,0);
			}
		}
	m_ticks = 0;
	CToolBar::OnLButtonUp( nFlags,  point);
}

void ctoolbox::OnLButtonDown(UINT nFlags, CPoint point)
{
	int idd = PointToSlot(point);
	if (idd < (int)m_nSlots)
		{
		if ((m_ids[idd] == ID_TOOL_ERASER_BOX) && HasDialog(m_id))
			{
			m_nEraserId = 1000+idd;// possibly a dlg req
			}
		else if (idd != m_id)
			{
			CloseDialog();
			m_id = idd;
			if (HasDialog(idd))
				m_pFrame->PostMessage(WM_COMMAND, m_ids[m_id], 0);
			}
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
			m_nEraserId = 0;
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
			BYTE * pSrc;
			if (!zz)
				pSrc = pTemp + 3 * sw * offset; // to specified image
			else
				pSrc = pTemp + 3 * 3 * sw;		// to selected image
			CGScaler scale;
			if (scale.Init(ww/4,hh,24,m_w,m_h))
				{
DPF("scale failure");
				break;
				}
			int q = scale.Custom(pDst, pSrc,p,sp);
DPF("after custom:%d",q);
			int z = scale.Copy();
			UINT x, y;
			x = m_w - 1; // kludge for scaling artifact
			for (y = 0; y < m_h; y++)
				{
				pDst[p*y+3*x+0] = pDst[p*y+3*x-3+0];
				pDst[p*y+3*x+1] = pDst[p*y+3*x-3+1];
				pDst[p*y+3*x+2] = pDst[p*y+3*x-3+2];
				pDst[p*y+0] = 200;
				pDst[p*y+1] = 200;
				pDst[p*y+2] = 200;
				}
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

	VERIFY (images.Create (m_w,m_h,ILC_COLOR24|ILC_MASK,2*m_nSlots,0));
	VERIFY (images.Add (&bm, RGB (200,200,200)) != -1);
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
	SetSlot(IDB_ERASERTOOL, ID_TOOL_ERASER_BOX);
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
	MakeImageList(images,2);
	barCtrl.SetDisabledImageList (&images);
	images.Detach();

	if (!SetButtons( m_ids,m_nSlots))
		{
		TRACE("Failed to create toolbar\n");
		return -1;      // fail to create
		}
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
	m_tool = tool;
	m_id = ToolToSlot(m_tool & 127);
	if (!HasDialog(m_id))
		CloseDialog();
	else if (m_bReqDlg || m_pDlg)
		DoDialog();

#if MAC
	UpdateSelectedButton (m_id);
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
			RECT rc;
			m_pDlg->GetWindowRect(&rc);
			m_dlgx = rc.left;
			m_dlgy = rc.top;
			m_pDlg->DestroyWindow();
			m_pDlg = 0;
			DoDialog();
			}
		}
	FocusFrame();
	return 0;
}

/*

from MS for drawing gripper if xparent toolbar

*/
void ctoolbox::OnNcPaint() 
{
	EraseNonClient();
}
void ctoolbox::EraseNonClient()
{

	// Get window DC that is clipped to the non-client area.
	CWindowDC dc(this);
	CRect rectClient;
	GetClientRect(rectClient);
	CRect rectWindow;
	GetWindowRect(rectWindow);
	ScreenToClient(rectWindow);
	rectClient.OffsetRect(-rectWindow.left, -rectWindow.top);
	dc.ExcludeClipRect(rectClient);

	// Draw the borders in the non-client area.
	rectWindow.OffsetRect(-rectWindow.left, -rectWindow.top);
	DrawBorders(&dc, rectWindow);

	// Erase the parts that are not drawn.
	dc.IntersectClipRect(rectWindow);
	SendMessage(WM_ERASEBKGND, (WPARAM)dc.m_hDC);

	// Draw the gripper in the non-client area.
	DrawGripper(&dc, rectWindow);
}

void ctoolbox::DoPaint(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	// Paint inside the client area.
	CRect rect;
	GetClientRect(rect);
	DrawBorders(pDC, rect);
	DrawGripper(pDC, rect);
}

void ctoolbox::DrawGripper(CDC* pDC, const CRect& rect)
{
	pDC->FillSolidRect( &rect, ::GetSysColor(COLOR_BTNFACE)); // Fill in the BG.
	CToolBar::DrawGripper(pDC,rect);
}
