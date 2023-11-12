
typedef struct {
	UINT iw;
	UINT ih;
	UINT cx;
	UINT cy;
	UINT xdpi;	
	UINT ydpi;
	int num;
	int den;
	UINT bestr;
	BYTE * pTemp;
	UINT ow;
	UINT oh;
	UINT ocy;
	BOOL bColor;
} SKEWINFO;



BYTE * MakeGray(SKEWINFO &Info, LPBITMAPINFOHEADER  lpbi);
BOOL  FindCircle(SKEWINFO & info);
BOOL  FindSkew(SKEWINFO & info);

void RotateIt(SKEWINFO & info, BYTE * pDst, BYTE * pSrc);

