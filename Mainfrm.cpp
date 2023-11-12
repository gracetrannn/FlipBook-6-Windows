// mainfrm.cpp : implementation of the CMainFrame class
//
#include "stdafx.h"
#if _MFC_VER < 0x0320
// IMPORTANT : Conditional compilation directives are
// necessary to maintain compatibility with future versions of MFC
#include <afxpriv.h>
#endif
#include "afxadv.h"
#include "sketch.h"
#include "sheet.h"
//#include "griddlg.h"
#include "ctoolbox.h"
#include "ccamtool.h"
#include "dialogs.h"
#include "tooldlg.h"
#include "toolbar1.h"
#include "mydlgbar.h"
#include "palette.h"
#include "lipsync.h"
#include "puck.h"
#include "subpal.h"
#include "camdlg.h"
#include "myview.h"
#include "mainfrm.h"
#include "cscene.h"
#include "mysound.h"
#include "sceneopt.h"
#include "camera.h"
#include "clayers.h"
#include "clevtbl.h"
#include <windowsx.h>
#include "scan.h"
#include "cnewpals.h"
#include "mylib.h"
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif
#define BPP 4 // 1 for gray
#define TIMING_NAMES
/////////////////////////////////////////////////////////////////////////////
// CMainFrame
#define MIN_WIDTH 26 // min column width
#define WRAPTOOL 16

#define SOUND_WIDTH 48

#define HEIGHT 60  // thumbnail height
/*
class CMyToolBar : public CToolBar
{
public:
	void SetRange(UINT max);
	void SetPosition(UINT pos);
	BOOL LoadBitmap32(UINT id);
	CSliderCtrl m_slider;
//	CSelectionSliderCtrl m_slider;
	CButton m_goto;
	CButton m_start;
	CButton m_stop;
protected:
	UINT m_pos;
	UINT m_code; // 0 is normal, 1 is shift down, 2 is shift up
	afx_msg void OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar );
	DECLARE_MESSAGE_MAP()
};
*/

class CThumbs 
{
public:
	CThumbs(UINT frames, UINT levels, UINT size);
	~CThumbs();
	BYTE * Buf(UINT Frame, UINT Level);
	UINT Count();
	int	Insert(UINT Start, UINT Count);
	int	Delete(UINT Start, UINT Count);
	int	Append(UINT Count);
	int InsertLevels(UINT Start, UINT count);
	int DeleteLevels(UINT Start, UINT count);
	int	AppendLevels(UINT Count);
protected:
	UINT m_grain;
	UINT m_size;
	UINT m_frames;
	UINT m_levels;
	LPBYTE * m_pFrames;
};

#if _MFC_VER < 0x0320
// IMPORTANT : Conditional compilation directives are
// necessary to maintain compatibility with future versions of MFC
/////////////////////////////////////////////////////////////////////////// 
class _AFX_BARINFO : public CObject
{
public:
// Implementation
   _AFX_BARINFO();
// Attributes
   UINT m_nBarID;
   BOOL m_bVisible;
   BOOL m_bFloating;
   BOOL m_bHorz;
   BOOL m_bDockBar;
   CPoint m_pointPos;
   CPtrArray m_arrBarID;
   CControlBar* m_pBar;
   virtual void Serialize(CArchive& ar);
   BOOL LoadState(LPCTSTR lpszProfileName, int nIndex);
   BOOL SaveState(LPCTSTR lpszProfileName, int nIndex);
};
#endif


IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_SYSCOMMAND()
//	ON_WM_ACTIVATE()
	ON_WM_CREATE()
	ON_WM_CLOSE()
//    ON_WM_SIZE()
    ON_WM_MOVE()
//	ON_MESSAGE(WM_DISPLAYCHANGE, OnDisplayChange)
//	ON_WM_ENTERMENULOOP()
//	ON_WM_EXITMENULOOP()
	ON_COMMAND_EX(ID_VIEW_VCR, OnViewBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_VCR, OnUpdateBarMenu)
#if !MAC
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOOLS, OnUpdateControlBarMenu)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CAM_TOOLS, OnUpdateControlBarMenu)
	ON_COMMAND_EX(ID_VIEW_TOOLS, OnBarCheck)
	ON_COMMAND_EX(ID_VIEW_CAM_TOOLS, OnBarCheck)
#endif
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_PASTEREVERSED, OnEditPasteReverse)
	ON_COMMAND(ID_EDIT_INS, OnEditInsert)
	ON_COMMAND(ID_EDIT_DEL, OnEditDelete)
	ON_COMMAND(ID_EDIT_BLANK, OnEditBlank)
	ON_COMMAND(ID_EDIT_APPEND, OnEditAppend)
	ON_COMMAND(IDC_SLD_START, OnSliderStart)
	ON_COMMAND(IDC_SLD_STOP, OnSliderStop)
	ON_UPDATE_COMMAND_UI(IDC_SLD_PLAY, OnUpdatePlay)
	ON_COMMAND(IDC_SLD_PLAY, OnSliderPlay)
	ON_COMMAND(IDC_SLD_BEGIN, OnSliderBegin)
	ON_COMMAND(IDC_SLD_BACK, OnSliderBack)
	ON_COMMAND(IDC_SLD_FWD, OnSliderFwd)
	ON_COMMAND(IDC_SLD_END, OnSliderEnd)
	ON_COMMAND(ID_VIEW_LAYER, OnLayer)
	ON_COMMAND(ID_OPT_CUST_TB, OnCustomizeToolBar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LAYER, OnUpdateLayer)
//	ON_UPDATE_COMMAND_UI(ID_VIEW_LAY_COMBO, OnUpdateLayer)
	ON_COMMAND(ID_FILE_IMPWAVE, OnImpWave)
	ON_UPDATE_COMMAND_UI(ID_FILE_IMPWAVE, OnUpdateImpWave)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdatePaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTEREVERSED, OnUpdatePasteRev)
	ON_UPDATE_COMMAND_UI(ID_EDIT_INS, OnUpdateInsert)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DEL, OnUpdateDelete)
	ON_UPDATE_COMMAND_UI(ID_EDIT_BLANK, OnUpdateBlank)
	ON_UPDATE_COMMAND_UI(ID_EDIT_APPEND, OnUpdateAppend)
	ON_WM_RENDERFORMAT()
	ON_WM_RENDERALLFORMATS()
//	ON_UPDATE_COMMAND_UI(ID_MAGIC_11, OnUpdateMagic11)
//	ON_UPDATE_COMMAND_UI(ID_MAGIC_12, OnUpdateMagic11)
//	ON_COMMAND(ID_MAGIC_11, OnMagic11)
//	ON_COMMAND(ID_MAGIC_12, OnMagic12)
	ON_NOTIFY(TBN_DROPDOWN, AFX_IDW_TOOLBAR, OnTBN_Drop)
	ON_UPDATE_COMMAND_UI(ID_TOOL_CAM_PAN, OnUpdatePan)
	ON_UPDATE_COMMAND_UI(ID_TOOL_CAM_ZOOM, OnUpdateZoom)
	ON_UPDATE_COMMAND_UI(ID_TOOL_CAM_ROTATE, OnUpdateRotate)
	ON_UPDATE_COMMAND_UI(ID_TOOL_CAM_BLUR, OnUpdateBlur)
	ON_UPDATE_COMMAND_UI(ID_TOOL_CAM_ALPHA, OnUpdateAlpha)
	ON_COMMAND_RANGE(ID_TOOL_CAM_PAN, ID_TOOL_CAM_PREV, OnCamTool)
	ON_COMMAND(ID_TOOL_CAM_DLG, OnCamDialog)
	ON_UPDATE_COMMAND_UI(ID_TOOL_CAM_DALL, OnUpdateDALLKeys)
	ON_UPDATE_COMMAND_UI(ID_TOOL_CAM_DTHIS, OnUpdateDTHISKey)
	ON_UPDATE_COMMAND_UI(ID_TOOL_CAM_NEXT, OnUpdateNextKey)
	ON_UPDATE_COMMAND_UI(ID_TOOL_CAM_PREV, OnUpdatePrevKey)
	ON_COMMAND(ID_FRAME_ENTER_KEY, OnReturn)
    ON_WM_KEYDOWN()
//	ON_WM_INITMENUPOPUP()
	ON_COMMAND_RANGE(ID_VIEW_LIB_0, ID_VIEW_LIB_0 + 9, OnViewLib)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_LIB_0, ID_VIEW_LIB_0+9,OnUpdateViewLib)
	ON_COMMAND(ID_LIB_LOAD, OnLibLoad)
	ON_COMMAND(ID_LIB_SAVE, OnLibSave)
	ON_COMMAND(ID_LIB_UNLOAD, OnLibUnloadAll)
	ON_COMMAND_RANGE(ID_LIB_UNLOAD_0,ID_LIB_UNLOAD_0 + 9,OnLibUnload)
	ON_COMMAND_RANGE(ID_OPT_ICON_LARGE,ID_OPT_ICON_TINY,OnIconSize)
	ON_UPDATE_COMMAND_UI_RANGE(ID_OPT_ICON_LARGE, ID_OPT_ICON_TINY,
				OnUpdateIconSize)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// arrays of IDs used to initialize control bars

// toolbar buttons - IDs are command buttons

#ifdef OLDSLIDE
#define SLIDER_COUNT 11 // number of images
#define SLIDER_GOTO 16 // field offsets
#define SLIDER_LEFT 18
#define SLIDER_SLIDER 19
#define SLIDER_RIGHT 20
static UINT BASED_CODE slider[] =
{
	// same order as in the bitmap 'slider.bmp'
	ID_VCR_EXIT,
	ID_SEPARATOR,
	ID_VCR_HOME, // rewind
	ID_VCR_BACK,
	ID_VCR_BSTEP,
	ID_VCR_STOP,
	ID_VCR_FSTEP,
	ID_VCR_PLAY,
	ID_VCR_END,
	ID_SEPARATOR,
	ID_VCR_LOOP,
	ID_SEPARATOR,
	ID_SEPARATOR,
	ID_RATE_UP,
	ID_RATE_DOWN,
	ID_SEPARATOR,
	ID_SEPARATOR,			// for goto
	ID_SEPARATOR,
	ID_SEPARATOR,//SLIDER_START,
	ID_SEPARATOR,           // for slider
	ID_SEPARATOR,//SLIDER_STOP
};
#else
#define SLIDER_COUNT 7 // number of images
#define SLIDER_GOTO 11 // field offsets
#define SLIDER_LEFT 13
#define SLIDER_SLIDER 14
#define SLIDER_RIGHT 15
static UINT BASED_CODE slider[] =
{
	// same order as in the bitmap 'slider.bmp'
	ID_VCR_HOME, // rewind
	ID_VCR_STOP,
	ID_VCR_PLAY,
	ID_VCR_END,
	ID_SEPARATOR,
	ID_VCR_LOOP,
	ID_SEPARATOR,
	ID_SEPARATOR,
	ID_RATE_UP,
	ID_RATE_DOWN,
	ID_SEPARATOR,
	ID_SEPARATOR,			// for goto
	ID_SEPARATOR,
	ID_SEPARATOR,//SLIDER_START,
	ID_SEPARATOR,           // for slider
	ID_SEPARATOR,//SLIDER_STOP
};
#endif
static UINT BASED_CODE indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_SEPARATOR,           // status line indicator
	ID_SEPARATOR,           // status line indicator
	ID_SEPARATOR,           // status line indicator
	ID_SEPARATOR,           // status line indicator
	ID_SEPARATOR,           // status line indicator
};

#if MAC_AQUA_TEST
static COLORREF crLevelSel = RGB(140,155,179);
#else
static COLORREF crLevelSel = RGB(90,150,125);
#endif
static COLORREF crLevelTxt = RGB(0,0,0);
static COLORREF crNormal = RGB(255,255,255);
#if MAC_AQUA_TEST
static COLORREF crNormalSel = RGB(181,213,254);
#else
static COLORREF crNormalSel = RGB(150,150,255);
#endif
static COLORREF crNormalTxt  = RGB(255,0,0);
static COLORREF crStack = RGB(255,255,128);
static COLORREF crStackSel = RGB(200,200,255);
static COLORREF crStackTxt  = RGB(255,0,0);
static COLORREF crOpen = RGB(128,255,128);
static COLORREF crOpenSel = RGB(0,255,180); 
static COLORREF crOpenTxt  = RGB(255,0,0);
/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
//	DPF("$1");
	DPF("Mainfrm construct");
	m_pStatusBar = new CStatusBar;
	m_pSheet = new CSheet;
	m_pTools = 0;//new ctoolbox;
	m_pCamTools = 0;//new ccamtool;
	m_pPalette = new CPaletteDlg;
	m_pSubPal = new CSubPalDlg;
	m_pIconMenu = 0;
	m_pOverrideMenu = 0;
//	m_pScan = new CScan;
	m_pHdrIcons = new BYTE[40 + 8 * (3 * 16 * 16)]; // extra for temp
	m_pScan = 0;
	m_tbsize = 0;
	m_oheight = 0;
//	m_owidth = 0;
	CSketchApp* pApp = (CSketchApp *)AfxGetApp();
	if (pApp->IsLite())
		m_nSoundCols = 1;
	else if (pApp->IsPro())
		m_nSoundCols = 3;
	else
		m_nSoundCols = 2;
	if (pApp->CanDoCamera())
		{
		m_pCamera = new camdlg;
		m_pCamera->m_hWnd = 0;
		}
	else
		m_pCamera = 0;
	if (pApp->IsLite() && !pApp->IsAnimate())
		m_pLipSync = 0;
	else
		{
		m_pLipSync = new CLipSyncDlg;
		}
	if (pApp->IsAnimate())
		{
		m_pPuckDlg = new CPuckDlg;
		m_pZoomerDlg = new CZoomerDlg;
		}
	else
		{
		m_pPuckDlg = 0;
		m_pZoomerDlg = 0;
		}
	m_pLibs = new CMyLibDlg *[10];
	m_nLibraries = 0;
#ifdef _DISNEY
	m_pToolBar = 0;
#else
	m_pToolBar = new CToolBar1;
#endif
	m_pSlider = new CMyDialogBar();
	m_pDoc = 0;
	m_pScene = 0;
	m_pSheet->m_hWnd = 0;
//	m_pTools->m_hWnd = 0;
	m_pSlider->m_hWnd = 0;
	m_pPalette->m_hWnd = 0;
	m_pSubPal->m_hWnd = 0;
//	m_pScan->m_hWnd = 0;
	m_vFrame = 0;
	m_Frames = 0;
	m_MaxFrame = 0;
	m_Levels = 0;
	m_pThumbs = 0;
	m_pDib = 0;
	m_pSound = 0;
	m_nViewDirty = 0;
	m_bDirtyThumbs = 0;
	m_nDoubleDigit = 0;
	m_bCanDoSound = 0;
	m_bShowSound = 0;
#if MAC
	m_bShowTools = 0;
	m_bShowCamTools = 0;
#endif
	m_bShowSheet = 0;
	m_bShowSlider = 0;
	m_bShowPalette = 0;
	m_bShowLipSync = 0;
	m_bShowPuck = 0;
	m_bShowZoomer = 0;
	m_bShowSubPal = 0;
	m_bShowCamera = 0;
	m_bDisPal = 0;
	m_bDisPal2 = 0;
	m_nDisPal = 0;
	m_bNeedExtClip = 0;
	m_bTiming = 0;
	m_bXSheetHighlight = 1;
	m_TimingLevel = 1;
	m_buddy_count = 0;
	m_onion_count = 0;
	m_stack_level = 1;
	m_stack_frame = 0;
	m_stack_flag = 0;
	m_MyClipCell = RegisterClipboardFormat("DigiCelClipBoard");
//	m_MyClipImage = RegisterClipboardFormat("DigiCelClipBrdImg");
	EnableToolTips(TRUE);
	GetHeaderIcons();
}

CMainFrame::~CMainFrame()
{
	DPF("mainfrm destruct");
	delete m_pSheet;
	delete m_pTools;
	delete m_pCamTools;
//	m_pTools->DestroyWindow();
	delete m_pPalette;
	delete m_pLipSync;
	UINT i;
	for (i = 0; i < m_nLibraries; i++)
		{
		delete m_pLibs[i];
		}
	delete m_pIconMenu;
	delete m_pOverrideMenu;
	delete []m_pLibs;
	delete m_pPuckDlg;
	delete m_pZoomerDlg;
	delete m_pSubPal;
	delete m_pScan;
	delete m_pCamera;
	delete m_pToolBar;
	delete m_pSlider;
	delete m_pStatusBar;
	m_pDoc = 0;
	delete m_pThumbs;
	m_pThumbs = 0;
	delete [] m_pDib;
	m_pDib = 0;
	delete m_pSound;
	m_pSound = 0;
	delete m_pHdrIcons;
	m_pHdrIcons = 0;
//#ifdef TABLET
//	WtExit();
//#endif
DPF("end destruct");
}

void CMainFrame::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	DPF("on activate,%d,%d",nState, bMinimized);
	if (m_pSheet && m_bShowSheet)
		m_pSheet->ShowWindow(SW_SHOW);
}

BOOL CMainFrame::EnableSheet(int code)
{
	if (!m_pSheet)
		return 0;
	if (code == 0)
		m_pSheet->EnableWindow(0);
	else if (code == 1)
		m_pSheet->EnableWindow(1);
	UINT mask = WS_DISABLED;
	UINT z = ::GetWindowLong(m_pSheet->m_hWnd,GWL_STYLE);
	return z & mask ? 0 : 1;
}

void CMainFrame::OnTBN_Drop(NMHDR *lpnm, LRESULT* pResult)
{
	DPF("notify drop,%d",lpnm->code);
	if (lpnm->code == TBN_DROPDOWN)
		{
		int item = ((LPNMTOOLBAR)lpnm)->iItem;
		if (item == ID_OPT_FG)
			{
			PostMessage(WM_COMMAND,ID_OPT_LIGHTBOX);
			}
		else if (item == ID_TOOL_PENCIL)
			{
//			m_pTools->DoDialog();
			}
		}
}
#ifdef NOT_USED
void CMainFrame::OnEnterMenuLoop(BOOL bIsTrackPopupMenu)
{
	DPF("entermenu");
	CFrameWnd::OnEnterMenuLoop(bIsTrackPopupMenu);
//	PostMessage(WM_COMMAND, ID_KEY_F1,0);
//	CSketchView * pView = (CSketchView *)GetActiveView();
//	if (pView)
//		pView->SetFocus();
}
void CMainFrame::OnExitMenuLoop(BOOL bIsTrackPopupMenu)
{
	DPF("exitmenu");
	CFrameWnd::OnExitMenuLoop(bIsTrackPopupMenu);
}
#endif
BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_SYSKEYDOWN && pMsg->wParam == VK_MENU)
		{
		DPF("got alt");
//		((CMainFrame *)AfxGetMainWnd())->m_hWndBeforeMenu = ::GetFocus();
		CSketchView * pView = (CSketchView *)GetActiveView();
		if (pView)
		pView->SetFocus();
//		AfxGetMainWnd()->SetFocus();
        }
/*
    if( pMsg->message == WM_KEYDOWN )
    {
        if(pMsg->wParam == VK_RETURN
            || pMsg->wParam == VK_ESCAPE )
        {
            ::TranslateMessage(pMsg);
            ::DispatchMessage(pMsg);
            return TRUE;                    // DO NOT process further
        }
    }
*/
    return CFrameWnd::PreTranslateMessage(pMsg);
}	

void PutInt(LPCSTR section, LPCSTR key, int v)
{
//DPF("put,%s,%s,%d",section,key,v);
	AfxGetApp()->WriteProfileInt(section, key, v);
}
int GetInt(LPCSTR section, LPCSTR key, int def)
{
	return AfxGetApp()->GetProfileInt(section,key, def);
}

void PutColor(LPCSTR section, LPCSTR key, COLORREF color)
{
//DPF("put,%s,%s,%d",section,key,v);
	char buf[20];
	sprintf(buf,"%d,%d,%d",GetRValue(color),GetGValue(color),GetBValue(color));
	AfxGetApp()->WriteProfileString(section, key, buf);
}

COLORREF GetColor(LPCSTR section, LPCSTR key, COLORREF def)
{
	char buf[20];
	sprintf(buf,"%d,%d,%d",GetRValue(def),GetGValue(def),GetBValue(def));
	CString temp = AfxGetApp()->GetProfileString(section,key, "z");
	UINT r,g,b;
	COLORREF result;
	if (sscanf((LPCSTR)temp,"%d,%d,%d",&r,&g,&b) == 3)
		result = RGB(r,g,b);
	else
		result = def;
	return result;
}

static char BASED_CODE szSettings[] = "Settings";
static char BASED_CODE szSheet[] = "XSheet";
static char BASED_CODE szMainPos[] = "MainPos";
static char BASED_CODE szShowSheet[] = "ShowSheet";
static char BASED_CODE szSheetPos[] = "SheetPos";
#if MAC
static char BASED_CODE szShowTools[] = "ShowTools";
static char BASED_CODE szShowCamTools[] = "ShowCamTools";
#endif
static char BASED_CODE szShowPalette[] = "ShowPalette";
static char BASED_CODE szShowLipSync[] = "ShowLipSync";
static char BASED_CODE szShowPuck[] = "ShowPuck";
static char BASED_CODE szShowZoomer[] = "ShowZoomer";
static char BASED_CODE szShowSubPal[] = "ShowSubPal";
static char BASED_CODE szShowCamera[] = "ShowCamera";
static char BASED_CODE szShowSound[] = "ShowSound";
static char BASED_CODE szToolPos[] = "ToolPos";
static char BASED_CODE szBarState[] = "ToolBars";
//static char BASED_CODE szToolDlgPos[] = "ToolDlgPos";
static char BASED_CODE szShowSlider[] = "ShowSlider";
static char BASED_CODE szSliderPos[] = "SliderPos";
static char BASED_CODE szSubPalPos[] = "SubPalPos";
static char BASED_CODE szPalettePos[] = "PalettePos";
static char BASED_CODE szLipSyncPos[] = "LipSyncPos";
static char BASED_CODE szPuckDlgPos[] = "PuckPos";
static char BASED_CODE szZoomerDlgPos[] = "ZoomerPos";
static char BASED_CODE szCameraPos[] = "CameraPos";

static char BASED_CODE szColorNormal[] = "RGBNormal";
static char BASED_CODE szColorNormalSel[] = "RGBNormalSelect";
static char BASED_CODE szColorStack[] = "RGBStack";
static char BASED_CODE szColorStackSel[] = "RGBStackSelect";
static char BASED_CODE szColorOpen[] = "RGBOpen";
static char BASED_CODE szColorOpenSel[] = "RGBOpenSelect";
static char BASED_CODE szColorOpenTxt[] = "RGBOpenText";
static char BASED_CODE szColorStackTxt[] = "RGBStackText";
static char BASED_CODE szColorNormalTxt[] = "RGBNormalText";
static char BASED_CODE szColorLevelTxt[] = "RGBLevelText";
static char BASED_CODE szColorLevelSel[] = "RGBLevelSelect";
static char BASED_CODE szIconSize[] = "IconSize";

static char szFormat[] = "%u,%u,%d,%d,%d,%d,%d,%d,%d,%d";
static BOOL PASCAL NEAR ReadWindowPlacement(LPWINDOWPLACEMENT pwp, LPCSTR which)
{
	CString strBuffer = AfxGetApp()->GetProfileString(szSettings, which);
	if (strBuffer.IsEmpty())
		return FALSE;

	WINDOWPLACEMENT wp;
	int nRead = sscanf(strBuffer, szFormat,
		&wp.flags, &wp.showCmd,
		&wp.ptMinPosition.x, &wp.ptMinPosition.y,
		&wp.ptMaxPosition.x, &wp.ptMaxPosition.y,
		&wp.rcNormalPosition.left, &wp.rcNormalPosition.top,
		&wp.rcNormalPosition.right, &wp.rcNormalPosition.bottom);

	if (nRead != 10)
		{
		wp.flags = 0;
		wp.showCmd = SW_SHOW;
		wp.ptMinPosition.x = wp.ptMinPosition.y =
				wp.ptMaxPosition.x = wp.ptMaxPosition.y = -1;
		wp.length = sizeof wp;
		*pwp = wp;
ASSERT(0);
		return FALSE;
		}
	wp.length = sizeof wp;
	*pwp = wp;
	return TRUE;
}
static void PASCAL NEAR WriteWindowPlacement(LPWINDOWPLACEMENT pwp, LPCSTR which)
	// write a window placement to settings section of app's ini file
{
	int c = sizeof("-32767")*8 + sizeof("65535")*2;
	char szBuffer[20 + sizeof("-32767")*8 + sizeof("65535")*2];

	sprintf(szBuffer, szFormat,
		pwp->flags, pwp->showCmd,
		pwp->ptMinPosition.x, pwp->ptMinPosition.y,
		pwp->ptMaxPosition.x, pwp->ptMaxPosition.y,
		pwp->rcNormalPosition.left, pwp->rcNormalPosition.top,
		pwp->rcNormalPosition.right, pwp->rcNormalPosition.bottom);
DPF("%s,%s",which,szBuffer);
	AfxGetApp()->WriteProfileString(szSettings, which, szBuffer);
}

DWORD CMainFrame::SceneId()
{
	if (m_pScene)
		return m_pScene->SceneId();
	else
		return 0;
}

UINT CMainFrame::SceneState()
{
	if (m_pScene)
		return m_pScene->SceneState();
	else
		return 0;
}

void CMainFrame::OnReturn()
{
#if !MAC // FIXME
	m_pSlider->DoReturn();
#endif
}

void CMainFrame::OnSliderStart()
{
#if !MAC // FIXME
	m_pSlider->SetStart();
#endif
}

void CMainFrame::OnSliderStop()
{
#if !MAC // FIXME
	m_pSlider->SetStop();
#endif
}

void CMainFrame::OnUpdatePlay(CCmdUI* pCmdUI)
{
	CSketchView * pView = (CSketchView *)GetActiveView();
	if (pView)
		{
#if !MAC
		if (pView->Playing() != m_pSlider->m_bPlaying)
			m_pSlider->GetDlgItem(IDC_SLD_PLAY)->Invalidate(0);
#endif
		}
}


void CMainFrame::DoSlider(int code)
{
	CSketchView * pView = (CSketchView *)GetActiveView();
	if (code == 1)
		code = ID_VCR_HOME;
	else if (code == 2)
		code = ID_VCR_BSTEP;
	else if (code == 3)
		code = ID_VCR_FSTEP;
	else if (code == 4)
		code = ID_VCR_END;
	else
		{
		int v = pView->Playing();
		if (v > 1) // 2 means it is in edit modeTR
			{
			pView->PauseIt(pView->CurrentFrame());
			return;
			}
		m_pSlider->m_bPlaying = !v;
		if (!m_pSlider->m_bPlaying)
			code = ID_VCR_STOP;
		else
			code = ID_VCR_PLAY;
		}
	PostMessage(WM_COMMAND,code);
}

void CMainFrame::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
DPF("char:%d",nChar);
	switch (nChar)
        {
		case VK_UP:
DPF("up");
			break;
		case VK_DOWN:
DPF("down");
			break;
		default:
			CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
			break;
        }
}

void CMainFrame::Placements(BOOL bSave)
{
	WINDOWPLACEMENT wp;
	if (bSave)
		{
		wp.length = sizeof wp;
		if (GetWindowPlacement(&wp))
			{
			wp.flags = 0;
			if (IsZoomed())
				wp.flags |= WPF_RESTORETOMAXIMIZED;
			// and write it to the .INI file
			WriteWindowPlacement(&wp, szMainPos);
			}
		wp.length = sizeof wp;
		PutInt(szSettings,szShowSheet, m_bShowSheet);
		if (m_pSheet->GetWindowPlacement(&wp))
			{
			wp.flags = 0;
			WriteWindowPlacement(&wp, szSheetPos);
			}
#if MAC
		PutInt(szSettings, szShowTools, m_bShowTools);
		PutInt(szSettings, szShowCamTools, m_bShowCamTools);
#endif
		PutInt(szSettings, szShowSlider, m_bShowSlider);
		wp.length = sizeof wp;
		if (m_pPalette->GetWindowPlacement(&wp))
			{
			wp.flags = 0;
			WriteWindowPlacement(&wp, szPalettePos);
			}
		PutInt(szSettings, szShowPalette, m_bShowPalette);

	if (m_pLipSync) {
		wp.length = sizeof wp;
		if (m_pLipSync->GetWindowPlacement(&wp))
			{
			wp.flags = 0;
			WriteWindowPlacement(&wp, szLipSyncPos);
			}
		PutInt(szSettings, szShowLipSync, m_bShowLipSync);
	}

	if (m_pPuckDlg) {
		wp.length = sizeof wp;
		if (m_pPuckDlg->GetWindowPlacement(&wp))
			{
			wp.flags = 0;
			WriteWindowPlacement(&wp, szPuckDlgPos);
			}
		PutInt(szSettings, szShowPuck, m_bShowPuck);
	}
	if (m_pZoomerDlg) {
		wp.length = sizeof wp;
		if (m_pZoomerDlg->GetWindowPlacement(&wp))
			{
			wp.flags = 0;
			WriteWindowPlacement(&wp, szZoomerDlgPos);
			}
		PutInt(szSettings, szShowZoomer, m_bShowZoomer);
	}
		wp.length = sizeof wp;
		if (m_pSubPal->GetWindowPlacement(&wp))
			{
			wp.flags = 0;
			WriteWindowPlacement(&wp, szSubPalPos);
			}
		PutInt(szSettings, szShowSubPal, m_bShowSubPal);

		wp.length = sizeof wp;
		if (m_pCamera && m_pCamera->GetWindowPlacement(&wp))
			{
			wp.flags = 0;
			WriteWindowPlacement(&wp, szCameraPos);
			}
		}
}

void CMainFrame::OnClose()
{
	DPF("On Frame Close");
	CSketchView * pView = (CSketchView *)GetActiveView();
	if (!pView || !pView->IsKindOf(RUNTIME_CLASS(CSketchView)))
		{
		CFrameWnd::OnClose();
		return;
		}
	pView->StartTimer();
//#ifdef TABLET
//	WtExit();
//#endif
	// before it is destroyed, save the position of the window
	Placements(TRUE);
	SaveLibInfo();
 //	SaveBarState(szBarState);
	PutInt(szSettings, szShowCamera, m_bShowCamera);
	PutInt(szSettings,szShowSound, m_bShowSound);
	PutInt(szSettings,szIconSize, m_nIconSize);

	PutColor(szSheet,szColorLevelSel,crLevelSel);
	PutColor(szSheet,szColorLevelTxt,crLevelTxt);
	PutColor(szSheet,szColorNormal,crNormal);
	PutColor(szSheet,szColorNormalSel,crNormalSel);
	PutColor(szSheet,szColorNormalTxt,crNormalTxt);
	PutColor(szSheet,szColorStack,crStack);
	PutColor(szSheet,szColorStackSel,crStackSel);
	PutColor(szSheet,szColorStackTxt,crStackTxt);
	PutColor(szSheet,szColorOpen,crOpen);
	PutColor(szSheet,szColorOpenSel,crOpenSel);
	PutColor(szSheet,szColorOpenTxt,crOpenTxt);
   // SaveBarState saves everything but the number of Columns in the
   // Palette; you need to do that yourself.
   SaveBarState(_T(szBarState));
#if _MFC_VER < 0x0320
// IMPORTANT : Conditional compilation directives are
// necessary to maintain compatibility with future versions of MFC
   CDockState state;
   state.LoadState(_T(szBarState));
      for (int i = 0; i < state.m_arrBarInfo.GetSize(); i++)
   {
      _AFX_BARINFO* pInfo = (_AFX_BARINFO*)state.m_arrBarInfo[i];
      int nSize = pInfo->m_arrBarID.GetSize();
      while ((nSize!=0) && (pInfo->m_arrBarID[nSize-1]==NULL))
      {
         nSize--;
         pInfo->m_arrBarID.RemoveAt(nSize);
      }
      if (nSize)
         pInfo->m_arrBarID.InsertAt(nSize, (void*)NULL);
   }
   state.SaveState(_T(szBarState));
#endif

	CFrameWnd::OnClose();
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	DPF("$1");
	DPF("mainfrm create");
	HWND hwnd = ::GetDesktopWindow();
	m_nIconSize = GetInt(szSettings, szIconSize,2);
#ifndef FLIPBOOK_MAC
	HDC hdc = ::GetDC(hwnd);
	UINT winw = ::GetDeviceCaps(hdc, HORZRES);
	m_tbsize = winw / (30 + 10 * (m_nIconSize & 3));
						// toolbar icons shall be 1 / 40 of screen width
	::ReleaseDC(hwnd,hdc);
#else
	m_tbsize = 40;
#endif

//	UINT editw;
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	FixOEMMenu();
DPF("1");
	GetSystemMenu(FALSE);
	WINDOWPLACEMENT wp;
	if (!ReadWindowPlacement(&wp, "MainPos"))
		{
		RECT rcDesk;
		UINT w, h;
		if (m_pScene)
			{
			w = m_pScene->Width();
			h = m_pScene->Height();
			}
		else
			{
			w = 640;
			h = 480;
			}
		w += 15 + 1;
		h += 130;
		w += 2 * (m_tbsize + 10); // for docked toolbox on right
		GetDesktopWindow()->GetWindowRect(&rcDesk);
		wp.rcNormalPosition.left = wp.rcNormalPosition.top = 0;
		DPR("desk",&rcDesk);
		if (rcDesk.right > (int)w)
			wp.rcNormalPosition.right = w;
		else
			wp.rcNormalPosition.right = rcDesk.right - 20;
		if (rcDesk.bottom > (int)h)
			wp.rcNormalPosition.bottom = h;
		else
			wp.rcNormalPosition.bottom = rcDesk.bottom - 50;
		}
	else
		wp.showCmd = SW_SHOW;
//	editw = wp.rcNormalPosition.right;
#ifndef FLIPBOOK_MAC // we don't want to show the main window yet on the Mac
	SetWindowPlacement(&wp);
#ifndef _DISNEY
	if (MakeToolBar())
		return -1;
#endif
#endif
DPF("2");

	if (!m_pStatusBar->Create(this) ||
		!m_pStatusBar->SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE("Failed to create status bar\n");
		return -1;      // fail to create
	}
DPF("b");
	m_pStatusBar->SetPaneInfo(0,0,SBPS_NORMAL,220); // status
	m_pStatusBar->SetPaneInfo(1,0,SBPS_NORMAL,50); // busy/idle
#ifdef TIMING_NAMES
	m_pStatusBar->SetPaneInfo(2,0,SBPS_NORMAL,100);  // frame
#else
	m_pStatusBar->SetPaneInfo(2,0,SBPS_NORMAL,50);  // frame
#endif
	m_pStatusBar->SetPaneInfo(3,0,SBPS_NORMAL,50);  // rate
	m_pStatusBar->SetPaneInfo(4,0,SBPS_NORMAL,110);  // tool info
	m_pStatusBar->SetPaneInfo(5,0,SBPS_NORMAL,50);  // zoom

	if (!CreateSlider())
		return -1;
DPF("c");

	if (!CreateXSheet())
		return -1;
DPF("d");
	if (!CreateToolDlg())
		return -1;
DPF("after tool bars");
	m_pToolBar->EnableDocking(CBRS_ALIGN_ANY);
	m_pTools->EnableDocking(CBRS_ALIGN_ANY);
	m_pCamTools->EnableDocking(CBRS_ALIGN_ANY);
	m_pSlider->EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(m_pToolBar);
	DockControlBar(m_pTools);
	DockControlBar(m_pCamTools);

DPF("e");
	LoadBarState(szBarState);
DPF("f");
#ifndef _DISNEY
	m_bShowSheet = GetInt(szSettings, szShowSheet,1);
//	m_bShowTools = GetInt(szSettings, szShowTools,1);
	m_bShowPalette = GetInt(szSettings, szShowPalette,1);
	m_bShowLipSync = GetInt(szSettings, szShowLipSync,0);
	m_bShowPuck   = GetInt(szSettings, szShowPuck,0);
	m_bShowZoomer  = GetInt(szSettings, szShowZoomer,0);
	m_bShowCamera = 0;//GetInt(szSettings, szShowCamera,1);
	m_bShowSlider = GetInt(szSettings, szShowSlider,1);
	m_bShowSound = GetInt(szSettings, szShowSound,1);
#endif
	m_bShowSubPal = GetInt(szSettings, szShowSubPal,1);
#ifdef TABLET
//	CString string;
//	if (string.LoadString(WINTAB) &&
//			AfxGetApp()->GetProfileInt("Options",(LPCSTR)string, 0))
//		WtInit(m_hWnd,AfxGetInstanceHandle());
#endif

#if !MAC_AQUA_TEST
	crLevelSel = GetColor(szSheet,szColorLevelSel,crLevelSel);
	crLevelTxt = GetColor(szSheet,szColorLevelTxt,crLevelTxt);
	crNormal = GetColor(szSheet,szColorNormal,crNormal);
	crNormalSel = GetColor(szSheet,szColorNormalSel,crNormalSel);
	crNormalTxt = GetColor(szSheet,szColorNormalTxt,crNormalTxt);
	crStack = GetColor(szSheet,szColorStack,crStack);
	crStackSel = GetColor(szSheet,szColorStackSel,crStackSel);
	crStackTxt = GetColor(szSheet,szColorStackTxt,crStackTxt);
	crOpen = GetColor(szSheet,szColorOpen,crOpen);
	crOpenSel = GetColor(szSheet,szColorOpenSel,crOpenSel);
	crOpenTxt = GetColor(szSheet,szColorOpenTxt,crOpenTxt);
#endif
	m_nDisPal = GetInt(szSettings, "MagicCount",0);
DPF("main create done");
	CMenu * pMainMenu = GetMenu();
	CSketchApp* pApp = (CSketchApp *)AfxGetApp();


	CMenu * pViewMenu = pMainMenu->GetSubMenu(2); // View menu
	if (!pApp->IsLite() || pApp->IsAnimate())
		pViewMenu->AppendMenu(MF_BYPOSITION, ID_VIEW_LIPSYNC,"&LipSync Window");
	if (pApp->IsAnimate())
		{
		pViewMenu->AppendMenu(MF_BYPOSITION, ID_VIEW_PUCK, "Puck");
		#if !MAC
		pViewMenu->AppendMenu(MF_BYPOSITION, ID_VIEW_ZOOMER, "Zoomer");
		#endif
		}

	if (pApp->IsAnimate())
		{
		CMenu * pOptMenu = pMainMenu->GetSubMenu(3); // Options menu
		pOptMenu->AppendMenu(MF_BYPOSITION, ID_OPT_EMBEDDED,"Embedded Options ...");
		pOptMenu->AppendMenu(MF_BYPOSITION, ID_OPT_FRAME_NBR,
				"Frame Number Render Options ...");

		m_pIconMenu = new CMenu;
		VERIFY(m_pIconMenu->CreatePopupMenu());
		m_pIconMenu->AppendMenu(MF_BYPOSITION, ID_OPT_ICON_TINY,"Tiny");
		m_pIconMenu->AppendMenu(MF_BYPOSITION, ID_OPT_ICON_SMALL,"Small");
		m_pIconMenu->AppendMenu(MF_BYPOSITION, ID_OPT_ICON_MED,"Medium");
		m_pIconMenu->AppendMenu(MF_BYPOSITION, ID_OPT_ICON_LARGE,"Large");
		pOptMenu->AppendMenu(MF_POPUP, (UINT)m_pIconMenu->m_hMenu,
					"Adjust Icon Sizes...");

		m_pOverrideMenu = new CMenu;
		VERIFY(m_pOverrideMenu->CreatePopupMenu());
		m_pOverrideMenu->AppendMenu(MF_BYPOSITION,
					ID_OPT_OVERRIDE_HOLD,"Disable Hold");
		m_pOverrideMenu->AppendMenu(MF_BYPOSITION,
					ID_OPT_OVERRIDE_SCRUB,"Disable Scrub");
		m_pOverrideMenu->AppendMenu(MF_BYPOSITION,
					ID_OPT_OVERRIDE_EMBED,"Use Embedded");
		pOptMenu->AppendMenu(MF_POPUP, (UINT)m_pOverrideMenu->m_hMenu,
					"Overrides...");
		}

	m_nLibraries = 0;
	
	int mv = 0;
	if (((CSketchApp*)AfxGetApp())->IsMaya())
		mv = 1;
	if (((CSketchApp*)AfxGetApp())->CanDoLibrary())
		mv |= 2;
	if (((CSketchApp*)AfxGetApp())->IsAnimate())
		mv |= 4;
	if (mv)
		{
		CMenu * pFileMenu = pMainMenu->GetSubMenu(0); // file menu
		int count = pFileMenu->GetMenuItemCount();
		int i;
		for (i = 0; i < count; i++)
			{
			CString str;
			if (pFileMenu->GetMenuString(i, str, MF_BYPOSITION) &&
    			     	(strcmp(str, "&Export ...") == 0))
				break;
			}
    	CMenu * pMenu2;
		if (i < count)
    		pMenu2 = pFileMenu->GetSubMenu(i); // File menu
		else
    		pMenu2 = pFileMenu;
		if (mv & 1)
			pMenu2->AppendMenu(MF_BYPOSITION, ID_EXPORT_MAYA, "Export Maya");
		if (mv & 2)
			pMenu2->AppendMenu(MF_BYPOSITION, ID_FILE_EXPORT_CAM,
				"Camera Moves ...");
		if (mv & 4)
			pMenu2->AppendMenu(MF_BYPOSITION, ID_FILE_EXPORT_EMB,
				"Export Embedded ...");
		}

	if (mv & 2)
		{
		CMenu * pFileMenu = pMainMenu->GetSubMenu(0); // file menu
		int count = pFileMenu->GetMenuItemCount();
		int i;
		for (i = 0; i < count; i++)
			{
			CString str;
			if (pFileMenu->GetMenuString(i, str, MF_BYPOSITION) &&
    			     	(strcmp(str, "&Import ...") == 0))
				break;
			}
    	CMenu * pMenu2;
		if (i < count)
    		pMenu2 = pFileMenu->GetSubMenu(i); // File menu
		else
    		pMenu2 = pFileMenu;
		pMenu2->AppendMenu(MF_BYPOSITION, ID_FILE_IMPORT_CAM,
				"Camera Moves...");
		}
	
	if (mv & 2)
		LoadLibInfo();	// add in the library  menu items
	else
		GetLibMenu(1);	// remove library stuff from menu

#ifdef _DEBUG
	CMenu * pFileMenu = pMainMenu->GetSubMenu(0); // file menu
	pFileMenu->AppendMenu(MF_BYPOSITION, ID_FILE_RECORD, "Record...");
#endif
DPF("really done");
	return 0;
}

void CMainFrame::SetLeftScroll()
{
	if (!m_pDoc) return;
	UINT a,b;
	if (m_pDoc->Option(LEFT_SCROLL))
		{
		a = 0;
		b = WS_EX_LEFTSCROLLBAR;
		}
	else
		{
		b = 0;
		a = WS_EX_LEFTSCROLLBAR;
		}
	m_pSheet->GetDlgItem(IDC_GRID)->ModifyStyleEx(a,b);
	m_pPalette->ModifyStyleEx(a,b);
	CSketchView * pView = (CSketchView *)GetActiveView();
	if (pView)
			pView->ModifyStyleEx(a,b);
}

BOOL CMainFrame::CreateXSheet()
{
	BOOL bOnTop = AfxGetApp()->GetProfileInt("Options","XSheet On Top", 0);
	if (!m_pSheet->MyCreate(this,bOnTop))
		{
		DPF("Failed to create XSheet\n");
		return -1;      // fail to create
		}
	WINDOWPLACEMENT wp;
	if (!ReadWindowPlacement(&wp, szSheetPos))
		{
		RECT rcDesk;
		RECT rcFrame;
		UINT w,tw,c;
		if (m_pScene)
			{
			tw = m_twidth;
			c = m_pScene->LevelCount();
			}
		else
			{
			c = 2;
			UINT scale = 480 / HEIGHT;
			tw = 640 / scale;
			}
		w = 50 + 25 + tw/ 2 + tw * c;
		GetDesktopWindow()->GetWindowRect(&rcDesk);
		DPR("desk",&rcDesk);
		GetWindowRect(&rcFrame);
		DPR("frame",&rcFrame);
		wp.rcNormalPosition.right = rcDesk.right;
		wp.rcNormalPosition.left = rcDesk.right - w;
		wp.rcNormalPosition.top = rcFrame.top;
		wp.rcNormalPosition.bottom = rcFrame.bottom;
		}
	wp.showCmd = SW_HIDE;
	m_pSheet->SetWindowPlacement(&wp);
	RedrawSheet(3);
	return TRUE;
}

LRESULT CMainFrame::OnDisplayChange(WPARAM wParam, LPARAM lParam)
{
   // cBitsPerPixel = wParam;
   // cxScreen = LOWORD(lParam);
   // cyScreen = HIWORD(lParam);
#ifndef _DISNEY
	if (m_pToolBar)
		{
		if (IsIconic())
			m_pToolBar->TotalWidth(1);
		else
			MakeToolBar();
		}
#endif
	return 0; //CFrameWnd::OnDisplayChange(wParam, lParam);
}

UINT CMainFrame::MakeToolBar()
{
DPF("make tool bar");
	CRect zrect;
	GetWindowRect(&zrect);
	UINT editw = zrect.right - zrect.left;
	delete m_pToolBar;
	m_pToolBar = new CToolBar1;
	if (!m_pToolBar->CreateEx(this, 
		TBSTYLE_FLAT | TBSTYLE_TRANSPARENT,
		WS_CHILD|WS_VISIBLE|CBRS_TOP|
		CBRS_GRIPPER |
		CBRS_SIZE_DYNAMIC| CBRS_TOOLTIPS|CBRS_FLYBY,
		CRect(0,0,0,0),
		ID_VIEW_TOOLBAR))
	{
		TRACE("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	UINT flags = 0;
	if (((CSketchApp*)AfxGetApp())->CanDoLayers())
		flags |= 1;
	if (((CSketchApp*)AfxGetApp())->IsMaya())
		flags |= 2;
	m_pToolBar->Setup(m_tbsize, m_tbsize,editw,flags);
DPF("made tool bar");
	return 0;

}


BOOL CMainFrame::CreateToolDlg()
{
DPF("creating tools");
	delete m_pTools;
	m_pTools = new ctoolbox;
	if (!m_pTools->CreateEx(this, TBSTYLE_FLAT | TBSTYLE_TRANSPARENT,
		WS_CHILD | WS_VISIBLE |
		CBRS_LEFT| CBRS_GRIPPER |
		CBRS_SIZE_DYNAMIC| CBRS_TOOLTIPS|CBRS_FLYBY,
		CRect(0,0,0,0),
		ID_VIEW_TOOLS))
	{
		TRACE("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	m_pTools->Setup(this,m_tbsize,m_tbsize,m_pDoc, 0);
	delete m_pCamTools;
	m_pCamTools = new ccamtool;
	if (!m_pCamTools->CreateEx(this, TBSTYLE_FLAT | TBSTYLE_TRANSPARENT,
		WS_CHILD | WS_VISIBLE |
		CBRS_GRIPPER | CBRS_SIZE_DYNAMIC| CBRS_TOOLTIPS|CBRS_FLYBY|CBRS_RIGHT,
		CRect(0,0,0,0),
		ID_VIEW_CAM_TOOLS))
	{
		TRACE("Failed to create camtoolbar\n");
		return -1;      // fail to create
	}
	m_pCamTools->Setup(this,m_tbsize,m_tbsize);
	WINDOWPLACEMENT wp;
	RECT rc;
	if (!m_pPalette->MyCreate(this)) //, nLeft, nTop+200))
		{
		TRACE("Failed to create PaletteDlg\n");
		return -1;      // fail to create
		}

	if (ReadWindowPlacement(&wp, szPalettePos))
		{
		wp.showCmd = SW_HIDE;
		m_pPalette->SetWindowPlacement(&wp);
		}
	else

		{
		RECT rcMe;
		GetWindowRect(&rcMe);
		int w = rcMe.right - rcMe.left;
		m_pPalette->SetWindowPos(NULL, rcMe.left,rcMe.bottom+35,
						w-4, 20+16+3 * (w / 16), 0);
		}
	
	if (m_pLipSync) {
	if (!m_pLipSync->MyCreate(this)) //, nLeft, nTop+200))
		{
		TRACE("Failed to create LipSyncDlg\n");
		return -1;      // fail to create
		}
		
	if (ReadWindowPlacement(&wp, szLipSyncPos))
		{
		wp.showCmd = SW_HIDE;
		m_pLipSync->SetWindowPlacement(&wp);
		}
	else

		{
		RECT rcMe;
		GetWindowRect(&rcMe);
		int w = rcMe.right - rcMe.left;
		m_pLipSync->SetWindowPos(NULL, rcMe.left,rcMe.bottom+35,
						w-4, 20+16+3 * (w / 16), 0);
		}
	}
	if (m_pPuckDlg) {
	if (!m_pPuckDlg->MyCreate(this))
		{
		TRACE("Failed to create PuckDlg\n");
		return -1;      // fail to create
		}
		
	if (ReadWindowPlacement(&wp, szPuckDlgPos))
		{
		wp.showCmd = SW_HIDE;
		wp.rcNormalPosition.right = 
					wp.rcNormalPosition.left + m_pPuckDlg->Width();
		wp.rcNormalPosition.bottom = 
					wp.rcNormalPosition.top + m_pPuckDlg->Height();
		m_pPuckDlg->SetWindowPlacement(&wp);
		}
	else

		{
		RECT rcMe;
		GetWindowRect(&rcMe);
		int w = rcMe.right - rcMe.left;
		m_pPuckDlg->SetWindowPos(NULL, rcMe.left,rcMe.bottom+35,
						0, 0, SWP_NOSIZE);
		}
	}

	if (m_pZoomerDlg) {
	if (!m_pZoomerDlg->MyCreate(this))
		{
		TRACE("Failed to create ZoomerDlg\n");
		return -1;      // fail to create
		}
		
	if (ReadWindowPlacement(&wp, szZoomerDlgPos))
		{
		wp.showCmd = SW_HIDE;
		wp.rcNormalPosition.right = 
					wp.rcNormalPosition.left + m_pZoomerDlg->Width();
		wp.rcNormalPosition.bottom = 
					wp.rcNormalPosition.top + m_pZoomerDlg->Height();
		m_pZoomerDlg->SetWindowPlacement(&wp);
		}
	else

		{
		RECT rcMe;
		GetWindowRect(&rcMe);
		int w = rcMe.right - rcMe.left;
		m_pZoomerDlg->SetWindowPos(NULL, rcMe.left,rcMe.bottom+35,
						0, 0, SWP_NOSIZE);
		}
	}


	if (!m_pSubPal->MyCreate(this)) //, nLeft, nTop+200))
		{
		TRACE("Failed to create SubPaletteDlg\n");
		return -1;      // fail to create
		}
/*
	for (int i=0;i < 18; i++)
		{
		char key[20];
		sprintf(key,"DisPal%02d",i);
		CString temp = AfxGetApp()->GetProfileString(szSettings,key, key);
		m_pSubPal->SetName(i,(LPCSTR)temp);
		}
*/
	if (!ReadWindowPlacement(&wp, szSubPalPos))
		{
		RECT rcMe;
		GetWindowRect(&rcMe);
		m_pSubPal->SetWindowPos(NULL, rcMe.left,rcMe.bottom-20,
						0, 0, SWP_NOSIZE);
//		int w = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
///		int h = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
//	wp.rcNormalPosition.left = rcMe.left;
//		wp.rcNormalPosition.top = rcMe.bottom;
//		wp.rcNormalPosition.right = wp.rcNormalPosition.left + w,
//		wp.rcNormalPosition.bottom = wp.rcNormalPosition.top + h;
		}
	else
		{
		wp.showCmd = SW_HIDE;
		m_pSubPal->SetWindowPlacement(&wp);
		}

	if (!m_pCamera) return -1;
	if (!m_pCamera->MyCreate(this)) //, nLeft, nTop+400))
		{
		TRACE("Failed to create CameraDlg\n");
		return -1;      // fail to create
		}
	m_pCamera->GetWindowRect(&rc);
	if (ReadWindowPlacement(&wp, szCameraPos))
		{
		wp.rcNormalPosition.right = 
			wp.rcNormalPosition.left + rc.right - rc.left;
		wp.rcNormalPosition.bottom =
			wp.rcNormalPosition.top + rc.bottom - rc.top;
		wp.showCmd = SW_HIDE;
		m_pCamera->SetWindowPlacement(&wp);
		}
	return TRUE;
}

BOOL CMainFrame::CreateSlider()
{
#if !MAC
	if (!m_pSlider->Create(this, IDD_SLIDER,
		CBRS_BOTTOM|CBRS_TOOLTIPS|CBRS_FLYBY, ID_VIEW_VCR))
	{
		TRACE0("Failed to create DlgBar\n");
		return -1;      // fail to create
	}
#else
	m_pSlider->SubclassDlgItem (IDD_SLIDER, this);
#endif
	m_pSlider->ShowWindow(m_bShowSlider ? SW_SHOW : SW_HIDE);
	m_pSlider->Setup(this);
	return TRUE;

}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::ToolStatus(int id, int arg1, int arg2)
{
	char buf[100];
	wsprintf(buf,"Size:%3d, Opacity:%3d",arg1,arg2);
	m_pStatusBar->SetPaneText(4,buf);
	if (m_pPuckDlg && m_bShowPuck)
		m_pPuckDlg->UpdateInfo();
}

void CMainFrame::Status(LPCSTR txt /* = 0 */)
{
	if (!m_pStatusBar) return;
	if (txt)	
		m_pStatusBar->SetPaneText(0,txt);
	else
		m_pStatusBar->SetPaneText(0,"                             ");
}

UINT CMainFrame::Rows()
{
	return m_Frames+1;
}
UINT CMainFrame::Columns()
{
	return m_Levels+1+SoundCols();
}

void CMainFrame::SetCamera()
{
DPF("set camera:%d",m_bShowCamera);
	if (!m_pCamera) return;
	m_pCamera->Enable(m_bShowCamera);
	m_pCamera->ShowWindow(m_bShowCamera ? SW_SHOW : SW_HIDE);
}

void CMainFrame::ModeChange()
{
	BOOL bColor = m_pScene->ColorMode();
	if (bColor)
		bColor = m_bShowPalette;
	m_pPalette->ShowWindow(bColor ? SW_SHOW : SW_HIDE);
//	m_pLipSync->ShowWindow(bColor ? SW_SHOW : SW_HIDE);
//	m_pSubPal->ShowWindow(bColor ? SW_SHOW : SW_HIDE);
	m_pPuckDlg->SetMode(bColor);
	m_pZoomerDlg->SetMode(bColor);
	SetCamera();
	CSketchView * pView = (CSketchView *)GetActiveView();
	m_pTools->Setup(this,m_tbsize,m_tbsize,
							m_pDoc, pView->Layer() ? 1 : 0);
	m_pCamTools->Setup(this,m_tbsize,m_tbsize);
}

void CMainFrame::Opener(BOOL bOpened)
{
//	m_bInternalSound = 0;
	m_bNeedExtClip = 0;
	m_nDoubleDigit = 0;
	m_buddy_count = 0;
	m_onion_count = 0;
	Timing(0);
	if (bOpened)
		{
		m_vFrame = 0;
		m_stack_frame = 0;
		m_stack_level = 1;
//DPF("res:,%d,%d",cx,cy);
//		int cx = m_pScene->ComW();
//		int cy = m_pScene->ComH();
		int cy = 2 * m_pScene->Height() / m_pScene->ZFactor(1);
		int q = m_pScene->Zoom();
		if (!q)
			q = 100;
		else if (q == 1)
			q = 25;
		else if (q == 2)
			q = 50;
		else if (q == 3)
			q = 200;
		else if (q == 4)
			q = 300;
		else if (q == 5)
			q = 400;
		else if (q == 6)
			q = 800;
		else
			q = 0; //1600; // actually 33 %
		if (q)
			cy = cy * q / 100;
		else
			cy = cy / 3;


//		if (((UINT)cx != m_owidth) || ((UINT)cy != m_oheight))
		if (((UINT)cy != m_oheight) && !m_oheight)
			{
//			m_owidth = cx;
			m_oheight = cy;
			RECT rcMe;
			GetWindowRect(&rcMe);
			rcMe.right = rcMe.right - rcMe.left;
			rcMe.left = 0;
			rcMe.top = 0;
			rcMe.bottom = cy;
			int x1 = rcMe.left;
			int x2 = rcMe.right;
			AdjustWindowRect(&rcMe, WS_THICKFRAME | WS_CAPTION, 1);
			DPR("rcme",&rcMe);
			rcMe.left = x1;
			rcMe.right = x2;
			int cx = rcMe.right - rcMe.left;
			cy = rcMe.bottom - rcMe.top;
			RECT rcTemp;
			int a1 = GetSystemMetrics(SM_CYDLGFRAME);
DPF("1:%d",a1);
		//	a1 = 3;
			int a2 = 0;//GetSystemMetrics(SM_CYFRAME);
DPF("2:%d",a2);
			int a3 = GetSystemMetrics(SM_CYBORDER);
DPF("3:%d",a3);
			int a4 = GetSystemMetrics(SM_CYCAPTION);
DPF("4:%d",a4);
			a4 = 0;
			if (m_pToolBar && (m_pToolBar->GetStyle() & WS_VISIBLE))
				{	
				m_pToolBar->GetWindowRect(&rcTemp);
DPR("toolbar",&rcTemp);
				cy += rcTemp.bottom-rcTemp.top - 2 * a3;
				}
			if (m_pStatusBar && (m_pStatusBar->GetStyle() & WS_VISIBLE))
				{	
				m_pStatusBar->GetWindowRect(&rcTemp);
DPR("statbar",&rcTemp);
				cy += rcTemp.bottom-rcTemp.top;
				}
			m_pSlider->ShowWindow(m_bShowSlider ? SW_SHOW : SW_HIDE);
			if (m_pSlider && (m_pSlider->GetStyle() & WS_VISIBLE))
				{
				m_pSlider->GetWindowRect(&rcTemp);
DPR("slider",&rcTemp);
		
				cy += rcTemp.bottom - rcTemp.top;
				}


			cy +=  a1 + a2 + a3 + a4;//46+a;//70;//66;
//			cy += a1 + a2 + a3 + a4;//46+a;//70;//66;
//			cx += 10;
			RECT rcDesk;
			GetDesktopWindow()->GetWindowRect(&rcDesk);
			DPR("desk",&rcDesk);
			int maxy = rcDesk.bottom - rcDesk.top - 40; // for toolbar
			if (cx > (rcDesk.right - rcDesk.left))
				cx = rcDesk.right - rcDesk.left;
			if (cy > maxy)
				cy = maxy;
			SetWindowPos(NULL, 0, 0, cx, cy, SWP_NOMOVE);
			}
		else
			{
			m_pSlider->ShowWindow(m_bShowSlider ? SW_SHOW : SW_HIDE);
			}
		m_pSheet->SetRange(m_Frames = m_pScene->FrameCount(),
				m_Levels = m_pScene->LevelCount(),1);
		m_MaxFrame = m_pScene->MaxFrameCount();
		m_pSlider->Setup(this);
		m_pSlider->SetRange(m_Frames);

		SetupThumb();
DPF("m1");
		CSketchView * pView = (CSketchView *)GetActiveView();
		m_pPalette->Setup(m_pDoc);
	if (m_pLipSync)
		{
		if (m_pLipSync->Setup(m_pDoc))
			{
			delete m_pLipSync;
			m_pLipSync = 0;
			}
		}
	if (m_pPuckDlg)
		{
		if (m_pPuckDlg->Setup(m_pDoc))
			{
			delete m_pPuckDlg;
			m_pPuckDlg = 0;
			}
		}
	if (m_pZoomerDlg)
		{
		if (m_pZoomerDlg->Setup(m_pDoc))
			{
			delete m_pZoomerDlg;
			m_pZoomerDlg = 0;
			}
		}
//	UINT li;
//	for (li = 0; li < m_nLibraries;li++)
//		{
//		m_pLibs[li]->Setup(m_pDoc);
//		}
DPF("m2");
//	m_pSlider->Setup(pDoc);
		char buf[20];
		sprintf(buf,"%d fps",m_pScene->FrameRate());
		m_pStatusBar->SetPaneText(3,buf);
		delete m_pSound;
		m_pSound = new CMySound();
		m_pSound->SetScene(m_pScene);
/*
		UINT size = m_pDoc->Appended(0);
		if (size &&
			(((CSketchApp *)AfxGetApp())->MyWarning(IDD_APPENDED_SND) == IDOK))
			{
			CString name;
			if (((CSketchApp*) AfxGetApp())->PromptFileName(name,14))
				{
				BYTE * pTemp = new BYTE[size];
				m_pDoc->Appended(pTemp);
				m_pSound->SaveWaveData(pTemp, size,name);
				delete [] pTemp;
				char temp[300];
				strcpy(temp, (LPCSTR)name);
				m_pScene->SceneOptionStr(SCOPT_WAVE0,temp,1);
				
				}
			}
*/
		UINT snip = m_pDoc->Option(SC_SNIP);
		m_bCanDoSound = m_pSound->SoundSetup(m_twidth,m_theight,m_Frames,
					m_pScene->FrameRate(), snip);
		if (!m_bCanDoSound)
		{
		if (m_bShowSound) SwitchSound();
		int idd;
		for (idd = 0; idd < 3; idd++)
			{
			char name[300];
			m_pScene->SceneOptionStr(SCOPT_WAVE0+idd,name);
			if (name[0])
				break;
			}
		if (idd < 3)
			{
			MyError(IDS_NO_SOUND);
			}
		}
		else
		{
		m_pSound->Volume(99,m_pScene->SceneOptionInt(SCOPT_MVOL));
		int idd;
		for (idd = 0; idd < 3; idd++)
			{
			double t = m_pScene->SceneOptionInt(SCOPT_SMARK0+idd);
			UINT vmark = 1+m_pScene->SceneOptionInt(SCOPT_VMARK0+idd);
			m_pSound->SetAVMarks(idd,t / 1000.0,vmark);
			int volume = m_pScene->SceneOptionInt(SCOPT_VOL0+idd);
			m_pSound->Volume(idd,volume);
			char name[300];
			m_pScene->SceneOptionStr(SCOPT_WAVE0+idd,name);
			if (name[0])
				{
				int res = m_pSound->LoadWaveFile(idd,(LPCSTR)name);
				if (res)
					{
//						MyError(IDS_NO_WAVE);
//						MyError(IDS_NO_WAVE,(LPCSTR)name);
DPF("sound load,res:%d",res);
						
					}
				}
			}
		m_pSound->Allow(1);
		}
		m_pSheet->ShowWindow(m_bShowSheet ? SW_SHOW : SW_HIDE);
		RedrawSheet(3);
//		m_wndToolBoxBar.ShowWindow(m_bShowTools ? SW_SHOW : SW_HIDE);
		if (m_pScene->Camera())
			{
			if (m_pCamera)
				m_pCamera->Setup(m_pDoc);
			}
		else
			{
			delete m_pCamera;
			m_pCamera = 0;
			}
		pView->Setup();
		if (m_pScene->ColorMode())
			{
			m_pPalette->ShowWindow(m_bShowPalette ? SW_SHOW : SW_HIDE);
			m_pSubPal->ShowWindow(m_bShowSubPal ? SW_SHOW : SW_HIDE);
			SetCamera();
			}
		if (m_pLipSync)
			m_pLipSync->ShowWindow(m_bShowLipSync ? SW_SHOW : SW_HIDE);
		if (m_pPuckDlg)
			m_pPuckDlg->ShowWindow(m_bShowPuck ? SW_SHOW : SW_HIDE);
		#if !MAC
		if (m_pZoomerDlg)
			m_pZoomerDlg->ShowWindow(m_bShowZoomer ? SW_SHOW : SW_HIDE);
		#endif
//		m_pSlider->ShowWindow(m_bShowSlider ? SW_SHOW : SW_HIDE);
	UINT li;
	for (li = 0; li < m_nLibraries;li++)
		{
		m_pLibs[li]->Setup(m_pDoc);
		}
		RecalcLayout();
		if (m_pScan)
			m_pScan->GetChanges();
#ifdef _DISNEY
		char pbuf[500];
		char * p;
		int i;
		for (i=0;i < 18; i++)
			pbuf[20*i] = 0;
		if (m_pScene)
			m_pScene->DisPalIO((BYTE *)pbuf,360,0);
		p = pbuf;
		for (i=0;i < 18; i++, p+=20)
			{
			if (!p[0])
				{
				p[0] = 'A' + i;
				p[1] = 0;
				}
			m_pSubPal->SetName(i,p);
			}
#endif
		AdjustDisPalWindows(1);
		m_pTools->Setup(this,m_tbsize, m_tbsize,m_pDoc, 
						pView->Layer() ? 1 : 0,1);
		m_pCamTools->Setup(this,m_tbsize, m_tbsize);
		SetLeftScroll();
		m_pSlider->Invalidate(1); // seems to repaint playback controls
		}
	else
		{
		m_pSheet->ShowWindow(SW_HIDE);
		m_pPalette->ShowWindow(SW_HIDE);
		UINT li;
		for (li = 0; li < m_nLibraries;li++)
			{
			m_pLibs[li]->ShowWindow(SW_HIDE);
			}
		if (m_pLipSync)
			m_pLipSync->ShowWindow(SW_HIDE);
		m_pSubPal->ShowWindow(SW_HIDE);
		if (m_pCamera)
			{
			m_pCamera->ShowWindow(SW_HIDE);
			m_pCamera->Setup(0);
			}
		m_pSlider->ShowWindow(SW_HIDE);
		CSketchView * pView = (CSketchView *)GetActiveView();
		pView->Setup(1);
		}
DPF("end of opener");
}

UINT CMainFrame::Progress(int code, int arg)
{
	char buf[100];
	UINT prev; 
	if (code == 998)
		{
		if (arg > (int)m_Frames)
			arg = m_Frames;
		prev = m_vFrame;
		m_vFrame = arg;
		if (prev && (prev <= m_Frames))
			m_pSheet->m_Grid.RedrawCell(prev,0);
		m_pSheet->m_Grid.RedrawCell(m_vFrame,0);
		UINT lvl;
		if (m_bTiming)
			{
			UINT min, max;
			lvl = m_TimingLevel;
			m_pScene->ThumbMinMax(min,max, arg-1, m_TimingLevel);
#ifdef TIMING_NAMES
			char name[50];
			m_pScene->CellName(name, arg-1, lvl);
			wsprintf(buf,"%s - %d",name,1 +max - arg);
#else
			wsprintf(buf,"%d - %d",arg,1 +max - arg);
#endif
			}
		else
			{
			lvl = m_stack_level;
			wsprintf(buf,"%3d",arg);
			}
		UINT cc = m_pSheet->m_Grid.GetColumnCount();
		m_pSheet->m_Grid.EnsureVisible(m_vFrame,cc-lvl-1);
		m_pStatusBar->SetPaneText(2,buf);
		m_pStatusBar->RedrawWindow();
		m_pSlider->SetFrame(arg);
		return 0;
		}
	if (code == 997)
		{
		m_pSlider->SetFrame(arg);
		return 0;
		}
	if (code == 999)
		{
		prev = m_vFrame;
		m_vFrame = 0;
		if (prev)
			{
			m_pSheet->m_Grid.RedrawCell(prev,0);
			m_pStatusBar->SetPaneText(2,"     ");
			m_pStatusBar->RedrawWindow();
			}
		return 0;
		}
	if (code == 995)
		{
		wsprintf(buf,"%d",arg);
		m_pStatusBar->SetPaneText(1,buf);
		m_pStatusBar->RedrawWindow();
		return 0;
		}
	if (code == 990)
		{
		wsprintf(buf,"%d%%",arg);
		m_pStatusBar->SetPaneText(5,buf);
		m_pStatusBar->RedrawWindow();
		return 0;
		}
	if (code == 0)
		wsprintf(buf,"   ");
	else if (code == 1)
		{
		m_range = arg;
		wsprintf(buf,"%3d%%",0);
		}
	else if (code == 996)
		wsprintf(buf,"Peg:%2d",arg);
	else 
		wsprintf(buf,"%3d%%",MulDiv(arg,100,m_range));
//DPF("setting pane:%s",buf);
	m_pStatusBar->SetPaneText(2,buf);
	m_pStatusBar->RedrawWindow();
	return 0;
}

BOOL CMainFrame::SwitchSheet(BOOL bSwitch)
{
	if (bSwitch)
		{
		m_bShowSheet ^= 1;
		m_pSheet->ShowWindow(m_bShowSheet ? SW_SHOW : SW_HIDE);
		}
	return m_bShowSheet;
}

BOOL CMainFrame::SwitchSound(BOOL bSwitch)
{
	if (bSwitch)
		{
		UINT f1, f2, l1, l2;
		GetSelection(f1,l1,f2,l2);
		m_bShowSound ^= 1;
		m_pSheet->SetRange(m_Frames,m_Levels);
		SetSelection(f1,l1,f2,l2);
		RedrawSheet(1);
		}
	return m_bShowSound;
}
#if MAC
BOOL CMainFrame::SwitchTools(BOOL bSwitch)
{
	m_bShowTools = m_pTools->IsWindowVisible();
	if (bSwitch)
		{
		m_bShowTools ^= 1;
		if (!m_bShowTools)
			m_pTools->CloseDialog();
		m_pTools->ShowWindow(m_bShowTools ? SW_SHOW : SW_HIDE);
		}
	
	return m_bShowTools;
}

BOOL CMainFrame::SwitchCamTools(BOOL bSwitch)
{
	m_bShowCamTools = m_pCamTools->IsWindowVisible();
	if (bSwitch)
		{
		m_bShowCamTools ^= 1;
		m_pCamTools->ShowWindow(m_bShowCamTools ? SW_SHOW : SW_HIDE);
		}
	return m_bShowCamTools;
}

#endif

BOOL CMainFrame::ToolMessage(int msg)
{
	if (!m_pTools)
		return 0;
	return m_pTools->ToolMessage(msg);
}

BOOL CMainFrame::SwitchPalette(BOOL bSwitch)
{
	if (bSwitch)
		{
		m_bShowPalette ^= 1;
		m_pPalette->ShowWindow(m_bShowPalette ? SW_SHOW : SW_HIDE);
		}
	return m_bShowPalette;
}

BOOL CMainFrame::SwitchLipSync(BOOL bSwitch)
{
	if (m_pLipSync)
		{
		if (bSwitch)
			{
			m_bShowLipSync ^= 1;
			m_pLipSync->ShowWindow(m_bShowLipSync ? SW_SHOW : SW_HIDE);
			}
		return m_bShowLipSync;
		}
	else
		{
		if (bSwitch)
			AfxMessageBox(IDS_NOT_FOR_LITE);
		return 0;
		}
}

BOOL CMainFrame::SwitchPuck(BOOL bSwitch)
{
	if (m_pPuckDlg)
		{
		if (bSwitch)
			{
			m_bShowPuck ^= 1;
			m_pPuckDlg->ShowWindow(m_bShowPuck ? SW_SHOW : SW_HIDE);
			if (m_bShowPuck)
				m_pPuckDlg->UpdateInfo();
			}
		return m_bShowPuck;
		}
	else
		{
		if (bSwitch)
			AfxMessageBox(IDS_NOT_FOR_LITE);
		return 0;
		}
}

BOOL CMainFrame::SwitchZoomer(BOOL bSwitch)
{
	if (m_pZoomerDlg)
		{
		if (bSwitch)
			{
			m_bShowZoomer ^= 1;
			m_pZoomerDlg->ShowWindow(m_bShowZoomer ? SW_SHOW : SW_HIDE);
			}
		return m_bShowZoomer;
		}
	else
		{
		if (bSwitch)
			AfxMessageBox(IDS_NOT_FOR_LITE);
		return 0;
		}
}

BOOL CMainFrame::SwitchSubPalette(BOOL bSwitch)
{
	if (bSwitch)
		{
		m_bShowSubPal ^= 1;
		m_pSubPal->ShowWindow(m_bShowSubPal ? SW_SHOW : SW_HIDE);
		}
	return m_bShowSubPal;
}

BOOL CMainFrame::SwitchCamera(BOOL bSwitch)
{
	if (!m_pCamera) return 0;
	if (bSwitch)
		{
		m_bShowCamera ^= 1;
		SetCamera();
		}
	return m_bShowCamera;
}

BOOL CMainFrame::Timing(UINT val /* = 3 */)
{
	if ((val < 2) && ((BOOL)val == m_bTiming))
		return m_bTiming;
	if (val == 2)
		val = m_bTiming ^ 1;
	if (val < 2)
		{
		m_bTiming = val;
		if (m_bTiming)
			{
			UINT l,f;
			GetSelection(f,l);
			m_TimingLevel = m_Level;
			SelectCell(f, m_Level, 4);
			}
		m_pSheet->Special(m_bTiming);
		}
	return m_bTiming;
}


void CMainFrame::AttachDoc(CSketchDoc * pDoc)
{
DPF("attach doc:%d,flag:%d",(UINT)m_pDoc,pDoc->DocOpened());
	m_pDoc = pDoc;
	m_max_frames = m_pDoc->MaxFrames();
	m_pScene = pDoc->m_pScene;
	Opener(pDoc->DocOpened());
DPF("m3");
	CSketchApp* pApp = (CSketchApp *)AfxGetApp();
	if (pApp->IsMaya())
		PostMessage(WM_COMMAND, ID_FILE_IMPORT_MAYA,0);
	else if (pDoc->CheckHelp())	// kludge for from avi at init, for thumbs
		{
		UpdateLevelThumbs(0);
		}
}

BOOL CMainFrame::OnViewBar(UINT nID)
{
	CWnd* pBar;
	if ((pBar = GetDlgItem(nID)) == NULL)
		return FALSE;   // not for us
	m_bShowSlider ^= 1;
	pBar->ShowWindow(m_bShowSlider);
	RecalcLayout();
	return TRUE;
}

void CMainFrame::SetFrameRate(int rate)
{
	if (!m_pScene) return;
	int oldrate = m_pScene->FrameRate();
	if (rate != oldrate)
		{
		char buf[20];
		sprintf(buf,"%d fps",rate);
		m_pStatusBar->SetPaneText(3,buf);
		m_pScene->SetFrameRate(rate);
		m_pSlider->m_rate.Assign(rate);
		m_pSound->SetRate(rate);
		}
}

void CMainFrame::SelectCell(UINT Frame, UINT Level, UINT who)
{
DPF("main select,f:%d,l:%d,w:%d",Frame,Level,who);
	if (who != 1 && who != 9)
		m_pSheet->SelectCell(Frame, Level);
	CSketchView * pView = (CSketchView *)GetActiveView();
	if (who != 3)
		{
	//	StackInit(Frame, Level);
	//	CSketchView * pView = (CSketchView *)GetActiveView();
		pView->ASelectCell(Frame,Level, who == 9 ? 1 : 0);
	//	StackInit(Frame, Level);
		if (who == 4)
			return;
		}
	m_pPalette->Select(Level);
	CopySubPal();
//	FillLayerCombo(pView->Layer(),TRUE);
	m_pTools->Setup(this,m_tbsize,m_tbsize,m_pDoc, pView->Layer() ? 1 : 0);
	m_pCamTools->Setup(this,m_tbsize,m_tbsize);
DPF("main done");
}

BOOL CMainFrame::XlateRowCol(UINT row, UINT col)
{
	m_Kind = 0;
	if (row < 1)
		{
		m_Frame = 0;
		m_Kind |= 4;
		}
	else
		m_Frame = row - 1;
	int cc = m_pSheet->m_Grid.GetColumnCount();
	if ((m_Frame >= m_Frames) || (col >= (UINT)cc))
		return TRUE;
	m_Level = cc - col - 1;
	if (col >= (UINT)cc)
		return TRUE;
	if (!col)
		m_Kind |= 2;
	else if (m_Level >= m_Levels)
		{
		if (!m_bShowSound)
			return TRUE;
		m_Kind |= 1;
		m_Level = col - 1;  // sound channel
		}
	return FALSE;
}

void CMainFrame::SelectGCell(UINT row, UINT col)
{
	if (XlateRowCol(row,col))
		return;
DPF("row:%d,col:%d,frm:%d,lvl:%d",row,col,m_Frame, m_Level);
	if (m_Kind & 4)  // top row
		{
		m_Kind ^= 4;		// clear for simplicity
		if (m_Kind == 2)    // left row, actually upper left corner
			return;
		else if (m_Kind)
			{
			if (!m_pSound->Enabled(m_Level))
				{
				m_pDoc->Option(SC_QUIET,1,1);
				}
			m_pSound->Enabled(m_Level, 2);
			}
		else
			{
			DWORD v = m_pScene->LevelFlags(m_Level) ^ 1;
			m_pScene->LevelFlags(m_Level,v);
			m_pScene->UpdateCache(0,m_Level,9999);
			CSketchView * pView = (CSketchView *)GetActiveView();
			pView->Update(m_stack_frame,m_stack_level,1);
			}
		m_pSheet->m_Grid.RedrawCell(0,col);
		return;
		}
	if (m_Kind)
		return;
	if (!m_Level)
		return;
	SetFocus();
	SelectCell(m_Frame, m_Level,1);
}

void CMainFrame::SetSheetMode(UINT code)
{
}

void CMainFrame::OnUpdateCamTool(CCmdUI* pCmdUI, UINT Id)
{
	UINT CurId = 98;
	CSketchView * pView = (CSketchView *)GetActiveView();
	if (pView && pView->CamState() && m_pCamera ) //&& m_bShowCamera)
			CurId = m_pCamera->ToolSelected();
	pCmdUI->SetCheck(CurId == Id ? 1 : 0);
}

void CMainFrame::OnUpdateCamButton(CCmdUI* pCmdUI, UINT Id)
{
	BOOL bEnable = 0;
	if (m_pCamera)
		bEnable = m_pCamera->ToolEnabled(Id);
	pCmdUI->Enable(bEnable);
}

void CMainFrame::OnCamTool(UINT Id)
{
	if (!m_pCamera) return;
/*
	if (!m_bShowCamera)
		{
		m_bShowCamera = 1;
		m_pCamera->Enable(m_bShowCamera);
		m_pCamera->ShowWindow(SW_SHOW);
		}
*/
	m_pCamera->ToolInit(Id);

}

void CMainFrame::OnCamDialog()
{
	if (!m_pCamera) return;
	m_pCamera->SendMessage(WM_COMMAND,ID_KEY_TOOL_TYPE,0);
}


void CMainFrame::OnUpdateBarMenu(CCmdUI* pCmdUI)
{
	CWnd* pBar;
	if ((pBar = GetDlgItem(pCmdUI->m_nID)) == NULL)
	{
		pCmdUI->ContinueRouting();
		return; // not for us
	}
	pCmdUI->SetCheck((pBar->GetStyle() & WS_VISIBLE) != 0);
}


/*
void CMainFrame::OnSize(UINT type, int cx, int cy)
{
DPF("size, t:%d,cx:%d,cy:%d",type,cx,cy);
	CFrameWnd::OnSize(type, cx, cy);
//	theApp.m_bMaximized = (nType == SIZE_MAXIMIZED);
//	if (nType == SIZE_RESTORED)
//		GetWindowRect(theApp.m_rectInitialFrame);
    static BOOL bAlreadyInsideThisProcedure = FALSE;
    if (bAlreadyInsideThisProcedure)
        return;

    if (!::IsWindow(m_hWnd))
        return;

    // Start re-entry blocking
    bAlreadyInsideThisProcedure = TRUE;

	if (type == SIZE_MINIMIZED)
		{
//		delete m_pToolBar;
//		m_pToolBar = 0;
		}
	else if (m_pToolBar)
		{
		BOOL bNow = m_pToolBar->TotalWidth() > (UINT)cx ? 1 : 0;
		if ((bNow != m_pToolBar->Double()) || !m_pToolBar->TotalWidth())
			 MakeToolBar();
		}
//	else
//		MakeToolBar();

    // End re-entry blocking
    bAlreadyInsideThisProcedure = FALSE;
}

*/

void CMainFrame::OnMove(int x, int y)
{
DPF("frame move, x:%d,y:%d",x,y);
	CSketchView * pView = (CSketchView *)GetActiveView();
	if (pView)
		pView->ViewMoved();
	CFrameWnd::OnMove(x,y);
}


void CMainFrame::CheckViewDirty()
{
	if (m_pDoc->Option(PEG_SHOWFG))
		m_nViewDirty |= 1;
DPF("dirty:%d",m_nViewDirty);
}
#if 0
UINT CMainFrame::PegCount(BOOL bClear /* == FALSE */)
{
DPF("PegCount:%d",bClear);
	if (bClear)
		{
		UINT i,c;
		c = m_pegbar.Count();
		for (i = c;i > 1;i--)
			m_pegbar.RemoveId(i-1);
		}
	return m_pegbar.Count();
}

UINT CMainFrame::PegContents(UINT &Frame, UINT & Level,UINT i,
							BOOL bPut /* = 0 */)
{
	return m_pegbar.Contents(Frame, Level,i, bPut);
}
#endif

void CMainFrame::StackInit(UINT frame, UINT level)
{
//	if ((m_stack_frame == frame) && 
//			(m_stack_level == level) )
//		return;
	if (!m_pDoc || !m_pScene || m_bTiming)
		return;
	StackClear();
	m_stack_frame = frame;
	m_stack_level = level;
	m_bXSheetHighlight = m_pDoc->Option(XSHEET_HIGHLIGHT);
	StackReload();
}

void CMainFrame::StackClear()
{
	m_stack_flag = 1;
	m_bXSheetHighlight = 1; // force erase
	StackDraw();
}



void CMainFrame::StackUpdate(BOOL bJustDraw )
{
	if (!m_pDoc || !m_pScene || m_bTiming)
		return;
	StackClear();
	m_stack_flag = 0;
	m_bXSheetHighlight = m_pDoc->Option(XSHEET_HIGHLIGHT);
	if (!bJustDraw)
		StackReload();
	else
		RedrawGrid(m_stack_frame,m_stack_level);
}

void CMainFrame::StackDraw()
{
	RedrawGrid(m_stack_frame,m_stack_level);
	UINT i;
	for (i = 0; i < m_buddy_count; i++)
		{
		RedrawGrid(m_stack_buddyf[i],m_stack_buddyl[i]);
		}
	for (i = 0; i < m_onion_count; i++)
		{
		RedrawGrid(m_stack_onion[i],m_stack_level);
		}
}

void CMainFrame::StackReload()
{
	if (m_pDoc->Option(PEG_SHOWBG) && m_stack_level)
		m_stack_bg = TRUE;
	else
		m_stack_bg = FALSE;
	m_stack_flag = 0;
	m_onion_count = 0;
	m_buddy_count = 0;
	if (!m_pDoc->Option(PEG_SHOWFG))
		{
		StackDraw();
		return;
		}
	UINT depth = m_pDoc->Option(PEG_DEPTH);
//	if (m_pScene->ColorMode())
//		{
//		}
//	else if (!m_pScene->SceneOptionInt(SCOPT_MRU))
	if (!m_pDoc->Option(SC_MRU))
		{
		UINT min, max;
		m_pScene->ThumbMinMax(min, max, m_stack_frame, m_stack_level);
		if (m_pDoc->Option(SC_PREV) && (min < m_Frames))
			m_stack_onion[m_onion_count++] = min;
		if (m_pDoc->Option(SC_NEXT) && (max < m_Frames))
			m_stack_onion[m_onion_count++] = max;
		if (m_pDoc->Option(SC_PREV1) && min && (min < m_Frames))
			{
			min--;
			if (m_pScene->FindPrevCell(min,m_stack_level))
				m_stack_onion[m_onion_count++] = min;
			}
		if (m_pDoc->Option(SC_NEXT1) && (max < m_Frames))
			{
			max++;
			if (m_pScene->FindNextCell(max,m_stack_level))
				m_stack_onion[m_onion_count++] = max;
			}
		}
	else if (m_pDoc->Option(SC_MRU) == 1)
		{
		m_onion_count = m_pScene->Before(m_stack_onion, depth,
					m_stack_frame, m_stack_level);
		}
	UINT i;
	for (i = 0; i < 4; i++)
		{
		UINT z = m_pScene->SceneOptionInt(SCOPT_BUD0+i);
		if (z && (z != m_stack_level))
			{
			if (m_pScene->Before(m_stack_buddyf+m_buddy_count,
							0,m_stack_frame, z))
				m_stack_buddyl[m_buddy_count++] = z;
			}
		}
	StackDraw();
}

int CMainFrame::StackFind(UINT f, UINT l, UINT code)
{
	if (code == 1)
		{
		if (m_stack_flag)
			return 0;
		if ((m_stack_frame==f) && (m_stack_level==l))
			return 1;
		if (!m_pDoc->Option(PEG_SHOWFG))
			return 0;
		UINT i;
		if (l == m_stack_level)
			{
			for (i = 0; i < m_onion_count;i++)
				if (f == m_stack_onion[i])
					return i+2;
			}
		for (i = 0; i < m_buddy_count;i++)
			if ((l == m_stack_buddyl[i]) && (f == m_stack_buddyf[i]))
					return i+12;
		}
	return 0;
}

void CMainFrame::GetCellText(LPSTR buf, UINT row, UINT col)
{
//	int cc = m_Grid.GetColumnCount();
//	cc = cc - col - 1;
	sprintf(buf,"Cell Text");
	return;
	if (col >= m_Levels)
		{
		if (row)
			sprintf(buf,"Frame %d", row);
		else
			buf[0] = 0;
		}
	else if (row)
		sprintf(buf,"%c-%d",'A' + col,row);
	else
		sprintf(buf,"Level %d",col);
}
void CMainFrame::DrawText(CDC * pDC, LPCRECT rect, UINT row, UINT col)
{
	if (m_pSheet->m_Grid.GetColumnWidth(col) < 20)
		return; // no text if narrow
	UINT nFormat = DT_SINGLELINE | DT_CENTER | DT_VCENTER;
	UINT z = 0;
	UINT q = 0;
	UINT Frame = row;
	UINT Level = m_pSheet->m_Grid.GetColumnCount() - col - 1;
	char buf[80];
	buf[0] = 0;
	if (Level >= m_Levels)
		{
		q = 1;
		if (col)
			{
			if(!Frame)
				{
				q = 2;
				sprintf(buf,"Sound %d",col);
				}
			}
		else if (Frame)
			sprintf(buf,"%d", Frame);
		else
			buf[0] = 0;
		}
	else
		{
		if (Frame)
			{
			if (m_bHaveThumbs)
				nFormat = DT_SINGLELINE | DT_RIGHT | DT_BOTTOM;
			z = StackFind(Frame-1, Level,1);
			m_pScene->CellName(buf, Frame-1, Level);
//			sprintf(buf,"X%c-%dX",'A' + Level,Frame);
			}
		else
//if (Level)
			{
			q = 2;
			m_pScene->LevelName(buf, Level);
			}
//			sprintf(buf,"ZLevel %dZ",Level);
//		else
//			sprintf(buf,"BG");
		}
	COLORREF crOld,crNew;
//	if (z || m_bHaveThumbs)
		crOld = pDC->GetTextColor();
	if (q)
		crNew = crLevelTxt;
	else if (z == 1)
    	crNew = crOpenTxt;
	else if (z && m_bXSheetHighlight)
    	crNew = crStackTxt;
	else // if (m_bHaveThumbs)
    	crNew = crNormalTxt;
    pDC->SetTextColor(crNew);
	if (q == 2)
		{
		RECT trect = *rect;  // center text aboove icons
#if MAC
		trect.top += 5;
#endif
		trect.bottom -= MIN_WIDTH;
	    pDC->DrawText(buf,-1, (LPRECT)&trect, nFormat);
		}
	else
		pDC->DrawText(buf,-1, (LPRECT)rect, nFormat);
	pDC->SetTextColor(crOld);
}
BOOL CMainFrame::SetupThumb()
{
	delete m_pThumbs;
	m_pThumbs = 0;
	delete [] m_pDib;
	m_pDib = 0;
	DWORD dwTemp;
	LPBITMAPINFOHEADER lpBI;
	BYTE * lpDib;
	UINT i;
	UINT height = HEIGHT;
//	if (m_pScene->SceneOptionInt(SCOPT_BLIND))
	if (m_pDoc->Option(SC_BLIND))
		height = 100;
	m_twidth = m_pScene->Width();
	m_theight = m_pScene->Height();
	UINT scale = m_theight / height;
	m_theight /= scale;
	m_twidth /= scale;
	m_tpitch = 4 *((BPP*m_twidth + 3) / 4);
	m_tsize = 4 + m_tpitch * m_theight;	// plus 4 for flags at front
	UINT thumbc = m_Frames * m_Levels;
	m_pThumbs = new CThumbs(m_Frames, m_Levels, m_tsize);
	if (!m_pThumbs)
		{
		AfxMessageBox(IDS_NOMEMORY);
		return 1;
		}
//	dwTemp = m_tsize * (1 + thumbc);
	dwTemp = m_tsize + sizeof(BITMAPINFOHEADER);
	if (BPP == 1)
		dwTemp += 1024;
	m_pDib = new BYTE[dwTemp];
	lpBI = (LPBITMAPINFOHEADER)(m_pDib + m_tsize);
	lpBI->biSize			= sizeof(BITMAPINFOHEADER);
	lpBI->biWidth			= m_twidth;
	lpBI->biHeight			= m_theight;
	lpBI->biPlanes			= 1;
	lpBI->biBitCount		= BPP > 1 ? 24 : 8;
	lpBI->biCompression		= BI_RGB;
	lpBI->biSizeImage		= m_tsize - 4;
	lpBI->biXPelsPerMeter	= 0;
	lpBI->biYPelsPerMeter	= 0;
	lpBI->biClrUsed			= BPP > 1 ? 0 : 256;
	lpBI->biClrImportant	= 0;
	lpDib = (BYTE *)lpBI;
	lpDib += lpBI->biSize;
	if (BPP == 1)
		{
		for (i = 0; i < 1024; i++)
			{
//			if ((i % 4) == 3)
//				lpDib[i] = 0;
//			else if ((i % 4) == 2)
//				lpDib[i] = 0;
//			else
				lpDib[i] = (char)(((i / 4) * (i / 4)) / 255); // gamma boost
			}
		}
	UINT j;
	m_bDirtyThumbs = m_pScene->Flag(SCN_FLG_DIRTY);	// dirty flag
	for ( j = 0; j < m_Levels; j++)
	for (i = 0; i < m_Frames;i++)
		{
		BYTE * pThumb = m_pThumbs->Buf(i,j);
		if (m_bDirtyThumbs)
				*((DWORD *)pThumb) = -1; // force update
		else
			{
			*((DWORD *)pThumb) = 0;
			m_pScene->GetThumb(4+pThumb, i, j, m_twidth, m_theight,j ? BPP : 3);
			}
		}
	return 0;
}

BOOL CMainFrame::CheckFrames(UINT frames)
{
	if (frames > m_max_frames)
		{
		AfxMessageBox(IDS_FRAME_COUNT_EXCEEDED);
		return 1;
		}
	return 0;
}

BOOL CMainFrame::ChangeFrames(UINT Start, UINT End)
{
DPF("change frames,start:%d,end:%d",Start,End);
	int z;
	if (Start == End)
		{
		if (CheckFrames(Start))
			return 1;
		z = m_pThumbs->Append(Start - m_Frames);
		}
	else if (Start < End)
		{
		if (CheckFrames(m_Frames + End - Start))
			return 1;
		z = m_pThumbs->Insert(Start, End - Start);
		m_bDirtyThumbs = 1;
		}
	else
		{
		z = m_pThumbs->Delete(End,Start - End);
		}
	if (z)
		{
		DPF("no,memory");
		AfxMessageBox(IDS_NOMEMORY);
		return 1;
		}
	if (Start != End)
		{
		m_pSound->Allow(0);
		int idd;
		for (idd = 0; idd < 3; idd++)
			{
			UINT vmark = m_pScene->SceneOptionInt(SCOPT_VMARK0+idd);
			UINT oldmark = vmark;
			if (Start < End)
				{
				if (vmark > Start)
					vmark += End - Start;
				}
			else
				{
				if (vmark > Start)
					vmark -= Start - End;
				}
			if (vmark != oldmark)
				{
				double smark = m_pScene->SceneOptionInt(SCOPT_SMARK0+idd);
				m_pSound->SetAVMarks(idd,smark / 1000.0, vmark+1);
				m_pScene->SceneOptionInt(SCOPT_VMARK0+idd,1,vmark);
				}
			}
		m_pSound->Allow(1);
		}
	m_Frames = m_pThumbs->Count();
	m_pScene->ScnChangeFrames(Start,End);
	m_pSound->SetFrameCount(m_Frames);
	m_pSheet->m_Grid.SetRowCount(m_Frames+1);
	m_pSlider->SetRange(m_Frames);
	if (m_pCamera)
		m_pCamera->SetFrames();
	if (m_bShowSound)
		RedrawSound();
	if (m_pScan)
		m_pScan->GetChanges(1);
	return 0;
}

void CMainFrame::SlideCells(UINT From,  UINT To, 
				UINT StartLevel, UINT EndLevel, UINT Count)
{
#ifdef MYBUG
DPF("mainslide cells,from:%d,to:%d,sl:%d,el:%d,cnt:%d",
		From, To, StartLevel, EndLevel, Count);
#endif
	StackClear();
	m_pScene->SlideCells(From, To,StartLevel,EndLevel,Count);
	if ((m_stack_level >= StartLevel) && (m_stack_level <= EndLevel)
			&& (m_stack_frame >= From) && (m_stack_frame < (From+Count)))
		m_stack_frame += To - From;
//	m_pegbar.PegSlide(From, To,StartLevel,EndLevel,Count);
	CSketchView * pView = (CSketchView *)GetActiveView();
	if (!m_bTiming) { // hack
		pView->Update(m_stack_frame,m_stack_level,1);
	}
DPF("after scene slide");
	UINT size = m_tsize * (EndLevel + 1 - StartLevel);
	UINT Limit = m_Frames;
	if (To < From)
		{
		if (Count)
			Limit = From + Count;
		for (; To < Limit; To++, From++)
			{
			BYTE * dst = m_pThumbs->Buf(To, StartLevel);
			if (From < Limit)
				memcpy(dst, m_pThumbs->Buf(From,StartLevel), size);
			else
				memset(dst, 255, size);
			}
		}
	else
		{
//		if ((To + Count) > m_Frames)
//			Count = m_Frames - To;
//		else
		if (Count)
			Limit = To + Count;
DPF("limit:%d,from:%d,to:%d",Limit,From,To);
		for (; --Limit >= From;)
			{
			BYTE * dst = m_pThumbs->Buf(Limit, StartLevel);
			UINT f = Limit + From - To;
			if (Limit >= To)
				memcpy(dst, m_pThumbs->Buf(f,StartLevel), size);
			else
				memset(dst, 255, size);
			if (!Limit)
				break;
			}
		}
	m_bDirtyThumbs = 1;
	StackReload();
}

void CMainFrame::UpdateCell(UINT Frame, UINT Level)
{
DPF("update cell, f:%d,l:%d",Frame,Level);
	DWORD	key = m_pScene->BlowCell(Frame, Level);
	if (!key) return;
	DWORD   cnt = m_pScene->RefCount(key); // w/o clipboard
DPF("key:%x,cnt:%d",key,cnt);
	UpdateThumb(Frame, Level);
	m_pScene->UpdateCache(Frame, Level);
	if (!cnt)
		return;
	UINT f, l;
	for (f = 0; cnt && (f < m_Frames); f++)
		{
		for (l = 0; cnt && (l < m_Levels); l++)
			{
			if ((f == Frame) && (l == Level))
				continue;
			if (m_pScene->GetCellKey(f,l) == key)
				{
				UpdateThumb(f,l);
				m_pScene->UpdateCache(f,l);
				cnt--;
				}
			}
		}
}

void CMainFrame::UpdateThumb(UINT Frame, UINT Level)
{
DPF("update thumb, f:%d,l:%d",Frame,Level);
	if (Frame >= m_Frames)
		{
		if (ChangeFrames(Frame + 1 , Frame + 1))
			return;
		}
	UINT Stop = m_pScene->UpdateCache(Frame, Level);
	if (Stop >= m_Frames)
		Stop--;
	Stop = Frame;
	for (;Frame <= Stop;Frame++)
		{
	BYTE * pThumb = m_pThumbs->Buf(Frame,Level);
	m_pScene->GetThumb(4+pThumb, Frame, Level, 
					m_twidth, m_theight,Level ? BPP : 3,1);
	* ((DWORD *)pThumb) = 0;
	RedrawGrid(Frame,Level);
		}
}

void CMainFrame::UpdateLevelThumbs(UINT Level)
{
	UINT i;
	BYTE * pThumb;
	for (i = 0; i < m_Frames;i++)
		{
		pThumb = m_pThumbs->Buf(i,Level);
		* ((DWORD *)pThumb) = -1;
		}
	m_bDirtyThumbs = TRUE;
}

void CMainFrame::DirtyThumb(UINT PalIndex /*= NEGONE */)
{
	UINT level;
	for ( level = 0; level < m_Levels; level++)
		{
		if ((PalIndex == NEGONE) || (m_pScene->PalIndex(level) == PalIndex))
			{
			UpdateLevelThumbs(level);
			}
		}
	m_pScene->Flag(SCN_FLG_DIRTY,1,1);	// sets dirty flag
}

void CMainFrame::GetHeaderIcons()
{
	int w,h,size;
	w = 16;
	h = 16;
	size = h * 4 * ((3 * w + 3) / 4);
	LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)(m_pHdrIcons);
	lpBI->biSize			= sizeof(BITMAPINFOHEADER);
	lpBI->biWidth			= w;
	lpBI->biHeight			= h;
	lpBI->biPlanes			= 1;
	lpBI->biBitCount		= 24;
	lpBI->biCompression		= BI_RGB;
	lpBI->biSizeImage		= size;
	lpBI->biXPelsPerMeter	= 0;
	lpBI->biYPelsPerMeter	= 0;
	lpBI->biClrUsed			= 0;
	lpBI->biClrImportant	= 0;
	int result = 0;
	int i;
	BYTE * pData = m_pHdrIcons + lpBI->biSize;
	for (i = 0; i < 7; i++)
		{
		int idd = IDB_COL_THIN + i;
		HRSRC hRes = FindResource(AfxGetApp()->m_hInstance,
					MAKEINTRESOURCE(idd),RT_BITMAP);
		if (!hRes)
			continue;
		HGLOBAL hand = LoadResource(NULL , hRes);    
		if (!hand)
			continue;
		BYTE * pSrc = (BYTE *) LockResource(hand);       
		if (!pSrc)
			{
			continue;
			}
		LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)pSrc;
//		UINT ww = lpBI->biWidth;
//		UINT hh = lpBI->biHeight;
//		UINT dd = lpBI->biBitCount;
//		DPF("c:%d,src %d,%d,%d", c,ww,hh,dd);
		pSrc +=  40;	// to bitmap data
		memmove(pData, pSrc, size);
		pData += size;
		}
}

void CMainFrame::DoHeaderIcon(CDC * pDC, int which,LPCRECT rect, BOOL bSel)
{
	LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)(m_pHdrIcons);
	int x,y,w,h;
	w = 16;
	h = 16;
	lpBI->biSize			= sizeof(BITMAPINFOHEADER);
	lpBI->biWidth			= w;
	lpBI->biHeight			= h;
	lpBI->biPlanes			= 1;
	lpBI->biBitCount		= 24;
	lpBI->biCompression		= BI_RGB;
	lpBI->biSizeImage		= 3*w*h;
	lpBI->biXPelsPerMeter	= 0;
	lpBI->biYPelsPerMeter	= 0;
	lpBI->biClrUsed			= 0;
	lpBI->biClrImportant	= 0;
	if (which == 6)					// info centered
		x = (rect->left + rect->right - w) / 2;
	else if (which / 2)
		x = rect->left;
	else
		x = rect->right - w;
	y = rect->bottom - h;
	int i;
	BYTE r,g,b;
	if (bSel)
		{
		r = GetRValue(crLevelSel);
		g = GetGValue(crLevelSel);
		b = GetBValue(crLevelSel);
		}
	else
		{
    	COLORREF cr3DFace       = ::GetSysColor(COLOR_3DFACE);
		r = GetRValue(cr3DFace);
		g = GetGValue(cr3DFace);
		b = GetBValue(cr3DFace);
		}
	UINT p = 4 * ((3 * w + 3) / 4);
	BYTE * pSrc = m_pHdrIcons + 40 + p * h * which;
	BYTE * pDst = m_pHdrIcons + 40 + p * h * 7; // to temp area

	for (i = 0; i < h; i++)
		{
		int j;
		for (j = 0;j < w; j++)
			{
			if ((pSrc[p*i+3*j+0] > 200) &&
				(pSrc[p*i+3*j+1] > 200) &&
				(pSrc[p*i+3*j+2] > 200))
				{
				pDst[p*i+3*j+0] = b;
				pDst[p*i+3*j+1] = g;
				pDst[p*i+3*j+2] = r;
				}
			else
				{
				pDst[p*i+3*j+0] = pSrc[p*i+3*j+0];
				pDst[p*i+3*j+1] = pSrc[p*i+3*j+1];
				pDst[p*i+3*j+2] = pSrc[p*i+3*j+2];
				}
			}
		}
	BOOL bSuccess;
	if (!pDC->IsPrinting())
		{
		bSuccess = ::SetDIBitsToDevice(pDC->m_hDC,x,y,w,h,
			0, 0,
			0, h, pDst,
			(LPBITMAPINFO)lpBI, DIB_RGB_COLORS);
		}
	else
		{
		w = rect->right - x;
		h = rect->bottom - y;
		bSuccess = ::StretchDIBits(pDC->m_hDC,x,y,w,h,
				0, 0, m_twidth,m_theight,pDst,
				(LPBITMAPINFO)lpBI, DIB_RGB_COLORS, SRCCOPY);
		}
	
}

void CMainFrame::DrawThumb(CDC * pDC, LPCRECT rect,
					UINT row, UINT col, DWORD state)
{
	if (!m_pDoc || !m_pScene)
		return;
	if (XlateRowCol(row,col))
		return;
	if (m_Kind & 4) 				// top row ?
		{
		m_Kind ^= 4;
		if (m_Kind == 2)
			return;					// left column
		DWORD v;
		if (m_Kind == 1)			// sound
			v = m_pSound->Enabled(m_Level);
		else
			v = m_pScene->LevelFlags(m_Level) & 1;
		if (v && (state != 99))
			pDC->FillSolidRect(rect, crLevelSel);
#if MAC
		if (row == 0) {
			if (v) {
				pDC->FillSolidRect(rect, crLevelSel);
			}
			else {
				pDC->FillSolidRect(rect, ::GetSysColor(COLOR_3DFACE));			
			}
		}
#endif
		int ww = m_pSheet->m_Grid.GetColumnWidth(col);
		DPF("lhdr,row:%d,col:%d,ww:%d",row,col,ww);
		DPR("lhdr",(LPRECT)rect);
		if (ww <= MIN_WIDTH)
			{
			DoHeaderIcon(pDC,0,rect,v);//IDB_THIN,rect,0);
			}
		else
			{
			DoHeaderIcon(pDC, v + (m_Kind == 1 ? 4 : 2), rect,v);
			DoHeaderIcon(pDC,6,rect,v);
			DoHeaderIcon(pDC,1,rect,v);//IDB_WIDE,rect,0);
			}
		return;
		}
	if (m_Kind == 2)		// left column
		{
		if (state == 99)
			return;
		if ((m_Frame+1) == m_vFrame)
			pDC->FillSolidRect(rect, RGB(0,200,0));
		else
			pDC->FillSolidRect(rect, RGB(192,192,192));
		return;
		}
	UINT z = 0;
	if (!m_bTiming)
		{
		if (state < 99)
			z = StackFind(m_Frame, m_Level,1);
		else
			state = 0;
		}
	COLORREF crColor;
	CSketchView * pView = (CSketchView *)GetActiveView();
	if (m_Kind == 1)			// sound
		crColor = crNormal;
	else if (z == 1)	// top of stack
		crColor = !state ? crOpen : crOpenSel;
	else if (z && (pView->ShiftTrace(m_Frame, m_Level,0) == 1))
		crColor = RGB(255,0,0);
	else if (z && m_bXSheetHighlight)
		crColor = !state ? crStack : crStackSel;
	else
		crColor = !state ? crNormal : crNormalSel;
	pDC->FillSolidRect(rect,crColor);
	if (!m_pDoc->Option(SHOWTHUMBS))
		{
//		pDC->FillSolidRect(rect, crColor);
		return;
		}
	UINT min;
	if (!m_Level)
		min = m_pDoc->Option(BGMIN);
	LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)(m_pDib + m_tsize);
	HPBYTE hpBuf;
	int color = 0;
	int x,y,w,h;
	int dx, dw;
	dx = 0;
	x = rect->left;
	y = rect->top;
	w = rect->right - x;
	h = rect->bottom - y;
	if ((UINT)w > m_twidth)
		dw = w = (int)m_twidth;
	else
		{
		dw = w;
		dx = (m_twidth - w) / 2;
		}
	BYTE * pTemp;
	pTemp = m_pDib;
	hpBuf = m_pThumbs->Buf(m_Frame, m_Level);
	if (m_Kind == 1)
		{
		BYTE * hpSnd = m_pSound->Image(m_Level, m_Frame);
		if (!hpSnd)
			return;
		UINT xx,yy,p,z;
		z = 4 * ((3 * m_twidth+3) / 4);
		p = 4 * ((m_twidth+3)/4);
		for (yy = 0; yy < m_theight;yy++)
			{
			for (xx = 0; xx < m_twidth;xx++)
				{
				BYTE v = hpSnd[yy*p+xx];
				pTemp[yy*z+3*xx+0] = v;
				pTemp[yy*z+3*xx+1] = v;
				pTemp[yy*z+3*xx+2] = v;
				}
			}
		}
	else if (*((DWORD *)hpBuf))
		{
//		pDC->FillSolidRect(rect, RGB(128,128,128));
		pDC->FillSolidRect(rect, RGB(255,255,255));
		return;
		}
	else
		{
		hpBuf += 4;
	BYTE r = GetRValue(crColor);
	BYTE g = GetGValue(crColor);
	BYTE b = GetBValue(crColor);
	UINT z = 4 * ((3 * m_twidth + 3) / 4);
	UINT p,q;
	if (m_Level) p = m_tpitch; else p = z;
	if (m_Level) q = 4; else q = 3;
	UINT xx,yy;
	for (yy = 0; yy < m_theight;yy++)
		{
		for (xx = 0; xx < m_twidth;xx++)
			{
			UINT af;
			if (m_Level)
				af = hpBuf[m_tpitch*yy+4*xx+3];
			else
				af = 128;
			UINT ab = 255 - af;
//			ab = (ab * ab) / 255;
//			af = 255 - ab;
			pTemp[yy*z+3*xx+0] = (b*ab + af * hpBuf[p*yy+q*xx+0]) / 255;
			pTemp[yy*z+3*xx+1] = (g*ab + af * hpBuf[p*yy+q*xx+1]) / 255;
			pTemp[yy*z+3*xx+2] = (r*ab + af * hpBuf[p*yy+q*xx+2]) / 255;
			}
		}
	if (m_pCamera && m_pCamera->IsKey(m_Frame, m_Level))
		{
		for (yy = 0; yy < 16;yy++)
			{
			for (xx = 0; xx <= (15-yy);xx++)
				{
				pTemp[(m_theight-1-yy)*z+3*xx+2] = 255;
				pTemp[(m_theight-1-yy)*z+3*xx+1] = 0;
				pTemp[(m_theight-1-yy)*z+3*xx+0] = 0;
				}
			}
		}
	}
	BOOL bSuccess;
	if (!pDC->IsPrinting())
	{
	bSuccess = ::SetDIBitsToDevice(pDC->m_hDC,x,y,w,h,
			dx, m_theight - h,
//			0, m_theight, hpBuf,
			0, m_theight, pTemp,
			(LPBITMAPINFO)lpBI, DIB_RGB_COLORS);
	}
	else
	{
	w = rect->right - x;
	h = rect->bottom - y;
	bSuccess = ::StretchDIBits(pDC->m_hDC,x,y,w,h,
			0, 0, m_twidth,m_theight,
			pTemp,
			(LPBITMAPINFO)lpBI, DIB_RGB_COLORS, SRCCOPY);
	}
}
/*
BYTE * CMainFrame::GetThumb( UINT Frame, UINT Level)
{
	BYTE * hpBuf = m_ppThumb;
	hpBuf += 40;
	if (BPP ==1 )
		hpBuf += 1024;
//	UINT z = Level * m_Frames + Frame;
	UINT z = Frame * m_Levels + Level;
	z *= m_tsize;
	hpBuf += z;
	return hpBuf;
}
*/
void CMainFrame::ClearThumb( UINT Frame, UINT Level)
{
	HPBYTE hpBuf;
	hpBuf = 4 + m_pThumbs->Buf(Frame, Level);
	memset(hpBuf, 255,m_tsize-4);
}

void CMainFrame::RedrawSound()
{
	if (m_bShowSound)
		{
		int i;
		for (i = 0; i < m_nSoundCols; i++)
			m_pSheet->m_Grid.RedrawColumn(i+1);
		}
}

void CMainFrame::RedrawGrid(UINT Frame, UINT Level, UINT w, UINT h)
{
	UINT i,j;
//DPF("redraw,f:%d,l:%d,w:%d,h:%d",Frame,Level,w,h);
	if (Frame == 9999)
		{
		m_pSheet->Invalidate(1);
		return;
		}
	if (Level == 9999)
		{
		h = 1;
		Level = 0;
		w = m_Levels;
		}
	UINT cc = m_pSheet->m_Grid.GetColumnCount();
	for (i = 0; i < w; i++)
	for (j = 0; j < h; j++)
		m_pSheet->m_Grid.RedrawCell(Frame+j+1,cc-Level-i-1);
//	m_pSheet->m_Grid.EnsureVisible(Frame+1,cc-Level-1);
}

//
//	updown = 0 is up; 1 is down
//
void CMainFrame::SlideIt(int updown, UINT Count)
{
	DPF("main slide:%d,%d",updown,Count);
//return;
	UINT StartFrame, EndFrame, StartLevel, EndLevel;
//	int cc = m_pSheet->m_Grid.GetColumnCount();
	int cr = m_pSheet->m_Grid.GetRowCount();
//DPF("cc:%d,cr:%d",cc,cr);
	if (!GetSelection(StartFrame,StartLevel, EndFrame, EndLevel))
		return;
DPF("sliding:f1:%d,f2:%d,l1:%d,l2:%d",StartFrame, EndFrame, StartLevel,EndLevel);
	UINT hh = EndFrame - StartFrame + 1;
	UINT Level = StartLevel;
	UINT Frame;
	if (updown == 1) 
		{
		if (CheckSelection(m_Frames - Count, StartLevel, m_Frames - 1, EndLevel))
			{
			if (ChangeFrames(m_Frames , m_Frames + Count))
				return;
			Frame = m_Frames;
			}
		Frame = m_Frames - 1;
		EndFrame = Frame - Count;
		}
	else
		{
		if (Count > StartFrame)
			return;
		EndFrame = cr - 2;
		Frame = StartFrame - Count;
		}
DPF("frame:%d",Frame);
DPF("sliding:f1:%d,f2:%d,l1:%d,l2:%d",StartFrame, EndFrame, StartLevel,EndLevel);
//	if (StartLevel != EndLevel)
//		return;
	if (StartLevel != Level)
		return;
	if (StartFrame == Frame)
		return;
	if (Frame < StartFrame)
		{
		UINT h = EndFrame + 1 - StartFrame;
		UINT d = StartFrame - Frame;
		if (!DeleteSelection(Frame, StartLevel,StartFrame-1,EndLevel))
			return;
		SlideCells(StartFrame, Frame, 
			StartLevel,EndLevel,h);
		SetSelection(Frame, StartLevel,Frame + hh - 1, EndLevel);
		RedrawGrid(Frame, StartLevel, EndLevel+1-StartLevel, d + h);
		}
	else if (Frame > EndFrame)
		{
		if (!DeleteSelection(EndFrame+1, StartLevel,Frame,EndLevel))
			return;
		UINT h = EndFrame + 1 - StartFrame;
		UINT d = Frame - EndFrame;
DPF("h:%d,d:%d",h,d);
		SlideCells(StartFrame, StartFrame+d, StartLevel,EndLevel,h);
		SetSelection(StartFrame + d, StartLevel,
				StartFrame + d + hh - 1, EndLevel);
		RedrawGrid(StartFrame, StartLevel, EndLevel+1-StartLevel,d+h);
		}
}

void CMainFrame::CheckReplaceFix()
{
	if (m_ChkRepFrame)
		{
		SelectCell(m_ChkRepFrame-1, m_ChkRepLevel-1, 9);
		}
	
}
BOOL CMainFrame::CheckReplace(UINT Frame, UINT Level,
						UINT w, UINT h, UINT kind /* = 0 */)
{
	UINT x,y;
	m_ChkRepFrame = 0;
	BOOL bViewDirty = 0;
	for (x = 0; x < w; x++)
	for (y = 0; y < h; y++)
		{
		if ((m_stack_frame == Frame+y) && (m_stack_level == Level+x))
			{
			CSketchView * pView = (CSketchView *)GetActiveView();
			bViewDirty = pView->Modified();
			m_ChkRepFrame = 1 + Frame + y;
			m_ChkRepLevel = 1 + Level + x;
			}
		}
	if (!bViewDirty && !CheckSelection(Frame, Level, Frame+h-1, Level+w-1))
		return TRUE;
	if (kind == 2)
		kind = IDD_DEL_CELLS;
	else if (kind == 1)
		kind = IDD_CUT_CELLS;
	else
		kind = IDD_REPLACE;
	CReplaceDlg dlg(kind);
	dlg.m_StartFrame = Frame + 1;
	dlg.m_StartLevel = Level;
	dlg.m_EndFrame = Frame + h - 1 + 1;
	dlg.m_EndLevel = Level + w - 1;
	if (dlg.DoModal() == IDOK)
		return TRUE;
	else
		return FALSE;
}

void CMainFrame::DropFile(LPCSTR szName)
{
	DPF("main drop file:%s",szName);
	SoundProp(9,szName);
}

void CMainFrame::Drop(UINT row, UINT col, BOOL bCopy)
{
	if (XlateRowCol(row,col))
		return;
	if (m_Kind) return;
	DPF("frm drop, frm:%d,lvl:%d",m_Frame, m_Level);
	UINT f, l;
	f = m_Frame;
	l = m_Level;
	UINT StartFrame, EndFrame, StartLevel, EndLevel;
	if (!GetSelection(StartFrame,StartLevel, EndFrame, EndLevel))
		return;
	UINT w = EndLevel + 1 - StartLevel;
	UINT h = EndFrame + 1 - StartFrame;
	if (l + w > m_Levels)
		{
	int answer = AfxMessageBox(IDS_WIDE_PASTE, MB_OKCANCEL);
		if (answer != IDOK)
			return;
		w = m_Levels - l;
		}
	if (CheckReplace(f,l,w,h))
		{
#ifdef FBVER7
		BOOL bBreak = 1;
		if (bCopy)
			{
			int answer = AfxMessageBox(IDS_PASTE_LINK, MB_YESNOCANCEL);
			if (answer == IDCANCEL)
				return;
			bBreak = (answer == IDNO) ? 1 : 0;
			}
#endif
		if (CutCopy(bCopy))
			return;
		m_bNeedExtClip = 0;		// not draggable, Yet
#ifdef FBVER7
		Paster(f,l,0,bBreak);
#else
		Paster(f,l,0,0);
#endif
		CheckReplaceFix();
		}
}

BOOL CMainFrame::CutCopy(BOOL bCopy)
{
DPF("cutcopy:%d",bCopy);
	CSketchView * pView = m_pDoc->GetDocView();
	if (pView->CutCopy(bCopy ? 2 : 1))
		return 1;
//	if (pView->IsSelectTool())
//		{
//		if (AfxMessageBox(IDS_CUT_WARNING, MB_YESNO) != IDYES)
//			return 1;
//		}
	pView->HasPaste(2); // clear image stuff
	RECT rect;
	int cc = m_pSheet->m_Grid.GetColumnCount();
	m_pSheet->GetSelection(&rect);
	int x,y,w,h,i,j;
	x = cc - rect.left - 1;
	y = rect.top - 1;
	w = rect.right - rect.left;
	h = rect.bottom - rect.top;
	x = x + 1 - w;
DPF("frm:%d,lvl:%d,w;%d,h:%d",y,x,w,h);
	int edit_frame = pView->CurrentFrame();
	int edit_level = pView->CurrentLevel();
	BOOL bActive = 0;
	if (!bCopy && (edit_frame >= y) && (edit_frame < (y + h)) &&
		(edit_level >= x) && (edit_level < (x + w)))
		{
		bActive = 1;
		pView->ForceSave(1);
//		AfxMessageBox(IDS_NO_CUT_EDIT, MB_OK);
//		return 1;
		}
	m_pScene->CutCopy(y,x,w,h,bCopy);
	m_bNeedExtClip = 1;
//	if (!bCopy)  // cut it
//		pView->ClearCell();
	if(OpenClipboard())
		{
		EmptyClipboard();
		SetClipboardData(m_MyClipCell,NULL);
		CloseClipboard();
		}
	if (!bCopy)
		{
		m_nViewDirty |= 1;
		StackClear();
		BOOL bFlag = 0;
		for (i = 0; i < w; i++)
			{
			for (j = 0; j < h; j++)
				{
				if (((y+j) == m_stack_frame) && ((x+i) == m_stack_level))
					m_nViewDirty |= 4;
//				m_pScene->DeleteCell1(y+j,x+i,FALSE);
				}
			for (j = 0; j < h; j++)
				UpdateThumb(y+j,x+i);
			}
		StackReload();
		RedrawGrid(y,x,w,h);
		}
	return 0;
}

void CMainFrame::OnUpdateCutCopy(CCmdUI* pCmdUI, UINT Id)
{
	CSketchView * pView = m_pDoc->GetDocView();
	CString t;
	BOOL bFlag = FALSE;
	UINT flags = MF_BYCOMMAND | MF_STRING;
	if (pView && pView->CutCopy())
		{
		bFlag = TRUE;
		if (Id == ID_EDIT_COPY)
			t = "Copy Image\tCtrl+C";
		else
			t = "Cut Image\tCtrl+X";
		}
	else
		{
		if (Id == ID_EDIT_COPY)
			t = "Copy\tCtrl+C ";
		else
			t = "Cut\tCtrl+X";
		if (m_pSheet->m_Grid.GetSelectedCount() <= 0)
			flags |= MF_GRAYED;
		else
			bFlag = TRUE;
		}
	CMenu * pMenu = GetMenu()->GetSubMenu(1);
	BOOL tt = pMenu->ModifyMenu(Id, flags,Id, t);
//	DrawMenuBar();
	pCmdUI->Enable(bFlag);
}

BOOL CMainFrame::SetSelection(UINT StartFrame, UINT StartLevel,
			UINT EndFrame, UINT EndLevel)
{
	RECT rect;
	rect.top = StartFrame;
	rect.bottom = EndFrame + 1;
	rect.left = StartLevel;
	rect.right = EndLevel + 1;
DPR("setsel",&rect);
	m_pSheet->SetSelection(rect);
	if (StartFrame == EndFrame)
		{
		int cc = m_pSheet->m_Grid.GetColumnCount();
		m_pSheet->m_Grid.EnsureVisible(StartFrame,cc-StartLevel-1);
		}
	if (m_pScan) m_pScan->GetChanges();
	return 0;
}

BOOL CMainFrame::GetSelection(UINT& Frame, UINT &Level)
{
	RECT rect;
	m_pSheet->GetSelection(&rect);
	if (rect.top >= rect.bottom)
		return FALSE;
	if (XlateRowCol(rect.top, rect.left))
		return FALSE;
	if (m_Kind) 
		return FALSE;
	Level = m_Level;
	Frame = m_Frame;
	return TRUE;
}

BOOL CMainFrame::GetSelection(UINT& StartFrame, UINT &StartLevel,
						UINT & EndFrame, UINT& EndLevel)
{
	RECT rect;
	m_pSheet->GetSelection(&rect);
	if (rect.top >= rect.bottom)
		return FALSE;
	int cc = m_pSheet->m_Grid.GetColumnCount();
	StartLevel = cc - rect.right;
	StartFrame = rect.top - 1;
	EndFrame = StartFrame + rect.bottom - rect.top - 1;
	EndLevel = StartLevel + rect.right - rect.left - 1;
	return TRUE;
}

void CMainFrame::GridChar(int nChar, int nRep)
{
	DPF("main, grid char,%d,%d,%d",nChar,nRep,m_nDoubleDigit);
	UINT dd = m_nDoubleDigit;
	m_nDoubleDigit = 0;
	if ((nChar >= VK_NUMPAD0) && (nChar <= VK_NUMPAD9))
		nChar -= (VK_NUMPAD0 - '0');
	if ((GetKeyState(VK_CONTROL) & (1 << (sizeof(SHORT)*8-1))) == 0 )
		{
		if (((nChar >= 'a') && (nChar <= 'z')) ||
			((nChar >= 'A') && (nChar <= 'Z')))
			{
			if (!m_pDoc->Option(USE_MOUTHS))
				{
				MessageBeep(0);
				return;
				}
			UINT f, l;
			GetSelection(f,l);
			if (CheckReplace(f,l,1,1))
				{
				int z = m_pDoc->InsertMouth((nChar | 32) - 'a',f,l);
				if (z)
					{
					FormattedMsg(IDS_ERR_BAD_MOUTH,z);
					}
				else
					CheckReplaceFix();
//			else if ((f+1) < m_Frames)
//				SetSelection(f+1,l,f+1,l);
				}
			return;
			}
		}
	if (m_bTiming)
		{
		UINT f,l,z;
		z = 1;
		GetSelection(f,l);
		UINT min, max;
		m_pScene->ThumbMinMax(min,max, f, m_TimingLevel);
		switch(nChar) {
//        case VK_TAB:
        case VK_DOWN:
        case VK_RIGHT:
			f = max;
			break;
        case VK_UP:
        case VK_LEFT:
			f = min;
			break;
//        case VK_NEXT:
        case VK_END:
			f = m_Frames -1;
			break;
//        case VK_PRIOR:
        case VK_HOME:
			f = 0;
			break;
		default:
			z = 0;
			break;
		}
		if ((f != m_Frame) && (f < m_Frames))
			{
			UINT cc = m_pSheet->m_Grid.GetColumnCount();
			m_pSheet->m_Grid.EnsureVisible(f,cc-l-1);
			SelectCell(f,l,4);
			}
		if (z)
			return;
		}
	if ((nChar < '0') || (nChar > '9'))
		{
		if (m_bTiming)
			return;
		if (GetKeyState(VK_CONTROL) < 0)
			{
			if ((nChar | 32) == 'p')
				SendMessage(WM_COMMAND, ID_FILE_PRINT, 0);
			else if ((nChar | 32) == 'b')
				SendMessage(WM_COMMAND, ID_EDIT_BLANK, 0);
			else if ((nChar | 32) == 'h')
				SendMessage(WM_COMMAND, ID_EDIT_OUTSIDE, 0);
			}
		return;
		}
	nChar -= '0';
	if (dd)
		nChar += 10 * (dd - 100);
	else if (GetKeyState(VK_CONTROL) < 0)
		{
		m_nDoubleDigit = 100 + nChar;
		return;
		}
	else if (!nChar)
		return;
	if (!m_pScene) return;
	DPF("main, grid char,%d,%d,%d",nChar,nRep,m_nDoubleDigit);
	UINT StartFrame, EndFrame, StartLevel, EndLevel;
	int cc = m_pSheet->m_Grid.GetColumnCount();
	int cr = m_pSheet->m_Grid.GetRowCount();
	if (!GetSelection(StartFrame,StartLevel, EndFrame, EndLevel))
		return;
	if (StartFrame != EndFrame)
		return;
	if (StartLevel != EndLevel)
		return;
	UINT z = StackFind(StartFrame, StartLevel,1);
	if (z > 100)
		{
		MessageBeep(0);
		return;
		}
	UINT dest = StartFrame + nChar;
	UINT source;
	for (source = StartFrame + 1;source < m_Frames;source++)
		{
		if(m_pScene->GetCellKey(source, StartLevel))
			break;
		}
	if (source >= m_Frames)
		{
		if (dest > m_Frames)
			{
			ChangeFrames(m_Frames , dest);
			}
		return;
		}
	if (dest > m_Frames)
		{
		if (ChangeFrames(m_Frames , dest))
			return;
		}
	DPF("source:%d,dest:%d",source,dest);
	SetSelection(source, StartLevel, source, EndLevel);
	if (source > dest)
		SlideIt(0, source - dest);
	else if (dest > source)
		SlideIt(1, dest - source);
	SetSelection(dest,StartLevel,dest,EndLevel);
//	RedrawGrid(dest,StartLevel,dest,EndLevel);
	m_pSheet->SetFocus();
}

void CMainFrame::DoPaste(BOOL bReverse)
{
	DPF("pasting:%d",bReverse);
	if (!m_pDoc)
		return;
	CSketchView * pView = m_pDoc->GetDocView();
	if (pView->HasPaste(1))
		return;
	UINT f,l,z;
	if (!GetSelection(f,l))
		return;
	z = 0;
	if (!m_pScene->ClipEmpty())
		{
		z = NewPaste(f,l);
		if (z == 1)
			MessageBeep(0);
		if (z)
			return;
		z = 1;
		}
//	else
//		z = m_pScene->ClipNoLinks();
	UINT w, h;
	m_pScene->ClipInfo(w,h);
	if (!w || !h)
		return;
	if (l + w > m_Levels)
		{
		int answer = AfxMessageBox(IDS_WIDE_PASTE, MB_OKCANCEL);
		if (answer != IDOK)
			return;
		w = m_Levels - l;
		}
	if (CheckReplace(f, l, w, h))
		{
#ifdef FBVER7
		int answer;
		if (!z)
			answer = AfxMessageBox(IDS_PASTE_LINK, MB_YESNOCANCEL);
		else
			answer = IDNO;
		if (answer == IDCANCEL)
			return;
		BOOL bBreak = (answer == IDNO) ? 1 : 0;
		Paster(f,l,bReverse,bBreak);
#else
		Paster(f,l,bReverse);
#endif
		CheckReplaceFix();
		}
	if (z)
		m_pScene->ClipEmpty(1);
}

void CMainFrame::Paster(UINT Frame, UINT Level, BOOL bReverse, BOOL bBreak)
{
	UINT x,y,w,h,c;
	m_pScene->ClipInfo(w,h);
DPF("paster frm:%d,lvl:%d,w:%d,h:%d,r:%d",Frame,Level,w,h,bReverse);
	if (Frame + h > m_Frames)
		{
		if (ChangeFrames(Frame + h + 1 , Frame + h + 1))
			return;
		}
	if (Level + w > m_Levels)
		w = m_Levels - Level;
	StackClear();
	if (Level != m_pScene->ClipLevel())
		{
		for (x = 0; x < w; x++)		//  check if target level is virgin
		{
		CLevelTable srctbl, dsttbl;
		m_pScene->LevelTable(Level+x, &dsttbl,0);
		for (y = 0; y < 11; y++)
			{
//DPF("x:%d,y:%d,b:%d,f:%d",levtbl.table[y].dx,levtbl.table[y].dy,
//			levtbl.table[y].blur,levtbl.table[y].flags);
			if (dsttbl.table[y].dx || dsttbl.table[y].dy ||
					dsttbl.table[y].blur || ((dsttbl.table[y].flags|768)!= 768))
				break;
			}
		if (y >= 11)  // virgin layer table
			{
			m_pScene->LevelTable(m_pScene->ClipLevel()+x, &srctbl,0);
			for (y = 0; y < 11; y++)
				{
				srctbl.table[y].flags |= 768;
				srctbl.table[y].flags ^= 768;
				srctbl.table[y].flags |= (dsttbl.table[y].flags & 768);
				}
			m_pScene->LevelTable(Level+x, &srctbl,1);
			}
		}
		}
	UINT * keys = new UINT[w*h];
	c = 0;
	for (x = 0; x < w; x++)
	for (y = 0; y < h; y++)
		{
		UINT newkey;
		if (bReverse)
			newkey = m_pScene->ClipGet(x,h - 1 - y);
		else
			newkey = m_pScene->ClipGet(x,y);
		if (bBreak)
			newkey = m_pScene->DuplicateCell(newkey);
		UINT oldkey = m_pScene->SwapCellKey(Frame+y, Level+x,newkey);
		if (oldkey)
			keys[c++] = oldkey;
		UpdateThumb(Frame+y,Level+x);
//		m_pScene->UpdateCache(Frame+y,Level+x);
		}
	StackReload();
	RedrawGrid(Frame,Level,w,h);
	m_pScene->UpdateLinks(keys,c);
	delete [] keys;
	SetSelection(Frame, Level, Frame + h - 1, Level + w - 1);
	CSketchView * pView = (CSketchView *)GetActiveView();
	pView->CheckUpdate();
	return;
}

void CMainFrame::OnUpdatePasters(CCmdUI* pCmdUI, UINT Id)
{
	CSketchView * pView = m_pDoc->GetDocView();
	CString t;
	UINT flags = MF_BYCOMMAND | MF_STRING;
	BOOL bFlag = 0;
	if (Id == ID_EDIT_PASTE)
		{
		if (pView && pView->HasPaste())
			{
			t = "Paste Image\tCtrl+V";
			bFlag = 1;
			if (pView && pView->CutCopy())
				flags |= MF_GRAYED;
			}
		else
			t = "Paste\tCtrl+V";
		CMenu * pMenu = GetMenu()->GetSubMenu(1);
		BOOL tt = pMenu->ModifyMenu(Id, flags,Id, t);
//		DrawMenuBar();
		}
	if (!bFlag)
	{
	RECT rect;
	m_pSheet->GetSelection(&rect);
	if (rect.top >= rect.bottom)
		bFlag = FALSE;
	else
		{
//		DPR("rect",&rect);
		if (((rect.top + 1) == rect.bottom) &&
				(IsSound(rect.left)))
			bFlag = ::IsClipboardFormatAvailable(CF_WAVE);
		if (!bFlag)// && !m_bShowSound)
				bFlag = ::IsClipboardFormatAvailable(m_MyClipCell);
		}
	}
	pCmdUI->Enable(bFlag);
}

void CMainFrame::OnUpdateInsert(CCmdUI* pCmdUI)
{
	BOOL bFlag = FALSE;
	int z = 0;
	RECT rect;
	int cc = m_pSheet->m_Grid.GetColumnCount();
	int cr = m_pSheet->m_Grid.GetRowCount();
	m_pSheet->GetSelection(&rect);
DPR("updins",&rect);
	if ((rect.left == 1) && (rect.right == cc))
		z |= 1;
	if ((rect.top == 1) && (rect.bottom == cr))
		z |= 2;
	if (rect.left && (rect.right >= rect.left))
		z |= 4;
	if (rect.top && (rect.bottom >= rect.top))
		z |= 8;
DPF("z:%d",z);
	if (z && ((z & 3) != 3))
		bFlag = TRUE;
	pCmdUI->Enable(bFlag);
}

void CMainFrame::InsertDelete(BOOL bDelete)
{
	if (bDelete)
		OnEditDelete();
	else
		OnEditInsert();
}
void CMainFrame::OnEditInsert()
{
	UINT StartFrame, EndFrame, StartLevel, EndLevel;
	int cc = m_pSheet->m_Grid.GetColumnCount();
	int cr = m_pSheet->m_Grid.GetRowCount();
	if (!GetSelection(StartFrame,StartLevel, EndFrame, EndLevel))
		return;
DPF("insert:%d,%d,%d,%d",StartFrame, EndFrame, StartLevel,EndLevel);
	if ((StartLevel == EndLevel) && (StartFrame == 0) &&
			((EndFrame + 1) == m_Frames))
		{
DPF("insert level:%d",StartLevel);
		if (!((CSketchApp*)AfxGetApp())->CanDoLibrary())
			return;
		if (!StartLevel) return;
		UINT count = EndLevel + 1 - StartLevel;
		if (!m_pDoc || ((m_Levels + count) > m_pDoc->MaxLevels()))
			return;

		m_pScene->InsertLevel(StartLevel,count,m_twidth);
		StackClear();
		if ((m_stack_level >= StartLevel) && (m_stack_level <= EndLevel)
			&& (m_stack_frame >= StartFrame) && (m_stack_frame <= EndFrame))
			m_stack_level += count;
		if (!m_bTiming) { // hack
			CSketchView * pView = (CSketchView *)GetActiveView();
			pView->Update(m_stack_frame,m_stack_level,1);
			}
		m_pThumbs->InsertLevels(StartLevel, count);
		m_Levels += count;
		m_bDirtyThumbs = 1;
		StackReload();
		m_nViewDirty = 4;
		SetSelection(StartFrame,StartLevel,EndFrame,EndLevel);
		m_pSheet->SetRange(m_Frames,m_Levels);
		if (m_pScan)
			m_pScan->GetChanges(1);
		}
	else if ((StartLevel == 0) && ((EndLevel + 1) == m_Levels))
		{
DPF("got frame:%d,%d", StartFrame,EndFrame);
		if (ChangeFrames(StartFrame, EndFrame+1))
			return;
		m_pScene->UpdateCache(StartFrame);
		}
	else
		{
		m_nViewDirty = 1;
DPF("got cells:%d,%d,%d,%d",StartFrame, EndFrame, StartLevel,EndLevel);
		UINT h = m_Frames - EndFrame - 1;
		if ((m_stack_level >= StartLevel) && (m_stack_level <= EndLevel))
			{
			if (m_stack_frame >= StartFrame)
				{
				m_nViewDirty = 2;
				}
			}
//		SetSelection(StartFrame, StartLevel, StartFrame, EndLevel);
		SlideIt(1, EndFrame+1-StartFrame);
		SetSelection(StartFrame,StartLevel,EndFrame,EndLevel);
//		RedrawGrid(StartFrame, StartLevel, EndLevel+1-StartLevel,
//				EndFrame+1-StartFrame);
//		ChangeCells(StartFrame, EndFrame+1, StartLevel,EndLevel+1,0);
		}
}

void CMainFrame::OnUpdateDelete(CCmdUI* pCmdUI)

{
	BOOL bFlag = FALSE;
	int z = 0;
	RECT rect;
	int cc = m_pSheet->m_Grid.GetColumnCount();
	int cr = m_pSheet->m_Grid.GetRowCount();
	m_pSheet->GetSelection(&rect);
DPR("upddel",&rect);
	if ((rect.left == 1) && (rect.right == cc))
		z |= 1;
	if ((rect.top == 1) && (rect.bottom == cr))
		z |= 2;
	if (rect.left && (rect.right >= rect.left))
		z |= 4;
	if (rect.top && (rect.bottom >= rect.top))
		z |= 8;
DPF("z:%d",z);
	if (z && ((z & 3) != 3))
		bFlag = TRUE;
	pCmdUI->Enable(bFlag);
}

void CMainFrame::OnEditDelete()
{
	UINT StartFrame, EndFrame, StartLevel, EndLevel;
//	int cc = m_pSheet->m_Grid.GetColumnCount();
//	int cr = m_pSheet->m_Grid.GetRowCount();
	if (!GetSelection(StartFrame,StartLevel, EndFrame, EndLevel))
		return;
	if (!DeleteSelection(StartFrame, StartLevel,EndFrame, EndLevel))
		return;
DPF("delete:%d,%d,%d,%d",StartFrame, EndFrame, StartLevel,EndLevel);
	if ((StartLevel == EndLevel) && (StartFrame == 0) &&
			((EndFrame + 1) == m_Frames))
		{
DPF("delete level:%d",StartLevel);
//		if (!((CSketchApp*)AfxGetApp())->CanDoLibrary())
//			return;
		if (!StartLevel) return;
		UINT count = EndLevel + 1 - StartLevel;
		if (!m_pDoc || (m_Levels < 1))
			return;
		m_pScene->DeleteLevel(StartLevel,count);
		StackClear();
		if ((m_stack_level > EndLevel)
			&& (m_stack_frame >= StartFrame) && (m_stack_frame <= EndFrame))
			m_stack_level -= count;
		if (!m_bTiming) { // hack
			CSketchView * pView = (CSketchView *)GetActiveView();
			pView->Update(m_stack_frame,m_stack_level,1);
			}
		m_pThumbs->DeleteLevels(StartLevel, count);
		m_Levels -= count;
		m_bDirtyThumbs = 1;
		StackReload();
		m_nViewDirty = 4;
		SetSelection(StartFrame,StartLevel,EndFrame,EndLevel);
		m_pSheet->SetRange(m_Frames,m_Levels);
		if (m_pScan)
			m_pScan->GetChanges(1);
		}
	else if ((StartLevel == 0) && ((EndLevel + 1) == m_Levels))
		{
		if ((StartFrame == 0) && ((EndFrame + 1) == m_Frames))
			{
			DPF("delete all");
			StartFrame = 1;
			}
DPF("got frame:%d,%d", StartFrame, EndFrame);
		if (ChangeFrames(EndFrame+1,StartFrame))
			return;
		}
	else
		{
		m_nViewDirty = 1;
DPF("got cells:%d,%d,%d,%d",StartFrame, EndFrame, StartLevel,EndLevel);
		UINT h = m_Frames - EndFrame - 1;
		if ((m_stack_level >= StartLevel) && (m_stack_level <= EndLevel))
			{
			if (m_stack_frame >= StartFrame)
				{
				if (m_stack_frame > EndFrame)
					m_nViewDirty = 2;
				else
					m_nViewDirty = 4; 
				}
			}
		SlideCells(EndFrame+1, StartFrame, StartLevel,EndLevel,h);
		SetSelection(StartFrame, StartLevel,StartFrame, EndLevel);
		RedrawGrid(StartFrame, StartLevel, EndLevel+1-StartLevel, m_Frames-StartFrame);
//		ChangeCells(StartFrame, EndFrame+1, StartLevel,EndLevel+1,1);
//		SetSelection(EndFrame+1, StartLevel, EndFrame+1, EndLevel);
//		SlideIt(0, EndFrame+1-StartFrame);
//		SetSelection(StartFrame,StartLevel,EndFrame,EndLevel);
//		RedrawGrid(StartFrame, StartLevel, EndLevel+1-StartLevel,
//				EndFrame+1-StartFrame);
		}
}

void CMainFrame::OnUpdateBlank(CCmdUI* pCmdUI)
{
	BOOL bFlag = FALSE;
	RECT rect;
	m_pSheet->GetSelection(&rect);
	if (rect.top && ((rect.top + 1) == rect.bottom))
		bFlag = TRUE;
	pCmdUI->Enable(bFlag);
}

void CMainFrame::OnEditBlank()
{
	UINT StartFrame, EndFrame, StartLevel, EndLevel;
	if (!GetSelection(StartFrame,StartLevel, EndFrame, EndLevel))
		return;
	if (StartFrame != EndFrame)
		return;
	UINT w = EndLevel + 1 - StartLevel;
	UINT h = EndFrame + 1 - StartFrame;
	StackClear();
	UINT kCount = 0;
	UINT * pKeys = 0;
	if (CheckReplace(StartFrame,StartLevel,w,h))
		{
		pKeys = new UINT[w * h];
		UINT x,y;
		for (x = 0; x < w; x++)
			{
			for (y = 0; y < h; y++)
				{
				pKeys[kCount++] = 
					m_pScene->GetCellKey(StartFrame+y, StartLevel+x);
				}
			}
//	m_pScene->ClipClear(pKeys,kCount);// clear clipboard if any in this area
//		delete [] pKeys;
		for (x = 0; x < w; x++)
			{
			for (y = 0; y < h; y++)
				{
				m_pScene->BlankCell(StartFrame+y, StartLevel+x);
				}
			if (StartFrame > 0)
				UpdateThumb(StartFrame-1, StartLevel+x);
			for (y = 0; y < h; y++)
				{
				UpdateThumb(StartFrame+y, StartLevel+x);
				}
			UpdateThumb(StartFrame+h, StartLevel+x);
			}
		CheckReplaceFix();
		m_pScene->UpdateLinks(pKeys,kCount);
		delete [] pKeys;
		}
	StackReload();
	RedrawGrid(StartFrame,StartLevel,
					EndLevel+1-StartLevel,EndFrame+1 - StartFrame);
}

void CMainFrame::OnUpdateAppend(CCmdUI* pCmdUI)
{
	BOOL bFlag = FALSE;
	if (m_pDoc && (m_Levels < m_pDoc->MaxLevels()))
		bFlag = TRUE;
	pCmdUI->Enable(bFlag);
}

void CMainFrame::OnEditAppend()
{
	DPF("append level");
	if (!m_pDoc || (m_Levels >= m_pDoc->MaxLevels()))
		return;
	m_Levels += 1;
	m_pScene->SetLevelCount(m_Levels, m_twidth);
	m_pThumbs->AppendLevels(1);
	m_pSheet->SetRange(m_Frames,m_Levels);
	if (m_pScan)
		m_pScan->GetChanges(1);
}

BOOL CMainFrame::CheckSelection(UINT StartFrame, UINT StartLevel,
			UINT EndFrame, UINT EndLevel)
{
	UINT Frame, Level;
	for (Frame = StartFrame; Frame <= EndFrame; Frame++)
	for (Level = StartLevel; Level <= EndLevel; Level++)
		{
		if (m_pScene->GetCellKey(Frame, Level))
			return TRUE;
		}
	return FALSE;
}

BOOL CMainFrame::DeleteSelection(UINT StartFrame, UINT StartLevel,
			UINT EndFrame, UINT EndLevel)
{
	UINT w = EndLevel + 1 - StartLevel;
	UINT h = EndFrame + 1 - StartFrame;
	BOOL bResult;
	if (bResult = CheckReplace(StartFrame,StartLevel,w,h,2))
		{
		UINT Frame, Level;
		for (Level = StartLevel; Level <= EndLevel; Level++)
		for (Frame = StartFrame;Frame <= EndFrame; Frame++)
			{
			m_pScene->ChangeCellKey(Frame, Level, 1);
			}
		for (Level = StartLevel; Level <= EndLevel; Level++)
		for (Frame = StartFrame;Frame <= EndFrame; Frame++)
			{
			UpdateThumb(Frame,Level);
			}
		CheckReplaceFix();
		m_pScene->UpdateLinks(0,0);//pKeys,kCount);
		}
	return bResult;
}

void CMainFrame::SetBusy(int percent)
{
	if (percent != m_nBusy)
		{
		m_nBusy = percent;
		if (m_nBusy)
			{
			char temp[30];
			sprintf(temp,"%3d%%",100 - m_nBusy);
			m_pStatusBar->SetPaneText(1,temp);
			}
		else
			m_pStatusBar->SetPaneText(1,"Idle");
		}
}

BOOL CMainFrame::IdleProcess()
{
//	return 0;
	BOOL bResult = FALSE;
	CSketchView * pView = (CSketchView *)GetActiveView();
	if (pView->MouseDown())
		{
		return FALSE;
		}
	if (m_pDoc->Busy())
		return FALSE;
	if (m_nViewDirty)
		{
		pView->Redraw(m_nViewDirty / 2 );
		m_nViewDirty = 0;
		return TRUE;
		}
	if (pView->ViewMode())
		{
		if (m_pScene && !m_pScene->Flag(SCN_FLG_AUTOCMP))
			{
			int nResult = m_pScene->CheckComposite();
			SetBusy(nResult);
			if (nResult)
				return TRUE;
			}
		}
	if (m_bDirtyThumbs)
		{
		UINT i, j;
		for (j = 0; j < m_Levels; j++)
		for (i = 0; i < m_Frames;i++)
			{
			BYTE * pThumb = m_pThumbs->Buf(i,j);
			if (*((DWORD *)pThumb))
				{
				*((DWORD *)pThumb) = 0;
				m_pScene->GetThumb(4+pThumb, i, j,
						m_twidth, m_theight,j ? BPP : 3,1);
				RedrawGrid(i,j);
				return TRUE;
				}
			}
	DPF("thumbs finished");
		m_bDirtyThumbs = 0;
		m_pScene->Flag(SCN_FLG_DIRTY,1,0);	// clears dirty flag
		}
	if (m_pScene)
		{
		if (m_pScene->Flag(SCN_FLG_AUTOCMP))
			return bResult;
		int nResult = m_pScene->CheckComposite();
		SetBusy(nResult);
		if (nResult)
			bResult = TRUE;
		}
	return bResult;
}

void CMainFrame::LClick(UINT row, UINT col)
{
	DPF("lclick,row:%d,col:%d",row,col);
	if (GetKeyState(VK_MENU) >= 0)
		{
		if (XlateRowCol(row,col))
			return;
DPF("row:%d,col:%d,frm:%d,lvl:%d",row,col,m_Frame, m_Level);
		if (((m_Kind & 7) == 4) && m_pCamera)
			{
			UINT peg = m_pScene->Camera()->PegFindLevel(m_Level);
			m_pCamera->SelectPeg(peg);
			}
		else if ((m_Kind & 7) == 2)
			{
			CSketchView * pView = m_pDoc->GetDocView();
			pView->PauseIt(m_Frame);
			}
		if (m_pScan) m_pScan->GetChanges();
		return;
		}
	if (XlateRowCol(row,col))
		return;
	if (m_Kind)
		{
		}
	return;
DPF("row:%d,col:%d,frm:%d,lvl:%d",row,col,m_Frame, m_Level);
}
void CMainFrame::RClick(UINT row, UINT column)
{
	DPF("rclick,row:%d,col:%d",row,column);

	if ((GetKeyState(VK_SHIFT) < 0) || (GetKeyState(VK_CONTROL) < 0))
		{
		XlateRowCol(row,column);
		UINT z = StackFind(m_Frame, m_Level,1);
		CSketchView * pView = m_pDoc->GetDocView();
		if (z == 1) // current edit
			{
			pView->ShiftTrace(m_Frame, m_Level,1);
			}
		else if ((z > 1) && (z < 12))
			{
			if (GetKeyState(VK_CONTROL) < 0)
				z += 20;
			pView->ShiftTrace(m_Frame, m_Level,z);
			StackDraw();
			}
		return;
		}

	if ((column == 0) && row)
		{
		UINT StartFrame, EndFrame, StartLevel, EndLevel;
//		int cc = m_pSheet->m_Grid.GetColumnCount();
//		int cr = m_pSheet->m_Grid.GetRowCount();
		if (!GetSelection(StartFrame,StartLevel, EndFrame, EndLevel))
			return;
DPF("rframe:%d,%d,%d,%d",StartFrame, EndFrame, StartLevel,EndLevel);
		BOOL bChange = FALSE;
		row--;
		if ((StartLevel != 0) || ((EndLevel + 1) != m_Levels))
			bChange = TRUE;
		else if ((row < StartFrame) || (row > EndFrame))
			bChange = TRUE;
		if (bChange)
			{
			StartFrame = EndFrame = row;
			EndLevel = m_Levels - 1;
			StartLevel = 0;
			SetSelection(StartFrame, StartLevel,EndFrame, EndLevel);
			}
DPF("sframe:%d,%d,%d,%d",StartFrame, EndFrame, StartLevel,EndLevel);
		CFrameDlg dlg;
		dlg.m_frame = StartFrame + 1;
		dlg.m_count = EndFrame + 1 - StartFrame;
		dlg.m_max_del = m_Frames - StartFrame;
		dlg.m_max_add = 100;
		int res = dlg.DoModal();
DPF("res:%d",res);
		if (res == 100)
			{
			EndFrame = StartFrame + dlg.m_count - 1;
			if (ChangeFrames(StartFrame, StartFrame + dlg.m_count))
				return;
			}
		else if (res == 101)
			{
			EndFrame = StartFrame + dlg.m_count - 1;
			if (!DeleteSelection(StartFrame, StartLevel,EndFrame, EndLevel))
				return;
DPF("delete:%d,%d,%d,%d",StartFrame, EndFrame, StartLevel,EndLevel);
			ChangeFrames(EndFrame+1,StartFrame);
			}
		else if (res == 102)
			{
			StartFrame = m_Frames;
			EndFrame = StartFrame + dlg.m_count - 1;
			if (ChangeFrames(m_Frames + dlg.m_count, m_Frames + dlg.m_count))
				return;
			}
		else
			return;
		UINT Frame, Level;
		for (Level = StartLevel;Level <= EndLevel; Level++)
		for (Frame = StartFrame;Frame <= EndFrame; Frame++)
			{
			UpdateThumb(Frame,Level);
			}
		return;
		}
	if (row == 0 && column > 0)
		{
		UINT cc = m_pSheet->m_Grid.GetColumnCount();
		UINT Level = cc - column - 1;
		if (IsSound(column))
			SoundProp(column-1);
		else
			LevelProp(Level);
		}
	if (row > 0 && column > 0)
		{
		UINT cc = m_pSheet->m_Grid.GetColumnCount();
		UINT level = cc - column - 1;
		if (IsSound(column))
			{
DPF("playing");
			m_pSound->Play(row-1,0);
			return;
			}
//		UINT z = m_pegbar.Find(row-1, level);
//		if (GetKeyState(VK_SHIFT) < 0)
			{
			CellProp(row-1,cc-column-1);
			return;
			}
		/*
		if (z == 1 && m_pegbar.Count() > 1)
			{
			UINT f, l;
			m_pegbar.Contents(f,l,1);
			SelectCell(f, l, 1);
			SetSelection(f,l,f,l);
			}
		if (z > 1)
			m_pegbar.RemoveId(z-1);
			*/
		}
		
}

void CMainFrame::LevelProp(UINT Level)
{
	DPF("level prop:%d",Level);
	CString	oname;
	CString	omdlname;
	UINT	opeg;
	BOOL bCamera = ((CSketchApp*)AfxGetApp())->CanDoCamera();
	CLevelDlg dlg(bCamera ? IDD_LEVEL_PRO : IDD_LEVEL);
	dlg.m_level = Level;
	UINT old_index = m_pScene->PalIndex(Level);
	dlg.m_index = old_index;
	dlg.m_peg = opeg = bCamera ? m_pScene->Camera()->PegFindLevel(Level) : 0;
	DWORD oldflags = m_pScene->LevelFlags(Level);
	dlg.m_bUseLevel = oldflags & 1;
	dlg.m_pScene = m_pScene;
	char name[300];
	m_pScene->LevelName(name, Level);
	dlg.m_name = name;
	oname = name;
	name[0] = 0;
	m_pScene->LevelModelName(name, Level);
	dlg.m_mdlname = name;
	omdlname = name;
	if (dlg.DoModal() != IDOK)
		{
		return;
		}
	BOOL bChange = 0;
	BOOL bDoLevel = FALSE;
	UINT vFlag = 0;
	if (dlg.m_index != old_index)
		{
		m_pScene->PalIndex(Level,dlg.m_index);
		vFlag = 32;
		bChange = 1;
		}
	DWORD newflags = oldflags & 0xfffffffe;
	if (dlg.m_bUseLevel)
		newflags |= 1;
	if (newflags != oldflags)
		{
		m_bDirtyThumbs = TRUE;
		bDoLevel = 1;
		m_pScene->LevelFlags(Level,newflags);
		}
	if (dlg.m_name != oname)
		{
		strcpy(name,dlg.m_name);
		m_pScene->LevelName(name,Level,TRUE);
		}
//	int i;
	if (bChange)
		{
		bDoLevel = 1;
		DirtyThumb(m_pScene->PalIndex(Level));
		CSketchView * pView = (CSketchView *)GetActiveView();
		pView->Update(0,Level,4 + vFlag);
		m_pPalette->ChangePals();
		}
	if (dlg.m_mdlname != omdlname)
		{
		strcpy(name,dlg.m_mdlname);
		m_pScene->LevelModelName(name,Level,TRUE);
		}
	m_pPalette->Select(Level);
	UINT cc = m_pSheet->m_Grid.GetColumnCount();
	m_pSheet->m_Grid.RedrawCell(0,cc-Level-1);
	if (dlg.m_peg != opeg)
		{
		bDoLevel = 1;
		(m_pScene->Camera())->PegAttach(Level, dlg.m_peg);
		CSketchView * pView = (CSketchView *)GetActiveView();
		m_pCamera->SelectPeg(dlg.m_peg);
		pView->Update(0,Level,4);
		}
	if (bDoLevel)
		{
		m_pScene->UpdateCache(0, Level,9999);
		}
}

void CMainFrame::CellProp(UINT Frame, UINT Level)
{
	DPF("cell prop:%d,%d",Frame, Level);
	CCellDlg dlg;
	char name[30];
	m_pScene->CellName(name, Frame, Level);
	dlg.m_name = name;
	if (dlg.DoModal() == IDOK)
		{
DPF("nam:%s|",(LPCSTR)dlg.m_name);
		strcpy(name,dlg.m_name);
		m_pScene->CellName(name,Frame, Level,TRUE);
		RedrawGrid(Frame,Level);
		}
}

void CMainFrame::SlideSound(int updown, int dx)
{
DPF("slide sound:%d,%d",updown,dx);
	int which = updown / 8;
	if (which < 0) return;
	if (!m_pSound->IsValid(which)) return;
	updown &= 7;
	UINT vmark = m_pSound->VideoMark(which);
	double smark = m_pSound->AudioMark(which);
	ASSERT(smark >= 0.0);
	double maxmark = m_pSound->AudioMark(which,TRUE);
	double delta = 1.0 / (double)m_pScene->FrameRate();
	if (updown == 2)   // start slide
		{
		m_smarksave = smark;
		return;
		}
	else if (updown > 1)	// actual slide
		{
DPF("vmark:%d",vmark);
DPF("smark:%d",(int)(1000*smark));
DPF("ssave:%d",(int)(1000*m_smarksave));
DPF("delta:%d",(int)(1000*delta));
DPF("max :%d",(int)(1000*maxmark));
		double temp = delta;
		temp *= (DOUBLE)-dx;
		temp /= (DOUBLE)m_theight;
		temp += m_smarksave;
DPF("temp:%d",(int)(1000*temp));
		for (;temp < 0;)
			{
			vmark++;
			if (vmark > m_Frames) 
				return;
			m_smarksave += delta;
			temp += delta;
			}
		for (;temp >= maxmark;)
			{
			if (delta > temp)
				break;
			if (vmark > 1)
				vmark--;
			else
				return;
			m_smarksave -= delta;
			temp -= delta;
			}
		smark = temp;
DPF("smark:%d",(int)(1000*smark));
		}
	else if (updown)
		{
		if (vmark < m_Frames) 
			vmark++;
		else if (smark >= delta)
			smark -= delta;
		}
	else
		{
		if (vmark > 1)
			vmark--;
		else if ((smark + delta) < maxmark)
			smark += delta;
		}
	ASSERT(smark >= 0.0);
	m_pSound->SetAVMarks(which, smark, vmark);
	m_pScene->SceneOptionInt(SCOPT_VMARK0 + which,1,vmark-1);
	smark *= 1000;
	m_pScene->SceneOptionInt(SCOPT_SMARK0 + which,1,(UINT)smark);
	RedrawSound();
}

void CMainFrame::PlaySnd(UINT frame, UINT which)
{
DPF("play,%d,%d",frame,which);
	if (frame == 9999)
		{
		m_pSound->Play(9999,0);
//		m_pSound->Play(9999,1);
		}
	else if (m_pDoc->Option(SC_QUIET))
		m_pSound->Play(frame,which);
}


void CMainFrame::SetSound(BOOL bUse, int idd, double smark, UINT frame, 
					UINT snip, LPSTR Name)
{
	m_pSound->Allow(0);
	if (Name[0])
		{
		int res = m_pSound->LoadWaveFile(idd,(LPCSTR)Name);
		if (res)
			{
			MyError(IDS_NO_WAVE,(LPCSTR)Name);
			DPF("res:%d",res);
			return;
			}
		}
	UINT temp = (UINT)(smark * 1000);
//	m_pScene->SceneOptionInt(SCOPT_QUIET,1,bUse);
	if (bUse)
		m_pDoc->Option(SC_QUIET,1,1);
	m_pScene->SceneOptionInt(SCOPT_VMARK0+idd,1,frame-1);
	m_pScene->SceneOptionInt(SCOPT_SMARK0+idd,1,temp);
	if ((int)snip  > 0)
		m_pDoc->Option(SC_SNIP,1,snip);
	m_pScene->SceneOptionStr(SCOPT_WAVE0+idd,Name,1);
	m_pSound->Enabled(idd,bUse ? 1 : 0);
	m_pSound->SetAVMarks(idd, smark, frame);
	if (snip != NEGONE)
		m_pSound->Snip(snip);
	m_pSound->Allow(1);
	RedrawSound();
}

void CMainFrame::OnUpdateImpWave(CCmdUI* pCmdUI)
{
	BOOL bEnable = 0;
//	RECT rect;
//	m_pSheet->GetSelection(&rect);
//	if ((rect.top < rect.bottom) && IsSound(rect.left))
//	if (SoundCols())
		bEnable = 1;
	pCmdUI->Enable(bEnable);
}

void CMainFrame::SoundProp(int idd, LPCSTR Name /* = 0 */)
{
	DPF("sound prop,id:%d",idd);
	BOOL bSwitched;
	if (bSwitched = !m_bShowSound)
		SwitchSound();
	if (idd == 9)
		{
		if (m_nSoundCols > 1)
			{
			CSoundTrackDlg tdlg;
			tdlg.m_max = m_nSoundCols;
			tdlg.m_nTrack = 1;
			if (tdlg.DoModal() != IDOK)
				{
				if (bSwitched)
					SwitchSound();
				return;
				}
			idd = tdlg.m_nTrack-1;
			}
		else
			idd = 0;
		}
//m_pSound->Play(0,1);
	CSoundDlg dlg;
//	dlg.m_bInternal = m_bInternalSound;
//	BOOL bUse = dlg.m_bUseSound = m_pDoc->Option(SC_QUIET);
	BOOL bUse = dlg.m_bUseSound = m_pSound->Enabled(idd);
	if (Name)
		dlg.m_bUseSound = TRUE;
	dlg.m_bChanged = Name ? 1 : 0;
	dlg.m_frames = m_Frames;	// for min max against max frames
	dlg.m_vmark = m_pSound->VideoMark(idd);
	dlg.m_smark = m_pSound->AudioMark(idd);
DPF("vmark:%d,smark:%d",dlg.m_vmark,dlg.m_smark);
	dlg.m_snip = m_pSound->Snip();
	int volume = dlg.m_nVolume = m_pSound->Volume(idd);
	dlg.m_pSound = m_pSound;
	dlg.m_id = idd;
	char name[300];
	if (Name)
		strcpy(name, Name);
	else
		m_pScene->SceneOptionStr(idd ? SCOPT_WAVE1 + idd - 1 :SCOPT_WAVE0,name);
	dlg.m_name = name;
//	if ((dlg.DoModal() == IDOK) && (dlg.m_name.GetLength()))
	if (dlg.DoModal() == IDOK)
		{
//		if (!m_bShowSound)
//			SwitchSound();
//		if (dlg.m_bInternal)
//			dlg.m_name = "";
//		else
			if (!dlg.m_name.GetLength())
			dlg.m_bUseSound = 0;
		
		if ((dlg.m_vmark != m_pSound->VideoMark(idd)) ||
			(dlg.m_smark != m_pSound->AudioMark(idd)) ||
			(dlg.m_bUseSound != bUse) ||
			strcmp(name, (LPCSTR)dlg.m_name) ||
			(dlg.m_snip != m_pSound->Snip()))
				dlg.m_bChanged = TRUE;
		if (strcmp(name, (LPCSTR)dlg.m_name))
			dlg.m_bUseSound = TRUE;
		if (dlg.m_bChanged)// && dlg.m_name.GetLength())
			{
			SetSound(dlg.m_bUseSound, idd, dlg.m_smark, dlg.m_vmark, dlg.m_snip,
						(LPSTR)(LPCSTR)dlg.m_name);
			}
		if (volume != dlg.m_nVolume)
			{
			m_pScene->SceneOptionInt(SCOPT_VOL0+idd,1,dlg.m_nVolume);
			}
//		m_bInternalSound = 0;//dlg.m_bUseSound ? dlg.m_bInternal : 0;
		}
	else
		{
		if (bSwitched)
			SwitchSound();
		if (volume != dlg.m_nVolume)
			m_pSound->Volume(idd,volume); // restore it
		}
}


void CMainFrame::AdjustDisPalWindows(BOOL bInit)
{
#ifndef _DISNEY
	if (SwitchSubPalette(0) != m_bDisPal)
		SwitchSubPalette(1);
	return;
#endif
#ifndef OLDDISPAL
m_bDisPal = 1;
#else
	if (bInit)
		{
		m_bDisPal = 0;
		if (m_pScene->ColorMode())
			{
			int i;
		BYTE * pPals = &m_pDoc->GetDocView()->Layers()->LevelTable()->pals[0];
			for (i = 1; i < 18; i++)
				if (!pPals[4*i+3])
					break;
			if (i < 18)
				m_bDisPal = 1;
			}
		}
    CMenu * pMenu = GetMenu();
	if (!pMenu) return;
    CMenu * pMenu1 = pMenu->GetSubMenu(4); // Tool menu
	if (!pMenu1) return;
	int count = pMenu1->GetMenuItemCount();
	for (int i = 0; i < count; i++)
		{
		CString str;
		if (pMenu1->GetMenuString(i, str, MF_BYPOSITION) &&
       	 	(strcmp(str, "Wand\tW") == 0))
				break;
		}
	if (m_bDisPal && (i >= count))
		{
		pMenu1->AppendMenu(MF_BYPOSITION, ID_TOOL_WAND, "Wand\tW");
		}
	if (!m_bDisPal && (i < count))
		{
		pMenu1->RemoveMenu(i,MF_BYPOSITION);
		}
#endif		
	PostMessage(WM_COMMAND,m_bDisPal ? ID_TOOL_WAND : ID_TOOL_PENCIL);
	if (SwitchSubPalette(0) != m_bDisPal)
		SwitchSubPalette(1);
	if (SwitchPalette(0) == m_bDisPal)
		SwitchPalette(1);
//	if (SwitchTools(0) == m_bDisPal)
//		SwitchTools(1);
//	if (SwitchCamera(0) == m_bDisPal)
//		SwitchCamera(1);
	if (SwitchSheet(0) == m_bDisPal)
		SwitchSheet(1);
	SetFocus();
}

void CMainFrame::CopySubPal()
{
	return;
	BYTE * pPals = 0;//&m_pDoc->GetDocView()->Layers()->LevelTable()->pals[0];
	int i;
	for (i = 0; i < 18; i++)
		m_pSubPal->SetColor(i,pPals[4*i+0],pPals[4*i+1],pPals[4*i+2],pPals[4*i+3]);
}
/*
void CMainFrame::OnUpdateMagic11(CCmdUI* pCmdUI)
{
	BOOL bColor = 0;
	if (m_pScene && m_pScene->ColorMode())
		bColor = TRUE;
	pCmdUI->Enable(bColor && (m_nDisPal != 0));
}

void CMainFrame::OnMagic11()
{
	UINT f,l;
	GetSelection(f,l);
	BYTE * pPals = &m_pDoc->GetDocView()->Layers()->LevelTable()->pals[0];
	if (m_bDisPal)
		{
		int i;
		for (i = 1; i < 18; i++)
			{
			COLORREF z = m_pSubPal->GetColor(i);
			pPals[4*i+3] = (BYTE)(z >> 24);
			}
		m_bDisPal = 0;
		}
	else
		{
		int i;
		for (i = 0; i < 18; i++)
			m_pSubPal->SetColor(i,pPals[4*i+0],
								pPals[4*i+1],pPals[4*i+2],pPals[4*i+3]);
//		for (i = 1; i < m_nDisPal; i++)
//			pPals[4*i+3] = 0;
		m_bDisPal = 1;
		m_bDisPal2 = 0; // reset auto play flag
		}
	m_pDoc->GetDocView()->Layers()->LevelTable(TRUE);
	m_pDoc->GetDocView()->Update(0,m_Level,4);
	m_pDoc->GetDocView()->CheckUpdate();
	DirtyThumb(m_Level);
	m_pPalette->ChangePals();
	m_pSubPal->Invalidate(0);
	AdjustDisPalWindows(0);
	m_pTools->Setup(this,m_tbsize,m_tbsize,m_pDoc,
					m_pDoc->GetDocView()->Layer() ? 1 : 0,1);
	m_pCamTools->Setup(this,m_tbsize,m_tbsize,1);
}

void CMainFrame::OnMagic12()
{
	UINT f,l;
	GetSelection(f,l);
	BYTE * pPals = &m_pDoc->GetDocView()->Layers()->LevelTable()->pals[0];
	int i;
	for (i = 1; i < m_nDisPal; i++)
		pPals[4*i+3] = 0;
	m_pDoc->GetDocView()->Layers()->LevelTable(TRUE);
	m_pDoc->GetDocView()->Update(0,m_Level,4);
	m_pDoc->GetDocView()->CheckUpdate();
	DirtyThumb(-1, m_Level);
	m_pPalette->ChangePals();
	m_pSubPal->Invalidate(0);
}



void CMainFrame::SetDisPaint(UINT index)
{
	COLORREF crSubPal = m_pSubPal->GetColor();
	SetColor(index, GetRValue(crSubPal), GetGValue(crSubPal),
			GetBValue(crSubPal), 255,0);//(BYTE)(crSubPal >> 24), 0);
	if (!m_bDisPal2)
		{
		BYTE * pPals = &m_pDoc->GetDocView()->Layers()->LevelTable()->pals[0];
		int i;
		for (i = 0; (i < 18) && pPals[4*i+3];i++);
		if (i >= 18)
			{
//			if (!m_bShowSlider)
//				SwitchSlider(1);
			m_bDisPal2 = 1;
			PostMessage(WM_COMMAND,ID_VCR_PLAY);
			}
		}
	
}
*/
void CMainFrame::SetColor(UINT index, BYTE r, BYTE g, BYTE b, BYTE alpha)
{
	m_pPalette->SetColor(index, r, g, b, alpha);
}

void CMainFrame::ColorTrap(int x, int y, int code)
{
	m_pPalette->ColorTrap(x,y,code);
}


BOOL CMainFrame::HasModel(UINT v /* = -1 */ ,UINT Level /* = - 1 */)
{
	BOOL bResult = FALSE;
	if (Level == -1)
		bResult = m_pPalette->HasModel(v);
	return bResult;
}

void CMainFrame::ChangeTool(BOOL bPaintOnly /*= 0*/)
{
	Status();
	if (!bPaintOnly)
		m_pTools->ChangeTool();
	m_pPalette->Invalidate();
}
/*
void CMainFrame::ChangeCamTool()
{
	Status();
	if (!bPaintOnly)
		m_pTools->ChangeTool();
	m_pPalette->Invalidate();
}
*/
// 1 for sound, 2 for columns
void CMainFrame::RedrawSheet(UINT flag /* = 0 */)
{
	BOOL bThumb;
	if (m_pDoc)
		m_bXSheetHighlight = m_pDoc->Option(XSHEET_HIGHLIGHT);
	if (!m_pDoc || !m_pDoc->Option(SHOWTHUMBS))
		bThumb = FALSE;
	else
		bThumb = TRUE;
	if ((flag & 3) || (bThumb != m_bHaveThumbs))
		{
		UINT h;
		if (bThumb)
			h = m_theight;
		else
			h = 25;
DPF("setting h:%d",h);
		m_pSheet->m_Grid.SetDefCellSize(m_twidth,h);
		m_bHaveThumbs = bThumb;
		m_pSheet->m_Grid.SetRowHeight(0,40);
		m_pSheet->m_Grid.SetColumnWidth(0,25);
		
		UINT i,w;
		if (flag & 2)
			{
			for (i = 0; i < m_Levels; i++)
				{
				w = m_pScene->GetLevelWidth(i,m_twidth);
				if (w > 511)
					w = MIN_WIDTH;
DPF("width,%d,%d",i,w);
				m_pSheet->m_Grid.SetColumnWidth(
					m_pSheet->m_Grid.GetColumnCount() - 1 - i,w);
				}
			}
		for (i = 0; i < SoundCols();i++)
			m_pSheet->m_Grid.SetColumnWidth(i+1,SOUND_WIDTH);//m_twidth/2);
		}
	m_pSheet->Invalidate();
}

void CMainFrame::SaveSettings(BOOL bClip)
{
	UINT i;
DPF("save settings");
	for (i = 0; i < m_Levels; i++)
		{
		UINT w = m_pSheet->m_Grid.GetColumnWidth(
					m_pSheet->m_Grid.GetColumnCount() - 1 - i);
		if (w <= MIN_WIDTH)  // if collapsed, just set a flag
			w = 512 | m_pScene->GetLevelWidth(i,m_twidth);
		m_pScene->PutLevelWidth(i,w);
		}
	m_pScene->PutLevelWidth(9999,0); // force write

	if (bClip && m_pScene->ClipEmpty() && m_bNeedExtClip)
		{
		int answer;
		answer = AfxMessageBox(IDS_PROMPT_KEEP_CLIP, MB_YESNO);
		if(OpenClipboard())
			{
			if (answer == IDYES)
				{
				EmptyClipboard();
				OnRenderFormat(m_MyClipCell);
				}
			CloseClipboard();
			}
		}
	if (!bClip)
		{
		if(OpenClipboard())
			{
			EmptyClipboard();
			CloseClipboard();
			}
		}
}
/*
UINT CMainFrame::CameraAttr(int which, int frame, int peg,double &v, UINT & kind)
{
	if (!m_pCamera)
		return 0;
	return m_pCamera->CameraAttr(which,frame,peg,v,kind);
}
*/


UINT CMainFrame::CameraPegXY(int & pegx,int & pegy,int & kind, int frame, int w, int h)
{
	if (!m_pCamera)
		return 0;
#ifdef FLIPBOOK_MAC
	return 0;
#else
	return m_pCamera->CameraPegXY(pegx, pegy, kind,frame,w,h);
#endif
}

UINT CMainFrame::CameraPacket(int kind, int v1, int v2)
{
	if (!m_pCamera) return 0;
	if (kind == 9)
		{
//		m_pSlider->m_slider.EnableWindow(FALSE);
		m_bShowCamera = 0;
		SetCamera();
		return 0;
		}
	else
		return m_pCamera->Packet(kind,v1,v2);
}

typedef struct {
	DWORD   dwId;
	DWORD   dwVersion;
	UINT	Width;
	UINT	Height;
	UINT	OrigFrame;
	UINT	OrigLevel;
	UINT	Frames;
	UINT	Levels;
	UINT	Cells;
} CLIPHEADER;


UINT CMainFrame::NewPaste(UINT Frame, UINT Level)
{
	DPF("new paste,f:%d,l:%d",Frame,Level);
	if ( !OpenClipboard() ) 
		return 1;
	HANDLE hmem = GetClipboardData(m_MyClipCell);
	if (!hmem)
		{
		CloseClipboard();
		return 1;
		}
	CMemFile sf((BYTE*) ::GlobalLock(hmem), ::GlobalSize(hmem));
	CLIPHEADER header;
	sf.Read(&header, sizeof(header));
	UINT w = header.Levels;
	UINT h = header.Frames;
	UINT * keys = 0;
	UINT err = 0;
	if ((header.dwId != 12345678) || !header.Cells)
		err = IDS_NO_PASTE;
	else if (header.Width != m_pScene->Width() ||
			header.Height != m_pScene->Height())
		err = IDS_NO_PASTE_RES;
	else
		{
		DPZ("id:%d,f:%d,l:%d",header.dwId,header.OrigFrame,header.OrigLevel);
		DPZ("h:%d,w:%d,c:%d",header.Frames,header.Levels,header.Cells);
		keys = new UINT[w * h];
		sf.Read(keys, 4 * w * h);
		UINT * cells = new UINT [header.Cells];
		UINT i;
		for (i = 0; i < header.Cells; i++)
			cells[i] = m_pScene->UnWrapCell(sf);
		for (i = 0; i < w*h;i++)
			if (keys[i])
				keys[i] = cells[keys[i]-1];
		delete [] cells;
		}
	GlobalUnlock( hmem);
	CloseClipboard();
	if (err)
		{
		AfxMessageBox(err, MB_OK);
		return err;
		}
	m_pScene->ClipStart(header.Levels,header.Frames,Level);
	DWORD x,y;
	for (x = 0; x < w; x++)
	for (y = 0; y < h; y++)
		m_pScene->ClipSet(x,y, keys[x*h+y]);
	m_pScene->ClipFinish();
	delete [] keys;
	m_bNeedExtClip = FALSE;
	return 0;
}


void CMainFrame::OnRenderFormat(UINT format)
{
	DPF("render format:%d",format);
	if (format != m_MyClipCell)
		return;
	if (!m_pScene->ClipEmpty())
		return;
	CLIPHEADER header;
	UINT x,y,w,h,c,i;
	m_pScene->ClipInfo(w,h);
	UINT * keys = new UINT [w * h];
	UINT * ukeys = new UINT [w * h+1];
	c = 0;
	for (x = 0; x < w; x++)
	for (y = 0; y < h; y++)
		{
		UINT key = m_pScene->ClipGet(x,y);
		if (key)
			{
			ukeys[c] = key;
			for (i = 0; ukeys[i] != key;i++);
			if (i >= c)
				c++;
			keys[x*h+y] = i + 1; // plus 1 for not empty
			}
		else
			keys[x*h+y] = 0; // for empty
		}
DPF("unique keys:%d",c);
	header.Cells = c;
	CSharedFile	sf;//(GMEM_MOVEABLE|GMEM_DDESHARE|GMEM_ZEROINIT);
	header.dwId = 12345678;
	header.dwVersion = 1;
	header.Width = m_pScene->Width();
	header.Height = m_pScene->Height();
	header.OrigFrame = m_pScene->ClipFrame();
	header.OrigLevel = m_pScene->ClipLevel();
	header.Frames = h;
	header.Levels = w;
	sf.Write(&header, sizeof(header));
	sf.Write(keys, 4 * w * h);		// actually indices
	delete [] keys;
	for (i = 0; i < c; i++)
		m_pScene->WrapCell(sf, ukeys[i]);
	delete [] ukeys;
	HGLOBAL hMem = sf.Detach();
	#if _MFC_VER <= 0x0600
		::GlobalUnlock(hMem);		// documented bug from knowledge base
	#endif 
	if (hMem)
		hMem = SetClipboardData(m_MyClipCell,hMem);
}

void CMainFrame::OnRenderAllFormats()
{
	DPF("render all formats");
}

CThumbs::CThumbs(UINT frames, UINT levels, UINT size)
{
	m_grain = 100;
	m_frames = frames;
	m_levels = levels;
	m_size = size;
	UINT i = m_grain * ((m_frames + m_grain - 1) / m_grain);
	m_pFrames = new LPBYTE[i];
	for (i = 0; i < m_frames; i++)
		m_pFrames[i] = new BYTE[levels * size];
}

CThumbs::~CThumbs()
{
	UINT i;
	for (i = 0; i < m_frames; i++)
		delete [] m_pFrames[i];
	delete [] m_pFrames;
}

BYTE * CThumbs::Buf(UINT Frame, UINT Level)
{
	return m_pFrames[Frame] + m_size * Level; 
}

UINT CThumbs::Count()
{
	return m_frames;
}

int	CThumbs::Insert(UINT Start, UINT Count)
{
	DPF("Insert,Start:%d,Cnt:%d",Start,Count);
	int z = Append(Count);
	if (z) return z;
	UINT i;
	UINT size = m_levels * m_size;
	UINT stop = Start + Count;
	for (i = m_frames - 1; i >= stop; i--) 
		{
		BYTE * dst = m_pFrames[i];
		BYTE * src = m_pFrames[i - Count];
		memcpy(dst, src, size);
		}
//	for (; i >= Start; i--) 
	for (;Count--;i--)
		{
		BYTE * dst = m_pFrames[i];
		memset(dst, 255, size);
		}
	return 0;
}

int	CThumbs::Delete(UINT Start, UINT Count)
{
	DPF("Delete,start:%d,Cnt:%d",Start,Count);
	UINT i;
	UINT size = m_levels * m_size;
	UINT stop = m_frames - Count;
	for (i = Start; i < stop; i++) 
		{
		BYTE * dst = m_pFrames[i];
		BYTE * src = m_pFrames[i + Count];
		memcpy(dst, src, size);
		}
	for (;i < m_frames; i++)
		delete [] m_pFrames[i];
	m_frames = stop;	
	return 0;
}

int	CThumbs::DeleteLevels(UINT Start, UINT Count)
{
	DPF("Delete,levesl start:%d,Cnt:%d",Start,Count);
	ASSERT(m_levels > Count);
	if (Count >= m_levels) return 0;
	UINT i;
	m_levels -= Count;
	for (i = 0; i < m_frames; i++)
		{
		LPBYTE pp = m_pFrames[i];
		m_pFrames[i]  = new BYTE[m_levels * m_size];
		memcpy(m_pFrames[i], pp, Start * m_size);
		memcpy(m_pFrames[i] + Start*m_size,pp+(Start + Count) * m_size,
						m_size * (m_levels - Count - Start));
		delete [] pp;
		}
	return 0;
}

int	CThumbs::Append(UINT Count)
{
	UINT i = (m_frames + m_grain - 1) / m_grain;
	if (((m_frames + Count + m_grain - 1) / m_grain) != i)
		{
		UINT i = m_grain * ((m_frames + Count + m_grain - 1) / m_grain);
		LPBYTE * pp = new LPBYTE[i];
		if (pp == NULL)
			return 1;
		for (i = 0; i < m_frames; i ++)
			pp[i] = m_pFrames[i];
		delete [] m_pFrames;
		m_pFrames = pp;
		}
	i = m_frames;
	m_frames += Count;
	for (; i < m_frames; i++)
		{
		m_pFrames[i]  = new BYTE[m_levels * m_size];
		if (m_pFrames[i] == NULL)
			return 2;
		}
	return 0;
}

int	CThumbs::AppendLevels(UINT Count)
{
	UINT i;
	UINT l = m_levels;
	m_levels += Count;
	for (i = 0; i < m_frames; i++)
		{
		LPBYTE pp = m_pFrames[i];
		m_pFrames[i]  = new BYTE[m_levels * m_size];
		memcpy(m_pFrames[i], pp, l * m_size);
		memset(m_pFrames[i]+l*m_size,0, Count * m_size);
		delete [] pp;
		}
	return 0;
}

int	CThumbs::InsertLevels(UINT start, UINT Count)
{
	UINT i;
	m_levels += Count;
	for (i = 0; i < m_frames; i++)
		{
		LPBYTE pp = m_pFrames[i];
		m_pFrames[i]  = new BYTE[m_levels * m_size];
		memcpy(m_pFrames[i], pp, start * m_size);
//		memset(m_pFrames[i]+start*m_size,255, Count * m_size);
		memset(m_pFrames[i]+start*m_size,0, Count * m_size);
		memcpy(m_pFrames[i]+(start+Count)*m_size,pp+start * m_size,
						m_size * (m_levels - Count - start));
		delete [] pp;
		}
	return 0;
}

void CMainFrame::OnUpdateLayer(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_pScene && m_pScene->ColorMode());
}

void CMainFrame::UpdateTools(BOOL bForce)
{
	CSketchView * pView = m_pDoc->GetDocView();
	m_pTools->Setup(this,m_tbsize,m_tbsize,m_pDoc, pView->Layer() ? 1 : 0,bForce);
}

void CMainFrame::OnLayer()
{
	CLayerDlg dlg;
	dlg.m_pView = m_pDoc->GetDocView();
	int Level = dlg.m_pView->CurrentLevel();
	if (!Level) return;
	CLevelTable * pTable = new CLevelTable;
	memmove(pTable, dlg.m_pView->Layers()->LevelTable(), sizeof(CLevelTable));
//	dlg.m_pView->Layers()->LevelTable()->layer =
//				m_pToolBar->m_comboBox.GetCurSel();
	if (dlg.DoModal() != IDOK)
		{
		memmove( dlg.m_pView->Layers()->LevelTable(), pTable, sizeof(CLevelTable));
		dlg.m_pView->Layers()->LevelTable(1);
		dlg.m_pView->Update(0,Level,4+16);
		}
//	dlg.m_pView->Update(0,Level,4+16);
	delete pTable;
}

void CMainFrame::OnCustomizeToolBar()
{
//	m_pToolBar->DoDialog();
	CToolBarDlg dlg;
	dlg.m_tb = m_pToolBar;
	dlg.DoModal();
}

void CMainFrame::FixOEMMenu()
{
#ifdef DO_OEM
	char buf[100];
	CMenu * pMenu = GetMenu();
	if (!pMenu)
		return;
    pMenu->GetMenuString(ID_APP_ABOUT, buf,99,MF_BYCOMMAND);
	OEM_Buf(buf);
    pMenu->ModifyMenu(ID_APP_ABOUT,MF_BYCOMMAND|MF_STRING,
					ID_APP_ABOUT, buf);
    pMenu->GetMenuString(ID_APP_REGISTER, buf,99,MF_BYCOMMAND);
	OEM_Buf(buf);
    pMenu->ModifyMenu(ID_APP_REGISTER,MF_BYCOMMAND|MF_STRING,
					ID_APP_REGISTER, buf);
#endif
}

void CMainFrame::GridMsg(int msg ,int row, int col)
{
	DPF("msg:%d,row:%d,col:%d",msg,row,col);
	if ((msg == 136) && !row && col)
		{
		int ww = m_pSheet->m_Grid.GetColumnWidth(col);
		if (ww <= MIN_WIDTH)
			{
			if (IsSound(col))
				ww = SOUND_WIDTH;//2*m_twidth / 3;
			else
				{
				int l = m_pSheet->m_Grid.GetColumnCount() - 1 - col;
				ww = m_pScene->GetLevelWidth(l,m_twidth) & 511;
				if (ww <= MIN_WIDTH)
					ww = m_twidth;
				}
			}
		else
			{
			if (!IsSound(col))
				{
				int l = m_pSheet->m_Grid.GetColumnCount() - 1 - col;
				m_pScene->PutLevelWidth(l,ww);
				}
			ww = MIN_WIDTH;
			}
		m_pSheet->m_Grid.SetColumnWidth(col,ww);
		m_pSheet->Invalidate();
		}
	
}


void CMainFrame::OnUpdateIconSize(CCmdUI* pCmdUI)
{
	int iid = pCmdUI->m_nID;
	iid -= ID_OPT_ICON_LARGE;
	pCmdUI->SetCheck(iid == m_nIconSize ? 1 : 0);
}

void CMainFrame::OnIconSize(UINT iid)
{
#ifndef FLIPBOOK_MAC
	iid -= ID_OPT_ICON_LARGE;
	m_nIconSize = iid;
	HWND hwnd = ::GetDesktopWindow();
	HDC hdc = ::GetDC(hwnd);
	UINT winw = ::GetDeviceCaps(hdc, HORZRES);
	m_tbsize = winw / (30 + 10 * (m_nIconSize & 3));
						// toolbar icons shall be 1 / 40 of screen width
	::ReleaseDC(hwnd,hdc);
#endif
	CRect zrect;
	GetWindowRect(&zrect);
	UINT editw = zrect.right - zrect.left;
	UINT flags = 0;
	if (((CSketchApp*)AfxGetApp())->CanDoLayers())
		flags |= 1;
	if (((CSketchApp*)AfxGetApp())->IsMaya())
		flags |= 2;
	m_pToolBar->Setup(m_tbsize, m_tbsize,editw,flags);
//	RecalcLayout();
//	MakeToolBar();
	CSketchView * pView = (CSketchView *)GetActiveView();
	m_pTools->Setup(this,m_tbsize,m_tbsize,
							m_pDoc, pView->Layer() ? 1 : 0);
	m_pCamTools->Setup(this,m_tbsize,m_tbsize);
	RecalcLayout();
	
	return;
}


void CMainFrame::OnSysCommand(UINT nID, LPARAM lParam)
{
#ifndef FLIPBOOK_MAC // FIXME 6
	CFrameWnd* pFrameWnd = GetTopLevelFrame();
	ENSURE_VALID(pFrameWnd);
	UINT nItemID = (nID & 0xFFF0);
	if (nItemID == SC_MINIMIZE)
		m_pSheet->ShowWindow(SW_MINIMIZE);
	else if (nItemID == SC_RESTORE)
		m_pSheet->ShowWindow(SW_RESTORE);
	CWnd::OnSysCommand(nID, lParam);
#endif
}

