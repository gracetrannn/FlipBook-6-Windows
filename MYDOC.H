#ifndef _MYDOC_H_
#define _MYDOC_H_

#include "cscene.h"
class CSketchView;
//class CFlip;
//class CCanvas;
class CSheet;
class CTool;
class CMySound;
//
// doit defs
//
#define DOIT_BYTE 1
#define DOIT_WORD 2
#define DOIT_LONG 3
#define DOIT_FLOAT 4
#define DOIT_BOOL 5
#define DOIT_STRING 6


//	defines for update def folder
//
#define UDF_DEF 0
#define UDF_EXP 1
#define UDF_IMP 2
#define UDF_LIB 3

class CSketchDoc : public CDocument
{
protected: // create from serialization only
	CSketchDoc();
	DECLARE_DYNCREATE(CSketchDoc)

// Attributes
public:
	CSketchView * GetDocView();
	BOOL DocOpened() const {return m_bOpened;};
	CScene  * GetDocScene() const {return m_pScene;};
	UINT CreateCell(UINT Frame, UINT Level, LPBITMAPINFOHEADER  lpbi,
				UINT Rotation, BOOL bMakeMono, UINT nHold, BOOL bNoUpdate = 0);
	UINT 	InsertMouth(UINT index, UINT frame, UINT level);
	UINT	Progress(int code = 0, int arg = 0);
	UINT	SelectTool(UINT tool = 9999);
	BOOL	ToolInfo(int code = 0, int val=0 );// 0 is get 1 is set, 2 is toggle 
	UINT	Radius(UINT arg = 3000, int which = 1000);
	UINT	Density(UINT arg = 3000, int which = 1000);
	UINT	Color(UINT arg = 3000, int which = 1000);
	UINT	Appended( BYTE * pData = 0);
	UINT	MaxLevels();
	UINT	MaxFrames();
	void	MaxSizes(UINT & maxw, UINT & maxh);
// Operations
public:
// Implementation
	void BeginBusy(LPCSTR txt = 0);
	void EndBusy();
	UINT ProcessAVI(LPCSTR lpName);
	BOOL ChooseAVI(CString &filename);
	UINT CreateMovie(LPCSTR name,UINT start,UINT stop, UINT rate);
	int	Option(int Id, int op = 0, int val = 0);
	int	ScOption(UINT & a,int Id, int op, int val);
	void OptionString(int Id, LPSTR pStr,  int op = 0);
	void OptionLoadSave(BOOL bSave = FALSE);
	void UpdateTools(UINT which);
	void DlgOptions();
	void DlgLightBox();
	void UpdateDefFolder(const char* pszPathName, int which = 0);
	void GetDefFolder(CString & path, int which = 0, BOOL bName = 0);
	BOOL Busy() { return m_nBusy != 0;};
	BOOL SimpleCursor() { return m_bSimpleCursor != 0;};
	VOID DoIt(int Kind, BOOL bPut, LPVOID lpData, LPSTR lpKey, BOOL bGray);
	UINT ProgressCallBack(UINT kind, UINT code, BYTE * pData);
	BOOL FolderWritable(LPCSTR lpFolder);
	BOOL	ExistCheck(UINT frame, UINT level,BOOL bSingle, UINT & flag);
	CScene * m_pScene;
	BOOL CheckModified();
	UINT SaveLibrary(UINT StartFrame, UINT EndFrame,
					UINT StartLevel, UINT EndLevel, UINT flags);
	UINT LoadLibrary(CString name);

	void ProcessImportMovie(const char* inFilePath,
			bool inDeleteOnFinish = false, float inFrameRateAdjustment = 1.0);
	void ImportAVIFile(LPCSTR szFile);
	BOOL CheckHelp();
	#ifdef _DEBUG
	void ShowLinks();
#endif
protected:
	virtual ~CSketchDoc();

	virtual BOOL OnSaveDocument(const char* pszPathName);
	virtual BOOL OnOpenDocument(const char* pszPathName);
	virtual BOOL SaveModified(); // return TRUE if ok to continue
	BOOL  SaveEmbeddedFiles(LPCSTR pName);	
	BOOL  SaveEmbeddedFile(LPCSTR name, int kind);	// 0 is sound
	void  ProcessEmbeddedFiles();

protected:
	CMyIO	*m_pIO;
//	CScene * m_pScene;
//	CDocView * m_pView;
//	CFlip * m_pFlip;
//	CCanvas * m_pCanvas;
//	CView * m_pCView;
	virtual void DeleteContents();  // deletes scene
//	UINT	m_wSelect;
//	UINT	m_mode;
	UINT m_nBusy;
	int		m_nNeedHelp; // major kludge, I got lazy and we were rushed
						// this is a flag indicating that sound is supposed
						// to be imported once scene is opened
						// only used when creating from avi on inital splash
	BOOL	m_bOptHold;
	BOOL	m_bMayaClosed;
	UINT	m_option_flags;
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
protected:
	void ExportMaya();
	void ExportCamera();
	void ExportJason(UINT StartFrame, UINT EndFrame,
					UINT StartLevel, UINT EndLevel);
	BOOL MakeMaya(UINT ApId, LPCSTR name);
	BOOL CheckPalettes();
	BOOL FromAVI();
	void OptionHelper(BOOL bSave = FALSE);
	virtual BOOL    OnNewDocument();
	void OpenInit(LPCSTR name, BOOL bModified);
	void DoGif(CString name, UINT w, UINT H, UINT d,UINT origw, UINT origh,
			UINT f1, UINT l1, UINT f2, UINT l2, UINT rate, BOOL bLoop);
	void DoAvi(CString name, UINT w, UINT H, UINT d,UINT origw, UINT origh,
			UINT f1, UINT l1, UINT f2, UINT l2, UINT rate,
					int tele, CMySound * pSound, void * pOpts);
	void DoFlv(CString name, UINT w, UINT H, UINT d,UINT origw, UINT origh,
			UINT f1, UINT l1, UINT f2, UINT l2, UINT rate,
					int tele, CMySound * pSound, CString encoder);
	int ImportMayaFile(UINT index);
	void SceneDefaults();
	void ProcessMissingPalette(UINT index);
	void ProcessDifferentPalette(UINT index);
	void ProcessUnusedPalette(UINT index);
	void ProcessMissingTexture(UINT index, UINT pindex);
// Generated message map functions
protected:
	//{{AFX_MSG(CSketchDoc)
	afx_msg void OnImportAVI();
	afx_msg void OnImportFile();
	afx_msg void OnExport();
	afx_msg void OnExtEdit();
//	afx_msg void OnExportAVI();
//	afx_msg void OnExportSWF();
//	afx_msg void OnExportGIF();
	afx_msg void OnPublish();
//	afx_msg void OnExportMovie();
	afx_msg void OnImportMovie();
	afx_msg void OnImportPalette();
	afx_msg void OnExportPalette();
	afx_msg void OnImportCamera();
	afx_msg void OnExportCamera();
	afx_msg void OnExportEmbed();
	afx_msg void OnFileSave();
	afx_msg void OnFileSaveAs();
	afx_msg void OnFileRevert();
	afx_msg void OnScan();
	afx_msg void OnUpdateSave(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRevert(CCmdUI* pCmdUI);
	afx_msg void OnUpdExtEdit(CCmdUI* pCmdUI);
	afx_msg void OnImportMaya();
	afx_msg void OnExportMaya();
	afx_msg void OnEmbOptions();
	afx_msg void OnUpExpEmbed(CCmdUI* pCmdUI);
	afx_msg void OnUpdateOverride(CCmdUI* pCmdUI);
	afx_msg void OnOverride(UINT iid);
	afx_msg void OnFrameNumber();
	
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	UINT m_dwFeatures;
#ifdef FBTPC
	int	m_bTabletUsed;
#endif
	int	m_bLeftScroll;
	int	m_bWinTab;
	int	m_bTabPrimary;
	int	m_wBGMinDark;

	int	m_bLeftHanded;
	int m_bToolsSameColor;
	int	m_bSheetShowBG;
	int	m_bPegShowBG;
	int	m_nPegShowFG;
//	int	m_bPegStack;
	int	m_bShowThumbs;
	int	m_wPegMaxDepth;
	int	m_wPegMinDensity;
	int	m_bSimpleCursor;
	int	m_bXSheetHighlight;

	UINT m_def_width;
	UINT m_def_height;
	UINT m_def_frames;
	UINT m_def_rate;
	UINT m_def_factor;
	UINT m_def_preview;
	UINT m_def_broadcast;
	UINT m_def_kind;
	UINT m_def_levels;
	BOOL	m_bCheckerBG;
	BOOL	m_bHardZoom;
	UINT m_nScanRotate;
	UINT m_nScanFieldSize;
	UINT m_nScanOffset;
	BOOL m_bPegsTop;
	UINT m_bCropPegs;
	UINT m_nPrintOpts;
	BOOL m_bScaleBG;
	UINT m_select_flags;
	UINT m_broadcast;
	CTool * m_pTools;

	UINT	m_wMaxPress;
	UINT	m_wMinPress;
	BOOL	m_bFlipFew;
	BOOL	m_bAutoKeep;
	BOOL	m_bLoopPlay;
	BOOL	m_bOpened;
	BOOL	m_bDoErase;
	UINT	m_vidcapx;
	UINT	m_vidcapy;
	UINT	m_vidcapopts;
	UINT	m_vidcapwht;
	UINT	m_vidcapred;
	UINT	m_vidcapgreen;
	UINT	m_vidcapblue;
	UINT	m_vidcapextent;
	UINT	m_vidcapwidth;
	UINT	m_vidcapheight;
	UINT	m_vidcapgam;
	UINT	m_defaulthold;
	UINT	m_exteditopts;
	UINT	m_nJpeg;		// jpeg quality, 0 is uncomp
	UINT	m_bUseMouths;	// use grid char for mouths 
	UINT	m_bEditScrub;
	UINT	m_scSound;		// scene options
	UINT	m_scQuiet;
	UINT	m_scSnip;
	UINT	m_scBlind;
	UINT	m_scCache;
//	UINT	m_scHold;
	UINT	m_scMRU;
	UINT	m_scTelecine;
	UINT	m_scPrev;
	UINT	m_scNext;
	UINT	m_scPrev1;
	UINT	m_scNext1;
	UINT	m_scPaint;
	UINT	m_scLPaint;
	UINT	m_scEmbOut;
	UINT	m_scEmbIn;
	UINT	m_scAviScale;
	UINT	m_scEmbTrim;
	UINT	m_scApplyFrame;
	BOOL	m_bRecVideo;
	BOOL	m_bRecAudio;
	BOOL	m_bCapCC;
	BOOL	m_bRecPreview;
	BOOL	m_bRecErase;
	UINT	m_nRecSize;
	UINT	m_nRecTimer;
	UINT	m_nRecDelay;
	UINT	m_nRecMaster;
	UINT	m_recordx;
	UINT	m_recordy;
	UINT	m_oflags;
};
#endif
/////////////////////////////////////////////////////////////////////////////

