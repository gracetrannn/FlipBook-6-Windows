#ifndef _MYOBJ_H_
#define _MYOBJ_H_

#include "CommonDefs.h"

enum kinds {
		KIND_BAD,
		KIND_SCENE,
		KIND_LEVELS,
		KIND_LEVEL,
		KIND_CELL,
		KIND_LEVEL_PAL,
		KIND_LEVEL_MODEL,
		KIND_CAMERA,
		KIND_IMAGE,
		KIND_LEVEL_MATTE,
		KIND_LEVEL_NEW,
		KIND_PALETTE};

class CMyIO;

class CMyObj
{
public:
	CMyObj(CMyIO * pIO);
	~CMyObj();
	int Read();
	int Write();
	DWORD GetKey() {return m_key;};
	void SetKey(DWORD key) {m_key = key;};
	DWORD CurSize() {return m_size;};
protected:
	CMyIO	*m_pIO;
	DWORD	m_key;
	DWORD 	m_kind;
	DWORD	m_size;
	BYTE *	m_pData;
};
#endif
