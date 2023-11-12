#include "stdafx.h"
#include "cscene.h"
#include "ccell.h"
#include "myio.h"
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//#define KEY_CLIP 13

void CSceneBase::ClipInfo(UINT & w, UINT &h)
{
	w = m_pClipBoard[0];
	h = m_pClipBoard[1];
}
UINT CSceneBase::ClipSet(UINT x, UINT y, UINT RecNbr)
{
DPF("clipset,x:%d,y:%d,k:%d",x,y,RecNbr);
	if (!m_pClipBoard) return 1;
	if (x >= m_pClipBoard[0]) return 2;
	if (y >= m_pClipBoard[1]) return 3;
	m_pClipBoard[y * m_pClipBoard[0] + x + 3] = RecNbr;
	return 0;
}

UINT CSceneBase::ClipGet(UINT x, UINT y)
{
	if (!m_pClipBoard) return 0;
	if (x >= m_pClipBoard[0]) return 0;
	if (y >= m_pClipBoard[1]) return 0;
	return m_pClipBoard[y * m_pClipBoard[0] + x + 3];
}

bool CSceneBase::IsLinked(UINT key)
{
	UINT i = FindRef(key);
	return i >= m_links ? 0 : 1;
}

#ifdef MYBUG
void CSceneBase::ShowLinks()
{
	DPF("scene links:%d",m_links);
	UINT i;
	for (i = 0; i < m_links; i++)
		{
		DPF("i:%2d:%5d,%d",i,m_pLinks[i].dwKey, m_pLinks[i].dwCount);
		}
	if (m_pClipBoard)
		{
		UINT w = m_pClipBoard[0];
		UINT h = m_pClipBoard[1];
		DPF("Clip,w:%d,h:%d",w,h);
		UINT x, y;
		for (x  = 0; x < w; x++)
		for (y  = 0; y < h; y++)
			{
			DPF("x:%d,y:%d,k:%5d",x,y,m_pClipBoard[y*w+x+3]);
			}
		}
}
#endif

UINT CSceneBase::ClipCount(UINT key)
{
	UINT cnt = 0;
	if (m_pClipBoard)
		{
		UINT w = m_pClipBoard[0];
		UINT h = m_pClipBoard[1];
		UINT x, y;
		for (x  = 0; x < w; x++)
		for (y  = 0; y < h; y++)
			{
			DPF("x:%d,y:%d,k:%d",x,y,m_pClipBoard[y*w+x+3]);
			if (m_pClipBoard[y * w + x + 3] == key)
				cnt++;
			}
		}
	return cnt;
}

UINT CSceneBase::ClipFinish()
{
//	UINT w = m_pClipBoard[0];
//	UINT h = m_pClipBoard[1];
//	if (m_pIO)
//		m_pIO->PutRecord(m_pClipBoard,(3 + w * h) * sizeof(UINT), KEY_CLIP);
	return 0;
}

//void CScene::ClipClear(UINT key)
//{
//}

//void CScene::ClipClear(UINT *pKeys, UINT count)
//{
//}

