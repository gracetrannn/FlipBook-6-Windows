// mainfrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////
#ifndef __MAINFRMH
#define __MAINFRMH

class CSheet;
class CPaletteDlg;
class CSubPalDlg;
class camdlg;
class ctoolbox;
class ccamtool;
class CMainFrame;
class CSketchDoc;
class CMySound;
class CScene;
class CToolBar1;
//class CMyToolBar;
class CMyDialogBar;
class CThumbs;
class CScan;
class CPuckDlg;
class CZoomerDlg;
class CLipSyncDlg;
class CMyLibDlg;

class CMainFrame : public CFrameWnd
{
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:
	BOOL CreateXSheet();
	BOOL CreateToolDlg();
	BOOL CreateSlider();
	void Status(LPCSTR txt = 0);
	void ToolStatus(int idd, int arg1, int arg2);
	UINT Progress(int code, int arg);
	void ModeChange();
	void Opener(BOOL bOpened);
	BOOL SwitchSheet(BOOL bSwitch = TRUE);
	BOOL SwitchSound(BOOL bSwitch = TRUE);
//#if MAC
	BOOL SwitchTools(BOOL bSwitch = TRUE);
	BOOL SwitchCamTools(BOOL bSwitch = TRUE);
//#endif
	BOOL CanDoSound() { return m_bCanDoSound;};
	BOOL SwitchPalette(BOOL bSwitch = TRUE);
	BOOL SwitchSubPalette(BOOL bSwicth = TRUE);
	BOOL SwitchLipSync(BOOL bSwitch = TRUE);
	BOOL SwitchPuck(BOOL bSwitch = TRUE);
	BOOL SwitchZoomer(BOOL bSwitch = TRUE);
	BOOL SwitchCamera(BOOL bSwitch = TRUE);
	BOOL Timing(UINT val = 3); // 0,1,2 is switch, 3 is read
	void SetCamera();
	void RedrawSheet(UINT flag = 0);
	void AttachDoc(CSketchDoc * pDoc);
	void SelectCell(UINT Frame, UINT Level, UINT who);
	void SelectGCell(UINT row, UINT col);
	void SetRange(UINT Frames, UINT Levels);
	void SetSheetMode(UINT mode);
	void UpdateCell(UINT Frame, UINT Level);
	void UpdateLevelThumbs(UINT Level);
	void DirtyThumb(UINT PalIndex = NEGONE);
	void 	SetFrameRate(int rate);
//	BOOL	InternalSound() { return m_bInternalSound;};
	void SetSound(BOOL bUse, int idd, double smark, UINT frame, UINT snip, LPSTR Name);
	UINT	StackFrame() { return m_stack_frame;};
	UINT	StackLevel() { return m_stack_level;};
	UINT	OnionCount() { return m_onion_count;};
	UINT	BuddyCount() { return m_buddy_count;};
	UINT	Onion(int index) { return m_stack_onion[index];};
	UINT	BuddyL(int index) { return m_stack_buddyl[index];};
	UINT	BuddyF(int index) { return m_stack_buddyf[index];};
//	UINT 	InitPegDepth(BOOL bClear = 0);
//	UINT 	PegCount(BOOL bClear = FALSE);
//	BOOL	RemoveId(UINT idd) { return m_pegbar.RemoveId(idd);};
//	UINT 	PegContents(UINT & Frame, UINT & Level, UINT idd, BOOL bPut = 0);
	void 	GetCellText(LPSTR buf, UINT row, UINT col);
//	UINT	CameraPacket(int kind, int v1, int v2);
	UINT	Frames() { return m_Frames;};
	UINT	MaxFrame() { return m_MaxFrame;};
	UINT	Levels() { return m_Levels;};
	UINT	Rows();
	UINT	Columns();
	void	DrawThumb(CDC * pDC, LPCRECT, UINT row, UINT col, DWORD State);
	void	DrawText(CDC * pDC, LPCRECT, UINT row, UINT col);
	void	Drop(UINT row, UINT col,BOOL bCopy);
	void	DropFile(LPCSTR szName);
//	void	DropFile(LPCSTR szName, UINT row, UINT col);
	void 	Paster(UINT Frame, UINT Level, BOOL bReverse, BOOL bBreak = 0);
//	BYTE *  GetThumb( UINT Frame, UINT Level);
	void	ClearThumb( UINT Frame, UINT Level);
	void  	ChangeLevels(UINT Start, UINT End);
	void  	SlideCells(UINT From,  UINT To, 
				UINT StartLevel, UINT EndLevel, UINT Count);
	void 	ChangeTool(BOOL bPaintOnly = FALSE );
	BOOL	HasModel(UINT v = NEGONE, UINT Level = NEGONE);
	BOOL	LipSyncMode() { return m_pLipSync ? 1 : 0;};
	void 	SetDisPaint(UINT index);
	void	SetColor(UINT index, BYTE r, BYTE g, BYTE b, BYTE alpha);
	void	ColorTrap(int x, int y, int code);
	void	LClick(UINT row, UINT col);
	void	RClick(UINT row, UINT col);
	void 	SaveSettings(BOOL bDoClip);
	void 	Placements(BOOL bSave);
	BOOL 	ToolMessage(int msg);
	void	SetLeftScroll();
// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	CSketchDoc * m_pDoc;
	CScene * m_pScene;
	CMySound * m_pSound;
	DWORD SceneId();
	UINT SceneState();
//	CSheet * SheetAddr() { return (CSheet *)&m_wndSheet;};
//	CToolDlg * ToolAddr() { return (CToolDlg *)&m_wndTools;};
	void RedrawGrid(UINT Frame, UINT Level, UINT w = 1, UINT h = 1);
	void RedrawSound();
	BOOL GetSelection(UINT& Frame, UINT &Level);
	BOOL GetSelection(UINT& StartFrame, UINT &StartLevel,
			UINT &EndFrame, UINT & EndLevel);
	BOOL SetSelection(UINT StartFrame, UINT StartLevel,
			UINT EndFrame, UINT EndLevel);
	BOOL DeleteSelection(UINT StartFrame, UINT StartLevel,
			UINT EndFrame, UINT EndLevel);
	BOOL CheckSelection(UINT StartFrame, UINT StartLevel,
			UINT EndFrame, UINT EndLevel);
	BOOL IdleProcess();
	void SetBusy(int percent);
	void LevelProp(UINT Level);
	void CellProp(UINT Frame, UINT Level);
	void SoundProp(int idd, LPCSTR name = 0);
	void PlaySnd(UINT frame=9999, UINT which=0);
	void SlideSound(int updown, int dx);
	void InsertDelete(BOOL bDelete);
	void GridChar(int nChar, int nRep);
	void GridMsg(int msg, int x, int y);
	void CheckViewDirty();
	void StackUpdate(BOOL bJustDraw = 0);
	UINT CameraPacket(int kind, int v1, int v2);
//	UINT CameraAttr(int which, int frame, int peg, double &v, UINT & kind);
	UINT CameraPegXY(int & pegx, int & pegy, int & kind, int frame, int w, int h);
	void UpdateTools(BOOL bForce = 0); 
	void	StackInit(UINT frame, UINT level);
	BOOL 	DisPalMode() { return m_bDisPal;};
	CScan	* m_pScan;
	BOOL	EnableSheet(int code);
	UINT	MaxSoundCols() { return m_nSoundCols;};
	void RedrawLibEntries();
	BOOL  	ChangeFrames(UINT Start, UINT End);
protected:  // control bar embedded members
	BOOL	CheckFrames(UINT frame);
	void UpdateThumb(UINT Frame, UINT Level);
	void SlideIt(int updown, UINT Count = 1);
	virtual BOOL PreTranslateMessage(MSG *pMsg);
	CStatusBar  * m_pStatusBar;

	CToolBar1 * m_pToolBar;
//	CMyToolBar * m_pSlider;
	CMyDialogBar * m_pSlider;
	CSheet	*	m_pSheet;
	ctoolbox *	m_pTools;
	ccamtool *	m_pCamTools;
	CPaletteDlg	* m_pPalette;
	CSubPalDlg	* m_pSubPal;
	CLipSyncDlg	* m_pLipSync;
	CPuckDlg	* m_pPuckDlg;
	CZoomerDlg	* m_pZoomerDlg;
	camdlg	* m_pCamera;
	CMyLibDlg * * m_pLibs;
	UINT  m_nLibraries;
	int  m_nIconSize;
	BOOL m_bCanDoSound;
	BOOL m_bShowSound;
//#if MAC
	BOOL m_bShowTools;
	BOOL m_bShowCamTools;
//#endif
	BOOL m_bShowSheet;
	BOOL m_bShowSlider;
	BOOL m_bShowPalette;
	BOOL m_bShowLipSync;
	BOOL m_bShowPuck;
	BOOL m_bShowZoomer;
	BOOL m_bShowSubPal;
	BOOL m_bShowCamera;
	BOOL m_bNeedExtClip;
	BOOL m_bTiming;
	BOOL m_bXSheetHighlight;
	UINT m_TimingLevel;
	UINT	m_nDoubleDigit;
	int  m_nBusy;
	int	 m_nSoundCols;
	int  m_nViewDirty;
	BOOL m_bDirtyThumbs;
	BOOL m_bHaveThumbs;
	int  m_nDisPal;			// count of number of pal entires to clear
	BOOL m_bDisPal;			// flag, set if in disp pal mode
	BOOL m_bDisPal2;		// flag set if we have already, auto played
//	BOOL	m_bInternalSound;
	double m_smarksave;
	int	m_range;		// for progress
	UINT	m_max_frames;
	UINT	m_MyClipCell;
//	UINT	m_MyClipImage;
	UINT	m_vFrame;	// the one in vcr window 
	UINT	m_Frames;
	UINT	m_Levels;
	UINT	m_Frame;
	UINT	m_Kind;   // from xlaterowcol
	UINT	m_MaxFrame;
	UINT	m_Level;
//	UINT	m_ClipFrame;
//	UINT	m_ClipLevel;
//	UINT	m_owidth;
	UINT	m_oheight;
	UINT	m_tbsize; // toolbar icon size
	UINT	m_twidth;
	UINT	m_theight;
	UINT	m_tpitch;
	UINT	m_tsize;
	UINT m_ChkRepFrame;
	UINT m_ChkRepLevel;
	BOOL	m_stack_bg;
	BOOL	m_stack_flag;
	UINT	m_buddy_count;
	UINT	m_onion_count;
	UINT	m_stack_frame;	// old peg stuff
	UINT	m_stack_level;
	UINT	m_stack_buddyl[4];
	UINT	m_stack_buddyf[4];
	UINT	m_stack_onion[5];
	CThumbs * m_pThumbs;
	BYTE *  m_pDib;
	BYTE *  m_pHdrIcons;
	UINT	SoundCols() { return m_bShowSound ? m_nSoundCols : 0;};
	BOOL	IsSound(int column)
				{ return column && (column <= (int)SoundCols())? 1:0;};
	void	StackReload();
	void 	StackClear();
	void	StackDraw();
	int		StackFind(UINT frame, UINT level, UINT code);
	UINT NewPaste(UINT Frame, UINT Level);	
	BOOL SetupThumb();
	BOOL CutCopy(BOOL bCopy);
	void DoPaste(BOOL bReverse);
	BOOL XlateRowCol(UINT row, UINT col);
	BOOL CheckReplace(UINT F, UINT L, UINT w, UINT h, UINT kind = 0);
	void CheckReplaceFix();
	void CopySubPal();
	void AdjustDisPalWindows(BOOL bInit);
	void FixOEMMenu();
public:
	UINT MakeToolBar();
protected:
	void DoSlider(int code);
	void GetHeaderIcons();
	void DoHeaderIcon(CDC* pDc,int which, LPCRECT rect, BOOL bSel);
//	void ChangeLayer(UINT which);
//	void UpdateLayer(CCmdUI * pUI, UINT which);
//	void FillLayerCombo(int sel, BOOL bFill);
	void OnUpdateCutCopy(CCmdUI* pCmdUI, UINT Idd);
	void OnUpdatePasters(CCmdUI* pCmdUI, UINT Idd);
	void OnUpdateCamTool(CCmdUI* pCmdUI, UINT Idd);
	void OnUpdateCamButton(CCmdUI* pCmdUI, UINT Idd);
	void OnCamTool(UINT Idd);
// Generated message map functions
protected:
	CMenu * GetLibMenu(BOOL bRemove = 0);
	CMenu * GetUnLoadMenu(CMenu * pLibMenu);
	CMenu * m_pIconMenu;
	CMenu * m_pOverrideMenu;
	BOOL 	LoadLib(LPCSTR filename);
	void RemoveLibEntries(UINT iid);
	void SaveLibInfo();
	void LoadLibInfo();
	//{{AFX_MSG(CMainFrame)
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg void OnReturn();
	afx_msg BOOL OnViewBar(UINT nID);
	afx_msg void OnUpdateBarMenu(CCmdUI* pCmdUI);
	afx_msg void OnEditCut() { CutCopy(FALSE);};
	afx_msg void OnEditCopy() { CutCopy(TRUE);};
	afx_msg void OnUpdateCopy(CCmdUI * pCmdUI)
					{OnUpdateCutCopy(pCmdUI, ID_EDIT_COPY);};
	afx_msg void OnUpdateCut(CCmdUI * pCmdUI)
					{OnUpdateCutCopy(pCmdUI, ID_EDIT_CUT);};
//	afx_msg void OnEditOutside();
	afx_msg void OnEditPaste() { DoPaste(0);};
	afx_msg void OnEditPasteReverse() { DoPaste(1);};
	afx_msg void OnUpdatePaste(CCmdUI* pCmdUI)
				{ OnUpdatePasters(pCmdUI,ID_EDIT_PASTE);};
	afx_msg void OnUpdatePasteRev(CCmdUI* pCmdUI)
				{ OnUpdatePasters(pCmdUI,ID_EDIT_PASTEREVERSED);};
	afx_msg void OnEditInsert();
	afx_msg void OnUpdateInsert(CCmdUI* pCmdUI);
	afx_msg void OnEditDelete();
	afx_msg void OnUpdateDelete(CCmdUI* pCmdUI);
	afx_msg void OnEditBlank();
	afx_msg void OnUpdateBlank(CCmdUI* pCmdUI);
	afx_msg void OnEditAppend();
	afx_msg void OnUpdateAppend(CCmdUI* pCmdUI);
	afx_msg void OnDummy() {};
	afx_msg void OnImpWave() { SoundProp(9);};
	afx_msg void OnUpdateImpWave(CCmdUI* pCmdUI);
	afx_msg void OnSliderStart();
	afx_msg void OnSliderStop();
	afx_msg void OnSliderPlay() { DoSlider(0);};
	afx_msg void OnSliderBegin() { DoSlider(1);};
	afx_msg void OnSliderBack() { DoSlider(2);};
	afx_msg void OnSliderFwd() { DoSlider(3);};
	afx_msg void OnSliderEnd() { DoSlider(4);};
	afx_msg void OnLayer();
//	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMove(int x, int y);
	afx_msg LRESULT OnDisplayChange(WPARAM wParam, LPARAM lParam);
	afx_msg void OnUpdateLayer(CCmdUI * pUI);
	afx_msg void OnUpdatePlay(CCmdUI * pUI);
//	afx_msg void OnLayerUp() {ChangeLayer(1);};
//	afx_msg void OnLayerDn() {ChangeLayer(0);};
//	afx_msg void OnUpdateLayerUp(CCmdUI * pUI) {UpdateLayer(pUI,1);};
//	afx_msg void OnUpdateLayerDn(CCmdUI * pUI) {UpdateLayer(pUI, 0);};
#ifdef NOT_USED
	afx_msg void OnEnterMenuLoop(BOOL bIsTrackPopupMenu);
	afx_msg void OnExitMenuLoop(BOOL bIsTrackPopupMenu);
#endif
	afx_msg void OnRenderFormat(UINT format);
	afx_msg void OnRenderAllFormats();
	afx_msg void OnCustomizeToolBar();
//	afx_msg void OnUpdateMagic11(CCmdUI* pCmdUI);
//	afx_msg void OnMagic11();
//	afx_msg void OnMagic12();
    afx_msg void OnTBN_Drop(NMHDR *lpnmtb, LRESULT* pResult);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnUpdateZoom(CCmdUI* pCmdUI) 
				{OnUpdateCamTool(pCmdUI,ID_TOOL_CAM_ZOOM);};
	afx_msg void OnUpdatePan(CCmdUI* pCmdUI)
				{OnUpdateCamTool(pCmdUI,ID_TOOL_CAM_PAN);};
	afx_msg void OnUpdateRotate(CCmdUI* pCmdUI)
				{OnUpdateCamTool(pCmdUI,ID_TOOL_CAM_ROTATE);};
	afx_msg void OnUpdateBlur(CCmdUI* pCmdUI)
				{OnUpdateCamTool(pCmdUI,ID_TOOL_CAM_BLUR);};
	afx_msg void OnUpdateAlpha(CCmdUI* pCmdUI)
				{OnUpdateCamTool(pCmdUI,ID_TOOL_CAM_ALPHA);};
	afx_msg void OnUpdateDALLKeys(CCmdUI* pCmdUI)
				{OnUpdateCamButton(pCmdUI,ID_TOOL_CAM_DALL);};
	afx_msg void OnUpdateDTHISKey(CCmdUI* pCmdUI)
				{OnUpdateCamButton(pCmdUI,ID_TOOL_CAM_DTHIS);};
	afx_msg void OnUpdateNextKey(CCmdUI* pCmdUI)
				{OnUpdateCamButton(pCmdUI,ID_TOOL_CAM_NEXT);};
	afx_msg void OnUpdatePrevKey(CCmdUI* pCmdUI)
				{OnUpdateCamButton(pCmdUI,ID_TOOL_CAM_PREV);};
	afx_msg void OnCamDialog();
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
//	afx_msg void OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar );
//    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnViewLib(UINT iid);
	afx_msg void OnUpdateViewLib(CCmdUI* pCmdUI);
	afx_msg void OnLibLoad();
	afx_msg void OnLibSave();
	afx_msg void OnLibUnloadAll();
	afx_msg void OnLibUnload(UINT iid);
	afx_msg void OnUpdateIconSize(CCmdUI* pCmdUI);
	afx_msg void OnIconSize(UINT iid);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

friend class CSheet;
};
#endif
/////////////////////////////////////////////////////////////////////////////

