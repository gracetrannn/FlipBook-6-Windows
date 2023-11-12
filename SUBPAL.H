#ifndef _SUBPALETTE_H_
#define _SUBPALETTE_H_


class CSubPalDlg : public CDialog
{
public:
	CSubPalDlg(CWnd* pParent = NULL);	// standard constructor
	~CSubPalDlg();
	//{{AFX_DATA(CSheet)
	enum { IDD = IDD_SUBPALETTE };
	//}}AFX_DATA
	BOOL MyCreate(CWnd * pOwner);//int x, int y);
//	WORD Setup(CSketchDoc * pDoc);
//	WORD Select(UINT Level);
//	void ColorTrap(int x, int y);
//	void SetPosition();
	void DoCursor(UINT index);
	void ChangePals();
	COLORREF GetColor(UINT Index) { return m_colors[Index];};
	COLORREF GetColor() { return m_colors[m_index];};
	void SetColor(UINT idd, UINT r, UINT g, UINT b, UINT alpha);
//	void ReceiveCurrentColorChange (COLORREF& inColor);
//	void ChangeColorOrModelKind();
	void SetName(UINT Index, LPCSTR txt) {strcpy(m_names[Index],txt);};
protected:
	UINT m_rows;
	UINT m_cols;
	UINT m_width;
	UINT m_height;
	UINT m_font;
	UINT m_index;
	COLORREF m_colors[18];
	char m_names[18][40];
/*
	UINT m_nColorTrap;
	BOOL m_bSavedColor;
	UINT m_SavedKind;
	CPoint m_SavedPoint1;
	CPoint m_SavedPoint2;
	COLORREF m_SavedColor1;
	COLORREF m_SavedColor2;
	BOOL XlatePoint(CPoint& pt);
	void WindowSize(int which);
	void GradIt();
	void ChangeIt(UINT id);
//	CMyButton m_button;
//	CSketchDoc * m_pDoc;
	UINT m_pindex; // palindex
	UINT m_cindex; // adv color index
	UINT m_yoffset;
	UINT m_index;
	UINT m_Level;
	BOOL m_bBusy;
	UINT m_nKind;
	UINT	m_width;
	UINT	m_height;
	UINT	m_pitch;
	
	UINT m_dstx;
	UINT m_dsty;
	UINT m_dstw;
	UINT m_dsth;
	void PaintKind1();
	void PaintKind2();
	CLayers * m_pLayers;
	CDib * m_pDib;
*/
//#ifdef FLIPBOOK_MAC
//	CWnd m_PaletteView;
//#endif
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
//	afx_msg void OnNcPaint();
//	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnClose();
//	afx_msg void OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/);
	afx_msg void OnLButtonDown(UINT /*nFlags*/, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	DECLARE_MESSAGE_MAP()
};

#endif
