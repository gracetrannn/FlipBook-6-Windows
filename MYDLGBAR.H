#ifndef _CMYDLGBAR_H_
#define _CMYDLGBAR_H_
class CMainFrame;
class CMyDialogBar;
class CMySlider2 : public CSliderCtrl
{
public:
	void Setup(int nId, UINT pos, 
			UINT min, UINT max){
			SetRange(min,max);
			m_id = nId;
			SetPos(pos);
			};
	void Assign(UINT v) { SetPos(v);};
	int m_id;
};

class CMySlider3 : public CSliderCtrl
{
public:
	CMySlider3::CMySlider3() { m_bTracking = 0; }
	void Setup(int nId, UINT pos, 
			UINT min, UINT max){
			SetRange(min,max);
			m_id = nId;
			SetPos(pos);
			};
	void TrackPoint(CPoint point);
	void Assign(UINT v) { SetPos(v);};
	int m_id;
	BOOL m_bTracking;
	CMyDialogBar * m_pParent;
	void OnCustomDraw(NMHDR *pNotifyStruct, LRESULT *result);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
DECLARE_MESSAGE_MAP()

};

class CMyDialogBar : public CDialogBar
{
//DECLARE_DYNAMIC(CMyDialogBar)
public:
	CMyDialogBar();
	~CMyDialogBar();
	int m_w;
	int m_h;
	void Setup(CMainFrame * pParent);
	void SetRange(UINT max);
	BOOL SetFrame(UINT frame);
	BOOL LoadBitmap32(UINT id);
	CMySlider3 m_slider;
	CMySlider2 m_rate;
	CMySlider2 m_volume;
//	CButton m_goto;
	CButton m_start;
	CButton m_stop;
	CMainFrame * m_pFrame;
	BOOL m_bPlaying;
	void ChangeSliderPos();
	void DoReturn();
	void SetStart();
	void SetStop();
protected:
	UINT FrameToSlide(UINT frame);
	UINT SlideToFrame(UINT pos);
	BYTE * m_pButtons;
	void GetButtons();
	virtual BOOL PreTranslateMessage(MSG *pMsg);
	UINT m_factor;
	UINT m_max_frame;
	int m_orig_width;
	int m_cur_width;
	UINT m_frame;
//	UINT m_code; // 0 is normal, 1 is shift down, 2 is shift up
	CWnd * GetRect(int id, CRect & rect);
	void MoveControl(int id, int delta);
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar );
	afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg BOOL OnToolTip(UINT id, NMHDR* pTTTStruct, LRESULT* pResult);
	afx_msg void OnDrawItem(int nId, LPDRAWITEMSTRUCT lpDrawItemStruct);
	DECLARE_MESSAGE_MAP()
};



#endif
