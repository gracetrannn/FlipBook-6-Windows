#ifndef _PUCK_H_
#define _PUCK_H_

class CSketchDoc;
class CScene;
class CNewPals;
class CSketchView;

class CPuckDlg : public CDialog
{
public:
	CPuckDlg(CWnd* pParent = NULL);	// standard constructor
	~CPuckDlg();
	enum { IDD = IDD_PUCK };
	BOOL MyCreate(CWnd * pOwner);//int x, int y);
	BOOL Setup(CSketchDoc * pDoc);
	void SetMode(BOOL bColor) {};
	UINT Width() {return m_orig_w;};
	UINT Height() {return m_orig_h;};
	void UpdateInfo();
protected:
	void DoThumb(CDC & dc, CRect& myrect, UINT index);
	CSketchDoc * m_pDoc;
	UINT m_init_r;
	UINT m_init_d;
	UINT m_orig_w;
	UINT m_orig_h;
	CPoint m_point;
	BYTE * m_pBuf; // orig thumbs plus pal
	BYTE * m_pCur;
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnClose();
	afx_msg void OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/);
	afx_msg void OnLButtonDown(UINT /*nFlags*/, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
};

class CZoomerDlg : public CDialog
{
public:
	CZoomerDlg(CWnd* pParent = NULL);	// standard constructor
	~CZoomerDlg();
	enum { IDD = IDD_ZOOMER };
	BOOL MyCreate(CWnd * pOwner);//int x, int y);
	BOOL Setup(CSketchDoc * pDoc);
	void SetMode(BOOL bColor) {};
	UINT Width() {return m_orig_w;};
	UINT Height() {return m_orig_h;};
protected:
	CSketchDoc * m_pDoc;
	CSketchView * m_pView;
	CString m_title;
	UINT m_nNum;
	UINT m_nDen;
	UINT m_orig_w;
	UINT m_orig_h;
	CPoint m_point;
	CPoint m_center;
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnFit();
	afx_msg void OnHand();
	afx_msg void On100();
	afx_msg void OnZoom();
	afx_msg void OnZoomer();
	afx_msg void OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/);
	afx_msg void OnLButtonDown(UINT /*nFlags*/, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
};

#endif
