#ifndef _NSCALER_H_
#define _NSCALER_H_

#include "CommonDefs.h"

/*
//
//	example of derived class using overriden I/O
//
class CMyScaler : public CNScaler
{
public:
	int Custom(CRixFile * pofile, CRixFile *  pifile);
protected:
	CRixFile * m_pifile;
	CRixFile * m_pofile;
	virtual int get_line();
	virtual int put_line();
};

int CMyScaler::Custom(CRixFile * pofile, CRixFile * pifile)
{
	m_pifile = pifile;
	m_pofile = pofile;
	return 0;
}

int CMyScaler::get_line()
{
	m_pifile->GetLine(m_ibuf);
}

int CMyScaler::put_line()
{
	m_pofile->PutLine(m_obuf);
}

int CNScaler::make_line()
{
	for (UINT i = 0; i < m_op; i++)
		m_obuf[i] = (BYTE)(m_acc[i] / m_divisor);
	return put_line();
}


*/
class CNScaler
{
public:
	CNScaler();
	~CNScaler();
	bool Init(UINT iw, UINT ih, UINT idd, UINT ow, UINT oh, UINT nHold = 0);
	int Copy();
	int PullLine(BYTE * buf);
	UINT Offx() { return m_offx;};
	UINT Offy() { return m_offy;};
protected:
	void Clear();
	void clearacc();
	int hline(UINT f, bool skip = 0);
	void hline0(UINT f);
	void hline1(UINT f);
	void hline3(UINT f);
	void hline4(UINT f);

	virtual int get_line();
	virtual int put_line();
	virtual int make_line();
	bool m_bMono;
	UINT m_iw;
	UINT m_ih;
	UINT m_ip;
	UINT m_id;
	UINT m_ow;
	UINT m_oh;
	UINT m_op;
	UINT m_iy;
	UINT m_offx;
	UINT m_offy;
	UINT m_acty;
	UINT m_cnt;
	UINT m_oy;
	DWORD m_divisor;
	DWORD m_adj;
	BYTE * m_ibuf;
	BYTE * m_obuf;
	DWORD * m_acc;
};

class CGScaler : public CNScaler
{
public:
	int Custom(HPBYTE hpDst, HPBYTE hpSrc, UINT pitch, UINT ip = 0);
protected:
	HPBYTE m_dst;
	HPBYTE m_src;
	UINT m_pitch;
	UINT m_ipitch;
	virtual int get_line();
	virtual int put_line();
};

#endif
