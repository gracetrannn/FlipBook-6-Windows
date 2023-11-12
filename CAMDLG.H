#if !defined(AFX_CAMDLG_H__BC49226F_12AC_4B20_AECE_889F70E22882__INCLUDED_)
#define AFX_CAMDLG_H__BC49226F_12AC_4B20_AECE_889F70E22882__INCLUDED_

#include "myspin.h"
#include "camera.h"
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// camdlg.h : header file
//
class CSketchDoc;
class CScene;
//class CCamera;
/////////////////////////////////////////////////////////////////////////////
// camdlg dialog

class camdlg : public CDialog
{
// Construction
public:
	camdlg(CWnd* pParent = NULL);   // standard constructor
	BOOL MyCreate(CWnd * pOwner);//int x, int y);
	UINT Setup(CSketchDoc * pDoc);
	void SelectPeg(UINT Peg);
	void Enable(BOOL bEnable);
	void ToolInit(UINT Id);
	BOOL ToolEnabled(UINT Id);
	UINT ToolSelected();
	UINT Packet(UINT kind, int v1, int v2);
//	UINT CameraAttr(int which, int frame, int peg, double &v, UINT & kind);
	UINT CameraPegXY(int & pegx, int & pegy, int & kind, int frame, int w, int h);
	void SetKind(int kind);
	void SetFrames();
	BOOL IsKey(UINT Frame, UINT Level);
	BOOL Changed(BOOL bSave = 0);
//	double	m_ypos;
//	double	m_xpos;
	CScene * m_pScene;
	CSketchDoc * m_pDoc;
	CCamera * m_pCamera;
	UINT m_which;
	UINT m_kind;
	UINT m_nPeg;
	UINT m_nGroup;
// Dialog Data
	//{{AFX_DATA(camdlg)
	enum { IDD = IDD_CAMDIALOG };
	CComboBox	m_Peg;
	CComboBox	m_Group;
	UINT	m_Frame;
	UINT	m_Frames;
	UINT 	m_width;
	UINT	m_height;
	UINT	m_factor;
	BOOL	m_bUpdating;
	double	m_orig_d1;
	double	m_orig_d2;
	int		m_orig_i1;
	int		m_orig_i2;
	double	m_pegcx;
	double	m_pegcy;
	BOOL	m_bPanning;
	BOOL	m_bFromTool;
	UINT	m_EnabledMask;
	UINT	m_kinds[CCamera::CATTR_EXTRA];
	double	m_values[CCamera::CATTR_EXTRA];
	//}}AFX_DATA


// Overrides
	void FlushChanges();
	void UpdateControls(BOOL bSetPeg = 1);
	void UpdateAttr(UINT which, BOOL bPut = FALSE);
	void UpdateAllAttr(BOOL bPut = FALSE);
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(camdlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void MyUpdate(int id, double &value, double min, double max);
	void MyXYUpdate();
	void SetCrossHair(BOOL bSetFocus = FALSE);
	void SetFrame(UINT frame);
	void AdjIt(int which, BOOL bApply = 1);
	void UpdateView();
	void SetDlgItem(int id, int which, int value);
	void SetupDialog();
	void FillInDialog();
	void KeyDialog(BOOL bTool);
	UINT WhichAttr(UINT which = NEGONE);
	double	m_pegcx2;
	double	m_pegcy2;
	UINT	m_kinds2[CCamera::CATTR_EXTRA];
	double	m_values2[CCamera::CATTR_EXTRA];
	// Generated message map functions
	//{{AFX_MSG(camdlg)
//    afx_msg void OnSpinClick(NMHDR *pNotifyStruct, LRESULT* pResult);
	afx_msg void OnUpdate();
	afx_msg void OnAdjN() {AdjIt(0);};
	afx_msg void OnAdjS() {AdjIt(1);};
	afx_msg void OnAdjE() {AdjIt(2);};
	afx_msg void OnAdjW() {AdjIt(3);};
	afx_msg void OnAdjSc() {AdjIt(4);};
	afx_msg void OnAdjRot() {AdjIt(5);};
	afx_msg void OnAdjBlur() {AdjIt(6);};
	afx_msg void OnAdjAlpha() {AdjIt(7);};
	afx_msg void OnAdjCenter() {AdjIt(8);};
	afx_msg void OnApply();
	afx_msg void OnPeg();
	afx_msg void OnGroup();
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnSelchangeComboGroup();
	afx_msg void OnSelchangeComboPeg();
	afx_msg void OnEditPeg();
	afx_msg void OnEditGroup();
	afx_msg void OnChangeFrame();
	afx_msg void OnKeyType() { KeyDialog(0);};
	afx_msg void OnKeyToolType() { KeyDialog(1);};
	afx_msg void OnNextKey();
	afx_msg void OnPrevKey();
	afx_msg void OnClearKeys();
	afx_msg void OnClearAllKeys();
	afx_msg void OnGoto();
	afx_msg void OnCamHome();
	afx_msg void OnCamEnd();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CAMDLG_H__BC49226F_12AC_4B20_AECE_889F70E22882__INCLUDED_)

