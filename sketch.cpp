#include "stdafx.h"
#include <afxsock.h>		// MFC socket extensions
#include "afxdisp.h"
#include "sketch.h"
#include "mainfrm.h"
#include "mydoc.h"
#include "myview.h"
#include "prevdlg.h"
#include "newdlg.h"
#ifndef FLIPBOOK_MAC
#include "htmlhelp.h"
#endif
#include "ctablet.h"
#include "mydialog.h"
#include "dialogs.h"
#include "myid.h"
#include "myusb.h"
#include "crypto.h"
#ifndef FBTPC
#include "wintab.h"
#endif
#include "FBQT.h"
#include "myio.h"
#include "cscene.h"
#include "clicense.h"
#include "cgfiltyp.h"
#include "cmaya.h"
#include "myver.h"
//#include <afxsock.h>		// MFC socket extensions
#include <afxinet.h>
#include "WinResourceLoader.h"


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//#define RED_BULL

#ifdef RED_BULL
#define EXPIRES
#define EXPIRE_DATE 11182011
//#define TESTCODE 4
#define TESTOPTIONS (BIT_CAMERA | BIT_HIRES)
//#define EXPIRE_NAME "Red Bull"
#define EXPIRE_CODE "Red Bull Canimation"
#define TEST_ID 1234567890
#endif

#ifdef DOMYBUG
#define RIXBITS (BIT_PLUS | BIT_LIBRARY | BIT_HIRES | BIT_DESKEW)
//#define RIXBITS (BIT_LIBRARY | BIT_HIRES | BIT_DESKEW | 4 | BIT_CAMERA)
#endif

#include "afxpriv.h"
#define TESTING
//#define EXPIRES
//#define EXPIRE_DATE 8312006
//#define EXPIRE_COUNT 500 // max is 9999
#ifdef TESTING
//
//	*** WARNING, WARNING
//
// this is for testing only, and avoids maiking new keys
//	STD is 1, LITE is 2, PT is 3 PRO is 4, DEMO is 0
// options are sum of above bits, or 0
#ifdef _DISNEY
#define TESTCODE 1
#define TESTOPTIONS 0
#else
#define TEST_ID 1234567890
//#define TESTCODE 4
#define TESTOPTIONS (BIT_CAMERA | BIT_COLORKEY | BIT_HIRES)
#endif
#endif // testing

//#define ASKOLDKEY
//#if defined (FLIPBOOK_MAC) && (MYVER <= 470)
//#define ACCEPTOLDKEY
//#endif


class CBiggy
{
public:
	CBiggy();
   // inline void operator=(CBiggy& biggy) {l = biggy.l;};
	void SetVer(int ver) { m_nVer = ver;};
	void Assign(int a, int b, int c, int d);
	void Prepare();
	void Extract(UINT &ver, UINT &idk, UINT & days);
	void * Addr() { return & l[0];};
	BOOL Eval(UINT idc, UINT idk, UINT idz);
	BOOL Eval2(UINT msgc, UINT sysc, UINT sysid);
//private:
	int  m_nVer;
	UINT l[2];
};

/////////////////////////////////////////////////////////////////////////////
// CSketchApp

BEGIN_MESSAGE_MAP(CSketchApp, CWinApp)
	//{{AFX_MSG_MAP(CSketchApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
	ON_COMMAND(ID_APP_HELP, OnHelp)
#ifndef EXPIRES
	ON_COMMAND(ID_APP_REGISTER, OnAppRegister)
	ON_COMMAND(ID_APP_UPDATES, OnUpdates)
#endif
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSketchApp construction

#define FLAG_COLOR 1
#define FLAG_CAMERA 2
#define FLAG_LAYERS 4

/////////////////////////////////////////////////////////////////////////////
// The one and only CSketchApp object
CSketchApp theApp;
int checkdigit(char * txt, int nAppend = 0);

/////////////////////////////////////////////////////////////////////////////
// CSketchApp initialization
//int __argc;
//char ** __argv;


BOOL CSketchApp::InitInstance()
{
	m_dwLicense = 0;
	m_pMaya = 0;
	m_bHaveWinTab = 0;
//	m_bNewProfiles = 0;
DPF("init instance");
	#ifdef FLIPBOOK_MAC
#ifdef USEQT
	FBQuickTimeInit();
#endif
	#endif

	InitCommonControls();
	AfxEnableControlContainer();
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.
//	SetDialogBkColor();        // Set dialog background color to gray
#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	//Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
	m_dwRemindFlags = 0;
	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
//	SetRegistryKey(_T("DigiCel FlipBook"));
	InitProfile();
	LoadStdProfileSettings();  // Load standard INI file options (including MRU)
	if (CheckProfile())
		return FALSE;
	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.
// 	CSplashDlg *ptr = new CSplashDlg(NULL, IDB_SPLASH, 4);
//	ptr->Create(IDD_SPLASH);
	if (GetProfileInt("Options","Check Updates", 0))
		{
		if (CheckForUpdate())
			return FALSE;
		}
	m_dwLicense = GetKeyInt(10) ? 2 : 0;
	m_dwRemindFlags = GetProfileInt("Options","Remind Flags", 0);
	if (IdSetup())
		return FALSE;
	// Check if Wintab is available
	m_bHaveWinTab = 0;
#ifndef FLIPBBOK_MAC
	if (!CTablet::HaveWinTab())
		{
		m_bHaveWinTab = FALSE;
		CString string;
		if (string.LoadString(WINTAB) &&
				AfxGetApp()->GetProfileInt("Options",(LPCSTR)string, 1))
			{
			AfxMessageBox(IDS_NOWINTAB);
			AfxGetApp()->WriteProfileInt("Options", (LPCSTR)string, 0);
			}
		}
	else
		m_bHaveWinTab = 1;
#endif
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
#ifdef _DISNEY
		IDR_COLORFRAME,
#else
		IDR_MAINFRAME,
#endif
		RUNTIME_CLASS(CSketchDoc),
		RUNTIME_CLASS(CMainFrame),     // main SDI frame window
		RUNTIME_CLASS(CSketchView));
	AddDocTemplate(pDocTemplate);

	// simple command line parsing
//	WriteProfileInt("Settings","Startup",1);
	if (CheckCommandLine())
		{
		if (GetProfileInt("Options","Open Previous", 0) ||
			GetProfileInt("Options","Not Closed", 0))
			{
			CString name;
			name = GetProfileString("Recent File List","File1");
			if (name.GetLength())
				{
DPF("opening:%s",(LPCSTR)name);
				if (MyOpenFile(name))
					return TRUE;
DPF("after open");
				}
			WriteProfileInt("Options","Open Previous", 0);
			}
		if (OnInitial())
			return FALSE;
	} else if (!m_pMaya)
	{
		// open an existing document
		MyOpenFile(m_lpCmdLine);
	}

	#ifndef FLIPBOOK_MAC
	FBQuickTimeInit();
	#endif

	return TRUE;
}

BOOL zTest(char * p, char * q)
{
	int i;
	for (i = 0;p[i];i++)
		if (p[i] != q[i])
			break;
	return !p[i] ? i : 0;
}

BOOL CSketchApp::CheckCommandLine()
{
	char name[300];
	if (m_lpCmdLine[0] == '\0')
		return 1;
	int l;
//return 1;
	if (!(l = zTest("Filename=",m_lpCmdLine)))
		return 0;
	strcpy(name,m_lpCmdLine+l);
	if (!AfxSocketInit())
		{
		AfxMessageBox("IDP_SOCKETS_INIT_FAILED");
		return 1;
		}

	m_pMaya = new CMaya;
	int res = m_pMaya->Init(name);
	if (res)
		{
		FormattedMsg(IDS_ERR_MAYA_INIT,res);
		delete m_pMaya;
		m_pMaya = 0;
		return 1;
		}
	OnFileNew();
	return 0;
}

void CSketchApp::OnFileOpen()
{
	CString newName;
	if (!PromptFileName(newName,0))
		return;
//	GetApFeatures();
	MyOpenFile(newName);
		// if returns NULL, the user has already been alerted
}


BOOL CSketchApp::MyOpenFile(LPCSTR name)
{
	if (m_dwLicense > 1)
		ChooseLicense();
	if (OpenDocumentFile(name))
		return TRUE;
	else
		return FALSE;
}

void CSketchApp::OnFileNew()
{
	DPF("my on file new");
//	GetApFeatures();
	if (m_dwLicense > 1)
		ChooseLicense();
	CWinApp::OnFileNew();
}

BOOL CSketchApp::TestScene(LPCSTR Name)
{
	CMyIO * pIO = new CMyIO;
	if (!pIO)
		return 0;
	pIO->Close(TRUE);
	if (pIO->Open(Name))
		{
		delete pIO;
		return 0;
		}
	CScene * pScene = new CScene(pIO, std::make_unique<WinResourceLoader>());
	if (!pScene)
		{
		delete pIO;
		return 0;
		}
	BOOL bResult = pScene->Read(TRUE) ? 0 : 1;
	delete pScene;
	delete pIO;
	return bResult;
}

BOOL CSketchApp::OnInitial()
{
	DPF("initial new");
	DEVMODE devmode;	
	if (::EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode))
		{
  //  sDevMode.Format("%i x %i, %i bits %dhtz",
   	int w =  devmode.dmPelsWidth;
	int h = devmode.dmPelsHeight;
	int b = devmode.dmBitsPerPel;
	int f = devmode.dmDisplayFrequency;
	DPF("w:%d,h:%d,b:%d,f:%d",w,h,b,f);
		if (devmode.dmBitsPerPel < 16)
			{
			AfxMessageBox("FlipBook requires more than 256 colors");
			return TRUE;
			}
		}
	Association();

/*
	if (m_dwLicense > 1)
		{
		if (ChooseLicense())
			return TRUE;
		}
*/
	CInitialDlg dlg;
//	dlg.m_expire_count = 1;
//	dlg.m_expire_days = 
	CString name = GetProfileString("Recent File List","File1");
#ifdef EXPIRES
	dlg.m_bCanExpire = 1;
#endif
	if (name.GetLength() && TestScene((LPCSTR)name))
		dlg.m_bPrevious = TRUE;
	else
		dlg.m_bPrevious = FALSE;
	dlg.m_bRegistered = (m_dwFeatures & 15) ? TRUE : FALSE;
	for (;;)
		{
		if (dlg.DoModal() != IDOK)
			return TRUE;
DPF("anser:%d",dlg.m_answer);
		if (dlg.m_answer == 1)
			OnFileNew();
		else if (dlg.m_answer == 2)
			OnFileOpen();
		else if (dlg.m_answer == 4)
			OnAppAbout();
		else if (dlg.m_answer == 5)
			{
			if (!dlg.m_bRegistered)
				OnAppRegister();
			else
				OnUpdates();
			}
		else
			{
DPF("open previous");
			name = GetProfileString("Recent File List","File1");
			if (name.GetLength())
				{
DPF("init opening:%s",(LPCSTR)name);
				MyOpenFile(name);
DPF("after init open");
				}
			}
#ifdef FLIPBOOK_MAC
		// the window creation order is a little different on the Mac; just check if it's visible yet
		if (m_pMainWnd->IsWindowVisible())
			break;
#else
		if (m_pMainWnd)
			break;
#endif
		}
	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CMyDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA
//	CString m_Name;
	CString m_Code;
//	CString m_Scene;
// Implementation
	UINT m_days;
	DWORD m_dwFeatures;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
	afx_msg void OnDetails() ;
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CMyDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
//	DDX_Text(pDX, IDC_NAME, m_Name);
	DDX_Text(pDX, IDC_KEYCODE, m_Code);
//	DDX_Text(pDX, IDC_SCENECODE, m_Scene);
	//}}AFX_DATA_MAP
}

BOOL CAboutDlg::OnInitDialog()
{ 
	int y3 = 98;
	CMyDialog::OnInitDialog();
	OEM_TEXT(this);
	char product[100];
	char temp[20];
	GetDlgItem(IDC_VERSION)->GetWindowText(product,99);
	OEM_BUF(product);
	temp[0] = '0' + (MYVER / 100);
	temp[1] = '.';
	temp[2] = '0' + (MYVER % 100) / 10;
	if ((MYVER % 100) % 10)
		temp[3] = '0' + (MYVER % 100) % 10;
	else
		temp[3] = ' ';
	temp[4] = 0;
	strcat(product, temp);
//	strcat(product, MYSTRPRODVERSION);
	SetDlgItemText(IDC_VERSION,product);
	int y1 = 1000;		// to discourage people from patching copyright msg
	int y2 = 1235;
	y3 <<= 3;
	y3 -= 6;
	y3 += y2;
#if MAC
	sprintf(product,"Bnoxqhfgs %c%c %d-%d ChfhBdk Hmb.",194,169,y1+1000,y3); // Mac uses UTF8 now
#else
	sprintf(product,"Bnoxqhfgs %c %d-%d ChfhBdk Hmb.",169,y1+1000,y3);
#endif
	int i;
	for (i = 0; product[i]; i++)
		if (product[i] >= 'A') product[i]++;
	SetDlgItemText(IDC_COPYRIGHT,product);
	OEM_DLG(this, IDD_ABOUTBOX2);
	CString string;
	CSketchApp* pApp = (CSketchApp*) AfxGetApp();
	UINT flags = m_dwFeatures & 15;
	CMainFrame * pFrame =(CMainFrame *)pApp->m_pMainWnd;
	if (pFrame && pFrame->SceneState())
		flags = 0;
	if (!string.LoadString(IDS_DEMO + (flags & 15)))
		string = "DEMO MODE";
	SetDlgItemText(IDC_APP_MODE,string);
	CheckDlgButton(IDC_FEAT_TELECINE,pApp->CanDoFeature(CSketchApp::CD_TELECINE));
	CheckDlgButton(IDC_FEAT_DESKEW  ,pApp->CanDoFeature(CSketchApp::CD_DESKEW));
	CheckDlgButton(IDC_FEAT_HIRES   ,pApp->CanDoFeature(CSketchApp::CD_HIRES));
	CheckDlgButton(IDC_FEAT_COLORKEY,pApp->CanDoFeature(CSketchApp::CD_COLORKEY));
	CheckDlgButton(IDC_FEAT_LAYERS,pApp->CanDoLayers());
	CheckDlgButton(IDC_FEAT_CAMERA,pApp->CanDoCamera());
	if (pApp->IsAnimate())
		CheckDlgButton(IDC_FEAT_PLUS, pApp->IsAnimate());
	else
		{
		GetDlgItem(IDC_FEAT_PLUS)->ShowWindow(SW_HIDE);
#if !MAC
		GetDlgItem(IDC_FEAT_PLUSX)->ShowWindow(SW_HIDE);
#endif
		}
	CheckDlgButton(IDC_FEAT_LIBRARY, pApp->CanDoLibrary());

	if (m_days)
		{
		CTimeSpan dt(m_days,0,0,0);
		CTime base(2012,1,1,0,0,0);
		base += dt;
		CString z = base.Format("%B %d, %Y");
		GetDlgItem(IDC_APP_EXPIRES)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_APP_EXP_DATE)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_APP_EXP_DATE)->SetWindowText((LPCSTR)z);
		}
	CenterWindow();
	return FALSE;
}

BEGIN_MESSAGE_MAP(CAboutDlg, CMyDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	ON_COMMAND(IDC_DETAILS, OnDetails)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

class CDetailDlg : public CMyDialog
{
// Construction
public:
	CDetailDlg(CWnd * pParent = NULL);	// standard constructor
	enum { IDD = IDD_DETAILS };
protected:
	virtual BOOL OnInitDialog();
	void Text(int iid, LPCSTR msg);
};

CDetailDlg::CDetailDlg(CWnd * pParent /* = NULL */)
			: CMyDialog(CDetailDlg::IDD, pParent)
{
}


void CDetailDlg::Text(int iid, LPCSTR msg)
{
	SetDlgItemText(iid,msg);
}

BOOL CDetailDlg::OnInitDialog()
{
	CString OsStr;
	CMyId * pId = new CMyId;
	int os = pId->GetOsStr(OsStr);
	Text(IDC_ABT_OS,(LPCSTR)OsStr);
	char temp[100];
  	DWORD dwTemp = pId->determineCpuSpeed();
	dwTemp = 50 * ((dwTemp + 49) / 50);
  	sprintf (temp, "%d mHz",dwTemp);
	Text(IDC_ABT_SPEED,temp);
	sprintf (temp, "%d%%",pId->MemoryLoad());
	Text(IDC_ABT_LOAD,temp);
  	sprintf (temp, "%d MB", pId->TotalPhysical()/1024);
	Text(IDC_ABT_TOTAL_RAM,temp);
  	sprintf (temp, "%d MB", pId->AvailPhysical()/1024);
	Text(IDC_ABT_AVAIL_RAM,temp);
  	sprintf (temp, "%d MB", pId->TotalVirtual()/1024);
	Text(IDC_ABT_TOTAL_VIRT,temp);
  	sprintf (temp, "%d MB", pId->AvailVirtual()/1024);
	Text(IDC_ABT_AVAIL_VIRT,temp);
	delete pId;
	CMyDialog::OnInitDialog();
	return TRUE;  // return TRUE  unless you set the focus to a control
}


void CAboutDlg::OnDetails() 
{
	CDetailDlg dlg;
	dlg.DoModal();
}

// App command to run the dialog
void CSketchApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
#ifdef EXPIRE_DATE
	aboutDlg.m_Code = EXPIRE_CODE;
//	aboutDlg.m_Name = EXPIRE_NAME;
	
	aboutDlg.m_dwFeatures = m_dwFeatures;
#else
	aboutDlg.m_days = m_days;
DPF("idk:%d,id:%8lx",m_dwIdk,m_dwId);
//	if (m_pLicense)
//		aboutDlg.m_Name = m_pLicense->m_client;
//	else
//		aboutDlg.m_Name = GetProfileString("Settings","User Name");
	char buf[55];
	DWORD cc;
	DWORD idc = m_dwIdk;
	if (!idc)
		cc = 'U';
	else if (idc == 1)
		cc = 'M';
	else if (idc == 2)
		cc = 'S';
	else if (idc == 3)
		cc = 'V';
	else if (idc < 7)
		cc = 'N' + idc - 4;
	else
		cc = '?';
	if (m_dwLicense)
		aboutDlg.m_Code = "Educational";
	else if (idc > 6)
		aboutDlg.m_Code = "Unregistered";
	else
		{
#ifndef ASKOLDKEY
	#ifdef FLIPBOOK_MAC
		char macc = 'A';
		UINT mac = 1;
	#else
		char macc = 'W';
		UINT mac = 0;
	#endif
		UINT ver = MYVER; // e.g.467
		sprintf(buf,"%d%d%03d%05d%05d",mac,idc,ver,
					m_dwId / 100000, m_dwId %100000);
		int v = checkdigit(buf,1);
		sprintf(buf,"%c%c%03d-%05d-%05d-%d",macc,cc,ver,
				m_dwId / 100000, m_dwId % 100000,v);
#else
		sprintf(buf,"%c%05d-%05d",cc,m_dwId / 100000, m_dwIdz % 100000);
#endif
	aboutDlg.m_Code = buf;
		}
	aboutDlg.m_dwFeatures = m_dwFeatures;
#endif
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CRegisterDlg dialog used for App About

class CRegisterDlg : public CMyDialog
{
public:
	CRegisterDlg();

// Dialog Data
	//{{AFX_DATA(CRegisterDlg)
#ifdef _THEDISC
	enum { IDD = IDD_REGISTER_DISC };
#else
	enum { IDD = IDD_REGISTER };
#endif
	//}}AFX_DATA
//	CString m_Name;
	CString m_Id;
	CString m_Code;
//	UINT m_flags;
// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
	afx_msg void OnRegIt();
	DECLARE_MESSAGE_MAP()
};

CRegisterDlg::CRegisterDlg() : CMyDialog(CRegisterDlg::IDD)
{
	//{{AFX_DATA_INIT(CRegisterDlg)
	//}}AFX_DATA_INIT
}

void CRegisterDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRegisterDlg)
//	DDX_Text(pDX, IDC_NAME, m_Name);
	DDX_Text(pDX, IDC_APID, m_Id);
	DDX_Text(pDX, IDC_CODE, m_Code);
	//}}AFX_DATA_MAP
}

BOOL CRegisterDlg::OnInitDialog()
{ 
	CMyDialog::OnInitDialog();
/*
	OEM_TEXT(this);
	OEM_DLG(this, IDD_REGISTER1);
	OEM_DLG(this, IDD_REGISTER2);
	OEM_DLG(this, IDD_REGISTER3);
	OEM_DLG(this, IDD_REGISTER4);
	OEM_DLG(this, IDD_REGISTER5);
	OEM_DLG(this, IDD_REGISTER6);
	OEM_DLG(this, IDD_REGISTER7);
	OEM_DLG(this, IDC_REG_IT);
*/
	GetDlgItem(IDC_CODE)->SetFocus();
	CenterWindow();
	return FALSE;
}

void CRegisterDlg::OnRegIt()
{
	EndDialog(IDCANCEL2);
}

BEGIN_MESSAGE_MAP(CRegisterDlg, CMyDialog)
	//{{AFX_MSG_MAP(CRegisterDlg)
	ON_COMMAND(IDC_REG_IT, OnRegIt)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//
//	klugey to check for a number with identical digits
//
BOOL zz(int z)
{
	char buf[10];
	sprintf(buf,"%d",z);
	if (!buf[0]) return 1;
	int i;
	for (i =1; buf[i];i++)
		if (buf[i] != buf[0])
			break;
	return buf[i] ? 0 : 1;
}



/////////////////////////////////////////////////////////////////////////////
// COnLineDlg dialog used for on line register

class COnLineDlg : public CMyDialog
{
public:
	COnLineDlg();

// Dialog Data
	//{{AFX_DATA(COnLineDlg)
	enum { IDD = IDD_REGISTER_IT };
	//}}AFX_DATA
	CString m_Name;
	CString m_Id;
	CString m_email1;
	CString m_email2;
	CString m_order;
	int m_kind;
// Implementation
protected:
	void OnKind(int v);
	void CheckEMail(CDataExchange* pDX, CString &str);    // DDX/DDV support
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
	afx_msg void OnOurs() { OnKind(0);};
	afx_msg void OnOther() {OnKind(1);};
	DECLARE_MESSAGE_MAP()
};

COnLineDlg::COnLineDlg() : CMyDialog(COnLineDlg::IDD)
{
	//{{AFX_DATA_INIT(COnLineDlg)
	//}}AFX_DATA_INIT
}

void COnLineDlg::CheckEMail(CDataExchange* pDX, CString &str)
{
	if (pDX->m_bSaveAndValidate)
		{
		int i, c;
		if (!(c = str.GetLength()))
			{
			AfxMessageBox(IDS_NO_EMAIL);
			pDX->Fail();
			}
		else
			{
			LPCSTR p = (LPCSTR)str;
			for (i = 0; (i < c) && (p[i] != '@'); i++);
			if (i >= c)		
				{
				AfxMessageBox(IDS_EMAIL_BAD);
				pDX->Fail();
				}
			}
		}
}
void COnLineDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COnLineDlg)
	DDX_Text(pDX, IDC_NAME, m_Name);
	if (pDX->m_bSaveAndValidate && !m_Name.GetLength())
		{
		AfxMessageBox(IDS_NO_NAME);
		pDX->Fail();
		}
	DDX_Text(pDX, IDC_APID, m_Id);
	if (pDX->m_bSaveAndValidate && !m_Id.GetLength())
		{
		OEMMessageBox(IDS_NO_ID);
		pDX->Fail();
		}
	DDX_Text(pDX, IDC_EMAIL1, m_email1);
	CheckEMail(pDX, m_email1);
	DDX_Text(pDX, IDC_EMAIL2, m_email2);
	CheckEMail(pDX, m_email2);
	if (pDX->m_bSaveAndValidate && (m_email1 != m_email2))
		{
		AfxMessageBox(IDS_EMAIL_MISMATCH);
		pDX->Fail();
		}
	DDX_Text(pDX, IDC_ORDER_NBR, m_order);
	if (pDX->m_bSaveAndValidate && !m_kind && !m_order.GetLength())
		{
		AfxMessageBox(IDS_NO_ORDER);
		pDX->Fail();
		}
}

BOOL COnLineDlg::OnInitDialog()
{ 
	CMyDialog::OnInitDialog();
	OEM_TEXT(this);
	OEM_DLG(this, IDC_OTHER_TEXT);
	OEM_DLG(this, IDC_EXP_BAD);
	OnKind(0);
	CenterWindow();
	GetDlgItem(IDC_NAME)->SetFocus();
	return FALSE;
}

void COnLineDlg::OnKind(int v)
{
	m_kind = v;
	CheckRadioButton(IDC_OUR_ORDER,IDC_OTHER_ORDER,IDC_OUR_ORDER+m_kind);
	GetDlgItem(IDC_OUR_TEXT)->ShowWindow(m_kind ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDC_ORDER_NBR)->ShowWindow(m_kind ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDC_OTHER_TEXT)->ShowWindow(!m_kind ? SW_HIDE : SW_SHOW);
}

BEGIN_MESSAGE_MAP(COnLineDlg, CMyDialog)
	//{{AFX_MSG_MAP(COnLineDlg)
	ON_COMMAND(IDC_OUR_ORDER, OnOurs)
	ON_COMMAND(IDC_OTHER_ORDER,OnOther)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


UINT idt(char c)
{
	char ids[10] = {'u','m','s','v','n','o','p'};
	UINT i;
	for (i = 0; (i < 7) && (ids[i] != c); i++);
	return i;
}

BOOL CSketchApp::GetId(CString & str, UINT & idz, UINT &idc)
{
	char hub[200];
	UINT port = 0;
	UINT code = FindUSB(idz,hub,port);
	DWORD cc = 0;
	int bver = 0;
	idc = 0;
#ifdef FLIPBOOK_MAC
	char macc = 'A';
	UINT mac = 1;
#else
	char macc = 'W';
	UINT mac = 0;
#endif
	if (code != 2)
		idz = GetSysId(idc,0); // zero since looking
DPF("cc:%d,z:%8lx",idc,idz);
	if (idc > 6)
		idc = 3;
	if (idc == 0)
		cc = 'U';
	else if (idc == 1)
		cc = 'M';
	else if (idc == 2)
		cc = 'S';
	else if (idc == 3)
		cc = 'V';
	else
		cc = 'J' + idc;
	UINT ver = MYVER; // e.g.467
	char buf[55];
	#ifndef ASKOLDKEY
	sprintf(buf,"%d%d%03d%05d%05d",mac,idc,ver,
					idz / 100000, idz %100000);
	int v = checkdigit(buf,1);
	sprintf(buf,"%c%c%03d-%05d-%05d-%d",macc,cc,ver,
				idz / 100000, idz % 100000,v);
	#else
	sprintf(buf,"%c%05d-%05d",cc,idz / 100000, idz % 100000);
	#endif
	str = buf;
	return 0;
}

// App command to run the dialog
void CSketchApp::OnAppRegister()
{
	UINT ttyps[7] = {0,1,2,3,1,1,1};
	UINT idq = 0;
	DWORD cc = 0;
	int bver = 0;
	UINT ver = MYVER; // e.g.467
	UINT mac, idc, idz;
	CRegisterDlg registerDlg;
	GetId(registerDlg.m_Id, idz, idc);	
	UINT code = 0;
	char buf[60];
	for (;;)
		{
		int res = registerDlg.DoModal();
		if (res != IDOK)
			{
			if (res == IDCANCEL2)
				code = 9;
			break;
			}
		if (!_stricmp("educational",(LPCSTR)registerDlg.m_Code))
			{
			code = 1;
			break;
			}
		if (!_stricmp("demo",(LPCSTR)registerDlg.m_Code))
			{
			code = 2;
			break;
			}
		LPCSTR p = (LPCSTR)registerDlg.m_Code;
		if (((p[0] | 32) == 'e') && ((p[1] | 32) == 'd'))
			{
			if (AfxMessageBox(IDS_EDUCATIONAL,MB_YESNO) == IDYES)
				{
				code = 1;
				break;
				}
			}
		DWORD t1 = GetTickCount(); // delay for one second to deter
		for (;;)
			{
			DWORD t2 = GetTickCount();
			if ((t2 < t1) || (t2 > (t1 + 1000)))
				break;
			}
		UINT a,b,c,d;
		int k = 0;
		cc |= 32; // force lower case
#ifdef ACCEPTOLDKEY
		if ((char)cc == (*p++ | 32))
			k = sscanf(p,"%u-%u-%u-%u",&a,&b,&c,&d);
#endif
		if (k != 4)
			{
			k = 0;
			int v, q;	// ignore W/A and version
			bver = (MYVER/100)-4;	// slightly differnet encryption
			if ((*p | 32) == 'w')
				mac = 0;
			else if ((*p | 32) == 'a')
				mac = 1;
			else
				{
				MyError(IDS_REGISTER3);
				continue;
				}
			p++;
			idq = idt(*p++ | 32);
			if ((idq > 6) || (ttyps[idq] != ttyps[idc]))
//			if ((char)cc != (*p++ | 32))
				{
				MyError(IDS_REGISTER4);
				continue;
				}
			k = sscanf(p,"%u-%u-%u-%u-%u-%u",&v,&a,&b,&c,&d,&q);
			if (k == 6)
				{
				sprintf(buf,"%d%d%03d%05d%05d%05d%05d%d",mac,idq,v,a,b,c,d,q);
				if (!checkdigit(buf))
					k = 4;
				else
					{
					MyError(IDS_REGISTER5);
					continue;
					}
				if (zz(b) && zz(c)) q = 10;
				if ((WORD)q > 9)
					{
					MyError(IDS_REGISTER2);
					continue;
					}
				if ((v / 100) != (MYVER / 100))
					{
					MyError(IDS_REGISTER6);
					continue;
					}
				}
			else
				k = 7;
			}
DPF("k:%d,%d-%d-%d-%d,%s|",k,a,b,c,d,(LPCSTR)registerDlg.m_Code);
		if (k != 4)
			{
			MyError(IDS_REGISTER1);
			continue;
			}
		CBiggy result;
		CBiggy out;
		result.Assign(a,b,c,d);
		CMyCrypto crypt(bver+1);
		crypt.Decrypt(out.Addr(),result.Addr());
		out.SetVer(bver);
		out.Prepare();
		UINT idk, days,zver;
		out.Extract(zver,idk,days);
		if (days)
			{
			CTimeSpan dt;
			int y,m,d;
			if (!bver)
				{
				y = 2003;
				m = 12;
				d = 11;
				}
			else if (bver == 1)
				{
				y = 2006;
				m = 8;
				d = 31;
				}
			else //if (bver == 2)
				{
				y = 2012;
				m = 1;
				d = 1;
				}
			CTime base(y,m,d,0,0,0);
			if (m_now < base)
				{
				zver = 9;
				}
			else
				{
				dt = m_now - base;
				if ( dt.GetDays() > (int)days)
					zver = 9;
				}
			}
		BOOL bBad = TRUE;
		if (zver)
			bBad = TRUE;
		else if ((idc > 3) || (idq > 3))
			bBad = out.Eval2(idq, idc, idz);
		else
			bBad = out.Eval(idc,idk,idz);
		if (bBad)
			{
			MyError(IDS_REGISTER2);
			continue;
			}
		KillLicense();  // no longer doing online
		m_dwLicense = 0;
		PutKeyInt(10,0);
		SaveKeyMessages(idc,registerDlg.m_Id,days,registerDlg.m_Code);
//		WriteProfileString(Section, "User Name",registerDlg.m_Name);
		MyDialog(IDD_REGFEEDBACK);
		ProcessMsg(idz, idc, out.l[1]);
		break;
		}
	if (code == 2)
		{
//		if (m_dwLicense)
			RevertToDemo();
		}
	else if (code == 1)
		{
			m_dwLicense = 2;
#ifndef FLIPBOOK_MAC
			ChooseLicense();
#else
			@try {
				ChooseLicense();
			}
			@catch (NSException *e) {
#if 0
				NSString* path = [@"~/Desktop/FlipBook.debug" stringByExpandingTildeInPath];
				FILE* fp = fopen ([path cStringUsingEncoding:NSUTF8StringEncoding], "a");
				if (fp) {
					fprintf (fp, "ChooseLicense exception: %s\n", [[e reason] cStringUsingEncoding:NSUTF8StringEncoding]);
					fclose (fp);
				}
#endif
			}
#endif
			
		}
	else if (code == 9)
		{
#ifndef FLIPBOOK_MAC
		SendEmail(registerDlg.m_Id);
#endif
		}
}

/*
	returns id
	which is set to indicate id kind
	upon entry which is zero for looking
	else specfied
	
*/
DWORD CSketchApp::GetSysId(UINT &which, DWORD mac_id)
{
	DWORD idd = 0;
	DWORD t;
	CMyId * pId = new CMyId;
	for (;;)
		{
		if (!which || (which == 1) || (which > 3))
			{
			if (!which)
				t = pId->GetNewMac(idd);
			else
				{
				if (which > 3)
					which -= 2; // nbr of mac addrs
				if (pId->ChkNewMac(mac_id, which))
					{
					t = 1;
					idd = mac_id;
					}
				}
//			idd = SWAPV(idd);
			if ((which == 1) || idd)
				break;
			}
		if (!which || (which == 2))
			{
			CString txt;
			pId->GetDiskNbr(txt,idd);
			t = 2;
			if ((which == 2) || idd)
				break;
			}
		if (!which || (which == 3))
			{
			t = 3;
			idd = pId->GetVolume();
			if ((which == 3) || idd)
				break;
			}
		t = 4;
		break;
		}
	delete pId;
	which = t;
	return idd;
}

/*
	checks msg against id
	returns 2 if msg is empty
	returns 1 if okay
*/
UINT CSketchApp::CheckMsg(UINT usbid, CString msg, BOOL bUSB)
{
#ifdef TESTCODE
	ProcessMsg(TEST_ID,1,TESTCODE | TESTOPTIONS);
	return 1;
#endif
	m_dwFlags = (FLAG_COLOR | FLAG_CAMERA | FLAG_LAYERS);	// demo does both
	m_dwFeatures = 0;
#ifdef RIXBITS
	m_dwFeatures |= RIXBITS;
#endif
	m_dwId = 0;
	m_dwIdk = 9;
	int bver,k;
	LPCSTR pMsg = (LPCSTR)msg;
	if (!pMsg[0])
		return 2;
	UINT id2;
	UINT idc,idq;
	UINT a,b,c,d,v,q;
	char ids[10] = {'u','m','s','v','n','o','p'};
	char cc = *pMsg++ | 32;
#ifdef ACCEPTOLDKEY
	idc = 0;
	for (;(idc < 4) && (cc != ids[idc]); idc++);
	if (idc >= 4)
		return 0;
	bver = 0;
	k = sscanf(pMsg,"%u-%u-%u-%u",&a,&b,&c,&d);
#else
	int mac;
	if (cc == 'a')
		mac = 1;
	else if (cc == 'w')
		mac = 0;
	else
		return 0;
	bver = (MYVER / 100) - 4;
	cc = *pMsg++ | 32;
	for (idc = 0;(idc < 7) && (cc != ids[idc]); idc++);
	if (idc > 6)
		return 0;
	k = sscanf(pMsg,"%u-%u-%u-%u-%u-%u",&v,&a,&b,&c,&d,&q);
	if (k == 6)
		{
		char buf[40];
		sprintf(buf,"%d%d%03d%05d%05d%05d%05d%d",mac,idc,v,a,b,c,d,q);
		if (!checkdigit(buf))
			{
			k = 4;
			if (zz(b) && zz(c)) q = 10;
			if ((WORD)q > 9)
				k = 8;
			if ((v / 100) != (MYVER / 100))
				k = 9;
			}
		}
	else
		k = 7;
#endif
	if (bUSB)
		{
		if (!idc)
			id2 = usbid;
		else
			return 0;
		}
	else if (!idc)
		return 0;
//DPF("k:%d,%d-%d-%d-%d,%s|",k,a,b,c,d,(LPCSTR)registerDlg.m_Code);
DPF("k:%d,%d-%d-%d-%d",k,a,b,c,d);
	if (k != 4)
		return 0;
	CBiggy result;
	CBiggy out;
	result.Assign(a,b,c,d);
	CMyCrypto crypt(bver+1);
	crypt.Decrypt(out.Addr(),result.Addr());
	out.SetVer(bver);
	out.Prepare();
	UINT ver, idk, days;
	out.Extract(ver,idk,days);
	if (idq = idc)
		{
		id2 = GetSysId(idc,out.l[0]);
		if (idc > 3)
			idc = idk;
		}
	if (out.Eval(idc,idk,id2))
		return 0;
	ProcessMsg(id2, idq, out.l[1]);
	return 1;
}

UINT CSketchApp::ProcessMsg(UINT idd, UINT idk, UINT code)
{
	m_dwId = idd;
	m_dwIdk = idk;
	m_dwFeatures = code & 0xffff;
#ifdef RIXBITS
	m_dwFeatures |= RIXBITS;
#endif
	m_dwFlags = 0;
	if ((code & 15) == FEAT_PRO)
		m_dwFlags |= FLAG_CAMERA | FLAG_LAYERS;	// camera;
	if (code & BIT_CAMERA)
		m_dwFlags |= FLAG_CAMERA;	// camera for lite;
	if (code & BIT_LAYERS)
		m_dwFlags |= FLAG_LAYERS;	// layers for standard;
	if ((code & 15) != FEAT_PT)
		m_dwFlags |= FLAG_COLOR;	// color allowed;
	return 0;
}

BOOL CSketchApp::SaveFileName(CString& fileName, UINT nIDSTitle, 
				DWORD lFlags, DWORD kind)
{
	BOOL bType = FALSE;
	if (kind == 4 || kind == 6 || kind == 8 || kind == 15 || kind == 30)
		bType = TRUE;
	CFileDialog dlgFile(bType);
	CString title;
	VERIFY(title.LoadString(nIDSTitle));
	dlgFile.m_ofn.Flags |= lFlags;
	CString strFilter;
	CString strDefault;
	if (kind == 1)
		{
		strFilter = "Bmp";
		strFilter += (char)'\0';        // next string please
		strFilter += "*.bmp";
		strFilter += (char)'\0';        // last string
		dlgFile.m_ofn.nMaxCustFilter++;
		dlgFile.m_ofn.lpstrDefExt = "BMP";
		}
	else if (kind == 5)
		{
		strFilter = "Tga";
		strFilter += (char)'\0';        // next string please
		strFilter += "*.tga";
		strFilter += (char)'\0';        // last string
		dlgFile.m_ofn.nMaxCustFilter++;
		dlgFile.m_ofn.lpstrDefExt = "TGA";
		}
	else if (kind == 6 || kind == 7)
		{
		strFilter = "Pal";
		strFilter += (char)'\0';        // next string please
		strFilter += "*.pal";
		strFilter += (char)'\0';        // last string
		dlgFile.m_ofn.nMaxCustFilter++;
		dlgFile.m_ofn.lpstrDefExt = "PAL";
		}
	else if (kind == 8 || kind == 9)
		{
		strFilter = "Model";
		strFilter += (char)'\0';        // next string please
		strFilter += "*.mdl";
		strFilter += (char)'\0';        // last string
		dlgFile.m_ofn.nMaxCustFilter++;
		dlgFile.m_ofn.lpstrDefExt = "MDL";
		}
	else if (kind == 4)
		{
		strFilter = "Exe";
		strFilter += (char)'\0';        // next string please
		strFilter += "*.exe";
		strFilter += (char)'\0';        // last string
		dlgFile.m_ofn.nMaxCustFilter++;
		dlgFile.m_ofn.lpstrDefExt = "EXE";
		}
	else if (kind == 10)
		{
		strFilter = "Gif";
		strFilter += (char)'\0';        // next string please
		strFilter += "*.gif";
		strFilter += (char)'\0';        // last string
		dlgFile.m_ofn.nMaxCustFilter++;
		dlgFile.m_ofn.lpstrDefExt = "GIF";
		}
	else if (kind == 20)
		{
		strFilter = "Flv";
		strFilter += (char)'\0';        // next string please
		strFilter += "*.flv";
		strFilter += (char)'\0';        // last string
		dlgFile.m_ofn.nMaxCustFilter++;
		dlgFile.m_ofn.lpstrDefExt = "FLV";
		}
	else if (kind == 11)
		{
		strFilter = "Mov";
		strFilter += (char)'\0';        // next string please
		strFilter += "*.mov";
		strFilter += (char)'\0';        // last string
		dlgFile.m_ofn.nMaxCustFilter++;
		dlgFile.m_ofn.lpstrDefExt = "MOV";
		}
	else if (kind == 12 || kind == 15)
		{
		strFilter = "Fbl";
		strFilter += (char)'\0';        // next string please
		strFilter += "*.fbl";
		strFilter += (char)'\0';        // last string
		dlgFile.m_ofn.nMaxCustFilter++;
		dlgFile.m_ofn.lpstrDefExt = "FBL";
		}
	else if (kind == 30 || kind == 31)
		{
		strFilter = "Txt";
		strFilter += (char)'\0';        // next string please
		strFilter += "*.txt";
		strFilter += (char)'\0';        // last string
		dlgFile.m_ofn.nMaxCustFilter++;
		dlgFile.m_ofn.lpstrDefExt = "TXT";
		}
	else if (kind == 14)
		{
		strFilter = "Wav";
		strFilter += (char)'\0';        // next string please
		strFilter += "*.wav";
		strFilter += (char)'\0';        // last string
		dlgFile.m_ofn.nMaxCustFilter++;
		dlgFile.m_ofn.lpstrDefExt = "WAV";
		}
	else if (kind >= 100)
		{
		char ext[50];
		FBQTGetExt(ext,(FBFileFormat)(kind - 100));
		strFilter = ext;
		strFilter += (char)'\0';        // next string please
		strFilter += "*.";
		strFilter += ext;
		strFilter += (char)'\0';        // last string
		dlgFile.m_ofn.nMaxCustFilter++;
		dlgFile.m_ofn.lpstrDefExt = ext;
		}
	else
		{
		strFilter = "Avi";
		strFilter += (char)'\0';        // next string please
		strFilter += "*.avi";
		strFilter += (char)'\0';        // last string
		dlgFile.m_ofn.nMaxCustFilter++;
		dlgFile.m_ofn.lpstrDefExt = "AVI";
		}
	dlgFile.m_ofn.lpstrFilter = strFilter;
	dlgFile.m_ofn.hwndOwner = AfxGetMainWnd()->GetSafeHwnd();
	dlgFile.m_ofn.lpstrTitle = title;
	dlgFile.m_ofn.lpstrFile = fileName.GetBuffer(_MAX_PATH);
	BOOL bRet = dlgFile.DoModal() == IDOK ? TRUE : FALSE;
	fileName.ReleaseBuffer();
	return bRet;
}


BOOL CSketchApp::PromptFileName(CString& fileName, UINT Kind)
{
	UINT flags = 0;
	BOOL bOpen = 0;
	UINT xKind = Kind;
	if (xKind >= 100)
		Kind = 5;
	UINT IdString = 0;
	switch (Kind) {
	case 2:
		IdString = IDS_EXPORTAVI;
		break;
	case 10:
		IdString = IDS_EXPORTGIF;
		break;
	case 14:
		IdString = IDS_EXTRACTWAV;
		break;
	case 3:
	case 13:
		IdString = FLIP_FEW;
		bOpen = 1;
		break;
	case 4:
		IdString = IDS_SELECTPLAY;
		bOpen = 1;
		break;
	case 5:
	case 1:
		IdString = IDS_EXPORTBMP;
		bOpen = 0;
		break;
	case 6:
		IdString = IDS_PAL_IMPORT;
		bOpen = 1;
		break;
	case 7:
		IdString = IDS_PAL_EXPORT;
		bOpen = 0;
		break;
	case 8:
		IdString = IDS_LOAD_MODEL;
		bOpen = 1;
		break;
	case 9:
		IdString = IDS_SAVE_MODEL;
		bOpen = 0;
		break;
	case 11:
		IdString = IDS_EXPORT_MOVIE;
		bOpen = 0;
		break;
	case 12:
		IdString = IDS_EXPORT_LIBRARY;
		bOpen = 0;
		break;
	case 15:
		IdString = IDS_IMPORT_LIBRARY;
		bOpen = 1;
		break;
	case 20:
		IdString = IDS_EXPORT_FLV;
		bOpen = 0;
		break;
	case 31:
		IdString = IDS_EXPORT_CAMERA;
		bOpen = 0;
		break;
	case 30:
		IdString = IDS_IMPORT_CAMERA;
		bOpen = 1;
		break;
	default:
		bOpen = 1;	// for dgc
		break;
	}
	return DoPromptFileName(fileName, IdString,flags,bOpen,xKind);
}

BOOL CSketchApp::DoPromptFileName(CString& fileName, UINT nIDSTitle,
			DWORD lFlags, BOOL bOpenFileDialog, UINT kind)
{
	char sfx[10];
	char names[250];
	char * pPath;
	char path[256];
	if (!bOpenFileDialog)
		return SaveFileName(fileName, nIDSTitle, lFlags, kind);
	if (kind == 4 || kind == 6 || kind == 8 || kind == 15 || kind == 30)
		return SaveFileName(fileName, nIDSTitle, lFlags, kind);
	strcpy(path,fileName);
	pPath = path;
//	bOpenFileDialog++;
//	lFlags++;
//	nIDSTitle++;
	if (kind == 2)
		{
		strcpy(sfx,"swf");
		strcpy(names,"SWF Files (*.swf)|*.swf||");
		}
	else if (kind == 1)
		{
		strcpy(sfx,"avi");
		strcpy(names,"AVI Files (*.avi)|*.avi||");
		}
	else if ((kind == 3) || (kind == 13))
		{
		strcpy(sfx,"tga");
		strcpy(names,"Image Files |*.bmp;*.tga;*.jpg;*.png;*.tif");
		strcat(names,"|BMP Files (*.bmp)|*.bmp|TGA Files(*.tga)|*.tga");
		strcat(names,"|JPG Files (*.jpg)|*.jpg|PNG Files(*.png)|*.png");
		strcat(names,"|TIFF Files (*.tif)|*.tif||");
		}
	else
		{
		if (m_pRecentFileList->GetSize())
			{
			strcpy(path,(*m_pRecentFileList)[0]);
			pPath = path;
			}
#ifdef _DISNEY
		strcpy(sfx,"cbk");
		strcpy(names,"Scene Files (*.cbk)|*.cbk||");
#else
		strcpy(sfx,"dgc");
		strcpy(names,"Scene Files (*.dgc)|*.dgc||");
#endif
		}

	CPreviewFileDlg dlgFile (TRUE,kind,
		sfx, pPath,
		OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,names);
//	if (kind == 3)
		dlgFile.m_pFrame = (CMainFrame *)m_pMainWnd;
	int v = GetProfileInt("Options","Preview Dlg", 256+240);
	dlgFile.m_bImportColor = (v & 512) ? 1 : 0;
	dlgFile.m_bKeying = (v & 256) ? 1 : 0;
	dlgFile.m_nAlpha = v & 255;
	BOOL bResult = dlgFile.DoModal() == IDOK ? TRUE : FALSE;
//	if (bResult)
		{
		v = dlgFile.m_nAlpha + 256 * (dlgFile.m_bKeying & 1) +
					512 * (dlgFile.m_bImportColor & 1);
		WriteProfileInt("Options","Preview Dlg", v);
		}
		fileName = dlgFile.GetPathName();
//	fileName.ReleaseBuffer();
	return bResult;
}

BOOL CSketchApp::OnIdle(LONG lCount)
{
	if (CWinApp::OnIdle(lCount))
		return TRUE;
	if (m_pMainWnd == NULL)
		return FALSE;
	if (lCount > 0)
		return ((CMainFrame *)m_pMainWnd)->IdleProcess();
	return FALSE;
}

/*
	Expire msg handler
	returns if 0 not expire
*/
//UINT CSketchApp::CheckTimer(BOOL bInitial)
//{
//	return 0;
//}
/*
	USB handler
	returns 0 if not usb
	1 if usb alright
	2 if usb bad, need abort
*/
UINT CSketchApp::CheckUSBKey(BOOL bInitial)
{
	UINT nResult = 0;
	CString msg;
	CString file;
	char hub[200];
	UINT port;
	BOOL bNewMsg = 0;
	if (bInitial)
		{
		GetKeyString(0,hub, sizeof(hub));
		msg = hub;
		if (!msg.GetLength())
			{
			if (!FindUSBMsg(file, msg)  || !msg.GetLength())
				return nResult;
			bNewMsg = TRUE;
			}
		}
	else
		{
		if (m_dwIdk)
			return 0;		// incase temp msg uses mac addr
		if (!GetKeyString(0, hub, sizeof(hub)))
			return nResult;
		msg = hub;
		}
	UINT iid;
	GetKeyString(3,hub,sizeof(hub));
	port = GetKeyInt(4);
	UINT code = FindUSB(iid,hub,port);
	if (code == 2)
		{
		PutKeyString(3,hub); // save new port info
		PutKeyInt(4, port);
		}
	UINT err = 0;
	if (code)
		{
		if (CheckMsg(iid, msg,1) == 1)
			{
			nResult = 1;
			if (bNewMsg)
				PutKeyString(0,msg);
			}
		else
			{
			err = IDS_DIFF_USB;
			if (bInitial && FindUSBMsg(file,msg) && msg.GetLength())
				{
				if (CheckMsg(iid,msg,1))
					{
					nResult = 1;
					if (bInitial)
						PutKeyString(0,msg);
					AfxMessageBox(err);
					err = 0;
					}
				}
			}
		}
	else
		{
		err = IDS_NO_USB;
		nResult = 2;
		}
	if (err)
		{
		AfxMessageBox(err);
		PutKeyString(0,"");
		if (nResult)
			RevertToDemo(1);
		}
	return nResult;
}

/*
	used at startup, like searching for usb
	returns true if there is a problem
*/
BOOL CSketchApp::IdSetup()
{
	BOOL bFail = TimeCheck();
#ifdef EXPIRES
	if (ExpireCheck(bFail))
		return TRUE;
#endif
#ifdef TESTCODE
	ProcessMsg(TEST_ID,1,TESTCODE | TESTOPTIONS);
	return 0;
#endif
	UINT code;
	m_days = 0;
//	m_days = 1500;

//	if (!(code = TempCheck()))
//		return 0;
//	if (code == 1) // erase if expired
//		PutKeyString(2,"");
		
	if (code = CheckUSBKey(1))
		{
		m_dwLicense = 0;
//		WriteProfileInt("Settings","Licensing",0); // leave for student
		return 0;//code - 1;
		}
	if (m_dwLicense)
		{
		if (ChooseLicense())
			return TRUE;
		if (m_dwLicense)
			return FALSE;
		}
	char temp[200];
	GetKeyString(1,temp,sizeof(temp));
	CString msg = temp;
	code = CheckMsg(0,msg,0);
	if (code == 0)
		{
		if (!(code = TempCheck()))
			return 0;
		if (code == 1) // erase if expired
			PutKeyString(2,"");
		int res = MyWarning(IDD_IDWARNING);
		if (res == IDOK)
			OnAppRegister();
		else
			return FALSE;
		}
	BOOL bResult = FALSE;
	for (;;)
		{
		if (m_dwFeatures & 15)
			break;// enabled
		CDemoDlg dlg;
		UINT idz, idc;
	 	GetId(dlg.m_id, idz, idc);	
 		int res = dlg.DoModal();
		if (res == IDOK)
			OnAppRegister();
		else if (res == IDC_BUY_IT)
			BuyIt();
		else if (res == IDC_TRY_IT)
			break;
		else
			{
			bResult = TRUE;
			break;
			}
		}
	return bResult;
}
/*
	used mostly when opening a scene
	major setup at ap init, like searching for usb
	returns true if there is a problem
*/
BOOL CSketchApp::IdCheck(UINT & Id, UINT & Features)
{
//	return TRUE;
#ifndef TESTCODE
	if (!m_dwIdk && CheckUSBKey(0) > 1)
		return TRUE;
	if (m_pLicense)
		{
		RenewLicense();
		if (!m_dwLicense)
			{
			RevertToDemo();
			return TRUE;
			}
		}
#endif
	Id = m_dwId;
	Features = m_dwFeatures;
	m_bCanCancel = 1;	// turns on timer renw if licensing
	return FALSE;
}
/*
	used to ensure usb key is still there and has not moved
*/
BOOL CSketchApp::USBCheck()
{
#ifdef TESTCODE
	return TRUE;
#endif
	if (m_dwIdk)
		return TRUE;
	char hub[200];
	UINT port;
	UINT iid;
	GetKeyString(3, hub, sizeof(hub));
	port = GetKeyInt(4);
	UINT code = FindUSB(iid,hub,port);
	if (code == 2)
		{
		PutKeyString(3,hub);
		PutKeyInt(4, port);
		}
	UINT err = 0;
	if (code)
		{
		if (iid != m_dwId)
			err = IDS_DIFF2_USB;
		}
	else
		{
		err = IDS_NO_USB;
		}
	if (err)
		{
		m_dwIdk = 9;
		AfxMessageBox(err);
		AfxMessageBox(IDS_CLOSING);
		PutKeyString(0,"");
		CMainFrame * pFrame =(CMainFrame *)m_pMainWnd;
		if (pFrame)
			{
			m_bCanCancel = 0;
			pFrame->PostMessage(WM_COMMAND,ID_FILE_CLOSE);
			}
		}
	return err ? 0 : 1;
	
}

BOOL CSketchApp::BuyIt()
{
	char szProgPath[MAX_PATH];
	HWND hwnd = 0;//AfxGetMainWnd()->m_hWnd;
	strcpy(szProgPath,"http://www.digicelinc.com/store.htm");
	int z = (int)ShellExecute(hwnd,0, szProgPath,0,0,SW_SHOW);
	if (z <= 32)
		MyError(IDS_NO_HELP);
	return 0;
}

void CSketchApp::OnHelp()
{
	HWND hwnd = AfxGetMainWnd()->m_hWnd;
	char szProgPath[MAX_PATH];
	::GetModuleFileName(NULL, szProgPath, sizeof(szProgPath)-1);
	int i,j;
	for (i = 0, j = 0; szProgPath[i];i++)
		if (szProgPath[i] == '\\')
			j = i;
	szProgPath[j+1] = 0;
	strcat(szProgPath,"index.htm");
	int z = (int)ShellExecute(hwnd,0, szProgPath,0,0,SW_SHOW);
	if (z <= 32)
		MyError(IDS_NO_HELP);
}

void PASCAL DDV_MyMinMax(CDataExchange * pDX, int iid,
					double & value, double min, double max, int places /* = 3 */)
{
	if (pDX->m_bSaveAndValidate)
		{
		DDX_Text(pDX, iid, value);
		DDV_MinMaxDouble(pDX, value, min, max);
		}
	else
		{
		CString str1;
		CString str2;
		str1.Format("%%.%df",places);
		str2.Format(str1,value);
		DDX_Text(pDX, iid, str2);
		}
}

/*
	checks for a temp[orary msg and handles it
	returns 0 if temp msg is handled
	returns 1 if expired
	else more processing is required
*/
BOOL CSketchApp::TempCheck()
{
	char temp[200];
	if (!GetKeyString(2, temp, sizeof(temp)))
		return 2;
	UINT usbcode = CheckUSBKey(1);
	int bver;
	UINT idc;
	char ids[10] = {'u','m','s','v','n','o','p'};
	LPCSTR pMsg = (LPCSTR)&temp;
	char cc = *pMsg++ | 32;
	int k;
	UINT a,b,c,d,v,q;
#ifdef ACCEPTOLDKEY
	idc = 0;
	for (;(idc < 4) && (cc != ids[idc]); idc++);
	if (idc >= 4)
		return 1;
	bver = 0;
	k = sscanf(pMsg,"%u-%u-%u-%u",&a,&b,&c,&d);
#else
	int mac;
	if (cc == 'a')
		mac = 1;
	else if (cc == 'w')
		mac = 0;
	else
		return 1;
	bver = (MYVER / 100) - 4;
	cc = *pMsg++ | 32;
	for (idc = 0;(idc < 7) && (cc != ids[idc]); idc++);
	if (idc > 6)
		return 1;
	k = sscanf(pMsg,"%u-%u-%u-%u-%u-%u",&v,&a,&b,&c,&d,&q);
	if (k == 6)
		{
		char buf[80];
		sprintf(buf,"%d%d%03d%05d%05d%05d%05d%d",mac,idc,v,a,b,c,d,q);
		if (!checkdigit(buf))
			{
			k = 4;
			if ((v / 100) != (MYVER / 100))
				k = 9;
			if (zz(b) && zz(c)) q = 10;
			if ((WORD)q > 9)
				k = 8;
			}
		}
	else
		k = 7;
#endif
DPF("k:%d,%d-%d-%d-%d",k,a,b,c,d);
	if (k != 4)
		return 1;
	CBiggy result;
	CBiggy out;
	result.Assign(a,b,c,d);
	CMyCrypto crypt(bver+1);
	crypt.Decrypt(out.Addr(),result.Addr());
	out.SetVer(bver);
	out.Prepare();
	UINT ver, idk, days;
	
	out.Extract(ver,idk,days);
	m_days = days;
//	m_dwMacKludge = out.l[0];
	int y,m,dd;
	if (!bver)
		{
		y = 2003;
		m = 12;
		dd = 11;
		}
	else if (bver == 1)
		{
		y = 2006;
		m = 8;
		dd = 31;
		}
	else // if (bver == 2)
		{
		y = 2012;
		m = 1;
		dd = 1;
		}
	CTime base(y,m,dd,0,0,0);
	CTimeSpan dt;
	if (m_now < base)
		ver = 9;
	else
		{
		dt = m_now - base;
		if ( dt.GetDays() > (int)days)
			ver = 9;
		}
	if (ver || out.Eval(idc,idk,out.l[0]))
		{
		PutKeyString(2,"");
		MyWarning(IDD_TEMPNOW);
		return 1;
		}

	UINT id2;
	if (!idc)
		{
		char hub[200];
		UINT port;
		GetKeyString(3,hub, sizeof(hub));
		port = GetKeyInt(4);
		if (!FindUSB(id2,hub,port))
			id2 = 0;
		}
	else 
		id2 = GetSysId(idc,out.l[0]);
	if (out.l[0] != id2)
		{
		return 1;	// id has changed
		}
	days -= (DWORD)dt.GetDays();
	if (days < 5)
		{
		MyWarning(IDD_TEMPSOON);
		}
	ProcessMsg(id2, idk, out.l[1]);
	return 0;
}


BOOL CSketchApp::TimeCheck()
{
	m_now = CTime::GetCurrentTime();
	int year2 = m_now.GetYear();
	int month2 = m_now.GetMonth();
	int day2 = m_now.GetDay();
	char buf[10];
	int c = GetKeyString(8,buf,10);
	if (c)
		{
		BOOL bBad = 0;
		if (c != 4)
			bBad = 1; // bad animal
		else
			{
			CTime then(2003 + buf[0] - 'A',
					buf[1] - 'A', 10 * (buf[2] - 'A') + buf[3] - 'A',0,0,0);	
//DPF("d1:%d,d2:%d,d3:%d,cnt1:%d",daze1,daze2,daze3,count1);
			if (m_now < then)
				bBad = 1;	// date has been set back
			}
		if (bBad)
			{
			return 1;
			}
		}
	buf[0] = 'A' + year2 - 2003;
	buf[1] = 'A' + month2;
	buf[2] = 'A' + day2 / 10;
	buf[3] = 'A' + day2 % 10;
	buf[4] = 0;
	PutKeyString(8,buf);
	return 0;
}


CSketchApp::CSketchApp()
{
	m_dwFlags = 0;
	m_dwId = 0;
	m_dwIdk = 9;
	m_dwFeatures = 0;
	m_bCanCancel = 0;
	m_pLicense = 0;
	m_pDeadLicense = 0;
	m_nLicenseBusy = 0;
	m_nTimer = 0;
	m_pMaya = 0;
	m_pTablet = 0;
//	m_pMyProfileName = 0;
//	m_pMyGlobalName = 0;
}

CSketchApp::~CSketchApp()
{
	WriteProfileInt("Settings","Still Running",0);
#ifndef FLIPBOOK_MAC
	if (m_nTimer)
		KillTimer(0,m_nTimer);
#endif
	delete m_pMaya;
	delete m_pLicense;
	delete m_pTablet;
	//delete [] m_pMyProfileName;
}

//	v 0 is clear, 1 is set, 2 is flip
// which 32 is getb all, 33 is clear all
//	else 
//
BOOL CSketchApp::RemindFlags(int which, int v /* = -1 */)
{
	DWORD old = m_dwRemindFlags;
	if (which == 32)
		return m_dwRemindFlags ? TRUE : FALSE;
	else if (which == 33)
		{
		m_dwRemindFlags = 0;
		which = 0;
		}
	UINT mask = 1 << which;
	if (v == 1)
		m_dwRemindFlags |= mask;
	else if (v == 2)
		m_dwRemindFlags ^= mask;
	else if (v == 0)
		m_dwRemindFlags &= (mask ^ 0xffffffff);
	if (old != m_dwRemindFlags)
		WriteProfileInt("Options", "Remind Flags", m_dwRemindFlags);
	return m_dwRemindFlags & mask ? TRUE : FALSE;
}

BOOL CSketchApp::CanDoFeature(CanDoKinds which)
{
	UINT mask;
	if (which == CD_TELECINE)
		mask = BIT_TELECINE;
	else if (which == CD_DESKEW)
		mask = BIT_DESKEW;
	else if (which == CD_HIRES)
		mask = BIT_HIRES;
	else if (which == CD_COLORKEY)
		mask = BIT_COLORKEY;
	else if (which == CD_RECORDER)
		mask = BIT_PLUS;
	else if (which == CD_EXP_FLV)
		mask = BIT_PLUS;
	else if (which == CD_LIBRARY)
		mask = BIT_LIBRARY;
	else
		mask = 0;
	UINT bits = m_dwFeatures;
#ifdef RIXBITS
	bits |= RIXBITS;
#endif
	if (!(bits & 15) &&		// demo does all but plus
			!(mask & BIT_PLUS) && !(mask & BIT_LIBRARY))
		return TRUE;
	if (bits & mask)
		return TRUE;
	else
		return FALSE;
}

BOOL CSketchApp::IsLite()
{
	if (!(m_dwFeatures & 15))
		return FALSE;
	if (!m_dwIdk && CheckUSBKey(0) > 1)
		return TRUE;
	if ((m_dwFeatures & 15) == FEAT_LITE)
		return TRUE;
	return FALSE;
}

BOOL CSketchApp::IsAnimate()
{
	if (m_dwFeatures & BIT_PLUS)
		return TRUE;
	return FALSE;
}

BOOL CSketchApp::IsPro()
{
	if (!(m_dwFeatures & 15))
		return TRUE;
	if (!m_dwIdk && CheckUSBKey(0) > 1)
		return FALSE;
	if ((m_dwFeatures & 15) == FEAT_PRO)
		return TRUE;
	return FALSE;
}

#ifdef EXPIRES
BOOL CSketchApp::ExpireCheck(BOOL bFail)
{
	BOOL bBad = 0;
	BOOL bSoon = 0;
	int count1 = 0;
	char bbuf[10];
#ifdef EXPIRE_DATE
	int daz3 = EXPIRE_DATE;
	int year3 = daz3 % 10000;
	daz3 /= 10000;
	int day3 = daz3 % 100;
	int month3 = daz3 / 100;
	CTime when(year3, month3, day3,0,0,0);
	if (bFail || (m_now >= when))
		bBad = TRUE;
	else
		{
		CTimeSpan dt = when - m_now;
		int hours = (int)dt.GetTotalHours();
DPF("hours:%d",hours);
		if (hours < 72)
			bSoon = TRUE;
		}
#endif
#ifdef EXPIRE_COUNT
	int count3 = EXPIRE_COUNT;
	int cc = GetKeyString(9,bbuf,10);
	if (!cc)
		count1 = 0;
	else if (cc != 4)
		count1 = 10000;
	else
		{
		count1 = (2 + bbuf[0]) % 10;
		count1 = 10 * count1 + (2+bbuf[1]) %10;	// 2 bumps x30 to zero
		count1 = 10 * count1 + (2+bbuf[2]) %10;
		count1 = 10 * count1 + (2+bbuf[3]) %10;
		count1++;
		}
	if ((count1 > 9999) || (count1 > count3))
		bBad = TRUE;
	else if ((count1 + 4) >= count3)
		bSoon = TRUE;
#endif
	if (bBad)
		count1 = 9999;
	if (count1)
		{
		sprintf(bbuf,"%04d",count1);
		PutKeyString(9,bbuf);
		}
	if (bBad)
		{
		MyDialog(IDD_EXPIRE);
		}
	else if (bSoon)
		{
		MyWarning(IDD_EXPIRESOON, RF_EXPIRE);
		}
	return bBad;
}
#endif

class CIdWarningDlg : public CMyDialog
{
// Construction
public:
	CIdWarningDlg(int iid, CWnd* pParent = NULL);
	BOOL m_bRemind;
	BOOL m_bHaveCheck;
protected:
	int m_id;
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
};

CIdWarningDlg::CIdWarningDlg(int iid, CWnd* pParent /*=NULL*/)
	: CMyDialog(iid, pParent)
{
	m_id = iid;
	m_bRemind = 0;
	m_bHaveCheck = 1;
}

BOOL CIdWarningDlg::OnInitDialog()
{
	CMyDialog::OnInitDialog();
	if (m_id == IDD_IDWARNING)
		{
		OEM_DLG(this, IDD_WARNING1);
		OEM_DLG(this, IDD_WARNING2);
		OEM_DLG(this, IDD_WARNING3);
		}
	else if (m_id == IDD_EXPIRESOON)
		{
		OEM_DLG(this, IDC_EXP_BAD);
		}
	else if (m_id == IDD_EXPIRESOON1)
		{
		OEM_DLG(this, IDC_EXP_BAD);
		}
	else if (m_id == IDD_ASSOC_EXE)
		{
		OEM_DLG(this, IDC_EXP_BAD);
		}
	return FALSE;
}

void CIdWarningDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	if (m_bHaveCheck)
		DDX_Check(pDX, IDC_REMIND, m_bRemind);
}

int CSketchApp::MyWarning(UINT iddialog, int flag)
{
	BOOL bCheck = 0;
	if (flag != -1)
		{
		if (RemindFlags(flag))
			return IDOK;
		bCheck = 1;
		}
	CIdWarningDlg dlg(iddialog);
	dlg.m_bHaveCheck = bCheck;
	int res = dlg.DoModal();
	if (dlg.m_bRemind)
		RemindFlags(flag,1);
	return res;
}


BOOL CSketchApp::CanCancel(int v)
{
	BOOL z = m_bCanCancel;
	if (v != -1)
		m_bCanCancel = v;
	return z;
}


void CSketchApp::RevertToDemo(BOOL bShowMsg)
{
	if (m_pLicense)
		{
		bShowMsg = 1;
		KillLicense();
		}
	m_dwLicense = 0;
	m_dwFlags = (FLAG_COLOR | FLAG_CAMERA | FLAG_LAYERS);	// demo does both
	m_dwFeatures = 0;
#ifdef RIXBITS
	m_dwFeatures |= RIXBITS;
#endif
	m_dwId = 0;
	m_dwIdk = 9;
//	WriteProfileInt("Settings","Licensing",0);
	if (bShowMsg)
		{
		AfxMessageBox(IDS_REVERT_DEMO);
		}
}

void CSketchApp::OnTimer(UINT arg)
{
	if (arg == m_nTimer)
		RenewLicense();
}

void CSketchApp::RenewLicense()
{
	if (!m_pLicense)
		return;
	if (!m_bCanCancel)
		return;
	if (m_pLicense->NoRenew())
		return;
	if (!m_nLicenseBusy++)			// already in kill logic
		{
DPF("renewing");
		if (m_pLicense->Renew())	// try to renew  it
			{
DPF("failed renew");
			m_dwLicense = 0;		// failure, disable it
			m_pLicense->Release();	// not really needed
			KillTimer(0,m_nTimer);
			m_nTimer = 0;
			AfxMessageBox(IDS_NO_RENEW);
			CMainFrame * pFrame =(CMainFrame *)m_pMainWnd;
			if (pFrame)
				{
				m_bCanCancel = 0;
				pFrame->PostMessage(WM_COMMAND,ID_FILE_CLOSE);
				}
			else
				{
				RevertToDemo();
				}
			}
		if (m_pDeadLicense)			// someone wanted it gone
			{						//
			delete m_pDeadLicense;	// delete it
			m_pDeadLicense = 0;
			KillTimer(0,m_nTimer);
			m_nTimer = 0;
			}
		}
	m_nLicenseBusy--;
}

void CSketchApp::KillLicense()
{
	if (!m_pLicense)			// nothing to do
		return;
	ASSERT(m_pDeadLicense == 0);
	m_pDeadLicense = m_pLicense;// stash it in case timer ahead of us
	m_pLicense = 0;
	if (!m_nLicenseBusy++)
		{
		delete m_pDeadLicense;
		m_pDeadLicense = 0;
		KillTimer(0,m_nTimer);
		m_nTimer = 0;
		}
	m_nLicenseBusy--;
}


class CProdDlg : public CMyDialog
{
// Construction
public:
	CProdDlg(CWnd* pParent = NULL);	// standard constructor

	enum { IDD = IDD_PRODUCTS };
	char m_default[80];
	char m_client[200];
	char m_arg[928];
	UINT m_product;
	UINT m_features;
	BOOL m_bFancy;
	BOOL m_bNoRenew;
	UINT m_error;
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL
// Implementation
protected:
	CListBox m_list;
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChange();
	afx_msg void OnNoLicense();
	DECLARE_MESSAGE_MAP()
};

CProdDlg::CProdDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CProdDlg::IDD, pParent)
{
	m_bNoRenew = 0;
}

void CProdDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PRODUCTS, m_list);
}

BEGIN_MESSAGE_MAP(CProdDlg, CMyDialog)
	ON_COMMAND(IDC_NO_LICENSE, OnNoLicense)
	ON_LBN_SELCHANGE(IDC_PRODUCTS, OnSelChange)
END_MESSAGE_MAP()

BOOL CProdDlg::OnInitDialog()
{
	CMyDialog::OnInitDialog();
	int i = 0;
	int j,k;
	int sel = 0;
	int prod_count = 0;
	m_error = 0;
	j = 0;
	m_arg[sizeof(m_arg)-1] = 0; // stopper
	for (j=0;m_arg[j] && (m_arg[j] <= ' ');j++);
	if (m_arg[j] == '*')
		{
		m_bNoRenew = TRUE;
		j++;
		}
	for (k=0; (k < sizeof(m_client)) &&
				m_arg[j] && (m_arg[j] != ';');m_client[k++] = m_arg[j++]);
	m_client[k] = 0;
	i = j+1;
	if (m_arg[j] != ';')
		{
		m_client[0] = 0;
		i = 0;
		m_arg[i] = 0;
		}
	else
		SetDlgItemText(IDC_LICENSEE,m_client);
//return 1;
//	m_list.ResetContent();
	for (;m_arg[i];i++)
		{
		int j;
		int prod,k,count;
		char buf[80];
		char name[50];
		char feat[30];
		char val[30];
		DPF("arg1:%s", m_arg + i);
		for (j = 0; m_arg[i+j] && (m_arg[i+j] != ',');val[j] = m_arg[i+j++]);
		if (m_arg[i+j] != ',')
			break;
		val[j++] = 0;
		i += j; 
		k = sscanf(val,"%d",&prod);
		if (k != 1)
			break;
		for (j = 0; m_arg[i+j] && (m_arg[i+j] != ',');name[j] = m_arg[i+j++]);
		if (m_arg[i+j] != ',')
			break;
		name[j++] = 0;
		i += j; 
		for (j = 0; m_arg[i+j] && (m_arg[i+j] != ',');val[j] = m_arg[i+j++]);
		if (m_arg[i+j] != ',')
			break;
		val[j++] = 0;
		i += j; 
		k = sscanf(val,"%d",&count);
		if (k != 1)
			break;
		feat[0] = 0;
		for (j = 0; m_arg[i+j];j++)
			{
DPF("arg2:%s",m_arg+i+j);
			if (m_arg[i+j] == ';')
				break;
			if (m_arg[i+j] == '=')
				break;
			feat[j] = m_arg[i+j];
			}
		feat[j++] = 0;
		i += j;
		char vv = '0' + (MYVER / 100);
		for (j = 0; m_arg[i+j];j++)
			{
			if (m_arg[i+j] == ';')
				break;
			if (m_arg[i+j] == vv)
				vv = 0;
			}
		if (m_arg[i+j] != ';')
			break;
		i += j+1;
		if (vv)
			continue;
		if (m_bFancy)
			sprintf(buf,"%s:%2d:%3d:%s",name,count,prod,feat);
		else
			strcpy(buf, name);
		UINT q;
		UINT f = 0;
		j = 0;
		if (feat[j] == ' ')
			j++;
		feat[j] |= 32;
		if (feat[j] == 's')
			f = 1;
		else if (feat[j] == 'l')
			f = 2;
		else if (feat[j] == 't')
			f = 3;
		else if (feat[j] == 'p')
			f = 4;
		else
			f = 0;
		for (j++; feat[j]; j++)
			{
			int z = feat[j];
			if (z > '+')
				z |= 32;
			switch(z) {
			case 't':
				f |= BIT_TELECINE;
				break;
			case 'd':
				f |= BIT_DESKEW;
				break;
			case 'h':
				f |= BIT_HIRES;
				break;
			case 'c':
				f |= BIT_COLORKEY;
				break;
			case 'l':
				f |= BIT_LAYERS;
				break;
			case '+':
				f |= BIT_PLUS;  // removed 9/8/10 reinstated 5/24/11
				break;
//			case 'd':
//				f |= BIT_BIT6;
//				break;
			case '7':
				f |= BIT_CAMERA;
				break;
			case 'a':
				f |= BIT_LIBRARY;
				break;
			default:
				break;
			}
		}
		q = prod | (f << 16);
		k = m_list.AddString(buf);
		m_list.SetItemData(k,q);
		if (!_stricmp(name,m_default))
			sel = k;
		prod_count++;
		if (!m_arg[i]) break;
		}
	if (!prod_count)
		m_error = 2;
	if (m_error)
		PostMessage(WM_COMMAND,IDCANCEL);
	m_list.SetCurSel(sel);
	OnSelChange();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CProdDlg::OnNoLicense()
{
	m_error = 97;
	PostMessage(WM_COMMAND,IDCANCEL);
}

void CProdDlg::OnSelChange()
{
	int z = m_list.GetCurSel();
	char buf[80];
	m_list.GetText(z,buf);
	if (m_bFancy)
		{
		int i;
		for (i = 0; buf[i] && (buf[i] != ':');i++);
		buf[i] = 0;
		}
	strcpy(m_default,buf);
	UINT q = m_list.GetItemData(z);
	m_product = q & 0xffff;
	m_features = q >> 16;
}
//
//	had trouble with traditional approach
//
void CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	theApp.OnTimer(idEvent);
}


BOOL CSketchApp::ChooseLicense()
{
	CMyId * pId = new CMyId;
	char mac[20];
	char ip[80];
	pId->GetMac((LPSTR)mac);
	delete pId;
	int i;
	for (i = 5; i >= 0; i--)
		{
		UINT v = mac[i];
		mac[2*i+0] = (v / 16) > 9 ? 'A' - 10 + ((v / 16) & 15) : '0' + ((v / 16) & 15);
		mac[2*i+1] = (v % 16) > 9 ? 'A' - 10 + (v & 15) : '0' + (v & 15);
		}
	mac[12] = 0;
	char name[80];
	GetKeyString(5,name,sizeof(name));
	int flags = GetKeyInt(6);
	ip[0] = 0;
//	strcpy(ip,"70.239.199.4");
//	strcpy(ip,"123.123.123.123");
//	strcpy(ip, "68.5.252.84");
//	strcpy(ip, "71.46.255.194");
	KillLicense();
	BOOL bFancy = (flags & 2) ? TRUE : FALSE;
	m_pLicense = new CLicense;
	if (flags & 1)
		m_pLicense->Copying(1);
	if (flags & 4)
		m_pLicense->SetExec(2);
	int res = m_pLicense->Setup(ip,mac);
	if (res < 0)
		{
		if ((res == -2) || (res == -3))
			{
			AfxMessageBox(IDS_NO_CLIENT);
			}
		else
			{
		//	char buf[80];
		//	sprintf(buf,"%d",res);
		//	CString msg;
		//	AfxFormatString1(msg,IDS_NO_LICENSEMGR, buf);
		//	AfxMessageBox(msg);
			FormattedMsg(IDS_NO_LICENSEMGR, res);
			}
		KillLicense();
		m_dwLicense = 0;
		PutKeyInt(10,0);
		return 0;
		}
	DWORD features = 0;
//	SetDlgItemInt(IDC_VERSION,pLic->Version());
	char info[1000];
	strncpy(info,m_pLicense->m_rsp,999);
	info[999] = 0;
	for (;;)
		{
		res = -99;
		CProdDlg dlg;
		dlg.m_bFancy = bFancy;
		strcpy(dlg.m_default, name);
		strcpy(dlg.m_arg,info);
		if (dlg.DoModal() != IDOK)
			{
			if (dlg.m_error == 97)
				res = 97;
			else if (dlg.m_error)
				AfxMessageBox(IDS_NO_LIC_PRODUCTS);
			break;
			}
		m_pLicense->NoRenew(dlg.m_bNoRenew);
		features = dlg.m_features;
		strcpy(m_pLicense->m_client,dlg.m_client);
		res = m_pLicense->Obtain(dlg.m_product);
		if (res < 0)
			{
			if (res == -1)
				{
				if (MyWarning(IDD_COPIES_IN_USE) != IDOK)
					break;
				}
			else
				{
				FormattedMsg(IDS_NO_LICENSEMGR, res);
				}
			}
		else
			{
			res = 0;
			strcpy(name, dlg.m_default);
			break;
			}
		}
	if (res)
		{
		KillLicense();
		if (res != -99)
			PutKeyInt(10,0);
		m_dwLicense = 0;
		}
	else
		{
		PutKeyInt(10,1);
		PutKeyString(5,name);
		m_dwLicense = 1;
		ProcessMsg(0,9,features);
		m_nTimer = ::SetTimer(0,1,540000, TimerProc);
		ASSERT(m_nTimer);
		}

	return 0;
}


void GetRegistryEntry ( LPCSTR szInPath, LPCSTR szInValue,
                        LPSTR  szOut,    UINT   uiOutSize )
{
    szOut[0] = 0;
#ifndef FLIPBOOK_MAC
    HKEY hKey;
    if (RegOpenKey(HKEY_CLASSES_ROOT, szInPath, &hKey)
        == ERROR_SUCCESS)
    {
        LONG lSize = (LONG) uiOutSize;
        RegQueryValue(hKey, szInValue, szOut, &lSize);
        RegCloseKey(hKey);
    }
#endif
}

void CSketchApp::SaveKeyMessages(UINT idc, LPCSTR iid, UINT days, LPCSTR code)
{
	if (idc)
		PutKeyString(7,iid);
	else
		PutKeyString(7,"");
	if (days)
		PutKeyString(2,code);
	else if (idc)
		PutKeyString(1, code);
	else
		{
		PutKeyString(0,code);
		CString file;
		CString msg;
		if (FindUSBMsg(file, msg))
			WriteUSBMsg(file,code);
		}
}

void CSketchApp::Association()
{
#ifndef FLIPBOOK_MAC
	char szProgPath[MAX_PATH];
	::GetModuleFileName(NULL, szProgPath, sizeof(szProgPath)-1);
	char szOut[MAX_PATH];
    szOut[0] = 0;

    char szExt[16];
#ifdef _DISNEY
	strcpy(szExt,".CBK");
#else
	strcpy(szExt,".DGC");
#endif
	int work = 1;
    char szName[256];
    GetRegistryEntry(szExt, "", szName, sizeof(szName));
    if (szName[0])
		{
	    lstrcat(szName, "\\shell\\open\\command");
	    GetRegistryEntry(szName, "", szOut, 299);
		int i;
		for (i = 0; szOut[i] && (szOut[i] != '%');i++);
		if ((szOut[i] == '%') && (i > 0)) szOut[i-1] = 0;
		
    	if (!_stricmp(szProgPath,szOut))
			work = 0;
		}
	CString StrExt;
	CString StrExe;
	if (GetProfileInt("Options","ExternalEditAssoc", 0))
		{
		StrExe = GetProfileString("Options","ExternalEditExe","");
		StrExt = GetProfileString("Options","ExternalEditExt","");
		if (StrExe.GetLength() && StrExt.GetLength())
			work |= 2;
		}
/*
	if (MyWarning(IDD_ASSOC_EXE, RF_ASSOC_EXE) != IDOK)
		return;

*/
	if (!work)
		return;

	CGCFileTypeAccess TheFTA;

	if (work & 1)
	{
	CString csTempText;
#ifdef _DISNEY
	TheFTA.SetExtension("CBK");
#else
	TheFTA.SetExtension("DGC");
#endif

	// just pass file path in quotes on command line
	csTempText  = szProgPath;
	csTempText += " %1 ";//" \"%1\"";
	TheFTA.SetShellOpenCommand(csTempText);
	TheFTA.SetDocumentShellOpenCommand(csTempText);
#ifdef _DISNEY
	TheFTA.SetDocumentClassName("ColoringBook.Document");
#else
#ifdef _THEDISC
	TheFTA.SetDocumentClassName("TheDISC!.Document");
#else
	TheFTA.SetDocumentClassName("FlipBook.Document");
#endif
#endif
	// use first icon in program
	csTempText  = szProgPath;
	csTempText += ",0";
	TheFTA.SetDocumentDefaultIcon(csTempText);

	TheFTA.RegSetAllInfo();
	}
//
//
//
	if (work & 2)
		{
		CString csTemp  = StrExe;
		csTemp += " %1 ";
		TheFTA.SetShellEditor(StrExt,csTemp);
		WriteProfileInt("Options","ExternalEditAssoc", 0);
		}
#endif
}

CBiggy::CBiggy()
{
	m_nVer = 0;
	l[0] = 0;
	l[1] = 0;
}

void CBiggy::Assign(int a, int b, int c, int d)
{
	l[0] = b + 100000 * a;
	l[1] = d + 100000 * c;
//	char temp[30];
//sprintf(temp,"%5u-%5u-%5u-%5u",result.l[0] / 100000,
//				result.l[0] % 100000,result.l[1] / 100000,result.l[1] % 100000);
//DPF("res:%10lu,%10lu",result.l[0],result.l[1]);
//		DPF("temp:%s",temp);
	l[0] = SWAPV(l[0]);
	l[1] = SWAPV(l[1]);
}

BOOL CBiggy::Eval(UINT idc, UINT idk, UINT idz)
{
	UINT zz = idc;
	UINT ver;
	if (!m_nVer)
		ver = l[1] >> 28;
	else
		ver = l[1] >> 29;
	l[1] ^= 0x5aa5;
	if (zz > 3) zz = 1;
	if ((l[0] != idz) || ver || (idk != zz) ||
				((l[1] & 15) > FEAT_PRO) || !(l[1] & 15))
		{
		l[1] = 0;
		return 1;
		}
	else
		return 0;
}

BOOL CBiggy::Eval2(UINT msgc, UINT sysc, UINT sysid)
{
	UINT ttyps[7] = {0,1,2,3,1,1,1};
	UINT ver;
	if (!m_nVer)
		ver = l[1] >> 28;
	else
		ver = l[1] >> 29;
	l[1] ^= 0x5aa5;
	if (ver || (ttyps[msgc] != ttyps[sysc]) ||
				((l[1] & 15) > FEAT_PRO) || !(l[1] & 15))
		{
		l[1] = 0;
		return 1;
		}
	sysc = sysc > 1 ? sysc - 3 : 0;
	msgc = msgc > 1 ? msgc - 3 : 0;
	UINT actc = msgc;
	if (sysc > actc) actc = sysc;	// smallest piece
	
	UINT sysshift = sysc ? 32 / (sysc + 1) : 0;
	UINT msgshift = msgc ? 32 / (msgc + 1) : 0;
	UINT actshift = actc ? 32 / (actc + 1) : 0;
	UINT actmask;
	if (actshift)
		{
		actmask = 1;
		actmask <<= actshift;
		actmask--;
		actmask <<= (32 - actshift); // 0xff000000
		}
	else
		actmask = 0xffffffff;

	UINT i, j;
	for (i = 0; i <= sysc; i++)
		{
// get bits into position, always bit 31 and lower
		UINT sysv = sysid << (sysshift * i);
		for (j = 0; j <= msgc; j++)
			{
			UINT msgv = l[0] << (msgshift * i);
			if ((sysv & actmask) == (msgv & actmask))
				return 0;
			}
		}
	return 1;
}

void CBiggy::Prepare()
{
	l[0] = SWAPV(l[0]);
	l[1] = SWAPV(l[1]);
//DPF("out:%10lu,%10lu",l[0],l[1]);
	if (!m_nVer)
		l[0] ^= 0x55aa55aa;
	else if (m_nVer == 1)
		l[0] ^= 0x5a5aa5a5;
	else
		l[0] ^= 0xaa55aa55;
//DPF("z:%8x,l0:%8x,l1:%8x",idz,l[0],l[1]);
//DPF("z:%10lu,l0:%10lu,l1:%8lu",idz,l[0],l[1]);
	l[1] ^= 0x5aa5;
}

void CBiggy::Extract(UINT &ver, UINT &idk, UINT & days)
{
	if (!m_nVer)
		{
		ver = l[1] >> 28;
		idk = (l[1] >> 26) & 3;
		days = (l[1] >> 16) & 1023;
		}
	else
		{
		ver = l[1] >> 29;
		idk = (l[1] >> 27) & 3;
		days = (l[1] >> 16) & 2047;
		}
}

int checkdigit(char * txt, int nAppend /*= 0*/)
{

int t[10][10] = {
	0,1,2,3,4,5,6,7,8,9,
	1,2,3,4,0,6,7,8,9,5,
	2,3,4,0,1,7,8,9,5,6,
	3,4,0,1,2,8,9,5,6,7,
	4,0,1,2,3,9,5,6,7,8,
	5,9,8,7,6,0,4,3,2,1,
	6,5,9,8,7,1,0,4,3,2,
	7,6,5,9,8,2,1,0,4,3,
	8,7,6,5,9,3,2,1,0,4,
	9,8,7,6,5,4,3,2,1,0};

int t2[8][10] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 
	1, 5, 7, 6, 2, 8, 3, 0, 9, 4, 
	5, 8, 0, 3, 7, 9, 6, 1, 4, 2, 
	8, 9, 1, 6, 0, 4, 3, 5, 2, 7, 
	9, 4, 5, 3, 1, 2, 6, 8, 7, 0, 
	4, 2, 8, 6, 5, 7, 3, 9, 0, 1, 
	2, 7, 9, 3, 8, 0, 6, 4, 1, 5, 
	7, 0, 4, 6, 9, 1, 3, 2, 5, 8}; 

int inv[10] = {0, 4, 3, 2, 1, 5, 6, 7, 8, 9};
	int i;
	int v = 0;
	int a = nAppend ? 1 : 0;
	int c = strlen(txt) + a;
	for (i = a; i < c; i++)
		{
		if ((txt[c-1-i] < '0') || (txt[c-1-i] > '9'))
			break;
		int z = txt[c-1-i] & 15;
		int q = t2[i % 8][z];
		int v1 = t[v][q];
//	printf("i:%d,arg:%d,v:%d,v1:%d,q:%d\n",i,z,v,v1,q);
		v = v1;
		}
	if (i < c)
		return -1;
	else if (!nAppend)
		return v;
	else
		{
		if (nAppend > 1)
			{
			txt[i-1] = '0' + inv[v];
			txt[i] = 0;
			}
		return inv[v];
		}
}

void CSketchApp::MayaInfo( UINT &w, UINT &h, UINT &n, UINT &l)
{
	m_pMaya->Info(w,h,n,l);
}


UINT CSketchApp::MyDualMessageBox(UINT Id1, UINT Id2, UINT code)
{
	CString str1;
	CString str2;
	str1.LoadString(Id1);
	str2.LoadString(Id2);
	str1 += "\n";
	str1 += str2;
	return AfxMessageBox(str1,code);
}

int SendIt(LPCSTR url, LPCSTR objs, LPCSTR args, LPSTR rsp, UINT max)
{
	rsp[0] = 0;
	const TCHAR szHeaders[]=
			_T("Accept: text/*\r\nUser-Agent: MFC_Tear_Sample\r\n");
    //  		_T("Content-Type: application/x-www-form-urlencoded");
	UINT RetCode = 0;
	int answer = 99;
	DWORD   dwAccessType = PRE_CONFIG_INTERNET_ACCESS;

	DWORD dwHttpRequestFlags =
		INTERNET_FLAG_EXISTING_CONNECT | INTERNET_FLAG_NO_AUTO_REDIRECT;

	CInternetSession session(_T("DigiCel Emailer"), dwAccessType);
	CHttpConnection* pServer = NULL;
	CHttpFile* pFile = NULL;
	try
	{
		CString strServerName = url;
		CString strObject = objs;
		strObject += '?';
		strObject += args;
		INTERNET_PORT nPort = 80;//nPort;
		DWORD dwServiceType = INTERNET_SERVICE_HTTP;

		pServer = session.GetHttpConnection(strServerName, nPort);

		pFile = pServer->OpenRequest(CHttpConnection::HTTP_VERB_GET,
			strObject, NULL, 1, NULL, NULL, dwHttpRequestFlags);
		pFile->AddRequestHeaders(szHeaders);
		pFile->SendRequest();

		DWORD dwRet;
		pFile->QueryInfoStatusCode(dwRet);

		if (dwRet == 200)
			{
			TCHAR sz[24];
		int index = 0;
		int state = 0;
		int j = 0;
		sz[0] = 0;
		for (;;)
			{
				if (!pFile->ReadString(sz, sizeof(sz) - 1))
					break;
				if ((strlen(rsp) + strlen(sz)) < max)
				strcat(rsp,sz);
				}
			}
		if (!rsp[0]) //strcmp(rsp,"<HTML>"))
			answer = 13;
		else
			answer = 0;
//		pFile->Close();
//		if (pServer)
//		pServer->Close();
	}
	catch (CInternetException* pEx)
	{
		// catch errors from WinINet

		TCHAR szErr[1024];
		pEx->GetErrorMessage(szErr, 1024);

		char buf[200];
		sprintf(buf,"Error:%s",szErr);
		DPF(buf);

		RetCode = 2;
		pEx->Delete();
	}


	if (pFile != NULL)
	{
		pFile->Close();
		delete pFile;
	}
	if (pServer != NULL)
	{
		pServer->Close();
		delete pServer;
	}
	session.Close();
	
	return answer;
}

void CSketchApp::SendEmail(CString iid)
{
	COnLineDlg dlg;
	dlg.m_Name = "";
	dlg.m_Id = iid;
	dlg.m_email1 = "";
	dlg.m_email2 = "";
	dlg.m_order = "";
	dlg.m_kind = 0;
//dlg.m_Name = "Rick";
//dlg.m_email1 = "rix@acm.org";
//dlg.m_order = "12345";
	if (dlg.DoModal() != IDOK)
		return;
	char url[328];
	char obj[328];
	char args[328];
	char rsp[300];
	UINT target;
	int answer = 99;
	for (target = 0; target < 1; target++)
		{
		if (target == 0)
			strcpy(url,"www.digicelinc.com");
		else
			strcpy(url,"www.flipbookpro.com");
		strcpy(obj,"/test/mailtest.php");
		strcpy(args,"passcode=");
		strcat(args,"toby");
		strcat(args,"&name=");
		strcat(args,dlg.m_Name);
		strcat(args,"&email_address=");
		strcat(args,dlg.m_email1);
		strcat(args,"&flipbook_id=");
		strcat(args,dlg.m_Id);
		if (dlg.m_kind)
			{
			strcat(args,"&other_vendor=?");
			strcat(args,"&order_number=?");
			}
		else
			{
			strcat(args,"&order_number=");
			strcat(args,dlg.m_order);
			strcat(args,"&other_vendor=?");
			}
		answer = SendIt(url, obj, args,rsp, sizeof(rsp));
		DPF("answer:%d",answer);
		DPF("rsp:%s",rsp);
		}
	if (answer)
		{
		FormattedMsg(IDS_ERR_LICENSE_UNLOCK , answer);
		}
	else if (dlg.m_kind)
		MyDialog(IDD_OTHER);
	else
		MyDialog(IDD_OURS);

}

UINT CSketchApp::CheckVersion()
{
	char url[328];
	char obj[328];
	char rsp[1300];
	int answer = 99;
	strcpy(url,"www.digicel.net");
	strcpy(obj,"/check4update.htm");
	answer = SendIt(url, obj, "",rsp,sizeof(rsp));
	DPF("answer:%d",answer);
	if (answer)
		{
	  	MyDualMessageBox(IDS_NO_DIGICEL_CONNECT, IDS_CHECK_INTERNET);
		return 0;
		}
	int i,j,c;
	char key[30];
	strcpy(key,"FlipBook PC Version");
	c = strlen(key) + 5;
	for (i = 0;rsp[i+c];i++)
		{
		for (j = 0; key[j]; j++)
			if (key[j] != rsp[i+j])
				break;
		if (!key[j])
			break;
		}
	if (key[j])
		return 0;
	UINT v1,v2;
	if (sscanf(rsp+i+c-5,"%u.%u",&v1,&v2) != 2)
		return 0;
	return 100*v1 + v2;
}

void CSketchApp::OnUpdates()
{
	UINT version = CheckVersion();
	if (MYVER < version)
		{
		AfxMessageBox(IDS_VERSION_OUT_OF_DATE);
//		if (MyDualMessageBox( IDS_VERSION_OUT_OF_DATE,
//						IDS_UPDATE_NOW, MB_YESNO) == IDYES)
//			return TRUE;
		}
	else
		AfxMessageBox(IDS_VERSION_UP_TO_DATE);
}


BOOL CSketchApp::CheckForUpdate()
{
	UINT version = CheckVersion();
	if (MYVER < version)
		{
		if (MyDualMessageBox( IDS_VERSION_OUT_OF_DATE,
						IDS_UPDATE_NOW, MB_YESNO) == IDYES)
			return TRUE;
		}
	return FALSE;
}

void GetFormattedText(CString & str, int Id, int arg)
{
	CString fmt;
	if (! fmt.LoadString( Id ))
		str.Format("Error Loading String:%d,%d", Id, arg); 
	else
		str.Format(fmt,arg);
}

void GetFormattedText(CString & str, int Id, LPCSTR arg)
{
	CString fmt;
	if (! fmt.LoadString( Id ))
		str.Format("Error Loading String:%d,%s", Id, arg); 
	else
		str.Format(fmt,arg);
}

void FormattedMsg(UINT Id, int arg)
{
	CString str;
	GetFormattedText(str,Id,arg);
	AfxMessageBox(str,MB_OK);
}

void FormattedMsg(UINT Id, LPCSTR txt)
{
	CString str;
	GetFormattedText(str,Id,txt);
	AfxMessageBox(str,MB_OK);
}

BOOL RemoveFile(LPCSTR pFileName)
{
	BOOL bRes = 0;
	try
		{
    	CFile::Remove(pFileName);
		}
	catch (CFileException* pEx)
		{
		bRes = 1;
		DPF("bad erase:%s|",pFileName);
	   	pEx->Delete();
		}
	return bRes;
}
