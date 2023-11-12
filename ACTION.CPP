#include "stdafx.h"
#include "mydialog.h"
#include "action.h"
#include "clayers.h"
#include "cscene.h"
#include "mainfrm.h"
#include "clevtbl.h"
#include "cnewpals.h"
CActionFillDlg::CActionFillDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CActionFillDlg::IDD, pParent)
{
	m_StartFrame = 1;
	m_EndFrame = 1;
	m_Level = 0;
	m_MaxFrame = 1;
	m_MaxLevel = 0;
}


void CActionFillDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_START_FRAME, m_StartFrame);
	DDV_MinMaxUInt(pDX, m_StartFrame, 1, m_MaxFrame);
	DDX_Text(pDX, IDC_END_FRAME, m_EndFrame);
	DDV_MinMaxUInt(pDX, m_EndFrame, m_StartFrame, m_MaxFrame);
	DDX_Text(pDX, IDC_LEVEL, m_Level);
	DDV_MinMaxUInt(pDX, m_Level, 0, m_MaxLevel);
}

BEGIN_MESSAGE_MAP(CActionFillDlg, CMyDialog)
	ON_BN_CLICKED(IDC_NW, UpdateControls)
	ON_BN_CLICKED(IDC_SW, UpdateControls)
	ON_BN_CLICKED(IDC_NE, UpdateControls)
	ON_BN_CLICKED(IDC_SE, UpdateControls)
END_MESSAGE_MAP()

BOOL CActionFillDlg::OnInitDialog()
{
	CMyDialog::OnInitDialog();
	CWnd* pWnd = GetDlgItem( IDC_PROGRESS );
	CRect rect;
	pWnd->GetWindowRect( &rect );
	DPR ("progress bar - get window rect", &rect);
	ScreenToClient( &rect );
	DPR ("progress bar - screen to client", &rect);
	m_Progress.Create( WS_VISIBLE | WS_CHILD | PBS_SMOOTH,
//	m_Progress.Create( WS_CHILD | PBS_SMOOTH,
						rect, this, IDC_PROGRESS_R );
	m_Progress.SetRange( 0,100);
	m_state = 0;
	m_mask = 0;
	UpdateControls();
	m_OrigFrame = m_StartFrame;
	CenterWindow();
	return FALSE;
}


void CActionFillDlg::UpdateControls()
{
	m_mask = 0;
	if (IsDlgButtonChecked(IDC_NW))
		m_mask |= 1;
	if (IsDlgButtonChecked(IDC_SW))
		m_mask |= 2;
	if (IsDlgButtonChecked(IDC_NE))
		m_mask |= 4;
	if (IsDlgButtonChecked(IDC_SE))
		m_mask |= 8;
	
	GetDlgItem(IDOK)->EnableWindow(m_mask != 0);
	
}

void CActionFillDlg::OnCancel()
{
	DPF("got cancel");
	if (m_state)
		m_state = 2;
	else
		EndDialog(0);
}

void CActionFillDlg::CheckAbort()
{
	MSG msg;

	// Process existing messages in the application's message queue.
	// When the queue is empty, do clean up and return.
	while (::PeekMessage(&msg,NULL,0,0,PM_NOREMOVE) && (m_state == 1))
	{
		if (!AfxGetThread()->PumpMessage())
			return;
	}
}


void CActionFillDlg::OnOK()
{
	DPF("got ok");
	if (m_state || !UpdateData(TRUE))
		return;
	GetDlgItem(IDC_PROGRESS_R)->ShowWindow(SW_SHOW);
	GetDlgItem(IDOK)->EnableWindow(FALSE);
	GetDlgItem(IDCANCEL)->SetWindowText("Abort");
	CLayers * pLayers = new CLayers;
	pLayers->Setup(m_pScene,0);
//	pLayers->InitPalette();
	pLayers->SelectLayer(m_pTable->layer);
	
	m_state = 1;
	UINT	frame;
	UINT keycount;
	DWORD * keys = new DWORD[m_EndFrame + 1 - m_StartFrame];
	keycount = 0;
	for (frame = m_StartFrame; (m_state == 1) && (frame <= m_EndFrame); frame++)
		{
		UINT level = m_Level;
//		keys[keycount] = m_pScene->GetCellKey(frame-1,level);
		DWORD key;
		m_pScene->GetImageKey(key,frame-1,level,98);
		if (!key)
			continue;
		keys[keycount] = key;
		UINT i;
		for (i = 0; keys[i] != keys[keycount];i++);
		if (i < keycount)
			{
DPF("skipping frm:%d,lvl:%d",frame-1,level);
			continue;
			}
		keycount++;
		pLayers->Select(frame-1, level);
		pLayers->Flood(m_color,m_pTable->layer ? 2 : 0);
		UINT fkind = 0;
		if (m_mask & 1)
			pLayers->Fill(0,0,0,TRUE,fkind);
//			pLayers->Fill(0,pLayers->Height() / 2,0,TRUE,fkind);
		if (m_mask & 2)
			pLayers->Fill(0,pLayers->Height()-1,0,TRUE,fkind);
		if (m_mask & 4)
			pLayers->Fill(pLayers->Width()-1,0,0,TRUE,fkind);
		if (m_mask & 8)
			pLayers->Fill(pLayers->Width()-1,pLayers->Height()-1,0,TRUE,fkind);
		if (m_pTable->layer)
			pLayers->Flood(m_color,3);
		if (pLayers->Put(1))
			m_pFrame->UpdateCell(frame-1,level);
		m_Progress.SetPos(MulDiv(100, frame-m_StartFrame,
				m_EndFrame +1 - m_StartFrame));
		CheckAbort();
		}
	delete [] keys;
	delete pLayers;
	m_state = 0;
	if (frame <= m_EndFrame)
		m_StartFrame = frame;
	else
		m_StartFrame = m_OrigFrame;
	UpdateData(0);
	GetDlgItem(IDOK)->EnableWindow(TRUE);
	GetDlgItem(IDCANCEL)->SetWindowText("Done");
	GetDlgItem(IDC_PROGRESS_R)->ShowWindow(SW_HIDE);
}

CActionFlipDlg::CActionFlipDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CActionFlipDlg::IDD, pParent)
{
	m_StartFrame = 1;
	m_EndFrame = 1;
	m_StartLevel = 0;
	m_EndLevel = 0;
	m_MaxFrame = 1;
	m_MaxLevel = 0;
}


void CActionFlipDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_START_FRAME, m_StartFrame);
	DDV_MinMaxUInt(pDX, m_StartFrame, 1, m_MaxFrame);
	DDX_Text(pDX, IDC_END_FRAME, m_EndFrame);
	DDV_MinMaxUInt(pDX, m_EndFrame, m_StartFrame, m_MaxFrame);
	DDX_Text(pDX, IDC_START_LEVEL, m_StartLevel);
	DDV_MinMaxUInt(pDX, m_StartLevel, 0, m_MaxLevel);
	DDX_Text(pDX, IDC_END_LEVEL, m_EndLevel);
	DDV_MinMaxUInt(pDX, m_EndLevel, m_StartLevel, m_MaxLevel);
}

BEGIN_MESSAGE_MAP(CActionFlipDlg, CMyDialog)
	ON_BN_CLICKED(IDC_HFLIP, UpdateControls)
	ON_BN_CLICKED(IDC_VFLIP, UpdateControls)
END_MESSAGE_MAP()

BOOL CActionFlipDlg::OnInitDialog()
{
	CMyDialog::OnInitDialog();
	CWnd* pWnd = GetDlgItem( IDC_PROGRESS );
	CRect rect;
	pWnd->GetWindowRect( &rect );
	ScreenToClient( &rect );
	m_Progress.Create( WS_VISIBLE | WS_CHILD | PBS_SMOOTH,
//	m_Progress.Create( WS_CHILD | PBS_SMOOTH,
						rect, this, IDC_PROGRESS_R );
	m_Progress.SetRange( 0,100);
	m_state = 0;
	m_mask = 0;
	UpdateControls();
	m_OrigFrame = m_StartFrame;
	CenterWindow();
	return FALSE;
}


void CActionFlipDlg::UpdateControls()
{
	m_mask = 0;
	if (IsDlgButtonChecked(IDC_HFLIP))
		m_mask |= 1;
	if (IsDlgButtonChecked(IDC_VFLIP))
		m_mask |= 2;
	GetDlgItem(IDOK)->EnableWindow(m_mask != 0);
	
}

void CActionFlipDlg::OnCancel()
{
	DPF("got cancel");
	if (m_state)
		m_state = 2;
	else
		EndDialog(0);
}

void CActionFlipDlg::CheckAbort()
{
	MSG msg;

	// Process existing messages in the application's message queue.
	// When the queue is empty, do clean up and return.
	while (::PeekMessage(&msg,NULL,0,0,PM_NOREMOVE) && (m_state == 1))
	{
		if (!AfxGetThread()->PumpMessage())
			return;
	}
}


void CActionFlipDlg::OnOK()
{
	DPF("got ok");
	if (m_state || !UpdateData(TRUE))
		return;
	GetDlgItem(IDC_PROGRESS_R)->ShowWindow(SW_SHOW);
	GetDlgItem(IDOK)->EnableWindow(FALSE);
	GetDlgItem(IDCANCEL)->SetWindowText("Abort");
	CLayers * pLayers = new CLayers;
	pLayers->Setup(m_pScene,0);
	
	m_state = 1;
	UINT	frame;
	UINT keycount;
	keycount = (m_EndFrame+1-m_StartFrame) * (m_EndLevel+1-m_StartLevel);
	DWORD * keys = new DWORD[keycount];
	keycount = 0;
	for (frame = m_StartFrame; (m_state == 1) && (frame <= m_EndFrame); frame++)
		{
		UINT level;
		for (level = m_StartLevel; level <= m_EndLevel; level++)
			{
//			keys[keycount] = m_pScene->GetCellKey(frame-1,level);
			DWORD key;
			m_pScene->GetImageKey(key,frame-1,level,98);
			if (!key)
				continue;
			keys[keycount] = key;
			UINT i;
			for (i = 0; keys[i] != keys[keycount];i++);
			if (i < keycount)
				{
DPF("skipping frm:%d,lvl:%d",frame-1,level);
				continue;
				}
			keycount++;
			pLayers->Select(frame-1, level);
			pLayers->Flipper(m_mask);
			if (pLayers->Put(1))
				m_pFrame->UpdateCell(frame-1,level);
			}
		m_Progress.SetPos(MulDiv(100, frame-m_StartFrame,
				m_EndFrame +1 - m_StartFrame));
		CheckAbort();
		}
	delete [] keys;
	delete pLayers;
	m_state = 0;
	if (frame <= m_EndFrame)
		m_StartFrame = frame;
	else
		m_StartFrame = m_OrigFrame;
	UpdateData(0);
	GetDlgItem(IDOK)->EnableWindow(TRUE);
	GetDlgItem(IDCANCEL)->SetWindowText("Done");
	GetDlgItem(IDC_PROGRESS_R)->ShowWindow(SW_HIDE);
}


CActionMatteDlg::CActionMatteDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CActionMatteDlg::IDD, pParent)
{
	m_StartFrame = 1;
	m_EndFrame = 1;
	m_Level = 0;
	m_MaxFrame = 1;
	m_MaxLevel = 0;
	m_blur = 0;
	m_offx = 0;
	m_offy = 0;
	m_color = 0;
//	m_kind = 0;
}


void CActionMatteDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_START_FRAME, m_StartFrame);
	DDV_MinMaxUInt(pDX, m_StartFrame, 1, m_MaxFrame);
	DDX_Text(pDX, IDC_END_FRAME, m_EndFrame);
	DDV_MinMaxUInt(pDX, m_EndFrame, m_StartFrame, m_MaxFrame);
	DDX_Text(pDX, IDC_LEVEL, m_Level);
	DDV_MinMaxUInt(pDX, m_Level, 0, m_MaxLevel);
	DDX_Text(pDX, IDC_BLUR, m_blur);
	DDX_Text(pDX, IDC_OFFX, m_offx);
	DDX_Text(pDX, IDC_OFFY, m_offy);
	DDX_Check(pDX, IDC_OUTSIDE, m_bOutSide);
//	DDV_MinMaxUInt(pDX, m_blur, 0, 15);
//	DDV_MinMaxInt(pDX, m_offx, -50, 50);
//	DDV_MinMaxInt(pDX, m_offy, -50, 50);
}

BEGIN_MESSAGE_MAP(CActionMatteDlg, CMyDialog)
//	ON_BN_CLICKED(IDC_COLOR, ChangeColor)
	ON_WM_DRAWITEM()
END_MESSAGE_MAP()

BOOL CActionMatteDlg::OnInitDialog()
{
	m_layer = m_pTable->layer;
	if (!m_layer)
		m_layer = 5;
	else if (m_layer < 6)
		m_layer--;
	m_pPals = m_pScene->LevelPalette(m_Level);
	m_blur = m_pTable->table[m_layer].blur;
	m_offx = m_pTable->table[m_layer].dx;
	m_offy = m_pTable->table[m_layer].dy;
	CMyDialog::OnInitDialog();
#ifdef FLIPBOOK_MAC
	// make sure color box is properly setup and can draw
	GetDlgItem (IDC_COLOR);
#endif
	CWnd* pWnd = GetDlgItem( IDC_PROGRESS );
	CRect rect;
	pWnd->GetWindowRect( &rect );
	ScreenToClient( &rect );
	m_Progress.Create( WS_VISIBLE | WS_CHILD | PBS_SMOOTH,
//	m_Progress.Create( WS_CHILD | PBS_SMOOTH,
						rect, this, IDC_PROGRESS_R );
	m_Progress.SetRange( 0,100);
	m_state = 0;
	m_OrigFrame = m_StartFrame;

	CenterWindow();
	return FALSE;
}

void CActionMatteDlg::OnDrawItem(int nId, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (lpDrawItemStruct->itemAction != ODA_DRAWENTIRE)	
		return;
	DPF("DrawItem:%d",nId);
	if (nId != IDC_COLOR) return;
	CWnd * pWnd = GetDlgItem(nId);
	if (!pWnd) return;
//	CDC* pDC = pWnd->GetDC();
//	ASSERT(pDC != NULL);
	CWindowDC appDC(pWnd);
	CRect rect;
	pWnd->GetClientRect(rect);
	BYTE pals[4];
	m_pPals->Color(pals,m_color);
	appDC.FillSolidRect(&rect,RGB(pals[0],pals[1],pals[2]));
}

/*
void CActionMatteDlg::ChangeColor()
{
	DPF("on change color");
}
*/
void CActionMatteDlg::OnCancel()
{
	DPF("got cancel");
	if (m_state)
		m_state = 2;
	else
		EndDialog(0);
}

void CActionMatteDlg::CheckAbort()
{
	MSG msg;

	// Process existing messages in the application's message queue.
	// When the queue is empty, do clean up and return.
	while (::PeekMessage(&msg,NULL,0,0,PM_NOREMOVE) && (m_state == 1))
	{
		if (!AfxGetThread()->PumpMessage())
			return;
	}
}


void CActionMatteDlg::OnOK()
{
	DPF("got ok");
	if (m_state || !UpdateData(TRUE))
		return;
	GetDlgItem(IDC_PROGRESS_R)->ShowWindow(SW_SHOW);
	GetDlgItem(IDOK)->EnableWindow(FALSE);
	GetDlgItem(IDCANCEL)->SetWindowText("Abort");
	CLayers * pLayers = new CLayers;
	pLayers->Setup(m_pScene,0);
//	pLayers->InitPalette();
//	pLayers->LevelTable()->layer = m_pTable->layer;
	m_state = 1;
	UINT	frame;
	UINT keycount;
	DWORD * keys = new DWORD[m_EndFrame + 1 - m_StartFrame];
	keycount = 0;
	int code = 1;
	if (m_bOutSide)
		code |= 4;
	for (frame = m_StartFrame; (m_state == 1) && (frame <= m_EndFrame); frame++)
		{
		UINT level = m_Level;
//		keys[keycount] = m_pScene->GetCellKey(frame-1,level);
		DWORD key;
		m_pScene->GetImageKey(key,frame-1,level,98);
		if (!key)
			continue;
		keys[keycount] = key;
		UINT i;
		for (i = 0; keys[i] != keys[keycount];i++);
		if (i < keycount)
			{
DPF("skipping frm:%d,lvl:%d",frame-1,level);
			continue;
			}
		keycount++;
		pLayers->Select(frame-1, level);

		pLayers->Flood(m_color,code);
		if (pLayers->Put(1))
			m_pFrame->UpdateCell(frame-1,level);
		m_Progress.SetPos(MulDiv(100, frame-m_StartFrame,
				m_EndFrame +1 - m_StartFrame));
		CheckAbort();
		}
	delete [] keys;
	delete pLayers;
	m_state = 0;
	if (frame <= m_EndFrame)
		m_StartFrame = frame;
	else
		m_StartFrame = m_OrigFrame;
	UpdateData(0);
	GetDlgItem(IDOK)->EnableWindow(TRUE);
	GetDlgItem(IDCANCEL)->SetWindowText("Done");
	GetDlgItem(IDC_PROGRESS_R)->ShowWindow(SW_HIDE);
}

CActionSpeckleDlg::CActionSpeckleDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CActionSpeckleDlg::IDD, pParent)
{
	m_StartFrame = 1;
	m_EndFrame = 1;
	m_StartLevel = 0;
	m_EndLevel = 0;
	m_MaxFrame = 1;
	m_MaxLevel = 0;
}


void CActionSpeckleDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_START_FRAME, m_StartFrame);
	DDV_MinMaxUInt(pDX, m_StartFrame, 1, m_MaxFrame);
	DDX_Text(pDX, IDC_END_FRAME, m_EndFrame);
	DDV_MinMaxUInt(pDX, m_EndFrame, m_StartFrame, m_MaxFrame);
	DDX_Text(pDX, IDC_START_LEVEL, m_StartLevel);
	DDV_MinMaxUInt(pDX, m_StartLevel, 0, m_MaxLevel);
	DDX_Text(pDX, IDC_END_LEVEL, m_EndLevel);
	DDV_MinMaxUInt(pDX, m_EndLevel, m_StartLevel, m_MaxLevel);
	DDX_Radio(pDX, IDC_SPECK1, m_Kind);
}

BOOL CActionSpeckleDlg::OnInitDialog()
{
	CMyDialog::OnInitDialog();
	CWnd* pWnd = GetDlgItem( IDC_PROGRESS );
	CRect rect;
	pWnd->GetWindowRect( &rect );
	ScreenToClient( &rect );
	m_Progress.Create( WS_VISIBLE | WS_CHILD | PBS_SMOOTH,
//	m_Progress.Create( WS_CHILD | PBS_SMOOTH,
						rect, this, IDC_PROGRESS_R );
	m_Progress.SetRange( 0,100);
	m_state = 0;
	m_OrigFrame = m_StartFrame;
	CenterWindow();
	return FALSE;
}


void CActionSpeckleDlg::OnCancel()
{
	DPF("got cancel");
	if (m_state)
		m_state = 2;
	else
		EndDialog(0);
}

void CActionSpeckleDlg::CheckAbort()
{
	MSG msg;

	// Process existing messages in the application's message queue.
	// When the queue is empty, do clean up and return.
	while (::PeekMessage(&msg,NULL,0,0,PM_NOREMOVE) && (m_state == 1))
	{
		if (!AfxGetThread()->PumpMessage())
			return;
	}
}


void CActionSpeckleDlg::OnOK()
{
	DPF("got ok,kind:%d",m_Kind);
	if (m_state || !UpdateData(TRUE))
		return;
	GetDlgItem(IDC_PROGRESS_R)->ShowWindow(SW_SHOW);
	GetDlgItem(IDOK)->EnableWindow(FALSE);
	GetDlgItem(IDCANCEL)->SetWindowText("Abort");
	CLayers * pLayers = new CLayers;
	pLayers->Setup(m_pScene,0);
	
	m_state = 1;
	UINT	frame;
	UINT keycount;
	keycount = (m_EndFrame+1-m_StartFrame) * (m_EndLevel+1-m_StartLevel);
	DWORD * keys = new DWORD[keycount];
	keycount = 0;
	for (frame = m_StartFrame; (m_state == 1) && (frame <= m_EndFrame); frame++)
		{
		UINT level;
		for (level = m_StartLevel; level <= m_EndLevel; level++)
			{
//			keys[keycount] = m_pScene->GetCellKey(frame-1,level);
			DWORD key;
			m_pScene->GetImageKey(key,frame-1,level,98);
			if (!key)
				continue;
			keys[keycount] = key;
			UINT i;
			for (i = 0; keys[i] != keys[keycount];i++);
			if (i < keycount)
				{
DPF("skipping frm:%d,lvl:%d",frame-1,level);
				continue;
				}
			keycount++;
			pLayers->Select(frame-1, level);
			pLayers->CDespeckle(1+m_Kind);
			if (pLayers->Put(1))
				m_pFrame->UpdateCell(frame-1,level);
			}
		m_Progress.SetPos(MulDiv(100, frame-m_StartFrame,
				m_EndFrame +1 - m_StartFrame));
		CheckAbort();
		}
	delete [] keys;
	delete pLayers;
	m_state = 0;
	if (frame <= m_EndFrame)
		m_StartFrame = frame;
	else
		m_StartFrame = m_OrigFrame;
	UpdateData(0);
	GetDlgItem(IDOK)->EnableWindow(TRUE);
	GetDlgItem(IDCANCEL)->SetWindowText("Done");
	GetDlgItem(IDC_PROGRESS_R)->ShowWindow(SW_HIDE);
}

