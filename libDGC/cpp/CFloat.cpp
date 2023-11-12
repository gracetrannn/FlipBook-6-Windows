#include "CFloat.h"
#include "NScaler.h"
#include "math.h"
#include "CNewPals.h"

#define PROX 9


CFloat::CFloat()
{
	m_pCur = 0;
	m_pOrig = 0;
	m_bRotated = 0;
	m_bNoMapPalette = 0;
	m_bHasName = 0;
}

CFloat::~CFloat()
{
	delete m_pCur;
	delete m_pOrig;
}

bool CFloat::Rotated(bool bValue, bool bSet /* = 0 */)
{
	if (bSet)
		{
		if (m_bRotated != bValue)
			{
			if (bValue)
				m_bRotated = 1;
			else
				m_bRotated = 0;
			Regen();
			}
		}
	return m_bRotated;
}

void CFloat::GetRect(RECT & rc)
{
	if (m_bRotated)
		{
		rc.left = m_x  - m_diag / 2;
		rc.top = m_y - m_diag / 2;
		rc.right = rc.left + m_diag;
		rc.bottom = rc.top + m_diag;
		}
	else
		{
		rc.left = m_x - m_curw / 2;
		rc.top = m_y - m_curh / 2;
		rc.right = rc.left + m_curw;
		rc.bottom = rc.top + m_curh;
		}
}

void CFloat::GetPoints(POINT * lpt, UINT cnt)
{
	if (!m_bRotated)
		{
		int hw = m_curw / 2;
		int hh = m_curh / 2;
		lpt->x = m_x - hw;
		lpt->y = m_y - hh;
		lpt++;
		lpt->x = m_x - hw + m_curw - 1;
		lpt->y = m_y - hh;
		lpt++;
		lpt->x = m_x - hw + m_curw - 1;
		lpt->y = m_y - hh + m_curh - 1;
		lpt++;
		lpt->x = m_x - hw;
		lpt->y = m_y - hh + m_curh - 1;
		return;
		}
	lpt->x = m_x - m_curw / 2;
	lpt->y = m_y - m_curh / 2;
	lpt++;
	lpt->x = m_x - m_curw / 2 + m_curw - 1;
	lpt->y = m_y - m_curh / 2;
	lpt++;
	lpt->x = m_x - m_curw / 2 + m_curw - 1;
	lpt->y = m_y - m_curh / 2 + m_curh - 1;
	lpt++;
	lpt->x = m_x - m_curw / 2;
	lpt->y = m_y - m_curh / 2 + m_curh - 1;
	lpt -= 3;
	int i;
	double twopi = 8.0 * atan(1.0);
	double angle = m_angle;
	angle = angle * twopi / 360;
	double	ssin = sin(angle);
	double	ccos = cos(angle);
	for (i = 0; i < 4; i++,lpt++)
		{
		int x = lpt->x - m_x;
		int y = lpt->y - m_y;
		double xx = x;
		double yy = y;
		lpt->x = m_x + (int)(xx * ccos + yy * ssin);
		lpt->y = m_y + (int)(xx * ssin - yy * ccos);
		}
}

void CFloat::Regen()
{

	if (m_bRotated)
		{
		RotateIt();
		return;
		}
	delete m_pCur;
	BYTE * pData;
	UINT size;
	CGScaler scale;
	UINT xp = 4 * ((3*m_curw + 3) / 4);
	UINT dp = 4 * ((m_curw + 3) / 4);
	UINT sp = 4 * ((m_w + 3) / 4);
	BYTE * pSrc = m_pOrig;
	if (m_q)
		{
		UINT size = 5 * m_curh * dp; // ink,paint,index,alpha
		m_pCur = new BYTE[size+m_curh*xp]; // plus room for 24 bit composite
		pData = m_pCur;
		int i;
		for (i = 0; i < 4; i++, pSrc += sp * m_h)
			{
			if (i)
				{
				UINT x,y;
				for (y = 0; y < m_curh;y++, pData += dp)
					{
					int iy = (y * m_h + m_curh / 2) / m_curh;
					for (x = 0; x < m_curw;x++)		// indexes cannot be scaled
						{
						int ix = (x * m_w + m_curw / 2) / m_curw;
						pData[x] = pSrc[sp*iy+ix];
						}
					}
				}
			else
				{
				scale.Init(m_w,m_h,8,m_curw,m_curh);
				int q = scale.Custom(pData, pSrc, dp, sp);
DPF("after custom:%d",q);
				int z = scale.Copy();
				pData += dp * m_curh;
				}
			}
		}
	else
		{
		size = 2 * m_curh * dp;
		m_pCur = new BYTE[size+m_curh*xp]; // extra for compositing area
		pData = m_pCur;
		scale.Init(m_w,m_h,8,m_curw,m_curh);
		int q = scale.Custom(pData, m_pOrig, dp, sp);
DPF("after custom:%d",q);
		int z = scale.Copy();
DPF("after scale:%d",z);
		pData += dp * m_curh;
		pSrc += sp * m_h;
		}
//
// now do mask
//
	UINT x,y;
	for (y = 0; y < m_curh;y++)
		{
		int iy = (y * m_h + m_curh / 2) / m_curh;
		for (x = 0; x < m_curw;x++)		// now edge smooth
			{
			int ix = (x * m_w + m_curw / 2) / m_curw;
			pData[m_curw*y+x] = pSrc[m_w*iy+ix];
			}
		}
	
}

void CFloat::SetOrig(UINT ww, UINT hh, UINT d, BYTE * pSrc)
{
	delete [] m_pCur;
	m_pCur = 0;
	delete [] m_pOrig;
	m_angle = 0;
	m_curw = m_w;
	m_curh = m_h;
	m_w = ww;
	m_h = hh;
	m_q = d & 1;
	UINT x,y,p, sp,h;
	BYTE * pDst;
	sp = m_w;
	p = 4 * ((m_w +3) / 4);
	if (m_q)
		h = 4 * m_h;
	else
		h = m_h;
	m_pOrig = new BYTE[(h+m_h)*p];
	pDst = m_pOrig;
	for (y = 0; y < h; y++)
	for (x = 0; x < m_w; x++)
		pDst[y*p+x] = pSrc[y*sp+x]; 
	if (d & 8)
		memcpy(pDst+h*p,pSrc+h*sp,sp*m_h);	// mask is not 32 bit aligned
	else
		memset(pDst+h*p,255,sp*m_h);	// mask is not 32 bit aligned
}

void CFloat::Make(BYTE  * pSrc)
{
	UINT * pHead = (UINT *)pSrc;
	UINT code,x,y,w,h;
	code = *pHead++;
	x = *pHead++;
	y = *pHead++;
	w = *pHead++;
	h = *pHead++;
	BYTE * pData = (BYTE *)pHead;
	m_bShiftOn = 0;
	m_bAspectHold = (code & 2) ? 1 : 0;
	m_bHasName = (code & 4) ? 1 : 0;
	if (code & 1)
		{
		int i;
		for (i = 0; i < 1024; i++)
			m_pals[i] = *pData++;
		}
	if (m_bHasName)
		{
		memcpy(m_name, pData, 16);
		m_name[15] = 0;
		pData += 16;
		}		
	m_x = x + w / 2;
	m_y = y + h / 2;
	m_w = w;
	m_h = h;
	SetOrig(w,h,code,pData);
	m_angle = 0;
//	m_bRotated = 1;
	m_diag = (UINT)sqrt((DOUBLE)(m_curw * m_curw + m_curh * m_curh));
	Regen();
}

void CFloat::Make(RECT & rect, BYTE * pData)
{
	if (m_pOrig)
		return;
	UINT * pHead = (UINT *)pData;
	UINT code,x,y,w,h;
	code = *pHead++;
	x = *pHead++;
	y = *pHead++;
	w = *pHead++;
	h = *pHead++;
	BYTE * pData1 = (BYTE *)pHead;
	m_bShiftOn = 0;
	m_bAspectHold = (code & 2) ? 1 : 0;
	m_bHasName = (code & 4) ? 1 : 0;
	if (code & 1)
		{
		int i;
		for (i = 0; i < 1024; i++)
			m_pals[i] = *pData1++;
		}
	if (m_bHasName)
		{
		memcpy(m_name, pData1, 16);
		m_name[15] = 0;
		pData1 += 16;
		}		
	m_angle = 0;
	m_x = rect.left;
	m_y = rect.top;
	m_w = rect.right - m_x;
	m_h = rect.bottom - m_y;
	m_x += m_w / 2;
	m_y += m_h / 2;
	SetOrig(w,h,code,pData1);
	Regen();
}

//void CFloat::FillIn( UINT w, UINT h, UINT d, BYTE * pSrc)
//{
//	SetOrig(w,h,d,pSrc);
//	Regen();
//}


UINT CFloat::Hit(CPoint point, bool bShift, bool bRotate /* = 0 */)
{
DPF("hit:%d,%d,%d,%d",m_x,m_y,point.x,point.y);
	if (m_bRotated || bRotate)
		{
		UINT i;
		POINT corners[5];
		bool bTemp = m_bRotated;
		m_bRotated = TRUE;
		GetPoints(corners,4);
		m_bRotated = bTemp;
		corners[4].x = m_x;
		corners[4].y = m_y;
		UINT max;
		if (m_curh > m_curw)
			max = m_curw;
		else
			max = m_curh;
		max = max / 2;
		max = max * max;
		for (i = 5; i-- > 0;)
			{
			int dx = point.x - (int)corners[i].x;
			int dy = point.y - (int)corners[i].y;
			if ((dx * dx + dy * dy) < (int)max)
				return 20 + i;
			max = PROX * PROX;
			}
		return 0;
		}
	bool bAspectHold = m_bAspectHold || m_bShiftOn || bShift;
	int x = m_x - m_curw / 2;
	int y = m_y - m_curh / 2;
	if ((point.x + PROX) < x)
		return 0;
	if ((point.y + PROX) < y)
		return 0;
	if ((point.x -PROX) >= (x+(int)m_curw))
		return 0;
	if ((point.y -PROX) >= (y+(int)m_curh))
		return 0;
	int qx = bAspectHold ? m_curw / 2 : PROX;
	if ((point.y - PROX) < y)
		{
		if ((point.x - qx) < x)
			return 1;
		else if ((point.x + qx) >= (x+(int)m_curw))
			return 3;
		else
			return 2;
		}
	if ((point.y + PROX) >= (y+(int)m_curh))
		{
		if ((point.x - qx) < x)
			return 7;
		else if ((point.x + qx) >= (x+(int)m_curw))
			return 5;
		else
			return 6;
		}
	if ((point.x - PROX) < x)
		{
		if (bAspectHold)
			{
			if (point.y < m_y)
				return 1;
			else
				return 7;
			}
		return 8;
		}
	else if ((point.x + PROX) >= (x+(int)m_curw))
		{
		if (bAspectHold)
			{
			if (point.y < m_y)
				return 3;
			else
				return 5;
			}
		return 4;
		}
	return 9;
}

UINT CFloat::StartDrag(CPoint point)
{
DPF("startdrag,%d,%d",point.x,point.y);
	ASSERT(m_pOrig);
	if (m_bRotated)
		{
//		m_angle = 0;//30;
//		m_diag = (UINT)sqrt(m_curw * m_curw + m_curh * m_curh);
//		m_x = m_x + m_curw / 2 - m_diag / 2; 
//		m_y = m_y + m_curh / 2 - m_diag / 2; 
		m_code = Hit(point);
		}
	else
		{
		m_code = Hit(point);
		}
	if (!m_code)
		return 0;
	m_ox = m_x;
	m_oy = m_y;
	m_px = point.x;
	m_py = point.y;
	m_ow = m_curw;
	m_oh = m_curh;
	return m_code;
}

UINT CFloat::DragIt(CPoint point)
{
	int t,dx,dy;
	if (!m_code)
		return 1;
	dx = point.x - m_px;
	dy = point.y - m_py;
	bool bHold = m_bShiftOn || m_bAspectHold;
	if (m_bRotated)
		{
		if (m_code == 24)
			{
			m_x = m_ox + dx;
			m_y = m_oy + dy;
			}
		else
			{
			double angle1 = atan2((double)(m_px - m_ox),
					(double)(m_py - m_oy));
			double angle2 = atan2((double)(point.x - m_ox),
					(double)(point.y - m_oy));
			angle1 -= angle2;
			double twopi = 8.0 * atan(1.0);
			m_angle += (360.0 * angle1) / twopi;
			m_px = point.x;
			m_py = point.y;
			}
		Regen();
		return 0;
		}
	if (m_code == 9)
		{
		m_x = m_ox + dx;
		m_y = m_oy + dy;
		return 0;
		}
	int mask = 0;
	switch (m_code) {
	case 1:
		mask = 1 + 2;
		break;
	case 2:
		mask = 1;
		bHold = 0;
		break;
	case 3:
		mask = 1 + 8;
		break;
	case 4 :
		mask = 8;
		bHold = 0;
		break;
	case 5 :
		mask = 4 + 8;
		break;
	case 6:
		mask = 4;
		bHold = 0;
		break;
	case 7:
		mask = 2 + 4;
		break;
	case 8:
		mask = 2;
		bHold = 0;
		break;
	default:
		ASSERT(0);
		break;
	}
	int ax = m_ox - m_ow / 2;
	int ay = m_oy - m_oh / 2;
	if (bHold)
		{
		bool bWide = (m_ow * m_curh > m_oh * m_curw) ? 1 : 0;
		int tx = ax + m_ow;
		int ty = ay + m_oh;
		if (mask & 4)
			{
			ty += dy;
			if (ty <= ay)
				ty = ay + 1;
			}
		else
			{
			ay += dy;
			if (ay >= ty)
				ay = ty - 1;
			}
		if (mask & 8)
			{
			tx += dx;
			if (tx <= ax)
				tx = ax + 1;
			}
		else
			{
			ax += dx;
			if (ax >= tx)
				ax = tx - 1;
			}
		m_curh = ty - ay;
		m_curw = tx - ax;
		if (m_ow * m_curh > m_oh * m_curw)
			m_curh = m_oh * m_curw / m_ow;
		else
			m_curw = m_ow * m_curh / m_oh;
		if (!(mask & 8))
			ax = tx - m_curw;
		if (!(mask & 4))
			ay = ty - m_curh;
		m_x = ax + m_curw / 2;
		m_y = ay + m_curh / 2;
		mask = 0;
		}
	if (mask & 1)
		{
		t = ay + m_oh;
		ay += dy;
		if (ay >= t)
			ay = t - 1;
		m_curh = t - ay;
		m_y = ay + m_curh / 2;
		}
	else if (mask & 4)
		{
		t = ay + m_oh + dy;
		if (t <= ay)
			t = ay + 1;
		m_curh = t - ay;
		m_y = ay + m_curh / 2;
		}
	if (mask & 2)
		{
		t = ax + m_ow;
		ax += dx;
		if (ax >= t)
			ax = t - 1;
		m_curw = t - ax;
		m_x = ax + m_curw / 2;
		}
	else if (mask & 8)
		{
		t = ax + m_ow + dx;
		if (t <= ax)
			t = ax + 1;
		m_curw = t - ax;
		m_x = ax + m_curw / 2;
		}
	Regen();
	return 0;
}


void CFloat::RotateOne(BYTE * pDst, BYTE * pSrc, bool bAvg, bool bMask)
{
	int  dcx, dcy,scx, scy;
	UINT op = bMask ? m_diag : 4 * ((m_diag + 3) / 4);
	UINT ip = bMask ? m_w : 4 * ((m_w + 3) / 4);
	memset(pDst, m_q ? 0 : 255, op * m_diag);
	double twopi = 8.0 * atan(1.0);
	double angle = m_angle;
	angle = angle *twopi / 360;
	double	ssin = sin(angle);
	double	ccos = cos(angle);
	scx = m_w / 2;
	scy = m_h / 2;
	dcx = m_diag / 2;
	dcy = m_diag / 2;
	int ox, oy;
	double sx = m_w;
	sx /=  m_curw;
	double sy = m_h;
	sy /=  m_curh;
	UINT mx = 255 * m_w;
	UINT my = 255 * m_h;
	for (oy = 0; oy < (int)m_diag; oy ++)
		{
		for (ox = 0; ox < (int)m_diag; ox++)
			{
			double x = ox - dcx;
			double y = oy - dcy;
			x *= sx;
			y *= sy;
			double ix = scx + x * ccos - y * ssin;
			double iy = scy + x * ssin + y * ccos;
//			BYTE v;
			ix *= 255;
			iy *= 255;
			UINT xx = 127 + (UINT)(INT)ix;
			UINT yy = 127 + (UINT)(INT)iy;
			if ((xx >= mx) || (yy >= my))
				{
				if (!m_q)
					pDst[oy * op + ox] = bMask ? 0 : 255;
				continue;
				}
			if (!bAvg)
				{
				pDst[oy * op + ox] = pSrc[ip * (yy / 255) + xx / 255];
				continue;
				}
			UINT x1, x2,fx;
			UINT y1, y2,fy;
			y1 = 255 * (yy / 255);
			fy = yy - y1;
			if (fy > 127)
				{
				y1 = yy / 255;
				y2 = y1 + 1;
				if (y2 >= m_h)
					{
					fy = 255;
					y2 = y1;
					}
				else
					fy = 383 - fy;
				}
			else
				{
				fy += 128;
				y1 = yy / 255;
				y2 = y1 - 1;
				if (!y1)
					{
					fy = 255;
					y2 = y1;
					}
				}
			x1 = 255 * (xx / 255);
			fx = xx - x1;
			if (fx > 127)
				{
				x1 = xx / 255;
				x2 = x1 + 1;
				if (x2 >= m_w)
					{
					fx = 255;
					x2 = x1;
					}
				else
					fx = 383 - fx;
				}
			else
				{
				x1 = xx / 255;
				x2 = x1 - 1;
				fx = 128 + fx;
				if (!x1)
					{
					fx = 255;
					x2 = x1;
					}
				}
			UINT sum = 0;
			sum += fy * fx * (UINT)pSrc[ip * y1 + x1];
			sum += fy * (255 - fx) * (UINT)pSrc[ip * y1 + x2];
			sum += (255 - fy) * fx * (UINT)pSrc[ip * y2 + x1];
			sum += (255 - fy) * (255 - fx) * (UINT)pSrc[ip * y2 + x2];
			sum /= 255 * 255;
			pDst[oy * op + ox] = sum;
			}
		}

}
void CFloat::RotateIt()
{
	delete m_pCur;
	BYTE * pData;
	BYTE * pSrc = m_pOrig;
	UINT size;
	m_diag = (UINT)sqrt((double)(m_curw * m_curw + m_curh * m_curh));
	UINT xp = 4 * ((3*m_diag+3) / 4);
	UINT dp = 4 * ((m_diag +3) / 4);
	UINT sp = 4 * ((m_w + 3) / 4);
	if (m_q)
		{
		size = 5 * m_diag * dp;
		m_pCur = new BYTE[size+m_diag*xp];
		pData = m_pCur;
		int i;
		for (i = 0; i < 4; i++)
			{
			RotateOne(pData, pSrc,i ? 0 : 1,0);
			pData += m_diag * dp;
			pSrc += sp * m_h;
			}
		}
	else
		{
		size = 2 * m_diag * dp;
		m_pCur = new BYTE[size+m_diag*xp];
		pData = m_pCur;
		RotateOne(pData, pSrc,1,0);
		pData += m_diag * dp;
		pSrc += sp * m_h;
		}
	RotateOne(pData, pSrc,0,1);
}

void CFloat::PalMap(BYTE * pMap, CNewPals * pPals)
{
	int i;
	if (!m_q || m_bNoMapPalette)
		{
		for (i = 0; i < 256; i++)
			pMap[i] = i;
		return;
		}
	for (i = 0; i < 256; i++)
		{
		int best = 0;
		int ee = 500000;
		int j;
		int r = (int)m_pals[4*i+0];
		int g = (int)m_pals[4*i+1];
		int b = (int)m_pals[4*i+2];
		int a = (int)m_pals[4*i+3];
		for (j = 0; j < 256; j++)
			{
			int dr = r - (int)pPals->Red(j);
			int dg = g - (int)pPals->Green(j);
			int db = b - (int)pPals->Blue(j);
			int da = a - (int)pPals->Alpha(j);
			int e = dr * dr + dg * dg + db * db + da * da;
			if (e < ee)
				{
				ee = e;
				best = j;
				}
			}
//best = 0;
		pMap[i] = best;
		}
}
