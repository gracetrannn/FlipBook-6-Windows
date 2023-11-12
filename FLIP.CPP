#include "stdafx.h"
#include "sketch.h"
#include <string.h>

//#include "flip.h"
#include "myview.h"
#include "camera.h"
#include "math.h"
#include "sceneopt.h"
#include "ccell.h"
#include "clayers.h"
void CSketchView::FClear()
{
	DPF("fclear");
}


UINT CSketchView::SetupFlip()
{
	m_bFirst = TRUE;
	m_flip_depth = 0;
	UINT f = m_pFrame->StackFrame();
	if (m_pScene->ColorMode())
		{
		m_count = 1;
		m_frames[m_flip_depth++] = f;
		}
	else if (!m_pDoc->Option(SC_MRU))
		{
		UINT c = m_pFrame->OnionCount();
		if (!c)
			{
			m_count = 1;
			m_frames[m_flip_depth++] = f;
			}
		else if (c == 1)
			{
			UINT v = m_pFrame->Onion(0);
			if (v < f)
				{
				m_frames[m_flip_depth++] = v;
				m_frames[m_flip_depth++] = f;
				}
			else
				{
				m_frames[m_flip_depth++] = f;
				m_frames[m_flip_depth++] = v;
				}
			}
		else
			{
			m_frames[m_flip_depth++] = m_pFrame->Onion(0);
			m_frames[m_flip_depth++] = f;
			m_frames[m_flip_depth++] = m_pFrame->Onion(1);
			}
		}
	else
		{
		UINT c = m_pFrame->OnionCount();
		UINT i;
		for (i = 0; i < c; i++)
			m_frames[m_flip_depth++] = m_pFrame->Onion(c-1-i);
		m_frames[m_flip_depth++] = f;
		}
	m_count = m_frames[m_flip_depth-1] + 1 - m_frames[0];
DPF("count:%d, dpeth:%d",m_count,m_flip_depth);
	m_Index = m_count / 2;
	return 0;
}

UINT CSketchView::SetupScrub(BOOL bSelect)
{
	m_count = m_pScene->FrameCount();
	if (bSelect)
		m_Index = m_count / 2;
	else if (m_Index >= m_count)
		m_Index = m_count - 1;
	return 0;
}

UINT CSketchView::FSetup()
{
	DPF("setup flip");
	if (!m_pDoc) return 0;
	FClear();
	if (m_Bret == 2)
		SetupFlip();
	else
		SetupScrub();
	return 0;
}

void CSketchView::FRedraw()
{
	if (m_pVideo)
		((CWnd *)m_pVideo)->Invalidate(FALSE);
	Invalidate(FALSE);
}

void CSketchView::FStartDraw(CPoint point)
{
	m_pFrame->PlaySnd();
	if ((m_Bret == 3) && (m_pDoc->Option(OVERRIDE_FLAGS) & 2))
		m_Index = m_Frame;
	else if (m_Bret == 2)
		SetupFlip();
	else
		SetupScrub();
//	point.x /= (int)m_scale;
//	point.y /= (int)m_scale;
//	DPF("flip start at:%d,%d, ox:%d,oy:%d",x,y,ox,oy);
	m_origx = point.x;
	m_origy = point.y;
	m_origf = m_Index;
	if (m_Bret == 1)
		QSetup(TRUE);
	FRedraw();
}

UINT CSketchView::QSetup(BOOL bStart /* = FALSE */)
{
	if (bStart)
		{
		UINT Stop = m_Frame;
		if (!m_pScene->FindNextCell(Stop, m_Level))
			Stop = m_pScene->FrameCount();
		UINT Frame;
		for (Frame = m_Frame;Frame < Stop; Frame++)
			m_pScene->CheckComposite(Frame);
		}
	return 0;
/*
//	if (m_bModified2 && bStart)
	if (bStart)
		{
DPF("faking");
		m_bModified2 = 0;
		if (m_pBits)
			m_pScene->FakeCache(m_Frame, m_Level, m_pBits);
		}
DPF("bret:%d,%d",m_BretFrame, m_BretLevel);
	if (m_bModified && !bStart && m_Bret)
		m_pScene->FakeCache(m_BretFrame, m_BretLevel, 0);
*/
	return 0;
}

void CSketchView::FStopDraw(CPoint point)
{
//	point.x /= (int)m_scale;
//	point.y /= (int)m_scale;
	DPF("flip stop at:%d,%d",point.x,point.y);
//	if (m_Bret == 1)
//		QSetup();
}
#define RANGE 200

void CSketchView::FMoveDraw(CPoint point)
{
//	point.x /= (int)m_scale;
//	point.y /= (int)m_scale;
DPF("flip move to:%d,%d",point.x,point.y);
	int dx,dy,z;
	WORD old;
	old = m_Index;
	int start = m_pScene->Selection_Start();
	int count = m_pScene->Selection_Stop() + 1 - start;
	if ((m_Bret == 3) && (m_pDoc->Option(OVERRIDE_FLAGS) & 2))
		{
		int x1 = m_origx - (RANGE * (m_origf - start)) / count;
		int f = start + (count * (point.x - x1)) / RANGE;
		if (f < start) f = start;
		if ((f - start) >= count) f = start + count - 1;
		m_Index = f;
		}
	else
		{
		dx = point.x - (int)m_origx;
		dy = point.y - (int)m_origy;
		if (abs(dx) > abs(dy))
			z = dx;
		else
			z = dx;
		DPF("flip move to:%d,%d,%d,%d,%d",point.x,point.y,dx,dy,z);
		z += RANGE / 2;
		if (z <= 0)
			z = 0;
		if (z > RANGE)
			z = RANGE;
		m_Index = start + MulDiv(count - 1, z, RANGE);
		}
	if (m_Index != old)
		{
		m_bPlaySnip = TRUE;
		FRedraw();
		}
}

void CSketchView::FMyPaint(CDC * pDC)
{
	CRect rectClip;
	pDC->GetClipBox(&rectClip);
	DPF("flip,my paint,cur:%d,cnt:%d",m_Index, m_count);
	DPR("flip,my paint, clip ",&rectClip);
	if (!m_count || !m_pScene)
		{
		pDC->FillSolidRect(rectClip, m_crBack);
		return;
		}
	BOOL bVCR = TRUE;
	UINT oldden,oldnum;
	UINT f = m_pScene->ZFactor();
	if ((m_Bret == 2) || (m_Bret == 4))
		bVCR = FALSE;
	UINT sw = m_offx + m_nNum * m_swidth / m_nDen;
	UINT sh = m_offy + m_nNum * m_sheight / m_nDen;
	if (m_offy)
		{
		CRect rectOff(0,0,rectClip.right,m_offy);
		rectClip.top = m_offy;
		pDC->FillSolidRect(rectOff, m_crBack);
		}
	if (m_offx)
		{
		CRect rectOff(0,rectClip.top,m_offx,rectClip.bottom);
		rectClip.left = m_offx;
		pDC->FillSolidRect(rectOff, m_crBack);
		}
	if ((DWORD)rectClip.right > sw)
		{
		CRect rectOff(sw,rectClip.top,rectClip.right,rectClip.bottom);
		rectClip.right = sw;
		pDC->FillSolidRect(rectOff, m_crBack);
		}
	if ((DWORD)rectClip.bottom > sh)
		{
		CRect rectOff(rectClip.left,sh,rectClip.right,rectClip.bottom);
		rectClip.bottom = sh;
		pDC->FillSolidRect(rectOff, m_crBack);
		}
	if (bVCR)
		{
		oldden = m_nDen;
		oldnum = m_nNum;
		m_nDen *= 2;// * m_nPreview;
//		m_nDen = m_nFactor;
		m_nNum *= f;
		}
DPF("f:%d,nfac:%d",f,m_nFactor);
	RECT src;
	XlateRect(rectClip,src,1);
//	src = rectClip;
DPR("clip",&rectClip);
DPR("src ",&src);
	if (bVCR)
		{
		m_nDen = oldden;
		m_nNum = oldnum;
		}

	BYTE * pTemp = 0;
	HPBYTE hpdib;
	UINT i;
	if ((m_Bret == 2) || (m_Bret == 4))
		{
		pTemp = new BYTE[m_sheight * m_spitch];
		for (i = 1; i < m_flip_depth; i++)
			{
			if ((m_frames[i] - m_frames[0]) > m_Index)
				break;
			}
		m_Frame = m_frames[i-1];
		m_Level = m_pFrame->StackLevel();
DPF("flip draw,frm:%d",m_Frame);
		if (m_Frame != m_pFrame->StackFrame())
			{
			m_pScene->GetGray(pTemp,m_Frame,m_Level);
		//	m_pScene->GetLayer(hpdib,m_Frame, m_Level, CCell::LAYER_INK);
			}
		else
			{
			memset(pTemp,255,m_sheight*m_spitch);
			m_pLayers->Update(pTemp,pTemp,m_spitch,1);
			}
		hpdib = pTemp;
		}
	else
		{
		m_count = m_pScene->FrameCount();
		if (m_Index >= m_count)
			m_Index = m_count - 1;
		m_Frame = m_Index;
		if (!m_bPlaying)
			{
			if (m_CamState)
				m_pScene->CheckComposite(m_Frame,3);
			else 
				m_pScene->CheckComposite(m_Frame);
			m_pFrame->CameraPacket(3,m_Frame,0);
			if (m_bPlaySnip)
				m_pFrame->PlaySnd(m_Frame,0);
			m_bPlaySnip = 0;
//		m_pFrame->Progress(995,m_pScene->CachedSize(m_Frame));
			}
		else if (m_Kludge)
			{
			m_pScene->CheckComposite(m_Frame);
			}
		else if (m_bFirst && !m_bDir)
			m_pFrame->PlaySnd(m_Frame,1);
		hpdib = m_pScene->GetCacheP(m_Frame);
		}
	m_bFirst = FALSE;
	if (hpdib && m_hdd)
		{
		LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)m_pDIB;
		UINT tw, th;
		if (bVCR)
			{
			tw = lpBI->biWidth;
			th = lpBI->biHeight;
            lpBI->biWidth  = 2 * lpBI->biWidth / f;
            lpBI->biHeight = 2 * lpBI->biHeight / f;
			}
		int x = src.left;
		int y = src.top;
		int w = src.right - src.left;
		int h = src.bottom - src.top;
		if (bVCR && m_pScene->Flag(SCN_FLG_REDBOX) && m_pScene->Flag(SCN_FLG_NLARGE))
			{
			x += w / 4;
			y += h / 4;
			w /= 2;
			h /= 2;
			}
		if (h > lpBI->biHeight) h = lpBI->biHeight;
		if (w > lpBI->biWidth) w = lpBI->biWidth;
DPF("flipdrawdib,w:%d,h:%d",w,h);
        BOOL z = DrawDibDraw(m_hdd,
                pDC->m_hDC,
				rectClip.left, rectClip.top, 
				rectClip.right - rectClip.left, rectClip.bottom - rectClip.top,
                lpBI, hpdib,	
				x,y,w,h,
				DDF_SAME_HDC);// | DDF_UPDATE | DDF_HURRYUP);
		if (m_pScene->Flag(SCN_FLG_GRID) || m_pScene->Flag(SCN_FLG_REDBOX))
			DoGrid(lpBI);
		delete [] pTemp;
		if (bVCR)
			{
			lpBI->biWidth = tw;
			lpBI->biHeight = th;
			}
DPF("flip,camstate:%d,tool%d",m_CamState,m_CamTool);
//		if ((m_Camera > 2) && (m_Camera < 12) && !m_bPlaying)
		
		if (m_CamState)
//		if (m_pFrame->SwitchCamera(0) && !m_bPlaying)
			{
			CPoint peg = CPoint(m_pegx,m_pegy);
			if (bVCR && m_pScene->Flag(SCN_FLG_REDBOX) && m_pScene->Flag(SCN_FLG_NLARGE))
				{
				peg.x = peg.x * 2 - w;
				peg.y = peg.y * 2 - h;
				}
DPF("m_pegx:%d,y:%d,px:%d,py:%d",m_pegx,m_pegy,peg.x,peg.y);
			XlateCPoint(peg,TRUE);

			int r = 7;
			CPoint q1,q2;
			q1.x = peg.x - r;
			q1.y = peg.y - r;
			q2.x = peg.x + r;
			q2.y = peg.y + r;
			MyDrawObj(pDC,2,q1,q2);
			}
		}
//	if (!m_bPlaying)
//			DrawCurve(pDC, m_Frame);
}

void CSketchView::FDocCursor()
{
//	m_pView->SetCursor(AfxGetApp()->LoadCursor(IDC_FLIP));
//	((CWnd *)m_pView)->SetCursor(AfxGetApp()->LoadCursor(IDC_FLIP));
	if (m_Bret == 3)
		SetCursor(AfxGetApp()->LoadCursor(IDC_VCR));
}

void CSketchView::OnTimerFire()
{
	DPF("have timer, cur:%d",m_Index);
	if (!m_bPlaying)
		{
		StartTimer();
		return;
		}
	UINT start, stop;
	if (!m_pScene)
		return;
	if (m_Bret != 4)
		{
		if (m_Kludge)
			{
			start = m_Index;
			}
		else
			{
			start = m_pScene->Selection_Start();
			stop  = m_pScene->Selection_Stop();
			m_count = stop + 1 - start;
			}
		}
	else
		start = 0;
	if (m_bDir)
		{
		if (m_Index > start)
			m_Index--;
		else if (m_Bret == 3)
			{
			if (m_Kludge)
				{
				m_Kludge = 0;
				m_bPlaying = FALSE;
				}
			if (m_pDoc->Option(LOOPPLAY))
				{
				SetupScrub(0);
				m_Index = stop;
				}
			else
				m_bPlaying = FALSE;
			}
		else if (m_Bret == 4)
			{
			if (GetKeyState(VK_RIGHT) >= 0)
				m_bPlaying = FALSE;
			else
				m_Index = stop;
			}
		else
			m_bPlaying = FALSE;
		}
	else
		{
		DPF("count:%d,%d",m_count,m_Index);
		if (m_Index < stop)
			m_Index++;
		else if (m_Bret == 3)
			{
			if (m_Kludge)
				{
				m_count = m_Kludge - 1;
				m_Kludge = 0;
				m_bPlaying = FALSE;
				}
			else if (m_pDoc->Option(LOOPPLAY))
				{
				SetupScrub(0);
				m_Index = start;
				m_bFirst = TRUE;
				}
			else
				m_bPlaying = FALSE;
			}
		else if (m_Bret == 4)
			{
			if (GetKeyState(VK_LEFT) >= 0)
				m_bPlaying = FALSE;
			else
				m_Index = start;
			}
		else
			m_bPlaying = FALSE;
		}
	FRedraw();
	if (!m_bPlaying)
		{
		StartTimer();
		m_pFrame->PlaySnd();
//		if (m_Bret == 4)
//			NewBret(2);
		if (m_Bret != 3)
			NewBret(0);
		}
	else
		{
//		FRedraw();
		if (m_rate != m_pScene->FrameRate())
			StartTimer(TRUE);
	//	m_pFrame->Progress(997,m_Frame+1);
		}
	m_pFrame->Progress(997,m_Frame+1);
}


#ifndef FLIPBOOK_MAC
void CSketchView::DrawCurve(CDC * pDC, UINT frame)
{
	RECT rectClip;
	pDC->GetClipBox(&rectClip);
	RECT src;
//	XlateRect(rectClip,src,1);
	XlateRect(src, rectClip,0);
	int x = src.left;
	int y = src.top;
	int w = src.right - src.left;
	int h = src.bottom - src.top;
	w = m_nNum * m_swidth / m_nDen;
	h = m_nNum * m_sheight / m_nDen;
	int c = m_pScene->FrameCount();
	int * px = new int[c];
	int * py = new int[c];
	int i;
	for (i = 0; i < c; i++)
		{
		int ix, iy,kind;
		m_pFrame->CameraPegXY(ix,iy,kind,i,w,h);
		ix += m_offx;
		iy += m_offy;
		ix *= 2;
		if (kind)
			ix++; 
		px[i] = ix;
		py[i] = iy;
		}
	CPen solid (PS_SOLID,1,RGB(255,255,255));
	CPen dotted (PS_DOT,1, RGB(0,0,0));
	HPEN hPenOld = (HPEN)pDC->SelectObject(solid);
	int old = pDC->SetROP2(R2_COPYPEN); // first draw solid white
	for (i = 0; i < 2; i++)
		{
		if (i)
			{
			pDC->SelectObject(dotted); // then draw dotted black
			}
		int j;
		for (j = 0; j < c; j++)
			{
			int k = px[j] & 1;
			int x = px[j] / 2;
			int y = py[j];
			if (!j)
				pDC->MoveTo(x,y);
			else
				pDC->LineTo(x,y);
			if (k)
				pDC->Ellipse(x-5,y-5,x+5,y+5);
			}
		for (j = 0; j < c; j++)
			{
			int r = (j == frame) ? 20 : 10;
			int a = j - 1;
			int b = j + 1;
			if (a < 0)
				a = 0;
			else if (b >= c)
				b = c-1;
			double angle = atan2((double)(px[b] / 2 - px[a] / 2),
								(double)(py[b] - py[a]));
			double	ssin = sin(angle);
			double	ccos = cos(angle);
			int x1 = (px[j] / 2) + (int)((double)r * ccos); 
			int x2 = (px[j] / 2) - (int)((double)r * ccos); 
			int y1 = py[j] - (int)((double)r * ssin); 
			int y2 = py[j] + (int)((double)r * ssin); 
			pDC->MoveTo(x1, y1);
			pDC->LineTo(x2, y2);
			}
		}
	delete [] px;
	delete [] py;
	pDC->SetROP2(old);
	pDC->SelectObject(hPenOld );
}
#endif

void CSketchView::XlateVCRPoint(CPoint & pt)
{
#ifdef FLIPBOOK_MAC
	CPoint pt1 = GetZeroScrollPosition();
#else
	CPoint pt1 = GetScrollPosition();
#endif
	int n = m_pScene->ZFactor(0) * m_nNum;
	int d = 2 * m_nDen;
	pt.y =  m_offy + (n * (pt.y)) / d - pt1.y;
	pt.x =  m_offx + (n * (pt.x)) / d - pt1.x;
}

void CSketchView::DoGrid(LPBITMAPINFOHEADER lpBI)
{
	CClientDC ddc(this);
#ifdef FLIPBOOK_MAC
	CPoint pt1 = GetZeroScrollPosition();
#else
	CPoint pt1 = GetScrollPosition();
#endif

	int old = ddc.SetROP2(R2_COPYPEN);
	HPEN hPenOldg = 0;
	HPEN hPenOldr = 0;
	if (m_pScene->Flag(SCN_FLG_GRID))
		{
		UINT w = lpBI->biWidth;
		UINT h = lpBI->biHeight;
		UINT ww = w / 2;
		UINT hh = h / 2;
		CPen gsolid (PS_SOLID,1,RGB(0,255,0));
		hPenOldg = (HPEN)ddc.SelectObject(gsolid);
		if (m_pScene->Flag(SCN_FLG_GRID_CTR))
			{
			CPoint p1(0,0);
			CPoint p2(w-1,h-1);
			XlateVCRPoint(p1);
			XlateVCRPoint(p2);
			ddc.MoveTo((p1.x + p2.x) / 2, p1.y);
			ddc.LineTo((p1.x + p2.x) / 2, p2.y);
			ddc.MoveTo(p1.x,(p1.y+p2.y)/2);
			ddc.LineTo(p2.x,(p1.y+p2.y)/2);
			}
		else
			{
			UINT x,y,xx,yy;
			UINT z = 12;
			if (m_pScene->Flag(SCN_FLG_REDBOX))
				z = 24;
		//	double zz = m_pScene->CamScale();
		//	if (zz)
		//		z = (UINT)(((double)z * 100.0) / zz);
			UINT www = ww * z;
			UINT hhh = hh * z;
			CPoint pt1, pt2;
			for (yy = 0; yy < hhh;yy+=hh)
				{
				y = yy / z;
				pt1.x = 0;
				pt1.y = hh + y; 
				XlateVCRPoint(pt1);
				pt2.y = hh + y;
				pt2.x = w - 1;
				XlateVCRPoint(pt2);
				ddc.MoveTo(pt1);
				ddc.LineTo(pt2);
				pt1.y = hh-y;
				pt1.x = 0;
				XlateVCRPoint(pt1);
				pt1.y--;
				pt2.y = hh-y;
				pt2.x = w - 1;
				XlateVCRPoint(pt2);
				pt2.y--;
				ddc.MoveTo(pt1);
				ddc.LineTo(pt2);
				}
			for (xx = 0; xx < www;xx+=ww)
				{
				x = xx / z;
				pt1.x = ww + x; 
				pt1.y = 0;
				XlateVCRPoint(pt1);
				pt2.x = ww + x;
				pt2.y = h - 1;
				XlateVCRPoint(pt2);
				ddc.MoveTo(pt1);
				ddc.LineTo(pt2);
				pt1.x = ww - x;
				pt1.y = 0;
				XlateVCRPoint(pt1);
				pt1.x--;
				pt2.x = ww - x;
				pt2.y = h - 1;
				XlateVCRPoint(pt2);
				pt2.x--;
				ddc.MoveTo(pt1);
				ddc.LineTo(pt2);
				}
			}
		}
	if (m_pScene->Flag(SCN_FLG_REDBOX))
		{
		CPen rsolid (PS_SOLID,1,RGB(255,0,0));
		int w = lpBI->biWidth;
		int h = lpBI->biHeight;
		CPoint pt1, pt2;
		pt1.x = w / 4;
		pt1.y = h / 4;
		pt2.y = 3 * h / 4;
		pt2.x = 3 * w / 4;
		XlateVCRPoint(pt1);
		XlateVCRPoint(pt2);
		hPenOldr = (HPEN)ddc.SelectObject(rsolid);
		ddc.MoveTo(pt1.x,pt1.y);
		ddc.LineTo(pt2.x,pt1.y);
		ddc.LineTo(pt2.x,pt2.y);
		ddc.LineTo(pt1.x,pt2.y);
		ddc.LineTo(pt1.x,pt1.y);
		}
	ddc.SetROP2(old);
	if (hPenOldg)
		ddc.SelectObject(hPenOldg );
	else
		ddc.SelectObject(hPenOldr );
}
