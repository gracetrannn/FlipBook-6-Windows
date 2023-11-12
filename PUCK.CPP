#include "stdafx.h"
#include "sketch.h"
#include "puck.h"
#include "mainfrm.h"
#include "mydoc.h"
#include "myview.h"
#include "cscene.h"
#include "nscaler.h"
#include "zlib.h"
#include "math.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


CPuckDlg::CPuckDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPuckDlg::IDD, pParent)
{     
	DPF("puck dlg construct");
	m_hWnd = 0;
	m_pDoc = 0;
	m_pBuf = 0;
	m_pCur = 0;
}

CPuckDlg::~CPuckDlg()
{
	DPF("puck destruct");
	delete [] m_pBuf;
	m_pBuf = 0;
	delete [] m_pCur;
	m_pCur = 0;
}

BOOL CPuckDlg::MyCreate( CWnd* pOwnerWnd)//, int x, int y)
{
	BOOL bResult = Create(CPuckDlg::IDD, pOwnerWnd);
	ShowWindow(SW_HIDE);
	DPF("create:%d",bResult);
	return bResult;
}

BEGIN_MESSAGE_MAP(CPuckDlg, CDialog)
	//{{AFX_MSG_MAP(CPuckDlg)
	ON_WM_PAINT()
//	ON_WM_SIZE()
//	ON_WM_VSCROLL()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



void CPuckDlg::OnClose()
{
	DPF("On Lipsync Close");
	((CMainFrame *)GetParent())->SwitchPuck(TRUE);
}


BOOL CPuckDlg::OnInitDialog() 
{
	DPF("puck init");
	CDialog::OnInitDialog();
	
	CRect rect;
	GetClientRect(rect);
	m_orig_w = rect.right - rect.left;
	m_orig_h = 30+rect.bottom - rect.top;


//	UINT lines = rect.bottom - rect.top;

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

	return TRUE;
}


void CPuckDlg::UpdateInfo()
{
	int r = m_pDoc->Radius();
	int d = m_pDoc->Density();
	SetDlgItemInt(IDC_PUCK_1,r,0);
	SetDlgItemInt(IDC_PUCK_2,d,0);
}

BOOL CPuckDlg::Setup(CSketchDoc * pDoc)
{
	DPF("puck setup");
	m_pDoc = pDoc;
	UpdateInfo();
	return 0;
}


void CPuckDlg::OnPaint() 
{
	CRect rect;
	GetClientRect(rect);
	DPR("pal dlg paint ",&rect);
	CPaintDC dc(this);
#if MAC
	CWnd* item = GetDlgItem(IDC_PUCK);
	CRect item_r;
	item->GetClientRect (&item_r);
	CBrush br;
	br.CreateStockObject(LTGRAY_BRUSH);
	dc.FrameRect(item_r, &br);
#endif
/*
	if (!m_pBuf) return;
	int boxw = (rect.right - rect.left);
	if ((boxw != m_curw) || (!m_pCur))
		{
		ReScale(boxw);
		UINT lines = (rect.bottom - rect.top) / m_curh;
		SetScrollRange(SB_VERT, 0, PAL_ROWS - lines, TRUE);
		}
	UINT lines = (rect.bottom - rect.top + m_curh - 1) / m_curh;
DPF("paint,boxw:%d,boxh:%d,lines:%d",m_curw,m_curh,lines);
	CRect zrect;
	UINT x,y,z;
	z = PAL_COLUMNS * m_yoffset;
	for (y = 0; y < lines; y++)
//	for (y = 0; z < PAL_TOTAL; y++)
		{
		for (x = 0; (x < PAL_COLUMNS) && (z < PAL_TOTAL); x++,z++)
			{
			zrect.top = y * m_curh;
			zrect.bottom = zrect.top + m_curh;
			zrect.left = x * m_curw;
			zrect.right = zrect.left + m_curw;
			DoThumb(dc,zrect,z);
			}
		}
*/
}

void CPuckDlg::OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/)
{
//	Capture(0);
	ReleaseCapture();
	GetParent()->SetFocus();
}

void CPuckDlg::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	CRect rect;
	SetCapture();
	m_init_r = m_pDoc->Radius();
	m_init_d = m_pDoc->Density();
	m_point = point;
	//ClientToScreen(&m_point);
	DPF("ax:%d, ay:%d",m_point.x, m_point.y);
//	GetClientRect(rect);
//	DPR("puck lbut down,rc ",&rect);
// save x, y
//	GetParent()->SetFocus();
}


void CPuckDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (this == GetCapture())
		{
		DPF("mouse,x:%d,y:%d",point.x,point.y);
		DPF("dx = %d, dy:%d", point.x - m_point.x, point.y - m_point.y);
		int newr = (point.x - m_point.x) / 10;
		newr += m_init_r;
		if (newr < 1) newr = 1;
		newr = m_pDoc->Radius(newr);
		int newd = (point.y - m_point.y);// / 2;
		newd = m_init_d - newd;
		if (newd < 1) newd = 1;
		newd = m_pDoc->Density(newd);
//		SetDlgItemInt(IDC_PUCK_1,newr,0);
//		SetDlgItemInt(IDC_PUCK_2, newd,0);
		}

	CWnd::OnMouseMove(nFlags, point);
}

CZoomerDlg::CZoomerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CZoomerDlg::IDD, pParent)
{     
	DPF("puck dlg construct");
	m_hWnd = 0;
	m_pDoc = 0;
}

CZoomerDlg::~CZoomerDlg()
{
	DPF("zoomer destruct");

}

BOOL CZoomerDlg::MyCreate( CWnd* pOwnerWnd)//, int x, int y)
{
	BOOL bResult = Create(CZoomerDlg::IDD, pOwnerWnd);
	ShowWindow(SW_HIDE);
	DPF("create:%d",bResult);
	return bResult;
}

BEGIN_MESSAGE_MAP(CZoomerDlg, CDialog)
	//{{AFX_MSG_MAP(CZoomerDlg)
	ON_WM_PAINT()
//	ON_WM_SIZE()
//	ON_WM_VSCROLL()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CLOSE()
	ON_COMMAND(IDC_ZOOM_FIT, OnFit)
	ON_COMMAND(IDC_ZOOM_100, On100)
	ON_COMMAND(IDC_ZOOM_HAND, OnHand)
	ON_COMMAND(IDC_ZOOM_ZOOM, OnZoom)
//	ON_COMMAND(IDC_ZOOM_ZOOMER, OnZoomer)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



void CZoomerDlg::OnClose()
{
	DPF("On zoomer Close");
	((CMainFrame *)GetParent())->SwitchZoomer(TRUE);
}


BOOL CZoomerDlg::OnInitDialog() 
{
	DPF("zoomer init");
	CDialog::OnInitDialog();
	GetWindowText(m_title);
	CRect rect;
	GetClientRect(rect);
	m_orig_w = rect.right - rect.left;
	m_orig_h = 30+rect.bottom - rect.top;
//	UINT lines = rect.bottom - rect.top;

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

	return TRUE;
}

void CZoomerDlg::OnFit()
{
	m_pView->SendMessage(WM_COMMAND, ID_ZOOM_FIT,0);
}

void CZoomerDlg::On100()
{
	m_pView->SendMessage(WM_COMMAND, ID_ZOOM_100,0);
}

void CZoomerDlg::OnHand()
{
	m_pView->SendMessage(WM_COMMAND, ID_TOOL_HAND,0);
}

void CZoomerDlg::OnZoom()
{
	m_pView->SendMessage(WM_COMMAND, ID_TOOL_ZOOM,0);
}

void CZoomerDlg::OnZoomer()
{
}

BOOL CZoomerDlg::Setup(CSketchDoc * pDoc)
{
	DPF("zoom setup");
	m_pDoc = pDoc;
	m_pView = m_pDoc->GetDocView();
	return 0;
}



void CZoomerDlg::OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/)
{
//	Capture(0);
	ReleaseCapture();
	GetParent()->SetFocus();
	SetWindowText(m_title);
}

void CZoomerDlg::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	CRect rect;
	SetCapture();
//	m_pView = m_pDoc->GetDocView();
	m_nNum = m_pView->m_nNum;
	m_nDen = m_pView->m_nDen;
	m_point = point;
	m_pView->GetClientRect(&rect);
	m_center.x = (rect.right + rect.left + 1) / 2;
	m_center.y = (rect.bottom + rect.top + 1) / 2;
	m_pView->XlatePoint(m_center);	// save center of image
	DPF("ax:%d, ay:%d, cx:%d,cy:%d",m_point.x, m_point.y,m_center.x,m_center.y);
	char buf[80];
	sprintf(buf,"%d",100 * m_nNum / m_nDen);
	SetWindowText(buf);
}


void CZoomerDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (this == GetCapture())
		{
		DPF("mouse,x:%d,y:%d",point.x,point.y);
		DPF("dx = %d, dy:%d", point.x - m_point.x, point.y - m_point.y);
		int deltax = (point.x - m_point.x);
		int deltay = (point.y - m_point.y);
		double f;
		if (abs(deltax) > abs(deltay))
			f = deltax;
		else
			f = deltay;
		f /= 200;
		double q = m_nNum;
		q = q * 100 / m_nDen;  // orig zoom
		double v = log(q);
		v = v * f;
		v = exp(v);
		v *= q;
		if (v < 1)
			v = 1;
		else if (v > 1600)
			v = 1600;
		m_pView->Adjust((int)v, 100);
//		m_pView->Adjust((int)v, 100,&m_center);
		char buf[80];
		sprintf(buf,"%d",(int)v);
		SetWindowText(buf);
		}

	CWnd::OnMouseMove(nFlags, point);
}

