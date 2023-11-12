#pragma once

#include "CSceneBase.h"

class CScene : public CSceneBase {
public:
	CScene(CMyIO* pIO, std::unique_ptr<ResourceLoader> resourceLoader) : CSceneBase(pIO, std::move(resourceLoader)) {}
	void ApplyFrameCount(HPBYTE hpDst, UINT frame);
	void CompositeFrame(BYTE* pBuf, UINT StartLevel, UINT EndLevel, UINT Frame, BOOL bBroadcast);
	void CompositeFrame32(BYTE* pBuf, UINT StartLevel, UINT EndLevel, UINT Frame, BOOL b32);
	void CompositeCacheFrame(UINT Frame, BOOL bBroadcast);
	int CheckComposite(UINT Frame = NEGONE, UINT Force = 0, BOOL bBrodcast = FALSE);
	UINT ConvertGray(HPBYTE hpDst, LPBITMAPINFOHEADER  lpbi, UINT Rotation, UINT nHold);
	UINT zCreateCell(UINT Frame, UINT Level, LPBITMAPINFOHEADER  lpbi, UINT Rotation, bool bMakeMono, UINT nHold);
	void MakeMono(HPBYTE hpDst);
	void MakeGray(HPBYTE hpDst);
	int CloseIt();
	UINT ConvertColor(HPBYTE hpDst, LPBITMAPINFOHEADER lpbi, UINT Od, UINT Rotation, UINT nFit);
};
