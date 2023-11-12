// camdlg.cpp : implementation file
//

#include "stdafx.h"
#include "sketch.h"
#include "camdlg.h"
#include "myview.h"
#include "camera.h"
#include "dialogs.h"
#include "mainfrm.h"
#include "math.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define W_N  0
#define W_S  1
#define W_E  2
#define W_W  3
#define W_SCALE 4
#define W_ROT 5
#define W_BLUR 6
#define W_ALPHA 7

/////////////////////////////////////////////////////////////////////////////
// camdlg dialog


camdlg::camdlg(CWnd* pParent /*=NULL*/)
	: CDialog(camdlg::IDD, pParent)
{
	m_pCamera = 0;
	m_pDoc = 0;
	m_kind = 0;
	m_EnabledMask = 3; // clear this and all
	m_which = W_N;
	//{{AFX_DATA_INIT(camdlg)
	m_pegcx = 0.0;
	m_pegcy = 0.0;
	m_values[CCamera::CATTR_X] = 0.0;
	m_values[CCamera::CATTR_Y] = 0.0;
	m_values[CCamera::CATTR_ROT] = 0.0;
	m_values[CCamera::CATTR_SCALE] = 100.0;
	m_values[CCamera::CATTR_BLUR] = 0.0;
	m_values[CCamera::CATTR_ALPHA] = 100.0;
	m_Frame = 1;
	m_Frames = 1;
	m_bUpdating = 0;
	m_bPanning = 0;
	m_bFromTool = 0;
	m_kinds[CCamera::CATTR_Y] = 0;
	m_kinds[CCamera::CATTR_X] = 0;
	m_kinds[CCamera::CATTR_SCALE] = 0;
	m_kinds[CCamera::CATTR_ROT] = 0;
	m_kinds[CCamera::CATTR_BLUR] = 0;
	m_kinds[CCamera::CATTR_ALPHA] = 0;
	//}}AFX_DATA_INIT
}
/*
void PASCAL DDV_MyMinMax(CDataExchange * pDX, int id,
					double & value, double min, double max, int places = 3)
{
	if (pDX->m_bSaveAndValidate)
		{
		DDX_Text(pDX, id, value);
		DDV_MinMaxDouble(pDX, value, min, max);
		}
	else
		{
		CString str1;
		CString str2;
		str1.Format("%%.%df",places);
		str2.Format(str1,value);
		DDX_Text(pDX, id, str2);
		}
}
*/
void camdlg::MyUpdate(int id, double &value, double min, double max)
{
	CString str1;
	CString str2;
	if (value > max) value = max;
	if (value < min) value = min;
	str1.Format("%%.%df",2);//places);
	str2.Format(str1,value);
	CWnd * pDlg = GetDlgItem(id);
	pDlg->SetWindowText(str2);
	pDlg->UpdateWindow();
}

void camdlg::MyXYUpdate()
{
	double ns = 0.0;
	double ew = 0.0;
	char nsc[3];
	char ewc[3];
	CWnd * pns = GetDlgItem(IDC_NS);
	CWnd * pew = GetDlgItem(IDC_EW);
	ns = m_values[CCamera::CATTR_Y];
	ew = m_values[CCamera::CATTR_X];
	nsc[1] = 0;
	ewc[1] = 0;
	if (ns < 0)
		{
		ns = 0.0 - ns;
		nsc[0] = 'N';
		}
	else if ((ns == 0.0) && (m_which == W_N))
		nsc[0] = 'N';
	else
		nsc[0] = 'S';
	if (ns > 150.0)
		{
		ns = 150.;
		m_values[CCamera::CATTR_Y] = ns;
		}
	if (ew < 0)
		{
		ew = 0.0 - ew;
		ewc[0] = 'E';
		}
	else if ((ew == 0.0) && (m_which == W_E))
		ewc[0] = 'E';
	else
		ewc[0] = 'W';
	if (ew > 150.0)
		{
		ew = 150.;
		m_values[CCamera::CATTR_X] = ew;
		}
	CString str;
	CWnd * pDlg;
	str.Format("%.2f",ns);
	if (pns)pns->SetWindowText((LPCSTR)&nsc);
	pDlg = GetDlgItem(IDC_EDIT_NS);
	pDlg->SetWindowText(str);
	pDlg->UpdateWindow();
	if (pew) pew->SetWindowText((LPCSTR)&ewc);
	str.Format("%.2f",ew);		
	pDlg = GetDlgItem(IDC_EDIT_EW);
	pDlg->SetWindowText(str);
	pDlg->UpdateWindow();
}

void camdlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(camdlg)
	DDX_Control(pDX, IDC_COMBO_PEG, m_Peg);
	DDX_Control(pDX, IDC_COMBO_GROUP, m_Group);
#ifndef NEWWAY
	double ns = 0.0;
	double ew = 0.0;
	char nsc[3];
	char ewc[3];
	CWnd * pns = GetDlgItem(IDC_NS);
	CWnd * pew = GetDlgItem(IDC_EW);
	if (pDX->m_bSaveAndValidate)
		{
		DDX_Text(pDX, IDC_EDIT_NS, ns);
		DDV_MinMaxDouble(pDX, ns, 0., 150.);
		DDX_Text(pDX, IDC_EDIT_EW, ew);
		DDV_MinMaxDouble(pDX, ew, 0., 150.);
		if (pns) pns->GetWindowText((LPSTR)&nsc,2);
		if (pew) pew->GetWindowText((LPSTR)&ewc,2);
		if (nsc[0] == 'N')
			ns = 0.0 - ns;
		if (ewc[0] == 'E')
			ew = 0.0 - ew;
		m_values[CCamera::CATTR_Y] = ns;
		m_values[CCamera::CATTR_X] = ew;
		}
	else
		{
		ns = m_values[CCamera::CATTR_Y];
		ew = m_values[CCamera::CATTR_X];
		nsc[1] = 0;
		ewc[1] = 0;
		if (ns < 0)
			{
			ns = 0.0 - ns;
			nsc[0] = 'N';
			}
		else if ((ns == 0.0) && (m_which == W_N))
			nsc[0] = 'N';
		else
			nsc[0] = 'S';
		if (ew < 0)
			{
			ew = 0.0 - ew;
			ewc[0] = 'E';
			}
		else if ((ew == 0.0) && (m_which == W_E))
			ewc[0] = 'E';
		else
			ewc[0] = 'W';
		CString str;
		str.Format("%.2f",ns);
		if (pns)pns->SetWindowText((LPCSTR)&nsc);
		DDX_Text(pDX, IDC_EDIT_NS, str);
		if (pew) pew->SetWindowText((LPCSTR)&ewc);
		str.Format("%.2f",ew);		
		DDX_Text(pDX, IDC_EDIT_EW, str);
		}
#else
	DDX_Text(pDX, IDC_EDIT_EW, m_values[CCamera::CATTR_X]);
	DDV_MinMaxDouble(pDX, m_values[CCamera::CATTR_X], -150., 150.);
	DDX_Text(pDX, IDC_EDIT_NS, m_values[CCamera::CATTR_Y]);
	DDV_MinMaxDouble(pDX, m_values[CCamera::CATTR_Y], -150., 150.);
#endif
	DDV_MyMinMax(pDX, IDC_EDIT_ROT, m_values[CCamera::CATTR_ROT],
				-3600., 3600.,2);
	if (!m_kind || m_nGroup)
		DDV_MyMinMax(pDX, IDC_EDIT_SC,
				m_values[CCamera::CATTR_SCALE], 10., 1000.,2);
	else
		DDV_MyMinMax(pDX, IDC_EDIT_SC,
				m_values[CCamera::CATTR_SCALE], 1.2, 120.,2);
	DDV_MyMinMax(pDX, IDC_EDIT_BLUR, m_values[CCamera::CATTR_BLUR], 0., 20.,2);
	DDV_MyMinMax(pDX, IDC_EDIT_ALPHA, m_values[CCamera::CATTR_ALPHA], 0., 100.,1);
	DDV_MyMinMax(pDX, IDC_PEG_CX, m_pegcx, -150., 150.,2);
	DDV_MyMinMax(pDX, IDC_PEG_CY, m_pegcy, -150., 150.,2);
//	DDX_Text(pDX, IDC_FRAME, m_Frame);
//	DDV_MinMaxUInt(pDX, m_Frame, 1, m_Frames);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(camdlg, CDialog)
	//{{AFX_MSG_MAP(camdlg)
	ON_WM_CLOSE()
//	ON_EN_KILLFOCUS(IDC_FRAME, OnUpdate)
	ON_BN_CLICKED(IDC_APPLY, OnApply)
	ON_BN_CLICKED(IDC_ADJ_N, OnAdjN)
	ON_BN_CLICKED(IDC_ADJ_S, OnAdjS)
	ON_BN_CLICKED(IDC_ADJ_E, OnAdjE)
	ON_BN_CLICKED(IDC_ADJ_W, OnAdjW)
	ON_BN_CLICKED(IDC_ADJ_SC, OnAdjSc)
	ON_BN_CLICKED(IDC_ADJ_ROT, OnAdjRot)
	ON_BN_CLICKED(IDC_ADJ_BLUR, OnAdjBlur)
	ON_BN_CLICKED(IDC_ADJ_ALPHA, OnAdjAlpha)

	ON_BN_CLICKED(IDC_PEGX, OnPeg)
	ON_BN_CLICKED(IDC_GROUP, OnGroup)

	ON_BN_CLICKED(IDC_KEY_TYPE, OnKeyType)
	ON_COMMAND(ID_KEY_TOOL_TYPE, OnKeyToolType)
	ON_BN_CLICKED(IDC_NEXTKEY,OnNextKey)
	ON_BN_CLICKED(IDC_PREVKEY,OnPrevKey)
	ON_BN_CLICKED(IDC_CLEARKEYS, OnClearKeys)
	ON_BN_CLICKED(IDC_CLEARALLKEYS, OnClearAllKeys)
	ON_BN_CLICKED(IDC_GOTO, OnGoto)
	ON_BN_CLICKED(IDC_CAMHOME, OnCamHome)
	ON_BN_CLICKED(IDC_CAMEND, OnCamEnd)

	ON_CBN_SELCHANGE(IDC_COMBO_GROUP, OnSelchangeComboGroup)
	ON_CBN_SELCHANGE(IDC_COMBO_PEG, OnSelchangeComboPeg)
	ON_BN_CLICKED(IDC_EDIT_PEG, OnEditPeg)
	ON_BN_CLICKED(IDC_EDIT_GROUP, OnEditGroup)
	ON_EN_CHANGE(IDC_FRAME, OnChangeFrame)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// camdlg message handlers

BOOL camdlg::Changed(BOOL bSave /* = 0 */)
{
	int i;
	BOOL bResult = 0;
	for (i = CCamera::CATTR_Y; i < CCamera::CATTR_EXTRA; i++)
		{
		if (bSave)
			{
			m_kinds2[i] = m_kinds[i];
			m_values2[i] = m_values[i];
			}
		else
			{
			if (m_kinds2[i] != m_kinds[i])
				{
//				DPF("1changed:%d,%d,%d,%d,%d",i,m_kinds[i],m_kinds2[i],(int)(10000*m_values[i]),(int)(10000*m_values2[i]));
				bResult = TRUE;
				break;
				}
			if((int)(100000*m_values2[i]) != (int)(100000*m_values[i]))
				{
DPF("2changed:%d,%d,%d",i,(int)(10000*m_values[i]),(int)(10000*m_values2[i]));
				bResult = TRUE;
				break;
				}
			}
		}
	if (bSave)
		{
		m_pegcx2 = m_pegcx;
		m_pegcy2 = m_pegcy;
		}
	else
		{
		if ((m_pegcx2 != m_pegcx) || ( m_pegcy2 != m_pegcy))
			bResult = TRUE;
		}
DPF("changed:%d",bResult);
	return bResult;
}

void camdlg::OnApply() 
{
DPF("on apply");
	UpdateData(1);
//	m_pCamera->m_max_diff = 0;
//	m_pCamera->m_min_diff = m_Frames;
	if (Changed())
		{
		FlushChanges();
		SetCrossHair(TRUE);
//		m_pScene->UpdateCache();//0, Level,9999);
		CSketchView * pView = m_pDoc->GetDocView();
		pView->Update(9999,0,30);
		
		Changed(TRUE);
		}
DPF("apply,min:%d",m_pCamera->m_min_diff);
	if (m_pCamera->m_min_diff < m_Frames)
		{
		if (m_pCamera->m_min_diff)
			m_pCamera->m_min_diff--;
		m_pScene->UpdateCache(m_pCamera->m_min_diff,9998,
				m_pCamera->m_max_diff);
		}
	m_pCamera->m_min_diff = m_Frames;
	CSketchView * pView = m_pDoc->GetDocView();
	pView->Update(m_Frame-1,0,9);
	FillInDialog();
}

UINT camdlg::WhichAttr(UINT which /* = -1 */)
{
	UINT res;
	if (which == -1)
		which = m_which;
	switch (which) {
	case W_N:
	case W_S:
		res = CCamera::CATTR_Y;
		break;
	case W_E:
	case W_W:
		res = CCamera::CATTR_X;
		break;
	case W_SCALE:
		res = CCamera::CATTR_SCALE;
		break;
	case W_ROT:
		res = CCamera::CATTR_ROT;
		break;
	case W_BLUR:
		res = CCamera::CATTR_BLUR;
		break;
	case W_ALPHA:
		res = CCamera::CATTR_ALPHA;
		break;
	default:
ASSERT(0);
		res = 10;
	}
	return res;
}

void camdlg::SetCrossHair(BOOL bSetFocus)
{
	if (!m_pDoc) return;
	CSketchView * pView = m_pDoc->GetDocView();
	int pegx, pegy;
	m_pCamera->PegXY(pegx, pegy, m_kind ? m_nGroup + 1000 : m_nPeg,
					m_Frame-1, m_width, m_height);
	pegx *= m_factor;
	pegy *= m_factor;
	pView->CameraInit(pegx / 2, pegy / 2,256);
	if (bSetFocus)
		pView->SetFocus();
}

UINT camdlg::CameraPegXY(int & x, int & y, int & kind, int frame,int w,int h)
{
	double dummy;
	int peg = m_kind ? m_nGroup + 1000 : m_nPeg;
	UINT k1,k2;
	m_pCamera->GetAttr(CCamera::CATTR_X, frame, peg,dummy,k1);
	m_pCamera->GetAttr(CCamera::CATTR_Y, frame, peg,dummy,k2);
	kind = k1 || k2 ? 1 : 0;
	m_pCamera->PegXY(x, y, peg, frame, w, h);
	return 0;
}

void camdlg::AdjIt(int val, BOOL bApply)
{
	m_which = val & 255;
	m_bPanning = (val & 256) && (m_which < W_SCALE) ? 1 : 0;
	CEdit * pEdit = 0;
	ASSERT(m_which <= W_ALPHA);
	char ns[2];
	char ew[2];
	ns[0] = 0;
	ns[1] = 0;
	ew[0] = 0;
	ew[1] = 0;
	if (m_which == W_N)
		ns[0] = 'N';
	else if (m_which == W_S)
		ns[0] = 'S';
	else if (m_which == W_E)
		ew[0] = 'E';
	else if (m_which == W_W)
		ew[0] = 'W';
	if (ns[0]) 
		GetDlgItem(IDC_NS)->SetWindowText((LPCSTR)&ns);
	if (ew[0]) 
		GetDlgItem(IDC_EW)->SetWindowText((LPCSTR)&ew);
	UINT attr = WhichAttr();
	m_kinds[attr] |= 1;  // make it a key
	if ((m_which == W_N) && (val & 256))
		m_kinds[CCamera::CATTR_X] |= 1;  // make ew also a key since pan tool
//		GetDlgItem(IDC_EDIT_NS+attr)->EnableWindow(TRUE);
	pEdit = (CEdit *)GetDlgItem(IDC_EDIT_NS+attr);
	if (bApply)
		OnApply();
	BOOL bKeyType = TRUE;//m_which < W_PEG ? TRUE : FALSE;
	GetDlgItem(IDC_KEY_TYPE)->EnableWindow(bKeyType);
	GetDlgItem(IDC_CLEARKEYS)->EnableWindow(bKeyType);
	UpdateView();
	if (pEdit)
		{
		pEdit->EnableWindow(TRUE);
		pEdit->SetSel(0,-1);
		pEdit->SetFocus();
		}
		
#ifdef FLIPBOOK_MAC
	CheckDlgButton (IDC_ADJ_N, m_which == W_N);
	CheckDlgButton (IDC_ADJ_S, m_which == W_S);
	CheckDlgButton (IDC_ADJ_E, m_which == W_E);
	CheckDlgButton (IDC_ADJ_W, m_which == W_W);
	CheckDlgButton (IDC_ADJ_SC, m_which == W_SCALE);
	CheckDlgButton (IDC_ADJ_ROT, m_which == W_ROT);
	CheckDlgButton (IDC_ADJ_BLUR, m_which == W_BLUR);
	CheckDlgButton (IDC_ADJ_ALPHA, m_which == W_ALPHA);
#endif
}


void camdlg::UpdateView()
{
	if (!m_pDoc)
		return;
	CSketchView * pView = m_pDoc->GetDocView();
	UINT mask = WhichAttr();
	if (m_which < W_SCALE)
		{
		if ((m_kinds[CCamera::CATTR_X] & 1) 
			&& (m_kinds[CCamera::CATTR_Y] & 1))
			mask |= 16;
		if (m_bPanning)
			mask |= 16;
		}
	pView->CameraInit(m_Frame-1,0, mask);
	pView->SetFocus();
}

void camdlg::OnPeg() 
{
	SetKind(0);
}

void camdlg::OnGroup() 
{
	SetKind(1);
}

void camdlg::OnSelchangeComboPeg()
{
	FlushChanges();
	m_nPeg = m_Peg.GetCurSel();
//	m_pScene->SelectPeg(m_nPeg);
	m_nGroup = m_pCamera->GroupFindPeg(m_nPeg);
	m_Group.SetCurSel(m_nGroup);
	DPF("new peg:%d",m_nPeg);
	FillInDialog();
	SetCrossHair();
}

void camdlg::OnSelchangeComboGroup()
{
	FlushChanges();
	m_nGroup = m_Group.GetCurSel();
	DPF("new grp:%d",m_nGroup);
	FillInDialog();
	SetCrossHair();
}

void camdlg::SelectPeg(UINT peg)
{
	if (peg == -1) return;
	if (m_kind) return;
//	m_pScene->SelectPeg(peg);
	m_nPeg = peg;
	m_Peg.SetCurSel(m_nPeg);
//	if (m_kind)
		{
		m_nGroup = m_pCamera->GroupFindPeg(m_nPeg);
		m_Group.SetCurSel(m_nGroup);
		}
	DPF("new peg:%d,grp:%d",m_nPeg,m_nGroup);
	FillInDialog();
	SetCrossHair();
}

void camdlg::OnChangeFrame()
{
//	char buf[12];
//	if (!GetDlgItem(IDC_FRAME)->GetWindowText(buf,10))
//		{
//		DPF("chg frame empty");
//		}
}

void camdlg::OnEditGroup()
{
	CGroupDlg dlg;
	dlg.m_pCamera = m_pCamera;
	dlg.m_group = m_nGroup;
	if (dlg.DoModal() == IDOK)
		{
		m_Group.DeleteString(m_nGroup);
		m_Group.InsertString(m_nGroup,(LPCSTR)dlg.m_name);
		m_Group.SetCurSel(m_nGroup);
		m_pCamera->GroupName((LPSTR)(LPCSTR)dlg.m_name,m_nGroup,TRUE);
		}
}

void camdlg::OnEditPeg()
{
	CPegDlg dlg;
	dlg.m_pCamera = m_pCamera;
	UINT group = m_pCamera->GroupFindPeg(m_nPeg);
	dlg.m_group = group;
	dlg.m_peg = m_nPeg;
	dlg.m_bAllowFieldComp = 
		((CSketchApp*)AfxGetApp())->CanDoFeature(CSketchApp::CD_TELECINE);
	dlg.m_nFieldComp = m_pCamera->PegFlags(m_nPeg, 1);
	BOOL bFComp = dlg.m_nFieldComp;
	if (dlg.DoModal() == IDOK)
		{
		m_Peg.DeleteString(m_nPeg);
		m_Peg.InsertString(m_nPeg,(LPCSTR)dlg.m_name);
		m_Peg.SetCurSel(m_nPeg);
		m_pCamera->PegName((LPSTR)(LPCSTR)dlg.m_name,m_nPeg,TRUE);
		if ((group != (UINT)dlg.m_group) || 
					(bFComp != dlg.m_nFieldComp))

			{
			m_pCamera->PegFlags(m_nPeg,1,dlg.m_nFieldComp);
			m_nGroup = dlg.m_group;
DPF("changing groups");
			m_pCamera->GroupAttach(dlg.m_peg,m_nGroup);
//		GetDlgItem(IDC_COMBO_GROUP)->EnableWindow(TRUE);
			m_Group.SetCurSel(m_nGroup);
//		GetDlgItem(IDC_COMBO_GROUP)->EnableWindow(FALSE);
			SetupDialog();
			OnApply();
			}
// reselect 
		}
}

void camdlg::UpdateControls(BOOL bSetPeg)
{
//	UpdateData();
	int i,j;
	if (m_which == 8)
		j = 8;
	else
		j = -1;
	for (i = W_N; i <= W_ALPHA; i++)
		{
		int a = WhichAttr(i);
		BOOL bKey = m_kinds[CCamera::CATTR_Y+a] & 1;
		if (bKey)			// if a key point
			{
			if ((j < 0) || (i == (int)m_which))
				j = i;
			else if (j != (int)m_which)
				j = 999;
			}
//		GetDlgItem(IDC_ADJ_NS+i)->EnableWindow(bKey);
		GetDlgItem(IDC_EDIT_NS+a)->EnableWindow(bKey);
		}
	if ((j > W_ALPHA) || (j < 0))
		j = W_N;
	if (j != (int)m_which)
		{
//		AdjIt(j);
		CheckRadioButton(IDC_ADJ_N, IDC_ADJ_ALPHA, IDC_ADJ_N + m_which);
//		GetDlgItem(IDC_ADJ_CENTER)->SendMessage(BM_CLICK, 0,0);
		}
	BOOL bKeyType = TRUE;//m_which < W_PEG ? TRUE : FALSE;
	GetDlgItem(IDC_KEY_TYPE)->EnableWindow(bKeyType);
	if (m_pCamera)
	{
	UINT peg;
	if (m_kind)
		peg = m_nGroup + 1000;
	else
		peg = m_nPeg;
	m_EnabledMask |= 4;
	if (m_pCamera->FindKey(peg,m_Frame-1,TRUE) == -1)
		m_EnabledMask ^= 4;
	m_EnabledMask |= 8;
	if (m_pCamera->FindKey(peg,m_Frame-1,FALSE) == -1)
		m_EnabledMask ^= 8;
	GetDlgItem(IDC_NEXTKEY)->EnableWindow((m_EnabledMask & 4) ? 1 : 0);
	GetDlgItem(IDC_PREVKEY)->EnableWindow((m_EnabledMask & 8) ? 1 : 0);
	}
}

void camdlg::SetKind(int kind)
{
	FlushChanges();
	m_kind = kind;
	if (!m_kind && m_pCamera)
		{
		m_nGroup = m_pCamera->GroupFindPeg(m_nPeg);
		m_Group.SetCurSel(m_nGroup);
		}
	DPF("new kind:%d",m_kind);
//	GetDlgItem(IDC_COMBO_PEG)->ShowWindow(m_kind ? SW_HIDE : SW_SHOW);
//	GetDlgItem(IDC_EDIT_PEG)->ShowWindow(m_kind ? SW_HIDE : SW_SHOW);
	GetDlgItem(IDC_COMBO_PEG)->EnableWindow(m_kind == 0);
	GetDlgItem(IDC_EDIT_PEG)->EnableWindow(m_kind == 0);
	GetDlgItem(IDC_COMBO_GROUP)->EnableWindow(m_kind != 0);
	GetDlgItem(IDC_EDIT_GROUP)->EnableWindow(m_kind != 0);
	FillInDialog();
	SetCrossHair();
}


BOOL camdlg::MyCreate( CWnd* pOwnerWnd)//, int x, int y)
{
	BOOL bResult = Create(camdlg::IDD, pOwnerWnd);
	ShowWindow(SW_HIDE);
	DPF("create:%d",bResult);
	return bResult;
}


void camdlg::Enable(BOOL bEnable)
{
	DPF("enable:%d",bEnable);
	if (bEnable)
		{
		//AdjIt(m_which,0);
		SetCrossHair(TRUE);
		}
	else
		{
		CSketchView * pView = m_pDoc->GetDocView();
		pView->CameraInit(0,0,257);
		}
}

void camdlg::SetFrames()
{
	m_Frames = m_pScene->FrameCount();
}

UINT camdlg::Setup(CSketchDoc * pDoc)
{
	DPF("cameradlg init");
	m_pDoc = pDoc;
	if (!pDoc)
		{
		m_pCamera = 0;
		return 0;
		}
	m_pScene = m_pDoc->m_pScene;
	m_factor = m_pScene->ZFactor();
	m_width = m_pScene->ComW();
	m_height = m_pScene->ComH();
	m_pCamera = m_pScene->Camera();
	SetFrames();
	if (m_Frame >= m_Frames)
		m_Frame = m_Frames;

#if MAC
	DoInitDialog();
	GetDlgItem(IDC_COMBO_PEG);
	GetDlgItem(IDC_EDIT_PEG);
	GetDlgItem(IDC_COMBO_GROUP);
	GetDlgItem(IDC_EDIT_GROUP);
#endif

//	m_peg = m_pCamera->PegFindLevel(m_level);
	m_Peg.ResetContent();
	int i,c;
	c = m_pCamera->PegCount();
	for (i = 0; i < c; i++)
		{
		char buf[40];
		m_pCamera->PegName(buf,i);
		m_Peg.AddString(buf);
		}
	m_Peg.SetCurSel(m_nPeg = 0);
	m_Group.ResetContent();
	c = m_pCamera->GroupCount();
	for (i = 0; i < c; i++)
		{
		char buf[40];
		m_pCamera->GroupName(buf,i);
		m_Group.AddString(buf);
		}
//	m_nGroup = 0;//m_pCamera->GroupFindPeg(m_nPeg);
	m_nGroup = m_pCamera->GroupFindPeg(m_nPeg);
	m_Group.SetCurSel(m_nGroup);
	m_which = W_N;//W_PEG;
	m_kind = 0;
	FillInDialog();
	SetKind(0);
	return 0;
}

int ddx[8] = {1,0,-1,-1,-1,0,1,1};
//int ddx[8] = {-1,0,1,1,1,0,-1,-1};
int ddy[8] = {-1,-1,-1,0,1,1,1,0};

UINT camdlg::Packet(UINT kind, int v1, int v2)
{
	DPF("packet,k:%d,v1:%d,v2:%d",kind,v1,v2);
//	char buf[50];
//	double x,y;	
	UINT kkind;
	double value;
	CSketchView * pView;
	m_factor = m_pScene->ZFactor();
	m_width = m_pScene->ComW();
	m_height = m_pScene->ComH();
	if (kind & 16)	// peg center adjustment
		{
		kind &= 3;
if (m_kind && !m_nGroup)
	return 0;
if (m_pScene->Flag(SCN_FLG_REDBOX) && m_pScene->Flag(SCN_FLG_NLARGE))
			{
			v1 = v1 / 2 + m_width / 4;
			v2 = v2 / 2 + m_height / 4;
DPF("flag3,v1:%d,v2:%d",v1,v2);
			}
		v1 = (2 * v1) / (int)m_factor;
		v2 = (2 * v2) / (int)m_factor;
		m_pCamera->PegVXY(m_pegcx, m_pegcy, v1,v2,
					m_kind ? m_nGroup + 1000 : m_nPeg, 
						m_Frame-1, m_width, m_height);
		MyUpdate(IDC_PEG_CX,m_pegcx,-150., 150.);
		MyUpdate(IDC_PEG_CY,m_pegcy,-150., 150.);
		UpdateAttr(99,TRUE);
	//	if ((kind == 0) || (kind == 1))
	//		AdjIt(W_PEG);
		SetCrossHair();
		pView = m_pDoc->GetDocView();
		pView->Update(m_Frame-1,0,8+2);
//			OnApply();
		return 0;
		
		}
	switch (kind) {
	case 6:
			if (v2)
				{
//				m_pScene->SelectPeg(v1);
		//		FlushChanges();
				SelectPeg(v1);
				}
			else
				UpdateView();
			break;
	case 0: // stop draw
DPF("done");
		OnApply();
		break;
	case 2:		// start draw
		v1 = (2 * v1) / (int)m_factor;
		v2 = (2 * v2) / (int)m_factor;
		m_orig_i1 = v1;
		m_orig_i2 = v2;
		if ((m_which == W_N)|| (m_which == W_S))
			{
			m_orig_d1 = m_values[CCamera::CATTR_X];
			m_orig_d2 = m_values[CCamera::CATTR_Y];
			m_kinds[CCamera::CATTR_Y] |= 1;
			if (m_bPanning)
				m_kinds[CCamera::CATTR_X] |= 1;
			}
		if ((m_which == W_E) || (m_which == W_W))
			{
			m_orig_d1 = m_values[CCamera::CATTR_X];
			m_orig_d2 = m_values[CCamera::CATTR_Y];
			m_kinds[CCamera::CATTR_X] |= 1;
			if (m_bPanning)
				m_kinds[CCamera::CATTR_Y] |= 1;
			}
		if (m_which == W_SCALE)
			{
			m_orig_d1 = m_values[CCamera::CATTR_SCALE];
			m_kinds[CCamera::CATTR_SCALE] |= 1;
			}
		if (m_which == W_ROT)
			{
			m_orig_d1 = m_values[CCamera::CATTR_ROT];
			m_kinds[CCamera::CATTR_ROT] |= 1;
			}
		if (m_which == W_BLUR)
			{
			m_orig_d1 = m_values[CCamera::CATTR_BLUR];
			m_kinds[CCamera::CATTR_BLUR] |= 1;
			}
		if (m_which == W_ALPHA)
			{
			m_orig_d1 = m_values[CCamera::CATTR_ALPHA];
			m_kinds[CCamera::CATTR_ALPHA] |= 1;
			}
//		UpdateControls();
		break;
	case 1: // mouse move
		v1 = (2 * v1) / (int)m_factor;
		v2 = (2 * v2) / (int)m_factor;
		if ((m_which == W_N) || (m_which == W_S) ||
				(m_which == W_E) || (m_which == W_W))
			{
			if (m_kind && !m_nGroup)
				value = 100.0;/// 1200.0 / m_values[CCamera::CATTR_SCALE];
			else
				{
				m_pCamera->GetAttr(CCamera::CATTR_SCALE, m_Frame-1,
						1000,value,kkind);
				if (!m_kind)
					{
					double tt = value;
					m_pCamera->GetAttr(CCamera::CATTR_SCALE, m_Frame-1,
						m_nGroup + 1000,value,kkind);
					value = (tt * value / 100.0);
					}
				}
			double temp = 2400.0 /  value;
			if (m_pScene->Flag(SCN_FLG_REDBOX) && !m_pScene->Flag(SCN_FLG_NLARGE))
				temp *= 2;
			m_values[CCamera::CATTR_X] =
				m_orig_d1 + (((double)(v1 - m_orig_i1)) * temp) / m_width;
			m_values[CCamera::CATTR_Y] = 
				m_orig_d2 - (((double)(v2 - m_orig_i2)) * temp) / m_height;
			MyXYUpdate();
			UpdateAttr(CCamera::CATTR_X,TRUE);
			UpdateAttr(CCamera::CATTR_Y,TRUE);
			}
		if (m_which == W_SCALE)
			{
			int pegx, pegy;
			m_pCamera->PegXY(pegx, pegy,
					m_kind ? m_nGroup + 1000 : m_nPeg, 
						m_Frame-1, m_width, m_height);
			int dx = m_orig_i1 - pegx;
			int dy = m_orig_i2 - pegy;
			double t1 = sqrt((double)dx * dx + dy * dy);
			DPF("dx:%d,dy:%d,t:%d",dx,dy,(int)t1);
			dx = v1 - pegx;
			dy = v2 - pegy;
			double temp = dx * dx + dy * dy;
			double t2 = sqrt(temp);
			DPF("dx:%d,dy:%d,t:%d",dx,dy,(int)t2);
			temp = m_orig_d1;
			if (m_kind && !m_nGroup)
				temp = 1200. / temp;
			temp = temp * t2 / t1;
			if (m_kind && !m_nGroup)
				temp = 1200. / temp;
			m_values[CCamera::CATTR_SCALE] = temp;
//			m_values[CCamera::CATTR_SCALE] = m_orig_d1 * t2 / t1;
			UpdateData(0);
			UpdateAttr(CCamera::CATTR_SCALE,TRUE);
			}
		if (m_which == W_ROT)
			{
			int pegx, pegy;
			m_pCamera->PegXY(pegx, pegy,
					m_kind ? m_nGroup + 1000 : m_nPeg, 
						m_Frame-1, m_width, m_height);
			int dx = m_orig_i1 - pegx;
			int dy = m_orig_i2 - pegy;
			DPF("dx:%d,dy:%d",dx,dy);
			double t1 = atan2((double)dy,(double)dx);
	 		dx = v1 - pegx;
	 		dy = v2 - pegy;
			DPF("dx:%d,dy:%d",dx,dy);
			double t2 = atan2((double)dy,(double)dx);
			double factor = 180 / 3.14159265;
//			factor = 45 / factor;
			t1 *= factor;
			t2 *= factor;
		int a = (int)t1;
		int b = (int)t2;
			if (t1 < 0) t1 += 360;
			if (t2 < 0) t2 += 360;
			double delta = t2 - t1;
			if ((delta > 0) && (delta > 180))
				delta = 360 - delta;
			if ((delta < 0) && (delta < -180))
				delta = 360 + delta;
			//		double delta = factor * (t2 - t1);
			DPF("a:%d,b:%d,delta:%d",a,b,(int)delta);
			delta += m_orig_d1;
		//	if (delta < 0) delta += 360;
		//	if (delta > 360) delta -= 360;
DPF("was:%d,is:%d",(int)m_values[CCamera::CATTR_ROT],(int)delta);

			m_values[CCamera::CATTR_ROT] = delta;
			UpdateData(0);
			UpdateAttr(CCamera::CATTR_ROT,TRUE);
			}
		if (m_which == W_ALPHA)
			{
			double	t = m_orig_d1 - 400 * (double)(v2 - m_orig_i2) / (double)m_height;
			if (t > 100) t = 100;
			if (t < 0) t = 0;
			m_values[CCamera::CATTR_ALPHA] = t;
			UpdateData(0);
			UpdateAttr(CCamera::CATTR_ALPHA,TRUE);
			}
		if (m_which == W_BLUR)
			{
			double	t = m_orig_d1 - 100 * (double)(v2 - m_orig_i2) / (double)m_height;
			if (t > 20) t = 20;
			if (t < 0) t = 0;
			m_values[CCamera::CATTR_BLUR] = t;
			UpdateData(0);
			UpdateAttr(CCamera::CATTR_BLUR,TRUE);
			}
		SetCrossHair(TRUE);
		pView = m_pDoc->GetDocView();
		pView->Update(m_Frame-1,0,8+2);
		break;
	case 3:
DPF("motion from flip,%d",v1);
		if (m_Frame != (UINT)(v1 + 1))
			{
			FlushChanges();
			m_Frame = v1+1;
			FillInDialog();
			SetCrossHair();
			}
		break;
	case 5:
		SetCrossHair();
		break;
	case 4:
DPF("packet, key,code:%d",v1);
		switch (WhichAttr()) {
		case CCamera::CATTR_Y:
			m_values[CCamera::CATTR_Y] += ((double)ddy[v1 & 7] * 24) / m_height; 
			break;
		case CCamera::CATTR_X:
			m_values[CCamera::CATTR_X] += ((double)ddx[v1 & 7] * 24) / m_width; 
			break;
		case CCamera::CATTR_SCALE:
			m_values[CCamera::CATTR_SCALE] += ((double)ddy[v1 & 7]) / 100; 
			break;
		case CCamera::CATTR_ROT:
			m_values[CCamera::CATTR_ROT] += (double)ddy[v1 & 7]; 
		//	if (m_values[CCamera::CATTR_ROT] < 0)
		//		m_values[CCamera::CATTR_ROT] += 360;
			if (m_values[CCamera::CATTR_ROT] >= 360)
				m_values[CCamera::CATTR_ROT] -= 360;
			break;
		case CCamera::CATTR_ALPHA:
			m_values[CCamera::CATTR_ALPHA] += ((double)ddy[v1 & 7]) / 100; 
			if (m_values[CCamera::CATTR_ALPHA] < 0)
				m_values[CCamera::CATTR_ALPHA] = 0;
			if (m_values[CCamera::CATTR_ALPHA] > 1)
				m_values[CCamera::CATTR_ALPHA] = 1;
			break;
		case CCamera::CATTR_BLUR:
//			m_values[CCamera::CATTR_BLUR] += ((double)ddy[v1 & 7]) / 100; 
//			if (m_values[CCamera::CATTR_BLUR] < 0)
//				m_values[CCamera::CATTR_BLUR] = 0;
//			if (m_values[CCamera::CATTR_BLUR] > 1)
//				m_values[CCamera::CATTR_BLUR] = 1;
			break;
		default:
			break;
		}
		UpdateData(0);
		OnApply();
		break;
	default:
DPF("bad packet:%d",kind);
		break;
	}
	return 0;
}

void camdlg::OnClose()
{
	DPF("On cam Close");
	((CMainFrame *)GetParent())->SwitchCamera(TRUE);
}


BOOL camdlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
//	m_black.Create(this, IDC_CAP_BLACK, black, 0, 255);
//	m_peg = m_pCamera->PegFindLevel(m_level);
	m_Peg.ResetContent();
	m_Group.ResetContent();
	UpdateData(FALSE);
	CheckRadioButton(IDC_PEGX, IDC_GROUP, IDC_PEGX + m_kind);
	CheckRadioButton(IDC_ADJ_N, IDC_ADJ_ALPHA, IDC_ADJ_N + m_which);
//	CheckDlgButton(IDC_KEY_NS, m_kinds[CCamera::CATTR_Y] & 1);
//	CheckDlgButton(IDC_KEY_EW, m_kinds[CCamera::CATTR_X] & 1);
//	CheckDlgButton(IDC_KEY_SC, m_kinds[CCamera::CATTR_SCALE] & 1);
//	CheckDlgButton(IDC_KEY_ROT,m_kinds[CCamera::CATTR_ROT] & 1);
//	CheckDlgButton(IDC_KEY_BLUR, m_kinds[CCamera::CATTR_BLUR] & 1);
//	CheckDlgButton(IDC_KEY_ALPHA, m_kinds[CCamera::CATTR_ALPHA] & 1);

#if !MAC
	SetKind(0);
	HICON hIcon;

	hIcon = (HICON)LoadImage(  AfxGetApp()->m_hInstance,
                           MAKEINTRESOURCE(IDR_MAINFRAME),
                           IMAGE_ICON,
                           GetSystemMetrics(SM_CXSMICON),
                           GetSystemMetrics(SM_CYSMICON),
                           0);
	if(hIcon)
		{
         SendMessage(WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		}
#endif

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void camdlg::OnOK()
{
	DPF("onOK");
//	OnUpdate();
	UpdateData();
	DPF("after ok");
//	CDialog::OnOK();
}

void camdlg::OnCancel() 
{
	DPF("on cancel");
//	CDialog::OnCancel();
}

void camdlg::OnUpdate()
{
	DPF("updating");
	if (!m_pCamera)
		return;
	UpdateAllAttr(TRUE);
	UpdateData(1);
	FillInDialog();
}

void camdlg::UpdateAttr(UINT which, BOOL bPut/*= FALSE*/)
{
	UINT result;
	UINT Kind;
	UINT peg;
	double value;
	if (m_kind)
		peg = m_nGroup + 1000;
	else
		peg = m_nPeg;
	if (which == 99)
		{
		m_pCamera->PegCenter(m_pegcx, m_pegcy, peg, bPut);
		m_pCamera->m_min_diff = 0;
		return;
		}
	if (bPut)
		{
		value = m_values[which];
		Kind = m_kinds[which];
if ((peg == 1000) && (which == CCamera::CATTR_SCALE))
	value = 1200. / value;
		result = m_pCamera->PutAttr(which, m_Frame-1, peg, value, Kind);
		m_pCamera->Compute(which);
		}
	else
		{
		result = m_pCamera->GetAttr(which, m_Frame-1,peg,value,Kind);
if ((peg == 1000) && (which == CCamera::CATTR_SCALE))
	value = 1200. / value;
		m_values[which] = value;
		m_kinds[which] = Kind;
		}
}

void camdlg::UpdateAllAttr(BOOL bPut/* = FALSE */)
{
	UpdateAttr(CCamera::CATTR_X,bPut);
	UpdateAttr(CCamera::CATTR_Y,bPut);
	UpdateAttr(CCamera::CATTR_SCALE,bPut);
	UpdateAttr(CCamera::CATTR_ROT,bPut);
	UpdateAttr(CCamera::CATTR_BLUR,bPut);
	UpdateAttr(CCamera::CATTR_ALPHA,bPut);
	UpdateAttr(99,bPut);
}

void camdlg::SetDlgItem(int id, int which, int value)
{
	CWnd * pWnd = GetDlgItem(id);
	if (!pWnd) return;
	if (which == 0)
		pWnd->ShowWindow(value);
	else if (which == 1)
		pWnd->EnableWindow(value);
}

void camdlg::SetupDialog()
{
	BOOL bEnable = !m_kind || m_nGroup ? TRUE : FALSE;
	SetDlgItem(IDC_PEG_CX,1,bEnable);
	SetDlgItem(IDC_PEG_CY,1,bEnable);
}

void camdlg::FillInDialog()
{
	if (m_kind)
		{
		DPF("filling grp:%d,frame:%d",m_nGroup,m_Frame);
		}
	else
		{
		DPF("filling peg:%d,frame:%d",m_nPeg,m_Frame);
		}
	SetupDialog();
	if (!m_pCamera)
		return;
	UpdateAllAttr();
	UpdateData(0);
	CheckRadioButton(IDC_PEGX, IDC_GROUP, IDC_PEGX + m_kind);
	CheckRadioButton(IDC_ADJ_N, IDC_ADJ_ALPHA, IDC_ADJ_N + m_which);
	Changed(TRUE);
	UpdateControls();
}

void camdlg::FlushChanges()
{
	if (m_kind)
		{
		DPF("flushing grp:%d,frame:%d",m_nGroup,m_Frame);
		}
	else
		{
		DPF("flushing peg:%d,frame:%d",m_nPeg,m_Frame);
		}
	if (!m_pCamera)
		return;
	UpdateAllAttr(TRUE);
}


void camdlg::KeyDialog(BOOL bTool)
{
	UINT attr = WhichAttr();
	if (bTool && (attr == CCamera::CATTR_X))
		attr = CCamera::CATTR_Y; // if cam tool lock ns to ew
	
	CKeyTypeDlg dlg;
	dlg.m_Id = IDS_NS_KEY + attr;
	UINT peg;
	if (m_kind)
		peg = m_nGroup + 1000;
	else
		peg = m_nPeg;
//	double value;
	UINT kind;
//	m_pCamera->GetAttr(k,m_Frame-1,peg,value,kind);
	kind = m_kinds[attr];
	dlg.m_Kind = (kind >> 1) & 1;
	dlg.m_Frame = m_Frame;
	dlg.m_Frames = m_Frames;
	dlg.m_EaseIn = (kind >> 2) & 32767;
	dlg.m_EaseOut = (kind >> 17) & 32767;
	if (dlg.DoModal() == IDOK)
		{
DPF("got okay,k:%d,in:%d,out:%d",dlg.m_Kind,dlg.m_EaseIn,dlg.m_EaseOut);
		kind = 1 + (dlg.m_Kind << 1);
		kind += (dlg.m_EaseIn << 2);
		kind += (dlg.m_EaseOut << 17);
//		m_pCamera->PutAttr(k,m_Frame-1,peg,value,kind);
		m_kinds[attr] = kind;
		if (bTool && (attr == CCamera::CATTR_Y))
			m_kinds[CCamera::CATTR_X] = kind;
		OnApply();
		}
}

void camdlg::OnNextKey()
{
	UINT peg;
	if (m_kind)
		peg = m_nGroup + 1000;
	else
		peg = m_nPeg;
	UINT z = m_pCamera->FindKey(peg, m_Frame-1,TRUE);
	if (z != -1)
		{
		SetFrame(z + 1);
		}
}

void camdlg::OnPrevKey()
{
	UINT peg;
	if (m_kind)
		peg = m_nGroup + 1000;
	else
		peg = m_nPeg;
	UINT z = m_pCamera->FindKey(peg, m_Frame-1,FALSE);
	if (z != -1)
		{
		SetFrame(z + 1);
		}
}

void camdlg::OnClearAllKeys()
{
	if (MyError(IDS_CLEARALL, MB_YESNO) == IDYES)
		{
		int i,c;
		c = m_pCamera->PegCount();
		m_pegcx = m_pegcy = 0.0;
		for (i = 0; i < c; i++)
			m_pCamera->PegCenter(m_pegcx, m_pegcy, i, TRUE);
		c = m_pCamera->GroupCount();
		for (i = 0; i < c; i++)
			m_pCamera->PegCenter(m_pegcx, m_pegcy, i+1000, TRUE);
		m_pCamera->PutAttr(99,0,0,0,0);
		m_pScene->UpdateCache();
		FillInDialog();
		CSketchView * pView = m_pDoc->GetDocView();
		pView->Update(9999,0,30);
		SetCrossHair();
		}
}

void camdlg::OnClearKeys()
{
	UINT i;
	UINT peg;
	UINT Frame = 0;
	if (m_kind)
		peg = m_nGroup + 1000;
	else
		peg = m_nPeg;
	UINT attr = WhichAttr();
	BOOL bBoth = 0;
	if ((m_which == W_N) && m_bFromTool)
		bBoth = 1;
	m_bFromTool = 0;
	CSketchView * pView = m_pDoc->GetDocView();
	if (GetKeyState(VK_CONTROL) < 0)
		{
		m_kinds[attr] = CCamAttr::CKIND_COMPUTE;
		for (i = 0; i < m_Frames;i++)
			m_pCamera->PutAttr(attr, i, peg, 0, CCamAttr::CKIND_COMPUTE);
		if (bBoth)
			{
			attr = WhichAttr(W_E);
			m_kinds[attr] = CCamAttr::CKIND_COMPUTE;
			for (i = 0; i < m_Frames;i++)
				m_pCamera->PutAttr(attr, i, peg, 0, CCamAttr::CKIND_COMPUTE);
			}
		pView->Update(9999,0,30);
		}
	else
		{
		Frame = m_Frame -1;
		m_kinds[attr] = CCamAttr::CKIND_COMPUTE;
		m_pCamera->PutAttr(attr, Frame, peg, 0, CCamAttr::CKIND_COMPUTE);
		pView->Update(m_Frame-1,9999,30);
		for (;Frame;)
			{
			Frame--;
			double dummy;
			UINT kind;
			m_pCamera->GetAttr(attr, Frame,peg,dummy,kind);
			if (kind != CCamAttr::CKIND_COMPUTE)
				break;
			}
		if (bBoth)
			{
			attr = WhichAttr(W_E);
			Frame = m_Frame -1;
			m_kinds[attr] = CCamAttr::CKIND_COMPUTE;
			m_pCamera->PutAttr(attr, Frame, peg, 0, CCamAttr::CKIND_COMPUTE);
			pView->Update(m_Frame-1,9999,30);
			for (;Frame;)
				{
				Frame--;
				double dummy;
				UINT kind;
				m_pCamera->GetAttr(attr, Frame,peg,dummy,kind);
				if (kind != CCamAttr::CKIND_COMPUTE)
					break;
				}
			}
		}
	m_pScene->UpdateCache(Frame);
	FillInDialog();
}

void camdlg::SetFrame(UINT frame)
{
	Packet(3,frame - 1,0);
	CSketchView * pView = m_pDoc->GetDocView();
	if (!pView) return;
	SetCrossHair();
	pView->Update(m_Frame-1,0,9);
}

BOOL camdlg::IsKey(UINT Frame, UINT Level)
{
	if (m_pCamera)
		return m_pCamera->IsKey(Frame,Level);
	else
		return 0;
}

void camdlg::OnGoto()
{
	return;
	/*
	CGotoDlg dlg;
	CRect rect;
	GetDlgItem(IDC_GOTO)->GetWindowRect(&rect);
	dlg.m_x = rect.left;
	dlg.m_y = rect.top;
	dlg.m_Frame = m_Frame;
	dlg.m_Frames = m_Frames;
	if (dlg.DoModal() == IDOK)
		SetFrame(dlg.m_Frame);
		*/
}

void camdlg::OnCamHome()
{
	SetFrame(1);
}

void camdlg::OnCamEnd()
{
	SetFrame(m_Frames);
}

void camdlg::ToolInit(UINT Id)
{
	int which;
//	m_bFromTool = 1;
	int msg = 0;
	if (Id == ID_TOOL_CAM_DALL)
		msg = IDC_CLEARALLKEYS;
	else if (Id == ID_TOOL_CAM_DTHIS)
		msg = IDC_CLEARKEYS;
	else if (Id == ID_TOOL_CAM_NEXT)
		msg = IDC_NEXTKEY;
	else if (Id == ID_TOOL_CAM_PREV)
		msg = IDC_PREVKEY;
	else if (Id == ID_TOOL_CAM_PAN)
		which = W_N;
	else if (Id == ID_TOOL_CAM_ZOOM)
		which = W_SCALE;
	else if (Id == ID_TOOL_CAM_ROTATE)
		which = W_ROT;
	else if (Id == ID_TOOL_CAM_BLUR)
		which = W_BLUR;
	else if (Id == ID_TOOL_CAM_ALPHA)
		which = W_ALPHA;
	else
		return;
	if (msg)
		{
		if (m_bFromTool)
			{
			m_bFromTool = 0;
			return;
			}
		m_bFromTool = 1;
		PostMessage(WM_COMMAND, msg, 0);
		}
	else //if ((which != m_which) || !m_bPanning)
		{
		CSketchView * pView = m_pDoc->GetDocView();
		if ((pView->CurrentFrame() + 1) != m_Frame)
			{
			FlushChanges();
			m_Frame = pView->CurrentFrame()+1;
			FillInDialog();
			}
		AdjIt(which + 256,0);
		SetCrossHair(TRUE);
		}
}

UINT camdlg::ToolSelected()
{
	UINT tool_id = 99;
	if ((m_which == W_N) || (m_which == W_S) ||
			(m_which == W_E) || (m_which == W_W))
		tool_id = ID_TOOL_CAM_PAN;
	else if (m_which == W_SCALE)
		tool_id = ID_TOOL_CAM_ZOOM;
	else if (m_which == W_ROT)
		tool_id = ID_TOOL_CAM_ROTATE;
	else if (m_which == W_BLUR)
		tool_id = ID_TOOL_CAM_BLUR;
	else if (m_which == W_ALPHA)
		tool_id = ID_TOOL_CAM_ALPHA;
	return tool_id;
}

BOOL camdlg::ToolEnabled(UINT Id)
{
	UINT mask = 0;
	if (Id == ID_TOOL_CAM_DALL)
		mask = 2;
	else if (Id == ID_TOOL_CAM_DTHIS)
		mask = 1;
	else if (Id == ID_TOOL_CAM_NEXT)
		mask = 4;
	else if (Id == ID_TOOL_CAM_PREV)
		mask = 8;
	else
		return 1;
	return (m_EnabledMask & mask) ? 1 : 0;
}

