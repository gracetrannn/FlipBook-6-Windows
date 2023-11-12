#ifndef _FLOAT_H_
#define _FLOAT_H_

#include "CommonDefs.h"

class CNewPals;
class CFloat
{
public:
	CFloat();
	~CFloat();
	void Make(BYTE * pData);
	void Make(RECT & rect, BYTE * pData);
	BYTE * Data() { return m_pCur;};
	UINT Hit(CPoint point, bool Shift = 0, bool bRotate = 0);
	UINT StartDrag(CPoint point);
	UINT DragIt(CPoint point);
	void GetPoints(POINT * lpt, UINT cnt);
	void GetRect(RECT&rc);
	bool Rotated(bool bValue = 0, bool bSet = 0);
	void ShiftOn(bool bShift) { m_bShiftOn = bShift;};
	void PalMap(BYTE *pmap, CNewPals * pPals);
	UINT m_q;	// 0 is ink alpha, 1 is ink and paint
	bool m_bNoMapPalette;
	bool m_bHasName;
	char m_name[16];
protected:
	void FillIn(UINT w, UINT h, UINT d, BYTE * pData);
	void SetOrig(UINT w, UINT h, UINT d, BYTE * pSrc);
	void Regen();
	void RotateIt();
	void RotateOne(BYTE * pDst, BYTE * pSRc, bool bAvg, bool bMask);
	bool m_bDisplayed;
	bool m_bActive;
	bool m_bRotated;
	bool m_bShiftOn;
	bool m_bAspectHold;
	UINT m_code;	// hit code
	int m_x;
	int m_y;
	int m_ox;
	int m_oy;
	int m_ow;
	int m_oh;
	int m_px;
	int m_py;
	UINT m_w;		// original
	UINT m_h;
	BYTE * m_pOrig;
	BYTE m_pals[1024];
	BYTE m_map[256];
	UINT m_curw;
	UINT m_curh;		// current sizes
	UINT m_diag;		// w and h when rotated
	double m_angle;
	BYTE * m_pCur;
};
#endif
