#if !defined(AFX_CCAMTOOL_H__EBE8894C_0055_404A_A0E7_BE83D6AB7390__INCLUDED_)
#define AFX_CCAMTOOL_H__EBE8894C_0055_404A_A0E7_BE83D6AB7390__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ccamtool.h : header file
//
class CMainFrame;
class ccamtool : public CToolBar
{
public:
	ccamtool();
// Construction
public:
	BOOL Setup(CMainFrame * pFrame, UINT w, UINT h, BOOL bForce = 0);
	void DoDialog();
	void FocusFrame();
	void DrawGripper(CDC* pDC, const CRect& rect); // from MS
	void EraseNonClient();
	virtual void DoPaint(CDC* pDC);

// Implementation
protected:
	BOOL MakeToolBar();
	BOOL MakeImageList(CImageList & images, UINT offset);
	BOOL HasDialog(UINT id);
	void SetSlot(int bmpid, int toolid, BOOL bDlg=0);
	void MakeBitmap(int vw);
	int PointToSlot(CPoint point);
	int ToolToSlot(int tool);
	void UpdateBox(int id);
	void WeMoved(int x, int y);
//	BOOL	m_bTiming;
	int		m_x;
	int		m_y;
	UINT	m_w;
	UINT	m_h;
	BOOL	m_bIgnoreRBut;
//	BOOL	m_bReqDlg;
	BOOL	m_bColor;
//	int		m_nKind;
	UINT	m_ticks;
	BOOL	m_bDown;
	UINT	m_id;
//	UINT	m_tool;
	UINT	m_nSlots;
	CMainFrame * m_pFrame;
//	CSketchDoc * m_pDoc;
	UINT	m_ids[20];
	UINT	m_bmids[20];
	UINT	m_bdlgs[20];
	// Generated message map functions
	//{{AFX_MSG(ccamtool)
	afx_msg void OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/);
	afx_msg void OnLButtonDown(UINT /*nFlags*/, CPoint point);
	afx_msg void OnRButtonUp(UINT /*nFlags*/, CPoint /*point*/);
	afx_msg void OnRButtonDown(UINT /*nFlags*/, CPoint point);
	afx_msg void OnTimer(UINT nTimerID);

	afx_msg void OnNcPaint(); // from ms

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};


#endif // !defined(AFX_CTOOLBOX_H__EBE8894C_0055_404A_A0E7_BE83D6AB7390__INCLUDED_)

