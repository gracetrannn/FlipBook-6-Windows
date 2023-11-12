#include "stdafx.h"
#include "keyer.h"
#include "math.h"

#define IND(a,b,c) ((a >> 2) << 12) + ((b >> 2) << 6) + (c >> 2)



#define POWER(a,b) (a > .0001 ? exp(b * log(a)) : 0)
#define DEPOWER(a,b) (a > .0001 ? exp(log(a) / b) : 0)
#define KR 903.3
double ref_X =  95.02; //  Observer= 2 degree, Illuminant= D65
double ref_Y = 100.000;
double ref_Z = 108.82;

#define KA 172.30
#define KB 67.20
double ff(double aa)
{
	if ( aa > 0.008856 )
		aa = DEPOWER(aa, 3.0);
	else
		aa = ( KR * aa + 16) / 116;
	return aa;
}


double fs(double q)
{
/*
	if ( q > 0.04045 )
		q = POWER((( q + 0.055 ) / 1.055), 2.4);
	else
		q = q / 12.92;
*/
	q = POWER(q,2.2);
	q = q * 100.0;
	return q;
}


void hlab(int & il, int & ia, int &ib, int r, int g, int b)
{
		double var_R = r / 255.0;
		double var_G = g / 255.0;
		double var_B = b / 255.0;
		var_R = fs(var_R);
		var_G = fs(var_G);
		var_B = fs(var_B);

//Observer. = 2 degree, Illuminant = D65
double X = var_R * 0.4124 + var_G * 0.3576 + var_B * 0.1805;
double Y = var_R * 0.2126 + var_G * 0.7152 + var_B * 0.0722;
double Z = var_R * 0.0193 + var_G * 0.1192 + var_B * 0.9505;

double var_X = X / ref_X;
double var_Y = Y / ref_Y;
double var_Z = Z / ref_Z;

	double YY = DEPOWER(var_Y,2.0);
	il = (int)(100 * YY);
	ia = (int)(KA * ((var_X - var_Y) / YY));
	ib = (int)(KB * ((var_Y - var_Z) / YY));
}

void lab(int & il, int & ia, int &ib, int r, int g, int b)
{
		double var_R = r / 255.0;
		double var_G = g / 255.0;
		double var_B = b / 255.0;
		var_R = fs(var_R);
		var_G = fs(var_G);
		var_B = fs(var_B);

//Observer. = 2 degree, Illuminant = D65
double X = var_R * 0.4124 + var_G * 0.3576 + var_B * 0.1805;
double Y = var_R * 0.2126 + var_G * 0.7152 + var_B * 0.0722;
double Z = var_R * 0.0193 + var_G * 0.1192 + var_B * 0.9505;

double var_X = X / ref_X;
double var_Y = Y / ref_Y;
double var_Z = Z / ref_Z;

	double fx = ff(var_X);
	double fy = ff(var_Y);
	double fz = ff(var_Z);

	double l = 116.0 * fy - 16;
	double a = 500.0 * (fx - fy);
	double bb= 200.0 * (fy - fz);
	il = (int)(100 * l);
	ia = (int)(100 * a);
	ib = (int)(100 * bb);
}



static void rgb_to_hls(float r, float g, float b, float *h, float *l, float *s)
{
  float rc, gc, bc;
  float maxcol, mincol;

  maxcol = max(r, max(g, b));
  mincol = min(r, min(g, b));
  *l = (mincol + maxcol) / 2;   /* lightness */
  if (maxcol == mincol) {   /* achromatic case */
    *s = 0;         /* hue is undefined in the achromatic case */
    *h = 0;
  } else {
    if (*l <= 0.5)
      *s = (maxcol - mincol) / (maxcol + mincol);
    else
      *s = (maxcol - mincol) / (2 - maxcol - mincol);
    
    rc = (maxcol - r) / (maxcol - mincol);
    gc = (maxcol - g) / (maxcol - mincol);
    bc = (maxcol - b) / (maxcol - mincol);
    if (r == maxcol)
      *h = bc - gc;
    else
      if (g == maxcol)
        *h = 2 + rc - bc;
      else 
        *h = 4 + gc - rc;
    
    *h *= (float)60;
    if (*h < (float)0)
      *h += (float)360;
    *h /= (float)360;
  }
}
void CColorKey::xlate(int &j, int &k, int &l, int r, int g, int b)
{
	if (m_kind == 0)
		{
		j = r;
		k = g;
		l = b;
		}
	else if (m_kind == 1)
		{
		j = (299 * r + 587 * g + 114 * b) / 1000;
		k = (596 * r + 275 * g + 321 * b) / 1000;
		l = (212 * r + 523 * g + 311 * b) / 1000;
		}
	else if (m_kind == 2)
		{
	   float  rr, gg, bb;
	   float  hh, ll, ss;
	
   /* convert from integer rgb (0..255) to floating point rgb (0.0..1.0) */
   		rr = r / (float)255;
   		gg = g / (float)255;
   		bb = b / (float)255;
	   rgb_to_hls(rr, gg, bb, &hh, &ll, &ss);

   /* convert from floating point hls (0.0..1.0) to integer hls (0..240) */
   		j = (int)(hh * (float)255+ 0.5);
   		k= (int)(ll * (float)255 + 0.5);
   		l = (int)(ss * (float)255 + 0.5);
		}
	else
		{
		int k1,k2,k3;
		hlab(k1,k2,k3,r,g,b);
		j = k1;
		k = k2;
		l = k3;
		}
}


void CColorKey::Setup()
{
	if (!m_p1)
		{
		int s = 64 * 64 * 64;
		m_p1 = new int[s];
		m_p2 = new int[s];
		m_p3 = new int[s];
		}
	int i,j,k,z;
	for (i = 0,z =0; i < 64;i++)
	for (j = 0; j < 64;j++)
	for (k = 0; k < 64;k++,z++)
		{
		int k1,k2,k3;
		xlate(k1,k2,k3,255*i/63,255*j/63,255*k/63);
		m_p1[z] = k1;
		m_p2[z] = k2;
		m_p3[z] = k3;
		}
}

void CColorKey::Process(BYTE * dstp, BYTE * srcp, UINT w, UINT h, UINT d,
			int rr, int gg, int bb, int ww, int ff)
{
DPF2("keyproc:%d",ff);
		BOOL bInvert = ff & 1;
		if ((ff >> 1) != (int)m_kind)
			{
			m_kind = ff >> 1;
			Setup();
			}
		UINT x,y,zz;
		int k1,k2,k3;
		int qq = IND(rr,gg,bb);
		k1 = m_p1[qq];
		k2 = m_p2[qq];
		k3 = m_p3[qq];
		zz = ww;
		if (zz > 100) zz = 100;
//		if (zz <= 100) zz = 100 - zz; else zz = 0;
		zz = zz * zz;// * 3800;
		int mz = 0;
DPF2("keyproc:%d,zz:%d,ww:%d",d,zz,ww);
		if (d == 3)
			{
			UINT p = 4 * ((d * w + 3) / 4);
			for (y = 0; y < h; y++)
				{
				for (x = 0; x < w; x++)
					{
					int z;
					int q1, q2, q3, qq;
					qq = IND(dstp[3*x+2],dstp[3*x+1],dstp[3*x+0]);
					q1 = m_p1[qq];
					q2 = m_p2[qq];
					q3 = m_p3[qq];
if (m_kind == 2)
	{
	if (q1 > k1)
		q1 = (q1 - k1) % 128;
	else
		q1 = (k1 - q1) % 128;
	}
else
	q1 -= k1;
					q2 -= k2;
					q3 -= k3;
					z = q1 * q1 + q2 * q2 + q3 * q3;
					if (z > mz) mz = z;
					if (z < (int)zz) 
						{
/*						
UINT f = (255 * z) / zz;
UINT b = 255 - f;
UINT i;
for (i = 0; i < 3; i++)
	{
	UINT v = f * dstp[3*x+i] + b * srcp[3*x+i];
	dstp[3*x+i] = v / 255;
	}
*/	
						dstp[3*x+0] = srcp[3*x+0];
						dstp[3*x+1] = srcp[3*x+1];
						dstp[3*x+2] = srcp[3*x+2];
						}
					else if (bInvert)
						{
						dstp[3*x+0] = bb;//srcp[3*x+0];
						dstp[3*x+1] = gg;//srcp[3*x+1];
						dstp[3*x+2] = rr;//srcp[3*x+2];
						}
					}
				srcp += p;
				dstp += p;
				}
			}
		else if (d == 4)
			{
			UINT p = 4 * ((3 * w + 3) / 4) - 3 * w;
			for (y = 0; y < h; y++)
				{
				for (x = 0; x < w; x++)
					{
					int z;
					int q1, q2, q3;
					int b = (*dstp++ = *srcp++);	
					int g = (*dstp++ = *srcp++);	
					int r = (*dstp++ = *srcp++);	
					xlate(q1,q2,q3,r,g,b);
					q1 -= k1;
					q2 -= k2;
					q3 -= k3;
					z = q1 * q1 + q2 * q2 + q3 * q3;
					if (z > mz) mz = z;
					if (z < (int)zz) 
						*dstp++ = 0;
					else
						*dstp++ = 255;
					}
				srcp += p;
				}
			 }
		
DPF2("mz:%d",mz);
}

CColorKey::CColorKey()
{
	m_p1 = 0;
	m_p2 = 0;
	m_p3 = 0;
	m_kind = 9999;
}

CColorKey::~CColorKey()
{
	delete [] m_p1;
	delete [] m_p2;
	delete [] m_p3;
}

