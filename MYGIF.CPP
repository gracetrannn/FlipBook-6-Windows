#include "stdafx.h"
#include "mygif.h"
//#define CLEAR 256
//#define EOI 257
//#define FIRST 258
#define LAST 4095
#define MAXLEN 255

//#define BEFORE

static BYTE dt2[4] = {0,2,3,1};
static BYTE dt44[16] = {0,2,4,9,3,1,6,11,5,7,8,13,10,12,14,15};
static BYTE dt4[16] = {0,8,2,10,12,4,14,6,3,11,1,9,15,7,13,5};
static BYTE dt8[64] = {
 0,32, 8,39, 2,33,10,41,
47,16,55,24,49,18,57,26,
12,43, 4,35,14,45, 6,37,
59,28,51,20,61,30,53,22,
 3,34,11,42, 1,32, 9,40,
50,19,58,27,48,17,56,25,
15,46, 7,38,13,44, 5,36,
62,31,54,23,60,29,52,21};
static BYTE odt[64]= {
	  0,128, 32,159,  8,135, 40,167,
	191, 64,223, 96,199, 72,231,104,
	 48,175, 16,143, 56,183, 24,151,
	239,112,207, 80,247,120,215, 88,
	 12,139, 44,171,  4,131, 36,163,
	203, 76,235,108,195, 68,227,100,
	 60,187, 28,155, 52,179, 20,147,
	251,124,219, 92,243,116,211, 84};

static BYTE optipal[768]= {
  0,  0,  0,
 17, 17, 17,
 17,  8, 63,
 17,  8,114,
 34, 34, 34,
 34, 25, 80,
 34, 12,131,
 34,  4,187,
 72, 12, 34,
 72,  4, 80,
 12, 80,  4,
 12, 72, 51,
 12, 59, 97,
 12, 51,148,
 12, 42,204,
 51, 59,  4,
 51, 51, 51,
 51, 42, 97,
 51, 29,148,
 51, 21,204,
 89, 42,  4,
 89, 29, 51,
 89, 21, 97,
 89, 12,148,
 89,  0,204,
127, 21,  4,
127, 12, 51,
127,  0, 97,
 29, 97, 21,
 29, 89, 68,
 29, 76,114,
 29, 68,165,
 29, 59,221,
 68, 76, 21,
 68, 68, 68,
 68, 59,114,
 68, 46,165,
 68, 38,221,
106, 59, 21,
106, 46, 68,
106, 38,114,
106, 29,165,
106, 17,221,
144, 38, 21,
144, 29, 68,
144, 17,114,
144,  8,165,
191, 12, 21,
191,  4, 68,
 0,140, 38,
 0,127, 85,
 0,119,131,
 0,110,182,
 0, 97,238,
 46,114, 38,
 46,106, 85,
 46, 93,131,
 46, 85,182,
 46, 76,238,
 85, 93, 38,
 85, 85, 85,
 85, 76,131,
 85, 63,182,
 85, 55,238,
123, 76, 38,
123, 63, 85,
123, 55,131,
123, 46,182,
123, 34,238,
161, 55, 38,
161, 46, 85,
161, 34,131,
161, 25,182,
161, 17,238,
208, 29, 38,
208, 21, 85,
208, 12,131,
208,  0,182,
 12,157, 55,
 12,144,102,
 12,136,148,
 12,127,199,
 63,131, 55,
 63,123,102,
 63,110,148,
 63,102,199,
102,110, 55,
102,102,102,
102, 93,148,
102, 80,199,
140, 93, 55,
140, 80,102,
140, 72,148,
140, 63,199,
178, 72, 55,
178, 63,102,
178, 51,148,
178, 42,199,
225, 46, 55,
225, 38,102,
225, 29,148,
225, 17,199,
 29,182, 12,
 29,174, 72,
 29,161,119,
 29,153,165,
 29,144,216,
 80,157, 12,
 80,148, 72,
 80,140,119,
 80,127,165,
 80,119,216,
119,140, 12,
119,127, 72,
119,119,119,
119,110,165,
119, 97,216,
157,119, 12,
157,110, 72,
157, 97,119,
157, 89,165,
157, 80,216,
195, 97, 12,
195, 89, 72,
195, 80,119,
195, 68,165,
195, 59,216,
242, 76, 12,
242, 63, 72,
242, 55,119,
242, 46,165,
242, 34,216,
 46,199, 29,
 46,191, 89,
 46,178,136,
 46,170,182,
 46,161,233,
 97,174, 29,
 97,165, 89,
 97,157,136,
 97,144,182,
 97,136,233,
136,157, 29,
136,144, 89,
136,136,136,
136,127,182,
136,114,233,
174,136, 29,
174,127, 89,
174,114,136,
174,106,182,
174, 97,233,
212,114, 29,
212,106, 89,
212, 97,136,
212, 85,182,
212, 76,233,
  8,246, 46,
  8,233,106,
  8,225,153,
  8,216,199,
  8,208,250,
 63,216, 46,
 63,208,106,
 63,195,153,
 63,187,199,
 63,178,250,
114,191, 46,
114,182,106,
114,174,153,
114,161,199,
114,153,250,
153,174, 46,
153,161,106,
153,153,153,
153,144,199,
153,131,250,
191,153, 46,
191,144,106,
191,131,153,
191,123,199,
191,114,250,
229,131, 46,
229,123,106,
229,114,153,
229,102,199,
229, 93,250,
 25,250,123,
 25,242,170,
 25,233,216,
 80,233, 63,
 80,225,123,
 80,212,170,
 80,204,216,
131,208, 63,
131,199,123,
131,191,170,
131,178,216,
170,191, 63,
170,178,123,
170,170,170,
170,161,216,
208,170, 63,
208,161,123,
208,148,170,
208,140,216,
246,148, 63,
246,140,123,
246,131,170,
246,119,216,
 42,250,233,
 97,250, 80,
 97,242,140,
 97,229,187,
 97,221,233,
148,242,  8,
148,225, 80,
148,216,140,
148,208,187,
148,195,233,
187,221,  8,
187,208, 80,
187,195,140,
187,187,187,
187,178,233,
225,204,  8,
225,187, 80,
225,178,140,
225,165,187,
225,157,233,
114,255,157,
114,246,204,
114,238,250,
165,242, 97,
165,233,157,
165,225,204,
165,212,250,
204,238, 25,
204,225, 97,
204,212,157,
204,204,204,
204,195,250,
242,221, 25,
242,204, 97,
242,195,157,
242,182,204,
242,174,250,
182,250,174,
182,242,221,
221,255, 42,
221,242,114,
221,229,174,
221,221,221,
238,246,191,
238,238,238,
255,255,255};

typedef struct _tn {
  int code;
  CTreeNode **node;
} TreeNode, * TreePtr;

class CTreeNode : public TreeNode
{
};

CMyGif::CMyGif()
{
	m_option = 0;
	m_pBuffer = 0;
	m_pPalette = 0;
	m_pTree = 0;
	m_bNewPals = 0;
	m_pTemp = 0;
	m_pPalette = new BYTE[768];
}

CMyGif::~CMyGif()
{
//printf("mygif destruct\n");
	ClearTree(m_pTree,m_clear);
	delete [] m_pBuffer;
	delete [] m_pTemp;
	delete [] m_pPalette;
}

UINT CMyGif::Encode(CFile * pFile, UINT w, UINT h, 
				UINT frames, UINT color, UINT rate, BOOL bLoop)
{
	m_pFile = pFile;
	m_width = w;
	m_height = h;
	m_flags = color >> 12;
	m_depth = (color >> 8) & 15;
	color &= 7;
	if ((m_depth == 3) && (color != 2))
		m_bGray = 1;
	else
		m_bGray = 0;
	m_bpp = color == 0 ? 4 : 8;
	m_clear = 1 << m_bpp;
	m_frames = frames;
	m_frame = 0;
	m_rate = rate;
	m_bLoop = rate ? bLoop : 0;
	m_pTemp = new BYTE[m_width * m_height];
	m_pBuffer = new BYTE[2000];

	BYTE * pos = m_pBuffer;
	*pos++ = 'G';
	*pos++ = 'I';
	*pos++ = 'F';
	*pos++ = '8';
	*pos++ = '9';
	*pos++ = 'a';
	*pos++ = 0xff & m_width;
  	*pos++ = m_width >> 8;
  	*pos++ = 0xff & m_height;
  	*pos++ = m_height >> 8;
 	*pos++ = m_bpp - 1 + 0x70 + (m_bNewPals ? 0x80 : 0);
	*pos++ = 0xff; // bg color index
	*pos++ = 0x0;		// aspect ratio
	if (m_bNewPals)
		{
		int i,c;
		c = 3 << m_bpp;
		for(i = 0; i < c;i++)
    		*pos++ = m_pPalette[i];
		m_bNewPals = 0;
		}
	if ((m_frames > 1) && m_bLoop)
		{
		*pos++ = 0x21;
		*pos++ = 0xFF;
		*pos++ = 0x0B;
		*pos++ = 'N';
		*pos++ = 'E';
		*pos++ = 'T';
		*pos++ = 'S';
		*pos++ = 'C';
		*pos++ = 'A';
		*pos++ = 'P';
		*pos++ = 'E';
		*pos++ = '2';
		*pos++ = '.';
		*pos++ = '0';
		*pos++ = 3;
		*pos++ = 1;
		*pos++ = 255;		// iteration count
		*pos++ = 255;
		*pos++ = 0;		// sub blockterminator
	}
	if (m_frames && m_rate)
		{
		m_fact1 = ((100 + m_rate - 1) / m_rate) - 1;
		m_dacc1 = m_dacc2 = 0;

		}
	Write(pos-m_pBuffer);
	return 0;
}


UINT CMyGif::SetPalette(BYTE * pPalette)
{
	int i;
	if (!pPalette)
		pPalette = (BYTE *)&optipal;
	for(i = 0; i < 768;i++)
    	m_pPalette[i] = pPalette[i];
	if (m_option > 1)
		{
		for (i = 0; i < 216; i++)
			{
			m_pPalette[3*i+0] = 51 * (i % 6);
			m_pPalette[3*i+1] = 51 * ((i / 6) % 6);
			m_pPalette[3*i+2] = 51 * ((i / 36) % 6);
			}
		}
	m_bNewPals = TRUE;
	return 0;
}

UINT CMyGif::AddFrame(BYTE * pData, UINT pitch /* = 0 */)
{
	BYTE * pBuf = m_pTemp;
	if (!pitch)
		pitch = m_width * m_depth;
	if ((m_depth != 3) || m_bGray)
		{
		UINT x,y;
		for (y = 0; y < m_height;y++)
		for (x = 0 ; x < m_width;x++)
			{
			UINT v;
			if (m_bGray)
				v = (11 * (UINT)pData[y*pitch+3*x+0] + 
					+ 59 * (UINT)pData[y*pitch+3*x+1] + 
					+ 30 * (UINT)pData[y*pitch+3*x+2] ) / 100;
			else
				v = pData[y*pitch+x];
			if (m_bpp == 4)
				v >>= 4;
			pBuf[(m_height - 1 - y)*m_width+x] = v;
			}
		}
	else if (m_option < 2)
		{
		UINT x,y;
		for (y = 0; y < m_height;y++)
		for (x = 0 ; x < m_width;x++)
			{
			int i,e,r,g,b,z;
			r = pData[y * pitch + 3*x+2];
			g = pData[y * pitch + 3*x+1];
			b = pData[y * pitch + 3*x+0];
			e = 500000;
			for (i = 0; i < 256; i++)
				{
				int rr,gg,bb,q;
				rr = r - m_pPalette[3*i+0];
				gg = g - m_pPalette[3*i+1];
				bb = b - m_pPalette[3*i+2];
				q = rr * rr + gg * gg + bb * bb;
				if (q < e)
					{
					e = q;
					z = i;
					if (!e)
						break;
					}
				}
			pBuf[(m_height - 1 - y)*m_width+x] = z;
			}
		}
	else
		{
		UINT x,y;
		for (y = 0; y < m_height;y++)
		for (x = 0 ; x < m_width;x++)
			{
			int r,g,b,z;
			r = pData[y * pitch + 3*x+2];
			g = pData[y * pitch + 3*x+1];
			b = pData[y * pitch + 3*x+0];
			int rr,gg,bb,dr,dg,db,dt;
			if (m_option == 5)
				{
				rr = r / 51;
				gg = g / 51;
				bb = b / 51;
				dr = r % 51;
				dg = g % 51;
				db = b % 51;
				dt = (17 * dt44[4*(y & 3) + (x & 3)]) / 5;
				}
			if (m_option == 4)
				{
				rr = r / 51;
				gg = g / 51;
				bb = b / 51;
				dr = r % 51;
				dg = g % 51;
				db = b % 51;
				dt = (17 * dt8[8*(y & 7) + (x & 7)]) / 21;
				}
			if (m_option == 3)
				{
				rr = r / 51;
				gg = g / 51;
				bb = b / 51;
				dr = r % 51;
				dg = g % 51;
				db = b % 51;
				dt = (17 * dt4[4*(y & 3) + (x & 3)]) / 5;
				}
			if (m_option == 2)
				{
				rr = r / 51;
				gg = g / 51;
				bb = b / 51;
				dr = r % 51;
				dg = g % 51;
				db = b % 51;
				dt = (17 * dt2[2*(y & 1) + (x & 1)]);
				}
			if (dr > dt) rr++;
			if (dg > dt) gg++;
			if (db > dt) bb++;
			z = rr + 6 * gg + 36 * bb;
			pBuf[(m_height - 1 - y)*m_width+x] = z;
			}
		}


	BYTE * pos = m_pBuffer;
	if ((m_frames > 1))// && m_bLoop)
		{
		UINT v = m_fact1;
		m_dacc1 += 100;
		m_dacc2 += m_rate * m_fact1;
		if ((m_dacc2 + m_rate) <= m_dacc1)
			{
			v++;
			m_dacc2 += m_rate;
			}
DPF("frame:%3d,dacc1:%4d,dacc2:%4d,v:%d",m_frame,m_dacc1,m_dacc2,v);
		*pos++ = 0x21;
		*pos++ = 0xf9;
		*pos++ = 0x04;
		*pos++ = 0;//(m_flags & 1) ? 1 : 0; // transparent
		*pos++ = v;//10; // delay in hundedths
		*pos++ = 0x00;
		*pos++ = 0;// 255;// white is transparnt 0x00;
		*pos++ = 0x00;
		}
	*pos++ = 0x2c;
	*pos++ = 0x00;
	*pos++ = 0x00;
	*pos++ = 0x00;
	*pos++ = 0x00;
	*pos++ = 0xff & m_width;
  	*pos++ = m_width >> 8;
  	*pos++ = 0xff & m_height;
  	*pos++ = m_height >> 8;
	if (m_bNewPals)
		*pos++ = 0x80 + m_bpp - 1;
	else
		*pos++ = m_bpp - 1;
	if (m_bNewPals)
		{
		int i;
		for(i = 0; i < 768;i++)
    		*pos++ = m_pPalette[i];
		m_bNewPals = 0;
		}
	*pos++ = m_bpp;
	Write(pos-m_pBuffer);
	m_cnt = 0;
	m_acc = 0;
	m_nBytes = 0;
	WORD next = m_clear + 2;
	m_nBits = m_bpp + 1;
	InitIndices(m_clear);
	AddCodeToBuffer(m_clear);
	LPBYTE end = pBuf + m_width * m_height;
	int len = 1;
	int code;
	while((pBuf + len) < end)
		{
	    code = SearchTreeForCode(m_pTree,pBuf,len, next,m_clear,0);
	    if(code != -1)
			{
			AddCodeToBuffer(code);
      		pBuf += len-1;
      		len = 0;
			if( next++ == (1 << m_nBits))
				m_nBits++;
			if(next == LAST)
				{
				InitIndices(m_clear);
				AddCodeToBuffer(m_clear);
				next = m_clear + 2;
				m_nBits = m_bpp + 1;
				}
			}
    	len++;
  		}
 	code = SearchTreeForCode(m_pTree,pBuf,len,next,m_clear,1);
	AddCodeToBuffer(code);
  	AddCodeToBuffer(m_clear+1);
	AddCodeToBuffer(0x0,-1);
	AddCodeToBuffer(0x0,8);
	Write(m_nBytes);
	m_frame++;
//	ClearTree(m_pTree,m_clear);
//	m_pTree = 0;
	return 0;
}

UINT CMyGif::FinishIt()
{
	m_nBytes = 0;
	AddCodeToBuffer(0x3b,8);
	Write(m_nBytes);
	return 0;
}

void CMyGif::InitIndices(int cc)
{
	int i;
	ClearTree(m_pTree,cc);
	m_pTree = AddNode(-1,cc);
	for(i = 0;i < cc; i++)
		m_pTree->node[i] = AddNode(i,cc);
}

CTreeNode * CMyGif::AddNode(int code,int n)
{
  CTreeNode * node;
  node = new CTreeNode;
  node->code = code;
  node->node = new CTreeNode *[n];
  do {
    node->node[--n] = 0;//NULL;
  } while (n);

  return node;
}

void CMyGif::ClearTree(CTreeNode * node,int n)
{
  int i;
  
//  if(node==NULL) return;
  if(!node) return;

  for(i = 0; i < n;i++)
		ClearTree(node->node[i],n);
  delete [] node->node;
  delete node;
}

void CMyGif::AddCodeToBuffer(int code,short n)
{
	if (n == 8)
		{
		m_pBuffer[m_nBytes++] = code;
		return;
		}
	if(n < 0)
		{
		if(m_cnt)
			m_pBuffer[1+m_nBytes++] = (BYTE)m_acc;
		FlushBuffer();
		return;
		}
	int m1 = 1;
	for (n = m_nBits;n;n--)
		{
#ifdef BEFORE
		if (m_cnt >= 8)
			{
			m_pBuffer[1+m_nBytes++] = (BYTE)m_acc;
			m_cnt = 0;
			m_acc = 0;
			if (m_nBytes >= MAXLEN)
				FlushBuffer();
			}
#endif
		int m2 = 1 << m_cnt;
		if (code & m1)
			m_acc |= m2;
		m1 <<= 1;
		m_cnt++;
#ifndef BEFORE
		if (m_cnt >= 8)
			{
			m_pBuffer[1+m_nBytes++] = (BYTE)m_acc;
			m_cnt = 0;
			m_acc = 0;
			if (m_nBytes >= MAXLEN)
				FlushBuffer();
			}
#endif
		}
}

int CMyGif::SearchTreeForCode(CTreeNode * node,LPBYTE str,int len,
					int next,int n,int done)
{
	if (len == 0)
		return done ? node->code : -1;
	if(node->node[*str])
		return SearchTreeForCode(node->node[*str],str+1,len-1,next,n,done);
	else
		{
	    node->node[*str] = AddNode(next,n);
    	return node->code;
  		}
}

UINT CMyGif::Write(UINT size)
{
	m_pFile->Write(m_pBuffer, size);
	return 0;
}

void CMyGif::FlushBuffer()
{
	if (m_nBytes)
		{
		m_pBuffer[0] = m_nBytes;
		Write(1 + m_nBytes);
		m_nBytes = 0;
		}
}
