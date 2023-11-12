#if !defined(AFX_NEWDLG_H__FA059761_F1C8_11D3_8B31_00E02936453E__INCLUDED_)
#define AFX_NEWDLG_H__FA059761_F1C8_11D3_8B31_00E02936453E__INCLUDED_
#include "mydialog.h"
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNewDlg dialog

class CNewDlg : public CMyDialog
{
// Construction
public:
	CNewDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNewDlg)
	enum { IDD = IDD_NEW };
	UINT	m_Frames;
	UINT	m_Levels;
	UINT	m_MaxFrames;
	UINT	m_MaxLevels;
	UINT	m_MaxHeight;
	UINT	m_MaxWidth;
	UINT	m_Height;
	UINT	m_Width;
	UINT	m_Rate;
	int		m_Factor;
	int		m_Preview;
	int		m_nJpeg;
	int		m_bcast;
//	int		m_Kind;
	BOOL	m_bXSheet;
	UINT	m_SFrames;
	UINT	m_SRate;
	BOOL	m_bFromAVI;
//	CString	m_Name;
//	CString	m_Location;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void NewRes(int w, int h);
	// Generated message map functions
	//{{AFX_MSG(CNewDlg)
    virtual BOOL OnInitDialog();
//	afx_msg void OnChange();
//	afx_msg void OnStory();
	afx_msg void OnJpeg();
	afx_msg void OnFromAVI();
//	afx_msg void OnXSheet();
	afx_msg void OnResQtr()  { NewRes(320,240);};
	afx_msg void OnResFull() { NewRes(640,480);};
	afx_msg void OnResNtsc() { NewRes(720,480);};
	afx_msg void OnResPal()  { NewRes(768,576);};
	afx_msg void OnResNuNtsc(){ NewRes(720,540);};
	afx_msg void OnResNuPal(){ NewRes(720,576);};
	afx_msg void OnResHdtv() { NewRes(1920,1080);};
	afx_msg void OnResAcad() { NewRes(3000,2000);};
	afx_msg void OnResCust() { NewRes(0,0);};
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CInitialDlg : public CMyDialog
{
// Construction
public:
	CInitialDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNewDlg)
#ifdef _THEDISC
	enum { IDD = IDD_INITIAL_DISC};
#else
	enum { IDD = IDD_INITIAL };
#endif
	CString	m_Name;
	BOOL	m_bPrevious;
	BOOL	m_bRegistered;
	BOOL	m_bCanExpire;
	UINT	m_answer;
	//}}AFX_DATA

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNewDlg)
    virtual BOOL OnInitDialog();
	afx_msg void OnNew() { m_answer = 1;OnOK();};
	afx_msg void OnOpen() { m_answer = 2;OnOK();};
	afx_msg void OnPrev() { m_answer = 3;OnOK();};
	afx_msg void OnAbout() { m_answer = 4;OnOK();};
	afx_msg void OnRegister() { m_answer = 5;OnOK();};
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CFromAVIDlg : public CMyDialog
{
public:
	CFromAVIDlg(CWnd* pParent = NULL);   // standard constructor
	enum { IDD = IDD_FROM_AVI};
	UINT	m_Frames;
	UINT	m_Levels;
	UINT	m_nHeight;
	UINT	m_nWidth;
	UINT	m_nRate;
	int		m_nJpeg;
	BOOL	m_bUseSound;
	UINT	m_MaxFrames;
	UINT	m_MaxHeight;
	UINT	m_MaxWidth;
	BOOL	m_bScaleIt;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
protected:
	void NewRes(int w, int h);
    virtual BOOL OnInitDialog();
	afx_msg void OnJpeg();
	DECLARE_MESSAGE_MAP()
};


#endif

