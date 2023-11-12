#if !defined(AFX_DIBSTATIC_H__40E4BDE3_BD3A_11D1_89F4_0000E81D3D27__INCLUDED_)
#define AFX_DIBSTATIC_H__40E4BDE3_BD3A_11D1_89F4_0000E81D3D27__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CDIBStatic window

class CDIBStatic : public CStatic
{
// Construction
public:
	CDIBStatic();

// Attributes
public:

public:
	BOOL IsValidDib() const { return (m_pDIB ? 1 : 0); }

// Operations
public:
	BOOL SetDib(BYTE * pData);
	void UpdateDib();
// Implementation
public:
	virtual ~CDIBStatic();

	BYTE * m_pDIB;
protected:
	void ClearDib();
	void PaintDib(BOOL bDibValid);

	// Generated message map functions
protected:
	//{{AFX_MSG(CDIBStatic)
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
//	afx_msg BOOL OnQueryNewPalette();
//	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIBSTATIC_H__40E4BDE3_BD3A_11D1_89F4_0000E81D3D27__INCLUDED_)
