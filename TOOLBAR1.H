#include "dialogs.h"
#ifndef _CTOOLBAR1_H_
#define _CTOOLBAR1_H_

class CToolBar1 : public CToolBar
{
public:
	BOOL MakeImageList(CImageList & images, UINT offset);

	int Setup(UINT w, UINT h, UINT editw, UINT flags);
	void SetTable(UINT * table, UINT cnt);
	UINT TotalWidth(BOOL bClear = 0);
	BOOL Double() { return m_bDouble;};
	UINT Info(UINT index, UINT which);
	void UpdateInfo(UINT id, BOOL bOn);
	int Config();
	void GetPutProfile(int code);
	CSize CalcDynamicLayout (int nLength, DWORD nMode);

	void DrawGripper(CDC* pDC, const CRect& rect); // from MS
	void EraseNonClient();
	virtual void DoPaint(CDC* pDC);


protected:
	UINT m_w;
	UINT m_h;
	UINT m_ww;
	UINT m_editw;
	BOOL m_bLay;
	BOOL m_bMaya;
	BOOL m_bDouble;
	UINT m_cur[50];
	UINT m_ocnt; // old count
	UINT m_cnt;	// number in buttons
	UINT m_scnt; // separator count
	UINT m_buttons[100]; // with extras

	afx_msg void OnNcPaint(); // from ms
	DECLARE_MESSAGE_MAP()

friend class CToolBarDlg;	
friend class CTBListBox;	
};

class CTBListBox : public CListBox
{
// Construction
public:
	CTBListBox();

// Attributes
public:
	BOOL m_bAvail;
	CToolBar1 * m_pTB;
	CToolBarDlg * m_pDlg;
// Operations
public:
//	int  GetCurSel();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTBListBox)
	public:
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMIS);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
	protected:
//	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
protected:
	int DrawBitmap(CDC * pDC,LPRECT rectFull,UINT Id, BOOL bSelected);
	void DrawIt(CDC * pDC,LPRECT rectFull,UINT Id, UINT code);
	//{{AFX_MSG(CTBListBox)
	afx_msg void OnSelchange();
//	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
//	CTypedPtrList<CPtrList,CExtItem *> m_pItemList;	// item list
	COLORREF			m_clrSel;	   // select color
	COLORREF			m_clrSel2;	   // select frame color
	COLORREF			m_clrBg;	   // background color
	COLORREF			m_clrText;	   // text color
	int					m_curSel;	   // current select index base on zero
	int 	m_size;
	DECLARE_MESSAGE_MAP()
};

class CToolBarDlg : public CMyDialog
{
// Construction
public:
	CToolBarDlg(CWnd* pParent = NULL);	// standard constructor
	CTBListBox m_avail;
	CTBListBox m_cur;
	CToolBar1 * m_tb;
	BOOL m_bFocus;
	enum { IDD = IDD_CUST_TOOLBAR};
	//}}AFX_DATA

// save area for revert
	UINT m_sav_cur[50];
	UINT m_sav_cnt;	// number in buttons
	UINT m_sav_scnt; // separator count



	void UpdateToolBar();
	void ChangeFocus(BOOL bAvail);
	void UpdateBoxes();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
	afx_msg void OnAdd();
	afx_msg void OnRemove();
	afx_msg void OnReset();
	afx_msg void OnRevert();
	afx_msg void OnMoveUp();
	afx_msg void OnMoveDown();



	DECLARE_MESSAGE_MAP()
};

#endif
