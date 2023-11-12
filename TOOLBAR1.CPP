#include "stdafx.h"
#include "toolbar1.h"
#include "nscaler.h"
#include "dialogs.h"

/*
a	ID_FILE_NEW,
b	ID_FILE_OPEN,
c	ID_FILE_SAVE,
		ID_SEPARATOR,
d	ID_EDIT_CUT,
e	ID_EDIT_COPY,
f	ID_EDIT_PASTE,
g	ID_EDIT_UNDO,
h	ID_EDIT_REDO,
		ID_SEPARATOR,
i	ID_FILE_PRINT,
		ID_SEPARATOR,
j	ID_OPT_SHOWTHUMBS,
k	ID_OPT_FG,
l	ID_OPT_BG,
m	ID_OPT_KEEP,
		ID_SEPARATOR,
n	ID_VIEW_SHEET,
o	ID_VIEW_SOUND,
p	ID_VIEW_TIMING,
		ID_SEPARATOR,
q	ID_FILE_CAPTURE,
r	ID_FILE_SCAN,
s	ID_VIEW_MODE,
t	ID_VIEW_CAMERA,
u	ID_FILE_EDIT,
		ID_SEPARATOR,
v	ID_VIEW_LAYER,
w	ID_EXPORT_MAYA,
*/


HBITMAP MyMake(LPBITMAPINFOHEADER lpBitmap)//, BYTE * lpBits)
{
#ifndef FLIPBOOK_MAC
	// make copy of BITMAPINFOHEADER so we can modify the color table
	UINT nSize = lpBitmap->biSize;
	LPBITMAPINFOHEADER lpBitmapInfo = (LPBITMAPINFOHEADER)::malloc(nSize);
	if (lpBitmapInfo == NULL)
		return NULL;
	memcpy(lpBitmapInfo, lpBitmap, nSize);

	int nWidth = (int)lpBitmapInfo->biWidth;
	int nHeight = (int)lpBitmapInfo->biHeight;
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
			lpBits, (LPBITMAPINFO)lpBitmapInfo, DIB_RGB_COLORS, SRCCOPY);
		SelectObject(hDCGlyphs, hbmOld);
		::DeleteDC(hDCGlyphs);
	}
	::ReleaseDC(NULL, hDCScreen);

	// free copy of bitmap info struct and resource itself
	::free(lpBitmapInfo);
	return hbm;
#else
	return NULL;
#endif
}

BEGIN_MESSAGE_MAP(CToolBar1, CToolBar)
	//{{AFX_MSG_MAP(ctoolbox)
	ON_WM_NCPAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CToolBar1::MakeImageList(CImageList & images, UINT offset)
{
	UINT c, p;
	UINT sw = 48;
	UINT sh = 48;
	p = 4 * ((3 * m_w * m_cnt + 3) / 4);
	BYTE * pBMP = new BYTE[40+m_h *p]; 
	UINT sp = 4 * ((9*sw+3)/ 4);
	BYTE * pTemp = new BYTE[sh*sp]; 
	BYTE * pData = pBMP;
	LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)pData;
	lpBI->biSize			= sizeof(BITMAPINFOHEADER);
	lpBI->biWidth			= m_w * m_cnt;
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
	for (c = 0; c < m_cnt; c++, pData += 3*m_w)
		{
		DPF("c:%d",c);
		UINT idd;
		idd = m_buttons[c];
		if (!idd)
			{
			pData -= 3*m_w; // skip separator
			continue;
			}
		if ((c >= m_cnt) && !offset)
			offset = 2;
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
			continue;
		LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)pSrc;
		UINT ww = lpBI->biWidth;
		UINT hh = lpBI->biHeight;
		UINT dd = lpBI->biBitCount;
		DPF("c:%d,src %d,%d,%d", c,ww,hh,dd);
		if (ww != sw*3)
			continue;
		if (hh != sh)
			continue;
		if (dd != 24)
			continue;
		ww /= 3;
		CGScaler scale;
		pSrc +=  40;
		memmove(pTemp, pSrc, sh * sp);
		pSrc = pTemp + sw * 3 * offset;
		if (scale.Init(ww,hh,24,m_w,m_h))
			{
DPF("scale failure");
			continue;
			}
		int q = scale.Custom(pData, pSrc,p,sp);
DPF("after custom:%d",q);
		int z = scale.Copy();
		UnlockResource(hand);
		}
//
//	here we should have correct bitmap
//
	CBitmap bm;
	HBITMAP hBitmap = MyMake(lpBI);
	bm.Attach(hBitmap);
	delete [] pBMP;
	delete [] pTemp;

	VERIFY (images.Create (m_w,m_h,ILC_COLOR24|ILC_MASK,m_cnt,0));
	VERIFY (images.Add (&bm, RGB (200, 200, 200)) != -1);
	bm.Detach();
	return 0;
}

UINT CToolBar1::TotalWidth(BOOL bClear /* = 0 */)
{
	if (bClear)
		m_ww = 0;
	return m_w * (m_cnt-m_scnt) + 7 * m_cnt + m_scnt * 3;
}

int CToolBar1::Setup(UINT w, UINT h, UINT editw, UINT flags)
{
	m_w = w;
	m_h = h;
	m_ocnt = 0;
	m_editw = editw;
	m_bLay = flags & 1 ? 1 : 0;
	m_bMaya= flags & 2 ? 1 : 0;
	m_bDouble = 0; 	// single line
	GetPutProfile(0);
	return Config();
}

void CToolBar1::GetPutProfile(int code)
{
	char szSection[] = "Settings";
	char szKey[] = "ToolBar";
	char szDef[] = "ABC0DEF0GH0QRST0LK0V";
	if (code == 1)	// put
		{
		char buf[80];
		UINT i;
		for (i = 0;i < m_cnt;i++)
			{
			char c;
			if (m_cur[i])
				c = 'A' + m_cur [i] - 1;
			else
				c = '0';
			buf[i] = c;
			}
		buf[i] = 0;
		AfxGetApp()->WriteProfileString(szSection, szKey, buf);
		}
	else
		{
		CString temp;
		if (code == 2)
			temp = szDef;
		else
			temp = AfxGetApp()->GetProfileString(szSection,szKey,szDef);
		UINT i;
		const char * p = (LPCSTR)temp;
		m_cnt = 0;
		m_scnt = 0;	// separators
		char c;
		for (i = 0;(m_cnt < 50) && (c = p[i]);i++)
			{
			int q;
			if (c == '0')
				{
				q = 0;
				m_scnt++;
				}
			else
				{
				c |= 32;
				if ((c >= 'a') && (c <= 'z'))
					q = 1 + c - 'a';
				else
					break;
				}

			UINT v = Info(q,0);
			if (v != 1)
				{
				m_buttons[m_cnt] = v;
				m_cur[m_cnt++] = q;
				}
			}

		}
}


void CToolBar1::SetTable(UINT * table, UINT cnt)
{
	UINT i;
	m_cnt = 0;
	m_scnt = 0;	// separators
	for (i = 0;(m_cnt < 50) && (i < cnt);i++)
		{
		if (!table[i])
			m_scnt++;
		m_buttons[m_cnt] = Info(table[i],0);
		m_cur[m_cnt++] = table[i];
		}
	GetPutProfile(1);
	Config();
	if (m_ocnt < m_cnt)
		((CFrameWnd*) AfxGetApp()->m_pMainWnd)->RecalcLayout();
	m_ocnt = m_cnt;

}

int CToolBar1::Config()
{
#ifndef FLIPBOOK_MAC
	m_ww = m_w * m_cnt;
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
	if (!SetButtons( m_buttons,m_cnt))
		{
		TRACE("Failed to create toolbar\n");
		return -1;      // fail to create
		}
//#ifdef HOTCOLD
//	ModifyStyle(0, TBSTYLE_FLAT |  TBSTYLE_TRANSPARENT );
//#else
//	m_pToolBar->ModifyStyleEx(0, TBSTYLE_WRAPABLE | TBSTYLE_FLAT);
//#endif
//	m_pToolBar->GetItemRect(LAY_BUTTON, &rect);

#ifdef WRAPTOOL
if ((TotalWidth() >= m_editw) && (m_cnt > WRAPTOOL))
	{
	UINT nStyle = GetButtonStyle( WRAPTOOL );
	// Augment the state for wrapping
	nStyle |= TBBS_WRAPPED;
	SetButtonStyle( WRAPTOOL, nStyle );
	m_bDouble = TRUE;
	}
#endif


DPF("a");
//	EnableDocking(CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM);// | CBRS_RIGHT);
//	EnableDocking(CBRS_ALIGN_ANY);

	barCtrl.SetExtendedStyle(TBSTYLE_EX_DRAWDDARROWS);
	TBBUTTONINFO tbi;
	UINT i;
	for (i = 0; i < m_cnt;i++)
		{
		int idx = m_cur[i];
		if (Info(idx,1) == 1) // is it a push button
			{
			UINT style = GetButtonStyle(i);
			style = TBBS_CHECKBOX;
//			if (Info(idx,0) == ID_OPT_FG)
//				style |= TBBS_DROPDOWN;
			SetButtonStyle(i,style);
			}
		}

//	barCtrl.GetButtonInfo(ID_OPT_BG, &tbi);
//	tbi.fsStyle = TBBS_CHECKBOX;
//			if (Info(idx,0) == ID_OPT_FG)
//UINT CToolBar1::Info(UINT index, UINT which)
	tbi.dwMask= TBIF_STYLE;
	tbi.cbSize= sizeof(TBBUTTONINFO);
	barCtrl.GetButtonInfo(ID_OPT_FG, &tbi);
	tbi.fsStyle |= TBSTYLE_CHECK | TBSTYLE_DROPDOWN;
	barCtrl.SetButtonInfo(ID_OPT_FG, &tbi);
//	barCtrl.GetButtonInfo(ID_OPT_BG, &tbi);
//	tbi.fsStyle = TBBS_CHECKBOX;
//	barCtrl.SetButtonInfo(ID_OPT_BG, &tbi);

#endif
	return 0;
}

CSize CToolBar1::CalcDynamicLayout (int nLength, DWORD nMode)
{
#ifndef FLIPBOOK_MAC
	BOOL m_bKeepSizeWhenDocked = 0;
	DWORD mask = LM_VERTDOCK;
	if (m_bKeepSizeWhenDocked && (nMode & mask))
		nMode = (nMode & ~mask) | (LM_MRUWIDTH);
	return CToolBar::CalcDynamicLayout(nLength,nMode);
#else
	return CSize();
#endif
}

UINT CToolBar1::Info(UINT index, UINT which)
{
	UINT idd, kind;
	if (which == 2)
		{
		if ((index == 22) && !m_bLay)
			return 1;
		if ((index == 23) && !m_bMaya)
			return 1;
		for (which = 0; which < m_cnt; which++)
			if (m_cur[which] == index)
				return 1;
		return 0;
		}
	kind = 0;
	switch(index) {
	case 0:
		idd = ID_SEPARATOR,
		kind = 2;
		break;
	case 1:
		idd = ID_FILE_NEW;
		break;
	case 2:
		idd = ID_FILE_OPEN;
		break;
	case 3:
		idd = ID_FILE_SAVE;
		break;
	case 4:
		idd = ID_EDIT_CUT;
		break;
	case 5:
		idd = ID_EDIT_COPY;
		break;
	case 6:
		idd = ID_EDIT_PASTE;
		break;
	case 7:
		idd = ID_EDIT_UNDO;
		break;
	case 8:
		idd = ID_EDIT_REDO;
		break;
	case 9:
		idd = ID_FILE_PRINT;
		break;
	case 10:
		kind = 1;
		idd = ID_OPT_SHOWTHUMBS;
		break;
	case 11:
		kind = 1;
		idd = ID_OPT_FG;
		break;
	case 12:
		kind = 1;
		idd = ID_OPT_BG;
		break;
	case 13:
		kind = 1;
		idd = ID_OPT_KEEP;
		break;
	case 14:
		kind = 1;
		idd = ID_VIEW_SHEET;
		break;
	case 15:
		kind = 1;
		idd = ID_VIEW_SOUND;
		break;
	case 16:
		kind = 1;
		idd = ID_VIEW_TIMING;
		break;
	case 17:
		idd = ID_FILE_CAPTURE;
		break;
	case 18:
		idd = ID_FILE_SCAN;
		break;
	case 19:
		kind = 1;
		idd = ID_VIEW_MODE;
		break;
	case 20:
		idd = ID_VIEW_CAMERA;
		break;
	case 21:
		idd = ID_FILE_EDIT;
		break;
	case 22:
		if (m_bLay)
			idd = ID_VIEW_LAYER;
		else
			idd = 1;
		break;
	case 23:
		if (m_bMaya)
			idd = ID_EXPORT_MAYA;
		else
			idd = 1;
		break;
	default:
		kind = 3;
		idd = 0;
	}
	if (which == 1)
		return kind;
	else
		return idd;
}

void CToolBar1::UpdateInfo(UINT Id, BOOL bDown)
{
	UINT i,z1;
	for (i = 0, z1 = 0; i < m_cnt; i++)
		if (m_buttons[i] == Id)
			break;
		else if (m_buttons[i] != ID_SEPARATOR)
			z1++;
	if (i >= m_cnt)
		return;
	UINT index = CommandToIndex(Id);
	SetButtonStyle(index, TBBS_CHECKBOX);
}

/*

from MS for drawing gripper if xparent toolbar

*/
void CToolBar1::OnNcPaint() 
{
	EraseNonClient();
}
void CToolBar1::EraseNonClient()
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

void CToolBar1::DoPaint(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

	// Paint inside the client area.
	CRect rect;
	GetClientRect(rect);
	DrawBorders(pDC, rect);
	DrawGripper(pDC, rect);
}

void CToolBar1::DrawGripper(CDC* pDC, const CRect& rect)
{
	pDC->FillSolidRect( &rect, ::GetSysColor(COLOR_BTNFACE)); // Fill in the BG.
	CToolBar::DrawGripper(pDC,rect);
}



CToolBarDlg::CToolBarDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CToolBarDlg::IDD, pParent)
{
}

BEGIN_MESSAGE_MAP(CToolBarDlg, CMyDialog)
	//
	ON_COMMAND(IDC_CT_REMOVE, OnRemove)
	ON_COMMAND(IDC_CT_RESET, OnReset)
	ON_COMMAND(IDC_CT_REVERT, OnRevert)
	ON_COMMAND(IDC_CT_ADD, OnAdd)
	ON_COMMAND(IDC_CT_MOVE_UP, OnMoveUp)
	ON_COMMAND(IDC_CT_MOVE_DN, OnMoveDown)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CToolBarDlg::UpdateBoxes()
{
	UINT i;
	m_cur.ResetContent();
	for (i = 0; i < m_tb->m_cnt;i++)
		{
		UINT v;
		v = m_tb->m_cur[i];
		char buf[30];
		sprintf(buf,"%d",v);
		int j = m_cur.AddString(buf);
		m_cur.SetItemData(j,v);
		}
	m_cur.SetCurSel(0);
	m_avail.ResetContent();
	for (i = 0; i < 50;i++)
		{
		UINT kind = m_tb->Info(i,1);
		if (kind > 2)
			break;
		if (!i || !m_tb->Info(i,2))
			{
			char buf[30];
			sprintf(buf,"%d",i);
			int j = m_avail.AddString(buf);
			m_avail.SetItemData(j,i);
			}
		}
	m_avail.SetCurSel(0);
}

BOOL CToolBarDlg::OnInitDialog()
{
	m_avail.m_bAvail = TRUE;
	m_avail.m_pTB = m_tb;
	m_avail.m_pDlg = this;
	m_cur.m_bAvail = 0;
	m_cur.m_pTB = m_tb;
	m_cur.m_pDlg = this;
	m_bFocus = 0;
	CMyDialog::OnInitDialog();
	
	UpdateBoxes();
	UINT i;
	m_sav_scnt = m_tb->m_scnt;
	m_sav_cnt = m_tb->m_cnt;
	for (i = 0; i < m_sav_cnt;i++)
		m_sav_cur[i] = m_tb->m_cur[i];
	GetDlgItem(IDC_CT_REVERT)->EnableWindow(0);
	return TRUE;
}

void CToolBarDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CT_AVAIL, m_avail);
	DDX_Control(pDX, IDC_CT_CURRENT, m_cur);
//	DDX_Text(pDX, IDC_PREFIX, m_prefix);
//	DDX_Text(pDX, IDC_COMPRESS, m_nComp);
//	DDV_MinMaxUInt(pDX, m_nComp, 0, 9);
	//}}AFX_DATA_MAP
}


void CToolBarDlg::UpdateToolBar()
{
	int i,c;
	UINT table[60];
	c = m_cur.GetCount();
	if (!c) return;
	for (i = 0; i < c; i++)
		table[i] = m_cur.GetItemData(i);
	m_tb->SetTable(table,c);
	GetDlgItem(IDC_CT_REMOVE)->EnableWindow((c > 1) ? 1 : 0);
	c = m_avail.GetCount();
	GetDlgItem(IDC_CT_ADD)->EnableWindow((c > 1) ? 1 : 0);
	BOOL bDiff = 1;
	if (( m_sav_scnt == m_tb->m_scnt) &&
			(m_sav_cnt = m_tb->m_cnt))
		{
		for (i = 0; (i < (int)m_sav_cnt) && (m_sav_cur[i] == m_tb->m_cur[i]);i++);
		if (i >= (int)m_sav_cnt)
			bDiff = 0;
		}
	GetDlgItem(IDC_CT_REVERT)->EnableWindow(bDiff);
}

void CToolBarDlg::ChangeFocus(BOOL bAvail)
{
	if (m_bFocus != bAvail)
		{
		m_bFocus = bAvail;
		if (bAvail)
			m_cur.Invalidate();
		else
			m_avail.Invalidate();
		}
}

void CToolBarDlg::OnRemove()
{
	m_avail.SetFocus();
	int z = m_cur.GetCurSel();
	if (z < 0) return;
	int j = m_cur.GetItemData(z);
	if (j)	// alraedy a separator
		{
		char buf[30];
		sprintf(buf,"%d",j);
		int i = m_avail.AddString(buf);
		m_avail.SetItemData(i,j);
		m_avail.SetCurSel(i);
		}
	m_cur.DeleteString(z);
	m_cur.SetCurSel(z);
	UpdateToolBar();
}

void CToolBarDlg::OnReset()
{
	m_tb->m_ocnt = 0;
	m_tb->GetPutProfile(2);
	UpdateBoxes();
	UpdateToolBar();
}

void CToolBarDlg::OnRevert()
{
	m_tb->m_ocnt = 0;
	UINT i;
	m_tb->m_scnt = m_sav_scnt;
	m_tb->m_cnt = m_sav_cnt;
	for (i = 0; i < m_sav_cnt;i++)
		m_tb->m_cur[i] = m_sav_cur[i];
	UpdateBoxes();
	UpdateToolBar();
}

void CToolBarDlg::OnAdd()
{
	m_cur.SetFocus();
	int z = m_avail.GetCurSel();
	if (z < 0) return;
	int j = m_avail.GetItemData(z);


	char buf[30];
	sprintf(buf,"%d",j);
	int k = m_cur.GetCurSel();
	if (k < 0) return;
	int i = m_cur.InsertString(k,buf);
	m_cur.SetItemData(i,j);
	m_cur.SetCurSel(i);
	if (z)
		{
		m_avail.DeleteString(z);
		m_avail.SetCurSel(z-1);
		}
	UpdateToolBar();
}

void CToolBarDlg::OnMoveUp()
{
	m_cur.SetFocus();
	char buf[30];
	int z = m_cur.GetCurSel();
	if (z < 1) return;
	int j = m_cur.GetItemData(z);
	m_cur.DeleteString(z);
	sprintf(buf,"%d",j);
	int i = m_cur.InsertString(z-1,buf);
	m_cur.SetItemData(i,j);
	m_cur.SetCurSel(i);
	UpdateToolBar();
}

void CToolBarDlg::OnMoveDown()
{
	m_cur.SetFocus();
	char buf[30];
	int z = m_cur.GetCurSel();
	int c = m_cur.GetCount();
	if (z < 0) return;
	if ((z + 2) > c)
		return;
	int j = m_cur.GetItemData(z);
	m_cur.DeleteString(z);
	sprintf(buf,"%d",j);
	int i = m_cur.InsertString(z+1,buf);
	m_cur.SetItemData(i,j);
	m_cur.SetCurSel(i);
	UpdateToolBar();
}

CTBListBox::CTBListBox()
{
	m_size = 48;//36;
	m_clrSel  = RGB(193,210,238);		// default selected color (blue)
	m_clrSel2 = RGB(0,85,227);		// default selected color (blue)
	m_clrBg   = RGB(255,255,255);	// default background color (white)
	m_clrText = RGB(0,0,0);//255,255,255);	// default text color (white)
}


BEGIN_MESSAGE_MAP(CTBListBox, CListBox)
	//{{AFX_MSG_MAP(CTBListBox)
	ON_CONTROL_REFLECT(LBN_SELCHANGE, OnSelchange)
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTBListBox message handlers
void CTBListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMIS)
{
	// TODO: Add your code to determine the size of specified item
	lpMIS->itemWidth   = 100;
	lpMIS->itemHeight  = m_size;
}

void CTBListBox::DrawItem(LPDRAWITEMSTRUCT lpDIS) 
{
#ifndef FLIPBOOK_MAC
	// TODO: Add your code to draw the specified item
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);

	CRect rectFull(lpDIS->rcItem);
//	UINT id = GetItemData(lpDIS->itemData);
	UINT id = lpDIS->itemData;
	if (!m_bAvail)
		id = id;//m_pTB->m_cur[id];
	else
	{
		id = id;
	}
	id = m_pTB->Info(id,0);
	pDC->SetBkMode(TRANSPARENT);

//	if (lpDIS->itemState & ODS_SELECTED)
//		m_pDlg->ChangeFocus(m_bAvail);
	UINT code = 0;
	if (lpDIS->itemState & ODS_SELECTED)
		code = 1;
	if (lpDIS->itemState & ODS_FOCUS)
		code += 2;
	DrawIt(pDC,&rectFull,id,code);
	if (code & 2)
		m_pDlg->ChangeFocus(m_bAvail);
#endif
}

void CTBListBox::OnSelchange() 
{
#ifndef FLIPBOOK_MAC
	static int oldSel = m_curSel;
	m_pDlg->m_bFocus = m_bAvail;

	if( oldSel != m_curSel )
	{	// refresh two rectangle,one is old item and another is new.
		CRect rect;					
		GetItemRect(oldSel,rect);
		InvalidateRect(rect,TRUE);

		GetItemRect(m_curSel,rect);
		InvalidateRect(rect,TRUE);
		oldSel = m_curSel;
	}
#endif
}

int CTBListBox::DrawBitmap(CDC * pDC,LPRECT rectFull,UINT id, BOOL bSelected)
{
	HRSRC hRes = FindResource(AfxGetApp()->m_hInstance,
					MAKEINTRESOURCE(id),RT_BITMAP);
//		LASTERR;
	if (!hRes)
		return 0;
	HGLOBAL hand = LoadResource(NULL , hRes);    
//		LASTERR;
	if (!hand)
		return 0;
	BYTE * pSrc = (BYTE *) LockResource(hand);       
	if (!pSrc)
		return 0;
	LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)pSrc;
	int ww = lpBI->biWidth;
	int hh = lpBI->biHeight;
	int dd = lpBI->biBitCount;
//	DPF("c:%d,src %d,%d,%d", c,ww,hh,dd);
	int sw = 48;
	int sh = 48;
	if ((dd != 24) || (ww != sw*3) || (hh != sh))
		return 0;
	BOOL bSuccess;
	int dx = rectFull->left;
	int dy = rectFull->top;
	int sx = bSelected ? sw : 0;
	if (m_size != sw)
		bSuccess = ::StretchDIBits(pDC->m_hDC,dx,dy,m_size,m_size,
			sx, 0, sw,sh,
			pSrc+40,
			(LPBITMAPINFO)lpBI, DIB_RGB_COLORS, SRCCOPY);
	else
		bSuccess = ::SetDIBitsToDevice(pDC->m_hDC,dx,dy,sw,sh,
					sx,0,0,hh,pSrc+40,
			(LPBITMAPINFO)lpBI, DIB_RGB_COLORS);
	return 0;
}

void CTBListBox::DrawIt(CDC * pDC,LPRECT rectFull,UINT id, UINT code)
{
#ifndef FLIPBOOK_MAC
	char txt[80];
	sprintf(txt,"* %d *",id);
	if (id == ID_SEPARATOR)
		{
		strcpy(txt,"Separator");
		}
	else
		{
		DrawBitmap(pDC,rectFull,id, (code & 1) ? 1 : 0);
		CString string;
		string.LoadString(id);
		const char * p = (LPCSTR)string;
		int i;
		for (i = 0; p[i] && (p[i] != '\n');i++);
		if (p[i])
			strcpy(txt,p+i+1);
		else
			{
			for (i = 0; (i < 12) && (txt[i] = p[i]); i++);
			txt[i] = 0;
			}
		}
	rectFull->left += m_size;
		// item has been selected - draw selected frame
	CRect rect(rectFull);
	if (code & 2)
		pDC->FillSolidRect(rect,m_clrSel);	// fill item frame
	if (code)
		pDC->FrameRect(rect,&CBrush((code & 2) ? m_clrSel2 : m_clrText));	// fill item frame
	else
		pDC->FillSolidRect(rect,m_clrBg);	// fill item frame
	pDC->SetTextColor(m_clrText);			// selected text color
	rect.CopyRect(rectFull);
	rect.DeflateRect(2,2);
	rect.OffsetRect(2,10);					// draw text
	pDC->DrawText(txt,lstrlen(txt),
					  rect,DT_LEFT | DT_SINGLELINE);
	
#endif
}

