#include "stdafx.h"
#include "dib.h"
#include <windowsx.h>
#include <afxadv.h>
#include <io.h>
#include <errno.h>
#include "nscaler.h"
#include "fbqt.h"
#include "utils.h"
/////////////////////////////////////////////////////////////////////////////
// CDib

IMPLEMENT_DYNAMIC(CDib, CObject)

CDib::CDib()
{
	m_pBMI = NULL;
	m_pBits = NULL;
	m_pPalette = NULL;
}

CDib::~CDib()
{
	Free();
}

void CDib::Free()
{
	// Make sure all member data that might have been allocated is freed.
#ifdef SEPARATE
	if (m_pBits)
	{
		MyFree(m_pBits);
		m_pBits = NULL;
	}
#endif
	if (m_pBMI)
	{
		MyFree(m_pBMI);
		m_pBMI = NULL;
	}
	if (m_pPalette)
	{
		m_pPalette->DeleteObject();
		delete m_pPalette;
		m_pPalette = NULL;
	}
}

/*************************************************************************
 *
 * Paint()
 *
 * Parameters:
 *
 * HDC hDC          - DC to do output to
 *
 * LPRECT lpDCRect  - rectangle on DC to do output to
 *
 * LPRECT lpDIBRect - rectangle of DIB to output into lpDCRect
 *
 * CPalette* pPal   - pointer to CPalette containing DIB's palette
 *
 * Return Value:
 *
 * BOOL             - TRUE if DIB was drawn, FALSE otherwise
 *
 * Description:
 *   Painting routine for a DIB.  Calls StretchDIBits() or
 *   SetDIBitsToDevice() to paint the DIB.  The DIB is
 *   output to the specified DC, at the coordinates given
 *   in lpDCRect.  The area of the DIB to be output is
 *   given by lpDIBRect.
 *
 ************************************************************************/

BOOL CDib::Paint(HDC hDC, LPRECT lpDCRect, LPRECT lpDIBRect) const
{
	if (!m_pBMI)
		return FALSE;

	HPALETTE hPal = NULL;           // Our DIB's palette
	HPALETTE hOldPal = NULL;        // Previous palette

	// Get the DIB's palette, then select it into DC
	if (m_pPalette != NULL)
	{
		hPal = (HPALETTE) m_pPalette->m_hObject;

		// Select as background since we have
		// already realized in forground if needed
		hOldPal = ::SelectPalette(hDC, hPal, TRUE);
	}

	/* Make sure to use the stretching mode best for color pictures */
	::SetStretchBltMode(hDC, COLORONCOLOR);

	/* Determine whether to call StretchDIBits() or SetDIBitsToDevice() */
	BOOL bSuccess;
	if ((RECTWIDTH(lpDCRect)  >= RECTWIDTH(lpDIBRect)) &&
	   (RECTHEIGHT(lpDCRect) >= RECTHEIGHT(lpDIBRect)))
		bSuccess = ::SetDIBitsToDevice(hDC,        // hDC
								   lpDCRect->left,             // DestX
								   lpDCRect->top,              // DestY
								   RECTWIDTH(lpDCRect),        // nDestWidth
								   RECTHEIGHT(lpDCRect),       // nDestHeight
								   lpDIBRect->left,            // SrcX
								   (int)Height() -
									  lpDIBRect->top -
									  RECTHEIGHT(lpDIBRect),     // SrcY
								   0,                          // nStartScan
								   (WORD)Height(),             // nNumScans
								   m_pBits,                    // lpBits
								   m_pBMI,                     // lpBitsInfo
								   DIB_RGB_COLORS);            // wUsage
   else
	  bSuccess = ::StretchDIBits(hDC,            // hDC
							   lpDCRect->left,               // DestX
							   lpDCRect->top,                // DestY
							   RECTWIDTH(lpDCRect),          // nDestWidth
							   RECTHEIGHT(lpDCRect),         // nDestHeight
							   lpDIBRect->left,              // SrcX
							   lpDIBRect->top,               // SrcY
							   RECTWIDTH(lpDIBRect),         // wSrcWidth
							   RECTHEIGHT(lpDIBRect),        // wSrcHeight
							   m_pBits,                      // lpBits
							   m_pBMI,                       // lpBitsInfo
							   DIB_RGB_COLORS,               // wUsage
							   SRCCOPY);                     // dwROP

	/* Reselect old palette */
	if (hOldPal != NULL)
	{
		::SelectPalette(hDC, hOldPal, TRUE);
	}

   return bSuccess;
}

/*************************************************************************
 *
 * CreatePalette()
 *
 * Return Value:
 *
 * TRUE if succesfull, FALSE otherwise
 *
 * Description:
 *
 * This function creates a palette from a DIB by allocating memory for the
 * logical palette, reading and storing the colors from the DIB's color table
 * into the logical palette, creating a palette from this logical palette,
 * and then returning the palette's handle. This allows the DIB to be
 * displayed using the best possible colors (important for DIBs with 256 or
 * more colors).
 *
 ************************************************************************/

BOOL CDib::CreatePalette()
{
	if (!m_pBMI)
		return FALSE;

   //get the number of colors in the DIB
   WORD wNumColors = NumColors();

   if (wNumColors != 0)
   {
		// allocate memory block for logical palette
		HANDLE hLogPal = ::GlobalAlloc(GHND, sizeof(LOGPALETTE) + sizeof(PALETTEENTRY)*wNumColors);

		// if not enough memory, clean up and return NULL
		if (hLogPal == 0)
			return FALSE;

		LPLOGPALETTE lpPal = (LPLOGPALETTE)::GlobalLock((HGLOBAL)hLogPal);

		// set version and number of palette entries
		lpPal->palVersion = PALVERSION;
		lpPal->palNumEntries = (WORD)wNumColors;

		for (int i = 0; i < (int)wNumColors; i++)
		{
			lpPal->palPalEntry[i].peRed = m_pBMI->bmiColors[i].rgbRed;
			lpPal->palPalEntry[i].peGreen = m_pBMI->bmiColors[i].rgbGreen;
			lpPal->palPalEntry[i].peBlue = m_pBMI->bmiColors[i].rgbBlue;
			lpPal->palPalEntry[i].peFlags = 0;
		}

		/* create the palette and get handle to it */
		if (m_pPalette)
		{
			m_pPalette->DeleteObject();
			delete m_pPalette;
		}
		m_pPalette = new CPalette;
		BOOL bResult = m_pPalette->CreatePalette(lpPal);
		::GlobalUnlock((HGLOBAL) hLogPal);
		::GlobalFree((HGLOBAL) hLogPal);
		return bResult;
	}

	return TRUE;
}

/*************************************************************************
 *
 * Width()
 *
 * Return Value:
 *
 * DWORD            - width of the DIB
 *
 * Description:
 *
 * This function gets the width of the DIB from the BITMAPINFOHEADER
 * width field 
 *
 ************************************************************************/

DWORD CDib::Width() const
{
	if (!m_pBMI)
		return 0;

	/* return the DIB width */
	return m_pBMI->bmiHeader.biWidth;
}

DWORD CDib::BitCount()    const
{
	if (!m_pBMI)
		return 0;

	/* return the DIB bitcount */
	return m_pBMI->bmiHeader.biBitCount;
}
/*************************************************************************
 *
 * Height()
 *
 * Return Value:
 *
 * DWORD            - height of the DIB
 *
 * Description:
 *
 * This function gets the height of the DIB from the BITMAPINFOHEADER
 * height field 
 *
 ************************************************************************/

DWORD CDib::Height() const
{
	if (!m_pBMI)
		return 0;
	
	/* return the DIB height */
	return m_pBMI->bmiHeader.biHeight;
}


/*************************************************************************
 *
 * PaletteSize()
 *
 * Return Value:
 *
 * WORD             - size of the color palette of the DIB
 *
 * Description:
 *
 * This function gets the size required to store the DIB's palette by
 * multiplying the number of colors by the size of an RGBQUAD 
 *
 ************************************************************************/

WORD CDib::PaletteSize() const
{
	if (!m_pBMI)
		return 0;

	return NumColors() * sizeof(RGBQUAD);
}


/*************************************************************************
 *
 * NumColors()
 *
 * Return Value:
 *
 * WORD             - number of colors in the color table
 *
 * Description:
 *
 * This function calculates the number of colors in the DIB's color table
 * by finding the bits per pixel for the DIB (whether Win3.0 or other-style
 * DIB). If bits per pixel is 1: colors=2, if 4: colors=16, if 8: colors=256,
 * if 24, no colors in color table.
 *
 ************************************************************************/

WORD CDib::NumColors() const
{
	if (!m_pBMI)
		return 0;

	WORD wBitCount;  // DIB bit count

	/*  The number of colors in the color table can be less than 
	 *  the number of bits per pixel allows for (i.e. lpbi->biClrUsed
	 *  can be set to some value).
	 *  If this is the case, return the appropriate value.
	 */

	DWORD dwClrUsed;

	dwClrUsed = m_pBMI->bmiHeader.biClrUsed;
	if (dwClrUsed != 0)
		return (WORD)dwClrUsed;

	/*  Calculate the number of colors in the color table based on
	 *  the number of bits per pixel for the DIB.
	 */
	wBitCount = m_pBMI->bmiHeader.biBitCount;

	/* return number of colors based on bits per pixel */
	switch (wBitCount)
	{
		case 1:
			return 2;

		case 4:
			return 16;

		case 8:
			return 256;

		default:
			return 0;
	}
}

/*************************************************************************
 *
 * Save()
 *
 * Saves the specified DIB into the specified CFile.  The CFile
 * is opened and closed by the caller.
 *
 * Parameters:
 *
 * CFile& file - open CFile used to save DIB
 *
 * Return value: Number of saved bytes or CFileException
 *
 *************************************************************************/
DWORD CDib::SaveIt(CFile& file, UINT kind /* = 0 */) const
{
	if (kind)
		return SaveTarga(file);
	BITMAPFILEHEADER bmfHdr; // Header for Bitmap file
	DWORD dwDIBSize;

	if (m_pBMI == NULL)
		return 0;

	// Fill in the fields of the file header

	// Fill in file type (first 2 bytes must be "BM" for a bitmap)
	bmfHdr.bfType = DIB_HEADER_MARKER;  // "BM"

	// Calculating the size of the DIB is a bit tricky (if we want to
	// do it right).  The easiest way to do this is to call GlobalSize()
	// on our global handle, but since the size of our global memory may have
	// been padded a few bytes, we may end up writing out a few too
	// many bytes to the file (which may cause problems with some apps).
	//
	// So, instead let's calculate the size manually (if we can)
	//
	// First, find size of header plus size of color table.  Since the
	// first DWORD in both BITMAPINFOHEADER and BITMAPCOREHEADER conains
	// the size of the structure, let's use this.
	dwDIBSize = *(LPDWORD)&m_pBMI->bmiHeader + PaletteSize();  // Partial Calculation

	// Now calculate the size of the image
	if ((m_pBMI->bmiHeader.biCompression == BI_RLE8) || (m_pBMI->bmiHeader.biCompression == BI_RLE4))
	{
		// It's an RLE bitmap, we can't calculate size, so trust the
		// biSizeImage field
		dwDIBSize += m_pBMI->bmiHeader.biSizeImage;
	}
	else
	{
		DWORD dwBmBitsSize;  // Size of Bitmap Bits only

		// It's not RLE, so size is Width (DWORD aligned) * Height
		dwBmBitsSize = ZWIDTHBYTES((m_pBMI->bmiHeader.biWidth)*((DWORD)m_pBMI->bmiHeader.biBitCount)) * m_pBMI->bmiHeader.biHeight;
		dwDIBSize += dwBmBitsSize;

		// Now, since we have calculated the correct size, why don't we
		// fill in the biSizeImage field (this will fix any .BMP files which
		// have this field incorrect).
		m_pBMI->bmiHeader.biSizeImage = dwBmBitsSize;
	}

	// Calculate the file size by adding the DIB size to sizeof(BITMAPFILEHEADER)
	bmfHdr.bfSize = dwDIBSize + sizeof(BITMAPFILEHEADER);
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;

	/*
	 * Now, calculate the offset the actual bitmap bits will be in
	 * the file -- It's the Bitmap file header plus the DIB header,
	 * plus the size of the color table.
	 */
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + m_pBMI->bmiHeader.biSize + PaletteSize();

	// Write the file header
	file.Write((LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER));
	DWORD dwBytesSaved = sizeof(BITMAPFILEHEADER); 

	// Write the DIB header
	UINT nCount = sizeof(BITMAPINFO) + (NumColors()-1)*sizeof(RGBQUAD);
	dwBytesSaved += nCount; 
	file.Write(m_pBMI, nCount);
	
	// Write the DIB bits
	DWORD dwBytes = 4 * ((m_pBMI->bmiHeader.biBitCount * Width() + 31) / 32);
/*
	DWORD dwBytes = m_pBMI->bmiHeader.biBitCount * Width();
  // Calculate the number of bytes per line
	if (dwBytes%32 == 0)
		dwBytes /= 8;
	else
		dwBytes = dwBytes/8 + (32-dwBytes%32)/8 + (((32-dwBytes%32)%8 > 0) ? 1 : 0); 
*/
	nCount = dwBytes * Height();
	dwBytesSaved += nCount; 
	file.Write(m_pBits, nCount);

	return dwBytesSaved;
}

/*************************************************************************

  Function:  Read (CFile&)

   Purpose:  Reads in the specified DIB file into a global chunk of
			 memory.

   Returns:  Number of read bytes.

*************************************************************************/

DWORD CDib::ReadIt(CFile& file)
{
	// Ensures no memory leaks will occur
	Free();
	
	BITMAPFILEHEADER bmfHeader;

	// Go read the DIB file header and check if it's valid.
	if (file.Read((LPSTR)&bmfHeader, sizeof(bmfHeader)) != sizeof(bmfHeader))
		return 0;
	if (bmfHeader.bfType != DIB_HEADER_MARKER)
		return ReadTarga(file);
	DWORD dwReadBytes = sizeof(bmfHeader);
	DWORD dwLength = (DWORD)file.GetLength() - sizeof(bmfHeader);

	// Allocate memory for DIB
	m_pBMI = (LPBITMAPINFO)MyAlloc(dwLength);
	if (m_pBMI == 0)
		return 0;
	

	// Read header.
	if (file.Read(m_pBMI, dwLength) != dwLength)
	{
		MyFree(m_pBMI);
		m_pBMI = NULL;
		return 0;
	}
	UINT extra = 0;
	UINT pitch;
	m_pBits = (LPBYTE)m_pBMI;
	pitch = 4*((m_pBMI->bmiHeader.biBitCount*m_pBMI->bmiHeader.biWidth + 31)/32); 
	UINT imgsize = m_pBMI->bmiHeader.biHeight * pitch;
	UINT size = imgsize;
	if (m_pBMI->bmiHeader.biBitCount < 9)
		{
		extra = 4 << m_pBMI->bmiHeader.biBitCount;
		}
	if ((size + bmfHeader.bfOffBits) > file.GetLength())
		{
		MyFree(m_pBMI);
		m_pBMI = NULL;
		return 0;
		}
	size += sizeof(BITMAPINFOHEADER) + extra;
	if (size > dwLength)
		{
		BYTE * pt = (BYTE *)MyAlloc(size);
		if (size == 0)
			{
			MyFree(m_pBMI);
			m_pBMI = NULL;
			return 0;
			}
		BYTE * ip = (BYTE *)m_pBMI;
		BYTE * op = pt;
		memmove(op,ip, sizeof(BITMAPINFOHEADER));
		op += sizeof(BITMAPINFOHEADER);
		ip += sizeof(BITMAPINFOHEADER);
		memmove(op,ip, extra);
		ip += extra - size + dwLength;
		op += extra;	
		memmove(op,ip, imgsize);
		MyFree(m_pBMI);
		m_pBMI = (LPBITMAPINFO)pt;
		m_pBMI->bmiHeader.biClrUsed = 1 << m_pBMI->bmiHeader.biBitCount;
		dwLength = size;
		}
	m_pBits += sizeof(BITMAPINFOHEADER) + extra;//colors*sizeof(RGBQUAD);

	dwReadBytes += dwLength;

	CreatePalette();
	return dwReadBytes;
}

/*************************************************************************

  Function:  ReadFromBMI (LPBITMAPINFO lpBI)

   Purpose:  Copies from an existing DIB

   Returns:  Number of read bytes.

*************************************************************************/

DWORD CDib::ReadFromBMI(LPBITMAPINFOHEADER lpBI, UINT depth)
{
	DWORD dwReadBytes = 0;
	int colors = 0;
	UINT bits;
	DPF("depth:%d,bits:%d,comp:%8X",depth,lpBI->biBitCount,lpBI->biCompression);
	bits = lpBI->biBitCount;
	if (depth)
		{
		if ((bits != 24) && (bits != 16))
			return 0;
		}
	Free();
	if (bits < 9)
		colors = 1 << bits;
	int nCount = sizeof(BITMAPINFOHEADER) + colors*sizeof(RGBQUAD);
	int nExtra = 0;
	DWORD pitch;
	if (depth == 8)
		{
		nExtra = 1024;
		pitch = 4*((lpBI->biWidth * 8 + 31) / 32);
		}
	else
		pitch = 4 * ((lpBI->biWidth * bits + 31) / 32);
DPF("pitch:%d",pitch);
	DWORD dwLength = pitch * lpBI->biHeight;
	// Allocate memory for DIB
	m_pBMI = (LPBITMAPINFO)MyAlloc(nCount + nExtra + dwLength);
	if (m_pBMI == 0)
		return 0;
	memmove(m_pBMI, lpBI, nCount);
	dwReadBytes += nCount;
DPF("count:%d,extra:%d",nCount,nExtra);
	if (depth)
		{
		m_pBMI->bmiHeader.biBitCount = depth;
		if (depth == 8)
		{
		int i;
		for (i = 0; i < 1024; i++)
			{
			m_pBMI->bmiColors[i / 4].rgbRed = i / 4;
			m_pBMI->bmiColors[i / 4].rgbGreen = i / 4;
			m_pBMI->bmiColors[i / 4].rgbBlue = i / 4;
			}
		}
		}
	m_pBits = (LPBYTE)m_pBMI;
	m_pBits += nCount + nExtra;
	BYTE * pSrc = (BYTE *)lpBI;
	pSrc += nCount;
	if (depth)
		{
		UINT ipitch = 4 * ((bits * lpBI->biWidth + 31) / 32);
		int x,y;
		for (y = 0; y < lpBI->biHeight; y++, pSrc += ipitch)
		for (x = 0; x < lpBI->biWidth; x++)
			{
			UINT r,g,b;
			UINT v;
			if (bits == 24)
				{
				b = (UINT)pSrc[3*x+0];
				g = (UINT)pSrc[3*x+1];
				r = (UINT)pSrc[3*x+2];
				}
			else if (bits == 16)
				{
				WORD vv = *((WORD *)(pSrc+2*x));
				r = (33 * ((vv >> 10) & 31)) / 4;
				g = (33 * ((vv >>  5) & 31)) / 4;
				b = (33 * (vv & 31)) / 4;
				}
			else
				{
				r = g = b = 0;
			}
			if (depth == 24)
				{
				m_pBits[y * pitch + 3 * x + 0] = b;
				m_pBits[y * pitch + 3 * x + 1] = g;
				m_pBits[y * pitch + 3 * x + 2] = r;
				}
			else
				{
				v =(11 * b + 59 * g + 30 * r) / 100;
				m_pBits[y * pitch + x] = v;
				}
			}

		}
	else if (bits != 32)
		memmove(m_pBits, pSrc,dwLength);
	else
		{
		UINT x, y;
		UINT w = lpBI->biWidth; 
		UINT h = lpBI->biHeight; 
		DPF("w:%d,h:%d,p:%d",w,h,pitch);
		for (y = 0; y < h; y++)
			for (x = 0; x < w; x++)
				{
				UINT a = m_pBits[y*pitch+4*x+3] = pSrc[y*pitch+4*x+3];
				a = 255 - a;
				a = a * a / 255;
				a = 255 - a;
				int j;
				for (j = 0; j < 3; j++)
					{
					UINT q = a * pSrc[y*pitch+4*x+j] + 255 * (255 - a);
					m_pBits[y*pitch+4*x+j] = q / 255;
					}
				}
		}
	dwReadBytes += dwLength;
	CreatePalette();
	return dwReadBytes;
}

DWORD CDib::ReadFromDib(CDib * pDib)
{
	DWORD dwReadBytes = 0;
	Free();
	int nCount = sizeof(BITMAPINFOHEADER) + pDib->NumColors()*sizeof(RGBQUAD);
	DWORD pitch = 4 * ((pDib->Width() * pDib->BitCount() + 31) / 32);
DPF("pitch:%d",pitch);
	DWORD dwLength = pitch * pDib->Height();
	m_pBMI = (LPBITMAPINFO)MyAlloc(nCount+dwLength);
	if (m_pBMI == 0)
		return 0;
	memmove(m_pBMI, pDib->m_pBMI, nCount);
	m_pBits = (LPBYTE)m_pBMI;
	m_pBits += nCount;
	dwReadBytes += nCount;
	memmove(m_pBits, pDib->m_pBits,dwLength);
	dwReadBytes += dwLength;
	CreatePalette();
	return dwReadBytes;
}

DWORD CDib::Create(UINT w, UINT h, UINT d)
{
	DWORD dwReadBytes = 0;
	Free();
	if ((d != 8) && (d != 24) && (d != 32)) return dwReadBytes;
	int nCount = sizeof(BITMAPINFOHEADER);
	if (d < 9)
		nCount += 256 * sizeof(RGBQUAD);
	DWORD pitch = 4 * ((w * d + 31) / 32);
DPF("pitch:%d",pitch);
	DWORD dwLength = pitch * h;
	m_pBMI = (LPBITMAPINFO)MyAlloc(nCount+dwLength);
	if (m_pBMI == 0)
		return 0;
	LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)m_pBMI;
	lpBI->biSize			= sizeof(BITMAPINFOHEADER);
	lpBI->biWidth			= w;
	lpBI->biHeight			= h;
	lpBI->biPlanes			= 1;
	lpBI->biBitCount		= d;
	lpBI->biCompression		= BI_RGB;
	lpBI->biSizeImage		= dwLength;
	lpBI->biXPelsPerMeter	= 0;
	lpBI->biYPelsPerMeter	= 0;
	lpBI->biClrUsed			= d > 8 ? 0 : 256;
	lpBI->biClrImportant	= 0;
	m_pBits = (BYTE *)lpBI;
	m_pBits += lpBI->biSize;
	if (d < 9)
		{
		UINT i;
		for (i = 0; i < 1024; i++)
			m_pBits[i] = (BYTE)(i / 4);
		m_pBits += 1024;
		}
	memset(m_pBits,255,dwLength);
	dwReadBytes += nCount;
	dwReadBytes += dwLength;
	if (d < 9)
		CreatePalette();
	return dwReadBytes;
}

DWORD CDib::Merge(CDib * pRef, CDib * pGrab)
{
	DWORD dwReadBytes = 0;
	if (pRef->Width() != pGrab->Width())
		return 0;
	if (pRef->Height() != pGrab->Height())
		return 0;
	if (pRef->NumColors() != pGrab->NumColors())
		return 0;
	Free();
	int nCount = sizeof(BITMAPINFOHEADER) + pGrab->NumColors()*sizeof(RGBQUAD);
	DWORD pitch = 4 * ((pGrab->Width() * pGrab->BitCount() + 31) / 32);
DPF("pitch:%d",pitch);
	DWORD dwLength = pitch * pGrab->Height();
	m_pBMI = (LPBITMAPINFO)MyAlloc(nCount+dwLength);
	if (m_pBMI == 0)
		return 0;
	memmove(m_pBMI, pGrab->m_pBMI, nCount);
	dwReadBytes += nCount;
	m_pBits = (LPBYTE)m_pBMI;
	m_pBits += nCount;
	BYTE * pref = pRef->m_pBits;
	BYTE * pgrb = pGrab->m_pBits;
	BYTE * pdst = m_pBits;
	UINT i;
	for (i = 0; i < dwLength; i++)
		{
		int a = *pgrb++;
		int b = *pref++;
		int v = 255 - (abs (a - b));
		v = (v * v) / 255;
		*pdst++ = v;
		}
	dwReadBytes += dwLength;
	CreatePalette();
	return dwReadBytes;
}

#ifdef _DEBUG
void CDib::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}
#endif

//////////////////////////////////////////////////////////////////////////
//// Clipboard support

//---------------------------------------------------------------------
//
// Function:   CopyToHandle
//
// Purpose:    Makes a copy of the DIB to a global memory block.  Returns
//             a handle to the new memory block (NULL on error).
//
// Returns:    Handle to new global memory block.
//
//---------------------------------------------------------------------

HGLOBAL CDib::CopyToHandle() const
{
	CSharedFile file;
	try
	{
		if (SaveIt(file)==0)
			return 0;
	}
	catch (CFileException* e)
	{
		e->Delete();
		return 0;
	}
		
	return file.Detach();
}

//---------------------------------------------------------------------
//
// Function:   ReadFromHandle
//
// Purpose:    Initializes from the given global memory block.  
//
// Returns:    Number of read bytes.
//
//---------------------------------------------------------------------

DWORD CDib::ReadFromHandle(HGLOBAL hGlobal)
{
	CSharedFile file;
	file.SetHandle(hGlobal, FALSE);
	DWORD dwResult = ReadIt(file);
	file.Detach();
	return dwResult;
}

//---------------------------------------------------------------------
//
// Function:   ReadFromClipboard
//
// Purpose:    Initializes from the clipboard.  
//
// Returns:    Number of read bytes.
//
//---------------------------------------------------------------------

DWORD CDib::ReadFromClipboard(UINT depth)
{
DPF("dib read from clip");
      if (!IsClipboardFormatAvailable (CF_DIB))
		return 0;
	HANDLE hDIB = ::GetClipboardData(CF_DIB);
#ifndef FLIPBOOK_MAC
	DPF("handle:%8x",(DWORD)hDIB);
#endif
	if (!hDIB)
		return 0;
	LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)::GlobalLock(hDIB);
	DWORD dwSize = ReadFromBMI(lpBI, depth);
	::GlobalUnlock(hDIB);
	return dwSize;
}

//////////////////////////////////////////////////////////////////////////
//// Serialization support

void CDib::Serialize(CArchive& ar) 
{
	CFile* pFile = ar.GetFile();
	ASSERT(pFile != NULL);
	if (ar.IsStoring())
	{	// storing code
		SaveIt(*pFile);
	}
	else
	{	// loading code
		ReadIt(*pFile);
	}
}

BOOL  CDib::AdjustPalette(UINT index, UINT r, UINT g, UINT b)
{
	if (!m_pBMI)
		return FALSE;
	if (m_pBMI->bmiHeader.biBitCount > 8)
		return FALSE;
	if (index > (UINT)(1 << m_pBMI->bmiHeader.biBitCount))
		return FALSE;
	m_pBMI->bmiColors[index].rgbRed = r;
	m_pBMI->bmiColors[index].rgbGreen = g;
	m_pBMI->bmiColors[index].rgbBlue = b;
	return TRUE;
}


DWORD CDib::ReadTarga(CFile& file)
{
	Free();
	WORD hdr[9];
	file.Seek(0,0);
	if (file.Read((LPSTR)&hdr, sizeof(hdr)) != sizeof(hdr))
		return 0;
	UINT k = hdr[1];
	UINT d = hdr[8] & 255;
	UINT w = hdr[6];
	UINT h = hdr[7];
	if ((k != 2) && (k != 10) && (k != 3))
		return 0;
	if ((d != 24) && (d != 32) && (d != 8))
		return 0;
	if ((k == 3) && (d != 8))
		return 0;
	if ((d == 8) && (k != 3))
		return 0;
	UINT op = 4 * ((d * w + 31) / 32);
	DWORD dwLength = op * h;
//	if (dwLength > 16000000)
//		return 0;
	UINT size = sizeof(BITMAPINFOHEADER)+dwLength;
	if (d == 8)
		size += 1024;
	m_pBMI = (LPBITMAPINFO)MyAlloc(size);
	if (m_pBMI == 0)
		return 0;
	LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)m_pBMI;
	lpBI->biSize			= sizeof(BITMAPINFOHEADER);
	lpBI->biWidth			= w;
	lpBI->biHeight			= h;
	lpBI->biPlanes			= 1;
	lpBI->biBitCount		= d;
	lpBI->biCompression		= BI_RGB;
	lpBI->biSizeImage		= dwLength;
	lpBI->biXPelsPerMeter	= 0;
	lpBI->biYPelsPerMeter	= 0;
	lpBI->biClrUsed			= d > 8 ? 0 : 256;
	lpBI->biClrImportant	= 0;
	m_pBits = (BYTE *)lpBI;
	m_pBits += lpBI->biSize;
	if (d == 8)
		{
		int i;
		for (i = 0; i < 256; i++)
			{
			*m_pBits++ = i;
			*m_pBits++ = i;
			*m_pBits++ = i;
			*m_pBits++ = 0;
			}
		}
	BYTE * p = m_pBits;
	UINT depth = d / 8;
	UINT ip = depth * w;
	UINT y;
	for (y = 0; y < h; y++)
		{
		if ((k == 2) || (k == 3))
			{
			if (file.Read(p, ip) != ip)
				break;
			}	
		else
			{
			BYTE * pp = p;
			UINT x;
			for (x = 0;x < w;)
				{
				BYTE code;
				file.Read(&code, 1);
				if (code & 128)
					{
					code = code & 127;
					if ((x + code) > w)
						code = w - x;
					BYTE * zp = pp;
					file.Read(zp, depth);
					pp += depth;
					x++;
					if ((x + code) > w)
						code = w - x;
					x += code;
					for (;code--; pp+=depth)
						memcpy(pp,zp,depth);
					}
				else
					{
					code = 1 + (code & 127);
					file.Read(pp, depth * code);
					x += code;
					pp += depth * code;
					}
				}
			}
		p += op;
		}
	if (y < h)
		return 0;
	CreatePalette();
	return size;
}


DWORD CDib::SaveTarga(CFile& file) const
{
	BOOL bCompressed = 1;
	WORD hdr[9];
// targa header;
	DWORD dwBytesSaved = 0;
	BYTE * dst;
	BYTE * src;
	src = m_pBits;
	DWORD w = Width();
	DWORD h = Height();
	DWORD d = BitCount();
	DWORD ip = 4 * ((w * d + 31) / 32);
	d = (d + 7) / 8;
	if ((d != 3) && (d!=4)) return 0;
	DWORD op = d * w;
	DWORD dsize = 20 + (h * op * 110) / 100; // spec says 2 %, 10 for sure
	dst = new BYTE[dsize];
	BYTE * dptr = dst;
	if (!dst) return dwBytesSaved;
	DWORD y;
	UINT x;
	UINT bpp = d;
	hdr[0] = 0;
	hdr[1] = bCompressed ? 10 : 2;
	hdr[2] = 0;
	hdr[3] = 0;
	hdr[4] = 0;
	hdr[5] = 0;
	hdr[6] = (WORD)w;
	hdr[7] = (WORD)h;
	hdr[8] = 8 * (WORD)d;// + ((d == 4) ? 2048 : 0);
	dwBytesSaved += 18;
	file.Write(&hdr, 18);
	UINT outcnt = 0;
	if (bCompressed)
		{
		for (y = 0; y < h; y++, src += ip)
			{
			for (x = 0;x < w;)
				{
				UINT i,cnt;
				BYTE * tp = src + x * bpp;
				UINT max;
				max = w - x;
				if (max > 128)
					max = 128;
				for (i = bpp; (i < max * bpp) && (tp[i-bpp] == tp[i]); i++);
				cnt = i / bpp;
				if (cnt > 1)
					{
					*dptr++ = 0x80 + cnt - 1;
					for (i = 0; i < bpp; i++)
						*dptr++ = tp[i];
					outcnt += 1 + bpp;
					x += cnt;
					continue;
					}
				for (cnt = 1; cnt < max; cnt++)
					{
					for (i = 0; i < bpp; i++)
						if (tp[(cnt-1)*bpp+i] != tp[(cnt*bpp)+i])
							break;
					if (i >= bpp)
						break;
					}
				*dptr++ = cnt - 1;
				for (i = 0; i < cnt * bpp; i++)
					*dptr++ = tp[i];
				outcnt += 1 + cnt * bpp;
				x += cnt;
				}
			}
		}
	else
		{
		for (y = 0; y < h; y++)
			memmove(dst+y*op,src+y*ip,op);
		outcnt = h * op;
		}
	file.Write(dst, outcnt);
	dwBytesSaved += outcnt;
	delete [] dst;
	return dwBytesSaved;
}

DWORD CDib::Convert(UINT newd)
{
	DWORD w = Width();
	DWORD h = Height();
	DWORD oldd = BitCount();
	if (newd == 24)
		{
		ASSERT(oldd != 24);
		if ((oldd != 32) && (oldd != 8))
			return 0;
		}
	else if (newd == 32)
		{
		if ((oldd != 24) && (oldd != 8))
			return 0;
		}
	else if (newd == 1)
		{
		if (oldd != 8)
			return 0;
		}
	else
		return 0;
	DWORD op = 4 * ((newd*w+31) / 32);
	DWORD ip = 4 * ((oldd*w+31) / 32);
	DWORD dwLength = op * h;
	BYTE * src = (BYTE *)m_pBMI;
	UINT size = sizeof(BITMAPINFOHEADER)+dwLength;
	if (newd == 1)
		size += 8;
	else if (newd == 8)
		size += 1024;
	BYTE * dst = (BYTE *)MyAlloc(size);
	if (*dst)
		return 0;
	memmove(dst, src, sizeof(BITMAPINFOHEADER));
	m_pBMI = (LPBITMAPINFO)dst;
	dst += sizeof(BITMAPINFOHEADER);
	src += sizeof(BITMAPINFOHEADER);
	if (newd == 1)
		{
		*((DWORD *)dst) = 0;
		dst += 4;
		*((DWORD *)dst) = 0xffffffff;
		dst += 4;
		}
	if (newd == 8)
		{
		int i;
		for (i = 0; i < 256;i++)
			{
			*dst = i;
			*dst = i;
			*dst = i;
			*dst = 0;
			}
		}
	m_pBits = dst;
	if (oldd == 8)
		src += 1024;
	if (newd == 24)
		{
		if (oldd == 8)
			{
			UINT x,y;
			for (y = 0; y < h; y++)
			for (x = 0; x < w; x++)
				{
				BYTE v;
				v = src[y*ip+x];
				dst[y*op+3*x+0] = v;
				dst[y*op+3*x+1] = v;
				dst[y*op+3*x+2] = v;
				}
			}
		else
			{
			UINT x,y;
			for (y = 0; y < h; y++)
			for (x = 0; x < w; x++)
				{
				int j;
				UINT v = src[y*ip+4*x+3];
				UINT nv = 255 - v;
				for (j = 0; j < 3; j++)
					dst[y*op+3*x+j] =
							(v * src[y*ip+4*x+j] + nv*255) / 255;
				}
			}
		}
	else if (newd == 32)
		{
		if (oldd == 8)
			{
			UINT x,y;
			for (y = 0; y < h; y++)
			for (x = 0; x < w; x++)
				{
				BYTE v,a;
				v = src[y*ip+x] == 255 ? 255 : 0;
				a = 255 - src[y*ip+x];
				dst[y*op+4*x+0] = v;
				dst[y*op+4*x+1] = v;
				dst[y*op+4*x+2] = v;
				dst[y*op+4*x+3] = a;
				}
			}
		else
			{
			UINT x,y;
			for (y = 0; y < h; y++)
			for (x = 0; x < w; x++)
				{
				dst[y*op+4*x+0] = src[y*ip+3*x+0];
				dst[y*op+4*x+1] = src[y*ip+3*x+1];
				dst[y*op+4*x+2] = src[y*ip+3*x+2];
				dst[y*op+4*x+3] = 255;
				}
			}
		}
	else if (newd == 8)
		{
		UINT x,y,z;
		z = oldd / 8;
		for (y = 0; y < h; y++)
		for (x = 0; x < w; x++)
			dst[y*op+x] = src[y*ip+z*x];
		}
	else if (newd == 1)
		{
		UINT x,y;
		UINT v = 1;
		for (y = 0; y < h; y++)
		{
		for (x = 0; x < w; x++)
			{
			v = v + v;
			if (src[y*ip+x] > 127)
				v++;
			if (v > 255)
				{
				dst[y*op+x/8] = v & 255;
				v = 1;
				}
			}
		if ((v != 1) && v)
			{
			for (;v < 256;v += v);
			dst[y*op+x/8] = v & 255;
			}
		}
		}
	if (oldd == 8)
		src -= 1024;
	src -= sizeof(BITMAPINFOHEADER);
	MyFree(src);
	LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)m_pBMI;
	lpBI->biBitCount		= newd;
	lpBI->biClrUsed			= newd < 9 ? 1 << newd : 0;
	lpBI->biSizeImage		= lpBI->biSize + 4 * lpBI->biClrUsed +
			lpBI->biHeight * 4 * ((lpBI->biWidth*lpBI->biBitCount+31) / 32);
	return op*h;
}


DWORD CDib::Scale(UINT neww, UINT newh)
{
	DWORD w = Width();
	DWORD h = Height();
	DWORD d = BitCount();
	DWORD op = 4 * ((d*neww+31) / 32);
	DWORD ip = 4 * ((d*w+31) / 32);
	DWORD dwLength = op * newh;
	BYTE * src = (BYTE *)m_pBMI;
	UINT extra = sizeof(BITMAPINFOHEADER);
	if (d < 9)
		extra += (4 << d);
	BYTE * dst = (BYTE *)MyAlloc(dwLength + extra);
	if (*dst)
		return 0;
	memmove(dst, src, extra);
	dst += extra;
	src += extra;
	m_pBits = dst;
	CGScaler scale;
	if (!scale.Init(w,h,d,neww,newh))
		{
		int q = scale.Custom(dst, src, op, ip);
DPF("after custom,q:%d",q);
		int z = scale.Copy();
DPF("after copy,z:%d",z);
		if (z || q)
			{
			dst -= extra;
			MyFree(dst);
			return 0;
			}
		}
	dst -= extra;
	m_pBMI = (LPBITMAPINFO)dst;
	src -= extra;
	MyFree(src);
	LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)m_pBMI;
	lpBI->biSizeImage		= dwLength;
	lpBI->biWidth = neww;
	lpBI->biHeight = newh;
	return dwLength;
}


DWORD CDib::Export(LPCSTR Name, FBFileFormat fkind)
{
#ifdef FLIPBOOK_MAC
	fkind = FBGetMacDefault();
#endif
	LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER) m_pBMI;
	UINT dwSize = sizeof(BITMAPINFOHEADER);
	UINT pitch = 4 *((lpBI->biBitCount*lpBI->biWidth + 31)/32); 
	dwSize += lpBI->biHeight * pitch;
	if (lpBI->biBitCount < 9)
		dwSize += 4 << lpBI->biBitCount;
	return FBExportStill (Name, lpBI, dwSize, fkind);
}

DWORD CDib::Importer(void * pData, UINT ssize)
{
	return ReadFromBMI((LPBITMAPINFOHEADER)pData,0) ? 0 : 1;
}

UINT DibInCB(FBDataKind Code, void * pData, int size, void * pClass)
{
	if (Code != kBMPDataType)
		return 0;
	return ((CDib*)pClass)->Importer(pData, size);
}

DWORD CDib::Import(LPCTSTR lpszFileName)
{
	return FBImportStill (lpszFileName, &DibInCB, (void*)this);
}
