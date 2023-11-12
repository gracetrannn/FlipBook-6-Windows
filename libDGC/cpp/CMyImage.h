#ifndef _CMYIMAGE_H_
#define _CMYIMAGE_H_
#include "MyObj.h"

class CMyImage : public CMyObj
{
public:
	CMyImage(CMyIO * pIO);
	~CMyImage();
	UINT Width() { return m_width;};
	UINT Height() { return m_height;};
	UINT Depth() { return m_depth;};
    void SetDepth(UINT depth) { };
	void Setup(UINT width, UINT height, UINT depth, UINT flags = 0)
		{m_width = width;m_height=height;m_depth=depth;m_flags = flags;};
	int Read(HPBYTE buf, DWORD size = 0);
	int Write(HPBYTE buf);
	int Compress(HPBYTE buf, DWORD size);
	int DeCompress(HPBYTE buf, DWORD size);
#ifdef MYBUG
	void Display();
#endif
protected:
	UINT	m_width;
	UINT	m_height;
	UINT	m_depth;
	UINT	m_flags;
};
#endif
