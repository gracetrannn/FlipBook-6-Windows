// sketch.h : main header file for the SKETCH application
//

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

//#include "resource.h"       // main symbols

#define WM_DOREALIZE   (WM_USER + 0)
/////////////////////////////////////////////////////////////////////////////
// CSketchApp:
// See sketch.cpp for the implementation of this class
//
//	candofeature types
//
//#define CD_FEAT_TELECINE  1
//#define CD_FEAT_DESKEW    2
//#define CD_FEAT_HIRES     3
//#define CD_FEAT_COLORKEY  4
//#define CD_FEAT_RECORDER  5
//#define CD_FEAT_EXP_FLV   6
//
//	remind flags
//
#define RF_DEMO 0
#define RF_EXTERNAL_EDIT 1
#define RF_EXTERNAL_EMPTY 2
#define RF_ASSOC_EXE 3
#define RF_NO_SAVE 4
#define RF_EXPIRE 5
#define RF_AVI_SCALE 6

#define MyApp ((CSketchApp*)AfxGetApp())

class CLicense;
class CMaya;
class CTablet;
class CSketchApp : public CWinApp
{
public:
	CSketchApp();
    ~CSketchApp();
	enum CanDoKinds {CD_TELECINE,CD_DESKEW,CD_HIRES,
						CD_COLORKEY, CD_RECORDER, CD_EXP_FLV, CD_LIBRARY};
// Overrides
	virtual BOOL InitInstance();
//	virtual BOOL ExitInstance();
	virtual BOOL OnIdle(LONG lCount);

// Implementation
	void BrowseForFolder(CString& str, HWND hWnd, LPCSTR lpszTitle, UINT nFlags);
	BOOL PromptFileName(CString& fileName, UINT Kind);
	BOOL OnInitial();
	BOOL IdCheck(UINT & ApId, UINT & Features);
	BOOL USBCheck(); // at cell open, avoid lamplighter
	BOOL ExpireCheck(BOOL bFail);
	BOOL IdSetup();
	BOOL IsLite();
	BOOL IsPro();
	BOOL IsAnimate();
	BOOL IsMaya() { return m_pMaya ? 1 : 0;};
	BOOL CanCancel(int v = -1);
#ifdef _THEDISC
	BOOL CanDoColor() { return TRUE;};
	BOOL CanDoCamera() { return TRUE;};
#else
	BOOL CanDoColor() { return m_dwFlags & 1 ? TRUE : FALSE;};
	BOOL CanDoCamera() { return m_dwFlags & 2 ? TRUE : FALSE;};
#endif
	BOOL CanDoLayers() { return m_dwFlags & 4 ? TRUE : FALSE;};
	BOOL CanDoFeature( CanDoKinds which);
	BOOL CanDoLibrary() { return CanDoFeature(CD_LIBRARY);};
	int  MyWarning(UINT iddialog, int flag=-1);
	BOOL RemindFlags(int which, int v = -1);
	void PrintSetup() { CWinApp::OnFilePrintSetup();};
	void FileSetup() { SaveStdProfileSettings();};
	BOOL CheckCommandLine();
	void MayaInfo(UINT &w, UINT &h, UINT &n, UINT &l);
	afx_msg void OnTimer(UINT arg);
	CMaya * GetMaya() { return m_pMaya;};
	UINT MyDualMessageBox(UINT Id1, UINT Id2, UINT code = MB_OK);
	CTablet * m_pTablet;
	BOOL HaveWinTab() { return m_bHaveWinTab;};
protected:
	void	MyKeyKey(LPSTR name, LPSTR section, LPSTR key, int which);
	int		GetKeyString(int which, LPSTR buf, int size);
	void    PutKeyString(int which, LPCSTR txt);
	int 	GetKeyInt(int which);
	void	PutKeyInt(int which, int val);

	UINT CheckVersion();
	BOOL ChooseLicense();
	void RenewLicense();
	void KillLicense();
	void Association();
	void RevertToDemo(BOOL bShowMsg = FALSE);
	void InitProfile();
	void MySection(LPSTR section, int which);
	int  CheckProfile();
	CLicense * m_pLicense;
	CLicense * m_pDeadLicense;
	BOOL  MyOpenFile(LPCSTR name);
//	DWORD GetApFeatures();
//	DWORD GetApId();
	BOOL  TestScene(LPCSTR Name);
	DWORD GetSysId(UINT & which, DWORD mac_id);
	UINT CheckUSBKey(BOOL bInitial);
	UINT CheckTimer(BOOL bInitial);
	UINT CheckMsg(UINT idd, CString msg, BOOL bUSB);
	UINT ProcessMsg(UINT idd, UINT idk, UINT code);
	BOOL DoPromptFileName(CString& fileName, UINT nIDSTitle,
			DWORD lFlags, BOOL bOpenFileDialog, UINT kind);
	BOOL SaveFileName(CString& fileName, UINT nIDSTitle, DWORD lFlags, DWORD kind);
	void SaveKeyMessages(UINT idc, LPCSTR id, UINT days, LPCSTR code);
	BOOL CheckForUpdate();
	BOOL TimeCheck();
	BOOL TempCheck();
	BOOL GetId(CString & str, UINT & idz, UINT & idc);
	BOOL BuyIt();
	void SendEmail(CString Id);
	CMaya * m_pMaya;
	BOOL  m_bHaveWinTab;
	BOOL  m_bCanCancel;
	DWORD m_dwRemindFlags;
	DWORD m_dwFlags;
	DWORD m_dwFeatures;
	DWORD m_dwId;
	DWORD m_dwIdk;  // key kind, 0 is usb, 1 is MAC, 2 is S, 3 is V,9 is ??
	DWORD m_dwLicense;
	UINT  m_nLicenseBusy;
	UINT  m_nTimer;
	UINT  m_days;
	CTime m_now;
	//{{AFX_MSG(CSketchApp)
	afx_msg void OnAppAbout();
	afx_msg void OnAppRegister();
	afx_msg void OnFileOpen();
	afx_msg void OnFileNew();
	afx_msg void OnHelp();
	afx_msg void OnUpdates();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

void PASCAL DDV_MyMinMax(CDataExchange * pDX, int id,
		double & value, double min, double max, int places = 3);

BOOL RemoveFile(LPCSTR file_name); // true if could not
void FormattedMsg(UINT Id, LPCSTR txt);
void FormattedMsg(UINT Id, int arg);
/////////////////////////////////////////////////////////////////////////////

