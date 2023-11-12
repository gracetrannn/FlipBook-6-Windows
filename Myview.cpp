//
// sketcvw.cpp : implementation of the CSketchView class
//

#include "stdafx.h"
#include "sketch.h"
#include "multimon.h"
#include "cfloat.h"
#include "mydoc.h"
#include "myview.h"
#include "clayers.h"
//#include "dibapi.h"
#include "mainfrm.h"
#include "mymmtim.h"
#include "action.h"
#include "dialogs.h"
#include "dib.h"
#include "clevtbl.h"
//#include "sceneopt.h"
#include "ctablet.h"
#include "math.h"
#include "cbrush.h"
#include "cnewpals.h"
#if !MAC
#include "tpcshrd.h"
#endif
#include "shiftrc.h"
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define CENTERED

/////////////////////////////////////////////////////////////////////////////
// CSketchView

class CVWnd : public CWnd
{
public:
	BOOL MyCreate(CSketchView * pParent,int x, int y, int w, int h);
protected:
	//{{AFX_MSG( CMainWindow )
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	CSketchView * m_pView;
	int m_initx;
	int m_inity;
};



IMPLEMENT_DYNCREATE(CSketchView, CScrollView)

BEGIN_MESSAGE_MAP(CSketchView, CScrollView)
	//{{AFX_MSG_MAP(CSketchView)
	//}}AFX_MSG_MAP
	// Standard printing commands
//	ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT, MyPrint)
//	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
//	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
//	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
//	ON_COMMAND(ID_FILE_MYPRINT, MyPrint)
	ON_MESSAGE(WM_DOREALIZE, OnDoRealize)
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
    ON_WM_SYSKEYDOWN()
    ON_WM_SYSKEYUP()
    ON_WM_KEYDOWN()
    ON_WM_KEYUP()
    ON_WM_ERASEBKGND()
	ON_COMMAND_RANGE(ID_KEY_PGUP,ID_KEY_END,OnFunctionKey)
	ON_COMMAND_RANGE(ID_KEY_F1, ID_KEY_F12, OnFunctionKey)
	ON_COMMAND_RANGE(ID_KEY_SF9, ID_KEY_SF12, OnFunctionKey)
	ON_COMMAND_RANGE(ID_KEY_CF9, ID_KEY_CF12, OnFunctionKey)
	ON_COMMAND_RANGE(ID_KEY_LEFT, ID_KEY_RIGHT, OnPositionKey)
	ON_COMMAND_RANGE(ID_KEY_UP, ID_KEY_DOWN, OnPositionKey)
	ON_COMMAND_RANGE(ID_KEY_CLEFT, ID_KEY_CDOWN, OnCameraKey)
	ON_WM_SIZE()
//	ON_WM_MOVE()
	ON_COMMAND_RANGE(ID_TOOL_PENCIL, ID_TOOL_SELECT, OnTool)
	ON_COMMAND_RANGE(ID_TOOL_CURVE, ID_TOOL_FREE, OnTool)
//	ON_UPDATE_COMMAND_UI_RANGE(ID_TOOL_PEN, ID_TOOL_ERASER, OnUpdateTool)
	ON_UPDATE_COMMAND_UI(ID_ACTION_FILL, OnUpdateActionFill)
	ON_COMMAND(ID_ACTION_FILL, OnActionFill)
	ON_UPDATE_COMMAND_UI(ID_ACTION_FLIP, OnUpdateActionFlip)
	ON_COMMAND(ID_ACTION_FLIP, OnActionFlip)
	ON_UPDATE_COMMAND_UI(ID_ACTION_MATTE, OnUpdateActionMatte)
	ON_COMMAND(ID_ACTION_MATTE, OnActionMatte)
	ON_UPDATE_COMMAND_UI(ID_ACTION_SPECK, OnUpdateActionSpeckle)
	ON_COMMAND(ID_ACTION_SPECK, OnActionSpeckle)
	ON_UPDATE_COMMAND_UI(ID_TOOL_PENCIL, OnUpdatePencil)
	ON_UPDATE_COMMAND_UI(ID_TOOL_TRACE, OnUpdatePen)
	ON_UPDATE_COMMAND_UI(ID_TOOL_BRUSH, OnUpdateBrush)
	ON_UPDATE_COMMAND_UI(ID_TOOL_FILL, OnUpdateBucket)
	ON_UPDATE_COMMAND_UI(ID_TOOL_ZOOM, OnUpdateZoom)
	ON_UPDATE_COMMAND_UI(ID_TOOL_HAND, OnUpdateHand)
	ON_UPDATE_COMMAND_UI(ID_TOOL_EYEDROP, OnUpdateEyeDrop)
	ON_UPDATE_COMMAND_UI(ID_TOOL_ERASER, OnUpdateEraser)
	ON_UPDATE_COMMAND_UI(ID_TOOL_ERASER_BOX, OnUpdateEraser)
	ON_COMMAND(ID_TOOL_ERASER_BOX, OnEraserBox)
	ON_UPDATE_COMMAND_UI(ID_TOOL_WAND, OnUpdateWand)
	ON_UPDATE_COMMAND_UI(ID_TOOL_SELECT, OnUpdateSelect)
	ON_UPDATE_COMMAND_UI(ID_TOOL_SCAN, OnUpdateScan)
	ON_UPDATE_COMMAND_UI(ID_TOOL_CAPTURE, OnUpdateCapture)
	ON_UPDATE_COMMAND_UI(ID_APP_PROPERTIES, OnUpdateProperties)
	ON_COMMAND(ID_APP_PROPERTIES, OnProperties)
	ON_COMMAND(ID_EDIT_OUTSIDE, OnEditOutside)
	ON_UPDATE_COMMAND_UI(ID_EDIT_OUTSIDE, OnUpdateOutside)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateUndo)
	ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REDO, OnUpdateRedo)
	ON_COMMAND(ID_EDIT_CLEARC, OnEditClear)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CLEARC, OnUpdateClear)
	ON_COMMAND(ID_EDIT_REVERT, OnEditRevert)
	ON_UPDATE_COMMAND_UI(ID_EDIT_REVERT, OnUpdateRevert)
	ON_COMMAND(ID_OPT_SETTING, OnOptions)
	ON_COMMAND(ID_OPT_LIGHTBOX, OnLightBox)
	ON_COMMAND(ID_OPT_FG, OnOptFG)
	ON_UPDATE_COMMAND_UI(ID_OPT_FG, OnUpdateFG)
	ON_COMMAND(ID_OPT_BG, OnOptBG)
	ON_UPDATE_COMMAND_UI(ID_OPT_BG, OnUpdateBG)
	ON_COMMAND(ID_OPT_KEEP, OnOptKeep)
	ON_UPDATE_COMMAND_UI(ID_OPT_KEEP, OnUpdateKeep)
	ON_COMMAND(ID_OPT_SOUND, OnOptSound)
	ON_UPDATE_COMMAND_UI(ID_OPT_SOUND, OnUpdateOptSound)
//	ON_COMMAND(ID_OPT_STACK, OnOptStack)
//	ON_UPDATE_COMMAND_UI(ID_OPT_STACK, OnUpdateStack)
	ON_COMMAND(ID_OPT_SHOWTHUMBS, OnOptThumbs)
	ON_UPDATE_COMMAND_UI(ID_OPT_SHOWTHUMBS, OnUpdateThumbs)
//	ON_COMMAND(ID_OPT_SHOWTHUMBS, OnViewTiming)
//	ON_UPDATE_COMMAND_UI(ID_OPT_SHOWTHUMBS, OnUpdateTiming)
	ON_COMMAND(ID_OPT_AUTOCOMP, OnOptComposite)
	ON_UPDATE_COMMAND_UI(ID_OPT_AUTOCOMP, OnUpdateComposite)
	ON_COMMAND(ID_VIEW_SHEET, OnViewSheet)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHEET, OnUpdateSheet)
	ON_COMMAND(ID_VIEW_TIMING, OnViewTiming)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TIMING, OnUpdateTiming)
	ON_COMMAND(ID_VIEW_MODE, OnViewMode)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MODE, OnUpdateMode)
	ON_COMMAND(ID_VIEW_PALETTE, OnViewPalette)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PALETTE, OnUpdatePalette)
	ON_COMMAND(ID_VIEW_LIPSYNC, OnViewLipSync)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LIPSYNC, OnUpdateLipSync)
	ON_COMMAND(ID_VIEW_PUCK, OnViewPuck)
	ON_UPDATE_COMMAND_UI(ID_VIEW_PUCK, OnUpdatePuck)
	ON_COMMAND(ID_VIEW_ZOOMER, OnViewZoomer)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ZOOMER, OnUpdateZoomer)
	ON_COMMAND(ID_VIEW_SUBPAL, OnViewSubPalette)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SUBPAL, OnUpdateSubPalette)
	ON_COMMAND(ID_VIEW_CAMERA, OnViewCamera)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CAMERA, OnUpdateCamera)
	ON_COMMAND(ID_VIEW_CAMERA_100, OnViewCamera100)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CAMERA_100, OnUpdateCamera100)
	ON_COMMAND(ID_VIEW_CAMERA_50, OnViewCamera50)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CAMERA_50, OnUpdateCamera50)
	ON_COMMAND(ID_VIEW_CAMERA_25, OnViewCamera25)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CAMERA_25, OnUpdateCamera25)
	ON_COMMAND(ID_VIEW_CAMERA_ENLARGE, OnViewCameraEnlarge)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CAMERA_ENLARGE, OnUpdateCameraEnlarge)
	ON_COMMAND(ID_VIEW_CAMERA_GRID, OnViewCameraGrid)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CAMERA_GRID, OnUpdateCameraGrid)
	ON_COMMAND(ID_VIEW_CAMERA_CENTER, OnViewCameraGridCenter)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CAMERA_CENTER, OnUpdateCameraGridCenter)
	ON_COMMAND(ID_VIEW_SOUND, OnViewSound)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SOUND, OnUpdateSound)
#if MAC
	ON_COMMAND(ID_VIEW_TOOLS, OnViewTools)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOOLS, OnUpdateTools)
	ON_COMMAND(ID_VIEW_CAM_TOOLS, OnViewCamTools)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CAM_TOOLS, OnUpdateCamTools)
#endif
	ON_COMMAND(ID_VIEW_VIDEO, OnViewVideo)
	ON_COMMAND(ID_VIEW_VIDEO2, OnViewVideo)
	ON_COMMAND(ID_VIEW_FULL, OnViewFull)
	ON_UPDATE_COMMAND_UI(ID_VIEW_VIDEO, OnUpdateVideo)
//	ON_COMMAND(ID_VCR_LOOP, OnLoop)
//	ON_UPDATE_COMMAND_UI(ID_VCR_LOOP, OnUpdateLoop)
	ON_COMMAND(IDC_SLD_LOOP, OnLoop)
	ON_UPDATE_COMMAND_UI(IDC_SLD_LOOP, OnUpdateLoop)
	ON_COMMAND_RANGE(ID_VCR_SLIDER_STOP, ID_VCR_SLIDER_STOP, OnVCR)
	ON_COMMAND_RANGE(ID_VCR_HOME, ID_VCR_FSTEP, OnVCR)
	ON_UPDATE_COMMAND_UI(ID_VCR_HOME, OnUpdateVCRHome)
	ON_UPDATE_COMMAND_UI(ID_VCR_BSTEP, OnUpdateVCRBStep)
	ON_UPDATE_COMMAND_UI(ID_VCR_FSTEP, OnUpdateVCRFStep)
	ON_COMMAND(ID_VCR_EXIT, OnVCRExit)
	ON_UPDATE_COMMAND_UI(ID_VCR_HOME, OnUpdateVCRHome)
//	ON_UPDATE_COMMAND_UI_RANGE(ID_VCR_HOME, ID_VCR_END, OnUpdateVCR)
	ON_UPDATE_COMMAND_UI(ID_RATE_UP, OnUpdateRateUp)
	ON_UPDATE_COMMAND_UI(ID_RATE_DOWN, OnUpdateRateDown)
	ON_COMMAND_RANGE(ID_RATE_UP, ID_RATE_DOWN, OnRate)
	ON_COMMAND(ID_KEY_SPACE, OnSpace)
	ON_COMMAND(ID_KEY_ENTER, OnReturn)
	ON_COMMAND(ID_KEY_BACK, OnBack)
	ON_COMMAND(ID_TIMER_FIRE, OnTimerFire)
	ON_COMMAND(ID_ZOOM_25, OnZoom25)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_25, OnUpdZoom25)
	ON_COMMAND(ID_ZOOM_33, OnZoom33)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_33, OnUpdZoom33)
	ON_COMMAND(ID_ZOOM_50, OnZoom50)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_50, OnUpdZoom50)
	ON_COMMAND(ID_ZOOM_100, OnZoom100)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_100, OnUpdZoom100)
	ON_COMMAND(ID_ZOOM_200, OnZoom200)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_200, OnUpdZoom200)
	ON_COMMAND(ID_ZOOM_300, OnZoom300)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_300, OnUpdZoom300)
	ON_COMMAND(ID_ZOOM_400, OnZoom400)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_400, OnUpdZoom400)
	ON_COMMAND(ID_ZOOM_800, OnZoom800)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_800, OnUpdZoom800)
	ON_COMMAND(ID_ZOOM_FIT, OnZoomFit)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_FIT, OnUpdZoomFit)
	ON_COMMAND(ID_PREVIEW_1, OnPreview1)
	ON_UPDATE_COMMAND_UI(ID_PREVIEW_1, OnUpdPreview1)
	ON_COMMAND(ID_PREVIEW_2, OnPreview2)
	ON_UPDATE_COMMAND_UI(ID_PREVIEW_2, OnUpdPreview2)
	ON_COMMAND(ID_PREVIEW_3, OnPreview3)
	ON_UPDATE_COMMAND_UI(ID_PREVIEW_3, OnUpdPreview3)
	ON_COMMAND(ID_PREVIEW_4, OnPreview4)
	ON_UPDATE_COMMAND_UI(ID_PREVIEW_4, OnUpdPreview4)
	ON_COMMAND(ID_FILE_SAVEAS_MODEL, OnMakeModel)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVEAS_MODEL, OnUpdateMakeModel)
	ON_COMMAND(ID_EDIT_MODEL, OnModel)
	ON_UPDATE_COMMAND_UI(ID_EDIT_MODEL, OnUpdateModel)
	ON_COMMAND(ID_FILE_CAPTURE, OnCapture)
	ON_COMMAND(ID_FILE_RECORD, OnRecord)
	ON_COMMAND(ID_FILE_GRAB, OnGrab)
	ON_COMMAND(ID_MY_PAINT, OnMyPaint)
	ON_COMMAND(ID_LIP_DEFAULT, OnLipDefault)
	ON_COMMAND(ID_LIP_SCENE, OnLipScene)
//	ON_MESSAGE(WM_TABLET_FLICK, OnFlickMsg)
//	ON_MESSAGE(WM_TABLET_QUERYSYSTEMGESTURESTATUS, OnTabletStatusMsg)
	ON_MESSAGE(WT_PACKET, OnWTPacket)
	ON_MESSAGE(TABLET_MESSAGE, OnTabletMsg)
	ON_COMMAND(ID_EDIT_CLEAR_SEL, OnClear)
#ifdef _DEBUG
	ON_COMMAND(ID_FILE_RECORD, OnRecord)
#endif
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSketchView construction/destruction
BOOL CSketchView::OnEraseBkgnd(CDC* pDC) 
{
	UNUSED_ALWAYS(pDC);
	return TRUE;
}

UINT CSketchView::Layer(int which /* = -1 */)
{
	if (m_pLayers)
		return m_pLayers->Layer(which);
	else
		return 0;
}

CSketchView::CSketchView()
{
	m_pTablet = 0;
	m_pFloat = 0;
	m_nSelect = 0;
	m_pClipBoard = 0;
	m_pLayers = new CLayers;
	m_pFrame = 0;
	m_pVideo = 0;
	m_pDoc = 0;
	m_pScene = 0;
	m_Bret = 0;
//	m_pWireFrame = 0;
	m_CamState = 0;
	m_CamTool = 0;
	m_wToolType = TOOL_PENCIL;
	m_nToolSaved = 0;
	m_nNum = 100;
	m_nDen = 100;
#ifdef FBVER7
	m_nScale = 100;
	m_bWasPlaying = 0;
#endif
//	m_nAngle = 0;
	m_offx = 0;
	m_offy = 0;
	m_nFactor = 2;
	m_bFit = FALSE;
	m_pDIB = 0;
	m_hdd = 0;
	m_pBG = 0;
	m_pSkin = 0;
	m_pZoom = 0;
	m_pPoints = 0;
	m_pShiftTrace = new CShiftTrace;
	m_pShiftTrace->Init(this);
	m_pBrush = new CBrusher;
	m_pBrush->Init(m_pLayers);
	m_brush_factor = m_pBrush->Factor();
	m_nWarnIfExist = 1; // f7 capture
	m_ocursr = -1;
	m_bCursor = 0;
	m_packet.command = 0;
	m_packet.pKeyDlg = 0;
	m_info.infStatus = 0;
	m_info.pBI = 0;
	m_info.avg = 8;
	m_pTemp1 = 0;
	m_nCounter = 0;
	m_LBState = 0;
//	m_palDIB = NULL;
	m_timer = new CMMTimer(0);
	m_crBack = RGB(90,90,90);
	m_scanpacket.flags = 0;
	m_scanpacket.pBed = 0;
	m_scanpacket.pPage = 0;
	SetScrollSizes(MM_TEXT, CSize(1,1));
	Reset();
}

void CSketchView::Reset()
{
	m_rate = 0;
	m_Frame = 0;
	m_Level = 0;
	m_Index = 0;
	m_EditFrame = 0;
	m_bPlaying = FALSE;
	m_play_factor = 1;
	m_Kludge = 0;
//	m_hDIB = NULL;
//	delete m_palDIB;
//	m_palDIB = NULL;
	m_bPlaySnip = 0;
//	m_hBuf = NULL;
//	m_hPeg = NULL;
//	m_sizeDoc = CSize(1,1);     // dummy value to make CScrollView happy
	m_cursr = 0;
	m_curcnt = 0;
	m_bInModel = 0;
	m_bCursor = 0;
//	m_nzoom = m_bDirty = 0;
	delete [] m_pZoom;
	m_pZoom = 0;
	m_nColorTrap = 0;
//	m_bMono = 0;
	m_bInClient = 0;
	m_bFull = 0;
//	m_width = 1;
//	m_height = 1;
	m_swidth = 1;
	m_sheight = 1;
	m_spitch = 1;
//	m_lpitch = 1;
	m_bModified = FALSE; 
	m_bDown = FALSE;
	m_bRDown = FALSE;
	m_rcSelect.left = m_rcSelect.right = 0;
//	m_rcLip.left = m_rcLip.right = 0;
//	m_maxr = 20;
//	m_scale = 6;
//	m_brushd = 100;
//	m_brushr = 10;//MAX_RADIUS;
//	m_pencild = 100;
//	m_pencilr = MAX_RADIUS;
//	m_penrad = MAX_RADIUS;
//	m_maskr = 0;
//	m_brushf = (255 - m_brushd) / (m_brushr + 1);
	if (m_hdd)
		DrawDibClose(m_hdd);
	m_hdd = DrawDibOpen();
//from flip
	DPF("flip construct");
	m_count = 0;
//	WORD i;
//	for (i = 0;i < MAX_BMPS; i++)
//		m_dibs[i] = 0;

//	m_Frame = -1;
	m_bShiftTrace = 0;
}
CSketchView::~CSketchView()
{
	DPF("myview destruct");
	//delete m_pTablet;
	delete m_pFloat;
	delete m_pClipBoard;
	m_bPlaying = FALSE;
	delete m_pLayers;
	if (m_timer)
		{
//		m_timer->StopTimer();
		delete m_timer;
		m_timer = 0;
		}
	Clear();
//	PClear();
	FClear();
	delete m_pVideo;
	delete m_pBrush;
	delete m_pShiftTrace;
	m_pVideo = 0;
	delete [] m_info.pBI;
	delete [] m_pTemp1;
	if (m_hdd)
		DrawDibClose(m_hdd);
	m_hdd = 0;
	delete [] m_scanpacket.pBed;
	delete [] m_scanpacket.pPage;
	delete [] m_pZoom;
	delete [] m_pPoints;
}

UINT CSketchView::ShiftTrace(UINT f, UINT l, UINT code)
{
	// f & l are on the stack
	// 0 is test
	// 1 is for toggle on edit cell
	// 2 thru 11 is for edit
	// 22 - 33 is for revert
	if (code == 1)
		{
		m_bShiftTrace = 0;
		m_pShiftTrace->Toggle();
		Redraw();
		return 0;
		}
	if ((code > 1) && (code < 12))
		{
		m_bShiftTrace = 1;
		m_pShiftTrace->Add(f,l,1);
		Redraw();
		return 0;
		}
	if ((code > 21) && (code < 32))
		{
		m_bShiftTrace = 1;
		m_pShiftTrace->Add(f,l,2);
		Redraw();
		return 0;
		}
	if (code == 0)
		return m_pShiftTrace->Test(f,l);
	ASSERT(0);
	return 0;
}

BOOL CSketchView::HasPaste(UINT code /*= 0*/)
{
	if (!m_pClipBoard)
		return 0;
//	if (m_pScene->ColorMode())
//		return 0;
	if (!code)
		return 1;
	if (code > 1)
		{
		delete m_pFloat;
		m_pFloat = 0;
		delete m_pClipBoard;
		m_pClipBoard = 0;
		m_nSelect = 0;
		Invalidate(0);
		return 0;
		}
	delete m_pFloat;
	m_pFloat = new CFloat;
	if (m_nSelect == 2)
		m_pFloat->Make(m_rcSelect,m_pClipBoard);
	else
		m_pFloat->Make(m_pClipBoard);
	m_nSelect = 0;
	Invalidate(0);
	return 1;
}

void CSketchView::OnUpdateOutside(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(ViewActive() && m_nSelect == 2 ? TRUE : FALSE);
}

void CSketchView::OnEditOutside()
{
	if (m_pLayers->Crop(m_rcSelect))
		m_bModified = TRUE;
	EchoIt();
	if (m_nSelect)
		{
		if(!(m_pDoc->Option(SELECT_FLAGS) & 1))
			m_nSelect = 0;
		}
	if(m_pDoc->Option(SELECT_FLAGS) & 2)
		OnFunctionKey(ID_KEY_F5);
}

void CSketchView::SaveSceneSettings()
{
	m_pScene->LipRect(&m_rcSelect,1);
}

void CSketchView::OnLipDefault()
{
	m_rcSelect.left = m_rcSelect.right = 1;
}

void CSketchView::OnLipScene()
{
	m_pScene->LipRect(&m_rcSelect);
}

void CSketchView::SetClipBoard(BYTE * p, UINT flags)
{
//	if (!flags)
//		{
//		delete m_pClipBoard;
//		m_pClipBoard = p;
//		}
//	PostMessage(ID_EDIT_PASTE,0);
	delete m_pFloat;
	m_pFloat = new CFloat;
	UINT * pHead = (UINT *)p;
	UINT w = pHead[3];
	UINT h = pHead[4];
	if (flags & 4)		// force rectangle
		{
		m_rcSelect.left = pHead[1];
		m_rcSelect.top = pHead[2];
		m_rcSelect.right = m_rcSelect.left + w;
		m_rcSelect.bottom = m_rcSelect.top + h;
		}
	else
		{
		if (m_rcSelect.right <= m_rcSelect.left)
			{
			if (!m_rcSelect.right)
				m_pScene->LipRect(&m_rcSelect);
			if (m_rcSelect.right <= m_rcSelect.left)
				{
				m_rcSelect.left = (int)(m_swidth - w) / 2;
				m_rcSelect.top  = (int)(m_sheight - h) / 2;
				m_rcSelect.right = (int)m_rcSelect.left + w;
				m_rcSelect.bottom = (int)m_rcSelect.top + h;
				}
			}
		}
	m_nSelect = 2;
	m_pFloat->Make(m_rcSelect,p);
	m_pFloat->m_bNoMapPalette = (flags & 2) ? 0 : 1;
	m_nSelect = 0;
	if (flags & 1)
		{
		ApplyFloat();
//		m_pFloat->GetRect(m_rcSelect);
		BlowFloat();
		}

	Invalidate(0);
}

BYTE * CSketchView:: MakeClipMask()
{
	int w = m_rcSelect.right - m_rcSelect.left;
	int h = m_rcSelect.bottom - m_rcSelect.top;
	if (m_nPoints < 3)
		return 0;
	BYTE * p = new BYTE[w * h];
	memset(p,1,w*h);	// clear it
	UINT i = 0;						// draw the lasso
	int px = m_pPoints[0].x;
	int py = h - 1 - m_pPoints[0].y;
	p[w*py+px] = 255;
	for (; i < m_nPoints; i++)
		{
		int x = m_pPoints[i].x;
		int y = h-1 - m_pPoints[i].y;
		if ((x == px) && (y == py))
			continue;
		int dx = x - px;
		int dy = y - py;
		if (abs(dy) > abs(dx))
			{
			int yy;
			int ddy = dy > 0 ? 1 : -1;
			for (yy = py; yy != y; yy += ddy)
				{
				int xx = px + ((yy - py) * dx) / dy;
				p[w*yy+xx] = 255;
				}
			}
		else
			{
			int xx;
			int ddx = dx > 0 ? 1 : -1;
			for (xx = px; xx != x; xx += ddx)
				{
				int yy = py + ((xx - px) * dy) / dx;
				p[w*yy+xx] = 255;
				}
			}
		px = x;
		py = y;
		}
	UINT stackc = 0;
	UINT stack_limit = 10000;
	UINT * stack = new UINT[stack_limit];
	stack[stackc++] = 0;
	stack[stackc++] = w-1;
	stack[stackc++] = w * (h-1);
	stack[stackc++] = w * h - 1; // seed with corners
	for (;stackc--;)			// pop next entry
		{
		UINT v = stack[stackc];
		int x = v % w;
		int y = v / w;
	//	DPF("popc:%d,x:%d,y:%d",stackc,x,y);
		ASSERT(x >= 0);
		ASSERT(y >= 0);
		ASSERT(x < w);
		ASSERT(y < h);
		if (p[v] == 1)						// if outside
			{
			p[v] = 0;						// clear alpha
			int z;
			for (z = 0; z < 4; z++)			// do four neighbors
				{
				int xx = x;
				int yy = y;
				if (z == 0)
					yy--;
				else if (z == 1)
					yy++;
				else if (z == 2)
					xx--;
				else
					xx++;
				if ((xx < 0) || (xx >= w) || (yy < 0) || (yy >= h))
					continue;
				if (stackc >= stack_limit) // check if more stack needed
					{
					UINT * tp = stack;
					stack_limit += 10000;
					stack = new UINT[stack_limit];
					memcpy(stack, tp, stackc * sizeof(*tp));
					delete [] tp;
					}
			//	DPF("stc:%d,x:%d,y:%d",stackc,xx,yy);
				stack[stackc++] = w * yy + xx;
				}
			}
		}
	delete [] stack;
DPF("stack limit:%d",stack_limit);
	int x, y;
	BYTE * pp = p;
	for (y = 0; y < h; y++, pp+=w)
		{
		for (x = 0; x < w; x++)
			if (pp[x] == 1)
				pp[x] = 255;
		}
	return p;
}

UINT CSketchView::CutCopy(UINT code)
{
	if (m_nSelect != 2)
		{
		if (code &&  (m_wToolType == TOOL_SELECT))
			{
			MessageBeep(0);
			return 1;
			}
		return 0;
		}
	if (!code--)		
		return 1;			// just testing
if ((m_rcSelect.left < 0) || (m_rcSelect.top < 0))
	{
//	ASSERT(0);
	}
//	if (m_rcSelect.left < 0) m_rcSelect.left = 0;
//	if (m_rcSelect.top  < 0) m_rcSelect.top = 0;
	BYTE * pMask = 0;
	if (m_bLassoing)
		{
		pMask = MakeClipMask();
		}
	if (code > 1)
		{
		m_pLayers->GetClip(m_rcSelect, code, pMask);
		}
	else
		{
		delete m_pClipBoard;
		m_pClipBoard = m_pLayers->GetClip(m_rcSelect, code, pMask);
		}
	delete [] pMask;
	if (code != 1)
		{
		if (m_pLayers->Modified())
			m_bModified = TRUE;
		EchoIt();
		}
	Invalidate();
	if (m_nSelect)
		{
		if(!(m_pDoc->Option(SELECT_FLAGS) & 1))
			m_nSelect = 0;
		}
	if(m_pDoc->Option(SELECT_FLAGS) & 2)
		OnFunctionKey(ID_KEY_F5);
	return 1;
}

void CSketchView::ApplyFloat()
{
	m_pLayers->StashUndo();
	m_pLayers->ApplyFloat(m_pFloat);
	if (m_pLayers->Modified())
		m_bModified = TRUE;
	EchoIt();
	return;
}
#ifdef FBTPC
void CSketchView::InitTablet()
{
	if (!m_pDoc)
		return;
	m_pTablet = ((CSketchApp*) AfxGetApp())->m_pTablet;
	if (!m_pTablet)
		m_pTablet = ((CSketchApp*) AfxGetApp())->m_pTablet = new CTablet;
	
	BOOL bUseWT = ((CSketchApp*) AfxGetApp())->HaveWinTab();
	bUseWT = bUseWT &&	m_pDoc->Option(WINTAB);
	

	BOOL bPrimary = m_pDoc->Option(TAB_PRIMARY);
	//m_pTablet = new CTablet;
	UINT res = m_pTablet->Open(m_hWnd, m_brush_factor, bPrimary, bUseWT);
	if (res == 1)
		return; // have wintab
	else if (bUseWT)
		{
		m_pDoc->Option(WINTAB,1,0);
		AfxMessageBox(IDS_NOWINTAB);
		}
	else if (res == 2)
		{
		m_pDoc->Option(TABLET_USED,1,1);
		}
	else
		{
		m_pTablet = 0;
		if (m_pDoc->Option(TABLET_USED))
			{
			m_pDoc->Option(TABLET_USED,1,0);
			AfxMessageBox(IDS_NO_TABLET, MB_OK);
			}
		}
}
#else
void CSketchView::WinTab()
{
	if (!m_pDoc)
		return;
//	delete m_pTablet;
//	m_pTablet = 0;
	if(m_pDoc->Option(WINTAB))
		{
		BOOL bPrimary = m_pDoc->Option(TAB_PRIMARY);
		m_pTablet = new CTablet;
		int xres = m_brush_factor;// * GetSystemMetrics(SM_CXVIRTUALSCREEN);
		int yres = m_brush_factor;// * GetSystemMetrics(SM_CYVIRTUALSCREEN);
		m_pTablet->Open(m_hWnd, xres,yres, bPrimary);
		}
}
#endif
/////////////////////////////////////////////////////////////////////////////
// CSketchView drawing

void CSketchView::OnDraw(CDC* pDC)
{
//	DPF("View:draw:paint,bret:%d", m_Bret);
	if (!pDC->IsPrinting() && m_pDoc)
		{
		if (m_packet.command && !m_Kludge)
			{
			CapPaint(pDC);
			return;
			}
		if (m_scanpacket.flags & 2)
			{
			ScanPaint(pDC);
			return;
			}
		MyCursor(0,0);// pDC->m_hDC);
		if (m_Bret == 0)
			{
			CMyPaint(pDC);
			DrawSelection();
			}
		else
			FMyPaint(pDC);
		MyCursor(1, 0);//pDC->m_hDC);
		Progress(998+m_bPlaying,m_Frame+1);
		}
	else
		CMyPrint(pDC);
}

LRESULT CSketchView::OnDoRealize(WPARAM wParam, LPARAM)
{
	return 0L;
}

void CSketchView::OnInitialUpdate()
{
	DPF("dibviewinitial");
	//HWND hWnd = ::GetDesktopWindow();
	ATOM atom = ::GlobalAddAtom(MICROSOFT_TABLETPENSERVICE_PROPERTY);    
	const DWORD_PTR dwHwndTabletProperty = 
 TABLET_DISABLE_PRESSANDHOLD | // disables press and hold (right-click) gesture
 TABLET_DISABLE_PENTAPFEEDBACK | // disables UI feedback on pen up (waves)
 TABLET_DISABLE_PENBARRELFEEDBACK | // disables UI feedback on pen button down(circle)
 TABLET_DISABLE_FLICKS // disables pen flicks (back, forward, drag down, drag up)
		;
	BOOL bRes = SetProp(m_hWnd, MICROSOFT_TABLETPENSERVICE_PROPERTY,
				reinterpret_cast<HANDLE>(dwHwndTabletProperty));
		bRes = bRes;
		LASTERR;

	SetScrollSizes(MM_TEXT, CSize(1,1));
	CScrollView::OnInitialUpdate();
	ASSERT(GetDocument() != NULL);
	m_pDoc = (CSketchDoc *)GetDocument();
	m_pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
//	m_pSheet = pAppFrame->SheetAddr();
	Reset();
	m_pFrame->AttachDoc(m_pDoc);
	m_bOldCursor = 1;
	UINT zz = AfxGetApp()->GetProfileInt("Options","BackColor", 90);
	if (zz > 255) zz = 255;
	m_crBack = RGB(zz,zz,zz);

	OnTool(ID_TOOL_PENCIL + m_pDoc->SelectTool());
//	OnTool(ID_TOOL_WAND);

//	Adjust();
//	Reset();
//	Setup();
	ViewMoved();
	DPF("dibviewinitial done");
}

UINT CSketchView::Setup(BOOL bClose /* = 0 */)
{
	DPF("view setup,close:%d",bClose);
	if (bClose)
		{
		Clear();
		m_pScene = 0;
		return 0;
		}
	m_Bret = 0;
#ifdef FBVER7
	if (!m_pDoc)
		{
DPF("no doc");
		return 1;
		}
	Clear();
	m_pScene = m_pDoc->GetDocScene();
	if (!m_pScene)
		{
DPF("no scene");
		return 2;
		}
#else
	CSetup();
#endif
	m_bTablet = FALSE;
	if (m_pScene && (m_pScene->LevelCount() > 1))
		VSelectCell(0,1);
	else
		VSelectCell(0,0);
	int q = 100;
	if (m_pScene)
		{
		q = m_pScene->Zoom();
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
			q = 1600; // actually 33 %
		}
	Adjust(q);
	return 0;
}


UINT CSketchView::ViewMode(UINT mode /*= 99*/)
{
	if (mode != 99)
		m_Bret = mode;
	return m_Bret;
}

BOOL CSketchView::IsUsingPenTool ()
{
	if (ViewMode() == 0) {
		if (m_wToolType == TOOL_PENCIL) {
			UINT zz = m_pDoc->ToolInfo(5);
			UINT z = ((zz / 32) & 4) + (zz & 3);
			UINT pencil_type = IDC_PEN_FREE + z;
			if ((pencil_type == IDC_PEN_FREE) || (pencil_type == IDC_PEN_SMART)) {
				return TRUE;
			}
			else {
				return FALSE;
			}
		}
		else if (m_wToolType == TOOL_TRACE) {
			return TRUE;
		}
		else if (m_wToolType == TOOL_BRUSH) {
			return TRUE;
		}
		else if (m_wToolType == TOOL_ERASER) {
			return TRUE;
		}
		else {
			return FALSE;
		}
	}
	else {
		return FALSE;
	}
}

void CSketchView::OnActivateView(BOOL bActivate, CView* pActivateView,
					CView* pDeactiveView)
{
	CScrollView::OnActivateView(bActivate, pActivateView, pDeactiveView);
	if (bActivate)
	{
		ASSERT(pActivateView == this);
		InitTablet();
		OnDoRealize((WPARAM)m_hWnd, 0);   // same as SendMessage(WM_DOREALIZE);
	}
}

void CSketchView::CenterIt(CRect &rect)
{
	int ccx = rect.right - rect.left;
	int ccy = rect.bottom - rect.top;
DPF("ccx:%d,ccy:%d",ccx,ccy);
	UINT sw, sh;
//	if (m_nAngle)
//		{
//		sh = m_roth;
//		sw = m_rotw;
//		}
////	else
//		{
		sw = m_nNum * m_swidth / m_nDen;
		sh = m_nNum * m_sheight / m_nDen;
//		}
	if (ccx > (int)sw)
		m_offx = (ccx - sw) / 2;
	else
		m_offx = 0;
	if (ccy > (int)sh)
		m_offy = (ccy - sh) / 2;
	else
		m_offy = 0;
DPF("sw:%d,sh:%d",sw,sh);
DPF("offx:%d,offy:%d",m_offx,m_offy);
}

void CSketchView::Adjust(UINT Num, UINT den /* = 0 */ , CPoint * pCenter /*= 0 */)
{
	int dw,dh;
	DPF("Adjust %d",Num);
	CPoint cp;
	CPoint pt;
	CPoint image;
	CRect rect;

	if (pCenter)
		{
		GetClientRect(&rect);
		DPR("client",&rect);
		cp.x = (rect.right + rect.left + 1) / 2;
		cp.y = (rect.bottom + rect.top + 1) / 2;
		image = *pCenter;
//		XlatePoint(image);
		DPF("b4 cp:%d,%d,im:%d,%d",cp.x,cp.y,image.x,image.y);
		}

	if (!den)
		{
		if (Num > 800)
			{
			Num = 40;
			den = m_nFactor * 60; // 40/120 = 33 %
			}
		else
			den = m_nFactor * 50;
		}
	m_nDen = den;
	m_nNum = Num;
/*
	if (m_Bret < 2)
		{
		if (m_bWasPlaying)
			m_nNum = m_nNum * m_nScale / 100;
		m_bWasPlaying = 0;
		}
	else
		{
		if (m_bWasPlaying)
			m_nNum = 100 * m_nNum / m_nScale;
		m_bWasPlaying = 0;
		}
*/
//	if (m_nAngle)
//		{
	//	SetRotateSizes();
//		dh = m_roth;//(UINT)sqrt(qh);
//		dw = m_rotw;//sh;
//		}
//	else
//		{
		dw = m_nNum * m_swidth / m_nDen;
		dh = m_nNum * m_sheight / m_nDen;
//		}
	DPF("sizes,w:%d,h:%d",dw,dh);
//	CSize size(MulDiv(dw,m_nNum,m_nDen),MulDiv(dh,m_nNum,m_nDen));
	CSize size(dw,dh);
	DPF("scroll,w:%d,h:%d",size.cx,size.cy);
	
	SetScrollSizes(MM_TEXT, size);
	DPF("before Invalidate");
	if (pCenter)
		{
		int cx = rect.right - rect.left;
		int cy = rect.bottom - rect.top;
		pt = GetScrollPosition();
		if (cx > dw)
			{
			m_offx = (cx - dw) / 2;
			pt.x = 0;
			}
		else
			{
			m_offx  = 0;
			pt.x = (image.x * m_nNum) / m_nDen - cp.x;
			}
		if (cy > dh)
			{
			m_offy = (cy - dh) / 2;
			pt.y = 0;
			}
		else
			{
			m_offy = 0;
			pt.y = (image.y * m_nNum) / m_nDen - cp.y;
			}
		ScrollToPosition(pt);
//		DPF("b4 cp:%d,%d,im:%d,%d",cp.x,cp.y,image.x,image.y);
		}
	else
		{
		GetClientRect(rect);
		CenterIt(rect);
		}
	Invalidate(FALSE);
	UpdateWindow();
	if (m_nDen)
		m_pFrame->Progress(990, 100 * m_nNum / m_nDen);
}


/////////////////////////////////////////////////////////////////////////////
// CSketchView diagnostics

void CSketchView::OnSize(UINT nType, int cx, int cy)
{ 
	if (m_bFit)
		{
		if (!m_nCounter++)
		{
		RECT	rect;
		GetClientRect(&rect);
		int cw = rect.right - rect.left;
		int ch = rect.bottom - rect.top;
		int n,d;
		if ((cw * m_sheight) < (ch * m_swidth))
			{
			n = cw;
			d = m_swidth;
			}
		else
			{
			n = ch;
			d = m_sheight;
			} 
		if (d &&((n / d) < 20))
			{
			m_nNum = n;
			m_nDen = d;
			CPoint pt;
			pt.x = 0;
			pt.y = 0;
			MyCursor(0);
			ScrollToPosition(pt);
			MyCursor(1);
			}
		}
	m_nCounter--;
	}
#ifdef CENTERED
	DPF("OnSize:%d,%d\n",cx,cy);
	CRect	rect;
	rect.top = 0;
	rect.left = 0;
	rect.right = cx;
	rect.bottom = cy;
//	GetParent()->CalcWindowRect(&rect,0);
	CalcWindowRect(&rect,0);
	CenterIt(rect);
/*
	int ccx = rect.right - rect.left;
	int ccy = rect.bottom - rect.top;
DPF("ccx:%d,ccy:%d",ccx,ccy);
	UINT sw = m_nNum * m_swidth / m_nDen;
	UINT sh = m_nNum * m_sheight / m_nDen;

	if (ccx > (int)sw)
		m_offx = (ccx - sw) / 2;
	else
		m_offx = 0;
	if (ccy > (int)sh)
		m_offy = (ccy - sh) / 2;
	else
		m_offy = 0;
DPF("offx:%d,offy:%d",m_offx,m_offy);
*/
#endif
	CScrollView::OnSize(nType,cx,cy);
}

//void CSketchView::OnMove(int x, int y)
void CSketchView::ViewMoved()
{
//PF("view move, x:%d,y:%d",x,y);
	m_winx = 0;
	m_winy = 0;
	if (GetSystemMetrics(SM_CMONITORS))
		{
		m_winx -= GetSystemMetrics(SM_XVIRTUALSCREEN);
		m_winy -= GetSystemMetrics(SM_YVIRTUALSCREEN);
		}
	CRect rectWin;
	GetWindowRect(rectWin);
	m_winx += rectWin.left;
	m_winy += rectWin.top;
//	CPoint pt(0,0);
//	ClientToScreen(&pt);
	
#ifdef WIN32
	m_winx += 3;
	m_winy += 3;
#endif
//	CScrollView::OnMove(x,y);
}

BOOL CSketchView::StartSelection(CPoint point)
{
	if (m_pFloat)
		{
		if (GetKeyState(VK_SHIFT) < 0)
			m_pFloat->ShiftOn(1);
		if (GetKeyState(VK_CONTROL) < 0)
			m_pFloat->Rotated(1,1);
		if (!m_pFloat->StartDrag(point))
			{
			ApplyFloat();
			m_pFloat->GetRect(m_rcSelect);
			BlowFloat();
			return 1;
			}
		SetCapture();
		m_bDown = TRUE;
		return 1;
		}
	if (m_nSelect == 14) // curve 0
		{
		m_Curve2 = m_Curve3 = point;
		SetCapture();
		Invalidate();
		m_bDown = TRUE;
		return 1;
		}
	if (m_nSelect == 15) // curve 
		{
		m_Curve3 = point;
		SetCapture();
		Invalidate();
		m_bDown = TRUE;
		return 1;
		}
	if (m_nSelect)
		{
	//	m_nSelect = 0;
		m_nSelectDrag = SelectionHit(point);
		if (!m_nSelectDrag || m_Bret)
			Invalidate();
		else
			{
			m_SelectStart = point;
			SetCapture();
			m_bDown = TRUE;
			return 1;
			}
		delete m_pPoints;
		m_pPoints = 0;
		}
	m_nSelect = 0;
#ifndef FBVER7
	if ((m_wToolType != TOOL_HAND) && m_pLayers && m_pLayers->IsOverlay())
		{
		return 0;
		}
#endif
	SetCapture();
	m_bDown = TRUE;
	if (m_Bret)
		return 0;
	if (!m_nSelect && (m_wToolType == TOOL_SELECT) && (GetKeyState(VK_CONTROL) < 0))
		{
//		m_Bret = 1;
		return 0;
		}
	if (m_wToolType == TOOL_PENCIL)
		{
		UINT v;
		v = m_pDoc->ToolInfo(5);
		v = ((v / 32) & 4) + (v & 3);
		if (v && (v < 5))
			{
DPF("v:%d",v);
			m_Curve4 = point;
			m_nSelect = 9 + v;
			MyCursor(0);
			}
		}
	else if (m_wToolType == TOOL_SELECT)
		{
		int z = m_pDoc->Option(SELECT_FLAGS);
		z |= 2;
		z ^= 2; // disable auto
		m_pDoc->Option(SELECT_FLAGS,1,z);
		m_pFrame->UpdateTools(1);
		m_bLassoing = (z & 4) || (GetKeyState(VK_SHIFT) < 0) ? 1 : 0;
		delete [] m_pPoints;
		m_pPoints = 0;
		m_nPoints = 0;
		m_nSelect = 1;
		}
	else if (m_wToolType == TOOL_ZOOM)
		m_nSelect = 9;
	if (m_nSelect)
		{
		m_SelectStart = point;
		m_rcSelect.top = m_rcSelect.bottom = point.y;
		m_rcSelect.left = m_rcSelect.right = point.x;
		return TRUE;
		}
	return FALSE;
}

void CSketchView::StartDraw( CPoint point)
{
	UINT t = GetTickCount();
	DPF("start draw,t:%u",t);
	m_wPressure = 255;
	m_wMaxPressure = 1000 * m_pDoc->Option(MIN_PRESSURE) +
							m_pDoc->Option(MAX_PRESSURE);
	m_bIgnore = FALSE;
	m_bPlaying = FALSE;
	if (m_Bret && (m_Bret != 3)) return;
DPF("start draw,x:%d,y:%d",point.x, point.y);
	DPF("b4 xlate,x:%d,y:%d",point.x,point.y);
	XlatePoint(point);
//	if (((UINT)point.x >= m_swidth) || ((UINT)point.y >= m_sheight))
//		return;
//ASSERT(point.x >= 0);
	DPF("before draw,x:%d,y:%d",point.x,point.y);
	if (m_bShiftTrace)
		{
		SetCapture();
		m_bDown = TRUE;
		m_pShiftTrace->Start(point);
		return;
		}
	if (StartSelection(point) && !m_nColorTrap)
		return;
DPF("before draw,x:%d,y:%d",point.x,point.y);
//	CPoint tp(rectClip.left,rectClip.top);
	if (m_CamState)
		{
		m_bDown = TRUE;
			CameraStart(point);
			return;
		}
	if (m_Bret == 3)
		{
		FStartDraw(point);
		return;
		}
#ifdef FBVER7
	if ((m_wToolType != TOOL_HAND) && (m_wToolType != TOOL_ZOOM) &&
		(m_wToolType != TOOL_PENCIL) && (m_wToolType != TOOL_BRUSH) &&
			m_pLayers && m_pLayers->IsOverlay())
#else
	if ((m_wToolType != TOOL_HAND) && m_pLayers && m_pLayers->IsOverlay())
#endif
		{
		m_bIgnore = TRUE;
		m_bDown = m_bRDown = FALSE;
		AfxMessageBox(IDS_USING_OVERLAY, MB_OK);
		return;
		}
	m_toolflags = m_pDoc->ToolInfo(5);
	UINT nDropping = 0;
	if (m_nColorTrap > 1)
		{
		m_nSelect = 0;
		if (m_nColorTrap == 4)
			{
			m_pFrame->ColorTrap(point.x , m_sheight - 1 - point.y,1);
			m_nColorTrap = 5;
			}
		else
			{
			m_bIgnore = TRUE;
			if ((GetKeyState(VK_CONTROL) < 0) || m_bRDown)
				point.x = -1;
			m_nColorTrap = m_bDown = m_bRDown = 0;//FALSE;
			m_pFrame->ColorTrap(point.x , m_sheight - 1 - point.y,0);
			}
		return;
		}
	if (m_pScene->ColorMode() && (GetKeyState(VK_CONTROL) < 0))
		{
		if (m_wToolType == TOOL_FILL || 
			m_wToolType == TOOL_BRUSH || 
			m_wToolType == TOOL_TRACE || 
			m_wToolType == TOOL_PENCIL)
			nDropping = 1;
		}
	if (m_wToolType == TOOL_WAND)
		{
		UINT index;
		if (index = m_pLayers->DisPaint(point.x, point.y))
			{
//			m_pFrame->SetDisPaint(index);
//			m_pFrame->ChangeTool(TRUE);
			m_bIgnore = TRUE;
			m_bDown = m_bRDown = FALSE;
			}
		return;
		}
	if (m_pScene->ColorMode() && (GetKeyState(VK_SHIFT) < 0))
		{
		if (m_wToolType == TOOL_EYEDROP)
			{
			UINT index = m_pScene->PalIndex(m_Level);
#ifdef ZZZZ
			if (m_pScene->PalLocked(index))
				{
				m_bIgnore = TRUE;
				m_bDown = m_bRDown = FALSE;
				AfxMessageBox(IDS_PALETTE_LOCKED, MB_OK);
				return;
				}
#else
			if (m_pScene->PalShared(index))
				{
				m_bIgnore = TRUE;
				m_bDown = m_bRDown = FALSE;
				CPalSharedDlg dlg;
				dlg.m_bModOne = 0;
				if (dlg.DoModal() != IDOK)
					{
					SetFocus();
					return;
					}
				if (dlg.m_bModOne)
					{
					CNewPals * pPals = m_pScene->PalettePtr(index);
					UINT nindex = m_pScene->NewPalette(dlg.m_name, pPals);
					m_pScene->PalIndex(m_Level,nindex);
					}
				}
#endif
			nDropping = 2;
			}
		}

	if (m_wToolType == TOOL_HAND)
		{
		m_Bret = 0;
		m_origx = point.x;// / (int)m_scale;
		m_origy = point.y;// / (int)m_scale;
		}
	else if (m_wToolType == TOOL_ZOOM)
		{
		m_Bret = 0;
		if (DoZoom(point))
			return;
		}
	else if (nDropping == 2)
		{
		UINT Index = m_pDoc->Color();//m_pLayers->GetIndex(point.x, point.y);
		UINT r,g,b;
		if (!m_pLayers->GetRGB(point.x, point.y, r,g,b))
			{
			int x = point.x;
			int y = m_sheight - 1 - point.y;
			r = m_pBG[m_spitch*y+3*x+2];
			g = m_pBG[m_spitch*y+3*x+1];
			b = m_pBG[m_spitch*y+3*x+0];
			}
		m_pFrame->SetColor(Index,r,g,b,255);
		m_pFrame->ChangeTool(TRUE);
		m_bIgnore = TRUE;
		m_bDown = m_bRDown = FALSE;
		return;
		}
	else if (m_wToolType == TOOL_EYEDROP || (nDropping == 1))
		{
//		point.x /= (int)m_scale;
//		point.y /= (int)m_scale;
		if (m_bRDown)
			{
			UINT layer;
			if (m_pLayers->HaveAlpha(point.x, point.y, &layer))
				{
				CLevelTable * pTable = m_pLayers->LevelTable();
				if (layer < 5)
					layer++;
				else if (layer == 5)
					layer = 0;
				pTable->layer = layer;
				m_pLayers->LevelTable(1);
				Update(0,0,20);
//				m_pDoc->Color(Index);
				m_pFrame->ChangeTool(TRUE);
				m_bIgnore = TRUE;
				}
			}
		else
			{
#ifdef FBVER7
			UINT Index = 0;//m_pLayers->GetIndex(point.x, point.y);
#else
			UINT Index = m_pLayers->GetIndex(point.x, point.y);
#endif
DPF("dropper:%d",Index);
			if (Index != -1)
				{
				m_pDoc->Color(Index);
				m_pFrame->ChangeTool(TRUE);
				if ((m_wToolType != TOOL_EYEDROP) || (m_nToolSaved != 4))
					m_bIgnore = TRUE;
				}
			}
		return;
		}
	else if (m_wToolType == TOOL_FILL)
		m_Bret = 0;
	else if (m_wToolType == TOOL_TRACE)
		m_Bret = 0;
	else if (GetKeyState(VK_SHIFT) < 0)
		m_Bret = 2;
	else if (GetKeyState(VK_CONTROL) < 0)
		m_Bret = 1;
	else
		m_Bret = 0;
	if (m_Bret == 0)
		{
		if ((m_wToolType != TOOL_HAND) && (m_wToolType != TOOL_ZOOM) && !nDropping)
			CStartDraw(point);
		}
	else
		{
		m_BretFrame = m_Frame;
		m_BretLevel = m_Level;
		FStartDraw(point);
		}
}

void CSketchView::OnLButtonDown(UINT, CPoint point)
{ 
DPF("lbut down,x:%d,y:%d",point.x, point.y);
	if (!m_LBState)
		{
		m_LBState = 2;
		StartDraw(point);
		}
}		

void CSketchView::OnRButtonDown(UINT, CPoint point)
{ 
DPF("rbut down,x:%d,y:%d",point.x, point.y);
	if (m_bDown) return;
	m_bRDown = TRUE;
	StartDraw(point);
}

BOOL CSketchView::DoZoom(CPoint point, BOOL bOut /* = 0 */)
{
	CPoint pt = GetScrollPosition();
	int x = point.x;
	int y = point.y;
DPF("zoom down,x:%d,y:%d",x,y);
DPF("scroll,%d,%d",pt.x,pt.y);
	int sx = x * m_nNum / m_nDen - pt.x + m_offx;
	int sy = y * m_nNum / m_nDen - pt.y + m_offy;
	UINT z = m_nNum;
	if ((GetKeyState(VK_MENU) < 0) || (GetKeyState(VK_CONTROL) < 0) || bOut)
		z /= 2;
	else
		z *= 2;
	z = (m_nFactor * 50 * z) / m_nDen;
	if ((z < 25) || (z > 800))
		{
		MessageBeep(0);
		return 1;
		}
	if (z > 400)
		z = 800;
	else if (z > 300)
		z = 400;
	else if (z > 200)
		z = 300;
	else if (z > 100)
		z = 200;
	else if (z > 50)
		z = 100;
	else if (z > 25)
		z = 50;
	else
		z = 25;
	Adjust(z);
DPF("num:%d,den:%d",m_nNum, m_nDen);
	pt.x = x * m_nNum / m_nDen - sx;
	pt.y = y * m_nNum / m_nDen - sy;
DPF("scroll,%d,%d",pt.x,pt.y);
	MyCursor(0);
	ScrollToPosition(pt);
	MyCursor(1);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////

void CSketchView::NoScrub()
{
	if ((m_Bret == 3) && (m_pDoc->Option(OVERRIDE_FLAGS) & 2))
		{
		m_Bret = 0;
		if (CheckModified(0))//bCanCancel))
			return;// 1;
		CSelect(m_Frame, m_Level);
		}
}

void CSketchView::StopDraw(CPoint point)
{
DPF("stop draw");
	m_LBState = 0;
	BOOL bIgnore = m_bIgnore;
	m_bIgnore = 0;
	if (GetCapture() != this)
		return;
	m_bDown = m_bRDown = FALSE;
	if (m_bShiftTrace)
		{
		XlatePoint(point);
		m_bShiftTrace = m_pShiftTrace->Stop(point);
		ReleaseCapture();
		if (!m_bShiftTrace)
			Invalidate();
		return;
		}
	if (m_nSelect == 9)
		{
		m_nSelect = 0;
		XlatePoint(point);
		DPF("point:%d,%d",point.x,point.y);
		int dx = abs(m_SelectStart.x - point.x);
		int dy = abs(m_SelectStart.y  - point.y);
		point.x = (point.x + m_SelectStart.x) / 2;
		point.y = (point.y + m_SelectStart.y) / 2;
		ReleaseCapture();
		int n,d;
		if ((dx * m_sheight) > (dy * m_swidth))
			{
			d = dx;
			n = m_swidth;
//			dy = m_swidth;
			}
		else
			{
			d = dy;
			n = m_sheight;
			}
		if (!d || ((n / d) > 20))
			{
DPF("bad zoon,n:%d,d:%d",n,d);
			DoZoom(point);
			return;
			}
DPF("point.x:%d,y:%d,n:%d,d:%d",point.x,point.y,n,d);
		Adjust(n,d);
		CPoint pt;
		pt.x = point.x - dx / 2;
		pt.y = point.y - dy / 2;
DPF("scroll,%d,%d",pt.x,pt.y);
		XlateXPoint(pt);
DPF("scroll,%d,%d",pt.x,pt.y);
		MyCursor(0);
		ScrollToPosition(pt);
		MyCursor(1);
		return;
		}
	if (m_nSelect == 16)
		{
		m_nSelect = 0;
		bIgnore = TRUE;
		}
	if (m_nSelect == 13) // curve 0
		{
		XlatePoint(point);
//		m_Curve4 = point;
		m_Curve2 = m_Curve3 = m_Curve4 = point;
		ReleaseCapture();
		m_nSelect = 14;
		EchoIt();
		MyCursor(1);
		return;
		}
	if (m_nSelect == 14) // curve 1
		{
		XlatePoint(point);
		m_Curve2 = m_Curve3 = point;
		ReleaseCapture();
		m_nSelect = 15;
//		EchoIt();
		MyCursor(1);
		return;
		}
	if (m_nSelect == 15)
		{
		XlatePoint(point);
		m_Curve3 = point;
//		ReleaseCapture();
//		m_nSelect = 0;
		}
	if (m_nSelect > 9)
		{
		ReleaseCapture();
//		AdjustSelection(point);
		m_bModified = TRUE; 
//		m_bTablet = m_pDoc->Option(WINTAB) && m_pDoc->Option(TAB_PRESSURE);
//		m_wMaxPressure = m_pDoc->Option(MAX_PRESSURE);
		m_bErasing = m_pDoc->ToolInfo(0);
		m_color = m_pDoc->Color();
		m_radius = m_pDoc->Radius() - 1;
		m_density = m_pDoc->Density();
//		m_density = MulDiv(m_density, m_density, 100);
		UINT type;
		if (m_bErasing)
			type = (m_toolflags & 1024) ? 6 : 0;
		else
			type = (m_toolflags & 16) ? 6 : 0;
		m_pLayers->DrawInit(m_color,type,m_bErasing);
		m_pBrush->SetSolid(m_pLayers->Solid());
		if (m_nSelect >= 13)
			m_pBrush->DrawCurve(m_SelectStart,m_Curve2,m_Curve3, m_Curve4,
						m_radius,m_density);
		else
			m_pBrush->DrawObject(m_nSelect - 10,
						CPoint(m_rcSelect.left,m_rcSelect.top),
						CPoint(m_rcSelect.right, m_rcSelect.bottom),
								m_radius,m_density);
		m_nSelect = 0;
		EchoIt();
		Invalidate(); // erase xor stuff
		MyCursor(1);
		return;
		}
	BOOL bRedraw = 0;
	if (m_nSelect)
		{
//ASSERT(m_nSelect == 1);
		//	if (m_nPoints < 5)
		//		m_nPoints = m_nPoints;
		if (((m_rcSelect.bottom - m_rcSelect.top) < 3) || 
			((m_rcSelect.right - m_rcSelect.left) < 3))
			{
			if (m_bLassoing)
				{
				delete [] m_pPoints;
				m_pPoints = 0;
				m_nPoints = 0;
				}
			m_nSelect = 0;
			}
		else
			{
			if ((m_nSelect == 1) && m_bLassoing && m_nPoints)
				{
				UINT i;	// make em relative
				for (i = 0; i < m_nPoints; i++)
					{
					m_pPoints[i].x -= m_rcSelect.left;
					m_pPoints[i].y -= m_rcSelect.top;
					}
				m_nSelect = 2;
				Invalidate();
				}
			}
		if (m_nSelect == 1)
			m_nSelect = 2;
		ReleaseCapture();
		return;
		}
	
	if (m_nColorTrap == 5)
		{
		bIgnore = TRUE;
		XlatePoint(point);
		ReleaseCapture();
		m_nColorTrap = 0;
		m_pFrame->ColorTrap(point.x , m_sheight - 1 - point.y,2);
		return;
		}
	if (m_CamState)
		{
		XlatePoint(point);
		CameraStop(point);
		bIgnore = TRUE;
		}
	if (m_pFloat)
		{
		m_pFloat->ShiftOn(0);
		bIgnore = TRUE;
		}
	if (m_Bret == 3 || bIgnore)
		{
		ReleaseCapture();
		NoScrub();
		return;
		}
	CPoint opoint = point;
	CClientDC dc(this);
	OnPrepareDC(&dc);  // set up mapping mode and viewport origin
	dc.DPtoLP(&point);
point.x -= m_offx;
point.y -= m_offy;
	point.x *= m_nDen;
	point.y *= m_nDen;
	CPoint zpoint = point;
	point.x /= m_nNum;
	point.y /= m_nNum;
	if (m_Bret == 0)
		{
		if ((m_wToolType != TOOL_HAND) && (m_wToolType != TOOL_ZOOM) &&
				(m_wToolType != TOOL_EYEDROP) && (m_wToolType != TOOL_WAND))
			{
			MyCursor(0,0);//dc.m_hDC);
			if (m_bTablet && (m_toolflags & 8))
				m_pBrush->SetDensity(0);
			if (m_bTablet && (m_toolflags & 4))
				m_pBrush->SetRadius(0);
			CStopDraw(zpoint);
			m_bTablet = FALSE;
			m_cursr = m_radius;//20;
			XlatePoint(opoint);
			m_cursx = opoint.x;
			m_cursy = opoint.y;
			MyCursor(1, 0);//dc.m_hDC);
			}
		else if ((m_wToolType == TOOL_EYEDROP) && (m_nToolSaved == 4))
			{
			m_nToolSaved = 0;
			UINT Index 	= m_pDoc->Color();
			OnTool(ToolId(m_wToolType = m_wSaveTool));
			m_pDoc->Color(Index);
			}
		}
	else
		FStopDraw(point);
	ReleaseCapture();   // Release the mouse capture established at
	if ((m_Bret == 1) || (m_Bret == 2))
		{
		m_Frame = m_BretFrame;
		m_Level = m_BretLevel;
		m_Bret = 0;
		bRedraw = 1;
		}
	if (bRedraw)
		Redraw();
}

void CSketchView::OnLButtonUp(UINT, CPoint point)
{
	StopDraw(point);
}

void CSketchView::OnRButtonUp(UINT, CPoint point)
{
//#ifndef FLIPBOOK_MAC
//	if (m_bDown) return;
//#endif
	StopDraw(point);
}



void CSketchView::ChangeTool()
{
	DPF("change tool");
	MyCursor(0);
	m_nSelect = 0;
	BlowFloat();
	m_toolflags = m_pDoc->ToolInfo(5);
	m_cursr = m_radius = m_maxradius = m_pDoc->Radius() - 1;
//	m_pBrush->SetRadius((m_toolflags & 4) ? 0 : m_radius);
	m_density = m_maxdensity = m_pDoc->Density();
//	m_pBrush->SetDensity((m_toolflags & 8) ? 0 : m_density);
ASSERT(m_cursr < 100); 
	Invalidate();
	MyCursor(1);
}

void CSketchView::MyCursor(BOOL bOn, HDC hDC /* = 0 */)
{
	return;
#ifdef FLIPBOOK_MAC
	return;
#endif
	if (m_wToolType >= TOOL_FILL)
		return;
	if (m_Bret)
		return;
DPF("my cursor,r:%d,old:%d",m_cursr,m_ocursr);
	if (bOn)
		{
		if (++m_curcnt != 0)
			{
			return;
			}
		}
	else
		{
		if (m_curcnt--)
			{
			return;
			}
		}
	BOOL bToggle = 0;
	if (bOn)
		bToggle = m_bOldCursor;
	else
		{
		if (m_bCursor)	// is it on
			{
			m_bOldCursor = 1;	// remember it was on
			bToggle = 1;
			}
		}
//DPF("mycurs:%d,%d,%d",bToggle,m_ocursr,m_cursr);
	if (bToggle)
		{
		if (bOn)
			{
			if (!m_cursr)
				{
				m_ocursr = 0;
				m_bCursor ^= 1;
				return;
				}
			m_ocursr = m_cursr;
			m_ocursx = m_cursx;
			m_ocursy = m_cursy;
			}
		else if (!m_ocursr)
			{
			m_bCursor ^= 1;
			return;
			}
		int x,y;
		UINT extra;
		BYTE * pBits = m_pBrush->CursorBits(extra,m_ocursr);
		LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)pBits;
		int w = lpBI->biWidth;
		int h = lpBI->biHeight;
		x = m_ocursx - w / 2;
		y = m_ocursy - h / 2;
		pBits += extra;
		RECT dst, src;
		src.left = x;
		src.top = y;
		src.right = x + w;
		src.bottom = y + h;
		XlateRect(dst,src,0);
		x = dst.left;
		y = dst.top;
		int ww = dst.right - dst.left;
		int hh = dst.bottom - dst.top;
		BOOL bSuccess;
		if (hDC)
			{
			bSuccess = ::StretchDIBits(hDC,x,y,ww,hh,0,0,w,h,
						  pBits, (LPBITMAPINFO)lpBI,DIB_RGB_COLORS,SRCINVERT);
			}
		else
			{
			CClientDC dc(this);
			bSuccess = ::StretchDIBits(dc.m_hDC,x,y,ww,hh,0,0,w,h,
					  pBits, (LPBITMAPINFO)lpBI,DIB_RGB_COLORS,SRCINVERT);
			}
		m_bCursor ^= 1;
		}
}

LRESULT CSketchView::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
	m_bInClient = FALSE;
	MyCursor(0);
	DPF("mouse leave,%d,%d,%d",m_curcnt, m_cursr,m_ocursr);
	return 0;
}

#define PROX 4

UINT CSketchView::SelectionHit(CPoint point)
{
DPF("sel hit:%d,%d,%d,%d",m_rcSelect.left,m_rcSelect.top,point.x,point.y);
	if ((point.x + PROX) < m_rcSelect.left)
		return 0;
	if ((point.y + PROX) < m_rcSelect.top)
		return 0;
	if ((point.x -PROX) >= (m_rcSelect.right))
		return 0;
	if ((point.y -PROX) >= (m_rcSelect.bottom))
		return 0;
	if ((point.y - PROX) < m_rcSelect.top)
		{
		if ((point.x - PROX) < m_rcSelect.left)
			return 1;
		else if ((point.x + PROX) >= (m_rcSelect.right))
			return 3;
		else
			return 2;
		}
	if ((point.y + PROX) >= (m_rcSelect.bottom))
		{
		if ((point.x - PROX) < m_rcSelect.left)
			return 7;
		else if ((point.x + PROX) >= (m_rcSelect.right))
			return 5;
		else
			return 6;
		}
	if ((point.x - PROX) < m_rcSelect.left)
		return 8;
	else if ((point.x + PROX) >= (m_rcSelect.right))
		return 4;
	return 9;
}

void CSketchView::UpdateCursor()
{
	CDocCursor();
}

UINT CSketchView::SelectionCursor(CPoint point)
{
	UINT code = SelectionHit(point);
	DPF("select hit:%d",code);
	LPSTR idd;
	switch(code) {
	case 1:
	case 5:
		idd = IDC_SIZENWSE;
		break;
	case 2:
	case 6:
		idd = IDC_SIZENS;
		break;
	case 3:
	case 7:
		idd = IDC_SIZENESW;
		break;

	case 4:
	case 8:
		idd = IDC_SIZEWE;
		break;
	case 9:
		idd = IDC_SIZEALL;
		break;
	default:
	//	ASSERT(0);
	//	if (m_bLassoing)
	//		{
	//		SetCursor(AfxGetApp()->LoadCursor(IDC_MYLASSO));
	//		return code;
	//		}
		idd = IDC_ARROW;
		break;
	}
	SetCursor(AfxGetApp()->LoadStandardCursor(idd));
	return code;
}


void CSketchView::AdjustSelection(CPoint point)
{
	if ((m_nSelect != 10) && (GetKeyState(VK_MENU) < 0))
		{
		int rad = (point.x - m_SelectStart.x) *
				(point.x - m_SelectStart.x) +
				(point.y - m_SelectStart.y) *
				(point.y - m_SelectStart.y);
		rad = (int)sqrt((double)rad);
		m_rcSelect.left = m_SelectStart.x - rad;
		m_rcSelect.top = m_SelectStart.y - rad;
		point.x = m_SelectStart.x + rad;
		point.y = m_SelectStart.y + rad;
		}
	else if ((m_nSelect != 10) && (GetKeyState(VK_SHIFT) < 0))
		{
		if (abs(point.x - m_rcSelect.left) >
				abs(point.y - m_rcSelect.top))
			{
			if (point.x < m_rcSelect.left)
				point.x = m_rcSelect.left -
							abs(point.y - m_rcSelect.top);
			else
				point.x = m_rcSelect.left +
							abs(point.y - m_rcSelect.top);
			}
		else
			{
			if (point.y < m_rcSelect.top)
				point.y = m_rcSelect.top -
							abs(point.x - m_rcSelect.left);
			else
				point.y = m_rcSelect.top +
							abs(point.x - m_rcSelect.left);
			}
		}
	m_rcSelect.right = point.x;
	m_rcSelect.bottom = point.y;
ASSERT(m_rcSelect.left >= 0);
}

UINT CSketchView::DragSelection(CPoint point)
{
//ASSERT(m_rcSelect.left >= 0);
	if (m_nSelect > 12)
		{
		if (m_nSelect == 13)
			m_Curve4 = point;
		else if (m_nSelect == 14)
			m_Curve2 = point;
		else if (m_nSelect == 15)
			m_Curve3 = point;
		return 0;
		}
	if ((m_nSelect == 1) && m_bLassoing)
		{
		if (!(m_nPoints % 500))
			{
			if (!m_nPoints)
				{
				m_pPoints = new CPoint[500];
				m_pPoints[m_nPoints++] = point; // first, extra
				}
			else
				{
				CPoint * tp = m_pPoints;
				m_pPoints = new CPoint[m_nPoints + 500];
				UINT i;
				for (i = 0; i < m_nPoints;i++)
					m_pPoints[i] = tp[i];
				delete [] tp;
				}
			}
		ASSERT(m_nPoints > 0);
		m_pPoints[m_nPoints - 1] = point;		// append new point
		m_pPoints[m_nPoints++] = m_pPoints[0];	// so it is closed

		if (point.x < m_rcSelect.left)
			m_rcSelect.left = point.x;
		if (point.x >= m_rcSelect.right)
			m_rcSelect.right = point.x+1;
		if (point.y < m_rcSelect.top)
			m_rcSelect.top = point.y;
		if (point.y >= m_rcSelect.bottom)
			m_rcSelect.bottom = point.y+1;		// update bounding box
//ASSERT(m_rcSelect.left >= 0);
		return 0;
		}
	if ((m_nSelect == 1) || (m_nSelect > 8))
		{
		if ((m_nSelect >= 10) && (m_nSelect <= 12))
			{
			AdjustSelection(point);
			return 0;
			}
		if (point.x > m_SelectStart.x)
			{
			m_rcSelect.left = m_SelectStart.x;
			m_rcSelect.right = point.x;
			}
		else
			{
			m_rcSelect.right = m_SelectStart.x;
			m_rcSelect.left = point.x;
			}
		if (point.y > m_SelectStart.y)
			{
			m_rcSelect.top = m_SelectStart.y;
			m_rcSelect.bottom = point.y;
			}
		else
			{
			m_rcSelect.bottom = m_SelectStart.y;
			m_rcSelect.top = point.y;
			}
/*
		if (point.x > m_rcSelect.left)
			m_rcSelect.right = point.x;
		else
			m_rcSelect.left = point.x;
		if (point.y > m_rcSelect.top)
			m_rcSelect.bottom = point.y;
		else
			m_rcSelect.top = point.y;
*/
DPR("sel",&m_rcSelect);
		return 0;
		}
ASSERT(m_rcSelect.left >= 0);
	int dx = point.x - m_SelectStart.x;
	int dy = point.y - m_SelectStart.y;
	m_SelectStart = point;
	switch (m_nSelectDrag) {
	case 1:
		m_rcSelect.left += dx;
		m_rcSelect.top += dy;
		break;
	case 2:
		m_rcSelect.top += dy;
		break;
	case 3:
		m_rcSelect.right += dx;
		m_rcSelect.top += dy;
		break;
	case 4:
		m_rcSelect.right += dx;
		break;
	case 5:
		m_rcSelect.right += dx;
		m_rcSelect.bottom += dy;
		break;
	case 6:
		m_rcSelect.bottom += dy;
		break;
	case 7:
		m_rcSelect.left += dx;
		m_rcSelect.bottom += dy;
		break;
	case 8:
		m_rcSelect.left += dx;
		break;
	case 9:
		m_rcSelect.left += dx;
		m_rcSelect.top += dy;
		m_rcSelect.right += dx;
		m_rcSelect.bottom += dy;
		break;
	default:
		break;
	}
		return 0;
}

void CSketchView::FloatCursor(CPoint point)
{
	BOOL bRotate = GetKeyState(VK_CONTROL) < 0 ? 1 : 0;
	BOOL bShift = GetKeyState(VK_SHIFT) < 0 ? 1 : 0;
	UINT code = m_pFloat->Hit(point, bShift, bRotate);
	DPF("float curs,hit:%d,%d,%d,%d",code,point.x,point.y,bRotate);
	LPSTR idd;
	UINT cid = 0;
	switch(code) {
	case 1:
	case 5:
		idd = IDC_SIZENWSE;
		break;
	case 2:
	case 6:
		idd = IDC_SIZENS;
		break;
	case 3:
	case 7:
		idd = IDC_SIZENESW;
		break;
	case 4:
	case 8:
		idd = IDC_SIZEWE;
		break;
	case 24:
	case 9:
		idd = IDC_SIZEALL;
		break;
	case 20:
	case 21:
	case 22:
	case 23:
		cid = IDC_ROTATE_CUR;
		break;
	default:
	//	ASSERT(0);
		idd = IDC_ARROW;
		break;
	}
	if (cid)
		SetCursor(AfxGetApp()->LoadCursor(cid));
	else
		SetCursor(AfxGetApp()->LoadStandardCursor(idd));
}

void CSketchView::BlowFloat()
{
	if (!m_pFloat)
		return;
	RECT rc,dst;
	m_pFloat->GetRect(rc);
	XlateRect(dst,rc,0);
	delete m_pFloat;
	m_pFloat = 0;
	InvalidateRect(&dst,0);
}

void CSketchView::DrawFloat()
{
	if (!m_pFloat)
		return;
	RECT rc,dst;
	m_pFloat->GetRect(rc);
	XlateRect(dst,rc,0);
	InvalidateRect(&dst,0);
}

void CSketchView::OnMouseMove(UINT, CPoint point)
{
	CPoint zpoint;
	if (!m_bInClient)
		{
#ifndef FLIPBOOK_MAC
		m_curcnt = -1;
		MyCursor(1);
		TRACKMOUSEEVENT tme;
		tme.cbSize      = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags     = TME_LEAVE;
		tme.hwndTrack   = m_hWnd;
		_TrackMouseEvent(&tme);
		m_bInClient = TRUE;
#endif
		}

#ifndef FLIPBOOK_MAC
	UINT extraInfo = ::GetMessageExtraInfo();
//	DPF("extra:%8x",extraInfo);
#endif
	if (GetCapture() != this)
		{
//		DPF("nsmove,%d,%d,%d,r:%d",point.x,point.y,m_cursr,m_radius);
		if (m_CamState)
		{
			CameraMove(point);
			return;
		}
		if (m_nSelect && !m_Bret)
			{
			XlatePoint(point);
			SelectionCursor(point);
			return;
			}
		if (m_bShiftTrace)
			{
			XlatePoint(point);
			m_pShiftTrace->Cursor(point);
			return;
			}
		if (m_pFloat)
			{
			XlatePoint(point);
			FloatCursor(point);
			return;
			}
		MyCursor(0);
//		m_radius = m_pDoc->Radius() - 1;
		m_cursr = m_radius;//20;
		XlatePoint(point);
		m_cursx = point.x;
		m_cursy = point.y;
		MyCursor(1);
		return;
		}
//DPF("mouse move:%d",m_nSelect);
	if (m_bShiftTrace)
		{
		XlatePoint(point);
		if (point != m_ptPrev)
			{
			DPF("(%d:%d),(%d:%d)",m_ptPrev.x,m_ptPrev.y,point.x,point.y);
			m_ptPrev = point;
			m_pShiftTrace->Drag(point);
			Redraw();
//			Invalidate();
			}
		return;
		}
	if (m_nSelect)
		{
//ASSERT(m_rcSelect.left >= 0);
		XlatePoint(point);
		if (point != m_ptPrev)
			{
				DPF("(%d:%d),(%d:%d)",m_ptPrev.x,m_ptPrev.y,point.x,point.y);
			m_ptPrev = point;
//ASSERT(m_rcSelect.left >= 0);
			DragSelection(point);
			Invalidate();
			}
		return;
		}
	if (m_nColorTrap)
		return;
	if (m_CamState)
		{
		CameraMove(point);
		return;
		}
	if (m_pFloat)
		{
		XlatePoint(point);
		DrawFloat();
		m_pFloat->DragIt(point);//, (GetKeyState(VK_SHIFT) < 0) ? 1 : 0);
		DrawFloat();
		return;
		}
DPF("bret:%d,ignore:%d",m_Bret,m_bIgnore);
	if (m_Bret > 3) return;
	if (m_bIgnore) return;
	CPoint opoint = point;
	CClientDC dc(this);
	OnPrepareDC(&dc);
	dc.DPtoLP(&point);
point.x -= m_offx;
point.y -= m_offy;
	point.x *= m_nDen;
	point.y *= m_nDen;
	zpoint = point;	// save in zoom coordinates
	point.y /= m_nNum;
	point.x /= m_nNum;


	if (m_Bret == 0)
		{
		if (m_wToolType == TOOL_HAND)
			{
			int dx,dy;
			dx = point.x - (int)m_origx;
			dy = point.y - (int)m_origy;
			CPoint pt1 = GetScrollPosition();
			pt1.x -= dx;
//			if (pt1.x < 0) {pt1.x = 0; m_origx = point.x;}
//			if (pt1.x >= (int)m_swidth) {pt1.x = m_swidth-1; m_origx = point.x;}
			pt1.y -= dy;
//			if (pt1.y < 0) {pt1.y = 0; m_origy = point.y;}
//			if (pt1.y > (int)m_sheight) {pt1.y = m_sheight-1; m_origy = point.y;}
			ScrollToPosition(pt1);
			CPoint pt2 = GetScrollPosition();
DPF("pt1,%d,%d,,pt2,%d,%d",pt1.x,pt1.y,pt2.x,pt2.y);
			if (pt1.x != pt2.x) m_origx = point.x;
			if (pt1.y != pt2.y) m_origy = point.y;
			}
		else if ((m_wToolType != TOOL_ZOOM) &&
				(m_wToolType != TOOL_WAND))
			{
//DPF("curcnt:%d",m_curcnt);
			MyCursor(0,0);//dc.m_hDC);
			if (m_bTablet && (m_toolflags & 8))
				m_pBrush->SetDensity((m_maxdensity * m_wPressure) / 255);
			if (m_bTablet && (m_toolflags & 4))
				m_pBrush->SetRadius((m_wPressure * m_maxradius) / 255); ///= 20;
			CMoveDraw(zpoint);
			m_cursr = m_radius;//20;
			XlatePoint(opoint);
			m_cursx = opoint.x;
			m_cursy = opoint.y;
			MyCursor(1, 0);//dc.m_hDC);
			}
		}
	else if (m_Bret <= 3)
		FMoveDraw(point);
	return;
}

void CSketchView::CamCursor()
{
	CPoint point;
	GetCursorPos(&point);
	DPF("apoint %d,%d",point.x,point.y);
	ScreenToClient(&point);
	CClientDC dc(this);
	dc.DPtoLP(&point);
	DPF("bpoint %d,%d",point.x,point.y);
	XlatePoint(point);
	DPF("cpoint %d,%d",point.x,point.y);
	int dx = point.x - m_pegx;
	int dy = point.y - m_pegy;
	if ((dx*dx + dy*dy) < 30)
		{
		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_CROSS));
		return;
		}
	if (m_CamState == 2)
		{
//	ASSERT(0);
		int t;
		if (m_CamTool & 16)
			t = IDC_NSEW_CUR;
		else
			t = IDC_NS_CUR + (m_CamTool & 15);
		SetCursor(AfxGetApp()->LoadCursor(t));
		}
	else
		SetCursor(AfxGetApp()->LoadCursor(IDC_CAM_SELECT));
}

BOOL CSketchView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
//DPF("set cursor:%d",m_Bret);
	if ((pWnd == this) && (GetFocus() != this))
		SetFocus();
	if ((pWnd == this) && !m_pFloat && m_pScene && m_pDoc && (m_Bret <= 3))
		{
		if (nHitTest == HTCLIENT)
			{
			if (m_CamState)
				CamCursor();
			else if (m_bShiftTrace)
				m_pShiftTrace->Cursor(CPoint(-1,-1));
			else if (m_nColorTrap > 1)
				SetCursor(AfxGetApp()->LoadCursor(IDC_GRAD1+ m_nColorTrap-2));
			else if (m_Bret == 0)
				CDocCursor();
			else
				FDocCursor();
			return FALSE;
			}
		}
	return CScrollView::OnSetCursor(pWnd, nHitTest, message);
}

void CSketchView::OnFunctionKey(UINT Id)
{
	BOOL bBad = FALSE;
	BOOL bLoop = m_pDoc->Option(LOOPPLAY);
	UINT Frame;
	switch (Id) {
	case ID_KEY_F1:
		CDocCursor();
		break;
	case ID_KEY_F2:
		if (!m_Bret && !m_bInModel)
			{
			if (m_pLayers->Speckle(2))
				{
				m_bModified = TRUE;
				EchoIt();
				}
			}
		else
			bBad = TRUE;
		break;
	case ID_KEY_F4:
	case ID_KEY_PGUP:
		if (m_Bret)
			{
			OnVCR(ID_VCR_BSTEP);
			break;
			}
		bBad = TRUE;
		if (m_bInModel)
			break;
		Frame = m_EditFrame;
		if (Frame--)
			bBad = !m_pScene->FindPrevCell(Frame, m_EditLevel);
		if (bBad && bLoop)
			{
			if (Frame = m_pScene->FrameCount())
				bBad = !m_pScene->FindPrevCell(Frame, m_EditLevel);
			}
		if (!bBad)
			bBad = VSelectCell(Frame, m_EditLevel);
		break;
	case ID_KEY_PGDN:
	case ID_KEY_F5:
		if (m_Bret)
			{
			OnVCR(ID_VCR_FSTEP);
			break;
			}
		bBad = TRUE;
		if (m_bInModel)
			break;
		Frame = m_EditFrame;
			Frame++;
			if (Frame < m_pScene->FrameCount())
				bBad = !m_pScene->FindNextCell(Frame, m_EditLevel);
		if (bBad && bLoop)
			{
			Frame = 0;
			bBad = !m_pScene->FindNextCell(Frame, m_EditLevel);
			}
		if (!bBad)
			bBad = VSelectCell(Frame, m_EditLevel);
		break;
	case ID_KEY_HOME:
		if (m_Bret)
			{
			OnVCR(ID_VCR_HOME);
			break;
			}
		bBad = TRUE;
		if (m_bInModel)
			break;
		Frame = 0;
		if (m_pScene->FindNextCell(Frame, m_EditLevel))
			bBad = VSelectCell(Frame, m_EditLevel);
		break;
	case ID_KEY_END:
		if (m_Bret)
			{
			OnVCR(ID_VCR_END);
			break;
			}
		bBad = TRUE;
		if (m_bInModel)
			break;
		Frame = m_pScene->FrameCount()-1;
		if (m_pScene->FindPrevCell(Frame, m_EditLevel))
			bBad = VSelectCell(Frame, m_EditLevel);
		break;
	case ID_KEY_F3:
	case ID_KEY_F6:
	default:
DPF("bad function:%d", Id);
			bBad = TRUE;
			break;
	case ID_KEY_F7:
		DupIt();
		break;
	case ID_KEY_F8:
		SendMessage(WM_COMMAND, ID_FILE_GRAB, 0);
		break;
	case ID_KEY_F9:
	case ID_KEY_F10:
	case ID_KEY_F11:
	case ID_KEY_F12:
		if (!m_Bret && !m_bInModel)
			{
			if (m_pLayers->CDespeckle(1 + Id - ID_KEY_F9))
				{
				m_bModified = TRUE;
				EchoIt();
				}
			}
		else
			bBad = TRUE;
		break;
	case ID_KEY_SF9:
	case ID_KEY_SF10:
	case ID_KEY_SF11:
	case ID_KEY_SF12:
		if (!m_Bret && !m_bInModel)
			{
			if (m_pLayers->UnCranny(1 + Id - ID_KEY_SF9))
				{
//				m_Undo = 1;
				m_bModified = TRUE;
				EchoIt();
				}
			}
		else
			bBad = TRUE;
		break;
	case ID_KEY_CF9:
//	case ID_KEY_CF10:
		if (!m_Bret && !m_bInModel)
			{
			if (m_pLayers->Speckle(1 + Id - ID_KEY_CF9))
				{
//				m_Undo = 1;
				m_bModified = TRUE;
				EchoIt();
				}
			}
		else
			bBad = TRUE;
		break;
	}
	if (bBad)
		MessageBeep(0);
}

void CSketchView::NewBret(UINT mode)
{
	if (!m_Bret && mode)
		{
		m_BretFrame = m_Frame;
		m_BretLevel = m_Level;
		m_Bret = mode;
		if (mode > 2)
			QSetup(TRUE);
		}
	else if (!mode && m_Bret)
		{
		m_Frame = m_BretFrame;
		m_Level = m_BretLevel;
//		if (m_Bret == 3)
//			QSetup(FALSE);
		m_Bret = 0;
		Redraw();
		}
DPF("new bret, desired:%d, act:%d", mode, m_Bret);
}
void CSketchView::CapKludge(UINT frame)
{
	if (m_Kludge)
		return;
	m_Kludge = frame;
	OnVCR(99);
}

void CSketchView::OnSpace()
{
	if (m_Bret == 3)
		{
		if (m_bPlaying)
			OnVCR(ID_VCR_STOP);
		else if (m_bDir)
			OnVCR(ID_VCR_BACK);
		else
			OnVCR(ID_VCR_PLAY);
		}
	else if ((m_Bret == 0) && !m_nToolSaved)
		{
		m_nToolSaved = 1;
		m_wSaveTool = m_wToolType;
		m_bErasing = m_pDoc->ToolInfo(0);
DPF("on space,saved:%d",m_wSaveTool);
		OnTool(ID_TOOL_HAND);
		}
//	else
//		MessageBeep(0);
}

void CSketchView::OnReturn()
{
	if (m_pScene->ColorMode() || !m_pDoc->Option(PEG_SHOWFG))
		{
		OnVCR(ID_VCR_PLAY);
		}
	else
		{
		SetupFlip();
		NewBret(4);
		m_Index = 0;
		m_CamState = 0;
		m_bDir = FALSE;
		Invalidate(FALSE);
		m_bPlaying = TRUE;
		m_play_factor = 1;
		StartTimer(TRUE);
		}
}

void CSketchView::OnBack()
{
	DPF("back, bret:%d", m_Bret);
	if (m_pScene->ColorMode() || !m_pDoc->Option(PEG_SHOWFG))
		{
		OnVCR(ID_VCR_BACK);
		}
	else
		{
		SetupFlip();
		NewBret(4);
		m_Index = m_count - 1;
		m_CamState = 0;
		m_bDir = TRUE;
		Invalidate(FALSE);
		m_bPlaying = TRUE;
		StartTimer(TRUE);
		}
}

void CSketchView::OnPositionKey(UINT Id)
{
	BOOL bBad = FALSE;
	DPF("position, bret:%d", m_Bret);
	if (m_Bret == 4)
		return;
	if (m_bPlaying)
		Id = 0;
	if ((m_Bret != 3) && (m_Bret))
		Id = 0;
	if (m_Bret == 3)
		{
		if (Id == ID_KEY_DOWN)
			Id = ID_KEY_RIGHT;
		else if (Id == ID_KEY_UP)
			Id = ID_KEY_LEFT;
		}
	if (m_pFrame->Timing())
		{
		int nChar = 0;
		if ((Id == ID_KEY_RIGHT) || (Id == ID_KEY_DOWN))
			nChar = VK_DOWN;
		else if ((Id == ID_KEY_LEFT) || (Id == ID_KEY_UP))
			nChar = VK_UP;
		if (nChar)
			m_pFrame->GridChar(nChar,1);
		return;
		}
	int l = 0;
	int f = 0;
	switch (Id) {
	case ID_KEY_UP:
		if (m_Frame)
			f = -1;
		else
			bBad = TRUE;
		break;
	case ID_KEY_DOWN:
		if ((m_Frame + 1) < m_pScene->FrameCount())
			f = 1;
		else
			bBad = TRUE;
		break;
	case ID_KEY_RIGHT:
		if (m_Bret == 3)
			{
			if ((m_Index+1) < m_count)
				{
				m_bPlaySnip = TRUE;
				m_Index++;
				Invalidate(FALSE);
				}
			else if (m_pDoc->Option(LOOPPLAY))
				{
				m_Index = 0;
				m_bPlaySnip = TRUE;
				Invalidate(FALSE);
				}
			else
				bBad = TRUE;
			break;
			}
		if (m_Level > 1)
			l = -1;
		else
			bBad = TRUE;
		break;
	case ID_KEY_LEFT:
		if (m_Bret == 3)
			{
			if (m_Index)
				{
				m_bPlaySnip = TRUE;
				m_Index--;
				Invalidate(FALSE);
				}
			else if (m_pDoc->Option(LOOPPLAY))
				{
				m_Index = m_count - 1;
				m_bPlaySnip = TRUE;
				Invalidate(FALSE);
				}
			else
				bBad = TRUE;
			break;
			}
		if ((m_Level + 1) < m_pScene->LevelCount())
			l = 1;
		else
			bBad = TRUE;
		break;
	default:
DPF("bad position key:%d", Id);
		bBad = TRUE;
		break;
	}
	if (bBad)
		MessageBeep(0);
	else if (l || f)
		VSelectCell(m_Frame+f,m_Level+l);
}

void CSketchView::OnCameraKey(UINT Id)
{
	BOOL bBad = FALSE;
	DPF("camera key");
	if (m_Bret == 4)
		return;
	if (m_CamState != 2)
		return;
	int code;
	switch (Id) {
		case ID_KEY_CUP:
			code = 5;
			break;
		case ID_KEY_CDOWN:
			code = 1;
			break;
		case ID_KEY_CRIGHT:
			code = 7;
			break;
		case ID_KEY_CLEFT:
			code = 3;
			break;
		default:
DPF("bad camera key:%d", Id);
			Id = 0;
			break;
		}
//	if ((m_Camera == 2) && (!(code & 2)))
//		Id = 0;
//	if ((m_Camera == 1) && (code & 2))
//		Id = 0;
DPF("Id:%d",Id);
	if (Id)
		m_pFrame->CameraPacket(4,code,0);
	else
		MessageBeep(0);
}

#ifdef _DEBUG
void CSketchView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CSketchView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSketchView message handlers

UINT CSketchView::ToolId(UINT which)
{
	return ID_TOOL_PENCIL + which;
}

void CSketchView::OnTool(UINT Id)
{
	int v,q;
	if (Id == ID_TOOL_CURVE)
		v = 4,q = IDC_PEN_CURVE;
	else if (Id == ID_TOOL_LINE)
		v = 1,q = IDC_PEN_LINE;
	else if (Id == ID_TOOL_FREE)
		v = 0,q = IDC_PEN_FREE;
	else 
		v = 99;
	if (v != 99)
		{
		OnTool(ID_TOOL_PENCIL);
		if (m_pFrame->ToolMessage(q))
			return;
		DPF("set pen:%d",v);
		int z = m_pDoc->ToolInfo(5);
		z |= 3 + 128;//FLAG_ERASE;
		z ^= 3 + 128;//FLAG_ERASE;
		z |= 32 * (v & 4) + (v & 3);
		m_pDoc->ToolInfo(6,z);
		return;
		}
	UINT f = 99999;
	if (m_Bret == 3)
		f = m_Frame;
	OnVCRExit();
#ifdef FBVER7
	if (f < 99999)
		{
		int x,y,scale, rot;
		if (!m_pScene->GetCamInfo(f,m_Level,x,y,scale,rot))
			{
			m_nScale = scale / 100;
			//Adjust(1000000 / scale, 100);
			Adjust(m_nNum, m_nDen);
			}
		m_pFrame->SelectCell(f, m_Level, 4);
		}
#else
	if ((f < 99999) && m_pDoc->Option(EDIT_SCRUB))
		{
		if (f != m_EditFrame)
		m_pFrame->SelectCell(f, m_Level, 4);
		}
#endif
	BOOL bRedraw = 0;
	if ((Id == ID_TOOL_BRUSH) || (Id == ID_TOOL_FILL))
		{
		if (m_pDoc->Option(PEG_SHOWFG)) // lighbox on
			{
			if ((m_pDoc->Option(SC_MRU) < 2) &&
					!m_pDoc->Option(SC_LIGHT_PAINT))
				{
				m_pDoc->Option(SC_LIGHT_PAINT,1,1);
				bRedraw = 1;
				}
			}
		}
	if (m_pFrame->Timing())
		{
		m_pFrame->Timing(0);
		bRedraw = 1;
		}
	if (bRedraw)
		Redraw();
	m_pLayers->FillStack(-2); // clear it
	if (Id == ID_TOOL_ERASER)
		{
		MyCursor(0);
		if (m_pDoc->ToolInfo(3))
			{
			m_pDoc->ToolInfo(2);	// toggle eraser
			CDocCursor();
			m_pFrame->ChangeTool();
			}
		MyCursor(1);
		return;
		}
	else if (Id == ID_TOOL_PENCIL)
		m_wToolType = TOOL_PENCIL;
	else if (Id == ID_TOOL_TRACE)
		m_wToolType = TOOL_TRACE;
	else if (Id == ID_TOOL_BRUSH)
		m_wToolType = TOOL_BRUSH;
	else if (Id == ID_TOOL_FILL)
		m_wToolType = TOOL_FILL;
	else if (Id == ID_TOOL_ZOOM)
		m_wToolType = TOOL_ZOOM;
	else if (Id == ID_TOOL_HAND)
		m_wToolType = TOOL_HAND;
	else if (Id == ID_TOOL_WAND)
		m_wToolType = TOOL_WAND;
	else if (Id == ID_TOOL_EYEDROP)
		{
		if (!m_nToolSaved)
			{
			if ((m_wToolType == TOOL_PENCIL) ||
					(m_wToolType == TOOL_BRUSH) ||
					(m_wToolType == TOOL_TRACE) ||
					(m_wToolType == TOOL_FILL))
				{
				m_nToolSaved = 4;
				m_wSaveTool = m_wToolType;
				}
			}
		m_wToolType = TOOL_EYEDROP;
		}
	else if (Id == ID_TOOL_SELECT)
		m_wToolType = TOOL_SELECT;
DPF("new tool:%d",m_wToolType);
	m_pDoc->SelectTool(m_wToolType);
	CDocCursor();
	m_pFrame->ChangeTool();
}

BOOL CSketchView::ViewActive()
{
	if (m_pDoc && m_pScene && m_pFrame && m_pDoc->DocOpened())
		return TRUE;
	else
		return FALSE;
}

void CSketchView::OnUpdateTool(CCmdUI* pCmdUI, UINT Id)
{
	BOOL bFlag = TRUE;
	if (!ViewActive())
		bFlag = FALSE;
	else if (!m_pScene->ColorMode())
		{
		if ((Id == ID_TOOL_BRUSH) ||
			(Id == ID_TOOL_FILL) ||
			(Id == ID_TOOL_TRACE) ||
			(Id == ID_TOOL_EYEDROP))
			bFlag = FALSE;
		}
/*			4/24/10 enable, if lightbox on, turn on show paint
	else
		{
		if ((m_pDoc->Option(SC_MRU) != 2) && (m_pDoc->Option(PEG_SHOWFG)))
			{
			if ((Id == ID_TOOL_BRUSH) ||
				(Id == ID_TOOL_FILL))
				bFlag = FALSE;
			}
		}
*/
	if (m_pFrame->DisPalMode())
		{
		bFlag = FALSE;
		if ((Id == ID_TOOL_WAND) || 
			(Id == ID_TOOL_HAND) || 
			(Id == ID_TOOL_ZOOM)) 
			bFlag = TRUE;
		}
	if (!m_pFrame->DisPalMode() && (Id == ID_TOOL_WAND))
		{
		bFlag = FALSE;
		}
	if (Id == ID_TOOL_ERASER)
		{
		if ((m_wToolType == TOOL_ZOOM) ||
			(m_wToolType == TOOL_HAND) ||
			(m_wToolType == TOOL_EYEDROP) ||
			(m_wToolType == TOOL_SELECT))
			bFlag = FALSE;
	//	else
	//		pCmdUI->SetCheck(TRUE);
		}
	if (!bFlag)
		pCmdUI->Enable(FALSE);
//	else
		{
		bFlag = FALSE;
		if ((Id == ID_TOOL_PENCIL) && (m_wToolType == TOOL_PENCIL))
			bFlag = TRUE;
		if ((Id == ID_TOOL_TRACE) && (m_wToolType == TOOL_TRACE))
			bFlag = TRUE;
		if ((Id == ID_TOOL_FILL) && (m_wToolType == TOOL_FILL))
			bFlag = TRUE;
		if ((Id == ID_TOOL_BRUSH) && (m_wToolType == TOOL_BRUSH))
			bFlag = TRUE;
		if ((Id == ID_TOOL_ZOOM) && (m_wToolType == TOOL_ZOOM))
			bFlag = TRUE;
		if ((Id == ID_TOOL_HAND) && (m_wToolType == TOOL_HAND))
			bFlag = TRUE;
		if ((Id == ID_TOOL_EYEDROP) && (m_wToolType == TOOL_EYEDROP))
			bFlag = TRUE;
		if ((Id == ID_TOOL_WAND) && (m_wToolType == TOOL_WAND))
			bFlag = TRUE;
		if ((Id == ID_TOOL_SELECT) && (m_wToolType == TOOL_SELECT))
			bFlag = TRUE;
		if ((Id == ID_TOOL_ERASER) && m_pDoc->ToolInfo(0))
			bFlag = TRUE;
//DPF("updating:%d,id:%d,flg:%d",m_wTool,Id,bFlag);
		pCmdUI->SetCheck(bFlag);
		}
}


void CSketchView::OnUpdateActionFill(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(ViewActive() &&
			m_pScene->ColorMode() && !m_Bret ? TRUE : FALSE);
}

void CSketchView::OnUpdateActionFlip(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(ViewActive() && !m_Bret ? TRUE : FALSE);
}

void CSketchView::OnUpdateActionSpeckle(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(ViewActive() && !m_Bret ? TRUE : FALSE);
}

void CSketchView::OnUpdateActionMatte(CCmdUI* pCmdUI)
{
#ifdef FBVER7
	pCmdUI->Enable(ViewActive() && !m_Bret && ToolKind() ? TRUE : FALSE);
#else
	pCmdUI->Enable(ViewActive() && !m_Bret && Layer() ? TRUE : FALSE);
#endif
}


void CSketchView::OnLoop()
{
	m_pDoc->Option(LOOPPLAY,2);
}

void CSketchView::OnUpdateLoop(CCmdUI* pCmdUI)
{
	if (!ViewActive())
		pCmdUI->Enable(FALSE);
	else
		pCmdUI->SetCheck(m_pDoc->Option(LOOPPLAY));
}

void CSketchView::OnEditUndo()
{
	m_pLayers->Undo();
	Redraw();
}

void CSketchView::OnUpdateUndo(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_pLayers && m_pLayers->CanUndo());
}

void CSketchView::OnEditRedo()
{
	m_pLayers->Redo();
	Redraw();
}

void CSketchView::OnUpdateRedo(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_pLayers && m_pLayers->CanRedo());
}

void CSketchView::OnEditClear()
{
	if (MyError(IDS_AREYOUSURE, MB_YESNO) == IDYES)
		{
		m_pLayers->Clear();
//SelectCell(32000,0,TRUE);
		Redraw();
		m_bModified = TRUE; 
		}
}

void CSketchView::ClearCell()
{
	m_pLayers->Clear();
	Redraw();
	KeepCell(1);
	m_bModified = FALSE; 
}

void CSketchView::OnUpdateClear(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(ViewActive());
}

void CSketchView::OnEditRevert()
{
	if (MyError(IDS_AREYOUSURE, MB_YESNO) == IDYES)
		ASelectCell(m_Frame, m_Level, TRUE);
}

void CSketchView::OnUpdateRevert(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(ViewActive());
}

void CSketchView::OnMakeModel()
{
	CString name;
	if (((CSketchApp*) AfxGetApp())->PromptFileName(name, 9))
		m_pLayers->SaveModel((LPCSTR)name);
}

void CSketchView::OnUpdateMakeModel(CCmdUI* pCmdUI)
{
	BOOL bModel = FALSE;
	if (ViewActive() && m_pLayers->CanModel())
		bModel = TRUE;
	pCmdUI->Enable(bModel);
}

void CSketchView::OnModel()
{
// check changes
	if (CheckModified(TRUE))
		return;
	if (m_bInModel)
		{
//		m_pLayers = m_pSaveLayers;
//		m_pSaveLayers = 0;
		m_bInModel = FALSE;
		ASelectCell(m_EditFrame, m_EditLevel,1);
		m_pFrame->HasModel(1);
		}
	else
		{
		m_pFrame->HasModel(0);
		m_EditFrame = m_Frame;
		m_EditLevel = m_Level;
		if (!m_pLayers->LoadModel(m_Level))
			{
			m_bInModel = TRUE;
			Redraw();
			}
		}
	
}

void CSketchView::OnUpdateModel(CCmdUI* pCmdUI)
{
	BOOL bModel = FALSE;
	if (m_bInModel)
		bModel = TRUE;		/// alow disable
	else if (ViewActive() && m_pScene->ColorMode() && m_pFrame->HasModel())
		bModel = TRUE;
	if (bModel)
		pCmdUI->SetCheck(m_bInModel);
	else
		pCmdUI->Enable(FALSE);
}

void CSketchView::OnLightBox()
{
	m_pDoc->DlgLightBox();
}

void CSketchView::OnOptions()
{
	DPF("options");
#ifdef FBVER7
	if (GetKeyState(VK_CONTROL) < 0)
		{
		m_pDoc->DlgSpecial();
		return;
		}
#endif
	UINT broad = m_pScene->Broadcast();
	m_pDoc->DlgOptions();
	if (broad != m_pScene->Broadcast())
		{
		if (m_pScene->Broadcast() == 2)
			{
			if (m_pScene->ZFactor(2) != 1)
				OnPreview(1);
			}
		}
}

void CSketchView::OnOptFG()
{
	if (!m_pDoc->Option(PEG_SHOWFG) && m_pScene->ColorMode())
		{
		if ((m_wToolType == TOOL_BRUSH) ||
				(m_wToolType == TOOL_FILL))
			{
			if (m_pDoc->Option(SC_MRU) < 2)
				m_pDoc->Option(SC_LIGHT_PAINT,1,1);
			}
		}
	m_pDoc->Option(PEG_SHOWFG,2);
}

void CSketchView::OnUpdateFG(CCmdUI* pCmdUI)
{
	if (!ViewActive())// || !m_pDoc->Option(STACK))// || m_pScene->ColorMode())
		pCmdUI->Enable(FALSE);
	else // if (pCmdUI->m_pMenu)
		pCmdUI->SetCheck(m_pDoc->Option(PEG_SHOWFG));
}

void CSketchView::OnOptBG()
{
	m_pDoc->Option(PEG_SHOWBG,2);
DPF("on opt bg");
/*
	if (m_pDoc->Option(PEG_SHOWBG,2))
		{
		UINT Frame, Level;
		UINT c = m_pFrame->PegCount();
		UINT i,j;
		j = 0;
		for (i = c; i-- > 1;)
			{
			if (m_pFrame->PegContents(Frame,Level,i))
				break;
DPF("bg contents,%d,f:%d,l:%d",i,Frame,Level);
			if (!Level)
				{
				m_pFrame->RemoveId(i);
				j++;
				}
			}
		if (j)
			Redraw();
		}
		*/
}

void CSketchView::OnUpdateBG(CCmdUI* pCmdUI)
{
	if (!ViewActive())
		pCmdUI->Enable(FALSE);
	else // if (pCmdUI->m_pMenu)
		pCmdUI->SetCheck(m_pDoc->Option(PEG_SHOWBG));
}

void CSketchView::OnOptKeep()
{
	m_pDoc->Option(AUTOKEEP,2);
}

void CSketchView::OnUpdateKeep(CCmdUI* pCmdUI)
{
	if (!ViewActive())
		pCmdUI->Enable(FALSE);
	else // if (pCmdUI->m_pMenu)
		pCmdUI->SetCheck(m_pDoc->Option(AUTOKEEP));
}

void CSketchView::OnOptSound()
{
	m_pDoc->Option(SC_QUIET,2);
}

void CSketchView::OnUpdateOptSound(CCmdUI* pCmdUI)
{
	if (!ViewActive())
		pCmdUI->Enable(FALSE);
	else // if (pCmdUI->m_pMenu)
		pCmdUI->SetCheck(m_pDoc->Option(SC_QUIET));
}

void CSketchView::OnOptStack()
{
//	m_pDoc->Option(STACK,2);
}

void CSketchView::OnUpdateStack(CCmdUI* pCmdUI)
{
//	if (!ViewActive())// || m_pScene->ColorMode())
//		pCmdUI->Enable(FALSE);
//	else if (pCmdUI->m_pMenu)
//		pCmdUI->SetCheck(m_pDoc->Option(STACK));
}

void CSketchView::OnOptThumbs()
{
	m_pDoc->Option(SHOWTHUMBS,2);
}

void CSketchView::OnUpdateThumbs(CCmdUI* pCmdUI)
{
	if (!ViewActive())
		pCmdUI->Enable(FALSE);
	else // if (pCmdUI->m_pMenu)
		pCmdUI->SetCheck(m_pDoc->Option(SHOWTHUMBS));
}

void CSketchView::OnOptComposite()
{
	BOOL bFlag = m_pScene->Flag(4);
	m_pScene->Flag(4,1,bFlag ^ 1);
}

void CSketchView::OnUpdateComposite(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_pScene && m_pScene->Flag(4) ^ 1);
}

void CSketchView::OnViewMode()
{
	if (CheckModified(TRUE))
		return;
m_bDown = TRUE;
	BOOL bColor = m_pScene->ColorMode();
	bColor ^= 1;
	if (m_pScene->ColorMode(bColor) == bColor)
		m_pFrame->ModeChange();
	else
		m_pScene->ColorMode(bColor ^ 1);
	if ((m_pDoc->Option(SC_MRU) == 2) && !bColor)
		m_pDoc->Option(SC_MRU,1,0);
	m_bDown = FALSE;
//	m_pScene->SetFactor(m_pScene->ZFactor(1) * m_nPreview);
	CSetup();
//	if (bColor && m_pDoc->Option(PEG_SHOWFG))
//		m_pDoc->Option(PEG_SHOWFG,1,0);
//	m_pDoc->Option(STACK,1,0);
	OnTool(ID_TOOL_PENCIL);
	ASelectCell(m_EditFrame, m_EditLevel,0);
	SetFocus();
}

void CSketchView::OnUpdateMode(CCmdUI* pCmdUI)
{
	BOOL bColor = ((CSketchApp*)AfxGetApp())->CanDoColor();
	if (!ViewActive() || !bColor || m_bInModel || m_pFrame->DisPalMode())
		pCmdUI->Enable(FALSE);
	else // if (pCmdUI->m_pMenu)
		pCmdUI->SetCheck(m_pScene->ColorMode());
}

void CSketchView::OnViewSheet()
{
	m_pFrame->SwitchSheet();
}

void CSketchView::OnUpdateSheet(CCmdUI* pCmdUI)
{
	if (!ViewActive() || m_pFrame->DisPalMode())
		pCmdUI->Enable(FALSE);
	else // if (pCmdUI->m_pMenu)
		pCmdUI->SetCheck(m_pFrame->SwitchSheet(FALSE));
}

void CSketchView::OnViewTiming()
{
	OnVCRExit();
	m_pFrame->Timing(2); // 2 is switch
	Redraw();
}

void CSketchView::OnUpdateTiming(CCmdUI* pCmdUI)
{
	if (!ViewActive() || m_pFrame->DisPalMode())
		pCmdUI->Enable(FALSE);
	else // if (pCmdUI->m_pMenu)
		pCmdUI->SetCheck(m_pFrame->Timing());
}

void CSketchView::OnViewPalette()
{
	m_pFrame->SwitchPalette();
}

void CSketchView::OnUpdatePalette(CCmdUI* pCmdUI)
{
	if (!ViewActive() || !m_pScene->ColorMode() || m_pFrame->DisPalMode())
		pCmdUI->Enable(FALSE);
	else
		pCmdUI->SetCheck(m_pFrame->SwitchPalette(FALSE));
}

void CSketchView::OnViewLipSync()
{
	m_pFrame->SwitchLipSync();
}

void CSketchView::OnViewPuck()
{
	m_pFrame->SwitchPuck();
}

void CSketchView::OnViewZoomer()
{
	m_pFrame->SwitchZoomer();
}

void CSketchView::OnUpdateLipSync(CCmdUI* pCmdUI)
{
//	if (!ViewActive() || !m_pFrame->LipSyncMode())
	if (!ViewActive())
		pCmdUI->Enable(FALSE);
	else
		pCmdUI->SetCheck(m_pFrame->SwitchLipSync(FALSE));
}

void CSketchView::OnUpdatePuck(CCmdUI* pCmdUI)
{
	if (!ViewActive())
		pCmdUI->Enable(FALSE);
	else
		pCmdUI->SetCheck(m_pFrame->SwitchPuck(FALSE));
}

void CSketchView::OnUpdateZoomer(CCmdUI* pCmdUI)
{
	if (!ViewActive())
		pCmdUI->Enable(FALSE);
	else
		pCmdUI->SetCheck(m_pFrame->SwitchZoomer(FALSE));
}

void CSketchView::OnViewSubPalette()
{
	m_pFrame->SwitchSubPalette();
}

void CSketchView::OnUpdateSubPalette(CCmdUI* pCmdUI)
{
	if (!ViewActive() || !m_pScene->ColorMode() || !m_pFrame->DisPalMode())
		pCmdUI->Enable(FALSE);
	else
		pCmdUI->SetCheck(m_pFrame->SwitchSubPalette(FALSE));
}

void CSketchView::OnViewCamera()
{
	m_pFrame->SwitchCamera();
}

void CSketchView::OnUpdateCamera(CCmdUI* pCmdUI)
{
	BOOL bCam = ((CSketchApp*)AfxGetApp())->CanDoCamera();
	if (!ViewActive() || !bCam || m_pFrame->DisPalMode())
		pCmdUI->Enable(FALSE);
	else // if (pCmdUI->m_pMenu)
		pCmdUI->SetCheck(m_pFrame->SwitchCamera(FALSE));
}

void CSketchView::OnViewCameraEnlarge()
{
	BOOL bBig = m_pScene->Flag(SCN_FLG_NLARGE);
	m_pScene->Flag(SCN_FLG_NLARGE,1,bBig ^ 1);
	m_pFrame->CameraPacket(5,0,0);
	PauseIt(m_Frame);
#if MAC
	Invalidate(0);
#endif
}

void CSketchView::OnUpdateCameraEnlarge(CCmdUI* pCmdUI)
{
	BOOL bCam = ((CSketchApp*)AfxGetApp())->CanDoCamera();
//	BOOL bCam = m_pFrame->SwitchCamera(FALSE);
	if (!ViewActive() || !bCam || !m_pScene->RedBox())
		pCmdUI->Enable(FALSE);
	else
		pCmdUI->SetCheck(m_pScene->Flag(SCN_FLG_NLARGE));
}

void CSketchView::OnViewCameraGrid()
{
	BOOL bGrid = m_pScene->Flag(SCN_FLG_GRID);
	m_pScene->Flag(SCN_FLG_GRID,1,bGrid ^ 1);
	m_pScene->CheckComposite(m_Frame,2);
	PauseIt(m_Frame);
#if MAC
	Invalidate(0);
#endif
}

void CSketchView::OnUpdateCameraGrid(CCmdUI* pCmdUI)
{
	BOOL bCam = ((CSketchApp*)AfxGetApp())->CanDoCamera();
//	BOOL bCam = m_pFrame->SwitchCamera(FALSE);
	if (!ViewActive() || !bCam )
		pCmdUI->Enable(FALSE);
	else
		pCmdUI->SetCheck(m_pScene->Flag(SCN_FLG_GRID));
}

void CSketchView::OnViewCameraGridCenter()
{
	BOOL bGrid = m_pScene->Flag(SCN_FLG_GRID_CTR);
	m_pScene->Flag(SCN_FLG_GRID_CTR,1,bGrid ^ 1);
	m_pScene->Flag(SCN_FLG_GRID,1,1);
	m_pScene->CheckComposite(m_Frame,2);
	PauseIt(m_Frame);
#if MAC
	Invalidate(0);
#endif
}

void CSketchView::OnUpdateCameraGridCenter(CCmdUI* pCmdUI)
{
	BOOL bCam = ((CSketchApp*)AfxGetApp())->CanDoCamera();
//	BOOL bCam = m_pFrame->SwitchCamera(FALSE);
	if (!ViewActive() || !bCam )
		pCmdUI->Enable(FALSE);
	else
		pCmdUI->SetCheck(m_pScene->Flag(SCN_FLG_GRID_CTR));
}

void CSketchView::OnViewCamera100()
{
	m_pScene->RedBox(0);
	m_pScene->CheckComposite(m_Frame,2);
	m_pFrame->CameraPacket(5,0,0);
	PauseIt(m_Frame);
#if MAC
	Invalidate(0);
#endif
}

void CSketchView::OnUpdateCamera100(CCmdUI* pCmdUI)
{
	BOOL bCam = ((CSketchApp*)AfxGetApp())->CanDoCamera();
//	BOOL bCam = m_pFrame->SwitchCamera(FALSE);
	if (!ViewActive() || !bCam)
		pCmdUI->Enable(FALSE);
	else
		pCmdUI->SetCheck(m_pScene->RedBox() ? 0 : 1);
}

void CSketchView::OnViewCamera50()
{
	m_pScene->RedBox(1);
	m_pScene->CheckComposite(m_Frame,2);
	m_pFrame->CameraPacket(5,0,0);
	PauseIt(m_Frame);
#if MAC
	Invalidate(0);
#endif
}

void CSketchView::OnUpdateCamera50(CCmdUI* pCmdUI)
{
	BOOL bCam = ((CSketchApp*)AfxGetApp())->CanDoCamera();
//	BOOL bCam = m_pFrame->SwitchCamera(FALSE);
	if (!ViewActive() || !bCam)
		pCmdUI->Enable(FALSE);
	else
		pCmdUI->SetCheck(m_pScene->RedBox() ? 1 : 0);
}

void CSketchView::OnViewCamera25()
{
}

void CSketchView::OnUpdateCamera25(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(FALSE);
}

void CSketchView::OnViewSound()
{
	m_pFrame->SwitchSound();
}

void CSketchView::OnUpdateSound(CCmdUI* pCmdUI)
{
	if (!ViewActive() || !m_pFrame->CanDoSound())
		pCmdUI->Enable(FALSE);
	else
		pCmdUI->SetCheck(m_pFrame->SwitchSound(FALSE));
}

#if MAC
void CSketchView::OnViewTools()
{
	m_pFrame->SwitchTools();
}

void CSketchView::OnUpdateTools(CCmdUI* pCmdUI)
{
	if (!ViewActive())// || m_pFrame->DisPalMode())
		pCmdUI->Enable(FALSE);
	else
		pCmdUI->SetCheck(m_pFrame->SwitchTools(FALSE));
}

void CSketchView::OnViewCamTools()
{
	m_pFrame->SwitchCamTools();
}

void CSketchView::OnUpdateCamTools(CCmdUI* pCmdUI)
{
	if (!ViewActive())// || m_pFrame->DisPalMode())
		pCmdUI->Enable(FALSE);
	else
		pCmdUI->SetCheck(m_pFrame->SwitchCamTools(FALSE));
}
#endif

void CSketchView::OnViewFull()
{
#ifndef FLIPBOOK_MAC // support this later
	if (!m_bFull)
		{
		m_pFrame->Placements(1);
//		m_pFrame->ShowWindow(SW_MINIMIZE);
//	 restore
		if (!m_pVideo)
			OnViewVideo();
    	DWORD dwModeNum = -1; // # of DEVMODEs
    	DEVMODE dvmd;
		dvmd.dmSize = sizeof(DEVMODE);
		dvmd.dmDriverExtra = 0;
   		EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dvmd);
		dvmd.dmPelsWidth = 640;
		dvmd.dmPelsHeight = 480;
		ChangeDisplaySettings(&dvmd,0);
		m_bFull = 1;
		}
	else
		{
		if (m_pVideo)
			OnViewVideo();
		ChangeDisplaySettings(0,0);
		m_pFrame->Placements(0);
//		m_pFrame->ShowWindow(SW_RESTORE);
		m_bFull = 0;
		}
#endif
}

void CSketchView::OnViewVideo()
{
	if (m_pVideo)
		{
		delete m_pVideo;
		m_pVideo = 0;
		}
	else
		{
		m_pVideo = new CVWnd;
		if (!m_pVideo->MyCreate(this, 100,100,
					m_pScene->ComW(), m_pScene->ComH()))
			{
			TRACE("Failed to create video window\n");
			delete m_pVideo;
			m_pVideo = 0;
			}
		}
}

void CSketchView::OnUpdateVideo(CCmdUI* pCmdUI)
{
	if (!ViewActive() || m_pFrame->DisPalMode())
		pCmdUI->Enable(FALSE);
	else
		pCmdUI->SetCheck(m_pVideo ? TRUE : FALSE);
}



void CSketchView::StartTimer(BOOL bStart)
{
	DPF("starting timer,start:%d",bStart);
	if (m_timer)
		m_timer->StopTimer();
	m_rate = 0;
	if (bStart)
		{
		if (!m_timer)
			m_timer = new CMMTimer(0);
		UINT rate = m_rate = m_play_factor * m_pScene->FrameRate();
		if (rate < 1) rate = 0;
		if (rate > 100) rate = 100;
		rate = (1000 + rate - 1) / rate;
		m_timer->StartTimer(this, rate,0);
	DPF("starting timer,delay:%d",rate);
		}
	else
		m_bPlaying = FALSE;

#if MAC
	CWnd* stop_button = GetDlgItem (ID_VCR_STOP);
	CWnd* play_button = GetDlgItem (ID_VCR_PLAY);
	if (stop_button && play_button) {
		stop_button->ShowWindow (bStart == TRUE ? SW_SHOW : SW_HIDE);
		play_button->ShowWindow (bStart != TRUE ? SW_SHOW : SW_HIDE);
	}
#endif
}

UINT CSketchView::VSelectCell(UINT Frame, UINT Level)
{
DPF("vselecting:%d,%d",Frame, Level);
//	if ((Frame == m_Frame) && (Level == m_Level))
//		return 0;
	m_pFrame->SelectCell(Frame, Level, 3);
	ASelectCell(Frame, Level,0);
	return 0;
}

UINT CSketchView::Update(UINT Frame, UINT Level, UINT Flags /* = 0 */)
{
DPF("view update,f:%d,l:%d,flg:%d",Frame,Level,Flags);
	if (!m_pScene) return 0;
	if (m_Frame > m_pScene->FrameCount())
		m_Frame = m_pScene->FrameCount()-1;
	if (Flags == 10)
		{
//		m_pScene->CheckComposite(Frame,1);
		Invalidate(0);
		UpdateWindow();
		return 0;
		}
	if (Flags == 30)
		{
		m_pFrame->RedrawGrid(Frame,Level);
		return 0;
		}
	if (Flags & 8)
		{
//		m_Frame = Frame;
		NewBret(3);
		m_Index = Frame;
//		if (Flags & 1)
//			m_pScene->CheckComposite(m_Index = Frame,1,1);
		if (!(Flags & 1))
			m_bPlaySnip = TRUE;
		Invalidate(0);
		return 0;
		}
	if (Flags & 1)
		{
		if (m_Bret == 0)
			{
			m_Frame = Frame;
			Progress(999,m_Frame+1);
			}
		else if (m_pScene->CheckComposite(m_Frame))
			{
			Invalidate(FALSE);
			}
		return 0;
		}
	if (Flags == 2)
		{
		if (Frame == m_EditFrame)
			ASelectCell(m_Frame, m_Level, TRUE);
		return 0;
		}
	if (Flags & 2)
		{
		if (Frame != m_EditFrame)
			return 0;
		if (!Level)
			{
//			m_pScene->GetBackground(m_pBG, Frame,m_pDoc->Option(BGMIN));
			Redraw();
			}
		return 0;
		}
	if (Flags & 4)
		{
		if ((Level == m_EditLevel) && (Flags & 32))
			{
			m_pLayers->NewPalette();
			}
		if ((Level == m_EditLevel) || (Flags & 16))
			{
//			m_pScene->CheckComposite(m_Frame);
			Redraw();
//			m_pFrame->UpdateBar(1);
			if (Flags == 20)
				m_pFrame->UpdateTools();
			}
		return 0;
		}
	m_Frame = Frame;
	m_Level = Level;
	Invalidate(FALSE);
	return 0;
}

UINT CSketchView::CheckUpdate()
{
	if (!m_Bret) return 0;
	if (m_pScene->CheckComposite(m_Frame))
		{
		Invalidate(FALSE);
		}
	return 0;
}

class CKeepLinkDlg : public CMyDialog
{
public:
	CKeepLinkDlg(int Id);
protected:
	afx_msg void OnYes() { EndDialog(IDYES);};
	afx_msg void OnNo() { EndDialog(IDNO);};
	afx_msg void OnIgnore() { EndDialog(IDIGNORE);};
	DECLARE_MESSAGE_MAP()
};

CKeepLinkDlg::CKeepLinkDlg(int Id) : CMyDialog(Id)
{
}

BEGIN_MESSAGE_MAP(CKeepLinkDlg, CMyDialog)
	ON_BN_CLICKED(IDYES, OnYes)
	ON_BN_CLICKED(IDNO, OnNo)
	ON_BN_CLICKED(IDIGNORE, OnIgnore)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

UINT CSketchView::ASelectCell(UINT Frame, UINT Level,
			BOOL bBlow, BOOL bCanCancel /* = 0 */)
{
DPF("select cell,camstate:%d",m_CamState);
	if (!m_pFrame->Timing())
	{
	if (m_CamState)
		m_pFrame->CameraPacket(9,0,0);
//	m_pScene->CheckComposite(m_EditFrame,1,1);	// flush old from cache
//	if ((Frame == m_Frame) && (Level == m_Level))
//		return 0;
	if (bBlow)
		m_bModified = 0;
	else
		OnVCRExit();
	if (CheckModified(bCanCancel))
		return 1;
	m_pFrame->StackInit(Frame,Level);
	}
	m_nFactor = m_pScene->ZFactor(1);
#ifdef FBVER7
	UINT w,h,d;
	UINT oldw = m_swidth;
	UINT oldh = m_sheight;
	if (!Level)
		w = Level; // for debugging
	m_pLayers->GetInfo(m_pScene,Frame,Level,w, h, d);
	//	{
		UINT p = 4 * ((d * w + 31) / 32);
		if ((w != m_swidth) || (h != m_sheight) || (p != m_spitch))
			{
			m_swidth = w;
			m_sheight = h;
			CSetup(1);
			}
	//	}
	UINT result = CSelect(Frame, Level);
	if  ((oldw != m_swidth) || (oldh != m_sheight))
		{
		Adjust(m_nDen,m_nNum);
		Redraw();
		}
#else
	UINT result = CSelect(Frame, Level);
#endif
	m_curcnt = 1;
	m_cursr = m_radius = m_maxradius = m_pDoc->Radius() - 1;
	m_density = m_maxdensity = m_pDoc->Density();
//	m_pBrush->SetDensity(m_toolflags & 8 ? 0 : m_density);
//	MyCursor(1);
	return result;
}

void CSketchView::Redraw(int flags)
{
DPF("redraw");
	m_nSelect = 0;
	if (!m_pScene)
		return;
	if (flags)
		{
		m_EditFrame = m_pFrame->StackFrame();
		m_EditLevel = m_pFrame->StackLevel();
		m_pLayers->ChangeFrame(m_EditFrame);
		if (flags == 2)
			{
			ASelectCell(m_EditFrame, m_EditLevel, 1);
			}
		}
	if (m_Bret == 0)
		CRedraw();
	else if (m_Bret < 3)
		FRedraw();
}
BOOL CSketchView::CheckModified(BOOL bCanCancel)
{
	BOOL bResult = FALSE;
	QSetup(FALSE);
	UINT key;
	if (Modified())
		{
		BOOL bNew = FALSE;
		int answer;
		if (m_bInModel)
			{
			if (m_pDoc->Option(AUTOKEEP))
				answer = IDYES;
			else if (bCanCancel)
				answer = AfxMessageBox(IDS_PROMPT_KEEP_MODEL, MB_YESNOCANCEL);
			else
				answer = AfxMessageBox(IDS_PROMPT_KEEP_MODEL, MB_YESNO);
			}
		else
			{
			key = m_pScene->GetCellKey(m_EditFrame,m_EditLevel);
//ASSERT(key != 0);
			if (m_pScene->IsLinked(key))
				{
//DPF("cnt:%d",cnt);
				int idd;
#ifdef FBVER7
				if (m_bConverted)
					idd = IDD_KEEP_LINK3;
				else
#endif
					idd = bCanCancel ? IDD_KEEP_LINK2 : IDD_KEEP_LINK1;
				CKeepLinkDlg linkDlg(idd);
				answer = linkDlg.DoModal();
				if (answer == IDNO)
					{
					m_pScene->DeleteCell(m_EditFrame, m_EditLevel);
					bNew = TRUE;
					answer = IDYES;
					}
				}
			else if (m_pDoc->Option(AUTOKEEP))
				answer = IDYES;
			else if (bCanCancel)
				answer = AfxMessageBox(IDS_PROMPT_KEEP_FRAME, MB_YESNOCANCEL);
			else
				answer = AfxMessageBox(IDS_PROMPT_KEEP_FRAME, MB_YESNO);
			}
		if (answer == IDCANCEL)
			bResult = TRUE;
		if (answer == IDYES)
			{
			if (m_bInModel)
				{
				m_pLayers->SaveModel();
				}
			else
				{
				KeepCell(bNew);
				if (m_pScene)
					m_pDoc->SetModifiedFlag(m_pScene->Modified());
				
				}
			}
		if (answer == IDNO)
			{
			if (m_pScene)
				{
				m_pScene->BlowCell(m_EditFrame, m_EditLevel);
				m_pScene->UpdateCache(m_EditFrame, m_EditLevel);
				m_pDoc->SetModifiedFlag(m_pScene->Modified());
				}
			}
		}
	CSketchApp* pApp = (CSketchApp *)AfxGetApp();
	if (!pApp->USBCheck())
		{
		return TRUE;
		}
	return bResult;
}

void CSketchView::OnVCRExit()
{
DPF("vcr exit:%d",m_Bret);
	if (m_CamState)
		m_pFrame->CameraPacket(9,0,0);
	if (m_Bret == 3)
		{
		if (m_bPlaying)
			{
			m_pFrame->PlaySnd();
			m_bPlaying = FALSE;
			StartTimer();
			}
		NewBret(0);
		}
}

void CSketchView::PauseIt(UINT frame)
{
	StartTimer(0);
	if (!m_Bret)
		m_Frame = frame;
	else if (!m_bPlaying)
		m_BretFrame = frame;
	OnVCR(ID_VCR_STOP);
}

void CSketchView::OnVCR(UINT Id)
{
	UINT start = m_pScene->Selection_Start();
	UINT stop = m_pScene->Selection_Stop();
	UINT Flag = 0;
	m_CamState = 0;
	SetupScrub(FALSE);
	if (m_pFrame->Timing())
		{
		m_pFrame->Timing(0);
		Redraw();
		}
	int old_factor = m_play_factor;
	m_play_factor = 1;
	switch (Id) {
	case 99:
		m_Index = m_Kludge;
		m_Kludge = 1 + m_count;
		m_count = m_Index;
		if (m_Index > 23)
			m_Index -= 24;
		else
			m_Index = 0;
		m_pFrame->PlaySnd(m_Index,1);
		m_bDir = FALSE;
		m_bPlaying = TRUE;
		break;
	case ID_VCR_BACK:
		m_pFrame->PlaySnd();
		if (m_Index == start)
			m_Index = stop;
		m_bDir = TRUE;
		m_bPlaying = TRUE;
		break;
	case ID_VCR_END:
	case ID_VCR_HOME:
	case ID_VCR_STOP:
		if (Id == ID_VCR_END)
			m_Index = stop;
		else if (Id == ID_VCR_HOME)
			m_Index = start;
		else if (!m_Bret)
			m_Index = m_Frame;
		else if (!m_bPlaying)
			m_Index = m_BretFrame;
		m_pFrame->PlaySnd();
		m_bPlaying = FALSE;
		break;
	case ID_VCR_SLIDER_STOP:
		if (m_bPlaying)
			{
//			m_Index = m_BretFrame;
			m_pFrame->PlaySnd();
			m_bPlaying = FALSE;
			}
		break;
	case ID_VCR_PLAY:
		if ((m_Index + 1) >= stop)
			m_Index = start;
		m_pFrame->PlaySnd(m_Index,1);
		m_bDir = FALSE;
		m_bPlaying = TRUE;
		break;
	case ID_VCR_BSTEP:
	case ID_VCR_FSTEP:
		if (m_bPlaying)
			{
			if (Id == ID_VCR_BSTEP)
				{
				if (!m_bDir)
					{
					if (old_factor > 1)
						m_play_factor = old_factor - 1;
					else
						m_bDir = 1;
					}
				else if (old_factor < 4)
					{
					m_play_factor = old_factor + 1;
					StartTimer(1);
					}
				else
					m_play_factor = old_factor;
				}
			else
				{
				if (m_bDir)
					{
					if (old_factor > 1)
						m_play_factor = old_factor - 1;
					else
						m_bDir = 0;
					}
				else if (old_factor < 4)
					{
					m_play_factor = old_factor + 1;
					StartTimer(1);
					}
				else
					m_play_factor = old_factor;
				}
			break;
			}
		if (!m_Bret)
			m_Index = m_Frame;
		else
			{
//			if (!m_bPlaying)
//				m_Index = m_BretFrame;
			m_pFrame->PlaySnd();
			}
		m_bPlaying = FALSE;
		if (Id == ID_VCR_BSTEP)
			{
			if (m_Index > start)
				m_Index--;
			else if (m_pDoc->Option(LOOPPLAY))
				m_Index = stop;
			else
				Flag = 2;
			}
		if (Id == ID_VCR_FSTEP)
			{
			if (m_Index < stop)
				m_Index++;
			else if (m_pDoc->Option(LOOPPLAY))
				m_Index = start;
			else
				Flag = 2;
			}
		break;
	default:
		Flag = 2;
		DPF("bad Onvcr:%d",Id);
		break;
	}
	if (m_bPlaying)
		m_CamState = 0;
	if (Flag == 0)
		{
		NewBret(3);
#ifdef FBVER7
		Adjust(m_nNum, m_nDen);
#endif
		if (m_bPlaying)
			StartTimer(TRUE);
		else
			{
//			m_pScene->CheckComposite(m_Index,1);
			FRedraw();
			}
		}
	else
		MessageBeep(0);
}

void CSketchView::OnUpdateVCR(CCmdUI* pCmdUI, UINT Id)
{
	BOOL bFlag = TRUE;
	if (!m_pFrame)
		bFlag = FALSE;
//	UINT mode = DocMode();
//	if (mode == 1)
//		bFlag = m_pCanvas->ActionAllowed(Id);
	pCmdUI->Enable(bFlag);
}

void CSketchView::OnRate(UINT Id)
{
	if (!m_Bret)
		{
		MessageBeep(0);
		return;
		}
	UINT rate = m_rate = m_pScene->FrameRate();
DPF("old rate:%d",rate);
	if ((Id == ID_RATE_UP) && (rate < 100))
		rate++;
	if ((Id == ID_RATE_DOWN) && (rate > 1))
		rate--;
	if (rate != m_rate)
		{
DPF("new rate:%d",rate);
		m_pFrame->SetFrameRate(rate);
		}
	else
		MessageBeep(0);
}

void CSketchView::OnUpdateRate(CCmdUI* pCmdUI, UINT Id)
{
	BOOL bFlag = TRUE;
	if (!m_pFrame || !m_Bret)
		bFlag = FALSE;
	pCmdUI->Enable(bFlag);
}

void CSketchView::OnUpdateScan(CCmdUI* pCmdUI)
{
	BOOL bFlag = FALSE;
	if (ViewActive())
		bFlag = TRUE;
	pCmdUI->Enable(bFlag);
}

void CSketchView::OnUpdateCapture(CCmdUI* pCmdUI)
{
	BOOL bFlag = FALSE;
	if (ViewActive())
		bFlag = TRUE;
	pCmdUI->Enable(bFlag);
}

void CSketchView::OnUpdateProperties(CCmdUI* pCmdUI)
{
	BOOL bFlag = FALSE;
	if (ViewActive() && m_pScene)
		bFlag = TRUE;
	pCmdUI->Enable(bFlag);
}

void CSketchView::OnZoom(UINT num, UINT denom)
{
	if (!num && !denom)
		{
		m_bFit ^= 1;
		if (m_pScene)
			{
			if (m_bFit)
				{
				m_ZoomSave = m_pScene->Zoom();
				CRect rect;
				GetClientRect(&rect);
				int cw = rect.right - rect.left;
				int ch = rect.bottom - rect.top;
				int n,d;
				if ((cw * m_sheight) < (ch * m_swidth))
					{
					n = cw;
					d = m_swidth;
					}
				else
					{
					n = ch;
					d = m_sheight;
					} 
				Adjust(n,d);
				}
			else
				{
				m_pScene->Zoom(m_ZoomSave);
				int q = m_ZoomSave;
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
					q = 1600; // actually 33 %
				Adjust(q);
				}
			m_pScene->Flag(SCN_FLG_FIT,1,m_bFit);
			}
		return;
		}
	m_bFit = 0;
	if (m_pScene)
		{
		int q = num;
		if (q == 100)
			q = 0;
		else if (q == 25)
			q = 1;
		else if (q == 50)
			q = 2;
		else if (q == 200)
			q = 3;
		else if (q == 300)
			q = 4;
		else if (q == 400)
			q = 5;
		else if (q == 800)
			q = 6;
		else
			{
			q = 7;
			num = 1600;
			}
		m_pScene->Zoom(q);
		}
	Adjust(num);
}

void CSketchView::OnUpdZoom(CCmdUI* pCmdUI, int num, int denom)
{
	BOOL bFlag = FALSE;
	if (!ViewActive())
		pCmdUI->Enable(FALSE);
	else
		{
		if (m_bFit)
			bFlag = (!num & !denom) ? 1 : 0;
		else if ((num == m_nNum) && (denom == ((2 * m_nDen) / m_nFactor)))
			bFlag = TRUE;
		pCmdUI->SetCheck(bFlag);
		}
}

void CSketchView::OnPreview(UINT factor)
{
//	m_nPreview = factor;
	m_pScene->SetFactor(factor);
//	CSketchView * pView = (CSketchView *)GetActiveView();
	CheckUpdate();
	return;
}

void CSketchView::OnUpdPreview(CCmdUI* pCmdUI, UINT factor)
{
	BOOL bFlag = FALSE;
	if (!ViewActive())
		pCmdUI->Enable(FALSE);
	else if (!m_pScene  || (m_pScene->Broadcast() == 2))
		pCmdUI->Enable(FALSE);
	else
		{
		int fact;
		if (m_pScene)
			fact = m_pScene->ZFactor(2);
		else
			fact = 1;
		if ((UINT)fact == factor)
			bFlag = TRUE;
		pCmdUI->SetCheck(bFlag);
		}
}

void CSketchView::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
DPF("got sys:%d",nChar);
	if (nChar == 115)
		{
		CWnd::OnSysKeyDown(nChar, nRepCnt, nFlags);
		return;
		}
	if (m_Bret == 0)
		{
		if (m_bDown || m_bRDown)
			return;
		if ((m_wToolType == TOOL_FILL) && m_pScene->ColorMode())
			{
			m_nToolSaved = 2;
			m_wSaveTool = m_wToolType;
			MyCursor(0);
			OnTool(ToolId(m_wToolType = TOOL_PENCIL));
			CDocCursor();
			}
		else if (m_wToolType == TOOL_ZOOM)
			CDocCursor();
		}
	else
		CWnd::OnSysKeyDown(nChar, nRepCnt, nFlags);
}

void CSketchView::OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
DPF("sys key up,%d,%d",m_Bret,m_wToolType);
	if (m_Bret == 0)
		{
		if ((m_nToolSaved == 2) && !m_bDown)
			{
			m_nToolSaved = 0;
			OnTool(ToolId(m_wSaveTool));
//			OnTool(ToolId(m_pDoc->ToolInfo(0)));
		//	m_wToolType = m_wSaveTool;
		//	m_nToolSaved = 0;
			CDocCursor();
			}
		else if (m_wToolType == TOOL_ZOOM)
			CDocCursor();
		else
			CWnd::OnSysKeyUp(nChar, nRepCnt, nFlags);
		}
	else
		CWnd::OnSysKeyUp(nChar, nRepCnt, nFlags);
}

void CSketchView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
DPF("key down,%d,bret:%d",nChar,m_Bret);
#ifdef _DEBUG
	if (nChar == 'J')
		OnDebug();
#endif
	if (m_pFrame->Timing())
		{
		m_pFrame->GridChar(nChar,nRepCnt);
		return;
		}
	if (m_pFloat && (nChar == VK_ESCAPE))
		{
		if (m_pFloat->Rotated())
			{
			DrawFloat();
			m_pFloat->Rotated(0,1);
			DrawFloat();
			}
		else
			BlowFloat();
		return;
		}
	if ((m_nSelect > 13) && (nChar == VK_ESCAPE))
		{
		if (!m_bDown && !m_bRDown)
			m_nSelect--;
		Invalidate();
		m_nSelect = (m_bDown || m_bRDown) ? 16 : 0;
		return;
		}
	if ((m_Bret == 0) && m_pLayers->Modified() && m_pLayers->m_zx && 
				(m_wToolType == TOOL_FILL) && (nChar == VK_ESCAPE))
		{
		UINT z = m_pLayers->LastUndoIndex();
		if (m_pLayers->DeGapper(0,m_color))
			{
			m_pLayers->Undo(z); // don't pop
			FillIt(m_pLayers->m_zx, m_pLayers->m_zy,0);
			m_bModified = TRUE;
			m_pLayers->m_zx = 0;	// disable it
			Redraw();
			}
		return;
		}
	if ((m_Bret == 0) && (nChar == VK_SPACE))
		{
DPF("got space down");
		m_nToolSaved = 1;
		m_wSaveTool = m_wToolType;
		MyCursor(0);
		m_wToolType = TOOL_HAND;
		CDocCursor();
		return;
		}
	int basekey = 0;
	int zoomkey = 0;
	if (m_Bret == 0)
		{
		if ((m_wToolType == TOOL_PENCIL) ||
				(m_wToolType == TOOL_BRUSH) ||
				(m_wToolType == TOOL_TRACE))
			{
			if (nChar == VK_ADD)
				zoomkey = 1;
			else if (nChar == VK_SUBTRACT)
				zoomkey = 2;
			else if ((nChar == '1') || (nChar == VK_NUMPAD1))
				{
				m_pDoc->Radius(m_pDoc->Radius(3000,1000)-1,1000);
				return;
				}
			else if ((nChar == '2') || (nChar == VK_NUMPAD2))
				{
				m_pDoc->Radius(m_pDoc->Radius(3000,1000)+1,1000);
				return;
				}
			}
		else if (m_wToolType == TOOL_FILL)
			{
			if (nChar == VK_ADD)
				zoomkey = 1;
			else if (nChar == VK_SUBTRACT)
				zoomkey = 2;
			else if ((nChar >= '0') && (nChar <= '9'))
				basekey = '0';
			else if ((nChar >= VK_NUMPAD0) && (nChar <= VK_NUMPAD9))
				basekey = VK_NUMPAD0;
			}
		}
	if (basekey || zoomkey)
		{
		if (basekey)
			{
DPF("got number:%d",nChar - basekey);
			CNumberFill(nChar - basekey);
			}
		else if (zoomkey)
			{
			CPoint point;
			GetCursorPos(&point);
			DPF("apoint %d,%d",point.x,point.y);
			ScreenToClient(&point);
			XlatePoint(point);
			DoZoom(point,zoomkey-1);
			}
		}
	else if (m_Bret == 4)
		{
		if ((nChar == VK_RIGHT) || (nChar == VK_LEFT))
			return;
		}
	else if (m_Bret == 3)
		{
		if (m_CamState)
			{
DPF("key down, cam:%d",m_CamState);
	//		if (nChar == VK_SHIFT)
	//			{
	//			if (m_Camera & 16)
	//				m_Camera |= 32;
	//			CamCursor();
	//			}
			}
		else
			{
			int code = 0;
			if ((nChar == 'A') || (nChar == 'a'))
				code = ID_KEY_LEFT;
			else if ((nChar == 'D') || (nChar == 'd'))
				code = ID_KEY_RIGHT;
			if (code)
				{
				OnPositionKey(code);
				return;
				}
			}
		}
	else if (!m_pFloat)
		{
		if ((m_Bret == 0) && (nChar == VK_CONTROL))
			QSetup(TRUE);
		if ((nChar == VK_CONTROL) || (nChar == VK_SHIFT))
			CDocCursor();
//		if ((m_Bret == 0) && (nChar == VK_CONTROL))
//			CDocCursor();
//		if ((m_Bret == 0) && (nChar == VK_SHIFT))
//			CDocCursor();
		}
	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}


void CSketchView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
DPF("key up");
	if ((m_nToolSaved == 1) && (nChar == VK_SPACE))
		{
DPF("got space up,saved:%d",m_wSaveTool);
//		OnTool(ToolId(m_pDoc->ToolInfo(0)));
		OnTool(ToolId(m_wSaveTool));
		m_wToolType = m_wSaveTool;
		if (m_bErasing)
			m_pDoc->ToolInfo(2);	// toggle eraser
		m_nToolSaved = 0;
//		if (m_Bret == 0)
			CDocCursor();			
		MyCursor(1);
		}
	else 
		{
		if ((nChar == VK_CONTROL) || (nChar == VK_SHIFT))
			CDocCursor();
//		if ((m_Bret == 0) && (nChar == VK_SHIFT))
//			CDocCursor();
	//	if ((m_Bret == 3) && m_Camera && (nChar == VK_SHIFT))
	//		{
	//		m_Camera &= 0xffdf;	
	//		CamCursor();
	//		}
		CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
		}
}

void CSketchView::OnActionFill()
{
DPF("action fill");
	CActionFillDlg dlg;
	dlg.m_pScene = m_pScene;
	dlg.m_pFrame = m_pFrame;
	UINT EndLevel;
	if (!m_pFrame->GetSelection(dlg.m_StartFrame,dlg.m_Level,
				dlg.m_EndFrame, EndLevel))
		{
		dlg.m_StartFrame = dlg.m_EndFrame = CurrentFrame();
//		dlg.m_StartLevel = dlg.m_EndLevel = CurrentLevel();
		dlg.m_Level = CurrentLevel();
		}
	if ((m_EditFrame >= dlg.m_StartFrame) && 
		(m_EditFrame <= dlg.m_EndFrame) &&
		(m_EditLevel >= dlg.m_Level) &&
		(m_EditLevel <= EndLevel))
		{
		if (CheckModified(TRUE))
			return;
		}
	dlg.m_StartFrame++;
	dlg.m_EndFrame++;
	dlg.m_MaxFrame = m_pScene->FrameCount();
	dlg.m_MaxLevel = m_pScene->LevelCount()-1;
	dlg.m_Kind = 0;
//	dlg.m_layer = Layer();
	dlg.m_color = m_pDoc->Color();
	dlg.m_pTable = Layers()->LevelTable();
	dlg.DoModal();
	ASelectCell(m_EditFrame, m_EditLevel,1);
}

void CSketchView::OnActionFlip()
{
DPF("action flip");
	CActionFlipDlg dlg;
	dlg.m_pScene = m_pScene;
	dlg.m_pFrame = m_pFrame;
	if (!m_pFrame->GetSelection(dlg.m_StartFrame,dlg.m_StartLevel,
				dlg.m_EndFrame, dlg.m_EndLevel))
		{
		dlg.m_StartFrame = dlg.m_EndFrame = CurrentFrame();
		dlg.m_StartLevel = dlg.m_EndLevel = CurrentLevel();
		}
	if ((m_EditFrame >= dlg.m_StartFrame) && 
		(m_EditFrame <= dlg.m_EndFrame) &&
		(m_EditLevel >= dlg.m_StartLevel) && 
		(m_EditLevel <= dlg.m_EndLevel))
		{
		if (CheckModified(TRUE))
			return;
		}
	dlg.m_StartFrame++;
	dlg.m_EndFrame++;
	dlg.m_MaxFrame = m_pScene->FrameCount();
	dlg.m_MaxLevel = m_pScene->LevelCount()-1;
	dlg.m_Kind = 0;
	dlg.DoModal();
	ASelectCell(m_EditFrame, m_EditLevel,1);
}

void CSketchView::OnActionMatte()
{
DPF("action matte");
	CActionMatteDlg dlg;
	dlg.m_pScene = m_pScene;
	dlg.m_pFrame = m_pFrame;
	UINT EndLevel;
	if (!m_pFrame->GetSelection(dlg.m_StartFrame,dlg.m_Level,
				dlg.m_EndFrame, EndLevel))
		{
		dlg.m_StartFrame = dlg.m_EndFrame = CurrentFrame();
		dlg.m_Level = CurrentLevel();
		}
	if ((m_EditFrame >= dlg.m_StartFrame) && 
		(m_EditFrame <= dlg.m_EndFrame) &&
		(m_EditLevel >= dlg.m_Level) && 
		(m_EditLevel <= EndLevel))
		{
		if (CheckModified(TRUE))
			return;
		}
	dlg.m_StartFrame++;
	dlg.m_EndFrame++;
	dlg.m_MaxFrame = m_pScene->FrameCount();
	dlg.m_MaxLevel = m_pScene->LevelCount()-1;
//	dlg.m_blur = 5;
//	dlg.m_offx = 3;
//	dlg.m_offy = -3;
	dlg.m_color = m_pDoc->Color();
	dlg.m_pTable = Layers()->LevelTable();
	dlg.m_bOutSide = 0;
	dlg.DoModal();
	ASelectCell(m_EditFrame, m_EditLevel,1);
}

void CSketchView::OnActionSpeckle()
{
DPF("action speckle");
	CActionSpeckleDlg dlg;
	dlg.m_pScene = m_pScene;
	dlg.m_pFrame = m_pFrame;
	if (!m_pFrame->GetSelection(dlg.m_StartFrame,dlg.m_StartLevel,
				dlg.m_EndFrame, dlg.m_EndLevel))
		{
		dlg.m_StartFrame = dlg.m_EndFrame = CurrentFrame();
		dlg.m_StartLevel = dlg.m_EndLevel = CurrentLevel();
		}
	if ((m_EditFrame >= dlg.m_StartFrame) && 
		(m_EditFrame <= dlg.m_EndFrame) &&
		(m_EditLevel >= dlg.m_StartLevel) && 
		(m_EditLevel <= dlg.m_EndLevel))
		{
		if (CheckModified(TRUE))
			return;
		}
	dlg.m_StartFrame++;
	dlg.m_EndFrame++;
	dlg.m_MaxFrame = m_pScene->FrameCount();
	dlg.m_MaxLevel = m_pScene->LevelCount()-1;
	dlg.m_Kind = 0;
	dlg.DoModal();
	ASelectCell(m_EditFrame, m_EditLevel,1);
}


UINT CSketchView::CameraInit(UINT frame, UINT qpeg, UINT mask)
{
DPF("view camera init,frm:%d,which:%d,mask:%d",frame,qpeg,mask);
	if (!m_pScene) return 0;
	if (mask == 257)
		{
		m_CamState = 0;
		m_CamTool = 0;
		Invalidate(0);
		return 0;
		}
	if (mask == 256)
		{
		m_pegx = frame;
		m_pegy = qpeg;
		return 0;
		}
	m_CamState = 1;
	m_CamTool = mask;// & 15;
	SetupScrub(FALSE);
//	m_Index = frame;
	m_bDir = FALSE;
	m_bPlaying = FALSE;
	NewBret(3);
	Invalidate(0);
	m_origx = 0;
	m_origy = 0;
	return 0;
}


class CPropertiesDlg : public CMyDialog
{
public:
	CPropertiesDlg();

// Dialog Data
	//{{AFX_DATA(CPropertiesDlg)
	enum { IDD = IDD_SCENE_PROP };
	//}}AFX_DATA
	UINT m_w;
	UINT m_h;
	UINT m_fact;
	UINT m_frames;
	UINT m_levels;
	UINT m_id;
	CString m_sid;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();
};

CPropertiesDlg::CPropertiesDlg() : CMyDialog(CPropertiesDlg::IDD)
{
	//{{AFX_DATA_INIT(CPropertiesDlg)
	//}}AFX_DATA_INIT
}

void CPropertiesDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropertiesDlg)
	DDX_Text(pDX, IDC_PROP_WIDTH, m_w);
	DDX_Text(pDX, IDC_PROP_HEIGHT, m_h);
	DDX_Text(pDX, IDC_PROP_FACTOR, m_fact);
	DDX_Text(pDX, IDC_PROP_FRAMES, m_frames);
	DDX_Text(pDX, IDC_PROP_LEVELS, m_levels);
	DDX_Text(pDX, IDC_PROP_ID, m_sid);
	//}}AFX_DATA_MAP
}

BOOL CPropertiesDlg::OnInitDialog()
{ 
	m_sid.Format("%d-%d",m_id / 100000,m_id %100000);
	CMyDialog::OnInitDialog();
	CenterWindow();
	return FALSE;
}

void CSketchView::OnProperties()
{
	CPropertiesDlg dlg;
	dlg.m_w = m_pScene->Width();
	dlg.m_h = m_pScene->Height();
	dlg.m_fact = (100 * m_pScene->ZFactor(1)) / 2;
	dlg.m_frames = m_pScene->FrameCount();
	dlg.m_levels = m_pScene->LevelCount();
	dlg.m_id = m_pScene->SceneId();
	dlg.DoModal();
}


void CSketchView::MyPrint()
{
	DPF("on file print");
	if (CheckModified(TRUE	))
		return;
	UINT opt = m_pDoc->Option(PRINTOPTS);
	CPrintDlg dlg;
DPF("opt:%x",opt);
	dlg.m_nLCount = 1 + (opt & 31);
	dlg.m_nFCount = 1 + ((opt >> 5) & 63);
	dlg.m_bDown = (opt >> 11) & 1;
	dlg.m_nFit = (opt >> 12) & 3;
	dlg.m_nKind= (opt >> 14) & 3;
	dlg.m_bMarks = 0;
	for (;;)
		{
		int result = dlg.DoModal();
		opt = dlg.m_nKind & 3;
		opt = (opt << 2) + (dlg.m_nFit & 3);
		opt = (opt << 1) + (dlg.m_bDown & 1);
		opt = (opt << 6) + ((dlg.m_nFCount - 1) & 63);
		opt = (opt << 5) + ((dlg.m_nLCount - 1) & 31);
		if (result == 5)
			{
DPF("print");
			m_pDoc->Option(PRINTOPTS,1, opt);

			m_nPrintKind = dlg.m_nKind;
			m_nPrintFit= dlg.m_nFit;
			if (m_nPrintFit == 1)
				m_nPrintCount = dlg.m_nLCount;
			else
				m_nPrintCount = dlg.m_nFCount;
			m_bPrintDown = dlg.m_bDown;
			m_bPrintMarks= dlg.m_bMarks;

			CScrollView::OnFilePrint();
			break;
			}
		else if (result == 6)
			{
			m_pDoc->Option(PRINTOPTS,1, opt);
DPF("preview,opt:%x",opt);

			m_nPrintKind = dlg.m_nKind;
			m_nPrintFit= dlg.m_nFit;
			if (m_nPrintFit == 1)
				m_nPrintCount = dlg.m_nLCount;
			else
				m_nPrintCount = dlg.m_nFCount;
			m_bPrintDown = dlg.m_bDown;
			m_bPrintMarks= dlg.m_bMarks;
			CScrollView::OnFilePrintPreview();
			break;
			}
		else if (result == 7)
			{
			m_pDoc->Option(PRINTOPTS,1, opt);
DPF("setup");
			((CSketchApp*)AfxGetApp())->PrintSetup();
			}
		else
			break;
		}
	
}

void CSketchView::SetupPrint(CDC * pDC)
{
	int pageHeight = pDC->GetDeviceCaps(VERTRES);
	pageHeight -= 100; // for header
	int pageWidth = pDC->GetDeviceCaps(HORZRES);
	if (m_nPrintKind == 0)
		{
		m_nPages = 1;
		m_hdots = 200;
		}
	else  if (m_nPrintKind == 1)
		{
		m_nPages = 1;
		m_hdots = 200;
		}
	else
		{
		UINT Levels = m_pFrame->Columns() - 1;
		UINT Frames = m_pFrame->Frames();
DPF("fit:%d,count:%d",m_nPrintFit, m_nPrintCount);
		if (m_nPrintFit == 0)
			{
			m_nPages = 1;
			m_ldots = (3 * pageWidth) / (1 + 3 * Levels);
			int ww = (m_ldots * (1 + 3 * Levels)) / 3;
			int hh;
			if (!m_pDoc->Option(SHOWTHUMBS))
				{
				m_fdots = pageHeight / (1 + Frames);
				hh = m_fdots * (1 + Frames);
				if ((m_ldots) > (m_fdots * 4))
					m_ldots = 4 * m_fdots;
				else
					m_fdots = m_ldots / 4;
				}
			else
				{
				m_fdots = (3 * pageHeight) / (1 + 3 * Frames);
				hh = (m_fdots * (1 + 3 * Frames)) / 3;
				if ((m_ldots * 3) > (m_fdots * 4))
					m_ldots = (4 * m_fdots) / 3;
				else
					m_fdots = (3 * m_ldots) / 4;
				}
DPF("ld:%d,fd:%d,ww:%d,hh:%d",m_ldots,m_fdots,ww,hh);
			m_mdots = m_ldots / 3;
			m_hdots =  m_ldots / 4;
			m_FramesPerPage = Frames;
			m_LevelsPerPage = Levels;
			}
		else if (m_nPrintFit == 1)
			{
			m_LevelsPerPage = m_nPrintCount;
			m_ldots = (3 * pageWidth) / (3 * m_nPrintCount + 1);
			m_mdots = m_ldots / 3;
			m_hdots =  m_ldots / 4;
			if (!m_pDoc->Option(SHOWTHUMBS))
				m_fdots = m_hdots;
			else
				m_fdots = (3 * m_ldots) / 4;
			m_FramesPerPage = (pageHeight - m_hdots + m_fdots - 1) / m_fdots;
			m_FramesPerPage--;
			m_nPages = (Frames + m_FramesPerPage - 1) / m_FramesPerPage;
			m_nPages *= (Levels + m_LevelsPerPage - 1) / m_LevelsPerPage;
			}
		else
			{
			m_FramesPerPage = m_nPrintCount;
			if (!m_pDoc->Option(SHOWTHUMBS))
				{
				m_fdots = pageHeight / (m_FramesPerPage + 1);
				m_ldots = 4 * m_fdots;
				}
			else
				{
				m_fdots = (3 * pageHeight) / (3 * m_FramesPerPage + 1);
				m_ldots = (4 * m_fdots) / 3;
				}
			m_mdots = m_ldots / 3;
			m_hdots =  m_ldots / 4;
			m_LevelsPerPage = (pageWidth - m_hdots + m_ldots - 1) / m_ldots;
			m_nPages = (Frames + m_FramesPerPage - 1) / m_FramesPerPage;
			m_nPages *= (Levels + m_LevelsPerPage - 1) / m_LevelsPerPage;
			}
		}
}

void CSketchView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	DPF("begin printing,kind:%d",m_nPrintKind);
	SetupPrint(pDC);
	pInfo->SetMaxPage(m_nPages);
#ifndef FLIPBOOK_MAC
	m_PrinterFont.DeleteObject();
#ifndef ZZZZ
    LOGFONT lf;
    GetObject(GetStockObject(SYSTEM_FONT), sizeof(LOGFONT), &lf);
	lf.lfHeight = m_hdots / 2;
	lf.lfWidth = 0;
	m_PrinterFont.CreateFontIndirect(&lf);
#else
	m_PrinterFont.CreateFont(m_hdots / 2, 0, 0, 0, 1000, FALSE, FALSE,
								0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
								CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
								DEFAULT_PITCH | FF_MODERN,
								"Courier New");
#endif
#endif
}


void CSketchView::OnEndPrinting(CDC* pDC, CPrintInfo* pInfo)
{
#ifndef FLIPBOOK_MAC
	DPF("end print:%d",pInfo->m_bContinuePrinting);
	m_PrinterFont.DeleteObject();
#endif
}

void CSketchView::OnPrepareDC(CDC* pDC, CPrintInfo* pInfo)
{
#ifndef FLIPBOOK_MAC
//	DPF("on prepare dc");
/*
	if (!pInfo)
		{
    	CScrollView::OnPrepareDC(pDC, pInfo);
		return;
		}
	if (!pDC->IsPrinting())
		return;
*/
	if (pDC->IsPrinting())
    {
        int pageHeight = pDC->GetDeviceCaps(VERTRES);
        int originY = pageHeight * (pInfo->m_nCurPage - 1);
        pDC->SetViewportOrg(0, -originY);
    }
    int nPrevBkMode = pDC->SetBkMode(TRANSPARENT);
    CScrollView::OnPrepareDC(pDC, pInfo);

	// OnDraw().
//	DPF("pinfo:%lx",(DWORD)pInfo);
	if (pInfo)
		{
	//pInfo->m_bContinuePrinting = FALSE;
	DPF("prepareDC:%d,page:%d",pInfo->m_bContinuePrinting,pInfo->m_nCurPage);
		}

	m_pOldFont = pDC->SelectObject(&m_PrinterFont);
#endif
}

BOOL CSketchView::OnPreparePrinting(CPrintInfo* pInfo)
{
DPF("on prepare printing");
     // default preparation
     return DoPreparePrinting(pInfo);

}

void CSketchView::PrintHeader(CDC* pDC, LPRECT lpRect, int w , int h,
				int frame, int level)
{
	DPF("print header");
	UINT pw = pDC->GetDeviceCaps(HORZRES);
	UINT ph = pDC->GetDeviceCaps(VERTRES);
	m_hhdots = 100;
	ph -= m_hhdots;
	if (m_nPrintKind != 2)
	{
	if (pw * h > ph * w)
		{
		lpRect->top = m_hhdots; 
		lpRect->bottom = lpRect->top + ph;
		UINT t = ((ph * w) / h);
		lpRect->left = (pw - t) / 2;
		lpRect->right = lpRect->left + t;
		}
	else
		{
		lpRect->left = 0; 
		lpRect->right = pw;
		UINT t = ((pw * h) / w);
		lpRect->top = m_hhdots + (ph - t) / 2;
		lpRect->bottom = lpRect->top + t;
		}
	}
	RECT rect;
	rect.top = 0;
	rect.left = 0;
	rect.right = pw;
	rect.bottom = m_hhdots;
	UINT nFormat = DT_SINGLELINE | DT_CENTER | DT_VCENTER;
	UINT z = 0;
	UINT q = 0;
	
	char buf[300];
	char tmp[80];
	
	strcpy(buf,m_pDoc->GetPathName());
	if (m_nPrintKind == 0)
		{
		m_pScene->CellName(tmp, frame, level);
		strcat(buf,"  ");
		strcat(buf,tmp);
		}
	else if (m_nPrintKind == 1)
		{
		sprintf(tmp," Frame%d", frame);
		strcat(buf,tmp);
//		PrintFrame(pDC, pInfo);
		}
	else
		{
//		PrintSheet(pDC, pInfo);
		}
    pDC->DrawText(buf,-1, (LPRECT)&rect, nFormat);
}

void CSketchView::PrintFooter()
{
	DPF("print footer");
}

void CSketchView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
	DPF("onprint,page:%d",pInfo->m_nCurPage);
	SetupPrint(pDC);
#ifndef FLIPBOOK_MAC
    int nPrevBkMode = pDC->SetBkMode(TRANSPARENT);
	pDC->SelectStockObject(BLACK_PEN);
#endif
	if (m_nPrintKind == 0)
		PrintCel(pDC, pInfo);
	else if (m_nPrintKind == 1)
		PrintFrame(pDC, pInfo);
	else
		PrintSheet(pDC, pInfo);
	pDC->SelectObject(m_pOldFont);
#ifndef FLIPBOOK_MAC
    pDC->SelectStockObject(NULL_PEN);
    pDC->SetBkMode(nPrevBkMode);
#endif
	DPF("on print done");
}	


void CSketchView::PrintCel(CDC* pDC, CPrintInfo* pInfo)
{
	UINT StartFrame, EndFrame, StartLevel, EndLevel;
	if (!m_pFrame->GetSelection(StartFrame, StartLevel, EndFrame, EndLevel))
		{
		StartFrame = EndFrame = CurrentFrame();
		StartLevel = EndLevel = CurrentLevel();
		}
	UINT w = m_pScene->Width();
	UINT h = m_pScene->Height();
	BOOL bColor = m_pScene->ColorMode();
	UINT d = bColor ? 24 : 8;
	CDib dib;
	dib.Create(w,h,bColor ? 32 : 8);
	m_pScene->CombineLayers(dib.m_pBits,StartLevel, 
						EndLevel, StartFrame);
	dib.Convert(24);
	RECT rcDest;
	PrintHeader(pDC, &rcDest,w,h,StartFrame,StartLevel);
	UINT PalMode = DIB_RGB_COLORS;
DPR("print dst",&rcDest);
	if (m_bPrintMarks)
	PrintRegMarks(dib.m_pBits,w,h,d);
	BOOL bSuccess = ::StretchDIBits(pDC->m_hDC,
							   rcDest.left, 
							   rcDest.top,                  // 
							   rcDest.right - rcDest.left,
							   rcDest.bottom - rcDest.top,
							   0,0,w,h,dib.m_pBits,
                					dib.m_pBMI,	
							   PalMode,                 // wUsage
							   SRCCOPY);                       // dwROP
	DPF("bsuccs:%d",bSuccess);
}

void CSketchView::PrintFrame(CDC* pDC, CPrintInfo* pInfo)
{
	UINT StartFrame, EndFrame, StartLevel, EndLevel;
	if (!m_pFrame->GetSelection(StartFrame, StartLevel, EndFrame, EndLevel))
		{
		StartFrame = EndFrame = CurrentFrame();
		StartLevel = EndLevel = CurrentLevel();
		}
	UINT w = m_pScene->ComW();
	UINT h = m_pScene->ComH();
	BOOL bColor = m_pScene->ColorMode();
	UINT d = bColor ? 24 : 8;
	CDib dib;
	dib.Create(w,h,d);
	m_pScene->CompositeFrame32(dib.m_pBits,StartLevel, 
						EndLevel, StartFrame, 0);
	RECT rcDest;
	PrintHeader(pDC, &rcDest,w,h,StartFrame,0);
	UINT PalMode = DIB_RGB_COLORS;
DPR("print dst",&rcDest);
	if (m_bPrintMarks)
	PrintRegMarks(dib.m_pBits,w,h,d);
	BOOL bSuccess = ::StretchDIBits(pDC->m_hDC,
							   rcDest.left, 
							   rcDest.top,                  // 
							   rcDest.right - rcDest.left,
							   rcDest.bottom - rcDest.top,
							   0,0,w,h,dib.m_pBits,
                					dib.m_pBMI,	
							   PalMode,                 // wUsage
							   SRCCOPY);                       // dwROP
	DPF("bsuccs:%d",bSuccess);
}

void CSketchView::PrintSheet(CDC* pDC, CPrintInfo* pInfo)
{
	int pageHeight = pDC->GetDeviceCaps(VERTRES);
	int pageWidth = pDC->GetDeviceCaps(HORZRES);
	UINT Levels = m_pFrame->Columns() - 1;
	UINT Frames = m_pFrame->Frames();
	pDC->SetViewportOrg(0,0);
	int Page = pInfo->m_nCurPage;
	if (Page) Page--;
DPF("ld:%d,fd:%d,md:%d,hd:%d",m_ldots,m_fdots,m_mdots,m_hdots);
DPF("lvls:%d,frms:%d,lpp:%d,fpp:%d",Levels,Frames,m_LevelsPerPage,m_FramesPerPage);
	int row, column;
	int rows = 1 + m_FramesPerPage;
	int columns = 1 + m_LevelsPerPage;
	int f,q,z;
	if (!m_bDown)
		{
		f = (Levels + m_LevelsPerPage - 1) / m_LevelsPerPage;
		z = Page / f;
		q = Page % f;
		}
	else
		{
		f = (Frames + m_FramesPerPage -1) / m_FramesPerPage;
		z = Page % f;
		q = Page / f;
		}
	int Level = q * m_LevelsPerPage;
	int Frame = z * m_FramesPerPage;
//	rows = 1 + m_FramesPerPage;
//	columns = 1 + m_LevelsPerPage;
	int x,y;
	RECT rect;
	PrintHeader(pDC, &rect,0,0,0,0);
	int zz = m_hhdots;
DPF("down:%d,level:%d,Frame:%d",m_bDown,Level,Frame);
	for (y = 0; y <= m_FramesPerPage; y++)
	for (x = 0; x <= m_LevelsPerPage; x++)
		{
		if (y == 0)
			{
			rect.top = zz;
			rect.bottom = rect.top+m_hdots;
			}
		else
			{
			rect.top = zz + m_hdots + m_fdots * (y - 1);
			rect.bottom = rect.top + m_fdots;
			}
		if (x == 0)
			{
			rect.left = 0;
			rect.right = m_mdots;
			}
		else
			{
			rect.left = m_mdots + (x - 1) * m_ldots;
			rect.right = rect.left + m_ldots;
			}
		if (y == 0)
			row = 0;
		else
			row = Frame + y;
		if (x == 0)
			column = 0;
		else
			column = Level + x;
		if ((UINT)column > Levels)
			continue;
		if ((UINT)row > Frames)
			continue;
		m_pFrame->DrawThumb(pDC,&rect,row,column,99);
		m_pFrame->DrawText(pDC,&rect,row,column);
		pDC->MoveTo(rect.left, rect.top);
		pDC->LineTo(rect.right,rect.top);
		if (((UINT)column == Levels) || (x == m_LevelsPerPage))
			pDC->LineTo(rect.right,rect.bottom);
		pDC->MoveTo(rect.left, rect.top);
		pDC->LineTo(rect.left, rect.bottom);
		if (((UINT)row == Frames) || (y == m_FramesPerPage))
			pDC->LineTo(rect.right, rect.bottom);
		}
}

void CSketchView::PrintRegMarks(LPBYTE pBits, UINT w, UINT h, UINT depth)
{
	UINT pitch = 4 * ((w * depth + 31) / 32);
	UINT radius = 15;
	UINT dd = 2 * radius + 1;
	UINT bpp = depth / 8;
	UINT q, i,j, x, y;
	for (q = 0; q < 4; q++)
		{
		y = q > 1 ? h - 1 - radius : radius;
		x = (q & 1) ? w - dd : 0;
		for (i = 0; i < dd; i++)
			for (j = 0; j < bpp; j++)
				{
				pBits[y * pitch + bpp*(x+i) + j] ^= 255;
//				pBits[(y+1) * pitch + bpp*(x+i) + j] ^= 255;
				}
		y = q > 1 ? h - dd : 0;
		x = (q & 1) ? w - 1 - radius : radius;
		for (i = 0; i < dd; i++)
			for (j = 0; j < bpp; j++)
				{
				pBits[(y+i) * pitch + bpp*x + j] ^= 255;
//				pBits[(y+i) * pitch + bpp*(x+1) + j] ^= 255;
				}
		}
}



BOOL CVWnd::MyCreate(CSketchView * pParent, int x, int y, int w, int h)
{
	m_pView = pParent;
	RECT rcwin;
	rcwin.top = y;
	rcwin.left = x;
	rcwin.bottom = y + h;
	rcwin.right = x + w;
	return CreateEx(0, NULL, NULL,
					WS_CHILD | WS_POPUP | WS_VISIBLE,// | WS_DLGFRAME,
					rcwin,pParent, 0);
}

// OnPaint:
// This routine draws the string "Hello, Windows!" in the center of the
// client area.  It is called whenever Windows sends a WM_PAINT message.
// Note that creating a CPaintDC automatically does a BeginPaint and
// an EndPaint call is done when it is destroyed at the end of this
// function.  CPaintDC's constructor needs the window (this).
//
void CVWnd::OnPaint()
{
//	CString s = "Hello, Windows!";
	CPaintDC dc( this );
	m_pView->FMyPaint(&dc);
}

BEGIN_MESSAGE_MAP( CVWnd, CWnd )
	//{{AFX_MSG_MAP( CMainWindow )
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CVWnd::OnLButtonDown(UINT, CPoint zpoint)
{ 
DPF("lbut down,x:%d,y:%d",zpoint.x, zpoint.y);
	SetCapture();
	CRect rect;
	GetWindowRect(&rect);
	CPoint point;
	GetCursorPos(&point);
	m_initx = rect.left - point.x;
	m_inity = rect.top - point.y;
}

void CVWnd::OnLButtonUp(UINT, CPoint point)
{
	if (GetCapture() == this)
		ReleaseCapture();
}

void CVWnd::OnMouseMove(UINT, CPoint zpoint)
{
	DPF("move,%d,%d",zpoint.x,zpoint.y);
	if (GetCapture() == this)
		{
		CPoint point;
		GetCursorPos(&point);
    	SetWindowPos(NULL, m_initx + point.x, m_inity + point.y,
				0,0,SWP_NOACTIVATE | SWP_NOSIZE );
		}
}


void  CSketchView::ProcessTabletCode(UINT code)
{
	UINT desired;

	if (code & 4)
		{
/*
		UINT touch = (code >> 8) & 16;
		if (touch)
			{
			if (!m_nToolSaved)
				{
				m_nToolSaved = 3;
				m_wSaveTool = m_wToolType;
				OnTool(ID_TOOL_HAND);
				}
			}
		else if (m_nToolSaved == 3)
			{
			OnTool(ToolId(m_wSaveTool));
			m_wToolType = m_wSaveTool;
			m_nToolSaved = 0;
			}
*/
		UINT cursor = (code >> 8) & 7;
		if (m_pDoc->ToolInfo(0))
			desired = 1;
		else
			desired = 0;
		DPF("change,cur:%d,des:%d",cursor,desired);
		if (desired != cursor)	
			{
//			MessageBeep(0);
			OnTool(ID_TOOL_ERASER);
			}
		}

	if (code & 1)
		{
		m_wPressure = code >> 16;
		}
}

LRESULT CSketchView::OnFlickMsg(WPARAM arg1, LPARAM arg2)
{
	return 1;//FLICK_WM_HANDLED_MASK;
}

LRESULT CSketchView::OnTabletStatusMsg(WPARAM arg1, LPARAM arg2)
{
	UINT mask = 
 TABLET_DISABLE_PRESSANDHOLD | // disables press and hold (right-click) gesture
 TABLET_DISABLE_PENTAPFEEDBACK | // disables UI feedback on pen up (waves)
 TABLET_DISABLE_PENBARRELFEEDBACK | // disables UI feedback on pen button down(circle)
 TABLET_DISABLE_FLICKS; // disables pen flicks (back, forward, drag down, drag up)
	return mask;
}

LRESULT CSketchView::OnTabletMsg(WPARAM arg1, LPARAM arg2)
{
	m_bTablet = TRUE;
	DPF("tab msg,%d,%d",arg1,arg2);
	if ((arg1 & 64) && !m_LBState)
		{
	//	if ((m_wToolType != TOOL_PENCIL) && (m_wToolType != TOOL_BRUSH))
	//		return 1;
		m_LBState = 2;
		int x = arg2 & 0x7fff;
		int y = arg2 >> 16;
		x /= m_brush_factor;
		y /= m_brush_factor;
		StartDraw(CPoint(x,y));
		}
	else
		ProcessTabletCode(arg1);
	return TRUE;
}

LRESULT CSketchView::OnWTPacket(WPARAM wSerial, LPARAM hCtx)
{
	if (!m_pTablet)
		return 0;
	m_bTablet = TRUE;
	UINT code = m_pTablet->Packet(wSerial, hCtx);
	ProcessTabletCode(code);
	return TRUE;
}

void CSketchView::SetColorTrap(UINT which,UINT kind, POINT p1, POINT p2)
{
	OnVCRExit();
	m_nSelect = 0;
	m_nColorTrap = 1+which;
	m_gradk = kind;
	m_grad1 = p1;
	m_grad2 = p2;
	Redraw();
}

void CSketchView::CameraStart(CPoint point)
{
	ASSERT(m_CamState == 1);
		SetCapture();
	m_origx = point.x;
	m_origy = point.y;
	int dx = point.x - m_pegx;
	int dy = point.y - m_pegy;
	if ((dx * dx + dy * dy) < 30)
		{
		m_CamState = 3;
		m_pFrame->CameraPacket(16+2,point.x,point.y);
// moving peg center
		return;
		}
// if over a different peg select it
//cam undo
	UINT peg = m_pScene->CamCurPeg();
	if (peg > 100)
		return;
	m_pFrame->CameraPacket(6,peg,1);
//		if ((m_Camera & 16) && (GetKeyState(VK_SHIFT) >= 0))
//			m_Camera |= 32;
	m_pScene->SelectPeg(peg); 
	m_pFrame->CameraPacket(2,point.x,point.y);
	int t;
	if (m_CamTool & 16)
		t = IDC_NSEW_CUR;
	else
		t = IDC_NS_CUR + (m_CamTool & 15);
	SetCursor(AfxGetApp()->LoadCursor(t));
	m_CamState = 2; 
}

void CSketchView::CameraMove(CPoint point)
{
	XlatePoint(point);
DPF("camera move,x:%d,y:%d",point.x,point.y);
	if (m_bDown)
		{
		if (m_CamState == 3)
			{
			m_pFrame->CameraPacket(16+3,point.x,point.y);
			return;
			}
		if (m_CamState == 1) return;
ASSERT(m_CamState == 2);
		if (!(m_CamTool & 16))
			{
			if (m_CamTool == 0)
				point.x = m_origx;
			if (m_CamTool  == 1)
				point.y = m_origy;
			}
		if ((point.x != m_origx) && (point.y != m_origy))
			{
			point.x = point.x;
			}
		m_pFrame->CameraPacket(1,point.x,point.y);
		}
	else
		{
		Progress(996,m_pScene->CamCurPeg());
		if (m_pScene->CamCursor(point.x, point.y))
			Invalidate();
		}
}

void CSketchView::CameraStop(CPoint point)
{
	if (m_CamState == 3)
		{
		m_pFrame->CameraPacket(16,point.x,point.y);
		m_CamState = 1;
		}
	else
		{
		if (m_CamState == 1)
			{
			return;
			}
ASSERT(m_CamState == 2);
		m_CamState = 1;
		m_pFrame->CameraPacket(0,point.x,point.y);
		m_pScene->SelectPeg(-1);
		}
}
#ifdef _DEBUG
void CSketchView::OnDebug()
{
	DPF("on debug");
	m_pScene->ShowLinks();
}
#endif
