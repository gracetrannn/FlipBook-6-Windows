#include "stdafx.h"
#include "sketch.h"
#include "mylib.h"
#include "mainfrm.h"
#include "mydoc.h"
#include "myview.h"
#include "cscene.h"
#include "nscaler.h"
#include "zlib.h"
#include "dialogs.h"
#include "ccell.h"
#include "cnewpals.h"
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

typedef struct {
	char Id[16];
	UINT version;
	UINT owidth;		//scene width and height
	UINT oheight;
	UINT flags;
	UINT frames;
	UINT levels;
} LIBHEADER;

typedef struct {
	UINT x;
	UINT y;
	UINT w;
	UINT h;
	BYTE pals[1024];
} LEVELHEADER;

#define FLAG_ASPECT 256
#define FLAG_MANIPULATE 512
#define FLAG_APPLY_NAME 1024
#define FLAG_FREEFORM 2048
#define FLAG_SHOW_GRID 4096
#define FLAG_SHOW 8192
#define FLAG_MATCH_COLOR 16384
#define FLAG_AUTO_LOAD 32768


#define TIMER_ID 15
#define TIMER_DELAY 100

void extern PutInt(LPCSTR section, LPCSTR key, int v);
int  extern GetInt(LPCSTR section, LPCSTR key, int def);

CMyLibDlg::CMyLibDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMyLibDlg::IDD, pParent)
{     
	DPF("mylib construct");
	m_pDoc = 0;
	m_pScene = 0;
	m_pBufs = 0;
	m_pCur = 0;
	m_pTmp = 0;
	m_yoffset = 0;
	m_curw = 10;
	m_curh = 10;
	m_levels = 0;
	m_desired_width = 10;
	m_nPaint = 0; // 1 is sizing, 2 is force paint
	m_bPainted = 0;
	m_oldw = 0;
	m_oldh = 0;
}

CMyLibDlg::~CMyLibDlg()
{
	DPF("mylib destruct");
	if (m_pBufs)
		{
		UINT i;
		for (i = 0; i < m_levels; i++)
			delete [] m_pBufs[i];
		delete [] m_pBufs;
		}
	m_pBufs = 0;
	delete [] m_pCur;
	m_pCur = 0;
	delete [] m_pTmp;
	m_pTmp = 0;
}

void CMyLibDlg::LibName(CString & name)
{
	name = m_name;
}

void CMyLibDlg::FileName(CString & name)
{
	name = m_filename;
}

void CMyLibDlg::SetName(LPCSTR filename)
{
	m_filename = filename;
	char temp[300];
	int i,j;
	j = 0;
	LPCSTR p = (LPCSTR)m_filename;
	i = strlen(p);
	for (;i--;)
		{
		if (p[i] == NATIVE_SEP_CHAR)
			break;
		if (p[i] == '.')
			temp[i] = 0;
		else
			temp[i] = p[i];
		}
	if (i >= 0)
		i++;
	else
		i = 0;
	m_name = temp + i;
}

UINT CMyLibDlg::GetInfo()
{
	return m_flags;
}

void CMyLibDlg::SetInfo(UINT v)
{
	if (m_flags & FLAG_FREEFORM)
		{
		if (!(v & FLAG_FREEFORM))
			m_curw = 0;
		else if ((v & 255) != m_nColumns)
			m_curw = 0;
		}
	else if (v & FLAG_FREEFORM)
		{
		m_curw = 0;
		}
	m_flags = v;
	SetRows();
}

BOOL CMyLibDlg::Switch(BOOL bSwitch)
{
	if (bSwitch)
		{
		m_flags ^= FLAG_SHOW;
		ShowWindow((m_flags & FLAG_SHOW) ? SW_SHOW : SW_HIDE);
		}
	return (m_flags & FLAG_SHOW) ? 1 : 0;
}

BOOL CMyLibDlg::Load( CWnd* pOwnerWnd, LPCSTR name)
{
	if (ReadFile(name))
		return 1;
	SetName(name);
	BOOL bResult = Create(CMyLibDlg::IDD, pOwnerWnd);
	ShowWindow(SW_HIDE);
	DPF("create:%d",bResult);
	return bResult ? 0 : 1;
}

BEGIN_MESSAGE_MAP(CMyLibDlg, CDialog)
	//{{AFX_MSG_MAP(CMyLibDlg)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_CLOSE()
	ON_WM_SYSCOMMAND()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CMyLibDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
#ifndef FLIPBOOK_MAC // FIXME 6
	if ((nID == ID_LIB_SETTINGS) || (nID == SC_CONTEXTHELP))
		LibSettings();
	else
		CWnd::OnSysCommand(nID, lParam);
#endif
}

void CMyLibDlg::OnClose()
{
	DPF("On MyLib  Close");
	m_flags |= FLAG_SHOW;
	m_flags ^= FLAG_SHOW;	// flag off
	ShowWindow(SW_HIDE);
}

void CMyLibDlg::OnCancel() 
{
	DPF("on cancel");
//	CDialog::OnCancel();
}

void CMyLibDlg::OnTimer(UINT nTimerID)
{
	if (nTimerID == TIMER_ID)
		{
		if (GetAsyncKeyState(VK_LBUTTON) != 0)
			return;			// hopefully still sizing
		m_nPaint = 2;
DPF("on timer,w :%d,h:%d",m_neww, m_newh);
		if ((m_newh < 10) || ( m_neww < 10))
			{
			ReCalcWindow(0);
			Invalidate(0);
			}
		else
			{
			Invalidate(0);
			KillTimer(TIMER_ID);
			}
		}
}


BOOL CMyLibDlg::OnInitDialog() 
{
	DPF("mylib init");
	CDialog::OnInitDialog();
	SetWindowText(m_name);
	m_yoffset = 0;
#if 0 // def FLIPBOOK_MAC
	m_PaletteView.SubclassDlgItem (1, this);
	m_ScrollBar.SubclassDlgItem (GetWindowScrollerControl (SB_VERT), this);
#endif
		CRect Crect;
		CRect Wrect;
		GetClientRect(Crect);
		ClientToScreen(Crect);
		GetWindowRect(&Wrect);
		m_ety = Crect.top - Wrect.top;
		m_eby = Wrect.bottom - Crect.bottom;
		m_ex =  Crect.left - Wrect.left;
		m_nPaint = 2;	// force a recalc
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
	CMenu * pSysMenu = GetSystemMenu(FALSE);
	pSysMenu->AppendMenu(MF_SEPARATOR, 0,"");
	pSysMenu->AppendMenu(MF_BYPOSITION, ID_LIB_SETTINGS, "&Settings");

	return TRUE;
}

BOOL CMyLibDlg::Setup(CSketchDoc * pDoc, BOOL bForce)
{
	DPF("mylibc init");
	m_pDoc = pDoc;
	m_pScene = m_pDoc->GetDocScene();
//bForce = 1;
	if (bForce)
		{
		SetRows();
		CRect rect;
		GetParent()->GetWindowRect(&rect);
		int x,y,w,h,z;
		if (m_nColumns > m_nRows)
			{
			w = rect.right - rect.left;
			z = w / m_nColumns;
			h = z * m_srch / m_srcw;
			h *= m_nRows;
			x = rect.left;
			y = (rect.top + rect.bottom - h) / 2;  
			}
		else
			{
			h = rect.bottom - rect.top;
			z = h / m_nRows;
			w = z * m_srcw / m_srch;
			w *= m_nColumns;
			y = rect.top;
			x = (rect.right + rect.left - w) / 2;  
			}
		m_flags |= FLAG_SHOW;
		m_nPaint = 2;  // force a recalc
		SetWindowPos(NULL, x,y,w,h, 0);
		}
	ShowWindow((m_flags & FLAG_SHOW) ? SW_SHOW : SW_HIDE);
	return 0;
}

BOOL CMyLibDlg::DoThumb(CDC & dc, LPRECT rect, UINT xx, UINT yy)
{
	UINT index = Index(xx,yy);
	if (index >= m_nCells)
		{
		COLORREF crBox;
		crBox = RGB(192,192,192);
		dc.FillSolidRect(rect, crBox);
		return 0;
		}

//	COLORREF crBox;
//	crBox = RGB(192,255,192);
//	dc.FillSolidRect(rect, crBox);
//	return 1;


//DPF("thumb,x:%d,y:%d,z:%d",xx,yy,index);
	LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)(m_pCur);
	BYTE * pSrc = m_pCur + 40 + index * m_curh * 4 * ((3*m_curw+3)/4);
	int x,y,w,h;
	int dx, dw;
	dx = 0;
	x = rect->left;
	y = rect->top;
	w = rect->right - x;
	h = rect->bottom - y;
	if ((UINT)w > m_curw)
		dw = w = m_curw;
	else
		{
		dw = w;
		dx = (m_curw - w) / 2;
		}
	BOOL bSuccess = ::SetDIBitsToDevice(dc.m_hDC,x,y,w,h,
			dx, m_curh - h,
			0, m_curh, pSrc,
			(LPBITMAPINFO)lpBI, DIB_RGB_COLORS);
	UINT level = index / m_frames;
	UINT frame = index % m_frames;
	BYTE * pName = m_pBufs[level];
	LEVELHEADER * plh = (LEVELHEADER *) pName;
	pName += sizeof(LEVELHEADER);
	pName += frame * (16 + 4 * plh->w * plh->h);
	char buf[32];
	strcpy(buf, (LPCSTR)pName);
	COLORREF crOld,crNew;
	UINT nFormat = DT_SINGLELINE | DT_RIGHT | DT_BOTTOM;
	crOld = dc.GetTextColor();
    crNew = RGB(255,0,0);
    dc.SetTextColor(crNew);
	int q = rect->top;
	rect->top = rect->bottom - 20;
	dc.SetBkMode(TRANSPARENT);
	dc.DrawText(buf,-1, rect, nFormat);
	dc.SetTextColor(crOld);
	rect->top = q; // restore it

	return 1;
}

void CMyLibDlg::SetRows()
{
	if (m_flags & FLAG_FREEFORM)
		{
		m_nColumns = m_flags & 255;
		if (!m_nColumns) m_nColumns = m_frames;
		m_nRows = (m_nCells + m_nColumns - 1) / m_nColumns;
		}
	else
		{
		m_nColumns = m_levels;
		m_nRows = m_nCells / m_levels;
		}
}

void CMyLibDlg::OnPaint() 
{
	CPaintDC dc(this);
#if MAC
	dc.Clear();
#endif
DPF("npaint:%d", m_nPaint);
	if (m_nPaint == 1)	// sizing
		{
		m_nPaint = 0;
		RECT rectClip;
		int q = dc.GetClipBox(&rectClip);
		if (q == ERROR) return;
		COLORREF crBack;
		crBack = RGB(255,255,255);
		dc.FillSolidRect(&rectClip, crBack);
		return;
		}
	m_nPaint = 0;
	if (ReCalcWindow(0))
		return;
	if (m_desired_width != m_curw)
		ReScale();
//DPF("paint,boxw:%d,boxh:%d,lines:%d,cols:%d",m_curw,m_curh,m_lines,m_nColumns);
	CBrush br;
	br.CreateStockObject(LTGRAY_BRUSH);
	RECT zrect;
	UINT x,y;
//COLORREF crBox;
	m_bPainted = 1;
	for (y = 0; y < m_lines; y++)
		{
		for (x = 0; x < m_nColumns; x++)
			{
			zrect.top = y * m_curh;
			zrect.bottom = zrect.top + m_curh;
			zrect.left = x * m_curw;
			zrect.right = zrect.left + m_curw;
			if (DoThumb(dc,&zrect,x,y + m_yoffset))
				{
				if (m_flags & FLAG_SHOW_GRID)
					dc.FrameRect(&zrect,&br);
				}
			}
		}
	GetParent()->SetFocus();
}

void CMyLibDlg::OnVScroll(UINT code , UINT pos , CScrollBar* pScrollBar)
{
	DPF("pal scroll,code:%d,pos:%d,off:%d",code,pos,m_yoffset);
	CRect rect;
	GetClientRect(rect);
	UINT lines = (rect.bottom - rect.top) / m_curh;
//	int boxw = (rect.right - rect.left) / m_columns;
//	int boxh = boxw * m_srch / m_srcw;
	//UINT lines = (rect.bottom - rect.top + boxh - 1) / boxh;
	switch (code) {
	case SB_PAGEUP:
		if (m_yoffset > lines)
			m_yoffset -= lines;
		else
			m_yoffset = 0;
		break;
	case SB_LINEUP:
		if (m_yoffset)
			m_yoffset--;
		break;
	case SB_PAGEDOWN:
		if ((m_yoffset + lines) <= m_nRows)
			m_yoffset += lines;
		else
			m_yoffset = m_nRows - lines;
		break;
	case SB_LINEDOWN:
		m_yoffset++;
		break;
	case SB_TOP:
		m_yoffset = 0;
		break;
	case SB_BOTTOM:
		m_yoffset = m_nRows - lines;
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
	DPF("lines:%d,off:%d",lines,m_yoffset);
	if ((lines + m_yoffset) > m_nRows)
		m_yoffset = m_nRows - lines;
	DPF("newoff:%d,scpos:%d",m_yoffset,GetScrollPos(SB_VERT));
	if (m_yoffset != (UINT)GetScrollPos(SB_VERT))
		{
DPF("new pos:%d",m_yoffset);
		SetScrollPos(SB_VERT, m_yoffset, FALSE);
		InvalidateRect(NULL, TRUE);
		}
//	SetFocus(hwndMain);
}


BOOL CMyLibDlg::ReCalcWindow(BOOL bUseClient)
{
	if (bUseClient)
		{
		CRect rect;
		GetClientRect(rect);
		m_winw = rect.right - rect.left;
		m_winh = rect.bottom - rect.top;
		}
	else
		{
DPF("old:%d,%d, new:%d,%d",m_oldw,m_oldh,m_neww, m_newh);
		if ((m_neww == m_oldw) &&
			(m_newh == m_oldh))
			return 0;
		m_winw = m_oldw = m_neww;
		m_winh = m_oldh = m_newh;
		}
	int smin, smax, nmax;
	SetRows();
	GetScrollRange(SB_VERT,&smin,&smax);
	int minx = 12 * m_nColumns;
	if ((int)m_winw < minx)
		m_winw = minx;
	int bw = m_winw / m_nColumns;
	int bh = bw * m_srch / m_srcw;
	int th = (int)m_nRows * bh;
	if ((int)m_winh > th)
		{
		m_lines = m_nRows;
		m_winh = th;
		}
	else
		{
		m_lines = m_winh / bh; // snap maller 
		if (!m_lines)			// unless empty
			{
			m_lines++;
			m_winh = bh;
			}
		else
			m_winh = bh * m_lines;
		}
	if (m_nRows > m_lines)
		nmax = m_nRows - m_lines;
	else
		nmax = 0;
	if (nmax != smax)
		{
DPF("setting range:%d",nmax);
		SetScrollRange(SB_VERT, 0, nmax, TRUE);
		}
	m_yoffset = 0;
	m_oldw = m_winw;
	m_oldh = m_winh;
DPF("recalc, desired,w:%d,h:%d",m_winw, m_winh);
	m_desired_width = bw;
	m_winh += m_ety + m_eby;
	m_winw += m_ex;

	int maxx = GetSystemMetrics(SM_CXMAXIMIZED) - 100; // to be sure
	int maxy = GetSystemMetrics(SM_CYMAXIMIZED) -100;
	int hs= GetSystemMetrics(SM_CXHSCROLL);
	int ht= GetSystemMetrics(SM_CXHTHUMB);
	if (m_lines >= m_nRows)
		m_winw += m_ex;
	else
		m_winw += m_ex + hs;


DPF("max:%d,maxy:%d.scrl:%d,thm:%d",maxx,maxy,hs,ht);
	if (m_winw >= (UINT)maxx)
		m_winw = maxx;
	if (m_winh >= (UINT)maxy)
		m_winh = maxy;

DPF("recalc swp, w:%d,h:%d",m_winw, m_winh);
	SetWindowPos(NULL, 0,0,m_winw,m_winh, SWP_NOMOVE);
	if (nmax)
		SetScrollPos(SB_VERT, 0, TRUE);
	return m_bPainted;
}

void CMyLibDlg::OnSize(UINT nType, int cx, int cy)
{
DPF("on size,w :%d,h:%d",cx,cy);
	m_neww = cx;
	m_newh = cy;
	VERIFY(SetTimer(TIMER_ID, TIMER_DELAY, NULL) == TIMER_ID);
	m_nPaint |= 1;
	UpdateWindow();
}

void CMyLibDlg::OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/)
{
	DPF("lbut up");
	GetParent()->SetFocus();
}


UINT CMyLibDlg::Index(UINT x, UINT y)
{
	if (m_flags & FLAG_FREEFORM)
		return x < m_nColumns ? y * m_nColumns + x : m_nCells;
	else
		return (y < m_frames) && (x < m_levels) ? x * m_frames + y : m_nCells;
}

void CMyLibDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CDialog::OnLButtonDblClk(nFlags, point);
	GetParent()->SetFocus();
}

void CMyLibDlg::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	CRect rect;

	GetClientRect(rect);
	DPR("pal lbut down,rc ",&rect);
//	DPF("LBUT x:%d,y:%d,id:%d",point.x,point.y,z);
//	UINT level, frame;
	UINT index = Index(point.x / m_curw, m_yoffset + (point.y / m_curh));
	if (index >= m_nCells)
		return;
	UINT level = index / m_frames;
	UINT frame = index % m_frames;
	BYTE * pSrc = m_pBufs[level];
	LEVELHEADER * plh = (LEVELHEADER *) pSrc;
	UINT srcw = plh->w;
	UINT srch = plh->h;
	UINT v;
	UINT size = 20;
	UINT off2 = srcw * srch;
	UINT off1 = srcw * srch;
	if (0)	///bw
		{
		v = 0;
		size += off1;
		}
	else
		{
		v = 1;
		size += 4 * off1 + 1024;
		}
	if (m_flags & FLAG_APPLY_NAME)
		size += 16;
	int x = plh->x;
	int y = plh->y;
	y = m_orig_h - plh->h - y;
	BYTE * pClip = new BYTE[size];
	UINT * pHead = (UINT *)pClip;
	if (m_flags & FLAG_ASPECT)
		v |= 2;
	if (m_flags & FLAG_APPLY_NAME)
		v |= 4;
	*pHead++ = v;
	*pHead++ = x;
	*pHead++ = y;
	*pHead++ = srcw;
	*pHead++ = srch;
	BYTE * pData = (BYTE *)pHead;
	UINT cellsize = 4 * srcw * srch;
	if (v & 1)
		{
		memcpy(pData,plh->pals,1024);	// palette data
		pData += 1024;
		}
	pSrc += sizeof(LEVELHEADER) + frame * (16 + cellsize);  // to cell
	if (m_flags & FLAG_APPLY_NAME)
		{
		memcpy(pData, pSrc, 16);
		pData += 16;
		}
	pSrc += 16; //to data
	memcpy(pData, pSrc, cellsize);
	UINT flags = 0;
	if (m_flags & FLAG_MATCH_COLOR)
		flags |= 2;
	if (!(m_flags & FLAG_MANIPULATE))
		flags |= 5;
	CSketchView * pView = m_pDoc->GetDocView();
	pView->SetClipBoard(pClip, flags);
	delete [] pClip;
	GetParent()->SetFocus();
}


#ifdef _NEEDSWAP
void SwapLibHdr(BYTE * p)
{
	p += 16;
	SwapEm(p, sizeof(LIBHEADER)-16);
}
#endif

int CMyLibDlg::ReadFile(LPCSTR name)
{
	CFile f;
	ASSERT(m_pBufs == 0);
	int result = 1;
	UINT max_temp = 0;
	LIBHEADER header;
	DWORD mode = CFile::modeReadWrite;
	if (!f.Open(name, mode))
		return result;
	f.Read(&header, sizeof(header));
	#ifdef _NEEDSWAP
	SwapLibHdr((BYTE *)&header);
	#endif
	if (_stricmp(header.Id,"DIGICEL LIBRARY"))
		result = 2;
	else if (header.version)
		result = 3;
	else
		{
		m_orig_w = header.owidth;
		m_orig_h = header.oheight;
		m_flags  = header.flags;
		m_levels = header.levels;
		m_frames = header.frames;
		m_nCells = m_levels * m_frames;
		UINT j;
		m_srcw = 0;
		result = 0;
		m_pBufs = new BYTE * [m_levels];
		for (j = 0; !result && (j < m_levels); j++)
			{
			LEVELHEADER lheader;
			f.Read(&lheader, sizeof(lheader));
			DOSWAP(lheader);
			UINT usize = 2 * lheader.w * lheader.h;
			UINT total_size = sizeof(lheader) + m_frames * (16 + 2 * usize);
			m_pBufs[j] = new BYTE[total_size];
			BYTE * pDst = m_pBufs[j];
			*((LEVELHEADER *)pDst) = lheader;
			if (lheader.w > m_srcw)
				{
				m_srcw = lheader.w;
				m_srch = lheader.h;	// use the largest one
				}
			if (4 * lheader.w * lheader.h > max_temp)
				max_temp = 4 * lheader.w * lheader.h;
			pDst += sizeof(lheader);
			BYTE * pTmp = new BYTE[11 * usize / 10]; // compressed data
			UINT i;
			for (i = 0; i < 2 * m_frames; i++)
				{
				if (!(i & 1))
					{
					f.Read(pDst, 16); // get name 
					pDst += 16;
					}
				UINT isize;
				DWORD vc;
				f.Read(&vc, 4);
				isize = SWAPV(vc);
				f.Read(pTmp, isize);
				vc = usize;
				UINT q = uncompress(pDst,&vc,pTmp,isize);
				if (q || (vc != usize))
					{
					result = 4;
DPF("decompress error:%d,%d,%d",q,vc,usize);
					break;
					}
				pDst += usize;
				}
			delete [] pTmp;
			}
		}
	f.Close();
	SetRows();
	if (max_temp)
		m_pTmp = new BYTE[max_temp];
	m_curw = 10; // default
	return result;
}

void CMyLibDlg::ReScale()
{
	delete [] m_pCur;
DPF("rescale,cur:%d,desw:%d",m_curw, m_desired_width);
	m_curw = m_desired_width;
	m_curh = m_curw * m_srch / m_srcw;
	UINT dpitch = 4 * ((3 * m_curw + 3) / 4);
	UINT size = m_curh * dpitch;
	m_pCur = new BYTE[40+m_nCells*size];
	LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)m_pCur;
	lpBI->biSize			= sizeof(BITMAPINFOHEADER);
	lpBI->biWidth			= m_curw;
	lpBI->biHeight			= m_curh;
	lpBI->biPlanes			= 1;
	lpBI->biBitCount		= 24;
	lpBI->biCompression		= BI_RGB;
	lpBI->biSizeImage		= size;
	lpBI->biXPelsPerMeter	= 0;
	lpBI->biYPelsPerMeter	= 0;
	lpBI->biClrUsed			= 0;
	lpBI->biClrImportant	= 0;

	CGScaler scale;
	UINT i,j;
	BYTE * pDst = m_pCur + 40;
	for (j = 0; j < m_levels; j++)
		{
		BYTE * pSrc = m_pBufs[j];
		LEVELHEADER * plh = (LEVELHEADER *) pSrc;
		BYTE * pPals = plh->pals;
		UINT srcw = plh->w;
		UINT srch = plh->h;
		UINT ip = srcw;
		pSrc += sizeof(LEVELHEADER);  // step over header
		for (i = 0; i < m_frames; i++)
			{
			pSrc += 16; // over name
			BYTE * pInkA = pSrc;
			BYTE * pInkI = pInkA + ip * srch;
			BYTE * pPntA = pSrc + 2 * ip * srch;
			BYTE * pPntI = pPntA + ip * srch;
			UINT y,x;
			UINT op = 4 * ((3 * srcw + 3 ) / 4);
			memset(m_pTmp,255,srch * op);
			for (y = 0; y < srch; y++)
				{
				for (x = 0; x < srcw ; x++)
					{
					UINT z,i;
					z = pPntA[y*ip+x];
					if (z)		// do paint
						{
						i = pPntI[y*ip+x];
				//	z =	z * pPals[4*i+3] / 255;
						m_pTmp[y*op+3*x+0] = pPals[4*i+2];
						m_pTmp[y*op+3*x+1] = pPals[4*i+1];
						m_pTmp[y*op+3*x+2] = pPals[4*i+0];
						}
					z = pInkA[y*ip+x];
					if (z)		// do Ink
						{
						i = pInkI[y*ip+x];
				//	z =	z * pPals[4*i+3] / 255;
						m_pTmp[y*op+3*x+0] = pPals[4*i+2];
						m_pTmp[y*op+3*x+1] = pPals[4*i+1];
						m_pTmp[y*op+3*x+2] = pPals[4*i+0];
						}
					}
				}
			if (scale.Init(srcw,srch,24,m_curw,m_curh,1))
				{
DPF("scale failure");
				break;
				}
			memset(pDst, 255, m_curh * dpitch);
			BYTE * pp = pDst + 3 * scale.Offx() + dpitch * scale.Offy();
			int q = scale.Custom(pp, m_pTmp, dpitch,op);
//DPF("after custom:%d",q);
			int z = scale.Copy();
//DPF("after scale:%d",z);
			pDst +=	m_curh * dpitch;	
			pSrc +=	4 * srch * ip;	
			}
		}
}

class CLibSettingsDlg : public CMyDialog
{
// Construction
public:
	CLibSettingsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSettingsDlg)
	enum { IDD = IDD_LIB_SETTINGS };
	BOOL	m_bAspect;
	BOOL	m_bShowGrid;
	BOOL	m_bManipulate;
	UINT	m_nColumns;
	BOOL	m_bFreeform;
	BOOL	m_bMatchColor;
	BOOL	m_bApplyName;
	BOOL	m_bAutoLoad;
//}}AFX_DATA


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
	afx_msg void OnGrid() ;
	afx_msg void OnManipulate() ;
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CLibSettingsDlg::CLibSettingsDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CLibSettingsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLibSettingsDlg)
	m_bAspect = 0;
	m_bManipulate = 0;
	m_bMatchColor = 0;
	m_nColumns = 12;
	m_bFreeform = 1;
	m_bApplyName = 1;
	m_bShowGrid = 1;
}


void CLibSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLibSettingsDlg)
	DDX_Check(pDX, IDC_ASPECT, m_bAspect);
	DDX_Check(pDX, IDC_LIB_GRID, m_bShowGrid);
	DDX_Check(pDX, IDC_MANIPULATE, m_bManipulate);
	DDX_Check(pDX, IDC_LIB_PALETTE, m_bMatchColor);
	DDX_Check(pDX, IDC_SET_NAME, m_bApplyName);
	DDX_Text(pDX, IDC_COLUMNS, m_nColumns);
	DDV_MinMaxUInt(pDX, m_nColumns, 1, 20);
	DDX_Radio(pDX, IDC_GRID, m_bFreeform);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CLibSettingsDlg, CMyDialog)
	ON_COMMAND(IDC_GRID, OnGrid)
	ON_COMMAND(IDC_MANIPULATE, OnManipulate)
	ON_COMMAND(IDC_FREEFORM, OnGrid)
END_MESSAGE_MAP()


BOOL CLibSettingsDlg::OnInitDialog()
{
	CMyDialog::OnInitDialog();
	GetDlgItem(IDC_COLUMNS)->ShowWindow(m_bFreeform ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_ASPECT)->EnableWindow(m_bManipulate);
	CenterWindow();
	return FALSE;
}

void CLibSettingsDlg::OnGrid()
{
	UpdateData();
	GetDlgItem(IDC_COLUMNS)->ShowWindow(m_bFreeform ? SW_SHOW : SW_HIDE);
}

void CLibSettingsDlg::OnManipulate()
{
	UpdateData();
	GetDlgItem(IDC_ASPECT)->EnableWindow(m_bManipulate);
}



void CMyLibDlg::LibSettings()
{
	CLibSettingsDlg dlg;
	dlg.m_bAspect    = m_flags & FLAG_ASPECT ? 1 : 0;
	dlg.m_bShowGrid =  m_flags & FLAG_SHOW_GRID ? 1 : 0;
	dlg.m_bManipulate= m_flags & FLAG_MANIPULATE ? 1 : 0;
	dlg.m_bFreeform  = m_flags & FLAG_FREEFORM ? 1 : 0;
	dlg.m_bMatchColor= m_flags & FLAG_MATCH_COLOR ? 1 : 0;
	dlg.m_bApplyName = m_flags & FLAG_APPLY_NAME ? 1 : 0;
	dlg.m_nColumns = m_flags & 255;
	if (dlg.m_nColumns < 1) dlg.m_nColumns = 1;
	if (dlg.DoModal() == IDOK)
		{
		BOOL bRecalc = 0;
		BOOL bPaint = 0;
		if (((m_flags & 255) != dlg.m_nColumns) ||
			(dlg.m_bFreeform != (m_flags & FLAG_FREEFORM ? 1 : 0)))
				bRecalc = 1;
		if (dlg.m_bShowGrid != (m_flags & FLAG_SHOW_GRID ? 1 : 0))
			bPaint = 1;
		m_flags &= FLAG_SHOW;
		m_flags |= dlg.m_nColumns;
		m_flags |= dlg.m_bAspect ? FLAG_ASPECT : 0;
		m_flags |= dlg.m_bShowGrid ? FLAG_SHOW_GRID : 0;
		m_flags |= dlg.m_bManipulate ? FLAG_MANIPULATE : 0;
		m_flags |= dlg.m_bFreeform ? FLAG_FREEFORM : 0;
		m_flags |= dlg.m_bMatchColor ? FLAG_MATCH_COLOR : 0;
		m_flags |= dlg.m_bApplyName ? FLAG_APPLY_NAME : 0;
		if (bRecalc)
			ReCalcWindow(1);
		if (bRecalc || bPaint)
			Invalidate();
		}
}


void CMainFrame::OnViewLib(UINT iid)
{
	iid -= ID_VIEW_LIB_0;
	CMyLibDlg * pLib = m_pLibs[iid];
	pLib->Switch(1);
}

void CMainFrame::OnUpdateViewLib(CCmdUI* pCmdUI)
{
	UINT iid = pCmdUI->m_nID;
	iid -= ID_VIEW_LIB_0;
	CMyLibDlg * pLib = m_pLibs[iid];
	if (m_pDoc && m_pScene && m_pDoc->DocOpened() && pLib)
		pCmdUI->SetCheck(pLib->Switch());
	else
		pCmdUI->Enable(FALSE);
}

CMenu * CMainFrame::GetLibMenu(BOOL bRemove)
{
	CMenu * pMainMenu = GetMenu();
	CMenu * pFileMenu = pMainMenu->GetSubMenu(0);
	int count = pFileMenu->GetMenuItemCount();
	int i;
	for (i = 0; i < count; i++)
		{
		CString str;
		if (pFileMenu->GetMenuString(i, str, MF_BYPOSITION) &&
         	(strcmp(str, "&Library ...") == 0))
			break;
		}
	if (i >= count)
		return 0;
	if (bRemove)
		{
		pFileMenu->RemoveMenu(i,MF_BYPOSITION); // popup
	//	pFileMenu->RemoveMenu(i,MF_BYPOSITION); // load
	//	pFileMenu->RemoveMenu(i,MF_BYPOSITION); //save
		pFileMenu->RemoveMenu(i,MF_BYPOSITION); // sep
		return 0;
		}
	CMenu * pLibMenu = pFileMenu->GetSubMenu(i);
	return pLibMenu;
}

CMenu * CMainFrame::GetUnLoadMenu(CMenu * pLibMenu)
{
//	CMenu * pFileMenu = pMainMenu->GetSubMenu(0);
	int count = pLibMenu->GetMenuItemCount();
	int i;
	for (i = 0; i < count; i++)
		{
		CString str;
		if (pLibMenu->GetMenuString(i, str, MF_BYPOSITION) &&
         	(strcmp(str, "&Library ...") == 0))
			break;
		}
	if (i >= count)
		return 0;
	CMenu * pMenu = pLibMenu->GetSubMenu(i);
	return pMenu;
}


BOOL CMainFrame::LoadLib(LPCSTR filename)
{
	if (m_nLibraries >= 10)
		return 1;
	CMyLibDlg * pLib = new CMyLibDlg;
	if (pLib->Load(this,filename))
		{
		delete pLib;
		return 1;
		}
	m_pLibs[m_nLibraries] = pLib;
	CMenu * pLibMenu = GetLibMenu();
	CString str;
	if (!m_nLibraries)  // if first
		{
		int count = pLibMenu->GetMenuItemCount();
		ASSERT(count == 2);
		CMenu NewPopupMenu;
		VERIFY(NewPopupMenu.CreatePopupMenu());
		HMENU hNewMenu = NewPopupMenu.GetSafeHmenu();
		pLibMenu->AppendMenu(MF_POPUP, (UINT) hNewMenu, "UnLoad...");
DPF("appending unload");
		NewPopupMenu.Detach();
		}
	CMenu * pUnLoadMenu = pLibMenu->GetSubMenu(2);  // get unload popup
	if (!m_nLibraries)		// if first add an ALL
		{
		str = "All";
		pUnLoadMenu->AppendMenu(MF_BYPOSITION,ID_LIB_UNLOAD,str);
		}
	m_pLibs[m_nLibraries]->LibName(str);
	DPF("i:%d,lib:%s",m_nLibraries,(LPCSTR)str);
	pUnLoadMenu->AppendMenu(MF_BYPOSITION,ID_LIB_UNLOAD_0 + m_nLibraries,str);

	CMenu * pMainMenu = GetMenu();
	CMenu * pViewMenu = pMainMenu->GetSubMenu(2); // View menu
	pViewMenu->AppendMenu(MF_BYPOSITION, ID_VIEW_LIB_0+m_nLibraries,str);

	m_nLibraries++;
	return 0;
}

void CMainFrame::OnLibLoad()
{
	if (m_nLibraries >= 10)
		return;
	CString name;
	m_pDoc->GetDefFolder(name,UDF_LIB,1);
	name += ".FBL";
	if (!((CSketchApp*) AfxGetApp())->PromptFileName(name,15))
		return;
	m_pDoc->UpdateDefFolder(name,UDF_LIB);
	if (!LoadLib(name))
		m_pLibs[m_nLibraries-1]->Setup(m_pDoc,1);
	else
		AfxMessageBox(IDS_LIB_LOAD);
}

void CMainFrame::OnLibSave()
{
	if (m_pDoc->CheckModified())
		return;
	CLibSaveDlg dlg;
	if (!GetSelection(dlg.m_StartFrame,dlg.m_StartLevel,
				dlg.m_EndFrame, dlg.m_EndLevel))
		{
		CSketchView * pView = (CSketchView *)GetActiveView();
		dlg.m_StartFrame = dlg.m_EndFrame = pView->CurrentFrame();
		dlg.m_StartLevel = dlg.m_EndLevel = pView->CurrentLevel();
		}
	if (!dlg.m_StartLevel)
		{
		dlg.m_StartLevel++;
		if (!dlg.m_EndLevel) dlg.m_EndLevel++;
		AfxMessageBox(IDS_LIB_NO_BG);
		}
	CString key = "Library Settings";
	CString section = "Settings";
	dlg.m_StartFrame++;
	dlg.m_EndFrame++;
	dlg.m_MaxFrame = m_pScene->FrameCount();
	dlg.m_MaxLevel = m_pScene->LevelCount()-1;
//	dlg.m_origwidth = dlg.m_width = m_pScene->Width();
//	dlg.m_origheight = dlg.m_height = m_pScene->Height();
	int flags = GetInt(section, key,10 + FLAG_APPLY_NAME);
	dlg.m_bAspect    = flags & FLAG_ASPECT ? 1 : 0;
	dlg.m_bShowGrid  = flags & FLAG_SHOW_GRID ? 1 : 0;
	dlg.m_bManipulate= flags & FLAG_MANIPULATE ? 1 : 0;
	dlg.m_bFreeform  = flags & FLAG_FREEFORM ? 1 : 0;
	dlg.m_bMatchColor= flags & FLAG_MATCH_COLOR ? 1 : 0;
	dlg.m_bApplyName = flags & FLAG_APPLY_NAME ? 1 : 0;
	dlg.m_bAutoLoad  = flags & FLAG_AUTO_LOAD ? 1 : 0;
	dlg.m_nColumns = flags & 255;
	if (dlg.DoModal() != IDOK)
		return;
	CString fileName;
	m_pDoc->GetDefFolder(fileName,UDF_LIB,1);
	fileName += ".FBL";
	if (!MyApp->PromptFileName(fileName, 12))
		return ;
	m_pDoc->UpdateDefFolder(fileName,UDF_LIB);
	flags = dlg.m_nColumns;
	flags |= dlg.m_bAspect ? FLAG_ASPECT : 0;
	flags |= dlg.m_bShowGrid ? FLAG_SHOW_GRID : 0;
	flags |= dlg.m_bManipulate ? FLAG_MANIPULATE : 0;
	flags |= dlg.m_bFreeform ? FLAG_FREEFORM : 0;
	flags |= dlg.m_bMatchColor ? FLAG_MATCH_COLOR : 0;
	flags |= dlg.m_bApplyName ? FLAG_APPLY_NAME : 0;
	flags |= dlg.m_bAutoLoad ? FLAG_AUTO_LOAD : 0;
	PutInt(section, key, flags);
	UINT v = CMyLibDlg::SaveLibrary(m_pScene, fileName,
					dlg.m_StartFrame - 1, dlg.m_EndFrame - 1,
					dlg.m_StartLevel, dlg.m_EndLevel, flags);
	if (!v)
		{
		if (flags & FLAG_AUTO_LOAD)
			{
			if (!LoadLib(fileName))
				m_pLibs[m_nLibraries-1]->Setup(m_pDoc,1);
			}
		}
	else
		{
		FormattedMsg(IDS_ERR_LIB_SAVE,v);
		}
	
}	


void CMainFrame::RemoveLibEntries(UINT which)
{
	ASSERT(m_nLibraries);
	CMenu * pMainMenu = GetMenu();
	CMenu * pViewMenu = pMainMenu->GetSubMenu(2); // View menu
//remove all from view
	int count = pViewMenu->GetMenuItemCount();
	int i;
	for (i = 0; i < count;)
		{
		UINT iid = pViewMenu->GetMenuItemID(i);
		if ((iid >= ID_VIEW_LIB_0) && (iid < (ID_VIEW_LIB_0+m_nLibraries)))
			{
			pViewMenu->RemoveMenu(i,MF_BYPOSITION);
			count--;
			}
		else
			i++;
		}
// remove one from unload menu
	CMenu * pLibMenu = GetLibMenu();
	if ((m_nLibraries == 1) ||  !which)
		{
	//	CMenu * pPopupMenu = pLibMenu->GetSubMenu(2);
	//	pPopupMenu->RemoveMenu(which,MF_BYPOSITION);
	//	CMenu * pPopupMenu = pLibMenu->GetSubMenu(2);
		pLibMenu->RemoveMenu(2,MF_BYPOSITION);
/// remove all
		}
	else
		{
		CMenu * pPopupMenu = pLibMenu->GetSubMenu(2);
		pPopupMenu->RemoveMenu(which,MF_BYPOSITION);
		}
}

void CMainFrame::RedrawLibEntries()
{
	ASSERT(m_nLibraries);
	CMenu * pMainMenu = GetMenu();
	CMenu * pViewMenu = pMainMenu->GetSubMenu(2); // View menu
//remove all from view
	UINT count = pViewMenu->GetMenuItemCount();
	UINT i;
	for (i = 0; i < count;)
		{
		UINT iid = pViewMenu->GetMenuItemID(i);
		if ((iid >= ID_VIEW_LIB_0) && (iid < (ID_VIEW_LIB_0+m_nLibraries)))
			{
			pViewMenu->RemoveMenu(i,MF_BYPOSITION);
			count--;
			}
		else
			i++;
		}
// remove all except all from unload menu
	CMenu * pLibMenu = GetLibMenu();
	CMenu * pUnloadMenu = pLibMenu->GetSubMenu(2);
	count = pUnloadMenu->GetMenuItemCount();
	ASSERT(count == (m_nLibraries + 1));
	for (i = 1; i < count;i++)
		{
		pUnloadMenu->RemoveMenu(1,MF_BYPOSITION);
		}
//
//	now add them back in with new names
//
	CString str;
	for (i = 0; i < m_nLibraries; i++)
		{
		m_pLibs[i]->LibName(str);
		pViewMenu->AppendMenu(MF_BYPOSITION, ID_VIEW_LIB_0+i,str);
		pUnloadMenu->AppendMenu(MF_BYPOSITION,ID_LIB_UNLOAD_0 + i,str);
		}
}

void CMainFrame::OnLibUnloadAll()
{
	RemoveLibEntries(0);
	for (;  m_nLibraries; )
		OnLibUnload(0);
}

void CMainFrame::OnLibUnload(UINT iid)
{
	BOOL bDoOne = 0;
	if (iid)
		{
		bDoOne = TRUE;
		iid = iid - ID_LIB_UNLOAD_0;
		RemoveLibEntries(iid+1);
		}
DPF("unload:%d",iid);
	if ((m_nLibraries) && (iid < m_nLibraries))
		{
		delete m_pLibs[iid];
		for (;iid < m_nLibraries;iid++)
			m_pLibs[iid] = m_pLibs[iid+1];
		m_nLibraries--;
		}
	if (bDoOne && m_nLibraries)
		{
		CMenu * pMainMenu = GetMenu();
		CMenu * pViewMenu = pMainMenu->GetSubMenu(2); // View menu
		UINT i;
		CString str;
		for (i = 0; i < m_nLibraries; i++)
			{
			m_pLibs[i]->LibName(str);
			pViewMenu->AppendMenu(MF_BYPOSITION, ID_VIEW_LIB_0+i,str);
			}
		}
}

void CMainFrame::SaveLibInfo()
{
	//return;
	CString section = "Libraries";
//
//	clear this section
//
	AfxGetApp()->WriteProfileString(section, NULL, NULL);
	UINT i;
	CString key;
	CString info;
	if (m_nLibraries)
		AfxGetApp()->WriteProfileInt(section, "Count", m_nLibraries);
	for (i = 0; i < m_nLibraries; i++)
		{
		CMyLibDlg * pLib = m_pLibs[i];
		key.Format("Info%d",i);
		int x,y,w,h,f;
		f = pLib->GetInfo();
		WINDOWPLACEMENT wp;
		wp.length = sizeof wp;
		if (!pLib->GetWindowPlacement(&wp))
			{
			CRect myRect;
			pLib->GetWindowRect(&myRect);
			x = myRect.left;
			y = myRect.top;
			w = myRect.right - x;
			h = myRect.bottom - y;
			}
		else
			{
			x = wp.rcNormalPosition.left;
			y = wp.rcNormalPosition.top;
			w = wp.rcNormalPosition.right - x;
			h = wp.rcNormalPosition.bottom - y;
			}
		info.Format("%u,%u,%u,%u,%u",x,y,w,h,f);
		AfxGetApp()->WriteProfileString(section, key, info);
		key.Format("Name%d",i);
		pLib->FileName(info);
		AfxGetApp()->WriteProfileString(section, key, info);
		}
}

void CMainFrame::LoadLibInfo()
{
	CString section = "Libraries";
	int c = AfxGetApp()->GetProfileInt(section,"Count", 0);
	if (!c)
		return;
	int i;
	CString key;
	CString info;
	for (i = 0; i < c; i++)
		{
		key.Format("Info%d",i);
		CString temp = AfxGetApp()->GetProfileString(section,key, "");
		key.Format("Name%d",i);
		CString name = AfxGetApp()->GetProfileString(section,key, "");
		if (!name.GetLength())
			{
			continue;
			}
		int x,y,w,h,f,z;
		z = sscanf((LPCSTR)temp,"%d,%d,%d,%d,%d",&x,&y,&w,&h,&f);
		if (!LoadLib(name))
			{
			ASSERT((i + 1 ) == m_nLibraries);
			if (z == 5)
				{
				WINDOWPLACEMENT wp;
				wp.flags = 0;
				wp.ptMinPosition.x = -1;
				wp.ptMinPosition.y = -1,
				wp.ptMaxPosition.x = -1;
				wp.ptMaxPosition.y = -1;
				wp.rcNormalPosition.left = x;
				wp.rcNormalPosition.top = y;
				wp.rcNormalPosition.right = x + w;
				wp.rcNormalPosition.bottom = y + h;
				wp.showCmd = SW_HIDE;
				m_pLibs[i]->SetWindowPlacement(&wp);
				m_pLibs[i]->SetInfo(f);
				}
			}
		}
}

void checkvalpha(BYTE * buf,int w,int h, int &hh)
{
	int x,y,p;
	p = 4 * ((w + 3) / 4);
	for (y = 0; y < hh; y++)
		{
		for (x = 0; x < w; x++)
			if (buf[y*p+x] || buf[(h-1-y)*p+x])
				break;
		if (x < w)
			{
			hh = y;
			break;
			}
		}
}

void checkhalpha(BYTE * buf,int w,int h,int & ww,int hh)
{
	int x,y,p;
	p = 4 * ((w + 3) / 4);
	int hhh = h - hh;
	for (y = hh; y < hhh; y++)
		{
		for (x = 0; x < ww; x++)
			if (buf[y*p+x] || buf[y*p+w-1-x])
				break;
		if (x < ww)
			{
			ww = x;
			}
		}
}

UINT CMyLibDlg::SaveLibrary(CScene * pScene , LPCSTR FileName,
					UINT StartFrame, UINT EndFrame,
					UINT StartLevel, UINT EndLevel, UINT flags)
{
	CFile f;
	UINT result = 1;
	LIBHEADER header;
	LEVELHEADER levelheader;
	DWORD mode = CFile::modeCreate | CFile::modeReadWrite;
	if (!f.Open(FileName, mode))
		return result;

	int w , h,p;
	w = pScene->Width();
	h = pScene->Height();
	p = 4 * ((w + 3) / 4);
	UINT size = 2 * h * 4 * ((w + 3)/4); 
	BYTE * buf = new BYTE[size]; 
	BYTE * buf2 = new BYTE[11 * size / 10]; 
	int ink = CCell::LAYER_INK;
	int paint = CCell::LAYER_PAINT;
	int frames = EndFrame + 1 - StartFrame;
	UINT levels = EndLevel + 1 - StartLevel;

	memset(&header, 0, sizeof(header));	
	strcpy(header.Id, "DIGICEL LIBRARY");
	header.version = 0;
	header.owidth = w;
	header.oheight = h;
	header.frames = frames;
	header.levels = levels;
	header.flags = flags;
	#ifdef _NEEDSWAP
	SwapLibHdr((BYTE *)&header);
	#endif
	f.Write(&header, sizeof(header));

	UINT j;
	for (j = 0; j < levels;j++)
		{
		int i;
		int ww, hh;
		hh = h / 2;
		ww = w / 2;
		for (i = 0; i < 2 * frames; i++)  // ink and paint
			{
			pScene->GetLayer(buf, StartFrame + i / 2, 
					EndLevel - j, (i & 1) ? paint : ink);
			checkvalpha(buf,w,h,hh);
			checkhalpha(buf,w,h,ww,hh);
			DPF("l:%d,f:%d,ww:%d,hh:%d",j,i,ww,hh);
			}
		if (ww) ww--;
		if (hh) hh--;	// expand rect slightly
		int x1 = ww;
		int x2 = w-ww;
		int y1 = hh;
		int y2 = h-hh;
		hh = y2 + 1 - y1;
		ww = x2 + 1 - x1;
		if (ww > w) ww = w;
		if (hh > h) hh = h;
		levelheader.x = x1;
		levelheader.w = ww;
		levelheader.y = y1;
		levelheader.h = hh;
		CNewPals * pPals = pScene->LevelPalette(EndLevel-j);
		for (i = 0; i < 256; i++)
			{
			levelheader.pals[4*i+0] = pPals->Red(i);
			levelheader.pals[4*i+1] = pPals->Green(i);
			levelheader.pals[4*i+2] = pPals->Blue(i);
			levelheader.pals[4*i+3] = pPals->Alpha(i);
			}
		DOSWAP(levelheader);
		f.Write(&levelheader, sizeof(levelheader));
		DOSWAP(levelheader);	// put it back
		int size = hh * ww;
		result = 0;
		char name[32];
		for (i = 0; i < 2 * frames;i++)
			{
			int x, y;
			if (!(i & 1))
				{
				pScene->CellName(name, StartFrame+i/2, EndLevel - j);
				name[15] = 0;	// truncate to 15
				f.Write(&name, 16);
				}
			pScene->GetLayer(buf2, StartFrame+ i / 2,
								EndLevel - j, (i & 1) ? paint : ink);
			for (y = 0; y < hh;y++)		// alpha
				{
				for (x = 0; x < ww; x++)
					{
					buf[y*ww+x] = buf2[p*(y1+y)+x1+x];
					}
				}
			for (y = 0; y < hh;y++)		// index
				{
				for (x = 0; x < ww; x++)
					{
					buf[size +y*ww+x] = buf2[p*(h+y1+y)+x1+x];
					}
				}
			DWORD dsize = 11*size / 10;
			UINT q = compress(buf2,&dsize,buf,2*size);
			if (q)
				{
				result = 5;
		DPF("compression failure:%d",q);
				break;
				}
			DWORD z = SWAPV(dsize);
			f.Write(&z, 4);
			f.Write(buf2, dsize);
			}
		}
	delete [] buf;
	delete [] buf2;
	f.Close();
	return result;
}

