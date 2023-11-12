#include "stdafx.h"
#include "deskew.h"
#include "math.h"

#define BLACK 25

BYTE * MakeGray(SKEWINFO & info, LPBITMAPINFOHEADER  lpbi)
{
	UINT w,h;
	info.iw = w = lpbi->biWidth;
	info.ih = h = lpbi->biHeight;
	info.xdpi = (2000 + 100 * lpbi->biXPelsPerMeter) / 3939;
	info.ydpi = (2000 + 100 * lpbi->biYPelsPerMeter) / 3939;
	info.pTemp = new BYTE[w * h + 32];
	UINT mt1[256];
	UINT mt2[256];
	UINT x,y,p,idd;
	idd = lpbi->biBitCount;
	p = 4 * ((w * idd + 31) / 32);
	BYTE * dst = info.pTemp;
	BYTE * src = (BYTE *)lpbi;
	src += 40;
	BYTE * pt = src; 
	if (idd == 1)
		{
		int i;
		BYTE blk = src[0];
		BYTE wht = src[4];
		for (i = 0; i < 256; i++)
			{
			UINT v;
			v = 0;
			UINT m;
			m = 16;
			for (v = 0; m < 256; m += m)
				{
				v <<= 8;
				if (i & m)
					v += wht;
				else
					v += blk;
				}
			mt1[i] = v;
			m = 1;
			for (v = 0; m < 16; m += m)
				{
				v <<= 8;
				if (i & m)
					v += wht;
				else
					v += blk;
				}
			mt2[i] = v;
			}
		}
	if (idd < 9)
		src += 4 << idd;
	for (y = 0;y < h; y++)
		{
		if (idd == 8)
			{
			for (x = 0; x < w; x++)
				dst[x] = pt[4*(UINT)src[x]];
			}
		
		else if (idd == 1)
			{
			UINT * pp = (UINT *) dst;
			UINT z, q, t;
			q = (w + 7) / 8;
			for (z = 0; z < q;)
				{
				t = src[z++];
				*pp++ = mt1[t];
				*pp++ = mt2[t];
				}
			}
		else if (idd == 24)
			{
			for (x = 0; x < w; x++)
				{
				UINT r = src[3*x+0];
				UINT g = src[3*x+1];
				UINT b = src[3*x+2];
//				dst[x] = (30 * r + 59 * g + 11 * b) / 100;
				if (r > g)
					dst[x] = r > b ? r : b;
				else
					dst[x] = g > b ? g : b;
				}
			}
		src += p;	
		dst += w;
		}
	return 0;
}

int * tx;
int * ty;
int * tz;
UINT ct; // table count
void MakeRTable(int r)
{
	int x, y, z;
	int s = r + r + 1;
	ct = 0;
	tx = new int[s * s];
	ty = new int[s * s];
	tz = new int[1 + s * s];
	for (x = -r ; x <= r; x++)
	for (y = -r ; y <= r; y++)
		{
		UINT i, j;
		z = x * x + y * y;
		for (i = 0; (i < ct) && (tz[i] <= z); i++);
		if (i < ct)	
			{
			for (j = ct; j > i; j--)
				{
				tz[j] = tz[j - 1];
				tx[j] = tx[j - 1];
				ty[j] = ty[j - 1];
				}
			 }
		tz[i] = z;
		tx[i] = x;
		ty[i] = y;
		ct++;
		}
	tz[ct] = 100 + 2 * r * r;
}	

//UINT cx, cy;, brestr;
int rerr, bestr, desiredr;
UINT bestx, besty;


UINT mindots,maxdots;
UINT bestsum;


void Check(BYTE * pdib, UINT p,UINT x,UINT y)
{
//DPF("check,x:%d,y:%d",x,h-1-y);
	UINT i;
	for (i = 0; i < ct; i++)
		{
		UINT xx = x + tx[i];
		UINT yy = y + ty[i];
		if (pdib[p*yy+xx] > BLACK)
			break;
		}
	if (i < ct)
		{
		int r = (int)sqrt(.99 + (double)tz[i]);
		int err = (r - desiredr);
		err = err * err;
		if (err < rerr)
			{
			rerr = err;
			bestr = r;
			bestx = x;
			besty = y;
			}
		}
}

void CheckLine(BYTE * pdib, UINT p, UINT x1, UINT x2,UINT y, UINT * pColumn, UINT r)
{
	UINT x;
	UINT sum = 0;
	for (x = x1;x < x1 + r + r;x++)
		sum += pColumn[x-x1];
	x = x1 + r;
	for (;(x + r) <= x2;x++)
		{
		sum += pColumn[x+r-x1];
		if (sum > bestsum)
			bestsum = sum;
		if ((sum > mindots) && (sum < maxdots))
			Check(pdib,p,x,y);
		sum -= pColumn[x-r-x1];
		}
}



void Search(SKEWINFO & info, UINT x1, UINT y1, UINT x2, UINT y2)
{
//	rerr = 10000;
	UINT r = info.xdpi / 6;
	UINT ww = x2 + 1 - x1;
	UINT * pColumn = new UINT[ww];
	UINT x,y,p;
	p = info.iw;
	BYTE * pdib = info.pTemp;
	bestsum = 0;
	if ((int)x1 < 0) x1 = 0;
	if ((int)y1 < 0) y1 = 0;
	if (x2 >= info.iw) x2 = info.iw - 1;
	if (y2 >= info.ih) y2 = info.ih - 1;
	for (x = 0; x < ww; pColumn[x++] = 0);
	for (y = y1;y < y1 + r + r;y++)
		for (x = 0; x < ww; x++)
			pColumn[x] += pdib[y*p+x1+x] > BLACK ? 0 : 1;
	y = y1 + r;
	for (;(y + r) < y2;y++)
		{
		for (x = 0; x < ww; x++)
			pColumn[x] += pdib[p*(y+r)+x1+x] > BLACK ? 0 : 1;
		CheckLine(pdib,p,x1,x2,y,pColumn,r);
		for (x = 0; x < ww; x++)
			pColumn[x] -= pdib[p*(y-r)+x1+x] > BLACK ? 0 : 1;
		}
DPF("bestsum:%d",bestsum);
DPF("bestr:%d,bestx:%d,besty:%d",bestr,bestx,info.ih - 1 - besty);
	delete [] pColumn;
}


BOOL  FindCircle(SKEWINFO & info)
{
	UINT iw = info.iw;
	UINT ih = info.ih;
//	dots == (22 * xdpi * xdpi) / 448; // dots in circle
	mindots = (20 * info.xdpi * info.xdpi) / 448;
	maxdots = (24 * info.xdpi * info.xdpi) / 448;
	mindots = (18 * info.xdpi * info.xdpi) / 448;
	maxdots = (26 * info.xdpi * info.xdpi) / 448;
	MakeRTable(info.xdpi / 5);
	bestr = 0;
	desiredr = info.xdpi / 8;
	rerr = 10000;
	if (ih > iw)
		{
		Search(info, 0, ih / 2 - 4*info.ydpi , info.xdpi, ih / 2  + 4*info.ydpi);
		if (rerr > 100)
			Search(info,iw - info.xdpi, ih / 2 - 4*info.ydpi , iw - 1,
													ih / 2  + 4*info.ydpi);
		}
	else
		{
		Search(info, iw / 2 - 4*info.xdpi, 0, iw / 2 + 4*info.xdpi, 2 * info.ydpi);
		if (rerr > 100)
			Search(info, iw / 2 - 4*info.xdpi, ih - 4*info.ydpi,
											iw / 2 + 6*info.xdpi, ih - 1);
		}
	delete [] tx;
	delete [] ty;
	delete [] tz;
	info.cx = bestx;
	info.cy = besty;
	return rerr < 100 ? 0 : 1;
}

BOOL CheckRound(SKEWINFO & info)
{
	UINT iw = info.iw;
	UINT ih = info.ih;
	int t1[2];
	int t2[2];
//	dots == (22 * xdpi * xdpi) / 448; // dots in circle
	mindots = (20 * info.xdpi * info.xdpi) / 448;
	maxdots = (24 * info.xdpi * info.xdpi) / 448;
	mindots = (18 * info.xdpi * info.xdpi) / 448;
	maxdots = (26 * info.xdpi * info.xdpi) / 448;
	MakeRTable(info.xdpi / 5);
	bestr = 0;
	desiredr = info.xdpi / 8;
	rerr = 10000;
	int ddpi;// den dpi
	if (ih > iw)
		{
		ddpi = info.ydpi;
		Search(info,info.cx - info.xdpi / 2, info.cy - 19 * info.ydpi / 4 ,
					info.cx + info.xdpi / 2, info.cy - 15 * info.ydpi / 4);
		if (rerr < 100)
			{
			t1[0] = bestx;
			t2[0] = besty;
		rerr = 10000;
		Search(info,info.cx - info.xdpi / 2, info.cy + 15 * info.ydpi / 4 ,
					info.cx + info.xdpi / 2, info.cy + 19 * info.ydpi / 4);
			t1[1] = bestx;
			t2[1] = besty;
			}
		}
	else
		{
		ddpi = info.xdpi;
		Search(info,info.cx - 19 * info.xdpi / 4, info.cy - info.ydpi / 2,
					info.cx	- 15 * info.xdpi / 4, info.cy + info.ydpi / 2);
		if (rerr < 100)
			{
			t1[0] = besty;
			t2[0] = bestx;
		rerr = 10000;
		Search(info,info.cx + 15 * info.xdpi / 4 ,info.cy - info.ydpi / 2,
					info.cx + 19 * info.xdpi / 4, info.cy + info.ydpi / 2);
			t1[1] = besty;
			t2[1] = bestx;
			}
		}
	delete [] tx;
	delete [] ty;
	delete [] tz;
	if (rerr >= 100)
		{
		return 1;
		}
	info.num = t1[0] - t1[1];
	info.den = t2[1] - t2[0];
	if (abs(abs(info.den) - 17*ddpi/2)> 10)
		{
DPF("bad den:%d",info.den);
		return 1;
		}
	if (abs(info.num) > (ddpi/4))
		{
DPF("bad num:%d",info.num);
		return 1;
		}
	DPF("good, cx:%d,cy:%d,num:%d,den:%d",info.cx,info.cy,info.num,info.den);
	return 0;
}

BOOL  FindSkew(SKEWINFO & info)
{
	if (!CheckRound(info))
		return 0;

	int t1[2];
	int t2[2];
	int cc;
	UINT h = info.ih;
	UINT w = info.iw;
	BYTE * pdib = info.pTemp;
	UINT p = info.iw;
	BOOL bFlip = 0;
	if (h > w)
		{
		cc = info.cx;
		if (cc > (int)(w / 2)) bFlip = TRUE;
		info.den = 8 * info.ydpi;
		int x,y,i,j,z;
		for (j = 0; j < 2; j++)
			{
			if (j)
				y = info.cy + 4 * info.ydpi;
			else
				y = info.cy - 4 * info.ydpi;
			z = info.xdpi / 4;
			for (i = 0; i < 1 + z + z;i++)
				{
				if (i & 1)
					x = info.cx - 1 - (i / 2);
				else
					x = info.cx + (i / 2);
				if (pdib[p*y+x] < BLACK)
					break;
				}
			int x1, x2;
			for (x2 = x; (x2 < (x + z + z)) && (pdib[p*y+x2]<BLACK)	;x2++	);
			for (x1 = x; x1 && (pdib[y*p+x1] < BLACK)	;x1--);
			t1[j] = x1;
			t2[j] = x2;
			}
		}
	else
		{
		cc = info.cy;
		if (cc > (int)(h / 2)) bFlip = TRUE;
		info.den = 8 * info.xdpi;
		int x,y,i,j,z;
		for (j = 0; j < 2; j++)
			{
			if (j)
				x = info.cx + 4 * info.xdpi;
			else
				x = info.cx - 4 * info.xdpi;
			z = info.ydpi / 4;
			for (i = 0; i < 1 + z + z;i++)
				{
				if (i & 1)
					y = info.cy - 1 - (i / 2);
				else
					y = info.cy + (i / 2);
				if (pdib[p*y+x] < BLACK)
					break;
				}
			int y1, y2;
			for (y2 = y; (y2 < (y + z + z)) && (pdib[p*y2+x] < BLACK)	;y2++	);
			for (y1 = y; y1 && (pdib[p*y1+x] < BLACK)	;y1--);
			t1[1-j] = y1;
			t2[1-j] = y2;
			}
		}
	DPF("t10:%d,t20:%d,t11:%d,t21:%d",t1[0],t2[0],t1[1],t2[1]);
	int d1 = t2[0] + 1 - t1[0];
	int d2 = t2[1] + 1 - t1[1];
	int z1 = (t2[0] + 1 + t1[0]) / 2 - cc;
	int z2 = (t2[1] + 1 + t1[1]) / 2 - cc;
	DPF("d1:%d,d2:%d,z1:%d,z2:%d",d1,d2,z1,z2);
	int mind = info.xdpi / 10;
	int maxd = info.xdpi / 6;
	int maxz = 7;
	if (d1 < mind || d1 > maxd || d2 < mind || d2 > maxd || abs(z1+z2) > maxz)
		{
		DPF("bad");
		return 1;
		}
	else
		{
		info.num = (1 + t2[0] + t1[0] - t2[1] - t1[1]) / 2;
//		if (bFlip)
//			info.num = 0 - info.num;
		DPF("good, cx:%d,cy:%d,num:%d,den:%d",info.cx,info.cy,info.num,info.den);
		return 0;
		}
}

void RotateIt(SKEWINFO & info, BYTE * pDst, BYTE * pSrc)
{
	BYTE * pBits = pDst + 40;
	BOOL bColor, bFlip;
	bFlip = TRUE;
	bColor = info.bColor;
	UINT bpp = bColor ? 3 : 1;
	UINT ocx,ocy;
	UINT oh, ow,oy;
	ocy = info.ocy;
	ow = info.ow;
	oh = info.oh;
	ocx = ow / 2;
	UINT op = 4 * (( bpp * ow + 3) / 4);
	double angle = atan2((double)info.num, (double)info.den);
	double	ssin = sin(angle);
	double	ccos = cos(angle);
	UINT ip = info.iw;
	if (bColor)
		ip = 4 * ((bpp * ip + 3) / 4);
	BYTE * pTemp;
	pTemp = pSrc;
	for (oy = 0; oy < oh; oy++)
		{
		int dy = oy - ocy;
		double yy;
		double ycos;
		double ysin;
		yy = (double)dy;
		ycos = yy * ccos;
		ysin = yy * ssin;
		if (info.iw > info.ih)
			{
			int dx = 0 - ocx;
			double xx = (double)dx;
			double tx,ty;
			if (bFlip)
				{
				ty = info.cx + (xx * ccos - ysin);
				tx = info.cy + (xx * ssin + ycos);
				}
			else
				{
				tx = info.cy - (xx * ccos - ysin);
				ty = info.cx + xx * ssin + ycos;
				}
			UINT ox;
			for (ox = 0; ox < ow; ox++)
				{
				int sxx = (int)(256 * ty);
				int sx = sxx / 256;
				UINT xf2 = sxx % 256;
				UINT xf1 = 255 - xf2;
				int syy = (int)(256 * tx);
				int sy = syy / 256;
				UINT yf2 = syy % 256;
				UINT yf1 = 255 - yf2;
				if (bFlip)
					{
					ty += ccos;
					tx += ssin;
					}
				else
					{
					ty -= ccos;
					tx -= ssin;
					}
				UINT v;
				UINT xx;
				for (xx = 0; xx < bpp;xx++)
					{
					if (((UINT)sx >= info.iw) || ((UINT)sy >= info.ih))
						v = 255 * xf1 * yf1;
					else
						v = (UINT)pTemp[sy*ip+bpp*sx+xx] * xf1 * yf1;
					if (((UINT)sx >= info.iw) || ((UINT)(sy+1) >= info.ih))
						v += 255 * xf1 * yf2;
					else
						v += (UINT)pTemp[sy*ip+ip+bpp*sx+xx] * xf1 * yf2;
					if (((UINT)(sx+1) >= info.iw) || ((UINT)sy >= info.ih))
						v += 255 * xf2 * yf1;
					else
						v += (UINT)pTemp[sy*ip+bpp*sx+bpp+xx] * xf2 * yf1;
					if (((UINT)(sx+1) >= info.iw) || ((UINT)(sy+1) >= info.ih))
						v += 255 * xf2 * yf2;
					else
						v += (UINT)pTemp[sy*ip+ip+bpp*sx+bpp+xx] * xf2 * yf2;
					pBits[op*oy+bpp*ox+xx] = v / 255/255;
					}
				}
			}
		else
			{
			if (!bFlip)
				{
				int dx = 0 - ocx;
				double xx = (double)dx;
				double tx = info.cy - (xx * ccos - ysin);
				double ty = info.cx + xx * ssin + ycos;
				UINT ox;
				for (ox = 0; ox < ow; ox++)
					{
					int sx = (int)ty;
					int sy = (int)tx;
					tx -= ccos;
					ty += ssin;
					BYTE v;
					if (bColor)
						{
						int xx;
						for (xx = 0; xx < 3; xx++)
							{
							if (((UINT)sx >= info.iw) || ((UINT)sy >= info.ih))
								v = 255;
							else
								v = pTemp[sy*ip+bpp*sx+xx];
							pBits[op*oy+bpp*ox+xx] = v;
							}
						}
					else
						{
						if (((UINT)sx >= info.iw) || ((UINT)sy >= info.ih))
							v = 255;
						else
							v = pTemp[sy*ip+bpp*sx];
						pBits[op*oy+bpp*ox] = v;
						}
					}
				}
			else
				{
					UINT ox;
				int dx = 0 - ocx;
				double xx = (double)dx;
				double tx = info.cy + (xx * ccos - ysin);
				double ty = info.cx - (xx * ssin + ycos);
				for (ox = 0; ox < ow; ox++)
					{
					int sx = (int)ty;
					int sy = (int)tx;
					tx += ccos;
					ty -= ssin;
					BYTE v;
					if (((UINT)sx >= info.iw) || ((UINT)sy >= info.ih))
						v = 255;
					else
						v = pTemp[sy*ip+bpp*sx];
					pBits[op*oy+bpp*ox] = v;
					}
				}
			}

	}
 }
