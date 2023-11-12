#include "math.h"
#include "CLayers.h"
#include "CLevtbl.h"

/*
	for variable blur stuff
	where bz is value of blur at boundary, normal is 50%
	
	z = bz * qq / 100
	h = qq / 2
	for (i = 0; i <= h; i++
		t[i] = i * z / h;
	for (; i<= qq; i++)
		t[i] = z + ((i - h) * (qq - z)) / (qq - h);
*/

#define TQ(a) (a < 7 ? a - 1 : a - 2)
void BlurLine(BYTE * pDst, UINT * pColumn, UINT w, UINT r, UINT pp, BYTE * p);
void BlurLineX(BYTE * pDst, UINT * pColumn, UINT w, UINT r, UINT z, BYTE * p);
void Blur(BYTE * pDst, BYTE * pSrc, UINT w, UINT h, UINT r, UINT f, UINT pitch);
void CLayers::BlurIt(UINT layer)
{
	BYTE * pAlpha = m_pLayers[layer].pData;
	BYTE * pDsp = pAlpha + 2 * m_pitch * m_height;
	BYTE * pIndex = pDsp + m_pitch * m_height;
	UINT j = TQ(layer);
	int r = m_pTable->table[j].blur;
	int c = m_pTable->table[j].color;
	int f = r / 256;
	r &= 255;
	memmove(pDsp, pAlpha, 2 * m_pitch*m_height);
	if (!r)
		{
//		memmove(pDsp, pAlpha, 2 * m_pitch*m_height);
		return;
		}
/*
	for (j = 0; j < m_pitch * m_height; j++)
		if (!pDsp[j])
			pIndex[j] = c;
		else
			pIndex[j] = pAlpha[m_pitch * m_height + j];

*/
//	memset(pIndex, 4 , m_pitch*m_height);
/*
	if (m_minx > r)
		m_minx -= r;
	else
		m_minx = 0;
	if (m_miny > r)
		m_miny -= r;
	else
		m_miny = 0;
	if ((m_maxx + r) >= m_width)
		m_maxx = m_width - 1;
	else
		m_maxx += r;
	if ((m_maxy + r) >= m_height)
		m_maxy = m_height - 1;
	else
		m_maxy += r;
*/
	Blur(pDsp, pAlpha, m_width, m_height, r, f, m_pitch);
	return;	
	UINT x, ay,y;
//	for (ay = m_miny; ay <= m_maxy; ay++)
	for (ay = 0; ay < m_height; ay++)
	{
	y = m_height -1 - ay;
//	for (x = m_minx; x <= m_maxx; x++)
	for (x = 0; x < m_width; x++)
		{
		if (pDsp[y*m_pitch+x] && !pIndex[y*m_pitch+x])
			{
			pIndex[y*m_pitch+x] = 2;
//			pAlpha[y*m_pitch+x] = 255;
			continue;
			int dx, dy;
			for (dy = -r; dy <= r; dy++)
			for (dx = -r; dx <= r; dx++)
				{
				UINT xx = x + dx;
				UINT yy = y + dy;
				if ((xx < m_width) && (yy < m_height) &&
							(pIndex[yy*m_pitch+xx]))
					{
					pIndex[y*m_pitch+x] = pIndex[yy*m_pitch+xx];
					dy = r+1;
					break;
					}
				}
			}
		}
	}
}

UINT zb(UINT a, UINT b)
{
	double g = 1.5;
/*
	double aa = a;
	double qq = aa / (double)b;
	double z;
	if (qq > 0.5)
		z = -4.0 * qq * qq * qq + 8.0 * qq * qq - 4.0 * qq + 1;
	else
		z = -4.0 * qq * qq * qq + 4.0 * qq * qq;
//	z = -1.0 * qq * qq * qq + 1.5 * qq * qq + 0.5 * qq;
//	z = exp(log(z) * g);
	z = qq;
	z *= 255;
	return (UINT) z;
*/
	return 255 * a / b;
}

void Blur(BYTE * pDst, BYTE * pSrc, UINT w, UINT h, UINT r, UINT f, UINT pitch)
{
	UINT q = r + r + 1;
	UINT qq = q * q;
	UINT pp = pitch * h;
	UINT * pColumn = new UINT[w+w+r+r+1];
	BYTE * pBlur = new BYTE[qq+1];
	UINT zz,x,y,hh;
	int x1, x2, x3;
	zz = (qq * f) / 2;
	hh = qq /2;
#ifdef BMAGIC
	for (x = 0; x <= hh; x++)
		pBlur[x] = zb(x * zz, hh * qq);
	for (; x <= qq; x++)
		pBlur[x] = zb(5 * zz + (255 * (x - hh) * (qq - zz)) / (qq - hh)) / qq;
		pBlur[x] = (255 * zz + (255 * (x - hh) * (qq - zz)) / (qq - hh)) / qq;
#else
	if (f == 0)
		{
		for (x = 0; x < hh; x++)
			pBlur[x] = 0;
		for (; x <= qq; x++)
			pBlur[x] = zb(x - hh,qq - hh);
		}
	else if (f > 1)
		{
		for (x = 0; x <= hh; x++)
			pBlur[x] = zb(x,hh);
		for (; x <= qq; x++)
			pBlur[x] = 255;
		}
	else
		for (x = 0; x <= qq; x++)
			pBlur[x] = zb(x,qq);
#endif
	for (x = 0; x < w; x++)
		{
		if (pSrc[x])
			{
			pColumn[x] = r + 1;
			pColumn[w+x+r] = pSrc[pp+x];
			}
		else
			{
			pColumn[x] = 0;
			pColumn[w+x+r] = 0;
			}
		}
	for (y = 1;y < r;y++)
		for (x = 0; x < w; x++)
			if (pSrc[y*pitch+x])
				{
				pColumn[x]++;
				x1 = x - r;
				x2 = x + r;
				for (x3 = x1; x3 <= x2; x3++)
				pColumn[w+x3+r] =pSrc[y * pitch + pp + x];
//				pColumn[w+x] = pSrc[y * pitch + pp + x];
				}
	for (y = 0;y < r;y++)
		{
		for (x = 0; x < w; x++)
			if (pSrc[(y+r)*pitch+x])
				{
				pColumn[x]++;
				x1 = x - r;
				x2 = x + r;
				for (x3 = x1; x3 <= x2; x3++)
				pColumn[w+x3+r] =pSrc[(y+r) * pitch + pp + x];
//				pColumn[w+x] = pSrc[(y+r) * pitch + pp + x];
				}
		BlurLine(pDst, pColumn, w, r, pp, pBlur);
		pDst += pitch;
		for (x = 0; x < w; x++)
			pColumn[x] -= pSrc[x] ? 1 : 0;
		}
	for (;(y + r) < h;y++)
		{
		for (x = 0; x < w; x++)
			if (pSrc[(y+r)*pitch+x])
				{
				pColumn[x]++;
				x1 = x - r;
				x2 = x + r;
				for (x3 = x1; x3 <= x2; x3++)
				pColumn[w+x3+r] = pSrc[(y+r) * pitch + pp + x];
//				pColumn[w+x] = pSrc[(y+r) * pitch + pp + x];
				}
		BlurLine(pDst, pColumn, w, r, pp, pBlur);
		pDst += pitch;
		for (x = 0; x < w; x++)
			pColumn[x] -= pSrc[(y-r)*pitch+x] ? 1 : 0;
		}
	for (;y < h;y++)
		{
		for (x = 0; x < w; x++)
			if (pSrc[(h-1)*pitch+x])
				{
				pColumn[x]++;
				x1 = x - r;
				x2 = x + r;
				for (x3 = x1; x3 <= x2; x3++)
				pColumn[w+x3+r] =pSrc[(h-1) * pitch + pp + x];
//				pColumn[w+x] = pSrc[(h-1) * pitch + pp + x];
				}
		BlurLine(pDst, pColumn, w,r, pp, pBlur);
		pDst += pitch;
		for (x = 0; x < w; x++)
			pColumn[x] -= pSrc[(y-r)*pitch+x] ? 1 : 0;
		}
	delete [] pColumn;
	delete [] pBlur;
}

//#define PUTOUT *pDst = pBlur[sum];pDst[pp] = pColumn[x+w];pDst++
#define PUTOUT pDst[pp] = pColumn[x+w+r]; *pDst++ = pBlur[sum]
void BlurLine(BYTE * pDst, UINT * pColumn, UINT w, UINT r, UINT pp, BYTE *pBlur)
{
	UINT q = r + r + 1;
	UINT qq = q * q;
	UINT x, sum;
	sum = (r + 1) * pColumn[0];
	for (x = 1;x < r;x++)
		sum += pColumn[x];
	for (x = 0;x < r;x++)
		{
		sum += pColumn[x+r];
		PUTOUT;
		sum -= pColumn[0];
		}
	for (;(x + r) < w;x++)
		{
		sum += pColumn[x+r];
		PUTOUT;
		sum -= pColumn[x-r];
		}
	for (;x < w;x++)
		{
		sum += pColumn[w-1];
		PUTOUT;
		sum -= pColumn[x-r];
		}
}

void BlurLineX(BYTE * pDst, UINT * pColumn, UINT w, UINT r, UINT z, BYTE * p);
void BlurXX(BYTE * pDst, BYTE * pSrc, UINT w, UINT h, 
		UINT r, UINT f, UINT z, UINT pitch);

void BlurX(BYTE * pDst, BYTE * pSrc, UINT w, UINT h, 
		UINT r, UINT f, UINT z, UINT pitch)
{
	UINT i;
	for (i = 0; i < z; i++)
		BlurXX(pDst++, pSrc++,w,h,r,f,z,pitch);
	return;
	BlurXX(pDst++, pSrc++,w,h,r,f,z,pitch);
	BlurXX(pDst++, pSrc++,w,h,r,f,z,pitch);
	BlurXX(pDst++, pSrc++,w,h,r,f,z,pitch);
	if (z == 4)
		BlurXX(pDst, pSrc,w,h,r,f,z,pitch);
}
void BlurXX(BYTE * pDst, BYTE * pSrc, UINT w, UINT h, UINT r,
			UINT f, UINT bpp, UINT pitch)
{
	UINT q = r + r + 1;
	UINT qq = q * q;
	UINT pp = pitch * h;
	UINT * pColumn = new UINT[w];
	BYTE * pBlur = new BYTE[qq+1];
	UINT zz,x,y,hh;
//	UINT x1, x2, x3;
	zz = (qq * f) / 2;
	hh = qq /2;
#ifdef BMAGIC
	for (x = 0; x <= hh; x++)
		pBlur[x] = zb(x * zz, hh * qq);
	for (; x <= qq; x++)
		pBlur[x] = zb(5 * zz + (255 * (x - hh) * (qq - zz)) / (qq - hh)) / qq;
		pBlur[x] = (255 * zz + (255 * (x - hh) * (qq - zz)) / (qq - hh)) / qq;
#else
	if (f == 0)
		{
		for (x = 0; x < hh; x++)
			pBlur[x] = 0;
		for (; x <= qq; x++)
			pBlur[x] = zb(x - hh,qq - hh);
		}
	else if (f > 1)
		{
		for (x = 0; x <= hh; x++)
			pBlur[x] = zb(x,hh);
		for (; x <= qq; x++)
			pBlur[x] = 255;
		}
	else
		for (x = 0; x <= qq; x++)
			pBlur[x] = zb(x,qq);
#endif
	for (x = 0; x < w; x++)
		pColumn[x] = pSrc[bpp*x] * (r + 1);
	for (y = 1;y < r;y++)
		for (x = 0; x < w; x++)
			pColumn[x] += pSrc[y*pitch+x*bpp];
	for (y = 0;y < r;y++)
		{
		for (x = 0; x < w; x++)
			pColumn[x] += pSrc[(y+r)*pitch+x*bpp];
		BlurLineX(pDst, pColumn, w, r, bpp,pBlur);
		pDst += pitch;
		for (x = 0; x < w; x++)
			pColumn[x] -= pSrc[x*bpp];
		}
	for (;(y + r) < h;y++)
		{
		for (x = 0; x < w; x++)
			pColumn[x] += pSrc[(y+r)*pitch+x*bpp];
		BlurLineX(pDst, pColumn, w, r, bpp, pBlur);
		pDst += pitch;
		for (x = 0; x < w; x++)
			pColumn[x] -= pSrc[(y-r)*pitch+x*bpp];
		}
	for (;y < h;y++)
		{
		for (x = 0; x < w; x++)
			pColumn[x] += pSrc[(h-1)*pitch+x*bpp];
		BlurLineX(pDst, pColumn, w,r, bpp,pBlur);
		pDst += pitch;
		for (x = 0; x < w; x++)
			pColumn[x] -= pSrc[(y-r)*pitch+x*bpp];
		}
	delete [] pColumn;
	delete [] pBlur;
}

//#define PUTOUT *pDst = pBlur[sum];pDst[pp] = pColumn[x+w];pDst++
#define PUTOUTX  *pDst = sum / qq; pDst += z;//pBlur[sum]; pDst += z 
void BlurLineX(BYTE * pDst, UINT * pColumn, UINT w, UINT r, UINT z, BYTE *pBlur)
{
	UINT q = r + r + 1;
	UINT qq = q * q;
	UINT x, sum;
	sum = (r + 1) * pColumn[0];
	for (x = 1;x < r;x++)
		sum += pColumn[x];
	for (x = 0;x < r;x++)
		{
		sum += pColumn[x+r];
		PUTOUTX;
		sum -= pColumn[0];
		}
	for (;(x + r) < w;x++)
		{
		sum += pColumn[x+r];
		PUTOUTX;
		sum -= pColumn[x-r];
		}
	for (;x < w;x++)
		{
		sum += pColumn[w-1];
		PUTOUTX;
		sum -= pColumn[x-r];
		}
}
