#include "stdafx.h"
#include "sketch.h"
#include "resource.h"
#include "optdlg.h"
#include "mydoc.h"
#include "myview.h"
#include "cscene.h"
#include "sceneopt.h"
#ifndef FLIPBOOK_MAC
#include "wintab.h"
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

COptions::COptions(CWnd* pParent /*=NULL*/)
	: CMyDialog(COptions::IDD, pParent)
{
	//{{AFX_DATA_INIT(COptions)
	//}}AFX_DATA_INIT
}

BOOL COptions::OnInitDialog()
{
	CMyDialog::OnInitDialog();
	OEM_TEXT(this);
	BOOL bWinTab = ((CSketchApp *)AfxGetApp())->HaveWinTab();

	GetDlgItem(IDC_USEWINTAB)->EnableWindow(bWinTab);
	GetDlgItem(IDC_MIN_PRESSURE)->EnableWindow(m_bTabletUsed || bWinTab);
	GetDlgItem(IDC_MAX_PRESSURE)->EnableWindow(m_bTabletUsed || bWinTab);
	GetDlgItem(IDC_TABLET_PRIMARY)->EnableWindow(m_bTabletUsed || bWinTab);

	if (!((CSketchApp *)AfxGetApp())->RemindFlags(32))
		GetDlgItem(IDC_ENABLE_SHOW)->EnableWindow(0);
	BOOL bJpeg = m_nJpeg ? 1 : 0;
	UINT show = bJpeg ? SW_SHOW : SW_HIDE;
	CheckDlgButton(IDC_USE_JPEG, bJpeg);
	GetDlgItem(IDC_QUAL_TEXT)->ShowWindow(show);
	GetDlgItem(IDC_QUALITY)->ShowWindow(show);
	show = ((CSketchApp *)AfxGetApp())->IsAnimate() ? SW_SHOW : SW_HIDE;
	GetDlgItem(IDC_SCRUB_EDIT)->ShowWindow(show);
	if (show == 99)//SW_HIDE)
		{
		GetDlgItem(IDC_EMBED_SND_BOX)->ShowWindow(show);
		GetDlgItem(IDC_EMBED_SND_NEVER)->ShowWindow(show);
		GetDlgItem(IDC_EMBED_SND_ALWAYS)->ShowWindow(show);
		GetDlgItem(IDC_EMBED_SND_PROMPT)->ShowWindow(show);
		RECT rc1,rcOK,rccancel,rcme;
		GetDlgItem(IDC_EMBED_SND_BOX)->GetWindowRect(&rc1);
		ScreenToClient(&rc1);
		GetDlgItem(IDOK)->GetWindowRect(&rcOK);
		ScreenToClient(&rcOK);
		GetDlgItem(IDCANCEL)->GetWindowRect(&rccancel);
		ScreenToClient(&rccancel);
		GetDlgItem(IDOK)->SetWindowPos(NULL, rcOK.left,rc1.top,
						0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOZORDER);
		GetDlgItem(IDCANCEL)->SetWindowPos(NULL, rccancel.left,rc1.top,
						0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOZORDER);
		GetWindowRect(&rcme);
    	SetWindowPos(NULL, 0, 0, rcme.right - rcme.left,
			rcme.bottom - rcme.top - (rccancel.top - rc1.top),
					SWP_NOZORDER | SWP_NOMOVE);
		}
	return FALSE;
}

#define MYDDV_MinMaxInt(a,b,c,d,e)  DDX_Text(a, b, c); DDV_MinMaxInt(a, c, d,e);
void COptions::DoDataExchange(CDataExchange* pDX)
{
	DPF("data xchg");
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptions)
	DDX_Check(pDX, IDC_USEWINTAB, m_bUseWinTab);
//	DDX_Check(pDX, IDC_TABPRESSURE, m_bTabPressure);
	DDX_Check(pDX, IDC_TABLET_PRIMARY, m_bTabPrimary);
	DDX_Check(pDX, IDC_LEFTHAND, m_bLeftHanded);
	DDX_Check(pDX, IDC_LEFT_SCROLL, m_bLeftScroll);
	DDX_Check(pDX, IDC_TOOL_COLOR, m_bToolsSameColor);
	DDX_Check(pDX, IDC_SIMPLE_CURSOR, m_bSimpleCursor);
//	DDX_Text(pDX, IDC_BGMINDARK, m_wBGMinDark);
//	DDV_MinMaxInt(pDX, IDC_BGMINDARK,0,100);
//	MYDDV_MinMaxInt(pDX, IDC_BGMINDARK, m_wBGMinDark,0,100);
//	DDX_Check(pDX, IDC_SHOWBG, m_bBGShow);
//	DDX_Check(pDX, IDC_PEGSHOWBG, m_bPegShowBG);//
//	DDX_Check(pDX, IDC_PEGSHOWFG, m_bPegShowFG);
//	DDX_Check(pDX, IDC_FLIPFEW, m_bFlipFew);
//	DDX_Check(pDX, IDC_AUTOKEEP, m_bAutoKeep);
	DDX_Check(pDX, IDC_CHECKBG, m_bCheckerBG);
	DDX_Check(pDX, IDC_PREVIOUS, m_bOpenPrevious);
	DDX_Check(pDX, IDC_SAVE_CACHE, m_bSaveCache);
	DDX_Check(pDX, IDC_HARD_ZOOM, m_bHardZoom);
	DDX_Check(pDX, IDC_SHEET_TOP, m_bSheetOnTop);
	DDX_Check(pDX, IDC_CHK_UPDATES, m_bChkUpdates);
	DDX_Check(pDX, IDC_USE_MOUTHS, m_bUseMouths);
	DDX_Check(pDX, IDC_SCRUB_EDIT, m_bEditScrub);
	if (pDX->m_bSaveAndValidate)
		{
		if (m_nJpeg)
			{
			DDX_Text(pDX, IDC_QUALITY, m_nJpeg);
			DDV_MinMaxInt(pDX, m_nJpeg,25,100);
			}
		}
	else
		{
		BOOL bJpeg = m_nJpeg ? 1 : 0;
		DDX_Check(pDX, IDC_USE_JPEG, bJpeg);
		DDX_Text(pDX, IDC_QUALITY, m_nJpeg);
		}
//	DDX_Check(pDX, IDC_SCALE_BG, m_bScaleBG);
	DDX_Radio(pDX, IDC_BROADCAST0, m_broadcast);

//	MYDDV_MinMaxInt(pDX, IDC_PEGMAXDEPTH, m_wPegMaxDepth,1,10);
//	MYDDV_MinMaxInt(pDX, IDC_PEGMINDENSITY, m_wPegMinDensity,5,100);

//	MYDDV_MinMaxInt(pDX, IDC_PENMINRAD, m_wPenMinRadius,1,50);
//	MYDDV_MinMaxInt(pDX, IDC_PENMAXRAD, m_wPenMaxRadius,5,50);
//	MYDDV_MinMaxInt(pDX, IDC_PENCILMINDENSITY, m_wPencilMinDensity,1,100);
//	MYDDV_MinMaxInt(pDX, IDC_PENCILMAXDENSITY, m_wPencilMaxDensity,5,100);
//	MYDDV_MinMaxInt(pDX, IDC_PRESSUREMAX, m_wMaxPressure,5,100);
	MYDDV_MinMaxInt(pDX, IDC_RATE, m_rate,1,70);
	MYDDV_MinMaxInt(pDX, IDC_MIN_PRESSURE, m_wMinPress,0,20);
	MYDDV_MinMaxInt(pDX, IDC_MAX_PRESSURE, m_wMaxPress,m_wMinPress+1,255);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptions, CMyDialog)
	//{{AFX_MSG_MAP(COptions)
	ON_BN_CLICKED(IDC_ENABLE_SHOW, OnEnableShows)
	ON_BN_CLICKED(IDC_USE_JPEG, OnJpeg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// COptions message handlers

void COptions::OnEnableShows()
{
	((CSketchApp *)AfxGetApp())->RemindFlags(33);
	GetDlgItem(IDC_ENABLE_SHOW)->EnableWindow(0);
}


void COptions::OnJpeg()
{
	if (m_nJpeg)
		m_nJpeg = 0;
	else
		{
		GetDlgItem(IDC_QUALITY)->SetWindowText("80");
		m_nJpeg = 80;
		}
	BOOL bShow = m_nJpeg ? 1 : 0;
	UINT show = m_nJpeg ? SW_SHOW : SW_HIDE;
	GetDlgItem(IDC_QUAL_TEXT)->ShowWindow(show);
	GetDlgItem(IDC_QUALITY)->ShowWindow(show);
}


CLightOptions::CLightOptions(CWnd* pParent /*=NULL*/)
	: CMyDialog(CLightOptions::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLightOptions)
	//}}AFX_DATA_INIT
}

BOOL CLightOptions::OnInitDialog()
{
	m_bDirty = FALSE;
	m_pScene = m_pDoc->GetDocScene();
	m_nLevels = m_pScene->LevelCount();
	m_nKind  = m_pDoc->Option(SC_MRU);
	if (m_nKind > 2) m_nKind = 2;
	m_Kind = m_nKind;
	
	m_wPegMaxDepth = m_Depth = m_pDoc->Option(PEG_DEPTH);
	m_wPegMinDensity = m_Den = m_pDoc->Option(PEG_DENSITY);
	m_wBGMinDark = m_Dark = m_pDoc->Option(BGMIN);
	m_bPegShowBG = m_BG = m_pDoc->Option(PEG_SHOWBG);
	m_bPegShowFG = m_FG = m_pDoc->Option(PEG_SHOWFG);
	m_bSheet = m_Sheet = m_pDoc->Option(XSHEET_HIGHLIGHT);
	m_bShowNext = m_Prev = m_pDoc->Option(SC_NEXT);
	m_bShowPrev = m_Next = m_pDoc->Option(SC_PREV);
	m_bShowNext1 = m_Prev1 = m_pDoc->Option(SC_NEXT1);
	m_bShowPrev1 = m_Next1 = m_pDoc->Option(SC_PREV1);
	m_bShowPaint = m_Paint = m_pDoc->Option(SC_PAINT);
	m_bShowLPaint = m_LPaint = m_pDoc->Option(SC_LIGHT_PAINT);
	if (m_nLevels < 3)
		{
		m_bud0 = 0;
		m_bud1 = 0;
		m_bud2 = 0;
		m_bud3 = 0;
		}
	else
		{
		m_bud0 = m_pScene->SceneOptionInt(SCOPT_BUD0) & 1023;
		if (m_bud0 >= (UINT)m_nLevels)
			m_bud0 = m_nLevels - 1;
		m_bud1 = m_pScene->SceneOptionInt(SCOPT_BUD1) & 1023;
		if (m_bud1 >= (UINT)m_nLevels)
			m_bud1 = m_nLevels - 1;
		m_bud2 = m_pScene->SceneOptionInt(SCOPT_BUD2) & 1023;
		if (m_bud2 >= (UINT)m_nLevels)
			m_bud2 = m_nLevels - 1;
		m_bud3 = m_pScene->SceneOptionInt(SCOPT_BUD3) & 1023;
		if (m_bud3 >= (UINT)m_nLevels)
			m_bud3 = m_nLevels - 1;
		}
	m_nBuddy0 = m_bud0;
	m_nBuddy1 = m_bud1;
	m_nBuddy2 = m_bud2;
	m_nBuddy3 = m_bud3;
	m_bShowBud0 = m_nBuddy0 ? 1 : 0;
	m_bShowBud1 = m_nBuddy1 ? 1 : 0;
	m_bShowBud2 = m_nBuddy2 ? 1 : 0;
	m_bShowBud3 = m_nBuddy3 ? 1 : 0;
	if (!m_bShowBud0) m_nBuddy0 = 1;
	if (!m_bShowBud1) m_nBuddy1 = 1;
	if (!m_bShowBud2) m_nBuddy2 = 1;
	if (!m_bShowBud3) m_nBuddy3 = 1;
	CMyDialog::OnInitDialog();
	BOOL bColorTool = FALSE;
	if (m_pScene->ColorMode())
		{
		UINT tool = m_pDoc->SelectTool();
		if ((tool == TOOL_BRUSH) || (tool == TOOL_FILL))
			{
			CheckDlgButton(IDC_LIGHTBOX_PAINT, 1);
			bColorTool = TRUE;
			}
		}
	GetDlgItem(IDC_LIGHT_TOOL)->ShowWindow(!bColorTool ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDC_LIGHTBOX_PAINT)->EnableWindow(!bColorTool);
	UpdateControls();
	return FALSE;
}

#define MYDDV_MinMaxInt(a,b,c,d,e)  DDX_Text(a, b, c); DDV_MinMaxInt(a, c, d,e);
void CLightOptions::DoDataExchange(CDataExchange* pDX)
{
	DPF("data xchg");
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLightOptions)
#if 0
	// on the Mac the control position is reserved, so we have to flip this value
	m_nKind = !m_nKind;
	DDX_Radio(pDX, IDC_LIGHT_INBETWEEN, m_nKind);
	m_nKind = !m_nKind;
#else
	DDX_Radio(pDX, IDC_LIGHT_INBETWEEN, m_nKind);	
#endif
	MYDDV_MinMaxInt(pDX, IDC_LIGHT_DEPTH, m_wPegMaxDepth,1,5);
	UINT min = m_wPegMinDensity % 1000;
	UINT max = m_wPegMinDensity / 1000;
	if (!max || (max > 100)) max = 100;
	MYDDV_MinMaxInt(pDX, IDC_LIGHT_DENSITY, min,1,99);
	MYDDV_MinMaxInt(pDX, IDC_LIGHT_MAX, max,min,100);
	m_wPegMinDensity = 1000 * max + min;
	
	DDX_Check(pDX, IDC_LIGHT_BG, m_bPegShowBG);//
	DDX_Check(pDX, IDC_LIGHT_XSHEET, m_bSheet);
	MYDDV_MinMaxInt(pDX, IDC_LIGHT_BG_DARK, m_wBGMinDark,0,100);

	DDX_Check(pDX, IDC_LIGHT_NEXT, m_bShowNext);
	DDX_Check(pDX, IDC_LIGHT_PREV, m_bShowPrev);
	DDX_Check(pDX, IDC_LIGHT_NEXT1, m_bShowNext1);
	DDX_Check(pDX, IDC_LIGHT_PREV1, m_bShowPrev1);
	DDX_Check(pDX, IDC_LIGHT_BUD_0, m_bShowBud0);
	DDX_Check(pDX, IDC_LIGHT_BUD_1, m_bShowBud1);
	DDX_Check(pDX, IDC_LIGHT_BUD_2, m_bShowBud2);
	DDX_Check(pDX, IDC_LIGHT_BUD_3, m_bShowBud3);
	DDX_Check(pDX, IDC_SHOW_PAINT, m_bShowPaint);
	DDX_Check(pDX, IDC_LIGHTBOX_PAINT, m_bShowLPaint);
	MYDDV_MinMaxInt(pDX, IDC_LIGHT_BUDDY0,m_nBuddy0,1,m_nLevels-1);
	MYDDV_MinMaxInt(pDX, IDC_LIGHT_BUDDY1,m_nBuddy1,1,m_nLevels-1);
	MYDDV_MinMaxInt(pDX, IDC_LIGHT_BUDDY2,m_nBuddy2,1,m_nLevels-1);
	MYDDV_MinMaxInt(pDX, IDC_LIGHT_BUDDY3,m_nBuddy3,1,m_nLevels-1);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CLightOptions, CMyDialog)
	//{{AFX_MSG_MAP(CLightOptions)
	ON_BN_CLICKED(IDC_LIGHT_MRU, UpdateControls)
	ON_BN_CLICKED(IDC_LIGHT_INBETWEEN, UpdateControls)
	ON_BN_CLICKED(IDC_LIGHT_PAINT, UpdateControls)
	ON_BN_CLICKED(IDC_LIGHT_BUD_0, UpdateControls)
	ON_BN_CLICKED(IDC_LIGHT_BUD_1, UpdateControls)
	ON_BN_CLICKED(IDC_LIGHT_BUD_2, UpdateControls)
	ON_BN_CLICKED(IDC_LIGHT_BUD_3, UpdateControls)
	ON_COMMAND(IDC_APPLY, OnApply)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CLightOptions::ChangeMode()
{
	CSketchView* pSketchView = m_pDoc->GetDocView();
	if (pSketchView != NULL)
		pSketchView->SendMessage(WM_COMMAND, ID_VIEW_MODE);
	m_bColor = m_pScene->ColorMode();
}

void CLightOptions::UpdateControls()
{
	UpdateData();
//	if ((m_nKind / 2) != (int)m_bColor)
//		{
//		ChangeMode();
//		ApplySettings();
//		}
	GetDlgItem(IDC_LIGHT_NEXT)->EnableWindow(!m_nKind);
	GetDlgItem(IDC_LIGHT_PREV)->EnableWindow(!m_nKind);
	GetDlgItem(IDC_LIGHT_NEXT1)->EnableWindow(!m_nKind);
	GetDlgItem(IDC_LIGHT_PREV1)->EnableWindow(!m_nKind);
	BOOL bLevels = m_nLevels > 2 ? 1 : 0;
	GetDlgItem(IDC_LIGHT_BUD_0)->EnableWindow(bLevels);
	GetDlgItem(IDC_LIGHT_BUD_1)->EnableWindow(bLevels);
	GetDlgItem(IDC_LIGHT_BUD_2)->EnableWindow(bLevels);
	GetDlgItem(IDC_LIGHT_BUD_3)->EnableWindow(bLevels);
	GetDlgItem(IDC_LIGHT_BUDDY0)->EnableWindow(m_bShowBud0);
	GetDlgItem(IDC_LIGHT_BUDDY1)->EnableWindow(m_bShowBud1);
	GetDlgItem(IDC_LIGHT_BUDDY2)->EnableWindow(m_bShowBud2);
	GetDlgItem(IDC_LIGHT_BUDDY3)->EnableWindow(m_bShowBud3);
	GetDlgItem(IDC_LIGHT_DEPTH)->EnableWindow(m_nKind == 1);
	GetDlgItem(IDC_LIGHT_DENSITY)->EnableWindow(m_nKind == 1);
	GetDlgItem(IDC_LIGHT_MAX)->EnableWindow(m_nKind == 1);
	GetDlgItem(IDC_SHOW_PAINT)->EnableWindow(m_nKind == 2);
}

void CLightOptions::ApplySettings()
{
	m_bDirty = TRUE;
	m_pDoc->Option(FIRST_OPTION);//OPT_HOLD);
	m_pDoc->Option(BGMIN, 1,m_wBGMinDark);
	m_pDoc->Option(PEG_SHOWBG, 1, m_bPegShowBG);
	m_pDoc->Option(PEG_SHOWFG, 1, m_bPegShowFG);
	m_pDoc->Option(PEG_DEPTH, 1, m_wPegMaxDepth);
	m_pDoc->Option(PEG_DENSITY, 1,m_wPegMinDensity);
	m_pDoc->Option(XSHEET_HIGHLIGHT, 1,m_bSheet);
	m_pDoc->Option(SC_MRU,1, m_nKind);
	m_pDoc->Option(SC_NEXT,1, m_bShowNext);
	m_pDoc->Option(SC_PREV,1, m_bShowPrev);
	m_pDoc->Option(SC_NEXT1,1, m_bShowNext1);
	m_pDoc->Option(SC_PREV1,1, m_bShowPrev1);
	m_pDoc->Option(SC_PAINT,1, m_bShowPaint);
	m_pDoc->Option(SC_LIGHT_PAINT,1, m_bShowLPaint);
	m_pScene->SceneOptionInt(SCOPT_BUD0,1, m_bShowBud0 ? m_nBuddy0 : 0);
	m_pScene->SceneOptionInt(SCOPT_BUD1,1, m_bShowBud1 ? m_nBuddy1 : 0);
	m_pScene->SceneOptionInt(SCOPT_BUD2,1, m_bShowBud2 ? m_nBuddy2 : 0);
	m_pScene->SceneOptionInt(SCOPT_BUD3,1, m_bShowBud3 ? m_nBuddy3 : 0);
	m_pDoc->Option(FIRST_OPTION,2);//OPT_HOLD,1);
}

void CLightOptions::OnApply()
{
	if (UpdateData(TRUE))
		{
		m_bPegShowFG = 1;
		ApplySettings();
		}
}

void CLightOptions::OnOK()
{
	if (!UpdateData(TRUE))
		return;
	m_bPegShowFG = 1;
	ApplySettings();
	CMyDialog::OnOK();
}

void CLightOptions::OnCancel() 
{
	DPF("on cancel");
	if (m_bDirty)
		{
		m_nKind = m_Kind;
		m_wPegMaxDepth = m_Depth;
		m_wPegMinDensity = m_Den;
		m_wBGMinDark = m_Dark;
		m_bPegShowBG = m_BG;
		m_bPegShowFG = m_FG;
		m_bShowNext = m_Prev;
		m_bShowPrev = m_Next;
		m_bShowNext1 = m_Prev1;
		m_bShowPrev1 = m_Next1;
		m_bShowPaint = m_Paint;
		m_bShowLPaint = m_LPaint;
		m_bSheet = m_Sheet;
		m_nBuddy0 = m_bud0;
		m_nBuddy1 = m_bud1;
		m_nBuddy2 = m_bud2;
		m_nBuddy3 = m_bud3;
		m_bShowBud0 = m_nBuddy0 ? 1 : 0;
		m_bShowBud1 = m_nBuddy1 ? 1 : 0;
		m_bShowBud2 = m_nBuddy2 ? 1 : 0;
		m_bShowBud3 = m_nBuddy3 ? 1 : 0;
		if (!m_bShowBud0) m_nBuddy0 = 1;
		if (!m_bShowBud1) m_nBuddy1 = 1;
		if (!m_bShowBud2) m_nBuddy2 = 1;
		if (!m_bShowBud3) m_nBuddy3 = 1;
		ApplySettings();
		}
	CMyDialog::OnCancel();
}

