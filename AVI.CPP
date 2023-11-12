#include "stdafx.h"
#include "sketch.h"
#include "mydoc.h"
#include "myview.h"
#include "newdlg.h"
//#include "avidlg.h"
#include "dialogs.h"
#include "cscene.h"
#include "sceneopt.h"
#include "mysound.h"
#include "nscaler.h"
#include <vfw.h>
#include "mygif.h"
#include "fbqt.h"
#if !MAC
#include "dsimport.h"
#endif
#include "myio.h"
#include "WinResourceLoader.h"
#include "utils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif
//#define BPP 3

/*
	following is callback class for crpogdlg kind = 2
	
*/
class CGifCallBack
{
public:
CGifCallBack();
~CGifCallBack();
BOOL Init(CFile  * file, UINT origw,UINT origh,UINT w,UINT h,UINT c,
			UINT startframe,UINT startlevel, 
				UINT endframe,UINT endlevel,UINT rate,UINT bLoop);
BOOL Perform(UINT index);
BOOL Finish();
	CScene * m_pScene;
	int m_option;
protected:
	UINT m_w;
	UINT m_h;
	UINT m_origw;
	UINT m_origh;
	CGScaler * m_pScale;
	CMyGif * m_pGif;
//	CScene * m_pScene;
	UINT m_d;	// source data depth
	UINT m_z;	// dialog option
	UINT m_startframe;
	UINT m_endframe;
	UINT m_startlevel;
	UINT m_endlevel;
	UINT m_pitch;
	BYTE * m_hpBuf1;
	BYTE * m_hpBuf2;
};

class CExportMovie
{
public:
CExportMovie();
~CExportMovie();
BOOL Init(LPCSTR name, UINT origw,UINT origh,UINT w,UINT h,UINT d,
			UINT startframe,UINT startlevel, 
				UINT endframe,UINT endlevel,UINT rate,UINT nTelecine, CMySound* pSound);
BOOL Perform(UINT index);
BOOL Finish();
	CScene * m_pScene;
protected:
	UINT m_w;
	UINT m_h;
	UINT m_d;
	UINT m_rate;
	UINT m_pitch;
	UINT m_size;
	CGScaler * m_pScale;
	BYTE * m_pQT;
	int m_tfactor;
	int m_toffset;
	UINT m_startframe;
	UINT m_endframe;
	UINT m_startlevel;
	UINT m_endlevel;
	BYTE * m_hpBuf1;
	BYTE * m_hpBuf2;
	BYTE * m_hpTBuf;
	CMySound* m_pSound;
};

class CImportMovie
{
public:
CImportMovie();
BOOL Init(void * pMov, UINT frame, UINT level, UINT firstmov);
BOOL Perform(UINT index);
int  ProcessDibData(void * pData, UINT ssize);
	CSketchDoc * m_pDoc;
	float m_frameRateAdjustment;
protected:
	void * m_pMov;
	UINT m_frame;
	UINT m_level;
	UINT m_firstmov;
};

#ifdef USEQT
/*
	Export Movie Callback is used by CProg2Dlg to export each frame
*/
unsigned int ExportMovieCallback(unsigned int index,
						void *UserObject, void *UserData )
{
	return ((CExportMovie *)UserObject)->Perform(index);
}
#endif

#ifndef FLIPBOOK_MAC
BOOL GoodWave( PAVISTREAM 	paudio);
BOOL PutWave( LPCSTR Name, PAVISTREAM 	paudio);
#endif

class CAviInDlg : public CMyDialog
{
// Construction
public:
	CAviInDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CAviInDlg)
	enum { IDD = IDD_AVIINDLG };
	UINT	m_frame;
	UINT	m_level;
	UINT	m_start;
	UINT	m_stop;
	UINT	m_maxframe;
	UINT	m_maxlevel;
	UINT	m_maxavi;
	BOOL 	m_nDiffRate;	// 0 is same, 1 is faster, 2 is slower
	BOOL	m_bRateIt;
	int		m_nDiffSize;	// 0 is same , 1 is larger, 2 is smaller
	int 	m_nBGDiffSize;	// diff if level 0, 3 if not BG
	BOOL	m_bScaleIt;
	BOOL	m_bDiffAspect;
	BOOL	m_bHoldAspect;
	UINT	m_nRate;
	UINT	m_nWidth;
	UINT	m_nHeight;
//	BOOL m_bEnableExtract;
//	BOOL m_bDoExtract;
	int	m_nMaxSoundCol;
	int	m_nTrack;
	BOOL m_nKind;
	//}}AFX_DATA
// Implementation
protected:
	void SizeStuff(int level);
	afx_msg void OnLevelChange();
    virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	// Generated message map functions
	//{{AFX_MSG(monitor)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


#if !MAC
int ProgCB(int code, void * pClass)
{
	CDSImport * pDSIM =  (CDSImport *)pClass;
	return pDSIM->Function(code);
}
#endif

class AviInBlock {
public:
	AviInBlock() { m_pDib = 0;m_bOpened = 0;m_name[0] = 0;}
	~AviInBlock(){delete [] m_pDib;Close(1);}
	BOOL Open()
		{
		if (!m_name[0])
			{
			GetTempPath(450,m_name);
			strcat_s(m_name,"FBAVI.TMP");
			}
		Close();
		DWORD mode = CFile::modeCreate | CFile::modeReadWrite;
		if (!m_file.Open(m_name, mode))
			return 1;
		m_file.Seek(0, CFile::begin);
		m_bOpened = TRUE;
		return 0;
		}
	void Rewind()
		{
			m_file.Seek(0, CFile::begin);
		}
	void Close(BOOL bErase = 0)
		{
		if (m_bOpened)
			m_file.Close();
		m_bOpened = FALSE;
		if (bErase)
			{
			TRY
				{
				m_file.Remove(m_name);
				}
			CATCH (CFileException, e)
				{
#if !MAC
				DPF2("bad erase:%s|",m_szName);
#endif
				}
			END_CATCH;
			}
		}
	int Handler(int code, void * pBuf)
		{
	if (code)
		{
			ASSERT(m_pDib == 0);
		BITMAPINFOHEADER * lpbi = (BITMAPINFOHEADER *)pBuf;
		UINT pitch = 4 * ((3 * lpbi->biWidth + 3) / 4);
		m_size = pitch * lpbi->biHeight;
		m_asize = lpbi->biSizeImage;
		m_count = 0;
		m_pDib = new BYTE[lpbi->biSize + m_size];
		memcpy(m_pDib, lpbi, lpbi->biSize);
		m_pBuf = m_pDib + lpbi->biSize;
		}
	else
		{
		m_file.Write(pBuf,m_asize);
		m_count++;
		}
	return 0;
	}
	LPBITMAPINFOHEADER GetDib(UINT index)
		{
		LPBITMAPINFOHEADER  lpbi = (LPBITMAPINFOHEADER)m_pDib;
		if (m_size != m_asize)
			{
			UINT pitch = 4 * ((3 * lpbi->biWidth + 3) / 4);
			int i;
			for (i = 0; i < lpbi->biHeight; i++)
				m_file.Read(m_pBuf + pitch * i, 3 * lpbi->biWidth);				
				
			}
		else
			m_file.Read(m_pBuf,m_size);				
		return (LPBITMAPINFOHEADER)m_pDib;
		}
	UINT m_size;
	UINT m_asize;
	BYTE * m_pDib;
	BYTE * m_pBuf;
	CFile m_file;
	BOOL m_bOpened;
	UINT m_count;
	char m_name[500];
};


int AviInCB(int code, void * pData, void * pClass)
{
	AviInBlock * pBlock = (AviInBlock *)pClass;
	return pBlock->Handler(code,pData);
}

class CAviProcess
{
public:
void Setup( CSketchDoc * pDoc, UINT frame, UINT level,
				UINT scene_rate, UINT avi_rate,
				BOOL bScale, BOOL bAspect, AviInBlock * pBlock)
	{
	m_pDoc = pDoc;
	m_Frame = frame;
	m_Level = level;
	m_expected = 0;
	m_bScale = bScale;
	m_bAspect = bAspect;
	m_pBlock = pBlock;
	m_scene_rate = scene_rate;
	m_avi_rate = avi_rate;
	}
UINT Perform(UINT index)
	{
	if (m_avi_rate)
		{
		index = index * m_scene_rate / m_avi_rate;
		if (index > m_expected)
			return 0;
		m_expected = index + 1;
		}
	LPBITMAPINFOHEADER lpBI = m_pBlock->GetDib(index);
	UINT arg = 0;
	if (m_bScale)
		{
		arg++;
		if (!m_bAspect)
			arg |= 2;
		}
//	arg = 8;
//	arg = 3;
	if (m_pDoc->CreateCell(m_Frame+index,m_Level,lpBI,0,1,arg))
		return 1;
	return 0;
	};
protected:
	CSketchDoc * m_pDoc;
	AviInBlock * m_pBlock;
	BOOL m_bScale;
	BOOL m_bAspect;
	UINT m_Frame;
	UINT m_Level;
	UINT m_scene_rate;
	UINT m_avi_rate;
	UINT m_expected;
};

/*
	Callback is used by CProg2Dlg to update cells
*/
unsigned int UpdateCellCallback(unsigned int index,
						void *UserObject, void *UserData )
{
	return ((CAviProcess *)UserObject)->Perform(index);
}

BOOL CSketchDoc::ChooseAVI(CString &filename)
{
	GetDefFolder(filename,UDF_IMP);
	filename += NATIVE_SEP_STRING;
	filename += "*.avi;*.wmv;*.mov";
	CFileDialog dlgFile(1);
	CString title;
	CString strFilter = "";
	strFilter += "Video Files (*.avi,*.mov,*.wmv)";
	strFilter += CHAR(0);
	strFilter += "*.avi;*.mov;*.wmv";
	strFilter += CHAR(0);
	strFilter += "AVI Files (*.avi)";
	strFilter += CHAR(0);
	strFilter += "*.avi";
	strFilter += CHAR(0);
	strFilter += "WMV Files (*.wmv)";
	strFilter += CHAR(0);
	strFilter += "*.wmv";
	strFilter += CHAR(0);
	strFilter += "MOV Files (*.mov)";
	strFilter += CHAR(0);
	strFilter += "*.mov";
	strFilter += CHAR(0);
	strFilter += "All Files (*.*)";
	strFilter += CHAR(0);
	strFilter += "*.*";
	strFilter += CHAR(0);
	strFilter += CHAR(0);
	title = "Open Media File";
	dlgFile.m_ofn.nMaxCustFilter = 3;
	dlgFile.m_ofn.lpstrDefExt = "*.avi";
	dlgFile.m_ofn.lpstrFilter = strFilter;
	dlgFile.m_ofn.hwndOwner = AfxGetMainWnd()->GetSafeHwnd();
	dlgFile.m_ofn.lpstrTitle = title;
	dlgFile.m_ofn.lpstrFile = filename.GetBuffer(_MAX_PATH);
	BOOL bRet = dlgFile.DoModal() == IDOK ? TRUE : FALSE;
	filename.ReleaseBuffer();
	return bRet;
}

void CSketchDoc::OnImportAVI()
{
#ifndef FLIPBOOK_MAC
	DPF("on import avi");
	CString filename;
	if (ChooseAVI(filename))
		{
		UpdateDefFolder(filename,UDF_IMP);
		ImportAVIFile((LPCSTR)filename);
		}
#endif
}

BOOL CSketchDoc::FromAVI()

{
	UINT ApId;
	CSketchApp* pApp = (CSketchApp *)AfxGetApp();
	if (pApp->IdCheck(ApId,m_dwFeatures))
		return FALSE;
//	CMainFrame * pFrame = (CMainFrame*) pApp->m_pMainWnd;
//	if (!pFrame)
//		return FALSE;
	CString filename;
	if (ChooseAVI(filename))
		UpdateDefFolder(filename,UDF_IMP);
	else
		return FALSE;

	AviInBlock * pBlock = new AviInBlock(); 
	CDSImport * pImport = new CDSImport;
	pImport->SetCallback( AviInCB, pBlock);
	int res = pImport->SelectFile((LPCSTR)filename,1);
	if (!res && !pImport->Frames())
		res = 7;
	if (res)
		{
		char buf[30];
		sprintf_s(buf,"DSImport Error:%d",res);
		AfxMessageBox(buf);
		delete pBlock;
		delete pImport;
		return FALSE;
		}
	CFromAVIDlg dlg;
	dlg.m_Frames = pImport->Frames();
	dlg.m_Levels = 2;
	dlg.m_nRate = pImport->Rate();
	dlg.m_nWidth = pImport->Width();
	dlg.m_nHeight = pImport->Height();
	dlg.m_bUseSound = pImport->HasSound();
	dlg.m_nJpeg = Option(JPEG_QUAL);
	dlg.m_MaxFrames = MaxFrames();
	MaxSizes(dlg.m_MaxWidth, dlg.m_MaxHeight);
	if (dlg.DoModal() != IDOK)
		{
		delete pBlock;
		delete pImport;
		return FALSE;
		}
	Option(JPEG_QUAL,1,dlg.m_nJpeg);
	
	if (IsModified())
		TRACE0("Warning: OnNewDocument replaces an unsaved document\n");

	DeleteContents();
	m_strPathName.Empty();      // no path name yet



//	Option(DEF_KIND,1,dlg.m_Kind);
/*
	Option(DEF_WIDTH,1,dlg.m_Width);
	Option(DEF_HEIGHT,1,dlg.m_Height);
	Option(DEF_FRAMES,1,dlg.m_Frames);
	Option(DEF_LEVELS,1,dlg.m_Levels);
	Option(DEF_RATE,1,dlg.m_Rate);
	Option(DEF_FACTOR,1,dlg.m_Factor);
	Option(DEF_PREVIEW,1,dlg.m_Preview);
	Option(DEF_BROADCAST,1,dlg.m_bcast);
	Option(JPEG_QUAL,1,dlg.m_nJpeg);
*/
	CFileStatus status;
	char name[500];
	char suffer[20];
	int i,j;
	for (i = 0, j = 0; name[i] = ((LPCSTR)filename)[i]; i++)
		if (name[i] == '.')
			j = i + 1;
	if (j)
		j--;
	else
		j = i;		// j should be at end
	for ( i= 0; i < 10000; i++)
		{
		name[j] = 0;
		if (i)
			sprintf_s(suffer,"%d.DGC",i);
		else
			sprintf_s(suffer,".DGC",i);
		strcat_s(name,suffer);
		if (!CFile::GetStatus(name,status))
			break;
		}
DPF("width:%d,height:%d",dlg.m_nWidth,dlg.m_nHeight);
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
		rate = dlg.m_nRate;
		levels = dlg.m_Levels;
		frames = dlg.m_Frames;
		factor = 0;//dlg.m_Factor;
		int bCast = 1;
		int Preview = 0;
DPF("new, fact:%d",factor);
		SceneDefaults();
		result = m_pScene->Make(ApId, m_dwFeatures,
				dlg.m_nWidth,dlg.m_nHeight,rate,frames,levels,
						factor, Preview,dlg.m_nJpeg,bCast);
		}
	if (result)
		{
		delete pBlock;
		delete pImport;
DPF("result:%d",result);
		MyError(IDS_SCENE_CREATE, MB_ICONINFORMATION | MB_OK);
		DeleteContents();
		return FALSE;
		}
//	qm_pScene->SceneOption(SCOPT_HOLD, Option(DEF_HOLD));
//	OptionLoadSave();
//	SetPathName(pszPathName);
	OpenInit(name, TRUE);
	CMainFrame * pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
	if (pFrame)
		{
		pFrame->AttachDoc(this);
		pFrame->Opener(TRUE);
		}
	
	if (pBlock->Open())
		{
		AfxMessageBox("Cannot Create Temp File");
		delete pBlock;
		delete pImport;
		return 0;
		}
	UINT start = 0;
	UINT stop = dlg.m_Frames - 1;
	UINT level = 0;
	CProgPullDlg pdlg;
	pImport->SetRange(start, stop, level);
	pdlg.SetRange(start, stop);
	pdlg.SetCallback(ProgCB,pImport);
	BeginBusy("Importing Video Frames");
	if ( pdlg.DoModal() != IDOK)
	{
		delete pBlock;
		delete pImport;
		return 0;
	}
	BOOL bModified = 0;
	if (pBlock->m_count)
		{
		pBlock->Rewind();
		bModified = TRUE;
		CAviProcess * pProcess = new CAviProcess;
		UINT vrate = pImport->Rate();
		pProcess->Setup(this, 0,0,
				m_pScene->FrameRate(), vrate, dlg.m_bScaleIt,0,pBlock);
		CProg2Dlg progger;
		progger.ProgSetup( UpdateCellCallback, pProcess,0,
		0, pBlock->m_count, IDS_AVI_IMPORT);
		progger.DoModal();
		delete pProcess;
		pBlock->Close();
		}
	EndBusy();
	if (dlg.m_bUseSound)
		{
		char tname[300];
		int a, b;
		for (a = 0, b = 500; a < 299; a++)
			{
			tname[a] = name[a];
			if (tname[a] == '.')
				b = a;
			else if (!tname[a])
				break;
			}
		if (b < 500)
			tname[b+1] = 0;
		else
			strcat_s(tname,".");
		strcat_s(tname,"WAV");
		pImport->WriteWave(tname);
		if (pFrame)
			{
			pFrame->SetSound(TRUE,0, 0.0,1,NEGONE,tname);
			}
		else
			m_nNeedHelp = 1;
		}
	if (!pFrame)
		m_nNeedHelp |= 2;
	delete pBlock;
	delete pImport;
	if (bModified)
		SetModifiedFlag(TRUE);
	return TRUE;
}

BOOL CSketchDoc::CheckHelp()
{
	BOOL bRes = (m_nNeedHelp & 2) ? 1 : 0;
	if (m_nNeedHelp & 1)
		{
		LPCSTR name = (LPCSTR)GetPathName();
		char tname[300];
		int a, b;
		for (a = 0, b = 500; a < 299; a++)
			{
			tname[a] = name[a];
			if (tname[a] == '.')
				b = a;
			else if (!tname[a])
				break;
			}
		if (b < 500)
			tname[b+1] = 0;
		else
			strcat_s(tname,".");
		strcat_s(tname,"WAV");
		double smark = 0.0;
		CMainFrame * pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
		if (pFrame)
			pFrame->SetSound(TRUE,0, smark,1,NEGONE,tname);
		}
	m_nNeedHelp = 0;
	return bRes;
}

void CSketchDoc::ImportAVIFile(LPCSTR szFile)
{
#ifndef FLIPBOOK_MAC
	char name[500];
	strcpy_s(name,szFile);
//	UpdateDefFolder(fileName,UDF_IMP);
	AviInBlock * pBlock = new AviInBlock(); 
	CDSImport * pImport = new CDSImport;
	pImport->SetCallback( AviInCB, pBlock);
	int res = pImport->SelectFile(name,1);
	if (!res && !pImport->Frames())
		res = 7;
	if (res)
		{
		char buf[30];
		sprintf_s(buf,"DSImport Error:%d",res);
		AfxMessageBox(buf);
		delete pBlock;
		delete pImport;
		return;
		}

	UINT nFrames = pImport->Frames();
	BOOL bHasSound = pImport->HasSound();
	CAviInDlg dlg;
	CMainFrame * pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
	dlg.m_nTrack = 0;
	dlg.m_nMaxSoundCol = bHasSound ? pFrame->MaxSoundCols() : 0;
	if (!pFrame->GetSelection(dlg.m_frame,dlg.m_level))
		{
		CSketchView* pSketchView = GetDocView();
		dlg.m_frame = pSketchView->CurrentFrame();
		dlg.m_level = pSketchView->CurrentLevel();
		}
	int result;
	dlg.m_frame++;
	dlg.m_maxframe = m_pScene->FrameCount();
	dlg.m_maxlevel = m_pScene->LevelCount();
	dlg.m_start = 1;
	dlg.m_maxavi = dlg.m_stop = nFrames;
	dlg.m_nRate = pImport->Rate();
	if (m_pScene->FrameRate() == pImport->Rate())
		dlg.m_nDiffRate = 0;
	else if (m_pScene->FrameRate() > (UINT)pImport->Rate())
		dlg.m_nDiffRate = 2;
	else
		dlg.m_nDiffRate = 1;
	dlg.m_bRateIt = 0;
	dlg.m_bScaleIt = 0;
	dlg.m_bDiffAspect = 0;
	if ((m_pScene->Width() == pImport->Width()) &&
				(m_pScene->Height() == pImport->Height()))
		dlg.m_nBGDiffSize = 0;
	else if (m_pScene->Width() * m_pScene->Height() > 
			(UINT)pImport->Width() * pImport->Height())
		dlg.m_nBGDiffSize = 1;
	else
		dlg.m_nBGDiffSize = 2;
	if (dlg.m_nBGDiffSize && 
				(m_pScene->Width() * pImport->Height() != 
						m_pScene->Height() * pImport->Width()))
		dlg.m_bDiffAspect = 1;
	dlg.m_bScaleIt = dlg.m_nBGDiffSize ? 1 : 0;
	dlg.m_nWidth = pImport->Width();
	dlg.m_nHeight = pImport->Height();

	BOOL bModified = FALSE;
	for (;;)
		{
		result = dlg.DoModal();
		if (result != IDOK)
			break;
		UINT future_frames = dlg.m_stop + 1 - dlg.m_start;
		if (dlg.m_nDiffRate && dlg.m_bRateIt)
			future_frames = future_frames * m_pScene->FrameRate() /
								pImport->Rate();
		future_frames += dlg.m_frame - 1;
		UINT new_frames = 0;
		if (future_frames > MaxFrames())
			{
			result = MyError(IDS_TOO_BIG_AVI, MB_YESNOCANCEL);
			if (result == IDYES)
				new_frames = MaxFrames();
			}
		else if (future_frames > m_pScene->FrameCount())
			{
			result = MyError(IDS_BIG_AVI, MB_YESNOCANCEL);
			if (result == IDYES)
				new_frames = future_frames;
			}
		else
			break;
		if (new_frames)
			{
			result = IDOK;
			CMainFrame * pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
			pFrame->ChangeFrames(new_frames, new_frames);
			new_frames -= dlg.m_frame - 1;
			if (dlg.m_nDiffRate && dlg.m_bRateIt)
				new_frames = new_frames * pImport->Rate() /
							m_pScene->FrameRate();
			dlg.m_stop = dlg.m_start + new_frames - 1;
			break;
			}
		if (result == IDNO)
			{
			dlg.m_stop = 
					dlg.m_start + m_pScene->FrameCount() - dlg.m_frame;
			result = IDOK;
			break;
			}
		}
	if (result == IDOK)
		{
		if (pBlock->Open())
			{
			AfxMessageBox("Cannot Create Temp File");
			delete pBlock;
			delete pImport;
			return;
			}
		CProgPullDlg pdlg;
		pImport->SetRange(dlg.m_start-1, dlg.m_stop-1, dlg.m_level);
		pdlg.SetRange(dlg.m_start-1, dlg.m_stop-1);
		pdlg.SetCallback(ProgCB,pImport);
	BeginBusy("Importing Video Frames");
		int res = pdlg.DoModal();
		if (pBlock->m_count)
			{
			pBlock->Rewind();
			bModified = TRUE;
			CAviProcess * pProcess = new CAviProcess;
			UINT vrate = dlg.m_bRateIt ? pImport->Rate() : 0;
			pProcess->Setup(this, dlg.m_frame-1,dlg.m_level,
				m_pScene->FrameRate(), vrate,
				dlg.m_bScaleIt, dlg.m_bHoldAspect,pBlock);
			CProg2Dlg progger;
			progger.ProgSetup( UpdateCellCallback, pProcess,0,
						dlg.m_frame, pBlock->m_count, IDS_AVI_IMPORT);
			progger.DoModal();
			delete pProcess;
			pBlock->Close();
			}
	EndBusy();
		if( res == IDOK)
			{
			if (dlg.m_nTrack)
				{
				char tname[300];
				if (dlg.m_nDiffRate && !dlg.m_bRateIt)
//				if (m_pScene->FrameRate() != pImport->Rate())
					{
					sprintf_s(tname,
						"Frame rate of sound (%d) differs from scene",
									pImport->Rate());
					AfxMessageBox(tname);
					}
				int a, b;
				for (a = 0, b = 500; a < 299; a++)
					{
					tname[a] = name[a];
					if (tname[a] == '.')
						b = a;
					else if (!tname[a])
						break;
					}
				if (b < 500)
					tname[b+1] = 0;
				else
					strcat_s(tname,".");
				strcat_s(tname,"WAV");
				pImport->WriteWave(tname);
				double smark = 0.0;
				if (pImport->Rate() > 4) // avoid large number
					smark = (1000.0 * (dlg.m_start - 1)) / pImport->Rate();
				pFrame->SetSound(TRUE,dlg.m_nTrack-1, smark,
								dlg.m_frame,NEGONE,tname);
//		HINSTANCE res  = ShellExecute(0, "open", "test.wav", 0, 0, 1);
				}				
			}
		}
	delete pBlock;
	delete pImport;
	if (bModified)
		SetModifiedFlag(TRUE);
#endif
}

CAviInDlg::CAviInDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CAviInDlg::IDD, pParent)
{
	m_nKind = 0;
	m_nTrack = 0;
	m_bScaleIt = 0;
	m_bRateIt = 0;
	m_bHoldAspect = 0;
	m_nMaxSoundCol = 0;
	m_nDiffRate = 0;
	m_nDiffSize = 0;
	m_bDiffAspect = 0;
}

#define MYDDV_MinMaxInt(a,b,c,d,e)\
		DDX_Text(a, b, c); DDV_MinMaxInt(a, c, d, e);
void CAviInDlg::DoDataExchange(CDataExchange* pDX)
{
	DPF("data xchg");
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAviInDlg)
	MYDDV_MinMaxInt(pDX, IDC_AVIINSTART, m_start,1, m_maxavi);
	MYDDV_MinMaxInt(pDX, IDC_AVIINSTOP, m_stop,m_start,m_maxavi);
	MYDDV_MinMaxInt(pDX, IDC_START_FRAME, m_frame,1,m_maxframe);
	MYDDV_MinMaxInt(pDX, IDC_START_LEVEL, m_level,0,m_maxlevel);
	DDX_Radio(pDX, IDC_AVI_TRACK0, m_nTrack);
	DDX_Check(pDX, IDC_AVI_SCALE_IT, m_bScaleIt);
	DDX_Check(pDX, IDC_AVI_RATE_ADJ, m_bRateIt);
	DDX_Check(pDX, IDC_AVI_ASPECT_HOLD, m_bHoldAspect);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAviInDlg, CMyDialog)
	ON_EN_CHANGE(IDC_START_LEVEL, OnLevelChange)
END_MESSAGE_MAP()


void CAviInDlg::SizeStuff(int level)
{
	CString strTemp;
	CString strExtra;
	if (!level)
		m_nDiffSize = m_nBGDiffSize;
	else
		m_nDiffSize = 3;
	UINT show1 = SW_HIDE;
	UINT show2 = SW_HIDE;
	if (m_nDiffSize == 3)
		{
		strTemp.LoadString(IDS_AVIDLG_GRAY);
		if (m_nBGDiffSize)
		SetDlgItemText(IDC_AVI_SPATIAL, strTemp);
		}
	else if (m_nDiffSize)
		{
		GetDlgItem(IDC_AVI_SPATIAL)->EnableWindow(1);
		if (m_nDiffSize == 2)
			strTemp.LoadString(IDS_AVIDLG_LARGER);
		else
			strTemp.LoadString(IDS_AVIDLG_SMALLER);
		strExtra.Format(" (%d,%d)", m_nWidth, m_nHeight);
		strTemp += strExtra;
		SetDlgItemText(IDC_AVI_SPATIAL, strTemp);
		show1 = SW_SHOW;
		GetDlgItem(IDC_AVI_SCALE_IT)->EnableWindow(1);
		if (m_bDiffAspect)
			{
			show2 = SW_SHOW;
			GetDlgItem(IDC_AVI_ASPECT_HOLD)->EnableWindow(1);
			m_bHoldAspect = 1;
			}
		}
	GetDlgItem(IDC_AVI_SCALE_IT)->ShowWindow(show1);
	GetDlgItem(IDC_AVI_ASPECT_HOLD)->ShowWindow(show2);
}

void CAviInDlg::OnLevelChange()
{
	int level = GetDlgItemInt(IDC_START_LEVEL);
	SizeStuff(level);
}

BOOL CAviInDlg::OnInitDialog()
{
	m_bHoldAspect = 0;
	m_bScaleIt = 1;
	CMyDialog::OnInitDialog();
	if (m_nKind)
		{
		SetWindowText("Import Movie Options");
		GetDlgItem(IDC_INAVI_FILE)->SetWindowText("MOV File");
		}
	CString strTemp;
	CString strExtra;
	if (m_nDiffRate)
		{
		GetDlgItem(IDC_AVI_TEMPORAL)->EnableWindow(1);
		if (m_nDiffRate == 2)
			strTemp.LoadString(IDS_AVIDLG_FASTER);
		else
			strTemp.LoadString(IDS_AVIDLG_SLOWER);
		strExtra.Format(" (%d)", m_nRate);
		strTemp += strExtra;
		SetDlgItemText(IDC_AVI_TEMPORAL, strTemp);
		GetDlgItem(IDC_AVI_RATE_ADJ)->EnableWindow(1);
		if (m_nDiffRate == 1)
			strTemp.LoadString(IDS_AVIDLG_DROP);
		else
			strTemp.LoadString(IDS_AVIDLG_ADD);
		SetDlgItemText(IDC_AVI_RATE_ADJ, strTemp);
		}
	SizeStuff(m_level);
	int i;
	for (i = 3; i > m_nMaxSoundCol; i--)
		GetDlgItem(IDC_AVI_TRACK0+i)->EnableWindow(0);
	return FALSE;
}

void Interlace(BYTE * tp, BYTE * hp, UINT pitch, UINT height, UINT off)
{
//	off ^= bOddFirst;
	if (off)
		{
		hp += pitch;
		tp += pitch;
		}
	for ( ; height > 1; height -= 2)
		{
		memcpy(tp, hp, pitch);
		hp += (2*pitch);
		tp += (2*pitch);
		}
}


void CSketchDoc::OnPublish()
{
	DPF("on publish");
	if (CheckModified())
		return;
	
#ifdef USEQT
	char name[300];
	name[0] = '\0';
	CPublishDlg dlg;
	dlg.m_pDoc = this;
	BOOL bGray = !m_pScene->ColorMode();
	dlg.m_opts.fccType = 0;
	dlg.m_opts.fccHandler = 0x6376736d; // video 1
	dlg.m_opts.dwKeyFrameEvery = 0;
	dlg.m_opts.dwQuality = 10000;
	dlg.m_opts.dwBytesPerSecond = 0;
	dlg.m_opts.dwFlags = 8;
	dlg.m_nKind = 4;

	if (!m_pScene->AVIInfo(&name, 299))
		{
		DWORD * pp = (DWORD *)&name;
		pp++; // skip over version
		dlg.m_opts.fccType = *pp++;
		dlg.m_opts.fccHandler = *pp++;
		dlg.m_opts.dwKeyFrameEvery = *pp++;
		dlg.m_opts.dwQuality = *pp++;
		dlg.m_opts.dwBytesPerSecond = *pp++;
		dlg.m_opts.dwFlags = *pp++;
		dlg.m_media = (LPCSTR)pp;
		char * p = (char *)pp;
		int zi,zj;
		zj = 1000;
		for (zi = 0; p[zi]; zi++)
			if (p[zi] == '.')
				zj = zi+1;
		if (zj == 1000)
			zj = zi;
		p += zj;
		if (!_stricmp(p,"gif"))
			dlg.m_nKind = 2;
		else if (!_stricmp(p,"mov"))
			dlg.m_nKind = 1;
		else if (!_stricmp(p,"flv"))
			dlg.m_nKind = 3;
		else
			dlg.m_nKind = 0;
		}
	else
		{
		DoIt(DOIT_LONG,0,&dlg.m_opts.fccType,"fccType",bGray);
		DoIt(DOIT_LONG,0,&dlg.m_opts.fccHandler,"fccHandler",bGray);
		DoIt(DOIT_LONG,0,&dlg.m_opts.dwKeyFrameEvery,"KeyRate",bGray);
		DoIt(DOIT_LONG,0,&dlg.m_opts.dwQuality,"Quality",bGray);
		DoIt(DOIT_LONG,0,&dlg.m_opts.dwBytesPerSecond,"DataRate",bGray);
		DoIt(DOIT_LONG,0,&dlg.m_opts.dwFlags,"Flags",bGray);
		dlg.m_media = AfxGetApp()->GetProfileString("Options","media name",name);
		dlg.m_nKind = AfxGetApp()->GetProfileInt("Options","media kind", 99);
		}

	if (dlg.m_nKind > 3)
		{
		strcpy_s(name,GetPathName());
		int zi,zj;
		zj = 1000;
		for (zi = 0; name[zi]; zi++)
			if (name[zi] == '.')
				zj = zi;
		if (zj == 1000)
			zj = zi;
		name[zj] = 0;
#ifndef FLIPBOOK_MAC
		strcat_s(name,"avi");
		dlg.m_nKind = 0;
#else
		strcat(name,"mov");
		dlg.m_nKind = 2;
#endif
		}
	UINT BPP = bGray ? 1 : 3;
	
	CMainFrame * pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
	CMySound * pSound = pFrame->m_pSound;
	if (!pFrame->GetSelection(dlg.m_StartFrame,dlg.m_StartLevel,
				dlg.m_EndFrame, dlg.m_EndLevel))
		{
		CSketchView* pSketchView = GetDocView();
		dlg.m_StartFrame = dlg.m_EndFrame = pSketchView->CurrentFrame();
		dlg.m_StartLevel = dlg.m_EndLevel = pSketchView->CurrentLevel();
		}
	dlg.m_nKind = AfxGetApp()->GetProfileInt("Options","media kind", 0);
	
	if (dlg.m_bEnableSound = pSound->IsValid())
		dlg.m_bUseSound = Option(SC_QUIET);
	dlg.m_StartFrame++;
	dlg.m_EndFrame++;
	dlg.m_rate = m_pScene->FrameRate();
	dlg.m_bAllowTelecine = 
		((CSketchApp*)AfxGetApp())->CanDoFeature(CSketchApp::CD_TELECINE);
//	if (dlg.m_bAllowTelecine)
		dlg.m_nTelecine = Option(SC_TELECINE);
//	else
//		dlg.m_nTelecine = 0;
	
	dlg.m_MaxFrame = m_pScene->FrameCount();
	dlg.m_MaxLevel = m_pScene->LevelCount()-1;
	m_pScene->PublishSizes(dlg.m_origwidth, dlg.m_origheight);
	dlg.m_width = dlg.m_origwidth;// = m_pScene->ComW();
	dlg.m_height = dlg.m_origheight;// = m_pScene->ComH();
	
	dlg.m_bGray = !m_pScene->ColorMode();
	UINT depth = dlg.m_bGray ? 1 : 3;
	dlg.m_color = dlg.m_bGray ? 1 : 2;
	dlg.m_bAspect = TRUE;
	dlg.m_bKeep = TRUE;
	int res = dlg.DoModal();
	if (res != IDOK)
		return;
	int code;
	if (dlg.m_nKind == 2)
		code = 10;
	else if (dlg.m_nKind == 3)
		code = 20;
	else if (dlg.m_nKind == 1)
		code = 11;
	else
		code = 2;
	/*
	if ((dlg.m_nKind == 3) && dlg.m_bUseSound)
		{
		UINT rate = pSound->Rate();
		if ((rate != 11025) && (rate != 22050) && (rate != 44100))
			{
			if (AfxMessageBox(
				"Sound sample rate must be 11025, 22050, or 44100 for FLV",
					MB_OKCANCEL) != MB_OK)
				return;
			dlg.m_bUseSound = 0;
			}
		}
	*/
#if MAC
	if ([[NSUserDefaults standardUserDefaults] boolForKey:@"ConvertToFlash"]) {
		if (!((CSketchApp*) AfxGetApp())->PromptFileName(dlg.m_media, 20))
			return;
	}
	else
#endif
	if (!((CSketchApp*) AfxGetApp())->PromptFileName(dlg.m_media, code))
		return;
	m_pScene->Broadcasting(1); // allocate bigger buffers (maybe)
	if (dlg.m_nKind == 2)
		{
		DoGif(dlg.m_media, dlg.m_width, dlg.m_height, 256 * depth + dlg.m_color,
			dlg.m_origwidth, dlg.m_origheight,
			dlg.m_StartFrame-1,dlg.m_StartLevel,
				dlg.m_EndFrame-1,dlg.m_EndLevel,dlg.m_rate,dlg.m_bLoop);
		}
	else if (dlg.m_nKind == 3)
		{
		DoFlv(dlg.m_media, dlg.m_width, dlg.m_height, BPP,
			dlg.m_origwidth, dlg.m_origheight,
			dlg.m_StartFrame-1,dlg.m_StartLevel,
				dlg.m_EndFrame-1,dlg.m_EndLevel,
				dlg.m_rate,dlg.m_nTelecine,
				dlg.m_bUseSound ? pSound : 0, dlg.m_encoder);
		}
	else if (dlg.m_nKind == 1)
		{
		BeginBusy("Writing MOV");
		CExportMovie * pMovie= new CExportMovie;
		res = pMovie->Init( (LPCSTR)dlg.m_media,dlg.m_origwidth,dlg.m_origheight,
						dlg.m_width,dlg.m_height,depth,
						dlg.m_StartFrame-1,dlg.m_StartLevel,
						dlg.m_EndFrame-1,dlg.m_EndLevel,
						dlg.m_rate,dlg.m_nTelecine,
						dlg.m_bUseSound ? pSound : 0);
		if (res)
			{
			FormattedMsg(IDS_ERR_QT_INIT, res);
			}
		else
			{
			pMovie->m_pScene = m_pScene;
			CProg2Dlg progger;
			progger.ProgSetup( ExportMovieCallback, pMovie,0,
				dlg.m_StartFrame,dlg.m_EndFrame + 1 - dlg.m_StartFrame);
			progger.DoModal();
			pMovie->Finish();
			#if MAC
				FBExportConvertToFlash ((LPCSTR)dlg.m_media);
			#endif
			}
		delete pMovie;
		EndBusy();
		}
	else
		{
		DoAvi(dlg.m_media, dlg.m_width, dlg.m_height, BPP,
			dlg.m_origwidth, dlg.m_origheight,
			dlg.m_StartFrame-1,dlg.m_StartLevel,
				dlg.m_EndFrame-1,dlg.m_EndLevel,
				dlg.m_rate,dlg.m_nTelecine,
				dlg.m_bUseSound ? pSound : 0, &dlg.m_opts);
		}
	m_pScene->Broadcasting(0);
#endif
}


void CSketchDoc::DoAvi(CString name, UINT w, UINT h, UINT BPP,
						UINT origw, UINT origh,
						UINT f1, UINT l1, UINT f2, UINT l2,
						UINT rate, int Telecine, CMySound * pSound, void *pOpts)
{
#ifndef FLIPBOOK_MAC
	UINT i;
	HRESULT hr;
	WORD wVer;
	UINT bits = 8 * BPP;
	CGScaler scale;

	/* first let's make sure we are running on 1.1 */
	wVer = HIWORD(VideoForWindowsVersion());
	if (wVer < 0x010a){
		 /* oops, we are too old, blow out of here */
		MessageBeep(MB_ICONHAND);
		MyError(IDS_OLD_VFW, MB_OK|MB_ICONSTOP);
		return;// FALSE;
	}
	BeginBusy("Writing AVI");
again:
	AVISTREAMINFO strhdr;
	PAVIFILE pfile = NULL;
	PAVISTREAM ps = NULL, psCompressed = NULL;
	UINT wLineLen = 4 * (( bits * w + 31) / 32);
	DWORD wColSize = sizeof(RGBQUAD)*((bits <= 8) ? 1<<bits : 0);
	DWORD dwSize = (DWORD)(UINT)wLineLen*(DWORD)(UINT)h;

	BYTE * hpBuf1 = new BYTE[dwSize];
	BYTE * hpBuf2;
	BYTE * hpBuf;
	BYTE * hpTBuf = 0;
	if ((origw != w) || (origh != h))
		{
		UINT owLineLen = 4 * (( BPP * origw+ 3) / 4);
		hpBuf2 = new BYTE[owLineLen * origh];
		hpBuf = hpBuf2;
		}
	else
		{
		hpBuf = hpBuf1;
		hpBuf2 = 0;
		}
	int toffset, tfactor;
	if (Telecine)
		{
		if (Telecine > 1)
			toffset = 1;
		else
			toffset = 0;
		tfactor = 4;
		hpTBuf = new BYTE[dwSize];
DPF("t:%d,off:%d,fac:%d",Telecine,toffset,tfactor);
		}
	else
		tfactor = 1;
	LPBITMAPINFOHEADER lpbi = (LPBITMAPINFOHEADER)new BYTE[40+1024];
	lpbi->biSize = sizeof(BITMAPINFOHEADER) ;
	lpbi->biWidth = w;
	lpbi->biHeight = h;
	lpbi->biPlanes = 1 ;
	lpbi->biBitCount = (WORD) bits ;
	lpbi->biCompression = BI_RGB ;
	lpbi->biSizeImage = dwSize;
	lpbi->biXPelsPerMeter = 0 ;
	lpbi->biYPelsPerMeter = 0 ;
	lpbi->biClrUsed = (bits <= 8) ? 1<<bits : 0;
	lpbi->biClrImportant = 0 ;
	for (i = 0; i < 1024; i++)
		((BYTE *)lpbi)[i+40] = i / 4;
	Progress(1,1 + f2 - f1);

	CloseHandle(CreateFile((LPCSTR)name,GENERIC_WRITE | DELETE,
								0,0,TRUNCATE_EXISTING,FILE_ATTRIBUTE_NORMAL,0));
	AVIFileInit();
	hr = AVIFileOpen(&pfile,		    // returned file pointer
		       (LPCSTR)name,		            // file name
		       OF_CREATE,	    // mode to open file with
		       NULL);			    // use handler determined
						    // from file extension....
DPF("aviopen:%lx",hr);
	if (hr != AVIERR_OK)
		goto error;
	// Fill in the header for the video stream....

	memset(&strhdr, 0, sizeof(strhdr));
	strhdr.fccType                = streamtypeVIDEO;// stream type
	strhdr.fccHandler             = 0;
	strhdr.dwScale                = 1;
	strhdr.dwRate                 = Telecine ? (5 * rate) / 4 : rate;
	strhdr.dwSuggestedBufferSize  = lpbi->biSizeImage;
	SetRect(&strhdr.rcFrame, 0, 0,		    // rectangle for stream
	    (int) lpbi->biWidth,
	    (int) lpbi->biHeight);

	// And create the stream;
	hr = AVIFileCreateStream(pfile,		    // file pointer
			         &ps,		    // returned stream pointer
			         &strhdr);	    // stream header
DPF("create stream:%lx",hr);
	if (hr != AVIERR_OK) {
		goto error;
	}
	hr = AVIMakeCompressedStream(&psCompressed, ps, (AVICOMPRESSOPTIONS *)pOpts, NULL);
DPF("create compressed:%lx",hr);
	if (hr != AVIERR_OK) {
		goto error;
	}

	hr = AVIStreamSetFormat(psCompressed, 0,
			       lpbi,	    // stream format
			       lpbi->biSize +   // format size
			       lpbi->biClrUsed * sizeof(RGBQUAD));
DPF("setstreamformat:%lx",hr);
	if (hr != AVIERR_OK) {
if (hr == AVIERR_BADFORMAT)
	{
DPF("bad format:%d",bits);
	if (bits == 8)
		{
		if (ps)
			AVIStreamClose(ps);
		if (psCompressed)
			AVIStreamClose(psCompressed);
		if (pfile)
			AVIFileClose(pfile);
		AVIFileExit();
		delete lpbi;
		delete hpBuf1;
		delete hpBuf2;
		bits = 24;
		goto again;
		}
	}

	goto error;
	}
	UINT Frame;
	UINT OFrame;
	if (hpBuf2)
		{
		if (scale.Init(origw,origh,8*BPP,w,h))
			{
DPF("scale failure");
			goto error;
			}
		}
	for (OFrame = 0, Frame = f1; Frame <= f2;Frame++)
		{
		m_pScene->CompositeFrame32(hpBuf,
				l1, l2, Frame,FALSE);
		if (hpBuf2)
			{
	int q = scale.Custom(hpBuf1, hpBuf2, 4 * ((BPP*w+3)/4));
DPF("after custom:%d",q);
	int z = scale.Copy();
DPF("after scale:%d",z);
			}
		if ((BPP == 1) && (bits == 24))
			{
			int x,y,p1,p2,p3,p4;
			p1 = 4 * ((w+3)/4);
			p2 = 4 * ((3*w+3)/4);
			p3 = p1 * (h - 1);
			p4 = p2 * (h - 1);
			for (y = h - 1; y >= 0; y--)
				{
				for (x = w -1 ; x >= 0; x--)
					{
					hpBuf1[p4+3*x+2] = hpBuf1[p3+x];
					hpBuf1[p4+3*x+1] = hpBuf1[p3+x];
					hpBuf1[p4+3*x+0] = hpBuf1[p3+x];
					}
				p4 -= p2;
				p3 -= p1;
				}
			}
//		m_pScene->GetGray(hpBuf, Frame, dlg.m_StartLevel);
//		for (Level = dlg.m_StartLevel+1; Level <= dlg.m_EndLevel;Level++)
//			m_pScene->ApplyGray(hpBuf, 100, Frame,Level);
DPF("putting:%d",Frame);
		UINT pitch = wLineLen;
		UINT height = h;
	
		switch((Frame - f1) % tfactor)
		{
		case 1:
			Interlace(hpTBuf, hpBuf1, pitch,height, toffset);
		case 0:
			hr = AVIStreamWrite(psCompressed, OFrame++,
			1, hpBuf1, lpbi->biSizeImage, AVIIF_KEYFRAME, NULL, NULL);
			break;
		case 2:
			Interlace(hpTBuf, hpBuf1, pitch,height, toffset ^ 1);
			hr = AVIStreamWrite(psCompressed, OFrame++,
			1, hpTBuf, lpbi->biSizeImage, AVIIF_KEYFRAME, NULL, NULL);
			Interlace(hpTBuf, hpBuf1, pitch,height, toffset);
			break;
		case 3:
			Interlace(hpTBuf, hpBuf1, pitch,height, toffset ^ 1);
			hr = AVIStreamWrite(psCompressed, OFrame++,
			1, hpTBuf, lpbi->biSizeImage, AVIIF_KEYFRAME, NULL, NULL);
			hr = AVIStreamWrite(psCompressed, OFrame++,
			1, hpBuf1, lpbi->biSizeImage, AVIIF_KEYFRAME, NULL, NULL);
			break;
		}
		if (hr != AVIERR_OK)
			break;
		Progress(2, Frame + 1 - f1);
	}
	DPF("last frame:%d",Frame);
	
	error:
/*
#define AVIERR_UNSUPPORTED      MAKE_AVIERR(101)
#define AVIERR_BADFORMAT        MAKE_AVIERR(102)
#define AVIERR_MEMORY           MAKE_AVIERR(103)
#define AVIERR_INTERNAL         MAKE_AVIERR(104)
#define AVIERR_BADFLAGS         MAKE_AVIERR(105)
#define AVIERR_BADPARAM         MAKE_AVIERR(106)
#define AVIERR_BADSIZE          MAKE_AVIERR(107)
#define AVIERR_BADHANDLE        MAKE_AVIERR(108)
#define AVIERR_FILEREAD         MAKE_AVIERR(109)
#define AVIERR_FILEWRITE        MAKE_AVIERR(110)
#define AVIERR_FILEOPEN         MAKE_AVIERR(111)
#define AVIERR_COMPRESSOR       MAKE_AVIERR(112)
#define AVIERR_NOCOMPRESSOR     MAKE_AVIERR(113)
#define AVIERR_READONLY		MAKE_AVIERR(114)
#define AVIERR_NODATA		MAKE_AVIERR(115)
#define AVIERR_BUFFERTOOSMALL	MAKE_AVIERR(116)
#define AVIERR_CANTCOMPRESS	MAKE_AVIERR(117)
#define AVIERR_USERABORT        MAKE_AVIERR(198)
#define AVIERR_ERROR            MAKE_AVIERR(199)
*/

DPF("hr:%lx",hr);
	Progress();
	EndBusy();
	//
	// Now close the file
	//
	if (ps)
		AVIStreamClose(ps);

	if (psCompressed)
		AVIStreamClose(psCompressed);
	
	if (hr == AVIERR_OK && pSound)
		{
//    AVISTREAMINFO   strhdr;
		UINT nFormat;
		UINT nAudio = pSound->WaveLength(1 + f2 - f1);
		BYTE * pAudio = pSound->WaveData(f1);
   // Fill in the header for the audio stream....
		memset(&strhdr, 0, sizeof(strhdr));
		strhdr.fccType                = streamtypeAUDIO;  // stream type
		strhdr.dwScale                = 1;                
		strhdr.dwRate                 = pSound->Rate();
		strhdr.dwSuggestedBufferSize  = pSound->Bytes();                    

   // And create the stream;
		hr = AVIFileCreateStream(pfile,             // file pointer
                              &ps,             // returned stream pointer
                              &strhdr);            // stream header
		if (hr != AVIERR_OK) 
			goto error2;
		WAVEFORMATEX pcm;
		nFormat = pSound->GetFormat(&pcm);
		hr = AVIStreamSetFormat(ps, 0, 
                              &pcm,       // stream format
                              nFormat);      // format size
		if (hr == AVIERR_OK) 

		hr = AVIStreamWrite(ps,                // stream pointer
                       0,                     // starting sample
                       nAudio / pSound->Bytes(),                     // number to write
                       pAudio,   // pointer to data
                       nAudio,     // size of the data
                       AVIIF_KEYFRAME,        // flags....
                       NULL, NULL);
error2:
		AVIStreamClose(ps);            // close the audio stream
		}

	if (pfile)
		AVIFileClose(pfile);

	AVIFileExit();
	delete lpbi;
	delete hpBuf1;
	delete hpTBuf;
	if (hpBuf2)
		delete hpBuf2;

	if (hr != NOERROR) {
//		LoadString(hInstance, IDS_APPERR, szTitle, BUFSIZE );
//		LoadString(hInstance, IDS_WRITEERR, szMessage, BUFSIZE );
		
//		MessageBox(NULL, szMessage, szTitle, MB_OK);
		MyError(IDS_BAD_WRITE, MB_OK);
	}
#endif
}

void CSketchDoc::DoGif(CString name, UINT w, UINT h, UINT c, 
				UINT origw, UINT origh,
			UINT startframe,UINT startlevel,
					UINT endframe, UINT endlevel, UINT rate, BOOL bLoop)
{
#ifndef FLIPBOOK_MAC
	DPF("dogif");
	CFile file;
	UINT mode = CFile::modeCreate | CFile::modeReadWrite;
	if (!file.Open((LPCSTR)name, mode))
		return;
	CGifCallBack * pGif = new CGifCallBack;
	pGif->m_option = AfxGetApp()->GetProfileInt("Options","Gif", 0);
	pGif->Init( & file,origw,origh,w,h,c,
				startframe,startlevel, endframe,endlevel,rate,bLoop);
	pGif->m_pScene = m_pScene;
	CProgDlg progress;
	progress.m_kind = 2;
	progress.m_base = startframe;
	progress.m_count = endframe + 1 - startframe;
	progress.m_pDoc = this;
	progress.m_pData = (BYTE *)pGif;
	progress.DoModal();
	pGif->Finish();
	file.Close();
	delete pGif;
#endif
}


#ifndef FLIPBOOK_MAC
BOOL GoodWave( PAVISTREAM 	paudio)
{
	BOOL bResult = FALSE;
    PCMWAVEFORMAT wf;
    long l = sizeof(wf);
    AVIStreamReadFormat(paudio, 0, &wf, &l);
DPF("l:%d",l);
    if (wf.wf.wFormatTag == WAVE_FORMAT_PCM)
		bResult = TRUE;
	return bResult;
}

BOOL PutWave( LPCSTR name, PAVISTREAM 	paudio)
{
//	strcpy(name,"C:\\test.wav");
DPF("put wave:%s",name);
	BYTE * bufs[1000];
	DWORD lngs[1000];
	BYTE buff[100];
	int count = 0;

    WAVEFORMATEX wf;
    long l = sizeof(wf);
    AVIStreamReadFormat(paudio, 0, &wf, &l);
    if (!l)
		return 0;
	DWORD offset = 0;
	DWORD dwTotal = 0;
	long nBytes;
	for (;count < 1000;)
		{
		long nSamples;
    	AVIStreamRead(paudio, offset,AVISTREAMREAD_CONVENIENT,
						0, 0, &nBytes, &nSamples);
		if (!nBytes)
			break;
		DPF("count:%d,nBytes:%d,nSamples:%d",count,nBytes,nSamples);
		bufs[count] = new BYTE[nBytes + 4]; // pad
		lngs[count] = nBytes;
		dwTotal += nBytes;
    	AVIStreamRead(paudio, offset,AVISTREAMREAD_CONVENIENT, bufs[count],
					nBytes, 0, 0);
		offset += nSamples;
		count++;
		}
	nBytes = dwTotal;
//	dwTotal += 4 + 4 + 4 + 4 + 4 + sizeof(WAVEFORMATEX) + 4 + 10;

	DWORD * pData = (DWORD *)buff;
	*pData++ = mmioFOURCC('R', 'I', 'F', 'F');
	*pData++ = 0;//dwTotal;
	*pData++ = mmioFOURCC('W', 'A', 'V', 'E');
	*pData++ = mmioFOURCC('f', 'm', 't', ' ');
	DWORD dwSize = sizeof(WAVEFORMATEX);
	*pData++ = dwSize;
	* ((WAVEFORMATEX *)pData) = wf;
	pData = (DWORD *)((BYTE *)pData + ((dwSize+1)&~1));
	*pData++ = mmioFOURCC('d', 'a', 't', 'a');
	dwSize = nBytes;
	*pData++ = dwSize;
	dwTotal = (BYTE *)pData - buff;
	pData = (DWORD *)buff;
	pData[1] = dwTotal + nBytes;
 //   AVIStreamRead(paudio, 0,AVISTREAMREAD_CONVENIENT, pData, nBytes, 0, 0);
//	pData = (DWORD *)((BYTE *)pData + ((dwSize+1)&~1));
	CFile file;
	DWORD mode = CFile::modeCreate | CFile::modeWrite;
	if (!file.Open(name, mode))
		{
		for (int i = 0; i < count;i++)
			delete bufs[i];
		return 0;
		}
	file.Write(buff,dwTotal);
	for (int i = 0; i < count;i++)
		{
		file.Write(bufs[i],lngs[i]);
		}
	file.Close();

	for (int j = 0; j < count;j++)
		delete bufs[j];
	return 1;
}

#endif

//
//	code == 9999 for start
// 	10000 for finish
//  else index
//	returns true if problem
//
UINT CSketchDoc::ProgressCallBack(UINT kind, UINT code, BYTE * pData)
{
DPF("progess callbcak, kind:%d,code:%d",kind,code);
	if (kind == 0)
		{
//	avi import
		}
	else if (kind == 1)
		{
//	avi export
		}
	else if (kind == 2)
		{
		return ((CGifCallBack *)pData)->Perform(code);
	// giff
		}
	else
		return 1;
	return 0;
}

CGifCallBack::CGifCallBack()
{
	BYTE * hpBuf1 = 0;
	BYTE * hpBuf2 = 0;
	m_pScale = 0;
	m_pGif = 0;
}

CGifCallBack::~CGifCallBack()
{
	Finish();
}

BOOL CGifCallBack::Init(CFile * file, UINT origw,UINT origh,
			UINT w,UINT h,UINT c,
			UINT startframe,UINT startlevel, 
				UINT endframe,UINT endlevel,UINT rate,UINT bLoop)
{
	m_startframe = startframe;
	m_endframe = endframe;
	m_startlevel = startlevel;
	m_endlevel = endlevel;
	m_w = w;
	m_h = h;
	m_origw = origw;
	m_origh = origh;
	m_pGif = new CMyGif();
	m_d = c >> 8;
	m_z = c & 7;
	m_pitch = 4 * ((m_w * m_d + 3) / 4);
	DWORD dwSize = m_pitch * m_h;
	if (dwSize < 1000)
		dwSize = 1000;	// for palette
	m_hpBuf1 = new BYTE[dwSize];
	if ((m_origw != m_w) || (m_origh != m_h))
		{
		UINT owLineLen = 4 * (( m_d * m_origw + 3) / 4);
		m_hpBuf2 = new BYTE[owLineLen * m_origh];
		}
	else
		m_hpBuf2 = 0;
	m_pGif->SetOption(m_option);
	if (!m_z)
		{
		int i;
		for (i = 0; i < 48; i++)
			m_hpBuf1[i] = 17 * (i / 3);
		m_pGif->SetPalette(m_hpBuf1);
		}
	else if (m_z == 1)
		{
		int i;
		for (i = 0; i < 768; i++)
			m_hpBuf1[i] = i / 3;
		m_pGif->SetPalette(m_hpBuf1);
		}
	else
		m_pGif->SetPalette(0);
	if (m_hpBuf2)
		{
		m_pScale = new CGScaler;
		if (m_pScale->Init(origw,origh,8*m_d,m_w,m_h))
			{
DPF("scale failure");
			return 1;
			}
		}
//	if (m_startlevel)
//		c += 4096; // transparent
	m_pGif->Encode(file,m_w,m_h,
					m_endframe + 1 - m_startframe,c,rate,bLoop);
	
	return 0;
}

BOOL CGifCallBack::Perform(UINT index)
{
	DPF("gifback,perform,idx:%d",index);
	m_pScene->CompositeFrame32(m_hpBuf2 ? m_hpBuf2 : m_hpBuf1,
				m_startlevel, m_endlevel, m_startframe + index,FALSE);
	if (m_hpBuf2)
		{
		int q = m_pScale->Custom(m_hpBuf1, m_hpBuf2, 4 * ((m_d*m_w+3)/4));
DPF("after custom:%d",q);
		int z = m_pScale->Copy();
DPF("after scale:%d",z);
		}
	m_pGif->AddFrame(m_hpBuf1,m_pitch);
	return 0;
}

BOOL CGifCallBack::Finish()
{
DPF("callback finish");
	if (m_pGif)
		m_pGif->FinishIt();
	delete m_pGif;
	m_pGif = 0;
	delete m_pScale;
	m_pScale = 0;
	delete [] m_hpBuf1;
	delete [] m_hpBuf2;
	m_hpBuf1 = 0;
	m_hpBuf2 = 0;
	return 0;
}


/*
	CExportMovie class is used to simply export movie progress callback
	it also isolates export logic from dialog code
*/

#ifdef USEQT
CExportMovie::CExportMovie()
{
	m_hpBuf1 = 0;	// composite buffer
	m_hpBuf2 = 0;	// scaling buffer
	m_hpTBuf = 0;	// telecine buffer
	m_pQT = 0;		// handle from FBQT package
	m_pScale = 0;	// pointer to scaler if used
	m_pSound = NULL; // pointer to sound object
}

CExportMovie::~CExportMovie()
{
	Finish();
}


BOOL CExportMovie::Init(LPCSTR name, UINT origw,UINT origh,
			UINT w,UINT h,UINT d,
			UINT startframe,UINT startlevel, 
				UINT endframe,UINT endlevel,UINT rate,UINT nTelecine, CMySound* pSound)
{
	m_startframe = startframe;
	m_endframe = endframe;
	m_startlevel = startlevel;
	m_endlevel = endlevel;
	m_pSound = pSound;
	m_w = w;
	m_h = h;
	m_d = d;
	m_pitch = 4 * ((m_w * m_d + 3) / 4);
	m_size = m_pitch * m_h;
	m_hpBuf1 = new BYTE[m_size];
	if ((origw != m_w) || (origh != m_h))
		{
		UINT owLineLen = 4 * (( m_d * origw + 3) / 4);
		m_hpBuf2 = new BYTE[owLineLen * origh];
		m_pScale = new CGScaler;
		if (m_pScale->Init(origw,origh,8*m_d,m_w,m_h))
			{
DPF("scale failure");
			return 1;
			}
		}
	else
		m_hpBuf2 = 0;
	if (nTelecine)
		{
		if (nTelecine > 1)
			m_toffset = 1;
		else
			m_toffset = 0;
		m_tfactor = 4;
		}
	else
		m_tfactor = 1;
	m_pQT = (BYTE *)FBExportMovieSetup (name,
			m_w,m_h, 8 * m_d, rate,m_endframe + 1 - m_startframe);
	if (!m_pQT)
		return 3;
	return 0;
}


BOOL CExportMovie::Perform(UINT index)
{
	DPF("exp mov,perform,idx:%d",index);
	m_pScene->CompositeFrame32(m_hpBuf2 ? m_hpBuf2 : m_hpBuf1,
				m_startlevel, m_endlevel, m_startframe + index,FALSE);
	if (m_hpBuf2)
		{
		int q = m_pScale->Custom(m_hpBuf1, m_hpBuf2, m_pitch);
DPF("after custom:%d",q);
		int z = m_pScale->Copy();
DPF("after scale:%d",z);
		}
DPF("putting:%d",index);
	int result;
	switch((index - m_startframe) % m_tfactor)
		{
		case 1:
			Interlace(m_hpTBuf, m_hpBuf1, m_pitch,m_h, m_toffset);
// dropping through
		case 0:
			result = FBExportMoviePut (m_pQT, m_hpBuf1, m_size);
			break;
		case 2:
			Interlace(m_hpTBuf, m_hpBuf1, m_pitch,m_h, m_toffset ^ 1);
			result = FBExportMoviePut (m_pQT, m_hpTBuf, m_size);
			Interlace(m_hpTBuf, m_hpBuf1, m_pitch,m_h, m_toffset);
			break;
		case 3:
			Interlace(m_hpTBuf, m_hpBuf1, m_pitch,m_h, m_toffset ^ 1);
			if (!(result = FBExportMoviePut (m_pQT, m_hpTBuf, m_size)))
				result = FBExportMoviePut (m_pQT, m_hpBuf1, m_size);
			break;
		}
	return result;
}

BOOL CExportMovie::Finish()
{
DPF("exp mov finish");
	if (m_pQT) {
		FBExportMovieSound(m_pQT, m_pSound, m_startframe, m_endframe);
		FBExportMovieClose(m_pQT);
	}
	m_pQT = 0;
	delete m_pScale;
	delete [] m_hpBuf1;
	delete [] m_hpBuf2;
	delete [] m_hpTBuf;
	m_hpBuf1 = 0;
	m_hpBuf2 = 0;
	m_hpTBuf = 0;
	m_pScale = 0;
	return 0;
}


/*
	CImporttMovie class is used to simplify import movie progress callback
	it also isolates import logic from dialog code
*/


CImportMovie::CImportMovie()
{
	m_pMov = 0;		// handle from FBQT package
	m_frameRateAdjustment = 1.0;
}


BOOL CImportMovie::Init(void * pMov, UINT frame,UINT level, UINT firstmov)
{
	m_frame = frame;
	m_level = level;
	m_firstmov = firstmov;
	m_pMov = pMov;
	return 0;
}

int CImportMovie::ProcessDibData(void * pData, UINT ssize)
{
	DPF("imp mov,perform,frame:%d",m_frame);
	LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)pData;
	int result;
	if (lpBI)
		{
		if (m_pDoc->CreateCell(m_frame++,m_level,lpBI,0,1,0))
			result = 95;
		else
			result = 0;
		}
	else
		result = 97;
	return result;
}

UINT ImportMovieCB(FBDataKind Code, void * pData, int size, void * pClass)
{
	if (Code != kBMPDataType)
		return 0;
	return ((CImportMovie*)pClass)->ProcessDibData(pData, size);
}

/*
	Import Movie Callback is used by CProg2Dlg to import each frame
*/
unsigned int ImportMovieCallback(unsigned int index,
						void *UserObject, void *UserData )
{
	return ((CImportMovie *)UserObject)->Perform(index);
}

BOOL CImportMovie::Perform(UINT index)
{
	int result = FBImportMovieFrame (m_pMov,
				ImportMovieCB, m_firstmov + index, (void *) this);
	return kNoError;
}

void CSketchDoc::ProcessImportMovie(const char* inFilePath, bool inDeleteOnFinish, float inFrameRateAdjustment)
{	
	bool bHasAudio;
	int nFrames;
	void * pMov = FBImportMovieOpen (inFilePath, &nFrames, &bHasAudio);//, inFrameRateAdjustment);
	if (!pMov || !nFrames)
		return;
	CAviInDlg dlg;
	CMainFrame * pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
	dlg.m_nTrack = 0;
	dlg.m_nMaxSoundCol = 0;
	if (bHasAudio)
		dlg.m_nMaxSoundCol = pFrame->MaxSoundCols();
	if (!pFrame->GetSelection(dlg.m_frame,dlg.m_level))
	{
		CSketchView* pSketchView = GetDocView();
		dlg.m_frame = pSketchView->CurrentFrame();
		dlg.m_level = pSketchView->CurrentLevel();
	}
	int result;
	dlg.m_frame++;
	dlg.m_maxframe = m_pScene->FrameCount();
	dlg.m_maxlevel = m_pScene->LevelCount();
	//	dlg.m_width = 400;//pfi.dwWidth;
	//	dlg.m_height = 300;//pfi.dwHeight;
	dlg.m_start = 0;
	dlg.m_maxavi = dlg.m_stop = nFrames-1;	
	for (;;)
	{
		result = dlg.DoModal();
		if (result != IDOK)
			break;
		if ((dlg.m_stop + 1 - dlg.m_start + dlg.m_frame - 1)
			> m_pScene->FrameCount())
		{
			result = MyError(IDS_BIG_AVI, MB_YESNOCANCEL);
			if (result == IDYES)
			{
				result = IDOK;
				CMainFrame * pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
				pFrame->ChangeFrames(m_pScene->FrameCount(),
									 dlg.m_stop+1-dlg.m_start+dlg.m_frame);
				break;
			}
			if (result == IDNO)
			{
				dlg.m_stop = 
				dlg.m_start + m_pScene->FrameCount() - dlg.m_frame;
				result = IDOK;
				break;
			}
		}
		else
			break;
	}
	if (result == IDOK)
	{
		if (dlg.m_nTrack)
		{
			//			fbqt make sound file(fileName)
			pFrame->SetSound(TRUE, dlg.m_nTrack-1, 0.0, dlg.m_frame,NEGONE,(char *)inFilePath); // snip was 3
		}
		BeginBusy("Writing MOV");
		CImportMovie * pMovie= new CImportMovie;
		pMovie->Init(pMov,dlg.m_frame-1,dlg.m_level,dlg.m_start);
		pMovie->m_pDoc = this;
		pMovie->m_frameRateAdjustment = inFrameRateAdjustment;
		CProg2Dlg progger;
		progger.ProgSetup( ImportMovieCallback, pMovie,0,dlg.m_frame,
						  dlg.m_stop + 1 - dlg.m_start);
		progger.DoModal();
		delete pMovie;
		EndBusy();
		SetModifiedFlag(TRUE);
	}
	FBImportMovieClose (pMov);

	if (inDeleteOnFinish) {
		CFile::Remove (inFilePath);
	}
}


void CSketchDoc::OnImportMovie()
{
	DPF("on import mov");
    OPENFILENAME ofn;
	char        fileName[300] = "";
	char	    gachFilter[512] = "Mov Files\0*.mov\0\0";
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = 0;//hwnd;
	ofn.hInstance = NULL;
	ofn.lpstrTitle = "Import AVI";
	ofn.lpstrFilter = gachFilter;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = sizeof(fileName);
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST |OFN_HIDEREADONLY;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = NULL;
	ofn.lCustData = 0;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;

	    //
	    // If we've got a filename, go open it
	    //
	if (!GetOpenFileNamePreview(&ofn))
		return;
	bool bHasAudio;
	int nFrames;
	void * pMov = FBImportMovieOpen (fileName, &nFrames, &bHasAudio);
	if (!pMov || !nFrames)
		return;
	CAviInDlg dlg;
	CMainFrame * pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
	dlg.m_nTrack = 0;
	dlg.m_nMaxSoundCol = 0;
	if (bHasAudio)
		dlg.m_nMaxSoundCol = pFrame->MaxSoundCols();
	if (!pFrame->GetSelection(dlg.m_frame,dlg.m_level))
		{
		CSketchView* pSketchView = GetDocView();
		dlg.m_frame = pSketchView->CurrentFrame();
		dlg.m_level = pSketchView->CurrentLevel();
		}
	int result;
	dlg.m_frame++;
	dlg.m_maxframe = m_pScene->FrameCount();
	dlg.m_maxlevel = m_pScene->LevelCount();
//	dlg.m_width = 400;//pfi.dwWidth;
//	dlg.m_height = 300;//pfi.dwHeight;
	dlg.m_start = 0;
	dlg.m_maxavi = dlg.m_stop = nFrames;	
	for (;;)
		{
		result = dlg.DoModal();
		if (result != IDOK)
			break;
		UINT new_frames = 0;
		if ((dlg.m_stop - dlg.m_start + dlg.m_frame)
							> MaxFrames())
			{
			result = MyError(IDS_TOO_BIG_AVI, MB_YESNOCANCEL);
			if (result == IDYES)
				new_frames = MaxFrames();
			}
		else if ((dlg.m_stop - dlg.m_start + dlg.m_frame)
							> m_pScene->FrameCount())
			{
			result = MyError(IDS_BIG_AVI, MB_YESNOCANCEL);
			if (result == IDYES)
				new_frames = dlg.m_stop - dlg.m_start + dlg.m_frame;
			}
		else
			break;
		if (new_frames)
			{
			result = IDOK;
			CMainFrame * pFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
			pFrame->ChangeFrames(new_frames, new_frames);
			dlg.m_stop = dlg.m_start + new_frames - dlg.m_frame;
			break;
			}
		if (result == IDNO)
			{
			dlg.m_stop = 
					dlg.m_start + m_pScene->FrameCount() - dlg.m_frame;
			result = IDOK;
			break;
			}
		}
	if (result == IDOK)
		{
		if (dlg.m_nTrack)
			{
//			fbqt make sound file(fileName)
//			pFrame->SetSound(TRUE, 0.0, dlg.m_frame,3,name);
			}
		BeginBusy("Writing MOV");
		CImportMovie * pMovie= new CImportMovie;
		pMovie->Init(pMov,dlg.m_frame-1,dlg.m_level,dlg.m_start-1);
		pMovie->m_pDoc = this;
		CProg2Dlg progger;
		progger.ProgSetup( ImportMovieCallback, pMovie,0,dlg.m_frame,
				dlg.m_stop + 1 - dlg.m_start);
		progger.DoModal();
		delete pMovie;
		EndBusy();
		SetModifiedFlag(TRUE);
		}
	FBImportMovieClose (pMov);
}
#endif

