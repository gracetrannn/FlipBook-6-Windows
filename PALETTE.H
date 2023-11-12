#ifndef _PALETTE_H_
#define _PALETTE_H_
#include "clevtbl.h"

class CSketchDoc;
class CLayers;
class CDib;
class CScene;
class CNewPals;

class CPaletteDlg : public CDialog
{
public:
	CPaletteDlg(CWnd* pParent = NULL);	// standard constructor
	~CPaletteDlg();
	//{{AFX_DATA(CSheet)
	enum { IDD = IDD_PALETTE2 };
	//}}AFX_DATA
	BOOL SetColor(UINT index, BYTE r, BYTE g, BYTE b, 
						BYTE alpha = 255);
	BOOL MyCreate(CWnd * pOwner);//int x, int y);
	BOOL HasModel(UINT v = NEGONE);
	WORD Setup(CSketchDoc * pDoc);
	WORD Select(UINT Level);
	void ColorTrap(int x, int y, int code);
	void DoCursor(UINT index);
	void ChangePals();
	void ReceiveCurrentColorChange (COLORREF& inColor);
//	void ChangeColorOrModelKind();
	void UpdatePals(int id = -1);
protected:
	int  PromptChangeColor();
	UINT m_nColorTrap;
	BOOL XlatePoint(CPoint& pt);
	void WindowSize(int which);
	void EditColor(UINT id);
	void EditOne();
	CSketchDoc * m_pDoc;
	CScene * m_pScene;
	UINT m_pal_index;
	UINT m_pindex; // palindex
	UINT m_grad1x;
	UINT m_grad1y;
	UINT m_yoffset;
	UINT m_index;
	UINT m_Level;
	BOOL m_bBusy;
	UINT m_nKind;
	UINT m_width;
	UINT m_height;
	UINT m_pitch;
	
	UINT m_dstx;
	UINT m_dsty;
	UINT m_dstw;
	UINT m_dsth;
	void PaintKind1();
	void PaintKind2();
	CLayers * m_pLayers;
	CDib * m_pDib;
#ifdef FLIPBOOK_MAC
	CWnd m_PaletteView;
	CScrollBar m_ScrollBar;
#endif
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnPaint();
	afx_msg void OnNcPaint();
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnClose();
	afx_msg void OnVScroll(UINT code , UINT pos , CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/);
	afx_msg void OnLButtonDown(UINT /*nFlags*/, CPoint point);
	afx_msg void OnNcLButtonDown(UINT /*nFlags*/, CPoint point);
	afx_msg void OnRButtonUp(UINT /*nFlags*/, CPoint /*point*/);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT /*nFlags*/, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	DECLARE_MESSAGE_MAP()
};

#endif
