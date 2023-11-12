#include "stdafx.h"
#include "sketch.h"
#include "dialogs.h"
#include "tooldlg.h"
#include "ctoolbox.h"
#include "mydoc.h"
#include "myview.h"


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define CYCAPTION 9     /* height of the caption */
#define EXTRA 0
#define HEIGHT 50
#define TIMER_ID 15
#define TIMER_DELAY 100

#define KIND_PENCIL 0
#define KIND_TRACE  1
#define KIND_BRUSH  2
#define KIND_FILL   3
#define KIND_ZOOM   4
#define KIND_HAND   5
#define KIND_EYEDROP  6
#define KIND_SELECT  8

#define KFLG_ERASE 128
#define FLAG_RADIUS 4
#define FLAG_DENSITY 8
#define FLAG_P_ANTI  16
#define FLAG_E_ANTI  1024
#define MASK_KIND  15
#define MASK_TEXT  64 | 32

CToolDlg::CToolDlg()//CWnd* pParent /*=NULL*/)
	: CDialog()
{     
	DPF("tooldlg construct");
	m_nDocked = 0;
//	m_pDoc = 0;
//	m_height = 1;
//	m_width = 1;
	//{{AFX_DATA_INIT(CToolDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CToolDlg::~CToolDlg()
{
	DPF("tooldlg destruct");
	DestroyWindow();
}
void CToolDlg::PostNcDestroy() 
{
	DPF("Post NC destroy");
    CDialog::PostNcDestroy();
    delete this;
}

BOOL CToolDlg::MyCreate( CWnd* pOwnerWnd, CSketchDoc * pDoc)//, int x, int y)
{
	m_pDoc = pDoc;
	m_pOwnerWnd = pOwnerWnd;
	m_which = 0;
	int idd;
	BOOL bErase = 0;
	if (m_kind & 256)
		m_kind = KIND_BRUSH;
	char buf[40];
	sprintf(buf,"%d",m_kind);
	switch (m_kind & MASK_KIND) {
	case KIND_PENCIL:
		idd = IDD_TOOL_RADDEN;
		strcpy(buf, "Pencil");
		bErase = m_kind & KFLG_ERASE;
		break;
	case KIND_TRACE:
		idd = IDD_TOOL_RAD;
		strcpy(buf, "Trace");
		bErase = m_kind & KFLG_ERASE;
		break;
	case KIND_BRUSH:
		idd = IDD_TOOL_RAD;
		strcpy(buf, "Brush");
		bErase = m_kind & KFLG_ERASE;
		break;
	case KIND_FILL:
		idd = IDD_TOOL_FILL;
		strcpy(buf, "Fill");
		bErase = m_kind & KFLG_ERASE;
		break;
	case KIND_ZOOM:
		idd = IDD_TOOL_ZOOM;
		strcpy(buf, "Zoom");
		break;
	case KIND_HAND:
		idd = IDD_TOOL_ZOOM;
		strcpy(buf, "Hand");
		break;
	case KIND_EYEDROP:
		idd = IDD_TOOL_ZOOM;
		strcpy(buf, "Eyedropper");
		break;
	case KIND_SELECT:
		idd = IDD_TOOL_SELECT;
		strcpy(buf, "Select");
		break;
	default:
		idd = IDD_TOOL_ZOOM;
		strcpy(buf, "Unknown");
		break;
	}
	if (bErase)
		strcat(buf," - Erasing");
//	m_list.m_pSheet = this;
	m_which = 0;
	BOOL bResult = Create(idd, pOwnerWnd);
//	ShowWindow(SW_HIDE);
	DPF("tooldlg,create:%d",bResult);
//	DPF("tooldlg w:%d,h:%d",m_width,m_height);
//	SetPosition();
	SetWindowText(buf);
	m_currad = m_pDoc->Radius();
	m_minrad = m_pDoc->Radius(1000);
	m_maxrad = m_pDoc->Radius(2000);
	m_curden = m_pDoc->Density();
	m_minden = m_pDoc->Density(1000);
	m_maxden = m_pDoc->Density(2000);
	SetDlgItemInt(IDC_DEN_VAL,m_curden);
	SetDlgItemInt(IDC_RAD_VAL,m_currad);

#if MAC
	ShowWindow(SW_SHOW);
#endif

	return bResult;
}

void CToolDlg::OnClose()
{
	DPF("On ToolDlg Close");
	((ctoolbox *)m_pOwnerWnd)->CloseDialog(1);
	DPF("after close");
	DestroyWindow();
//	delete this;
}

void CToolDlg::OnMove(int x, int y)
{
DPF("tool dlg move, x:%d,y:%d",x,y);
	RECT rc1;
	GetWindowRect(&rc1);
	RECT rc;
	m_pOwnerWnd->GetWindowRect(&rc);
	int a = GetSystemMetrics(SM_CYSMCAPTION);
//	int ddx = GetSystemMetrics(SM_CXFIXEDFRAME);
//	y -= a;
//	x -= ddx;
	x = rc1.left;
	y = rc1.top;
	int xx = rc.left;
	int yy = rc.bottom;
	int dx = x - xx;
	int dy = y - yy;
	int dist = dx * dx + dy * dy;
	DPF("x:%d,y:%d",x,y);
DPF("tool dlg move, xx:%d,yy:%d,dist:%d",xx,yy,dist);
	if (dist && (dist < 315))
		{
		m_nDocked = 1;
		DPF("setting,%d,%d",xx,yy);
		SetWindowPos(NULL, xx-2, yy+2, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
		return;
		}
	xx = rc.right;
	yy = rc.top - a - 2;
	dx = x - xx;
	dy = y - yy;
	dist = dx * dx + dy * dy;
	DPF("x:%d,y:%d",x,y);
DPF("tool dlg move, xx:%d,yy:%d,dist:%d",xx,yy,dist);
	if (dist && (dist < 315))
		{
		m_nDocked = 2;
		DPF("setting,%d,%d",xx,yy);
		SetWindowPos(NULL, xx+2, yy, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
		return;
		}
	m_nDocked = 0;
}

void CToolDlg::AdjustTextures(BOOL bAnti)
{
	if ((m_kind & 15) != KIND_PENCIL)
		return;
	UINT show = bAnti ? SW_HIDE : SW_SHOW;
	GetDlgItem(IDC_DEN_TEXT0)->ShowWindow(show);
	GetDlgItem(IDC_DEN_TEXT1)->ShowWindow(show);
	GetDlgItem(IDC_DEN_TEXT2)->ShowWindow(show);
	GetDlgItem(IDC_DEN_TEXT3)->ShowWindow(show);

}

BOOL CToolDlg::OnInitDialog()
{
	DPF("tooldlg Init,kind:%d",m_kind);
	CDialog::OnInitDialog();
	int sflags = m_pDoc->Option(SELECT_FLAGS);
	CSketchView * pView = m_pDoc->GetDocView();
	UINT count,z,zz,s,m;
	switch (m_kind & MASK_KIND) {
	case KIND_SELECT:
	
		CheckDlgButton(IDC_SELECT_STAY, sflags & 1 ? 1 : 0);
		CheckDlgButton(IDC_SELECT_AUTO, sflags & 2 ? 1 : 0);
		CheckDlgButton(IDC_SELECT_LASSO,sflags & 4 ? 1 : 0);
		break;
	case KIND_FILL:
		count = pView->FillStack(-1);	
		m_FillScroll.SetRange(0,count,TRUE);
		m_FillScroll.SetPos(count);
		CheckDlgButton(IDC_FILL_CHECK,count != 0);
		GetDlgItem(IDC_FILL_SCROLL)->EnableWindow(count != 0);
		break;
	case KIND_PENCIL:
		s = (m_kind & KFLG_ERASE) ? 8 : 5;
		m = (m_kind & KFLG_ERASE) ? FLAG_E_ANTI : FLAG_P_ANTI;
		zz = m_pDoc->ToolInfo(5);
		z = ((zz / 32) & 4) + (zz & 3);
		CheckRadioButton(IDC_PEN_FREE,IDC_PEN_SMART,IDC_PEN_FREE+z);
		CheckDlgButton(IDC_PEN_TAB_RAD,zz & 4 ? 1 : 0);
		CheckDlgButton(IDC_PEN_TAB_DEN,zz & 8 ? 1 : 0);
		CheckRadioButton(IDC_DEN_TEXT0,IDC_DEN_TEXT3,
				IDC_DEN_TEXT0+((zz >> s) & 3));
		CheckDlgButton(IDC_RAD_ANTI,(zz & m) ? 0 : 1);
		AdjustTextures(zz & m);
		break;
	case KIND_BRUSH:
		zz = m_pDoc->ToolInfo(5);
		m = (m_kind & KFLG_ERASE) ? FLAG_E_ANTI : FLAG_P_ANTI;
		CheckDlgButton(IDC_RAD_ANTI,(zz & m) ? 0 : 1);
		break;
	default:
		break;
	}

#ifdef FLIPBOOK_MAC
	// items won't draw until they have been fetched at least once
	GetDlgItem (IDC_RAD);
	GetDlgItem (IDC_RAD_UP);
	GetDlgItem (IDC_RAD_DN);
	GetDlgItem (IDC_DEN);
	GetDlgItem (IDC_DEN_DN);
	GetDlgItem (IDC_DEN_UP);
	Invalidate();
#endif
	return FALSE;
}

void CToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	if ((m_kind & MASK_KIND) == KIND_FILL)
		DDX_Control(pDX, IDC_FILL_SCROLL, m_FillScroll);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CToolDlg, CDialog)
	//{{AFX_MSG_MAP(CToolDlg)
//	ON_WM_SIZE()
	ON_WM_DRAWITEM()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_COMMAND(IDC_PEN_FREE,OnPenFree)
	ON_COMMAND(IDC_PEN_LINE,OnPenLine)
	ON_COMMAND(IDC_PEN_RECT,OnPenRect)
	ON_COMMAND(IDC_PEN_ELLIPSE,OnPenEllipse)
	ON_COMMAND(IDC_PEN_CURVE,OnPenCurve)
	ON_COMMAND(IDC_PEN_SMART,OnPenSmart)
	ON_COMMAND(IDC_PEN_TAB_RAD,OnPenTabRad)
	ON_COMMAND(IDC_PEN_TAB_DEN,OnPenTabDen)
	ON_COMMAND(IDC_SELECT_STAY,OnSelectStay)
	ON_COMMAND(IDC_SELECT_AUTO,OnSelectAuto)
	ON_COMMAND(IDC_SELECT_LASSO,OnSelectLasso)
	ON_COMMAND(IDC_RAD_ANTI,OnRadAnti)
	ON_COMMAND(IDC_DEN_TEXT0,OnText0)
	ON_COMMAND(IDC_DEN_TEXT1,OnText1)
	ON_COMMAND(IDC_DEN_TEXT2,OnText2)
	ON_COMMAND(IDC_DEN_TEXT3,OnText3)
	ON_WM_CLOSE()
	ON_WM_MOVE()
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CToolDlg::OnDrawItem(int nId, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
//	WORD sw;
	if (lpDrawItemStruct->itemAction != ODA_DRAWENTIRE)	
		return;

	DPF("DrawItem:%d",nId);
	CWnd * pWnd = GetDlgItem(nId);
	if (!pWnd) return;
//	CDC* pDC = pWnd->GetDC();
//	ASSERT(pDC != NULL);
	CWindowDC appDC(pWnd);
	CRect rect;
	pWnd->GetClientRect(rect);
	int w = rect.Width();
	int h = rect.Height();
	int x =	rect.TopLeft().x;
	int	y = rect.TopLeft().y;
	CBrush br;
	CBrush br2;
	br.CreateStockObject(DKGRAY_BRUSH);
	appDC.FrameRect(rect,&br);
	DPF("w:%d,h:%d",w,h);
	switch(nId) {
	case IDC_RAD:
		br2.CreateStockObject(GRAY_BRUSH);
		rect.top++;
		rect.bottom--;
		rect.left++;
		rect.right = x + MulDiv(w,CurRad() - MinRad(), MaxRad()-MinRad());
		appDC.FillRect(rect,&br2);
		appDC.MoveTo(x , y + h / 2);
		appDC.LineTo(x + w, y);
		appDC.MoveTo(x , y + h / 2);
		appDC.LineTo(x + w, y + h);
		break;
	case IDC_RAD_UP:
		appDC.Ellipse(rect);
		break;
	case IDC_RAD_DN:
		rect.InflateRect(-7,-7);
		appDC.Ellipse(rect);
		break;
	case IDC_DEN:
		br2.CreateStockObject(GRAY_BRUSH);
		rect.top++;
		rect.bottom--;
		rect.left++;
		rect.right = x + MulDiv(w,CurDen() - MinDen(), MaxDen()-MinDen());
		appDC.FillRect(rect,&br2);
		appDC.MoveTo(x , y + h);
		appDC.LineTo(x + w, y);
		break;
	case IDC_DEN_DN:
		br2.CreateStockObject(WHITE_BRUSH);
		appDC.FillRect(rect,&br2);
		break;
	case IDC_DEN_UP:
		br2.CreateStockObject(BLACK_BRUSH);
		appDC.FillRect(rect,&br2);
		break;
	default:
		DPF("draw unknown:%d",nId);
		break;
	}
}

int CToolDlg::IdFromPoint(CPoint pt)
{
	int ids[] ={IDC_RAD_DN,IDC_RAD_UP,IDC_RAD,
				IDC_DEN_DN,IDC_DEN_UP,IDC_DEN,0};
	ClientToScreen(&pt);
//	DPF("pt:%d,%d",pt.x,pt.y);
	int i;
	for (i = 0;ids[i];i++)
		{
		CWnd * pWnd;
		if (!(pWnd = GetDlgItem(ids[i])))
			continue;
		CRect rect;
		pWnd->GetWindowRect(rect);
//DPF("i:%d,id:%d,x:%d,y:%d",i,ids[i],rect.left,rect.top);
		if (rect.PtInRect(pt))
			break;
		}
	return ids[i];
}

void CToolDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (this == GetCapture())
		{
		int nNewId = IdFromPoint(point);
		if (nNewId == m_Id)
			return;
		if (m_Id)
			{
			KillTimer(TIMER_ID);
			m_Id = 0;
			}
		}

	CWnd::OnMouseMove(nFlags, point);
}

void CToolDlg::FocusIt()
{
	//((ctoolbox *)m_pOwnerWnd)->FocusFrame();
}

void CToolDlg::OnLButtonUp(UINT /*nFlags*/, CPoint point)
{
	Capture(0);
	DPF("which:%d,r:%d,d:%d",m_which, m_currad,m_curden);
	if (!m_which)
		m_pDoc->Radius(m_currad);
	else
		m_pDoc->Density(m_curden);
	FocusIt();
#ifdef FLIPBOOK_MAC
	// force an erase/redraw here to make things look clean
	int nId = IdFromPoint(point);
	CWnd* w = GetDlgItem (nId);
	if (w) {
		w->Invalidate();
	}
#endif
}

void CToolDlg::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
//		CWnd * pWnd;
		CRect rect;
	int z;
	WORD Min,Max;
	int nId = IdFromPoint(point);
DPF("lb down,id:%d",nId);
	switch (nId)
	{
	case IDC_RAD_UP:
		m_Inc = 1;
		m_which = 0;
		break;
	case IDC_RAD_DN:
		m_Inc = -1;
		m_which = 0;
		break;
	case IDC_DEN_UP:
		m_Inc = 1;
		m_which = 1;
		break;
	case IDC_DEN_DN:
		m_Inc = -1;
		m_which = 1;
		break;
	case IDC_DEN:
		CWnd * pWnd;
		if (!(pWnd = GetDlgItem(nId)))
			{
			nId = 0;
			break;
			}
		ClientToScreen(&point);
		pWnd->GetWindowRect(rect);
		z = point.x - rect.left;
		Min = MinDen();
		Max = MaxDen();
		m_which = 1;
DPF("den,min:%d,max:%d",Min,Max);
		m_curden = Min + MulDiv(Max - Min,z,rect.right - rect.left);
		SetDlgItemInt(IDC_DEN_VAL,m_curden);
		GetDlgItem(IDC_DEN)->Invalidate();
		nId = 0;
		break;
	case IDC_RAD:
		if (!(pWnd = GetDlgItem(nId)))
			{
			nId = 0;
			break;
			}
		ClientToScreen(&point);
		pWnd->GetWindowRect(rect);
		z = point.x - rect.left;
		Min = MinRad();
		Max = MaxRad();
		m_which = 0;
DPF("rad,min:%d,max:%d",Min,Max);
		m_currad = Min + MulDiv(Max - Min, z,rect.right - rect.left);
		SetDlgItemInt(IDC_RAD_VAL,m_currad);
		GetDlgItem(IDC_RAD)->Invalidate();
		nId = 0;
		break;
	default:
		nId = 0;
	}
	if (nId)
		{
		MoveIt();
		Capture(nId);
		}
}

void CToolDlg::OnTimer(UINT nTimerID)
{
	if (nTimerID == TIMER_ID)
		{
		MoveIt();
		}
}

void CToolDlg::MoveIt()
{
	if (!m_which)
		{
		int old = CurRad();
		int nu = old + m_Inc;
		if (nu > (int)m_maxrad) nu = m_maxrad;
		if (nu < (int)m_minrad) nu = m_minrad;
		if (nu == old)
			m_Inc = 0;
		else
			{
			SetDlgItemInt(IDC_RAD_VAL,m_currad=nu);
			GetDlgItem(IDC_RAD)->Invalidate();
			}
		DPF("rad:%d",nu);
		}
	else 
		{
		int old = CurDen();
		int nu = old + 5 * m_Inc;
		if (nu > (int)m_maxden) nu = m_maxden;
		if (nu < (int)m_minden) nu = m_minden;
		if (nu == old)
			m_Inc = 0;
		else
			{
			SetDlgItemInt(IDC_DEN_VAL,m_curden=nu);
			GetDlgItem(IDC_DEN)->Invalidate();
			}
		DPF("den:%d",nu);
		}
	if (!m_Inc)
		{
		DPF("bad move");
		Capture(0);
		return;
		}
}

void CToolDlg::Capture(int nId)
{
	if (nId)
		{
		DPF("capture:%d",nId);
		SetCapture();
		m_Id = nId;
		VERIFY(SetTimer(TIMER_ID, TIMER_DELAY, NULL) == TIMER_ID);
		}
	else if (m_Id)
		{
		DPF("capture:%d",nId);
		ReleaseCapture();
		KillTimer(TIMER_ID);
		m_Id = 0;
		}
}


void CToolDlg::SetPen(int v)
{
	DPF("set pen:%d",v);
	int z = m_pDoc->ToolInfo(5);
	z |= 3 + 128;
	z ^= 3 + 128;
	z |= 32 * (v & 4) + (v & 3);
	m_pDoc->ToolInfo(6,z);
	FocusIt();
}

void CToolDlg::SetText(int v)
{
	DPF("set pen:%d",v);
	int z = m_pDoc->ToolInfo(5);
	int s;
	if (m_kind & KFLG_ERASE)
		s = 8;
	else
		s = 5;
	v = (v & 3) << s;
	int q = 3 << s;
	z |= q;
	z ^= q;
	z |= v;
	m_pDoc->ToolInfo(6,z);
	FocusIt();
}

void CToolDlg::SetPenTab(int v)
{
	DPF("set pen:%d",v);
	UINT m = (m_kind & KFLG_ERASE) ? FLAG_E_ANTI : FLAG_P_ANTI;
	int z = m_pDoc->ToolInfo(5);
	if (v == 1)
		z ^= FLAG_DENSITY;
	else if (v == 0)
		z ^= FLAG_RADIUS;
	else if (v == 2)
		z ^= m;
	m_pDoc->ToolInfo(6,z);
	if ((v == 2) && ((m_kind & MASK_KIND) == KIND_PENCIL))
		AdjustTextures(z & m);
	FocusIt();
}

void CToolDlg::SelectFlags(int v)
{
	int z = m_pDoc->Option(SELECT_FLAGS);
	z ^= v;
	m_pDoc->Option(SELECT_FLAGS,1,z);
	FocusIt();
}

void CToolDlg::OnHScroll( UINT nCode, UINT nPos, CScrollBar* pScrollBar )
{
	CSliderCtrl * pSlider = (CSliderCtrl *)pScrollBar;
	UINT pos = pSlider->GetPos();
	DPF("hscroll, code:%d, pos:%d,act:%d",nCode, nPos,pos);
	CSketchView * pView = m_pDoc->GetDocView();
//	if (nCode == 8)
	pView->FillStack(pos);	
	if (nCode == 8)
		FocusIt();
//#ifdef FLIPBOOK_MAC
	pView->Invalidate();
//#endif
}

