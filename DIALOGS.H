
#if !defined(DIALOGS_H)

#define DIALOGS_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSketchView;
class CSketchDoc;
class CPaletteDlg;
#include "mydialog.h"
#include "vfw.h"
#include "clevtbl.h"
class CNewPals;
class CCamMoves;

void FileName(CString& name, LPCSTR path);

class CPublishDlg : public CMyDialog
{
// Construction
public:
	CPublishDlg(CWnd* pParent = NULL);   // standard constructor
	WORD CodecOptionsDlg();
	void CodecName();
// Dialog Data
	//{{AFX_DATA(CAviDlg)
	enum { IDD = IDD_PUBLISH };
	int	m_nKind; // 0 is avi, 1 is mov,2 is gif
	CSketchDoc * m_pDoc;
	UINT	m_StartFrame;
	UINT	m_EndFrame;
	UINT	m_MaxFrame;
	UINT	m_StartLevel;
	UINT	m_EndLevel;
	UINT	m_MaxLevel;
	BOOL	m_bKeep;
	BOOL	m_bEnableSound;
	BOOL	m_bUseSound;
	int		m_nTelecine;
	BOOL	m_bAllowTelecine;
	
	UINT	m_rate;
	UINT	m_width;
	UINT	m_height;
	BOOL	m_bKludge;
	BOOL	m_bGray;	// gif stuff
	int	m_color;		// ,..
	BOOL	m_bLoop;	// ..
	UINT	m_origwidth;
	UINT	m_origheight;
	BOOL	m_bAspect;
	BOOL	m_bWidth;
	AVICOMPRESSOPTIONS m_opts;
	CString m_media;
	CString m_codec;
	CString m_encoder;
	UINT	m_f1;
	UINT	m_f2;
	UINT	m_l1;
	UINT	m_l2;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void	OnKindChange();
	// Generated message map functions
	//{{AFX_MSG(CNewDlg)
    virtual BOOL OnInitDialog();
	afx_msg void OnHeightChange() ;
	afx_msg void OnWidthChange() ;
	afx_msg void OnRateChange() ;
	afx_msg void OnCodecChange() ;
	afx_msg void OnSaveSettings() ;
	afx_msg void OnSelectAll() ;
	afx_msg void OnAspect() ;
	afx_msg void OnFLVChange() ;
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


class CExpDlg : public CMyDialog
{
// Construction
public:
	CExpDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CExpDlg)
	enum { IDD = IDD_EXPORT };
	UINT	m_StartFrame;
	UINT	m_EndFrame;
	UINT	m_MaxFrame;
	UINT	m_StartLevel;
	UINT	m_EndLevel;
	UINT	m_MaxLevel;
	int		m_Type;
	int		m_Kind;
	int		m_Flags;
	UINT	m_width;
	UINT	m_height;
	UINT	m_origwidth;
	UINT	m_origheight;
	UINT	m_fact;
	BOOL	m_bKludge;
	BOOL	m_bAspect;
	BOOL	m_bWidth;
	BOOL	m_bScaleBG;
	CString m_prefix;
	CComboBox	m_kinds;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExpDlg)
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	int AddToCombo(LPCSTR txt, UINT kind, UINT v);
	// Generated message map functions
	//{{AFX_MSG(CNewDlg)
    virtual BOOL OnInitDialog();
	afx_msg void OnHeightChange() ;
	afx_msg void OnWidthChange() ;
	afx_msg void OnTypeChange() ;
	afx_msg void OnSelChangeKind();
	afx_msg void OnAspect() ;
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CLibSaveDlg : public CMyDialog
{
// Construction
public:
	CLibSaveDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CExpDlg)
	enum { IDD = IDD_LIB_SAVE };
	UINT	m_StartFrame;
	UINT	m_EndFrame;
	UINT	m_MaxFrame;
	UINT	m_StartLevel;
	UINT	m_EndLevel;
	UINT	m_MaxLevel;
	BOOL	m_bAspect;
	BOOL	m_bShowGrid;
	BOOL	m_bManipulate;
	UINT	m_nColumns;
	BOOL	m_bFreeform;
	BOOL 	m_bMatchColor;
	BOOL 	m_bApplyName;
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


class CExpCamDlg : public CMyDialog
{
// Construction
public:
	CExpCamDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CExpDlg)
	enum { IDD = IDD_EXPORT_CAM };
	UINT	m_StartFrame;
	UINT	m_EndFrame;
	UINT	m_MaxFrame;
	UINT	m_StartLevel;
	UINT	m_EndLevel;
	UINT	m_MaxLevel;
	UINT	m_Flags;
	BOOL	m_bDoGroups;
	BOOL	m_bCamX;
	BOOL	m_bCamY;
	BOOL	m_bCamZ;
	BOOL	m_bCamR;
	BOOL	m_bCamB;
	BOOL	m_bCamA;
	BOOL	m_bCamComputed;
	BOOL	m_bActiveOnly;
	CString m_name;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExpDlg)
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	void CheckEm(BOOL v);
// Implementation
protected:
	// Generated message map functions
    virtual BOOL OnInitDialog();
	afx_msg void OnAll() {CheckEm(1);};
	afx_msg void OnNone() {CheckEm(0);};
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CImpCamDlg : public CMyDialog
{
// Construction
public:
	CImpCamDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CExpDlg)
	enum { IDD = IDD_IMPORT_CAM };
	UINT	m_StartFrame;
	UINT	m_EndFrame;
	UINT	m_MaxFrame;
	UINT	m_StartLevel;
	UINT	m_EndLevel;
	UINT	m_MaxLevel;
	UINT	m_Flags;
	UINT	m_nPeg;
	BOOL	m_bCamX;
	BOOL	m_bCamY;
	BOOL	m_bCamZ;
	BOOL	m_bCamR;
	BOOL	m_bCamB;
	BOOL	m_bCamA;
	BOOL	m_bHasGroup;
	BOOL	m_bCamComputed;
	BOOL	m_bCamGroupInfo;
	CCamMoves * m_pMoves;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExpDlg)
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	void HideEm();
	void CheckEm(BOOL v);
// Implementation
protected:
	CListBox m_list;
	// Generated message map functions
    virtual BOOL OnInitDialog();
	afx_msg void OnAll() {CheckEm(1);};
	afx_msg void OnNone() {CheckEm(0);};
	afx_msg void OnSelChange();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


class CExtEditDlg : public CMyDialog
{
// Construction
public:
	CExtEditDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CExpDlg)
	enum { IDD = IDD_EXTEDIT };
	UINT	m_StartFrame;
//	UINT	m_EndFrame;
	UINT	m_MaxFrame;
	UINT	m_StartLevel;
//	UINT	m_EndLevel;
	UINT	m_MaxLevel;
//	BOOL	m_bColor;
	BOOL	m_b24;
	BOOL	m_bKeying;
	UINT	m_nAlpha;
	BOOL	m_bNotAgain;
//	int		m_Type;
//	int		m_Kind;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	// Generated message map functions
    virtual BOOL OnInitDialog();
	afx_msg void OnKeying();
	afx_msg void OnEdit24();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CCamera;
class CScene;
class CLevelDlg : public CMyDialog
{
// Construction
public:
	CLevelDlg(UINT idd, CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CLevelDlg)
//	enum { IDD = IDD_LEVEL };
	BOOL	m_bPro;
	UINT	m_level;
	UINT	m_peg;
	UINT	m_index;
	BOOL	m_bUseLevel;
	BOOL	m_bUseModel;
	CCamera * m_pCamera;
	CScene *  m_pScene;
	CString	m_name;
	CString	m_mdlname;
	CString	m_palname;
	CComboBox	m_pals;
	CComboBox	m_pegs;
	//}}AFX_DATA
// Implementation
protected:
	void FillPalCombo();
	void Update();
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();
	// Generated message map functions
	//{{AFX_MSG(CLevelDlg)
//	afx_msg void OnImport();
	afx_msg void OnChangeMdl();
//	afx_msg void OnExport();
	afx_msg void OnModelUse();
	afx_msg void OnSelChangePeg();
	afx_msg void OnSelChangePal();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

class CGroupDlg : public CMyDialog
{
// Construction
public:
	CGroupDlg(CWnd* pParent = NULL);	// standard constructor
	UINT m_group;
// Dialog Data
	//{{AFX_DATA(CGroupDlg)
	enum { IDD = IDD_GROUP };
	CCamera * m_pCamera;
	CString	m_name;
	//}}AFX_DATA
// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();
	// Generated message map functions
	//{{AFX_MSG(CLevelDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CPegDlg : public CMyDialog
{
// Construction
public:
	CPegDlg(CWnd* pParent = NULL);	// standard constructor
	UINT m_peg;
// Dialog Data
	//{{AFX_DATA(CPegDlg)
	enum { IDD = IDD_PEG };
	UINT	m_group;
	CCamera * m_pCamera;
	CString	m_name;
	CComboBox	m_groups;
	int		m_nFieldComp;
	BOOL	m_bAllowFieldComp;
	//}}AFX_DATA
// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();
	// Generated message map functions
	//{{AFX_MSG(CLevelDlg)
	afx_msg void OnSelChangeGroup();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};


class CCellDlg : public CMyDialog
{
// Construction
public:
	CCellDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CCellDlg)
	enum { IDD = IDD_CELL };
	CString	m_name;
	//}}AFX_DATA
// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	// Generated message map functions
	//{{AFX_MSG(CCellDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


class CSoundTrackDlg : public CMyDialog
{
// Construction
public:
	CSoundTrackDlg(CWnd* pParent = NULL);	// standard constructor
	enum { IDD = IDD_SNDTRACK_DLG};
	int		m_nTrack;
	int		m_max;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
};

class CMySound;
class CSoundDlg : public CMyDialog
{
// Construction
public:
	CSoundDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CSoundDlg)
	enum { IDD = IDD_SOUND };
	CString	m_name;
	BOOL	m_bUseSound;
	BOOL	m_bChanged;
	UINT m_frames;
	UINT m_vmark;
	int		m_nVolume;
	double	m_smark;
	UINT	m_snip;
	int m_id;
	CMySound * m_pSound;
	//}}AFX_DATA
// Implementation
protected:
	BOOL	m_bQT;
	CSliderCtrl m_slider;
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
    virtual BOOL OnInitDialog();
	// Generated message map functions
	//{{AFX_MSG(CSoundDlg)
	afx_msg void OnVScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar );
	afx_msg void OnChange();
//	afx_msg void OnInternal();
//	afx_msg BOOL OnToolTip(UINT id, NMHDR* pTTTStruct, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


class CFrameDlg : public CMyDialog
{
public:
	CFrameDlg(CWnd* pParent = NULL);	// standard constructor

	//{{AFX_DATA(CFrameDlg)
	enum { IDD = IDD_FRAME };
	//}}AFX_DATA
// Implementation
	UINT m_frame;
	UINT m_count;
	UINT m_max_del;
	UINT m_max_add;
protected:
	UINT m_max;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

protected:
	// Generated message map functions
	//{{AFX_MSG(CFrameDlg)
	afx_msg void OnInsert();// { if( UpdateData() EndDialog(100);};
	afx_msg void OnDelete();// { EndDialog(101);};
	afx_msg void OnAppend();// { EndDialog(101);};
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CYesRestDlg : public CMyDialog
{
// Construction
public:
	CYesRestDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CYesRestDlg)
	enum { IDD = IDD_CELLOVER };
	UINT	m_Frame;
	UINT	m_Level;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CYesRestDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
    virtual BOOL OnInitDialog();

	// Generated message map functions
	//{{AFX_MSG(CYesRestDlg)
	afx_msg void OnYes();
	afx_msg void OnYesRest();
	afx_msg void OnNo();
	afx_msg void OnNoRest();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CReplaceDlg : public CMyDialog
{
// Construction
public:
	CReplaceDlg(UINT idd, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CReplaceDlg)
//	enum { IDD = IDD_REPLACE };
	UINT	m_nKind;
	UINT	m_StartFrame;
	UINT	m_StartLevel;
	UINT	m_EndFrame;
	UINT	m_EndLevel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReplaceDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};



class CKeyTypeDlg : public CMyDialog
{
// Construction
public:
	CKeyTypeDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CKeyTypeDlg)
	enum { IDD = IDD_KEY_TYPE };
	UINT	m_Id;
	UINT	m_Frame;
	UINT	m_Frames;
	UINT	m_Kind; // 0 is smooth, 1 is linear
	UINT	m_EaseIn;
	UINT	m_EaseOut;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKeyTypeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void SetKind(UINT kind);
	void UpdateControls();
	// Generated message map functions
	//{{AFX_MSG(CKeyTypeDlg)
    virtual BOOL OnInitDialog();
//	afx_msg void OnHeightChange() ;
	afx_msg void OnSmooth() { SetKind(1) ;};
	afx_msg void OnLinear() { SetKind(0) ;};
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


class CLogDlg : public CMyDialog
{
// Construction
public:
	CLogDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CLogDlg)
	enum { IDD = IDD_VIEWLOG };
	//}}AFX_DATA
	CString m_title;
	CFont	m_font;
	char * m_pData;
	virtual BOOL OnInitDialog();
};


class CPrintDlg : public CMyDialog
{
// Construction
public:
	CPrintDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CLogDlg)
	enum { IDD = IDD_PRINTING };
	//}}AFX_DATA
	int m_nKind;		// cel, frame, xsheet 
	int m_nFit;		// 0:fit to page, 1:fit width, 2:fit height
	int m_nLCount;
	int m_nFCount;
	BOOL m_bDown;		// over, down
	BOOL m_bMarks;
	void MySetup();
	void MyExit(int code);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//{{AFX_MSG(CPrintDlg)
	afx_msg void OnPrint() { MyExit(5);};
	afx_msg void OnPreview() { MyExit(6);};
	afx_msg void OnSetup() { MyExit(7);};
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CLevelTable;

class CLayerDlg : public CMyDialog
{
// Construction
public:
	CLayerDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CKeyTypeDlg)
	enum { IDD = IDD_LAYER };
	//}}AFX_DATA
	CSketchView * m_pView;
	CLevelTable * m_pTable;
protected:
	CListBox m_list;
	int m_nSelect;
	int	m_nCurrent;
	void UpdateFields(BOOL bPut= 0);
	void UpdateList(int index, BOOL bAppend = 0);
	CString m_name;
	BOOL m_bDisplay;
	BOOL m_bActive;
	BOOL m_bWithin;
	int m_nBlurType;
	int m_dx;
	int m_dy;
	int m_blur;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKeyTypeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CKeyTypeDlg)
    virtual BOOL OnInitDialog();
	afx_msg void OnSelChange();
//	afx_msg void OnRadioChange();
	afx_msg void OnApply();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


class CProgressDlg : public CMyDialog
{
public:
	CProgressDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CKeyTypeDlg)
	enum { IDD = IDD_PROGRESS };
	//}}AFX_DATA
	UINT m_kind;
	UINT m_count;
	UINT m_index1;
	UINT m_index2;
	UINT m_index3;
	void * m_pData;
	void * m_pDoc;
	UINT m_error;
protected:
	UINT m_idx;
	UINT m_prevtime;
	UINT m_starttime;
	UINT	m_state;
	BOOL	m_bAbort;
	CProgressCtrl   m_Progress;
	void UpdateInfo();
	void CheckAbort();
	virtual void OnOK();
	virtual void OnCancel();
    virtual BOOL OnInitDialog();
	afx_msg void OnPerform();
	afx_msg void OnResume();
	afx_msg void OnExit();
	afx_msg void OnClose();
	DECLARE_MESSAGE_MAP()
};

class CProgDlg : public CMyDialog
{
public:
	CProgDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CKeyTypeDlg)
	enum { IDD = IDD_PROGRESS };
	//}}AFX_DATA
	UINT m_kind;
	UINT m_count;
	UINT m_base;
	BYTE * m_pData;
	CSketchDoc * m_pDoc;
	UINT m_error;
protected:
	UINT m_idx;
	UINT m_prevtime;
	UINT m_starttime;
	UINT	m_state;
	BOOL	m_bAbort;
	CProgressCtrl   m_Progress;
	void UpdateInfo();
	void CheckAbort();
	virtual void OnOK();
	virtual void OnCancel();
    virtual BOOL OnInitDialog();
	afx_msg void OnPerform();
	afx_msg void OnResume();
	afx_msg void OnExit();
	afx_msg void OnClose();
	DECLARE_MESSAGE_MAP()
};


typedef unsigned int (*ProgressCallback)(unsigned int index,
						void *UserObject, void *UserData );
class CProg2Dlg : public CMyDialog
{
public:
	CProg2Dlg(CWnd* pParent = NULL);   // standard constructor
	int ProgSetup(ProgressCallback inFunction,
				void* UserObject, void * UserData,
				unsigned int first, unsigned int count, int idstr = 0);

// Dialog Data
	//{{AFX_DATA(CKeyTypeDlg)
	enum { IDD = IDD_PROGRESS };
	//}}AFX_DATA
	void Msg(LPCSTR txt);
	void Label(LPCSTR txt);
protected:
	UINT m_id_string;
	UINT m_count;
	UINT m_index;
	UINT m_first;
	ProgressCallback m_callback;
	BYTE * m_pUser;
	BYTE * m_pData;
	UINT m_error;
	UINT m_prevtime;
	UINT m_starttime;
	UINT m_state;
	BOOL m_bAbort;
	CProgressCtrl   m_Progress;
	void UpdateInfo();
	void CheckAbort();
	virtual void OnOK();
	virtual void OnCancel();
    virtual BOOL OnInitDialog();
	afx_msg void OnPerform();
	afx_msg void OnResume();
	afx_msg void OnExit();
	afx_msg void OnClose();
	DECLARE_MESSAGE_MAP()
};

typedef int (*ProgPollCallback)(int code, void *userdata );
class CProgPullDlg : public CDialog
{
public:
	CProgPullDlg(CWnd* pParent = NULL);   // standard constructor
	enum { IDD = IDD_PROGRESS };
	void SetCallback( ProgPollCallback pCB, void * pClass)
				{ m_pCallback = pCB; m_pClass = pClass;};
	//}}AFX_DATA
	void SetRange(UINT first, UINT last)
			{ m_first = first;m_last = last;};
protected:
	UINT m_first;
	UINT m_last;
	UINT m_factor;
	BOOL m_bAbort;
	UINT m_state;
	UINT m_index;
	UINT m_prevtime;
	UINT m_starttime;
	ProgPollCallback m_pCallback;	
	void * m_pClass;
	CProgressCtrl   m_Progress;
	 
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnPerform();
	afx_msg void OnExit();
	afx_msg void OnClose();
	virtual void OnCancel();
};


class CExternalDlg : public CMyDialog
{
// Construction
public:
	CExternalDlg(CWnd* pParent = NULL);   // standard constructor
	enum { IDD = IDD_EXTERNAL };
	char 	m_name[300];
	BOOL CheckWindow(HWND hwnd);
protected:
	UINT 	m_nState;
	HWND	m_hWnd;
	BOOL	FindWindow();
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nTimerID);
	DECLARE_MESSAGE_MAP()
};

class CExistOneDlg : public CMyDialog
{
public:
	CExistOneDlg(CWnd* pParent = NULL);   // standard constructor
	enum { IDD = IDD_EXIST_SINGLE};
	UINT		m_Frame;
	UINT		m_Level;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnYesRest();
	DECLARE_MESSAGE_MAP()
};

class CDemoDlg : public CMyDialog
{
public:
	CDemoDlg(CWnd* pParent = NULL);   // standard constructor
	CString m_id;
#ifdef _THEDISC
	enum { IDD = IDD_DEMO_DISC};
#else
	enum { IDD = IDD_DEMO};
#endif
	DECLARE_MESSAGE_MAP()
protected:
	virtual BOOL OnInitDialog();
	afx_msg void OnBuyIt();
	afx_msg void OnTryIt();
};

int MyDialog(int idd);

class CPalSelDlg : public CMyDialog
{
// Construction
public:
	CPalSelDlg(CWnd* pParent = NULL);	// standard constructor
	enum { IDD = IDD_SEL_PAL };
	UINT	m_index;
	CScene * m_pScene;
protected:
	CListBox	m_pals;
	void FillListBox();
	void Update();
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangePal();
	DECLARE_MESSAGE_MAP()
};

class CPalSharedDlg : public CMyDialog
{
// Construction
public:
	CPalSharedDlg(CWnd* pParent = NULL);	// standard constructor
	enum { IDD = IDD_PAL_SHARED };
	UINT	m_index;
	CScene * m_pScene;
	BOOL m_bModOne;
	CString m_name;
protected:
	void FillListBox();
	void Update();
	void DoMod(int v);
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnModAll() {DoMod(0);};
	afx_msg void OnModOne() {DoMod(1);};
	
	DECLARE_MESSAGE_MAP()
};

class CPalLinkedDlg : public CMyDialog
{
// Construction
public:
	CPalLinkedDlg(CWnd* pParent = NULL);	// standard constructor
	enum { IDD = IDD_PAL_LINKED };
	CString m_name;
	CString m_fname;
	BOOL	m_bBreak;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();
	void DoBreak(int v);
	afx_msg void OnBreak() {DoBreak(1);};
	afx_msg void OnUpdate() {DoBreak(0);};
	DECLARE_MESSAGE_MAP()
};

class CPalDifferentDlg : public CMyDialog
{
// Construction
public:
	CPalDifferentDlg(CWnd* pParent = NULL);	// standard constructor
	enum { IDD = IDD_PAL_DIFF };
	CString m_name;
	CString m_fname;
	BOOL	m_bBreak;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();
	void DoBreak(int v);
	afx_msg void OnBreak() {DoBreak(1);};
	afx_msg void OnUpdate() {DoBreak(0);};
	DECLARE_MESSAGE_MAP()
};

class CPalMissingDlg : public CMyDialog
{
// Construction
public:
	CPalMissingDlg(CWnd* pParent = NULL);	// standard constructor
	enum { IDD = IDD_PAL_MISSING };
	CString m_name;
	CString m_fname;
	BOOL	m_bBreak;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();
	void DoBreak(int v);
	afx_msg void OnBreak() {DoBreak(1);};
	afx_msg void OnLocate() {DoBreak(0);};
	DECLARE_MESSAGE_MAP()
};

class CPalModDlg : public CMyDialog
{
// Construction
public:
	CPalModDlg(CWnd* pParent = NULL);	// standard constructor
	enum { IDD = IDD_PAL_EXT_MOD };
	CString m_name;
	CString m_fname;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnNo();
	
	DECLARE_MESSAGE_MAP()
};

class CPalUnusedDlg : public CMyDialog
{
// Construction
public:
	CPalUnusedDlg(CWnd* pParent = NULL);	// standard constructor
	enum { IDD = IDD_PAL_UNUSED};
	CString m_name;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnNo();
	
	DECLARE_MESSAGE_MAP()
};

class CMissingTextureDlg : public CMyDialog
{
// Construction
public:
	CMissingTextureDlg(CWnd* pParent = NULL);	// standard constructor
	enum { IDD = IDD_PAL_NO_TEXTURE};
	CString m_fname;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnLocate();
	
	DECLARE_MESSAGE_MAP()
};

class CEmbOptionsDlg : public CMyDialog
{
public:
	CEmbOptionsDlg(CWnd* pParent = NULL);	// standard constructor
	enum { IDD = IDD_EMBED_OPT};
	BOOL m_bOverride;
	int m_in;
	int m_out;
protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
};

class CMyIO;
class CEmbedInDlg : public CMyDialog
{
public:
	CEmbedInDlg(CWnd* pParent = NULL);	// standard constructor
	enum { IDD = IDD_EMBED_OPEN };
	CCheckListBox m_list;
	CMyIO * m_pIO;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
    virtual BOOL OnInitDialog();
	virtual void OnOK();
};

class CEmbedOutDlg : public CMyDialog
{
public:
	CEmbedOutDlg(CWnd* pParent = NULL);	// standard constructor
	enum { IDD = IDD_EMBED_SAVE };
	CCheckListBox m_list;
	CMyIO * m_pIO;
	CScene * m_pScene;
protected:
	void AddName(LPCSTR name, UINT kind, UINT index);
	void CheckExtFile(LPCSTR name, UINT kind);
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
    virtual BOOL OnInitDialog();
	virtual void OnOK();
};

class CExpEmbedDlg : public CMyDialog
{
public:
	CExpEmbedDlg(CWnd* pParent = NULL);	// standard constructor
	enum { IDD = IDD_EXPORT_EMBED};
	CMyIO * m_pIO;
protected:
	CListBox m_list;
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
    virtual BOOL OnInitDialog();
	afx_msg void OnExport();
	DECLARE_MESSAGE_MAP()
};

class CFrameNumberDlg : public CMyDialog
{
public:
	CFrameNumberDlg(CWnd* pParent = NULL);	// standard constructor
	enum { IDD = IDD_FRAME_NUMBER };
	int m_corner;
	int m_start;
	BOOL m_bPlayback;
	BOOL m_bPublish;
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
};


#endif
