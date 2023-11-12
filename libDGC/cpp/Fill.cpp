#include "CLayers.h"
#include "math.h"
#include "CLevtbl.h"
#include "CCell.h"
//#define LAY_UNDO   0
//#define LAY_SHADOW 1
//#define LAY_PAINT  2
//#define LAY_INK    3
//#define LAY_TINT   4

#define FILL_STACK
//#define FILL_LIMIT
#define TQ(a) (a < 7 ? a - 1 : a - 2)

typedef struct {
	UINT	kind;
	UINT	flags;
	BYTE *	pData;
} LAYER;

#ifdef FILL_STACK
typedef struct {
	WORD	x1;
	WORD	y;
	WORD	x2;
} FILLSTACK;
#endif

typedef struct {
	int		x;
	int 	y;
	int 	z;
} FSTACK;

UINT FillKind = 0;
UINT bErase = 0;
int magicx, magicy,magicc;
BYTE newColor = 1;
BYTE oldColor = 1;
#define LIMIT 1000
FSTACK * pStack = 0;
int findex = 0; // renamed to not conflict with index() method in string.h
/*
static BYTE zAlpha(int x, int y)
{
	LAYER * pLayers = (LAYER *)m_pLayers;
	BYTE * pAlpha = pLayers[m_nInk].pData;
	pAlpha += m_pitch * y;
	return pAlpha[x];
}
*/
static int dx[8] = {-1, 0, 1, 1, 1, 0,-1,-1};
static int dy[8] = {-1,-1,-1, 0, 1, 1, 1, 0};
#define Alpha(ax,ay) pInkA[m_pitch*(ay)+ax]

int CLayers::Try(int x, int y, int px, int py)
{
//	return 1;
	int	z,z1,z2,zz;
	int zs, ze;
	int xx,yy;
	BYTE	alpha;
	BYTE	tbl[8];
	LAYER * pLayers = (LAYER *)m_pLayers;
	BYTE * pInkA = pLayers[m_nInk].pData;
	if (m_pControl)
		pInkA = m_pControl;
	BYTE * pPntA = pLayers[m_nPaint].pData;
	BYTE * pPntI = pPntA + m_height * m_pitch;
	alpha = Alpha(px,py);
	if (x > px)
		{
		if (y > py)
			zs = 4;
		else
			zs = 2;
		}
	else
		{
		if (y > py)
			zs = 6;
		else
			zs = 0;
		}
	ze = zs;// + 7;
	x = px;
	y = py;
//
//	get alphas for all neighbors
//
	for (z = 0; z < 8; z++)
		{
		UINT xx = x + dx[z];
		UINT yy = y + dy[z];
		if ((xx >= m_width) || (yy >= m_height))
			tbl[z] = 0;
		else
			tbl[z] = Alpha(xx,yy);
		}
//
//	now process them
//
	for (zz = zs; zz <= ze; zz++)
		{
		z = zz % 8;
		for(;;)
			{

			if (tbl[z] == 255)
				break;
			if (!(z & 1))
				{
				z1 = (z + 7) % 8;
				z2 = (z + 1) % 8;
				if ((tbl[z1] > alpha ) && (tbl[z2] > alpha ))
					{
					UINT xx = x + dx[z] + dx[z1];
					UINT yy = y + dy[z] + dy[z1];
					if ((xx < m_width) && (yy < m_height) &&
							(Alpha(xx,yy) < alpha))
						break;
					xx = x + dx[z] + dx[z2];
					yy = y + dy[z] + dy[z2];
					if ((xx >= m_width) && (yy >= m_height) &&
							(Alpha(xx,yy) < alpha))
						break;
					}
				else if (tbl[z] < alpha)
					break;
				}
			else
				{
				if (tbl[z] < alpha)
					break;
				}
			xx = x + dx[z];
			yy = y + dy[z];
			if ((UINT)xx >= m_width)
				break;
			if ((UINT)yy >= m_height)
				break;
			if (bErase > 1)
				{
				if (pPntA[yy*m_pitch+xx])
					break;
				}
			else if (bErase)
				{
				if (!pPntA[yy*m_pitch+xx])
					break;
				}
			else	
				{
   	   			if ((pPntI[yy*m_pitch+xx] == newColor) &&
								pPntA[yy*m_pitch+xx])
					break;
				}
			DPF("tryied, %d,%d",xx,yy);
			return 0;
			}
		}
	return 1;
}



//int findruns(int x, int y, int z, int py, int qq);
//void fillline(int x, int y, int z);

UINT CLayers::FillOffset(int x, int y, UINT color, bool bbErase,UINT kind)
{ 
	if (kind == 50) // rbut down fill
		{
		kind = 0;
		color = 0;
		bbErase = 1;
		}
	m_dot_type = 0;
	if (kind >= 10)
		{
		if (kind == 10)
			kind = 11;
		m_dot_type = kind - 11;
DPF("dot type:%d",m_dot_type);
		kind = 0;
		}
	LAYER * pLayers = (LAYER *)m_pLayers;
	if (m_pControl && !kind)
		{
		UINT c = m_height * m_pitch;
		UINT i;
		BYTE * pInk = pLayers[m_nInk].pData;
		for (i = 0; i < c ; i++)
			{
			if (!pInk[i])
				{
				m_pControl[i] = m_pControl[c+c+i];
				m_pControl[i+c] = m_pControl[c+c+c+i];
				}
			else
				{
				m_pControl[i] = pInk[i];
				m_pControl[i+c] = pInk[i+c];
				}
			}
		}
	
	if (pLayers[m_nCurLayer].kind >= CCell::LAYER_MATTE0)
		{
		x -= m_pTable->table[TQ(m_nCurLayer)].dx;
		y -= m_pTable->table[TQ(m_nCurLayer)].dy;
		UINT result = Fill(x,y,color,bbErase,kind);
		m_minx += m_pTable->table[TQ(m_nCurLayer)].dx;
		m_miny += m_pTable->table[TQ(m_nCurLayer)].dy;
		m_maxx += m_pTable->table[TQ(m_nCurLayer)].dx;
		m_maxy += m_pTable->table[TQ(m_nCurLayer)].dy;
		if (m_minx < 0) m_minx = 0;
		if (m_miny < 0) m_miny = 0;
		if ((UINT)m_maxx >= m_width) m_maxx = m_width - 1;
		if ((UINT)m_maxy >= m_height) m_maxy = m_height - 1;
		return result;
		}
	else
		return Fill(x,y,color,bbErase,kind);
}

UINT CLayers::Fill(int x, int y, UINT color, bool bbErase,UINT kind)
{
	LAYER * pLayers = (LAYER *)m_pLayers;
	int q,z;
	q = 0;
	m_zx = x;
	m_zy = y;
	magicx = 0;
	if (kind == 99)
		FillKind = 2;
	else if (m_pTable->layer)
		{
		if (kind == 0)
			{
			Fill(x,y,color,bbErase,99);
			if (magicx)
				{
				int minx = m_minx;
				int miny = m_miny;
				int maxx = m_maxx;
				int maxy = m_maxy;
				Fill(magicx,m_height -1 - magicy,color,bbErase,99);
				if (minx < m_minx) m_minx = minx;
				if (miny < m_miny) m_miny = miny;
				if (maxx > m_maxx) m_maxx = maxx;
				if (maxy > m_maxy) m_maxy = maxy;
				}
			return 0;
			}
		else if (kind == 1)
			{
			BYTE * pPntA = pLayers[m_nPaint].pData;
			if (!pPntA[m_pitch * y + x])
				{
				Flood(color, 5);
				return 0;
				}
			}
		FillKind = 2 + kind;
		}
	else
		FillKind = kind;
	bErase = bbErase;
if (color > 255)
	bErase = 2;
	newColor = color & 255;
	DPF("New Improved Filling, x:%d,y:%d,color:%d,kind:%d",x,y,newColor,kind);
//if(kind) return 0;
	pStack = new FSTACK[LIMIT];
	findex = 0;
	m_minx = m_maxx = x;
	m_miny = m_maxy = y;
	y = m_height - 1 - y;
	UINT layer;
	if ((FillKind == 2) || (FillKind == 3) || (FillKind == 4))
		layer = m_nCurLayer;
	else if (FillKind)
		layer = m_nInk;
	else
		layer = m_nPaint;
	BYTE * pAlpha = pLayers[layer].pData;
	BYTE * pIndex = pAlpha + m_pitch * m_height;
	if (FillKind == 2)
		{
		if (pAlpha[m_pitch * y + x])
			{
			FillKind = 4;
			oldColor = pIndex[m_pitch * y + x];
			}
		}
	if (FillKind)
		Findruns2(x,y,x+1,y);
	else
		Findruns(x,y,x+1,y);
	if (findex)
		{
		m_bDirty = TRUE;	// needs display update
		m_bNeedFake = m_bModified = TRUE;
		pLayers[layer].flags = 3;//FLAG_ACTIVE | FLAG_MODIFIED;
#ifdef FILL_STACK
		delete [] m_pFillStack;
		m_pFillStack = 0;
		m_nFillCount = 0;
		m_nFillIndex = 0;
		m_nFillMax = 0;
#endif
		}
#ifdef FILL_LIMIT
	UINT cc = 0;
	UINT limit = m_width * m_height;
	if (m_dot_type == 1)
		limit /= 2;
	else if (m_dot_type == 2)
		limit /= 4;
	else if (m_dot_type == 3)
		limit /= 10;
	else if (m_dot_type == 4)
		limit /= 20;
#endif	
	for (;findex--;)
		{
		x = pStack[findex].x;
		y = pStack[findex].y;
		z = pStack[findex].z - 1;
//		q = pStack[findex].q;
//DPF("index:%d,x:%d,y:%d,c:%d",findex,x,y,z);
		y = m_height - 1 - y;
		if (x < m_minx) m_minx = x;
		if (z > m_maxx) m_maxx = z;
		if (y < m_miny) m_miny = y;
		if (y > m_maxy) m_maxy = y;
		y = m_height - 1 - y;
#ifdef FILL_STACK
		if (m_nFillCount >= m_nFillMax)
			{
			BYTE * pTemp;
			m_nFillMax += 10000;
			if (m_nFillMax)
				{
				pTemp = new BYTE[m_nFillMax * sizeof(FILLSTACK)];
				memmove(pTemp, m_pFillStack,m_nFillCount*sizeof(FILLSTACK));
				delete [] m_pFillStack;
				m_pFillStack = pTemp;
				}
			else
				{
				ASSERT(m_pFillStack == NULL);
				m_pFillStack = new BYTE[m_nFillMax * sizeof(FILLSTACK)];
				}
			}
		FILLSTACK * pFill = (FILLSTACK *)m_pFillStack;
		pFill += m_nFillCount;
		pFill->x1 = x;
		pFill->y = y;
		pFill->x2 = z;
		m_nFillCount++;
#endif

#ifdef FILL_LIMIT
		cc += z + 1 - x;
		if (cc > limit)
			break;
#endif
		int xx;
		if (FillKind == 1)
			{
			for (xx = x;xx <= z; xx++)
				pIndex[m_pitch*y+xx] = newColor;
			}
		else if (bErase == 1)
			{
			for (xx = x;xx <= z; xx++)
				pAlpha[m_pitch*y+xx] = 0;
			}
		else
			{
			for (xx = x;xx <= z; xx++)
				{
				pAlpha[m_pitch*y+xx] = 255;
				pIndex[m_pitch*y+xx] = newColor;
				}
			}
		if (FillKind)
			{
			if (Findruns2(x,y-1,z+1,y))
				break;
			if (Findruns2(x,y+1,z+1,y))
				break;
			}
		else
			{
			if (Findruns(x,y-1,z+1,y))
				break;
			if (Findruns(x,y+1,z+1,y))
				break;
			}
		}
	delete [] pStack;
	pStack = 0;
	DPF("c:%d,max:%d",m_nFillCount, m_nFillMax);
	m_nFillIndex = m_nFillCount;
//	EchoIt();
//	bAuto = FALSE;
//	return bResult;
	return 0;
}

int CLayers::Findruns(int x, int y, int z, int py)
{
	BYTE * pPntA;
	BYTE * pPntI;
	BYTE * pInkA;
	BYTE * pInkI;
	BYTE * pPInkA;
	WORD alpha,palpha;
	int left, right, first;
	if ((UINT)y >= m_height)
		return 0;
	LAYER * pLayers = (LAYER *)m_pLayers;
	pInkI = pLayers[m_nInk].pData;
	if (m_pControl)
		pInkI = m_pControl;
	pInkA = pPInkA = pInkI;
	pPntI = pPntA = pLayers[m_nPaint].pData;
	pPntI += m_pitch * m_height;
	pInkI += m_pitch * m_height;
	pInkA += m_pitch * y;
	pPInkA += m_pitch * py;
	pPntI += m_pitch * y;
	pInkI += m_pitch * y;
	pPntA += m_pitch * y;
	left = x;
//DPF("finding,x:%d,y:%d,z:%d",x,y,z);
	for (;left < z;)
		{
		alpha = pPInkA[left];
		if (pInkA[left] < alpha)
			{
			left++;
			continue;
			}
		if (bErase > 1)
			{
			if (pPntA[left])
				{
				left++;
				continue;
				}
			}
		else if (bErase)
			{
			if (!pPntA[left])
				{
				left++;
				continue;
				}
			}
		else
			{
			if (pPntA[left] && (pPntI[left] == newColor))
				{
				left++;
				continue;
				}
			}
		right = left;				// save starting point
		for (;left > 0;)			// expand to the left
			{
			alpha = pInkA[left];
			left--;
			if (left >= x)
				{
				palpha = pPInkA[left];
				if (alpha > palpha)
					alpha = palpha;
				}
			if (bErase > 1)
				{
				if (pPntA[left])
					{
					left++;
					break;
					}
				}
			else if (bErase)
				{
				if (!pPntA[left])
					{
					left++;
					break;
					}
				}
			else
				{
				if ((pPntA[left]) && (pPntI[left] == newColor))
					{
					left++;
					break;
					}
				}
			if (pInkA[left] < alpha)
				{
				if (left < x)
					{
					left++;
					break;
					}
				if (Try(left,y,left+1,py))
					{
					left++;				// restore last good one
					break;
					}
				}
			}
		first = left;
		left = right;				// back to where we started
//		for (;(UINT)(left+1) < m_width;)	// expand right
		for (;;)	// expand right
			{
			if ((UINT)(left + 1) >= m_width)
				{
				left++;
				break;
				}
			alpha = pInkA[left];
			left++;
			if (left < z)
				{
				palpha = pPInkA[left];
				if (alpha > palpha)
					alpha = palpha;
				}
			if (bErase > 1)
				{
				if (pPntA[left])
					break;
				}
			else if (bErase)
				{
				if (!pPntA[left])
					break;
				}
			else
				{
				if ((pPntA[left]) && (pPntI[left] == newColor))
					break;
				}
			if (pInkA[left] < alpha)
				{
				if (left >= z)
						break;
					if (Try(left,y,left-1,py))
						break;
				}
			}
		if ((findex + 1) < LIMIT)
			{
			x = first;
			pStack[findex].x = first;
			pStack[findex].y = y;
			pStack[findex].z = left;
			findex++;
			}
		else
			{
DPF("foverflow1");
			return 1;
			}
		}
	return 0;
}


int CLayers::Findruns2(int x, int y, int z, int py)
{
	BYTE * pPntA;
	BYTE * pPntI;
	BYTE * pInkA;
	BYTE * pInkI;
	BYTE * pTonA;
	BYTE * pTonI;
	int left, right, first;
	ASSERT(FillKind > 0);
	if ((UINT)y >= m_height)
		return 0;
	LAYER * pLayers = (LAYER *)m_pLayers;
//	if (FillKind == 1)
		{
		pInkA = pLayers[m_nInk].pData;
		pInkA += m_pitch * y;
		pInkI = pInkA + m_pitch * m_height;
		}
//	else if (FillKind == 3)
		{
		pPntA = pLayers[m_nPaint].pData;
		pPntA += m_pitch * y;
		pPntI = pPntA + m_pitch * m_height;
		}
//	if (FillKind > 1)
		{
		pTonA = pLayers[m_nCurLayer].pData;
		pTonA += m_pitch * y;
		pTonI = pTonA + m_pitch * m_height;
		}
//	if (FillKind == 3)
//		pPntA = pTonA + m_pitch * m_height;
	left = x;
	for (;left < z;)
		{
		if (FillKind == 1)
			{
			if (!pInkA[left] || (pInkI[left] == newColor))
				{
				left++;
				continue;
				}
			}
		else if (FillKind == 3)
			{
//			if (!pTonA[left] || (pTonI[left] == newColor) || !pPntA[left])
			if (!pPntA[left])
				{
				left++;
				continue;
				}
			if (pTonA[left] && (pTonI[left] == newColor))
				{
				left++;
				continue;
				}
			}
		else if (FillKind == 4)
			{
			if (!pTonA[left] || (pTonI[left] != oldColor))
				{
				left++;
				continue;
				}
			}
		else if (bErase)
			{
			if (!pTonA[left])
				{
				left++;
				continue;
				}
			}
		else
			{
			if (pTonA[left])
				{
				if (pTonI[left] != newColor)
					{
					if (!magicx && (left > 0) && ((UINT)y < m_height))
						{
						magicx = left;
						magicy = y;
						magicc = pTonI[left];
						}
					}
				left++;
				continue;
				}
			}
		right = left;				// save starting point
		for (;left > 0;)			// expand to the left
			{
			left--;
			if (FillKind == 1)
				{
				if (!pInkA[left] || (pInkI[left] == newColor))
					break;
				}
			else if (FillKind == 3)
				{
				if (!pPntA[left])
					break;
				if (pTonA[left] && (pTonI[left] == newColor))
					break;
				}
			else if (FillKind == 4)
				{
				if (!pTonA[left] || (pTonI[left] != oldColor))
					break;
				}
			else if (bErase)
				{
				if (!pTonA[left])
					break;
				}
			else
				{
				if (pTonA[left])
					{
					if (pTonI[left] != newColor)
						{
						if (!magicx && (left > 0) && ((UINT)y < m_height))
							{
							magicx = left;
							magicy = y;
							magicc = pTonI[left];
							}
						}
					break;
					}
				}
			}
		first = left+1;
		left = right;				// back to where we started
		for (;;)	// expand right
			{
			if ((UINT)(left + 1) >= m_width)
				{
				left++;
				break;
				}
			left++;
			if (FillKind == 1)
				{
				if (!pInkA[left] || (pInkI[left] == newColor))
					break;
				}
			else if (FillKind == 3)
				{
				if (!pPntA[left])
					break;
				if (pTonA[left] && (pTonI[left] == newColor))
					break;
				}
			else if (FillKind == 4)
				{
				if (!pTonA[left] || (pTonI[left] != oldColor))
					break;
				}
			else if (bErase)
				{
				if (!pTonA[left])
					break;
				}
			else
				{
				if (pTonA[left])
					{
					if (pTonI[left] != newColor)
						{
						if (!magicx && (left > 0) && ((UINT)y < m_height))
							{
							magicx = left;
							magicy = y;
							magicc = pTonI[left];
							}
						}
					break;
					}
				}
		}
		if ((findex + 1) < LIMIT)
			{
			x = first;
			pStack[findex].x = first;
			pStack[findex].y = y;
			pStack[findex].z = left;
			findex++;
			}
		else
			{
DPF("foverflow2");
			return 1;
			}
		}
	return 0;
}


UINT CLayers::FillStack(UINT pos)
{
	m_zx = 0;
	if (pos == -1)
		return m_nFillCount;
	else if (pos == -2)
		{
		delete [] m_pFillStack;
		m_pFillStack = 0;
		m_nFillMax = m_nFillCount = 0;
		return 0;
		}
	if (!m_nFillCount || !m_pFillStack)
		return 0;
	if (m_nFillIndex == pos)
		return pos;
	LAYER * pLayers = (LAYER *)m_pLayers;
	UINT layer;
	if ((FillKind == 2) || (FillKind == 3) || (FillKind == 4))
		layer = m_nCurLayer;
	else if (FillKind)
		layer = m_nInk;
	else
		layer = m_nPaint;
	BYTE * pAlpha = pLayers[layer].pData;
	BYTE * pIndex = pAlpha + m_pitch * m_height;
	BYTE * pUndoa = LastUndo();
	BYTE * pUndoi = pUndoa + m_pitch * m_height;
	int x, y, z;
	
	m_minx = m_width;
	m_maxx = 0;
	m_miny = m_height;
	m_maxy = 0;
	if (m_nFillIndex > pos)
		{
		for (;m_nFillIndex && (m_nFillIndex > pos);)
			{
			m_nFillIndex--;
			FILLSTACK * pFill = (FILLSTACK *)m_pFillStack;
			pFill += m_nFillIndex;
			x = pFill->x1;
			y = pFill->y;
			z = pFill->x2;
			y = m_height - 1 - y;
			if (x < m_minx) m_minx = x;
			if (z > m_maxx) m_maxx = z;
			if (y < m_miny) m_miny = y;
			if (y > m_maxy) m_maxy = y;
			y = m_height - 1 - y;
			if (FillKind == 1)
				{
				for (;x <= z; x++)
					pIndex[m_pitch*y+x] = pUndoi[m_pitch*y+x];
				}
			else if (bErase == 1)
				{
				for (;x <= z; x++)
					pAlpha[m_pitch*y+x] = pUndoa[m_pitch*y+x];
				}
			else
				{
				for (;x <= z; x++)
					{
					pAlpha[m_pitch*y+x] = pUndoa[m_pitch*y+x];
					pIndex[m_pitch*y+x] = pUndoi[m_pitch*y+x];
					}
				}
			}
		}
	else
		{
		for (;(m_nFillIndex < m_nFillCount) && (m_nFillIndex < pos);m_nFillIndex++)
			{
			FILLSTACK * pFill = (FILLSTACK *)m_pFillStack;
			pFill += m_nFillIndex;
			x = pFill->x1;
			y = pFill->y;
			z = pFill->x2;
			y = m_height - 1 - y;
			if (x < m_minx) m_minx = x;
			if (z > m_maxx) m_maxx = z;
			if (y < m_miny) m_miny = y;
			if (y > m_maxy) m_maxy = y;
			y = m_height - 1 - y;
			if (FillKind == 1)
				{
				for (;x <= z; x++)
					pIndex[m_pitch*y+x] = newColor;
				}
			else if (bErase == 1)
				{
				for (;x <= z; x++)
					pAlpha[m_pitch*y+x] = 0;
				}
			else
				{
				for (;x <= z; x++)
					{
					pAlpha[m_pitch*y+x] = 255;
					pIndex[m_pitch*y+x] = newColor;
					}
				}
			}
		}
	m_bDirty = TRUE;	// needs display update

	return m_nFillIndex;
}
