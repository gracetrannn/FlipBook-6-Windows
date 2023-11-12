#include "mainfrm.h"
#include "mydoc.h"
#ifndef _MYVIEW_H_
#define _MYVIEW_H_
#include <vfw.h> // for drawdib stuff


//#define MAX_RADIUS 50
//#define MAX_MASK (1 + 2 * MAX_RADIUS) * (1 + 2 * MAX_RADIUS)
//#define MAX_BMPS 100

#define TOOL_PENCIL	0
#define TOOL_TRACE	1
#define TOOL_BRUSH	2
#define TOOL_FILL	3
#define TOOL_ZOOM	4
#define TOOL_HAND	5
#define TOOL_EYEDROP	6
#define TOOL_ERASER 7
#define TOOL_SELECT 8
#define TOOL_WAND 9

#ifndef CAPINFODEF
#define CAPINFODEF
typedef struct {
	UINT command;
	UINT color; // scene mode
	UINT gmode; // 0 is gray, 1 color keyed
	UINT process; // 0 is live
	UINT rotate;
	UINT prev; // 1 is prev
	UINT align; // 0 is keyed, 1 is alternate lines
	UINT flags;
	UINT whitekey;
	UINT white;
	UINT gamma;
	UINT red;
	UINT green;
	UINT blue;
	UINT extent;
	CWnd * pParent;
	CWnd * pKeyDlg;
} CAPPACKET, * PCAPPACKET;


typedef struct {
	UINT   infStatus;
	CWnd * infParent;
	UINT   Width;
	UINT   Height;
	UINT   Depth;
	UINT   Size;
	UINT	Counter;
	UINT	avg;  // number of frames to average
	BYTE *  pBI;
	BYTE *  pBits;
} CAPINFO, * PCAPINFO;

#endif

typedef struct {
	UINT flags;
	BYTE * pPage;
	BYTE * pBed;
} SCANPACKET, * PSCANPACKET;


class CVWnd;
class CMMTimer;
class CSheet;
class CLayers;
class CTablet;
class CFloat;
class CBrusher;
class CNewPals;
class CShiftTrace;
class CSketchView : public CScrollView
{
protected: // create from serialization only
	CSketchView();
	DECLARE_DYNCREATE(CSketchView)

// Attributes
public:
	CSketchDoc* GetDocument()
		{
			ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CSketchDoc)));
			return (CSketchDoc*) m_pDocument;
		}
	void Redraw(int flags=0);  // 1 is frame change, 2 is reload
#ifdef FBTPC
	void InitTablet();
#else
	void WinTab();
#endif
	UINT ViewMode(UINT mode = 99);
	BOOL IsUsingPenTool ();
	BOOL IsSelectTool() { return m_wToolType == TOOL_SELECT ? 1 : 0;};
	void SetSelection(UINT frame, UINT level)
		{ m_pFrame->SetSelection(frame,level,frame,level);};
	UINT CurrentFrame() { return m_Frame;};
	UINT CurrentLevel() { return m_Level;};
	UINT VSelectCell(UINT wFrame, UINT Level);
	UINT ASelectCell(UINT wFrame, UINT Level, BOOL bBlow, BOOL bCancel = 0);
	UINT Update(UINT wFrame, UINT Level, UINT Flags = 0);
	UINT CheckUpdate();
	UINT CameraInit(UINT frame, UINT peg, UINT mask);
//	UINT DrawWireFrame(int x, int y);
	UINT Setup(BOOL bClose = 0);
	void DupIt();
	BOOL Modified() { return m_bModified;};
	BOOL ViewActive();// {return m_pFrame && m_pDoc->DocOpened() ? TRUE : FALSE;};
	BOOL CheckModified(BOOL bCanCancel);
	void ForceSave(BOOL bNew) { KeepCell(bNew);};
	void ClearCell();
	UINT Width() { return m_swidth;};
	UINT Height() { return m_sheight;};
	UINT Pitch() { return m_spitch;};
	BYTE * CapBuffer() { return m_pTemp1;};
	BOOL m_bFit;
	int	 m_ZoomSave;
	UINT m_nCounter;
	int m_nNum;
	int m_nDen;
#ifdef FBVER7
	int m_nScale;	// for insitu
	BOOL m_bWasPlaying;
	BOOL ToolKind() {return 0;};
#endif
	int m_nFactor;
	int m_nPreview;
	UINT Layer(int which = -1);
	CLayers * Layers() { return m_pLayers;};
	BOOL MouseDown() { return m_bDown;};
//
//	from docview
//
	void FMyPaint(CDC * pDc);
	void FullPaint(CDC * pDc);
	void ChangeTool();
	void UpdatePrevious();
	BOOL DoZoom(CPoint point, BOOL bOut = 0);
//	UINT Selection();// 1 is selection, 2 is floater
	BOOL HasPaste(UINT code = 0);
	UINT CutCopy(UINT code = 0); // 0 is test, 1 is cut, 2 is copy, 3 is delete
	UINT FillStack(UINT pos);
	void CapKludge(UINT frame);
	BYTE * ScanPacket(BOOL bClear = 0, BOOL bStart = 0);
	void SetColorTrap(UINT which,UINT kind, POINT p1, POINT p2);
	void SetColorTrap() { m_nColorTrap = 0;Redraw();};
	int Playing() { if (!m_Bret) return 2; else return m_bPlaying & 1;};
	UINT CamState() { return m_CamState;};
	void SetClipBoard(BYTE *p, UINT Flags = 0);
	void SaveSceneSettings();
	UINT ShiftTrace(UINT f, UINT l, UINT code);
	void UpdateCursor();
	void ViewMoved();
	BOOL XlatePoint(CPoint& pt);
	void NoScrub();
protected:
	BOOL StartSelection(CPoint point);
	void DrawSelection(BOOL bBlow = 0);
	void DrawCurve(BOOL bBlow = 0);
	void DrawColorTrap();
	UINT SelectionHit(CPoint point);
	UINT SelectionCursor(CPoint point);
	UINT DragSelection(CPoint point);
	void AdjustSelection(CPoint point);
	void Reset();
	void Clear();
	void FClear();
	UINT	Progress(int code = 0, int arg = 0)
			{ return m_pDoc->Progress(code, arg);};
	
	void StartDraw( CPoint point);
	void StopDraw(CPoint point);
//	UINT Setup();
#ifdef FBVER7
	UINT CSetup(UINT flag = 0);
#else
	UINT CSetup();
#endif
	UINT FSetup();
	BOOL XlateRect(RECT& dst, RECT&src, BOOL bUseDst);
	BOOL XlateXPoint(CPoint& pt);
	BOOL XlatePoints(POINT * pt, BOOL bDir = 0, UINT cnt = 0);
	BOOL XlateCPoint(CPoint& pt, BOOL bDir = 0);
	void XlateVCRPoint(CPoint & pt);
	UINT QSetup(BOOL bStart = FALSE);
	UINT SetupFlip();
	void NewBret(UINT Mode);
	UINT SetupScrub(BOOL bSelect = TRUE);
	UINT CSelect(UINT Frame, UINT Level);
	void CRedraw();
	void CStartDraw( CPoint point);
	void CMoveDraw(CPoint point);
	void CStopDraw(CPoint point);
	void CMyPaint(CDC * pDc);
	void DrawPoints(CPoint point, int code);
	void BlowFloat();
	void ApplyFloat();
	void DrawFloat();
	void FloatPaint();
	void FloatOutline();
	void FloatCursor(CPoint point);
	void CMyPrint(CDC * pDc);
	void PrintRegMarks(LPBYTE pBits, UINT w, UINT h, UINT depth);
	void MyCursor(BOOL bOn, HDC hDC = 0);
	void CDocCursor();
	void CamCursor();

	void CapPaint(CDC * pDC);
	void ScanPaint(CDC * pDC);
	void BuildTable();
	void ProcessSample();
	void MergeSample();
	void CapKeyLogic(CDC * pDC);
	UINT m_nColorTrap;
	UINT m_nWarnIfExist;
	UINT m_gradk;
	POINT m_grad1;
	POINT m_grad2;
	UINT m_white;
	UINT m_gamma;
	BYTE m_table[256];
	CAPPACKET m_packet;
	CAPINFO m_info;
	BYTE * m_pTemp1;
	SCANPACKET m_scanpacket;
	void FRedraw();
	void FStartDraw( CPoint point);
	void FMoveDraw(CPoint point);
	void FStopDraw(CPoint point);
//	void FMyPaint(CDC * pDc);
	void FDocCursor();
//	UINT Scale();
	UINT ToolId(UINT which);
	void SetParameter(UINT p);
	void KeepCell(BOOL bNew);
	BOOL PegBG(BOOL bShow = FALSE, BOOL bSet = FALSE);
	BOOL PegFG(BOOL bShow = FALSE, BOOL bSet = FALSE);
	int  PegDepth(int Depth = 0, BOOL bSet = FALSE);
	int  PegDensity(int Density = 0, BOOL bSet = FALSE);
//	virtual void Action(UINT Id);
//	virtual BOOL ActionAllowed(UINT Id);
protected:
	void CameraStart(CPoint point);
	void CameraStop(CPoint point);
	void CameraMove(CPoint point);

	CPoint      m_ptPrev;       // the last mouse pt in the stroke in progress
	UINT m_Frame;
	UINT m_Level;
	UINT m_Index;
//	UINT m_Peg;
	UINT m_EditFrame;
	UINT m_EditLevel;
	int	m_winx;
	int	m_winy;
	CMMTimer * m_timer;
	UINT m_rate;
	UINT m_CamState;
	UINT m_CamTool;
	BOOL	m_bFull;
	RECT	m_rcFull;	// save ofr prev size
	int  m_Bret;		// if zero, normal view
	UINT m_BretFrame;
	UINT m_BretLevel;
	UINT m_Kludge;
	CTablet * m_pTablet;
	CLayers * m_pLayers;
	CFloat * m_pFloat;
	BYTE * m_pClipBoard;
	int m_offx;
	int m_offy;
	UINT m_bShiftTrace;  // on if editing
	CShiftTrace * m_pShiftTrace;
//
//	from docview
//
//	CView * m_pView;
//	CSheet * m_pSheet;
	CMainFrame * m_pFrame;
	CScene * m_pScene;
	CSketchDoc * m_pDoc;
	CVWnd * m_pVideo;
	UINT	m_qwidth;
	UINT	m_qheight;
	UINT	m_swidth;
	UINT	m_sheight;
	UINT	m_spitch;
	UINT	m_nSelect;	// 0:nothing, 1:creating; 2:active
	BOOL	m_bLassoing; // only in nSelect == 1
	UINT	m_nSelectDrag;
	CPoint	m_SelectStart;
	CPoint	m_Curve2;
	CPoint	m_Curve3;
	CPoint  m_Curve4;
	RECT	m_rcSelect;
//	UINT	m_scale;	// scale factor
// from canvas
	COLORREF m_crBack;
//	UINT	m_depth;
//	DWORD	m_pitch;
//	DWORD	m_pegoffset;
	BOOL	m_bPlaying;
	BOOL	m_bIgnore;
	BOOL	m_bDown;
	BOOL	m_bRDown;
	BOOL	m_bPlaySnip;
	BOOL	m_bModified;
	int 	m_play_factor;
//	int		m_prevx;
//	int		m_prevy;
	int		m_cursx;
	int		m_cursy;
	int		m_ocursr;
	int		m_ocursx;
	int		m_ocursy;
	int		m_curcnt;
	int		m_cursr;
	BOOL m_bInModel;
	BOOL	m_bInClient;
	BOOL	m_bCursor;
	BOOL	m_bOldCursor;
	BOOL	m_bErasing;
	UINT 	m_wMaxPressure;
	UINT 	m_wPressure;
	UINT 	m_radius;
	UINT	m_density;
	UINT 	m_maxradius;
	UINT	m_maxdensity;
	UINT	m_color;
	BOOL	m_bLeftHanded;
	BOOL	m_bFastZoom;
//	UINT	m_sbufoffset;
	UINT	m_wToolType;
	UINT	m_nToolSaved; // 1 if space, 2 if alt
	UINT	m_wSaveTool;
	BYTE * 	m_pSkin;
	BYTE * 	m_pBG;
	BYTE * 	m_pDIB;
	BYTE *  m_pBits;

	BYTE *  MakeClipMask();
	CPoint * m_pPoints;
	UINT	m_nPoints;
	CBrusher * m_pBrush;
	int		m_brush_factor;
//	UINT	m_wirew;
//	UINT	m_wireh;
	
//	CBitmap *  m_pWireFrame;
//	BITMAPINFOHEADER m_DIBHdr;
//	BYTE	m_mask[9][MAX_MASK];
//	UINT	m_maskr;
	void	MakeMask();
//	void	DrawMask(int dx, int dy, HPBYTE hpDib, UINT x, UINT y);
//	void	DrawMask(int dx, int dy, UINT x, UINT y);
	void ScaleDib();
	void DrawIt(CPoint point);
	void DrawLine(CPoint point);
	void EchoIt();
	int	 CNumberFill(int count);
	int  FillIt(int x, int y, int kind = 0);
	void UpdateColor(int x1, int y1, int x2, int y2);
	void SetupGray();
	void SetupStack();
	void SetupBetween();
	void SetupColor();
#ifdef FBVER7
	void SetupOverlay();
#endif
	void UpdateGray(int x1, int y1, int x2, int y2);
	void ApplyLayer(UINT factor,CNewPals * pPals,BYTE * pLayer);
	void ApplyInk(UINT factor,UINT Frame,UINT Level, BOOL bHold=1);
	void MyDraw(HDC hDC, int sdx, int sdy, int dx, int dy, int dw, int dh);
	void DoSoft8(int dx, int dy, int dh, int dw);
	void DoSoft24(int dx, int dy, int dh, int dw);
	void DoHard8(int dx, int dy, int dh, int dw);
	void DoHard24(int dx, int dy, int dh, int dw);
	HDRAWDIB m_hdd;
//	UINT	m_minzx;
//	UINT	m_maxzx;
//	UINT	m_minzy;
//	UINT	m_maxzy;
	UINT	m_znum;
	UINT	m_zden;
	UINT	m_zbits;
	BOOL	m_zb;
	UINT	m_zx;
	UINT	m_zy;
	UINT	m_zw;
	UINT	m_zh;
	UINT	m_zsize;
	BYTE * m_pZoom;
//	UINT	m_minz;
//	UINT	m_maxz;
//	BOOL	m_bbb;
//	BOOL	m_bDirty;
//	BOOL	m_bChange;
	BOOL	m_bTablet;
	int     m_LBState;
	UINT	m_toolflags;	// 4 is rad, 8 is den (if pencil)
//	int		m_minx;
//	int		m_miny;
//	int		m_maxx;
//	int		m_maxy;
// from flip
	UINT	m_count;
	UINT	m_flip_depth;
	UINT	m_origx, m_origy;
	UINT	m_origf;
	UINT	m_pegx, m_pegy;
	BOOL	m_bDir;
	BOOL	m_bFirst;
	UINT	m_frames[10];
// Operations
public:
	void Adjust(UINT Num = 100, UINT den = 0, CPoint * pCenter = 0);
	void CenterIt(CRect &rect);
	void StartTimer(BOOL bStart = FALSE);
	void PauseIt(UINT frame);
	void DoGrid(LPBITMAPINFOHEADER lpBI);
// Implementation
public:
	virtual ~CSketchView();
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual void OnInitialUpdate();
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView,
					CView* pDeactiveView);
	afx_msg void OnVCRExit();
	afx_msg	BOOL OnEraseBkgnd(CDC* pDC);
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
// from mainfrm
	afx_msg void OnReturn();
	afx_msg void OnBack();
protected:
	// Printing support
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnPrepareDC(CDC* pDC, CPrintInfo* pInfo = 0);
	void SetupPrint(CDC * pDC);
	void DrawCurve(CDC * pDC, UINT frame);
	void MyDrawBox(CDC * pDC, LPRECT lpRect);
	void MyDrawObj(CDC * pDC, int which, CPoint p1, CPoint p2);
	void MyDrawObj(CDC * pDC, int which, int x1, int y1, int x2, int y2);
	BOOL WtIt(UINT code, CPoint &point);
	void OnUpdateTool(CCmdUI* pCmdUI, UINT Id);
	void OnUpdateVCR(CCmdUI * pc, UINT id);
	void OnUpdateRate(CCmdUI * pc, UINT id);
	void OnZoom(UINT num, UINT denom);
	void OnUpdZoom(CCmdUI* pCmdUI, int num, int denom);
	void OnPreview(UINT factor);
	void OnUpdPreview(CCmdUI* pCmdUI, UINT factor);

	void PrintCel(CDC* pDC, CPrintInfo* pInfo);
	void PrintFrame(CDC* pDC, CPrintInfo* pInfo);
	void PrintSheet(CDC* pDC, CPrintInfo* pInfo);
	void PrintHeader(CDC * pDC, LPRECT lpRect, int w , int h, int f, int l);
	void PrintFooter();
	void OnPrint(CDC* pDC, CPrintInfo* pInfo);

	void CapSetup();
#ifdef FBTPC
	void ProcessTabletCode(UINT code);
#endif
// Generated message map functions
protected:
	//{{AFX_MSG(CSketchView)
	afx_msg LRESULT OnDoRealize(WPARAM wParam, LPARAM lParam);  // user message
	afx_msg void OnUpdateActionFill(CCmdUI* pCmdUI);
	afx_msg void OnActionFill();
	afx_msg void OnUpdateActionFlip(CCmdUI* pCmdUI);
	afx_msg void OnActionFlip();
	afx_msg void OnUpdateActionMatte(CCmdUI* pCmdUI);
	afx_msg void OnActionMatte();
	afx_msg void OnUpdateActionSpeckle(CCmdUI* pCmdUI);
	afx_msg void OnActionSpeckle();
	afx_msg void OnSize(UINT nType, int cx, int cy);
//	afx_msg void OnMove(int x, int y);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
//	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
//	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
//	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
//	afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
//	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnFunctionKey(UINT id);
	afx_msg void OnPositionKey(UINT id);
	afx_msg void OnCameraKey(UINT id);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnVCR(UINT id);
afx_msg void OnUpdateVCRHome(CCmdUI * pc) {OnUpdateVCR(pc,ID_VCR_HOME);};
afx_msg void OnUpdateVCRBack(CCmdUI * pc) {OnUpdateVCR(pc,ID_VCR_BACK);};
afx_msg void OnUpdateVCRStop(CCmdUI * pc) {OnUpdateVCR(pc,ID_VCR_STOP);};
afx_msg void OnUpdateVCRPlay(CCmdUI * pc) {OnUpdateVCR(pc,ID_VCR_PLAY);};
afx_msg void OnUpdateVCREnd (CCmdUI * pc) {OnUpdateVCR(pc,ID_VCR_END);};
afx_msg void OnUpdateVCRFStep(CCmdUI * pc) {OnUpdateVCR(pc,ID_VCR_FSTEP);};
afx_msg void OnUpdateVCRBStep(CCmdUI * pc) {OnUpdateVCR(pc,ID_VCR_BSTEP);};
afx_msg void OnUpdateRateUp(CCmdUI * pc) {OnUpdateRate(pc,ID_RATE_UP);};
afx_msg void OnUpdateRateDown(CCmdUI * pc) {OnUpdateRate(pc,ID_RATE_DOWN);};
	afx_msg void OnRate(UINT id);
//	afx_msg void OnUpdateTool(CCmdUI* pCmdUI, UINT Id);
afx_msg void OnUpdatePen(CCmdUI* pCmdUI)   {OnUpdateTool(pCmdUI,ID_TOOL_TRACE);};
afx_msg void OnUpdatePencil(CCmdUI* pCmdUI){OnUpdateTool(pCmdUI,ID_TOOL_PENCIL);};
afx_msg void OnUpdateEraser(CCmdUI* pCmdUI){OnUpdateTool(pCmdUI,ID_TOOL_ERASER);};
afx_msg void OnEraserBox() {};
afx_msg void OnUpdateSelect(CCmdUI* pCmdUI){OnUpdateTool(pCmdUI,ID_TOOL_SELECT);};
afx_msg void OnUpdateZoom(CCmdUI* pCmdUI){OnUpdateTool(pCmdUI,ID_TOOL_ZOOM);};
afx_msg void OnUpdateHand(CCmdUI* pCmdUI){OnUpdateTool(pCmdUI,ID_TOOL_HAND);};
afx_msg void OnUpdateBucket(CCmdUI* pCmdUI){OnUpdateTool(pCmdUI,ID_TOOL_FILL);};
afx_msg void OnUpdateBrush(CCmdUI* pCmdUI){OnUpdateTool(pCmdUI,ID_TOOL_BRUSH);};
afx_msg void OnUpdateEyeDrop(CCmdUI* pCmdUI){OnUpdateTool(pCmdUI,ID_TOOL_EYEDROP);};
afx_msg void OnUpdateWand(CCmdUI* pCmdUI){OnUpdateTool(pCmdUI,ID_TOOL_WAND);};
//	afx_msg void OnUpdatePencil(CCmdUI* pCmdUI, UINT Id);
//	afx_msg void OnUpdateEraser(CCmdUI* pCmdUI, UINT Id);
	afx_msg void OnTool(UINT Id);

//	afx_msg void OnReturn();
//	afx_msg void OnBack();
	afx_msg void OnSpace();// { OnPositionKey(ID_KEY_RIGHT);};
	afx_msg void OnLoop();
	afx_msg void OnUpdateLoop(CCmdUI* pCmdUI);
	afx_msg void OnEditUndo();
	afx_msg void OnUpdateUndo(CCmdUI* pCmdUI);
	afx_msg void OnEditRedo();
	afx_msg void OnUpdateRedo(CCmdUI* pCmdUI);
	afx_msg void OnEditClear();
	afx_msg void OnUpdateClear(CCmdUI* pCmdUI);
	afx_msg void OnEditRevert();
	afx_msg void OnUpdateRevert(CCmdUI* pCmdUI);
	
	afx_msg void OnOptions();
	afx_msg void OnLightBox();

	afx_msg void OnOptStack();
	afx_msg void OnUpdateStack(CCmdUI* pCmdUI);
	afx_msg void OnOptFG();
	afx_msg void OnUpdateFG(CCmdUI* pCmdUI);
	afx_msg void OnOptBG();
	afx_msg void OnUpdateBG(CCmdUI* pCmdUI);
	afx_msg void OnOptKeep();
	afx_msg void OnUpdateKeep(CCmdUI* pCmdUI);
	afx_msg void OnOptSound();
	afx_msg void OnUpdateOptSound(CCmdUI* pCmdUI);
	afx_msg void OnOptThumbs();
	afx_msg void OnUpdateThumbs(CCmdUI* pCmdUI);
	afx_msg void OnOptComposite();
	afx_msg void OnUpdateComposite(CCmdUI* pCmdUI);

	afx_msg void OnUpdateOutside(CCmdUI* pCmdUI);
	afx_msg void OnEditOutside();

	afx_msg void OnViewMode();
	afx_msg void OnUpdateMode(CCmdUI* pCmdUI);
	afx_msg void OnViewSheet();
	afx_msg void OnUpdateSheet(CCmdUI* pCmdUI);
	afx_msg void OnViewTiming();
	afx_msg void OnUpdateTiming(CCmdUI* pCmdUI);
	afx_msg void OnViewPalette();
	afx_msg void OnUpdatePalette(CCmdUI* pCmdUI);
	afx_msg void OnViewLipSync();
	afx_msg void OnUpdateLipSync(CCmdUI* pCmdUI);
	afx_msg void OnViewPuck();
	afx_msg void OnUpdatePuck(CCmdUI* pCmdUI);
	afx_msg void OnViewZoomer();
	afx_msg void OnUpdateZoomer(CCmdUI* pCmdUI);
	afx_msg void OnViewSubPalette();
	afx_msg void OnUpdateSubPalette(CCmdUI* pCmdUI);
	afx_msg void OnViewCamera();
	afx_msg void OnUpdateCamera(CCmdUI* pCmdUI);
	afx_msg void OnViewCamera100();
	afx_msg void OnUpdateCamera100(CCmdUI* pCmdUI);
	afx_msg void OnViewCamera50();
	afx_msg void OnUpdateCamera50(CCmdUI* pCmdUI);
	afx_msg void OnViewCamera25();
	afx_msg void OnUpdateCamera25(CCmdUI* pCmdUI);
	afx_msg void OnViewCameraEnlarge();
	afx_msg void OnUpdateCameraEnlarge(CCmdUI* pCmdUI);
	afx_msg void OnViewCameraGrid();
	afx_msg void OnUpdateCameraGrid(CCmdUI* pCmdUI);
	afx_msg void OnViewCameraGridCenter();
	afx_msg void OnUpdateCameraGridCenter(CCmdUI* pCmdUI);
	afx_msg void OnViewSound();
	afx_msg void OnUpdateSound(CCmdUI* pCmdUI);
#if MAC
	afx_msg void OnViewTools();
	afx_msg void OnUpdateTools(CCmdUI* pCmdUI);
	afx_msg void OnViewCamTools();
	afx_msg void OnUpdateCamTools(CCmdUI* pCmdUI);
#endif
	afx_msg void OnViewVideo();
	afx_msg void OnViewFull();
	afx_msg void OnUpdateVideo(CCmdUI* pCmdUI);
	afx_msg void OnZoom25() { OnZoom(25,100);};
	afx_msg void OnUpdZoom25(CCmdUI* pCmdUI) { OnUpdZoom(pCmdUI, 25,100);};
	afx_msg void OnZoom33() { OnZoom(40,120);};
	afx_msg void OnUpdZoom33(CCmdUI* pCmdUI) { OnUpdZoom(pCmdUI,40,120);};
	afx_msg void OnZoom50() { OnZoom(50,100);};
	afx_msg void OnUpdZoom50(CCmdUI* pCmdUI) { OnUpdZoom(pCmdUI,50,100);};
	afx_msg void OnZoom100() { OnZoom(100,100);};
	afx_msg void OnUpdZoom100(CCmdUI* pCmdUI) { OnUpdZoom(pCmdUI,100,100);};
	afx_msg void OnZoom200() { OnZoom(200,100);};
	afx_msg void OnUpdZoom200(CCmdUI* pCmdUI) { OnUpdZoom(pCmdUI,200,100);};
	afx_msg void OnZoom300() { OnZoom(300,100);};
	afx_msg void OnUpdZoom300(CCmdUI* pCmdUI) { OnUpdZoom(pCmdUI,300,100);};
	afx_msg void OnZoom400() { OnZoom(400,100);};
	afx_msg void OnUpdZoom400(CCmdUI* pCmdUI) { OnUpdZoom(pCmdUI,400,100);};
	afx_msg void OnZoom800() { OnZoom(800,100);};
	afx_msg void OnUpdZoom800(CCmdUI* pCmdUI) { OnUpdZoom(pCmdUI,800,100);};
	afx_msg void OnZoomFit() { OnZoom(0,0);};
	afx_msg void OnUpdZoomFit(CCmdUI* pCmdUI) { OnUpdZoom(pCmdUI,0,0);};
	afx_msg void OnPreview1() { OnPreview(1);};
	afx_msg void OnUpdPreview1(CCmdUI* pCmdUI) { OnUpdPreview(pCmdUI,1);};
	afx_msg void OnPreview2() { OnPreview(2);};
	afx_msg void OnUpdPreview2(CCmdUI* pCmdUI) { OnUpdPreview(pCmdUI,2);};
	afx_msg void OnPreview3() { OnPreview(3);};
	afx_msg void OnUpdPreview3(CCmdUI* pCmdUI) { OnUpdPreview(pCmdUI,3);};
	afx_msg void OnPreview4() { OnPreview(4);};
	afx_msg void OnUpdPreview4(CCmdUI* pCmdUI) { OnUpdPreview(pCmdUI,4);};
	afx_msg void OnTimerFire();
	afx_msg void OnUpdateScan(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCapture(CCmdUI* pCmdUI);
	afx_msg void OnProperties();
	afx_msg void OnUpdateProperties(CCmdUI * pCmdUI);
	afx_msg void OnMakeModel();
	afx_msg void OnUpdateMakeModel(CCmdUI* pCmdUI);
	afx_msg void OnModel();
	afx_msg void OnUpdateModel(CCmdUI* pCmdUI);
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);  // user message
	afx_msg void MyPrint();
	afx_msg void OnGrab();
	afx_msg void OnCapture();
	afx_msg void OnRecord();
	afx_msg void OnLipDefault();
	afx_msg void OnLipScene();
	afx_msg void OnMyPaint();
	afx_msg void OnClear() { CutCopy(3);};
	afx_msg LRESULT OnWTPacket(WPARAM, LPARAM);
	afx_msg LRESULT OnFlickMsg(WPARAM, LPARAM);
	afx_msg LRESULT OnTabletStatusMsg(WPARAM, LPARAM);
#ifdef FBTPC
	afx_msg LRESULT OnTabletMsg(WPARAM, LPARAM);
#endif
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CFont m_PrinterFont;
	CFont * m_pOldFont;
	CSize m_PrintSize;
	int m_nPrintKind;
	int m_nPrintFit;
	int m_nPrintCount;
	BOOL m_bPrintDown;
	BOOL m_bPrintMarks;
	int m_ldots;	// horiz dots per level 
	int m_fdots;	// vert dots per frame
	int m_mdots;	// hroz dost gor margin (frame nbr)
	int m_hdots;	// vert dots for column header
	int m_hhdots;	// vert dots for scene header
	int m_FramesPerPage;
	int m_LevelsPerPage;
	int m_nPages;
friend class CShiftTrace;
#ifdef _DEBUG
	void OnDebug();
#endif
};
#endif
/////////////////////////////////////////////////////////////////////////////

