#include "stdafx.h"
#include "myview.h"
#include "clayers.h"
#include "cbrush.h"
#include "math.h"

#ifdef _DEBUG
//#define LOGGING
#endif

#define MAX_RADIUS 16

#define MAX_SMARTS 5000

CBrusher::CBrusher()
{
	m_texture = 0;
	m_density = 100;
	m_radius = 0;
	m_pMasks = 0;
	m_factor = 7;	// extened spce
	m_brushf = 10;	// brush halo size
	m_width = 640;
	m_height = 480;
	m_ramp = 4;
	m_pbb = 0;
	m_pbz = 0;
	m_bFirst = 0;
	m_bShow = 1;
	m_pc = 0;
	m_pz = 0;
	m_pq = 0;
	m_pSmarts = 0;
	m_pInfo = 0;
	m_nSmarts = 0;
	m_pSmarts = new POINT[MAX_SMARTS];
	m_pInfo = new int[MAX_SMARTS];
}

CBrusher::~CBrusher()
{
	delete [] m_pbz;
	delete [] m_pbb;
	delete [] m_pMasks;
	delete [] m_pc;
	delete [] m_pz;
	delete [] m_pq;
	delete [] m_pSmarts;
	delete [] m_pInfo;
}

void CBrusher::Init(CLayers * pLayers)
{
	m_pLayers = pLayers;
	m_width = m_pLayers->Width();
	m_height = m_pLayers->Height();
	m_bSolid = m_pLayers->Solid();
	MakeBrushes();
	SetupCache(1);
	SetRadius(5);// MakeBrush(89 * m_factor / 20);
}

void CBrusher::ZPoint(int i, int x, int y)
{
//if (i && !m_radius) m_radius++;
ZPoint(i,x,y,m_radius, m_density);
}


void CBrusher::ZPoint(int i, int x, int y, int r, int d)
{
	m_p[i].x = x;
	m_p[i].y = y;
	m_p[i].d = d;
	m_p[i].r = r;
#ifdef LOGGING
	if (m_logc < 100)
	m_log[m_logc++] = m_p[i];
#endif
}

void CBrusher::StartBrush()
{
	m_nSmarts = 0;
	m_count = 0;
	m_bFirst = 1;
	m_pdir = 99;
	m_px = -1;
	SetupCache(1);
#ifdef LOGGING
	m_logc = 0;
#else
	m_logc = 1000;
#endif
}

void CBrusher::AppendBrush(CPoint point)
{
	AppendIt(point.x, point.y);
}

void CBrusher::StopBrush()
{
	DoBack();
#ifdef LOGGING
	if (!m_logc || (m_logc > 100))
		return;
	int i;
	char amsg[280];
	CFile file;
	DWORD mode = CFile::modeWrite;// | CFile::modeCreate;
	mode |= CFile::modeCreate;
	strcpy(amsg,"c:\\brush.txt");
	if (!file.Open(amsg, mode))
		{
		mode |= CFile::modeCreate;
		if (!file.Open(amsg, mode))
			return;
		}
	file.SeekToEnd();
//	sprintf(amsg, "Extx:%4d,Exty:%4d\r\n",m_ExtX, m_ExtY);
//	file.Write(amsg, strlen(amsg));

	for (i = 0; i < m_logc;i++)
		{
		sprintf(amsg, "i:%4d,x:%4d,y:%4d,d:%4d,r:%4d\r\n",
				i,m_log[i].x,m_log[i].y,m_log[i].d,
					m_log[i].r);

		file.Write(amsg, strlen(amsg));
		}
	file.Close();
#endif
}

void CBrusher::AppendIt(int x, int y)
{
	if (m_count && 
				(abs(x - m_prevx) <= (int)m_factor) &&
				(abs(y - m_prevy) <= (int)m_factor))
		{
		m_p[m_count-1].d = m_density;
		m_p[m_count-1].r = m_radius;
		return;
		}

	if (m_pSmarts && (m_nSmarts < MAX_SMARTS))
		{
	//	if (!m_nSmarts || (abs( x - m_pSmarts[m_nSmarts-1].x) <= m_factor)
	//				   || (abs( y - m_pSmarts[m_nSmarts-1].y) <= m_factor))
			{
			m_pInfo[m_nSmarts] = m_radius * 1000 + m_density;
			m_pSmarts[m_nSmarts].x = x;
			m_pSmarts[m_nSmarts++].y = y;
			}
		}
	m_prevx = x;
	m_prevy = y;
	if (!m_count)
		{
		ZPoint(0,x,y);
		m_count++;
		return;
		}
	else if (m_count == 1)
		{
		ZPoint(1,x,y);
		m_count++;
		return;
		}

	else if (m_count == 2)
		{
		ZPoint(2,x,y);
		m_count++;
//		DoFront();		// postpone to avoid glitch
		return;
		}
	ZPoint(3,x,y);
	m_count = 4;
	DoMiddle();
	m_p[0] = m_p[1];
	m_p[1] = m_p[2];
	m_p[2] = m_p[3];
}

void CBrusher::ApplyBrush(int x, int y)//, int r, int d)
{
	if ((x == m_px) && (y == m_py))
		return;
	int dir;
	int xx = x;// / m_factor;
	int yy = y;// / m_factor;
	int px = m_px;// / m_factor;
	int py = m_py;// / m_factor;
	if (m_pdir > 90)	// first dot
		{
		dir = 80;
		}
	else
		{
		if ( xx > px)
			{
			if (yy < py)
				dir = 3;
			else if (yy > py)
				dir = 5;
			else
				dir = 4;
			}
		else if (xx < px)
			{
			if (yy < py)
				dir = 1;
			else if (yy > py)
				dir = 7;
			else
				dir = 0;
			}
		else
			{
			if (yy < py)
				dir = 2;
			else if (yy > py)
				dir = 6;
			else
				dir = 99;	// never
			}
		ASSERT(dir < 8);
		}
	BOOL bClear = 0;
	if (m_pdir > 90)
		{
		bClear = 1;
		}
	else if (m_pdir > 7)
		{
		bClear = 0;
		}
	else if (dir == ((m_pdir + 3) % 8))
		bClear = 1;
	else if (dir == ((m_pdir + 4) % 8))
		bClear = 1;
	else if (dir == ((m_pdir + 5) % 8))
		bClear = 1;
	if (bClear)
{
	m_cache_count = 0;
}
	m_pdir = dir;
	m_px = x;
	m_py = y;
	int w = m_brushw;
	int h = w;
	int j = 0;
	int i = 0;
	int q = h;
	int z = m_offsets[m_radius];
	UpdateCache();
	for (; i < h; i++)
		{
		int ty = i + y - m_brushw / 2;
		if ((UINT)ty >= m_height * m_factor)
			continue;
		if (ty % m_factor)
			continue;
		ty /= m_factor;
		j = 0;
		for (; j < w; j++)
			{
			int tx = j + x - m_brushw / 2;
			if ((UINT)tx >= m_width * m_factor)
				continue;
			if (tx % m_factor)
				continue;
			tx /= m_factor;
			UINT zz = m_pbz[z + q * i + j];
			if (!zz)
				continue;
			UINT qz = CheckCache(tx,ty);
			if (m_bSolid)
				{
				if (m_pbb[z+q*i+j] < 128)
					continue;
				UINT v = (m_density * 255 ) / 100;
				m_pLayers->DrawDot(tx,ty,v);
				}
			else if (zz > m_pz[qz])
				{
/*
	if first time cache ink layer so that subequent writes
	are all relative to the original
	
*/
				if (m_pz[qz])
					m_pLayers->PutDotAlpha(tx,ty,m_pc[qz]);
				else
					m_pc[qz] = m_pLayers->GetDotAlpha(tx,ty);
				m_pz[qz] = zz;
				UINT v = m_pbb[z+q*i+j];
				UINT t;
				if (m_texture == 3)
					t = (rand() % zz) + (rand() % zz); 
				else if (m_texture == 2)
					t = rand() % 510; 
				else if (m_texture == 1)
					t = 255 + rand() % 256; 
				if (m_texture)
					v = (v * t * m_density ) / 51000;
				else
					v = (m_density * v ) / 100;
				m_pLayers->DrawDot(tx,ty,v);
				}
			}
		}
}



void CBrusher::DrawObject(UINT which, CPoint p1, CPoint p2, UINT rad, UINT den)
{
	BOOL bOld = SetupCache(0);
	if (which == 0)
		{
		DrawBegin(p1.x, p1.y, rad, den);
		DrawAgain(p2.x, p2.y);
		}
	else if (which == 1)
		{
		CPoint p = p1;
		DrawBegin(p1.x, p1.y, rad, den);
		DrawAgain(p2.x, p1.y);
		DrawAgain(p2.x, p2.y);
	 	DrawAgain(p1.x, p2.y);
		DrawAgain(p1.x, p1.y);
		}
	else if (which == 2)
		{
		int x1,y1;//,x2,y2;
		int w, h;
		w = p2.x + 1 - p1.x;
		h = p2.y + 1 - p1.y;
		int a = w / 2;
		int b = h / 2;
		int c = 0;
		int x,y;
		x1 = (p1.x + p2.x) / 2;
		y1 = (p1.y + p2.y) / 2;
		DrawBegin(x1+a, y1, rad, den);
		double twopi = 1;
		twopi = atan(twopi);
		twopi *= 8;
		double angle;
	//	double	ssin = sin(angle);
	//	double	ccos = cos(angle);
		int px = -10000;
		int py = -10000;
		int aa;
		x1 *= m_factor;
		y1 *= m_factor;
		a *= m_factor;
		b *= m_factor;
		px = x1 + a;
		py = y1;
		for (aa = 0; aa < 3600; aa++)
			{
			angle = (aa * twopi) / 3600;
			double	ssin = sin(angle);
			double	ccos = cos(angle);
			x = (int)((double)a * ccos);
			y = (int)((double)b * ssin);
			if ((x != px) || (y != py))
				{
				ApplyBrush(x1 + x, y1 + y);
				px = x; py = y;
				}
			}
		}
	SetupCache(bOld);
}



void CBrusher::DrawBegin(UINT x, UINT y, UINT radius, UINT density)
{
	m_d1 = m_d2 = m_density = density;
	SetRadius(radius);
	m_r1 = m_r2 = m_radius;
	m_px = x * (int)m_factor;
	m_py = y * (int)m_factor;
	m_pdir = 99;
}
//
//	x,y in gray space
//
void CBrusher::DrawAgain(UINT ux, UINT uy)
{
	ux *= (int)m_factor;
	uy *= (int)m_factor;
	if (((int)ux == m_px) && ((int)uy == m_py))
		return;
	LineTo(ux,uy);
}

void CBrusher::LineTo(UINT ux, UINT uy)
{
	int ox = m_px;
	int oy = m_py;
	int nx = ux;// * m_factor;
	int ny = uy;// * m_factor;
	int ddx = nx - ox;
	int ddy = ny - oy;
	int dx = abs(ddx);
	int dy = abs(ddy);
	int delx;
	int dely;
	int x,y;
	int dd = m_d2 - m_d1;
	int rr = m_r2 - m_r1;
	if (dx > dy)
		{
		if (nx > ox)
			delx = 1;
		else
			delx = -1;
		x = ox;
		for(;x != nx;)
			{
			x += delx;
			y = oy + ((x - ox) * ddy) / ddx;
			m_density = m_d1 + ((x - ox) * dd) / ddx;
			UINT zr = m_r1 + ((x - ox) * rr) / ddx;
			if (zr != m_radius)
				SetMyRadius(zr);
			ApplyBrush(x,y);
			}
		}
	else
		{
		if (ny > oy)
			dely = 1;
		else
			dely = -1;
		y = oy;
		for(;y != ny;)
			{
			y += dely;
			x = ox + ((y - oy) * ddx) / ddy;
			m_density = m_d1 + ((y - oy) * dd) / ddy;
			UINT zr = m_r1 + ((y - oy) * rr) / ddy;
			if (zr != m_radius)
				SetMyRadius(zr);
			ApplyBrush(x,y);
			}
		}
	m_px = nx;
	m_py = ny;
}


void CBrusher::MakeBrushes()
{
// brush is 4.4.r.1.r.4.4
	UINT rr;
	int ts = 0;
	for (rr = 0; rr < MAX_RADIUS * m_factor; rr++)
		{
		m_brushw = 1 + 2 * (m_brushf + rr);
		ts += m_brushw * m_brushw;
		}
	DPF("brushes size :%d",ts);
	delete [] m_pbz;
	delete [] m_pbb;
	m_pbz = new BYTE[ts];
	m_pbb = new BYTE[ts];
	int z = 0;
	for (rr = 0; rr < MAX_RADIUS * m_factor; rr++)
		{
		m_offsets[rr] = z;
		int q = 1 + 2 * (m_brushf + rr);
		int i, j;
		int mm = rr + m_brushf;
		int qq = rr;
		int zq = rr / 2;
		int r = qq;
		double rq = mm;
		double A = 2.0 / (rq * rq * rq);
		double B = -3.0 / (rq * rq);
		double C = 0.0;
		double D = 1.0;
		for (i = 0; i < q; i++)
		for (j = 0; j < q; j++,z++)
			{
			int dx = i - r - (int)m_brushf;
			int dy = j - r - (int)m_brushf;
			int d = dx * dx + dy * dy;
			int zz = (int)(sqrt((double)(255 * 255 * d)));
			if (zz <= 255 * mm)
				{
				m_pbz[z] = 255 - zz / mm;
				if (m_ramp == 5)
					{
					if (zz <= 255 * zq)
						m_pbb[z] = 255;
					else
						{
						double zrq = mm - zq;
						double dd = sqrt((double)d) - (double)zq;
						double phi = (4 * dd * atan(1.0)) / (double)zrq;
						phi = cos(phi);
						phi += 1.0;
						phi /= 2.0;
						m_pbb[z] = (BYTE)(255 * phi);
						}
					}
				else if (m_ramp == 4)
					{
double dd = sqrt((double)d);
double v = A * dd * dd * dd + B * dd * dd + C * dd + D;
m_pbb[z] = (BYTE)(255 * v);
					}
				else if (zz <= 255 * qq)
					m_pbb[z] = 255;//rand() % 256;//255;
				else
					{
					double vv = sqrt((double)d) - qq;
					vv = vv / m_brushf;
					if (m_ramp == 3)
//						vv = -2.0 * vv * vv * vv + 3.0 * vv * vv;
						vv = -1.0 * vv * vv * vv + 1.0 * vv * vv + 1.0 * vv;
					else if (m_ramp == 2)
						vv = vv * vv;
					else if (m_ramp == 1)
						vv = vv;
					else
						vv = sqrt(vv);//vv * vv;
//					m_pbb[z] = (BYTE)((rand() % 256) * (1 - vv));
					m_pbb[z] = (BYTE)(255 * (1 - vv));
//					m_pbb[z] = 1000 * (255 - (zz - 255 * qq) / m_factor);
					}
				}
			else
				{
				m_pbz[z] = m_pbb[z] = 0;
				}
			}
		}
	MakeMasks();
}

void CBrusher::MakeMasks()
{
	delete [] m_pMasks;
	UINT z = 40 + 1024;
	UINT rr;
	for (rr = 0; rr < MAX_RADIUS; rr++)
		{
		UINT w = 1 + 2 + rr + rr;
		UINT h = 1 + 2 + rr + rr;
		z += 2 * w * h;  // solid and perimteer
		}
	m_pMasks = new BYTE[z];
	LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)m_pMasks;
	BYTE * pBits = m_pMasks + sizeof(BITMAPINFOHEADER);
	z = 40 + 1024;
	lpBI->biSize			= sizeof(BITMAPINFOHEADER);
	lpBI->biWidth			= 16;
	lpBI->biHeight			= 16;
	lpBI->biPlanes			= 1;
	lpBI->biBitCount		= 8;
	lpBI->biCompression		= BI_RGB;
	lpBI->biSizeImage		= 0;
	lpBI->biXPelsPerMeter	= 0;
	lpBI->biYPelsPerMeter	= 0;
	lpBI->biClrUsed			= 256;
	lpBI->biClrImportant	= 0;
	int i;
	BYTE v = 0;
	for (i = 0; i < 256; i++)
		{
		pBits[4*i+0] = v;
		pBits[4*i+1] = v;
		pBits[4*i+2] = v;
		pBits[4*i+3] = 0;
		v++;//v = 255;
		}
	for (rr = 0; rr < MAX_RADIUS; rr++)
		{
		BYTE * pMask = m_pMasks + z;
		m_masks[rr] = z;
		UINT w = 1 + 2 + rr + rr;
		UINT h = 1 + 2 + rr + rr;
		pMask[0] = 255;
		BYTE v1 = 32;
		BYTE v2 = 64;
		BYTE v3 = 128;
		int r = rr + 1;
		int xx,yy,x,y,p,s;
		p = 1 + 2 * r;
		int pp = 4 * ((p + 3) / 4);
		if (r == 1)
			{
			pMask[0] = v1;
			pMask[1] = v2;
			pMask[4] = v2;
			pMask[5] = 255;
			}
		else if (r == 2)
			{
			pMask[0] = 0;
			pMask[1] = v1;
			pMask[2] = v2;
			pMask[8] = v1;
			pMask[9] = v3;
			pMask[10] = 255;
			pMask[16] = v2;
			pMask[17] = 255;
			pMask[18] = 255;
			}
		else if (r == 3)
			{
			pMask[0] = 0;
			pMask[1] = 0;
			pMask[2] = v1;
			pMask[3] = v2;
			pMask[8] = 0;
			pMask[9] = v2;
			pMask[10] = v3;
			pMask[11] = 255;
			pMask[16] = v1;
			pMask[17] = v3;
			pMask[18] = 255;
			pMask[19] = 255;
			pMask[24] = v2;
			pMask[25] = 255;
			pMask[26] = 255;
			pMask[27] = 255;
			}
	if (pMask[0] != 255)
		{
//
//	reflect three corners
//
		int i, j;
		for (i = 0; i <= r; i++)
		for (j = 1; j <= r; j++)
			pMask[pp*i+r+j] = pMask[pp*i+r-j];
		for (i = 1; i <= r; i++)
		for (j = 0; j <= (r+r); j++)
			pMask[pp*(r+i)+j] = pMask[pp*(r-i)+j];
		}
	else
	{
	int z = (r-2) * (r-2);
	int zzz = r * r;
	for (yy = 0; yy < p;yy++)
		{
		y = yy - r;
		s = y * y;
		for ( xx = 0; xx < p;xx++)
			{
			x = xx - r;
			BYTE vv;
			int zz = x * x + s;
			if (zz < z)
				vv = 255;
			else if	(zz < zzz)
				{
				double v = zzz-zz;
				v /= zzz - z;
				v = 255 * sqrt(v);
//				v = 210;
				vv = (BYTE)(v);
				}
			else
				vv = 0;
			pMask[yy*pp+xx] = vv;
			}
		}
	}
		
		z += w * h;
		z += w * h;
		}

}

void CBrusher::SetRadius(UINT rr)
{
	SetMyRadius((rr * m_factor + 1) / 2);
}

void CBrusher::SetMyRadius(UINT rr)
{
	m_radius = rr;
	m_brushw = 1 + 2 * (m_brushf + rr);
}

BYTE * CBrusher::CursorBits(UINT & extra, UINT rad)
{
//	rad *= m_factor;
	rad = (rad * m_factor+1) / 2;
	LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)m_pMasks;
	lpBI->biWidth			= 3 + 2 * (rad / m_factor);
	lpBI->biHeight			= 3 + 2 * (rad / m_factor);
	extra = m_masks[rad / m_factor];
	return m_pMasks;
}

BOOL CBrusher::SetupCache(BOOL bCaching)
{
	m_bCaching = bCaching;
	m_cache_count = 0;
	delete [] m_pc;
	delete [] m_pz;
	delete [] m_pq;
	m_pq = 0;
	UINT siz;
	if (bCaching)
		{
		siz = 5000;
		m_pq = new UINT[siz];
		memset(m_pq,255, 4 * siz);
		m_cache_count = 0;
		}
	else
		siz = m_width *  m_height;
	m_pc = new UINT[siz];
	m_pz = new UINT[siz];
	if (!m_bCaching)
		memset(m_pz,0, 4 * siz);  // clear cache
	return m_bCaching;
}

void CBrusher::UpdateCache()
{
	if (m_cache_count == 0)
		m_first = 0;
	if (!m_first)
		{
		m_first = 2500;
		m_start = 0;
		}
	else
		{
		m_first = 0;
		m_start = 2500;
		}
	m_stop = m_start + m_cache_count;
	m_cache_count = 0;
}

UINT CBrusher::CheckCache(UINT x, UINT y)
{
	if (!m_bCaching)
		return y * m_width + x;
	UINT key = (y << 16 ) + x;
	UINT index = m_start;
	UINT v,c;
	m_pq[m_stop] = key;
	for (; m_pq[index++] != key;);
	index--;
	if (index == m_stop)
		v = c = 0;
	else
		{
ASSERT(m_pq[index] == key);
		v = m_pz[index];
		c = m_pc[index];
		}
	index = m_first + m_cache_count++;
	m_pq[index] = key;
	m_pz[index] = v;
	m_pc[index] = c;
	return index;
}

/*
	draws points from x1,y1 to x2,y2
*/
void CBrusher::DoMiddle()
{
	double dx = m_p[0].x + 4.0 * m_p[1].x + m_p[2].x;
	double dy = m_p[0].y + 4.0 * m_p[1].y + m_p[2].y;
	double cx = -3.0 * m_p[0].x + 3.0 * m_p[2].x;
	double cy = -3.0 * m_p[0].y + 3.0 * m_p[2].y;
	double bx =  3.0 * m_p[0].x -6.0 * m_p[1].x + 3.0 * m_p[2].x;
	double by =  3.0 * m_p[0].y -6.0 * m_p[1].y + 3.0 * m_p[2].y;
	double ax = -1.0 * m_p[0].x + 3.0 * m_p[1].x - 3.0 * m_p[2].x + m_p[3].x; 
	double ay = -1.0 * m_p[0].y + 3.0 * m_p[1].y - 3.0 * m_p[2].y + m_p[3].y; 
	int dz;
	int addx = abs(m_p[2].x - m_p[1].x);
	int addy = abs(m_p[2].y - m_p[1].y);
	if (addx > addy)
		dz = addx;
	else
		dz = addy;
	if (!dz) dz = 1;
	double tt;
	double dd = (double)m_p[2].d - (double)m_p[1].d;
	double rr = (double)m_p[2].r - (double)m_p[1].r;
	int k;
	for (k = 1; k <= dz; k++)
		{
		tt = (double)k / (double)dz;
		m_density = m_p[1].d + (UINT)(k * dd / dz);  
		UINT zr = m_p[1].r + (UINT)(k * rr / dz);  
		if (zr != m_radius)
			SetMyRadius(zr);
		double ttt = tt * tt;
		double tttt = tt * ttt;
		double xx = ax * tttt + bx * ttt + cx * tt + dx;
		double yy = ay * tttt + by * ttt + cy * tt + dy;
		xx /= 6.0;
		yy /= 6.0;
		ApplyBrush((int)(xx +0.5),(int)(yy + 0.5));
		}
}

/*
	draws points from x0,y0 to x1,y1
*/
void CBrusher::DoFront()
{
	ASSERT(m_count == 3);
// formula is At^2 + Bt + c = x 
// slope is 2At + B = m1
//we set slope at x1 to x2 -x0
double m1;
double ax,bx,cx;
double p0,p1;
	p0 = m_p[0].x;
	p1 = m_p[1].x;
	m1 = (m_p[2].x - m_p[0].x) / 2;
//assume evaluation of parametric of 0 at p1
	cx = p1;
	ax = m1 - p1 + p0;
	bx = 2 * (p1 - p0) - m1;
double ay,by,cy;
	p0 = m_p[0].y;
	p1 = m_p[1].y;
	m1 = (m_p[2].y - m_p[0].y) / 2;
//assume evaluation of parametric of 0 at p1
	cy = p1;
	ay = m1 - p1 + p0;
	by = 2 * (p1 - p0) - m1;
	
	int dz;
	int addx = abs(m_p[1].x - m_p[0].x);
	int addy = abs(m_p[1].y - m_p[0].y);
	if (addx > addy)
		dz = addx;
	else
		dz = addy;
	if (!dz) dz = 1;
	double tt;
	double dd = (double)m_p[1].d - (double)m_p[0].d;
	double rr = (double)m_p[1].r - (double)m_p[0].r;
	for (int k = 0; k <= dz;k++)
		{
		tt = (double)k / (double)dz;
		m_density = m_p[0].d + (UINT)(k * dd / dz);  
		UINT zr = m_p[0].r + (UINT)(k * rr / dz);  
		if (zr != m_radius)
			SetMyRadius(zr);
		double ts = tt * tt;
		double xx = ax * ts + bx * tt + cx;
		double yy = ay * ts + by * tt + cy;
		ApplyBrush((int)(xx +0.5),(int)(yy + 0.5));
/*
if (k == dz)
{
	DPF("p2x:%d,%d",m_p[2].x,(int)xx);
	DPF("p2y:%d,%d",m_p[2].y,(int)yy);
}
*/
		}

}
/*
	draws points from x2,y2 to x3,y3

*****
	note array has last three poisnts in 0,1,2
*/
void CBrusher::DoBack()
{
	for (;m_count < 3;m_count++)		// fill out array
		m_p[m_count] = m_p[m_count-1];

// formula is At^2 + Bt + c = x 
// slope is 2At + B = m1
//we set slope at x2 to x3 -x1
double m2;
double ax,bx,cx;
double p2,p3;
	p2 = m_p[1].x;
	p3 = m_p[2].x;
	m2 = (m_p[2].x - m_p[0].x) / 2;
//assume evaluation of parametric of 0 at p1
	cx = p2;
	bx = m2;
	ax = p3 - p2 - m2;
double ay,by,cy;
	p2 = m_p[1].y;
	p3 = m_p[2].y;
	m2 = (m_p[2].y - m_p[0].y) / 2;
//assume evaluation of parametric of 0 at p1
	cy = p2;
	by = m2;
	ay = p3 - p2 - m2;
	
	int dz;
	int addx = abs(m_p[2].x - m_p[1].x);
	int addy = abs(m_p[2].y - m_p[1].y);
	if (addx > addy)
		dz = addx;
	else
		dz = addy;
	if (!dz) dz = 1;
	double tt;
	double dd = (double)m_p[2].d - (double)m_p[1].d;
	double rr = (double)m_p[2].r - (double)m_p[1].r;
	int k;
	for (k = 1; k <= dz; k++)
		{
		tt = (double)k / (double)dz;
		m_density = m_p[1].d + (UINT)(k * dd / dz);  
		UINT zr = m_p[1].r + (UINT)(k * rr / dz);  
		if (zr != m_radius)
			SetMyRadius(zr);
		double ttt = tt * tt;
		double xx = ax * ttt + bx * tt + cx;
		double yy = ay * ttt + by * tt + cy;
		ApplyBrush((int)(xx +0.5),(int)(yy + 0.5));
		}
}

void CBrusher::DumpSmart()
{
	char amsg[280];
	CFile file;
	DWORD mode = CFile::modeWrite;// | CFile::modeCreate;
	mode |= CFile::modeCreate;
	strcpy(amsg,"c:\\flipbook\\camsk6\\smart.txt");
	if (!file.Open(amsg, mode))
		{
		mode |= CFile::modeCreate;
		if (!file.Open(amsg, mode))
			return;
		}
	file.SeekToEnd();
	UINT i;
	for (i = 1; i < m_nSmarts; i++)
		{
		sprintf(amsg, "i:%3d,x:%3d,y:%3d,r:%d,d:%d\r\n",
					i,m_pSmarts[i].x, m_pSmarts[i].y,
					m_pInfo[i]/1000,m_pInfo[i]%1000);
		file.Write(amsg, strlen(amsg));
		}
	file.Close();
}

void CBrusher::Smoother()
{
	UINT i;
	for (i = 1; (i+2) < m_nSmarts; i++)
		{
		if ((abs(m_pSmarts[i].x - m_pSmarts[i+1].x) <= (int)m_factor)
			&&(abs(m_pSmarts[i].y - m_pSmarts[i+1].y)<= (int)m_factor))
			{
			UINT j;
			for (j = i+1; (j+1) < m_nSmarts;j++)
				{
				m_pSmarts[j] = m_pSmarts[j+1];
				m_pInfo[j] = m_pInfo[j+1];
				}
			m_nSmarts--;
			continue;
			}
		if ((abs(m_pSmarts[i-1].x - m_pSmarts[i+1].x) <= 10 * (int)m_factor)
			&&(abs(m_pSmarts[i-1].y - m_pSmarts[i+1].y)<= 10 * (int)m_factor))
		{
		m_pSmarts[i].x = (2 * m_pSmarts[i-1].x + 
								m_pSmarts[i].x + 2 * m_pSmarts[i+1].x) / 5;
		m_pSmarts[i].y = (2 * m_pSmarts[i-1].y + 
								m_pSmarts[i].y + 2 * m_pSmarts[i+1].y) / 5;
			}	

		}
	
}

void CBrusher::Replay()
{
	m_count = 0;
	m_bFirst = 1;
	SetupCache(1);
	UINT i;
//	DumpSmart();
	if (m_nSmarts < 1)
		return;
	if (m_nSmarts < 3)
		{
		for (i = m_nSmarts;i < 4;i++)
			{
			m_pSmarts[i] = m_pSmarts[i-1];
			m_pInfo[i] = m_pInfo[i-1];
			}
		}
	else
		Smoother();
	ZPoint(0,m_pSmarts[0].x,m_pSmarts[0].y,m_pInfo[0]/1000,m_pInfo[0]%1000);
	ZPoint(1,m_pSmarts[1].x,m_pSmarts[1].y,m_pInfo[1]/1000,m_pInfo[1]%1000);
	ZPoint(2,m_pSmarts[2].x,m_pSmarts[2].y,m_pInfo[2]/1000,m_pInfo[2]%1000);
	m_count = 3;
//	DoFront();
	m_count = 4;
	for (i = 3; i < m_nSmarts; i++)
		{
		ZPoint(3,m_pSmarts[i].x,m_pSmarts[i].y,m_pInfo[i]/1000,m_pInfo[i]%1000);
		DoMiddle();
		m_p[0] = m_p[1];
		m_p[1] = m_p[2];
		m_p[2] = m_p[3];
		}
	DoBack();
}

void CBrusher::SetSmart(BOOL bOn)
{
	delete [] m_pSmarts;
	delete [] m_pInfo;
	if (bOn)
		{
		m_pSmarts = new POINT[MAX_SMARTS];
		m_pInfo = new int[MAX_SMARTS];
		}
	else
		{
		m_pSmarts = 0;
		m_pInfo = 0;
	}
}

void CBrusher::DrawCurve(CPoint p1, CPoint p2, CPoint p3, CPoint p4,
					UINT rad, UINT den)
{
	BOOL bOld = SetupCache(0);
	DrawBegin(p1.x, p1.y, rad, den);
	int it;
	for (it = 0; it < 10000; it++)
		{
		double t = (double)it / 10000.0;
#ifndef ZMAT
		double tt = t;
		double x = p1.x;
		double y = p1.y;
		x += t * ( -3.0 * p1.x + 3.0 * p2.x);
		y += t * ( -3.0 * p1.y + 3.0 * p2.y);
		t *= t;
		x += t * (3.0 * p1.x -6.0 * p2.x + 3.0 * p3.x);
		y += t * (3.0 * p1.y -6.0 * p2.y + 3.0 * p3.y);
		t *= tt;
		x += t * (-p1.x + 3.0 * p2.x - 3.0 * p3.x + p4.x); 
		y += t * (-p1.y + 3.0 * p2.y - 3.0 * p3.y + p4.y); 
#else
		double zx = (1.0 - t) * (1.0 - t) * (1.0 - t) * p1.x;
			zx += 3.0 * t * (1.0 - t ) * (1.0 - t) * p2.x;
			zx += 3.0 * t * t * (1.0 - t ) * p3.x;
			zx +=  t * t * t * p4.x;
		double zy = (1.0 - t) * (1.0 - t) * (1.0 - t) * p1.y;
			zy += 3.0 * t * (1.0 - t ) * (1.0 - t) * p2.y;
			zy += 3.0 * t * t * (1.0 - t ) * p3.y;
			zy +=  t * t * t * p4.y;
#endif
//int zxx = (int)(m_factor * zx);
//int zyy = (int)(m_factor * zy);
		int xx = (int)(m_factor * x);
		int yy = (int)(m_factor * y);
		/*
if ((zxx != xx) || (zyy != yy))
	{
	DPF("zxx:%d,zyy:%d,xx:%d,yy:%d",zxx,zyy,xx,yy);
	}
	*/
		ApplyBrush(xx, yy);
		}
	SetupCache(bOld);
}

