#ifndef _LEVEL_H_
#define _LEVEL_H_

#include "MyObj.h"
#include "CNewPals.h"
#include <functional>

class CLevelTable;
class CLevel : public CMyObj
{
public:
	CLevel(CMyIO * pIO);
	DWORD Size(DWORD count);
	DWORD Select(int Frame, bool bHold = FALSE);
	void MinMax(UINT & min, UINT & max, UINT Frame);
	UINT  Before(UINT * pList, UINT max, UINT Frame);
	UINT  Next(UINT Frame);
	DWORD Insert(int Frame, DWORD key = 0, bool bSwap = 0);
	DWORD DeleteCell(int Frame);
	DWORD Flags(DWORD v = NEGONE, bool bInit = 0);
	UINT	Cleanup(UINT frames);
//	UINT Pals(BYTE * pPals, CScene * pScene);
	UINT PalIndex(UINT v = NEGONE);
	CLevelTable * Table();  // used for read only
	UINT Table(CLevelTable * pTable, bool bCheck);
	void  Name(LPSTR name, bool bPut = FALSE);
//	void  PalName(LPSTR name, bool bPut = FALSE);
	void  ModelName(LPSTR name, bool bPut = FALSE);
	int	 MoveFrames(UINT From, UINT To, UINT Count = 0);
	int zInsertFrame(int Start, int Count = 1);
	int Read();
	int Write();
	void setSceneClosures(std::function<UINT(BYTE*, BYTE*)> forgePalsClosure, std::function<CNewPals* (UINT)> palettePtrClosure);
#ifdef FBVER7
	UINT CellCount();
#endif
#ifdef MYBUG
	void Display();
#endif

private:
	std::function<UINT(BYTE*, BYTE*)> m_forgePalsClosure;
	std::function<CNewPals* (UINT)> m_palettePtrClosure;
};
#endif
