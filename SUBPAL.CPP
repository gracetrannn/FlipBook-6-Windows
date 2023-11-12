#include "stdafx.h"
#include "sketch.h"
#include "subpal.h"
#include "mainfrm.h"

CSubPalDlg::CSubPalDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSubPalDlg::IDD, pParent)
{     
	DPF("sub palette dlg construct");
	m_rows = 3;
	m_cols = 6;
	m_width = 120;
	m_height = 40;
	m_font = 18;
	m_index = 0;
	for (int i = 0; i < 18;m_names[i++][0] = 0);
/*
	m_bBusy = 0;
	m_bSavedColor = 0;
	m_nColorTrap = 0;
	m_nKind = 0;
	m_pDoc = 0;
//	m_height = 1;
//	m_width = 1;
	m_pLayers = 0;
	m_pDib = 0;	
*/
}
CSubPalDlg::~CSubPalDlg()
{
	DPF("subpaldlg destruct");
//	delete m_pLayers;
//	m_pLayers = 0;
//	delete m_pDib;
//	m_pDib = 0;
}

BOOL CSubPalDlg::MyCreate( CWnd* pOwnerWnd)//, int x, int y)
{
//	m_pOwnerWnd = pOwnerWnd;
//	m_list.m_pSheet = this;
	BOOL bResult = Create(CSubPalDlg::IDD, pOwnerWnd);
//	SetWindowPos(NULL, 0, 0,m_cols * m_width, m_rows * m_height, SWP_NOMOVE);
	ShowWindow(SW_HIDE);
	DPF("create:%d",bResult);
	return bResult;
}

BEGIN_MESSAGE_MAP(CSubPalDlg, CDialog)
	ON_WM_PAINT()
//	ON_WM_NCPAINT()
//	ON_WM_NCACTIVATE()
//	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
//	ON_WM_NCLBUTTONDOWN()
//	ON_WM_LBUTTONUP()
//	ON_WM_LBUTTONDBLCLK()
//	ON_WM_RBUTTONDOWN()
//	ON_WM_RBUTTONUP()
	ON_WM_SETCURSOR()
	ON_WM_CLOSE()
END_MESSAGE_MAP()



void CSubPalDlg::OnClose()
{
	DPF("On Sub Pal Close");
	((CMainFrame *)GetParent())->SwitchSubPalette(TRUE);
}


BOOL CSubPalDlg::OnInitDialog() 
{
	DPF("sub palette init");
	CDialog::OnInitDialog();
	int i;
	for (i = 0; i < 18; i++)
		{
		UINT r,g,b;
		r = (i / 6) == 0 ? 51 * (i % 6) : 0;
		g = (i / 6) == 1 ? 51 * (i % 6) : 0;
		b = (i / 6) == 2 ? 51 * (i % 6) : 0;
		m_colors[i] = RGB(r,g,b);
		}
#ifndef FLIPBOOK_MAC
	HICON hIcon;

	hIcon = (HICON)LoadImage(  AfxGetApp()->m_hInstance,
                           MAKEINTRESOURCE(IDR_MAINFRAME),
                           IMAGE_ICON,
                           GetSystemMetrics(SM_CXSMICON),
                           GetSystemMetrics(SM_CYSMICON),
                           0);
	if(hIcon)
		{
         SendMessage(WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		}
#endif
	RECT rcWin, rcClient;
	GetWindowRect(&rcWin);
	GetClientRect(&rcClient);
	DPR("win",&rcWin);
	DPR("cli",&rcClient);
	SetWindowPos(NULL, 0, 0,m_cols * m_width, 
		rcWin.bottom - rcWin.top - rcClient.bottom + rcClient.top + m_rows * m_height, SWP_NOMOVE);
	return TRUE;
}


void CSubPalDlg::OnPaint() 
{
	CRect rect;
#ifdef FLIPBOOK_MAC
	// Painting at the window level is problematic on the Mac. Although it is supported by the Cocoa Foundation Class 
	// framework, in this case of the palette dialog we are better off accessing our custom view directly so that the 
	// scrollview works. For windows of a fixed width and height, these kind of "hacks" are not necessary.
	#if 0 // fixme 5
	m_PaletteView.GetLocalRect(&rect);
	DPR("pal dlg paint ",&rect);
	CPaintDC dc (&m_PaletteView);
	dc.Clear();
	#endif
#else
	GetClientRect(rect);
	DPR("pal dlg paint ",&rect);
#endif
	CPaintDC dc(this);
	UINT x,y,z;
	CRect zrect;
	COLORREF crOld;
	COLORREF crNew = RGB(255,255,255);
	UINT nFormat = DT_SINGLELINE | DT_CENTER | DT_VCENTER;
	crOld = dc.GetTextColor();
	for (y = 0, z = 0; y < m_rows; y++)
		{
		for (x = 0; x < m_cols; x++,z++)
			{
			zrect.top = y * m_height;
			zrect.bottom = zrect.top + m_height;
			zrect.left = x * m_width;
			zrect.right = zrect.left + m_width;
			COLORREF crBox = m_colors[z] & 0x00ffffff;
			if (z == m_index)
				{
				crBox ^= 0x00ffffff;
				dc.FillSolidRect(zrect, crBox);
				zrect.top += 3;
				zrect.left += 3;
				zrect.bottom -= 3;
				zrect.right -= 3;
				crBox ^= 0x00ffffff;
				}
			dc.FillSolidRect(zrect, crBox);
			crBox ^= 0x00ffffff;
//			sprintf(buf,"%d,%d",y,x);
	    	dc.SetTextColor(crBox);
#ifndef FLIPBOOK_MAC			
    		dc.DrawText(m_names[z],-1, (LPRECT)zrect, nFormat);
#else
    		dc.DrawText(m_names[z],-1, zrect, nFormat);
#endif
			}
		}
   dc.SetTextColor(crOld);
}

void CSubPalDlg::SetColor(UINT idd, UINT r, UINT g, UINT b, UINT a)
{
	if (idd < 18)
		{
		COLORREF z = RGB(r,g,b);
		z |= a << 24;
		m_colors[idd] = z;
		}
}

void CSubPalDlg::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
//	CRect rect;
//	GetClientRect(rect);
//	DPR("pal lbut down,rc ",&rect);
//	int boxw = (rect.right - rect.left) / m_cols;
	UINT id = m_cols * (point.y / m_height) + point.x / m_width;
	DPF("LBUT x:%d,y:%d,id:%d",point.x,point.y,id);
	m_index = id;
	Invalidate();
	if (GetParent())
		GetParent()->SetFocus();
}

BOOL CSubPalDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	int cursor = 0;
	if (pWnd == this)
		{
		if (nHitTest == HTCLIENT)
			{
//			CPoint point;
//			GetCursorPos(&point);
//			DPF("apoint %d,%d",point.x,point.y);
//			ScreenToClient(&point);
//			DPF("bpoint %d,%d",point.x,point.y);
			
			cursor = IDC_EYEDROP1;
			}
		}
	if (cursor)
		{
		SetCursor(AfxGetApp()->LoadCursor(cursor));
		return FALSE;
		}
	return CDialog::OnSetCursor(pWnd, nHitTest, message);
}
