#ifndef _LEVELS_H_
#define _LEVELS_H_
#include "MyObj.h"

class CLevels : public CMyObj
{
public:
	CLevels(CMyIO * pIO);
	~CLevels();
	DWORD Size(DWORD count);
	DWORD Select(int Level);
	DWORD Insert(int Level);
	DWORD zCount(int v = -1);
	bool Check();
	int InsertLevel(int Start, int Count = 1);
	int Read();
	int Write();
#ifdef MYBUG
	void Display();
#endif
protected:
};
#endif
