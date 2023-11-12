#include "stdafx.h"
#include "resource.h"
#include "prevdlg.h"
#include "mydialog.h"
#include <vfw.h>
#include "myio.h"
#include "mainfrm.h"
#include <dlgs.h>
#include "cscene.h"
#include "mydoc.h"
#include "fbqt.h"
#include "sceneopt.h"
#include "cmyimage.h"
#include "ccell.h"
#include "dib.h"
#include "WinResourceLoader.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
/////////////////////////////////////////////////////////////////////////////
// CPreviewFileDlg

IMPLEMENT_DYNAMIC(CPreviewFileDlg, CFileDialog)

CPreviewFileDlg::CPreviewFileDlg(BOOL bOpenFileDialog,
		DWORD dwKind,
		LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
		DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
		CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName,
							dwFlags, lpszFilter, pParentWnd,0,0)
{
	m_dib = new CDIBStatic();
	strcpy(m_title,"File Import");
	m_ofn.Flags |= (OFN_EXPLORER | OFN_ENABLETEMPLATE);
	m_ofn.lpstrInitialDir = lpszFileName;
	m_frame = 1;
	m_level = 0;
	m_hold = 3;
	m_maxlevel = 0;
	m_maxframe = 0;
	m_rotation = 0;
	m_bScale = 0;
	m_bKeying = 1;
	m_bImportColor = 0;
	m_nAlpha = 240;
	m_pData = 0;
	m_label = "";
	if (dwKind == 3)
		{
		m_pData = new BYTE[33000];
		m_pData[0] = 0;
		m_ofn.lpstrFile = (LPSTR)m_pData;
		m_ofn.nMaxFile = 32700;
 		m_ofn.Flags |= OFN_ALLOWMULTISELECT;
		m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_FILEIMPORT);
		m_ofn.lpstrTitle = (LPCSTR)&m_title;
		}
	else
		{
#ifndef FLIPBOOK_MAC
		m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_FILEOPENPREVIEW);
#endif
		}

	m_kind = dwKind;
	m_bPreview = TRUE;
}

CPreviewFileDlg::~CPreviewFileDlg()
{
	delete m_dib;
	delete [] m_pData;
	m_pData = 0;
}

BEGIN_MESSAGE_MAP(CPreviewFileDlg, CFileDialog)
	//{{AFX_MSG_MAP(CPreviewFileDlg)
	ON_BN_CLICKED(IDC_PREVIEW, OnPreview)
	ON_BN_CLICKED(IDC_KEYING, OnKeying)
	ON_EN_CHANGE(IDC_LEVEL, OnLevel)
	ON_BN_CLICKED(IDC_IMP_DRAWING, OnDrawing)
	ON_BN_CLICKED(IDC_IMP_COLOR, OnColor)
	ON_WM_QUERYNEWPALETTE()
	ON_WM_PALETTECHANGED()
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CPreviewFileDlg::LoadAVI(LPCTSTR lpszFileName)
{
#ifndef FLIPBOOK_MAC
	PAVIFILE	pfile;
	PAVISTREAM 	pstream;
	PGETFRAME	pget;

	HRESULT		hResult;
	LPBITMAPINFOHEADER  lpbi;
	AVIFILEINFO pfi;
	
DPF("before open");
	hResult = AVIFileOpen(&pfile, lpszFileName, 0, 0L);
DPF("result:%x",(DWORD)hResult);
	if (hResult)
	return;
	AVIFileInfo(pfile,&pfi,sizeof(pfi));
DPF("w:%ld,h:%ld",pfi.dwWidth,pfi.dwHeight);
	hResult = AVIFileGetStream(pfile, &pstream, streamtypeVIDEO, 0);
DPF("result:%x",(DWORD)hResult);
	if (hResult)
	return;
	pget = AVIStreamGetFrameOpen(pstream, NULL);
	if(pget)
		{
		lpbi = (LPBITMAPINFOHEADER)AVIStreamGetFrame(pget, 0);
		if (lpbi)
			{
//			m_DIBStaticCtrl.LoadDib(lpbi); // the control will handle errors
			}
		AVIStreamGetFrameClose(pget);
		}
	AVIStreamRelease( pstream );
	AVIFileRelease( pfile );
#endif
}
void CPreviewFileDlg::LoadScene(LPCTSTR lpszFileName)
{
//	m_DIBStaticCtrl.m_DIB.Invalidate();
	m_dib->SetDib(0); // no preview
//	m_DIBStaticCtrl.LoadDib(lpszFileName); // the control will handle errors
	CMyIO * pIO = new CMyIO;
	if (!pIO) return;
	pIO->Close(TRUE);
	if (pIO->Open(lpszFileName))
		{
		delete pIO;
		return;
		}
	CScene * pScene = new CScene(pIO, std::make_unique<WinResourceLoader>());
	if (!pScene)
		{
		delete pIO;
		return;
		}
	if (pScene->Read(TRUE))
		{
		delete pScene;
		delete pIO;
		return;
		}
DPF("got scene");
	DWORD key;
	pScene->GetImageKey(key, 0, 0,CCell::LAYER_THUMB);
	if (key == 0)
		pScene->GetImageKey(key, 0, 1,CCell::LAYER_THUMB);
DPF("thumb key:%d",key);
	if (key == 0)
		{
		delete pScene;
		delete pIO;
		return;
		}
	CMyImage * pImage = new CMyImage(pIO);
	if (pImage == NULL)
		{
		delete pScene;
		delete pIO;
		return;
		}
	pImage->SetKey(key);
	if (pImage->Read(NULL))
		{
		delete pImage;
		delete pScene;
		delete pIO;
		return;
		}
	DWORD w = pImage->Width();
	DWORD h = pImage->Height();
	DWORD d = pImage->Depth();
	DWORD od = d;
	DPF("reading existing thumb,w:%d,h:%d,d:%d",w,h,d);
	DWORD size = h * 4 * ((d * w + 31) / 32);
	UINT offset = 0;
	if (d < 9)
		offset = 1024;
	else if (d > 24)
		d = 24;
	BYTE * pData = new BYTE[size+40+offset];
	pImage->Read(pData+40+offset);
	LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)pData;
	lpBI->biSize			= sizeof(BITMAPINFOHEADER);
	lpBI->biWidth			= w;
	lpBI->biHeight			= h;
	lpBI->biPlanes			= 1;
	lpBI->biBitCount		= (int)d;
	lpBI->biCompression		= BI_RGB;
	lpBI->biXPelsPerMeter	= 0;
	lpBI->biYPelsPerMeter	= 0;
	lpBI->biClrUsed			= d < 9 ? 1 << d : 0;
	lpBI->biSizeImage		= lpBI->biSize + 4 * lpBI->biClrUsed +
			lpBI->biHeight * 4 * ((lpBI->biWidth*lpBI->biBitCount+31) / 32);
	lpBI->biClrImportant	= 0;
	if (od > 24)
		{
		UINT x,y,p;
		p = 4 * ((3 *w +3) / 4);
		for (y = 0; y < h; y++)
		for (x = 0; x < w; x++)
			{
			UINT b = 255 - pData[40+4*w*y+4*x+3];
			b = b * b;
			UINT f = 255 - (b / 255);
			pData[40+y*p+3*x+0] = (f * pData[40+4*w*y+4*x+0] + b) / 255 ;
			pData[40+y*p+3*x+1] = (f * pData[40+4*w*y+4*x+1] + b) / 255 ;
			pData[40+y*p+3*x+2] = (f * pData[40+4*w*y+4*x+2] + b) / 255 ;
			}
		}


	if (d < 9)
	{
	int i;
	for (i = 0; i < 1024;i++)
//		pData[40+i] = i / 4;
		pData[40+i] = (char)(((i / 4) * (i / 4)) / 255); // gamma boost
	}
	m_dib->SetDib(pData);
//	delete pData;
	delete pImage;
//	pIO->Close();
	delete pScene;
	delete pIO;
	return;
}

int CPreviewFileDlg::ProcessDibData(void * pData, UINT ssize)
{
	LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)pData;
	if (!lpBI) return 97;
	UINT w = lpBI->biWidth;
	UINT h = lpBI->biHeight;
	UINT bits = lpBI->biBitCount;
	UINT pitch = 4 * (( bits * w + 31) / 32);
	UINT size = sizeof(BITMAPINFOHEADER) + h * pitch;
	if (bits < 9)
		size += 4 << bits;
	BYTE * pTemp = new BYTE[size];
	if (!pTemp)
		return 98;
	memcpy(pTemp, pData, size);
	m_dib->SetDib((LPBYTE)pTemp);
	return 0;
}

UINT PreviewDlgCB(FBDataKind Code, void * pData, int size, void * pClass)
{
	if (Code != kBMPDataType)
		return 0;
	return ((CPreviewFileDlg*)pClass)->ProcessDibData(pData, size);
}

void CPreviewFileDlg::LoadDib(LPCTSTR lpszFileName)
{
	FBImportStill (lpszFileName, &PreviewDlgCB, (void*)this);
}

void CPreviewFileDlg::LoadIt(LPCTSTR lpszFileName)
{
	if (!lpszFileName[0])
		return;
	switch (m_kind) {
	case PD_KIND_SCENE :
		LoadScene(lpszFileName);
		break;
	case PD_KIND_AVI:
		LoadAVI(lpszFileName);
		break;
	default:
		LoadDib(lpszFileName);
		break;
	}
	m_dib->UpdateDib();
}

void CPreviewFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CFileDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPreviewFileDlg)
	if (m_kind == PD_KIND_CELL)
	{
	DDX_Text(pDX, IDC_FRAME, m_frame);
	DDV_MinMaxUInt(pDX, m_frame, 1, m_maxframe);
	DDX_Text(pDX, IDC_LEVEL, m_level);
	DDV_MinMaxUInt(pDX, m_level, 0, m_maxlevel);
	DDX_Text(pDX, IDC_HOLD, m_hold);
	DDV_MinMaxUInt(pDX, m_hold, 1, 256);
	DDX_Text(pDX, IDC_LABEL, m_label);
	DDX_Text(pDX, IDC_ALPHA_KEY, m_nAlpha);
	DDV_MinMaxUInt(pDX, m_nAlpha, 1, 255);
	DDX_Radio(pDX, IDC_IMP_ROT0, m_rotation);
	DDX_Check(pDX, IDC_SCALE_BG, m_bScale);
	DDX_Check(pDX, IDC_KEYING, m_bKeying);
	}
	//}}AFX_DATA_MAP
}


BOOL CPreviewFileDlg::OnFileNameOK()
{
	DPF("got ok");
	UpdateData(1);
	if (m_kind != PD_KIND_CELL)
		return 0;
	int i;
	i = m_ofn.nFileOffset;
	if (i && !m_ofn.lpstrFile[i-1])
		{
		DoMultiple();
		return 1;
		}
	CScene * pScene = m_pFrame->m_pScene;
	CSketchDoc * pDoc = m_pFrame->m_pDoc;
DPF("frm:%d,lvl:%d",m_frame, m_level);
	if ( !pScene || !pDoc || !m_frame || (m_level > m_maxlevel) ||
						(m_frame > m_maxframe))
		return 1;
	CString name = GetPathName();
	UpdateData(TRUE);	// fetch args from dialog
	DPF("got dib,rot:%d",m_rotation);
	LPBITMAPINFOHEADER  lpbi = ( LPBITMAPINFOHEADER)m_dib->m_pDIB;
	UINT info = 0;
	if (!lpbi)
		return 1;
	if ((lpbi->biBitCount == 24) && m_level && m_bImportColor)
		{
		if (m_bKeying)
			info = m_nAlpha;
		else
			info = 255;
		info = (info << 16) + 128;
		}
	else if ((lpbi->biBitCount == 32) && m_level && m_bImportColor)
		{
		info = 128;
		}
	if (lpbi)
		{
		pScene->ProcessCellLabel(m_label, m_hold);
		WORD z = pDoc->CreateCell(m_frame-1,m_level,lpbi,
					m_rotation + info,1,m_bScale);
		if (z)
			{
			}
		else
			{
			m_frame += m_hold;
			pDoc->Option(DEF_HOLD,1,m_hold);
			UpdateData(0);
			}
		}
	else
		{
		}
	return 1;
}

BOOL CPreviewFileDlg::OnInitDialog() 
{
	if (m_kind)
		{
		GetParent()->SetDlgItemText(IDCANCEL,"Done");
		GetParent()->SetDlgItemText(IDOK,"Import");
		}
	if (m_kind == PD_KIND_CELL)
		{
		m_pFrame->GetSelection(m_frame, m_level);
		m_maxlevel = m_pFrame->Levels() - 1;
		m_maxframe = m_pFrame->MaxFrame() - 1;
		m_frame++;
		CSketchDoc * pDoc = m_pFrame->m_pDoc;
		m_hold = pDoc->Option(DEF_HOLD);
		}
	CFileDialog::OnInitDialog();
#ifndef FILPBOOK_MAC
	m_dib->SubclassDlgItem(IDC_IMAGE, this);
#endif
	if (m_kind == PD_KIND_CELL)
		{
		GetDlgItem(IDC_IMP_24)->ShowWindow(SW_HIDE);
		OnLevel();
//		AdjustItems();
//		CheckRadioButton(IDC_IMP_DRAWING,IDC_IMP_COLOR,
//					IDC_IMP_DRAWING+m_bImportColor);
		}
#ifndef FLIPBOOK_MAC
	GetDlgItem(IDC_PREVIEW)->SendMessage(BM_SETCHECK, (m_bPreview) ? 1 : 0);
#endif
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPreviewFileDlg::SetColor(int v)
{
	m_bImportColor = v;
	AdjustItems();
}

void CPreviewFileDlg::AdjustItems()
{
	if (m_kind != PD_KIND_CELL)
		return;
	UINT show = m_level && m_bImportColor ? SW_SHOW : SW_HIDE;
	GetDlgItem(IDC_ALPHA_KEY)->ShowWindow(show);
	GetDlgItem(IDC_KEYING)->ShowWindow(show);
	GetDlgItem(IDC_SCALE_BG)->ShowWindow(m_bImportColor ? SW_SHOW : SW_HIDE);
//	GetDlgItem(IDC_IMP_24)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_ALPHA_KEY)->EnableWindow(m_bKeying);
	GetDlgItem(IDC_KEYING)->EnableWindow(m_bImportColor && (m_level));
}

void CPreviewFileDlg::OnFileNameChange() 
{
	CFileDialog::OnFileNameChange();
	if (m_bPreview)
		LoadIt(GetPathName()); // the control will handle errors
}

void CPreviewFileDlg::OnFolderChange() 
{
	CFileDialog::OnFolderChange();
	m_dib->SetDib(0);
}

void CPreviewFileDlg::OnPreview() 
{
	m_bPreview = !m_bPreview;
	if (!m_bPreview)
		m_dib->SetDib(0);
	else
		LoadIt(GetPathName()); // the control will handle errors
	Invalidate();
}

void CPreviewFileDlg::OnKeying() 
{
	m_bKeying = !m_bKeying;
	GetDlgItem(IDC_ALPHA_KEY)->EnableWindow(m_bKeying);
}

void CPreviewFileDlg::OnLevel()
{
	CString tmp;
	UINT l = 0;
	if (GetDlgItemText(IDC_LEVEL,tmp))
		{
		UpdateData(1);
		l = m_level;
		}
	if (!(m_level = l))
		m_bImportColor = 1;
	AdjustItems();
	if (m_level)
		GetDlgItem(IDC_IMP_COLOR)->SetWindowText("Import As Colored Overlay");
	else
		GetDlgItem(IDC_IMP_COLOR)->SetWindowText("Background");
	GetDlgItem(IDC_IMP_DRAWING)->EnableWindow(m_level ? 1 : 0);
	GetDlgItem(IDC_IMP_COLOR)->EnableWindow(m_level ? 1 : 0);
	CheckRadioButton(IDC_IMP_DRAWING,IDC_IMP_COLOR,
				IDC_IMP_DRAWING+(m_level ? m_bImportColor : 1));
}

class CImportStatDlg : public CMyDialog
{
// Construction
public:
	CImportStatDlg(CWnd* pParent = NULL);   // standard constructor
	enum { IDD = IDD_IMP_STATUS };
	UINT m_Frame;
	UINT m_Level;
	UINT m_Hold;
	UINT m_Rotation;
	BOOL m_bImportColor;
	BOOL m_bScale;
	BOOL m_bKeying;
	UINT m_nAlpha;
	CString m_label;
	UINT m_state;
	UINT m_count;
	UINT m_total;
	UINT m_index;
	BOOL	m_bAbort;
	LPSTR m_pData;
	CMainFrame * m_pFrame;
	char m_text[300];
	CProgressCtrl   m_Progress;
	void UpdateInfo();
	void CheckAbort();
	int ProcessDibData (void * pData, UINT ssize);

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	virtual void OnOK();
	virtual void OnCancel();
    virtual BOOL OnInitDialog();
	afx_msg void OnPerform();
	afx_msg void OnResume();
	afx_msg void OnExit();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


void CPreviewFileDlg::DoMultiple()
{
	int i, n;
	char temp[300];
	i = m_ofn.nFileOffset;
	for (;;)
		{
		strcpy(temp,m_ofn.lpstrFile);
		n = strlen(m_ofn.lpstrFile+i);
		if (!n)
			break;
		strcat(temp,NATIVE_SEP_STRING);
		strcat(temp,m_ofn.lpstrFile+i);
		DPF("%s|",temp);
		i += n + 1;
		}
	CImportStatDlg dlg;
	dlg.m_Frame = m_frame;
	dlg.m_Level = m_level;
	dlg.m_Hold = m_hold;
	dlg.m_Rotation = m_rotation;
	dlg.m_bImportColor = m_bImportColor;
	dlg.m_bScale = m_bScale;
	dlg.m_bKeying = m_bKeying;
	dlg.m_nAlpha = m_nAlpha;
	dlg.m_label = m_label;
	dlg.m_index = m_ofn.nFileOffset;
	dlg.m_pData = m_ofn.lpstrFile;
	dlg.m_pFrame = m_pFrame;
	dlg.DoModal();
	m_frame = dlg.m_Frame;
	m_label = dlg.m_label;
	UpdateData(0);
}

void CImportStatDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CImportStatDlg, CMyDialog)
	ON_BN_CLICKED(IDC_EXIT, OnExit)
	ON_COMMAND(ID_MY_PERFORM, OnPerform)
	ON_COMMAND(IDC_RESUME, OnResume)
	ON_COMMAND(ID_MY_EXIT, OnExit)
END_MESSAGE_MAP()

CImportStatDlg::CImportStatDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CImportStatDlg::IDD, pParent)
{
}


BOOL CImportStatDlg::OnInitDialog()
{
	m_state = 0;
	CMyDialog::OnInitDialog();

	CWnd* pWnd = GetDlgItem( IDC_PROGRESS );
	CRect rect;
	pWnd->GetWindowRect( &rect );
	ScreenToClient( &rect );
	m_Progress.Create( WS_VISIBLE | WS_CHILD | PBS_SMOOTH,
						rect, this, IDC_PROGRESS );
	m_Progress.SetRange( 0,100);
	GetDlgItem(IDC_RESUME)->EnableWindow(0);
	PostMessage(WM_COMMAND, ID_MY_PERFORM, 0);
	return 1;
}

void CImportStatDlg::OnExit()
{
	DPF("got exit");
	if (m_state)
		m_bAbort = TRUE;
	else
		EndDialog(0);
}

void CImportStatDlg::OnCancel()
{
	DPF("got cancel");
	if (m_state)
		m_bAbort = TRUE;
	else
		EndDialog(0);
}


void CImportStatDlg::OnOK()
{
	DPF("got ok");
}


void CImportStatDlg::OnClose()
{
	DPF("on close");
	if (m_state )
		m_bAbort = TRUE;
	else
	   CMyDialog::OnClose();
}


void CImportStatDlg::CheckAbort()
{
	MSG msg;

	// Process existing messages in the application's message queue.
	// When the queue is empty, do clean up and return.
	while (::PeekMessage(&msg,NULL,0,0,PM_NOREMOVE) && !m_bAbort)
	{
		if (!AfxGetThread()->PumpMessage())
			return;
	}
}

void CImportStatDlg::OnResume() 
{
	GetDlgItem(IDC_RESUME)->EnableWindow(0);
	PostMessage(WM_COMMAND, ID_MY_PERFORM, 0);
}

UINT ImportMultipleCallback (FBDataKind Code, void * pData, int size, void * pClass)
{
	if (Code != kBMPDataType)
		return 0;
	return ((CImportStatDlg*)pClass)->ProcessDibData(pData, size);
}

int CImportStatDlg::ProcessDibData (void * pData, UINT ssize)
{
	LPBITMAPINFOHEADER lpbi = (LPBITMAPINFOHEADER)pData;
	CSketchDoc * pDoc = m_pFrame->m_pDoc;
	UINT info = 0;
	if (!lpbi)
		return 0;
	if ((lpbi->biBitCount == 24) && m_Level && m_bImportColor)
		{
		if (m_bKeying)
			info = m_nAlpha;
		else
			info = 255;
		info = (info << 16) + 128;
		}
	else if ((lpbi->biBitCount == 32) && m_Level && m_bImportColor)
		{
		info = 128;
		}
	pDoc->m_pScene->ProcessCellLabel(m_label, m_Hold);
	WORD z = pDoc->CreateCell(m_Frame-1,m_Level,lpbi,
					info + m_Rotation,1,m_bScale);
	if (z)
		{
		DPF("make cell error:%d",z);
		}
	return 0;
}

void CImportStatDlg::OnPerform() 
{
	DPF("on perform");
	ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EXIT)->SetWindowText("Abort");
	m_bAbort = 0;
	m_state = 1;
	int i = m_index;
	m_count = m_total = 0;
	char * p = new char[33000];
	strcpy(p,m_pData);
	for (;;)
		{
		int Best = 0;
		int j = m_index;
		for (;;)
			{
			int n = strlen(m_pData+j);
			if (!n)
				break;
			if (m_pData[j] != 1)
				{
				if (!Best || (strcmp(m_pData+j,m_pData+Best) < 0))
					Best = j;
				}
			j += n + 1;
			}
		if (!Best)
			break;
		int n = strlen(m_pData+Best);
		strcpy(p+i,m_pData+Best);
		m_pData[Best] = 1;// mark as used
		i += n + 1;
		m_total++;
		}
	p[i] = 0;
	CDib dib;
	DPF("total:%d",m_total);
	for (;;)
		{
		BOOL bError = 0;
		char temp[300];
		strcpy(temp,m_pData);
		int n = strlen(p+m_index);
		if (!n)
			break;
		strcat(temp,NATIVE_SEP_STRING);
		SetDlgItemText(IDC_NAME, p+m_index);
		SetDlgItemInt(IDC_FRAME, m_Frame);
		SetDlgItemInt(IDC_LEVEL, m_Level);
		strcat(temp,p+m_index);
		DPF("%s|",temp);
		
		// callback sets the bitmap data
		bError = FBImportStill (temp, &ImportMultipleCallback, (void*)this);
		if (bError)
			{
			if (MyError(IDS_FILE_READ,p+m_index,MB_OKCANCEL)==IDCANCEL)
				break;
			}
		m_index += n + 1;
		m_Frame += m_Hold;
		m_count++;
		UpdateInfo();
		CheckAbort();
		if (m_bAbort)
			break;
		}
	delete [] p;
	m_state = 0;
	if (m_bAbort)
		GetDlgItem(IDC_RESUME)->EnableWindow( 1);
	else
		PostMessage(WM_COMMAND, ID_MY_EXIT, 0);
	GetDlgItem(IDC_EXIT)->SetWindowText("Exit");
}


void CImportStatDlg::UpdateInfo()
{
	m_Progress.SetPos(MulDiv(100, m_count, m_total));
	UpdateData(FALSE);
	UpdateWindow();
}


