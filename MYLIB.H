#ifndef _MYLIB_H_
#define _MYLIB_H_

class CSketchDoc;
class CScene;
class CNewPals;

class CMyLibDlg : public CDialog
{
public:
	CMyLibDlg(CWnd* pParent = NULL);	// standard constructor
	~CMyLibDlg();
	enum { IDD = IDD_MYLIB };
	BOOL Load(CWnd * pOwner, LPCSTR name);
	BOOL Setup(CSketchDoc * pDoc, BOOL bForce = 0);
	void DoCursor(UINT index);
	void LibName(CString & Name);
	void FileName(CString & Name);
	UINT GetInfo();
	void SetInfo(UINT v);
	BOOL Switch(BOOL bSwitch = 0);
static 	UINT SaveLibrary(CScene * pScene, LPCSTR filename,
					UINT start_frame, UINT end_frame, 
					UINT start_level, UINT end_level, UINT Flags);
protected:
	void SetName(LPCSTR filename);
	void SetRows();
	CString m_name;	// for title and menus
	CString m_filename;	// orig file name
	int ReadFile(LPCSTR name);
	BOOL DoThumb(CDC & dc, LPRECT myrect, UINT x, UINT y);
	UINT Index(UINT x, UINT y);
	CSketchDoc * m_pDoc;
	CScene * m_pScene;
	UINT m_flags;
	UINT m_yoffset;		//vertical scroll offset
	UINT m_orig_w;
	UINT m_orig_h;
	UINT m_srcw;		// largest cell size
	UINT m_srch;
	UINT m_nCells;		// total cells
	UINT m_desired_width;
	UINT m_curw;
	UINT m_curh;
	UINT m_lines;		// lines of cells
	UINT m_nRows;		// rows of cells
	UINT m_nColumns;	// 
	UINT m_levels;		// original nbr of levels
	UINT m_frames;		// and frames
	UINT m_ety;			// size of title bar
	UINT m_eby;			// size of frame
	UINT m_ex;		
	UINT m_winw;
	UINT m_winh;
	UINT m_neww;
	UINT m_newh;
	UINT m_oldw;
	UINT m_oldh;
	BYTE * m_pCur;
	BYTE * m_pTmp;
	BYTE * * m_pBufs;
	UINT	m_nPaint;
	BOOL	m_bPainted;
	BOOL ReCalcWindow(BOOL bUseClient);
	void ReScale();
	void LibSettings();
#ifdef FLIPBOOK_MAC
	CWnd m_PaletteView;
	CScrollBar m_ScrollBar;
#endif
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnPaint();
	afx_msg void OnClose();
	afx_msg void OnVScroll(UINT code , UINT pos , CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonUp(UINT /*nFlags*/, CPoint /*point*/);
	afx_msg void OnLButtonDown(UINT /*nFlags*/, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnTimer(UINT nTimerID);
	DECLARE_MESSAGE_MAP()
};

#endif
