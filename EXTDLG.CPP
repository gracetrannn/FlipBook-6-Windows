#include "stdafx.h"
#include "sketch.h"
//#include "psapi.h"
#include "dialogs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define TIMER_ID 15
#define TIMER_DELAY 1000
CExternalDlg::CExternalDlg(CWnd* pParent /*=NULL*/)
	: CMyDialog(CExternalDlg::IDD, pParent)
{
	m_name[0] = 0;
}

BOOL CExternalDlg::OnInitDialog()
{
	CMyDialog::OnInitDialog();
	CenterWindow();
	if (m_name[0])
		{
		m_nState = 0;
		int i, j;
		for (j=0, i = 0;m_name[i];i++)
			if (m_name[i] == '.') j = i;
		if (j) m_name[j] = 0;
		char buf[300];
		strcpy(buf,"Looking for Edit Window:");
		strcat(buf,m_name);
		SetWindowText(buf);
//		SetWindowText("Looking for Edit Window");
		VERIFY(SetTimer(TIMER_ID, TIMER_DELAY, NULL) == TIMER_ID);
		}
	else
		m_nState = 2;
	return FALSE;
}

BEGIN_MESSAGE_MAP(CExternalDlg, CMyDialog)
	ON_WM_TIMER()
END_MESSAGE_MAP()

void CExternalDlg::OnTimer(UINT nTimerID)
{
	if (nTimerID != TIMER_ID)
		return;
	if (!m_nState)
		{
		if (FindWindow())
			{
			SetWindowText("Waiting for Edit To Finish");
			m_nState = 1;
			}
		}
	else if (m_nState == 1)
		{
		DPF("checking");
		char title[256];
		::GetWindowText(m_hWnd,title,sizeof(title));
//		DPF("title:%s|",title);			
		int i;
		for (i = 0; m_name[i]; i++)
			if ((m_name[i] | 32) != (title[i] | 32))
				break;
		if (m_name[i])
			{
			EndDialog(IDOK);
			}
		}
}

BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam)
{
	return ((CExternalDlg *)lParam)->CheckWindow(hWnd);
}

BOOL CExternalDlg::CheckWindow(HWND hWnd)
{
	char title[256];
	::GetWindowText(hWnd,title,sizeof(title));
	int i;
	for (i = 0; m_name[i]; i++)
		if ((m_name[i] | 32) != (title[i] | 32))
			break;
	if (!m_name[i])
		{
		m_hWnd = hWnd;
		return FALSE;
		}
	else
		return TRUE;
}


BOOL CExternalDlg::FindWindow()
{
	m_hWnd = 0;
	EnumChildWindows(::GetDesktopWindow(), &EnumChildProc, (LPARAM)this);
	return m_hWnd ? 1 : 0;
}
