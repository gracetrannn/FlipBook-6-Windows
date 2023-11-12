#include "CLayers.h"
#include "CSceneBase.h"
#include "CCell.h"
#include "zlib.h"
#include "CLevtbl.h"
#include "CLevel.h"
#include "math.h"
#include "CNewPals.h"
#include "CFloat.h"
#include "utils.h"
#include <vector>

#define MAGIC_COLOR 240

typedef struct {
	char Id[16];
	UINT version;
	UINT	width;
	UINT height;
	UINT nLays;
} MDLHEADER;

class CUndo
{
public:
	UINT	layer;
	BYTE *	pData;
};

class CPixels
{
public:
	BYTE * m_pPaint;
	BYTE * m_pInk;
	UINT m_width;
	UINT m_height;
	UINT m_pitch;
	UINT m_color;
	UINT m_offset;
	UINT m_size;
inline void Ink(int delta, BYTE v, BYTE a = 255)
			{m_pInk[delta+m_offset] = a;m_pInk[m_size+delta+m_offset] = v;};
inline 	bool Paint(int delta = 0)
		{return m_pPaint[m_offset+delta] && !m_pInk[m_offset+delta] &&
			(m_pPaint[m_size+m_offset+delta] == m_color) ? TRUE : FALSE;};

inline 	bool Ink(int delta = 0) {return m_pInk[m_offset+delta] ? TRUE : FALSE;};
};


class CEmbedFile
{
public:
CEmbedFile(CSceneBase * pScene)
	{
	UINT m_index = NEGONE;
	m_pScene = pScene;
	m_bOpened = 0;
	m_pEmbBuf = 0;
	m_pFile = 0;
	};
~CEmbedFile()
	{
	delete [] m_pEmbBuf;
	if (m_pFile)
		delete m_pFile;
	m_pFile = 0;
	};

bool Open(LPCSTR name, UINT kind)
	{
	m_index = m_pScene->EmbFind(name, kind);//EMB_KIND_MODEL);
	if (m_index == NEGONE)
		{
		m_pFile = new CFile;
		DWORD mode = CFile::modeReadWrite;
		if (!m_pFile->Open(name, mode))
			{
			delete m_pFile;
			m_pFile = 0;
			return 1;
			}
		}
	else
		{
		m_max = m_pScene->EmbData(m_index,0);
		m_pEmbBuf = new BYTE[m_max];
		m_cur = 0;
		m_pScene->EmbData(m_index,m_pEmbBuf);
		}
	return 0;
	};

UINT Read(void * buf, UINT size)
{
	if (m_index != NEGONE)
		{
		if ((m_cur + size) > m_max)
			size = m_max - m_cur;
		memmove(buf,m_pEmbBuf + m_cur,size);
		m_cur += size;
		}
	else
		{
		size = m_pFile->Read(buf, size);
		}
	return size;
	};
protected:
	CSceneBase * m_pScene;
	BYTE * m_pEmbBuf;
	BYTE * m_pGet;
	bool m_bOpened;
	UINT m_index;
	UINT m_cur;
	UINT m_max;
	CFile * m_pFile;
};

#define FLAG_MODIFIED 1
#define FLAG_ACTIVE 2

#define TQ(a) (a < 7 ? a - 1 : a - 2)


//#define LAY_UNDO   0
//#define LAY_SHADOW 1
//#define LAY_PAINT  2
//#define LAY_INK    3
//#define LAY_TINT   4

//#define NLAY_PAINT  6
//#define NLAY_INK    7

static int dx[8] = {-1, 0, 1, 1, 1, 0,-1,-1};
static int dy[8] = {-1,-1,-1, 0, 1, 1, 1, 0};


typedef struct {
        UINT kind;
		COLORREF c1;
		COLORREF c2;
		CPoint	p1;
		CPoint	p2;
} CINFO;

static UINT kindex = 0;
static UINT kcolor = 0;

CLayers::CLayers()
{
	m_pLayers = 0;
	m_pUndo = 0;
	m_nMaxUndo = 0;
	m_pOverlay = 0;
	m_pControl = 0;
	m_nLayers = 0;
	m_pTable = 0;
	m_width = m_height = 0;
	m_nFillMax = m_nFillCount = 0;
	m_pFillStack = 0;
}

CLayers::~CLayers()
{
	EmptyUs();
}


CLevelTable * CLayers::LevelTable(bool bPut /* = 0 */)
{
	if (bPut)
		m_pScene->LevelTable(m_level, m_pTable, 1);
	return m_pTable;		
}

//BYTE * CLayers::InitPalette()
//{
//	m_pScene->LevelTable(m_level, m_pTable, 0); // get a copy
//	return &m_pTable->pals[0];
//}

void CLayers::EmptyUs()
{
	UINT i;
	for (i = 0; i < m_nLayers; i++)
		delete [] m_pLayers[i].pData;
	delete[] m_pLayers;
	m_pLayers = 0;
	if (m_pUndo)
		{
		for (i = 0; i < m_nMaxUndo; i++)
			delete [] m_pUndo[i].pData;
		delete[] m_pUndo;
		m_pUndo = 0;
		}
	delete [] m_pOverlay;
	m_pOverlay = 0;
	delete [] m_pControl;
	m_pControl = 0;
	delete m_pTable;
	m_pTable = 0;
//	delete m_pColors;
//	m_pColors = 0;
	delete [] m_pFillStack;
	m_pFillStack = 0;
	m_nFillMax = m_nFillCount = 0;
}

#define MAX_RADIUS 80
#define MAX_MASK (1 + 2 * MAX_RADIUS) * (1 + 2 * MAX_RADIUS)

UINT CLayers::CreateLayers()
{
	m_nCurLayer = 0;
	m_pTable = new CLevelTable;
	m_pLayers = new CLayer[m_nLayers];
	m_pUndo = new CUndo[m_nMaxUndo];
//	delete m_pColors;
//	m_pColors = new CColors;
	UINT i;

	for (i = 1; i < m_nLayers; i++)
		{
		UINT size;
		if ((m_nLayers < 3) || ( i == 6) || ( i == 7))
			size = 2 * m_pitch * m_height;
		else
			size = 4 * m_pitch * m_height;
		m_pLayers[i].flags = 0;
		m_pLayers[i].pData = new BYTE[size];
		memset(m_pLayers[i].pData,0,size);
		}
	if (m_nLayers < 3)
		{
		m_nPaint = 0;
		m_nInk = 1;
		m_pLayers[m_nInk].kind = CCell::LAYER_INK;
		}
	else if (m_nLayers < 4)
		{
		m_nPaint = 1;
		m_nInk = 2;
		m_pLayers[m_nInk].kind = CCell::LAYER_INK;
		m_pLayers[m_nPaint].kind = CCell::LAYER_PAINT;
		}
	else
		{
ASSERT(m_nLayers == 13);
		m_nPaint = 6;
		m_nInk = 7;
		for (i = 0; i < 5; i++)
			m_pLayers[1 + i].kind = CCell::LAYER_MATTE0 + i;
		m_pLayers[m_nInk].kind = CCell::LAYER_INK;
		m_pLayers[m_nPaint].kind = CCell::LAYER_PAINT;
		for (i = 7; i < 12; i++)
			m_pLayers[1 + i].kind = CCell::LAYER_MATTE5 + i - 7;
		}
	m_nRedo = m_nUndo = 0;
	m_nPushCount = 0;
	m_pLayers[0].flags = 0;
	m_pLayers[0].kind = 99; // undo
	m_pLayers[0].pData = 0;

	for (i = 0; i < m_nMaxUndo; i++)
		{
		m_pUndo[i].layer = 0;
		m_pUndo[i].pData = new BYTE[4 * m_pitch * m_height];
		}
	return 0;
}

UINT CLayers::Setup(CSceneBase * pScene, bool bScene, UINT Level /* = -1 */)
{
	EmptyUs();
	m_pScene = pScene;
	if (Level != -1)
		{
		m_level = Level;
		UINT result = LoadModel(Level, TRUE);
		if (result)
			return result;
		}
	else
		{
		if (bScene)
			m_pScene->SetLayer(this);
		m_width = m_pScene->Width();
		m_height = m_pScene->Height();
		m_pitch = 4 * ((m_width + 3) / 4);
		UINT size = m_pitch * m_height;
		if (size > 1000000)
			m_nMaxUndo = (20000000 + size - 1) / size;  
		else
			m_nMaxUndo = 20;
		if (m_pScene->ColorMode() || !bScene)
			m_nLayers = 13;
		else
			m_nLayers = 2;	// just ink and undo
		CreateLayers();
		}
	return 0;
}

UINT CLayers::SelectLayer(UINT Layer)
{
	if (m_pTable->layer = Layer)
		{
		if (m_pTable->layer > 5)
			m_nCurLayer = 2 + m_pTable->layer;
		else
			m_nCurLayer = m_pTable->layer;
		}
	else
		m_nCurLayer = 0;
	return m_nCurLayer;
}

void CLayers::NewPalette()
{
	CNewPals * pPals = m_pScene->LevelPalette(m_level);
}

UINT CLayers::Select(UINT Frame, UINT Level)
{
	UINT i;
	m_frame = Frame;
	m_level = Level;
	m_pScene->LevelTable(m_level, m_pTable, 0);
//	m_pPals = m_pScene->LevelPalette(Level);
	m_maxx = m_width-1;
	m_minx = 0;
	m_maxy = m_height-1;
	m_miny = 0;
	m_bNeedFake = m_bModified = FALSE;
	m_bHasName = 0;
DPF("clayer sel:%d,%d",m_frame, m_level);
	m_cellkey = m_pScene->GetCellKey(Frame,Level);
	DWORD key;
	m_pScene->GetImageKey(key, Frame, Level, CCell::LAYER_OVERLAY);
	if (key)
		{
		if (!m_pOverlay)
			{
			UINT pitch;
			if (m_pScene->ColorMode())
				pitch = 4 * m_width;
			else
				pitch = 4 * ((m_width+3)/4);
			m_pOverlay = new BYTE[m_height*pitch];
			}
		m_pScene->FetchCell(m_pOverlay, Frame,Level,1,1);
		return 0;
		}
	delete [] m_pOverlay;
	m_pOverlay = 0;
	for (i = 1; i < m_nLayers; i++)
		{
		if (m_pLayers[i].kind >= CCell::LAYER_MATTE0)
			memset(m_pLayers[i].pData,0,4*m_height*m_pitch);
		else
			memset(m_pLayers[i].pData,0,2*m_height*m_pitch);
		if (!m_pScene->GetLayer(m_pLayers[i].pData, m_frame, m_level,
							m_pLayers[i].kind))
			m_pLayers[i].flags = FLAG_ACTIVE;
		else
			{
//			memset(m_pLayers[i].pData,0,2*m_height*m_pitch);
			m_pLayers[i].flags = 0;	// clear modified
			}
		}
	m_nRedo = m_nUndo = 0;
	m_nPushCount = 0;
	return 0;
}

void CLayers::UpdateLayer(void *data, size_t size, UINT kind) {
    for (int i = 0; i < m_nLayers; ++i) {
        if(m_pLayers[i].kind == kind) {
            memcpy(m_pLayers[i].pData, data, size);
            m_pLayers[i].flags |= (FLAG_ACTIVE | FLAG_MODIFIED);
            m_bModified = TRUE;
            break;
        }
    }
}

void CLayers::DupCell(UINT Frame, UINT Level)
{
//	Put();			// write out current
	m_frame = Frame;	// change frame
	m_level = Level;
//	Put(1);			// create new cell
	m_cellkey = m_pScene->GetCellKey(Frame,Level);
}

UINT CLayers::Put(bool bForce /* = 0 */)
{
	if (!bForce && !m_bModified)
		return 0;
	UINT layer, j;
	j = 0;
	for (layer = 1; layer < m_nLayers;layer++)
		{
		if (!(m_pLayers[layer].flags & FLAG_ACTIVE))
			continue;
		if ((m_pLayers[layer].flags & FLAG_MODIFIED) || bForce)
			{
			j = 1;
			m_pScene->PutLayer(m_pLayers[layer].pData, m_frame, m_level,
							m_pLayers[layer].kind);
			m_pLayers[layer].flags = FLAG_ACTIVE;	// clear modified
			}
		}
	if (m_bHasName)
		{
		m_pScene->CellName(m_cell_name,m_frame, m_level,TRUE);
		}
	m_bNeedFake = m_bModified = FALSE;
	return j;
}


LPCSTR CLayers::LayerName(int layer)
{
	return (LPCSTR)m_pTable->table[layer].name;
}

UINT CLayers::DisPaint(UINT x, UINT y)
{
	if (x >= m_width || y >= m_height)
		return 0;
	y = m_height - 1 - y;
	UINT z = y * m_pitch + x;
	if (!(m_pLayers[m_nPaint].flags & FLAG_ACTIVE))
		return 0;
	if (m_pLayers[m_nPaint].pData[z])
		return m_pLayers[m_nPaint].pData[m_pitch * m_height + z];
	return 0;
}

UINT CLayers::HaveAlpha(UINT x, UINT y, UINT * pLayer , BYTE * p)
{
	UINT layer;
	if (x >= m_width || y >= m_height)
		return 0;
	y = m_height - 1 - y;
	UINT z = y * m_pitch + x;
#ifdef MYBUG
	if (p)
	{
	UINT c = 4 * y * ((3*m_width + 3) / 4);
	UINT q = z + m_pitch * m_height;
//DPF("x:%4d,y:%4d,i:(%3d:%3d),p:(%3d:%3d),t:(%3d:%3d),%3d,%3d,%3d",x,y,
//		m_pLayers[LAY_INK].pData[z], m_pLayers[LAY_INK].pData[q],
//		m_pLayers[LAY_PAINT].pData[z], m_pLayers[LAY_PAINT].pData[q],
//		m_pLayers[LAY_TINT].pData[z], m_pLayers[LAY_TINT].pData[q],
//		p[c+3*x+0],p[c+3*x+1],p[c+3*x+2]);
	}
#endif
/*	
	for (layer = 1; layer < m_nLayers; layer++)
		{
		if (m_pLayers[layer].pData[z])
			break;
		}
	return layer >= m_nLayers ? 0 : 1;
*/
	for (layer = m_nLayers; layer-- > 1;)
		{
		if (!(m_pLayers[layer].flags & FLAG_ACTIVE))
			continue;
		UINT xx = x;
		UINT yy = y;
		UINT qq = TQ(layer);
		if ((layer != m_nPaint) && (layer != m_nInk))
			{
//			UINT qq = TQ(layer);
			if (!(m_pTable->table[qq].flags & 0x200)) // active
				continue;
			xx -= m_pTable->table[qq].dx;
			yy -= m_pTable->table[qq].dy;
			}
		if (xx >= m_width || yy >= m_height)
			continue;
		UINT z = yy * m_pitch + xx;
		if (m_pLayers[layer].pData[z])
			{
			if (pLayer)
				*pLayer = qq;
//			index = m_pLayers[layer].pData[m_pitch * m_height + z];
			break;
			}
		}
	return layer ? 1 : 0;
}

UINT CLayers::GetIndex(UINT x, UINT y)
{
	UINT layer;
	UINT index = -1;
	y = m_height - 1 - y;
//	UINT z = y * m_pitch + x;
//	for (layer = 1; layer < m_nLayers;layer++)
	for (layer = m_nLayers; layer-- > 1;)
		{
		if (!(m_pLayers[layer].flags & FLAG_ACTIVE))
			continue;
		UINT xx = x;
		UINT yy = y;
		if ((layer != m_nPaint) && (layer != m_nInk))
			{
			UINT qq = TQ(layer);
			if (!(m_pTable->table[qq].flags & 0x200)) // active
				continue;
			xx -= m_pTable->table[qq].dx;
			yy -= m_pTable->table[qq].dy;
			}
		if (xx >= m_width || yy >= m_height)
			continue;
		UINT z = yy * m_pitch + xx;
		if (m_pLayers[layer].pData[z])
			{
			index = m_pLayers[layer].pData[m_pitch * m_height + z];
			break;
			}
		}
	return index;
}

UINT CLayers::GetDotAlpha(UINT xx, UINT yy)
{
	if (xx >= m_width || yy >= m_height)
		return 0; // so it is xparent
	yy = m_height - 1 - yy;
	UINT z = yy * m_pitch + xx;
	if (m_bSolid)
		return m_pLayers[m_nCurLayer].pData[z];
	BYTE * pAlpha = m_pLayers[m_nCurLayer].pData;
	BYTE * pIndex = pAlpha + m_pitch * m_height;
	if (pIndex[z] != m_color) return 0;
	return m_pLayers[m_nCurLayer].pData[z];
}

void CLayers::PutDotAlpha(UINT xx, UINT yy, BYTE v)
{
	if (xx >= m_width || yy >= m_height)
		return;
	yy = m_height - 1 - yy;
	UINT z = yy * m_pitch + xx;
	m_pLayers[m_nCurLayer].pData[z] = v;
}

UINT CLayers::GetAlpha(UINT x, UINT y)
{
	UINT layer;
	UINT index = -1;
	y = m_height - 1 - y;
//	UINT z = y * m_pitch + x;
//	for (layer = 1; layer < m_nLayers;layer++)
	for (layer = m_nLayers; layer-- > 1;)
		{
		if (!(m_pLayers[layer].flags & FLAG_ACTIVE))
			continue;
		UINT xx = x;
		UINT yy = y;
		if ((layer != m_nPaint) && (layer != m_nInk))
			{
			UINT qq = TQ(layer);
			if (!(m_pTable->table[qq].flags & 0x200)) // active
				continue;
			xx -= m_pTable->table[qq].dx;
			yy -= m_pTable->table[qq].dy;
			}
		if (xx >= m_width || yy >= m_height)
			continue;
		UINT z = yy * m_pitch + xx;
		if (m_pLayers[layer].pData[z])
			{
			index = m_pLayers[layer].pData[z];
			break;
			}
		}
	return index;
}

UINT CLayers::GetRGB(int x, int y, UINT &r,UINT &g,UINT &b)
{
	UINT layer;
	if ((UINT)x >= m_width || (UINT)y >= m_height)
		return 0;
	y = m_height - 1 - y;
	UINT z = y * m_pitch + x;
#ifdef QMYBUG
	if (p)
	{
	UINT c = 4 * y * ((3*m_width + 3) / 4);
	UINT q = z + m_pitch * m_height;
//DPF("x:%4d,y:%4d,i:(%3d:%3d),p:(%3d:%3d),t:(%3d:%3d),%3d,%3d,%3d",x,y,
//		m_pLayers[LAY_INK].pData[z], m_pLayers[LAY_INK].pData[q],
//		m_pLayers[LAY_PAINT].pData[z], m_pLayers[LAY_PAINT].pData[q],
//		m_pLayers[LAY_TINT].pData[z], m_pLayers[LAY_TINT].pData[q],
//		p[c+3*x+0],p[c+3*x+1],p[c+3*x+2]);
	}
#endif
/*	
	for (layer = 1; layer < m_nLayers; layer++)
		{
		if (m_pLayers[layer].pData[z])
			break;
		}
	return layer >= m_nLayers ? 0 : 1;
*/
	for (layer = m_nLayers; layer-- > 1;)
		{
		if (!(m_pLayers[layer].flags & FLAG_ACTIVE))
			continue;
		UINT xx = x;
		UINT yy = y;
		UINT qq = TQ(layer);
		if ((layer != m_nPaint) && (layer != m_nInk))
			{
//			UINT qq = TQ(layer);
			if (!(m_pTable->table[qq].flags & 0x200)) // active
				continue;
			xx -= m_pTable->table[qq].dx;
			yy -= m_pTable->table[qq].dy;
			}
		if (xx >= m_width || yy >= m_height)
			continue;
		UINT z = yy * m_pitch + xx;
		if (m_pLayers[layer].pData[z])
			{
//			index = m_pLayers[layer].pData[m_pitch * m_height + z];
			break;
			}
		}
	if (layer)
		{
		int index = m_pLayers[layer].pData[m_pitch * m_height + z];
		CNewPals * pPals = m_pScene->LevelPalette(m_level);
		r = pPals->Red(index);
		g = pPals->Green(index);
		b = pPals->Blue(index);
		}
	return layer ? 1 : 0;
}

void Blur(BYTE * pDst, BYTE * pSrc, UINT w, UINT h, UINT r, UINT f, UINT pitch);

void CLayers::ApplyLayer(BYTE *pDst, BYTE * pSrc, BYTE * pPaint, BYTE * pInk,
				 UINT kind, UINT qq, CNewPals * pPals)
{
	BYTE * pAlpha = pSrc;
	UINT dx = 0;
	UINT dy = 0;
	if (kind >= CCell::LAYER_MATTE0)
		{
		pAlpha +=  2 * m_pitch * m_height;
		int r = m_pTable->table[qq].blur;
		int c = m_pTable->table[qq].color;
		int ff = r / 256;
		r &= 255;
		memmove(pAlpha,pSrc,2 * m_pitch*m_height);
		if (r)
			Blur(pAlpha,pSrc, m_width, m_height, r, ff, m_pitch);
		dx = m_pTable->table[qq].dx;
		dy = m_pTable->table[qq].dy;
		}
	BYTE * pIndex = pAlpha + m_pitch * m_height;
	UINT y,x;
	UINT op = 4 * m_width;
	for (y = 0; y < m_height; y++)
		{
		UINT sy = y + dy;
		if ((UINT)sy >= m_height)
			continue;
		for (x = 0; x < m_width ; x++)
			{
			UINT sx = x - dx;
			if (sx >= m_width)
				continue;
			UINT z,i;
			z = pAlpha[m_pitch*sy+sx];
			if (z && (m_pTable->table[qq].flags & 1) && !pPaint[y*m_pitch+x])
				z = pInk[m_pitch*y+x];
			if (!z)
				continue;
			UINT q = pDst[op*y+4*x+3];
			i = pIndex[m_pitch*sy+sx];
			BYTE color[4];
			pPals->Color(color,i,sx,sy);
			z =	z * color[3] / 255;
			UINT j;
			if ((z == 255) || !q)
				{
				pDst[op*y+4*x+3] = z;
				for (j = 0; j < 3; j++)
					pDst[op*y+4*x+j] = color[2-j];
				}
			else if (z)
				{
				WORD v = (255 - z) * q;
				v += z * 255;//pPals[4*i+3];
				pDst[op*y+4*x+3] = v / 255;
				for (j = 0; j < 3; j++)
					{
					WORD v = (255 - z) * pDst[op*y+4*x+j];
					v += z * color[2-j];
					pDst[op*y+4*x+j] = v / 255;
					}
				}
			}
		}

}
void CLayers::ApplyCell(BYTE * pDst, CSceneBase * pScene, 
				CNewPals * pPals, CLevelTable * pTable, CCell * pCell)
{
	m_width = pScene->Width();
	m_height = pScene->Height();
	m_pitch = 4 * ((m_width + 3) / 4);
	if (!pScene->ColorMode())
		{
		BYTE * pAlpha = new BYTE[2 * m_pitch * m_height];
		DWORD key = (pCell)->Select(CCell::LAYER_INK);
		if (key > 1)
			{
			pScene->ReadImage(pAlpha, key);
			memmove(pDst, pAlpha, m_pitch * m_height);
			}
		if (!key)
			{
			if (key = (pCell)->Select(CCell::LAYER_GRAY))
				{
				pScene->ReadImage(pAlpha, key);
				for (UINT i = 0; i < m_width * m_height; i++)
					pDst[i] = pAlpha[i] ^ 255;
				}
			}
//		delete pCell;
		delete [] pAlpha;
		return;
		}
	bool bActive = 0;
	CLevelTable * pSave = m_pTable; 
	m_pTable = pTable;
	if ((pTable->table[0].name[0] == 0 ) &&
				(pTable->table[0].name[1] == (char)255))
		bActive = TRUE;
	BYTE * hpTmp = new BYTE[8 * m_pitch * m_height];
	UINT i;
	BYTE * pPaint = hpTmp + 4 * m_pitch * m_height;
	BYTE * pInk = hpTmp + 6 * m_pitch * m_height;
	DWORD key = pCell->Select(CCell::LAYER_PAINT);
	if (key > 1)
		pScene->ReadImage(pPaint, key);
	else
		memset(pPaint,0, m_pitch * m_height);
	key = pCell->Select(CCell::LAYER_INK);
	if (key > 1)
		pScene->ReadImage(pInk, key);
	else
		memset(pInk,0, m_pitch * m_height);
	UINT f = 1;
	for (i = 0; i < 12; i++)
		{
		UINT kind, qq;
		qq = TQ(i+1);
		if (!bActive && !(pTable->table[qq].flags & 0x100)) // active
			continue;
		if (i < 5) 
			kind = CCell::LAYER_MATTE0 + i;
		else if (i == 5)
			kind = CCell::LAYER_PAINT;
		else if (i == 6)
			kind = CCell::LAYER_INK;
		else
			kind = CCell::LAYER_MATTE5 + i - 7;
		if (kind == CCell::LAYER_PAINT)
			memmove(hpTmp, pPaint, 2 * m_pitch * m_height);
		else if (kind == CCell::LAYER_INK)
			memmove(hpTmp, pInk, 2 * m_pitch * m_height);
		else
			{
			DWORD key = pCell->Select(kind);
			if (key > 1)
				pScene->ReadImage(hpTmp, key);
			else if (!key && (i == 6))
				{
				key = pCell->Select(CCell::LAYER_GRAY);
				if (!key) continue;
				pScene->ReadImage(hpTmp, key);
				UINT s = m_pitch * m_height;
				UINT z;
				for (z = 0; z < s; hpTmp[z++] ^= 255);
				memset(hpTmp+s,0,s);
				}
			else
				continue;
			}
		ApplyLayer(pDst, hpTmp, pPaint,pInk, kind, qq,pPals);
		f = 0;
		}
	m_pTable = pSave;
//	delete pCell;
	delete [] hpTmp;
DPZ("clayers,apply cell:%d",f);
	return;
}

bool CLayers::NeedFake(UINT Frame) 
{
	bool bResult = 0;
	if (m_frame == Frame)
		bResult = m_bNeedFake;
	else if (Frame > m_frame)
		{
		UINT Stop = m_frame;
		if (!m_pScene->FindNextCell(Stop, m_level))
			Stop = m_pScene->FrameCount();
		if (Frame < Stop)
			bResult = m_bNeedFake;
		}
//	m_bNeedFake = 0;
	return bResult;
}

bool CLayers::GetFakeFrame(UINT &FakeFrame, UINT Level)
{
	if (m_level != Level)
		return TRUE;
	if (!m_bNeedFake)
		return TRUE;
	FakeFrame = m_frame;
	return 0;
}


bool CLayers::UseFake(UINT cellkey)
{
	bool bResult = 0;
	if (m_cellkey == cellkey)
		{
		bResult = m_bNeedFake;
		m_bNeedFake = 0;
		}
	return bResult;
}

bool CLayers::FakeIt(BYTE * pDst)
{
	UINT ox,oy,op;
	if (m_pScene->ColorMode())
		{
		op = 4 * m_width;
		memset(pDst, 255, m_height * op);
		for (oy = 0; oy < m_height; oy++)
		for (ox = 0; ox < m_width ; ox++)
			pDst[op*oy+4*ox+3] = 0;
		}
	else
		{
		op = m_pitch;
		memset(pDst, 0, m_height * m_pitch);
		}
	if (m_pOverlay)
		{
return 0;
		for (int yy = m_miny; yy <= m_maxy;yy++)
			{
			int y = m_height - 1 - yy;
			for (int x = m_minx; x <= m_maxx; x++)
				{
				WORD z;		
				z = m_pOverlay[4*(y*m_width+x)+3];
				if (!z)
					continue;
				for (int j = 0; j < 3; j++)
					{
					WORD v = (255 - z) * pDst[op*y+4*x+j];
					v += z * m_pOverlay[4*(y*m_width+x)+j];
					pDst[op*y+4*x+j] = v / 255;
					}
				}
			}
		return 0;
		}
	if (m_pScene->ColorMode())
		{
		BYTE * pPaint = m_pLayers[m_nPaint].pData;
		BYTE * pInk = m_pLayers[m_nInk].pData;
		CNewPals * pPals = m_pScene->LevelPalette(m_level);
		UINT layer;
		for (layer = 1; layer < m_nLayers;layer++)
			{
			UINT kind, qq;
			if (!(m_pLayers[layer].flags & FLAG_ACTIVE))
				continue;
			qq = TQ(layer);
			if (!(m_pTable->table[qq].flags & 0x100)) // active
				continue;
			kind = m_pLayers[layer].kind;
			BYTE * pSrc = m_pLayers[layer].pData;
			ApplyLayer(pDst, pSrc, pPaint,pInk, kind, qq, pPals);
			}
		}
	else
		{
		BYTE * pAlpha = m_pLayers[1].pData;
		for (UINT i = 0; i < m_width * m_height; i++)
				pDst[i] = pAlpha[i];
		}
	return 0;
}

void CLayers::Update(BYTE * pBits, BYTE * pBG, UINT pitch, bool bAll /* = 0 */)
{
	if (bAll)
		{
		m_minx = m_miny = 0;
		m_maxx = m_width - 1;
		m_maxy = m_height - 1;
		}
//	if (m_bDirty && (m_pLayers[m_nCurLayer].kind >= CCell::LAYER_MATTE0))
//		{
//		BlurIt();
//		}
	if (m_nLayers < 3)
		{
		UpdateGray(pBits, pBG, pitch);
		}
	else
		{
		int x, y,yy;
		if (pBG)
			{
			for (yy = m_miny; yy <= m_maxy;yy++)
				{
				y = m_height - 1 - yy;
				for (x = m_minx; x <= m_maxx; x++)
					{
					pBits[pitch*y+3*x+0] = pBG[pitch*y+3*x+0];
					pBits[pitch*y+3*x+1] = pBG[pitch*y+3*x+1];
					pBits[pitch*y+3*x+2] = pBG[pitch*y+3*x+2];
					}
				}
			}
		if (!(m_nFlags & 1))
			UpdateInk(pBits,pitch);
		else
			UpdateColor(pBits,pitch);
		}
//	m_maxx = m_maxy = 0;
//	m_minx = m_width - 1;
//	m_miny = m_height - 1;
	m_bDirty = FALSE;
}

bool CLayers::GetPointerToLayerImage(BYTE*& pData, UINT kind) {
    for(int i = 0; i < m_nLayers; ++i) {
        if(m_pLayers[i].kind == kind) {
            pData = m_pLayers[i].pData;
            return true;
        }
    }
    return false;
}

void CLayers::GetLayerImage(BYTE* pBits, UINT kind, bool premultipliedAlpha, bool swapRedBlue) {
    uint8_t* pAlpha = nullptr;
    for(int i = 0; i < m_nLayers; ++i) {
        if(m_pLayers[i].kind == kind) {
            pAlpha = m_pLayers[i].pData;
            break;
        }
    }
    
    if(!pAlpha)
        return;
    
    uint8_t* pImage = pAlpha + m_width*m_height;
    const int bpp = 4;
    const int stride = m_width*bpp;
    
    CNewPals * pPals = m_pScene->LevelPalette(m_level);
    
    BYTE color[4];
    for(int y = m_maxy; y >= m_miny; --y) {
        for(int x = m_minx; x < m_maxx; ++x) {
            int index = (m_maxy - y)*m_width + x;
            pPals->Color(color, pImage[index], x, y);
            uint8_t alpha = pAlpha[index];
            alpha = (alpha*color[3])/255;
            
            // RGB -> BGR
            if(swapRedBlue) {
                uint8_t red = color[0];
                uint8_t blue = color[2];
                color[0] = blue;
                color[2] = red;
            }
            
            if(premultipliedAlpha) {
                color[0] = (color[0]*alpha)/255;
                color[1] = (color[1]*alpha)/255;
                color[2] = (color[2]*alpha)/255;
            }
            
            color[3] = alpha;
            if(alpha > 0) { // copy only non-transparent pixels
                memcpy(pBits + y*stride + x*bpp, color, 4);
            }
        }
    }
}

void CLayers::GetImage(BYTE* pBits, UINT pitch) {    
    UINT tbl[12] = {0};
    UINT flags[12];
    UINT cnt;
    UINT i;
    const int bpp = 4; // bytes per pixel in pBits
    
    for (i = 1, cnt = 0;i < m_nLayers;i++)
        {
        UINT j = TQ(i);
        if ((m_pLayers[i].flags & FLAG_ACTIVE) &&
                (m_pTable->table[j].flags & LevelLayerFlags::DISPLAYABLE)) // displayable
            {
            flags[cnt] = m_pTable->table[j].flags;
            tbl[cnt++] = i;
            }
        }
//for (i = 0; i < cnt; i++)
//    {
//    DPF("i:%d,tbl:%d,%x",i,tbl[i],flags[i]);
//    }
    if (m_pOverlay)
        {
        for (int yy = m_miny; yy <= m_maxy;yy++)
            {
            int y = m_height - 1 - yy;
            for (int x = m_minx; x <= m_maxx; x++)
                {
                WORD z;
                z = m_pOverlay[4*(y*m_width+x)+3];
                if (!z)
                    continue;
                    
                    uint8_t curOpacity = pBits[pitch*y + bpp*x + (bpp - 1)];
                    for (int j = 0; j < 3; j++) {
                        WORD v = (255 - z) * pBits[pitch*y+bpp*x+j];
                        v += z * m_pOverlay[4*(y*m_width+x)+j];
                        pBits[pitch*y+bpp*x+j] = v / 255;
                    }
                    pBits[pitch*y + bpp*x + (bpp - 1)] = z + (255 - z)*curOpacity/255;
                }
            }
        return;
        }
    BYTE * pPaint = m_pLayers[m_nPaint].pData;
    BYTE * pInk = m_pLayers[m_nInk].pData;
    
    CNewPals * pPals = m_pScene->LevelPalette(m_level);
    for (i = 0; i < cnt; i++)
        {
        UINT dx = 0;
        UINT dy = 0;
        UINT l = tbl[i];
DPF("i:%d,l:%d,k:%d",i,l,m_pLayers[l].kind);
        BYTE * pAlpha = m_pLayers[l].pData;
        if (m_pLayers[l].kind >= CCell::LAYER_MATTE0)
            {
            BlurIt(l);
            pAlpha += 2 * m_pitch * m_height;
            dx = m_pTable->table[TQ(l)].dx;
            dy = m_pTable->table[TQ(l)].dy;
            }
        BYTE * pIndex = pAlpha + m_pitch * m_height;
        UINT csize = 2 * m_height * m_pitch;
        for (int yy = m_miny; yy <= m_maxy;yy++)
            {
            int y = m_height - 1 - yy;
            UINT sy = y + dy;
            if ((UINT)sy >= m_height)
                continue;
            for (int x = m_minx; x <= m_maxx; x++)
                {
                uint8_t curOpacity = pBits[pitch*y + bpp*x + (bpp - 1)];
                UINT sx = x - dx;
                if (sx >= m_width)
                    continue;
                WORD z,p;
                z = pAlpha[sy*m_pitch+sx];
                if (z && (flags[i] & LevelLayerFlags::COMPOSITE_WITHIN) && !pPaint[y*m_pitch+x])
                        z = pInk[y*m_pitch+x];
                if (!z)
                    {
                    if (((cnt>1)&&(m_pLayers[l].kind != CCell::LAYER_INK))
                                || !m_pControl || (m_nFlags & 2))
                        continue;
                    z = m_pControl[csize +sy*m_pitch+sx];
                    if (!z)
                        continue;
                    int j;
                    BYTE q[3];
                    q[0] = 190;
                    q[1] = 190;
                    q[2] = 255;
                    for (j = 0; j < 3; j++)
                        {
                        WORD v = (255 - z) * pBits[pitch*y+bpp*x+j];
                        v += z * q[j];
                        pBits[pitch*y+bpp*x+j] = v / 255;
                        }
                        pBits[pitch*y + bpp*x + (bpp - 1)] = z + (255 - z)*curOpacity/255;
                    continue;
                    }
                BYTE color[4];
                p = pIndex[sy*m_pitch+sx];
                pPals->Color(color,p,sx,sy);
                z = (z * color[3]) / 255;
                int j;
                for (j = 0; j < 3; j++)
                    {
                    WORD v = (255 - z) * pBits[pitch*y+bpp*x+j];
                    v += z * color[j];
                    pBits[pitch*y+bpp*x+j] = v / 255;
                    }
                    pBits[pitch*y + bpp*x + (bpp - 1)] = z + (255 - z)*curOpacity/255;
                }
            }
        }
}

void CLayers::UpdateColor(BYTE * pBits, UINT pitch)
{
    UINT tbl[12] = {0};
	UINT flags[12];
	UINT cnt;
	UINT i;
	for (i = 1, cnt = 0;i < m_nLayers;i++)
		{
		UINT j = TQ(i);
		if ((m_pLayers[i].flags & FLAG_ACTIVE) &&
				(m_pTable->table[j].flags & LevelLayerFlags::DISPLAYABLE)) // displayable
			{
			flags[cnt] = m_pTable->table[j].flags;
			tbl[cnt++] = i;
			}
		}
//for (i = 0; i < cnt; i++)
//	{
//	DPF("i:%d,tbl:%d,%x",i,tbl[i],flags[i]);
//	}
	if (m_pOverlay)
		{
		for (int yy = m_miny; yy <= m_maxy;yy++)
			{
			int y = m_height - 1 - yy;
			for (int x = m_minx; x <= m_maxx; x++)
				{
				WORD z;		
				z = m_pOverlay[4*(y*m_width+x)+3];
				if (!z)
					continue;
				for (int j = 0; j < 3; j++)
					{
					WORD v = (255 - z) * pBits[pitch*y+3*x+j];
					v += z * m_pOverlay[4*(y*m_width+x)+j];
					pBits[pitch*y+3*x+j] = v / 255;
					}
				}
			}
		return;
		}
	BYTE * pPaint = m_pLayers[m_nPaint].pData;
	BYTE * pInk = m_pLayers[m_nInk].pData;
	CNewPals * pPals = m_pScene->LevelPalette(m_level);
	for (i = 0; i < cnt; i++)
		{
		UINT dx = 0;
		UINT dy = 0;
		UINT l = tbl[i];
DPF("i:%d,l:%d,k:%d",i,l,m_pLayers[l].kind);
		BYTE * pAlpha = m_pLayers[l].pData;
		if (m_pLayers[l].kind >= CCell::LAYER_MATTE0)
			{
			BlurIt(l);
			pAlpha += 2 * m_pitch * m_height;
			dx = m_pTable->table[TQ(l)].dx;
			dy = m_pTable->table[TQ(l)].dy;
			}
		BYTE * pIndex = pAlpha + m_pitch * m_height;
		UINT csize = 2 * m_height * m_pitch;
		for (int yy = m_miny; yy <= m_maxy;yy++)
			{
			int y = m_height - 1 - yy;
			UINT sy = y + dy;
			if ((UINT)sy >= m_height)
				continue;
			for (int x = m_minx; x <= m_maxx; x++)
				{
				UINT sx = x - dx;
				if (sx >= m_width)
					continue;
				WORD z,p;
				z = pAlpha[sy*m_pitch+sx];
				if (z && (flags[i] & LevelLayerFlags::COMPOSITE_WITHIN) && !pPaint[y*m_pitch+x])
						z = pInk[y*m_pitch+x];
				if (!z)
					{
					if (((cnt>1)&&(m_pLayers[l].kind != CCell::LAYER_INK)) 
								|| !m_pControl || (m_nFlags & 2))
						continue;
					z = m_pControl[csize +sy*m_pitch+sx];
					if (!z)
						continue;
					int j;
					BYTE q[3];
					q[0] = 190;
					q[1] = 190;
					q[2] = 255;
					for (j = 0; j < 3; j++)
						{
						WORD v = (255 - z) * pBits[pitch*y+3*x+j];
						v += z * q[j];
						pBits[pitch*y+3*x+j] = v / 255;
						}
					continue;
					}
				BYTE color[4];
				p = pIndex[sy*m_pitch+sx];
				pPals->Color(color,p,sx,sy);
				z = (z * color[3]) / 255;
				int j;
				for (j = 0; j < 3; j++)
					{
					WORD v = (255 - z) * pBits[pitch*y+3*x+j];
					v += z * color[2-j];
					pBits[pitch*y+3*x+j] = v / 255;
					}
				}
			}
		}
}

void CLayers::UpdateInk(BYTE * pBits, UINT pitch)
{
//	BYTE * pPalette = &m_pTable->pals[0];
	BYTE * pInk = m_pLayers[m_nInk].pData;
	BYTE * pIndex = pInk + m_pitch * m_height;
	CNewPals * pPals = m_pScene->LevelPalette(m_level);
	UINT csize = 2 * m_height * m_pitch;
	for (int yy = m_miny; yy <= m_maxy;yy++)
		{
		int y = m_height - 1 - yy;
		UINT sy = y;
		if ((UINT)sy >= m_height)
			continue;
		for (int x = m_minx; x <= m_maxx; x++)
			{
			UINT sx = x;
			if (sx >= m_width)
				continue;
			WORD z,p;
			z = pInk[sy*m_pitch+sx];
			if (!z)
				continue;
			p = pIndex[sy*m_pitch+sx];
			BYTE color[4];
			pPals->Color(color,p,sx,sy);
			z = (z * color[3]) / 255;
			int j;
			for (j = 0; j < 3; j++)
				{
				WORD v = (255 - z) * pBits[pitch*y+3*x+j];
				v += z * color[2-j];
				pBits[pitch*y+3*x+j] = v / 255;
				}
			}
		}
}


//#define ZQ
void CLayers::UpdateGray(BYTE * pBits, BYTE * pSkin, UINT pitch)
{	
	int x,y,yy;
	BYTE * pSrc;
	if (m_pOverlay)
		pSrc = m_pOverlay;
	else
		pSrc = m_pLayers[1].pData;
	for (yy = m_miny; yy <= m_maxy;yy++)
		{
		y = m_height - 1 - yy;
		for (x = m_minx; x <= m_maxx; x++)
			{
			UINT v1 = pSkin[m_pitch*y+x];
			UINT v2 = pSrc[m_pitch*y+x];
			if (v2 == 255)
				v1 = 0;
			else if (v2)
				v1 = (v1*(255-v2)) / 255;
			pBits[pitch*y+x] = v1;
			}
		}
}


int CLayers::CDespeckle(int count)
{
	DPF("despeckle,%d",count);
	UINT size = 2 * m_height * m_pitch;
	BYTE * pInk = m_pLayers[m_nInk].pData;
	BYTE * pUndo= PushUndo(m_nInk,1);
	int dx[300];
	int dy[300];
	m_minx = m_width;
	m_maxx = 0;
	m_miny = m_height;
	m_maxy = 0;
	int x, y,c;
	int r,i,j;
	int xx,yy;
	c = 0;
	r = count;
	for (i = -r; i <= r; i++)
		{
		dx[c] = i;
		dy[c++] = -r;
		dx[c] = i;
		dy[c++] = r;
		}
	for (i = 1-r; i < r; i++)
		{
		dx[c] = r;
		dy[c++] = i;
		dx[c] = -r;
		dy[c++] = i;
		}
//	for (x = 0; x < c; x++)
//		{
//DPF("%d, %d,%d",x,dx[x],dy[x]);
//		}
	memmove(pUndo,pInk,size);
	for (y = 0; y < (int)m_height; y++)
	for (x = 0; x < (int)m_width; x++)
		{
		if (!pUndo[y * m_pitch + x])
			continue;
		for (i = 0; i < (int)c; i++)
			{
			xx = x + dx[i];
			yy = y + dy[i];
			if (((UINT)xx < m_width) &&
					((UINT)yy < m_height) &&
					pUndo[yy * m_pitch + xx])
				break;
			}
		if (i < (int)c)
			continue;
		for (i = 1-r; i< r;i++)
		for (j = 1-r; j< r;j++)
			{
			xx = x + i;
			yy = y + j;
			if (((UINT)(xx) < m_width) &&
					((UINT)(yy) < m_height))
				{
				pInk[yy * m_pitch + xx] = 0;
				if (xx < m_minx) m_minx = xx;
				if (xx > m_maxx) m_maxx = xx;
				yy = m_height - 1 - yy;
				if (yy < m_miny) m_miny = yy;
				if (yy > m_maxy) m_maxy = yy;
				}
			}
		}
	if ((UINT)m_minx < m_width)
		{
		m_bNeedFake = m_bModified = TRUE; 
		m_bDirty = TRUE;
		m_pLayers[m_nInk].flags = FLAG_ACTIVE | FLAG_MODIFIED;
		}
	return m_bDirty;
}

void CLayers::DrawInit(UINT color, UINT type, bool bErasing)
{
	FillStack(-2);
	UINT vtype = type;
	m_color = color;
	m_bErasing = bErasing;
	m_dot_type = type;	// 0 is pencil, 1 is trace, 2 is brush
	m_bFirstDot = TRUE;
	m_bDirty = FALSE;
	m_bSolid = FALSE;
	if (m_nLayers < 3)
		m_nCurLayer = 1;
	else if (type == 3)
		{
		m_bSolid = 1;
		m_nCurLayer = m_nInk; 
		}
	else if (type == 2)
		{
		m_nCurLayer = m_nPaint;
		}
	else if (type == 4)
		{
		m_bSolid = TRUE;
		type = 2;
		m_nCurLayer = m_nPaint;
		}
	else if (type == 6)
		{
		m_bSolid = 1;
		m_nCurLayer = m_nInk;
		}
	else if (type == 1)
		{
		m_bSolid = 1;
		m_nCurLayer = m_nInk;
		}
	else
		m_nCurLayer = m_nInk;
	if (m_pTable->layer)
		{
		if (m_pTable->layer > 5)
			m_nCurLayer = 2 + m_pTable->layer;
		else
			m_nCurLayer = m_pTable->layer;
		UINT j = TQ(m_nCurLayer);
		m_pTable->table[j].color = m_color;
if (type && (vtype != 4))
		m_bSolid = TRUE;
		}
	UINT size = m_height * 2 * m_pitch;
	PushUndo(m_nCurLayer);
	m_pLayers[m_nCurLayer].flags = FLAG_ACTIVE | FLAG_MODIFIED;	// set modified
	m_bNeedFake = m_bModified = TRUE;
}

void CLayers::DrawDot(int x, int y, BYTE alpha)
{
//DPF("draw dot,x:%d,y:%d",x,y);
	if ((UINT)x >= m_width)
		return;
	if ((UINT)y >= m_height)
		return;
//	alpha /= 4;
//	m_bDirty = TRUE;
	if (m_bFirstDot)
		{
		m_minx = x;
		m_maxx = x;
		m_miny = y;
		m_maxy = y;
		m_bFirstDot = FALSE;
		}
	else
		{
		if (x > m_maxx)
			m_maxx = x;
		else if (x < m_minx)
			m_minx = x;
		if (y > m_maxy)
			m_maxy = y;
		else if (y < m_miny)
			m_miny = y;
		}
	if (m_pLayers[m_nCurLayer].kind >= CCell::LAYER_MATTE0)
		{
		x -= m_pTable->table[TQ(m_nCurLayer)].dx;
		y -= m_pTable->table[TQ(m_nCurLayer)].dy;
		if ((UINT)x >= m_width)
			return;
		if ((UINT)y >= m_height)
			return;
		}
	BYTE * pAlpha = m_pLayers[m_nCurLayer].pData;
	BYTE * pIndex = pAlpha + m_pitch * m_height;
	UINT yyy = m_height - 1 - y;
	if (m_dot_type == 1) // trace
		{
		if (m_pLayers[m_nCurLayer].kind >= CCell::LAYER_MATTE0)
			{
			BYTE * pPAlpha = m_pLayers[m_nPaint].pData;
//			BYTE * pPIndex = pAlpha + m_pitch * m_height;
			if (pPAlpha[m_pitch*yyy+x])
				{
				if (m_bErasing)
					pAlpha[m_pitch*yyy+x] = 0;
				else
					{
					pAlpha[m_pitch*yyy+x] = 255;//pPAlpha[m_pitch*yyy+x];
					pIndex[m_pitch*yyy+x] = m_color;//pPIndex[m_pitch*yyy+x];
					}
				m_bDirty = TRUE;
				}
			}
		else if (pAlpha[m_pitch*yyy+x])
			{
			if (m_bErasing)
				pIndex[m_pitch*yyy+x] = 0;
			else
				pIndex[m_pitch*yyy+x] = m_color;
			m_bDirty = TRUE;
			}
		}
	else if (m_dot_type == 3) // untrace
		{
		if (!pAlpha[m_pitch*yyy+x])
			{
//			if (m_bErasing)
//				pIndex[m_pitch*yyy+x] = 0;
//			else
				pIndex[m_pitch*yyy+x] = m_color;
			pAlpha[m_pitch*yyy+x] = alpha;
			m_bDirty = TRUE;
			}
		}
	else
		{
		UINT v;
		if (m_bSolid)
			v = m_bErasing ? 0 : 255;
		else if (pIndex[m_pitch*yyy+x] != m_color)
			v = alpha;
		else if (m_bErasing)
			{
			v = pAlpha[m_pitch*yyy+x];
			v = v - (v * alpha) / 255;
			if (v < 20) v = 0;
			}
		else
			{
			v = pAlpha[m_pitch*yyy+x];
			v = v + alpha - (v * alpha) / 255;
			}
		pAlpha[m_pitch*yyy+x] = v;
//		if (!m_bErasing)
			pIndex[m_pitch*yyy+x] = m_color;
		m_bDirty = TRUE;
		}
}


UINT CLayers::Flood(UINT index, int kind)
{
	UINT layer;
	if (!kind)
		{
		if (!(layer = m_pTable->layer))
			{
			if (!m_pScene->ColorMode())
				layer = 1;
			else
				layer = m_nPaint;
			}
		else if (layer > 5)
			layer += 2;
		m_bDirty = TRUE;	// needs display update
		m_pLayers[layer].flags = FLAG_ACTIVE | FLAG_MODIFIED;
		BYTE * pAlpha = m_pLayers[layer].pData;
		UINT size = m_height * m_pitch;
		if (m_pLayers[layer].kind >= CCell::LAYER_MATTE0)
			{
			memcpy(pAlpha+m_height*m_pitch, pAlpha, size); 	//save tone matte
			}
		memset(pAlpha, 255, size); 	
		BYTE * pIndex = pAlpha + size;
		memset(pIndex, index, size); 	
		}
	else if ((kind & 3) == 1)
		{
		BYTE * pPaint = m_pLayers[m_nPaint].pData;
		layer = m_pTable->layer;
		if (layer > 5)
			layer += 2;
//		UINT j = TQ(m_nCurLayer);
//		m_pTable->table[j].color = m_color;
		m_bDirty = TRUE;	// needs display update
		m_pLayers[layer].flags = FLAG_ACTIVE | FLAG_MODIFIED;
		BYTE * pAlpha = m_pLayers[layer].pData;
		BYTE * pIndex = pAlpha + m_pitch * m_height;
		ASSERT(m_pLayers[layer].kind >= CCell::LAYER_MATTE0);
		UINT i;
		if (kind & 4)
			for (i = 0; i < m_pitch * m_height; i++)
				{
				if (!pPaint[i])
					{
					pAlpha[i] = 255;
					pIndex[i] = index;
					}
				}
		else
			for (i = 0; i < m_pitch * m_height; i++)
				{
				if (pPaint[i])
					{
					pAlpha[i] = 255;
					pIndex[i] = index;
					}
				}
		}
	else if (kind == 2)
		{
		layer = m_pTable->layer;
		if (layer > 5)
			layer += 2;
//		UINT j = TQ(m_nCurLayer);
//		m_pTable->table[j].color = m_color;
		m_bDirty = TRUE;	// needs display update
		m_pLayers[layer].flags = FLAG_ACTIVE | FLAG_MODIFIED;
		BYTE * pAlpha = m_pLayers[layer].pData;
		BYTE * pIndex = pAlpha + m_pitch * m_height;
		ASSERT(m_pLayers[layer].kind >= CCell::LAYER_MATTE0);
		BYTE desired;
		UINT i;
		if (!index)
			{
			desired = 1;
			for (i = 0; i < m_pitch * m_height; i++)
				if (!pIndex[i])
					pIndex[i] = desired;
			}
		else
			desired = 0;
		for (i = 0; i < m_pitch * m_height; i++)
			{
			if (!pAlpha[i] || (pIndex[i] != desired))
				{
				pAlpha[i] = 255;
				pIndex[i] = index;
				}
			}
		}
	else 
		{
		layer = m_pTable->layer;
		if (layer > 5)
			layer += 2;
//		UINT j = TQ(m_nCurLayer);
//		m_pTable->table[j].color = m_color;
		m_bDirty = TRUE;	// needs display update
		m_pLayers[layer].flags = FLAG_ACTIVE | FLAG_MODIFIED;
		BYTE * pAlpha = m_pLayers[layer].pData;
		BYTE * pIndex = pAlpha + m_pitch * m_height;
		ASSERT(m_pLayers[layer].kind >= CCell::LAYER_MATTE0);
		UINT i;
		BYTE desired;
		if (!index)
			{
			desired = 1;
			}
		else
			desired = 0;
		for (i = 0; i < m_pitch * m_height; i++)
			{
			if (pAlpha[i] && (pIndex[i] == desired))
				{
				pAlpha[i] = 255;
				pIndex[i] = index;
				}
			}
		}
	return 0;
}

void CLayers::Flipper(UINT mask)
{
	UINT layer;
	BYTE * pAlpha;
//	BYTE * pIndex;
	for (layer = 1; layer < m_nLayers;layer++)
		{
		pAlpha = m_pLayers[layer].pData;
		UINT x, hw;
		UINT y, hh;
		hw = m_width / 2;
		hh = (m_height + 1) / 2;
		for (int q = 0; q < 2; q++)
		{
		for (y = 0; y < hh;y++)
			{
			UINT y1,y2;
			if (mask & 2)
				{
				y2 = y;
				y1 = m_height - 1 - y;
				}
			else
				{
				y1 = y;
				y2 = m_height - 1 - y;
				}
			for (x = 0; x < hw; x++)
				{
				BYTE a,b,c,d;
				if (mask & 1)
					{
					b = pAlpha[y1*m_pitch+x];
					a = pAlpha[y1*m_pitch+m_width-1-x];
					d = pAlpha[y2*m_pitch+x];
					c = pAlpha[y2*m_pitch+m_width-1-x];
					}
				else
					{
					a = pAlpha[y1*m_pitch+x];
					b = pAlpha[y1*m_pitch+m_width-1-x];
					c = pAlpha[y2*m_pitch+x];
					d = pAlpha[y2*m_pitch+m_width-1-x];
					}
				pAlpha[y*m_pitch+x] = a;
				pAlpha[y*m_pitch+m_width-1-x] = b;
				pAlpha[(m_height-1-y)*m_pitch+x] = c;
				pAlpha[(m_height-1-y)*m_pitch+m_width-1-x] = d;

//				v = pIndex[y*m_pitch+x];
//				pIndex[y*m_pitch+x] = pIndex[y*m_pitch + m_width - 1 - x];
//				pIndex[y*m_pitch + m_width - 1 - x] = v;
				}
			}
		pAlpha +=  m_pitch * m_height; // now index
		}
	}
}


UINT CLayers::Clear()
{
	UINT layer;
	UINT size = m_pitch * m_height;
	m_bNeedFake = m_bModified = TRUE;
	for (layer = 1; layer < m_nLayers;layer++)
		{
		m_pLayers[layer].flags = FLAG_ACTIVE | FLAG_MODIFIED;
		memset( m_pLayers[layer].pData,0,size+size);
		}
	return 0;
}

bool CLayers::CanUndo()
{
	return m_nUndo ? 1 : 0;
}

bool CLayers::CanRedo()
{
	return m_nRedo ? 1 : 0;
}

BYTE * CLayers::PushUndo(UINT layer, bool bSkip)
{
	UINT size = m_height * 2 * m_pitch;
	UINT index = m_nPushCount;
	m_nPushCount = (m_nPushCount + 1) % m_nMaxUndo;
	BYTE * pp = m_pUndo[index].pData;
	m_pUndo[index].layer = layer;
	bool bDouble = 0;
	if (layer == 99)
		{
		bDouble = 1;
		layer = m_nInk;
		}
	if (!bSkip)
		{
		memmove(pp,m_pLayers[layer].pData,size);
		if (bDouble)
		memmove(pp+size,m_pLayers[m_nPaint].pData,size);
		}
	m_nUndo++;
	if (m_nUndo >= m_nMaxUndo)
		m_nUndo = m_nMaxUndo;
	m_nRedo = 0;
	return pp;
}

void CLayers::Undo(UINT zindex  /* = NEGONE */)
{
	UINT size = m_height * 2 * m_pitch;
	UINT index;
	if (zindex == NEGONE)
		{
		FillStack(-2);
		m_nPushCount = (m_nPushCount + m_nMaxUndo - 1) % m_nMaxUndo;
		m_nUndo--;
		m_nRedo++;
		index = m_nPushCount;
		}
	else
		index = zindex;
	UINT layer = m_pUndo[index].layer;
	bool bDouble = 0;
	if (layer == 99)
		{
		bDouble = 1;
		layer = m_nInk;
		}
	BYTE * p1;
	BYTE * p2;
	UINT i;
	BYTE v;
	p1 = m_pUndo[index].pData;
	p2 = m_pLayers[layer].pData;
	if (zindex != NEGONE)
		memmove(p2,p1,size);
	else
		{
		for (i = 0; i < size; i++)
			{
			v = *p1;
			*p1++ = *p2;
			*p2++ = v;
			}
		if (bDouble)
		{
		p2 = m_pLayers[m_nPaint].pData;
		for (i = 0; i < size; i++)
			{
			v = *p1;
			*p1++ = *p2;
			*p2++ = v;
			}
		}
		}
}


void CLayers::Redo()
{
	FillStack(-2);
	UINT index = m_nPushCount;
	m_nPushCount = (m_nPushCount + 1) % m_nMaxUndo;
	UINT size = m_height * 2 * m_pitch;
	BYTE * pp = m_pUndo[index].pData;
	m_nRedo--;
	m_nUndo++;
	bool bDouble = 0;
	UINT layer = m_pUndo[index].layer;
	if (layer == 99)
		{
		bDouble = 1;
		layer = m_nInk;
		}
	UINT i;
	BYTE v;
	BYTE * p1 = m_pUndo[index].pData;
	BYTE * p2 = m_pLayers[layer].pData;
	for (i = 0; i < size; i++)
		{
		v = *p1;
		*p1++ = *p2;
		*p2++ = v;
		}
	if (bDouble)
		{
		p2 = m_pLayers[m_nPaint].pData;
	for (i = 0; i < size; i++)
		{
		v = *p1;
		*p1++ = *p2;
		*p2++ = v;
		}
		}
}

UINT CLayers::SaveModel(LPCSTR name /* = 0 */)
{
	LPCSTR pname;
	if (name)
		pname = name;
	else
		pname = (LPCSTR)&m_name;
DPF("save model:%s|",pname);
	CFile f;
	UINT result = 1;
	MDLHEADER header;
	UINT lays[16];
	DWORD mode = CFile::modeCreate | CFile::modeReadWrite;
	if (!f.Open(pname, mode))
		return result;
	BYTE * tbuf = 0;
	for (;;)
		{
		memset(&header, 0, sizeof(header));	
		strcpy(header.Id, "DIGICEL MODEL");
		header.width = m_width;
		header.height= m_height;
		header.version = 1;
		header.nLays = 0;
		UINT layer;
		for (layer = 1; layer < m_nLayers; layer++)
			{
			if (m_pLayers[layer].flags & FLAG_ACTIVE)
				lays[header.nLays++] = layer ^ 4;
			}
		f.Write(&header, sizeof(header));
		f.Write(&lays, header.nLays * sizeof(UINT));
		UINT i,size;
		size = 2 * m_pitch * m_height;	// alpha and index
		DWORD ddsize = 20 + (size * 102) / 100;
		tbuf = new BYTE[ddsize];
		for (i = 0; i < header.nLays; i++)
			{
			unsigned long dsize = ddsize;
			UINT q = compress(tbuf,&dsize,m_pLayers[lays[i]^4].pData, size);
			if (q)
				{
				DPF("compression failure:%d",q);
//				delete tbuf;
				result = 2;
				break;
				}
			f.Write(&dsize, sizeof(UINT));
			f.Write(tbuf, dsize);
			}
		if (i >= header.nLays)
			result = 0;
		break;
		}
	delete [] tbuf;
	f.Close();
	return result;
}

UINT CLayers::LoadModel(UINT Level, bool bCreate)
{
	UINT result = 1;
	m_pScene->LevelModelName((LPSTR)&m_name,Level);
	if (!m_name[0])
		return 7;
//	m_pPalette = m_pScene->PalAddr(Level);
	MDLHEADER header;
	UINT lays[16];
	CEmbedFile f(m_pScene);
	if (f.Open(m_name,2)) //EMB_KIND_MODEL);
		{
		return 9;
		}
	BYTE * tbuf = 0;
	for (;;)
		{
		f.Read(&header, sizeof(header));
		f.Read(&lays, header.nLays * sizeof(UINT));
		if (_stricmp(header.Id, "DIGICEL MODEL"))
			break;
		UINT i;
		for (i = 0; i < header.nLays; i++)
			{
			lays[i] ^= 4;		// translate from old to new
			}
		result++;
		if (header.version != 1)
			break;
		if (bCreate)
			{
			m_width = header.width;
			m_height = header.height;
			m_pitch = 4 * ((m_width + 3) / 4);
			m_nLayers = 13;
			CreateLayers();
			}
		else
			{
			if ( header.width != m_width ||
					header.height!= m_height)
				break;
			}
		result++;
		UINT size = 2 * m_pitch * m_height;	// alpha and index
		DWORD ddsize = 20 + (size * 102) / 100;
		tbuf = new BYTE[ddsize];
		for (i = 0; i < header.nLays; i++)
			{
			UINT dsize;
			f.Read(&dsize, sizeof(UINT));
			if (dsize > ddsize)
				break;
			f.Read(tbuf, dsize);
			unsigned long vc = size;
			UINT q = uncompress(m_pLayers[lays[i]].pData,&vc,tbuf,dsize);

			DPF("uncom,%d,%d",vc,size);
			if (q)
				{
DPF("decompress error:%d",q);
				break;
				}
			m_pLayers[lays[i]].flags = FLAG_ACTIVE;
//			if (!m_firstlayer)
//				m_firstlayer = lays[i];
//			else
//				m_pLayers[j].link = lays[i];
//			j = lays[i];
			}
		if (i >= header.nLays)
			result = 0;
		break;
		}
	delete [] tbuf;
//f.Close();
	return result;
}

UINT TestModel(LPCSTR Name, UINT width, UINT height)
{
	CFile f;
	UINT result = 1;
	MDLHEADER header;
	DWORD mode = CFile::modeReadWrite;
	if (!f.Open(Name, mode))
		return result;
	BYTE * tbuf = 0;
	for (;;)
		{
		f.Read(&header, sizeof(header));
		if (_stricmp(header.Id, "DIGICEL MODEL"))
			break;
		result++;

		if (
/*
			header.width != width ||
			header.height!= height ||
*/
			header.version != 1)
			break;
		result = 0;
		break;
		}
	f.Close();
	return result;
}

int CLayers::UnCranny(int count)
{
	DPF("uncranny,%d",count);
	UINT layer = m_nPaint;
	if (!m_pScene->ColorMode())
		return 0;
	BYTE * pInk = m_pLayers[m_nInk].pData;
	BYTE * pPaint = m_pLayers[m_nPaint].pData;
	BYTE * pUndo= PushUndo(layer,1);
	m_minx = m_width;
	m_maxx = 0;
	m_miny = m_height;
	m_maxy = 0;
	UINT size = m_pitch * m_height;
	memset(pUndo,0,size + size);
	int x, y;
	for (y = 0; y < (int)m_height; y++)
	for (x = 0; x < (int)m_width; x++)
		{
		UINT offset = m_pitch * y + x;
//		if (!pInk[offset])
//			continue;
		if (!pPaint[offset])
			{
			int z,c;
			UINT q,a;
			c = 0;
			for (z = 0; z < 8; z++)
				{
				int xx = x + dx[z];
				int yy = y + dy[z];
				if (((UINT)xx >= m_width) || ((UINT)yy >= m_height))
					continue;
				if (!pPaint[m_pitch * yy + xx])
					continue;
				q = pPaint[size + m_pitch * yy + xx];
				a = pPaint[m_pitch * yy + xx];
				c++;
				}
			if ((c + count)> 6)
				{
				pUndo[offset] = a;
				pUndo[offset+size] = q;
				if (x < m_minx) m_minx = x;
				if (x > m_maxx) m_maxx = x;
				int yy = m_height - 1 - y;
				if (yy < m_miny) m_miny = yy;
				if (yy > m_maxy) m_maxy = yy;
				}
			}
/*
		else
			{
			UINT v = pPaint[offset + size];
			int z;
			UINT q,a;
			for (z = 0; z < 8; z++)
				{
				UINT xx = x + dx[z];
				UINT yy = y + dy[z];
				if ((xx >= m_width) || (yy >= m_height))
					continue;
				if (!pPaint[m_pitch * yy + xx])
					continue;
				q = pPaint[size + m_pitch * yy + xx];
				if (q == v)
					break;
				a = pPaint[m_pitch * yy + xx];
				}
			if (z >= 9)//8)
				{
				pUndo [offset] = pPaint[offset];
				pUndo [offset+size] = pPaint[offset+size];
				pPaint[offset] = a;
				pPaint[offset+size] = q;
				if (x < m_minx) m_minx = x;
				if (x > m_maxx) m_maxx = x;
				int yy = m_height - 1 - y;
				if (yy < m_miny) m_miny = yy;
				if (yy > m_maxy) m_maxy = yy;
				}
			}
*/
	}
	for (x = 0; (UINT)x < size; x++)
		{
		UINT a, v;
		if (pUndo [x])
			{
			a = pUndo[x];
			v = pUndo[x+size];
			pUndo[x] = pPaint[x];
			pUndo[x+size] = pPaint[x+size];
			pPaint[x] = a;
			pPaint[x+size] = v;
			}
		else
			{
			pUndo[x] = pPaint[x];
			pUndo[x+size] = pPaint[x+size];
			}
		}
	if ((UINT)m_minx < m_width)
		{
		m_bNeedFake = m_bModified = TRUE; 
		m_bDirty = TRUE;
		m_pLayers[layer].flags = FLAG_ACTIVE | FLAG_MODIFIED;
		}
//	EchoIt();
	return m_bDirty;
	return 0;
}

int CLayers::Speckle(int count)
{
	DPF("speckle,%d",count);
	if (count > 1)
		return Magical();
	UINT layer = m_nPaint;
	if (!m_pScene->ColorMode())
		return 0;
	BYTE * pPaint = m_pLayers[m_nPaint].pData;
	BYTE * pUndo= PushUndo(layer,1);
	m_minx = m_width;
	m_maxx = 0;
	m_miny = m_height;
	m_maxy = 0;
	UINT size = m_pitch * m_height;
	memset(pUndo,0,size + size);
	int x, y;
	int ww, hh;
	ww = m_width - 1;
	hh = m_height - 1;
	int t[4];
	t[0] = -1;
	t[1] = 1;
	t[2] = m_pitch;
	t[3] = -(int)m_pitch;
	UINT offset = 1 + m_pitch;
	for (y = 1; y < hh; y++, offset += m_pitch - m_width - 1)
		{
		for (x = 1; x < ww; x++, offset++)
			{
			if (pPaint[offset])
				continue;
			int c, d;
			UINT cc, cv;
			for (c = 0, d = 0; d < 4; d++)
				{
				if (pPaint[offset+t[d]])
					{
					cc = pPaint[offset+t[d]];
					cv = pPaint[size + offset+t[d]];
					c++;
					}
				}
			if (c > 2)
				{
				if (x < m_minx) m_minx = x;
				if (x > m_maxx) m_maxx = x;
				int yy = m_height - 1 - y;
				if (yy < m_miny) m_miny = yy;
				if (yy > m_maxy) m_maxy = yy;
				pUndo[offset] = cc;
				pUndo[offset+size] = cv;
				}
			}
		}
	for (x = 0; (UINT)x < size; x++)
		{
		UINT a,v;
		if (pUndo [x])
			{
			a = pUndo[x];
			v = pUndo[x+size];
			pUndo[x] = 0;
			pUndo[x+size] = 0;
			pPaint[x] = a;
			pPaint[x+size] = v;
			}
		else
			{
			pUndo[x] = pPaint[x];
			pUndo[x+size] = pPaint[x+size];
			}
		}
	if ((UINT)m_minx < m_width)
		{
		m_bNeedFake = m_bModified = TRUE; 
		m_bDirty = TRUE;
		m_pLayers[layer].flags = FLAG_ACTIVE | FLAG_MODIFIED;
		}
//	EchoIt();
	return m_bDirty;
	return 0;
}

UINT CLayers::Layer(int which /* = -1 */)
{
	if (which != -1)
		{
		m_pTable->layer = which;
		}
	else if (!m_pTable)
		return 0;	
	return m_pTable->layer;
}

void CLayers::LoadControl(UINT Frame, UINT Level, bool bClear /* = FALSE */)
{
	UINT size = 2 * m_pitch * m_height;
	if (!m_pControl)
		m_pControl = new BYTE[ size + size + size];
	if (bClear)
		{
DPF("clearing control");
		memset(m_pControl+size, 0, size);
		return;
		}
DPF("loading control,lvl:%d",Level);
//	DWORD key  = m_pScene->GetCellKey(Frame,Level,TRUE);
	DWORD key  = m_pScene->GetCellKey(Frame,Level);
	if (!key)
		return;
	if (m_pScene->GetLayer(m_pControl+size+size,Frame, Level,
				CCell::LAYER_INK, key))
		return;
	UINT i,c;
	c = m_pitch * m_height;
	for (i = 0; i < c; i++)
		{
		if (m_pControl[i+size+size])
			{
			m_pControl[i+size] = m_pControl[i+size+size];
			m_pControl[i+size+c] = m_pControl[i+size+size+c];
			}
		}

}

int CLayers::Magical()
{
	DPF("magical");
	UINT layer = m_nPaint;
	if (!m_pScene->ColorMode())
		return 0;
	int r = 4;
	int mr = r + r + 1;
	int qr = mr * mr;
	int tr[90];
	int tx[90];
	int ty[90];
	int xx, yy, c;
	c = 0;
	for (xx = -r; xx <= r; xx++)
	for (yy = -r; yy <= r; yy++)
		{
		tr[c] = xx * xx + yy * yy;
		tx[c] = xx;
		ty[c++] = yy;
		}		// table of dx, dy, distance for center
	int i, j;
//	for (i = 0;i < c; i++)
//		{
//		DPF("i:%d, %d,%d,%d",i,tr[i],tx[i],ty[i]);
//		}
	for (i = 0; i < (c-1); i++)
	{
	for (j = i+1; j < c; j++)
		{
		if (tr[i] > tr[j])
			{
			int tmp = tr[i];
			tr[i] = tr[j];
			tr[j] = tmp;
			tmp = tx[i];
			tx[i] = tx[j];
			tx[j] = tmp;
			tmp = ty[i];
			ty[i] = ty[j];
			ty[j] = tmp;
			}
		}
	}	// sorted accoring to distance
//	for (i = 0;i < c; i++)
//		{
//		DPF("i:%d, %d,%d,%d",i,tr[i],tx[i],ty[i]);
//		}
	BYTE * pPaint = m_pLayers[m_nPaint].pData;
	BYTE * pInk = m_pLayers[m_nInk].pData;
	BYTE * pUndo= PushUndo(layer,1);
	m_pLayers[0].kind = layer;
	m_pLayers[0].flags = FLAG_ACTIVE | FLAG_MODIFIED;
	UINT size = m_pitch * m_height;
	memset(pUndo,0,size + size);
	UINT x, y;							// fill forur corners with magic color
	if (!pPaint[(m_height-1)* m_pitch])
		FillOffset(0,0,256+MAGIC_COLOR,0,0);
	if (!pPaint[size - 1])
		FillOffset(m_width-1,0,256+MAGIC_COLOR,0,0);
	if (!pPaint[0])
		FillOffset(0,m_height-1,256+MAGIC_COLOR,0,0);
	if (!pPaint[m_width-1])
		FillOffset(m_width-1,m_height-1,256+MAGIC_COLOR,0,0);
	for (y = 0; y < m_height; y++)
		{
		for (x = 0; x < m_width; x++)
			{
			UINT offset = x + y * m_pitch;
			if (pPaint[offset])
				continue;				// look for a not painted pixel
			int i;
			
			UINT pp;
			for (i = 1; i < c; i++)	// find a neighbor on this side of ink
				{
				UINT xx = x + tx[i];
				UINT yy = y + ty[i];
				if ((xx < m_width) && (yy < m_height))
					{
					pp = yy * m_pitch + xx;
					if (pPaint[pp] && (pPaint[size+pp] != MAGIC_COLOR))
						{
			//			if (!pInk[size + pp] || (pInk[pp] > pInk[size+pp]))  
							break;
						}
					}
				}
			if (i < c)
				{
				m_bDirty = 1;
				pPaint[offset] = pPaint[pp];
				pPaint[offset+size] = pPaint[size + pp];
				if (y > (UINT)r)
					y -= r + 1;
				else
					y = 0;
			//	else
					x = 0;
				break;
				}
			}
		}	

//	now restore all magix paint pixels to null
//
	
	for (y = 0; y < (int)m_height; y++)
		{
		for (x = 0; x < (int)m_width; x++)
			{
			UINT offset = x + y * m_pitch;
			if (pPaint[offset] && (pPaint[offset+size] == MAGIC_COLOR))
				{
				pPaint[offset] = 0;
				pPaint[offset+size] = 0;
				}
			}
		}	

	m_maxx = m_width-1;			// force update of canvas
	m_minx = 0;
	m_maxy = m_height-1;
	m_miny = 0;
	return m_bDirty;
}

#define PAINTA(o) pPaint[offset+o]
#define PAINTI(o) pPaint[size + offset+o]
#define PAINT(d) (PAINTA(m_pitch*dy[d]+dx[d]) && \
					(PAINTI(m_pitch*dy[d]+dx[d]) == color))
#define INKA(o) pPaint[offset+o]
#define INKI(o) pPaint[size + offset+o]
#define INK(d) INKA(m_pitch*dy[d]+dx[d])

int CLayers::DeGapper(int count,UINT color)
{
//	int t[5][10];
//	int c;
	DPF("de gapper:%d",count);
	if (!m_pScene->ColorMode())
		return 0;
	CPixels cp;
	UINT layer = m_nInk;
	cp.m_pPaint = m_pLayers[m_nPaint].pData;
	cp.m_pInk   = m_pLayers[m_nInk].pData;
	cp.m_color = m_color;
	cp.m_width = m_width;
	cp.m_height = m_height;
	cp.m_pitch = m_pitch;
	cp.m_size = m_pitch * m_height;
//	BYTE * pUndo= PushUndo(layer,1);
	m_pLayers[0].kind = layer;
	m_pLayers[0].flags = FLAG_ACTIVE | FLAG_MODIFIED;
	m_minx = m_width;
	m_maxx = 0;
	m_miny = m_height;
	m_maxy = 0;
	count = 2;
	PushUndo(m_nInk);
	UINT size = m_pitch * m_height;
//	memset(pUndo,0,size + size);
	int x, y;
	int ww, hh;
	ww = m_width - 1 - count;
	hh = m_height - 1 - count;
	for (y = 1; y < hh; y++)
		{
		for (x = 1; x < ww; x++)
			{
			cp.m_offset = y * m_pitch + x;
			if (!cp.Paint())
				continue;
			int delta = 0;
			if (cp.Ink(-1) && cp.Ink(m_pitch) && 
						cp.Paint(m_pitch - 1))
				{
				if (cp.Paint(1) && cp.Paint(2) && cp.Paint(3))
					delta = m_pitch - 1;
				else if (cp.Paint(-(int)m_pitch) && cp.Paint(-2*(int)m_pitch))
					delta = m_pitch - 1;
				else if (cp.Paint(m_pitch - 2) && cp.Paint(m_pitch - 3))
					delta = m_pitch - 1;
				else if (cp.Paint(2 * m_pitch-1) && cp.Paint(3*m_pitch-1))
					delta = m_pitch - 1;
				}
			else if (cp.Ink(1) && cp.Ink(m_pitch) && 
						cp.Paint(m_pitch + 1))
				{
				if (cp.Paint(-1) && cp.Paint(-2) && cp.Paint(-3))
					delta = m_pitch + 1;
				else if (cp.Paint(-(int)m_pitch) && cp.Paint(-2*(int)m_pitch))
					delta = m_pitch + 1;
				else if (cp.Paint(m_pitch + 2) && cp.Paint(m_pitch + 3))
					delta = m_pitch + 1;
				else if (cp.Paint(2 * m_pitch+1) && cp.Paint(3*m_pitch+1))
					delta = m_pitch + 1;
				}
			else if (cp.Ink(-(int)m_pitch) && cp.Ink(m_pitch) &&
					cp.Paint(1) && cp.Paint(2) && cp.Paint(-1) && cp.Paint(-2))
					delta = 9999;
			if (delta)
				{
DPF("found,x:%d,y:%d,d:%d",x,y,delta);
				int i,z,a,b;
				b = 0;
				for (i = 0; i < 8;i++)
					{
					a = cp.m_pInk[x+dx[i] + (y+dy[i]) * m_pitch];
					if (a > b)
						{
						z = i;
						b = a;
						}
					}
				a = cp.m_pInk[cp.m_size+ x+dx[z] + (y+dy[z]) * m_pitch];
b = 1;
				cp.Ink(0,a,b);
				if (delta != 9999)
					cp.Ink(delta,a,b);
				m_bDirty = TRUE;
				}
			}
		}
	return m_bDirty;
}

UINT CLayers::LastUndoIndex()
{
	return (m_nPushCount + m_nMaxUndo - 1) % m_nMaxUndo;
}

BYTE * CLayers::LastUndo()
{
	UINT size = m_height * 2 * m_pitch;
	UINT index = (m_nPushCount + m_nMaxUndo - 1) % m_nMaxUndo;
	BYTE * pp = m_pUndo[index].pData;
#ifdef _DEBUG
	DPF("last undo,layer:%d",m_pUndo[index].layer);
#endif
	return pp;
}


void CLayers::CleanUp(UINT code)
{
	if (code)
		return;
	BYTE * pInk = m_pLayers[m_nInk].pData;
	BYTE * pOld = LastUndo();
	UINT size, c;
	size = m_height * m_pitch;
	for (c = 0; c < size; c++)
		{
		if (pInk[c] != pOld[c])
			{
//			ASSERT(pInk[c] == 255);
			pInk[c] = 1;
			pInk[c+size] = 0;
			UINT y = c / m_pitch;
			UINT x = c % m_pitch;
			}
		}
}
//
//	codes:0 is cut, 1 is copy, 2 is clear
//
BYTE * CLayers::GetClip(RECT & rcSelect, UINT code, BYTE * pMask)
{
	int xx,yy,x,y,w,h;
	x = rcSelect.left;
	y = rcSelect.top;
	w = rcSelect.right - x;
	h = rcSelect.bottom - y;
//	if (x < 0) x = 0;
//	if (y < 0) y = 0;
	UINT maskw = w;
	UINT maskh = h;
//	if ((UINT)(x+w) >= m_width) w = m_width - x;
//	if ((UINT)(y+h) >= m_height) h = m_height - y;
	UINT v;
	UINT size = 20;
	UINT off2 = m_height * m_pitch;// only for color
	UINT off1 = w * h;
	UINT offm; // to mask
	if (m_nLayers < 3)
		{
		v = 0;
		size += off1;
		offm = off1;
		}
	else
		{
		v = 1;
		size += 4 * off1 + 1024;
		offm = 4 * off1;
		}
	size += off1; // for mask
	BYTE * pClipBoard = 0;
	BYTE * pData;
	if (code < 2)
		{
		pClipBoard = new BYTE[size];
		UINT * pHead = (UINT *)pClipBoard;
		*pHead++ = v | 8;
		*pHead++ = x;
		*pHead++ = y;
		*pHead++ = w;
		*pHead++ = h;
		pData = (BYTE *)pHead;
		}
	else
		pData = 0;
	if (v && pData)
		{
		int i;
		CNewPals * pPals = m_pScene->LevelPalette(m_level);
		for (i = 0; i < 256; i++)
			{
			*pData++ = pPals->Red(i);
			*pData++ = pPals->Green(i);
			*pData++ = pPals->Blue(i);
			*pData++ = pPals->Alpha(i);
			}
		}
	if (code != 1)
		PushUndo(v ? 99 : m_nInk);
	for (yy = 0; yy < h; yy++)
	for (xx = 0; xx < w; xx++)
		{
		int xxx = x + xx;
		int yyy = y+h-1-yy;
		yyy = m_height - 1 - yyy;
		UINT z = yyy * m_pitch + xxx;
		if (((UINT)(xxx) >= m_width) || ((UINT)(yyy) >= m_height))
			{
			if (pData)
				{
				if (!v)
					*pData++ = 255;
				else
					{
					pData[0] = 0;
					pData[off1] = 0;
					pData[off1*2] = 0;
					pData[off1*3] = 0;
					pData++;
					}
				}
			}
		else if (!v)
			{
			if (pMask && !pMask[maskw*yy+xx])
				{	
				if (pData)
					*pData++ = 255;
				}
			else
				{
				if (pData)
					*pData++ = 255 - m_pLayers[m_nInk].pData[z];
				if (code != 1)
					m_pLayers[m_nInk].pData[z] = 0;
				}
			}
		else
			{
			if (pMask && !pMask[maskw*yy+xx])
				{	
				if (pData)
					{
					pData[0] = 0;
					pData[off1] = 0;
					pData[off1*2] = 0;
					pData[off1*3] = 0;
					pData++;
					}
				}
			else
				{
				if (pData)
					{
					pData[0] = m_pLayers[m_nInk].pData[z];
					pData[off1] = m_pLayers[m_nInk].pData[z+off2];
					pData[off1*2] = m_pLayers[m_nPaint].pData[z];
					pData[off1*3] = m_pLayers[m_nPaint].pData[z+off2];
					pData++;
					}
				if (code != 1)
					{
					m_pLayers[m_nInk].pData[z] = 0;
					m_pLayers[m_nPaint].pData[z] = 0;
					}
				}
			}
		}

	if (pData)
		{
		if (v)
			pData += 3 * off1;
		if (pMask)
			memcpy(pData, pMask, w * h);
		else
			memset(pData, 255, w * h);	// rect implies all on
		}

	if (code != 1)
		{
		m_pLayers[m_nInk].flags = FLAG_ACTIVE | FLAG_MODIFIED;
		if (v)
			m_pLayers[m_nPaint].flags = FLAG_ACTIVE | FLAG_MODIFIED;
		m_minx = 0;
		m_maxx = m_width - 1;
		m_miny = 0;
		m_maxy = m_height - 1;
		m_bDirty = m_bModified = TRUE;
		}
	return pClipBoard;
}

void CLayers::ApplyFloat(CFloat * pFloat, bool bMapPalette)
{
	if (!pFloat)
		return;
	if (m_bHasName = pFloat->m_bHasName)
		memcpy(m_cell_name, pFloat->m_name, 16);
	BYTE * pData = pFloat->Data();
	if (!pData)
		return;
	RECT src;
	pFloat->GetRect(src);
	int dx,dy,x,y,w,h;
	dx = src.left;
	dy = src.top;
	w = src.right - dx;
	h = src.bottom - dy;
	bool bSrcColor = pFloat->m_q ? 1 : 0;
	bool bColor = m_nLayers > 2 ? 1 : 0;
	UINT fp = 4 * ((w + 3) / 4);
/*
	if ((dx + w) > m_maxx)
		m_maxx = dx+w;
	if (dx < m_minx)
		m_minx = dx;
	if ((dy+h) > m_maxy)
		m_maxy = dy+h;
	if (dy < m_miny)
		m_miny = dy;
*/
	m_pLayers[m_nInk].flags = FLAG_ACTIVE | FLAG_MODIFIED;
	if (bColor)
		{
		BYTE map[256];
		m_pLayers[m_nPaint].flags = FLAG_ACTIVE | FLAG_MODIFIED;
		UINT size = m_pitch * m_height;
		BYTE * pPaint = m_pLayers[m_nPaint].pData;
		BYTE * pInk = m_pLayers[m_nInk].pData;
		CNewPals * pPals = m_pScene->LevelPalette(m_level);
		if (bSrcColor)
			{
			if (bMapPalette)
				pFloat->PalMap(map, pPals);
			else
				{
				for (y = 0; y < 256; y++)
					map[y] = (BYTE)y;
				}
			}
		for (y = 0; y < h; y++)
			{
			UINT ay = m_height - dy - h + y;
			for (x = 0; x < w; x++)
				{
				UINT ax = x + dx;
				if ((ax < m_width) && (ay < m_height))
					{
					UINT pa, pi;
					UINT ia, ii;
					UINT npi, nii,opi,oii;
			//		UINT nia, npa;
					if (bSrcColor)
						{
						ia = pData[y*fp+x];
						ii = pData[y*fp+x+h*fp];
						pa = pData[y*fp+x+2*h*fp];
						pi = pData[y*fp+x+3*h*fp];
						npi = map[pi];
						nii = map[ii];
						oii = pInk[m_pitch * ay + ax + size];
						opi = pPaint[m_pitch * ay + ax + size];
						}
					else
						{
						ia = 255 - pData[y*fp+x];
						nii = 0;
						}
					if (bSrcColor && pa)
						{
						if (npi != opi)
							{
							pPaint[m_pitch * ay + ax] = pa;
							pPaint[m_pitch * ay + ax+size] = npi;
							}
						else
							{
							UINT opa = pPaint[m_pitch * ay + ax];
							pPaint[m_pitch * ay + ax] =
									pa+opa-(opa*pa/255);
							}
						}
					if (ia)
					{
					if (nii != oii)
						{
						pInk[m_pitch * ay + ax] = ia;
						pInk[m_pitch * ay + ax+size] = nii;
						}
					else
						{
						UINT oia = pInk[m_pitch * ay + ax];
						pInk[m_pitch * ay + ax] =
								ia+oia-(oia*ia/255);
						}
					}
					}
				}
			}
		}
	else
		{
		BYTE * pInk = m_pLayers[m_nInk].pData;
		BYTE zz = bSrcColor ? 0 : 255;
		for (y = 0; y < h; y++)
			{
			UINT ay = m_height - dy - h + y;
			for (x = 0; x < w; x++)
				{
				UINT ax = x + dx;
				if ((ax < m_width) && (ay < m_height))
					{
					UINT v = pData[y*fp+x];
					if (v != zz)
						{
						if (!bSrcColor)
							v = 255 - v;
						UINT ov = pInk[m_pitch*ay+ax];
						pInk[m_pitch*ay+ax] = (v+ov - (v*ov)/255);
						}
					}
				}
			}
		}
	m_minx = 0;
	m_maxx = m_width - 1;
	m_miny = 0;
	m_maxy = m_height - 1;
	m_bDirty = m_bModified = TRUE;
}

void CLayers::StashUndo()
{
	int c = m_nLayers < 3 ? 1 : 2;
	PushUndo((c > 1) ? 99 : m_nInk);
}

bool CLayers::Crop(RECT & rcSelect)
{
	int xx,yy,x1,y1,x2,y2;
	x1 = rcSelect.left;
	y2 = m_height - 1 - rcSelect.top;
	x2 = rcSelect.right;
	y1 = m_height -1 - rcSelect.bottom;
	int i;
	int c = m_nLayers < 3 ? 1 : 2;
	PushUndo((c > 1) ? 99 : m_nInk);
	for (i = 0; i < c; i++)
		{
		int layer = (i == (c - 1)) ? m_nInk : m_nPaint;
		m_pLayers[layer].flags = FLAG_ACTIVE | FLAG_MODIFIED;
		BYTE * p = m_pLayers[layer].pData;
		for (yy = 0; yy < y1; yy++)
			for (xx = 0; xx < (int)m_width;xx++)
				p[yy*m_pitch+xx] = 0;
		for (;yy < y2; yy++)
			{
			for (xx = 0; xx < x1;xx++)
				p[yy*m_pitch+xx] = 0;
			for (xx = x2; xx < (int)m_width;xx++)
				p[yy*m_pitch+xx] = 0;
			}
		for (; yy < (int)m_height; yy++)
			for (xx = 0; xx < (int)m_width; xx++)
				p[yy*m_pitch+xx] = 0;
		}
	m_bDirty = 1;
	m_bModified = 1;
	return TRUE;//m_bModified;
}
