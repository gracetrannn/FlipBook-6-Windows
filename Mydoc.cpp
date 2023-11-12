#include "stdafx.h"
#include "sketch.h"
#include "mydoc.h"
#include "myio.h"
#include "myview.h"
//#include "sheet.h"
//#include "canvas.h"
//#include "flip.h"
#include "mainfrm.h"
#include "newdlg.h"
#include "direct.h"
#include "scan.h"
#include "sceneopt.h"
#include "dialogs.h"
#include "dirdlg.h"
#include "dib.h"
#include "mysound.h"
#include "cmaya.h"
//#include "shfolder.h"
#include "shlobj.h"
#include "fbqt.h"
#include "ccell.h"
#include "zlib.h"
#include "cnewpals.h"
#include "camera.h"
#include "WinResourceLoader.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

void extern PutInt(LPCSTR section, LPCSTR key, int v);
int  extern GetInt(LPCSTR section, LPCSTR key, int def);
//#define DOJASON
//#define TOOL_MASK 2047
#define TOOL_MASK 16383
#define NBR_TOOLS 9
class CTool
{
public:
	UINT	Kind;
	UINT	Flags; // 2 can erase, 1 in tool 0 is erasing
	UINT	Radius;
	UINT	Density;
	UINT	Color;
	UINT	MaxRadius;
	UINT	MinRadius;
	UINT	MaxDensity;
	UINT	MinDensity;
	UINT	Previous;	// used by trace and hand
	UINT	eRadius;
	UINT	eDensity;
	UINT	eColor;
void Init(UINT Kind, UINT Radius = 2, UINT Density = 100, UINT Color = 0);
};

void CTool::Init(UINT kind, UINT radius, UINT density, UINT color)
{
	Kind = kind;
	Radius = radius;
	Density = density;
	Color = color;
	Previous = 0;
	Flags = 0;
	MaxRadius = 15;
	MinRadius = 1;
	MaxDensity = 100;
	MinDensity = 0;
	eRadius = radius;
	eDensity = density;
	eColor = 255;
	if (kind == 0)
		Flags = 4 * (8+4) + 2;
	else if (kind == 1 || kind == 2 || kind == 3)
		Flags = 2;		// allow erase
	if (kind == 3)
		Radius = 0;
}


//#define KEY_CLIP 13
/////////////////////////////////////////////////////////////////////////////
// CSketchDoc

IMPLEMENT_DYNCREATE(CSketchDoc, CDocument)

BEGIN_MESSAGE_MAP(CSketchDoc, CDocument)
	//{{AFX_MSG_MAP(CSketchDoc)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateSave)
	ON_UPDATE_COMMAND_UI(ID_FILE_REVERT, OnUpdateRevert)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	ON_COMMAND(ID_FILE_IMPAVI, OnImportAVI)
#ifdef FLIPBOOK_MAC
	ON_COMMAND(ID_FILE_IMPMOV, OnImportMovie)
#endif
	ON_COMMAND(ID_FILE_IMPFILE, OnImportFile)
	ON_COMMAND(ID_FILE_EXPORT, OnExport)
	ON_COMMAND(ID_FILE_EDIT, OnExtEdit)
	ON_UPDATE_COMMAND_UI(ID_FILE_EDIT, OnUpdExtEdit)
//	ON_COMMAND(ID_FILE_EXPAVI, OnExportAVI)
//	ON_COMMAND(ID_FILE_EXPSWF, OnExportSWF)
	ON_COMMAND(ID_FILE_PUBLISH, OnPublish)
	ON_COMMAND(ID_FILE_SCAN, OnScan)
	ON_COMMAND(ID_FILE_REVERT, OnFileRevert)
	ON_COMMAND(ID_FILE_IMPORT_MAYA, OnImportMaya)
	ON_COMMAND(ID_EXPORT_MAYA, OnExportMaya)
	ON_COMMAND(ID_FILE_EXPORT_PAL, OnExportPalette)
	ON_COMMAND(ID_FILE_IMPORT_PAL, OnImportPalette)
	ON_COMMAND(ID_FILE_EXPORT_CAM, OnExportCamera)
	ON_COMMAND(ID_FILE_IMPORT_CAM, OnImportCamera)
	ON_COMMAND(ID_FILE_EXPORT_EMB, OnExportEmbed)
	ON_COMMAND(ID_OPT_EMBEDDED, OnEmbOptions)
	ON_UPDATE_COMMAND_UI(ID_FILE_EXPORT_EMB, OnUpExpEmbed)
	ON_COMMAND(ID_OPT_FRAME_NBR, OnFrameNumber)

	ON_COMMAND_RANGE(ID_OPT_OVERRIDE_HOLD,ID_OPT_OVERRIDE_EMBED,OnOverride)
	ON_UPDATE_COMMAND_UI_RANGE(ID_OPT_OVERRIDE_HOLD,ID_OPT_OVERRIDE_EMBED,
				OnUpdateOverride)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSketchDoc construction/destruction

CSketchDoc::CSketchDoc()
{
	m_nBusy = 0;
	m_bMayaClosed = 0;
	m_pScene = NULL;
	m_pIO = NULL;
//	m_pClipBoard = 0;
// m_pCanvas = new CCanvas;
//	m_pFlip = new CFlip;
//	m_mode = 9999;
//	m_wSelect = 0;
	m_select_flags = 0;
	m_pTools = new CTool[NBR_TOOLS+1];
		m_pTools[0].Init(0);
		m_pTools[1].Init(1);
		m_pTools[2].Init(2);
		m_pTools[3].Init(3);
		m_pTools[4].Init(4);
		m_pTools[5].Init(5);
		m_pTools[6].Init(6);
		m_pTools[7].Init(8);
		m_pTools[8].Init(9);
		m_pTools[9].Init(10);
	m_bOpened = FALSE;
	m_bDoErase = FALSE;
	m_bLeftScroll = 0;
	m_nNeedHelp = 0;
DPF("before options");
	OptionLoadSave();
DPF("after options");
}

CSketchDoc::~CSketchDoc()
{
#ifndef FLIPBOOK_MAC
	int a,b;
	a = GetSystemMetrics(SM_CXCURSOR);
	b = GetSystemMetrics(SM_CYCURSOR);
	DPF("my doc destruct,%d,%d",a,b);
#endif
	OptionLoadSave(TRUE);
#ifdef MYBUG
if (m_pScene && m_pScene->Modified())
	{
AfxMessageBox("doc destruct scene not closed");
	}
#endif
//	ClipEmpty(1);
	delete m_pScene;
	delete m_pTools;
	delete m_pIO;
}

void CSketchDoc::OnUpdateSave(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_pIO->ReadOnly());
}

void CSketchDoc::OnUpdateRevert(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_bDoErase);
}

void CSketchDoc::OnUpdExtEdit(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_pScene && m_pScene->ColorMode());
}

void CSketchDoc::OnFileRevert()
{
	CString current = GetPathName();
	// are you sure
	DeleteContents();
	if (m_pIO)
		{
		m_pIO->Wipe(1);
		m_pIO->Close();
		}
	SetPathName("temp", 0);
	AfxGetApp()->OpenDocumentFile(current);
}

void CSketchDoc::OnExportEmbed()
{
	CExpEmbedDlg dlg;
	dlg.m_pIO = m_pIO;
	dlg.DoModal();
}

void CSketchDoc::OnUpExpEmbed(CCmdUI* pCmdUI)
{	
	BOOL bHaveSome = 0;
	if (m_pIO && m_pIO->EmbFlag(EMB_ANY_IN_MEMORY,0))
		bHaveSome = 1;
	pCmdUI->Enable(bHaveSome);
	return;
}

void CSketchDoc::OnEmbOptions()
{
	CEmbOptionsDlg dlg;
	int v = Option(OVERRIDE_FLAGS);
	dlg.m_bOverride = (v & 4) ? 1 : 0;
	int iv, ov;
	if (dlg.m_bOverride)
		{
		iv = (v >> 4) & 3;
		ov = (v >> 6) & 3;
		}
	else
		{
		iv = Option(SC_EMBED_IN);
		ov = Option(SC_EMBED_OUT);
		}
	dlg.m_in = iv;
	dlg.m_out = ov;
	if (dlg.DoModal() == IDOK)
		{
		if (dlg.m_bOverride)
			{
			v |= 0xf0;
			v ^= 0xf0;
			v |= (dlg.m_in << 4);
			v |= (dlg.m_out << 6);
			Option(OVERRIDE_FLAGS,1,v);
			}
		else
			{
			if (dlg.m_in != iv)
				Option(SC_EMBED_IN, 1, dlg.m_in);
			if (dlg.m_out != ov)
				Option(SC_EMBED_OUT, 1, dlg.m_out);
			}
		}
}

void CSketchDoc::OnUpdateOverride(CCmdUI* pCmdUI)
{	
	int iid = pCmdUI->m_nID;
	iid -= ID_OPT_OVERRIDE_HOLD;
	BOOL bFlag = ((1 << iid) & Option(OVERRIDE_FLAGS)) ? 1 : 0;
	pCmdUI->SetCheck(bFlag);
}

void CSketchDoc::OnOverride(UINT iid)
{
	iid -= ID_OPT_OVERRIDE_HOLD;
	int v = Option(OVERRIDE_FLAGS);
	v ^= (1 << iid);
	Option(OVERRIDE_FLAGS,1,v);
	if (!iid && m_pScene)
		{
		m_pScene->OverHold(v & 1);
		CMainFrame * pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
		if (pFrame)
		pFrame->DirtyThumb();
		}
}

void CSketchDoc::DeleteContents()
{
DPF("delete contents");
	if (m_bOpened)
		{
		m_pScene->CloseIt();
		if (m_bDoErase)
			m_pIO->Save(0,1);
		}
	m_bOpened = FALSE;
	CMainFrame * pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
	if (pFrame)
		pFrame->Opener(FALSE);
#ifdef MYBUG
if (m_pScene && m_pScene->Modified())
	{
AfxMessageBox("doc delete scene not closed");
	}
//check scene closed
#endif
	SetModifiedFlag(FALSE);     // make clean
	if (m_pScene)
		m_pScene->ClipEmpty(1);
//	ClipEmpty(1);
	delete m_pScene;
	m_pScene = 0;
	AfxGetApp()->WriteProfileInt("Options","Not Closed", 0);
DPF("delete done");
}


UINT CSketchDoc::InsertMouth(UINT index, UINT frame, UINT level)
{
	CScene * pScene = m_pScene;
DPF("ins mouth,frm:%d,lvl:%d",frame, level);
	CString name = "mouth";
	name += char('A'+index);
	CDib dib;

#ifndef FLIPBOOK_MAC
	char szProgPath[MAX_PATH];
	::GetModuleFileName(NULL, szProgPath, sizeof(szProgPath)-1);
	int i,j;
	for (i = 0, j = 0; szProgPath[i];i++)
		if (szProgPath[i] == '\\')
			j = i;
	szProgPath[j+1] = 0;
	CString mouth_name = szProgPath;
	mouth_name += name;
	mouth_name += ".BMP";
	if (dib.Import(mouth_name))
		return 1;
#else
	// Mac needs full paths inside app bundle
	NSString* mouth_path = [[NSBundle mainBundle] pathForResource:[NSString stringWithCString:name.c_str()] ofType:@"bmp"];
	if (dib.Import([mouth_path cStringUsingEncoding:NSUTF8StringEncoding]))
		return 1;
#endif

	LPBITMAPINFOHEADER  lpbi = ( LPBITMAPINFOHEADER)dib.m_pBMI;
	UINT info = 0;
	if (!level)
		return 2;
	if (lpbi->biBitCount == 24)
		info = (250 << 16) + 128;
	UINT z = CreateCell(frame,level,lpbi,info,1,9);
	return z ? 10+z : 0;
}

UINT CSketchDoc::CreateCell(UINT Frame, UINT Level, LPBITMAPINFOHEADER  lpbi,
				UINT Rotation, BOOL bMakeMono, UINT nHold, BOOL bNoUpdate)
{
//	if (!nHold)
//		nHold = Option(SCALE_BG);
	CSketchView * pView = GetDocView();
	Rotation += 256 * pView->Layer();
	WORD z = m_pScene->zCreateCell(Frame,Level,lpbi,
					Rotation,bMakeMono, nHold);
	if (z)
		{
		DPF("make cell error:%d",z);
		}
	else if (!bNoUpdate)
		{
		CMainFrame * pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
		if (pFrame)
		{
		ASSERT(pFrame->m_pDoc == this);
		pFrame->UpdateCell(Frame, Level);
		}
		pView->Update(Frame, Level,2);
		}
	return z;
}

UINT CSketchDoc::MaxLevels()
{
#ifdef _THEDISC
	return 2;
#else
	UINT feat = m_dwFeatures & 15;
	if (feat == FEAT_LITE)
		{
		if (m_dwFeatures & BIT_PLUS)
			return 3;
		else
			return 2;
		}
	if ((feat == FEAT_PRO) || !feat)
		return 100;
	else
		return 6;
#endif
}

UINT CSketchDoc::MaxFrames()
{
#ifdef _THEDISC
	return 300;
#else
	if ((m_dwFeatures & 15) == FEAT_LITE)
		{
		if (m_dwFeatures & BIT_PLUS)
			return 1000;
		else
			return 300;
		}
	else if ((m_dwFeatures & 15) == FEAT_STD)
		return 1000;
	else if ((m_dwFeatures & 15) == FEAT_PT)
		return 1000;
	else
		return 1500;
#endif
}

void CSketchDoc::MaxSizes(UINT & maxw, UINT & maxh)
{
	maxw = 800;
	maxh= 600;
	if (((CSketchApp*)AfxGetApp())->CanDoFeature(CSketchApp::CD_HIRES))
		{
		maxw = 2048;
		maxh = 1566;
		}
	else if (((CSketchApp*)AfxGetApp())->IsAnimate())
		{
		maxw = 1280;
		maxh = 720;
		}
}


BOOL CSketchDoc::FolderWritable(LPCSTR lpFolder)
{
	char name[400];
	strcpy(name, lpFolder);
	strcat(name, NATIVE_SEP_STRING);
	strcat(name, "untitled9999.dgc");
	HANDLE hObj = CreateFile((LPCSTR)&name,GENERIC_READ,0,0,
				OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL,0);
	if (hObj == INVALID_HANDLE_VALUE)
		{
		DWORD zz = GetLastError();
		if (zz == ERROR_ACCESS_DENIED)
			return FALSE;
		}
	else
		{
		CloseHandle(hObj);
		DeleteFile(name);
		}
	return TRUE;
}


void CSketchDoc::OnExportMaya()
{
DPF("export maya");
	if (CheckModified())
		return;
	BeginBusy("Exporting Maya");
	ExportMaya();
	EndBusy();
}

void CSketchDoc::ExportMaya()
{
	CSketchApp* pApp = (CSketchApp *)AfxGetApp();
	CMaya * pMaya = pApp->GetMaya();
	if (m_pScene && pMaya)
		{
		m_bMayaClosed = TRUE;
		int z = pMaya->Close(m_pScene);
		if (z)
			{
			FormattedMsg(IDS_ERR_MAYA_CLOSE,z);
			}
		}
}

BOOL CSketchDoc::MakeMaya(UINT ApId, LPCSTR name)
{
	UINT width,height,frames,levels;
	UINT rate = 24;
	UINT factor = Option(DEF_FACTOR);
	UINT preview = Option(DEF_PREVIEW);
	UINT jpeg = Option(JPEG_QUAL);
	UINT bcast = Option(DEF_BROADCAST);
	CSketchApp* pApp = (CSketchApp *)AfxGetApp();
	pApp->MayaInfo(width,height,frames,levels);
	if (IsModified())
		TRACE0("Warning: OnNewDocument replaces an unsaved document\n");
	DeleteContents();
	m_strPathName.Empty();      // no path name yet
	if (m_pIO)
		delete m_pIO;
	m_pIO = new CMyIO;
	int result = m_pIO->Create(name);
	m_bDoErase = TRUE;
	if (!result)
		{
		if (m_pScene)
			delete m_pScene;
		m_pScene = new CScene(m_pIO, std::make_unique<WinResourceLoader>());
		SceneDefaults();
		UINT param = m_dwFeatures;
		if ( Option(OVERRIDE_FLAGS) & 1)
			param |= 0x10000;	// disable hold
		result = m_pScene->Make(ApId, param,
				width,height,rate,frames,levels,factor,preview,jpeg,bcast);
		}
	if (result)
		{
DPF("result:%d",result);
		MyError(IDS_SCENE_CREATE, MB_ICONINFORMATION | MB_OK);
		DeleteContents();
		return FALSE;
		}
//	m_pScene->SceneOption(SCOPT_HOLD, Option(DEF_HOLD));
	OpenInit(name, TRUE);
	return TRUE;
}

BOOL CSketchDoc::OnNewDocument()
{
	DPF("new logic");
	UINT ApId;
	CSketchApp* pApp = (CSketchApp *)AfxGetApp();
	if (pApp->IdCheck(ApId,m_dwFeatures))
		return FALSE;
	char name[356];
	char cwd[356];
	_getcwd(cwd,sizeof(cwd));
	CString strFolder = 
			pApp->GetProfileString("Settings","DefFolder", (LPCSTR)cwd);

#ifdef FLIPBOOK_MAC
	// new untitled documents should be created in the temporary items folder
	GetTempPath (356, cwd);
#endif

#ifdef OLDER
	DWORD dwAttrib = GetFileAttributes(strFolder);
	strcpy(cwd, (LPCSTR)strFolder);
	DPF("attr:%8x",dwAttrib);
	if ((dwAttrib & 0x1f) != 0x10) // not readonly nor hidden
		SHGetSpecialFolderPath(NULL, cwd, CSIDL_PERSONAL,TRUE);
#else
	if (!FolderWritable(cwd))
		{
		SHGetSpecialFolderPath(NULL, cwd, CSIDL_PERSONAL,TRUE);
		for (;;)
			{
			if (FolderWritable(cwd))
				break;
			CDirDialog ddlg(cwd, "||",0);//this);
	        ddlg.m_ofn.lpstrTitle = "Select Folder for New Scene";
		    int res = ddlg.DoModal();
			if (res != IDOK)
				return FALSE;
			strcpy(cwd,(LPCSTR)ddlg.GetPath());
			}
		}
#endif
	int i;
	i = strlen(cwd);
	if (i && (cwd[i-1] == NATIVE_SEP_CHAR))
		cwd[i-1] = 0;
	for ( i= 0; i < 10000; i++)
		{
#ifdef _DISNEY
		if (i)
			sprintf(name,"%s%sUntitled%d.CBK",cwd, NATIVE_SEP_STRING, i);
		else
			sprintf(name,"%s%sUntitled.CBK",cwd, NATIVE_SEP_STRING);
#else
		if (i)
			sprintf(name,"%s%sUntitled%d.DGC",cwd, NATIVE_SEP_STRING, i);
		else
			sprintf(name,"%s%sUntitled.DGC",cwd, NATIVE_SEP_STRING);
#endif
		HANDLE hObj = CreateFile((LPCSTR)&name,GENERIC_READ,0,0,
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,0);
		if (hObj == INVALID_HANDLE_VALUE)
			{
			DWORD zz = GetLastError();
			if (zz == ERROR_FILE_NOT_FOUND)
				break;
			}
		else
			CloseHandle(hObj);
		}
	DPF("name:%s|",name);
	if (pApp->IsMaya())
		return MakeMaya(ApId, name);

	CNewDlg dlg;
	
//	dlg.m_Kind = Option(DEF_KIND);
	dlg.m_Width = Option(DEF_WIDTH);
	dlg.m_Height = Option(DEF_HEIGHT);
	MaxSizes(dlg.m_MaxWidth,dlg.m_MaxHeight);
	dlg.m_MaxFrames = MaxFrames();
	dlg.m_MaxLevels = MaxLevels();

	dlg.m_Frames = Option(DEF_FRAMES);
	if (dlg.m_Frames > dlg.m_MaxFrames)
		dlg.m_Frames = dlg.m_MaxFrames;
	dlg.m_Levels = Option(DEF_LEVELS);
	if (dlg.m_Levels > dlg.m_MaxLevels)
		dlg.m_Levels = dlg.m_MaxLevels;
	if (dlg.m_Levels < 2)
		dlg.m_Levels = 2;
	dlg.m_Rate = Option(DEF_RATE);
	dlg.m_Factor = Option(DEF_FACTOR);
	dlg.m_Preview = Option(DEF_PREVIEW);
	dlg.m_bcast = Option(DEF_BROADCAST);
	dlg.m_nJpeg = Option(JPEG_QUAL);
	dlg.m_bXSheet = 1;
	if (dlg.DoModal() != IDOK)
		return FALSE;
	CMainFrame * pFrame = (CMainFrame*) pApp->m_pMainWnd;
	if (dlg.m_bFromAVI)
		{
		//UpdateData(1);
		Option(JPEG_QUAL,1,dlg.m_nJpeg);
		return FromAVI();
		}
/*
	AfxGetApp()->WriteProfileString("Settings", "NewFolder",
				(LPCSTR)dlg.m_Location);
*/
//	if (!CDocument::OnNewDocument())
//		return FALSE;
	if (IsModified())
		TRACE0("Warning: OnNewDocument replaces an unsaved document\n");

	DeleteContents();
	m_strPathName.Empty();      // no path name yet

//	Option(DEF_KIND,1,dlg.m_Kind);
	Option(DEF_WIDTH,1,dlg.m_Width);
	Option(DEF_HEIGHT,1,dlg.m_Height);
	Option(DEF_FRAMES,1,dlg.m_Frames);
	Option(DEF_LEVELS,1,dlg.m_Levels);
	Option(DEF_RATE,1,dlg.m_Rate);
	Option(DEF_FACTOR,1,dlg.m_Factor);
	Option(DEF_PREVIEW,1,dlg.m_Preview);
	Option(DEF_BROADCAST,1,dlg.m_bcast);
	Option(JPEG_QUAL,1,dlg.m_nJpeg);
	
DPF("width:%d,height:%d",dlg.m_Width,dlg.m_Height);
//	DeleteContents();
	if (m_pIO)
		delete m_pIO;
		//m_pIO->Close();
//	else
		m_pIO = new CMyIO;
	int result = m_pIO->Create(name);
	m_bDoErase = TRUE;
	if (!result)
		{
		if (m_pScene)
			delete m_pScene;
		m_pScene = new CScene(m_pIO, std::make_unique<WinResourceLoader>());
		UINT frames, rate,levels,factor;
		if (dlg.m_bXSheet)
			{
			rate = dlg.m_Rate;
			levels = dlg.m_Levels;
			frames = dlg.m_Frames;
			factor = dlg.m_Factor;
			}
		else
			{
			frames = dlg.m_SFrames;
			rate = dlg.m_SRate;
			levels = 1;
			factor = 2;
			}
DPF("new, fact:%d",factor);
		SceneDefaults();
		if (!dlg.m_bXSheet)
			Option(SC_BLIND,1,1);
		UINT param = m_dwFeatures;
		if ( Option(OVERRIDE_FLAGS) & 1)
			param |= 0x10000;	// disable hold
		result = m_pScene->Make(ApId, param,
				dlg.m_Width,dlg.m_Height,rate,frames,levels,
						factor, dlg.m_Preview,dlg.m_nJpeg,dlg.m_bcast);
		}
	if (result)
		{
DPF("result:%d",result);
		MyError(IDS_SCENE_CREATE, MB_ICONINFORMATION | MB_OK);
		DeleteContents();
		return FALSE;
		}
//	qm_pScene->SceneOption(SCOPT_HOLD, Option(DEF_HOLD));
//	OptionLoadSave();
//	SetPathName(pszPathName);
	OpenInit(name, TRUE);
	return TRUE;
}

void CSketchDoc::OpenInit(LPCSTR name, BOOL bModified)
{
DPF("open init");
	SetPathName(name);
	SetModifiedFlag(bModified);
	((CSketchApp*) AfxGetApp())->FileSetup();
	m_bOpened = TRUE;
//	CSketchView* pSketchView = GetDocView();
//	pSketchView->Setup();
//	CMainFrame * pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
//	if (pFrame)
//		{
//		pFrame->AttachDoc(this);
//		pFrame->Opener(TRUE);
//		}
	if (bModified)
		m_pScene->MakeModified();
	if (m_pScene->Tools(m_pTools, sizeof(CTool) * NBR_TOOLS,0))
		{
		m_pTools[0].Init(0);
		m_pTools[1].Init(1);
		m_pTools[2].Init(2);
		m_pTools[3].Init(3);
		m_pTools[4].Init(4);
		m_pTools[5].Init(5);
		m_pTools[6].Init(6);
		}
	m_pTools[7].Init(8); // always clear select
//	m_pTools[2].Flags = 2;
//	m_pTools[3].Flags = 2;
	m_pScene->SetMinBG(Option(BGMIN),1);
DPF("open init done");
	AfxGetApp()->WriteProfileInt("Options","Not Closed", 1);
}

void GetFolderKey(CString & key, int which)
{
	if (which == UDF_EXP)
		key =  "ExpFolder";
	else if (which == UDF_IMP)
		key =  "ImpFolder";
	else if (which == UDF_LIB)
		key = "LibFolder";
	else
		key = "DefFolder";
}

void CSketchDoc::UpdateDefFolder(const char* pszPathName, int which)
{
	int i,j;
	j = -1;
	char buf[350];
	for (i = 0; buf[i] = pszPathName[i]; i++)
		if (pszPathName[i] == NATIVE_SEP_CHAR)
			j = i;
	if (j >= 0)
		buf[j] = 0;
	CString key;
	GetFolderKey(key,which);
	AfxGetApp()->WriteProfileString("Settings", key, buf);
}


void CSketchDoc::GetDefFolder(CString & path, int which, BOOL bName)
{
	CString temp;
	CString key;
	GetFolderKey(key,which);
	temp = AfxGetApp()->GetProfileString("Settings",key, "");
	if (!temp.GetLength())
		{
		if (which)
			GetDefFolder(temp);	// use default
		else
			{
			char cwd[356];
			_getcwd(cwd,sizeof(cwd));
			temp = cwd;
			}
		}
	if (bName)		// need a name
		{
		char scene[356];
		LPCSTR pName = m_pIO->Name();
		int i,j,k;
		j = -1;
		k = -1;
		for (i = 0; pName[i]; i++)
			{
			if (pName[i] == NATIVE_SEP_CHAR)
				j = i;
			else if (pName[i] == '.')
				k = i;
			}
		if (k <= j)		// if no period use end of string
			k = i;
		k = k - (j + 1); // get count after separator
		for (i = 0; i < k; i++)
			scene[i] = pName[i+j+1];	// copy chars
		scene[i] = 0;					// finish it
		temp += NATIVE_SEP_CHAR;		// append sep and name to def path
		temp += scene;
		}
	path = temp;
}


BOOL CSketchDoc::OnOpenDocument(const char* pszPathName)
{
	WORD wResult;
DPF("open:%s",pszPathName);
	UINT ApId;
	if (((CSketchApp*) AfxGetApp())->IdCheck(ApId,m_dwFeatures))
		return FALSE;
	BeginBusy("Opening File");
	CMyIO * pIO = new CMyIO;
	wResult = pIO->Test(pszPathName);
	delete pIO;
	m_bDoErase = 0;
	if (!wResult)
		{
		UpdateDefFolder(pszPathName);
		DeleteContents();
		if (m_pIO)
			m_pIO->Close();
		else
			m_pIO = new CMyIO;
		wResult = m_pIO->Open(pszPathName);
//		DeleteContents();
		if (!m_pScene)
			m_pScene = new CScene(m_pIO, std::make_unique<WinResourceLoader>());
		UINT param = m_dwFeatures;
		if ( Option(OVERRIDE_FLAGS) & 1)
			param |= 0x10000;	// disable hold
		wResult = m_pScene->Read(0,param, ApId);
		if (!wResult)
			ProcessEmbeddedFiles();
DPF("after scene load:%d",wResult);
		}
	EndBusy();
	UINT idd = IDS_SCENE_LOAD;
	UINT c = wResult & 0x3000;
	UINT r = wResult & 0x4000;
	UINT q = wResult & 0x8000;
	wResult &= 0xfff;
	if (!wResult)
		{
		UINT maxw, maxh;
		MaxSizes(maxw,maxh);
		if ((m_pScene->ComW() > maxw) || 
				(m_pScene->ComH() > maxh))
			{
			if (!((CSketchApp*)AfxGetApp())->CanDoFeature(CSketchApp::CD_HIRES))
				{
				idd = IDS_NOT_HIRES;
				wResult = 99;
				}
			}
		}
	if (!wResult)
	{	
		BeginBusy("Loading Cache");
		m_pScene->LoadCache(pszPathName);
		EndBusy();
	BYTE * pLog = m_pScene->LoggedData(0);
	if (pLog)	// logged errors
		{
		CLogDlg m_dlg;
		m_dlg.m_title = "Scene Warnings";
		m_dlg.m_pData = (LPSTR)pLog;
		m_dlg.DoModal();
		m_pScene->LoggedData(TRUE);
		}
	if (m_pScene->ReadErrors(NEGTWO))
		{
		UINT count = m_pScene->ReadErrors(NEGTWO);
		if (count)
			{
			UINT i;
			for (i = 0; i < count; i++)
				{
				UINT code = m_pScene->ReadErrors(i);
				UINT pal = code & 255;
				UINT pidx = (code >> 8) & 255;
				code >>= 16;
				DPF("code:%d,i:%d,pal:%d",code,pidx,pal);
				if (code == 1)
					ProcessMissingPalette(pal);
				else if (code == 2)
					ProcessDifferentPalette(pal);
				else if (code == 4)
					ProcessUnusedPalette(pal);
				else if (code == 3)
					ProcessMissingTexture(pal,pidx);
				}
			m_pScene->ReadErrors();
			}
		}
	}
	if (wResult)
		{
DPF("wResult:%d",wResult);
		if (wResult == 21)
			idd = IDS_TOO_MANY_FRAMES;
		else if (wResult == 3)
			idd = IDS_SCENE_NEWER;
		else if (wResult == 22)
			idd = IDS_TOO_MANY_LEVELS;
		if (wResult == 9)
			idd = IDS_NOTLITE;
		else if (wResult == 8)
			idd = IDS_NOTPT;
		else if (wResult == 10)
			idd = IDS_NOTSTD;
		OEMMessageBox(idd, MB_ICONINFORMATION | MB_OK);
//		DeleteContents();
		return FALSE;
		}
DPF("rc:%d",c);
	CMainFrame * pFrame1 = (CMainFrame*) AfxGetApp()->m_pMainWnd;
	OpenInit(pszPathName,c ? 1 : 0);
	if (r)
		{
		MyDialog(IDD_REPAIRED);
		}
	else if (c)
		{
		MyDialog(IDD_DGXWARNING);
		}
	if ((m_dwFeatures & 15) && m_pScene->SceneState())
		{
		MyError(IDS_DEMO_WARNING);
		}
	if (q)
		((CSketchApp*)AfxGetApp())->MyWarning(IDD_NOSAVE, RF_NO_SAVE);
DPF("open done");
	return TRUE;
}
/*
UINT CSketchDoc::Appended(BYTE * pData)
{
	return 0;//m_pIO->Appended(pData);
}
*/
void CSketchDoc::BeginBusy(LPCSTR txt)
{
	CMainFrame * pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
	if (pFrame)
	pFrame->Status(txt);
	m_nBusy++;
	BeginWaitCursor();
}

void CSketchDoc::EndBusy()
{
	CMainFrame * pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
	if (pFrame)
	pFrame->Status();
	m_nBusy--;
	EndWaitCursor();
}

BOOL CSketchDoc::CheckPalettes()
{

	int i,c;
	c = NBR_PALS;
	for (i = 0; i < c; i++)
		{
		CNewPals * pPal = m_pScene->PalettePtr(i);
		if (!pPal)
			continue;
/*
		if (pPal->Dirty())
			{
			CPalModDlg dlg;
			dlg.m_name = pPal->GetPalName();
			dlg.m_fname = pPal->GetFileName();
			int res = dlg.DoModal();
			if (res == IDCANCEL)
				return TRUE;
			else if (res == IDOK)
				pPal->PaletteIO(dlg.m_name, 1);
			}
*/
/*
		if (!m_pScene->PalUsed(i))
			{
			CPalUnusedDlg dlg;
			dlg.m_name = pPal->GetPalName();
			int res = dlg.DoModal();
			if (res == IDCANCEL)
				return TRUE;
			else if (res == IDOK)
				m_pScene->DeletePalette(i);
			}
			*/
		}

	return FALSE;
}

BOOL CSketchDoc::OnSaveDocument(const char* pszPathName)
{
DPF("saving:%s",pszPathName);
	if (!m_pScene)
		return FALSE;
	UINT idd = 0;
	if (!(m_dwFeatures & 15))
		{
		if (m_pScene->SceneState() < 2)
			idd = IDS_SCENE_SAVE1;
		else if (m_pScene->FrameCount() > m_pScene->MaxFrameCount())
			idd = IDS_MAX_FRAMES;
		}
	else if (m_pScene->SceneState())
		idd = IDS_SCENE_SAVE2;
	else if (m_pScene->FrameCount() > m_pScene->MaxFrameCount())
		idd = IDS_MAX_FRAMES;
	if (idd)
		{
		MyError(idd, MB_ICONINFORMATION | MB_OK);
		return FALSE;
		}
	BOOL bSaveAs = 0;
	if (strcmp(GetPathName(),pszPathName))
		bSaveAs = 1;
	BOOL bHaveEmbedded = SaveEmbeddedFiles(pszPathName);
	m_pScene->SetEmbedded(bHaveEmbedded);
	CSketchView * pView = GetDocView();
	pView->SaveSceneSettings();
//	if (CheckPalettes())
//		return FALSE;
	BeginBusy("Saving File");
	int nResult;
	m_pScene->ClipEmpty(2);
//	ClipEmpty(2);
	char cachename[300];
	strcpy(cachename,pszPathName);
	int c = strlen(cachename);
	if (!c)
		strcpy(cachename,"CACHE.DGQ");
	else
		cachename[c - 1] = 'Q';
	if (bSaveAs)
		{
DPF("saveas");
		nResult = m_pIO->Save(pszPathName, m_bDoErase);
		}
	else
		{
DPF("save");
		if (m_pIO->ReadOnly())
			nResult = 9;
		else
			{
			nResult = m_pIO->Save();
			}
		}
	if (bHaveEmbedded)
		m_pScene->EmbAppend(nResult ? 1 : 0);
	UpdateDefFolder(pszPathName);
	m_bDoErase = 0;

	EndBusy();
	if (Option(SC_SCACHE))
		{
		BeginBusy("Saving Cache");
		m_pScene->SaveCache(cachename);
		}
	else
		{
		BeginBusy("Erasing Cache");
		RemoveFile(cachename);
		}
	EndBusy();
	DPF("mio save:%d",nResult);
	if (nResult)
		{
		MyError(IDS_SCENE_SAVE, MB_ICONINFORMATION | MB_OK);
		return FALSE;
		}
	else
		{
		SetModifiedFlag(FALSE);     // back to unmodified
		m_pScene->Modified(TRUE);
		}
	return TRUE;
}

void CSketchDoc::OnFileSave()
{
	if (m_bDoErase)
		OnFileSaveAs();
	else if (!CheckModified())
		CDocument::OnFileSave();
}

void CSketchDoc::OnFileSaveAs()
{
	if (!CheckModified())
		CDocument::OnFileSaveAs();
}

CSketchView * CSketchDoc::GetDocView()
{
	CView* pView;
	CSketchView* pSketchView;
	POSITION pos = GetFirstViewPosition();
	if (pos != NULL)
		{
		pView = GetNextView(pos);
		pSketchView = DYNAMIC_DOWNCAST(CSketchView, pView);
		}
	else
		pSketchView = NULL;
//	ASSERT(pSketchView != NULL);
	return pSketchView;
}

BOOL CSketchDoc::CheckModified()
{
	CSketchView* pSketchView = GetDocView();
#ifndef FLIPBOOK_MAC
DPF("check modified,view:%8x",(DWORD)pSketchView);
#endif
	if (!m_pScene)
		return FALSE;
	SetModifiedFlag(m_pScene->Modified());
	if (pSketchView != NULL)
		return pSketchView->CheckModified(TRUE);
	else
		return FALSE;
}

BOOL CSketchDoc::SaveModified()
{
DPF("save modified");
	if (!m_pScene)
		return TRUE;
	BOOL bCanCancel = ((CSketchApp *) AfxGetApp())->CanCancel(1);
	CSketchApp* pApp = (CSketchApp *)AfxGetApp();
	CMainFrame * pFrame = (CMainFrame*)pApp->m_pMainWnd;
	if (m_dwFeatures & 15)
		pFrame->SaveSettings(TRUE);
	else
		{
		pFrame->SaveSettings(FALSE);
//		ClipEmpty(1);
		}
	if (CheckModified())
		return FALSE;
	if (!m_bMayaClosed)
		ExportMaya();
	if (!IsModified() || pApp->IsMaya())
		return TRUE;        // ok to continue

	// get name/title of document
	CString name = m_strPathName;
	if (name.IsEmpty())
	{
		name = m_strTitle;
		if (name.IsEmpty())
			VERIFY(name.LoadString(AFX_IDS_UNTITLED));
	}

	CString prompt;
	AfxFormatString1(prompt, AFX_IDP_ASK_TO_SAVE, name);
	switch (AfxMessageBox(prompt, 
			bCanCancel ? MB_YESNOCANCEL : MB_YESNO, AFX_IDP_ASK_TO_SAVE))
	{
	case IDCANCEL:
		if (bCanCancel)
			return FALSE;       // don't continue
		break;
	case IDYES:
		if (!DoSave(m_strPathName) && bCanCancel)
			return FALSE;   // don't continue
		break;

	case IDNO:
			m_pScene->Modified(TRUE);
		//	ClipEmpty(1);
			m_pIO->Wipe(0);
		// If not saving changes, revert the document
		break;

	default:
		ASSERT(FALSE);
		break;
	}
	return TRUE;    // keep going
}
//
//	code = 0, finished, 1 is start(range), 2 is value
//
UINT CSketchDoc::Progress(int code, int arg)
{
	return ((CMainFrame*) AfxGetApp()->m_pMainWnd)->Progress(code,arg);
}

void CSketchDoc::UpdateTools(UINT which)
{
	UINT siz = sizeof(CTool)*NBR_TOOLS;
	UINT z = m_pScene->Tools(m_pTools, siz,1);
DPF("put tools:%d,size:%d",z,siz);
}

/*
	5 returns flags without bottom two bits
	1,2,128 are pencil type )..7)
	4 is density affcets radius
	8 is density
	16 is solid pencil
	32,64 are pencil texture
	256,512, are eraser texture
	1024 is solid eraser
	2048, 4096, 8192 are eraser kinds

*/
BOOL CSketchDoc::ToolInfo(int code, int val)
// 0 is get, 1 is set, 2 is toggle ,
//	3 gets can erase
// 4 gets previous
//	5 gets flags
//	6 sets flags
{
	UINT current = m_pTools[0].Previous;// always has current
	if (current == 8)
		current = 7;
	else if (current >= NBR_TOOLS)
		current = 0;
	CTool * pTool = m_pTools + current;
	if (code == 4)
		return pTool->Previous;
	if (code == 3)
		return pTool->Flags & 2 ? 1 : 0; 
	if (code == 5)
		{
		if (pTool->Flags & 1) // if eraser
			{
			UINT v = (pTool->Flags >> 2) & TOOL_MASK;
			v |= 131;
			v ^= 131; // clear kind 
			if (v & 2048) v |= 1;
			if (v & 4096) v |= 2;
			if (v & 8192) v |= 128;
			v |= (2048|4096|8192);
			v ^= (2048|4096|8192);
			return v;
			}
		return (pTool->Flags >> 2) & TOOL_MASK; 
		}
//	pTool = m_pTools;
	UINT old = pTool->Flags;
	if (code == 2)
		pTool->Flags ^= 1;
	else if (code == 1)
		{
		pTool->Flags |= 1;
		pTool->Flags ^= 1;
		pTool->Flags |= (val & 1);
		}
	else if (code == 6)
		{
		if (pTool->Flags & 1) // if eraser
			{
			UINT v = (pTool->Flags >> 2) & TOOL_MASK;
			v &= 131;	// save non erase kind
			val |= (2048|4096|8192);
			val ^= (2048|4096|8192);
			if (val & 1)   val |= 2048;
			if (val & 2)   val |= 4096;
			if (val & 128) val |= 8192;
			val |= 131;
			val ^= 131;
			val |= v;
			}
		pTool->Flags |= (TOOL_MASK << 2);
		pTool->Flags ^= (TOOL_MASK << 2);
		pTool->Flags |= ((val & TOOL_MASK) << 2);
		}
	if (pTool->Flags != old)
		{
		CMainFrame * pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
		pFrame->ToolStatus(0,Radius(3000,current),Density(3000,current));
		UpdateTools(current);
		CSketchView * pView = GetDocView();
		pView->ChangeTool();
		}
//DPF("tool flags(%d),%d,%d",current,pTool->Kind,pTool->Flags);
	return pTool->Flags & 1; 
}

UINT CSketchDoc::SelectTool(UINT arg)
{
	if (arg != 9999)
		{
		if (arg == TOOL_FILL)
			m_pTools[TOOL_FILL].Radius = 0;
		if (arg == TOOL_EYEDROP)
			{
			UINT c = m_pTools[m_pTools[0].Previous].Color;
			UINT k = m_pTools[m_pTools[0].Previous].Kind;
			m_pTools[TOOL_EYEDROP].Previous = k;
			m_pTools[TOOL_EYEDROP].Color = c;
			}
		if (m_pTools[0].Previous != arg)
			{
DPF("setting prev:%d",arg);
			CTool * pTool = m_pTools + arg;
			pTool->Flags |= 1;
			pTool->Flags ^= 1;  // clear erase
			m_pTools[0].Previous = arg; // kludge but effective
			CMainFrame * pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
			pFrame->ToolStatus(0,Radius(),Density());
			CSketchView * pView = GetDocView();
			pView->ChangeTool();
			UpdateTools(TOOL_EYEDROP);
			}
		}
	return m_pTools[0].Previous;
}
/*
	arg = 3000, return radius
	arg = 1000, return min
	arg = 2000, return max
	which = 1000, current tool
	else which == 2 for pencil
*/
UINT CSketchDoc::Radius(UINT arg, int which)
{
	CTool * pTool;
//DPF("Radius arg:%d,which:%d",arg,which);
	if (which == 1000)
		which = SelectTool();
	else if (which >= NBR_TOOLS)
		{
DPF("bad tool:%d",which);
		which = 0;
		}
	pTool = m_pTools + which;
	BOOL bErase = 0;
	if ((pTool->Flags & 2) && (m_pTools[0].Flags & 1))
		bErase = TRUE;
	if (arg == 3000)
		{
		if (pTool->Kind > 3)
			return 0;
		return bErase ? pTool->eRadius : pTool->Radius;
		}
	if (arg == 1000)
		return pTool->MinRadius;
	if (arg == 2000)
		return pTool->MaxRadius;
	if (arg > pTool->MaxRadius) arg = pTool->MaxRadius;
	if (arg < pTool->MinRadius) arg = pTool->MinRadius;
	if (bErase)
		pTool->eRadius = arg;
	else
		pTool->Radius = arg;
	UpdateTools(which);
	CMainFrame * pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
	pFrame->ToolStatus(0,Radius(3000,which),Density(3000,which));
	CSketchView * pView = GetDocView();
	pView->ChangeTool();
	return Radius(3000,which);
}

UINT CSketchDoc::Density(UINT arg, int which)
{
	CTool * pTool;
//DPF("Density arg:%d,which:%d",arg,which);
	if (which == 1000)
		which = SelectTool();
	else if (which >= NBR_TOOLS)
		{
DPF("bad tool:%d",which);
		which = 0;
		}
	pTool = m_pTools + which;
	BOOL bErase = 0;
	if ((pTool->Flags & 2) && (m_pTools[0].Flags & 1))
		bErase = TRUE;
	if (arg == 3000)
		return bErase & 1 ? pTool->eDensity : pTool->Density;
	if (arg == 1000)
		return pTool->MinDensity;
	if (arg == 2000)
		return pTool->MaxDensity;
	if (arg > pTool->MaxDensity) arg = pTool->MaxDensity;
	if (arg < pTool->MinDensity) arg = pTool->MinDensity;
	if (bErase)
		pTool->eDensity = arg;
	else
		pTool->Density = arg;
	UpdateTools(which);
	CMainFrame * pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
	pFrame->ToolStatus(0,Radius(3000,which),Density(3000,which));
	CSketchView * pView = GetDocView();
	pView->ChangeTool();
	return Density(3000,which);
}

UINT CSketchDoc::Color(UINT arg, int which)
{
DPF("color arg:%d,which:%d",arg,which);
	CTool * pTool;
	if (m_bToolsSameColor)
		which = 0;
	if (which == 1000)
		which = SelectTool();
	else if (which >= NBR_TOOLS)
		{
DPF("bad tool:%d",which);
		which = 0;
		}
	pTool = m_pTools + which;
	if (arg != 3000)
		{
		pTool->Color = arg;
		UpdateTools(which);
		}
	return pTool->Color;
}

void CSketchDoc::OnScan()
{
	
//	CScan dlg;
	CMainFrame * pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
	if (pFrame->m_pScan) return;
	pFrame->m_pScan = new CScan;
	CScan * pdlg = pFrame->m_pScan;
	CSketchView* pSketchView = GetDocView();
	pdlg->m_pDoc = this;
	pFrame->m_pScan->GetChanges();
	pdlg->m_Skip = Option(DEF_HOLD);
	pdlg->m_FieldSize = 8.0;
	pdlg->m_Offset = 5.0;
	pdlg->m_bAuto = ((CSketchApp*)AfxGetApp())->CanDoFeature(CSketchApp::CD_DESKEW);
	int v = Option(SCANROTATE);
	pdlg->m_iRotation = v & 7;
	pdlg->m_b24 = v & 8 ? 1 : 0;
	pdlg->m_bFile = v & 16 ? 1 : 0;
	pdlg->m_bScale = v & 32 ? 1 : 0;
	pdlg->m_bAutoHold = v & 64 ? 1 : 0;
	pdlg->m_bSave = v & 128 ? 1 : 0;
	pdlg->m_bPegTop = Option(SCAN_PEGTOP);
//	pdlg->m_bCropPeg = Option(SCAN_CROPPEG);
	pdlg->m_FieldSize = (double)Option(SCAN_FIELD) / 1000;
	pdlg->m_Offset = (double)Option(SCAN_OFFSET) / 1000;
	
	pdlg->Create(CScan::IDD, pFrame);
	pdlg->ShowWindow(SW_SHOW);
}

unsigned int MayaCallback(unsigned int index,
						void *UserObject, void *UserData )
{
	return ((CMaya *)UserObject)->DoFile(index,(CSketchDoc *)UserData);
}


void CSketchDoc::OnImportMaya()
{
	DPF("on import maya");
	CSketchApp* pApp = (CSketchApp *)AfxGetApp();
	CMaya * pMaya = pApp->GetMaya();
	BeginBusy("Importing Images");
	CProg2Dlg progger;
	progger.ProgSetup( MayaCallback, pMaya,this,0,pMaya->Count());
	progger.DoModal();
	EndBusy();
}


void CSketchDoc::OnImportPalette()
{
	DPF("on import palette");
	CString fileName;
	if (!((CSketchApp*) AfxGetApp())->PromptFileName(fileName, 6))
		return;
	char pname[50];
	LPCSTR name = (LPCSTR)fileName;
	int j = 0;
	int i;
	for (i = 0; name[i]; i++)
		{
		if (name[i] == NATIVE_SEP_CHAR)
			j = i + 1;
		if (name[i] == '.')
			break;
		}
	int zz;
	for (zz = 0; ((zz + 2) < sizeof(pname)) && (j <  i);)
		pname[zz++] = name[j++];
	pname[zz] = 0;
	UINT index = m_pScene->NewPalette(pname);
	CNewPals * pPal = m_pScene->PalettePtr(index);
DPF("import:%s",(LPCSTR)fileName);
	UINT z = pPal->PaletteIO(fileName,0);
	if (z == 1)
		{
		MyError(IDS_FILE_OPEN, (LPCSTR)fileName);
		}
	else if (z == 2)
		{
		MyError(IDS_FILE_READ, (LPCSTR)fileName);
		}
	if (!z && ((CSketchApp *)AfxGetApp())->IsLite() && !pPal->Simple())
		{
		MyError(IDS_PAL_IMP_LITE, (LPCSTR)fileName);
		z = 3;
		}
	if (z)
		m_pScene->DeletePalette(index);
	else
		{
		pPal->SetFileName(fileName);
		m_pScene->PalWrite(index);
		}

}

void CSketchDoc::OnExportPalette()
{
	DPF("on export palette");
	CPalSelDlg dlg;
	dlg.m_pScene = m_pScene;
	if (dlg.DoModal() != IDOK)
		return;
//	CString fileName = "";m_palname;
	CString fileName = "";//m_palname;
	CNewPals * pPal = m_pScene->PalettePtr(dlg.m_index);
	FileName(fileName, pPal->GetPalName());
	if (!((CSketchApp*) AfxGetApp())->PromptFileName(fileName,7))
		return;
DPF("export:%s",(LPCSTR)fileName);
	UINT z = pPal->PaletteIO(fileName,1);
	if (z)
		{
		MyError(IDS_FILE_CREATE, (LPCSTR)fileName);
		}
	else
		pPal->SetFileName(fileName);
}

void CSketchDoc::OnImportCamera()
{
	DPF("on import camera");
	if (CheckModified())
		return;
	if (!m_pScene)
		return;
	CCamera * pCamera = m_pScene->Camera();
	if (!pCamera)
		return;
	CCamMoves * pMoves = new CCamMoves;
	if (!pMoves)
		return;

	CString fileName;
	GetDefFolder(fileName,UDF_DEF,1);
	fileName += ".TXT";
	if (!MyApp->PromptFileName(fileName, 30))
		return ;
	UpdateDefFolder(fileName,UDF_DEF);

	int result = pMoves->Read(pCamera, fileName);
	if (result)
		{
		delete pMoves;
		FormattedMsg(IDS_ERR_FILE_READ,result);
		return;
		}
	CImpCamDlg dlg;
	CMainFrame * pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
	if (!pFrame->GetSelection(dlg.m_StartFrame,dlg.m_StartLevel,
				dlg.m_EndFrame, dlg.m_EndLevel))
		{
		CSketchView * pView = GetDocView();
		dlg.m_StartFrame = dlg.m_EndFrame = pView->CurrentFrame();
		dlg.m_StartLevel = dlg.m_EndLevel = pView->CurrentLevel();
		}
	CString key = "Export Camera Settings";
	CString section = "Settings";
	dlg.m_StartFrame++;
	dlg.m_EndFrame++;
	dlg.m_MaxFrame = m_pScene->FrameCount();
	dlg.m_MaxLevel = m_pScene->LevelCount()-1;
	int flags = GetInt(section, key,0);
	dlg.m_bCamY    = flags & 1 ? 1 : 0;
	dlg.m_bCamX    = flags & 2 ? 1 : 0;
	dlg.m_bCamZ    = flags & 4 ? 1 : 0;
	dlg.m_bCamR    = flags & 8 ? 1 : 0;
	dlg.m_bCamB    = flags & 16 ? 1 : 0;
	dlg.m_bCamA    = flags & 32 ? 1 : 0;
	dlg.m_bCamComputed= flags & 64 ? 1 : 0;
	dlg.m_bCamGroupInfo =    flags & 128 ? 1 : 0;

	dlg.m_pMoves = pMoves;
	if (dlg.DoModal() != IDOK)
		{
		delete pMoves;
		return;
		}
	flags = 0;
	flags |= dlg.m_bCamY ? 1 : 0;
	flags |= dlg.m_bCamX ? 2 : 0;
	flags |= dlg.m_bCamZ ? 4 : 0;
	flags |= dlg.m_bCamR ? 8 : 0;
	flags |= dlg.m_bCamB ? 16 : 0;
	flags |= dlg.m_bCamA ? 32 : 0;
	flags |= dlg.m_bCamComputed ? 64 : 0;
	flags |= dlg.m_bCamGroupInfo ? 128 : 0;
	PutInt(section, key, flags);
	result = pMoves->Apply(dlg.m_nPeg, flags,
				dlg.m_StartFrame-1, dlg.m_StartLevel);
	delete pMoves;
	pFrame->RedrawSheet();		// update all thumbs (just in case)
	if (pMoves->NeedComposite())
		m_pScene->UpdateCache();    // recomposite
}

void CSketchDoc::OnExportCamera()
{
	DPF("on export camera");

	if (CheckModified())
		return;
	if (!m_pScene)
		return;
	CCamera * pCamera = m_pScene->Camera();
	if (!pCamera)
		return;
	CExpCamDlg dlg;
	CMainFrame * pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
	if (!pFrame->GetSelection(dlg.m_StartFrame,dlg.m_StartLevel,
				dlg.m_EndFrame, dlg.m_EndLevel))
		{
		CSketchView * pView = GetDocView();
		dlg.m_StartFrame = dlg.m_EndFrame = pView->CurrentFrame();
		dlg.m_StartLevel = dlg.m_EndLevel = pView->CurrentLevel();
		}
	CString key = "Export Camera Settings";
	CString section = "Settings";
	dlg.m_StartFrame++;
	dlg.m_EndFrame++;
	dlg.m_MaxFrame = m_pScene->FrameCount();
	dlg.m_MaxLevel = m_pScene->LevelCount()-1;
	int flags = GetInt(section, key,0);
	dlg.m_bCamY    = flags & 1 ? 1 : 0;
	dlg.m_bCamX    = flags & 2 ? 1 : 0;
	dlg.m_bCamZ    = flags & 4 ? 1 : 0;
	dlg.m_bCamR    = flags & 8 ? 1 : 0;
	dlg.m_bCamB    = flags & 16 ? 1 : 0;
	dlg.m_bCamA    = flags & 32 ? 1 : 0;
	dlg.m_bCamComputed= flags & 64 ? 1 : 0;
	dlg.m_bActiveOnly = flags & 256 ? 1 : 0;
	dlg.m_bDoGroups =    flags & 128 ? 1 : 0;
	if (dlg.DoModal() != IDOK)
		return;

	CString fileName;
	GetDefFolder(fileName,UDF_DEF,1);
	fileName += ".TXT";
	if (!MyApp->PromptFileName(fileName, 31))
		return ;
	UpdateDefFolder(fileName,UDF_DEF);
	flags = 0;
	flags |= dlg.m_bCamY ? 1 : 0;
	flags |= dlg.m_bCamX ? 2 : 0;
	flags |= dlg.m_bCamZ ? 4 : 0;
	flags |= dlg.m_bCamR ? 8 : 0;
	flags |= dlg.m_bCamB ? 16 : 0;
	flags |= dlg.m_bCamA ? 32 : 0;
	flags |= dlg.m_bCamComputed ? 64 : 0;
	flags |= dlg.m_bDoGroups ? 128 : 0;
	flags |= dlg.m_bActiveOnly ? 256 : 0;
	PutInt(section, key, flags);
	

	CString SceneName;
	FileName(SceneName, m_pIO->Name());


	CCamMoves * pMoves = new CCamMoves;
	int result = pMoves->Write(pCamera, fileName, SceneName, m_pScene, flags,
						dlg.m_StartFrame-1, dlg.m_StartLevel,
						dlg.m_EndFrame-1, dlg.m_EndLevel);
	delete pMoves;
	if (result == 1)
		result = IDS_NO_CREATE;
	else if (result == 2)
		result = IDS_NO_WORK;
	if (result)
		AfxMessageBox(result);
}


void CSketchDoc::OnImportFile()
{
	DPF("on import file");
	CString fileName;
	GetDefFolder(fileName,UDF_IMP);
	((CSketchApp*) AfxGetApp())->PromptFileName(fileName, 3);
	UpdateDefFolder(fileName,UDF_IMP);
	DPF("got name:%s|",(LPCSTR)fileName);
}

void CSketchDoc::OnExport()
{
#ifdef USEQT
	DPF("on export stills");
	if (CheckModified())
		return;
	CExpDlg dlg;
	CMainFrame * pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
	if (!pFrame->GetSelection(dlg.m_StartFrame,dlg.m_StartLevel,
				dlg.m_EndFrame, dlg.m_EndLevel))
		{
		CSketchView* pSketchView = GetDocView();
		dlg.m_StartFrame = dlg.m_EndFrame = pSketchView->CurrentFrame();
		dlg.m_StartLevel = dlg.m_EndLevel = pSketchView->CurrentLevel();
		}
	dlg.m_StartFrame++;
	dlg.m_EndFrame++;
#ifdef FLIPBOOK_MAC
	// for Mac we default to PNG
	dlg.m_Kind = kPNGFileFormat;
#else
	dlg.m_Kind = 0;
#endif
	dlg.m_Type = 0;
	dlg.m_MaxFrame = m_pScene->FrameCount();
	dlg.m_MaxLevel = m_pScene->LevelCount()-1;
	dlg.m_origwidth = dlg.m_width = m_pScene->Width();
	dlg.m_origheight = dlg.m_height = m_pScene->Height();
	dlg.m_fact = m_pScene->ZFactor(1);
	CString fileName;
	GetDefFolder(fileName,UDF_EXP);
	CString path = fileName;
	BOOL bSingle;
	char szFormat[100];
	int  nIndex;
	for (;;)
		{
		if (dlg.DoModal() != IDOK)
			return;
		BOOL b32;
		if (dlg.m_Type == 2) // combined
			b32 = 1;
		else if (dlg.m_Type == 1)
			b32 = dlg.m_StartLevel > 0 ? 1 : 0; // 24 if onto bg
		else
			b32 = dlg.m_EndLevel ? 1 : 0;	// 24 if just bG
		if (b32 && !(dlg.m_Flags & 2))
			{
			if (((CSketchApp*)AfxGetApp())->MyWarning(IDD_LOSE_ALPHA) != IDOK)
				continue;
			}
		if (dlg.m_StartFrame != dlg.m_EndFrame)
			bSingle = FALSE;
		else if (dlg.m_Type)
			bSingle = TRUE;
		else if (dlg.m_StartLevel == dlg.m_EndLevel)
			bSingle = TRUE;
		else
			bSingle = FALSE;
		if (dlg.m_Type)
			{
			if (dlg.m_prefix.GetLength())
				{
				strcpy(szFormat,(LPCSTR)dlg.m_prefix);
				int l = strlen(szFormat);
				int v = 0;
				int f = 1;
				int i;
				for (i = l -1;;i--)
					{
					if ((szFormat[i] < '0') || (szFormat[i] > '9'))
						break;
					v += f * (szFormat[i] & 15);
					f *= 10;
					if (!i)
						break;
					}	
				if (i < (l - 1))
					{
					szFormat[i+1] = 0;
					strcat(szFormat,"%04d");
					nIndex = v;
					}
				else
					{
					strcat(szFormat,"%04d");
					nIndex = dlg.m_StartFrame;
					}
				}
			else
				{
				strcpy(szFormat,"FRM%05d");
				nIndex = dlg.m_StartFrame;
				}
			}
		if (bSingle)
			{
			fileName += NATIVE_SEP_STRING;
			char buf[100];
			if (dlg.m_Type)
				sprintf(buf, szFormat, nIndex);
			else
				sprintf(buf, "L%03dF%03d", dlg.m_StartLevel,dlg.m_StartFrame);
			fileName += buf;
			if (((CSketchApp*) AfxGetApp())->PromptFileName(fileName,
						100 + dlg.m_Kind))
				break;
			}
		else
			{
			CDirDialog ddlg(path, "||",0);//this);
		    int res = ddlg.DoModal();
			if (res == IDOK)
				{
				path = (LPCSTR)ddlg.GetPath();
				break;
				}
			}
		}
	if (bSingle)
		UpdateDefFolder(fileName,UDF_EXP);
	else
		UpdateDefFolder(path,UDF_EXP);
	dlg.m_StartFrame--;	
	dlg.m_EndFrame--;
	char buf[100];
	char ext[15];
	int kinds = FBQTGetExt(ext,(FBFileFormat)dlg.m_Kind);
	ASSERT(kinds == dlg.m_Flags);
	BOOL bColor = m_pScene->ColorMode();
	BOOL bDone = FALSE;
	BOOL b32 = kinds & 2 ? 1 : 0;
	BeginBusy("Exporting Files");
	if (dlg.m_Type == 2)	// combining
		{
		UINT w = m_pScene->Width();
		UINT h = m_pScene->Height();
		UINT Frame;
		for (Frame = dlg.m_StartFrame;!bDone && Frame <= dlg.m_EndFrame;Frame++)
			{
			CDib dib;
			dib.Create(w,h,bColor ? 32 : 8);
			m_pScene->CombineLayers(dib.m_pBits,dlg.m_StartLevel, 
						dlg.m_EndLevel, Frame);
			if (!b32)
				dib.Convert(24);
			else if (!bColor)
				dib.Convert(32);
			
			if ((dlg.m_width != w) || (dlg.m_height != h))
				dib.Scale(dlg.m_width,dlg.m_height);
			bDone = bSingle;
			if (!bSingle)
				{
				sprintf(buf, szFormat, nIndex++);
				fileName = path;
				fileName += buf;
				fileName += ".";
				fileName += ext;
				pFrame->Status(buf);
				}
			int res = FBExportStill (fileName, dib.m_pBMI, 0,
							(FBFileFormat) dlg.m_Kind);
			if (res)
				{
				DPF("bad create");
				if (MyError(IDS_FILE_CREATE, (LPCSTR)fileName, MB_OKCANCEL) == IDCANCEL)
					bDone = TRUE;
				}
			}
		}
	
	else if (dlg.m_Type)	// composited
		{
//
//	changed 8/20/04 back to comW
//
		UINT w, h;
		if (m_pScene->Broadcast())
			{
			w = 2 * m_pScene->Width() / m_pScene->ZFactor(1);
			h = 2 * m_pScene->Height() / m_pScene->ZFactor(1);
			}
		else
			{
			w = m_pScene->ComW();
			h = m_pScene->ComH();
			}
		UINT Frame;
		for (Frame = dlg.m_StartFrame;!bDone && Frame <= dlg.m_EndFrame;Frame++)
			{
			CDib dib;
			int depth = dlg.m_StartLevel ? 32 : 24;
			if (!bColor)
				dib.Create(w,h,8);
			else
				dib.Create(w,h,depth);
			m_pScene->CompositeFrame32(dib.m_pBits,dlg.m_StartLevel, 
						dlg.m_EndLevel, Frame, bColor && (depth == 32));
			bDone = bSingle;
			if (!bSingle)
				{
				sprintf(buf, szFormat, nIndex++);
				fileName = path;
				fileName += buf;
				fileName += ".";
				fileName += ext;
				pFrame->Status(buf);
				}
			if ((dlg.m_width != w) || (dlg.m_height != h))
				dib.Scale(dlg.m_width,dlg.m_height);
			if (!bColor)
				dib.Convert(depth);
			if ((depth == 32) && !b32)
				dib.Convert(24);
			int res = FBExportStill (fileName, dib.m_pBMI, 0,
							(FBFileFormat)dlg.m_Kind);
			if (res)
				{
				DPF("bad create");
				if (MyError(IDS_FILE_CREATE, (LPCSTR)fileName, MB_OKCANCEL) == IDCANCEL)
					bDone = TRUE;
				}
			}
		}

	else			// individual cells
		{
		UINT w = m_pScene->Width();
		UINT h = m_pScene->Height();
		UINT Frame, Level;
		for (Frame = dlg.m_StartFrame;!bDone&&Frame <= dlg.m_EndFrame; Frame++)
			{
			for (Level = dlg.m_StartLevel;!bDone&&Level<=dlg.m_EndLevel; Level++)
				{
				CDib dib;
				if (Level)
					{
					dib.Create(w,h,bColor ? 32 : 8);
					m_pScene->FetchCell(dib.m_pBits, 
										Frame, Level, 1,0,1);
					if ((dlg.m_width != w) || (dlg.m_height != h))
						dib.Scale(dlg.m_width,dlg.m_height);
					if (!bColor)
						dib.Convert(32);
					if (!b32)
						dib.Convert(24);
					}
				else
					{
					UINT cw, ch,kk;
					if (m_pScene->CellInfo(0,Frame, Level,1,cw,ch,kk))
						{
						dib.Create(cw,ch,24);
						m_pScene->ReadImage(dib.m_pBits,kk);
						if (dlg.m_bScaleBG)
							{
							if ((dlg.m_width != w) || (dlg.m_height != h))
								dib.Scale(dlg.m_width * cw / w,
											dlg.m_height * ch / h);
							}
						}
					else
						continue;
					}
				
				bDone = bSingle;
				if (!bSingle)
					{
					char buf[15];
					sprintf(buf, "L%03dF%03d", Level,Frame+1);
					fileName = path;
					fileName += buf;
					fileName += ".";
					fileName += ext;
					pFrame->Status(buf);
					}
//DPF("filename:%s",fileName);
				int res = FBExportStill (fileName, dib.m_pBMI, 0,
							(FBFileFormat) dlg.m_Kind);
				if (res)
					{
					DPF("bad create");
					if (MyError(IDS_FILE_CREATE,
								(LPCSTR)fileName, MB_OKCANCEL) == IDCANCEL)
						bDone = TRUE;
					}
				}
			}
		}
	EndBusy();
//	pFrame->Status("       ");
#endif
}
class CMyBytes
{
public:
	CMyBytes() { m_pData = 0;};
	~CMyBytes() { delete m_pData;m_pData = 0;};
	BYTE * Setup(UINT size) { m_pData = new BYTE[size];return m_pData;};
	BYTE * pp() { return m_pData;};
	BYTE * m_pData;
} ;


void CSketchDoc::OnExtEdit()
{
	if (CheckModified())
		return;
	CExtEditDlg dlg;
	CMainFrame * pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
	if (!pFrame->GetSelection(dlg.m_StartFrame,dlg.m_StartLevel))
		{
		CSketchView* pSketchView = GetDocView();
		dlg.m_StartFrame = pSketchView->CurrentFrame();
		dlg.m_StartLevel = pSketchView->CurrentLevel();
		}
	dlg.m_StartFrame++;
	int opts = Option(EXTEDIT_OPTS);
	dlg.m_b24 = opts > 255;
	dlg.m_nAlpha = opts & 255;
	dlg.m_bKeying = dlg.m_nAlpha == 255 ? FALSE : TRUE;
//	dlg.m_EndFrame = dlg.m_StartFrame;
	dlg.m_MaxFrame = m_pScene->FrameCount();
	dlg.m_MaxLevel = m_pScene->ColorMode() ? m_pScene->LevelCount()-1 : 0;
	if (!((CSketchApp*) AfxGetApp())->RemindFlags(RF_EXTERNAL_EDIT))
		{
		if (dlg.DoModal() != IDOK)
			return;
		int opts = dlg.m_nAlpha;
		if (!dlg.m_bKeying)
			opts = 255;
		if (dlg.m_b24)
			opts += 256;
		Option(EXTEDIT_OPTS,1,opts);
		if (dlg.m_bNotAgain)
			((CSketchApp*) AfxGetApp())->RemindFlags(RF_EXTERNAL_EDIT, 1);
		}
	UINT kind = 0;
	dlg.m_StartFrame--;	
	UINT Frame = dlg.m_StartFrame;
	UINT Level = dlg.m_StartLevel;
	CDib odib;
	CDib tdib;
	UINT cw, ch;
	if (Level)
		{
		UINT kk;
		if (m_pScene->CellInfo(0,Frame, Level,0,cw,ch,kk))
			kk = 1;
		else
			{
			if (((CSketchApp*)AfxGetApp())->MyWarning(IDD_EXT_EMPTY,
							RF_EXTERNAL_EMPTY | 128) != IDOK)
					return;
			kk = 0;
			cw = m_pScene->Width();
			ch = m_pScene->Height();
			}
		if (dlg.m_b24)
			{
			kind = 0;
			odib.Create(cw,ch,24);
			tdib.Create(cw,ch,32);
			if (kk)
				{
				BYTE * pDst = odib.m_pBits;
				BYTE * pSrc = tdib.m_pBits;
				m_pScene->CellInfo(pSrc,Frame, Level,0,cw,ch,kk);
				UINT x, y;
				UINT op = 4 * ((3 * cw + 3) / 4);
				UINT ip = 4 * cw;
				for (y = 0; y < ch; y++)
					for (x = 0; x < cw; x++)
						{
						pDst[y*op+3*x+0] = pSrc[y*ip+4*x+0];
						pDst[y*op+3*x+1] = pSrc[y*ip+4*x+1];
						pDst[y*op+3*x+2] = pSrc[y*ip+4*x+2];
						}
				}
			}
		else
			{
			kind = 2;
			odib.Create(cw,ch,32);
			if (kk)
				m_pScene->CellInfo(odib.m_pBits,Frame, Level,0,cw,ch,kk);
			}
		}
	else
		{
		UINT cw, ch,kk;
		if (m_pScene->CellInfo(0,Frame, Level,0,cw,ch,kk))
			{
			odib.Create(cw,ch,24);
			m_pScene->ReadImage(odib.m_pBits,kk);
			}
		else
			{
			if (((CSketchApp*)AfxGetApp())->MyWarning(IDD_EXT_EMPTY,
							RF_EXTERNAL_EMPTY | 128) != IDOK)
					return;
			cw = m_pScene->Width();
			ch = m_pScene->Height();
			odib.Create(cw,ch,24);
			}
		}
	FBFileFormat fkind;
	char ext[30];
#ifndef FLIPBOOK_MAC
	char fileName[80];
	strcpy(fileName,"DIGICELX.");
	if (kind)
		{
		CSketchApp* pApp = (CSketchApp *)AfxGetApp();
		CString strExt;
		strExt = pApp->GetProfileString("Options","ExternalEditExt", "");
		if (!_stricmp((LPCSTR)strExt,"TGA"))
			fkind = kTARGAFileFormat;
		else if (!_stricmp((LPCSTR)strExt,"BMP"))
			fkind = kBMPFileFormat;
		else
			fkind = kPNGFileFormat;
		}
	else
		fkind = kBMPFileFormat;
#else
	char fileName[512];
	MakePathForTempDirectory(fileName,"DIGICELX.");
	fkind = FBGetMacDefault();
#endif
	FBQTGetExt(ext,fkind);
	strcat(fileName,ext);
	if (odib.Export(fileName, fkind))
		{
		DPF("bad create");
		if (MyError(IDS_FILE_CREATE, (LPCSTR)fileName, MB_OKCANCEL) == IDCANCEL)
			{
			return;
			}
		}
	DPF("before shell");
	SHELLEXECUTEINFO ShellInfo; // Name structure
	memset(&ShellInfo, 0, sizeof(ShellInfo)); // Set up memory block
	ShellInfo.cbSize = sizeof(ShellInfo); // Set up structure size
	ShellInfo.hwnd = AfxGetMainWnd()->m_hWnd;
	ShellInfo.lpVerb = "edit"; // Open the file with default program
	ShellInfo.lpFile = fileName; // File to open
	ShellInfo.nShow = SW_NORMAL; // Open in normal window
	ShellInfo.fMask = SEE_MASK_NOCLOSEPROCESS; // Necessary if you want to wait for
												//spawned process
	BOOL res = ShellExecuteEx(&ShellInfo); // Call to function
	DPF("res:%d",res);
	CExternalDlg edlg;
	if (res)
		strcpy(edlg.m_name, fileName);
	else
		{
		RemoveFile(fileName);
		return;
		}
//		edlg.m_name[0] = 0;
	AfxGetMainWnd()->ShowWindow(SW_MINIMIZE);
	int eres = edlg.DoModal();
	DPF("eres:%d",eres);

	AfxGetMainWnd()->ShowWindow(SW_SHOWNORMAL);
	if (eres != IDOK)
		{
		RemoveFile(fileName);
		return;
		}
	CDib dib;
	BOOL bError = 0;
	if (dib.Import(fileName))
		bError = TRUE;
	RemoveFile(fileName);
	if (bError)
		{
		MyError(IDS_FILE_READ,ShellInfo.lpFile);
		DPF("make cell error");
		return;
		}
	LPBITMAPINFOHEADER  lpbi;
	if (Level && dlg.m_b24)
		{
		lpbi = ( LPBITMAPINFOHEADER)tdib.m_pBMI;
		BYTE * pDst = dib.m_pBits;
		BYTE * pSrc = tdib.m_pBits;
		UINT x, y;
		UINT op = 4 * ((3 * cw + 3) / 4);
		UINT ip = 4 * cw;
		for (y = 0; y < ch; y++)
			for (x = 0; x < cw; x++)
				{
				if ((pDst[y*op+3*x+0] != pSrc[y*ip+4*x+0]) ||
					(pDst[y*op+3*x+1] != pSrc[y*ip+4*x+1]) ||
					(pDst[y*op+3*x+2] != pSrc[y*ip+4*x+2]))
						{
						pSrc[y*ip+4*x+0] = pDst[y*op+3*x+0];
						pSrc[y*ip+4*x+1] = pDst[y*op+3*x+1];
						pSrc[y*ip+4*x+2] = pDst[y*op+3*x+2];
						if (dlg.m_bKeying &&
								(pDst[y*op+3*x+0] > dlg.m_nAlpha) &&
								(pDst[y*op+3*x+1] > dlg.m_nAlpha) &&
								(pDst[y*op+3*x+2] > dlg.m_nAlpha))
							pSrc[y*ip+4*x+3] = 0;
						else
							pSrc[y*ip+4*x+3] = 255;
						}
				}
		}
	else
		{
		lpbi = ( LPBITMAPINFOHEADER)dib.m_pBMI;
		}
	WORD z = CreateCell(Frame,Level,lpbi,128,1,Level ? 1 : 0);
	if (z)
		{
		MyError(IDS_FILE_READ,fileName);
		DPF("make cell error:%d",z);
		}
}

BOOL CSketchDoc::ExistCheck(UINT frame, UINT level,BOOL bSingle, UINT & flag)
{
	BOOL bAnswer;
	if (bSingle)
		{
		if (!flag)
			return 0;
		if (!m_pScene->GetCellKey(frame,level))
			return 0;
		CExistOneDlg dlg;
		dlg.m_Frame = frame+1;
		dlg.m_Level = level;
		int res = dlg.DoModal();
		if (res == IDC_YESREST)
			{
			bAnswer = 0;
			flag = 0;
			}
		else if (res == IDOK)
			bAnswer = 0;
		else 
			bAnswer = 1;
		}
	return bAnswer;
}


/////////////////////////////////////////////////////////////////////////////
// CSketchDoc diagnostics

#ifdef _DEBUG
void CSketchDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CSketchDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}


#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSketchDoc commands

typedef struct {
	char Id[16];
	UINT version;
	UINT owidth;
	UINT oheight;
	UINT width;
	UINT height;
	UINT flags;
	UINT frames;
	UINT levels;
} LPSHEADER;

void CSketchDoc::ProcessDifferentPalette(UINT index)
{
	CNewPals * pPals = m_pScene->PalettePtr(index);
	if (!pPals) return; // deleted
	for (;;)
		{
		CPalDifferentDlg dlg;
		dlg.m_fname = pPals->GetFileName();
		int res = dlg.DoModal();
		if (res != IDOK)
			continue;
		if (dlg.m_bBreak)
			{
			pPals->SetFileName("");
			m_pScene->NewPalName(dlg.m_name, index);
			}
		else
			{
			pPals->PaletteIO(dlg.m_fname,0);
			UINT z = pPals->PaletteIO(dlg.m_fname,0);
			if (z == 1)
				{
				MyError(IDS_FILE_OPEN, (LPCSTR)dlg.m_fname);
				}
			else if (z == 2)
				{
				MyError(IDS_FILE_READ, (LPCSTR)dlg.m_fname);
				}
			if (z)
				continue;
			m_pScene->PalChanged(index);
			m_pScene->Flag(SCN_FLG_DIRTY,1,1);	// sets dirty flag
			}
		break;
		}
}

void CSketchDoc::ProcessUnusedPalette(UINT index)
{
	CNewPals * pPals = m_pScene->PalettePtr(index);
	for (;;)
		{
		CPalUnusedDlg dlg;
		dlg.m_name = pPals->GetPalName();
		int res = dlg.DoModal();
		if (res == 99)
			break;
		else if (res == IDOK)
			{
			m_pScene->DeletePalette(index);
			break;
			}
		}
}

void CSketchDoc::ProcessMissingPalette(UINT index)
{
	CNewPals * pPals = m_pScene->PalettePtr(index);
	if (!pPals) return; // deleted
	for (;;)
		{
		CPalMissingDlg dlg;
		dlg.m_fname = pPals->GetFileName();
		int res = dlg.DoModal();
		if (res != IDOK)
			continue;
		if (dlg.m_bBreak)
			{
			pPals->SetFileName("");
			m_pScene->NewPalName(dlg.m_name,index);
			break;
			}
		CString fileName = dlg.m_fname;
		if (!((CSketchApp*) AfxGetApp())->PromptFileName(fileName, 6))
			continue;
		char pname[50];
		LPCSTR name = (LPCSTR)fileName;
		int j = 0;
		int i;
		for (i = 0; name[i]; i++)
			{
			if (name[i] == NATIVE_SEP_CHAR)
				j = i + 1;
			if (name[i] == '.')
				break;
			}
		int zz;
		for (zz = 0; ((zz + 2) < sizeof(pname)) && (j <  i);)
			pname[zz++] = name[j++];
		pname[zz] = 0;
		UINT z = pPals->PaletteIO(fileName,0);
		if (z == 1)
			{
			MyError(IDS_FILE_OPEN, (LPCSTR)fileName);
			}
		else if (z == 2)
			{
			MyError(IDS_FILE_READ, (LPCSTR)fileName);
			}
		if (z)
			continue;
		pPals->SetPalName(pname);
		pPals->SetFileName(fileName);
		m_pScene->PalChanged(index);
		m_pScene->Flag(SCN_FLG_DIRTY,1,1);	// sets dirty flag
		break;
		}
}

void CSketchDoc::ProcessMissingTexture(UINT index, UINT pindex)
{
	CNewPals * pPals = m_pScene->PalettePtr(index);
	if (!pPals) return; // deleted
	for (;;)
		{
		CMissingTextureDlg dlg;
		dlg.m_fname = pPals->FileName(pindex);
		int res = dlg.DoModal();
		if (res == IDOK)
			{
			pPals->Kind(pindex,0);
//			m_pScene->PalChanged(index);
			break;
			}
		else if (res == 99)
			{
			CString fileName = dlg.m_fname;
			if (!((CSketchApp*) AfxGetApp())->PromptFileName(fileName, 13))
				continue;
			pPals->LoadTexture(pindex,fileName);
			break;
			}
		}
	m_pScene->PalChanged(index);
	m_pScene->Flag(SCN_FLG_DIRTY,1,1);	// sets dirty flag
}

void CSketchDoc::ProcessEmbeddedFiles()
{
	CEmbedInDlg dlg;
	dlg.m_pIO = m_pIO;
	int opt;
	int v = Option(OVERRIDE_FLAGS);
	if (v & 4) // embed override
		opt = (v >> 4) & 3;	// use in overide
	else
		opt = Option(SC_EMBED_IN);
	if (!((CSketchApp*)AfxGetApp())->IsAnimate())
		opt = 1;
	if (opt == 0) // never
		m_pIO->EmbFlag(EMB_RESET_ALL, EMB_ENABLED);
	else
		if (m_pIO->EmbFlag(EMB_ANY_SET,EMB_ENABLED) && (opt == 2))
		{
		dlg.DoModal();
		}
}

BOOL CSketchDoc::SaveEmbeddedFile(LPCSTR name, int kind)
{
	UINT idx = m_pIO->EmbFind(name, kind);
	if ((idx != NEGONE) && m_pIO->EmbFlag(idx, EMB_SEEN))
		return 1;				// don not add it twice
	if (idx == NEGONE)
		{
		idx = m_pIO->EmbAdd(name,kind);
		m_pIO->EmbFlag(idx,EMB_SEEN,1);
		}
	m_pIO->EmbFlag(idx,EMB_DO_SAVE,1); // set do save 
	return 1; 
}


BOOL CSketchDoc::SaveEmbeddedFiles(LPCSTR pName)
{
	int opt;
	int v = Option(OVERRIDE_FLAGS);
	if (v & 4) // embed override
		opt = (v >> 6) & 3;	// use out overide
	else
		opt = Option(SC_EMBED_OUT);
	if (opt == 2)
		{
		m_pIO->EmbFlag(EMB_RESET_ALL,EMB_DO_SAVE); // reset all do save flags
		CEmbedOutDlg dlg;
		dlg.m_pIO = m_pIO;
		dlg.m_pScene = m_pScene;
		if (dlg.DoModal() != IDOK)
			{
			m_pIO->EmbFlag(EMB_SET_ALL,EMB_DO_SAVE); // set all do save flags
			/// this only saves already embedded files
			}
		}
	else if (opt == 1)		// always
		{
		m_pIO->EmbFlag(EMB_SET_ALL,EMB_DO_SAVE); // set all do save flags
		m_pIO->EmbFlag(EMB_RESET_ALL,EMB_SEEN);
		int idd;
		char name[300];
		for (idd = 0; idd < 3; idd++)
			{
			m_pScene->SceneOptionStr(SCOPT_WAVE0+idd,name);
			if (name[0])
				SaveEmbeddedFile(name,EMB_KIND_SOUND);
			}
		UINT level, levels;
		levels = m_pScene->LevelCount();
		for (level = 0; level < levels; level++)
			{
			m_pScene->LevelModelName(name, level);
			if (name[0])
				SaveEmbeddedFile(name,EMB_KIND_MODEL);
			}
		}
	else
		{
		m_pIO->EmbFlag(EMB_RESET_ALL,EMB_DO_SAVE); // reset all do save flags
		}
	BOOL bHaveSome = m_pIO->EmbFlag(EMB_ANY_SET,EMB_DO_SAVE); // true if any are to be saved
	m_pIO->EmbUnHook(pName,bHaveSome);
	return bHaveSome;
}

void CSketchDoc::OnFrameNumber()
{
	CFrameNumberDlg dlg;
	int old = Option(SC_APPLY_FRAME) & 31;
	dlg.m_corner = old & 3;
	dlg.m_start = (old >> 2) & 1; 
	dlg.m_bPlayback = (old >> 3) & 1; 
	dlg.m_bPublish = (old >> 4) & 1; 
	if (dlg.DoModal() == IDOK)
		{
		int v = dlg.m_bPublish;
		v = v * 2 + dlg.m_bPlayback;
		v = v * 2 + dlg.m_start;
		v = v * 4 + dlg.m_corner;
		if (v != old)
			{
			Option(SC_APPLY_FRAME, 1, v);
			m_pScene->UpdateCache();
			}
		}
}

