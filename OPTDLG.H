#include "dialogs.h"
class COptions : public CMyDialog
{
// Construction
public:
	COptions(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(COptions)
	enum { IDD = IDD_OPTIONS };
#ifdef FBTPC
	BOOL	m_bTabletUsed;
#endif
	BOOL	m_bUseWinTab;
//	BOOL	m_bTabPressure;
	BOOL	m_bTabPrimary;
	BOOL	m_bOpenPrevious;
	BOOL	m_bLeftHanded;
	int 	m_broadcast;
	BOOL	m_bToolsSameColor;
	BOOL	m_bSimpleCursor;
	BOOL	m_bHardZoom;
	BOOL	m_bSheetOnTop;
	BOOL	m_bChkUpdates;

//	int	m_wBGMinDark;
	BOOL	m_bBGShow;		// also on toolbar
	BOOL	m_bUseMouths;
	BOOL	m_bEditScrub;
//	int	m_bPegShowBG;	 // also on toolbar
//	int	m_bPegShowFG;	 // also on toolbar
//	int	m_wPegMaxDepth;
//	int	m_wPegMinDensity;

	int	m_wPenMinRadius;
	int	m_wPenMaxRadius;
	int	m_wPencilMinDensity;
	int	m_wPencilMaxDensity;
	int	m_wEraserMinDensity;
	int	m_wEraserMaxDensity;
	int	m_wMinPress;
	int	m_wMaxPress;

	int	m_bFlipFew;
	int	m_bAutoKeep;
	int	m_bLoopPlay;
	int m_bCheckerBG;
	int m_bSaveCache;
	int m_nJpeg;
	int m_bLeftScroll;
//	int m_bScaleBG;
	UINT m_rate;
	//}}AFX_DATA
// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	// Generated message map functions
	//{{AFX_MSG(monitor)
	virtual BOOL OnInitDialog();
//	virtual void OnCancel();
//	virtual void OnOK();
	//}}AFX_MSG
	afx_msg void OnJpeg();
	afx_msg void OnEnableShows();
	DECLARE_MESSAGE_MAP()
};

class CSketchDoc;
class CScene;
class CLightOptions : public CMyDialog
{
// Construction
public:
	CLightOptions(CWnd* pParent = NULL);	// standard constructor
	CSketchDoc * m_pDoc;
	CScene * m_pScene;
// Dialog Data
	enum { IDD = IDD_LIGHTBOX };
	BOOL m_bDirty;
	BOOL m_bSheet;
	int m_nLevels;
	int m_nKind;	// 0 is MRU, 1 is In Between
//
//	MRU Options
//
	int	m_wPegMaxDepth;
	int	m_wPegMinDensity;
	int	m_wBGMinDark;

	int	m_bPegShowBG;	 // also on toolbar
	int	m_bPegShowFG;	 // also on toolbar

//
//	In Between options
//
	BOOL m_bShowNext;
	BOOL m_bShowPrev;
	BOOL m_bShowNext1;
	BOOL m_bShowPrev1;
	BOOL m_bShowPaint;
	BOOL m_bShowLPaint;
	BOOL m_bShowBud0;
	BOOL m_bShowBud1;
	BOOL m_bShowBud2;
	BOOL m_bShowBud3;
	UINT m_nBuddy0;
	UINT m_nBuddy1;
	UINT m_nBuddy2;
	UINT m_nBuddy3;
//
//	saved settings for restore on cancel
//
	BOOL m_bColor;
	int m_Kind;
	int	m_Depth;
	int	m_Den;
	int	m_Dark;
	int	m_BG;
	int	m_FG;
	BOOL m_Sheet;
	BOOL m_Next;
	BOOL m_Prev;
	BOOL m_Next1;
	BOOL m_Prev1;
	BOOL m_Paint;
	BOOL m_LPaint;
	UINT m_bud0;
	UINT m_bud1;
	UINT m_bud2;
	UINT m_bud3;
	//}}AFX_DATA
// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	// Generated message map functions
	//{{AFX_MSG(monitor)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	void UpdateControls();
	void ApplySettings();
	void ChangeMode();
	afx_msg void OnApply();
};
