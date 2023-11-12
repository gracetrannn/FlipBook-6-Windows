#include "stdafx.h"
#include "sketch.h"
#include "lipsync.h"
#include "mainfrm.h"
#include "mydoc.h"
#include "myview.h"
#include "cscene.h"
#include "nscaler.h"
#include "zlib.h"

#define PAL_ROWS    3
#define PAL_COLUMNS 12
#define PAL_TOTAL 36

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif



typedef struct {
	char Id[16];
	UINT version;
	UINT owidth;
	UINT oheight;
	UINT width;
	UINT height;
	UINT flags;
	UINT frames;
	UINT levels;
} LPSHEADER;




CLipSyncDlg::CLipSyncDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLipSyncDlg::IDD, pParent)
{     
	DPF("palette dlg construct");
	m_pDoc = 0;
	m_pScene = 0;
	m_pBuf = 0;
	m_pCur = 0;
	m_bBusy = 1;
	m_srcw = 10;
	m_srch = 10;
	m_yoffset = 0;
}

CLipSyncDlg::~CLipSyncDlg()
{
	DPF("lipsync destruct");
	delete [] m_pBuf;
	m_pBuf = 0;
	delete [] m_pCur;
	m_pCur = 0;
}

BOOL CLipSyncDlg::MyCreate( CWnd* pOwnerWnd)//, int x, int y)
{
	BOOL bResult = Create(CLipSyncDlg::IDD, pOwnerWnd);
	ShowWindow(SW_HIDE);
	DPF("create:%d",bResult);
	GetResource();
	return bResult;
}

BEGIN_MESSAGE_MAP(CLipSyncDlg, CDialog)
	//{{AFX_MSG_MAP(CLipSyncDlg)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_LBUTTONDOWN()
//	ON_WM_LBUTTONUP()
//	ON_WM_LBUTTONDBLCLK()
//	ON_WM_SETCURSOR()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



void CLipSyncDlg::OnClose()
{
	DPF("On Lipsync Close");
	((CMainFrame *)GetParent())->SwitchLipSync(TRUE);
}

void CLipSyncDlg::OnCancel() 
{
	DPF("on cancel");
//	CDialog::OnCancel();
}


BOOL CLipSyncDlg::OnInitDialog() 
{
	DPF("lipsysnc init");
	CDialog::OnInitDialog();
	m_yoffset = 0;
	m_bBusy = 1;

#if 0 // def FLIPBOOK_MAC
	m_PaletteView.SubclassDlgItem (1, this);
	m_ScrollBar.SubclassDlgItem (GetWindowScrollerControl (SB_VERT), this);
#endif
	CRect rect;
	GetClientRect(rect);
	m_curw = (rect.right - rect.left) / PAL_COLUMNS;
	m_curh = m_curw * m_srch / m_srcw;
	UINT lines = (rect.bottom - rect.top) / m_curh;

	SetScrollRange(SB_VERT, 0, PAL_ROWS - lines, FALSE);
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


BOOL CLipSyncDlg::Setup(CSketchDoc * pDoc)
{
	DPF("lipsync init");
	m_pDoc = pDoc;
	m_bBusy = 1;
	m_pScene = m_pDoc->GetDocScene();
//	if (!ReadFile("lipsync1.lps"))
		m_bBusy = 0;
	return m_bBusy;
}

void CLipSyncDlg::DoThumb(CDC & dc, CRect& rect, UINT index)
{

	COLORREF crBox;
	crBox = RGB(m_pBuf[4*index+0],m_pBuf[4*index+1],m_pBuf[4*index+2]);
	dc.FillSolidRect(rect, crBox);
//	return;

	LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)(m_pCur);
	BYTE * pSrc = m_pCur + 40 + index * m_curh * 4 * ((3*m_curw+3)/4);
	int x,y,w,h;
	int dx, dw;
	dx = 0;
	x = rect.left;
	y = rect.top;
	w = rect.right - x;
	h = rect.bottom - y;
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
	
}


void CLipSyncDlg::OnPaint() 
{
	CRect rect;
	GetClientRect(rect);
	DPR("pal dlg paint ",&rect);
	CPaintDC dc(this);
#if MAC
	dc.Clear();
#endif
	if (!m_pBuf) return;
	int boxw = (rect.right - rect.left) / PAL_COLUMNS;
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
}

void CLipSyncDlg::OnVScroll(UINT code , UINT pos , CScrollBar* pScrollBar)
{
	DPF("pal scroll,code:%d,pos:%d,off:%d",code,pos,m_yoffset);
	CRect rect;
	GetClientRect(rect);
//	UINT lines = (rect.bottom - rect.top) / m_curh;
	int boxw = (rect.right - rect.left) / PAL_COLUMNS;
	int boxh = boxw * m_srch / m_srcw;
	UINT lines = (rect.bottom - rect.top) / boxh;
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
		m_yoffset = PAL_ROWS - lines;
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
	if ((lines + m_yoffset) > PAL_ROWS)
		m_yoffset = PAL_ROWS - lines;
	DPF("newoff:%d,scpos:%d",m_yoffset,GetScrollPos(SB_VERT));
	if (m_yoffset != (UINT)GetScrollPos(SB_VERT))
		{
DPF("new pos:%d",m_yoffset);
		SetScrollPos(SB_VERT, m_yoffset, FALSE);
		InvalidateRect(NULL, TRUE);
		}
//	SetFocus(hwndMain);
}

void CLipSyncDlg::OnSize(UINT nType, int cx, int cy)
{
	if (m_bBusy)
		return;
	m_bBusy = TRUE;
	DPF("pal size,w:%d,h:%d",cx,cy);
	int smin, smax;
	GetScrollRange(SB_VERT,&smin,&smax);
	int ocx = cx;
	int ocy = cy;
	if (cx < 150)
		cx = 150;
	if (cy < 30)
		cy = 30;
	int bw = cx / PAL_COLUMNS;
	int bh = bw * m_srch / m_srcw;
	if (cy > (PAL_ROWS * bh))
		cy = PAL_ROWS * bh;
//	if ((cx == ocx) && (cy == ocy))
//		{
//		CDialog::OnSize(nType, cx, cy);
//		Invalidate();
//		return;
//		}
		DPF("new cx:%d,cy:%d",cx,cy);
	SetWindowPos(NULL, 0, 0, cx+4, cy+35, SWP_NOMOVE);
//	bw = cx / PAL_COLUMNS;
	int lines = cy / bh;
	DPF("cy:%d,bh:%d,bw:%d",cy,bh,bw);
//	if (lines > PAL_ROWS)
//		lines = (PAL_TOTAL / PAL_COLUMNS);
DPF("old max:%d,new:%d",smax,lines);
	if (lines != (PAL_ROWS - smax))
		{
DPF("setting range:%d",PAL_ROWS-lines);
		SetScrollRange(SB_VERT, 0, PAL_ROWS - lines, TRUE);
		}
	m_yoffset = 0;
	SetScrollPos(SB_VERT, 0, TRUE);
	
	Invalidate();
	m_bBusy = 0;
}

void CLipSyncDlg::OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/)
{
	GetParent()->SetFocus();
}

void CLipSyncDlg::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	CRect rect;

	GetClientRect(rect);
	DPR("pal lbut down,rc ",&rect);
//	int boxw = (rect.right - rect.left) / PAL_COLUMNS;
	UINT id = PAL_COLUMNS * (m_yoffset + (point.y / m_curh)) + point.x / m_curw;
	if (id >= m_nCells)
		return;
	DPF("LBUT x:%d,y:%d,id:%d",point.x,point.y,id);
		
	CSketchView * pView = m_pDoc->GetDocView();
	UINT v;
	UINT size = 20;
	UINT off2 = m_srcw * m_srch;
	UINT off1 = m_srcw * m_srch;
	if (0)
		{
		v = 0;
		size += off1;
		}
	else
		{
		v = 1;
		size += 4 * off1 + 1024;
		}
	int x = 0;
	int y = 0;
	BYTE * pClip = new BYTE[size];
	UINT * pHead = (UINT *)pClip;
	*pHead++ = v;
	*pHead++ = x;
	*pHead++ = y;
	*pHead++ = m_srcw;
	*pHead++ = m_srch;
	BYTE * pData = (BYTE *)pHead;
	if (v)
		{
		memcpy(pData,m_pBuf,1024);	// palette data
		pData += 1024;
		}
	UINT offset = id * 4 * m_srch * m_srcw;
	memcpy(pData,m_pBuf+1024+offset, 4 * m_srcw * m_srch);
	pView->SetClipBoard(pClip);
//	GetParent()->PostMessage(ID_EDIT_PASTE,0);
	GetParent()->SetFocus();
}

/*

BOOL CLipSyncDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
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
*/
/*
int CLipSyncDlg::ReadFile(LPCSTR name)
{
	CFile f;
	int result = 1;
	LPSHEADER header;
	DWORD mode = CFile::modeReadWrite;
	if (!f.Open(name, mode))
		return result;
	f.Read(&header, sizeof(header));
	if (_stricmp(header.Id,"DIGICEL LIPSYNC"))
		result = 2;
	else if (header.version)
		result = 3;
	else
		{
		m_orig_w = header.owidth;
		m_orig_h = header.oheight;
		m_srcw =  header.width;
		m_srch =  header.height;
		m_nCells = header.frames;
//		header.levels = 1;
		int size = 1024 + m_nCells * 4 * m_srch * m_srcw;
		m_pBuf = new BYTE[size + m_srch * 4 * ((3* m_srcw+3)/4)]; 
		BYTE * pTmp = m_pBuf + 1024 + m_nCells * 4 * m_srch * m_srcw;
		BYTE * pDst = m_pBuf + 1024;
		f.Read(m_pBuf, 1024);
		UINT usize = 2 * m_srcw * m_srch;
		UINT i;
		for (i = 0; i < 2 * m_nCells; i++)
			{
			UINT isize;
			f.Read(&isize, 4);
			f.Read(pTmp, isize);
			DWORD vc = usize;
			UINT q = uncompress(pDst,&vc,pTmp,isize);
			if (q || (vc != usize))
				{
DPF("decompress error:%d,%d,%d",q,vc,usize);
				break;
				}
			
			pDst += usize;
			}
		result = 0;
		}
	f.Close();
	m_curw = 0;
	return result;
}
*/
int CLipSyncDlg::GetResource()
{
	int result = 0;
	HRSRC hRes = FindResource(AfxGetApp()->m_hInstance, "LIPSYNC", "DGCRES");
	if (!hRes)
		{
		DPF("cannot find lipsync");
		return 11;
		}
	HGLOBAL hand = LoadResource(AfxGetApp()->m_hInstance , hRes);    
	if (!hand)
		{
		DPF("cannot load lipsync");
		return 12;
		}
	BYTE * hpSrc = (BYTE *) LockResource(hand);       
	if (!hpSrc)
		{
		DPF("cannot lock lipsync");
		return 13;
		}
	LPSHEADER * pHdr = (LPSHEADER *)hpSrc;
	hpSrc += sizeof(LPSHEADER);
	if (_stricmp(pHdr->Id,"DIGICEL LIPSYNC"))
		result = 2;
	else if (SWAPV(pHdr->version))
		result = 3;
	if (result)
		{
		UnlockResource(hand);
		return result;
		}
	m_orig_w = SWAPV(pHdr->owidth);
	m_orig_h = SWAPV(pHdr->oheight);
	m_srcw =   SWAPV(pHdr->width);
	m_srch =   SWAPV(pHdr->height);
	m_nCells = SWAPV(pHdr->frames);
	int size = 1024 + m_nCells * 4 * m_srch * m_srcw;
	m_pBuf = new BYTE[size + m_srch * 4 * ((3* m_srcw+3)/4)]; 
	BYTE * pTmp = m_pBuf + 1024 + m_nCells * 4 * m_srch * m_srcw;
	BYTE * pDst = m_pBuf + 1024;
	memcpy(m_pBuf, hpSrc, 1024);
	hpSrc += 1024;
	UINT usize = 2 * m_srcw * m_srch;
	UINT i;
	for (i = 0; i < 2 * m_nCells; i++)
		{
		UINT isize = SWAPV(*((UINT *)hpSrc));
		hpSrc += 4;
		memcpy(pTmp, hpSrc, isize);
		hpSrc += isize;
		DWORD vc = usize;
		UINT q = uncompress(pDst,&vc,pTmp,isize);
		if (q || (vc != usize))
			{
DPF("decompress error:%d,%d,%d",q,vc,usize);
			break;
			}
		pDst += usize;
		}
	UnlockResource(hand);
	result = 0;
	m_curw = 0;
	return result;
}

void CLipSyncDlg::ReScale(UINT boxw)
{
	delete [] m_pCur;
	m_curw = boxw;
	m_curh = m_curw * m_srch / m_srcw;
	UINT dpitch = 4 * ((3 * m_curw + 3) / 4);
	UINT ipitch = m_srcw;
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
	UINT i;
	BYTE * pPals = m_pBuf;
	BYTE * pSrc = m_pBuf + 1024;
	BYTE * pDst = m_pCur + 40;
	BYTE * pTmp = m_pBuf + 1024 + m_nCells * 4 * m_srch * m_srcw;
	for (i = 0; i < m_nCells; i++)
		{
		BYTE * pInkA = pSrc;
		BYTE * pInkI = pInkA + ipitch * m_srch;
		BYTE * pPntA = pSrc + 2 * ipitch * m_srch;
		BYTE * pPntI = pPntA + ipitch * m_srch;
		UINT y,x;
		UINT op = 4 * ((3 * m_srcw + 3 ) / 4);
		
		memset(pTmp,255,m_srch * op);
		for (y = 0; y < m_srch; y++)
			{
			for (x = 0; x < m_srcw ; x++)
				{
				UINT z,i;
				z = pPntA[y*ipitch+x];
				if (z)		// do paint
					{
					i = pPntI[y*ipitch+x];
				//	z =	z * pPals[4*i+3] / 255;
					pTmp[y*op+3*x+0] = pPals[4*i+2];
					pTmp[y*op+3*x+1] = pPals[4*i+1];
					pTmp[y*op+3*x+2] = pPals[4*i+0];
					}
				z = pInkA[y*ipitch+x];
				if (z)		// do Ink
					{
					i = pInkI[y*ipitch+x];
				//	z =	z * pPals[4*i+3] / 255;
					pTmp[y*op+3*x+0] = pPals[4*i+2];
					pTmp[y*op+3*x+1] = pPals[4*i+1];
					pTmp[y*op+3*x+2] = pPals[4*i+0];
					}
				}
			}
			
		if (scale.Init(m_srcw,m_srch,24,m_curw,m_curh))
			{
DPF("scale failure");
			break;
			}
		int q = scale.Custom(pDst, pTmp, dpitch,op);
//DPF("after custom:%d",q);
		int z = scale.Copy();
//DPF("after scale:%d",z);
		pDst +=	m_curh * dpitch;	
		pSrc +=	4 * m_srch * ipitch;	
		}
}
