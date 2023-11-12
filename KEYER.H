#ifndef _KEYER_H_
#define _KEYER_H_
class CColorKey
{
public:
	CColorKey();
	~CColorKey();
	void Process(BYTE * pDst, BYTE * pSrc, UINT w, UINT h, UINT d,
					int r, int g, int b, int ww, int flags);
protected:
//	void lab(int & l, int & a, int &bb, int r, int g, int b);
//	void hlab(int & l, int & a, int &bb, int r, int g, int b);
	void xlate(int & j, int & k, int &l, int r, int g, int b);
	void Setup();
//	UINT m_r;
//	UINT m_g;
//	UINT m_b;
//	UINT m_x;
	UINT m_kind;
//	BOOL m_bInvert;
	int * m_p1;
	int * m_p2;
	int * m_p3;
};

#endif
