#if !defined(AFX_CTOOLBOX_H__EBE8894C_0055_404A_A0E7_BE83D6AB7390__INCLUDED_)
#define AFX_CTOOLBOX_H__EBE8894C_0055_404A_A0E7_BE83D6AB7390__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ctoolbox.h : header file
//
class CMainFrame;
class CSketchDoc;
class CToolDlg;
class ctoolbox : public CToolBar
{
public:
	ctoolbox();
	~ctoolbox();
// Construction
public:
	BOOL Setup(CMainFrame * pFrame, UINT w, UINT h, CSketchDoc * pDoc,
							BOOL bToneMatte, BOOL bForce = 0);
	void ChangeTool();
	void CloseDialog(BOOL bMe = 0);
	void DoDialog();
	void FocusFrame();
	void SetupDialog (UINT inSlotIndex);
	BOOL ToolMessage(int msg);

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
	CToolDlg *  m_pDlg;
	int		m_dlgx;
	int		m_dlgy;
	BOOL	m_bTiming;
	int		m_nEraserId;
	int		m_x;
	int		m_y;
	UINT	m_w;
	UINT	m_h;
	BOOL	m_bReqDlg;
	BOOL	m_bColor;
	BOOL	m_bToneMatte;
	UINT	m_ticks;
	BOOL	m_bDown;
	UINT	m_camtool;
	UINT	m_id;
	UINT	m_tool;
	UINT	m_nSlots;
	CMainFrame * m_pFrame;
	CSketchDoc * m_pDoc;
	UINT	m_ids[20];
	UINT	m_bmids[20];
	UINT	m_bdlgs[20];
    virtual void OnBarStyleChange(DWORD dwOldStyle, DWORD dwNewStyle);
	// Generated message map functions
	//{{AFX_MSG(ctoolbox)
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