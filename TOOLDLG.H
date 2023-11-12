#ifndef _TOOLDLG_H_
#define _TOOLDLG_H_

class CSketchDoc;

class CToolDlg : public CDialog
{
public:
	CToolDlg();//int kind, CWnd* pParent = NULL);
	~CToolDlg();
	BOOL MyCreate(CWnd* pOwnerWnd, CSketchDoc * pDoc);//, int x, int y);
	WORD Setup(CSketchDoc * pDoc);
	UINT m_kind;
	CWnd*   m_pOwnerWnd;
	UINT	m_minden;
	UINT	m_maxden;
	UINT	m_curden;
	UINT	m_minrad;
	UINT	m_maxrad;
	UINT	m_currad;
	UINT	m_nDocked; // 1 is below, 2 is right
protected:
	void FocusIt();
	int m_Inc;
	int m_DlgId;
	int m_Id;
	int m_which;
	CSketchDoc * m_pDoc;
	WORD m_height;
	WORD m_width;
	int	IdFromPoint(CPoint pt);
	void Capture(int nId);
	void MoveIt();
	void AdjustTextures(BOOL bAnti);
	UINT CurRad() { return m_currad;};
	UINT MinRad() { return m_minrad;};
	UINT MaxRad() { return m_maxrad;};
	UINT CurDen() { return m_curden;};
	UINT MinDen() { return m_minden;};
	UINT MaxDen() { return m_maxden;};
//	void SetFillLimit(int v);
	void SetPen(int v);
	void SetText(int v);
	void SetPenTab(int v);
	void SelectFlags(int v);
	CSliderCtrl m_FillScroll;
	void PostNcDestroy(); 
	
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
//	CWnd*   m_pOwnerWnd;
	virtual BOOL OnInitDialog();
	//{{AFX_MSG(CSheet)
	afx_msg void OnDrawItem(int nId, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnTimer(UINT nTimerID);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/);
	afx_msg void OnLButtonDown(UINT /*nFlags*/, CPoint point);
	afx_msg void OnClose();
	afx_msg void OnMove(int x, int y);
	afx_msg void OnSelectStay() {SelectFlags(1);};
	afx_msg void OnSelectAuto() {SelectFlags(2);};
	afx_msg void OnSelectLasso() {SelectFlags(4);};
	afx_msg void OnPenFree(){SetPen(0);};
	afx_msg void OnPenLine(){SetPen(1);};
	afx_msg void OnPenRect(){SetPen(2);};
	afx_msg void OnPenEllipse(){SetPen(3);};
	afx_msg void OnPenCurve(){SetPen(4);};
	afx_msg void OnPenSmart(){SetPen(5);};
	afx_msg void OnPenTabRad(){SetPenTab(0);};
	afx_msg void OnPenTabDen(){SetPenTab(1);};
	afx_msg void OnRadAnti(){SetPenTab(2);};
	afx_msg void OnText0(){SetText(0);};
	afx_msg void OnText1(){SetText(1);};
	afx_msg void OnText2(){SetText(2);};
	afx_msg void OnText3(){SetText(3);};
	afx_msg void OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
#endif
