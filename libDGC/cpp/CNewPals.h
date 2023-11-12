#ifndef _CNEWPALS_H_
#define _CNEWPALS_H_

#include "CommonDefs.h"

#define NPAL_FLAG_HSYM 16
#define NPAL_FLAG_VSYM 32
#define NPAL_FLAG_HAS_ALPHA 64
#define NPAL_FLAG_NORGB 128
#define NPAL_FLAG_INTERNAL 256

enum GradientKind : UINT {
	RADIAL = 0, LINEAR = 1
};

struct GRADENTRY {
	UINT kind;
	union {
	BYTE rgb1[4];
	COLORREF c1; };
	union {
	BYTE rgb2[4];
	COLORREF c2;};			// grad colors
	POINT	p1,p2;			// reference for grads		
};

enum ColorKind: UINT {
	SIMPLE = 0, GRADIENT = 1, TEXTURE = 2
};

typedef struct ATTRIBUTE_PACKED {
	UINT	kind;			// color kind, 0 is simple, 1 is grad
	union {
		BYTE rgbo[4];
		COLORREF rgb;
	};
	GRADENTRY grad;
	UINT	cname;			// index to color name
	UINT	fname;			// index to file name
	UINT 	w,h,flags;			// width and height of pattern or texture
	UINT	size;
	BYTE *  pTexture;		// data assigned to texture or pattern
} NPALENTRY;

class CMyIO;

class CNewPals
{
public:
	CNewPals();
	~CNewPals();
	int ReadWrite(CMyIO * pIO, DWORD key, bool bPut);
	int Write(LPCSTR name);
	int Read(LPCSTR name);
	void Assign(UINT Index, BYTE r, BYTE g, BYTE b, BYTE o, bool KeepKind=0);
	void Assign(UINT Index, COLORREF crColor);
	void Legacy(BYTE * pOldPals, LPCSTR FileName = 0);
//	void Init(BYTE * pOldPals, LPCSTR Name, bool bExternal);
	void Initial(UINT Index, BYTE r, BYTE g, BYTE b, BYTE o);
	void Color(BYTE * Pals, UINT index, UINT x = 0, UINT y = 0);
	COLORREF Color(UINT index, UINT x = 0, UINT y = 0);
	UINT Kind(UINT index) { return pals[index].kind;};
	UINT Kind(UINT index, UINT kind);
	COLORREF ColorRef(UINT index) { return pals[index].rgb;};
	UINT Red(UINT index) { return pals[index].rgbo[0];};
	UINT Green(UINT index) { return pals[index].rgbo[1];};
	UINT Blue(UINT index) { return pals[index].rgbo[2];};
	UINT Alpha(UINT index) { return pals[index].rgbo[3];};
	UINT Flags(UINT index) { return pals[index].flags;};
	UINT Flags(UINT index, UINT flags) { return pals[index].flags = flags;};
	bool Simple(bool bForce = 0);// { return m_bSimple;}
	bool GetSetGrad(UINT Index, GRADENTRY & grad, bool bSet); 
	bool LoadTexture(UINT index,LPCSTR name);
	bool Compare(CNewPals * pPals);// {return Compare(pPals->pals);};
	bool Compare(CNewPals & zpals);// {return Compare((NEWPALS)zpals);};
	CNewPals * Clone(LPCSTR Name = 0);
	LPCSTR FileName(UINT index, int code = 0);
	bool CompareOne(NPALENTRY & pal, UINT index);
	void CleanUp();
	void SetPalName(LPCSTR Name);
	void SetFileName(LPCSTR Name);
	LPCSTR GetPalName() { return (LPCSTR)&m_pal_name;};
	LPCSTR GetFileName() { return (LPCSTR)&m_file_name;};
	int  Linked(int v); // 0 is test, 1 is compare, ret 1 if no, 2 is misma
//	bool Protected(int v = -1);
//	bool IfShared(int v = -1);
//	bool Dirty(int v = -1);
	UINT PaletteIO(LPCSTR name, bool bWrite);
	void SaveEntry(UINT index);
	void RestEntry(int code);// 0 is just empty, 1 is rest, 2 is both
	int  ProcessTextureData(void * pData, UINT ssize);
protected:
	UINT m_ctindex;
	UINT m_flags;
	char m_pal_name[30];
	char m_file_name[300];
	NPALENTRY pals[256];  // new palette structures
	NPALENTRY m_save;
	UINT	m_save_index;
	UINT	m_nNameSize;
	BYTE * m_pNames;
	UINT PutPaletteFile(LPCSTR name);
	UINT GetPaletteFile(LPCSTR name);
	UINT CompareFile();
	void CleanUpIndex(UINT Index);
	void DoGrad(BYTE * pals, UINT index,int x, int y);
	UINT AddName(LPCSTR name);
	UINT GetTextureSize();
//#ifdef FBMAC
	void SwapEntries( NPALENTRY * pp);
//#endif
};


#endif
