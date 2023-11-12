// Scan.cpp : implementation file
//

#include "stdafx.h"
#include "afxdisp.h"
#include "sketch.h"
#include "mydoc.h"
#include "dialogs.h"
#include "Scan.h"
#include "deskew.h"
#include "math.h"
#include "dib.h"
#include "mainfrm.h"
#include "myview.h"
#include "fbqt.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScan dialog


CScan::CScan(CWnd* pParent /*=NULL*/)
	: CDialog(CScan::IDD, pParent)
{
	//{{AFX_DATA_INIT(CScan)
	m_Frame = 10;
	m_MaxFrame = 20;
	m_Level = 0;
	m_MaxLevel = 20;
	m_Skip = 1;
	m_MaxSkip = 200;
	m_bAutoHold = 0;
	m_pLog = 0;
	m_b24 = 0;
	m_bScale = 0;
	m_bKeying = 0;
	m_nAlpha = 255;
	m_bInit = 0;
	m_bSave = 0;
	//}}AFX_DATA_INIT
}

CScan::~CScan()
{
	delete [] m_pLog;
}

void CScan::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CScan)
	DDX_Text(pDX, IDC_FRAME, m_Frame);
	DDV_MinMaxUInt(pDX, m_Frame, 1, m_MaxFrame);
	DDX_Text(pDX, IDC_LEVEL, m_Level);
	DDV_MinMaxUInt(pDX, m_Level, 0, m_MaxLevel);
	DDX_Text(pDX, IDC_HOLD, m_Skip);
	DDV_MinMaxUInt(pDX, m_Skip, 1, 256);
	DDX_Check(pDX, IDC_SCAN_PEGSTOP, m_bPegTop);
//	DDX_Check(pDX, IDC_SCAN_PEGCROP, m_bCropPeg);
	DDX_Check(pDX, IDC_SCAN_SAVE, m_bSave);
	DDX_Text(pDX, IDC_LABEL, m_label);
	DDX_Text(pDX, IDC_ALPHA_KEY, m_nAlpha);
	DDV_MinMaxUInt(pDX, m_nAlpha, 1, 255);
	DDX_Check(pDX, IDC_IMP_24, m_b24);
	DDX_Check(pDX, IDC_SCAN_FILE, m_bFile);
	DDX_Check(pDX, IDC_SCALE_BG, m_bScale);
	DDX_Check(pDX, IDC_KEYING, m_bKeying);
	DDX_Check(pDX, IDC_SCAN_AUTOHOLD, m_bAutoHold);
 //	DDV_MinMaxUInt(pDX, m_Skip, 1, m_MaxSkip);
	DDV_MyMinMax(pDX, IDC_SCAN_FIELDSIZE, m_FieldSize, 2., 17.);
	DDV_MyMinMax(pDX, IDC_SCAN_OFFSET, m_Offset, 2., 7.);
	DDX_Radio(pDX, IDC_SCAN_ROT0, m_iRotation);
	//}}AFX_DATA_MAP
}

BOOL CScan::OnInitDialog()
{
	m_bNoDPI = 0;
//	m_bSave = 0;
//	m_Frame++;
//	m_MaxFrame++;
//	m_Level++;
//	m_iRotation = m_pDoc->Option(SCANROTATE);
	if (!m_bAuto)
		{
		if (m_iRotation == 4)
			{
			CheckRadioButton(IDC_SCAN_ROT0,
				IDC_SCAN_AUTO, IDC_SCAN_ROT0);
			m_iRotation = 0;
			}
		GetDlgItem(IDC_SCAN_AUTO)->EnableWindow(0);
		}
	CDialog::OnInitDialog();
	GetDlgItem(IDC_SCAN_SELECT)->EnableWindow(!m_bFile);
	OnScanRot();
	CreateSpinCtrls();
	OnLevel();
	OnScanAutoHold();
	if (TwainAvailable()) {
		SetDefWindow(this);
		if (!OpenSourceManager()) {
			// OpenSourceManager does not report errors
			TwainError(TWERR_OPEN_DSM);
		}
	}
	m_bInit = 1;
	return TRUE;
}

void CScan::GetChanges(BOOL bJustChanges /* = 0 */)
{
	if (!m_pDoc)
		return;
	CScene * pScene = m_pDoc->GetDocScene();
	if (!pScene)
		return;
	if (!bJustChanges)
		{
		m_Frame = 1;
		m_Level = 0;
		m_MaxFrame = 1;
		m_MaxLevel = 0;
		CMainFrame * pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
		if (!pFrame) return;
		CSketchView* pSketchView = m_pDoc->GetDocView();
		if (!pSketchView) return;
		if (!pFrame->GetSelection(m_Frame,m_Level))
			{
			m_Frame = pSketchView->CurrentFrame();
			m_Level = pSketchView->CurrentLevel();
			}
		m_Frame++;
		}
	m_MaxFrame = pScene->MaxFrameCount();
	if (m_Frame > m_MaxFrame)
		m_Frame = m_MaxFrame;
	m_MaxLevel = pScene->LevelCount() - 1;
	if (m_bInit)
		UpdateData(0);
}

void CScan::TwainError(TW_ERR e)
{
	MyError(IDS_TWAIN_ERROR , MB_ICONINFORMATION | MB_OK);
}


void CScan::CreateSpinCtrls()
{
	DWORD dwStyles=0;
	dwStyles |= UDS_ALIGNRIGHT;     // Control is placed to the right of buddy, if set
	dwStyles |= UDS_ARROWKEYS;      // Up/Down arrow keys inc/decrement, if set
	dwStyles |= UDS_SETBUDDYINT;    // Control updates buddy edit with position, if set

	dwStyles |= WS_VISIBLE | WS_CHILD;

	CRect ignored_rect; // for Mac/Xcode compatibility

	m_SpinLevel.Create(dwStyles, ignored_rect, this, IDC_SPINZ_LVL );
	m_SpinLevel.SetBuddy(GetDlgItem( IDC_LEVEL ));
	m_SpinLevel.SetRange( 0, m_MaxLevel);

	m_SpinFrame.Create(dwStyles, ignored_rect, this, IDC_SPINZ_FRM );
	m_SpinFrame.SetBuddy(GetDlgItem( IDC_FRAME ));
	m_SpinFrame.SetRange( 1, m_MaxFrame);

	m_SpinSkip.Create(dwStyles, ignored_rect, this, IDC_SPINZ_SKP );
	m_SpinSkip.SetBuddy(GetDlgItem( IDC_HOLD ));
	m_SpinSkip.SetRange( 1, m_MaxSkip);

}


BEGIN_MESSAGE_MAP(CScan, CDialog)
	//{{AFX_MSG_MAP(CScan)
	ON_BN_CLICKED(IDC_SCAN_SCAN, OnScanScan)
	ON_BN_CLICKED(IDC_SCAN_SELECT, OnScanSelect)
	ON_BN_CLICKED(IDC_SCAN_ROT0, OnScanRot)
	ON_BN_CLICKED(IDC_SCAN_ROT90, OnScanRot)
	ON_BN_CLICKED(IDC_SCAN_ROT180, OnScanRot)
	ON_BN_CLICKED(IDC_SCAN_ROT270, OnScanRot)
	ON_BN_CLICKED(IDC_SCAN_AUTO, OnScanRot)
	ON_BN_CLICKED(IDC_SCAN_AUTOHOLD, OnScanAutoHold)
	ON_BN_CLICKED(IDC_KEYING, OnKeying)
	ON_BN_CLICKED(IDC_IMP_24, OnOverlay)
	ON_BN_CLICKED(IDC_SCAN_FILE, OnFile)
	ON_EN_CHANGE(IDC_LEVEL, OnLevel)
	ON_BN_CLICKED(IDC_SCAN_VIEWLOG, OnViewLog)
	ON_BN_CLICKED(IDC_SCAN_CLEARLOG, OnClearLog)
	ON_BN_CLICKED(IDC_SAVE_SETTINGS, OnScanSave)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CScan message handlers

void CScan::OnScanRot()
{
	BOOL bEnable = IsDlgButtonChecked(IDC_SCAN_AUTO);
	GetDlgItem(IDC_SCAN_PEGSTOP)->EnableWindow(bEnable);
	GetDlgItem(IDC_SCAN_FILE)->EnableWindow(bEnable);
	GetDlgItem(IDC_SCAN_FIELDSIZE)->EnableWindow(bEnable);
	GetDlgItem(IDC_SCAN_OFFSET)->EnableWindow(bEnable);
//	GetDlgItem(IDC_SCAN_CLEARLOG)->EnableWindow(bEnable);
//	GetDlgItem(IDC_SCAN_VIEWLOG)->EnableWindow(bEnable);
}

void CScan::OnKeying()
{
	m_bKeying = !m_bKeying;
	GetDlgItem(IDC_ALPHA_KEY)->EnableWindow(m_bKeying);
}

void CScan::OnLevel()
{
	UpdateData(1);
	if (m_Level)
		{
		m_b24 = !m_b24;
		GetDlgItem(IDC_IMP_24)->EnableWindow(TRUE);
		OnOverlay();
		}
	else
		{
		GetDlgItem(IDC_SCALE_BG)->EnableWindow(TRUE);
		GetDlgItem(IDC_IMP_24)->EnableWindow(0);
		GetDlgItem(IDC_ALPHA_KEY)->EnableWindow(0);
		GetDlgItem(IDC_KEYING)->EnableWindow(0);
		}
}

void CScan::OnOverlay()
{
	m_b24 = !m_b24;
	if (!m_b24)
		{
		m_bKeying = 0;
		GetDlgItem(IDC_ALPHA_KEY)->EnableWindow(m_bKeying);
		}
	GetDlgItem(IDC_SCALE_BG)->EnableWindow(m_b24);
	GetDlgItem(IDC_KEYING)->EnableWindow(m_b24);
}

void CScan::OnFile()
{
	m_bFile = !m_bFile;
	GetDlgItem(IDC_SCAN_SELECT)->EnableWindow(!m_bFile);
}

void CScan::OnScanAutoHold()
{
	UpdateData();
//	m_bAutoHold ^= 1;//BOOL bEnable = IsDlgButtonChecked(IDC_SCAN_AUTOHOLD);
	UINT sw = m_bAutoHold ? SW_HIDE : SW_SHOW;
	GetDlgItem(IDC_SCAN_TEXT)->ShowWindow(sw);
	GetDlgItem(IDC_HOLD)->ShowWindow(sw);
	GetDlgItem(IDC_SCAN_TEXT2)->ShowWindow(sw);
	GetDlgItem(IDC_LABEL)->ShowWindow(sw);
	GetDlgItem(IDC_SPINZ_SKP)->ShowWindow(sw);
	if (m_bAutoHold)
		{
		m_Frame--;
		m_pDoc->m_pScene->FindNextCell(m_Frame,m_Level);
		m_Frame++;
		UpdateData(0);
		}
}


void CScan::OnViewLog()
{
	ASSERT(m_pLog);
	if (!m_pLog)
		return;
	CLogDlg dlg;
	dlg.m_title = "Scan Warnings";
	dlg.m_pData = (LPSTR)m_pLog+8;// skip two uints
	dlg.DoModal();
}

void CScan::OnClearLog()
{
	delete [] m_pLog;
	m_pLog = 0;
	GetDlgItem(IDC_SCAN_CLEARLOG)->EnableWindow(FALSE);
	GetDlgItem(IDC_SCAN_VIEWLOG)->EnableWindow(FALSE);
	
}

void CScan::LogIt(LPCSTR ztxt)
{
	UINT * pTemp;
	char buf[80];
	sprintf(buf,"Level:%4d, Frame:%4d-%s",m_Level,m_Frame,ztxt); 
	if (!m_pLog)
		{
		GetDlgItem(IDC_SCAN_CLEARLOG)->EnableWindow(TRUE);
		GetDlgItem(IDC_SCAN_VIEWLOG)->EnableWindow(TRUE);
		m_pLog = new BYTE[1000];
		pTemp = (UINT *)m_pLog;
		pTemp[1] = 1000;
		pTemp[0] = 8;	// two uints

		}
	else
		pTemp = (UINT *)m_pLog;
	int c = strlen(buf);
	UINT max = pTemp[1];
	UINT size = pTemp[0];
	if ((c + 3 + size) >= max)
		{
		max += 1000;	// grow it
		BYTE * tp = new BYTE[max];
		UINT i;
		for (i = 0; i < size; i++)
			tp[i] = m_pLog[i];
		delete [] m_pLog;
		m_pLog = tp;
		pTemp = (UINT *)m_pLog;
		pTemp[1] = max;	// new max
		}
	strcpy((LPSTR)m_pLog+size, buf);
	size += c;
	m_pLog[size++] = 13;
	m_pLog[size++] = 10;
	m_pLog[size] = 0;
	pTemp[0] = size;
}

void CScan::OnScanSave()
{
	UpdateData();	// get label contents
	m_pDoc->Option(DEF_HOLD,1,m_Skip);
	m_pDoc->Option(SCANROTATE,1,m_iRotation +  (m_bFile ? 16 : 0) +
						(m_bScale ? 32 : 0) + (m_b24 ? 8 : 0) +
						(m_bAutoHold ? 64 : 0) + (m_bSave ? 128 : 0));
	m_pDoc->Option(SCAN_PEGTOP,1,m_bPegTop);
//	Option(SCAN_CROPPEG,1,m_bCropPeg);
	m_pDoc->Option(SCAN_FIELD,1,(int)(1000.0 * m_FieldSize));
	m_pDoc->Option(SCAN_OFFSET,1,(int)(1000.0 * m_Offset));
}

void CScan::OnScanScan() 
{
	m_pDoc->Option(PEG_SHOWFG,1,0); // turn off light
	SetShowUI(TRUE);
	UpdateData(TRUE);
	m_Level = m_SpinLevel.GetPos();
	m_Frame = m_SpinFrame.GetPos();
	m_Level = m_SpinLevel.GetPos();
	if (m_bFile)
		GetFileImage();
	else
		BeginAcquire();
}

void CScan::OnScanSelect() 
{
	m_bNoDPI = 0;
	SelectSource();
}

void CScan::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
	CMainFrame * pFrame = (CMainFrame*)m_pParentWnd;
	if (!pFrame)
		pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
	delete pFrame->m_pScan;
	pFrame->m_pScan = 0;
}

BOOL CScan::PreTranslateMessage(MSG* pMsg)
{
	return (TwainMessageHook(pMsg)) ||
		CDialog::PreTranslateMessage(pMsg);
}

void CScan::DibReceived(HGLOBAL hdib)
{
	UpdateData(TRUE);
	DPF("got dib,rot:%d",m_iRotation);
	if (m_iRotation == 4)
		{
		DoAuto(hdib);
		return;
		}
	LPBITMAPINFOHEADER  lpbi;
	lpbi = (LPBITMAPINFOHEADER)::GlobalLock(hdib);
	if (lpbi)
		{
		UINT z,q;
		BOOL bBad = 0;
		if (m_bSave)
			bBad = SaveImage(lpbi);
		q = m_iRotation;
		if ((lpbi->biBitCount == 24) && m_Level && m_b24)
			q += 128 + (m_nAlpha << 16);
		if (!m_bAutoHold)
			m_pDoc->m_pScene->ProcessCellLabel(m_label, m_SpinSkip.GetPos());
		z = m_pDoc->CreateCell(m_Frame-1,m_Level,lpbi, q,1,m_bScale);
		::GlobalUnlock(hdib);
		if (bBad)
			AfxMessageBox( IDS_ERR_SCAN_SAVE);
		if (z)
			{
			DPF("make cell error:%d",z);
			}
		else
			{
			if (m_bAutoHold)
				{
				m_pDoc->m_pScene->FindNextCell(m_Frame,m_Level);
				m_Frame++;
				}
			else
				m_Frame += m_SpinSkip.GetPos();
			}
		UpdateData(0);
		}
	else
		{
DPF("no lpbi");
		}
	if (hdib)
		::GlobalFree(hdib);
}

void CScan::DoAuto(HGLOBAL hdib)
{
	SKEWINFO info;
	LPBITMAPINFOHEADER  lpbi;
	lpbi = (LPBITMAPINFOHEADER)::GlobalLock(hdib);
	BOOL bColor = lpbi->biBitCount == 24 ? TRUE : FALSE;
	BOOL bMono = lpbi->biBitCount == 1 ? TRUE : FALSE;
	info.pTemp = 0;
	UINT ow, oh;
	if (lpbi)
		MakeGray(info,lpbi);
	else
		{
		::GlobalFree(hdib);
		LogIt("Memory Problem");
		return;
		}
	if (!bColor)
		::GlobalFree(hdib);
	UINT abc = AfxGetApp()->GetProfileInt("Settings","AutoScanDPI", 0);
	if (abc)
		{
		info.xdpi = info.ydpi = abc;
		abc = (UINT)((double)info.iw / m_FieldSize) / 25;
		if (abc == info.xdpi / 25)
			abc = 0;
		}
	BOOL bBadDPI = 0;
	if (info.iw > info.ih)
		{
		if (!info.xdpi || !info.ydpi)
			{
			info.xdpi = info.ydpi = 
					AfxGetApp()->GetProfileInt("Options","DPI", 0);
			}
		if (!info.xdpi || !info.ydpi)
			{
			info.xdpi = 25 * ((UINT)((double)info.iw / m_FieldSize) / 25);
			info.ydpi = info.xdpi;
			bBadDPI = 1;
			}
		ow = (UINT)(m_FieldSize * (double)info.xdpi);
		}
	else
		{
		if (!info.xdpi || !info.ydpi)
			{
			info.xdpi = info.ydpi = 
					AfxGetApp()->GetProfileInt("Options","DPI", 0);
			}
		if (!info.xdpi || !info.ydpi)
			{
			info.xdpi = 25 * ((UINT)((double)info.ih / m_FieldSize) / 25);
			info.ydpi = info.xdpi;
			bBadDPI = 1;
			}
		ow = (UINT)(m_FieldSize * (double)info.ydpi);
		}
	if (!m_bNoDPI && bBadDPI)
		{
		char buf[40];
		sprintf(buf,"Guessing DPI:%d",info.xdpi);
		LogIt(buf);
		m_bNoDPI = 1;
		}
	if (!m_bNoDPI && abc)
		{
		char buf[40];
		sprintf(buf,"Possible DPI Mismatch,%d,%d",info.xdpi,25*abc);
		LogIt(buf);
		m_bNoDPI = 1;
		}
	oh = (ow * m_pDoc->m_pScene->Height()) / m_pDoc->m_pScene->Width();
	info.num = 0;
	info.den = 1;
	BOOL bFlip = 0;
	BOOL bBad = 0;
	UINT ocy = oh / 2;
	if (FindCircle(info))
		{
		bBad = 1;
		LogIt("Cannot Find Circle");
		info.cx = info.iw / 2;
		info.cy = info.ih / 2;
		}
	else 
		{
		if (FindSkew(info))
			{
			bBad = 1;
			LogIt("Cannot Find Peg Holes");
			}
		if (m_bPegTop)
			ocy += (UINT)((double)info.ydpi * m_Offset);
		else
			ocy -= (UINT)((double)info.ydpi * m_Offset);

		if (info.iw > info.ih)
			{
			if (info.cy < info.ih / 2)
				{
//				info.cy += (INT)((double)info.ydpi * m_Offset);
				if (!m_bPegTop)
					bFlip = TRUE;
				}
			else
				{
//				info.cy -= (INT)((double)info.ydpi * m_Offset);
				if (m_bPegTop)
					bFlip = TRUE;
				}
			}
		else
			{
			if (info.cx < info.iw / 2)
				{
//				info.cx += (INT)((double)info.xdpi * m_Offset);
				if (m_bPegTop)
					bFlip = TRUE;
				}
			else
				{
//				info.cx -= (INT)((double)info.xdpi * m_Offset);
				if (!m_bPegTop)
					bFlip = TRUE;
				}
			}

		}
	BYTE * pImage = 0;
	BYTE * pBits;
	UINT p;
	UINT bpp = bColor ? 3 : 1;
	UINT extra = 40 + (bColor ? 0 : 1024);
	if (bBad & !bColor)
		{
		ow = info.iw;
		oh = info.ih;
		p = 4 * (( bpp * ow + 3) / 4);
		pImage = new BYTE[extra + oh * p];
		pBits = pImage + extra;
		BYTE * sp = info.pTemp;
		for (UINT yy = 0; yy < oh;yy++,pBits += p,sp += ow)
			memmove(pBits,sp,ow);
		}
/*
	else if (0)//bColor)
		{
//		info.ow = ow;
//		info.oh = oh;
//		info.ocy = ocy;
//		info.bColor = bColor;
		p = 4 * (( 3 * bpp * ow + 3) / 4);
		pImage = new BYTE[extra + oh * p];
		pBits = pImage + extra;
//		RotateIt(info, pImage,(LPBYTE)lpbi+40);
		}
*/
else if (!bBad)
	{
	p = 4 * (( bpp * ow + 3) / 4);
	pImage = new BYTE[extra + oh * p];
	pBits = pImage + extra;
	UINT ox, oy;
	UINT ocx;
	ocx = ow / 2;
	double angle = atan2((double)info.num, (double)info.den);
	double	ssin = sin(angle);
	double	ccos = cos(angle);
	UINT ip = info.iw;
	if (bColor)
		ip = 4 * ((bpp * ip + 3) / 4);
	BYTE * pTemp;
	if (bColor)
		pTemp = (LPBYTE)lpbi + 40; 
	else
		pTemp = info.pTemp;
	for (oy = 0; oy < oh; oy++)
		{
		int dy = oy - ocy;
		double yy;
		double ycos;
		double ysin;
		yy = (double)dy;
		ycos = yy * ccos;
		ysin = yy * ssin;
		double zarg1, zarg2;
		double tx, ty;
		int dx = 0 - ocx;
		double xx = (double)dx;
		if (info.iw > info.ih)
			{
			if (bFlip)
				{
				ty = info.cx + (xx * ccos - ysin);
				tx = info.cy + (xx * ssin + ycos);
				zarg1 = ccos;
				zarg2 = ssin;
				}
			else
				{
				ty = info.cx - (xx * ccos - ysin);
				tx = info.cy - (xx * ssin + ycos);
				zarg1 = -ccos;
				zarg2 = -ssin;
				}
			if (bpp > 1)
				{
				for (ox = 0; ox < ow; ox++)
					{
					int sx = (int)ty;
					int sy = (int)tx;
					ty += zarg1;
					tx += zarg2;
					UINT xx;
					for (xx = 0; xx < bpp;xx++)
						{
						BYTE v;
						if (((UINT)sx >= info.iw) || ((UINT)sy >= info.ih))
							v = 255;
						else
							v = pTemp[sy*ip+bpp*sx+xx];
						pBits[p*oy+bpp*ox+xx] = v;
						}
					}
				}
			else
				{
				for (ox = 0; ox < ow; ox++)
					{
					int sx = (int)ty;
					int sy = (int)tx;
					ty += zarg1;
					tx += zarg2;
					BYTE v;
					if (((UINT)sx >= info.iw) || ((UINT)sy >= info.ih))
						v = 255;
					else
						v = pTemp[sy*ip+sx];
					pBits[p*oy+ox] = v;
					}
				}
			}
		else
			{
			if (bFlip)
				{
				tx = info.cy + (xx * ccos - ysin);
				ty = info.cx - (xx * ssin + ycos);
				zarg1 = ccos;
				zarg2 = -ssin;
				}
			else
				{
				tx = info.cy - (xx * ccos - ysin);
				ty = info.cx + (xx * ssin + ycos);
				zarg1 = -ccos;
				zarg2 = ssin;
				}
			if (bpp > 1)
				{
				for (ox = 0; ox < ow; ox++)
					{
					int sx = (int)ty;
					int sy = (int)tx;
					tx += zarg1;
					ty += zarg2;
					int xx;
					for (xx = 0; xx < 3; xx++)
						{
						BYTE v;
						if (((UINT)sx >= info.iw) || ((UINT)sy >= info.ih))
							v = 255;
						else
							v = pTemp[sy*ip+bpp*sx+xx];
						pBits[p*oy+bpp*ox+xx] = v;
						}
					}
				}
			else
				{
				for (ox = 0; ox < ow; ox++)
					{
					int sx = (int)ty;
					int sy = (int)tx;
					tx += zarg1;
					ty += zarg2;
					BYTE v;
					if (((UINT)sx >= info.iw) || ((UINT)sy >= info.ih))
						v = 255;
					else
						v = pTemp[sy*ip+sx];
					pBits[p*oy+ox] = v;
					}
				}
		}

	}
//	rotate and place image
	}
//	if (!bColor)
		delete [] info.pTemp;
	if (!bBad || !bColor)
		{
		lpbi = (LPBITMAPINFOHEADER)pImage;
		lpbi->biSize			= sizeof(BITMAPINFOHEADER);
		lpbi->biWidth			= ow;
		lpbi->biHeight			= oh;
		lpbi->biPlanes			= 1;
		lpbi->biBitCount		= bColor ? 24 : 8;
		lpbi->biCompression		= BI_RGB;
		lpbi->biXPelsPerMeter	= (50 + info.xdpi * 3939 / 100);
		lpbi->biYPelsPerMeter	= (50 + info.ydpi * 3939 / 100);
		lpbi->biClrUsed		= bColor ? 0 : 256;
		lpbi->biClrImportant	= 0;
		lpbi->biSizeImage		= lpbi->biSize + 4 * lpbi->biClrUsed +
			lpbi->biHeight * 4 * ((lpbi->biWidth*lpbi->biBitCount+31) / 32);
		if (!bColor)
			for (int k = 0; k < 1024; k++)
				pImage[40+k] = k / 4; // false palette
		}
	UINT ff = m_Frame;
	BOOL bBadsave = 0;
	if (m_bSave)
		bBadsave = SaveImage(lpbi,bMono);
	if (!m_bAutoHold)
		m_pDoc->m_pScene->ProcessCellLabel(m_label, m_SpinSkip.GetPos());
	WORD z = m_pDoc->CreateCell(ff-1,m_Level,lpbi, bMono ? 4 : 0,1,m_bScale);
	if (z)
		{
		LogIt("Cannot create Cell");	
		DPF("make cell error:%d",z);
		}
	else
		{
		if (m_bAutoHold)
			{
			m_pDoc->m_pScene->FindNextCell(m_Frame,m_Level);
			m_Frame++;
			}
		else
			m_Frame += m_SpinSkip.GetPos();
		}
	UpdateData(0);
	if (bBadsave)
		LogIt("Cannot save image");	
	delete [] pImage;
	if (bColor)
		::GlobalFree(hdib);
}

BOOL CScan::SaveImage(void * p, BOOL bCvt)
{
	BOOL bResult = 1;
	LPBITMAPINFOHEADER lpbi = (LPBITMAPINFOHEADER)p;
	char name[300];
	strcpy(name,m_pDoc->GetPathName());
	int zi,zj;
	zj = 1000;
	for (zi = 0; name[zi]; zi++)
		if (name[zi] == '.')
			zj = zi;
	if (zj == 1000)
		name[zi++] = 'X';
	else
		zi = zj;
	name[zi] = 0;
	CreateDirectory(name,0);
	CString fileName = name;
		fileName += NATIVE_SEP_CHAR;
	if (m_label.GetLength())
		fileName += m_label;
	else
		{
		char buf[15];
		sprintf(buf, "L%03dF%03d", m_Level,m_Frame);
		fileName += buf;
		}
#ifndef FLIPBOOK_MAC
	fileName += ".BMP";
	FBFileFormat kind = kBMPFileFormat;
#else
	fileName += ".PICT";
	FBFileFormat kind = kPICTFileFormat; 
#endif
	int res;
	if (bCvt && (lpbi->biBitCount == 8))
		{
		CDib dib;
		dib.ReadFromBMI(lpbi);
		dib.Convert(1);
		res = FBExportStill (fileName, dib.m_pBMI, 0, kind);
		}
	else
		res = FBExportStill (fileName, lpbi, 0, kind);
	return res ? 1 : 0;
}	


UINT ScanDlgInCB(FBDataKind Code, void * pData, int size, void * pClass)
{
	HGLOBAL hDib;
	if (Code != kBMPDataType)
		return 0;
	hDib = GlobalAlloc(GHND, size);
	if (!hDib)
		return 0;
	BYTE * pBits = (BYTE *)::GlobalLock(hDib);
	if (!pBits)
		return 0;
	memcpy(pBits,pData,size);
	::GlobalUnlock(hDib);
	((CScan *)pClass)->MyDibReceived(hDib);
	return 0;
}

#define ALLOW_MULTIPLE

BOOL CScan::GetFileImage()
{
	CFileDialog fileDlg( TRUE, "bmp", NULL,
			0, "BMP Files (*.BMP)|*.BMP||", this);
// Initializes m_ofn structure 
	fileDlg.m_ofn.lpstrTitle = "Choose File";
	BYTE * pData = 0;
	fileDlg.m_ofn.Flags |= OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
#ifdef ALLOW_MULTIPLE
	fileDlg.m_ofn.Flags |= OFN_EXPLORER;
	pData = new BYTE[33000];
	pData[0] = 0;
	fileDlg.m_ofn.lpstrFile = (LPSTR)pData;
	fileDlg.m_ofn.nMaxFile = 32700;
 	fileDlg.m_ofn.Flags |= OFN_ALLOWMULTISELECT;
#endif
// Call DoModal
	if ( fileDlg.DoModal() == IDOK)
		{
		int i;
		i = fileDlg.m_ofn.nFileOffset;
		if (i && !fileDlg.m_ofn.lpstrFile[i-1])
			DoMultiple((LPSTR)pData,i);
		else
			{
		    CString fileName = fileDlg.GetPathName();
			FBImportStill ((LPCSTR)fileName, &ScanDlgInCB, (void*)this);
			}
		}
	delete [] pData;
	return 0;
}


UINT CScan::Import(int index)
{
		BOOL bError = 0;
		char temp[300];
		strcpy(temp,m_pData);
		int n = strlen(m_pData+m_index);
		DPF("import,indx:%d,mindex:%d,len:%d",index,m_index,n);
		if (!n)
			return 0;
		strcat(temp,NATIVE_SEP_STRING);
		strcat(temp,m_pData+m_index);
		DPF("%s|",temp);
		
		bError = FBImportStill ((LPCSTR)temp, &ScanDlgInCB, (void*)this);
		if (bError)
			{
			if (MyError(IDS_FILE_READ,m_pData+m_index,MB_OKCANCEL)==IDCANCEL)
				return 1;
			}
		m_index += n + 1;
//		m_Frame += m_Hold;
//		m_count++;
	return 0;
}

unsigned int MultiScanCallback(unsigned int index,
						void *UserObject, void *UserData )
{
	return ((CScan *)UserObject)->Import(index);
}


void CScan::DoMultiple(char * pData, int offset)
{
	int i;
#ifdef _DEBUG
	char temp[300];
	i = offset;
	for (;;)
		{
		strcpy(temp,pData);
		int n = strlen(pData+i);
		if (!n)
			break;
		strcat(temp,NATIVE_SEP_STRING);
		strcat(temp,pData+i);
		DPF("%s|",temp);
		i += n + 1;
		}
#endif
	i = m_index = offset;
	m_pData = new char[33000];
	strcpy(m_pData,pData);
	int total = 0;
	for (;;)
		{
		int Best = 0;
		int j = m_index;
		for (;;)
			{
			int n = strlen(pData+j);
			if (!n)
				break;
			if (pData[j] != 1)
				{
				if (!Best || (strcmp(pData+j,pData+Best) < 0))
					Best = j;
				}
			j += n + 1;
			}
		if (!Best)
			break;
		int n = strlen(pData+Best);
		strcpy(m_pData+i,pData+Best);
DPF("name:%s|",pData+Best);
		pData[Best] = 1;// mark as used
		i += n + 1;
		total++;
		}
	m_pData[i] = 0;

	m_pDoc->BeginBusy("Importing Images");
	CProg2Dlg progger;
	progger.ProgSetup( MultiScanCallback, this,0,m_Frame,total);
	progger.DoModal();
	m_pDoc->EndBusy();

	delete [] m_pData;
	m_pData = 0;
}
