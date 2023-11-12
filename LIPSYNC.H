#ifndef _LIPSYNC_H_
#define _LIPSYNC_H_

class CSketchDoc;
class CScene;
class CNewPals;

class CLipSyncDlg : public CDialog
{
public:
	CLipSyncDlg(CWnd* pParent = NULL);	// standard constructor
	~CLipSyncDlg();
	enum { IDD = IDD_LIPSYNC };
	BOOL MyCreate(CWnd * pOwner);//int x, int y);
	BOOL Setup(CSketchDoc * pDoc);
	void DoCursor(UINT index);
protected:
	int GetResource();
//	int ReadFile(LPCSTR name);
	void DoThumb(CDC & dc, CRect& myrect, UINT index);
	CSketchDoc * m_pDoc;
	CScene * m_pScene;
	UINT m_yoffset;
	UINT m_orig_w;
	UINT m_orig_h;
	UINT m_srcw;
	UINT m_srch;
	UINT m_nCells;
//	UINT m_width;
//	UINT m_height;
//	UINT m_pitch;
	UINT m_curw;
	UINT m_curh;
	BOOL m_bBusy;
	BYTE * m_pBuf; // orig thumbs plus pal
	BYTE * m_pCur;
	void ReScale(UINT boxw);
#ifdef FLIPBOOK_MAC
	CWnd m_PaletteView;
	CScrollBar m_ScrollBar;
#endif
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnPaint();
//	afx_msg void OnNcPaint();
//	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnClose();
	afx_msg void OnVScroll(UINT code , UINT pos , CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/);
	afx_msg void OnLButtonDown(UINT /*nFlags*/, CPoint point);
//	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
//	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	DECLARE_MESSAGE_MAP()
};

#endif
