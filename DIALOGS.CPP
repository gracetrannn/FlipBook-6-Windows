#include "stdafx.h"
#include "sketch.h"
#include "mydialog.h"
#include "dialogs.h"
#include "camera.h"
#include "clayers.h"
#include "clevtbl.h"
#include "cnewpals.h"
#include "myview.h"
#include "mydoc.h"
#include "fbqt.h"
#include "sceneopt.h"
#include "palette.h"
#include "mysound.h"
#include "camera.h"
#include "myio.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void FileName(CString& name, LPCSTR path)
{
	LPCSTR p = (LPCSTR)path;
	int i, n, e;
	e = 0;
	n = 0;
	for (i = 0; p[i];i++)
		{
		if (p[i] == '.')
			e = i;
		else if ((p[i] == WINDOWS_SEP_CHAR) || (p[i] == MAC_SEP_CHAR))
			n = i+1;
		}
	if (!e) e = i;
	char temp[100];
	for (i = 0; (i+n) < e;i++)
		temp[i] = p[n+i];
	temp[i] = 0;
	name = temp;
}

BOOL Exists(LPCSTR name)
{
	CFileStatus status;
	return CFile::GetStatus( name, status ); 
}


CMyDialog::CMyDialog(UINT Idd, CWnd* pParent /*=NULL*/)
	: CDialog(Idd, pParent)
{
	m_bSheet = FALSE;
}

BOOL CMyDialog::OnInitDialog()
{ 
	CMainFrame * pFrame = (CMainFrame *)AfxGetApp()->m_pMainWnd;
	if (pFrame)
		{
		if (m_bSheet = pFrame->EnableSheet(2))
			pFrame->EnableSheet(0);
		}
	CDialog::OnInitDialog();
#ifndef FLIPBOOK_MAC
	SendMessage(TTM_SETMAXTIPWIDTH,0,10);
#endif
//	EnableToolTips(TRUE);
	return FALSE;
}

void CMyDialog::EndModalLoop(int code)
{
	if (m_bSheet)
		((CMainFrame*) AfxGetApp()->m_pMainWnd)->EnableSheet(1);

	CDialog::EndModalLoop(code);
}

BEGIN_MESSAGE_MAP(CMyDialog, CDialog)
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTip)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CMyDialog::OnToolTip(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
#ifndef FLIPBOOK_MAC
    TOOLTIPTEXT *pTTT = (TOOLTIPTEXT*)pNMHDR;
    UINT nID = pNMHDR->idFrom;
    if(pTTT->uFlags & TTF_IDISHWND)
    {
        // idFrom is actually the HWND of the tool
        nID = ::GetDlgCtrlID((HWND)nID);
        if(nID)
        {
		CString buf;
		char msg[100];
		if (buf.LoadString(nID))
			{
			strcpy(msg, (LPCSTR)buf);
			LPSTR p = msg;
			if (p[0] == '#')
				{
				if (IsDlgButtonChecked(nID))
					p++;
				else
					{
					buf.LoadString(nID+1);
					strcpy(msg, (LPCSTR)buf);
					p++;
					}
				}
			CWnd * pWnd = CWnd::FromHandle(pNMHDR->hwndFrom);
			pWnd->SendMessage(TTM_SETMAXTIPWIDTH, 0, 300);
			pWnd->ModifyStyle(0, 0x40,0);//TTS_BALLOON,0);
//			::SendMessage(pNMHDR->hwndFrom, TTM_SETMAXTIPWIDTH, 0, 300);
			pTTT->lpszText = (LPSTR)p;
			pTTT->hinst = 0;
            return(TRUE);
			}
        }
    }
#endif
    return(FALSE);
}


class CSimpleDlg : public CMyDialog
{
public:
	CSimpleDlg(int idd, CWnd* pParent = NULL);
protected:
    virtual BOOL OnInitDialog();
	int m_id;
};

CSimpleDlg::CSimpleDlg(int idd, CWnd* pParent /*=NULL*/)
	: CMyDialog(idd, pParent)
{
	m_id = idd;
}

BOOL CSimpleDlg::OnInitDialog()
{
	CMyDialog::OnInitDialog();
	if (m_id == IDD_EXPIRE)
		{
		OEM_TEXT(this);
		OEM_DLG(this, IDC_EXP_BAD);
		char product[100];
		char temp[20];
		GetDlgItem(IDC_EXP_BAD2)->GetWindowText(product,99);
		OEM_BUF(product);
		temp[0] = '0' + (MYVER / 100);
		temp[1] = '.';
		temp[2] = '0' + (MYVER % 100) / 10;
		if ((MYVER % 100) % 10)
			temp[3] = '0' + (MYVER % 100) % 10;
		else
			temp[3] = ' ';
		temp[4] = 0;
		strcat(product, temp);
		GetDlgItem(IDC_EXP_BAD2)->SetWindowText(product);
		}
	else if (m_id == IDD_REGFEEDBACK)
		{
		OEM_DLG(this, IDD_REGFEEDBACK);
		}
	else if ((m_id == IDD_OTHER) || (m_id == IDD_OURS))
		{
		OEM_DLG(this, IDC_EXP_BAD);
		}
	return FALSE;
}

int MyDialog(int idd)
{
	CSimpleDlg dlg(idd);
	return dlg.DoModal();
}

#ifdef USEQT
CPublishDlg::CPublishDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CPublishDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPublishDlg)
	m_StartFrame = 1;
	m_EndFrame = 1;
	m_StartLevel = 0;
	m_EndLevel = 0;
	m_MaxFrame = 1;
	m_MaxLevel = 0;
	m_bEnableSound = 0;
	m_bUseSound = 0;
	m_nTelecine = 0;
	m_bAllowTelecine = 0;
	m_bAspect = TRUE;
	m_bKeep = TRUE;
	m_encoder = "";
	//}}AFX_DATA_INIT
}


void CPublishDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPublishDlg)
	DDX_Text(pDX, IDC_START_FRAME, m_StartFrame);
	DDV_MinMaxUInt(pDX, m_StartFrame, 1, m_MaxFrame);
	DDX_Text(pDX, IDC_END_FRAME, m_EndFrame);
	DDV_MinMaxUInt(pDX, m_EndFrame, m_StartFrame, m_MaxFrame);
	DDX_Text(pDX, IDC_START_LEVEL, m_StartLevel);
	DDV_MinMaxUInt(pDX, m_StartLevel, 0, m_MaxLevel);
	DDX_Text(pDX, IDC_END_LEVEL, m_EndLevel);
	DDV_MinMaxUInt(pDX, m_EndLevel, m_StartLevel, m_MaxLevel);
	DDX_Text(pDX, IDC_WIDTH, m_width);
	DDV_MinMaxUInt(pDX, m_width, 40, m_origwidth);
	DDX_Text(pDX, IDC_HEIGHT, m_height);
	DDV_MinMaxUInt(pDX, m_height, 30, m_origheight);
	DDX_Text(pDX, IDC_RATE, m_rate);
	DDV_MinMaxUInt(pDX, m_rate, 1, 100);
	DDX_Text(pDX, IDC_CODECNAME, m_codec);
	DDX_Check(pDX, IDC_USE_SOUND, m_bUseSound);
	DDX_Radio(pDX, IDC_TELECINE, m_nTelecine);
#ifndef FLIPBOOK_MAC
	DDX_Radio(pDX, IDC_PUB_AVI, m_nKind);
#else
	m_nKind = 1; // Mac only supports QuickTime export
#endif
	DDX_Check(pDX, IDC_REPEAT, m_bLoop);
	DDX_Radio(pDX, IDC_GIF_GRAY4, m_color);
	//}}AFX_DATA_MAP
}

BOOL CPublishDlg::OnInitDialog()
{ 
	int mask = 0;
	m_bKludge = 0;	// clear aspect race flag
	m_encoder = AfxGetApp()->GetProfileString("Settings","FLVEncExe", "");
	#ifndef FLIPBOOK_MAC
		if (FBQuickTime())
			mask = 7;
		else
			{
			if (m_nKind == 1)
				m_nKind = 0;
			mask = 5;
			}	
	 #else
		m_nKind = 1; // force movie
		mask = 2;
	#endif
	if (MyApp->CanDoFeature(CSketchApp::CD_EXP_FLV)) 
		mask |= 8;
	if (m_nKind == 3)
		{
		if (!m_encoder.GetLength() || !Exists( m_encoder) ||
			!MyApp->CanDoFeature(CSketchApp::CD_EXP_FLV)) 
		#ifndef FLIPBOOK_MAC
			m_nKind = 0;
		#else
			m_nKind = 1;
		#endif
		}
	GetDlgItem(IDC_PUB_AVI)->EnableWindow(mask & 1 ? 1 : 0);
	GetDlgItem(IDC_PUB_MOV)->EnableWindow(mask & 2 ? 1 : 0);
	GetDlgItem(IDC_PUB_GIF)->EnableWindow(mask & 4 ? 1 : 0);
#if !MAC
	GetDlgItem(IDC_PUB_FLV)->ShowWindow(mask & 8 ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_PUB_FLV2)->ShowWindow(mask & 8 ? SW_SHOW : SW_HIDE);
#endif
	GetDlgItem(IDC_PUB_FLV)->EnableWindow(mask & 8 ? 1 : 0);
	CMyDialog::OnInitDialog();
	CheckRadioButton(IDC_PUB_AVI,IDC_PUB_FLV,IDC_PUB_AVI+m_nKind);
	CheckDlgButton(IDC_ASPECT, m_bAspect);
	m_bWidth = 1;
	OnKindChange();
	CodecName();
	CenterWindow();
	return FALSE;
}


BEGIN_MESSAGE_MAP(CPublishDlg, CMyDialog)
	//{{AFX_MSG_MAP(CPublishDlg)
	ON_EN_CHANGE(IDC_WIDTH, OnWidthChange)
	ON_EN_CHANGE(IDC_HEIGHT, OnHeightChange)
	ON_EN_CHANGE(IDC_RATE, OnRateChange)
//	ON_COMMAND(IDC_PLAY, OnPlay)
	ON_COMMAND(IDC_SAVE_SETTINGS, OnSaveSettings)
//	ON_COMMAND(IDC_CHANGE_PLAYER, OnPlayerChange)
	ON_COMMAND(IDC_CHANGE_CODEC, OnCodecChange)
//	ON_COMMAND(IDC_CHANGE_NAME, OnMediaChange)
	ON_COMMAND(IDC_SELECT_ALL, OnSelectAll)
	ON_COMMAND(IDC_PUB_AVI, OnKindChange)
	ON_COMMAND(IDC_PUB_MOV, OnKindChange)
	ON_COMMAND(IDC_PUB_GIF, OnKindChange)
	ON_COMMAND(IDC_PUB_FLV, OnKindChange)
	ON_COMMAND(IDC_ASPECT, OnAspect)
	ON_COMMAND(IDC_PUB_FLV2, OnFLVChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CPublishDlg::OnSaveSettings()
{
	AfxGetApp()->WriteProfileString("Options","media name",m_media);
	AfxGetApp()->WriteProfileInt("Options", "media kind", m_nKind);
	CScene * pScene = m_pDoc->GetDocScene();
	if (m_bAllowTelecine)
		m_pDoc->Option(SC_TELECINE,1, m_nTelecine);
	m_pDoc->DoIt(DOIT_LONG,1,&m_opts.fccType,"fccType",m_bGray);
	m_pDoc->DoIt(DOIT_LONG,1,&m_opts.fccHandler,"fccHandler",m_bGray);
	m_pDoc->DoIt(DOIT_LONG,1,&m_opts.dwKeyFrameEvery,"KeyRate",m_bGray);
	m_pDoc->DoIt(DOIT_LONG,1,&m_opts.dwQuality,"Quality",m_bGray);
	m_pDoc->DoIt(DOIT_LONG,1,&m_opts.dwBytesPerSecond,"DataRate",m_bGray);
	m_pDoc->DoIt(DOIT_LONG,1,&m_opts.dwFlags,"Flags",m_bGray);
	char name[300]; // temp for args
	DWORD * pp = (DWORD *)&name;
	*pp++ = 1;
	*pp++ = m_opts.fccType;
	*pp++ = m_opts.fccHandler;
	*pp++ = m_opts.dwKeyFrameEvery;
	*pp++ = m_opts.dwQuality;
	*pp++ = m_opts.dwBytesPerSecond;
	*pp++ = m_opts.dwFlags;
	strcpy((LPSTR)pp, m_media);
	pScene->AVIInfo(&name, 299,1);	// save name
	
#if MAC
	[[NSUserDefaults standardUserDefaults] setBool:[[NSUserDefaults standardUserDefaults] boolForKey:@"ConvertToFlash"] forKey:@"RestoreConvertToFlash"];
#endif
}

void CPublishDlg::OnSelectAll() 
{
	UpdateData(1);
	BOOL bEnable;
	if (IsDlgButtonChecked(IDC_SELECT_ALL))
		{
		bEnable = 0;
		m_f1 = m_StartFrame;
		m_f2 = m_EndFrame;
		m_l1 = m_StartLevel;
		m_l2 = m_EndLevel;
		m_StartFrame = 1;
		m_EndFrame = m_MaxFrame;
		m_StartLevel = 0;
		m_EndLevel = m_MaxLevel;
		}
	else
		{
		bEnable = 1;
		m_StartFrame = m_f1;
		m_EndFrame = m_f2;
		m_StartLevel = m_l1;
		m_EndLevel = m_l2;
		}
	GetDlgItem(IDC_START_FRAME)->EnableWindow(bEnable);
	GetDlgItem(IDC_START_LEVEL)->EnableWindow(bEnable);
	GetDlgItem(IDC_END_FRAME)->EnableWindow(bEnable);
	GetDlgItem(IDC_END_LEVEL)->EnableWindow(bEnable);
	UpdateData(0);
}

void CPublishDlg::OnFLVChange()
{
	for (;;)
	{
	CFileDialog fileDlg( TRUE, "mencoder.exe", NULL,
						0, NULL, this);
	fileDlg.m_ofn.lpstrTitle = "Choose mencoder Executable";
	fileDlg.m_ofn.Flags |= 
		OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_HIDEREADONLY;
	if ( fileDlg.DoModal() == IDOK)
		{
#if MAC
		NSString* check_file = [NSString stringWithCString:fileDlg.GetPathName() encoding:NSUTF8StringEncoding];
		if ([[FBFlashVideo flashVideo] checkMencoderIsValid:check_file])
#else
		if (!fileDlg.GetFileName().CompareNoCase("mencoder.exe"))
#endif
			{
			m_encoder = fileDlg.GetPathName();
			AfxGetApp()->WriteProfileString("Settings","FLVEncExe",m_encoder);
			break;
			}
		}
	else
		break;
	}
}

void CPublishDlg::OnKindChange()
{
	int kind = m_nKind;
	UpdateData();
	if (m_nKind == 3)
		{
		if (!m_encoder.GetLength() || !Exists( m_encoder) ) 
			OnFLVChange();
		if (!m_encoder.GetLength() || !Exists( m_encoder) ) 
			{
			m_nKind = kind;
			CheckRadioButton(IDC_PUB_AVI,IDC_PUB_FLV,IDC_PUB_AVI+m_nKind);
			}
		}
	BOOL bGif = m_nKind == 2 ? 1 : 0;
	if (bGif)
		{
		GetDlgItem(IDC_USE_SOUND)->ShowWindow(SW_HIDE);
		}
	else
		{
		GetDlgItem(IDC_USE_SOUND)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_USE_SOUND)->EnableWindow(m_bEnableSound);
		}
#ifndef FLIPBOOK_MAC
	UINT temp = bGif ? SW_SHOW : SW_HIDE;
	GetDlgItem(IDC_GIF_OPTIONS)->ShowWindow(temp);
	GetDlgItem(IDC_GIF_GRAY4)->ShowWindow(temp);
	GetDlgItem(IDC_GIF_GRAY8)->ShowWindow(temp);
	GetDlgItem(IDC_GIF_COLOR)->ShowWindow(temp);
	GetDlgItem(IDC_REPEAT)->ShowWindow(temp);
#endif
	if (!bGif && m_bAllowTelecine && (m_rate == 24))
		{
		GetDlgItem(IDC_TELECINE_TEXT)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_TELECINE)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_TELECINEODD)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_TELECINEEVEN)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_TELECINE)->EnableWindow(m_bAllowTelecine);
		GetDlgItem(IDC_TELECINEODD)->EnableWindow(m_bAllowTelecine);
		GetDlgItem(IDC_TELECINEEVEN)->EnableWindow(m_bAllowTelecine);
		}
	else
		{
		GetDlgItem(IDC_TELECINE_TEXT)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_TELECINE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_TELECINEODD)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_TELECINEEVEN)->ShowWindow(SW_HIDE);
		}
#ifndef FLIPBOOK_MAC
	temp = !m_nKind ? SW_SHOW : SW_HIDE;
	GetDlgItem(IDC_CODECNAME)->ShowWindow(temp);
	GetDlgItem(IDC_CHANGE_CODEC)->ShowWindow(temp);
#endif
}


void CPublishDlg::OnAspect()
{
	if (m_bWidth)
		OnWidthChange();
	else
		OnHeightChange();

}

void CPublishDlg::OnWidthChange() 
{
	m_bWidth = 1;
	if (m_bKludge)	
		{
		m_bKludge = 0;	// we must have changed the height
		return;
		}
	m_width = GetDlgItemInt(IDC_WIDTH);
	if (IsDlgButtonChecked(IDC_ASPECT))
		{
		m_bKludge = 1;
		m_height = MulDiv(m_width, m_origheight, m_origwidth);
		SetDlgItemInt(IDC_HEIGHT, m_height,0);
		}
}

void CPublishDlg::OnHeightChange() 
{
	m_bWidth = 0;
	if (m_bKludge)
		{
		m_bKludge = 0;  // width was changed
		return;
		}
	m_height = GetDlgItemInt(IDC_HEIGHT);
	if (IsDlgButtonChecked(IDC_ASPECT))
		{
		m_bKludge = 1;
		m_width = MulDiv(m_height, m_origwidth, m_origheight);
		SetDlgItemInt(IDC_WIDTH, m_width,0);
		}
}


void CPublishDlg::OnRateChange() 
{
//	UpdateData(1);
	UINT show;
	BOOL bEnable;
	m_rate = GetDlgItemInt(IDC_RATE);
	if (m_bAllowTelecine && (m_rate == 24))
		{
		bEnable = TRUE;
		show = SW_SHOW;
		}
	else
		{
		bEnable = FALSE;
		show = SW_HIDE;
		}
	GetDlgItem(IDC_TELECINE)->EnableWindow(bEnable);
	GetDlgItem(IDC_TELECINEODD)->EnableWindow(bEnable);
	GetDlgItem(IDC_TELECINEEVEN)->EnableWindow(bEnable);
	GetDlgItem(IDC_TELECINE_TEXT)->ShowWindow(show);
	GetDlgItem(IDC_TELECINE)->ShowWindow(show);
	GetDlgItem(IDC_TELECINEODD)->ShowWindow(show);
	GetDlgItem(IDC_TELECINEEVEN)->ShowWindow(show);
}


void CPublishDlg::OnCodecChange() 
{
	UpdateData(1);
	CodecOptionsDlg();
}


void CPublishDlg::CodecName()
{
#ifndef FLIPBOOK_MAC
	ICINFO icinfo;
	HIC	hic;

	if (m_opts.fccHandler == mmioFOURCC('D','I','B',' '))
		m_codec = "Uncompressed";
	else
	{
	hic = ICOpen(mmioFOURCC('v', 'i', 'd', 'c'),m_opts.fccHandler,ICMODE_QUERY);
	DPF("hic:%lx",(DWORD)hic);
	if (!hic) return ;
   ICGetInfo(hic, &icinfo, sizeof(icinfo));
	m_codec = icinfo.szDescription;
	}
#endif
	UpdateData(0);
}

WORD CPublishDlg::CodecOptionsDlg()
{
#ifndef FLIPBOOK_MAC
	int			status,w,h;
	AVISTREAMINFO strhdr;
	PAVIFILE		pfile = NULL;
	PAVISTREAM		ps = NULL;
	HRESULT		hr;

	char name[400];
	if (!GetTempPath(380, name))
		name[0] = 0;
	strcat(name,"zoutput.avi");
	w = 640;
	h = 480;
	AVIFileInit();
	BOOL bHaveFile = 0;
	hr = AVIFileOpen(&pfile, name, OF_CREATE,	 NULL);

	if (hr != AVIERR_OK)
		status = 1;
	else
		status = 0;
	if (!status)
		{
		bHaveFile = 1;
		memset(&strhdr, 0, sizeof(strhdr));
		strhdr.fccType                = streamtypeVIDEO;
		strhdr.fccHandler             = 0;
		strhdr.dwScale                = 1;
		strhdr.dwRate                 = 30;
		strhdr.dwSuggestedBufferSize  = (DWORD)h * (DWORD)(4 * (w + 3) / 4);
		SetRect(&strhdr.rcFrame, 0, 0,	w,h);
	
	// And create the stream;
		hr = AVIFileCreateStream(pfile, &ps, &strhdr);
		if (hr != AVIERR_OK)
			status = 2;
		}
	if (!status)
		{
		LPAVICOMPRESSOPTIONS  galpAVIOptions[1];
		int i = 0;
		galpAVIOptions[i] = &m_opts;

	//
	// clear options structure to zeroes
	//
#ifdef XXXX
//		memset(galpAVIOptions[i], 0, sizeof(AVICOMPRESSOPTIONS));

		galpAVIOptions[i]->fccType = streamtypeVIDEO;

		galpAVIOptions[i]->dwFlags = AVICOMPRESSF_VALID |
				AVICOMPRESSF_KEYFRAMES | AVICOMPRESSF_DATARATE;
		galpAVIOptions[i]->fccHandler = 0;
		galpAVIOptions[i]->dwQuality = (DWORD)ICQUALITY_DEFAULT;
		galpAVIOptions[i]->dwKeyFrameEvery = (DWORD)-1; // Default
		galpAVIOptions[i]->dwBytesPerSecond = 0;
		galpAVIOptions[i]->dwInterleaveEvery = 1;
#endif
		galpAVIOptions[i]->lpFormat = 0;
		galpAVIOptions[i]->cbFormat = 0;
		galpAVIOptions[i]->lpParms = 0;
		galpAVIOptions[i]->cbParms = 0;
//
//DPF("opts:%8lx,%8lx,%8lx",opts.fccType,opts.fccHandler,opts.dwFlags);
//		LPAVICOMPRESSOPTIONS popts = (LPAVICOMPRESSOPTIONS)&m_opts;
		if (AVISaveOptions(m_hWnd,
				ICMF_CHOOSE_KEYFRAME | ICMF_CHOOSE_DATARATE
				|	ICMF_CHOOSE_ALLCOMPRESSORS
				
				,1, &ps, galpAVIOptions))
			CodecName();

//DPF("opts:%8lx,%8lx,%8lx",opts.fccType,opts.fccHandler,opts.dwFlags);
		AVISaveOptionsFree(1,galpAVIOptions);
//DPF("opts:%8lx,%8lx,%8lx",opts.fccType,opts.fccHandler,opts.dwFlags);
		}
	if (ps)
		AVIStreamRelease(ps);
	if (pfile)
		AVIFileRelease(pfile);
	AVIFileExit();
	if (bHaveFile)
		RemoveFile(name);
	return status;
#else
	return 0;
#endif
}


CExpDlg::CExpDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CExpDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExpDlg)
	m_StartFrame = 1;
	m_EndFrame = 1;
	m_StartLevel = 0;
	m_EndLevel = 0;
	m_MaxFrame = 1;
	m_MaxLevel = 0;
	m_Type = 0;
	m_Flags = 0;
	m_prefix = "";
	m_bAspect = 1;
	m_bScaleBG = 0;
	//}}AFX_DATA_INIT
}


void CExpDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExpDlg)
	DDX_Control(pDX, IDC_EXP_COMBO, m_kinds);
	DDX_Text(pDX, IDC_START_FRAME, m_StartFrame);
	DDV_MinMaxUInt(pDX, m_StartFrame, 1, m_MaxFrame);
	DDX_Text(pDX, IDC_END_FRAME, m_EndFrame);
	DDV_MinMaxUInt(pDX, m_EndFrame, m_StartFrame, m_MaxFrame);
	DDX_Text(pDX, IDC_WIDTH, m_width);
	DDV_MinMaxUInt(pDX, m_width, 40, m_origwidth);
	DDX_Text(pDX, IDC_HEIGHT, m_height);
	DDV_MinMaxUInt(pDX, m_height, 30, m_origheight);
	DDX_Text(pDX, IDC_PREFIX, m_prefix);
	DDX_Radio(pDX, IDC_EXP_CELLS, m_Type);
#ifdef FLIPBOOK_MAC
	// fix for incorrect radio button ordering
	if (m_Type == 1) {
		m_Type = 2;
	}
	else if (m_Type == 2) {
		m_Type = 1;
	}
	DDX_Radio(pDX, IDC_EXP_CELLS, m_Type);
	if (m_Type == 1) {
		m_Type = 2;
	}
	else if (m_Type == 2) {
		m_Type = 1;
	}
#else
	DDX_Radio(pDX, IDC_EXP_CELLS, m_Type);
#endif	
	DDX_Check(pDX, IDC_SCALE_BG, m_bScaleBG);
	UINT MinLevel;
	if (m_Type == 2)
		MinLevel = 1;
	else
		MinLevel = 0;
	DDX_Text(pDX, IDC_START_LEVEL, m_StartLevel);
	DDV_MinMaxUInt(pDX, m_StartLevel, MinLevel, m_MaxLevel);
	DDX_Text(pDX, IDC_END_LEVEL, m_EndLevel);
	DDV_MinMaxUInt(pDX, m_EndLevel, m_StartLevel, m_MaxLevel);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CExpDlg, CMyDialog)
	ON_EN_CHANGE(IDC_WIDTH, OnWidthChange)
	ON_EN_CHANGE(IDC_HEIGHT, OnHeightChange)
	ON_BN_CLICKED(IDC_EXP_CELLS, OnTypeChange)
	ON_BN_CLICKED(IDC_EXP_COMPOSITE, OnTypeChange)
	ON_BN_CLICKED(IDC_EXP_COMBINE, OnTypeChange)
	ON_CBN_SELCHANGE(IDC_EXP_COMBO, OnSelChangeKind)
	ON_COMMAND(IDC_ASPECT, OnAspect)
END_MESSAGE_MAP()


int CExpDlg::AddToCombo(LPCSTR txt, UINT kind, UINT v)
{
	int i = 0;
	i = m_kinds.AddString(txt);
	m_kinds.SetItemData(i,(v << 8) | kind); 
	return i;
}

BOOL CExpDlg::OnInitDialog()
{
	m_bKludge = 0;
	CMyDialog::OnInitDialog();
	GetDlgItem(IDC_PREFIX)->EnableWindow(m_Type);
	CheckDlgButton(IDC_ASPECT, m_bAspect);
	m_bWidth = 1;
	m_kinds.ResetContent();
	int i;
	int j = 0;
	for (i = 0; i < 10; i++)
		{
		int v;
		char buf[30];
		if (v = FBQTGetExt(buf,(FBFileFormat)i))
			{
			int k =  AddToCombo(buf,i,v);
			if (i == m_Kind)
				{
				j = k;
				m_Flags = v;
				}
			}
	
		}
	m_kinds.SetCurSel(j);
	CenterWindow();
	return FALSE;
}

void CExpDlg::OnTypeChange()
{
	UpdateData(1);
	if (!m_Type && !m_StartLevel && m_MaxLevel)
		m_StartLevel = 1;
	if (m_Type == 1)
		{
		m_width = 2 * m_origwidth / m_fact;
		m_height = 2 * m_origheight / m_fact;
		}
	else
		{
		m_width = m_origwidth;
		m_height =  m_origheight;
		}
	GetDlgItem(IDC_PREFIX)->EnableWindow(m_Type);
	UpdateData(0);
}

void CExpDlg::OnSelChangeKind()
{
	int index = m_kinds.GetCurSel();
	if (index >= 0)
		{
		int v = m_kinds.GetItemData(index);
		m_Flags = v >> 8;
		m_Kind = v & 255;
DPF("kind:%d,flags:%d",m_Kind,m_Flags);
		}
}

void CExpDlg::OnAspect() // use last one as driving value
{
	if (m_bWidth)
		OnWidthChange();
	else
		OnHeightChange();

}

void CExpDlg::OnWidthChange() 
{
	m_bWidth = 1;
	if (m_bKludge)	
		{
		m_bKludge = 0;	// we must have changed the height
		return;
		}
	m_width = GetDlgItemInt(IDC_WIDTH);
	if (IsDlgButtonChecked(IDC_ASPECT))
		{
		m_height = MulDiv(m_width, m_origheight, m_origwidth);
		m_bKludge = 1;
		SetDlgItemInt(IDC_HEIGHT, m_height,0);
		}
}

void CExpDlg::OnHeightChange() 
{
	m_bWidth = 0;
	if (m_bKludge)	
		{
		m_bKludge = 0;	// we must have changed thewidth
		return;
		}
	m_height = GetDlgItemInt(IDC_HEIGHT);
	if (IsDlgButtonChecked(IDC_ASPECT))
		{
		m_width = MulDiv(m_height, m_origwidth, m_origheight);
		m_bKludge = 1;	// set race flag to avoid changing width again
		SetDlgItemInt(IDC_WIDTH, m_width,0);
		}
}
#endif

CExpCamDlg::CExpCamDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CExpCamDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExpCamDlg)
	m_StartFrame = 1;
	m_EndFrame = 1;
	m_StartLevel = 0;
	m_EndLevel = 0;
	m_MaxFrame = 1;
	m_MaxLevel = 0;
	m_name = "";
	m_Flags = 0;
	//}}AFX_DATA_INIT
}


void CExpCamDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExpCamDlg)
	DDX_Text(pDX, IDC_START_FRAME, m_StartFrame);
	DDV_MinMaxUInt(pDX, m_StartFrame, 1, m_MaxFrame);
	DDX_Text(pDX, IDC_END_FRAME, m_EndFrame);
	DDV_MinMaxUInt(pDX, m_EndFrame, m_StartFrame, m_MaxFrame);
	DDX_Text(pDX, IDC_START_LEVEL, m_StartLevel);
	DDV_MinMaxUInt(pDX, m_StartLevel, 0, m_MaxLevel);
	DDX_Text(pDX, IDC_END_LEVEL, m_EndLevel);
	DDV_MinMaxUInt(pDX, m_EndLevel, m_StartLevel, m_MaxLevel);
	DDX_Check(pDX, IDC_CAM_GROUPS, m_bDoGroups);
	DDX_Check(pDX, IDC_CAM_COMPUTED, m_bCamComputed);
	DDX_Check(pDX, IDC_CAM_ONLY_ACTIVE, m_bActiveOnly);
	DDX_Check(pDX, IDC_CAM_X, m_bCamX);
	DDX_Check(pDX, IDC_CAM_Y, m_bCamY);
	DDX_Check(pDX, IDC_CAM_Z, m_bCamZ);
	DDX_Check(pDX, IDC_CAM_R, m_bCamR);
	DDX_Check(pDX, IDC_CAM_B, m_bCamB);
	DDX_Check(pDX, IDC_CAM_A, m_bCamA);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CExpCamDlg, CMyDialog)
	ON_COMMAND(IDC_CAM_ALL, OnAll)
	ON_COMMAND(IDC_CAM_NONE, OnNone)
END_MESSAGE_MAP()


BOOL CExpCamDlg::OnInitDialog()
{
	CMyDialog::OnInitDialog();
	if ((m_StartLevel != 0) || (m_EndLevel != m_MaxLevel))
		GetDlgItem(IDC_CAM_ONLY_ACTIVE)->ShowWindow(SW_HIDE);
	CenterWindow();
	return FALSE;
}


void CExpCamDlg::CheckEm(BOOL v) 
{
	CheckDlgButton(IDC_CAM_X, v);
	CheckDlgButton(IDC_CAM_Y, v);
	CheckDlgButton(IDC_CAM_Z, v);
	CheckDlgButton(IDC_CAM_R, v);
	CheckDlgButton(IDC_CAM_B, v);
	CheckDlgButton(IDC_CAM_A, v);
}

CImpCamDlg::CImpCamDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CImpCamDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CImpCamDlg)
	m_StartFrame = 1;
	m_EndFrame = 1;
	m_StartLevel = 0;
	m_EndLevel = 0;
	m_MaxFrame = 1;
	m_MaxLevel = 0;
	m_Flags = 0;
	m_nPeg = 9999;
	//}}AFX_DATA_INIT
}


void CImpCamDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImpCamDlg)
	DDX_Control(pDX, IDC_CAM_LIST, m_list);
	DDX_Text(pDX, IDC_START_FRAME, m_StartFrame);
	DDV_MinMaxUInt(pDX, m_StartFrame, 1, m_MaxFrame);
//	DDX_Text(pDX, IDC_END_FRAME, m_EndFrame);
//	DDV_MinMaxUInt(pDX, m_EndFrame, m_StartFrame, m_MaxFrame);
	DDX_Text(pDX, IDC_START_LEVEL, m_StartLevel);
	DDV_MinMaxUInt(pDX, m_StartLevel, 0, m_MaxLevel);
//	DDX_Text(pDX, IDC_END_LEVEL, m_EndLevel);
//	DDV_MinMaxUInt(pDX, m_EndLevel, m_StartLevel, m_MaxLevel);
//	DDX_Check(pDX, IDC_CAM_GROUPS, m_bDoGroups);
	DDX_Check(pDX, IDC_CAM_COMPUTED, m_bCamComputed);
	DDX_Check(pDX, IDC_CAM_X, m_bCamX);
	DDX_Check(pDX, IDC_CAM_Y, m_bCamY);
	DDX_Check(pDX, IDC_CAM_Z, m_bCamZ);
	DDX_Check(pDX, IDC_CAM_R, m_bCamR);
	DDX_Check(pDX, IDC_CAM_B, m_bCamB);
	DDX_Check(pDX, IDC_CAM_A, m_bCamA);
	DDX_Check(pDX, IDC_CAM_IMP_GROUP, m_bCamGroupInfo);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CImpCamDlg, CMyDialog)
	ON_COMMAND(IDC_CAM_ALL, OnAll)
	ON_COMMAND(IDC_CAM_NONE, OnNone)
	ON_LBN_SELCHANGE(IDC_CAM_LIST, OnSelChange)
END_MESSAGE_MAP()


BOOL CImpCamDlg::OnInitDialog()
{
	CMyDialog::OnInitDialog();
	m_list.ResetContent();
	int i = m_list.AddString("All");
	m_nPeg = 9999;
	m_list.SetItemData(i,m_nPeg); 
	int j,c;
	c = m_pMoves->Pegs();
	CString id,str,str2;
	for (j = 0; j < c; j++)
		{
		if (m_pMoves->IsPeg(j))
			{
			m_pMoves->Name(j,id,str);
			str2.Format("%s - %s",id,str);
			int i = m_list.AddString(str2);
			m_list.SetItemData(i,j); 
			}
		}
	m_list.SetCurSel(0);
	CenterWindow();
	HideEm();
	return FALSE;
}

void CImpCamDlg::HideEm()
{
	UINT show = (m_nPeg == 9999) ? SW_HIDE : SW_SHOW;
	GetDlgItem(IDC_CAM_IMP_RANGE)->ShowWindow(show);
	GetDlgItem(IDC_CAM_IMP_FRAME)->ShowWindow(show);
	GetDlgItem(IDC_CAM_IMP_LEVEL)->ShowWindow(show);
	GetDlgItem(IDC_START_FRAME)->ShowWindow(show);
	GetDlgItem(IDC_START_LEVEL)->ShowWindow(show);
	if (m_nPeg != 9999)
		{
		if (!m_pMoves->HasGroup(m_nPeg))
			show = SW_HIDE;
		}
	GetDlgItem(IDC_CAM_IMP_GROUP)->ShowWindow(show);
}

void CImpCamDlg::OnSelChange()
{
	int index = m_list.GetCurSel();
	m_nPeg = m_list.GetItemData(index);
	HideEm();
}


void CImpCamDlg::CheckEm(BOOL v) 
{
	CheckDlgButton(IDC_CAM_X, v);
	CheckDlgButton(IDC_CAM_Y, v);
	CheckDlgButton(IDC_CAM_Z, v);
	CheckDlgButton(IDC_CAM_R, v);
	CheckDlgButton(IDC_CAM_B, v);
	CheckDlgButton(IDC_CAM_A, v);
}

CLibSaveDlg::CLibSaveDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CLibSaveDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLibSaveDlg)
	m_StartFrame = 1;
	m_EndFrame = 1;
	m_StartLevel = 0;
	m_EndLevel = 0;
	m_MaxFrame = 1;
	m_MaxLevel = 0;
	m_bAspect = 1;
	m_bShowGrid = 1;
	m_bManipulate = 1;
	m_bMatchColor = 0;
	m_nColumns = 12;
	m_bFreeform = 1;
	m_bApplyName= 1;
	m_bAutoLoad = 0;
}


void CLibSaveDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLibSaveDlg)
	DDX_Text(pDX, IDC_START_FRAME, m_StartFrame);
	DDV_MinMaxUInt(pDX, m_StartFrame, 1, m_MaxFrame);
	DDX_Text(pDX, IDC_END_FRAME, m_EndFrame);
	DDV_MinMaxUInt(pDX, m_EndFrame, m_StartFrame, m_MaxFrame);
	DDX_Text(pDX, IDC_START_LEVEL, m_StartLevel);
	DDV_MinMaxUInt(pDX, m_StartLevel, 1, m_MaxLevel);
	DDX_Text(pDX, IDC_END_LEVEL, m_EndLevel);
	DDV_MinMaxUInt(pDX, m_EndLevel, m_StartLevel, m_MaxLevel);
	DDX_Check(pDX, IDC_ASPECT, m_bAspect);
	DDX_Check(pDX, IDC_LIB_GRID, m_bShowGrid);
	DDX_Check(pDX, IDC_MANIPULATE, m_bManipulate);
	DDX_Check(pDX, IDC_LIB_PALETTE, m_bMatchColor);
	DDX_Check(pDX, IDC_SET_NAME, m_bApplyName);
	DDX_Check(pDX, IDC_AUTO_LOAD, m_bAutoLoad);
	DDX_Text(pDX, IDC_COLUMNS, m_nColumns);
	DDV_MinMaxUInt(pDX, m_nColumns, 1, 20);
	DDX_Radio(pDX, IDC_GRID, m_bFreeform);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CLibSaveDlg, CMyDialog)
	ON_COMMAND(IDC_GRID, OnGrid)
	ON_COMMAND(IDC_FREEFORM, OnGrid)
	ON_COMMAND(IDC_MANIPULATE, OnManipulate)
END_MESSAGE_MAP()


BOOL CLibSaveDlg::OnInitDialog()
{
	CMyDialog::OnInitDialog();
	GetDlgItem(IDC_COLUMNS)->ShowWindow(m_bFreeform ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_ASPECT)->EnableWindow(m_bManipulate);
	CenterWindow();
	return FALSE;
}

void CLibSaveDlg::OnGrid()
{
	UpdateData();
	GetDlgItem(IDC_COLUMNS)->ShowWindow(m_bFreeform ? SW_SHOW : SW_HIDE);
}


void CLibSaveDlg::OnManipulate()
{
	UpdateData();
	GetDlgItem(IDC_ASPECT)->EnableWindow(m_bManipulate);
}


CExtEditDlg::CExtEditDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CExtEditDlg::IDD, pParent)
{
	m_StartFrame = 1;
	m_StartLevel = 0;
	m_MaxFrame = 1;
	m_MaxLevel = 0;
//	m_Type = 0;
//	m_Kind = 0;
	m_b24 = 0;
	m_bKeying = 0;
	m_nAlpha = 255;
}


BEGIN_MESSAGE_MAP(CExtEditDlg, CMyDialog)
	ON_BN_CLICKED(IDC_EXT_24, OnEdit24)
	ON_BN_CLICKED(IDC_KEYING, OnKeying)
END_MESSAGE_MAP()


void CExtEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImpCamDlg)
	DDX_Text(pDX, IDC_START_FRAME, m_StartFrame);
	DDV_MinMaxUInt(pDX, m_StartFrame, 0, m_MaxFrame);
	DDX_Text(pDX, IDC_START_LEVEL, m_StartLevel);
	DDV_MinMaxUInt(pDX, m_StartLevel, 0, m_MaxLevel);
	DDX_Text(pDX, IDC_ALPHA_KEY, m_nAlpha);
	DDV_MinMaxUInt(pDX, m_nAlpha, 0, 255);
	DDX_Check(pDX, IDC_EXT_24, m_b24);
	DDX_Check(pDX, IDC_KEYING, m_bKeying);
	DDX_Check(pDX, IDC_NOT_AGAIN, m_bNotAgain);
	//}}AFX_DATA_MAP
}

BOOL CExtEditDlg::OnInitDialog()
{
//	GetDlgItem(IDC_EXP_TGA)->EnableWindow(m_bColor);
//	GetDlgItem(IDC_EXP_TGA32)->EnableWindow(m_bColor);
	CMyDialog::OnInitDialog();
	GetDlgItem(IDC_ALPHA_KEY)->EnableWindow(m_b24 && m_bKeying);
	GetDlgItem(IDC_KEYING)->EnableWindow(m_b24);
//	GetDlgItem(IDC_ALPHA_KEY)->ShowWindow(m_b24 && m_bKeying ? SW_SHOW : SW_HIDE);
//	GetDlgItem(IDC_KEYING)->ShowWindow(m_b24 ? SW_SHOW : SW_HIDE);
	CenterWindow();
	return FALSE;
}

void CExtEditDlg::OnEdit24() 
{
	UpdateData();
//	m_b24 = !m_b24;
	if (!m_b24)
		m_bKeying = 0;
	GetDlgItem(IDC_ALPHA_KEY)->EnableWindow(m_b24 && m_bKeying);
	GetDlgItem(IDC_KEYING)->EnableWindow(m_b24);
}

void CExtEditDlg::OnKeying() 
{
	UpdateData();
//	m_bKeying = !m_bKeying;
	GetDlgItem(IDC_ALPHA_KEY)->EnableWindow(m_bKeying);
}


CLevelDlg::CLevelDlg(UINT Id, CWnd* pParent /*=NULL*/)
	: CMyDialog(Id, pParent)
{
	//{{AFX_DATA_INIT(CLevelDlg)
	m_bPro = Id == IDD_LEVEL_PRO;
	//}}AFX_DATA_INIT
	
}

void CLevelDlg::DoDataExchange(CDataExchange* pDX)
{
	DPF("level data xchg");
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLevelDlg)
	DDX_Check(pDX, IDC_LEVEL_FLAG, m_bUseLevel);
	DDX_Check(pDX, IDC_MDL_USE, m_bUseModel);
	DDX_Text(pDX, IDC_LEVEL_NAME, m_name);
	DDX_Text(pDX, IDC_MDL_NAME, m_mdlname);
	DDX_Control(pDX, IDC_LEVEL_PAL, m_pals);
	if (m_bPro)
		DDX_Control(pDX, IDC_PEGS, m_pegs);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CLevelDlg, CMyDialog)
	//{{AFX_MSG_MAP(CLevelDlg)
	ON_COMMAND(IDC_MDL_CHANGE, OnChangeMdl)
//	ON_COMMAND(IDC_LEVEL_PAL_IMP, OnImport)
//	ON_COMMAND(IDC_LEVEL_PAL_EXP, OnExport)
	ON_COMMAND(IDC_MDL_USE, OnModelUse)
	ON_CBN_SELCHANGE(IDC_LEVEL_PAL, OnSelChangePal)
	ON_CBN_SELCHANGE(IDC_PEGS, OnSelChangePeg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CLevelDlg::FillPalCombo()
{
	m_pals.ResetContent();
	int j,i,c;
	c = NBR_PALS;
	j = 0;
	for (i = 0; i < c; i++)
		{
		CNewPals * pPal = m_pScene->PalettePtr(i);
		if (pPal)
			{
			char buf[80];
#ifdef MYBUG
			sprintf(buf,"%s [%d]",pPal->GetPalName(),i);
#else
			strcpy(buf,pPal->GetPalName());
#endif
			int z = m_pals.AddString(buf);
			if (i == m_index)
				j = z;
			m_pals.SetItemData(z,i); 
			}
		}
	m_pals.SetCurSel(j);
	BOOL bEnable = m_level ? 1 : 0;
	GetDlgItem(IDC_LEVEL_PAL)->EnableWindow(bEnable);
//	GetDlgItem(IDC_LEVEL_PAL_EXP)->EnableWindow(bEnable);
//	GetDlgItem(IDC_LEVEL_PAL_IMP)->EnableWindow(bEnable);
}

BOOL CLevelDlg::OnInitDialog()
{
	CenterWindow();
	// we need to initialize these (especially m_bUseModel) before calling OnInitDialog to not confuse the Mac version
	m_pCamera = m_pScene->Camera();
	m_bUseModel = !m_mdlname.IsEmpty();
	CMyDialog::OnInitDialog();
	FillPalCombo();
	GetDlgItem(IDC_MDL_CHANGE)->EnableWindow(m_bUseModel);
	if (m_bPro)
		{
		m_pegs.ResetContent();
		int i,c;
		c = m_pCamera->PegCount();
		for (i = 0; i < c; i++)
			{
			char buf[40];
			m_pCamera->PegName(buf, i);
			m_pegs.AddString(buf);
			}
		m_pegs.SetCurSel(m_peg);
		}
	UpdateData(FALSE);
	return FALSE;
}

void CLevelDlg::OnSelChangePal()
{
	UINT i = m_pals.GetCurSel();
	m_index = m_pals.GetItemData(i);
//	m_pScene->PalIndex(m_level,m_index);
	DPF("lvl pal sel chng:%d",m_index);
}

void CLevelDlg::OnSelChangePeg()
{
	UINT i = m_pegs.GetCurSel();
	m_peg = i;
	DPF("lvl peg sel chng:%d",m_peg);
}

void CLevelDlg::OnModelUse()
{
	UpdateData(1);
	if (m_bUseModel)
		OnChangeMdl();
	else
		m_mdlname = "";
	m_bUseModel = !m_mdlname.IsEmpty();
	GetDlgItem(IDC_MDL_CHANGE)->EnableWindow(m_bUseModel);
	UpdateData(0);
}

void CLevelDlg::OnChangeMdl()
{
	UpdateData(1);
	CString fileName = m_mdlname;
	if (!((CSketchApp*) AfxGetApp())->PromptFileName(fileName, 8))
		return;
	m_mdlname = fileName;
	UpdateData(0);
//	m_bMdlChanged = TRUE;
}



CGroupDlg::CGroupDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CGroupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGroupDlg)
	//}}AFX_DATA_INIT
	
}

void CGroupDlg::DoDataExchange(CDataExchange* pDX)
{
	DPF("group data xchg");
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGroupDlg)
	DDX_Text(pDX, IDC_GROUP_NAME, m_name);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CGroupDlg, CMyDialog)
	//{{AFX_MSG_MAP(CGroupDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CGroupDlg::OnInitDialog()
{
	CenterWindow();
	CMyDialog::OnInitDialog();
	char buf[80];
	m_pCamera->GroupName(buf, m_group);
	m_name = buf;
	UpdateData(FALSE);
	return FALSE;
}

/*
void CGroupDlg::OnSelChangePeg()
{
	int i = m_pegs.GetCurSel();
	int j = m_pegs.GetItemData(i);
	if (j != m_peg)
		m_bPegChanged = TRUE;
	m_peg = j;
	DPF("lvl peg sel chng:%d",m_peg);
}

*/

CPegDlg::CPegDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CPegDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPegDlg)
	m_nFieldComp = 0;
	m_bAllowFieldComp = 0;
	//}}AFX_DATA_INIT
	
}

void CPegDlg::DoDataExchange(CDataExchange* pDX)
{
	DPF("level data xchg");
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPegDlg)
	DDX_Text(pDX, IDC_PEG_NAME, m_name);
	DDX_Control(pDX, IDC_PEG_GROUP, m_groups);
	DDX_Check(pDX, IDC_PEG_FIELDCOMP, m_nFieldComp);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPegDlg, CMyDialog)
	//{{AFX_MSG_MAP(CPegDlg)
	ON_CBN_SELCHANGE(IDC_PEG_GROUP, OnSelChangeGroup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CPegDlg::OnInitDialog()
{
	GetDlgItem(IDC_PEG_FIELDCOMP)->EnableWindow(m_bAllowFieldComp);
	if (!m_bAllowFieldComp)
		GetDlgItem(IDC_PEG_FIELDCOMP)->ShowWindow(SW_HIDE);
	CenterWindow();
	CMyDialog::OnInitDialog();
//	m_group = m_pCamera->GroupFindPeg(m_peg);
	m_groups.ResetContent();
	int i;
	int c = m_pCamera->GroupCount();
	for (i = 1; i < c; i++)
		{
		char buf[40];
		m_pCamera->GroupName(buf, i);
		m_groups.AddString(buf);
		}
	m_groups.SetCurSel(m_group-1);
	char buf[80];
	m_pCamera->PegName(buf, m_peg);
	m_name = buf;
	UpdateData(FALSE);
	return FALSE;
}


void CPegDlg::OnSelChangeGroup()
{
	m_group = 1 + m_groups.GetCurSel();
	DPF("peg group, was:%d,sel chng:%d",m_peg,m_group);
}


CCellDlg::CCellDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CCellDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCellDlg)
	//}}AFX_DATA_INIT
}

void CCellDlg::DoDataExchange(CDataExchange* pDX)
{
	DPF("cell data xchg");
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCellDlg)
	DDX_Text(pDX, IDC_CELL_NAME, m_name);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCellDlg, CMyDialog)
	//{{AFX_MSG_MAP(CCellDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CSoundTrackDlg::CSoundTrackDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CSoundTrackDlg::IDD, pParent)
{
}

void CSoundTrackDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_SND_TRACK, m_nTrack);
	DDV_MinMaxInt(pDX, m_nTrack, 1, m_max);
	//}}AFX_DATA_MAP
}


CSoundDlg::CSoundDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CSoundDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSoundDlg)
	//}}AFX_DATA_INIT
}

void CSoundDlg::DoDataExchange(CDataExchange* pDX)
{
	DPF("cell data xchg");
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSoundDlg)
	DDX_Check(pDX, IDC_SND_ENABLE, m_bUseSound);
//	DDX_Check(pDX, IDC_SND_INTERNAL, m_bInternal);
	DDX_Text(pDX, IDC_SND_WAVE, m_name);
	DDX_Text(pDX, IDC_SND_VMARK, m_vmark);
	DDV_MinMaxUInt(pDX, m_vmark, 1, m_frames);
	DDX_Text(pDX, IDC_SND_SNIP, m_snip);
	DDV_MinMaxUInt(pDX, m_snip, 1, 10);
//	DDX_Text(pDX, IDC_SND_VOLUME, m_nVolume);
//	DDV_MinMaxInt(pDX, m_nVolume,-18, +18);
	DDX_Text(pDX, IDC_SND_SMARK, m_smark);
	DDV_MinMaxDouble(pDX, m_smark, 0, 10);
	DDX_Control(pDX, IDC_SND_SLIDER, m_slider);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSoundDlg, CMyDialog)
	//{{AFX_MSG_MAP(CSoundDlg)
	ON_BN_CLICKED(IDC_SND_CHANGE, OnChange)
//	ON_BN_CLICKED(IDC_SND_INTERNAL, OnInternal)
	ON_WM_VSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CSoundDlg::OnInitDialog()
{ 
	int mask = 0;
	m_bQT = 0;
	#ifndef FLIPBOOK_MAC
#ifdef USEQT
		if (FBQuickTime())
			m_bQT = 1;
#endif
	#else
		m_bQT = 1;
	#endif
	CMyDialog::OnInitDialog();
//	m_bInternal ^= 1;
//	OnInternal();
	m_slider.SetRange(0,200);
	m_slider.SetPos(100-m_nVolume);
	m_slider.SetTicFreq(10);
	return FALSE;
}

void CSoundDlg::OnVScroll( UINT nCode, UINT nPos, CScrollBar* pScrollBar )
{
	CSliderCtrl * pSlider = (CSliderCtrl *)pScrollBar;

	m_nVolume = 100- pSlider->GetPos();
	DPF("snd,hscroll, code:%d, pos:%d,act:%d",nCode, nPos,m_nVolume);
	m_pSound->Volume(m_id, m_nVolume);
}

/*
void CSoundDlg::OnInternal() 
{
	GetDlgItem(IDC_SND_CHANGE)->EnableWindow(m_bInternal);
	GetDlgItem(IDC_SND_WAVE)->EnableWindow(m_bInternal);
	m_bInternal ^= 1;
DPF("on internal:%d", m_bInternal);
	CheckDlgButton(IDC_SND_INTERNAL, m_bInternal);
}
*/
void CSoundDlg::OnChange() 
{
    OPENFILENAME ofn;
	char suffix[10];
	char	    Filter[222];
	strcpy(suffix,"WAV");
	if (m_bQT)
		{
		strcpy(Filter,"Sound Files |*.wav;*.mp3;*.aif");
		strcat(Filter,"|WAV Files (*.wav)|*.wav|MP3 Files(*.mp3)|*.mp3");
		strcat(Filter,"|AIF Files (*.aif)|*.aif||");
		}
	else
		{
		strcpy(Filter,"Wave Files (*.wav)|*.WAV||");
		}
	int i;
	for (i = 1;i <222 ;i++)
		{
		if (Filter[i] == '|')
			{
			Filter[i] = '\0';
			if (!Filter[i-1])
				break;
			}
		}
	char        fileName[300] = "";
//	char	    gachFilter[22] = "Wave\0*.WAV\0\0";
//	char	    gachFilter[122] = "Wave|*.WAV|MP3 |*.mp3||";
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = m_hWnd;
	ofn.hInstance = NULL;
	ofn.lpstrTitle = "Open Sound";
	ofn.lpstrFilter = Filter;
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
	ofn.lpstrDefExt = (LPCSTR)&suffix;
	ofn.lCustData = 0;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;
	if (!GetOpenFileNamePreview(&ofn))
		return;
DPF("got name:%s",fileName);
	UpdateData(1);
	m_name = fileName;
	UpdateData(0);
	m_bChanged = 1;
	m_vmark = 1;
	m_smark = 0;
//	hResult = AVIFileOpen(&pfile, fileName, 0, 0L);
//DPF("result:%x",(DWORD)hResult);
//	if (hResult)
	return;
}

CFrameDlg::CFrameDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CFrameDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFrameDlg)
	//}}AFX_DATA_INIT
}

void CFrameDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_FRAME, m_frame);
	DDX_Text(pDX, IDC_COUNT, m_count);
	DDV_MinMaxUInt(pDX, m_count, 1, m_max);
}

BEGIN_MESSAGE_MAP(CFrameDlg, CMyDialog)
	//{{AFX_MSG_MAP(CFrameDlg)
	ON_BN_CLICKED(IDC_FRM_INS, OnInsert)
	ON_BN_CLICKED(IDC_FRM_DEL, OnDelete)
	ON_BN_CLICKED(IDC_FRM_APPEND, OnAppend)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CFrameDlg::OnInsert()
{
	m_max = m_max_add;
	if (UpdateData())
		EndDialog(100);
}

void CFrameDlg::OnDelete()
{
	m_max = m_max_del;
	if (UpdateData())
		EndDialog(101);
}

void CFrameDlg::OnAppend()
{
	m_max = m_max_add;
	if (UpdateData())
		EndDialog(102);
}





CExistOneDlg::CExistOneDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CExistOneDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExistOneDlg)
	//}}AFX_DATA_INIT
}


void CExistOneDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExistOneDlg)
	DDX_Text(pDX, IDC_FRAME, m_Frame);
	DDX_Text(pDX, IDC_LEVEL, m_Level);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExistOneDlg, CMyDialog)
	//{{AFX_MSG_MAP(CExistOneDlg)
	ON_BN_CLICKED(IDC_YESREST, OnYesRest)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExistOneDlg message handlers


void CExistOneDlg::OnYesRest() 
{
	EndDialog(IDC_YESREST);
}

CDemoDlg::CDemoDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CDemoDlg::IDD, pParent)
{
}


BEGIN_MESSAGE_MAP(CDemoDlg, CMyDialog)
	//{{AFX_MSG_MAP(CExistOneDlg)
	ON_BN_CLICKED(IDC_BUY_IT, OnBuyIt)
	ON_BN_CLICKED(IDC_TRY_IT, OnTryIt)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CDemoDlg::OnInitDialog()
{
	CMyDialog::OnInitDialog();
	SetDlgItemText(IDC_APID,m_id);
	return FALSE;
}

void CDemoDlg::OnBuyIt() 
{
	EndDialog(IDC_BUY_IT);
}

void CDemoDlg::OnTryIt() 
{
	EndDialog(IDC_TRY_IT);
}


CYesRestDlg::CYesRestDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CYesRestDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CYesRestDlg)
	//}}AFX_DATA_INIT
}


void CYesRestDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CYesRestDlg)
	DDX_Text(pDX, IDC_CELLFRM, m_Frame);
	DDX_Text(pDX, IDC_CELLLVL, m_Level);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CYesRestDlg, CMyDialog)
	//{{AFX_MSG_MAP(CYesRestDlg)
	ON_BN_CLICKED(IDC_YES, OnYes)
	ON_BN_CLICKED(IDC_YESREST, OnYesRest)
	ON_BN_CLICKED(IDC_NOO, OnNo)
	ON_BN_CLICKED(IDC_NOREST, OnNoRest)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CYesRestDlg message handlers

BOOL CYesRestDlg::OnInitDialog()
{
	CMyDialog::OnInitDialog();
	return FALSE;
}

void CYesRestDlg::OnYes() 
{
	EndDialog(IDC_YES);
}

void CYesRestDlg::OnNo() 
{
	EndDialog(IDC_NOO);
}

void CYesRestDlg::OnYesRest() 
{
	EndDialog(IDC_YESREST);
}

void CYesRestDlg::OnNoRest() 
{
	EndDialog(IDC_NOREST);
}

CReplaceDlg::CReplaceDlg(UINT idd, CWnd* pParent /*=NULL*/)
	: CMyDialog(idd, pParent)
{
}


void CReplaceDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CReplaceDlg)
	DDX_Text(pDX, IDC_START_FRAME, m_StartFrame);
	DDX_Text(pDX, IDC_START_LEVEL, m_StartLevel);
	DDX_Text(pDX, IDC_END_FRAME, m_EndFrame);
	DDX_Text(pDX, IDC_END_LEVEL, m_EndLevel);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CReplaceDlg, CMyDialog)
	//{{AFX_MSG_MAP(CReplaceDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


CKeyTypeDlg::CKeyTypeDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CKeyTypeDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CKeyTypeDlg)
	//}}AFX_DATA_INIT
}


void CKeyTypeDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CKeyTypeDlg)
	DDX_Text(pDX, IDC_EASEIN, m_EaseIn);
	DDV_MinMaxUInt(pDX, m_EaseIn, 0, m_Frames - m_Frame);
	DDX_Text(pDX, IDC_EASEOUT, m_EaseOut);
	DDV_MinMaxUInt(pDX, m_EaseOut, 0, m_Frame - 1);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CKeyTypeDlg, CMyDialog)
	//{{AFX_MSG_MAP(CKeyTypeDlg)
	ON_BN_CLICKED(IDC_KEY_SMOOTH, OnSmooth)
	ON_BN_CLICKED(IDC_KEY_LINEAR, OnLinear)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CKeyTypeDlg::OnInitDialog()
{ 
	CString string;
	if (string.LoadString(m_Id))
		SetWindowText(string);
	CMyDialog::OnInitDialog();
	if ((m_Frame <= 1) || (m_Frame >= m_Frames))
		m_Kind = 0;
	SetKind(m_Kind);
	CheckRadioButton(IDC_KEY_LINEAR, IDC_KEY_SMOOTH, IDC_KEY_LINEAR+m_Kind);
//	UpdateControls();
//	CheckDlgButton(IDC_ASPECT, m_bAspect);
//	CenterWindow();
	return FALSE;
}


void CKeyTypeDlg::SetKind(UINT kind)
{
	m_Kind = kind;
	UpdateControls();
}

void CKeyTypeDlg::UpdateControls()
{
	BOOL bEnableKind = TRUE;
	BOOL bEnableI = m_Kind ? FALSE : TRUE;
	BOOL bEnableO = m_Kind ? FALSE : TRUE;
	if (m_Frame < 2)
		bEnableO = bEnableKind = FALSE;
	if (m_Frame >= m_Frames)
		bEnableI = bEnableKind = FALSE;
	GetDlgItem(IDC_KEY_SMOOTH)->EnableWindow(bEnableKind);
	GetDlgItem(IDC_EI_TEXT)->EnableWindow(bEnableI);
	GetDlgItem(IDC_EASEIN)->EnableWindow(bEnableI);
	GetDlgItem(IDC_EO_TEXT)->EnableWindow(bEnableO);
	GetDlgItem(IDC_EASEOUT)->EnableWindow(bEnableO);
}

CLogDlg::CLogDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CLogDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLogDlg)
	//}}AFX_DATA_INIT
}

BOOL CLogDlg::OnInitDialog()
{
	CMyDialog::OnInitDialog();
	SetWindowText(m_title);
//	CFont font;
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));       // zero out structure
	lf.lfHeight = 15;                      // request a 12-pixel-height font
#ifndef FLIPBOOK_MAC
	lf.lfPitchAndFamily = FIXED_PITCH;
#endif
	strcpy(lf.lfFaceName, "Courier New");
	VERIFY(m_font.CreateFontIndirect(&lf));  // create the font
	GetDlgItem(IDC_TXTLOG)->SetFont(&m_font);
//	font.DeleteObject();
	GetDlgItem(IDC_TXTLOG)->SetWindowText(m_pData);
	return TRUE;
}


CPrintDlg::CPrintDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CPrintDlg::IDD, pParent)
{
}

BEGIN_MESSAGE_MAP(CPrintDlg, CMyDialog)
	ON_BN_CLICKED(IDC_PRINT_SETUP, OnSetup)
	ON_BN_CLICKED(IDC_PRINT_PREVIEW, OnPreview)
	ON_BN_CLICKED(IDC_PRINT_PRINT, OnPrint)
	ON_BN_CLICKED(IDC_PRINT_SINGLE,MySetup)
	ON_BN_CLICKED(IDC_PRINT_LEVELS,MySetup)
	ON_BN_CLICKED(IDC_PRINT_FRAMES,MySetup)
	ON_BN_CLICKED(IDC_PRINT_CEL,MySetup)
	ON_BN_CLICKED(IDC_PRINT_FRAME,MySetup)
	ON_BN_CLICKED(IDC_PRINT_XSHEET,MySetup)
END_MESSAGE_MAP()


void CPrintDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Radio(pDX, IDC_PRINT_CEL, m_nKind);
	DDX_Radio(pDX, IDC_PRINT_SINGLE, m_nFit);
	DDX_Radio(pDX, IDC_PRINT_OVER, m_bDown);
	DDX_Check(pDX, IDC_PRINT_MARKS, m_bMarks);
	DDX_Text(pDX, IDC_PRINT_LEVEL_CNT, m_nLCount);
	DDV_MinMaxInt(pDX, m_nLCount, 1, 20);
	DDX_Text(pDX, IDC_PRINT_FRAME_CNT, m_nFCount);
	DDV_MinMaxInt(pDX, m_nFCount, 1, 50);
}

BOOL CPrintDlg::OnInitDialog()
{
	CMyDialog::OnInitDialog();
	MySetup();
	return TRUE;
}


void CPrintDlg::MyExit(int code)
{
DPF("my exit:%d",code);
	UpdateData(1);
EndDialog( code);
}
void CPrintDlg::MySetup()
{
	UpdateData(1);
	BOOL bEnable = m_nKind == 2;
	
	GetDlgItem(IDC_PRINT_SINGLE)->EnableWindow(bEnable);
	GetDlgItem(IDC_PRINT_LEVELS)->EnableWindow(bEnable);
	GetDlgItem(IDC_PRINT_FRAMES)->EnableWindow(bEnable);
	GetDlgItem(IDC_PRINT_OVER)->EnableWindow(bEnable);
	GetDlgItem(IDC_PRINT_DOWN)->EnableWindow(bEnable);
	UINT nFit = m_nFit;
	if (!bEnable)
		nFit = 0;
	GetDlgItem(IDC_PRINT_LEVEL_CNT)->ShowWindow(nFit == 1 ? SW_SHOW:SW_HIDE);
	GetDlgItem(IDC_PRINT_FRAME_CNT)->ShowWindow(nFit == 2 ? SW_SHOW:SW_HIDE);
	GetDlgItem(IDC_PRINT_MARKS)->ShowWindow(!bEnable ? SW_SHOW:SW_HIDE);
}


CLayerDlg::CLayerDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CLayerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLayerDlg)
	//}}AFX_DATA_INIT
}


void CLayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLayerDlg)
	DDX_Control(pDX, IDC_LAYER, m_list);
	DDX_Text(pDX, IDC_LAY_NAME, m_name);
	DDX_Text(pDX, IDC_LAY_BLUR, m_blur);
	DDV_MinMaxInt(pDX, m_blur, 0, 50);
	DDX_Text(pDX, IDC_LAY_OFFX, m_dx);
	DDV_MinMaxInt(pDX, m_dx, -100, 100);
	DDX_Text(pDX, IDC_LAY_OFFY, m_dy);
	DDV_MinMaxInt(pDX, m_dy, -100, 100);
	DDX_Check( pDX, IDC_LAY_DISPLAY, m_bDisplay);
	DDX_Check( pDX, IDC_LAY_ACTIVE, m_bActive);
	DDX_Check( pDX, IDC_LAY_WITHIN, m_bWithin);
	DDX_Radio(pDX, IDC_LAY_BLURIN, m_nBlurType);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CLayerDlg, CMyDialog)
	//{{AFX_MSG_MAP(CLevelDlg)
	ON_LBN_SELCHANGE(IDC_LAYER, OnSelChange)
	ON_COMMAND(IDC_LAY_APPLY, OnApply)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



BOOL CLayerDlg::OnInitDialog()
{
	m_pTable = m_pView->Layers()->LevelTable();
	m_nSelect = m_pTable->layer;
	if (!m_nSelect)
		m_nSelect = 5;
	else if (m_nSelect < 6)
		m_nSelect--;
m_nSelect = 10 - m_nSelect;
	m_nCurrent = m_nSelect;
	UpdateFields();
	CMyDialog::OnInitDialog();
	m_list.ResetContent();
//	for (int i = 0; i < 11;i++)
	for (int i = 10; i >= 0;i--)
		UpdateList(i,1);
//	m_list.SetCurSel(10-m_nCurrent);
	m_list.SetCurSel(m_nCurrent);
	UpdateFields();
	return FALSE;
}


void CLayerDlg::UpdateList(int i, BOOL bAppend /* = 0 */)
{
	char buf[60];
	char * p = buf;
	*p++ = m_pTable->table[i].flags & 0x200 ? 'D': ' ';
	*p++ = 9;
	*p++ = m_pTable->table[i].flags & 0x100 ? 'A': ' ';
	*p++ = 9;
	*p++ = m_pTable->table[i].flags & 0x001 ? 'C': ' ';
	*p++ = 9;
	sprintf(p,"%5d",m_pTable->table[i].dx);
	p += 5;
	*p++ = 9;
	sprintf(p,"%5d",m_pTable->table[i].dy);
	p += 5;
	*p++ = 9;
	sprintf(p,"%5d %d",
			m_pTable->table[i].blur & 255,m_pTable->table[i].blur / 256);
	p += 7;
	*p++ = 9;

	strcpy(p, m_pTable->table[i].name);
	if (bAppend)
		m_list.AddString(buf);
	else
		{
		m_list.DeleteString(10-i);
		m_list.InsertString(10-i,buf);
		m_list.SetCurSel(10-i);
		}
}

void CLayerDlg::UpdateFields(BOOL bPut /* = 0 */)
{
	int i = 10 - m_nCurrent;
	if (bPut)
		{
//		UpdateData();
		int j = 10 - m_nSelect;
		if (j == 5)
			j = 0;
		else if (j < 5)
			j = j + 1;
		m_pTable->layer = j;
		m_pTable->table[i].blur = m_blur + 256 * m_nBlurType;
		m_pTable->table[i].dx = m_dx;
		m_pTable->table[i].dy = m_dy;
		UINT v = m_bWithin ? 1 : 0;
		if (m_bDisplay)
			v |= 0x200;
		if (m_bActive)
			v |= 0x100;
		m_pTable->table[i].flags = v;
		strncpy(m_pTable->table[i].name,(LPCSTR)m_name,19);
		m_pTable->table[i].name[19] = 0;
		UpdateList(i);
		}
	else
		{
		m_name = m_pTable->table[i].name;
		m_blur= m_pTable->table[i].blur & 255;
		m_nBlurType = m_pTable->table[i].blur / 256;
		m_dx = m_pTable->table[i].dx;
		m_dy = m_pTable->table[i].dy;
		m_bDisplay = m_pTable->table[i].flags & 0x200 ? 1 : 0;
		m_bActive =  m_pTable->table[i].flags & 0x100 ? 1 : 0;
		m_bWithin =  m_pTable->table[i].flags & 0x001 ? 1 : 0;
		UpdateData(0);
		}
}

void CLayerDlg::OnSelChange()
{
	int next = m_list.GetCurSel();
	UpdateData();
	UpdateFields(1);
	m_nCurrent = m_nSelect = next;
	m_list.SetCurSel(m_nCurrent);
	UpdateFields();
}

void CLayerDlg::OnApply()
{
	UpdateData();
	UpdateFields(1);
	m_pView->Layers()->LevelTable(1);
	m_pView->Update(0,0,20);
	m_pView->CheckUpdate();
}

void CLayerDlg::OnOK()
{
	DPF("got ok");
	if (!UpdateData(TRUE))
		return;
	UpdateFields(1);
	m_pView->Layers()->LevelTable(1);
	m_pView->Update(0,0,20);
	m_pView->CheckUpdate();
	CMyDialog::OnOK();
}



BEGIN_MESSAGE_MAP(CProgressDlg, CMyDialog)
	ON_BN_CLICKED(IDC_EXIT, OnExit)
	ON_COMMAND(ID_MY_PERFORM, OnPerform)
	ON_COMMAND(IDC_RESUME, OnResume)
	ON_COMMAND(ID_MY_EXIT, OnExit)
END_MESSAGE_MAP()

CProgressDlg::CProgressDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CProgressDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProgressDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
}

BOOL CProgressDlg::OnInitDialog()
{
	m_state = 0;
	m_error = 0;
	m_idx = 0;
	CMyDialog::OnInitDialog();

	if (m_kind == 0)
		{
		SetWindowText("Importing AVI");
		GetDlgItem(IDC_INDEX)->SetWindowText("Frame:");
		}
	// Initially create progress control in horizontal position
	CWnd* pWnd = GetDlgItem( IDC_PROGRESS );
	CRect rect;
	pWnd->GetWindowRect( &rect );
	ScreenToClient( &rect );
	m_Progress.Create( WS_VISIBLE | WS_CHILD | PBS_SMOOTH,
						rect, this, IDC_PROGRESS );
	m_Progress.SetRange( 0,100);
	GetDlgItem(IDC_RESUME)->EnableWindow(0);
	PostMessage(WM_COMMAND, ID_MY_PERFORM, 0);
	return 1;
}

void CProgressDlg::OnExit()
{
	DPF("got exit");
	if (m_state)
		m_bAbort = TRUE;
	else
		EndDialog(0);
}

void CProgressDlg::OnCancel()
{
	DPF("got cancel");
	if (m_state)
		m_bAbort = TRUE;
	else
		EndDialog(0);
}


void CProgressDlg::OnOK()
{
	DPF("got ok");
}


void CProgressDlg::OnClose()
{
	DPF("on close");
	if (m_state )
		m_bAbort = TRUE;
	else
	   CMyDialog::OnClose();
}


void CProgressDlg::CheckAbort()
{
	MSG msg;

	// Process existing messages in the application's message queue.
	// When the queue is empty, do clean up and return.
	while (::PeekMessage(&msg,NULL,0,0,PM_NOREMOVE) && !m_bAbort)
	{
		if (!AfxGetThread()->PumpMessage())
			return;
	}
}

void CProgressDlg::OnResume() 
{
	GetDlgItem(IDC_RESUME)->EnableWindow(0);
	PostMessage(WM_COMMAND, ID_MY_PERFORM, 0);
}

void CProgressDlg::OnPerform() 
{
	DPF("on perform");
	ShowWindow(SW_SHOW);
	m_bAbort = 0;
	m_state = 1;
	m_prevtime = m_starttime = GetTickCount() / 1000;
	GetDlgItem(IDC_RESUME)->EnableWindow(0);
	GetDlgItem(IDC_EXIT)->SetWindowText("Abort");
	if (m_kind)
		return;
	else
		{
#ifndef FLIPBOOK_MAC
		CSketchDoc * pDoc = (CSketchDoc *)m_pDoc;
		PGETFRAME	pget = (PGETFRAME)m_pData;
		for (; m_idx < m_count; m_idx++)
			{
DPF("doing:%d",m_idx);
			UpdateInfo();
			CheckAbort();
			if (m_bAbort)
				break;
			LPBITMAPINFOHEADER lpbi = (LPBITMAPINFOHEADER)
					AVIStreamGetFrame(pget, m_index1+m_idx);
			if (lpbi)
				{
				WORD z = pDoc->CreateCell(m_index2+m_idx,m_index3,lpbi,0,1,0);
				if (z)
					m_error = 2;
				}
			else
				m_error = 3;
			}
#endif
		}
	if (m_bAbort)
		GetDlgItem(IDC_RESUME)->EnableWindow( 1);
	GetDlgItem(IDC_EXIT)->SetWindowText("Exit");
	m_state = 0;
	if (!m_bAbort)
		PostMessage(WM_COMMAND, ID_MY_EXIT, 0);
}


void CProgressDlg::UpdateInfo()
{
	_int64 temp;
	if (m_idx)
		{
		m_prevtime = GetTickCount() / 1000;
		temp = m_prevtime - m_starttime;
		temp *= m_count - m_idx;
		temp /= m_idx;
		}
	else
		temp = 0;
	char buf[80];
	int h,m,s;
	h = (int)temp / 3600;
	temp = temp % 3600;
	m = (int)temp / 60;
	s = (int)temp % 60;
	if (h)
		sprintf(buf,"%d:%02d:%02d",h,m,s);
	else if (m)
		sprintf(buf,"%d:%02d",m,s);
	else
		sprintf(buf,"%d sec",s);
	SetDlgItemText(IDC_TIME, buf);
	SetDlgItemInt(IDC_COUNT, m_index2+m_idx);
	m_Progress.SetPos(MulDiv(100, m_idx, m_count));
	UpdateData(FALSE);
	UpdateWindow();
}



BEGIN_MESSAGE_MAP(CProgDlg, CMyDialog)
	ON_BN_CLICKED(IDC_EXIT, OnExit)
	ON_COMMAND(ID_MY_PERFORM, OnPerform)
	ON_COMMAND(IDC_RESUME, OnResume)
	ON_COMMAND(ID_MY_EXIT, OnExit)
END_MESSAGE_MAP()

CProgDlg::CProgDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CProgDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProgDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
}

BOOL CProgDlg::OnInitDialog()
{
	m_state = 0;
	m_error = 0;
	m_idx = 0;
	CMyDialog::OnInitDialog();

	if (m_kind == 2)
		{
		SetWindowText("Exporting GIF");
		GetDlgItem(IDC_INDEX)->SetWindowText("Frame:");
		}
	// Initially create progress control in horizontal position
	CWnd* pWnd = GetDlgItem( IDC_PROGRESS );
	CRect rect;
	pWnd->GetWindowRect( &rect );
	ScreenToClient( &rect );
	m_Progress.Create( WS_VISIBLE | WS_CHILD | PBS_SMOOTH,
						rect, this, IDC_PROGRESS );
	m_Progress.SetRange( 0,100);
	GetDlgItem(IDC_RESUME)->EnableWindow(0);
	PostMessage(WM_COMMAND, ID_MY_PERFORM, 0);
	CenterWindow();
	return 0;
}

void CProgDlg::OnExit()
{
	DPF("got exit");
	if (m_state)
		m_bAbort = TRUE;
	else
		EndDialog(0);
}

void CProgDlg::OnCancel()
{
	DPF("got cancel");
	if (m_state)
		m_bAbort = TRUE;
	else
		EndDialog(0);
}


void CProgDlg::OnOK()
{
	DPF("got ok");
}


void CProgDlg::OnClose()
{
	DPF("on close");
	if (m_state )
		m_bAbort = TRUE;
	else
	   CMyDialog::OnClose();
}


void CProgDlg::CheckAbort()
{
	MSG msg;

	// Process existing messages in the application's message queue.
	// When the queue is empty, do clean up and return.
	while (::PeekMessage(&msg,NULL,0,0,PM_NOREMOVE) && !m_bAbort)
	{
		if (!AfxGetThread()->PumpMessage())
			return;
	}
}

void CProgDlg::OnResume() 
{
	GetDlgItem(IDC_RESUME)->EnableWindow(0);
	PostMessage(WM_COMMAND, ID_MY_PERFORM, 0);
}

void CProgDlg::OnPerform() 
{
	DPF("on perform");
	ShowWindow(SW_SHOW);
	m_bAbort = 0;
	m_state = 1;
	m_prevtime = m_starttime = GetTickCount() / 1000;
	GetDlgItem(IDC_RESUME)->EnableWindow(0);
	GetDlgItem(IDC_EXIT)->SetWindowText("Abort");
	CSketchDoc * pDoc = (CSketchDoc *)m_pDoc;
	for (; m_idx < m_count; m_idx++)
		{
DPF("doing:%d",m_idx);
		UpdateInfo();
		CheckAbort();
		if (m_bAbort)
			break;
		m_error = m_pDoc->ProgressCallBack(m_kind, m_idx, (BYTE *)m_pData);
		}
	if (m_bAbort)
		GetDlgItem(IDC_RESUME)->EnableWindow( 1);
	GetDlgItem(IDC_EXIT)->SetWindowText("Exit");
	m_state = 0;
	if (!m_bAbort)
		PostMessage(WM_COMMAND, ID_MY_EXIT, 0);
}


void CProgDlg::UpdateInfo()
{
	_int64 temp;
	if (m_idx)
		{
		m_prevtime = GetTickCount() / 1000;
		temp = m_prevtime - m_starttime;
		temp *= m_count - m_idx;
		temp /= m_idx;
		}
	else
		temp = 0;
	char buf[80];
	int h,m,s;
	h = (int)temp / 3600;
	temp = temp % 3600;
	m = (int)temp / 60;
	s = (int)temp % 60;
	if (h)
		sprintf(buf,"%d:%02d:%02d",h,m,s);
	else if (m)
		sprintf(buf,"%d:%02d",m,s);
	else
		sprintf(buf,"%d sec",s);
	SetDlgItemText(IDC_TIME, buf);
	SetDlgItemInt(IDC_COUNT, m_base+m_idx);
	m_Progress.SetPos(MulDiv(100, m_idx, m_count));
	UpdateData(FALSE);
	UpdateWindow();
}



CProg2Dlg::CProg2Dlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CProg2Dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProg2Dlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
}


BEGIN_MESSAGE_MAP(CProg2Dlg, CMyDialog)
	ON_BN_CLICKED(IDC_EXIT, OnExit)
	ON_COMMAND(ID_MY_PERFORM, OnPerform)
	ON_COMMAND(IDC_RESUME, OnResume)
	ON_COMMAND(ID_MY_EXIT, OnExit)
END_MESSAGE_MAP()


int CProg2Dlg::ProgSetup(ProgressCallback inFunction,
					void* pUserObject, void * pUserData,
				unsigned int first, unsigned int count, int idstr)
{
	m_id_string = idstr;
	m_callback = inFunction;
	m_pUser = (BYTE *)pUserObject;
	m_pData = (BYTE *)pUserData;
	m_count = count;
	m_first = first;
	return 0;
}

BOOL CProg2Dlg::OnInitDialog()
{
	m_state = 0;
	m_error = 0;
	m_index = 0;
	CMyDialog::OnInitDialog();
	if (m_id_string)
		{
		CString buf;
		if (buf.LoadString(m_id_string))
			SetWindowText(buf);
		}
	// Initially create progress control in horizontal position
	CWnd* pWnd = GetDlgItem( IDC_PROGRESS );
	CRect rect;
	pWnd->GetWindowRect( &rect );
	ScreenToClient( &rect );
	m_Progress.Create( WS_VISIBLE | WS_CHILD | PBS_SMOOTH,
						rect, this, IDC_PROGRESS );
	m_Progress.SetRange( 0,100);
	GetDlgItem(IDC_RESUME)->EnableWindow(0);
	PostMessage(WM_COMMAND, ID_MY_PERFORM, 0);
	CenterWindow();
	return 0;
}

//void CProg2Dlg::Msg(LPCSTR txt)
//{
//GetDlgItem(IDC_STATUS)->SetWindowText(txt);
//}

void CProg2Dlg::Label(LPCSTR txt)
{
GetDlgItem(IDC_COUNT)->SetWindowText(txt);
}

void CProg2Dlg::OnExit()
{
	DPF("got exit");
	if (m_state)
		m_bAbort = TRUE;
	else
		EndDialog(0);
}

void CProg2Dlg::OnCancel()
{
	DPF("got cancel");
	if (m_state)
		m_bAbort = TRUE;
	else
		EndDialog(0);
}


void CProg2Dlg::OnOK()
{
	DPF("got ok");
}


void CProg2Dlg::OnClose()
{
	DPF("on close");
	if (m_state )
		m_bAbort = TRUE;
	else
	   CMyDialog::OnClose();
}


void CProg2Dlg::CheckAbort()
{
	MSG msg;

	// Process existing messages in the application's message queue.
	// When the queue is empty, do clean up and return.
	while (::PeekMessage(&msg,NULL,0,0,PM_NOREMOVE) && !m_bAbort)
	{
		if (!AfxGetThread()->PumpMessage())
			return;
	}
}

void CProg2Dlg::OnResume() 
{
	GetDlgItem(IDC_RESUME)->EnableWindow(0);
	PostMessage(WM_COMMAND, ID_MY_PERFORM, 0);
}

void CProg2Dlg::OnPerform() 
{
	DPF("on perform");
	ShowWindow(SW_SHOW);
	m_bAbort = 0;
	m_state = 1;
	m_prevtime = m_starttime = GetTickCount() / 1000;
	GetDlgItem(IDC_RESUME)->EnableWindow(0);
	GetDlgItem(IDC_EXIT)->SetWindowText("Abort");
	for (; m_index < m_count; m_index++)
		{
DPF("doing:%d",m_index);
		if ((GetTickCount() / 1000) !=  m_prevtime)
			{
			UpdateInfo();
			CheckAbort();
			if (m_bAbort)
				break;
			}
		if (m_error = m_callback(m_index, m_pUser,m_pData))
			{
			FormattedMsg(IDS_ERR_PROG2,m_error);
			break;
			}
		}
	GetDlgItem(IDC_EXIT)->SetWindowText("Exit");
	m_state = 0;
	if (m_bAbort)
		GetDlgItem(IDC_RESUME)->EnableWindow( 1);
	else
		PostMessage(WM_COMMAND, ID_MY_EXIT, 0);
}


void CProg2Dlg::UpdateInfo()
{
	_int64 temp;
	m_prevtime = GetTickCount() / 1000;
	if (m_index)
		{
		temp = m_prevtime - m_starttime;
		temp *= m_count - m_index;
		temp /= m_index;
		}
	else
		temp = 0;
	char buf[80];
	int h,m,s;
	h = (int)temp / 3600;
	temp = temp % 3600;
	m = (int)temp / 60;
	s = (int)temp % 60;
	if (h)
		sprintf(buf,"%d:%02d:%02d",h,m,s);
	else if (m)
		sprintf(buf,"%d:%02d",m,s);
	else
		sprintf(buf,"%d sec",s);
	SetDlgItemText(IDC_TIME, buf);
	SetDlgItemInt(IDC_COUNT, m_first+m_index);
	m_Progress.SetPos(MulDiv(100, m_index, m_count));
	UpdateData(FALSE);
	UpdateWindow();
}




CPalSelDlg::CPalSelDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CPalSelDlg::IDD, pParent)
{
	m_index = NEGONE;
}

void CPalSelDlg::DoDataExchange(CDataExchange* pDX)
{
	DPF("pal sel data xchg");
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPalSelDlg)
	DDX_Control(pDX, IDC_SEL_PAL, m_pals);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPalSelDlg, CMyDialog)
	ON_LBN_SELCHANGE(IDC_SEL_PAL, OnSelChangePal)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CPalSelDlg::FillListBox()
{
	m_pals.ResetContent();
	int j,i,c;
	c = NBR_PALS;
	j = 0;
	for (i = 0; i < c; i++)
		{
		CNewPals * pPal = m_pScene->PalettePtr(i);
		if (pPal)
			{
			if (m_index == NEGONE)
				m_index = i;
			int z = m_pals.AddString(pPal->GetPalName());
			if (i == m_index)
				j = z;
			m_pals.SetItemData(z,i); 
			}
		}
	m_index = m_pals.GetItemData(j);
	m_pals.SetCurSel(j);
}

BOOL CPalSelDlg::OnInitDialog()
{
	CenterWindow();
	CMyDialog::OnInitDialog();
	FillListBox();
	UpdateData(0);
	return FALSE;
}

void CPalSelDlg::OnSelChangePal()
{
	UINT i = m_pals.GetCurSel();
	m_index = m_pals.GetItemData(i);
}


CPalSharedDlg::CPalSharedDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CPalSharedDlg::IDD, pParent)
{
}

void CPalSharedDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPalSharedDlg)
	DDX_Text(pDX, IDC_PAL_NEW_NAME, m_name);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPalSharedDlg, CMyDialog)
	ON_COMMAND(IDC_PAL_MOD_ALL, OnModAll)
	ON_COMMAND(IDC_PAL_MOD_ONE, OnModOne)
END_MESSAGE_MAP()


BOOL CPalSharedDlg::OnInitDialog()
{
	CenterWindow();
	CMyDialog::OnInitDialog();
	UpdateData(0);
	DoMod(m_bModOne);
	return FALSE;
}

void CPalSharedDlg::DoMod(int v)
{
	m_bModOne = v;
	GetDlgItem(IDC_PAL_NEW_TEXT)->ShowWindow(m_bModOne ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_PAL_NEW_NAME)->ShowWindow(m_bModOne ? SW_SHOW : SW_HIDE);
	CheckRadioButton(IDC_PAL_MOD_ALL,IDC_PAL_MOD_ONE,IDC_PAL_MOD_ALL+m_bModOne);
}

CPalLinkedDlg::CPalLinkedDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CPalLinkedDlg::IDD, pParent)
{
	m_bBreak = 0;
}

BEGIN_MESSAGE_MAP(CPalLinkedDlg, CMyDialog)
	ON_COMMAND(IDC_PAL_EXT_NEW, OnBreak)
	ON_COMMAND(IDC_PAL_EXT_UPDATE, OnUpdate)
END_MESSAGE_MAP()


void CPalLinkedDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPalSharedDlg)
//	DDX_Text(pDX, IDC_PAL_EXT_FNAME, m_fname);
	DDX_Text(pDX, IDC_PAL_EXT_NAM, m_name);
	//}}AFX_DATA_MAP
}

#if !MAC
AFX_STATIC void AFXAPI _AfxAbbreviateName(__inout_z LPTSTR lpszCanon,
				int cchMax, BOOL bAtLeastName);
#endif

BOOL CPalLinkedDlg::OnInitDialog()
{
	CenterWindow();
	char name[300];
	strcpy(name,m_fname);
	FileName(m_name, m_fname);
	CMyDialog::OnInitDialog();
	_AfxAbbreviateName(name,60,1);
	GetDlgItem(IDC_PAL_EXT_FNAME)->SetWindowText(name);
	DoBreak(0);
	return FALSE;
}

void CPalLinkedDlg::DoBreak(int v)
{
	m_bBreak = v;
	GetDlgItem(IDC_PAL_EXT_NAME_TEXT)->ShowWindow(!m_bBreak ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDC_PAL_EXT_NAM)->ShowWindow(!m_bBreak ? SW_HIDE : SW_SHOW);
	CheckRadioButton(IDC_PAL_EXT_UPDATE,IDC_PAL_EXT_NEW,IDC_PAL_EXT_UPDATE+m_bBreak);
}

CPalModDlg::CPalModDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CPalModDlg::IDD, pParent)
{
}

BEGIN_MESSAGE_MAP(CPalModDlg, CMyDialog)
	ON_COMMAND(IDC_PAL_NO, OnNo)
END_MESSAGE_MAP()


void CPalModDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPalSharedDlg)
	DDX_Text(pDX, IDC_PAL_NAME, m_name);
	DDX_Text(pDX, IDC_PAL_FNAME, m_fname);
	//}}AFX_DATA_MAP
}


BOOL CPalModDlg::OnInitDialog()
{
	CenterWindow();
	CMyDialog::OnInitDialog();
	return FALSE;
}


void CPalModDlg::OnNo()
{
	EndDialog(99);
}

CPalUnusedDlg::CPalUnusedDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CPalUnusedDlg::IDD, pParent)
{
}

BEGIN_MESSAGE_MAP(CPalUnusedDlg, CMyDialog)
	ON_COMMAND(IDC_PAL_NO, OnNo)
END_MESSAGE_MAP()


void CPalUnusedDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPalSharedDlg)
	DDX_Text(pDX, IDC_PAL_NAME, m_name);
	//}}AFX_DATA_MAP
}


BOOL CPalUnusedDlg::OnInitDialog()
{
	CenterWindow();
	CMyDialog::OnInitDialog();
	return FALSE;
}


void CPalUnusedDlg::OnNo()
{
	EndDialog(99);
}

CPalMissingDlg::CPalMissingDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CPalMissingDlg::IDD, pParent)
{
	m_bBreak = 0;
}

BEGIN_MESSAGE_MAP(CPalMissingDlg, CMyDialog)
	ON_COMMAND(IDC_PAL_EXT_NO_LOCATE, OnBreak)
	ON_COMMAND(IDC_PAL_EXT_LOCATE, OnLocate)
END_MESSAGE_MAP()


void CPalMissingDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPalSharedDlg)
//	DDX_Text(pDX, IDC_PAL_EXT_FNAME, m_fname);
	DDX_Text(pDX, IDC_PAL_EXT_NAM, m_name);
	//}}AFX_DATA_MAP
}

#if !MAC
AFX_STATIC void AFXAPI _AfxAbbreviateName(__inout_z LPTSTR lpszCanon,
				int cchMax, BOOL bAtLeastName);
#endif

BOOL CPalMissingDlg::OnInitDialog()
{
	CenterWindow();
	char name[300];
	strcpy(name,m_fname);
	FileName(m_name, m_fname);
	CMyDialog::OnInitDialog();
	_AfxAbbreviateName(name,60,1);
	GetDlgItem(IDC_PAL_EXT_FNAME)->SetWindowText(name);
	DoBreak(0);
	return FALSE;
}

void CPalMissingDlg::DoBreak(int v)
{
	m_bBreak = v;
	GetDlgItem(IDC_PAL_EXT_NAME_TEXT)->ShowWindow(!m_bBreak ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDC_PAL_EXT_NAM)->ShowWindow(!m_bBreak ? SW_HIDE : SW_SHOW);
	CheckRadioButton(IDC_PAL_EXT_LOCATE,
					IDC_PAL_EXT_NO_LOCATE,IDC_PAL_EXT_LOCATE+m_bBreak);
}

CPalDifferentDlg::CPalDifferentDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CPalDifferentDlg::IDD, pParent)
{
	m_bBreak = 0;
}

BEGIN_MESSAGE_MAP(CPalDifferentDlg, CMyDialog)
	ON_COMMAND(IDC_PAL_EXT_NEW, OnBreak)
	ON_COMMAND(IDC_PAL_EXT_UPDATE, OnUpdate)
END_MESSAGE_MAP()


void CPalDifferentDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPalSharedDlg)
//	DDX_Text(pDX, IDC_PAL_EXT_FNAME, m_fname);
	DDX_Text(pDX, IDC_PAL_EXT_NAM, m_name);
	//}}AFX_DATA_MAP
}

#if !MAC
AFX_STATIC void AFXAPI _AfxAbbreviateName(__inout_z LPTSTR lpszCanon,
				int cchMax, BOOL bAtLeastName);
#endif

BOOL CPalDifferentDlg::OnInitDialog()
{
	CenterWindow();
	char name[300];
	strcpy(name,m_fname);
	FileName(m_name, m_fname);
	CMyDialog::OnInitDialog();
	_AfxAbbreviateName(name,60,1);
	GetDlgItem(IDC_PAL_EXT_FNAME)->SetWindowText(name);
	DoBreak(0);
	return FALSE;
}

void CPalDifferentDlg::DoBreak(int v)
{
	m_bBreak = v;
	GetDlgItem(IDC_PAL_EXT_NAME_TEXT)->ShowWindow(!m_bBreak ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDC_PAL_EXT_NAM)->ShowWindow(!m_bBreak ? SW_HIDE : SW_SHOW);
	CheckRadioButton(IDC_PAL_EXT_UPDATE,IDC_PAL_EXT_NEW,IDC_PAL_EXT_UPDATE+m_bBreak);
}

CMissingTextureDlg::CMissingTextureDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CMissingTextureDlg::IDD, pParent)
{
}

BEGIN_MESSAGE_MAP(CMissingTextureDlg, CMyDialog)
	ON_COMMAND(IDC_TEXTURE_LOCATE, OnLocate)
END_MESSAGE_MAP()


void CMissingTextureDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_MISSING_TEXTURE, m_fname);
	//}}AFX_DATA_MAP
}


BOOL CMissingTextureDlg::OnInitDialog()
{
	CenterWindow();
	CMyDialog::OnInitDialog();
	return FALSE;
}


void CMissingTextureDlg::OnLocate()
{
	EndDialog(99);
}

CProgPullDlg::CProgPullDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProgPullDlg::IDD, pParent)
{
	m_factor = 100;
}


BEGIN_MESSAGE_MAP(CProgPullDlg, CDialog)
	ON_COMMAND(ID_MY_PERFORM, OnPerform)
	ON_COMMAND(IDC_EXIT, OnExit)
END_MESSAGE_MAP()

void CProgPullDlg::OnExit()
{
	DPF("got exit");
	if (m_state)
		m_bAbort = TRUE;
	else
		EndDialog(0);
}

void CProgPullDlg::OnCancel()
{
	DPF("got cancel");
	if (m_state)
		m_bAbort = TRUE;
	else
		EndDialog(0);
}

void CProgPullDlg::OnClose()
{
	DPF("on close");
	if (m_state )
		m_bAbort = TRUE;
	else
	   CDialog::OnClose();
}


BOOL CProgPullDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	GetDlgItem(IDC_INDEX)->SetWindowText("Frame:");
	SetWindowText("Decompressing Video Frames");
	CWnd* pWnd = GetDlgItem( IDC_PROGRESS );
	CRect rect;
	pWnd->GetWindowRect( &rect );
	ScreenToClient( &rect );
	m_Progress.Create( WS_VISIBLE | WS_CHILD | PBS_SMOOTH,
						rect, this, IDC_PROGRESS );
	m_Progress.SetRange( 0,100);
	GetDlgItem(IDC_RESUME)->EnableWindow(0);
//	GetDlgItem(IDCANCEL)->SetWindowText("Exit");
	PostMessage(WM_COMMAND, ID_MY_PERFORM, 0);
	CenterWindow();
	return 0;
}


void CProgPullDlg::OnPerform() 
{
	DPF("on perform");
	ShowWindow(SW_SHOW);
	m_bAbort = 0;
	m_starttime = GetTickCount() / 1000;
	m_prevtime = 0;//GetTickCount() / m_factor;
	m_state = 1;
	m_pCallback(0, m_pClass);
	int percent;
	for (; ;)
		{
//		DPF("doing:%d",m_index);
		if ((GetTickCount() / m_factor) !=  m_prevtime)
			{
			percent = m_pCallback(2, m_pClass);
			if (percent < 0)
				break;
			char buf[80];
			if ((UINT)percent > m_first)
			{
			_int64 temp;
			UINT time = GetTickCount() / 1000;
			temp = time - m_starttime;
			temp *= m_last + 1 - percent;
			temp /= (percent - m_first);
			int h,m,s;
			h = (int)temp / 3600;
			temp = temp % 3600;
			m = (int)temp / 60;
			s = (int)temp % 60;
			if (h)
				sprintf_s(buf,"%d:%02d:%02d",h,m,s);
			else if (m)
				sprintf_s(buf,"%d:%02d",m,s);
			else
				sprintf_s(buf,"%d sec",s);
			}
			else
				buf[0] = 0;
			SetDlgItemText(IDC_TIME, buf);
			SetDlgItemInt(IDC_COUNT, percent+1);
			UpdateData(FALSE);
		//	UpdateWindow();
			percent = 100 * (percent - m_first) / (m_last + 1 - m_first);
			m_Progress.SetPos(percent);
			MSG msg;
		// Process existing messages in the application's message queue.
		// When the queue is empty, do clean up and return.
			while (::PeekMessage(&msg,NULL,0,0,PM_NOREMOVE) && !m_bAbort)
				{
				if (!AfxGetThread()->PumpMessage())
					return;
				}
			if (m_bAbort)
				break;
			}
		}
	m_pCallback(1, m_pClass);
	m_state = 0;
//	PostMessage(WM_COMMAND, ID_MY_EXIT, 0);
	EndDialog(m_bAbort ? IDCANCEL : IDOK);
}

CEmbOptionsDlg::CEmbOptionsDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CEmbOptionsDlg::IDD, pParent)
{
}

BOOL CEmbOptionsDlg::OnInitDialog()
{
	CMyDialog::OnInitDialog();
	GetDlgItem(IDC_EMB_OVERRIDE)->ShowWindow(m_bOverride ? SW_SHOW : SW_HIDE);
	return FALSE;
}

void CEmbOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_EMB_IN_NEVER, m_in);
	DDX_Radio(pDX, IDC_EMB_OUT_NEVER, m_out);
}

CExpEmbedDlg::CExpEmbedDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CExpEmbedDlg::IDD, pParent)
{
}

void CExpEmbedDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EMBED_LIST, m_list);
}

BEGIN_MESSAGE_MAP(CExpEmbedDlg, CMyDialog)
//	ON_LBN_SELCHANGE(IDC_EMBED_LIST, OnSelChange)
	ON_COMMAND(IDC_EXP_EMBED, OnExport)
END_MESSAGE_MAP()

BOOL CExpEmbedDlg::OnInitDialog()
{
	CMyDialog::OnInitDialog();
	CSize   sz;
	CDC*    pDC = m_list.GetDC();
	sz = pDC->GetTextExtent("Texture   ");
 	int dx = sz.cx;
	m_list.ReleaseDC(pDC);
#if !MAC
	m_list.SetTabStops((dx*4/3 * 4) / LOWORD(::GetDialogBaseUnits()));
#endif

	m_list.ResetContent();

	UINT index;
	for (index = 0;;index++)
		{
		char buf[400];
		char name[300];
		if (!m_pIO->EmbName(index,name))
			break;
		UINT kind = m_pIO->EmbKind(index);
		char szKind[20];
		if (kind  == EMB_KIND_TEXTURE)
			strcpy(szKind,"Texture");
		else if (kind  == EMB_KIND_MODEL)
			strcpy(szKind,"Model");
		else if (kind  == EMB_KIND_PALETTE)
			strcpy(szKind,"Palette");
		else if (kind  == EMB_KIND_SOUND)
			strcpy(szKind,"Sound");
		else
			strcpy(szKind,"Unknown");
		char stat[20];
		if (m_pIO->EmbFlag(index,EMB_ENABLED))
			strcpy(stat,"Enabled");
		else
			strcpy(stat,"Disabled");
		sprintf(buf,"%s%c%s%c%s",szKind,9,stat,9,name);
		int k = m_list.AddString(buf);
//		m_list.SetItemData(k,q);
		}
	m_list.SetCurSel(0);
//	OnSelChange();
	return TRUE;
}

void CExpEmbedDlg::OnExport()
{
	int z = m_list.GetCurSel();
	char buf[400];
	m_list.GetText(z,buf);
	int i,j,k;
	for (i = 0, j= 0, k = 0; buf[i];i++)
		{
		if (!j && (buf[i] == 9)) j = i + 1; // first tab
		if (buf[i] == '.') k = i + 1;      // last period
		}
	char name[300];
	strcpy(name, buf+j);
	BOOL bType = FALSE;
	CFileDialog dlgFile(bType);
	CString title;
	VERIFY(title.LoadString(IDS_EXPORT_EMBED));
	CString strFilter;
	CString strDefault;
	strFilter = "All Files (*.*)";
	strFilter += char(0);
	strFilter += "*.*";
	dlgFile.m_ofn.lpstrFilter = strFilter;
	dlgFile.m_ofn.hwndOwner = AfxGetMainWnd()->GetSafeHwnd();
//	dlgFile.m_ofn.lpstrTitle = title;
	dlgFile.m_ofn.lpstrFile = name;
	dlgFile.m_ofn.lpstrDefExt = name+k;
	dlgFile.m_ofn.nMaxCustFilter++;
	BOOL bRet = dlgFile.DoModal() == IDOK ? TRUE : FALSE;
//fileName.ReleaseBuffer();
	if (bRet)
		{
		UINT index = z;
		CFile file;
		if (file.Open(name, CFile::modeCreate | CFile::modeWrite))
			{
			UINT size = m_pIO->EmbData(index, 0); // just the size
			BYTE * pData = new BYTE[size];
			m_pIO->EmbData(index, pData);
			file.Write(pData, size);
			delete [] pData;
			file.Close();
			}
		}
}

CEmbedInDlg::CEmbedInDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CEmbedInDlg::IDD, pParent)
{
}

BOOL CEmbedInDlg::OnInitDialog()
{
	CMyDialog::OnInitDialog();
	m_list.ResetContent();
	UINT index;
	for (index = 0;;index++)
		{
		char name[300];
		if (!m_pIO->EmbName(index,name))
			break;
		DWORD mode = CFile::modeRead;
		CFile xfile;
		if (!xfile.Open(name, mode))		// check for local copy
			continue;
		xfile.Close();
		int k = m_list.AddString(name);
		m_list.SetItemData(k,index);
		m_list.SetCheck(k,1);
		}
	return FALSE;
}

void CEmbedInDlg::OnOK()
{
	int c = m_list.GetCount();
	int i;
	for (i = 0; i < c; i++)
		{
		if (!m_list.GetCheck(i))
			{
			int j = m_list.GetItemData(i);
			m_pIO->EmbFlag(j,EMB_ENABLED,2); // clear enabled flag
			}
		}
	EndDialog(IDOK);
}

void CEmbedInDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EMBED_LIST, m_list);
}

CEmbedOutDlg::CEmbedOutDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CEmbedOutDlg::IDD, pParent)
{
}

void CEmbedOutDlg::AddName(LPCSTR name, UINT kind, UINT index)
{
	char info[20];
	char buf[400];
	BOOL bCheck = 1;
	if (index == NEGONE)
		{
		strcpy(info,"External");
		index = 10000 + kind; // major kludge
		}
	else if (m_pIO->EmbFlag(index,EMB_ENABLED))
		strcpy(info,"Enabled");
	else if (!m_pIO->EmbFlag(index,EMB_SEEN))
		{
		strcpy(info,"UnUsed");
		bCheck = 0;
		}
	else
		{
		strcpy(info,"Disabled");
		bCheck = 0;
		}
	sprintf(buf,"%s | %s",info,name);
	int k = m_list.AddString(buf);
	m_list.SetItemData(k,index);
	m_list.SetCheck(k,bCheck);
}

void CEmbedOutDlg::CheckExtFile(LPCSTR name, UINT kind)
{
	UINT idx = m_pIO->EmbFind(name, kind);
	AddName(name,kind,idx);
}

BOOL CEmbedOutDlg::OnInitDialog()
{
	CMyDialog::OnInitDialog();
	m_list.ResetContent();
	int idd;
	char name[300];
	for (idd = 0; idd < 3; idd++)
		{
		m_pScene->SceneOptionStr(SCOPT_WAVE0+idd,name);
		if (name[0])
			CheckExtFile(name,EMB_KIND_SOUND);
		}
	UINT level, levels;
	levels = m_pScene->LevelCount();
	for (level = 0; level < levels; level++)
		{
		m_pScene->LevelModelName(name, level);
		if (name[0])
			CheckExtFile(name,EMB_KIND_MODEL);
		}
	UINT index;
	for (index = 0;;index++)
		{
		char name[300];
		if (!m_pIO->EmbName(index,name))
			break;
		UINT kind = m_pIO->EmbKind(index);
		if (!m_pIO->EmbFlag(index,EMB_SEEN))
			AddName(name,kind,index);
		}
	return FALSE;
}

void CEmbedOutDlg::OnOK()
{
	int c = m_list.GetCount();
	int i;
	for (i = 0; i < c; i++)
		{
		UINT index = m_list.GetItemData(i);
		if (m_list.GetCheck(i))
			{
			if (index >= 10000)
				{
				char name[300];
				m_list.GetText(i, name);
				int j;
				for (j = 0; name[j] && (name[j] != '|');j++);
				if (name[j] && name[j+1])
					j += 2;
				index = m_pIO->EmbAdd(name+j,index - 10000);
				}
			m_pIO->EmbFlag(index,EMB_DO_SAVE,1);
			}
		}
	EndDialog(IDOK);
}

void CEmbedOutDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EMBED_LIST, m_list);
}

CFrameNumberDlg::CFrameNumberDlg(CWnd* pParent)
	: CMyDialog(CFrameNumberDlg::IDD, pParent)
{
}

void CFrameNumberDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_PUBLISH, m_bPublish);
	DDX_Check(pDX, IDC_PLAYBACK, m_bPlayback);
	DDX_Radio(pDX, IDC_UL, m_corner);
	DDX_Radio(pDX, IDC_START_AT_0, m_start);
}

