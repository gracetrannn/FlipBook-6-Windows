
#if !defined(MYDIALOGS_H)

#define MYDIALOGS_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSketchView;
class CSketchDoc;
class CPaletteDlg;
#include "vfw.h"
#include "clevtbl.h"
class CNewPals;

class CMyDialog : public CDialog
{
public:
	CMyDialog (UINT idd = 0,CWnd* pParent = NULL);
protected:
	virtual void EndModalLoop(int code);
	BOOL m_bSheet;
    virtual BOOL OnInitDialog();
	afx_msg BOOL OnToolTip(UINT id, NMHDR* pTTTStruct, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};

#endif
