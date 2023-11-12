#ifndef _SHEET_H_
#define _SHEET_H_

#ifndef FLIPBOOK_MAC
#include "GridCtrl.h"
#endif

class CSketchDoc;
class CMainFrame;
class CSheet : public CDialog
{
public:
	CSheet(CWnd* pParent = NULL);	// standard constructor
	~CSheet();
	//{{AFX_DATA(CSheet)
	enum { IDD = IDD_GRIDDLG };
	//}}AFX_DATA
//	CMyListBox m_list;
//	BOOL MyCreate(CMainFrame* pOwnerWnd, int x, int y);
	BOOL MyCreate(CMainFrame* pOwnerWnd, BOOL bOnTop);
//	UINT Setup(CSketchDoc * pDoc);
	void SetPosition();
	void UpdateThumb(UINT Frame, UINT Level, BOOL bSelect = FALSE);
//	CLRU	m_pegbar;
	void GetSelection(LPRECT rect);
	void SetSelection(RECT& rect);
	void GetCellText(CString& txt, int row, int col);
	void DrawCellText(CDC * pDC, LPCRECT rect, int row, int col);
	void DrawCellImage(CDC * pDC, LPCRECT rect, int row, int col, DWORD state);
	void Drop(int row, int col, BOOL bCopy);
	void DropFile(LPCSTR szName);
	void SelectCell(UINT Frame, UINT Level);
	void SetRange(UINT Frames, UINT Levels, BOOL bClear = 0);
	CGridCtrl m_Grid;	// public for enum proc
	BOOL Special(UINT val = 2) { return m_Grid.Special(val);};
protected:
//	BOOL OwnerModal();
	void ReDraw();
	CSize m_OldSize;
	virtual BOOL PreTranslateMessage(MSG *pMsg);
	CMainFrame*   m_pOwnerWnd;
	CSketchDoc * m_pDoc;
//	CScene * m_pScene;
	UINT m_height;
	UINT m_width;

	CStatic	m_GridRect;
	int		m_nFixCols;
	int		m_nFixRows;
	int		m_nCols;
	int		m_nRows;
	BOOL	m_bEditable;
	BOOL	m_bHorzLines;
	BOOL	m_bListMode;
	BOOL	m_bVertLines;
	BOOL	m_bSelectable;
	BOOL	m_bAllowColumnResize;
	BOOL	m_bAllowRowResize;
	BOOL	m_bHeaderSort;
	BOOL	m_bReadOnly;
	BOOL	m_bItalics;
	BOOL	m_btitleTips;
	BOOL	m_bSingleSelMode;
	BOOL	m_bCellURL;


	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//{{AFX_MSG(CSheet)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	afx_msg void OnListmode();
	afx_msg void OnItalics();
	afx_msg void OnTitletips();
	afx_msg void OnInsertRow();
	afx_msg void OnDeleteRow();
	afx_msg void OnClose();


	afx_msg void OnExpandToFit();
	afx_msg void OnAutoSize();
	//}}AFX_MSG
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnGridSlideUp(NMHDR *pNotifyStruct, LRESULT* pResult);
    afx_msg void OnGridSlideDown(NMHDR *pNotifyStruct, LRESULT* pResult);
    afx_msg void OnGridSoundSlide(NMHDR *pNotifyStruct, LRESULT* pResult);
    afx_msg void OnGridDelete(NMHDR *pNotifyStruct, LRESULT* pResult);
    afx_msg void OnGridInsert(NMHDR *pNotifyStruct, LRESULT* pResult);
    afx_msg void OnGridCut(NMHDR *pNotifyStruct, LRESULT* pResult);
    afx_msg void OnGridCopy(NMHDR *pNotifyStruct, LRESULT* pResult);
    afx_msg void OnGridPaste(NMHDR *pNotifyStruct, LRESULT* pResult);
    afx_msg void OnGridPasteReverse(NMHDR *pNotifyStruct, LRESULT* pResult);
    afx_msg void OnGridDblClick(NMHDR *pNotifyStruct, LRESULT* pResult);
    afx_msg void OnGridClick(NMHDR *pNotifyStruct, LRESULT* pResult);
    afx_msg void OnGridRClick(NMHDR *pNotifyStruct, LRESULT* pResult);
    afx_msg void OnGridChar(NMHDR *pNotifyStruct, LRESULT* pResult);
    afx_msg void OnGridSpecialChar(NMHDR *pNotifyStruct, LRESULT* pResult);
    afx_msg void OnGridMsg(NMHDR *pNotifyStruct, LRESULT* pResult);
    afx_msg void OnAltKeyUp(NMHDR *pNotifyStruct, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};

#endif
