#ifndef _CCELL_H_
#define _CCELL_H_
#include "MyObj.h"

class CCell : public CMyObj
{
public:
static DWORD Duplicate(CMyIO * pIO, DWORD key, bool bKeepName = 0);
	CCell(CMyIO * pIO);
	~CCell();
	DWORD Size(DWORD count);
	DWORD Select(int which);
	DWORD Insert(int which);
#ifdef FBVER7
	DWORD ChangeKind(DWORD key, int which);
#endif
	DWORD DeleteLayer(int which = 99); // 99 for all layers
	void  Name(LPSTR name, bool bPut = FALSE);
	DWORD Get(CFile & sf);
	DWORD Put(CFile & sf);
	DWORD Flags(UINT v = NEGONE);
enum layers {
	LAYER_THUMB,		// 0
	LAYER_INK,			// 1
	LAYER_PAINT,		// 2	
	LAYER_SHADOW,		// 3
	LAYER_TINT,			// 4	
	LAYER_MONO,			// 5
	LAYER_GRAY,			// 6
#ifdef FBVER7
	LAYER_OLD_BG,		// 7
	LAYER_OLD_OVERLAY,	// 8
#else
	LAYER_BG,		// 7
	LAYER_OVERLAY,	// 8
#endif
	LAYER_MATTE0,		// 9
	LAYER_MATTE1,		// 10
	LAYER_MATTE2,
	LAYER_MATTE3,
	LAYER_MATTE4,
	LAYER_MATTE5,
	LAYER_MATTE6,
	LAYER_MATTE7,
	LAYER_MATTE8,
	LAYER_MATTE9, 
	LAYER_XTRA,			// 18
	LAYER_COLOR};
	int Read();
	int Write();
#ifdef MYBUG
	void Display();
#endif
protected:
	UINT	m_width;
	UINT	m_height;
};
#endif
