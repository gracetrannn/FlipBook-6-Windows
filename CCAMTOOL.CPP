// ccamtool.cpp : implementation file
//
#include "stdafx.h"
#include "sketch.h"
#include "ccamtool.h"
#include "myview.h"
#include "mainfrm.h"
#include "nscaler.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define TIMER_ID 13
#define TIMER_DELAY 150
/////////////////////////////////////////////////////////////////////////////

ccamtool::ccamtool()
{
	DPF("toolbox construct");
	m_nSlots = 0;
	m_bIgnoreRBut = 0;
	m_w = 0;
	m_id = NEGONE;
}

void ccamtool::WeMoved(int x, int y)
{
DPF("tool box move, x:%d,y:%d",x,y);
	m_y = x;
	m_x = y;
}


BEGIN_MESSAGE_MAP(ccamtool, CToolBar)
	//{{AFX_MSG_MAP(ccamtool)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_TIMER()
	ON_WM_NCPAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ccamtool message handlers


void ccamtool::SetSlot(int bmid, int toolid, BOOL bDlg /* = 0 */)
{
	m_bmids[m_nSlots] = bmid;
	m_bdlgs[m_nSlots] = bDlg;
	m_ids[m_nSlots++] = toolid;
}

void ccamtool::FocusFrame()
{
#ifndef FLIPBOOK_MAC
	m_pFrame->SetFocus();
#endif
}

void ccamtool::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (!m_bIgnoreRBut && HasDialog(m_id))
		{
		m_pFrame->PostMessage(WM_COMMAND, ID_TOOL_CAM_DLG, 0);
		}
	m_bIgnoreRBut = 0;
//	CToolBar::OnRButtonUp(nFlags, point);
}

BOOL ccamtool::HasDialog(UINT idd)
{
	if ((idd != NEGONE) && (idd < m_nSlots))
		return m_bdlgs[idd];
	else
		return 0;
}

void ccamtool::OnRButtonDown(UINT nFlags, CPoint point)
{
	UINT i = PointToSlot(point);
	if (i < (int)m_nSlots)
		{
		m_id = i;
		m_pFrame->PostMessage(WM_COMMAND, m_ids[i], 0);
		}
//	CToolBar::OnRButtonDown(nFlags, point);
}

int  ccamtool::PointToSlot(CPoint point)
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

void ccamtool::OnLButtonUp(UINT nFlags, CPoint point)
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

void ccamtool::OnLButtonDown(UINT nFlags, CPoint point)
{
	int idd = PointToSlot(point);

	UINT code = m_ids[idd];
	if ((code == ID_TOOL_CAM_DALL) ||
		(code == ID_TOOL_CAM_DTHIS) ||
		(code == ID_TOOL_CAM_NEXT) ||
		(code == ID_TOOL_CAM_PREV) ||
		(idd != m_id))
			m_pFrame->PostMessage(WM_COMMAND, code, 0);
	if (HasDialog(idd))
		{
		m_ticks = 4;
//		m_pFrame->PostMessage(WM_COMMAND, m_ids[m_id], 0);
		}
	CToolBar::OnLButtonDown(nFlags, point);
}

void ccamtool::OnTimer(UINT nTimerID)
{
	if (nTimerID != TIMER_ID)
		return;
	if (!(GetStyle() & WS_VISIBLE))
		{
		m_ticks = 0;
		}
	else if (m_ticks)
		{
		m_ticks--;
		if (!m_ticks && HasDialog(m_id))
			{
 	CToolBarCtrl&	barCtrl = GetToolBarCtrl();
	barCtrl.PressButton(m_ids[m_id],0);
			m_pFrame->PostMessage(WM_COMMAND, ID_TOOL_CAM_DLG, 0);
			}
		}
	CToolBar::OnTimer(nTimerID);

}


//
//	in ctoolbox.cpp
//
extern HBITMAP zMyMake(LPBITMAPINFOHEADER lpBitmap);//, BYTE * lpBits)

BOOL ccamtool::MakeImageList(CImageList & images, UINT offset)
{
	UINT c, p, factor;
	UINT sw = 48;
	UINT sh = 48;
	factor = 3;//m_nKind > 1 ? 3 : 4;
	p = 4 * ((2 * 3 * m_w * m_nSlots + 3) / 4); // two rows of buttons
	BYTE * pBMP = new BYTE[40+m_h *p]; 
	UINT sp = 4 * ((factor*3*sw+3)/ 4);			// source is factor
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
		if (ww != sw * factor)
			continue;
		if (hh != sh)
			continue;
		if (dd != 24)
			continue;
		pSrc +=  40;	// to bitmap data
		memmove(pTemp, pSrc, sh * sp);
		UnlockResource(hand);
//	pData is dest ptr with pitch of p, two sets of three
		BYTE * pDst = pData;
		for (int zz = 0; zz < 2; zz++, pDst += m_nSlots * 3 * m_w)
			{
			BYTE * pSrc;
			if (!zz)
				pSrc = pTemp + 3 * sw * offset; // to specified image
			else
				pSrc = pTemp + 3 * sw * 2;		// to last image

			CGScaler scale;
			if (scale.Init(ww/factor,hh,24,m_w,m_h))
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

	VERIFY (images.Create (m_w,m_h,ILC_COLOR24|ILC_MASK,2*m_nSlots,0));
	VERIFY (images.Add (&bm, RGB (200,200, 200)) != -1);
	bm.Detach();
	return 0;
}


BOOL ccamtool::MakeToolBar()
{
	DPF("Making toolbar");
	m_nSlots = 0;
	SetSlot(IDB_CAM_PAN_TOOL, ID_TOOL_CAM_PAN,1);
	SetSlot(IDB_CAM_ZOOM_TOOL, ID_TOOL_CAM_ZOOM,1);
	SetSlot(IDB_CAM_ROTATE_TOOL, ID_TOOL_CAM_ROTATE,1);
	SetSlot(IDB_CAM_BLUR_TOOL, ID_TOOL_CAM_BLUR,1);
	SetSlot(IDB_CAM_ALPHA_TOOL, ID_TOOL_CAM_ALPHA,1);
	SetSlot(IDB_CAM_DALLKEYS, ID_TOOL_CAM_DALL);
	SetSlot(IDB_CAM_DTHISKEY, ID_TOOL_CAM_DTHIS);
	SetSlot(IDB_CAM_NEXTKEY, ID_TOOL_CAM_NEXT);
	SetSlot(IDB_CAM_PREVKEY, ID_TOOL_CAM_PREV);
	UINT ww = m_w * m_nSlots;
	SetSizes(CSize(m_w+7,m_h+6),CSize(m_w,m_h));
	CImageList images;
 	CToolBarCtrl&	barCtrl = GetToolBarCtrl();
	MakeImageList(images,0);
	barCtrl.SetImageList (&images);
	images.Detach();
/*
	MakeImageList(images,1);
	barCtrl.SetHotImageList (&images);
	images.Detach();
*/
	MakeImageList(images,2);
	barCtrl.SetDisabledImageList (&images);
	images.Detach();

	if (!SetButtons( m_ids,m_nSlots))
		{
		TRACE("Failed to create toolbar\n");
		return -1;      // fail to create
		}

DPF("a");
	RECT rc;
	GetWindowRect(&rc);
	m_x = rc.left;
	m_y = rc.top;
	return 0;
}

int  ccamtool::ToolToSlot(int tool)
{
	UINT i;
	m_ids[m_nSlots] = tool;
	for (i = 0; m_ids[i] != tool; i++);
	ASSERT(i < m_nSlots);
	return i;
}

BOOL ccamtool::Setup(CMainFrame * pFrame,UINT w, UINT h, BOOL bForce)
{
	SetWindowText("Camera Tools");
	m_pFrame = pFrame;
//	m_pDoc = pDoc;
	if ((m_w != w) || (m_h != h))
		{
		m_w = w;
		m_h = h;
		if (MakeToolBar())
			return 1;
		}
	VERIFY(SetTimer(TIMER_ID, TIMER_DELAY, NULL) == TIMER_ID);
//	m_bTiming = 1;
	
	FocusFrame();
	return 0;
}

/*

from MS for drawing gripper if xparent toolbar

*/
void ccamtool::OnNcPaint() 
{
	EraseNonClient();
}
void ccamtool::EraseNonClient()
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

void ccamtool::DoPaint(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	// Paint inside the client area.
	CRect rect;
	GetClientRect(rect);
	DrawBorders(pDC, rect);
	DrawGripper(pDC, rect);
}

void ccamtool::DrawGripper(CDC* pDC, const CRect& rect)
{
	pDC->FillSolidRect( &rect, ::GetSysColor(COLOR_BTNFACE)); // Fill in the BG.
	CToolBar::DrawGripper(pDC,rect);
}
