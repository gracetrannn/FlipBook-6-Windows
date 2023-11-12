#include "stdafx.h"
#include "sketch.h"
#include "myview.h"
#include "fbqt.h"
#include "mydialog.h"
#include "mymci.h"

#define TIMER_ID 17
#define TIMER_LOOP 11
//#define TIMER_DELAY 200
#ifdef _DEBUG
#define _QDEBUG
#endif

class CRecordDlg;
class CVWnd2 : public CWnd
{
public:
	BOOL MyCreate(CWnd * pParent,CWnd * pDlg,int x, int y, int w, int h, BOOL bMCI);
	HWND m_MCI;
protected:
	LRESULT OnMyNotify(WPARAM hFont, LPARAM /*lParam */);
	DECLARE_MESSAGE_MAP()
	CWnd * m_pParent;
	CRecordDlg * m_pDlg;
	int m_initx;
	int m_inity;
};

class CRecOptDlg : public CMyDialog
{
public:
	CRecOptDlg(CWnd* pParent = NULL);	// standard constructor
	enum { IDD = IDD_RECOPT};
	CRecordDlg * m_pMyParent;
	BOOL m_bPreview;
	BOOL m_bAudio;
	BOOL m_bErase;
	UINT m_nCapSize;
	UINT m_TimeLimit;
	UINT m_Delay;
	void FillCombos(UINT mask);
	void UpdateButtons();
	void UpdateDeviceInfo();
	void OnChange();
	char m_szVidDevice[180];
	char m_szAudDevice[180];
	char m_szCompressor[180];

	CComboBox	m_vid_devices;
	CComboBox	m_aud_devices;
	CComboBox	m_vid_dialogs;
	CComboBox	m_aud_dialogs;
	CComboBox	m_compressors;

	virtual BOOL OnInitDialog();
	afx_msg void OnPreview();
//	afx_msg void OnVideo();
	afx_msg void OnAudio();
	afx_msg void OnExport();
	afx_msg void OnSelChangeVidDev();
	afx_msg void OnSelChangeAudDev();
	afx_msg void OnSelChangeCompressor();
	afx_msg void OnSelChangeVidDlg();
	afx_msg void OnSelChangeAudDlg();
	afx_msg void OnSaveSettings();
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	DECLARE_MESSAGE_MAP()
};

class CRecordDlg : public CMyDialog
{
// Construction
public:
	CRecordDlg(CWnd* pParent = NULL);	// standard constructor
	~CRecordDlg();
	enum { IDD = IDD_RECORD };
	CSketchDoc * m_pDoc;
	CSketchView * m_pView;
	int Option(int id);
	void Option(int id, int val);
	void PutOptionString(int id, LPSTR txt);
	void GetOptionString(int id, LPSTR txt, int size);

	BOOL m_bPreview;
	BOOL m_bAudio;
	BOOL m_bErase;
	UINT m_TimeLimit;
	UINT m_Delay;
	UINT m_nCapSize; // pre allocate capture size
	char m_szCaptureFile[300];
	char m_szPlayFile[300];
	char m_szVidDevice[180];
	char m_szAudDevice[180];
	char m_szCompressor[180];
	void SaveSettings(CRecOptDlg * pDlg);
	protected:
	
	int m_width;
	int m_height;
	int m_x;
	int m_y;
	int m_w;
	int m_h;
	UINT m_timer;

	void LoadSettings();
//	HICON m_hIcon;
	CVWnd2 * m_pVideo; 
	CVWnd2 * m_pPlayWnd; 
//	CWnd * m_pParent;
	CMyMCI * m_pMyMCI;
	void OnChange();
	void CheckPath();
	BOOL m_bCanPlay;

	CSliderCtrl m_slider;

	UINT m_state; // 0 is live, 1 is paused, 2 is playing
	BOOL m_bLoop;
	UINT m_Notified;
	void SetupSlider();
	void UpdateSlider();
	void PlaySetup();
	void SetWindowSize();
	void DoIt(int cmd);
	void PauseIt(BOOL bRewind);
	void ChangePos(UINT Pos);

	void UpdateSettings(CRecOptDlg & dlg);
	void StartCapture();
//	void StopCapture();
	
	UINT m_frames;
	UINT m_frame;
	void GetKey(CString & name, CString & section, CString & key, int id);
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnStartUp();
	afx_msg void OnLooper();
	afx_msg void OnRecord();
	afx_msg void OnPlay();
	afx_msg void OnStop();
	afx_msg void OnBegin() { DoIt(0);};
	afx_msg void OnEnd() { DoIt(1);};
	afx_msg void OnFwd() { DoIt(2);};
	afx_msg void OnBack() { DoIt(3);};
	afx_msg void OnRepeat();
	afx_msg void OnImport();
	afx_msg void OnSettings();
	afx_msg void OnFileClose();
	afx_msg void OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar );
	virtual BOOL DestroyWindow();
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	void UpdateDeviceInfo();
	afx_msg void OnTimer(UINT nTimerID);
	DECLARE_MESSAGE_MAP()
friend
	CVWnd2;
};

class CRecordingDlg : public CMyDialog
{
// Construction
public:
	CRecordingDlg(CWnd* pParent = NULL);	// standard constructor
// Dialog Data
	UINT m_timer1;
	UINT m_timer2;
	UINT m_state;
	enum { IDD = IDD_RECORDING};
protected:
	UINT m_timer;
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnNow();
	afx_msg void OnTimer(UINT nTimerID);
	DECLARE_MESSAGE_MAP()
};


CRecordDlg::CRecordDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CRecordDlg::IDD, pParent)
{
//	m_pParent = pParent;	
	m_pVideo = 0;
	m_pPlayWnd = 0;
	m_pMyMCI = 0;
//	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}


CRecordDlg::~CRecordDlg()
{
	FBVideoRecordStop(1);
	delete m_pVideo;
	delete m_pPlayWnd;
	delete m_pMyMCI;
}

BOOL CRecordDlg::DestroyWindow() 
{
	m_pMyMCI->Destroy();
	FBVideoRecordStop(0);
    return CMyDialog::DestroyWindow();
}

void CRecordDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_SLD_SLIDER, m_slider);
}

BEGIN_MESSAGE_MAP(CRecordDlg, CMyDialog)
	ON_COMMAND(IDC_STARTUP, OnStartUp)
	ON_COMMAND(IDC_REC_LOOPER, OnLooper)
	ON_COMMAND(IDC_SLD_RECORD, OnRecord)
	ON_COMMAND(IDC_SLD_PLAY, OnPlay)
	ON_COMMAND(IDC_SLD_STOP, OnStop)
	ON_COMMAND(IDC_SLD_BEGIN, OnBegin)
	ON_COMMAND(IDC_SLD_END, OnEnd)
	ON_COMMAND(IDC_SLD_BACK, OnBack)
	ON_COMMAND(IDC_SLD_FWD, OnFwd)
	ON_COMMAND(IDC_SLD_LOOP, OnRepeat)
//	ON_COMMAND(IDC_DO_REPEAT, OnDoRepeat)
	ON_COMMAND(IDC_SETTINGS, OnSettings)
	ON_COMMAND(IDC_IMPORT, OnImport)
	ON_WM_HSCROLL()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CRecordDlg::OnTimer(UINT nTimerID)
{
	if ((nTimerID == TIMER_LOOP) && (m_state == 2) && m_bLoop)
		{
		if (m_Notified == 2)
			{
			m_Notified = 3;
			PostMessage(WM_COMMAND, IDC_REC_LOOPER);
			}
		else if (m_Notified == 1)
			m_Notified = 2;
		}	
}

void CRecordDlg::OnLooper()
{
	DPF("on looper");
	m_pMyMCI->Pause();
	m_pMyMCI->Seek(0);
	m_pMyMCI->Play();
	m_Notified = 0;
}

void CRecordDlg::ChangePos(UINT pos)
{
	if ((m_state == 2) && m_bLoop)
		{
		if ((m_Notified == 2) || (m_Notified == 0))
			m_Notified = 1;
		}
}

LRESULT CVWnd2::OnMyNotify(WPARAM wParam, LPARAM lParam )
{
	DPF("notify:%d", lParam);
	LRESULT result = 0;
	if ((HWND)wParam == m_MCI)
		{
		UINT pos = (UINT)lParam;
		((CRecordDlg *)m_pParent)->ChangePos(pos);
		}
	else
		result = Default();
	return result;
}

void CRecordDlg::OnImport()
{
	EndDialog(IDC_IMPORT);
}

BOOL CRecordDlg::OnInitDialog()
{
	m_bPreview = 1;
	m_bAudio = 1;
	m_bErase = 0;
	m_state = 0;
	m_bCanPlay = 0;
	m_bLoop = 1;
	m_Delay = 10;
	CMyDialog::OnInitDialog();
	GetDlgItem(IDC_SLD_STOP)->EnableWindow(FALSE);
	CheckDlgButton(IDC_SLD_LOOP, m_bLoop);

	LoadSettings();

	RECT rcView;
	m_pView->GetWindowRect(&rcView);
	m_width = rcView.right - rcView.left;
	m_height = rcView.bottom - rcView.top;
	if (m_width * 3 > 4 * m_height)
		{
		m_h = m_height;
		m_w = 4 * m_height / 3;
		m_x = (m_width - m_w) / 2;
		m_y = 0;
		}
	else
		{
		m_w = m_width;
		m_h =  3 * m_width / 4;
		m_x = 0;
		m_y = (m_height - m_h) / 2;
		}
	m_pMyMCI = new CMyMCI;
	m_pVideo = new CVWnd2;
	if (!m_pVideo->MyCreate(m_pView, this,
				rcView.left,rcView.top,m_width, m_height,0))
		{
		TRACE("Failed to create video window\n");
		delete m_pVideo;
		m_pVideo = 0;
		}
	m_pPlayWnd = new CVWnd2;

	if (!m_pPlayWnd->MyCreate(m_pView,this,rcView.left, rcView.top,
						m_width, m_height,1))
		{
			TRACE("Failed to create play video window\n");
			delete m_pPlayWnd;
			m_pPlayWnd = 0;
			}
		else
			PlaySetup();

#ifndef FLIPBOOK_MAC
	RECT rcDesk;
	RECT rcMe;
	int winx = Option(RECORD_X);
	int winy = Option(RECORD_Y);
	GetDesktopWindow()->GetWindowRect(&rcDesk);
	GetWindowRect(&rcMe);
	if ((int)winx < rcDesk.left)
		winx = rcDesk.left;
	else if (((int)winx + rcMe.right - rcMe.left) > rcDesk.right)
		winx = rcDesk.right - rcMe.right + rcMe.left;
	if ((int)winy < rcDesk.top)
		winy = rcDesk.top;
	else if (((int)winy + rcMe.bottom - rcMe.top) > rcDesk.bottom)
		winy = rcDesk.bottom - rcMe.bottom + rcMe.top;
	SetWindowPos(&wndTop,winx,winy,0,0,SWP_NOSIZE|SWP_SHOWWINDOW);
#endif
	GetDlgItem(IDC_SLD_RECORD)->SetFocus();
	SetWindowText("Allocating Capture File");

	PostMessage(WM_COMMAND, IDC_STARTUP);
	VERIFY(SetTimer(TIMER_LOOP, 2000, NULL) == TIMER_LOOP);
	return FALSE;  // return TRUE  unless you set the focus to a control
}

BOOL CVWnd2::MyCreate(CWnd * pParent, CWnd * pDlg, 
					int x, int y, int w, int h, BOOL bMCI)
{
	m_pParent = pDlg;
	RECT rcwin;
	rcwin.top = y;
	rcwin.left = x;
	rcwin.bottom = y + h;
	rcwin.right = x + w;
	BOOL bRes;
	if (bMCI)

		bRes = CreateEx(0, NULL, NULL,
				WS_CHILD | WS_POPUP,
					rcwin,pParent, 0);
	else
		bRes = CreateEx(0, NULL, NULL,
				WS_CHILD | WS_POPUP | WS_VISIBLE,
					rcwin, pParent, 0);
	return bRes;
}

BEGIN_MESSAGE_MAP( CVWnd2, CWnd )
    ON_MESSAGE(MCIWNDM_NOTIFYPOS, OnMyNotify)
END_MESSAGE_MAP()

void CRecordDlg::SaveSettings(CRecOptDlg * pDlg)
{
	RECT rect;
	GetWindowRect(&rect);
	int winx = rect.left;
	int winy = rect.top;

	int index;
	char name[80];
	index = FBVideoRecordCurrentDevice (0);
	FBVideoRecordDeviceName (name, index, 0);
	PutOptionString(REC_VNAME, name);
	index = FBVideoRecordCurrentDevice (1);
	FBVideoRecordDeviceName (name, index, 1);
	PutOptionString(REC_ANAME, name);
	index = FBVideoRecordCurrentDevice (2);
	FBVideoRecordDeviceName (name, index, 2);
	PutOptionString(REC_CNAME, name);


	Option(REC_CAP_AUD,pDlg->m_bAudio);

	Option(REC_PREVIEW,pDlg->m_bPreview);
	Option(REC_ERASE,pDlg->m_bErase);

	Option(REC_TIME,pDlg->m_TimeLimit);
	Option(REC_DELAY,pDlg->m_Delay);
	Option(REC_SIZE,pDlg->m_nCapSize);

	Option(RECORD_X,winx);
	Option(RECORD_Y,winy);
}

void CRecordDlg::OnSettings()
{
	if (m_state)
		OnStop();
	ASSERT(m_state == 0);
//	m_pMyMCI->Close();
	int nVid = FBVideoRecordCurrentDevice (0);
	int nAud = FBVideoRecordCurrentDevice (1);
	int nCmp = FBVideoRecordCurrentDevice (2);
	CRecOptDlg dlg;
	dlg.m_pMyParent = this;
	dlg.m_bPreview = m_bPreview;
	dlg.m_bAudio = m_bAudio;
	dlg.m_bErase = m_bErase;
	dlg.m_nCapSize = m_nCapSize;
	dlg.m_TimeLimit = m_TimeLimit;
	dlg.m_Delay = m_Delay;
	UINT SaveSize = m_nCapSize;
	if (dlg.DoModal() == IDOK)
		{
		m_bPreview = dlg.m_bPreview;	// alreday done

		m_bAudio = dlg.m_bAudio;
		m_bErase = dlg.m_bErase;
		m_nCapSize = dlg.m_nCapSize;
		m_TimeLimit = dlg.m_TimeLimit;
		m_Delay = dlg.m_Delay;
		if (SaveSize != m_nCapSize)
			RemoveFile(m_szCaptureFile);
		}
	else
		{
		FBVideoRecordSelectDevice (nVid, 0);
		FBVideoRecordSelectDevice (nAud, 1);
		FBVideoRecordSelectDevice (nCmp, 2);
		FBVideoRecordPreview(m_bPreview);
		FBVideoRecordDoAudio(m_bAudio);
		}
//	m_pMyMCI->Open(m_szPlayFile);
}

void CRecordDlg::OnStartUp()
{
	int zz = FBVideoRecordBegin (m_pVideo,m_szVidDevice, m_szAudDevice,
			m_szCompressor,	m_szCaptureFile, m_bAudio, m_bPreview, m_nCapSize);
DPF("z:%d",zz);
//	if (m_pPlayWnd)
//		PlaySetup();  // must be after record begin
	SetWindowText("Ready To Record");
//	m_pMyMCI->Open(m_szCaptureFile);
	if (!FBVideoRecordDeviceCount(1))
		{
DPF("empty");
		PostMessage(WM_COMMAND, IDCANCEL,0);
		}
	UpdateDeviceInfo();
}


int  CRecordDlg::Option(int id)
{
	return m_pDoc->Option(id);

}

void CRecordDlg::Option(int id, int val)
{
	m_pDoc->Option(id,1,val);
}

void CRecordDlg::PutOptionString(int id, LPSTR txt)
{
	m_pDoc->OptionString(id, txt,1);
}

void CRecordDlg::GetOptionString(int id, LPSTR buf, int size)
{
	m_pDoc->OptionString(id, buf);
}

void CRecordDlg::UpdateDeviceInfo()
{
	int index = FBVideoRecordCurrentDevice(0);
	FBVideoRecordDeviceName (m_szVidDevice,index, 0);
//	GetDlgItem(IDC_CAP_DEVICE)->SetWindowText(m_szVidDevice);
	index = FBVideoRecordCurrentDevice(1);
	FBVideoRecordDeviceName(m_szAudDevice,index,1);
	index = FBVideoRecordCurrentDevice(2);
	FBVideoRecordDeviceName(m_szCompressor,index,2);
}

void CRecordDlg::LoadSettings()
{
	GetOptionString(REC_VNAME,m_szVidDevice,sizeof(m_szVidDevice));
	GetOptionString(REC_ANAME,m_szAudDevice,sizeof(m_szAudDevice));
	GetOptionString(REC_CNAME,m_szCompressor,sizeof(m_szCompressor));
//	GetOptionString(OPT_STR_REC_GUID, m_szGUID,sizeof(m_szGUID));
#ifdef _DEBUG
	strcpy(m_szCaptureFile,"c:\\flipbook\\avi\\fbrecord.avi");
	strcpy(m_szPlayFile,"c:\\flipbook\\avi\\fbrplay.avi");
#else
	GetTempPath(300, m_szCaptureFile);
	if (strlen(m_szCaptureFile) &&
				(m_szCaptureFile[strlen(m_szCaptureFile)-1] !=NATIVE_SEP_CHAR))
			strcat(m_szCaptureFile,NATIVE_SEP_STRING);
	strcpy(m_szPlayFile,m_szCaptureFile);
	strcat(m_szCaptureFile,"FBRECORD.AVI");
	strcat(m_szPlayFile,"FBRCPLAY.AVI");
#endif

	PAVIFILE	pfile = 0;
	AVIFILEINFO pfi;
DPF("before open");
	HRESULT hResult = AVIFileOpen(&pfile, m_szCaptureFile, 0, 0L);
DPF("result:%x",(DWORD)hResult);
	if (!hResult)
		{
		AVIFileInfo(pfile,&pfi,sizeof(pfi));
		DPF("streams:%d, max rate:%d",pfi.dwStreams,pfi.dwMaxBytesPerSec);
DPF("flags:%lX, caps:%lX",pfi.dwFlags,pfi.dwCaps);
#ifdef _DEBUG
	if (pfi.dwFlags & AVIFILEINFO_HASINDEX)
			DPF("AVIFILEINFO_HASINDEX");
//The AVI file has an index at the end of the file. For good performance, all AVI
//files should contain an index.
	if (pfi.dwFlags & AVIFILEINFO_MUSTUSEINDEX)
			DPF("AVIFILEINFO_MUSTUSEINDEX");
//The file index contains the playback order for the chunks in the file. Use the
//index rather than the physical ordering of the chunks when playing back the
//data. This could be used for creating a list of frames for editing.
	if (pfi.dwFlags & AVIFILEINFO_ISINTERLEAVED)
			DPF("AVIFILEINFO_ISINTERLEAVED");
//The AVI file is interleaved.
		if (pfi.dwFlags & AVIFILEINFO_WASCAPTUREFILE)
				DPF("AVIFILEINFO_WASCAPTUREFILE");
//The AVI file is a specially allocated file used for capturing real-time video.
//Applications should warn the user before writing over a file with this flag set
//because the user probably defragmented this file.
		if (pfi.dwFlags & AVIFILEINFO_COPYRIGHTED)
				DPF("AVIFILEINFO_COPYRIGHTED");
//The AVI file contains copyrighted data and software. When this flag is used,
//software should not permit the data to be duplicated.
 
		if (pfi.dwCaps & AVIFILECAPS_CANREAD)
			DPF("AVIFILECAPS_CANREAD");
//An application can open the AVI file with the read privilege.
		if (pfi.dwCaps & AVIFILECAPS_CANWRITE)
			DPF("AVIFILECAPS_CANWRITE");
//An application can open the AVI file with the write privilege.
		if (pfi.dwCaps & AVIFILECAPS_ALLKEYFRAMES)
			DPF("AVIFILECAPS_ALLKEYFRAMES");
//Every frame in the AVI file is a key frame.
		if (pfi.dwCaps & AVIFILECAPS_NOCOMPRESSION)
				DPF("AVIFILECAPS_NOCOMPRESSION");
//The AVI file does not use a compression method.
 #endif



DPF("w:%ld,h:%ld",pfi.dwWidth,pfi.dwHeight);
DPF("scale:%d, rate:%d,,length:%d",pfi.dwScale, pfi.dwRate,pfi.dwLength);
DPF("type:%s|",pfi.szFileType);
		AVIFileRelease( pfile );
		}
	m_bAudio = Option(REC_CAP_AUD);
	m_bPreview = Option(REC_PREVIEW);
	m_TimeLimit = Option(REC_TIME);
	m_bErase = Option(REC_ERASE);
	m_Delay = Option(REC_DELAY);
	m_nCapSize = Option(REC_SIZE);
}

void CRecordDlg::StartCapture()
{
	m_pMyMCI->Close();
	FBVideoRecordCapture(1);
}
/*
void CRecordDlg::StopCapture()
{
//	FBVideoRecordCapture(0);
	FBVideoRecordCopy(m_szPlayFile);
	m_pMyMCI->Open(m_szPlayFile);
	FBVideoRecordPreview(1);
}
*/
void CRecordDlg::OnRecord()
{
	DPF("test dlg On record");
	m_pMyMCI->Pause();
	m_pMyMCI->Close();
	m_state = 3;
	CRecordingDlg dlg;
	dlg.m_timer1 = m_Delay;
	dlg.m_timer2 = m_TimeLimit;
	int res = dlg.DoModal();
	if (!dlg.m_state)
		{
		FBVideoRecordPreview(1);
		m_state = 0;
		}
	else
		{
		FBVideoRecordCopy(m_szPlayFile);
		m_state = 1;
		m_pMyMCI->Open(m_szPlayFile);
		CheckPath();
		m_pMyMCI->Pause();
		if (m_bCanPlay)
			SetupSlider();
		}
}

void CRecordDlg::SetWindowSize()
{
	m_pMyMCI->SetSize(m_width, m_height);
}


void CRecordDlg::CheckPath()
{
	m_bCanPlay = m_pMyMCI->CanPlay();
	GetDlgItem(IDC_SLD_PLAY)->EnableWindow(m_bCanPlay);
	GetDlgItem(IDC_SLD_FRAMES)->ShowWindow(m_bCanPlay ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_SLD_FRAME)->ShowWindow(m_bCanPlay ? SW_SHOW : SW_HIDE);
}

void CRecordDlg::PlaySetup()
{
	HWND hWnd = m_pPlayWnd->m_hWnd;
	int z = m_pMyMCI->Create(hWnd, 0);//m_szCaptureFile);
	m_pPlayWnd->m_MCI = m_pMyMCI->ThisWnd();
//	m_pMyMCI->Open(m_szTempFile);
//	CheckPath();
//	m_pMyMCI->Close();
	GetDlgItem(IDC_SLD_SLIDER)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SLD_FRAME)->ShowWindow(SW_HIDE);
	SetWindowSize();
	SetupSlider();
}

void CRecordDlg::OnStop()
{
	ASSERT(m_state);
	m_pVideo->ShowWindow(SW_SHOW);
	m_pPlayWnd->ShowWindow(SW_HIDE);
	m_pMyMCI->Pause();
	m_pMyMCI->Close();
	if (m_bPreview)
		FBVideoRecordPreview(1);
	m_state = 0;
	GetDlgItem(IDC_SLD_SLIDER)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SLD_FRAME)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_SLD_PLAY)->SetWindowText("Play");
	GetDlgItem(IDC_SLD_STOP)->EnableWindow(FALSE);
	GetDlgItem(IDC_SLD_SLIDER)->EnableWindow(FALSE);
}

void CRecordDlg::SetupSlider()
{
	m_frames = m_pMyMCI->Length();
	m_frame = 0;
	m_slider.SetRange( 0, m_frames, TRUE );
	CString tmp;
	tmp.Format("Frames : %d", m_frames);
	GetDlgItem(IDC_SLD_FRAMES)->SetWindowText(tmp.GetBuffer());
	UpdateSlider();
}

void CRecordDlg::UpdateSlider()
{
	CString tmp;
	tmp.Format("Frame : %d", m_frame+1);
	GetDlgItem(IDC_SLD_FRAME)->SetWindowText(tmp.GetBuffer());
}

void CRecordDlg::DoIt(int cmd)
{
	if (!m_state)
		{
		FBVideoRecordPreview(0);
		m_pVideo->ShowWindow(SW_HIDE);
		m_pPlayWnd->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_SLD_STOP)->EnableWindow(TRUE);
		GetDlgItem(IDC_SLD_SLIDER)->EnableWindow(TRUE);
		GetDlgItem(IDC_SLD_SLIDER)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_SLD_FRAME)->ShowWindow(SW_SHOW);
		m_pMyMCI->Open(m_szPlayFile);
		CheckPath();
		if (m_bCanPlay)
			{
			SetWindowSize();
			SetupSlider();
			}
		else
			{
			}
		m_state = 1;
		}
	else if (m_state == 2)
		{
		PauseIt(0);
		}
	UINT old = m_frame;
	if ((cmd == 3) && m_frame)
		m_frame--;
	if ((cmd == 2) && ((m_frame + 1) < m_frames))
		m_frame++;
	if (cmd == 1)
		m_frame = m_frames - 1;
	if (cmd == 0)
		m_frame = 0;
	if (m_frame != old)
		{
		m_pMyMCI->Seek(m_frame);
		m_slider.SetPos(m_frame);
		UpdateSlider();
		}
}

void CRecordDlg::OnRepeat()
{
	m_bLoop = IsDlgButtonChecked(IDC_SLD_LOOP) ? 1 : 0;
}

void CRecordDlg::PauseIt(BOOL bRewind)
{
	ASSERT(m_state == 2);
	m_state = 1;
	GetDlgItem(IDC_SLD_PLAY)->SetWindowText("Play");
	m_pMyMCI->Pause();
	if (bRewind)
		m_pMyMCI->Seek(m_frame = 0);
	else	
		m_frame = m_pMyMCI->Where();
	m_slider.SetPos(m_frame);
	UpdateSlider();
	GetDlgItem(IDC_SLD_SLIDER)->EnableWindow(TRUE);
}

void CRecordDlg::OnPlay()
{
// if state == 2 then actually pause
/*
	CheckPath();
	if (!m_bCanPlay)
		{
		ASSERT(m_state == 0);
		return;
		}
*/
	if(!m_state)
		{
		FBVideoRecordPreview(0);
		m_pVideo->ShowWindow(SW_HIDE);
		m_pMyMCI->Open(m_szPlayFile);
		m_pPlayWnd->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_SLD_STOP)->EnableWindow(TRUE);
		SetupSlider();
		GetDlgItem(IDC_SLD_SLIDER)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_SLD_FRAME)->ShowWindow(SW_SHOW);
		}
	if (m_state == 2) // playing
		{
		PauseIt(0);
		}
	else
		{
		if (m_state == 1)
			m_pMyMCI->Resume();
		else
			m_pMyMCI->Play();
	//	m_pMyMCI->Repeat(1);
		m_Notified = 0;
		m_state = 2;
		GetDlgItem(IDC_SLD_PLAY)->SetWindowText("Pause");
		}
}

void CRecordDlg::OnHScroll( UINT nCode, UINT nPos, CScrollBar* pScrollBar )
{
	if (!m_state)
		return;
	if (m_state == 2)
		{
		m_state = 1;
		GetDlgItem(IDC_SLD_PLAY)->SetWindowText("Play");
		m_pMyMCI->Pause();
		}
	CSliderCtrl * pSlider = (CSliderCtrl *)pScrollBar;
	int min, max;
	pSlider->GetRange(min, max);
	m_frame = pSlider->GetPos();
	m_pMyMCI->Seek(m_frame);
	UpdateSlider();
}

CRecOptDlg::CRecOptDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CRecOptDlg::IDD, pParent)
{
	m_bPreview = 1;
	m_bAudio = 1;
	m_TimeLimit = 5;
}

BOOL CRecOptDlg::OnInitDialog()
{
	CMyDialog::OnInitDialog();
	CheckDlgButton(IDC_DO_VIDEO, 1);
	CheckDlgButton(IDC_PREVIEW, m_bPreview);
//	CheckDlgButton(IDC_USE_FRAMES, m_bUseFrames);
	GetDlgItem(IDC_DO_VIDEO)->EnableWindow(FALSE);
//	GetDlgItem(IDC_FRAME_RATE)->ShowWindow(m_nSetRate == 2 ? SW_SHOW : SW_HIDE);
//	CheckRadioButton(IDC_RATE_SCENE,IDC_USE_FRAMES,IDC_RATE_SCENE+m_nSetRate);
//	CheckRadioButton(IDC_MASTER_NONE,IDC_MASTER_AUDIO,IDC_MASTER_NONE+m_Master);
	FillCombos(8);
	UpdateButtons();
	return TRUE;
}

BEGIN_MESSAGE_MAP(CRecOptDlg, CMyDialog)
	ON_COMMAND(IDC_SAVE_SETTINGS, OnSaveSettings)
	ON_CBN_SELCHANGE(IDC_VID_DEVICES, OnSelChangeVidDev)
	ON_CBN_SELCHANGE(IDC_AUD_DEVICES, OnSelChangeAudDev)
	ON_CBN_SELCHANGE(IDC_COMPRESSORS, OnSelChangeCompressor)
	ON_CBN_SELCHANGE(IDC_VID_DIALOGS, OnSelChangeVidDlg)
	ON_CBN_SELCHANGE(IDC_AUD_DIALOGS, OnSelChangeAudDlg)
//	ON_COMMAND(IDC_USE_FRAMES, OnRateSet)
//	ON_COMMAND(IDC_RATE_VIDEO, OnRateVideo)
//	ON_COMMAND(IDC_RATE_SCENE, OnRateScene)
	ON_COMMAND(IDC_PREVIEW, OnPreview)
//	ON_COMMAND(IDC_DO_VIDEO, OnVideo)
	ON_COMMAND(IDC_DO_AUDIO, OnAudio)
	ON_COMMAND(IDC_CAP_EXPORT, OnExport)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CRecOptDlg::OnSelChangeVidDev()
{
	int dev = m_vid_devices.GetCurSel();
	FBVideoRecordSelectDevice (dev, 0);
	FillCombos(1);
//	UpdateDeviceInfo();
}
void CRecOptDlg::OnSelChangeAudDev()
{
	int dev = m_aud_devices.GetCurSel();
	FBVideoRecordSelectDevice (dev, 1);
	FillCombos(2);
//	UpdateDeviceInfo();
}

void CRecOptDlg::OnSelChangeCompressor()
{
	int dev = m_compressors.GetCurSel();
	FBVideoRecordSelectDevice (dev, 2);
	FillCombos(4);
//	UpdateDeviceInfo();
}
void CRecOptDlg::OnSelChangeVidDlg()
{
	int id = m_vid_dialogs.GetCurSel();
	FBVideoRecordOption (id, 0);
//	m_vid_dialogs.SetCurSel(-1); // turn off since it is a command
	UpdateDeviceInfo();
}
void CRecOptDlg::OnSelChangeAudDlg()
{
	int id = m_aud_dialogs.GetCurSel();
//	m_aud_dialogs.SetCurSel(-1); // turn off since it is a command
	FBVideoRecordOption (id, 1);
	UpdateDeviceInfo();
}

void CRecOptDlg::UpdateDeviceInfo()
{
	int index = FBVideoRecordCurrentDevice(0);
	FBVideoRecordDeviceName (m_szVidDevice,index, 0);
	index = FBVideoRecordCurrentDevice(1);
	FBVideoRecordDeviceName(m_szAudDevice,index,1);
	index = FBVideoRecordCurrentDevice(2);
	FBVideoRecordDeviceName(m_szAudDevice,index,2);
}


void CRecOptDlg::DoDataExchange(CDataExchange* pDX)	// DDX/DDV support
{
	CMyDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_VID_DEVICES, m_vid_devices);
	DDX_Control(pDX, IDC_AUD_DEVICES, m_aud_devices);
	DDX_Control(pDX, IDC_VID_DIALOGS, m_vid_dialogs);
	DDX_Control(pDX, IDC_AUD_DIALOGS, m_aud_dialogs);
	DDX_Control(pDX, IDC_COMPRESSORS, m_compressors);
//	DDX_Check(pDX, IDC_PREVIEW, m_bPreview);
//	DDX_Check(pDX, IDC_DO_VIDEO, m_bVideo);
	DDX_Check(pDX, IDC_DO_AUDIO, m_bAudio);
	DDX_Check(pDX, IDC_ERASE_CAP_FILE, m_bErase);
	DDX_Text(pDX, IDC_TIME_LIMIT, m_TimeLimit);
	DDV_MinMaxUInt(pDX, m_TimeLimit, 5, 20);
	DDX_Text(pDX, IDC_REC_DELAY, m_Delay);
	DDV_MinMaxUInt(pDX, m_Delay, 0, 30);
	DDX_Text(pDX, IDC_REC_SIZE, m_nCapSize);
	DDV_MinMaxUInt(pDX, m_nCapSize, 0, 900);
}

void CRecOptDlg::FillCombos(UINT mask)
{
	int i;
	if (1)//m_bVideo)
		{
		if (mask & 8) // initdialog
			{
			m_vid_devices.ResetContent();
			int c = FBVideoRecordDeviceCount(0);
			int j = 0;
			for (i = 0; i < c; i++)
				{
				char dev[100];
				FBVideoRecordDeviceName (dev, i, 0);
				m_vid_devices.AddString(dev);
				if (!strcmp(dev,m_szVidDevice))
					j = i + 1;
				}
			if (j)
				{
				m_vid_devices.SetCurSel(j-1);
				FBVideoRecordSelectDevice (j-1, 0);
				}
			else
				m_vid_devices.SetCurSel(FBVideoRecordCurrentDevice(0));
			}
		if (mask & 9)
			{
			m_vid_dialogs.ResetContent();
			for (i = 0;i < 20 ; i++)
				{
				char opt[100];
				if (FBVideoRecordOption(i,0,opt))
					break;
				m_vid_dialogs.AddString(opt);
				}
			m_vid_dialogs.SetCurSel(0);//FBVideoRecordCurrentDevice(1));
			}
		}
	if (1)//m_bAudio)
		{
		if (mask & 8)
			{
			m_aud_devices.ResetContent();
			int c = FBVideoRecordDeviceCount(1);
			int j = 0;
			for (i = 0; i < c; i++)
				{
				char dev[100];
				FBVideoRecordDeviceName (dev, i, 1);
				m_aud_devices.AddString(dev);
				if (!strcmp(dev,m_szAudDevice))
					j = i+1;
				}
			if (j)
				{
				m_aud_devices.SetCurSel(j-1);
				FBVideoRecordSelectDevice (j-1, 1);
				}
			else
				m_aud_devices.SetCurSel(FBVideoRecordCurrentDevice(1));
			}
		if (mask & 10)
			{
			m_aud_dialogs.ResetContent();
			for (i = 0;i < 20 ; i++)
				{
				char opt[100];
				if (FBVideoRecordOption(i,1,opt))
					break;
				m_aud_dialogs.AddString(opt);
				}
			m_aud_dialogs.SetCurSel(0);
			}
		}
	if (1)//compressors)
		{
		if (mask & 8)
			{
			m_compressors.ResetContent();
			int c = FBVideoRecordDeviceCount(2);
			int j = 0;
			for (i = 0; i < c; i++)
				{
				char dev[100];
				FBVideoRecordDeviceName (dev, i, 2);
				m_compressors.AddString(dev);
				if (!strcmp(dev,m_szCompressor))
					j = i+1;
				}
			if (j)
				{
				m_compressors.SetCurSel(j-1);
				FBVideoRecordSelectDevice (j-1, 2);
				}
			else	
				m_compressors.SetCurSel(FBVideoRecordCurrentDevice(2));
			}
		}
	UpdateDeviceInfo();
}

//void CRecOptDlg::DoRate(int v)
//{
///	m_nSetRate = v;
//	GetDlgItem(IDC_FRAME_RATE)->ShowWindow(v == 2 ? SW_SHOW : SW_HIDE);
//}

void CRecOptDlg::OnPreview()
{
	m_bPreview ^= 1;
	FBVideoRecordPreview(m_bPreview);
}

void CRecOptDlg::OnAudio()
{
	m_bAudio ^= 1;
	FBVideoRecordDoAudio(m_bAudio);
	UpdateButtons();
}

void CRecOptDlg::OnExport()
{
// file dialog
	CString name;
	m_pMyParent->m_pDoc->GetDefFolder(name,UDF_EXP,1);
	name += ".AVI";
	CSketchApp* pApp = (CSketchApp *)AfxGetApp();
	if (pApp->PromptFileName(name, 2))
		{
		m_pMyParent->m_pDoc->UpdateDefFolder(name,UDF_EXP);
		FBVideoRecordCopy((LPCSTR)name);
		}
}

void CRecOptDlg::OnSaveSettings()
{
	UpdateData(1);
	m_pMyParent->SaveSettings(this);
}

void CRecOptDlg::UpdateButtons()
{
//	GetDlgItem(IDC_VID_DEVICES)->ShowWindow( m_bVideo ? SW_SHOW : SW_HIDE);
//	GetDlgItem(IDC_VID_DIALOGS)->ShowWindow( m_bVideo ? SW_SHOW : SW_HIDE);
//	GetDlgItem(IDC_VID_DEV_TXT)->ShowWindow( m_bVideo ? SW_SHOW : SW_HIDE);
//	GetDlgItem(IDC_VID_DLG_TXT)->ShowWindow( m_bVideo ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_AUD_DEVICES)->ShowWindow( m_bAudio ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_AUD_DIALOGS)->ShowWindow( m_bAudio ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_AUD_DEV_TXT)->ShowWindow( m_bAudio ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_AUD_DLG_TXT)->ShowWindow( m_bAudio ? SW_SHOW : SW_HIDE);
}


CRecordingDlg::CRecordingDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CRecordingDlg::IDD, pParent)
{
}

BEGIN_MESSAGE_MAP(CRecordingDlg, CMyDialog)
	ON_WM_TIMER()
	ON_COMMAND(IDC_NOW, OnNow)
END_MESSAGE_MAP()

BOOL CRecordingDlg::OnInitDialog()
{
	char msg[80];
	CMyDialog::OnInitDialog();
	m_timer = m_timer1;
	m_state = 0;
	if (m_timer)
		{
		SetWindowText("Waiting to Record");
		GetDlgItem(IDC_NOW)->SetWindowText("Start Now");
		sprintf_s(msg, "Recording Will Start in %d Secords", m_timer);
		GetDlgItem(IDC_STATUS)->SetWindowText(msg);
		}
	else
		{
		PostMessage(WM_COMMAND, IDC_NOW);
		}
	VERIFY(SetTimer(TIMER_ID, 1000, NULL) == TIMER_ID);
	return TRUE;
}

void CRecordingDlg::OnTimer(UINT nTimerID)
{
	char msg[80];
	if ((nTimerID == TIMER_ID) && (m_state < 2))
		{
		if(m_timer-- == 1)
			PostMessage(WM_COMMAND, IDC_NOW);
		else
			{
			if (m_state == 1)
				sprintf_s(msg, "Recording Will Stop in %d Secords", m_timer);
			else
				sprintf_s(msg, "Recording Will Start in %d Secords", m_timer);
			GetDlgItem(IDC_STATUS)->SetWindowText(msg);
			if (m_state && FBVideoRecordStatus(msg, sizeof(msg)))
				{
				GetDlgItem(IDC_MSG)->SetWindowText(msg);
				}
			}
		}
}

void CRecordingDlg::OnNow()
{
	if (m_state == 0)
		{
		m_state = 1;
		SetWindowText("Recording");
		GetDlgItem(IDC_NOW)->SetWindowText("Stop Now");
		m_timer = m_timer2;
		FBVideoRecordCapture(1);
		}
	else
		{
		m_state = 2;
		FBVideoRecordCapture(0);
		KillTimer(TIMER_ID);
		SetWindowText("Recording Finished");
		GetDlgItem(IDC_STATUS)->SetWindowText("                  ");
		GetDlgItem(IDC_NOW)->ShowWindow(SW_HIDE);
		GetDlgItem(IDCANCEL)->SetWindowText("Exit");
		char msg[80];
		if (FBVideoRecordStatus(msg, sizeof(msg),1))
			GetDlgItem(IDC_MSG)->SetWindowText(msg);
		}
}

void CRecordingDlg::OnCancel()
{
	DPF("got cancel");
	if (m_state == 1)
		OnNow();
	else
		EndDialog(0);
}

void CRecordingDlg::OnOK()
{
	DPF("got ok");
}


void CRecordingDlg::OnClose()
{
	DPF("on close");
	if (m_state == 1)
		OnNow();
	else
		CDialog::OnClose();
}

void CSketchView::OnRecord()
{
	CRecordDlg dlg(this);
	dlg.m_pView = this;
	dlg.m_pDoc = m_pDoc;
	if (dlg.DoModal() == IDC_IMPORT)
		{
		LPSTR p = dlg.m_szCaptureFile;
		m_pDoc->ImportAVIFile(p);
		}
	if (dlg.m_bErase)
		{
		RemoveFile(dlg.m_szCaptureFile);
		RemoveFile(dlg.m_szPlayFile);
		}
}

