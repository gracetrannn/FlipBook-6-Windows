#ifndef _INC_DIB
#define _INC_DIB
#include "fbqt.h"
/* DIB constants */
#define PALVERSION   0x300

/* Dib Header Marker - used in writing DIBs to files */
#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')

/* DIB Macros*/
#define RECTWIDTH(lpRect)     ((lpRect)->right - (lpRect)->left)
#define RECTHEIGHT(lpRect)    ((lpRect)->bottom - (lpRect)->top)

// WIDTHBYTES performs DWORD-aligning of DIB scanlines.  The "bits"
// parameter is the bit count for the scanline (biWidth * biBitCount),
// and this macro returns the number of DWORD-aligned bytes needed
// to hold those bits.
//#ifndef WIDTHBYTES
#define ZWIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)
//#endif
class CDib : public CObject
{
	DECLARE_DYNAMIC(CDib)

// Constructors
public:
	CDib();

// Attributes
//protected:
	LPBYTE m_pBits;
	LPBITMAPINFO m_pBMI;
public:	
	CPalette* m_pPalette;

public:
	DWORD Width()     const;
	DWORD Height()    const;
	DWORD BitCount()    const;
	WORD  NumColors() const;
	BOOL  IsValid()   const { return (m_pBMI != NULL); }

// Operations
public:
	BOOL  Paint(HDC, LPRECT, LPRECT) const;
	HGLOBAL CopyToHandle()           const;
	DWORD Export(LPCSTR name, FBFileFormat fkind);
	DWORD Import(LPCSTR name);
	DWORD Importer(void * pData, UINT ssize);
	DWORD Create(UINT w, UINT h, UINT d);
	DWORD ReadFromBMI(LPBITMAPINFOHEADER lpBI, UINT depth = 0);
	DWORD ReadFromHandle(HGLOBAL hGlobal);
	DWORD ReadFromClipboard(UINT depth = 0);
	DWORD ReadFromDib(CDib * pDib);
	DWORD Merge(CDib * pRef, CDib * pGrab);
	DWORD Convert(UINT new_dpeth);
	DWORD Scale(UINT new_w, UINT new_h);
	BOOL  AdjustPalette(UINT index, UINT r, UINT G, UINT B);
	void Invalidate() { Free(); }

	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CDib();

protected:
	DWORD SaveIt(CFile& file, UINT kind = 0)   const;
	DWORD ReadIt(CFile& file);
	BOOL  CreatePalette();
	WORD  PaletteSize() const;
	DWORD ReadTarga(CFile& file);
	DWORD SaveTarga(CFile& file) const;
	void Free();

public:
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CDib& operator = (CDib& dib);
};

#endif //!_INC_DIB
