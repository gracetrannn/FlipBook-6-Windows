#include "stdafx.h"
#include "afxpriv.h"
#include "sketch.h"
#include "palette.h"
#include "mainfrm.h"
#include "mydoc.h"
#include "myview.h"
#include "cscene.h"
#include "dib.h"
#include "clayers.h"
//#include "clevtbl.h"
#include "cnewpals.h"
#include "dialogs.h"
#include "math.h"
#define PAL_COLUMNS 16

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//
//	 just a stub a allow link with old dialogs.cpp
//
BOOL MyColorPrompt(COLORREF & color, CMyDialog * pDlg)
{
	return FALSE;
}

#ifndef FLIPBOOK_MAC
#define RGBA(r,g,b,a) ((COLORREF)((BYTE)(r)| (((WORD)(BYTE)(g)) << 8)| \
					(((DWORD)(BYTE)(b))<<16) | (((DWORD)(BYTE)(a))<<24)))
#endif
/*
class CMySlider1 : public CSliderCtrl
{
public:
	void Setup(int nId, UINT pos, 
			UINT min, UINT max){
			SetRange(min,max);
			m_id = nId;
			SetPos(pos);
			};
	void Assign(UINT v) { SetPos(v);};
	int m_id;
};
*/
#define W_RGB	1
#define W_HSV	2
#define W_LUM	4
#define W_OPAC	8
#define W_FLAGS 16
#define W_PALS	32
#define W_COLOR 64
#define W_WHEEL 128
#define W_KIND  256
#define W_NEW   512


typedef struct {
	union {
		BYTE rgbo[4];
		COLORREF rgb;
	};} MYRGB;

class CColorDlg : public CMyDialog
{
public:
	CColorDlg(CWnd* pParent = NULL);   // standard constructor
	enum { IDD = IDD_COLOR_DLG };
	UINT m_index;
	CNewPals * m_pPals;
	CSketchView * m_pView;
	CPaletteDlg * m_pPalette;
	int	m_posx;
	int m_posy;
	int m_nModel;
	int m_nGradColor;
protected:
	int	m_red;
	int m_green;
	int m_blue;
	int m_hue;
	int m_sat;
	int m_lum;
	int m_opacity;
	CSliderCtrl m_lum_slider;
	CSliderCtrl m_opac_slider;
	int	m_nKind;
	int m_nGradKind;
	GRADENTRY m_grad;
	BOOL m_bHSym;
	BOOL m_bVSym;
	BOOL m_bAlpha;
//	CString m_pal_name;
	CString m_file_name;
	COLORREF m_crCurrent;
	void SetSlider();
	void DoWork(UINT work);
	void GetColor(BOOL bSet = 1);
	void NewColor(BOOL bHSL);
	void SetModel(int which);
	void SetPoint(int which);
	void SetColor(int which);
	void SetKind(int which);
	void SetGrad(int which);
	void ApplyKind();
	void RGBtoHSL();
	void HSLtoRGB();
	void DoCross();
	BOOL SelectColor(int x, int y, int w, int h);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSetHSL() { SetModel(0);};
	afx_msg void OnSetHSV() { SetModel(1);};
	afx_msg void OnSetRed() { SetModel(2);};
	afx_msg void OnSetGreen() { SetModel(3);};
	afx_msg void OnSetBlue() { SetModel(4);};
	afx_msg void OnSetColor1() { SetColor(0);};
	afx_msg void OnSetColor2() { SetColor(1);};
	afx_msg void OnSetPoint1() { SetPoint(0);};
	afx_msg void OnSetPoint2() { SetPoint(1);};
	afx_msg void OnSetPoint3() { SetPoint(2);};
	afx_msg void OnChangeToSimple() { SetKind(0);}; 
	afx_msg void OnChangeToGrad() { SetKind(1);}; 
	afx_msg void OnChangeToTexture() { SetKind(2);}; 
	afx_msg void OnChangeLin()   { SetGrad(1);}; 
	afx_msg void OnChangeRad()   { SetGrad(0);}; 
	afx_msg void OnDrawItem(int nId, LPDRAWITEMSTRUCT lpDrawItemStruct);
//	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnChangeName();
	afx_msg void OnChangeText();
	afx_msg void OnFlags();
//	afx_msg void OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar );
	afx_msg void OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar );
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/);
	afx_msg void OnLButtonDown(UINT /*nFlags*/, CPoint point);
	afx_msg void OnRGBChange() ;
	afx_msg void OnHSLChange() ;
	afx_msg void OnOpacChange() ;
	afx_msg LRESULT OnKickIdle(WPARAM, LPARAM);
	afx_msg void OnRevert() ;
	DECLARE_MESSAGE_MAP()
	friend class CPaletteDlg;
};


void FillIt(CDC & dc, CRect& myrect,BOOL bInvert,
			int index, CNewPals * pPals)
{
	COLORREF crBox;
	UINT cc;
	if (pPals)
		{
		BYTE pals[4];
		pPals->Color(pals,index);
		crBox = RGB(pals[0],pals[1],pals[2]);
		cc = pPals->Kind(index);
		}
	else
		{
		crBox = RGB(255,255,255);
		cc = 0;
		}
	if (cc == 1)
		{
		GRADENTRY gcolor;
		pPals->GetSetGrad(index, gcolor,0); 
		int w = myrect.right - myrect.left;
		myrect.right = myrect.left + w / 2;
		crBox = gcolor.c1 & 0xffffff;
		if (bInvert)
			{
			crBox ^= 0x00ffffff;
			dc.FillSolidRect(myrect, crBox);
			myrect.top += 3;
			myrect.left += 3;
			myrect.bottom -= 3;
			crBox ^= 0x00ffffff;
			dc.FillSolidRect(myrect, crBox);
			myrect.top -= 3;
			myrect.left -= 3;
			myrect.bottom += 3;
			}
		else
			dc.FillSolidRect(myrect, crBox);
		myrect.right = myrect.left + w;
		myrect.left += w / 2;
		crBox = gcolor.c2 & 0xffffff;
		if (bInvert)
			{
			crBox ^= 0x00ffffff;
			dc.FillSolidRect(myrect, crBox);
			myrect.top += 3;
			myrect.right -= 3;
			myrect.bottom -= 3;
			crBox ^= 0x00ffffff;
			dc.FillSolidRect(myrect, crBox);
			}
		else
			dc.FillSolidRect(myrect, crBox);
		}
	else
		{
		if (bInvert)
			{
			crBox ^= 0x00ffffff;
			dc.FillSolidRect(myrect, crBox);
			myrect.top += 3;
			myrect.left += 3;
			myrect.bottom -= 3;
			myrect.right -= 3;
			crBox ^= 0x00ffffff;
			}
		dc.FillSolidRect(myrect, crBox);
		}
}

void CPaletteDlg::WindowSize(int which)
{
	char szSettings[] = "Settings";
	int w, h;
	CString keyw;
	CString keyh;
	if (which == 0)
		{
		if (m_nKind)
			{
			w = AfxGetApp()->GetProfileInt(szSettings, "kind", 0);
			if (w)
				m_nKind = w;
			}
		}
	if (m_nKind == 2)
		{
		keyw = "ModelW";
		keyh = "ModelH";
		}
	else
		{
		keyw = "PalW";
		keyh = "PalH";
		}
	if (which != 1)
		{
		AfxGetApp()->WriteProfileInt(szSettings, "kind", m_nKind);
		w = AfxGetApp()->GetProfileInt(szSettings,keyw, -1);
		h = AfxGetApp()->GetProfileInt(szSettings,keyh, -1);
		if ((w > 0) && (h > 0))
			SetWindowPos(NULL, 0, 0, w, h, SWP_NOMOVE);
		}
	else
		{
		CRect rect;
		GetWindowRect(&rect);
		AfxGetApp()->WriteProfileInt(szSettings,keyw,rect.Width());
		AfxGetApp()->WriteProfileInt(szSettings,keyh,rect.Height());
		}
	if (m_nKind == 2)
		SetWindowText("Model(Palette)");
	else if (m_nKind == 1 )
		SetWindowText("Palette(Model)");
	else
		SetWindowText("Palette Only");
}

CPaletteDlg::CPaletteDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPaletteDlg::IDD, pParent)
{     
	DPF("palette dlg construct");
	m_bBusy = 0;
	m_nColorTrap = 0;
	m_nKind = 0;
	m_pDoc = 0;
	m_pal_index = 0;
	m_pScene = 0;
	m_pLayers = 0;
	m_pDib = 0;	
	m_Level = 0;
	//{{AFX_DATA_INIT(CPaletteDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}
CPaletteDlg::~CPaletteDlg()
{
	DPF("paldlg destruct");
	delete m_pLayers;
	m_pLayers = 0;
	delete m_pDib;
	m_pDib = 0;
}

BOOL CPaletteDlg::MyCreate( CWnd* pOwnerWnd)//, int x, int y)
{
	BOOL bResult = Create(CPaletteDlg::IDD, pOwnerWnd);
	ShowWindow(SW_HIDE);
	DPF("create:%d",bResult);
	return bResult;
}

BEGIN_MESSAGE_MAP(CPaletteDlg, CDialog)
	//{{AFX_MSG_MAP(CPaletteDlg)
	ON_WM_PAINT()
	ON_WM_NCPAINT()
	ON_WM_NCACTIVATE()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_LBUTTONDOWN()
	ON_WM_NCLBUTTONDOWN()
//	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_SETCURSOR()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



void CPaletteDlg::OnClose()
{
	DPF("On Pal Close");
	((CMainFrame *)GetParent())->SwitchPalette(TRUE);
}

void CPaletteDlg::OnCancel() 
{
	DPF("on cancel");
//	CDialog::OnCancel();
}


BOOL CPaletteDlg::OnInitDialog() 
{
	DPF("palette init");
	CDialog::OnInitDialog();
	m_yoffset = 0;
	m_index = 1000;

#ifdef FLIPBOOK_MAC
	m_PaletteView.SubclassDlgItem (1, this);
	m_ScrollBar.SubclassDlgItem (GetWindowScrollerControl (SB_VERT), this);
#endif

	SetScrollRange(SB_VERT, 0, 14, FALSE);
	SetScrollPos(SB_VERT, m_yoffset, TRUE);

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
//	WindowSize(0);

	return TRUE;
}


WORD CPaletteDlg::Setup(CSketchDoc * pDoc)
{
	DPF("tooldlg init");
	m_pDoc = pDoc;
	m_pScene = m_pDoc->GetDocScene();
	return 0;
}

WORD CPaletteDlg::Select(UINT Level)
{
//	CWnd * pWnd = SetFocus();
	m_Level = Level;
	m_nKind = 0;

	m_pal_index = m_pScene->PalIndex(Level);
	delete m_pLayers;
	delete m_pDib;
	m_pLayers = new CLayers;
	if (!m_pLayers->Setup(m_pScene, FALSE, Level))
		{
		m_width = m_pLayers->Width();
		m_height = m_pLayers->Height();
		m_pitch = 4 * ((3 * m_width + 3) / 4);
		m_pDib = new CDib;
		m_pDib->Create(m_width, m_height, 24);
		m_nKind = 2;
		WindowSize(0);
		memset(m_pDib->m_pBits,255,m_pitch * m_height);
		m_pLayers->m_nFlags = 1;
		m_pLayers->Update(m_pDib->m_pBits, 0,m_pitch,1);//0, 0, m_width-1, m_height-1);
		}
	else
		{
		m_pDib = 0;
		delete m_pLayers;
		m_pLayers = 0;
		SetWindowText("Palette Only");
		}
	InvalidateRect(NULL, FALSE);
//	if (pWnd)
//		pWnd->SetFocus();

#if MAC
	[[NSNotificationCenter defaultCenter] postNotificationName:@"FBModelDidChangeNotification" object:nil];
#endif
	
	return 0;
}

#define CURSOR_THICK 3

void CPaletteDlg::PaintKind1() 
{
	CRect rect;
#if MAC
	// Painting at the window level is problematic on the Mac. Although it is supported by the Cocoa Foundation Class 
	// framework, in this case of the palette dialog we are better off accessing our custom view directly so that the 
	// scrollview works. For windows of a fixed width and height, these kind of "hacks" are not necessary.
	m_PaletteView.GetLocalRect(&rect);
	DPR("pal dlg paint ",&rect);
	CPaintDC dc (&m_PaletteView);
	dc.Clear();
#else
	GetClientRect(rect);
	DPR("pal dlg paint ",&rect);
	CPaintDC dc(this);
#endif
	int boxw = (rect.right - rect.left) / PAL_COLUMNS;
	if (!boxw) return;
	UINT lines = (rect.bottom - rect.top) / boxw;
	UINT index = m_pDoc->Color();
	if (index != m_index)
		{
		m_index = index;
		}
DPF("color index:%d",index);
	UINT x,y,z;
//	COLORREF crBox;
	CRect zrect;
	z = PAL_COLUMNS * m_yoffset;
	CNewPals * pPals = m_pScene->PalettePtr(m_pal_index);
	for (y = 0; y < lines; y++)
		{
		zrect.left = 0;
		zrect.right = boxw;
		for (x = 0; (x < PAL_COLUMNS) && (z < 256); x++,z++)
			{
			zrect.top = y * boxw;
			zrect.bottom = zrect.top + boxw;
			zrect.left = x * boxw;
			zrect.right = zrect.left + boxw;
			FillIt(dc,zrect,z == index,z,pPals);
			}
		}
}


void CPaletteDlg::PaintKind2() 
{
	CRect PaintRect;
#if MAC
	m_PaletteView.GetLocalRect(&PaintRect);
	CPaintDC dc (&m_PaletteView);
#else	
	GetClientRect(&PaintRect);    
	CPaintDC dc(this);
#endif
	DPR("painting 1",&PaintRect);
	dc.FillSolidRect(PaintRect, RGB(255,255,255));
	if (m_pDib)
		{
		if (m_pDib->Width() < (DWORD)PaintRect.Width() && m_pDib->Height() < (DWORD)PaintRect.Height())
		{ // If the image fits, just center it
			m_dstx = (PaintRect.Width() - m_pDib->Width())/2;
			m_dsty = (PaintRect.Height() - m_pDib->Height())/2;
			m_dstw = m_pDib->Width();
			m_dsth = m_pDib->Height();
		}
		else
		{ // The bitmap doesn't fit, scale to fit 
			if ((PaintRect.Width() * m_pDib->Height()) <= 
								(PaintRect.Height() * m_pDib->Width()))
			{ // Width is constraint
				m_dstw = PaintRect.Width();
				m_dsth = (m_dstw * m_pDib->Height()) / m_pDib->Width();
				m_dstx = 0;
				m_dsty = (PaintRect.Height() - m_dsth) /2;
			}
			else
			{ // Height is constraint		
				m_dsth = PaintRect.Height();
				m_dstw = (m_dsth * m_pDib->Width()) / m_pDib->Height();
				m_dstx = (PaintRect.Width() - m_dstw) /2;
				m_dsty = 0;
			}
		}

		CRect RectDest(PaintRect.left + m_dstx, PaintRect.top + m_dsty,
				PaintRect.left+m_dstx + m_dstw,PaintRect.top+m_dsty+m_dsth);
		CRect RectDib(0, 0, m_pDib->Width(), m_pDib->Height());
	DPR("painting dst",&RectDest);
	DPR("painting dib",&RectDib);
		m_pDib->Paint(dc, &RectDest, &RectDib);     
	}
	else
	{
		dc.MoveTo(PaintRect.TopLeft());
		dc.LineTo(PaintRect.BottomRight());
		dc.MoveTo(PaintRect.right, PaintRect.top);
		dc.LineTo(PaintRect.left, PaintRect.bottom);
	}
}

void CPaletteDlg::OnPaint() 
{
	if (m_nKind == 2)
		PaintKind2();
	else
		PaintKind1();
}


BOOL CPaletteDlg::OnNcActivate(BOOL bActive) 
{
   OnNcPaint(); 
   return TRUE; 
}

void CPaletteDlg::OnNcPaint() 
{
//	CDialog::OnNcPaint();
	Default();
	CWindowDC dc(this);
	CRect rectClient;
	GetClientRect(rectClient);
	CRect rectWindow;
	GetWindowRect(rectWindow);
	ScreenToClient(rectWindow);
	rectClient.OffsetRect(-rectWindow.left, -rectWindow.top);
	dc.ExcludeClipRect(rectClient);
	DPR("nc client",&rectClient);
	DPR("nc window",&rectWindow);
	CRect myrect;
	int h = rectClient.top;
	int w = rectClient.right;
	myrect.top = 3;
	myrect.left = w - 3 * h;
	myrect.bottom = h-1;
	myrect.right = w - h;
	DPR("myrect",&myrect);
	CNewPals * pPals = 0;
	if (m_pDoc && m_pScene)
		{
		m_index = m_pDoc->Color();
		pPals = m_pScene->PalettePtr(m_pal_index);
		}
	FillIt(dc,myrect,TRUE,m_index,pPals);
					
}

void CPaletteDlg::OnVScroll(UINT code , UINT pos , CScrollBar* pScrollBar)
{
	DPF("pal scroll,code:%d,pos:%d",code,pos);
	CRect rect;
	GetClientRect(rect);
	int boxw = (rect.right - rect.left) / PAL_COLUMNS;
	if (!boxw) return;
	UINT lines = (rect.bottom - rect.top) / boxw;
	switch (code) {
	case SB_PAGEUP:
	case SB_LINEUP:
		if (m_yoffset)
			m_yoffset--;
		break;
	case SB_PAGEDOWN:
	case SB_LINEDOWN:
		m_yoffset++;
		break;
	case SB_TOP:
		m_yoffset = 0;
		break;
	case SB_BOTTOM:
		m_yoffset = 14;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		m_yoffset = pos;
		break;
	case SB_ENDSCROLL:
		GetParent()->SetFocus();
		return;
	default:
		return;
	}
	if ((lines + m_yoffset) > (256 / PAL_COLUMNS))
		m_yoffset = (256 / PAL_COLUMNS) - lines;
	if (m_yoffset != (UINT)GetScrollPos(SB_VERT))
		{
DPF("new pos:%d",m_yoffset);
		SetScrollPos(SB_VERT, m_yoffset, TRUE);
		InvalidateRect(NULL, FALSE);
		}
//	SetFocus(hwndMain);
}

void CPaletteDlg::OnSize(UINT nType, int cx, int cy)
{
	if (m_bBusy)
		return;
	m_bBusy = TRUE;
	DPF("pal size,w:%d,h:%d",cx,cy);
	int smin, smax;
	GetScrollRange(SB_VERT,&smin,&smax);
	int bw = cx / PAL_COLUMNS;
	if (m_nKind == 2)
		{
		CDialog::OnSize(nType, cx, cy);
		Invalidate();
		}
	else if (cx >= 120 && cy >= 20 && cy < (PAL_COLUMNS * bw + 20))
		{
		CDialog::OnSize(nType, cx, cy);
		Invalidate();
		}
	else
		{
		if (cy > (20 + PAL_COLUMNS * bw))
			cy = 20 + PAL_COLUMNS * bw;
		if (cx < 150) cx = 150;
		if (cy < 30) cy = 30;
		DPF("new cx:%d,cy:%d",cx,cy);
		SetWindowPos(NULL, 0, 0, cx, cy, SWP_NOMOVE);
		}
	bw = cx / PAL_COLUMNS;
	int lines = cy / bw;
	if (lines > (256 / PAL_COLUMNS))
		lines = (256 / PAL_COLUMNS);
DPF("old max:%d,new:%d",smax,lines);
	if (lines != smax)
		{
DPF("setting range:%d",(256 / PAL_COLUMNS)-lines);
		SetScrollRange(SB_VERT, 0, (256 / PAL_COLUMNS) - lines, FALSE);
		SetScrollPos(SB_VERT, 0, TRUE);
		}
	m_bBusy = 0;
}

void CPaletteDlg::OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/)
{
	GetParent()->SetFocus();
}

void CPaletteDlg::OnNcLButtonDown(UINT nFlags, CPoint point)
{
	DPF("flipper");
	CRect rect;
	GetWindowRect(rect);
	DPF("LNCBUT x:%d,y:%d,id:%d",point.x,point.y);
	DPR("client",&rect);
	if (!m_nKind || (GetKeyState(VK_SHIFT) >= 0)
		|| (point.y >= rect.top + GetSystemMetrics(SM_CYMENU))
		|| (point.x >= rect.right - GetSystemMetrics(SM_CYMENU)))
		{
		CDialog::OnNcLButtonDown(nFlags, point);
		return;
		}
	WindowSize(1);
	m_nKind ^= 3;
	WindowSize(2);
	CDialog::OnNcLButtonDown(nFlags,point);
	Invalidate(0);
	GetParent()->SetFocus();
}

#if MAC
void CPaletteDlg::ReceiveCurrentColorChange (COLORREF& inColor)
{
	#if 0 // FIXME: something broke here in FB 6, but may not be necessary anymore
	BYTE * pPals = &m_pDoc->GetDocView()->Layers()->LevelTable()->pals[0];

	int idd = m_pDoc->Color();

	pPals[4*idd+0] = GetRValue(inColor);
	pPals[4*idd+1] = GetGValue(inColor);
	pPals[4*idd+2] = GetBValue(inColor);
	pPals[4*idd+3] = GetAValue(inColor);
	UpdatePals(idd);
	#endif
}
#endif
/*
void CPaletteDlg::ChangeColorOrModelKind ()
{
	if (m_nKind) {
		WindowSize(1);
		m_nKind ^= 3;
		WindowSize(2);
		CDialog::OnNcLButtonDown(nFlags,point);
		Invalidate(0);
		GetParent()->SetFocus();
	}
}
*/

void CPaletteDlg::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	CRect rect;
	if (m_nKind == 2)
		{
		if (XlatePoint(point))
			{
			DPF("cpoint %d,%d",point.x,point.y);
			if (m_pLayers->HaveAlpha(point.x,point.y))
				{
				m_pDoc->Color(m_pLayers->GetIndex(point.x,point.y));
	SendMessage(WM_NCPAINT,0,0);
				}
			}
		}
	else
		{
		GetClientRect(rect);
	DPR("pal lbut down,rc ",&rect);
	int boxw = (rect.right - rect.left) / PAL_COLUMNS;
	if (!boxw) return;
	UINT id = PAL_COLUMNS * (m_yoffset + (point.y / boxw)) + point.x / boxw;
	DPF("LBUT x:%d,y:%d,id:%d",point.x,point.y,id);
	m_pDoc->Color(id);
	SendMessage(WM_NCPAINT,0,0);
	
		Invalidate();
		}
}

void CPaletteDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	OnRButtonUp (nFlags, point);
}

void CPaletteDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	DPF("pal r but down");
//	Capture(0);

	// select the color first
	OnLButtonDown (nFlags, point);
}

void CPaletteDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
	UINT idd;
	if (m_nKind == 2)
		{
		if (!XlatePoint(point))
			{
			GetParent()->SetFocus();
			return;
			}
		if (!m_pLayers->HaveAlpha(point.x,point.y))
			{
			GetParent()->SetFocus();
			return;
			}
		idd = m_pDoc->Color(m_pLayers->GetIndex(point.x,point.y));
		}
	else
		{
		CRect rect;
		GetClientRect(rect);
		DPR("pal dlg paint ",&rect);
		int boxw = (rect.right - rect.left) / PAL_COLUMNS;
		if (!boxw) return;
		idd = PAL_COLUMNS * (m_yoffset + (point.y / boxw)) + point.x / boxw;
		}
	DPF("RBUT x:%d,y:%d,id:%d",point.x,point.y,idd);
	EditColor(idd);
}

void CPaletteDlg::EditColor(UINT id)
{
	if (m_pScene->PalShared(m_pal_index))
		{
		CPalSharedDlg dlg;
		dlg.m_bModOne = 0;
		if (dlg.DoModal() != IDOK)
			{
			GetParent()->SetFocus();
			return;
			}
		if (dlg.m_bModOne)
			{
			CNewPals * pPals = m_pScene->PalettePtr(m_pal_index);
			UINT index = m_pScene->NewPalette(dlg.m_name, pPals);
			m_pScene->PalIndex(m_Level,index);
			m_pal_index = index;
			}
		}
	m_pindex = id;
	EditOne();
}

void CPaletteDlg::EditOne()
{
	char szSettings[] = "Settings";
	CNewPals * pPals = m_pScene->PalettePtr(m_pal_index);
	pPals->SaveEntry(m_pindex);
	CColorDlg dlg;
	dlg.m_index = m_pindex;
	dlg.m_pPalette = this;
	dlg.m_pView = m_pDoc->GetDocView();
	dlg.m_pPals = pPals;
	int k = AfxGetApp()->GetProfileInt(szSettings,"ColorDlgM", 0);
	dlg.m_nGradColor = k & 1;
	k /= 2;
	dlg.m_nModel = k;
	dlg.m_posx = AfxGetApp()->GetProfileInt(szSettings,"ColorDlgX", -1);
	dlg.m_posy = AfxGetApp()->GetProfileInt(szSettings,"ColorDlgY", -1);
	int res = dlg.DoModal();
	BOOL bKeepTrap = 0;
	if ((res == 9) || (res == 10) || (res == 11) || (res == IDOK))
		{
		int v = 2 * dlg.m_nModel + (dlg.m_nGradColor & 1);
		AfxGetApp()->WriteProfileInt(szSettings, "ColorDlgM", v);
		AfxGetApp()->WriteProfileInt(szSettings, "ColorDlgX", dlg.m_posx);
		AfxGetApp()->WriteProfileInt(szSettings, "ColorDlgY", dlg.m_posy);
		if ((res == 9) || (res == 10) || (res == 11))
			{
			bKeepTrap = 1;
			m_nColorTrap = 1 + res - 9;
			m_pDoc->GetDocView()->SetColorTrap(m_nColorTrap,dlg.m_grad.kind,
				dlg.m_grad.p1,dlg.m_grad.p2);// in case kind has changed
			}
		else
			{
			if (pPals->Linked(0))
				{
				CPalLinkedDlg dlg;
				dlg.m_fname = pPals->GetFileName();
				int res = dlg.DoModal();
				if (res == IDOK)
					{
					pPals->RestEntry(0); // just empty it
					if (dlg.m_bBreak)
						{
						m_pScene->NewPalName(dlg.m_name, m_pal_index);
					//	pPals->SetPalName(dlg.m_name);
						pPals->SetFileName("");
						}
					else
						pPals->PaletteIO(dlg.m_fname,1);
					}
				else
					{
					pPals->RestEntry(2); // revert and empty
					UpdatePals();
					}
				}
			}
		}
	else
		{
		pPals->RestEntry(2); // revert and empty
		UpdatePals();
		}
	if (!bKeepTrap)
		m_pDoc->GetDocView()->SetColorTrap();
	if (GetParent())
		GetParent()->SetFocus();
}

void CPaletteDlg::ColorTrap(int x, int y, int code)
{
	if (!m_nColorTrap || (m_nColorTrap > 3))
		return;
	if (code == 1)
		{
		m_grad1x = x;
		m_grad1y = y;
		return;
		}
	if (x < 0)
		{
		m_pDoc->GetDocView()->Update(0,m_Level,4); // remove selection
		return;
		}
	CPoint point = CPoint(x,y);
	GRADENTRY grad;
	CNewPals * pPals = m_pScene->PalettePtr(m_pal_index);
	if (pPals->GetSetGrad(m_pindex, grad,0)) 
		return;
	if (code == 2)
		{
		grad.p2 = point;
		grad.p1.x = m_grad1x;
		grad.p1.y = m_grad1y;
		}
	else if (m_nColorTrap > 1)
		grad.p2 = point;
	else
		grad.p1 = point;
	pPals->GetSetGrad(m_pindex, grad,1);
	UpdatePals();
	EditOne();
}

void CPaletteDlg::UpdatePals(int id)
{
	m_pScene->PalChanged(m_pal_index);
	m_pDoc->GetDocView()->Update(0,m_Level,4);
	m_pDoc->GetDocView()->CheckUpdate();
	((CMainFrame *)GetParent())->DirtyThumb(m_pal_index);
	if (id >= 0)
		m_pDoc->Color(id);
	ChangePals();
}

BOOL CPaletteDlg::SetColor(UINT id, BYTE r, BYTE g, BYTE b,  BYTE alpha)
{
	CNewPals * pc = m_pScene->LevelPalette(m_Level);
//			(CNewPals *)&m_pDoc->GetDocView()->Layers()->LevelTable()->cpals;
	pc->Assign(id,r,g,b,alpha);
//	m_pDoc->GetDocView()->Layers()->LevelTable(TRUE);
	m_pDoc->GetDocView()->Update(0,m_Level,4);
	m_pDoc->GetDocView()->CheckUpdate();
	((CMainFrame *)GetParent())->DirtyThumb(m_pScene->PalIndex(m_Level));
	m_pDoc->Color(id);
	ChangePals();
	return 1;
}

BOOL CPaletteDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	int cursor = 0;
	if (pWnd == this)
		{
		if (nHitTest == HTCLIENT)
			{
			CPoint point;
			GetCursorPos(&point);
//			DPF("apoint %d,%d",point.x,point.y);
			ScreenToClient(&point);
//			DPF("bpoint %d,%d",point.x,point.y);
			
			if (m_nKind == 2)
				{
				if (XlatePoint(point))
					{
//					DPF("cpoint %d,%d",point.x,point.y);
					if (m_pLayers->HaveAlpha(point.x,point.y))
						cursor = IDC_EYEDROP1;
					else
						cursor = IDC_EYEDROP2;
					}
				}
			else
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

BOOL CPaletteDlg::XlatePoint(CPoint& point)
{
	UINT x = point.x - m_dstx;
	UINT y = point.y - m_dsty;
	if (x < m_dstw && y < m_dsth)
		{
		point.x = (x * m_width) / m_dstw;
		point.y = (y * m_height) / m_dsth;
		return 1;
		}
	return 0;
}

BOOL CPaletteDlg::HasModel(UINT v /* = -1 */)
{
	BOOL bResult = FALSE;
	if (m_pLayers != NULL)
		{
		if (v != -1)
			{
			m_nKind = v + 1;
			if (m_nKind == 2)
				{
	Select(m_Level);
//	m_pLayers->Update(m_pDib->m_pBits, m_pitch,0, 0, m_width-1, m_height-1);
//				SetWindowText("Model(Palette)");
				}
			else
				SetWindowText("Palette(Model)");
			Invalidate(0);
			bResult = TRUE;
			}
		else
			{
			if ((m_pScene->Width() == m_width) &&
					(m_pScene->Height() == m_height))
				bResult = TRUE;
			}
		}
	 return bResult;
}

void CPaletteDlg::ChangePals()
{
	if (m_pLayers)
		m_pLayers->Update(m_pDib->m_pBits, 0,m_pitch,1);
	Invalidate(0);
	SendMessage(WM_NCPAINT,0,0);
//	UpdateWindow();
	
}

CColorDlg :: CColorDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CColorDlg::IDD, pParent)
{
	m_posx = -1;
}

BEGIN_MESSAGE_MAP(CColorDlg, CMyDialog)

	ON_EN_CHANGE(IDC_CD_RED, OnRGBChange)
	ON_EN_CHANGE(IDC_CD_GREEN,OnRGBChange)
	ON_EN_CHANGE(IDC_CD_BLUE, OnRGBChange)
	ON_EN_CHANGE(IDC_CD_HUE, OnHSLChange)
	ON_EN_CHANGE(IDC_CD_SAT, OnHSLChange)
	ON_EN_CHANGE(IDC_CD_LUM, OnHSLChange)
	ON_EN_CHANGE(IDC_CD_OPAC, OnOpacChange)

	ON_COMMAND(IDC_CD_HSL, OnSetHSL)
	ON_COMMAND(IDC_CD_HSV, OnSetHSV)
	ON_COMMAND(IDC_CD_R, OnSetRed)
	ON_COMMAND(IDC_CD_G, OnSetGreen)
	ON_COMMAND(IDC_CD_B, OnSetBlue)
	ON_COMMAND(IDC_CD_GRAD_C1, OnSetColor1)
	ON_COMMAND(IDC_CD_GRAD_C2, OnSetColor2)
	ON_COMMAND(IDC_CD_GRAD_P1, OnSetPoint1)
	ON_COMMAND(IDC_CD_GRAD_P2, OnSetPoint2)
	ON_COMMAND(IDC_CD_GRAD_PP, OnSetPoint3)
	ON_COMMAND(IDC_CD_SIMPLE,  OnChangeToSimple)
	ON_COMMAND(IDC_CD_GRAD,    OnChangeToGrad)
	ON_COMMAND(IDC_CD_TEXTURE, OnChangeToTexture)
	ON_COMMAND(IDC_CD_GRAD_RAD,OnChangeRad)
	ON_COMMAND(IDC_CD_GRAD_LIN,OnChangeLin)
	ON_COMMAND(IDC_CD_TEXT_CHANGE,OnChangeName)
	ON_COMMAND(IDC_CD_TEXT_H,OnChangeText)
	ON_COMMAND(IDC_CD_TEXT_V,OnChangeText)
	ON_COMMAND(IDC_CD_TEXT_ALPHA,OnChangeText)
	ON_COMMAND(IDC_CD_REVERT,OnRevert)
	ON_WM_DRAWITEM()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
END_MESSAGE_MAP()

void CColorDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_CD_HSL, m_nModel);
	DDX_Radio(pDX, IDC_CD_SIMPLE, m_nKind);
	DDX_Radio(pDX, IDC_CD_GRAD_RAD, m_nGradKind);
	DDX_Radio(pDX, IDC_CD_GRAD_C1, m_nGradColor);
	DDX_Check(pDX, IDC_CD_TEXT_H, m_bHSym);
	DDX_Check(pDX, IDC_CD_TEXT_V, m_bVSym);
	DDX_Check(pDX, IDC_CD_TEXT_ALPHA, m_bAlpha);
	DDX_Control(pDX, IDC_CD_SLIDER, m_lum_slider);
	DDX_Control(pDX, IDC_CD_OPACITY, m_opac_slider);
	DDX_Text(pDX, IDC_CD_RED, m_red);
	DDV_MinMaxInt(pDX, m_red,0, 255);
	DDX_Text(pDX, IDC_CD_GREEN, m_green);
	DDV_MinMaxInt(pDX, m_green,0, 255);
	DDX_Text(pDX, IDC_CD_BLUE, m_blue);
	DDV_MinMaxInt(pDX, m_blue,0, 255);
	DDX_Text(pDX, IDC_CD_HUE, m_hue);
	DDV_MinMaxInt(pDX, m_hue,0, 240);
	DDX_Text(pDX, IDC_CD_SAT, m_sat);
	DDV_MinMaxInt(pDX, m_sat,0, 240);
	DDX_Text(pDX, IDC_CD_LUM, m_lum);
	DDV_MinMaxInt(pDX, m_lum,0, 240);
	DDX_Text(pDX, IDC_CD_OPAC, m_opacity);
	DDV_MinMaxInt(pDX, m_opacity,0, 100);
}


BOOL CColorDlg::OnInitDialog()
{
	m_red   = m_pPals->Red(m_index);
	m_green = m_pPals->Green(m_index);
	m_blue  = m_pPals->Blue(m_index);
	m_red = m_pPals->Red(m_index);
	m_opacity = 100;
	m_nGradKind = 1;
	m_sat = 0;
	m_hue = 0;
	m_lum = 0;
	m_pPals->GetSetGrad(m_index,m_grad,0);
	m_bHSym = 0;
	m_bVSym = 0;
	m_bAlpha = 0;
	CMyDialog::OnInitDialog();
	BOOL bEnable = ((CSketchApp *)AfxGetApp())->IsLite() ? 0 : 1;
	GetDlgItem(IDC_CD_GRAD)->EnableWindow(bEnable);
	GetDlgItem(IDC_CD_TEXTURE)->EnableWindow(bEnable);
	
	GetColor(0);
	if ((m_posx > 0) && (m_posy > 0))
		SetWindowPos(NULL, m_posx, m_posy, 0, 0, SWP_NOSIZE|SWP_SHOWWINDOW);
	m_file_name = m_pPals->FileName(m_index);
	GetDlgItem(IDC_CD_TEXT_NAME)->SetWindowText(m_pPals->FileName(m_index));
//	m_lum_slider.Setup(IDC_CD_SLIDER,128,0,255);
	m_lum_slider.SetRange(0,255);
	m_lum_slider.SetPos(128);
//	m_opac_slider.Setup(IDC_CD_OPACITY,m_opacity,0,100);
	m_opac_slider.SetRange(0,100);
	m_opac_slider.SetPos(m_opacity);
	SetModel(m_nModel);
	ApplyKind();


#if MAC
	DoWork(W_COLOR | W_PALS);
#endif


	return 0;
}

void CColorDlg::OnOK()
{
	DPF("got ok");
	RECT rect;
	GetWindowRect(&rect);
	m_posx = rect.left;
	m_posy = rect.top;
	CMyDialog::OnOK();
}


void CColorDlg::DoCross()
{
	int xx, yy;
	CRect rect;
	CWnd * pWnd = GetDlgItem(IDC_CD_WHEEL);
	if (!pWnd)
		return;
	pWnd->GetClientRect(rect);
	int w = rect.right - rect.left;
	int h = rect.bottom - rect.top;
	int r = m_red;
	int g = m_green;
	int b = m_blue;
	if (m_nModel == 4)
		{
		xx = w * r / 255;
		yy = h * g / 255;
		}
	else if (m_nModel == 3)
		{
		xx = w * r / 255;
		yy = h * b / 255;
		}
	else if (m_nModel == 2)
		{
		xx = w * g / 255;
		yy = h * b / 255;
		}
	else
		{
		int ss,hh,ll;
  		int maxcol = max(r, max(g, b));
  		int mincol = min(r, min(g, b));
  		ll = (mincol + maxcol) / 2;   /* lightness */
  		if (maxcol == mincol)   /* achromatic case */
			{
    		ss = 0;         /* hue is undefined in the achromatic case */
    		hh = 0;
			}
		else
			{
			int delta = maxcol - mincol;
    		if (ll <= 128)
     			ss = 255 * delta / (maxcol + mincol);
    		else
      			ss = 255 * delta / (510 - maxcol - mincol);
    		if (r == maxcol)
     			hh = g - b;
			else if (g == maxcol)
        		hh = 2 * delta + b - r;
      		else 
        		hh = 4 * delta + r - g;
			hh = (60 * hh / delta + 360) % 360;
			DPF("hh:%d,ss:%d",hh,ss);
			}
		if (m_nModel)
			{
			xx = hh * w / 360;
			yy = ss * h / 255;
			}
		else
			{
			int mr;
			if (w > h)
				mr = 9 * h / 20;
			else
				mr = 9 * w / 20;
			double f = 180.0 / 3.14159265;
			int cx = (rect.left + rect.right) / 2;
			int cy = (rect.bottom + rect.top) / 2;
			int rr = mr * ss / 255;
			double angle = (double)hh / f;
			xx = cx + (int)((double)rr * cos(angle));
			yy = cy + (int)((double)rr * sin(angle));
			}
		}
	if (m_nModel)
		yy = h - 1 - yy;
	CWindowDC appDC(pWnd);
	int z = 6;
	HPEN	hPenOld = (HPEN)appDC.SelectObject(GetStockObject( WHITE_PEN ) );
	int old = appDC.SetROP2(R2_BLACK);
	
	#if MAC
		// adjust for view bounds
		if (m_nModel > 0) {
			xx += rect.left;
			yy += rect.top;
		}
	#endif
	
	appDC.MoveTo(xx - z, yy);
	appDC.LineTo(xx + z, yy);
	appDC.MoveTo(xx, yy - z);
	appDC.LineTo(xx, yy + z);
	appDC.SetROP2(old);
	appDC.SelectObject(hPenOld );
}

void CColorDlg::OnDrawItem(int nId, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (lpDrawItemStruct->itemAction != ODA_DRAWENTIRE)	
		return;
	CRect rect;
	CWnd * pWnd = GetDlgItem(nId);
	if (!pWnd) return;
	CWindowDC appDC(pWnd);
	pWnd->GetClientRect(rect);
	int w = rect.right - rect.left;
	int h = rect.bottom - rect.top;
	if (nId == IDC_CD_WHEEL)
		{
		int x,y,mr;
		int r = m_red;//GetRValue(cr);
		int g = m_green;//GetGValue(cr);
		int b = m_blue;//GetBValue(cr);

		if (w > h)
			mr = 9 * h / 20;
		else
			mr = 9 * w / 20;
		double f = 180.0 / 3.14159265;
 		MYRGB crBack;
		crBack.rgb = (::GetSysColor (COLOR_BTNFACE));
		int cx = (rect.left + rect.right) / 2;
		int cy = (rect.bottom + rect.top) / 2;
		for (y = rect.top; y  < rect.bottom; y++)
		{
		for (x = rect.left; x < rect.right; x++)
			{
			MYRGB rgb = crBack;
			int rem = 0;
			if (m_nModel > 1)
			{
				if (m_nModel == 2)
					{
					rgb.rgbo[0] = m_red;
					rgb.rgbo[1] = (BYTE)(255 * (x - rect.left) / w);
					rgb.rgbo[2] = (BYTE)(255 - 255 * (y - rect.top) / h);
					}
				else if (m_nModel == 3)
					{
					rgb.rgbo[1] = m_green;
					rgb.rgbo[0] = (BYTE)(255 * (x - rect.left) / w);
					rgb.rgbo[2] = (BYTE)(255 - 255 * (y - rect.top) / h);
					}
				else if (m_nModel == 4)
					{
					rgb.rgbo[2] = m_blue;
					rgb.rgbo[0] = (BYTE)(255 * (x - rect.left) / w);
					rgb.rgbo[1] = (BYTE)(255 - 255 * (y - rect.top) / h);
					}
				}
			else
			{
			int l = 128;
			int r = (x - cx) * (x - cx) + (y - cy) * (y - cy);
			if (m_nModel || (r < mr * mr))
				{
				int sat, hue;
				if (m_nModel == 1)
					{
					sat = 255 * (rect.bottom - 1 - y) / h;
					hue = 360 + 360 * (x - rect.left) / w;
					}
				else
					{
					double rr = sqrt((double)r);
					rem = (int)(255 * mr - 255 * rr);
					if (rem > 254) rem = 0;
					sat = (int)(255 * rr / (double)mr);
					double angle = f * atan2(double(y-cy),double(x-cx));
					hue = 360 + (int)angle;
					}
				int m1, m2;
				if (l < 128)
					m2 = l + (l * sat) / 255;
				else
					m2 = l + sat - (l * sat) / 255;
				m1 = 2 * l - m2;
				if (sat)
					{
					int i;
					for (i = 0; i < 3; i++)
						{
						UINT z;
						int h = (hue + 120 * ( 1 - i)) % 360;
						if (h < 60)
							z = m1 + (m2  - m1) * h / 60;
						else if (h < 180)
							z = m2;
						else if (h < 240)
							z = m1 + (m2 - m1) * (240 - h) / 60;
						else
							z = m1;
						if (rem)
							rgb.rgbo[i] = (z * rem + rgb.rgbo[i] * (255 - rem)) / 255;
						else
							rgb.rgbo[i] = z;
						}
					}
				}
//			BYTE pals[4];
//			m_pPals->Color(pals,m_index,x,y);
			}
			appDC.SetPixel(x,y,rgb.rgb);
			}
		}
		DoCross();
		}

	if (nId == IDC_CD_COLOR)
		{
	//	CWindowDC appDC(pWnd);
		if (!m_pPals->Kind(m_index))
			appDC.FillSolidRect(rect, m_crCurrent);
		else if (m_pPals->Kind(m_index) == 1)
			{
			int x, y;
			MYRGB rgb1, rgb2;
			rgb1.rgb = m_grad.c1;
			rgb2.rgb = m_grad.c2;
			for (y = 0; y < h; y++)
			for (x = 0; x < w; x++)
				{
				BYTE rgb[4];
				UINT f2;
				if (m_grad.kind)
					f2 = 255 * x / (w-1);
				else
					f2 = 255 * (abs(w/2 - x)) / (w/2);
				UINT f1 = 255 - f2;
				rgb[0] = (f1 * rgb1.rgbo[0] + f2 * rgb2.rgbo[0]) / 255;
				rgb[1] = (f1 * rgb1.rgbo[1] + f2 * rgb2.rgbo[1]) / 255;
				rgb[2] = (f1 * rgb1.rgbo[2] + f2 * rgb2.rgbo[2]) / 255;
				appDC.SetPixel(x,y,RGB(rgb[0], rgb[1],rgb[2]));
				}
			}
		else
			{
			int x, y;
			for (y = 0; y < h; y++)
			for (x = 0; x < w; x++)
				{
				BYTE rgb[4];
				m_pPals->Color(rgb,m_index,x,y);
				appDC.SetPixel(x,y,RGB(rgb[0], rgb[1],rgb[2]));
				}
			}
		}
}


void CColorDlg::OnFlags()
{
	UpdateData(0);
	UINT flags = m_pPals->Flags(m_index);
	flags |= NPAL_FLAG_HSYM | NPAL_FLAG_VSYM | NPAL_FLAG_NORGB;
	if (!m_bHSym) flags ^= NPAL_FLAG_HSYM;
	if (!m_bVSym) flags ^= NPAL_FLAG_VSYM;
	if (!m_bAlpha) flags ^= NPAL_FLAG_NORGB;
	m_pPals->Flags(m_index,flags);
	DoWork(W_FLAGS);
}

void CColorDlg::OnChangeName()
{
	CString name;
	if (!((CSketchApp*) AfxGetApp())->PromptFileName(name, 13))
		return;
	m_file_name = name;
	m_pPals->LoadTexture(m_index,name);
	GetDlgItem(IDC_CD_TEXT_NAME)->SetWindowText(m_pPals->FileName(m_index));
	DoWork(W_PALS);
}

void CColorDlg::OnChangeText()
{
	UpdateData();
	UINT f = 0;
	if (m_bHSym)
		f |= NPAL_FLAG_HSYM;
	if (m_bVSym)
		f |= NPAL_FLAG_VSYM;
	if (m_bAlpha)
		f |= NPAL_FLAG_NORGB;
	m_pPals->Flags(m_index,f);
	DoWork(W_PALS);
}

void CColorDlg::SetModel(int which)
{
	char buf[10];
	m_nModel = which;
	if (m_nModel == 4)
		strcpy(buf,"Blue");
	else if (m_nModel == 3)
		strcpy(buf,"Green");
	else if (m_nModel == 2)
		strcpy(buf,"Red");
	else
		strcpy(buf,"Lum");
	GetDlgItem(IDC_CD_WHEEL)->Invalidate();
	GetDlgItem(IDC_CD_SLIDE_TEXT)->SetWindowText(buf);
	SetSlider();
}

void CColorDlg::SetColor(int which)
{
	m_nGradColor = which;
	GetColor(1);
}

void CColorDlg::SetPoint(int which)
{
	BOOL bRes = m_pPals->GetSetGrad(m_index,m_grad,0);
	ASSERT(bRes == 0);
	RECT rect;
	GetWindowRect(&rect);
	m_posx = rect.left;
	m_posy = rect.top;
	EndDialog(9+which);
}

void CColorDlg::SetGrad(int which)
{
	m_pPals->GetSetGrad(m_index,m_grad,0);
	m_grad.kind = which;
	m_pPals->GetSetGrad(m_index,m_grad,1);
	DoWork(W_COLOR | W_PALS);
	if (!which)
		{
		GetDlgItem(IDC_CD_GRAD_P1)->SetWindowText("Set Center");
		GetDlgItem(IDC_CD_GRAD_P2)->SetWindowText("Set Radius");
		}
	else
		{
		GetDlgItem(IDC_CD_GRAD_P1)->SetWindowText("Set Point 1");
		GetDlgItem(IDC_CD_GRAD_P2)->SetWindowText("Set Point 2");
		}
	m_pView->SetColorTrap(0,m_grad.kind,m_grad.p1,m_grad.p2);
}

void CColorDlg::SetKind(int which)
{
	//if (which == m_nKind)
	//	return;
	m_nKind = which;
	DoWork(W_KIND | W_PALS);
}

void CColorDlg::ApplyKind()
{
	BOOL bEnable;
	bEnable = m_nKind == 0 ? 1 : 0;

	bEnable = m_nKind == 1 ? 1 : 0;
	
	GetDlgItem(IDC_CD_GRAD_C1)->EnableWindow(bEnable);
	GetDlgItem(IDC_CD_GRAD_C2)->EnableWindow(bEnable);
	GetDlgItem(IDC_CD_GRAD_LIN)->EnableWindow(bEnable);
	GetDlgItem(IDC_CD_GRAD_RAD)->EnableWindow(bEnable);
	GetDlgItem(IDC_CD_GRAD_P1)->EnableWindow(bEnable);
	GetDlgItem(IDC_CD_GRAD_P2)->EnableWindow(bEnable);
	GetDlgItem(IDC_CD_GRAD_PP)->EnableWindow(bEnable);
	GetDlgItem(IDC_CD_GRAD_OR)->EnableWindow(bEnable);
	GetDlgItem(IDC_CD_GRAD_AND)->EnableWindow(bEnable);

	bEnable = m_nKind == 2 ? 1 : 0;
	UINT flags = m_pPals->Flags(m_index);
//	UINT show = (flags & NPAL_FLAG_HAS_ALPHA) ? SW_SHOW : SW_HIDE;
//	GetDlgItem(IDC_CD_TEXT_ALPHA)->ShowWindow(show);
	GetDlgItem(IDC_CD_TEXT_H)->EnableWindow(bEnable);
	GetDlgItem(IDC_CD_TEXT_V)->EnableWindow(bEnable);
	GetDlgItem(IDC_CD_TEXT_ALPHA)->EnableWindow(bEnable);
	GetDlgItem(IDC_CD_TXT_NAME)->EnableWindow(bEnable);
	GetDlgItem(IDC_CD_TEXT_NAME)->EnableWindow(bEnable);
	GetDlgItem(IDC_CD_TEXT_CHANGE)->EnableWindow(bEnable);


}
//
//	called whenever index, kind, grad, etc changes
//  calls set color
//
void CColorDlg::GetColor(BOOL bSet)
{
	if (bSet)
		m_pPals->Kind(m_index, m_nKind);
	else
		m_nKind = m_pPals->Kind(m_index);
	if (m_nKind == 2)
		{
		UINT f = m_pPals->Flags(m_index);
		m_bHSym = (f & NPAL_FLAG_HSYM) ? 1 : 0;
		m_bVSym = (f & NPAL_FLAG_VSYM) ? 1 : 0;
		m_bAlpha = (f & NPAL_FLAG_NORGB) ? 1 : 0;
		if (bSet && m_file_name)
			m_pPals->LoadTexture(m_index,m_file_name);
		else
			m_file_name = m_pPals->FileName(m_index);
//		m_crCurrent = m_pPals->Color(m_index);
		m_crCurrent = m_pPals->ColorRef(m_index);
		}
	else if (m_nKind == 1)
		{
		m_pPals->GetSetGrad(m_index,m_grad,0);
		if (m_nGradColor)
			m_crCurrent = m_grad.c2;
		else
			m_crCurrent = m_grad.c1;
		m_nGradKind = m_grad.kind;
		m_pView->SetColorTrap(0,m_grad.kind,m_grad.p1,m_grad.p2);
		}
	else 
		m_crCurrent = m_pPals->Color(m_index);
	m_opacity = 100 * (m_crCurrent >> 24) / 255;
	m_opac_slider.SetPos(m_opacity);
	m_crCurrent &= 0xffffff;
	m_red =   GetRValue(m_crCurrent);
	m_green = GetGValue(m_crCurrent);
	m_blue =  GetBValue(m_crCurrent);
	RGBtoHSL();
	SetSlider();
	UpdateData(0);
	GetDlgItem(IDC_CD_WHEEL)->Invalidate(0);
	GetDlgItem(IDC_CD_COLOR)->Invalidate(0);
	if (m_nKind == 1)
		m_pView->SetColorTrap(0,m_grad.kind,m_grad.p1,m_grad.p2);
	else
		m_pView->SetColorTrap();
}

void CColorDlg::SetSlider()
{
	int v;
	if (m_nModel == 4)
		v = m_blue;
	else if (m_nModel == 3)
		v = m_green;
	else if (m_nModel == 2)
		v = m_red;
	else
		v = (255 * m_lum + 120) / 240;
	m_lum_slider.SetPos(v);
}

void CColorDlg::NewColor(BOOL bHSL)
{
	if (bHSL)
		HSLtoRGB();
	else
		RGBtoHSL();
	m_crCurrent = RGB(m_red, m_green, m_blue);
	UpdateData(0);
	SetSlider();
	DoWork(W_RGB);
}

void CColorDlg::OnHScroll( UINT nCode, UINT nPos, CScrollBar* pScrollBar )
{
	CSliderCtrl * pSlider = (CSliderCtrl *)pScrollBar;
	CRect Rect;
	CPoint Point;
	GetCursorPos(&Point);
	int min, max;
	pSlider->GetRange(min, max);
	pScrollBar->GetClientRect(Rect);
	pScrollBar->ScreenToClient(&Point);
	int ww = Rect.right - Rect.left;
	int col = pScrollBar->GetDlgCtrlID();
	if (ww)
		{
		int z = ((Point.x - Rect.left) * (max - min) + ww / 2) / ww;
		z += min;
		pSlider->SetPos(z);
		}
	if (pSlider->GetDlgCtrlID() == IDC_CD_SLIDER)
//	if (pSlider->m_id == IDC_CD_SLIDER)
		{
		int v = pSlider->GetPos();
		if (m_nModel == 4)
			m_blue = v;
		else if (m_nModel == 3)
			m_green = v;
		else if (m_nModel == 2)
			m_red = v;
		else
			m_lum = (240 * v + 127) / 255;
		NewColor(m_nModel < 2 ? 1 : 0);
	GetDlgItem(IDC_CD_WHEEL)->Invalidate(0);
		}
	else if (pSlider->GetDlgCtrlID() == IDC_CD_OPACITY)
//	else if (pSlider->m_id == IDC_CD_OPACITY)
		{
		m_opacity = pSlider->GetPos();
		DoWork(W_OPAC);
		}
}

void CColorDlg::RGBtoHSL()
{
	int r = m_red;
	int g = m_green;
	int b = m_blue;
	//double f = 180.0 / 3.14159265;
	int ss,hh,ll;
  	int maxcol = max(r, max(g, b));
  	int mincol = min(r, min(g, b));
  	ll = (mincol + maxcol) / 2;   /* lightness */
  	if (maxcol == mincol)   /* achromatic case */
		{
    	ss = 0;         /* hue is undefined in the achromatic case */
    	hh = 0;
		}
	else
		{
		int delta = maxcol - mincol;
    	if (ll <= 128)
    		ss = 240 * delta / (maxcol + mincol);
    	else
    		ss = 240 * delta / (510 - maxcol - mincol);
//	   	int rc = 40 * (maxcol - r) / (maxcol - mincol);
//    	int gc = 40 * (maxcol - g) / (maxcol - mincol);
//   	int bc = 40 * (maxcol - b) / (maxcol - mincol);
    	if (r == maxcol)
     		hh = 40 * (g - b) / delta;
		else if (g == maxcol)
        	hh = 80 + 40*(b - r) / delta;
      	else 
        	hh = 160 + 40 *(r - g) / delta;
		hh = (hh + 240) % 240;
		}
	m_sat = ss;
	m_hue = hh;
	m_lum = (240 * ll + 127) / 255;
}

void CColorDlg::HSLtoRGB()
{
	int hue = m_hue;
	int sat = (255 * m_sat + 119) / 240;
	int lum = (255 * m_lum + 119) / 240;
	int rgb[3];
	int m1, m2;
	if (lum < 128)
		m2 = lum + (lum * sat) / 255;
	else
		m2 = lum + sat - (lum * sat) / 255;
	m1 = 2 * lum - m2;
	if (sat)
		{
		int i;
		for (i = 0; i < 3; i++)
			{
			int h = (hue + 240 + 80 * ( 1 - i)) % 240;
			if (h < 40)
				rgb[i] = m1 + (m2  - m1) * h / 40;
			else if (h < 120)
				rgb[i] = m2;
			else if (h < 160)
				rgb[i] = m1 + (m2 - m1) * (160 - h) / 40;
			else
				rgb[i] = m1;
			}
		}
	else
		rgb[0] = rgb[1] = rgb[2] = lum;

	m_red = rgb[0];
	m_green = rgb[1];
	m_blue = rgb[2];
}

void CColorDlg::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	CRect rect;
	CWnd * pWnd;
	ClientToScreen(&point);
	if (!(pWnd = GetDlgItem(IDC_CD_WHEEL)))
		return;
	pWnd->GetWindowRect(rect);
	if (!rect.PtInRect(point))
		return;
	if (SelectColor(point.x - rect.left, point.y - rect.top,
					rect.right - rect.left, rect.bottom - rect.top))
		return;
	NewColor(0);
	pWnd->Invalidate(0);
	SetCapture();
}

void CColorDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (this == GetCapture())
		{
		CRect rect;
		CWnd * pWnd;
		ClientToScreen(&point);
		if (!(pWnd = GetDlgItem(IDC_CD_WHEEL)))
			return;
		pWnd->GetWindowRect(rect);
		if (!rect.PtInRect(point))
			return;
		if (!SelectColor(point.x - rect.left, point.y - rect.top,
					rect.right - rect.left, rect.bottom - rect.top))
			{
			NewColor(0);
			}
		}
	CWnd::OnMouseMove(nFlags, point);
}


void CColorDlg::OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/)
{
	if (GetCapture() == this)
		ReleaseCapture();
}

BOOL CColorDlg::SelectColor(int x, int y, int w, int h)
{
	int rgb[3];
	int rem = 0;
	rgb[0] = rgb[1] = rgb[2] = 255;
	if (m_nModel)
		y = h - 1 - y;
	if (m_nModel > 1)
		{
		if (m_nModel == 2)
			{
			rgb[0] = m_red;
			rgb[1] = 255 * x / w;
			rgb[2] = 255 * y / h;
			}
		else if (m_nModel == 3)
			{
			rgb[1] = m_green;
			rgb[0] = 255 * x / w;
			rgb[2] = 255 * y / h;
			}
		else if (m_nModel == 4)
			{
			rgb[2] = m_blue;
			rgb[0] = 255 * x / w;
			rgb[1] = 255 * y / h;
			}
		}
	else
		{
		double f = 180.0 / 3.14159265;
		int mr;
		if (w > h)
			mr = 9 * h / 20;
		else
			mr = 9 * w / 20;
		int l = (255 * m_lum + 120) / 240;
		int cx = w / 2;
		int cy = h / 2;
		int r = (x - cx) * (x - cx) + (y - cy) * (y - cy);
		if (!m_nModel && (r >= mr * mr))
			return 1;
		int sat, hue;
		if (m_nModel == 1)
			{
			sat = 255 * y / h;
			hue = 360 + 360 * x / w;
			}
		else
			{
			double rr = sqrt((double)r);
			rem = (int)(255 * mr - 255 * rr);
			if (rem > 254) rem = 0;
			sat = (int)(255 * rr / (double)mr);
			double angle = f * atan2(double(y-cy),double(x-cx));
			hue = 360 + (int)angle;
			}
		int m1, m2;
		if (l < 128)
			m2 = l + (l * sat) / 255;
		else
			m2 = l + sat - (l * sat) / 255;
		m1 = 2 * l - m2;
		if (sat)
			{
			int i;
			for (i = 0; i < 3; i++)
				{
				int h = (hue + 120 * ( 1 - i)) % 360;
				if (h < 60)
					rgb[i] = m1 + (m2  - m1) * h / 60;
				else if (h < 180)
					rgb[i] = m2;
				else if (h < 240)
					rgb[i] = m1 + (m2 - m1) * (240 - h) / 60;
				else
					rgb[i] = m1;
				if (rem)
					rgb[i] = (rgb[i] * rem + 255 * (255 - rem)) / 255;
				}
			}
		}
	m_red = rgb[0];
	m_green = rgb[1];
	m_blue = rgb[2];
	return 0;
}

void CColorDlg::DoWork(UINT work)
{
	int update = 0;
	if (work & W_FLAGS)
		{
		work |= W_COLOR | W_PALS;
//	ChangeColor();
//	GetDlgItem(IDC_TEXTURE)->Invalidate();
		}
	if (work & W_KIND)
		{
		GetColor(1);
		ApplyKind();
		}
	if (work & W_RGB)
		{
		work |= W_NEW;
		}
	if (work & W_OPAC)
		{
		work |= W_NEW;
		update = 1;
		}
	if (work & W_LUM)
		{
		if (m_nModel < 2)
			HSLtoRGB();
		else
			RGBtoHSL();
		work |= W_NEW;
		update = 1;
		}
	if (work & W_NEW)
		{
		int o = (m_opacity * 255 + 50) / 100;
		if (m_nKind == 1)
			{
			if (m_nGradColor)
				m_grad.c2 = RGBA(m_red,m_green, m_blue,o);
			else
				m_grad.c1 = RGBA(m_red,m_green, m_blue,o);
			m_pPals->GetSetGrad(m_index,m_grad,1);
			}
		else if (m_nKind == 2)
			{
			m_pPals->Assign(m_index, m_red,m_green,m_blue, o,1);
//			m_pPals->NewAlpha(m_index,o);
			}
		else	
			m_pPals->Assign(m_index, m_red,m_green,m_blue, o);
		m_crCurrent = RGB(m_red,m_green,m_blue);
		work |= W_COLOR | W_PALS;
		}
	if (update == 1)
		UpdateData(0);
	else if (update == 2)
		UpdateData(1);
	if (work & W_COLOR)
		{
		GetDlgItem(IDC_CD_COLOR)->Invalidate(0);
		GetDlgItem(IDC_CD_COLOR)->UpdateWindow();
		GetDlgItem(IDC_CD_WHEEL)->Invalidate(0);
		GetDlgItem(IDC_CD_WHEEL)->UpdateWindow();
		}
	if (work & W_PALS)
		m_pPalette->UpdatePals();
}

void CColorDlg::OnRGBChange() 
{
	m_red = min(GetDlgItemInt(IDC_CD_RED),255);
	m_green = min(GetDlgItemInt(IDC_CD_GREEN),255);
	m_blue = min(GetDlgItemInt(IDC_CD_BLUE),255);
	NewColor(0);
}


void CColorDlg::OnHSLChange() 
{
	m_hue = min(GetDlgItemInt(IDC_CD_HUE),240);
	m_sat = min(GetDlgItemInt(IDC_CD_SAT),240);
	m_lum = min(GetDlgItemInt(IDC_CD_LUM),240);
	NewColor(1);
}

void CColorDlg::OnOpacChange() 
{
	m_opacity = min(GetDlgItemInt(IDC_CD_OPAC),100);
	m_opac_slider.SetPos(m_opacity);
	DoWork(W_OPAC);
}

void CColorDlg::OnRevert()
{
	DPF("do revert");
	m_pPals->RestEntry(1);
	GetColor(0);
	DoWork(W_NEW);
}

LRESULT CColorDlg::OnKickIdle(WPARAM, LPARAM)
{ 
	DPF("kicking");
	return ((CMainFrame *)m_pPalette->GetParent())->IdleProcess();
    return TRUE;
}

