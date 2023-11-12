#if !defined(AFX_SCAN_H__82AB0900_3663_4E97_A4C2_DAA3B3DD82BF__INCLUDED_)
#define AFX_SCAN_H__82AB0900_3663_4E97_A4C2_DAA3B3DD82BF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Scan.h : header file
//

#ifdef FLIPBOOK_MAC
#include "MacTwain.h"
#else
#include "CTwain.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// CScan dialog
class CSketchDoc;
class CScan : public CDialog, CTwain
{
// Construction
public:
	CScan(CWnd* pParent = NULL);   // standard constructor
	~CScan();
// Dialog Data
	//{{AFX_DATA(CScan)
	enum { IDD = IDD_SCANSKEW };
	void	GetChanges(BOOL bJustLimits = 0);
	UINT	m_Frame;
	UINT	m_Level;
	UINT	m_MaxFrame;
	UINT	m_MaxLevel;
	UINT	m_Skip;
	UINT	m_MaxSkip;
	UINT	m_nAlpha;
	BOOL 	m_bSave;
	BOOL	m_bInit;
	BOOL m_bNoDPI;
	BOOL m_bAuto;
	BOOL m_b24;
	BOOL m_bFile;
	BOOL m_bScale;
	BOOL m_bKeying;
	BOOL m_bAutoHold;
	BOOL m_bPegTop;
//	BOOL m_bCropPeg;
	BYTE * m_pLog;
	double m_Offset;
	double m_FieldSize;
	int		m_iRotation;
	CString m_label;
	CSketchDoc * m_pDoc;
	char * m_pData; // for multi-import
	UINT  m_index;
	void MyDibReceived(HGLOBAL hDib) { DibReceived(hDib);};
	UINT Import(int index);
	//}}AFX_DATA

	CSpinButtonCtrl m_SpinLevel;
	CSpinButtonCtrl m_SpinFrame;
	CSpinButtonCtrl m_SpinSkip;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScan)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	virtual void TwainError(TW_ERR e);
	virtual void DibReceived(HGLOBAL hDib);
	//}}AFX_VIRTUAL

// Implementation
protected:
	void DoAuto(HGLOBAL hDib);
	BOOL GetFileImage();
	void CreateSpinCtrls();
	void LogIt(LPCSTR txt);
	BOOL SaveImage(void * p, BOOL bCvt = 0);
	void DoMultiple(char * pData, int offset);
	
	// Generated message map functions
	//{{AFX_MSG(CScan)
	afx_msg void OnScanRot();
	afx_msg void OnScanSave();
	afx_msg void OnScanScan();
	afx_msg void OnScanSelect();
	afx_msg void OnScanAutoHold();
	afx_msg void OnKeying();
	afx_msg void OnLevel();
	afx_msg void OnOverlay();
	afx_msg void OnFile();
	afx_msg void OnClearLog();
	afx_msg void OnViewLog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCAN_H__82AB0900_3663_4E97_A4C2_DAA3B3DD82BF__INCLUDED_)

