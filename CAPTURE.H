//
// capture.h : header file
//

#if !defined(CAPTURE_H_INCLUDED)
#define CAPTURE_H_INCLUDED
#include "mydialog.h"

#include "myspin.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMySlider : public CSliderCtrl
{
public:
	void Setup(CWnd * pBuddy, UINT offset, UINT pos, 
				UINT min, UINT max);
	void Assign(UINT V);
	void UpdateBuddy();
	UINT m_argoffset;
	CWnd * m_pBuddy;
};

/////////////////////////////////////////////////////////////////////////////
// CCapDlg dialog

class CColorKey;

class CCapDlg : public CMyDialog
{
// Construction
public:
	CCapDlg(CWnd* pParent = NULL);	// standard constructor
	~CCapDlg();
// Dialog Data
	//{{AFX_DATA(CCapDlg)
	enum { IDD = IDD_NUCAP_DIALOG };
	PCAPPACKET m_pArgs;
	PCAPINFO m_pInfo;
	char m_szDevice[80];
//	BOOL m_bLive;
//	int	m_nAlign; // 0 is normal, 1 is keyed, 2 is alternate
	int	m_bPrev;	
	int	 m_gmode;  // 0 is gray, 1 is color keying
	UINT frame;
	UINT level;
	UINT hold;
	UINT white;
	UINT gamma;
	UINT whitek;
	UINT maxframe;
	UINT maxlevel;
//	UINT m_old_level;
//	UINT m_old_frame;
	BOOL m_bNeedCell;
	UINT	winx;
	UINT winy;
	UINT m_nAuto;
	CEdit	m_whiteq;
	CEdit	m_gamma;
	CMySlider m_wslider;
	CMySlider m_gslider;
	CMySpin	m_frame;
	CMySpin	m_level;
	CMySpin	m_skip;
	CString m_label;
	BOOL	m_bBozo;
//	BOOL m_bNumbers;
	BOOL m_bColorKey;
	UINT m_nExistFlag;
	//}}AFX_DATA
	int CallBack(BYTE * pData, int kind, int size);
	void GetDeviceName(LPSTR name, int maxlen);
	void UpdateControls();
	void UpdateDeviceInfo();
	void GetCellImage();
	void SetupReference();
	BOOL MyApply(UINT frame, BOOL bHold);
	BOOL GetLevel0(UINT frame, BOOL bHold, BOOL bPrev);
	CColorKey * Keyer() { return m_pKeyer;};
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCapDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CWnd * m_pParent;
	CColorKey * m_pKeyer;
	// Generated message map functions
	//{{AFX_MSG(CCapDlg)
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG *pMsg);
//	afx_msg void OnDialog(UINT Id);
	afx_msg void OnCapture();
	afx_msg void OnSaveSettings();
	afx_msg void OnReceive();
	afx_msg void OnChange();
	afx_msg void OnChange1();
	afx_msg void OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar );
	afx_msg void OnChangeCapFrame();
	afx_msg void OnChangeCapLevel();
	afx_msg void OnChangeCapHold();
	afx_msg void OnCapColorKey();
//	afx_msg void OnCapNumbers();
	afx_msg void OnCapOptions();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CGrabDlg : public CMyDialog
{
// Construction
public:
	CGrabDlg(CWnd* pParent = NULL);	// standard constructor
	~CGrabDlg();
// Dialog Data
	//{{AFX_DATA(CCapDlg)
	enum { IDD = IDD_GRAB_DLG };
	PCAPPACKET m_pArgs;
	PCAPINFO m_pInfo;
	char m_szDevice[80];
	UINT frame;
	UINT level;
	UINT white;
	UINT gamma;
	//}}AFX_DATA
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCapDlg)
	public:
	virtual BOOL DestroyWindow();
	//}}AFX_VIRTUAL

// Implementation
protected:
	CColorKey * m_pKeyer;
	CWnd * m_pParent;
	// Generated message map functions
	//{{AFX_MSG(CCapDlg)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnCapture();
	afx_msg void OnReceive();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CColorKeyDlg : public CMyDialog
{
// Construction
public:
	CColorKeyDlg(CWnd* pParent = NULL);	// standard constructor
// Dialog Data
	//{{AFX_DATA(CCapDlg)
	enum { IDD = IDD_COLOR_KEY };
	void SetColor(BOOL bStop);
	PCAPPACKET m_pArgs;
	UINT m_extent;
	UINT m_red;
	UINT m_green;
	UINT m_blue;
	BOOL m_bInvert;
	int m_kind;
	void EnableControls(BOOL bSelect);
protected:
	CEdit	m_extentxt;
	CMySlider m_slider;
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	// Generated message map functions
	//{{AFX_MSG(CColorKeyDlg)
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG *pMsg);
	afx_msg void OnSelect();
	afx_msg void OnStop();
	afx_msg void OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar );
	afx_msg void OnChange();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
