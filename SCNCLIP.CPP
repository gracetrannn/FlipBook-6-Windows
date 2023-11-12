#include "stdafx.h"
#include "cscene.h"
#include "ccell.h"
#include "myio.h"
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//#define KEY_CLIP 13

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

//void CScene::ClipClear(UINT key)
//{
//}

//void CScene::ClipClear(UINT *pKeys, UINT count)
//{
//}


