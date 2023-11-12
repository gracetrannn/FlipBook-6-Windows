#include "stdafx.h"
#include "sketch.h"
#include "mydoc.h"
#include "myview.h"
//#include "sheet.h"
//#include "canvas.h"
//#include "flip.h"
#include "mainfrm.h"
#include "optdlg.h"
#include "sceneopt.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


void CSketchDoc::OptionString(int Id, LPSTR pStr, int op /* = 0 */)
{
	CString string;
	if (!string.LoadString(Id))
		return;
	if (op)
		AfxGetApp()->WriteProfileString("Options",(LPCSTR)string, pStr);
	else
		{
		CString temp =
		AfxGetApp()->GetProfileString("Options",(LPCSTR)string, "");
		strcpy(pStr, (LPCSTR)temp);
		}
}

void CSketchDoc::SceneDefaults()
{
	m_bOptHold = 2;		// so read from memory
	m_pScene->SceneOptionLock(1); // postpone writing of option record
	m_pScene->SceneOptionInt(SCOPT_SOUND, Option(SC_SOUND));
	m_pScene->SceneOptionInt(SCOPT_QUIET, Option(SC_QUIET));
	m_pScene->SceneOptionInt(SCOPT_SNIP, Option(SC_SNIP));
	m_pScene->SceneOptionInt(SCOPT_BLIND, Option(SC_BLIND));
	m_pScene->SceneOptionInt(SCOPT_SCACHE, Option(SC_SCACHE));
	m_pScene->SceneOptionInt(SCOPT_TELECINE, Option(SC_TELECINE));
	m_pScene->SceneOptionInt(SCOPT_MRU, Option(SC_MRU));
	m_pScene->SceneOptionInt(SCOPT_NEXT, Option(SC_NEXT));
	m_pScene->SceneOptionInt(SCOPT_PREV, Option(SC_PREV));
	m_pScene->SceneOptionInt(SCOPT_NEXT1, Option(SC_NEXT1));
	m_pScene->SceneOptionInt(SCOPT_PREV1, Option(SC_PREV1));
	m_pScene->SceneOptionInt(SCOPT_PAINT, Option(SC_PAINT));
	m_pScene->SceneOptionInt(SCOPT_LIGHT_PAINT, Option(SC_LIGHT_PAINT));
	m_pScene->SceneOptionInt(SCOPT_EMBED_OUT, Option(SC_EMBED_OUT));
	m_pScene->SceneOptionInt(SCOPT_EMBED_IN, Option(SC_EMBED_IN));
	m_pScene->SceneOptionInt(SCOPT_EMBED_TRIM, Option(SC_EMBED_TRIM));
	m_pScene->SceneOptionInt(SCOPT_APPLY_FRAME, Option(SC_APPLY_FRAME));
//	m_pScene->OptionLock(0); // make will do it
	m_bOptHold = 0;
}

void CSketchDoc::OptionLoadSave(BOOL bSave)
{
	int i,v;
	CString string;
	if (!bSave)
		{
		Option(FIRST_OPTION); // OPT_HOLD);
		m_bWinTab = TRUE;//FALSE;
		m_bTabletUsed = FALSE;
		m_bLeftScroll = FALSE;
		m_wBGMinDark = 50;
		m_bPegShowBG = TRUE;
		m_nPegShowFG = TRUE;
		m_bLeftHanded = FALSE;
		m_bToolsSameColor = TRUE;
		m_bSimpleCursor = TRUE;
		m_bXSheetHighlight = TRUE;
//		m_bPegStack = TRUE;
		m_bShowThumbs = 1;
		m_bCheckerBG = 0;
		m_bHardZoom = 0;
		m_nScanRotate = 0;
		m_nPrintOpts = 0;
		m_bScaleBG = 1;
		m_wPegMaxDepth = 5;
		m_wPegMinDensity = 50;
/*
		m_Pen.MinRadius = 1;
		m_Pen.MaxRadius = 30;
		m_Pen.MinDensity = 5;
		m_Pen.MaxDensity = 100;

		m_Pencil.MinRadius = 1;
		m_Pencil.MaxRadius = 30;
		m_Pencil.MinDensity = 5;
		m_Pencil.MaxDensity = 100;

		m_Eraser.MinRadius = 1;
		m_Eraser.MaxRadius = 30;
		m_Eraser.MinDensity = 5;
		m_Eraser.MaxDensity = 100;
*/
		m_bSheetShowBG = TRUE;
		m_bFlipFew = TRUE;
		m_bAutoKeep = TRUE;
		m_bLoopPlay = FALSE;
		m_bTabPrimary = FALSE;
		m_wMaxPress = 255;
		m_wMinPress = 0;
		m_def_width = 640;
		m_def_height = 480;
		m_def_frames = 10;
		m_def_levels = 1;
		m_def_rate = 24;
		m_def_factor = 0;
		m_def_preview = 0;
		m_def_broadcast = 1;
		m_def_kind = 1;
		m_broadcast = 1;
		m_vidcapx = 0;
		m_vidcapy = GetSystemMetrics(SM_CYSCREEN) - 180;
		m_vidcapopts = 0;
		m_vidcapwht = 250;
		m_vidcapred = 255;
		m_vidcapgreen = 255;
		m_vidcapblue = 255;
		m_vidcapextent = 100;
		m_vidcapwidth = 640;
		m_vidcapheight = 480;
		m_vidcapgam = 100;
		m_defaulthold = 2;
		m_nScanFieldSize = 8000;
		m_nScanOffset = 5000;
		m_bPegsTop  = 0;
		m_bCropPegs  = 1;
		m_exteditopts = 255;
		m_scSound = 0;		// scene options
		m_scQuiet = 0;
		m_scSnip = 3;
		m_scBlind = 0;
		m_scCache = 0;
		m_scMRU = 0;
		m_scTelecine = 0;
		m_scPrev = 1;
		m_scNext = 1;
		m_scPrev1 = 0;
		m_scNext1 = 0;
		m_scPaint = 0;
		m_scLPaint = 0;
		m_scAviScale = 0;
		m_nJpeg = 0;
		m_bUseMouths = 0;
		m_bEditScrub = 0;
		m_scEmbOut = 0;
		m_scEmbIn = 0;
		m_scEmbTrim = 0;
		m_bRecVideo = 1;
		m_bRecAudio = 1;
		m_bCapCC = 0;
		m_bRecPreview = 1;
		m_bRecErase = 0;
		m_nRecSize = 300;
		m_nRecTimer = 10;
		m_nRecDelay = 5;
		m_nRecMaster = 2;
		m_recordx = 0;
		m_recordy = GetSystemMetrics(SM_CYSCREEN) - 180;
		m_oflags = 0;
		}
	for (i = FIRST_OPTION + 1; i <= LAST_OPTION ; i++)
		{
		if (i == CAPTURE_DEVICE)
			continue;
		if ((i == REC_VNAME) || (i == REC_ANAME) ||
					(i == REC_CNAME) || (i == REC_FILE))
			continue;
		if (!string.LoadString(i))
			continue;
		v = Option(i);
		if (bSave)
			{
			AfxGetApp()->WriteProfileInt("Options", (LPCSTR)string, v);
			}
		else
			{
			v = AfxGetApp()->GetProfileInt("Options",(LPCSTR)string, v);
			Option(i,1,v);
			}
		}
	if (!bSave)
		Option(FIRST_OPTION,1); //OPT_HOLD,1);
}

int	CSketchDoc::ScOption(UINT & a,int Id, int op, int val)
{
	if (!m_pScene || (m_bOptHold == 2))
		return a;
	if (!op)
		return m_pScene->SceneOptionInt(Id);
	return a = m_pScene->SceneOptionInt(Id,op,val);
}

#define DOIT(a) oldval=a;if (op == 2) a ^= 1; else if (op == 1) a=val; val=a
#define SDOIT(a,b) return ScOption(a,b,op,val);

int	CSketchDoc::Option(int Id, int op, int val)
{
//  if op = 0 return value
// if op = 2 toggle
// if op = 1 set it
//DPF("option,id:%d,op:%d,val:%d",Id,op,val);
	int oldval;
//
//	mask 1 -> redraw canvas
//		2 -> redraw thumb
//
	BOOL bUpdate = 0;
	int mask = 0;
	switch (Id) {
	case FIRST_OPTION: //OPT_HOLD:
		if (!op)
			{
			m_bOptHold = 1;
			m_option_flags = 0;
			}
		else
			{
			if (op == 2)
				m_option_flags |= 33;
			m_bOptHold = 0;
			}
		break;
	case WINTAB:
		DOIT(m_bWinTab);
		if (op)
			mask = 8;
		break;
	case TABLET_USED:
		DOIT(m_bTabletUsed);
//		if (op)
//			mask = 8;
		break;
	case LEFT_SCROLL:
		DOIT(m_bLeftScroll);
		if (op)
			mask = 128 + 3;
		break;
	case TAB_PRIMARY:
		DOIT(m_bTabPrimary);
		if (op)
			mask = 8;
		break;
	case LEFTHAND:
		DOIT(m_bLeftHanded);
		mask = 1;
		break;
	case FLIP_FEW:
		DOIT(m_bFlipFew);
		mask = 1;
		break;
	case AUTOKEEP:
		DOIT(m_bAutoKeep);
		mask = 16;
		break;
	case LOOPPLAY:
		DOIT(m_bLoopPlay);
		break;
	case BGMIN:
		DOIT(m_wBGMinDark);
		if (m_pScene && (m_wBGMinDark != oldval))
			m_pScene->SetMinBG(m_wBGMinDark);
		mask = 3;
		break;
	case PEG_SHOWBG:
		DOIT(m_bPegShowBG);
		mask = 1;
		break;
	case PEG_SHOWFG:
		int bit;
		if (m_pScene && m_pScene->ColorMode())
			bit = 2;
		else
			bit = 1;
		oldval = (m_nPegShowFG & bit) / bit;
		if (op == 2)
			m_nPegShowFG ^= bit;
		else if (op == 1)
			{
			m_nPegShowFG |= bit;
			if (!val)
				m_nPegShowFG ^= bit;
			}
		val = (m_nPegShowFG & bit) / bit;
		mask = 33;
		break;
		/*
	case ZSTACK:
		DOIT(m_bPegStack);
		if (op)
			{
			CMainFrame * pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
	//			if (!m_bPegStack)
	//				pFrame->PegCount(TRUE);
	//				pFrame->InitPegDepth(m_bPegStack == 0);
			}
		if (op)
		mask = 3;
		break;
		*/
//	case CAPTURE_DEVICE:
//		DOIT(m_szCapDevice);
//		break;
	case SHOWTHUMBS:
		DOIT(m_bShowThumbs);
		mask = 2;
		break;
	case PEG_DEPTH:
		DOIT(m_wPegMaxDepth);
		if (m_wPegMaxDepth > 5) m_wPegMaxDepth = 5;
		mask = 3;
		break;
	case PEG_DENSITY:
		DOIT(m_wPegMinDensity);
		mask = 1;
		break;
		/*
	case PEN_MINRAD:
		DOIT(m_Pen.MinRadius);
		break;
	case PEN_MAXRAD:
		DOIT(m_Pen.MaxRadius);
		break;
	case PNC_MINDEN:
		DOIT(m_Pencil.MinDensity);
		break;
	case PNC_MAXDEN:
		DOIT(m_Pencil.MaxDensity);
		break;
	case ERS_MINDEN:
		DOIT(m_Eraser.MinDensity);
		break;
	case ERS_MAXDEN:
		DOIT(m_Eraser.MaxDensity);
		break;
		*/
	case MAX_PRESSURE:
		DOIT(m_wMaxPress);
		break;
	case MIN_PRESSURE:
		DOIT(m_wMinPress);
		break;
	case SHT_SHOWBG:
		DOIT(m_bSheetShowBG);
		mask = 2;
		break;
	case DEF_WIDTH:
		DOIT(m_def_width);
		break;
	case DEF_HEIGHT:
		DOIT(m_def_height);
		break;
	case DEF_FRAMES:
		DOIT(m_def_frames);
		break;
	case DEF_LEVELS:
		DOIT(m_def_levels);
		break;
	case DEF_RATE:
		DOIT(m_def_rate);
		break;
	case DEF_FACTOR:
		DOIT(m_def_factor);
		break;
	case JPEG_QUAL:
		DOIT(m_nJpeg);
		break;
	case DEF_PREVIEW:
		DOIT(m_def_preview);
		break;
	case DEF_BROADCAST:
		DOIT(m_def_broadcast);
		break;
	case CHECKERBG:
		DOIT(m_bCheckerBG);
		mask = 1;
		break;
	case HARD_ZOOM:
		DOIT(m_bHardZoom);
		mask = 1;
		break;
	case SCANROTATE:
		DOIT(m_nScanRotate);
		break;
	case SCAN_OFFSET:
		DOIT(m_nScanOffset);
		break;
	case SCAN_FIELD:
		DOIT(m_nScanFieldSize);
		break;
	case SCAN_PEGTOP:
		DOIT(m_bPegsTop);
		break;
	case SCAN_CROPPEG:
		DOIT(m_bCropPegs);
		break;
	case PRINTOPTS:
		DOIT(m_nPrintOpts);
		break;
	case XSCALE_BG:
		DOIT(m_bScaleBG);
		break;
	case DEF_KIND:
		DOIT(m_def_kind);
		break;
	case VIDCAP_X:
		DOIT(m_vidcapx);
		break;
	case VIDCAP_Y:
		DOIT(m_vidcapy);
		break;
	case VIDCAP_OPTS:
		DOIT(m_vidcapopts);
		break;
	case VIDCAP_WHITE:
		DOIT(m_vidcapwht);
		break;
	case VIDCAP_RED:
		DOIT(m_vidcapred);
		break;
	case VIDCAP_GREEN:
		DOIT(m_vidcapgreen);
		break;
	case VIDCAP_BLUE:
		DOIT(m_vidcapblue);
		break;
	case VIDCAP_EXTENT:
		DOIT(m_vidcapextent);
		break;
	case VIDCAP_WIDTH:
		DOIT(m_vidcapwidth);
		break;
	case VIDCAP_HEIGHT:
		DOIT(m_vidcapheight);
		break;
	case VIDCAP_GAMMA:
		DOIT(m_vidcapgam);
		break;
//	case DEF_HOLD:
//		DOIT(m_defaulthold);
//		if (m_pScene && (op == 1)) 
//			m_pScene->SceneOption(SCOPT_HOLD,1,m_defaulthold);
//		break;
	case TOOLSAME:
		DOIT(m_bToolsSameColor);
		break;
	case SELECT_FLAGS:
		DOIT(m_select_flags);
		break;
	case SIMPLE_CURSOR:
		mask = 1;
		DOIT(m_bSimpleCursor);
		break;
	case EXTEDIT_OPTS:
		DOIT(m_exteditopts);
		break;
	case XSHEET_HIGHLIGHT:
		DOIT(m_bXSheetHighlight);
		mask = 64;// force xsheet redraw
		break;
	case USE_MOUTHS:
		DOIT(m_bUseMouths);
		break;
	case EDIT_SCRUB:
		DOIT(m_bEditScrub);
		break;
	case DEF_HOLD:
		SDOIT(m_defaulthold, SCOPT_HOLD);
//	case SC_RATE:
//	case SC_BGMIN:
	case SC_SOUND:
		SDOIT(m_scSound, SCOPT_SOUND);
	case SC_QUIET:
		SDOIT(m_scQuiet,SCOPT_QUIET);
	case SC_SNIP:
		SDOIT(m_scSnip, SCOPT_SNIP);
	case SC_BLIND:
		SDOIT(m_scBlind, SCOPT_BLIND);
//	case SC_STORY:
//	case SC_SRATE:
	case SC_SCACHE:
		SDOIT(m_scCache, SCOPT_SCACHE);
	case SC_TELECINE:
		SDOIT(m_scTelecine, SCOPT_TELECINE);
	case SC_MRU:
		SDOIT(m_scMRU, SCOPT_MRU);
	case SC_NEXT:
		SDOIT(m_scNext, SCOPT_NEXT);
	case SC_PREV:
		SDOIT(m_scPrev, SCOPT_PREV);
	case SC_NEXT1:
		SDOIT(m_scNext1, SCOPT_NEXT1);
	case SC_PREV1:
		SDOIT(m_scPrev1, SCOPT_PREV1);
	case SC_PAINT:
		SDOIT(m_scPaint, SCOPT_PAINT);
	case SC_LIGHT_PAINT:
		SDOIT(m_scLPaint, SCOPT_LIGHT_PAINT);
	case SC_AVI_SCALE:
		SDOIT(m_scAviScale, SCOPT_AVI_SCALE);
	case SC_EMBED_OUT:
		SDOIT(m_scEmbOut, SCOPT_EMBED_OUT);
	case SC_EMBED_IN:
		SDOIT(m_scEmbIn, SCOPT_EMBED_IN);
	case SC_EMBED_TRIM:
		SDOIT(m_scEmbTrim, SCOPT_EMBED_TRIM);
	case SC_APPLY_FRAME:
		SDOIT(m_scApplyFrame, SCOPT_APPLY_FRAME);

	case REC_CAP_VID:
		DOIT(m_bRecVideo);
		break;
	case REC_CAP_AUD:
		DOIT(m_bRecAudio);
		break;
	case REC_CAP_CC:
		DOIT(m_bCapCC);
		break;
	case REC_PREVIEW:
		DOIT(m_bRecPreview);
		break;
	case REC_ERASE:
		DOIT(m_bRecErase);
		break;
	case REC_SIZE:
		DOIT(m_nRecSize);
		break;
	case REC_TIME:
		DOIT(m_nRecTimer);
		break;
	case ZREC_MASTER:
		DOIT(m_nRecMaster);
		break;
	case REC_DELAY:
		DOIT(m_nRecDelay);
		break;
	case RECORD_X:
		DOIT(m_recordx);
		break;
	case RECORD_Y:
		DOIT(m_recordy);
		break;
	case OVERRIDE_FLAGS:
		mask = 2 + 4;
		DOIT(m_oflags);
		break;
	}
	if (mask)
		{
		if (op && (val != oldval))
			m_option_flags |= mask;
		}
	if (!m_bOptHold && m_option_flags)
		{
		if (m_option_flags & 19)
			bUpdate = 1;
		if (m_option_flags & 64)
			{
			m_option_flags &= (~64);
			CMainFrame * pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
			if (pFrame)
				pFrame->StackUpdate(TRUE); // jsut redraw
			}
		if (m_option_flags & 32)
			{
			m_option_flags &= (~32);
			CMainFrame * pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
			if (pFrame)
				{
				UINT h = m_bOptHold;
				UINT f = m_option_flags;
				pFrame->StackUpdate();
				m_bOptHold = h;
				m_option_flags = f;
				}
			}
		if (m_option_flags & 128)
			{
			m_option_flags ^= 128;
			CMainFrame * pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
			if (pFrame)
				pFrame->SetLeftScroll();
			}
		if (m_option_flags & 1)
			{
			m_option_flags ^= 1;
			CSketchView* pSketchView = GetDocView();
			if (pSketchView != NULL)
				pSketchView->Redraw();
			}
		if (m_option_flags & 2)
			{
			m_option_flags ^= 2;
			CMainFrame * pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
			if (pFrame)
				pFrame->RedrawSheet();
			}
		if (m_option_flags & 4)
			{
			m_option_flags ^= 4;
			if (m_pScene)
				m_pScene->UpdateCache();
			}
		if (m_option_flags & 8)
			{
			m_option_flags ^= 8;
			CSketchView* pSketchView = GetDocView();
			if (pSketchView != NULL)
				pSketchView->InitTablet();
			}
		m_option_flags = 0;
		}
	return val;
}

void CSketchDoc::DlgOptions()
{
	DPF("dlgoptions");
	COptions Options;
	Options.m_bUseWinTab = 
		((CSketchApp *)AfxGetApp())->HaveWinTab() ? Option(WINTAB) : 0;
	Options.m_bTabletUsed = Option(TABLET_USED);
	Options.m_bLeftScroll = Option(LEFT_SCROLL);
	Options.m_bTabPrimary = Option(TAB_PRIMARY);
	Options.m_bLeftHanded = Option(LEFTHAND);
	Options.m_bToolsSameColor = Option(TOOLSAME);
	Options.m_bSimpleCursor = Option(SIMPLE_CURSOR);
//	Options.m_bTabPressure = Option(TAB_PRESSURE);
//	Options.m_wBGMinDark = Option(BGMIN);
//	Options.m_bBGShow = Option(SHT_SHOWBG);
//	Options.m_bPegShowBG = Option(PEG_SHOWBG);
//	Options.m_bPegShowFG = Option(PEG_SHOWFG);
//	Options.m_wPegMaxDepth = Option(PEG_DEPTH);
//	Options.m_wPegMinDensity = Option(PEG_DENSITY);

	Options.m_wPenMinRadius = Option(PEN_MINRAD);
	Options.m_wPenMaxRadius = Option(PEN_MAXRAD);
	Options.m_wPencilMinDensity= Option(PNC_MINDEN);
	Options.m_wPencilMaxDensity = Option(PNC_MAXDEN);
	Options.m_wEraserMinDensity= Option(PNC_MINDEN);
	Options.m_wEraserMaxDensity = Option(PNC_MAXDEN);
	int v = Option(MAX_PRESSURE);
	Options.m_wMaxPress = v % 1000; // for version 6.00 thru 6.05
	if (v > 1000)
		Options.m_wMinPress = v / 1000;
	else
		Options.m_wMinPress = Option(MIN_PRESSURE);
	Options.m_bFlipFew = Option(FLIP_FEW);
	Options.m_bAutoKeep = Option(AUTOKEEP);
	Options.m_bLoopPlay = Option(LOOPPLAY);
	Options.m_bCheckerBG = Option(CHECKERBG);
	Options.m_bHardZoom = Option(HARD_ZOOM);
	Options.m_bUseMouths = Option(USE_MOUTHS);
	Options.m_bEditScrub = Option(EDIT_SCRUB);
	Options.m_nJpeg = Option(JPEG_QUAL);
//	Options.m_nJpeg = m_pSceneOption(JPEG_QUAL);
//	Options.m_bScaleBG = Option(SCALE_BG);
	if (m_pScene)
		{
		Options.m_broadcast = m_pScene->Broadcast();
		Options.m_rate = m_pScene->FrameRate();
		Options.m_bSaveCache = m_pScene->SceneOptionInt(SCOPT_SCACHE);
		Options.m_nJpeg = m_pScene->JpegQuality();
		}
	else
		{
		Options.m_broadcast = 1;
		Options.m_rate = 24;
		Options.m_bSaveCache = 0;
		}
	Options.m_bOpenPrevious = 
			AfxGetApp()->GetProfileInt("Options","Open Previous", 0);
	Options.m_bChkUpdates = 
			AfxGetApp()->GetProfileInt("Options","Check Updates", 0);
	Options.m_bSheetOnTop = 
			AfxGetApp()->GetProfileInt("Options","XSheet On Top", 0);

	int result = Options.DoModal();
	DPF("res:%d",result);
	if (result == IDOK)
		{
		Option(FIRST_OPTION);//OPT_HOLD);
		if (((CSketchApp *)AfxGetApp())->HaveWinTab())
			Option(WINTAB,1,Options.m_bUseWinTab);
		Option(LEFTHAND,1,Options.m_bLeftHanded);
		Option(TOOLSAME,1,Options.m_bToolsSameColor);
		Option(SIMPLE_CURSOR,1,Options.m_bSimpleCursor);
		Option(TAB_PRIMARY,1,Options.m_bTabPrimary);
//		Option(BGMIN, 1,Options.m_wBGMinDark);
		Option(SHT_SHOWBG, 1, Options.m_bBGShow);
//		Option(PEG_SHOWBG, 1, Options.m_bPegShowBG);
//		Option(PEG_SHOWFG, 1, Options.m_bPegShowFG);
//		Option(PEG_DEPTH, 1, Options.m_wPegMaxDepth);
//		Option(PEG_DENSITY, 1,Options.m_wPegMinDensity);

		Option(PEN_MINRAD, 1, Options.m_wPenMinRadius);
		Option(PEN_MAXRAD,1, Options.m_wPenMaxRadius);
		Option(PNC_MINDEN,1, Options.m_wPencilMinDensity);
		Option(PNC_MAXDEN,1, Options.m_wPencilMaxDensity);
		Option(ERS_MINDEN,1, Options.m_wEraserMinDensity);
		Option(ERS_MAXDEN,1, Options.m_wEraserMaxDensity);
		Option(MAX_PRESSURE,1, Options.m_wMaxPress);
		Option(MIN_PRESSURE,1, Options.m_wMinPress);
		Option(FLIP_FEW,1, Options.m_bFlipFew);
		Option(AUTOKEEP,1, Options.m_bAutoKeep);
		Option(CHECKERBG,1, Options.m_bCheckerBG);
		Option(HARD_ZOOM,1, Options.m_bHardZoom);
		Option(USE_MOUTHS,1,Options.m_bUseMouths);
		Option(EDIT_SCRUB,1,Options.m_bEditScrub);
		Option(JPEG_QUAL,1,Options.m_nJpeg);
		Option(LEFT_SCROLL,1, Options.m_bLeftScroll);
//		Option(SCALE_BG,1, Options.m_bScaleBG);
		Option(FIRST_OPTION,1);//OPT_HOLD,1);
		CMainFrame * pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
		if (pFrame)
			pFrame->SetFrameRate(Options.m_rate);
		if (m_pScene)
			{
			m_pScene->Broadcast(Options.m_broadcast);
//			m_pScene->SceneOption(SCOPT_SCACHE,1,Options.m_bSaveCache);
			Option(SC_SCACHE,1,Options.m_bSaveCache);
			m_pScene->JpegQuality(Options.m_nJpeg);
			}
		AfxGetApp()->WriteProfileInt("Options",
					"Open Previous", Options.m_bOpenPrevious);
		AfxGetApp()->WriteProfileInt("Options",
					"Check Updates", Options.m_bChkUpdates);
		AfxGetApp()->WriteProfileInt("Options",
					"XSheet On Top", Options.m_bSheetOnTop);
		}
}

VOID CSketchDoc::DoIt(int Kind, BOOL bPut, LPVOID lpData, LPSTR lpKey,
			BOOL bGray)
{
	char tmpStr[200];
	char szSection[30];
	WORD i,f;
	DWORD dwTemp;

	if (bGray)
		strcpy(szSection,"Gray AVI Options");
	else
		strcpy(szSection,"Color AVI Options");
//DPF("doit,%s,%s,G/P:%d,Gray:%d",szSection,lpKey,bPut,bGray);
	if (bPut)
		{
		switch (Kind)
			{
			case DOIT_BOOL:
				sprintf(tmpStr, "%u", *((WORD *)lpData));
				break;
			case DOIT_LONG:
				sprintf(tmpStr, "%8lx", *((long *)lpData));
				break;
			case DOIT_FLOAT:
				sprintf(tmpStr, "%f", *((float *)lpData));
				break;
			case DOIT_STRING:
					sprintf(tmpStr, "%s", (char *)lpData);
				break;
			default:
			case DOIT_WORD:
				sprintf(tmpStr, "%u", *((WORD *)lpData));
				break;
			}
//DPF("put:%s=%s",lpKey,tmpStr);
		AfxGetApp()->WriteProfileString(szSection, lpKey, tmpStr);
		}
	else
		{
		CString temp = AfxGetApp()->GetProfileString(
				szSection,lpKey, "");
		strcpy(tmpStr, (LPCSTR)temp);
//DPF("get:%s=%s",lpKey,tmpStr);
		switch (Kind)
			{
			case DOIT_FLOAT:
				if (tmpStr[0])
					*((float *)lpData) = (float)atof(tmpStr);
				break;
			case DOIT_STRING:
				if (tmpStr[0])
					strcpy((LPSTR)lpData, tmpStr);
				break;
			default:
				DPF("bad kind:%d",Kind);
			case DOIT_WORD:
			case DOIT_LONG:
			case DOIT_BOOL:
			case DOIT_BYTE:
				if (!tmpStr[0])
					break;
				if (Kind == DOIT_LONG)
					f = 16;
				else
					f = 10;
				for (i = 0, dwTemp = 0; tmpStr[i]; i++)
					{
					dwTemp *= f;
					if ((tmpStr[i] >= '0') && (tmpStr[i] <= '9'))
						dwTemp += tmpStr[i] & 15;
					else if (f == 16)
						dwTemp += 9 + (tmpStr[i] & 7);
					}
//DPF("getlong:%s=%lx",lpKey,dwTemp);
				if (Kind == DOIT_LONG)
					*((DWORD *)lpData) = dwTemp;
				else if (Kind == DOIT_WORD)
					*((WORD *)lpData) = (WORD)dwTemp;
				else if (Kind == DOIT_BYTE)
					*((BYTE *)lpData) = (BYTE)dwTemp;
				else
					*((BOOL *)lpData) = (BOOL)dwTemp;
				break;
			}
		}
}


void CSketchDoc::DlgLightBox()
{
	CLightOptions dlg;
	dlg.m_pDoc = this;
	dlg.DoModal();
}
