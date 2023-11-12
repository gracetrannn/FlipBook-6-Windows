#pragma once

#include "NScaler.h"

class CSScaler : public CNScaler
{
public:
	int Custom(HPBYTE hpDst, UINT opitch,
		HPBYTE hpSrc, UINT ipitch, BYTE* pals, UINT d = 8, UINT rot = 0);
	UINT m_aw;
	UINT m_ah;
protected:
	HPBYTE m_dst;
	HPBYTE m_src;
	UINT m_ipitch;
	UINT m_opitch;
	UINT m_depth;
	UINT m_rot;
	BYTE m_gray[256];
	//	BYTE gray(BYTE * p) { return (BYTE) (( 11 * (WORD)*p++ + 
	//							59 * (WORD)*p++ + 30 * (WORD)*p++) / 100);};
	virtual int get_line();
	virtual int put_line();
};

class CCScaler : public CNScaler
{
public:
	int Custom(HPBYTE hpDst, UINT opitch,
		HPBYTE hpSrc, UINT ipitch, BYTE* pals, UINT id, UINT rot = 0);
	UINT m_aw;
	UINT m_ah;
protected:
	HPBYTE m_dst;
	HPBYTE m_src;
	UINT m_ipitch;
	UINT m_opitch;
	UINT m_depth;
	UINT m_rot;
	UINT m_key;
	BOOL m_bCvt24;
	BYTE* m_pals;
	virtual int get_line();
	virtual int put_line();
};