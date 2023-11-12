
#if !defined(AFX_PREVIEWFILEDLG_H__1D054314_0872_11D2_8A46_0000E81D3D27__INCLUDED_)
#define AFX_PREVIEWFILEDLG_H__1D054314_0872_11D2_8A46_0000E81D3D27__INCLUDED_

#include "dibctrl.h"
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// PreviewFileDlg.h : header file
//
class CSketchDoc;
class CMainFrame;
/////////////////////////////////////////////////////////////////////////////
// CPreviewFileDlg dialog

#define PD_KIND_SCENE 0
#define PD_KIND_AVI 1
#define PD_KIND_CELL 3

class CPreviewFileDlg : public CFileDialog
{
	DECLARE_DYNAMIC(CPreviewFileDlg)

public:
	CPreviewFileDlg(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
		DWORD dwKind,
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);
	~CPreviewFileDlg();
// Attributes
public:
  BOOL m_bPreview;
	DWORD m_kind;
	BYTE * m_pData;
	CDIBStatic* m_dib;
//	CSketchDoc * m_pDoc;
	UINT m_frame;
	UINT m_level;
	UINT m_maxlevel;
	UINT m_maxframe;
	UINT m_hold;
	int m_rotation;
	BOOL m_bScale;
	BOOL m_bKeying;
	BOOL m_bImportColor;
	UINT m_nAlpha;
	CMainFrame * m_pFrame;
	char m_title[20];
	CString m_label;
	int ProcessDibData(void * pData, UINT ssize);
protected:
	void LoadIt(LPCTSTR lpszFileName);
	void LoadAVI(LPCTSTR lpszFileName);
	void LoadScene(LPCTSTR lpszFileName);
	void LoadDib(LPCTSTR lpszFileName);
	void DoMultiple();
	void SetColor(int v);
	void AdjustItems();
	//{{AFX_MSG(CPreviewFileDlg)
	virtual BOOL OnFileNameOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnPreview();
	afx_msg void OnKeying();
	afx_msg void OnLevel();
	afx_msg void OnDrawing() { SetColor(0);};
	afx_msg void OnColor() { SetColor(1);};
//	afx_msg void OnSetFocus(CWnd* pOldWnd);
	//}}AFX_MSG
	virtual void OnFileNameChange();
	virtual void OnFolderChange();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PREVIEWFILEDLG_H__1D054314_0872_11D2_8A46_0000E81D3D27__INCLUDED_)

