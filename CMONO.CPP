#include "stdafx.h"
#include "clayers.h"
#ifdef DOMONO
#define FASTMONO
#define MAX_RADIUS 80
#define MAX_MASK (1 + 2 * MAX_RADIUS) * (1 + 2 * MAX_RADIUS)

class CLayer
{
public:
	UINT	kind;
	UINT	flags;	// 1 is modified, 2 is active
	BYTE *	pData;
};


static int dt[25] = {22,20,12,16,24,
					 15, 8, 1, 7,17,
					 11, 6, 0, 4, 9,
					 19, 3, 5, 2,13,
					 23,14,10,18,21};

//#ifdef FASTMONO
static BYTE mdt[650];
#endif
void CLayers::DrawBegin(UINT x, UINT y, UINT radius, UINT density)
{
#ifdef DOMONO
	if (!radius)
		radius = 2;
	else if (radius == 1)
		radius = 4;
	else
		radius = (radius - 1) * m_nMono;
//	radius *= m_nMono;
//	if (!radius)
//		radius = (m_nMono + 1) / 2;
//	else
	m_density = density;

	if (radius != m_radius)
		{
		m_radius = radius;
		MakeMonoMask();
		}
	x = x * m_nMono + 2;
	y = y * m_nMono + 2;
	MonoMask(0,0,x,y);
#endif
	m_prevx = x;
	m_prevy = y;
}
//
//	x,y in gray space
//
void CLayers::DrawAgain(UINT x, UINT y)
{
#ifdef DOMONO
	x = x * m_nMono + 2;
	y = y * m_nMono + 2;
	if ((x != m_prevx) || (y != m_prevy))
		MonoLine(x,y);
#endif
}

void CLayers::Change(UINT which, UINT v)
{
#ifdef DOMONO
	if (m_bFirstDot)
		return;
	if (!v)
		v = 2;
	else if (v == 1)
		v = 4;
	else
		v = (v - 1) * m_nMono;
//	m_density = density;
	if (v != m_radius)
		{
		if (v > m_radius)
			{
			MakeDeltaMask(v);
			m_radius = v;
			MonoMask(0,0,m_prevx,m_prevy);
			MakeMonoMask();
			}
		else
			{
			m_radius = v;
			MakeMonoMask();
			}
		}
#endif
}


#ifdef DOMONO
void CLayers::InitMono()
{
	int i,j;
	for (i = 0; i < 26; i++)
		{
		for (j = 0; j < 25;j++)
			{
			if (i > dt[j])
				mdt[25*i+j] = 255;
			else
				mdt[25*i+j] = 0;
			}
		}
}
//#endif
//
//	puts single dot into mono
//	x,y in mono space
//
void CLayers::DrawMono(UINT x, UINT y)
{
	if ((UINT)x >= m_nMono * m_width)
		return;
	if ((UINT)y >= m_nMono * m_height)
		return;
	UINT gx = x / m_nMono;
	UINT gy = y / m_nMono;
	if (m_bFirstDot)
		{
		m_minx = gx;
		m_maxx = gx;
		m_miny = gy;
		m_maxy = gy;
		m_bFirstDot = FALSE;
		}
	else
		{
		if ((int)gx > m_maxx)
			m_maxx = gx;
		else if ((int)gx < m_minx)
			m_minx = gx;
		if ((int)gy > m_maxy)
			m_maxy = gy;
		else if ((int)gy < m_miny)
			m_miny = gy;
		}
	UINT mp = m_width * m_nMono;
	y = m_height * m_nMono - 1 - y;
	m_pMono[y * mp + x] = m_bErasing ? 0 : 255;
	BYTE * pIndex = m_pLayers[m_nInk].pData;
	gy = m_height - 1 - gy;
	if (!m_bErasing)
	pIndex[m_pitch*(m_height + gy) + gx] = m_color;
	m_bMonoModified = 1;
	m_bDirty = TRUE;
}

void CLayers::MonoMask(int dx, int dy, UINT xx, UINT yy)
{
	int i,ix,iy,x,y,z;
	int m,r;
	r = m_radius;
	y = yy - r;
	z = 1 + 2 * r;
	m = 3 * (1 + dy) + 1 + dx;
	BYTE * pMask = m_pMask + m * MAX_MASK;
	for (i = 0, iy = 0; iy < z; iy++,y++)
		{
		x = xx - r;
		for (ix = 0; ix < z; ix++,i++,x++)
			if (pMask[i] && ((UINT)(rand() % 100) < m_density))
//			if (pMask[i])
				DrawMono(x,y);
		}

/*

	int xx = x;
	int yy = y;
	int i, j,r;
	r = m_radius + 1;
	for (i = -r; i <= r; i++)
	for (j = -r; j <= r; j++)
		DrawMono(xx + j, yy + i);
*/
}

//
//	x, y in mono space
//
void CLayers::MonoLine(UINT nx, UINT ny)
{
//	DWORD r = m_radius;
	int dx = abs((int)nx - (int)m_prevx);
	int dy = abs((int)ny - (int)m_prevy);
	int delx;
	int dely;
	int x = m_prevx;
	int y = m_prevy;
	if (nx > m_prevx)
		delx = 1;
	else
		delx = -1;
	if (ny > m_prevy)
		dely = 1;
	else
		dely = -1;
	int qq = 0;
	if (dx > dy)
		{
		for(;;)
			{
			qq += dy;
			x += delx;
			if (qq >= dx)
				{
				y += dely;
				qq -= dx;
				MonoMask(delx, dely,x,y);
				}
			else
				MonoMask(delx, 0,x,y);
			if (x == (int)nx)
				break;
			}
		}
	else
		{
		for(;;)
			{
			qq += dx;
			y += dely;
			if (qq >= dy)
				{
				x += delx;
				qq -= dy;
				MonoMask(delx, dely,x,y);
				}
			else
				MonoMask(0, dely,x,y);
			if (y == (int)ny)
				break;
			}
		}
//	EchoIt();
	m_prevx = nx;
	m_prevy = ny;
//	m_bFirstDot = TRUE;
}
#ifdef FASTMONO

UINT CLayers::MakeMono()
{
	ASSERT(m_nMono == 5);
	BYTE * pSrc;
	UINT x, y;
	UINT pp = m_nMono * m_width;
	pSrc = m_pLayers[m_nInk].pData;
	BYTE * pMono = m_pMono;
	for (y = 0; y < m_height;y++)
		{
		for (x = 0; x < m_width;x++)
			{
			BYTE v = pSrc[m_pitch * y + x] / 10;
			BYTE * tp = mdt+25*v;
			UINT j;
			for (j = 0; j < m_nMono; j++,pMono+=pp-m_nMono)
			{
//				for (i = 0; i < m_nMono; i++)
					*pMono++ = *tp++;
			*pMono++ = *tp++;
			*pMono++ = *tp++;
			*pMono++ = *tp++;
			*pMono++ = *tp++;
			}
	pMono += m_nMono - pp * m_nMono;
			}
		pMono += pp * (m_nMono - 1);
		}
	m_bMonoModified = 0;
	return 0;
}
#else
UINT CLayers::MakeMono()
{
	BYTE * pSrc;
	UINT x, y;
	UINT sx, sy;
	UINT tx, ty;
	UINT h = m_nMono * m_height;
	UINT w = m_nMono * m_width;
	pSrc = m_pLayers[m_nInk].pData;
	BYTE * pMono = m_pMono;
	for (y = 0; y < h; y++)
		{
		sy = y / m_nMono;
		ty = y % m_nMono;
		for (x = 0; x < w; x++)
			{
			sx = x / m_nMono;
			tx = x % m_nMono;
			BYTE v = pSrc[m_pitch * sy + sx] / 10;
			BYTE q = dt[5 * ty + tx];
			if (v > q)
				v = 255;
			else
				v = 0;
			*pMono++ = v;
			}
		}
	m_bMonoModified = 0;
	return 0;
}
#endif
void CLayers::MonoDots(UINT x, UINT y, UINT v)
{
	UINT mp = m_width * m_nMono;
	BYTE * pMono = m_pMono;
	UINT i, j, t;
	v = (v + 9) / 10;
	pMono = m_pMono + mp * (y * m_nMono) + x * m_nMono;
	for (i = 0, t = 0; i < m_nMono; i++)
		{
		for (j = 0; j < m_nMono; j++, t++)
			if ((int)v > dt[t])
				pMono[j] = 255;
			else
				pMono[j] = 0;
		pMono += mp;
		}
}

void CLayers::MakeDeltaMask(UINT newrad)
{
	int r,x,y,z,zz,q,p,s,sq;
	r = newrad;
	p = 1 + 2 * r;
	z = r * r;
	zz = m_radius * m_radius;
	BYTE * pBase = m_pMask + 4 * MAX_MASK;
	for (q = 0, y = -r; y <= r;y++)
		{
		s = y * y;
		for ( x = -r; x <= r;x++,q++)
			{
			sq = x * x + s;
			if ((sq > zz) && (sq <= z))
				pBase[q] = 1;
			else
				pBase[q] = 0;
			}
		}
}

void CLayers::MakeMonoMask()
{
	int r,x,y,z,q,p,m,s;
	r = m_radius;
	p = 1 + 2 * r;
	z = r * r;
	BYTE * pBase = m_pMask + 4 * MAX_MASK;
	BYTE * pMask;
	for (q = 0, y = -r; y <= r;y++)
		{
		s = y * y;
		for ( x = -r; x <= r;x++,q++)
			{
			if (x * x + s <= z)
				pBase[q] = 1;
			else
				pBase[q] = 0;
			}
		}
	int dx, dy;
	for (dy = -1, m = 0; dy < 2; dy++) 
	for (dx = -1; dx < 2; dx++,m++)
		{
		if (m == 4)
			continue;
		pMask = m_pMask + m * MAX_MASK;
		for (q = 0, y = -r; y <= r;y++)
			{
			for ( x = - r; x <= r;x++,q++)
				{
				pMask[q] = 0;
				if (pBase[q])
					{
					int xx = r + x + dx;
					int yy = r + y + dy;
					if (xx < 0)
						pMask[q] = 1;
					else if (xx >= p)
						pMask[q] = 1;
					else if (yy < 0)
						pMask[q] = 1;
					else if (yy >= p)
						pMask[q] = 1;
					else if (!pBase[yy*p+xx])
						pMask[q] = 1;
					}
				}
			}
		}
}

void CLayers::MonoToInk()
{
	int x,y,yy;
	BYTE * pSrc;
	pSrc = m_pLayers[m_nInk].pData;
	for (yy = 0; yy < (int)m_height;yy++)
		{
		UINT mp = m_nMono * m_width;
		y = m_height - 1 - yy;
		for (x = 0; x < (int)m_width; x++)
			{
			UINT sum = 0;
			UINT i,j;
			BYTE * pMono = m_pMono + y * m_nMono * mp + x * m_nMono;
			for (i = 0; i < m_nMono; i++)
				{
				for (j = 0; j < m_nMono; j++)
					sum += pMono[j];
				pMono += mp;
				}
			sum	= sum / 25;//51 * sum / 5;
			pSrc[m_pitch*y+x] = sum;
			}
		}
}

void CLayers::UpdateMono()
{
	int x,y,yy;
	BYTE * pDst = m_pLayers[m_nInk].pData;
	UINT mp = m_nMono * m_width;
	for (yy = m_miny; yy <= m_maxy;yy++)
		{
		y = m_height - 1 - yy;
		for (x = m_minx; x <= m_maxx; x++)
			{
			UINT sum = 0;
			UINT i,j;
			BYTE * pMono = m_pMono + y * m_nMono * mp + x * m_nMono;
			for (i = 0; i < m_nMono; i++)
				{
				for (j = 0; j < m_nMono; j++)
					sum += pMono[j];
				pMono += mp;
				}
			sum	= sum / 25;//51 * sum / 5;
			pDst[m_pitch*y+x] = sum;
			}
		}
}

#endif
