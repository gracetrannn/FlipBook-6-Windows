
#include "stdafx.h"
#include "mydlgbar.h"
#include "mainfrm.h"
#include "cscene.h"
#include "myview.h"
#include "mysound.h"
BEGIN_MESSAGE_MAP(CMySlider3, CSliderCtrl)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
END_MESSAGE_MAP()


#define BTN_CNT 9

void CMySlider3::OnCustomDraw(NMHDR *pNotifyStruct, LRESULT *result)
{
    NMCUSTOMDRAW *pNmcd = (NMCUSTOMDRAW *)pNotifyStruct;

    switch( pNmcd->dwDrawStage ) {
    case CDDS_PREPAINT:
        if( GetStyle() & TBS_ENABLESELRANGE ) {
            *result = CDRF_NOTIFYITEMDRAW;
        }
        break;
    case CDDS_ITEMPREPAINT:
        switch( pNmcd->dwItemSpec ) {
        case TBCD_CHANNEL: {
            CDC *pDC = CDC::FromHandle(pNmcd->hdc);
            CRect rcChannel;
            GetChannelRect(&rcChannel);
         //   rcChannel.DeflateRect(6, 3);
            rcChannel.DeflateRect(1, 1);

            int min, max, selmin, selmax;
            GetRange(min, max);
            GetSelection(selmin, selmax);
            max -= min;
            selmin = (int)(((double)(selmin - min) / max * rcChannel.Width()) +
0.5) + rcChannel.left;
            selmax = (int)(((double)(selmax - min) / max * rcChannel.Width()) +
0.5) + rcChannel.left;
            rcChannel.left = selmin;
            rcChannel.right = selmax;

            pDC->FillSolidRect(rcChannel, RGB(100,100,200));
			//	static_cast<CXTPMDIFrameWnd*>(AfxGetMainWnd())->GetCommandBars()->GetPaintManager()->GetXtremeColor(COLOR_HIGHLIGHT));
            pDC->ExcludeClipRect(rcChannel);
            *result = CDRF_DODEFAULT | CDRF_NOTIFYPOSTPAINT;
            break; }
        }
        break;
    case CDDS_ITEMPOSTPAINT:
        switch( pNmcd->dwItemSpec ) {
        case TBCD_CHANNEL: {
            CDC *pDC = CDC::FromHandle(pNmcd->hdc);
            pDC->SelectClipRgn(NULL);
            break; }
        }
        break;
    }
}


//IMPLEMENT_DYNAMIC(CMyDialogBar, CDialogBar)


CMyDialogBar::CMyDialogBar()
{
	m_orig_width = 0;
	m_frame = 1;
	m_pButtons = 0;
	m_bPlaying = 0;
}

CMyDialogBar::~CMyDialogBar()
{
	delete [] m_pButtons;
}

void CMyDialogBar::DoDataExchange(CDataExchange* pDX)
{
	CDialogBar::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLD_SLIDER, m_slider);
	DDX_Control(pDX, IDC_SLD_RATE, m_rate);
	DDX_Control(pDX, IDC_SLD_VOLUME, m_volume);
	DDX_Text(pDX, IDC_SLD_FRAME, m_frame);
}


BEGIN_MESSAGE_MAP(CMyDialogBar, CDialogBar)
	//{{AFX_MSG_MAP(CSlider)
	ON_WM_HSCROLL()
	ON_WM_SIZE()
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTip)
	ON_WM_DRAWITEM()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


CWnd * CMyDialogBar::GetRect(int id, CRect & rect)
{
	CWnd * pWnd = GetDlgItem(id);
	pWnd->GetWindowRect(&rect);
	ScreenToClient(rect);
	return pWnd;
}

void CMyDialogBar::MoveControl(int id, int delta)
{
	CRect rcMe;
	GetWindowRect(&rcMe);
	DPR("me",&rcMe);
	
	CRect rcControl;
	CWnd * pWnd = GetDlgItem(id);
	pWnd->GetWindowRect(&rcControl);
	ScreenToClient(rcControl);
	rcControl.left += delta;
	BOOL bRes = pWnd->SetWindowPos(&wndTop, rcControl.left,rcControl.top,
						0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOZORDER);
//	DPR("play",&crPlay);
}

void CMyDialogBar::OnSize(UINT type, int cx, int cy)
{
	CDialogBar::OnSize(type, cx, cy);
	if (!m_orig_width)
		{
DPF("size, t:%d,cx:%d,cy:%d",type,cx,cy);
		m_orig_width = cx;
		m_cur_width = cx;
		}
	else if (!cx || (cx == m_cur_width))
		return;
	int delta;
	CRect rcControl;
	CWnd * pWnd = GetDlgItem(IDC_SLD_START);
	if (!pWnd)
		return;
	pWnd->GetWindowRect(rcControl);
	ScreenToClient(rcControl);
	int d1 = rcControl.left;
	int d2 = rcControl.right;
//	int w1 = rcControl.right - d1;
	pWnd = GetDlgItem(IDC_SLD_STOP);
	pWnd->SetWindowPos(&wndTop, cx - d2,rcControl.top,
						0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOZORDER);
	pWnd = GetRect(IDC_SLD_SLIDER, rcControl);
	int d3 = rcControl.left;
	pWnd->SetWindowPos(&wndTop, 0,0,cx - d3 - d3,rcControl.bottom - rcControl.top,
						SWP_NOMOVE | SWP_SHOWWINDOW | SWP_NOZORDER);

	GetRect(IDC_SLD_PLAY, rcControl);
	int center = cx/ 2;
	delta = center - (rcControl.left + rcControl.right) / 2;
	if (!delta)
		return;
	DPF("delta:%d", delta);
//delta = 1;
	MoveControl(IDC_SLD_BEGIN,delta);
	MoveControl(IDC_SLD_BACK,delta);
	MoveControl(IDC_SLD_PLAY,delta);
	MoveControl(IDC_SLD_FWD,delta);
	MoveControl(IDC_SLD_END,delta);
	MoveControl(IDC_SLD_RATE,delta);
	MoveControl(IDC_SLD_LOOP,delta);
	MoveControl(IDC_SLD_FRAME,delta);
	MoveControl(IDC_SLD_VOLUME,delta);
	MoveControl(IDC_SLD_VOLTXT,delta);
	MoveControl(IDC_SLD_FPS,delta);
	MoveControl(ID_RATE_UP,delta);
	MoveControl(ID_RATE_DOWN,delta);

	m_cur_width = cx;
	DPF("new width:%d",m_cur_width);
	




/*
	CFrameWnd::OnSize(type, cx, cy);
//	theApp.m_bMaximized = (nType == SIZE_MAXIMIZED);
//	if (nType == SIZE_RESTORED)
//		GetWindowRect(theApp.m_rectInitialFrame);
    static BOOL bAlreadyInsideThisProcedure = FALSE;
    if (bAlreadyInsideThisProcedure)
        return;

    if (!::IsWindow(m_hWnd))
        return;

    // Start re-entry blocking
    bAlreadyInsideThisProcedure = TRUE;

	if (type == SIZE_MINIMIZED)
		{
//		delete m_pToolBar;
//		m_pToolBar = 0;
		}
	else if (m_pToolBar)
		{
		BOOL bNow = m_pToolBar->TotalWidth() > (UINT)cx ? 1 : 0;
		if ((bNow != m_pToolBar->Double()) || !m_pToolBar->TotalWidth())
			 MakeToolBar();
		}
//	else
//		MakeToolBar();

    // End re-entry blocking
    bAlreadyInsideThisProcedure = FALSE;
*/
}


void CMyDialogBar::Setup(CMainFrame * pParent)
{
	m_factor = 9999;
	if (m_slider.m_hWnd)
		m_slider.SetRange(0,m_factor);
#ifdef FLIPBOOK_MAC
	m_slider.SubclassDlgItem (IDC_SLD_SLIDER, this);
	m_slider.SubclassDlgItem (IDC_SLD_FRAME, this);
	//m_slider.Create (0, r, this, IDC_SLD_SLIDER);
#endif
	m_slider.m_pParent = this;
	m_pFrame = pParent;
	if (pParent->m_pScene)
		{
		m_slider.m_id = IDC_SLD_SLIDER;
		m_rate.Setup(IDC_SLD_RATE,pParent->m_pScene->FrameRate(),1,50);
		int volume = 0;
		if (m_pFrame->m_pSound)
			volume = m_pFrame->m_pSound->Volume(99);
		m_volume.Setup(IDC_SLD_VOLUME,100-volume,0,200);
		}
	GetButtons();
	UpdateData(FALSE);
}

void CMyDialogBar::SetRange(UINT max)
{
	DPF("slider set range:%d",max);
//	m_slider.SetRange(0,max-1);
	m_max_frame = max - 1;
	if (m_frame >= m_max_frame)
		{
		m_frame = m_max_frame;
		}
	m_slider.ModifyStyle( 0, TBS_FIXEDLENGTH );
	m_slider.SendMessage(TBM_SETTHUMBLENGTH,18,0);
}

UINT CMyDialogBar::FrameToSlide(UINT frame)
{
	UINT slide = frame * m_factor;
	if (m_max_frame)
		slide /= m_max_frame;
	else
		slide = 0;
	return slide;
}

UINT CMyDialogBar::SlideToFrame(UINT pos)
{
	UINT frame = pos * m_max_frame;
	frame += m_factor / 2;
	frame /= m_factor;
	return frame;
}

BOOL CMyDialogBar::SetFrame(UINT frame)
{
	DPF("slider set frame:%d",frame);
	frame--;
	BOOL bRes = frame != m_frame ? 1 : 0;
	m_frame = frame;
	if (!m_slider.m_bTracking)
	m_slider.SetPos(FrameToSlide(frame));
	SetDlgItemInt(IDC_SLD_FRAME, frame+1);//)->SetWindowTextA(buf);
	DPF("after set position");
	return bRes;
}
/*   
BOOL CMyDialogBar::LoadBitmap32(UINT id)
{

#ifndef FLIPBOOK_MAC
	CImageList imagelist;
	CBitmap		bm;
	int w = 20;
	int h = 20;
	int c = SLIDER_COUNT;
	SetSizes(CSize(w+7,h+6),CSize(w,h));
	VERIFY (bm.Attach (::LoadImage (::AfxFindResourceHandle(
 				MAKEINTRESOURCE (id), RT_BITMAP),
 				MAKEINTRESOURCE (id), IMAGE_BITMAP, 0, 0,
 				(LR_DEFAULTSIZE | LR_CREATEDIBSECTION))));
// ::ReplaceBackgroundColor (bm);

	VERIFY (imagelist.Create (w,h,ILC_COLOR24|ILC_MASK,c,0));
	VERIFY (imagelist.Add (&bm, RGB (255, 255, 255)) != -1);
	bm.Detach();

	CToolBarCtrl&	barCtrl = GetToolBarCtrl();
	barCtrl.SetImageList (&imagelist);
	imagelist.Detach();
#endif
	
	return 1;
}
*/

void CMyDialogBar::ChangeSliderPos()
{
	UINT pos = m_slider.GetPos();
	UINT frame = SlideToFrame(pos);
	DPF("slider pos:%d, frm:%d,%d",pos,frame,m_frame);
	if (frame != m_frame)
		{
		m_frame = frame;
		CSketchView * pView = (CSketchView *)m_pFrame->GetActiveView();
		if (pView)
//				pView->Update(frame,nCode == SB_ENDSCROLL ? 1 : 0,8);
			pView->Update(frame,0,8);
		}
	PostMessage(WM_COMMAND, ID_VCR_SLIDER_STOP, 0);
}

void CMyDialogBar::OnHScroll( UINT nCode, UINT nPos, CScrollBar* pScrollBar )
{
	CMySlider2 * pSlider = (CMySlider2 *)pScrollBar;
	CRect Rect;
	CPoint Point;
	GetCursorPos(&Point);
	int min, max;
	pSlider->GetRange(min, max);
//	pSlider->GetClientRect(Rect);
	pSlider->ScreenToClient(&Point);
	DPF("hscrool, min:%d,max:%d",min,max);
//	CRect rcChan;
	pSlider->GetChannelRect(Rect);
	//DPF("ch,x:%d,y:%d,w:%d,h:%d",rcChan.left,rcChan.top,rcChan.right,rcChan.bottom);
#if !MAC
	if (Rect.PtInRect(Point)) /// allows seting pos with click
		{
		int ww = Rect.right - Rect.left;
DPF("rectw:%d,x:%d",ww,Point.x-Rect.left);
		if (ww)
		{
			int z = ((Point.x - Rect.left) * (max - min) + ww / 2) / ww;
			z += min;
			pSlider->SetPos(z);
			}
//		return;
		}
#endif

	if (pSlider->m_id == IDC_SLD_SLIDER)
		{
		ChangeSliderPos();
		}
	else if (pSlider->m_id == IDC_SLD_RATE)
		{
		UINT pos = m_rate.GetPos();
		m_pFrame->SetFrameRate(pos);
		DPF("hscroll, code:%d, pos:%d,act:%d",nCode, nPos,pos);
		}
	else if (pSlider->m_id == IDC_SLD_VOLUME)
		{
		UINT pos = m_volume.GetPos();
		m_pFrame->m_pSound->Volume(99, pos - 100);
		}
}


BOOL CMyDialogBar::OnToolTip(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
#ifndef FLIPBOOK_MAC
    TOOLTIPTEXT *pTTT = (TOOLTIPTEXT*)pNMHDR;
    UINT nID = pNMHDR->idFrom;
    if(pTTT->uFlags & TTF_IDISHWND)
    {
        // idFrom is actually the HWND of the tool
        nID = ::GetDlgCtrlID((HWND)nID);
        if(nID)
        {
		CString buf;
		char msg[100];
		if (buf.LoadString(nID))
			{
			strcpy(msg, (LPCSTR)buf);
			LPSTR p = msg;
			if (p[0] == '#')
				{
				if (IsDlgButtonChecked(nID))
					p++;
				else
					{
					buf.LoadString(nID+1);
					strcpy(msg, (LPCSTR)buf);
					p++;
					}
				}
			CWnd * pWnd = CWnd::FromHandle(pNMHDR->hwndFrom);
			pWnd->SendMessage(TTM_SETMAXTIPWIDTH, 0, 300);
			pWnd->ModifyStyle(0, 0x40,0);//TTS_BALLOON,0);
//			::SendMessage(pNMHDR->hwndFrom, TTM_SETMAXTIPWIDTH, 0, 300);
			pTTT->lpszText = (LPSTR)p;
			pTTT->hinst = 0;
            return(TRUE);
			}
        }
    }
#endif
    return(FALSE);
}


BOOL CMyDialogBar::PreTranslateMessage(MSG* pMsg) 
{
//if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_TAB || pMsg->wParam ==
//VK_RETURN))
//return PreTranslateInput(pMsg);
	if( pMsg->message == WM_KEYDOWN )
		{
		if (GetFocus() == GetDlgItem(IDC_SLD_FRAME))
			{
			if (pMsg->wParam == VK_BACK ||
				pMsg->wParam == VK_RIGHT ||
				pMsg->wParam == VK_UP ||
				pMsg->wParam == VK_DOWN ||
				pMsg->wParam == VK_LEFT)
				{
#ifndef FLIPBOOK_MAC
           		 ::TranslateMessage(pMsg);
           		 ::DispatchMessage(pMsg);
#endif
            	return TRUE;                    // DO NOT process further
				}
			else if (pMsg->wParam == VK_RETURN)
				{
				m_pFrame->PostMessage(WM_COMMAND, ID_FRAME_ENTER_KEY);
				return TRUE;
				}
        	}
		}

    return CDialogBar::PreTranslateMessage(pMsg);
}	


void CMyDialogBar::OnDrawItem(int nId, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if (lpDrawItemStruct->itemAction != ODA_DRAWENTIRE)	
		return;
	DPF("DrawItem:%d",nId);
	CWnd * pWnd = GetDlgItem(nId);
	if (!pWnd) return;
	CSketchView * pView = (CSketchView *)m_pFrame->GetActiveView();
	if (!pView)
		return;
	int z;
	if (nId == IDC_SLD_START)
		z = 7;
	else if (nId == IDC_SLD_STOP)
		z = 8;
	else if (nId == IDC_SLD_BEGIN)
		z = 0;
	else if (nId == IDC_SLD_END)
		z = 3;
	else if (nId == IDC_SLD_BACK)
		z = 6;
	else if (nId == IDC_SLD_FWD)
		z = 5;
	else if (m_bPlaying = pView->Playing())
		z = 1;
	else
		z = 2;
	CWindowDC appDC(pWnd);
	CRect rect;
	pWnd->GetClientRect(rect);
	int w = rect.Width();
	int h = rect.Height();
	int x =	rect.TopLeft().x + ( w - m_w) / 2;
	int	y = rect.TopLeft().y + (h - m_h) / 2;
	int bSuccess = ::StretchDIBits(appDC.m_hDC,x,y,m_w,m_h,
				z * m_w, 0, m_w,m_h,m_pButtons + 40,
				(LPBITMAPINFO)m_pButtons, DIB_RGB_COLORS, SRCCOPY);
	DPF("w:%d,h:%d",w,h);
}

void CMyDialogBar::GetButtons()
{
	delete [] m_pButtons;
	int size;
	m_w = 20;
	m_h = 20;
	size = m_h * 4 * ((3 * m_w + 3) / 4);
	m_pButtons = new BYTE[sizeof(BITMAPINFOHEADER)+ BTN_CNT * size];
	LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)(m_pButtons);
	lpBI->biSize			= sizeof(BITMAPINFOHEADER);
	lpBI->biWidth			= BTN_CNT * m_w;
	lpBI->biHeight			= m_h;
	lpBI->biPlanes			= 1;
	lpBI->biBitCount		= 24;
	lpBI->biCompression		= BI_RGB;
	lpBI->biSizeImage		= BTN_CNT*size;
	lpBI->biXPelsPerMeter	= 0;
	lpBI->biYPelsPerMeter	= 0;
	lpBI->biClrUsed			= 0;
	lpBI->biClrImportant	= 0;
	int result = 0;
	BYTE * pData = m_pButtons + lpBI->biSize;
	int idd = IDB_SLIDER;
	HRSRC hRes = FindResource(AfxGetApp()->m_hInstance,
					MAKEINTRESOURCE(idd),RT_BITMAP);
	if (!hRes)
		return;
	HGLOBAL hand = LoadResource(NULL , hRes);    
	if (!hand)
		return;
	BYTE * pSrc = (BYTE *) LockResource(hand);       
	if (!pSrc)
		return;
	lpBI = (LPBITMAPINFOHEADER)pSrc;
	UINT ww = lpBI->biWidth;
	UINT hh = lpBI->biHeight;
	UINT dd = lpBI->biBitCount;
//	DPF("c:%d,src %d,%d,%d", c,ww,hh,dd);
	ASSERT(ww == BTN_CNT * m_w);
	ASSERT(hh == m_w);
	ASSERT(dd == 24);
	pSrc +=  40;	// to bitmap data
	UINT x, y, sp;
 	const COLORREF		buttonColor = (::GetSysColor (COLOR_BTNFACE));
	UINT rr = GetRValue (buttonColor);
	UINT gg = GetGValue (buttonColor);
	UINT bb = GetBValue (buttonColor);
	sp = 4 * ((3 * ww + 3) / 4);
	memmove(pData, pSrc, BTN_CNT*size);
	for (y = 0; y < hh; y++)
	for (x = 0; x < ww; x++)
		{
		if ((pSrc[sp*y+3*x+0] == 200) && 
					(pSrc[sp*y+3*x+1] == 200) &&
					(pSrc[sp*y+3*x+2] == 200))
			{
			pData[sp*y+3*x+0] = bb;
			pData[sp*y+3*x+1] = gg;
			pData[sp*y+3*x+2] = rr;
			}
		}
//	memmove(pData, pSrc, 7*size);
}
/*
void CMainFrame::DoHeaderIcon(CDC * pDC, int which,LPCRECT rect, BOOL bSel)
{
	LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)(m_pHdrIcons);
	int x,y,w,h;
	w = 16;
	h = 16;
	lpBI->biSize			= sizeof(BITMAPINFOHEADER);
	lpBI->biWidth			= w;
	lpBI->biHeight			= h;
	lpBI->biPlanes			= 1;
	lpBI->biBitCount		= 24;
	lpBI->biCompression		= BI_RGB;
	lpBI->biSizeImage		= 3*w*h;
	lpBI->biXPelsPerMeter	= 0;
	lpBI->biYPelsPerMeter	= 0;
	lpBI->biClrUsed			= 0;
	lpBI->biClrImportant	= 0;
	if (which == 4)
		x = (rect->left + rect->right - w) / 2;
	else if (which / 2)
		x = rect->left;
	else
		x = rect->right - w;
	y = rect->bottom - h;
	int i;
	BYTE r,g,b;
	if (bSel)
		{
		r = GetRValue(crLevelSel);
		g = GetGValue(crLevelSel);
		b = GetBValue(crLevelSel);
		}
	else
		{
    	COLORREF cr3DFace       = ::GetSysColor(COLOR_3DFACE);
		r = GetRValue(cr3DFace);
		g = GetGValue(cr3DFace);
		b = GetBValue(cr3DFace);
		}
	UINT p = 4 * ((3 * w + 3) / 4);
	BYTE * pSrc = m_pHdrIcons + 40 + p * h * which;
	BYTE * pDst = m_pHdrIcons + 40 + p * h * 5; // to temp area

	for (i = 0; i < h; i++)
		{
		int j;
		for (j = 0;j < w; j++)
			{
			if ((pSrc[p*i+3*j+0] > 200) &&
				(pSrc[p*i+3*j+1] > 200) &&
				(pSrc[p*i+3*j+2] > 200))
				{
				pDst[p*i+3*j+0] = b;
				pDst[p*i+3*j+1] = g;
				pDst[p*i+3*j+2] = r;
				}
			else
				{
				pDst[p*i+3*j+0] = pSrc[p*i+3*j+0];
				pDst[p*i+3*j+1] = pSrc[p*i+3*j+1];
				pDst[p*i+3*j+2] = pSrc[p*i+3*j+2];
				}
			}
		}
	BOOL bSuccess;
	if (!pDC->IsPrinting())
		{
		bSuccess = ::SetDIBitsToDevice(pDC->m_hDC,x,y,w,h,
			0, 0,
			0, h, pDst,
			(LPBITMAPINFO)lpBI, DIB_RGB_COLORS);
		}
	else
		{
		w = rect->right - x;
		h = rect->bottom - y;
		bSuccess = ::StretchDIBits(pDC->m_hDC,x,y,w,h,
				0, 0, m_twidth,m_theight,pDst,
				(LPBITMAPINFO)lpBI, DIB_RGB_COLORS, SRCCOPY);
		}
	
}
*/

void CMySlider3::TrackPoint(CPoint Point)
{
	int min, max;
	DPF("mowse,%d,%d",Point.x,Point.y);
//	ScreenToClient(&Point);
	CRect Rect;
	GetChannelRect(Rect);
	DPF("ch,x:%d,y:%d,w:%d,h:%d",Rect.left,Rect.top,Rect.right,Rect.bottom);
	BOOL bTracking;
	if (!m_bTracking)
		bTracking = Rect.PtInRect(Point); /// allows seting pos with click
	else
		bTracking = (Point.x >= Rect.left) && (Point.x < Rect.right) ? 1 : 0;
	if (bTracking)
		{
		GetRange(min, max);
		int ww = Rect.right - Rect.left;
DPF("rectw:%d,x:%d",ww,Point.x-Rect.left);
		if (!ww)
			return;
		int z = ((Point.x - Rect.left) * (max - min) + ww / 2) / ww;
		z += min;
		SetPos(z);
		m_pParent->ChangeSliderPos();
		}
}

void CMySlider3::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetCapture();
	TrackPoint(point);
	m_bTracking = TRUE;
}

void CMySlider3::OnMouseMove(UINT nFlags, CPoint Point)
{
	if(GetCapture() == this)
		{
		ASSERT(m_bTracking);
		TrackPoint(Point);
		}
	else
		CSliderCtrl::OnMouseMove(nFlags, Point);
}

void CMySlider3::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bTracking = 0;
	ReleaseCapture();
	CMainFrame * pFrame = m_pParent->m_pFrame;
	CSketchView * pView = (CSketchView *)pFrame->GetActiveView();
	if (pView)
		pView->NoScrub();

}

void CMyDialogBar::SetStart()
{
	int sstart, sstop, spos;
	spos = m_slider.GetPos();
	m_slider.GetSelection(sstart,sstop);
	int start = SlideToFrame(sstart);
	int stop  = SlideToFrame(sstop);
	int pos = SlideToFrame(spos);
	if (!stop)
		{
		stop = m_max_frame;
		sstop = m_factor;
		}
	if (pos == start)
		{
		start = 0;
		sstart = 0;
		}
	else
		{
		start = pos;
		sstart = spos;
		}
	if (start > stop)
		{
		stop = start;
		sstop = sstart;
		}
DPF("setting,%d,%d",start,stop);
	m_pFrame->m_pScene->SetSelection(start,stop);
	if (!start && (stop == m_max_frame))
		{
		m_slider.ClearSel(1);
		m_slider.SetSelection(0,0);
		}
	else
		{
		m_slider.SetSelection(sstart,sstop);
		m_slider.Invalidate();
		}
	CheckDlgButton(IDC_SLD_START,start != 0);
}

void CMyDialogBar::SetStop()
{
	int sstart, sstop, spos;
	m_slider.GetSelection(sstart,sstop);
	spos = m_slider.GetPos();
	int start = SlideToFrame(sstart);
	int stop  = SlideToFrame(sstop);
	int pos =   SlideToFrame(spos);
	if (!stop)
		{
		stop = m_max_frame;
		sstop = m_factor;
		}
	if (pos == stop)
		{
		stop = m_max_frame;
		sstop = m_factor;
		}
	else
		{
		stop = pos;
		sstop = spos;
		}
	if (stop < start)
		{
		start = stop;
		sstart = sstop;
		}
DPF("setting,%d,%d",start,stop);
	m_pFrame->m_pScene->SetSelection(start,stop);
	if (!start && (stop == m_max_frame))
		{
		m_slider.ClearSel(1);
		m_slider.SetSelection(0,0);
		}
	else
		{
		m_slider.SetSelection(sstart, sstop);
		m_slider.Invalidate();
		}
	CheckDlgButton(IDC_SLD_STOP,stop != m_max_frame);
}

void CMyDialogBar::DoReturn()
{
	UINT frame = GetDlgItemInt(IDC_SLD_FRAME);
	if (!frame)
		frame = 1;
	else if (frame > (m_max_frame + 1))
		frame = m_max_frame + 1;
	CSketchView * pView = (CSketchView *)m_pFrame->GetActiveView();
	if (SetFrame(frame))
		{
		if (pView)
			pView->Update(frame-1,1,8);

		}
	PostMessage(WM_COMMAND, ID_VCR_SLIDER_STOP, 0);
}
