
#include "stdafx.h"
#include "myspin.h"

BOOL CMySpin::Create(CWnd * pParent, UINT Id, UINT val, UINT min, UINT max)
{
	DWORD dwStyles=0;
	dwStyles |= UDS_ALIGNRIGHT;     // Control is placed to the right of buddy, if set
	dwStyles |= UDS_ARROWKEYS;      // Up/Down arrow keys inc/decrement, if set
	dwStyles |= UDS_NOTHOUSANDS;
//	dwStyles |= UDS_AUTOBUDDY;
	dwStyles |= UDS_SETBUDDYINT;    // Control updates buddy edit with position, if set

	dwStyles |= WS_VISIBLE | WS_CHILD;

	CRect ignored_rect; // for Mac/Xcode compatibility
	
//	CSpinButtonCtrl::Create(dwStyles, (CRect)NULL, pParent, Id);
	CSpinButtonCtrl::Create(dwStyles, ignored_rect, pParent, 0);
	SetBuddy(pParent->GetDlgItem( Id ));
	SetRange( min, max);
	SetPos(val);
	return TRUE;
}
void CMySpin::Change()
{
	int val, min, max;
	GetRange(min,max);
	val = GetPos();
	DPF("val:%d,min:%d,max:%d",val,min,max);
	if (val == 65537)
		val = 0;
	if (val == 66536)
		SetPos(max);
	else if (val == 65546)
		SetPos(min);
	else if (val < min)
		SetPos(min);
	else if (val > max)
		SetPos(max);
}
