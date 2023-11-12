
#include "dialogs.h"
#if !defined(ACTION_H)
#define ACTION_H

class CMainFrame;
class CScene;
class CLevelTable;
class CNewPals;
class CActionFillDlg : public CMyDialog
{
// Construction
public:
	CActionFillDlg(CWnd* pParent = NULL);   // standard constructor

	enum { IDD = IDD_FXFILL };
	UINT	m_StartFrame;
	UINT	m_EndFrame;
	UINT	m_MaxFrame;
	UINT	m_Level;
	UINT	m_MaxLevel;
	UINT	m_color;
	int	m_Kind;
	CScene * m_pScene;
	CMainFrame * m_pFrame;
	CLevelTable * m_pTable;
protected:
	CProgressCtrl   m_Progress;
	UINT 	m_OrigFrame;
	UINT	m_state;
	UINT	m_mask;
	void CheckAbort();
	void UpdateControls();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	DECLARE_MESSAGE_MAP()
};

class CActionFlipDlg : public CMyDialog
{
// Construction
public:
	CActionFlipDlg(CWnd* pParent = NULL);   // standard constructor

	enum { IDD = IDD_FXFLIP };
	UINT	m_StartFrame;
	UINT	m_EndFrame;
	UINT	m_MaxFrame;
	UINT	m_StartLevel;
	UINT	m_EndLevel;
	UINT	m_MaxLevel;
	int	m_Kind;
	CScene * m_pScene;
	CMainFrame * m_pFrame;
protected:
	CProgressCtrl   m_Progress;
	UINT 	m_OrigFrame;
	UINT	m_state;
	UINT	m_mask;
	void CheckAbort();
	void UpdateControls();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	DECLARE_MESSAGE_MAP()
};

class CActionSpeckleDlg : public CMyDialog
{
// Construction
public:
	CActionSpeckleDlg(CWnd* pParent = NULL);   // standard constructor

	enum { IDD = IDD_FXSPECK };
	UINT	m_StartFrame;
	UINT	m_EndFrame;
	UINT	m_MaxFrame;
	UINT	m_StartLevel;
	UINT	m_EndLevel;
	UINT	m_MaxLevel;
	int	m_Kind;
	CScene * m_pScene;
	CMainFrame * m_pFrame;
protected:
	CProgressCtrl   m_Progress;
	UINT 	m_OrigFrame;
	UINT	m_state;
	void CheckAbort();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
};



class CActionMatteDlg : public CMyDialog
{
// Construction
public:
	CActionMatteDlg(CWnd* pParent = NULL);   // standard constructor

	enum { IDD = IDD_FXMATTE };
	UINT	m_StartFrame;
	UINT	m_EndFrame;
	UINT	m_MaxFrame;
	UINT	m_Level;
	UINT	m_MaxLevel;
	BOOL m_bOutSide;
	CScene * m_pScene;
	CMainFrame * m_pFrame;
	UINT	m_color;
	CLevelTable * m_pTable;
protected:
	CNewPals * m_pPals;
	UINT	m_layer;
	UINT	m_blur;
	int m_offx;
	int m_offy;
	CProgressCtrl   m_Progress;
	UINT 	m_OrigFrame;
	UINT	m_state;
	void CheckAbort();
//	void ChangeColor();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnDrawItem(int nId, LPDRAWITEMSTRUCT lpDrawItemStruct);
	DECLARE_MESSAGE_MAP()
};


#endif
