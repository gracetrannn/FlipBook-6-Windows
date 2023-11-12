// NewDlg.cpp : implementation file
//

#include "stdafx.h"
#include "sketch.h"
#include "NewDlg.h"
#include "dirdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewDlg dialog

#define CUSTOM_WHICH 4

CNewDlg::CNewDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CNewDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewDlg)
	m_bXSheet = TRUE;
	m_Frames = 1;
	m_Height = 60;
	m_Levels = 2;
	m_Width = 80;
	m_SFrames = 1;
	m_SRate = 1;
	m_bFromAVI = 0;
//	m_Name = _T("");
//	m_Location = _T("h:\\sketch32");
//	m_Location = _T("");
	//}}AFX_DATA_INIT
}


void CNewDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewDlg)
	DDX_Text(pDX, IDC_NEW_FRAMES, m_Frames);
	DDV_MinMaxUInt(pDX, m_Frames, 1, m_MaxFrames);
	DDX_Text(pDX, IDC_NEW_HEIGHT, m_Height);
	DDV_MinMaxUInt(pDX, m_Height, 60, m_MaxHeight);
	DDX_Text(pDX, IDC_NEW_LEVELS, m_Levels);
	DDV_MinMaxUInt(pDX, m_Levels, 2, m_MaxLevels);
	DDX_Text(pDX, IDC_NEW_WIDTH, m_Width);
	DDV_MinMaxUInt(pDX, m_Width, 80, m_MaxWidth);
	DDX_Text(pDX, IDC_NEW_RATE, m_Rate);
	DDV_MinMaxUInt(pDX, m_Rate, 1, 60);
//	DDX_Text(pDX, IDC_NAME, m_Name);
//	DDV_MaxChars(pDX, m_Name, 50);
//	DDX_Text(pDX, IDC_LOCATION, m_Location);
//	DDV_MaxChars(pDX, m_Location, 100);
//	DDX_Radio(pDX, IDC_NEW_QTR, m_Kind);
	DDX_Radio(pDX, IDC_NEW_FACT_100, m_Factor);
	DDX_Radio(pDX, IDC_NEW_PREV1, m_Preview);
	DDX_Radio(pDX, IDC_BROADCAST0, m_bcast);
//	DDX_Radio(pDX, IDC_NEW_STORY, m_bXSheet);
	//DDX_Text(pDX, IDC_NEW_SFRAMES, m_SFrames);
//	DDV_MinMaxUInt(pDX, m_SFrames, 1, 1000);
//	DDX_Text(pDX, IDC_NEW_SRATE, m_SRate);
//	DDV_MinMaxUInt(pDX, m_SRate, 1, 60);
	if (pDX->m_bSaveAndValidate)
		{
		if (m_nJpeg)
			{
			DDX_Text(pDX, IDC_NEW_QUALITY, m_nJpeg);
			DDV_MinMaxInt(pDX, m_nJpeg,25,100);
			}
		}
	else
		{
		BOOL bJpeg = m_nJpeg ? 1 : 0;
		DDX_Check(pDX, IDC_NEW_JPEG, bJpeg);
		DDX_Text(pDX, IDC_NEW_QUALITY, m_nJpeg);
		}
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewDlg, CMyDialog)
	//{{AFX_MSG_MAP(CNewDlg)
//	ON_BN_CLICKED(IDC_CHANGE, OnChange)
	ON_BN_CLICKED(IDC_NEW_QTR, OnResQtr)
	ON_BN_CLICKED(IDC_NEW_FULL, OnResFull)
	ON_BN_CLICKED(IDC_NEW_NTSC, OnResNtsc)
	ON_BN_CLICKED(IDC_NEW_PAL, OnResPal)
	ON_BN_CLICKED(IDC_NEW_HDTV, OnResHdtv)
	ON_BN_CLICKED(IDC_NEW_NTSC540,OnResNuNtsc)
	ON_BN_CLICKED(IDC_NEW_PAL720,OnResNuPal)
//	ON_BN_CLICKED(IDC_NEW_ACAD, OnResAcad)
	ON_BN_CLICKED(IDC_NEW_CUST, OnResCust)
//	ON_BN_CLICKED(IDC_NEW_STORY, OnStory)
//	ON_BN_CLICKED(IDC_NEW_XSHEET, OnXSheet)
	ON_BN_CLICKED(IDC_NEW_JPEG, OnJpeg)
	ON_BN_CLICKED(IDC_FROM_AVI, OnFromAVI)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewDlg message handlers

BOOL CNewDlg::OnInitDialog()
{ 
DPF("new init");
DPF("w:%d,h:%d",m_Width,m_Height);
	NewRes(m_Width, m_Height);
	CMyDialog::OnInitDialog();
	BOOL bJpeg = m_nJpeg ? 1 : 0;
	UINT show = bJpeg ? SW_SHOW : SW_HIDE;
	CheckDlgButton(IDC_NEW_JPEG, bJpeg);
	GetDlgItem(IDC_NEW_QUALTEXT)->ShowWindow(show);
	GetDlgItem(IDC_NEW_QUALITY)->ShowWindow(show);
	GetDlgItem(IDC_FROM_AVI)->ShowWindow(
				MyApp->IsAnimate() ? SW_SHOW : SW_HIDE);
	CenterWindow();
	return FALSE;
}

void CNewDlg::NewRes(int w, int h)
{
//	m_Width = w;
//	m_Height = h;
	int idd;
	if  (w == 320 && h == 240)
		idd = IDC_NEW_QTR;
	else if (w == 640 && h == 480)
		idd = IDC_NEW_FULL;
	else if (w == 720 && h == 480)
		idd = IDC_NEW_NTSC;
	else if (w == 768 && h == 576)
		idd = IDC_NEW_PAL;
	else if (w == 720 && h == 540)
		idd = IDC_NEW_NTSC540;
	else if (w == 720 && h == 576)
		idd = IDC_NEW_PAL720;
	else if (w == 1920 && h == 1080)
		idd = IDC_NEW_HDTV;
//	else if (w == 3000 && h == 2000) ||
//		id = IDC_NEW_ACAD;
	//	bCustom = 0;
	else
		idd = IDC_NEW_CUST;
	
	BOOL bCustom = idd == IDC_NEW_CUST ? TRUE : FALSE;
	if (!bCustom)
		{
		m_Width = w;
		m_Height = h;
		}
	CheckRadioButton(IDC_NEW_QTR, IDC_NEW_CUST, idd);
	GetDlgItem(IDC_NEW_WIDTH)->EnableWindow(bCustom);
	GetDlgItem(IDC_NEW_HEIGHT)->EnableWindow(bCustom);
//	if (bCustom)
		{
		SetDlgItemInt(IDC_NEW_WIDTH, m_Width,0);
		SetDlgItemInt(IDC_NEW_HEIGHT, m_Height,0);
		}
}

void CNewDlg::OnFromAVI()
{
	m_bFromAVI = 1;
	OnOK();
}

void CNewDlg::OnJpeg()
{
	if (m_nJpeg)
		m_nJpeg = 0;
	else
		{
		GetDlgItem(IDC_NEW_QUALITY)->SetWindowText("80");
		m_nJpeg = 80;
		}
	BOOL bShow = m_nJpeg ? 1 : 0;
	UINT show = m_nJpeg ? SW_SHOW : SW_HIDE;
	GetDlgItem(IDC_NEW_QUALTEXT)->ShowWindow(show);
	GetDlgItem(IDC_NEW_QUALITY)->ShowWindow(show);
}
/*
void CNewDlg::OnStory()
{
}

void CNewDlg::OnXSheet()
{
}
*/
/*
void CNewDlg::OnChange() 
{
//	DPF("loc:%s",(LPCSTR)m_Location);
	CDirDialog ddlg(m_Location,
        "||",
 //       "Executable files (*.exe;*.dll)|*.exe;*.dll|All Files (*.*)|*.*||",
		this);//AfxGetApp()->m_pMainWnd);//this);
    int res = ddlg.DoModal();
	if (res == IDOK)
		{
		UpdateData(TRUE);
		m_Location = (LPCSTR)ddlg.GetPath();
DPF("new got:%s",(LPCSTR)m_Location);//ddlg.GetPath());
		UpdateData(FALSE);
		}
}

*/

CFromAVIDlg::CFromAVIDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CFromAVIDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFromAVIDlg)
	//}}AFX_DATA_INIT
}


void CFromAVIDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFromAVIDlg)
//	DDX_Text(pDX, IDC_NEW_FRAMES, m_Frames);
//	DDV_MinMaxUInt(pDX, m_Frames, 1, m_MaxFrames);
//	DDX_Text(pDX, IDC_NEW_HEIGHT, m_nHeight);
//	DDV_MinMaxUInt(pDX, m_Height, 60, m_MaxHeight);
	DDX_Text(pDX, IDC_NEW_LEVELS, m_Levels);
//	DDV_MinMaxUInt(pDX, m_Levels, 2, m_MaxLevels);
//	DDX_Text(pDX, IDC_NEW_WIDTH, m_nWidth);
//	DDV_MinMaxUInt(pDX, m_Width, 80, m_MaxWidth);
	DDX_Text(pDX, IDC_NEW_RATE, m_nRate);
//	DDV_MinMaxUInt(pDX, m_Rate, 1, 60);
//	DDX_Text(pDX, IDC_NAME, m_Name);
//	DDV_MaxChars(pDX, m_Name, 50);
//	DDX_Text(pDX, IDC_LOCATION, m_Location);
//	DDV_MaxChars(pDX, m_Location, 100);
//	DDX_Radio(pDX, IDC_NEW_QTR, m_Kind);
//DDX_Radio(pDX, IDC_NEW_FACT_100, m_Factor);
//	DDX_Radio(pDX, IDC_NEW_PREV1, m_Preview);
//	DDX_Radio(pDX, IDC_BROADCAST0, m_bcast);
//	DDX_Radio(pDX, IDC_NEW_STORY, m_bXSheet);
	//DDX_Text(pDX, IDC_NEW_SFRAMES, m_SFrames);
//	DDV_MinMaxUInt(pDX, m_SFrames, 1, 1000);
//	DDX_Text(pDX, IDC_NEW_SRATE, m_SRate);
//	DDV_MinMaxUInt(pDX, m_SRate, 1, 60);
	DDX_Check(pDX, IDC_USE_SOUND, m_bUseSound);
	if (pDX->m_bSaveAndValidate)
		{
		if (m_nJpeg)
			{
			DDX_Text(pDX, IDC_NEW_QUALITY, m_nJpeg);
			DDV_MinMaxInt(pDX, m_nJpeg,25,100);
			}
		}
	else
		{
		BOOL bJpeg = m_nJpeg ? 1 : 0;
		DDX_Check(pDX, IDC_NEW_JPEG, bJpeg);
		DDX_Text(pDX, IDC_NEW_QUALITY, m_nJpeg);
		}
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFromAVIDlg, CMyDialog)
	//{{AFX_MSG_MAP(CFromAVIDlg)
	ON_BN_CLICKED(IDC_NEW_JPEG, OnJpeg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFromAVIDlg message handlers

BOOL CFromAVIDlg::OnInitDialog()
{ 
DPF("from avi dlg init");
DPF("w:%d,h:%d",m_nWidth,m_nHeight);
	CMyDialog::OnInitDialog();
	BOOL bJpeg = m_nJpeg ? 1 : 0;
	UINT show = bJpeg ? SW_SHOW : SW_HIDE;
	GetDlgItem(IDC_NEW_QUALTEXT)->ShowWindow(show);
	GetDlgItem(IDC_NEW_QUALITY)->ShowWindow(show);
	SetDlgItemInt(IDC_NEW_WIDTH, m_nWidth);
	SetDlgItemInt(IDC_NEW_HEIGHT, m_nHeight);
	SetDlgItemInt(IDC_NEW_FRAMES, m_Frames);
	if (m_bUseSound)
		CheckDlgButton(IDC_USE_SOUND, 1);
	else
		GetDlgItem(IDC_USE_SOUND)->ShowWindow(SW_HIDE);
	char buf[100];
	if (m_Frames > m_MaxFrames)
		{
sprintf(buf,"This Movie Has Too Many Frames, Truncated to %d",m_MaxFrames);
		m_Frames = m_MaxFrames;
		SetDlgItemText(IDC_AVI_MSG1, buf);
		}
	else
		SetDlgItemText(IDC_AVI_MSG1, " ");
	m_bScaleIt = 0;
	if (m_nWidth > m_MaxWidth)
		{
		m_bScaleIt = 1;
		m_nWidth = m_MaxWidth;
		}
	if (m_nHeight > m_MaxHeight)
		{
		m_bScaleIt = 1;
		m_nHeight = m_MaxHeight;
		}
	if (m_bScaleIt)
		{
sprintf(buf,"This Movie Is Too Large, Scaling to %d X %d",m_MaxWidth, m_MaxHeight);
		SetDlgItemText(IDC_AVI_MSG2, buf);
		}
	else
		SetDlgItemText(IDC_AVI_MSG2, " ");
	CenterWindow();
	return FALSE;
}

void CFromAVIDlg::OnJpeg()
{
	if (m_nJpeg)
		m_nJpeg = 0;
	else
		{
		GetDlgItem(IDC_NEW_QUALITY)->SetWindowText("80");
		m_nJpeg = 80;
		}
	BOOL bShow = m_nJpeg ? 1 : 0;
	UINT show = m_nJpeg ? SW_SHOW : SW_HIDE;
	GetDlgItem(IDC_NEW_QUALTEXT)->ShowWindow(show);
	GetDlgItem(IDC_NEW_QUALITY)->ShowWindow(show);
}



CInitialDlg::CInitialDlg(CWnd* pParent /*=NULL*/)
: CMyDialog(CInitialDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInitialDlg)
	m_Name = _T("");
	m_bCanExpire = 0;
	//}}AFX_DATA_INIT
}

BEGIN_MESSAGE_MAP(CInitialDlg, CMyDialog)
	//{{AFX_MSG_MAP(CInitialDlg)
	ON_BN_CLICKED(IDC_INIT_NEW, OnNew)
	ON_BN_CLICKED(IDC_INIT_OPEN, OnOpen)
	ON_BN_CLICKED(IDC_INIT_PREV, OnPrev)
	ON_BN_CLICKED(IDC_INIT_ABOUT, OnAbout)
	ON_BN_CLICKED(IDC_INIT_REGISTER, OnRegister)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFromAVIDlg message handlers

BOOL CInitialDlg::OnInitDialog()
{ 
DPF("initial init");
	CMyDialog::OnInitDialog();
#ifdef FLIPBOOK_MAC
	// for Mac we want to just hide the button, because it doesn't look right on the blue background when disabled
	if (!m_bPrevious) {
		GetDlgItem(IDC_INIT_PREV)->ShowWindow (SW_HIDE);
	}

	if (m_bRegistered) {
		GetDlgItem(IDC_INIT_REGISTER)->ShowWindow (SW_HIDE);
	}
#else
	GetDlgItem(IDC_INIT_PREV)->EnableWindow(m_bPrevious);
//	GetDlgItem(IDC_INIT_REGISTER)->ShowWindow(m_bRegistered ? SW_HIDE : SW_SHOW);
	if (m_bCanExpire)
		GetDlgItem(IDC_INIT_REGISTER)->ShowWindow(SW_HIDE);
	else if (m_bRegistered)
		GetDlgItem(IDC_INIT_REGISTER)->SetWindowText("Check For Update");
#endif
	CenterWindow();
	return FALSE;
}

