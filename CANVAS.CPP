#include "stdafx.h"
#include "sketch.h"
#include "ccell.h"
#include <string.h>
#include "clayers.h"
#include "math.h"
#include "sceneopt.h"
//#include <limits.h>
#include "cbrush.h"
#include "myview.h"
#include "cfloat.h"
#include "ctablet.h"
#include "cnewpals.h"
#include "shiftrc.h"
static int ddx[8] = {-1, 0, 1, 1, 1, 0,-1,-1};
static int ddy[8] = {-1,-1,-1, 0, 1, 1, 1, 0};

typedef struct {
	union {
		BYTE rgbo[4];
		COLORREF rgb;
	};} MYRGB;


void CSketchView::Clear()
{
	m_wPressure = 255;
//	m_minz = 0;
//	m_maxz = 10;
	delete [] m_pDIB;
	m_pDIB = 0;
	delete [] m_pBG;
	m_pBG = 0;
	delete [] m_pSkin;
	m_pSkin = 0;
//	delete m_pWireFrame;
//	m_pWireFrame = 0;
}


void InitDib(void * m_pDIB, UINT m_swidth, UINT m_sheight, UINT bits)
{
	LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)m_pDIB;
	lpBI->biSize			= sizeof(BITMAPINFOHEADER);
	lpBI->biWidth			= m_swidth;
	lpBI->biHeight			= m_sheight;
	lpBI->biPlanes			= 1;
	lpBI->biBitCount		= bits;
	lpBI->biCompression		= BI_RGB;
	lpBI->biXPelsPerMeter	= 0;
	lpBI->biYPelsPerMeter	= 0;
	lpBI->biClrUsed			= bits < 9 ? 1 << bits : 0;
	lpBI->biClrImportant	= 0;
	lpBI->biSizeImage		= lpBI->biSize + 4 * lpBI->biClrUsed +
			lpBI->biHeight * 4 * ((lpBI->biWidth*lpBI->biBitCount+31) / 32);
}


UINT CSketchView::CSetup()
{
	DPF("setup canvas");
	if (!m_pDoc)
		{
DPF("no doc");
		return 1;
		}
	Clear();
	m_pScene = m_pDoc->GetDocScene();
	if (!m_pScene)
		{
DPF("no scene");
		return 2;
		}
	m_swidth = m_pScene->Width();
	m_sheight = m_pScene->Height();
	UINT bits = 8;
	if (m_pScene->ColorMode())
		bits = 24;
	m_spitch = 4 * ((bits * m_swidth + 31) / 32);
DPF("dgc-dib,w:%d,h:%d",m_swidth,m_sheight);
	m_pLayers->Setup(m_pScene,1);
	m_pBrush->Init(m_pLayers);
	DWORD dwTemp = ((DWORD)m_spitch) * ((DWORD)m_sheight);
	m_pBG = new BYTE[dwTemp];
	delete [] m_pSkin;
	DWORD ink_size = 16 * m_sheight * ((m_swidth + 3) / 4);
	m_pSkin = new BYTE[dwTemp + ink_size];
	if (bits < 9)
		dwTemp += 1024;
	dwTemp += sizeof(BITMAPINFOHEADER);
	m_pDIB = new BYTE[dwTemp];
	if (!m_pBG || !m_pDIB)
		return 3;
	m_pBits = m_pDIB + sizeof(BITMAPINFOHEADER);
	if (bits < 9)
		{
		for (int i = 0; i < 1024; i++)
			m_pBits[i] = i / 4;
		m_pBits += 1024;
		}
	InitDib(m_pDIB, m_swidth, m_sheight, bits);
	return 0;
}

UINT CSketchView::CSelect(UINT Frame, UINT Level)
{
	if (!m_pScene) return 0;
	m_pDoc->SetModifiedFlag(m_pScene->Modified());
DPF("cselecting:%d,%d",Frame, Level);
	if (Frame != 32000)
		{
		m_BretFrame = m_Frame = Frame;
		m_Level = Level;
		m_EditFrame = Frame;
		m_EditLevel = Level;
		}
	m_znum = 0;
	m_pLayers->Select(m_Frame, m_Level);
//	m_bDirty = 1;
	m_bModified = FALSE; 
	CRedraw();
	return 0;
}

void CSketchView::DupIt()
{
	if (m_pScene)
		m_pDoc->SetModifiedFlag(m_pScene->Modified());
	KeepCell(0);
	if (m_pLayers->IsOverlay())
		{
DPF("cannot ");
		MessageBeep(0);
		
		return;
		}
	m_Frame += m_pDoc->Option(DEF_HOLD);
	m_EditFrame = m_Frame;
	if (m_CamState)
		m_pFrame->CameraPacket(9,0,0);
	m_nFactor = m_pScene->ZFactor();
	
	m_pLayers->DupCell(m_Frame, m_Level);
	KeepCell(1);
	m_pFrame->SelectCell(m_Frame, m_Level, 3);
//	CRedraw();

}

void CSketchView::KeepCell(BOOL bNew)
{
DPF("keep frame:%d,lvl:%d",m_EditFrame,m_EditLevel);
	if (m_pLayers->Put(bNew))
		{
		m_pScene->BlowCell(m_EditFrame, m_EditLevel);
		m_pFrame->UpdateCell(m_EditFrame, m_EditLevel);
		}
	QSetup();
	m_bModified = FALSE;
}


void CSketchView::ApplyLayer(UINT factor,CNewPals * pPals,BYTE * pLayer)
{
	UINT span = 4 * ((m_swidth + 3) / 4);
	UINT offset = m_sheight * span;
	UINT x, y;
	for (y = 0; y < m_sheight; y++,pLayer += span)
		{
		for (x = 0; x < m_swidth; x++)
			{
			UINT i = pLayer[offset+x];
			UINT q = pPals->Alpha(i);
			UINT z = (q * factor * (UINT)pLayer[x]) / 25500;
			//UINT z = factor * (UINT)Layer[x] / 100;
			if (!z)
				continue;
			if (z == 255)
				{
				m_pSkin[m_spitch*y+3*x+0] = pPals->Blue(i);
				m_pSkin[m_spitch*y+3*x+1] = pPals->Green(i);
				m_pSkin[m_spitch*y+3*x+2] = pPals->Red(i);
				}
			else
				{
				WORD v;
				v = (255 - z) * m_pSkin[m_spitch*y+3*x+0];
				v += z * pPals->Blue(i);
				m_pSkin[m_spitch*y+3*x+0] = v / 255;
				v = (255 - z) * m_pSkin[m_spitch*y+3*x+1];
				v += z * pPals->Green(i);
				m_pSkin[m_spitch*y+3*x+1] = v / 255;
				v = (255 - z) * m_pSkin[m_spitch*y+3*x+2];
				v += z * pPals->Red(i);
				m_pSkin[m_spitch*y+3*x+2] = v / 255;
				}
			}
		}
}


void CSketchView::ApplyInk(UINT factor,UINT Frame,UINT Level, BOOL bHold)
{
	DWORD key  = m_pScene->GetCellKey(Frame,Level, bHold);
	if (!key)
		return;
	BOOL bPaint = 0;
	if (factor > 100)
		{
		factor = 100;
		bPaint = 1;
		}
	BYTE * pInk = m_pSkin + m_spitch * m_sheight;
	if (m_pScene->GetLayer(pInk,Frame, Level,CCell::LAYER_INK, key))
		return;
	if (m_pScene->ColorMode())
		{
		CNewPals * pPals = m_pScene->LevelPalette(Level);
		if (bPaint)
			{
			BYTE * pPaint = pInk + m_sheight * 8 * ((m_swidth + 3) / 4);;
			if (!m_pScene->GetLayer(pPaint,Frame, Level,
						CCell::LAYER_PAINT, key))
				ApplyLayer(factor,pPals,pPaint);
			}
		ApplyLayer(factor,pPals,pInk);
		}
	else
		{
		UINT x, y;
		UINT span = 4 * (( m_swidth + 3) / 4);
		UINT k;
		if (k = m_pShiftTrace->Test(Frame, m_Level))
			{
			k--;
			int xx,yy,rot,num;
			int den = 100;
			m_pShiftTrace->Get(k,xx,yy,rot,num);
			num = (int)(100.0 * exp((double)num / 100.0));
//			num += 100;
DPF("cangle:%d",rot);
			double twopi = 8.0 * atan(1.0);
			double angle = (rot * twopi) / 360;
			double cssin = sin(angle);
			double cccos = cos(angle);
			int cx = m_swidth / 2;
			int cy = m_sheight / 2;
			UINT x, y;
			UINT span = 4 * (( m_swidth + 3) / 4);
			for (y = 0; y < m_sheight; y++)
			for (x = 0; x < m_swidth; x++)
				{
				double x1 = (int)x - cx - xx;
				double y1 = (int)y - cy + yy;

				double x2 = x1 * cccos - y1 * cssin;
				double y2 = x1 * cssin + y1 * cccos;

				UINT x3 = (UINT)((x2 * den) / num);
				UINT y3 = (UINT)((y2 * den) / num);
				x3 += cx;
				y3 += cy;
				if ((x3 < m_swidth) && (y3 < m_sheight))
					{
					UINT z = (factor * (UINT)pInk[y3*span+x3]) / 100;
					if (!z)
						continue;
					UINT q = m_pSkin[m_spitch*y+x];
					q = ((255-z) * q) / 255;
					m_pSkin[m_spitch*y+x] = (BYTE)q;
					}
				}
			}
		else
			{
			for (y = 0; y < m_sheight; y++,pInk += span)
				{
				for (x = 0; x < m_swidth; x++)
					{
					UINT z = (factor * (UINT)pInk[x]) / 100;
					if (!z)
						continue;
					UINT q = m_pSkin[m_spitch*y+x];
					q = ((255-z) * q) / 255;
					m_pSkin[m_spitch*y+x] = (BYTE)q;
					}
				}
			}
	}
}

void CSketchView::SetupStack()
{
	DWORD i,d,dd,min,max,f;

	if (m_pDoc->Option(PEG_SHOWFG))
		i = m_pFrame->OnionCount();
	else
		i = 0;
	d = m_pDoc->Option(PEG_DENSITY);
	max = d / 1000;
	if (!max || (max > 100))
		max = 100;
	min = d = d % 1000;
	if (min > max)
		min = max / 2;
//DPF("bgf:%d,min:%d,i:%d",bgf,min,i);
	if (i > 0)
		{
		dd = (max - min) / i;
		}
DPF("redraw count:%d,d:%d",i,d);
	for (;i-- ;d += dd)
		{
		UINT Frame;
		Frame = m_pFrame->Onion(i);
		f = min + ((d - min) * (d - min)) / (100 - min);
		DPF("redrawing,i:%d,frm%d,lvl:%d,f:%d",i,Frame,m_Level,f);
		ApplyInk(f, Frame,m_Level);
		}
}

void CSketchView::SetupBetween()
{
	if (!m_pDoc->Option(PEG_SHOWFG))
		return;
	UINT max = m_pDoc->Option(PEG_DENSITY) / 1000;
	if (!max || (max > 100))
		max = 100;
	max = 20;
	UINT c = m_pFrame->OnionCount();
	UINT i;
	for (i = 0; i < c; i++)
		{
		UINT Frame = m_pFrame->Onion(i);
		m_pShiftTrace->Add(Frame, m_Level,0);
		ApplyInk(max, Frame,m_Level);
		}
}

void CSketchView::SetupGray()
{
	UINT size = m_sheight * 4 * ((m_swidth + 3) / 4);
	if (!m_pFrame->Timing())
		{
		if (m_pDoc->Option(PEG_SHOWBG) && m_Level)
			{
			m_pScene->GetBackground(m_pBG, m_Frame,m_pDoc->Option(BGMIN));
			memmove(m_pSkin,m_pBG, size);
			}
		else
			memset(m_pSkin,255,size);
		if (!m_pDoc->Option(SC_MRU))
			SetupBetween();
		else
			SetupStack();
		UINT i, c;
		UINT max = m_pDoc->Option(PEG_DENSITY) / 1000;
		if (!max || (max > 100))
			max = 100;
		max = 20;
		c = m_pFrame->BuddyCount();
		for (i = 0; i < c; i++)
			ApplyInk(max, m_pFrame->BuddyF(i),m_pFrame->BuddyL(i));
		}
	else
		memset(m_pSkin,255,size);
	m_pLayers->Update(m_pBits, m_pSkin, m_spitch,1);
}

void CSketchView::SetupColor()
{
	UINT x, y,yy;
	int option = m_pDoc->Option(SC_MRU);
	if (!m_pDoc->Option(PEG_SHOWFG))
		option = 3;

	if (option == 2)
		{
		m_pLayers->LoadControl(m_Frame, 0,1);	// clear it
		UINT i,c;
		c = m_pFrame->BuddyCount();
		for (i = 0;i < c;i++)
			{
			UINT l = m_pFrame->BuddyL(i);
			UINT f = m_pFrame->BuddyF(i);
			m_pLayers->LoadControl(f,l);
			}
		}
	UINT size = m_sheight * 4 * ((3 * m_swidth + 3) / 4);
	CNewPals *pPals = m_pScene->LevelPalette(m_Level);
	BOOL bBG = 0;
	if (m_pDoc->Option(PEG_SHOWBG) && m_Level)
		bBG = m_pScene->GetBackground(m_pBG, m_Frame,m_pDoc->Option(BGMIN));
	if (bBG)
		memmove(m_pSkin,m_pBG, size);
	else
		{
		BOOL bCheck = m_pDoc->Option(CHECKERBG);
		for (yy = 0; yy < m_sheight;yy++)
			{
			y = m_sheight - 1 - yy;
			for (x = 0; x < m_swidth; x++)
				{
				if (!bCheck || (((x /8) & 1) != ((y / 8) & 1)))
					{
					BYTE color[4];
					pPals->Color(color,255);
#ifdef FBVER7
					m_pSkin[m_spitch*y+3*x+0] = color[0];
					m_pSkin[m_spitch*y+3*x+1] = color[1];
					m_pSkin[m_spitch*y+3*x+2] = color[2];
#else
					m_pSkin[m_spitch*y+3*x+0] = color[2];
					m_pSkin[m_spitch*y+3*x+1] = color[1];
					m_pSkin[m_spitch*y+3*x+2] = color[0];
#endif
					}
				else
					{
					m_pSkin[m_spitch*y+3*x+0] = 240;
					m_pSkin[m_spitch*y+3*x+1] = 240;
					m_pSkin[m_spitch*y+3*x+2] = 240;
					}
				}
			}
		}
	m_pLayers->m_nFlags = 0;
	if (option == 3)  // no bulb
		{
		m_pLayers->m_nFlags |= 1;
		m_pLayers->m_nFlags |= 2;
		}
	else
		{
		if (!option)
			SetupBetween();
		else if (option == 1)
			SetupStack();
		m_pLayers->m_nFlags |= (option == 2) ? 1 : 0;
		m_pLayers->m_nFlags |= m_pDoc->Option(SC_PAINT) ? 2 : 0;
		m_pLayers->m_nFlags |= 4;
		if ((option != 2) || m_pDoc->Option(SC_PAINT))
			{
			UINT i, c;
			UINT max = m_pDoc->Option(PEG_DENSITY) / 1000;
			if (!max || (max > 100))
				max = 100;
			max = 20;
			if (option == 2)// m_pDoc->Option(SC_PAINT)) // do paint here
				max = 1000;
			c = m_pFrame->BuddyCount();
			for (i = 0; i < c; i++)
				ApplyInk(max,m_pFrame->BuddyF(i),m_pFrame->BuddyL(i));
			}
		}
	if ((option < 2) && m_pDoc->Option(SC_LIGHT_PAINT))
		m_pLayers->m_nFlags |= 1;
	m_pLayers->Update(m_pBits, m_pSkin, m_spitch,1);
}

void CSketchView::CRedraw()
{
DPF("credraw");
	m_bLeftHanded = m_pDoc->Option(LEFTHAND);
	m_bFastZoom = m_pDoc->Option(HARD_ZOOM);
	if (m_pScene->ColorMode())
		SetupColor();
	else
		SetupGray();
	Invalidate(FALSE);
}

void CSketchView::CStartDraw(CPoint point)
{
	DPF("cstart at:%d,%d",point.x,point.y);
	m_bModified = TRUE; 
//	m_bTablet = m_pDoc->Option(WINTAB) && m_pDoc->Option(TAB_PRESSURE);
//	m_wMaxPressure = m_pDoc->Option(MAX_PRESSURE);
//	m_pWnd = pWnd;
//	m_bLeftHanded = m_pDoc->Option(LEFTHAND);
	m_bErasing = m_pDoc->ToolInfo(0);
	m_color = m_pDoc->Color();
	m_radius = m_pDoc->Radius() - 1;
	m_density = m_pDoc->Density();
//	m_density = MulDiv(m_density, m_density, 100);
//	point.x /= (int)m_scale;
//	point.y /= (int)m_scale;
DPF("density:%d",m_density);
	CPoint tpoint = point;
	if (point.x < 0 )
		point.x = 0;
	else if (point.x >= (int)m_swidth)
		point.x = m_swidth - 1;
	if (point.y < 0 )
		point.y = 0;
	else if (point.y >= (int)m_sheight)
		point.y = m_sheight - 1;
	if ((m_wToolType == TOOL_FILL) && (point != tpoint))
		return;		// dont't start fill if outside

DPF("point,%d,%d",point.x,point.y);
	
//	m_minz = 0;//m_pDoc->Density(1000);
//	m_maxz = 10;//m_pDoc->Density(2000);
	
	m_toolflags = m_pDoc->ToolInfo(5);
//	if (m_bTablet && (m_toolflags & 4) && (m_wPressure != 255)) // radius
//		m_radius = m_minz;

	m_pBrush->SetTexture(0);
	UINT type, tooltype;
	tooltype = m_wToolType;
	if (Layer())
		{
		if (tooltype == TOOL_PENCIL)
			tooltype = TOOL_FILL;
		else if (tooltype == TOOL_TRACE)
			tooltype = TOOL_BRUSH;
		}
	if (tooltype == TOOL_PENCIL)
		{
		if (m_bErasing)
			m_pBrush->SetTexture((m_toolflags >> 8) & 3);
		else
			m_pBrush->SetTexture((m_toolflags >> 5) & 3);
		if (m_nToolSaved == 2)
			{
			type = 3;
			m_radius = 0;
			}
		else
			{
			if (m_bErasing)
				type = (m_toolflags & 1024) ? 6 : 0;
			else
				type = (m_toolflags & 16) ? 6 : 0;
			}
		}
	else if (m_wToolType == TOOL_TRACE)
		{
		type = 1;
		}
	else if (tooltype == TOOL_BRUSH)
		{
		if (m_bErasing)
			type = (m_toolflags & 1024) ? 4 : 2;
		else
			type = (m_toolflags & 16) ? 4 : 2;
		}
	else
		type = 2;
//	if (m_bErasing)
//		m_pBrush->SetTexture((m_toolflags >> 8) & 3);
//	else
//		m_pBrush->SetTexture((m_toolflags >> 5) & 3);
	m_pLayers->DrawInit(m_color,type,m_bErasing);
	m_pBrush->SetSolid(m_pLayers->Solid());
	m_bIgnore = FALSE;
	if (tooltype == TOOL_FILL)
		{
		FillIt(point.x,point.y, m_wToolType == TOOL_PENCIL ? 1 : 0);
//		m_pFrame->UpdateTools(1);
		}
	else if ((tooltype == TOOL_TRACE) && (GetKeyState(VK_SHIFT) < 0))
		{
		m_bIgnore = TRUE;
		FillIt(point.x,point.y,1);
		}
	else
		{
		if (!m_bTablet || !(m_toolflags & 4) )//|| (m_wPressure == 255))
			m_pBrush->SetRadius(m_radius);
		else
			m_pBrush->SetRadius(0);
		if (!m_bTablet || !(m_toolflags & 8)  )//|| (m_wPressure == 255))
			m_pBrush->SetDensity(m_density);
		else
			m_pBrush->SetDensity(0);
		point.x *= m_nNum;
		point.y *= m_nNum;
		DrawPoints(point,2);
		}
}

void CSketchView::CStopDraw(CPoint point)
{
	DPF("stop at:%d,%d",point.x,point.y);
	WORD ToolType = m_wToolType;
	if (Layer())
		{
		if (ToolType == TOOL_PENCIL)
			ToolType = TOOL_FILL;
		else if (ToolType == TOOL_TRACE)
			ToolType = TOOL_BRUSH;
		}
	if (ToolType == TOOL_EYEDROP)
		{
		UINT index = m_pDoc->Color();
		OnTool(ToolId(m_pDoc->ToolInfo(4)));
		m_pDoc->Color(index);
		}
	else if (ToolType != TOOL_FILL)
		{
		DrawPoints(point,1);
		m_wPressure = 255;
		}
	else if (GetKeyState(VK_SHIFT) < 0)
		{	
		UINT f = m_Frame + 1;
		if (m_pScene->FindNextCell(f,m_Level))
			m_pFrame->SelectCell(m_Frame = f,m_Level,4);
		}
	else
		m_pFrame->UpdateTools(1);
}

void CSketchView::CMoveDraw(CPoint point)
{
	DPF("move to:%d,%d,den:%d",point.x,point.y,m_density);
	WORD ToolType = m_wToolType;
	if (Layer())
		{
		if (ToolType == TOOL_PENCIL)
			ToolType = TOOL_FILL;
		else if (ToolType == TOOL_TRACE)
			ToolType = TOOL_BRUSH;
		}
	if (ToolType == TOOL_EYEDROP || m_bIgnore)
		return;
//	point.x /= (int)m_scale;
//	point.y /= (int)m_scale;
//	if (((UINT)point.x >= m_swidth) || ((UINT)point.y >= m_sheight))
//		return;


DPF("point,%d,%d",point.x,point.y);
	if (ToolType == TOOL_FILL)
		{
		if (!(GetKeyState(VK_CONTROL) < 0))
			{
			point.x /= m_nNum;
			point.y /= m_nNum;
			if (!m_pLayers->Layer() && 
					((UINT)point.x < m_swidth) && 
					((UINT)point.y < m_sheight))
				FillIt(point.x,point.y);
			}
		return;
		}
	if (point.x < 0 )
		point.x = 0;
	else if (point.x >= (int)(m_nNum * m_swidth))
		point.x = m_nNum * m_swidth - 1;
	if (point.y < 0 )
		point.y = 0;
	else if (point.y >= (int)(m_nNum * m_sheight))
		point.y = m_nNum * m_sheight - 1;
	DrawPoints(point,0);
}

void CSketchView::DrawPoints(CPoint xpoint, int code)
{
	DPF("drawpoints:%d,%d,%d",xpoint.x,xpoint.y,code);
	if (code == 2)
		{
		m_pBrush->StartBrush();
		if (m_pTablet)
			m_pTablet->ButtonDown(1,m_wMaxPressure/1000,
					m_wMaxPressure % 1000, m_winx, m_winy);
		}

	if (!m_pTablet || !m_bTablet)
		{
		xpoint.x *= m_brush_factor;
		xpoint.x /= m_nNum;
		xpoint.y *= m_brush_factor;
		xpoint.y /= m_nNum;
		m_pBrush->AppendBrush(xpoint);
		if (code == 1)
			{
			m_pBrush->StopBrush();
			if (m_pTablet)
				m_pTablet->ButtonDown(0);
			if ((m_toolflags & (128+3)) == 129)
				{
				m_pLayers->Undo(m_pLayers->LastUndoIndex());
				m_pBrush->Replay();
				Redraw();
				}
			}
		EchoIt();
		return;
		}
		
#ifdef FLIPBOOK_MAC
	CPoint pt1 = GetZeroScrollPosition();
#else
	CPoint pt1 = GetScrollPosition();
#endif

//	int i;
//	int z = m_pTablet->m_pindex;
//	ASSERT(z < 30);
	for (; m_pTablet->m_ocount < m_pTablet->m_icount;m_pTablet->m_ocount++)
		{
		UINT i = m_pTablet->m_ocount % MAX_BUF;
		UINT press = m_pTablet->m_press[i];
//		if ((code == 2) && !i)
//			press = 0;
		if (m_toolflags & 8) 
			m_pBrush->SetDensity((m_maxdensity * press) / 255);
		if (m_toolflags & 4)
			m_pBrush->SetRadius((m_maxradius * press) / 255); ///= 20;
		CPoint point;
		point.x = m_pTablet->m_past[i].x;
		point.y = m_pTablet->m_past[i].y;
#ifndef FLIPBOOK_MAC // fixme 5
#ifdef FBTPC
		point.x -= m_brush_factor*(m_offx - pt1.x);
		point.y -= m_brush_factor*(m_offy - pt1.y);
#else
		point.x -= m_brush_factor*(m_winx + m_offx - pt1.x);
		point.y -= m_brush_factor*(m_winy + m_offy - pt1.y);
#endif
#endif
		point.x *= m_nDen;
		point.y *= m_nDen;
		point.x = point.x / m_nNum;//* m_pBrush->Factor() / m_nNum;
		point.y = point.y / m_nNum;//* m_pBrush->Factor() / m_nNum;
//DPF("x:%d,y:%d",point.x,point.y);
		m_pBrush->AppendBrush(point);
		}
//	ASSERT(z == m_pTablet->m_pindex);
//	m_pTablet->m_pindex = 0;
	if (code == 1)
		m_pBrush->StopBrush();
	EchoIt();
	if (code == 1)
		m_pTablet->ButtonDown(0);
	if ((code == 1)&& (m_toolflags & (128+3)) == 129)
		{
		m_pLayers->Undo(m_pLayers->LastUndoIndex());
		m_pBrush->Replay();
		Redraw();
		}
}


BOOL CSketchView::XlatePoint(CPoint& pt)
{
#ifdef FLIPBOOK_MAC
	CPoint pt1 = GetZeroScrollPosition();
#else
	CPoint pt1 = GetScrollPosition();
#endif
	pt.x -= m_offx;
	pt.y -= m_offy;
	pt.x = (m_nDen * (pt.x + pt1.x)) / m_nNum;
	pt.y = (m_nDen * (pt.y + pt1.y)) / m_nNum;
	return 0;
}

BOOL CSketchView::XlateXPoint(CPoint& pt)
{
	pt.x = ((pt.x * m_nNum) / m_nDen);
	pt.y = ((pt.y * m_nNum) / m_nDen);
	return 0;
}

BOOL CSketchView::XlateCPoint(CPoint& pt, BOOL bDir /* = 0 */)
{
#ifdef FLIPBOOK_MAC
	CPoint pt1 = GetZeroScrollPosition();
#else
	CPoint pt1 = GetScrollPosition();
#endif
	if (bDir)
		{
		pt.x = ((pt.x * m_nNum) / m_nDen) - pt1.x + m_offx;
		pt.y = ((pt.y * m_nNum) / m_nDen) - pt1.y + m_offy;
		}
	else
		{
		pt.x = (m_nDen * (pt.x + pt1.x - m_offx)) / m_nNum;
		pt.y = (m_nDen * (pt.y + pt1.y - m_offy)) / m_nNum;
		}
	return 0;
}

BOOL CSketchView::XlateRect(RECT& dst, RECT&src, BOOL bUseDst)
{
#ifdef FLIPBOOK_MAC
	CPoint pt1 = GetZeroScrollPosition();
#else
	CPoint pt1 = GetScrollPosition();
#endif
DPF("xlate,scrl,x:%d,y:%d",pt1.x,pt1.y);
	if (bUseDst)
		{
//		pt1.x = 0;
//		pt1.y = 0;
		src.left = (m_nDen * (dst.left - m_offx)) / m_nNum;
		src.right= 1 + (m_nDen * (dst.right - 1 - m_offx)) / m_nNum;
		src.top =  (m_nDen * (dst.top - m_offy)) / m_nNum;
		src.bottom = 1 + (m_nDen * (dst.bottom - 1 - m_offy)) / m_nNum;
		}
	else
		{
//		pt1.x = 0;
//		pt1.y = 0;
		dst.left =  m_offx + (m_nNum * src.left) / m_nDen - pt1.x;
		dst.right = m_offx + (m_nNum * src.right + m_nDen - 1) / m_nDen - pt1.x;
//		dst.right = m_offx + (m_nNum * src.right - 1) / m_nDen - pt1.x;
		dst.top =   m_offy + (m_nNum * src.top) / m_nDen - pt1.y;
		dst.bottom =m_offy + (m_nNum * src.bottom + m_nDen - 1) / m_nDen -pt1.y;
//		dst.bottom =m_offy + (m_nNum * src.bottom - 1) / m_nDen -pt1.y;
		}
	return 0;
}

BOOL CSketchView::XlatePoints(POINT * lpt, BOOL bDir /* = 0 */, UINT Cnt /* = 1*/)
{
#ifdef FLIPBOOK_MAC
	CPoint pt1 = GetZeroScrollPosition();
#else
	CPoint pt1 = GetScrollPosition();
#endif
	for (;Cnt--;lpt++)
	if (bDir)
		{
		lpt->x = ((lpt->x * m_nNum) / m_nDen) - pt1.x + m_offx;
		lpt->y = ((lpt->y * m_nNum) / m_nDen) - pt1.y + m_offy;
		}
	else
		{
		lpt->x = (m_nDen * (lpt->x + pt1.x - m_offx)) / m_nNum;
		lpt->y = (m_nDen * (lpt->y + pt1.y - m_offy)) / m_nNum;
		}
	return 0;
}

void CSketchView::DrawColorTrap()
{
	RECT dst,src;
	int x1 = m_grad1.x;
	int x2 = m_grad2.x;
	int y1 = m_sheight - 1 - m_grad1.y;
	int y2 = m_sheight - 1 - m_grad2.y;
	CClientDC ddc(this);
	if (m_gradk == 0)
		{
		int dx = x1 - x2;
		int dy = y1 - y2;
		int r = (int)sqrt((double)dx * dx + dy * dy);
		src.left = x1-5;
		src.top =  y1-5;
		src.right = x1+5;
		src.bottom = y1+5;
		XlateRect(dst,src,0);
		MyDrawObj(&ddc,2,CPoint(dst.left,dst.top),CPoint(dst.right,dst.bottom));
		src.left = x1-r;
		src.top =  y1-r;
		src.right = x1+r;
		src.bottom = y1+r;
		XlateRect(dst,src,0);
		MyDrawObj(&ddc,2,CPoint(dst.left,dst.top),CPoint(dst.right,dst.bottom));
		}
	else
		{
		src.left = x1-5;
		src.top =  y1-5;
		src.right = x1+5;
		src.bottom = y1+5;
		XlateRect(dst,src,0);
		MyDrawObj(&ddc,2,CPoint(dst.left,dst.top),CPoint(dst.right,dst.bottom));
		src.left = x2-9;
		src.top =  y2-9;
		src.right = x2+9;
		src.bottom = y2+9;
		XlateRect(dst,src,0);
		MyDrawObj(&ddc,2,CPoint(dst.left,dst.top),CPoint(dst.right,dst.bottom));
		src.left = x1;
		src.top =  y1;
		src.right = x2;
		src.bottom = y2;
		XlateRect(dst,src,0);
		MyDrawObj(&ddc,0,dst.left,dst.top,dst.right,dst.bottom);
		}
}

void CSketchView::MyDrawBox(CDC * pDC, LPRECT lpRect)
{
	MyDrawObj(pDC,1,lpRect->left,lpRect->top,
			lpRect->right, lpRect->bottom);
}

void CSketchView::MyDrawObj(CDC * pDC, int which, CPoint p1, CPoint p2)
{
	MyDrawObj(pDC,which,p1.x,p1.y,p2.x,p2.y);
}

void CSketchView::MyDrawObj(CDC * pDC, int which, int x1, int y1,int x2, int y2)
{
	RECT rcObj;

	CPen solid (PS_SOLID,1,RGB(255,255,255));
	CPen dotted (PS_DOT,1, RGB(0,0,0));
	
	HPEN hPenOld = (HPEN)pDC->SelectObject(solid);
	int old = pDC->SetROP2(R2_COPYPEN); // first draw solid white
	int i;
	CPoint * pp = 0;
	if ((which == 3) || (which == 4))
		{
		pp = new CPoint[m_nPoints];
		memcpy(pp,m_pPoints, m_nPoints * sizeof(*pp));
		if (which == 4)
			{
			for (i = 0; i < (int)m_nPoints; i++)
				{
				pp[i].x += m_rcSelect.left;
				pp[i].y += m_rcSelect.top;
				}
			}
		XlatePoints(pp,1,m_nPoints);
		}
	for (i = 0; i < 2; i++)
		{
		if (i)
			{
			pDC->SelectObject(dotted); // then draw dotted black
			}
		if  (which == 1)
			{
			pDC->MoveTo(x1,y1);
			pDC->LineTo(x2,y1);
			pDC->LineTo(x2,y2);
			pDC->LineTo(x1,y2);
			pDC->LineTo(x1,y1);
			}
		else if (which == 2)
			{
			rcObj.left = x1;
			rcObj.right = x2;
			rcObj.top = y1;
			rcObj.bottom = y2;
			pDC->Arc(&rcObj,CPoint(x1,y1),CPoint(x1,y1));
			}
		else if ((which == 3) || (which == 4))
			{
			pDC->Polyline(pp, which == 3 ? m_nPoints - 1 : m_nPoints);
			}
		else
			{
			pDC->MoveTo(x1,y1);
			pDC->LineTo(x2,y2);
			}
		}
	pDC->SetROP2(old);
	pDC->SelectObject(hPenOld );
	if (pp)
		delete [] pp;
}

void CSketchView::DrawSelection(BOOL bBlow /* = 0 */)
{
	if (m_nColorTrap)
		{
		DrawColorTrap();
		return;
		}
	if (m_bShiftTrace)
		{
		m_pShiftTrace->Draw();
		return;
		}
	if (!m_nSelect)
		return;
	if (m_nSelect >= 13)
		{
		DrawCurve(bBlow);
		return;
		}
	if ((m_rcSelect.top == m_rcSelect.bottom) &&
					(m_rcSelect.right == m_rcSelect.left))
		{
		if (bBlow)
			m_nSelect = 0;
		return;
		}
	RECT dst;
	CClientDC ddc(this);
	XlateRect(dst,m_rcSelect,0);
	if ((m_nSelect > 9) && (m_nSelect < 13))
		{
		if (m_nSelect == 10)
			{
			MyDrawObj(&ddc,0,dst.left,dst.top,dst.right,dst.bottom);
			}
		else if  (m_nSelect == 11)
			{
			MyDrawBox(&ddc,&dst);
			}
		else if (m_nSelect == 12)
			{
			MyDrawObj(&ddc,2,dst.left,dst.top,dst.right,dst.bottom);
			}
		return;
		}
//DPR("edst",&dst);
//	ddc.DrawFocusRect(&dst);
	if ((m_nSelect == 1) && (m_bLassoing))
		{
		MyDrawObj(&ddc,3,dst.left,dst.top,dst.right, dst.bottom);
		}
	else if ((m_nSelect == 2) && (m_bLassoing))
		{
		MyDrawObj(&ddc,4,dst.left,dst.top,dst.right, dst.bottom);
		}
	else
		MyDrawBox(&ddc,&dst);
	if (bBlow)
		m_nSelect = 0;
}

void CSketchView::DrawCurve(BOOL bBlow /* = 0 */)
{
	POINT points[4];
	if (m_SelectStart == m_Curve4)
		return;
	CClientDC ddc(this);
	CPen solid (PS_SOLID,1,RGB(255,255,255));
	CPen dotted (PS_DOT,1, RGB(0,0,0));
	
	HPEN hPenOld = (HPEN)ddc.SelectObject(solid);
	int old = ddc.SetROP2(R2_COPYPEN); // first draw solid white
	int i;
	for (i = 0; i < 2; i++)
		{
		points[0] = m_SelectStart;
		if (i)
			{
			ddc.SelectObject(dotted); // then draw dotted black
			}
	if ((m_nSelect == 13) || ((m_Curve2 == m_Curve3) && (m_Curve2 == m_Curve4)))
		{
		points[1] = m_Curve4;
		XlatePoints(points, 1, 2);
		ddc.MoveTo(points[0].x, points[0].y);
		ddc.LineTo(points[1].x, points[1].y);
		}
	else
		{
		points[1] = m_Curve2;
		if  (m_nSelect == 14)
			points[2] = m_Curve2;
		else
			points[2] = m_Curve3;
		points[3] = m_Curve4;
//DPF("select:%d",m_nSelect);
//DPF("point0:%d,%d",points[0].x,points[0].y);
//DPF("point1:%d,%d",points[1].x,points[1].y);
//DPF("point2:%d,%d",points[2].x,points[2].y);
//DPF("point3:%d,%d",points[3].x,points[3].y);
		XlatePoints(points, 1, 4);
		ddc.PolyBezier(points,4);
		}
	}
	ddc.SetROP2(old);
	ddc.SelectObject(hPenOld );
}


void CSketchView::FloatPaint()
{
	BYTE * pData = m_pFloat->Data();
	RECT dst, src;
	m_pFloat->GetRect(src);
	BOOL bSrcColor = m_pFloat->m_q ? 1 : 0;
	int sx = src.left;
	int sy = src.top;
	DPR("esrc",&src);
	CClientDC ddc(this);
	XlateRect(dst,src,0);
	BYTE back[4];
	
	back[0] = GetRValue(m_crBack);
	back[1] = GetGValue(m_crBack);
	back[2] = GetBValue(m_crBack);
DPR("edst",&dst);
	if (!pData)
		{
ASSERT(0);
		FloatOutline();
//		ddc.DrawFocusRect(&dst);
		return;
		}
	BYTE tmp[1064];
	LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)&tmp;
	UINT w = src.right - src.left;
	UINT h = src.bottom - src.top;
	
	BOOL bColor = m_pScene->ColorMode();
		

	lpBI->biSize			= sizeof(BITMAPINFOHEADER);
	lpBI->biWidth			= w;
	lpBI->biHeight			= h;
	lpBI->biPlanes			= 1;
	lpBI->biBitCount		= bColor ? 24 : 8;
	lpBI->biCompression		= BI_RGB;
	lpBI->biSizeImage		= w * h;
	lpBI->biXPelsPerMeter	= 0;
	lpBI->biYPelsPerMeter	= 0;
	lpBI->biClrUsed			= bColor ? 0 : 256;
	lpBI->biClrImportant	= 0;
	if (!bColor)
		{
		int i;
		for (i = 0; i < 256; i++)
			{
			tmp[40+4*i+0] = i;
			tmp[40+4*i+1] = i;
			tmp[40+4*i+2] = i;
			tmp[40+4*i+3] = 0;
			}
		for (i = 0; i < 3; i++)
			{
			tmp[40+4*254+i] = (255 * 3 + 85 * (3 - i)) / 4; // blue tint
			tmp[40+4*253+i] = back[i]; // back ground
			}
		}
	UINT x,y;
	UINT sp = m_spitch;
	UINT fp = 4 * ((w + 3) / 4);
	UINT dp;
	if (bColor)
		dp = 4 * ((3 * w + 3) / 4);
	else
		dp = fp;
	BYTE * pSrc = m_pBits;
	BYTE * pMask;
	if (bSrcColor)
		{
		pMask = pData + 4 * fp * h;	// skip over ink paint stuff
		}
	else
		{
		pMask = pData + fp * h;
		}
	BYTE * pDst = pMask + fp * h; // temp display buffer after cur buffer
	UINT iioff = h * fp;
	UINT paoff = 2 * h * fp;
	UINT pioff = 3 * h * fp;
	if (bColor)
		{
		BYTE map[256];
//		MYRGB back;
//		back.rgb = m_crBack;
		CNewPals * pPals = m_pScene->LevelPalette(m_Level);
		if (bSrcColor)
			m_pFloat->PalMap(map, pPals);
		for (y = 0; y < h; y++)
			{
			UINT ay = m_sheight-1-sy-h+1+y;
			for (x = 0; x < w; x++)
				{
				UINT ax = x + sx;
				UINT j;
				UINT ia = pData[y*fp+x];   // ink alpha
				UINT ii;
				BYTE color[4];
				if ((ax < m_swidth) && (ay < m_sheight))
					{
					for (j = 0; j < 3; j++)
						pDst[y*dp+3*x+j] = pSrc[ay*sp+3*ax+j];
					}
				else
					{
					for (j = 0; j < 3; j++)
						pDst[y*dp+3*x+j] = back[j];
					}
				if (bSrcColor)
					{
					UINT pa = pData[y*fp+x+paoff];
					UINT pi = map[pData[y*fp+x+pioff]];
					if ((ax < m_swidth) && (ay < m_sheight))
						pPals->Color(color,pi,ax,ay);
					else
						pPals->Color(color,pi,0,0);
					pa = pa * color[3] / 255;
					if (pa == 255)
						{
						for (j = 0; j < 3; j++)
#ifdef FBVER7
							pDst[y*dp+3*x+j] = color[j];
#else
							pDst[y*dp+3*x+j] = color[2-j];
#endif
						}
					else if (pa)
						{
						for (j = 0; j < 3; j++)
							{
							WORD v = (255 - pa) * pDst[y*dp+3*x+j];
#ifdef FBVER7
							v += pa * color[j];
#else
							v += pa * color[2-j];
#endif
							pDst[y*dp+3*x+j] = v / 255;
							}
						}
					ii = map[pData[y*fp+x+iioff]];
					}
				else
					{
					ia = 255 - ia;
					ii = 0;
					}
				if ((ax < m_swidth) && (ay < m_sheight))
					pPals->Color(color,ii,ax,ay);
				else
					pPals->Color(color,ii,0,0);
				ia = ia * color[3] / 255;
				if (ia == 255)
					{
					for (j = 0; j < 3; j++)
#ifdef FBVER7
						pDst[y*dp+3*x+j] = color[j];
#else
						pDst[y*dp+3*x+j] = color[2-j];
#endif
					}
				else if (ia)
					{
					for (j = 0; j < 3; j++)
						{
						WORD v = (255 - ia) * pDst[y*dp+3*x+j];
#ifdef FBVER7
						v += ia * color[j];
#else
						v += ia * color[2-j];
#endif

						pDst[y*dp+3*x+j] = v / 255;
						}
					}
				if(pMask[y*w+x])
					{
					for (j = 0; j < 3; j++)
						{
						UINT v = pDst[y*dp+3*x+j];
						v = 3 * v + 85 * (3 - j); // blue tint
						pDst[y*dp+3*x+j] = v / 4;
						}
					}
				}
			}
		}
	else			// bw mode
		{
		for (y = 0; y < h; y++)
			{
			UINT ay = m_sheight-1-sy-h+1+y;
			for (x = 0; x < w; x++)
				{
				UINT ax = x + sx;
				UINT v1;
				if ((ax < m_swidth) && (ay < m_sheight))
					v1 = pSrc[ay*sp+ax];
				else
					v1 = 255;
			//		UINT v2 = 255 - pData[y*fp+x];
					UINT v2 = pData[y*fp+x];
					if (!bSrcColor)
						v2 = 255 - v2;
					if (v2 == 255)
						v1 = 0;
					else if (v2)
						v1 = (v1*(255-v2)) / 255;
					if (v1 == 255)
						{
						if (pMask[y*w+x])
							v1 = 254;
						else if ((ax >= m_swidth) || (ay >= m_sheight))
							v1 = 253;
					}
			//		}
			//	else
			//		v1 = pMask[y*w+x] ? 254 : 253;
				pDst[y*dp+x] = v1;
				}
			}
		}
	DrawDibDraw(m_hdd,
              ddc.m_hDC,
				dst.left, dst.top,
                dst.right - dst.left, dst.bottom - dst.top,	
                lpBI, pDst,	
				0,0,w, h,
                0);
	FloatOutline();
}

void CSketchView::FloatOutline()
{
	CClientDC ddc(this);
	POINT corners[5];
	m_pFloat->GetPoints(corners,4);
	XlatePoints(corners,1,4);
	corners[4] = corners[0];
	CPen solid (PS_SOLID,1,RGB(255,255,255));
	CPen dotted (PS_DOT,1, RGB(0,0,0));
	
	HPEN hPenOld = (HPEN)ddc.SelectObject(solid);
	int old = ddc.SetROP2(R2_COPYPEN); // first draw solid white
	int i;
	for (i = 0 ; i < 2; i++)
		{
		if (i)
			ddc.SelectObject(dotted); // then draw dotted black
		ddc.Polyline(corners,5);
		}
	ddc.SelectObject(hPenOld);
	ddc.SetROP2(old);
}

void CSketchView::EchoIt()
{
//	int	diby; //dibx;
//	int offx;
	if (!m_pLayers->m_bDirty || !m_pDIB || !m_hdd)
		return;
//	m_pLayers->Update(m_pBits, 
//				m_pScene->ColorMode() ? m_pBG : m_pSkin, m_spitch);
	m_pLayers->Update(m_pBits, m_pSkin, m_spitch);
	int sx, sy, sw, sh;
	sx = m_pLayers->m_minx;
	sy = m_pLayers->m_miny;
	sw = 3 + m_pLayers->m_maxx - sx;
	sh = 3 + m_pLayers->m_maxy - sy;
	m_pLayers->m_maxx = m_pLayers->m_maxy = 0;
	m_pLayers->m_minx = m_swidth - 1;
	m_pLayers->m_miny = m_sheight - 1;
//DPR("esrc",&src);
		if ((sh <= 0) || (sw <= 0))
			return;
	CClientDC ddc(this);
//DPR("edst",&dst);
	MyCursor(0, ddc.m_hDC);
#ifdef FLIPBOOK_MAC
		CPoint pt1 = GetZeroScrollPosition();
#else
		CPoint pt1 = GetScrollPosition();
#endif
	int dx, dy, dx1, dy1;
	if (sx > 0) sx--;
	if (sy > 0) sy--;
	if ((sy+ sh) > (int)m_sheight) sh = m_sheight-sy;
	if ((sw+sx) > (int)m_swidth)  sw = m_swidth-sx;
	dx = m_nNum * sx / m_nDen;
	dy = m_nNum * sy / m_nDen;
	dx1 = dx + sw * m_nNum / m_nDen;
	dy1 = dy + sh * m_nNum / m_nDen;
	int dw = dx1 - dx;
	int dh = dy1 - dy;
	int adx = dx;
	int ady = dy;
	dx -= pt1.x;
	dy -= pt1.y;
	dx += m_offx;
	dy += m_offy;
DPF("dx:%d,adx:%d",dx,adx);
#if !MAC
	if (m_nNum > m_nDen)
		{
		MyDraw(ddc.m_hDC,dx,dy,adx,ady,dw,dh);
		}
	else
#endif
		{
		DrawDibDraw(m_hdd,
               ddc.m_hDC,
          		dx,dy,dw,dh,
                (LPBITMAPINFOHEADER)m_pDIB, m_pBits,	
				sx, sy,sw,sh, 
                0);
		}
		
/*
	else
		{
		RECT dst;
		XlateRect(dst,src,0);
		DrawDibDraw(m_hdd,
               ddc.m_hDC,
          		dst.left,dst.top,						// upper left corner
                dst.right - dst.left, dst.bottom - dst.top,	
                (LPBITMAPINFOHEADER)m_pDIB, m_pBits,	
				src.left, src.top, 
				src.right - src.left, src.bottom - src.top,
                0);
		}
*/
	MyCursor(1,ddc.m_hDC);
}


void CSketchView::DoHard8(int dx, int dy, int dh,int dw)
{
	int extra = 40 + 1024;
	BYTE * dp = m_pZoom + extra;
	BYTE * sp = m_pBits;
	UINT op = 4 * ((m_zw * 8 + 31) / 32);
	int ip = 4 * ((m_swidth + 3) / 4);
	int x, y;
	dp += op * (dh - 1);
	for (y = 0;y < dh;y++,dp-=op)
		{
		int oy = (y + dy) * m_nDen;
		int sy1 = oy / m_nNum;
		if ((UINT)sy1 >= m_sheight) sy1 = m_sheight - 1;
		BYTE * sp1 = sp + ip * (m_sheight - 1 - sy1);
		for (x = 0; x < dw; x++)
			{
			int sx1 = (x + dx) * m_nDen / m_nNum;
			if ((UINT)sx1 >= m_swidth)
				sx1 = m_swidth - 1;
			dp[x] = sp1[sx1];
			}
		}
}

void CSketchView::DoHard24(int dx, int dy, int dh,int dw)
{
	int extra = 40;
	BYTE * dp = m_pZoom + extra;
	BYTE * sp = m_pBits;
	UINT op = 4 * ((dw * 24 + 31) / 32);
	int ip = 4 * ((3 * m_swidth + 3) / 4);
	int x,y;
	dp += op * (dh - 1);
	for (y = 0;y < dh;y++,dp -= op)
		{
		int oy = (y + dy) * m_nDen;
		int sy1 = oy / m_nNum;
		if ((UINT)sy1 >= m_sheight) sy1 = m_sheight - 1;
		BYTE * sp1 = sp + ip * (m_sheight - 1 - sy1);
		for (x =0; x < dw; x++)
			{
			int ox = x + dx;
			int sx1 = m_nDen * ox / m_nNum;
			if ((UINT)sx1 >= m_swidth)
				sx1 = m_swidth - 1;
			dp[3*x+0] = sp1[3*sx1+0];
			dp[3*x+1] = sp1[3*sx1+1];
			dp[3*x+2] = sp1[3*sx1+2];
			}
		}
}

void CSketchView::DoSoft8(int dx, int dy, int dh,int dw)
{
	int extra = 40 + 1024;
	BYTE * dp = m_pZoom + extra;
	BYTE * sp = m_pBits;
	UINT op = 4 * ((m_zw * 8 + 31) / 32);
	int ip = 4 * ((m_swidth + 3) / 4);
	int x, y;
	dp += op * (dh - 1);
	UINT sq = m_nNum * m_nNum;
	UINT hq = 1 + m_nNum / 2;
	for (y = 0;y < dh;y++, dp-=op)
		{
		int oy;
		oy = (y + dy) * m_nDen - hq;
		int sy1,sy2;
		int yf1, yf2;
		if (oy < 0)
			{
			sy1 = 0;
			sy2 = 0;
			yf2 = 0;
			}
		else
			{
			sy1 = oy / m_nNum;
			sy2 = sy1 + 1;
			yf2 = oy % m_nNum;
			if (sy2 >= (int)m_sheight)
				{
				sy2 = m_sheight - 1;
				yf2 = m_nNum;
				}
			}
		yf1 = m_nNum - yf2;
		ASSERT(sy1 < (int)m_sheight);
		BYTE * sp1 = sp + ip * (m_sheight - 1 - sy1);
		BYTE * sp2 = sp + ip * (m_sheight - 1 - sy2);
		for (x = 0; x < dw; x++)
			{
			int ox = (x + dx) * m_nDen - hq;
			int xf1, xf2;
			int sx1, sx2;
			if (ox < 0)
				{
				sx1 = 0;
				sx2 = 0;
				xf2 = 0;
				}
			else
				{
				sx1 = ox / m_nNum;
				sx2 = sx1 + 1;
				xf2 = ox % m_nNum;
				if (sx2 >= (int)m_swidth)
					{
					sx2 = m_swidth - 1;
					xf2 = m_nNum;
					}
				}
			xf1 = m_nNum - xf2;
			ASSERT(sx1 < (int)m_swidth);
			UINT v = sp1[sx1] * xf1 * yf1;
				v += sp1[sx2] * xf2 * yf1;
				v += sp2[sx1] * xf1 * yf2;
				v += sp2[sx2] * xf2 * yf2;
			dp[x] = v / sq;
			}
		}
}

void CSketchView::DoSoft24(int dx, int dy, int dh,int dw)
{
	int extra = 40;
	BYTE * dp = m_pZoom + extra;
	BYTE * sp = m_pBits;
	UINT op = 4 * ((dw * 24 + 31) / 32);
	int ip = 4 * ((3 * m_swidth + 3) / 4);
	int x,y;
	dp += op * (dh - 1);
	UINT sq = m_nNum * m_nNum;
	UINT hq = 1 + m_nNum / 2;
	for (y = 0;y < dh;y++,dp -= op)
		{
		int oy;
		oy = (y + dy) * m_nDen - hq;
		int sy1,sy2;
		int yf1, yf2;
		if (oy < 0)
			{
			sy1 = 0;
			sy2 = 0;
			yf2 = 0;
			}
		else
			{
			sy1 = oy / m_nNum;
			sy2 = sy1 + 1;
			yf2 = oy % m_nNum;
			if (sy2 >= (int)m_sheight)
				{
				sy2 = m_sheight - 1;
				yf2 = m_nNum;
				}
			}
		yf1 = m_nNum - yf2;
		ASSERT(sy1 < (int)m_sheight);
		BYTE * sp1 = sp + ip * (m_sheight - 1 - sy1);
		BYTE * sp2 = sp + ip * (m_sheight - 1 - sy2);
		for (x =0; x < dw; x++)
			{
			int ox = (x + dx) * m_nDen - hq;
			int xf1, xf2;
			int sx1, sx2;
			if (ox < 0)
				{
				sx1 = 0;
				sx2 = 0;
				xf2 = 0;
				}
			else
				{
				sx1 = ox / m_nNum;
				sx2 = sx1 + 1;
				xf2 = ox % m_nNum;
				if (sx2 >= (int)m_swidth)
					{
					sx2 = m_swidth - 1;
					xf2 = m_nNum;
					}
				}
			xf1 = m_nNum - xf2;
			ASSERT(sx1 < (int)m_swidth);
			for (int j = 0; j < 3; j++)
				{
			UINT v = sp1[3*sx1+j] * xf1 * yf1;
				v += sp1[3*sx2+j] * xf2 * yf1;
				v += sp2[3*sx1+j] * xf1 * yf2;
				v += sp2[3*sx2+j] * xf2 * yf2;
				dp[3*x+j] = v / sq;
				}
			}
		}
}

void CSketchView::MyDraw(HDC hDC, int sdx, int sdy,int dx, int dy, int dw, int dh)
{
	int bits = m_pScene->ColorMode() ? 24 : 8;
	int extra = 40 + 4 * (bits < 9 ? (1 << bits):0);
	int code = 0;
	UINT dp = 4 * ((dw * bits + 31) / 32);
	UINT siz = extra + dh * dp;
	int sx, sy;
	if (!m_pZoom || (siz > m_zsize))
		code = 1; // need new buffer
//	if (((int)m_zden != m_nDen) || ((int)m_znum != m_nNum) ||
//				(bits != (int)m_zbits) ||
//		(dx < m_zx) || ((dx + dw) > (m_zx + m_zw) ) ||
//				(dy < m_zy) || ((dy + dh) > (m_zy + m_zh) ) )
		code |= 2; // need redo buffer
//DPF("myd,%d,%d,%d,%d,%d,%d,%d",sdx,sdy,dx,dy,dw,dh,code);
	if (code & 1)
		{
		delete [] m_pZoom;
		m_pZoom = new BYTE [siz];
		m_zsize = siz;
		code = 2; // force redo
		}
	if (code & 2)
		{
		m_zw = dw;
		m_zh = dh;
		m_znum = m_nNum;
		m_zden = m_nDen;
		m_zbits = bits;
		LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)m_pZoom;
		memcpy(m_pZoom, m_pDIB, extra);
		lpBI->biWidth			= dw;
		lpBI->biHeight			= dh;
		lpBI->biSizeImage		= m_zsize;
		if (m_bFastZoom)
			{
			if (bits == 8)
				DoHard8(dx,dy,dh,dw);
			else
				DoHard24(dx,dy,dh,dw);
			}
		else
			{
			if (bits == 8)
				DoSoft8(dx,dy,dh,dw);
			else
				DoSoft24(dx,dy,dh,dw);
			}
		sx = sy = 0;
		m_zx = dx;
		m_zy = dy;
		}
	else
		{
		sx = dx - m_zx;
		sy = dy - m_zy;//m_zh - dy - dh;
		DPF("zx:%d,zy:%d,zh:%d,zw:%d,sx:%d,sy:%d",m_zx,m_zy,m_zh,m_zw,sx,sy);
		}
	DrawDibDraw(m_hdd,hDC,
              sdx, sdy,dw,dh,
			(LPBITMAPINFOHEADER)m_pZoom,
				m_pZoom + extra,                    // lpBits
               	sx,sy,dw, dh,
                0);
/*
	BOOL bSuccess = ::SetDIBitsToDevice(hDC,        // hDC
						sdx,sdy,dw,dh,sx,sy,
						
					0, (WORD)(m_zh),
					m_pZoom + extra,                    // lpBits
					  (LPBITMAPINFO)m_pZoom, DIB_RGB_COLORS);            // wUsage	
*/
}

#ifdef MEMDC
void CSketchView::CMyPaint(CDC * ppDC)
{
    CMemDC pDC(ppDC);
#else
void CSketchView::CMyPaint(CDC * pDC)
{
#endif
	RECT rectClip;
	int q = pDC->GetClipBox(&rectClip);
	if (q == ERROR) return;
//	if (q == COMPLEXREGION) return;
//	if (q == NULLREGION) return;
//	if (q != SIMPLEREGION)
//		return;
	if (!m_pDIB || !m_hdd)
		{
		pDC->FillSolidRect(&rectClip, m_crBack);
DPF("cant paint");
		return;
		}
DPF("clip, q:%d",q);
DPR("paint ",&rectClip);
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
if ((rectClip.left > rectClip.right) || 
		(rectClip.bottom < rectClip.top))
		{
		if (m_pFloat)				// in case it is off canvas
			FloatPaint();//pDC);
		return;
		}
		
	DPR("my paint clip ",&rectClip);
#if !MAC
	if (m_nNum > m_nDen)
		{
		MyDraw(pDC->m_hDC,rectClip.left, rectClip.top,
				rectClip.left - m_offx, rectClip.top - m_offy,
			rectClip.right-rectClip.left, rectClip.bottom-rectClip.top);
		}
	else
#endif
	{
		RECT src;
		XlateRect(rectClip,src,1);
		delete [] m_pZoom;
		m_pZoom = 0;
		DrawDibDraw(m_hdd,
                pDC->m_hDC,
				rectClip.left,rectClip.top, 
				rectClip.right - rectClip.left,
				rectClip.bottom - rectClip.top,
                (LPBITMAPINFOHEADER)m_pDIB,	 m_pBits,
				src.left, src.top, 
				src.right - src.left, src.bottom - src.top,
                0);
		}
	if (m_pFloat)
		FloatPaint();//pDC);
}

void CSketchView::CMyPrint(CDC * pDC)
{
	RECT rcDest;
	HPBYTE hpdib;
DPF("print,bret:%d",m_Bret);
	if (m_Bret == 3)
		hpdib = m_pScene->GetCacheP(m_Index);
	else
		hpdib = m_pBits;
	if (!m_pDIB || !hpdib)
		{
DPF("cant print, no dib");
		return;
		}
	int cxPage = pDC->GetDeviceCaps(HORZRES);
	int cyPage = pDC->GetDeviceCaps(VERTRES);
	int cxInch = pDC->GetDeviceCaps(LOGPIXELSX);
	int cyInch = pDC->GetDeviceCaps(LOGPIXELSY);
	rcDest.top = rcDest.left = 0;
	rcDest.bottom = (int)(((double)m_sheight * cxPage * cyInch)
					/ ((double)m_swidth * cxInch));
	rcDest.right = cxPage;

	UINT PalMode = DIB_RGB_COLORS;
DPR("print dst",&rcDest);
	BOOL bSuccess = ::StretchDIBits(pDC->m_hDC,
							   rcDest.left, 
							   rcDest.top,                  // 
							   rcDest.right - rcDest.left,
							   rcDest.bottom - rcDest.top,
							   0,0,m_swidth,m_sheight,
							   hpdib,
                			(LPBITMAPINFO)m_pDIB,	
							   PalMode,                 // wUsage
							   SRCCOPY);                       // dwROP
	DPF("bsuccs:%d",bSuccess);
}

void CSketchView::CDocCursor()
{
	DWORD wCursor;
	UINT nDropping = 0;
	if (m_wToolType == TOOL_SELECT)
		{
		if ((GetKeyState(VK_CONTROL) < 0) && !m_pFloat)
			SetCursor(AfxGetApp()->LoadCursor(IDC_SCRUB));
// 10/20/10 back to arrow for selection
//		else if (m_pDoc->SimpleCursor())
//			SetCursor(AfxGetApp()->LoadStandardCursor(IDC_CROSS));
//		else
	//	if (m_bLassoing)
	//		SetCursor(AfxGetApp()->LoadCursor(IDC_MYLASSO));
	//	else
			SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
		return;
		}
	else if (m_wToolType == TOOL_ZOOM)
		{
		if ((GetKeyState(VK_MENU) < 0) || (GetKeyState(VK_CONTROL) < 0))
			wCursor = IDC_MAGCUR1;
		else
			wCursor = IDC_MAGCUR;
		}
	else if (m_wToolType == TOOL_HAND)
		wCursor = IDC_HANDCUR;
	else if (m_wToolType == TOOL_WAND)
		{
		CPoint point;
		GetCursorPos(&point);
//		DPF("apoint %d,%d",point.x,point.y);
		ScreenToClient(&point);
		CClientDC dc(this);
		dc.DPtoLP(&point);
//		DPF("bpoint %d,%d",point.x,point.y);
		XlatePoint(point);
//		DPF("cpoint %d,%d",point.x,point.y);
		UINT index;
		if (index = m_pLayers->DisPaint(point.x, point.y))
			{
			m_pFrame->Status("Paint");

			wCursor = IDC_MYWAND2;
#ifdef MYBUG
		UINT index = m_pLayers->GetIndex(point.x, point.y);
		UINT alpha = m_pLayers->GetAlpha(point.x, point.y);
DPF("idx:%d,%d",index,alpha);
#endif
			}
		else
			{
m_pFrame->Status();
			wCursor = IDC_MYWAND;
			}
		}
	else if (m_pScene->ColorMode())
		{
		if (m_wToolType == TOOL_EYEDROP)
			nDropping = 1;
		if (GetKeyState(VK_CONTROL) < 0 )
			{
			if (m_wToolType == TOOL_BRUSH ||
				m_wToolType == TOOL_FILL ||
				m_wToolType == TOOL_TRACE ||
				m_wToolType == TOOL_PENCIL)
				nDropping = 1;
			}

		else if (GetKeyState(VK_SHIFT) < 0 )
			{
			if (m_wToolType == TOOL_EYEDROP) //BRUSH ||
//				m_wToolType == TOOL_FILL ||
//				m_wToolType == TOOL_TRACE ||
//				m_wToolType == TOOL_PENCIL)
				nDropping = 2;
			}

		if ((m_wToolType < 2) && Layer())
			wCursor = IDC_TFILL + m_wToolType;
		else
			wCursor = IDC_PENCIL + m_wToolType;
		if (m_pDoc->ToolInfo(0))
			wCursor += 10;
		if (m_bLeftHanded)
			wCursor += 20;
		if (nDropping == 1)
			{
			CPoint point;
			GetCursorPos(&point);
//			DPF("apoint %d,%d",point.x,point.y);
			ScreenToClient(&point);
			CClientDC dc(this);
			dc.DPtoLP(&point);
//			DPF("bpoint %d,%d",point.x,point.y);
			XlatePoint(point);
//		DPF("cpoint %d,%d",point.x,point.y);
			UINT nLayer;
#ifdef FBVER7
			if (m_pLayers->HaveAlpha(point.x, point.y, &nLayer,1))
				{
int a1;
BYTE tt[4];
a1 = m_pLayers->GetPixelAlpha(point.x,point.y,nLayer);
UINT vv = m_pLayers->GetPixelIndex(point.x,point.y,nLayer);
UINT * pd = (UINT *)tt;
*pd = vv;
char buf[80];
//sprintf(buf, m_pLayers->LayerName(nLayer));
sprintf(buf, "%s,%d,%d,%d,%d,%d",m_pLayers->LayerName(nLayer),
			a1,tt[0],tt[1],tt[2],tt[3]);
m_pFrame->Status(buf);

				wCursor = IDC_EYEDROP1;
#ifdef MYBUG
		UINT index = m_pLayers->GetIndex(point.x, point.y);
		UINT alpha = m_pLayers->GetAlpha(point.x, point.y);
DPF("nlayer:%d,idx:%d,%d",nLayer,index,alpha);
#endif
#else
			if (m_pLayers->HaveAlpha(point.x, point.y, &nLayer,m_pBits))
//			if (m_pLayers->HaveAlpha(point.x, point.y))
				{
char buf[80];
sprintf(buf, m_pLayers->LayerName(nLayer));
m_pFrame->Status(buf);

				wCursor = IDC_EYEDROP1;
#ifdef MYBUG
		UINT index = m_pLayers->GetIndex(point.x, point.y);
		UINT alpha = m_pLayers->GetAlpha(point.x, point.y);
DPF("nlayer:%d,idx:%d,%d",nLayer,index,alpha);
#endif
#endif
				}
			else
				{
m_pFrame->Status();
				wCursor = IDC_EYEDROP2;
				}
			}
		else if (nDropping == 2)
			wCursor = IDC_EYEDROP3;
		else
			{
			m_pFrame->Status();
//			if (m_wToolType < TOOL_FILL)
//				return;
			if ((m_wToolType <= TOOL_FILL) && m_pDoc->SimpleCursor())
				{
				#if !MAC
				wCursor = IDC_MYCROSS;
				#else
				SetCursor(AfxGetApp()->LoadStandardCursor(IDC_CROSS));
				return;
				#endif
				}
			}
		}
	else
		{
//DPF("bw doc cursor");
		if (GetKeyState(VK_CONTROL) < 0)
			wCursor = IDC_SCRUB;
		else if (GetKeyState(VK_SHIFT) < 0)
//		else if ((GetKeyState(VK_SHIFT) < 0) &&
//					((m_wToolType != TOOL_PENCIL) ||
//							!(m_pDoc->ToolInfo(5) & 3)))
			wCursor = IDC_FLIP;
		else
			{
			if (m_pDoc->SimpleCursor())
				{
				#if !MAC
				wCursor = IDC_MYCROSS;
				#else
				SetCursor(AfxGetApp()->LoadStandardCursor(IDC_CROSS));
				return;
				#endif
				}
			else
				{
				wCursor = IDC_PENCIL + m_wToolType;
				if (m_pDoc->ToolInfo(0))
					wCursor += 10;
				if (m_bLeftHanded)
					wCursor += 20;
				}
			}
		}
	SetCursor(AfxGetApp()->LoadCursor(wCursor));
}


UINT CSketchView::FillStack(UINT pos)
{
	UINT z = m_pLayers->FillStack(pos);
	if (pos != -1)
		EchoIt();
	return z;
}

int CSketchView::FillIt(int x, int y, int kind/* = 0*/)
{
	if ((m_wToolType == TOOL_FILL) && m_bRDown)
		kind = 50;
	else if (!kind)
		kind = 10 + m_pDoc->Radius(); // for arguments
	m_pLayers->FillOffset(x,y,m_color,m_bErasing,kind);
	if (m_pLayers->Modified())
		m_bModified = TRUE;
	EchoIt();
	return 0;
}

int CSketchView::CNumberFill(int count)
{
	CPoint point;
	GetCursorPos(&point);
	DPF("number fill,%d,%d,%d",count,point.x,point.y);
	ScreenToClient(&point);
	CClientDC dc(this);
	dc.DPtoLP(&point);
	DPF("bpoint %d,%d",point.x,point.y);
	XlatePoint(point);
	if (((UINT)point.x >= m_swidth) || ((UINT)point.y >= m_sheight))
		{
		MessageBeep(0);
		return 1;
		}
	UINT Frame;
	UINT First = m_EditFrame;
//	m_Undo = 0;
//	UINT size = m_sheight * 8 * ((m_swidth+3) / 4);
//	pLayers[LAY_UNDO].kind = layer;
//	pLayers[LAY_UNDO].flags = 1;
//	m_Undo = 1;
//	memmove(pLayers[LAY_UNDO].pData,pLayers[layer].pData,size);
	m_color = m_pDoc->Color();
	m_bErasing = m_pDoc->ToolInfo(0);
	m_pLayers->DrawInit(m_color,0,m_bErasing);
	m_pBrush->SetSolid(m_pLayers->Solid());

	for (;;)
		{
		Frame = m_EditFrame;

		m_bModified = TRUE; 
		FillIt(point.x, point.y);
//		KeepCell();
		
		if (count == 1)
			break;
		if (count)
			count--;
		Frame++;
		if (!m_pScene->FindNextCell(Frame, m_EditLevel))
			{
			count = 0;
			break;
			}
		if (ASelectCell(Frame, m_EditLevel,0,1))
			break;
		}
	if (!count)
		Frame = First;
	else
		{
		Frame++;
		if (!m_pScene->FindNextCell(Frame, m_EditLevel))
			Frame = First;
		}
	VSelectCell(Frame, m_EditLevel);
	return 0;
}
#ifdef XCV
UINT CSketchView::DrawWireFrame(int ddx, int ddy)
{
	if (!m_Camera) return 0;
DPF("draw wire,%,%d",ddx,ddy);
	int dx, dy;
	BOOL bRes;
	CDC dcMemory;
	CClientDC cDC(this);
	dcMemory.CreateCompatibleDC(&cDC);

	CBitmap* pOldBitmap = dcMemory.SelectObject(m_pWireFrame);
/*
	if (m_bDown)
		{
		dx = m_prevx - m_origx;
		dy = m_prevy - m_origy;
		cDC.BitBlt(dx,dy, m_wirew, m_wireh,
				&dcMemory,  0, 0, SRCINVERT);
		}
*/
	dx = ddx - m_origx;
	dy = ddy - m_origy;
	m_prevx = ddx;
	m_prevy = ddy;
	bRes = cDC.BitBlt(dx,dy, m_wirew, m_wireh,
			&dcMemory,  0, 0, SRCINVERT);
	dcMemory.SelectObject(pOldBitmap);
DPF("bitblt,dx:%d,dy:%d,bres:%d",dx,dy,bRes);
	return 0;
}
#endif

