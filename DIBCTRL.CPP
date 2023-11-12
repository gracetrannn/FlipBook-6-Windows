#include "stdafx.h"
#include "dibctrl.h"
//#include "myglobal.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define RECTWIDTH(lpRect)     ((lpRect)->right - (lpRect)->left)
#define RECTHEIGHT(lpRect)    ((lpRect)->bottom - (lpRect)->top)

BOOL PaintDIB(HDC     hDC,
					LPRECT  lpDCRect,
					LPBITMAPINFOHEADER lpBI,
					LPRECT  lpDIBRect,
					CPalette* pPal)
{
	BOOL     bSuccess=FALSE;      // Success/fail flag
	HPALETTE hPal=NULL;           // Our DIB's palette
	HPALETTE hOldPal=NULL;        // Previous palette

	if (!lpBI)
		return FALSE;
	LPSTR lpBits = (LPSTR)lpBI;
	UINT offset = lpBI->biSize;
	if (lpBI->biBitCount < 9)
		offset += 4 << lpBI->biBitCount;
	lpBits += offset;
/*
	// Get the DIB's palette, then select it into DC
	if (pPal != NULL)
	{
		hPal = (HPALETTE) pPal->m_hObject;

		// Select as background since we have
		// already realized in forground if needed
		hOldPal = ::SelectPalette(hDC, hPal, TRUE);
	}
*/
	/* Make sure to use the stretching mode best for color pictures */
	::SetStretchBltMode(hDC, COLORONCOLOR);

	/* Determine whether to call StretchDIBits() or SetDIBitsToDevice() */
	if ((RECTWIDTH(lpDCRect)  == RECTWIDTH(lpDIBRect)) &&
	   (RECTHEIGHT(lpDCRect) == RECTHEIGHT(lpDIBRect)))
		bSuccess = ::SetDIBitsToDevice(hDC,                    // hDC
								   lpDCRect->left,             // DestX
								   lpDCRect->top,              // DestY
								   RECTWIDTH(lpDCRect),        // nDestWidth
								   RECTHEIGHT(lpDCRect),       // nDestHeight
								   lpDIBRect->left,            // SrcX
								   (int)lpBI->biHeight -
									  lpDIBRect->top -
									  RECTHEIGHT(lpDIBRect),   // SrcY
								   0,                          // nStartScan
								   (WORD)lpBI->biHeight,  // nNumScans
								   lpBits,                  // lpBits
								   (LPBITMAPINFO)lpBI,
								   DIB_RGB_COLORS);            // wUsage
   else
	  bSuccess = ::StretchDIBits(hDC,                          // hDC
							   lpDCRect->left,                 // DestX
							   lpDCRect->top,                  // DestY
							   RECTWIDTH(lpDCRect),            // nDestWidth
							   RECTHEIGHT(lpDCRect),           // nDestHeight
							   lpDIBRect->left,                // SrcX
							   lpDIBRect->top,                 // SrcY
							   RECTWIDTH(lpDIBRect),           // wSrcWidth
							   RECTHEIGHT(lpDIBRect),          // wSrcHeight
							   lpBits,                      // lpBits
							   (LPBITMAPINFO)lpBI,         // lpBitsInfo
							   DIB_RGB_COLORS,                 // wUsage
							   SRCCOPY);                       // dwROP

   	/* Reselect old palette */
	if (hOldPal != NULL)
	{
		::SelectPalette(hDC, hOldPal, TRUE);
	}

   return bSuccess;
}


/////////////////////////////////////////////////////////////////////////////
// CDIBStatic

CDIBStatic::CDIBStatic()
{
	m_pDIB = 0;
}

CDIBStatic::~CDIBStatic()
{
	delete [] m_pDIB;
}


BEGIN_MESSAGE_MAP(CDIBStatic, CStatic)
	//{{AFX_MSG_MAP(CDIBStatic)
	ON_WM_CTLCOLOR_REFLECT()
//	ON_WM_QUERYNEWPALETTE()
//	ON_WM_PALETTECHANGED()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CDIBStatic::SetDib(BYTE * pData)
{
	delete [] m_pDIB;
	m_pDIB = pData;
	return 0;
}

void CDIBStatic::ClearDib()
{
	ASSERT_VALID(this);
	
	CClientDC dc(this);
	CRect rectPaint;
    
	GetClientRect(&rectPaint);    
	rectPaint.InflateRect(-1,-1);
	    
	dc.FillSolidRect(&rectPaint, RGB(255,255,255));
}

void CDIBStatic::PaintDib(BOOL bDibValid)
{
	ASSERT_VALID(this);
	ClearDib();
	LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)m_pDIB;
	if (!lpBI || !m_hWnd) return;
	CRect PaintRect;
	GetClientRect(&PaintRect);    
	PaintRect.InflateRect(-1, -1);
	CClientDC dc(this);
	int w = lpBI->biWidth;
	int h = lpBI->biHeight;
	if (bDibValid)
	{
		int nDestX, nDestY, nDestWidth, nDestHeight;
		if (w < PaintRect.Width() && h < PaintRect.Height())
		{ // If the image fits, just center it
			nDestX = PaintRect.left + (PaintRect.Width() - w)/2;
			nDestY = PaintRect.top + (PaintRect.Height() - h)/2;
			nDestWidth = w;
			nDestHeight = h;
		}
		else
		{ // The bitmap doesn't fit, scale to fit 
			if ((PaintRect.Width()/(float)w)
					<= (PaintRect.Height()/(float)h))
			{ // Width is constraint
				nDestWidth = PaintRect.Width();
				nDestHeight = (nDestWidth*h) / w;
				nDestX = PaintRect.left;
				nDestY = PaintRect.top + (PaintRect.Height() - nDestHeight) /2;
			}
			else
			{ // Height is constraint		
				nDestHeight = PaintRect.Height();
				nDestWidth = (nDestHeight*w) / h;
				nDestX = PaintRect.left + (PaintRect.Width() - nDestWidth) /2;
				nDestY = PaintRect.top;
			}
		}

		CRect RectDest(nDestX, nDestY, nDestX+nDestWidth, nDestY+nDestHeight);
		CRect RectDib(0, 0, w, h);
		PaintDIB(dc, &RectDest, lpBI, &RectDib, 0);     
	}
	else
	{
		dc.MoveTo(PaintRect.TopLeft());
		dc.LineTo(PaintRect.BottomRight());
		dc.MoveTo(PaintRect.right, PaintRect.top);
		dc.LineTo(PaintRect.left, PaintRect.bottom);
	}
		
	return;
}

void CDIBStatic::UpdateDib()
{
	ASSERT_VALID(this);
	PaintDib(IsValidDib());
}

/////////////////////////////////////////////////////////////////////////////
// CDIBStatic message handlers

HBRUSH CDIBStatic::CtlColor(CDC* pDC, UINT nCtlColor) 
{
	UpdateDib();

	// TODO: Return a non-NULL brush if the parent's handler should not be called
	return (HBRUSH)GetStockObject(NULL_BRUSH);
}
/*
BOOL CDIBStatic::OnQueryNewPalette() 
{
	return DoRealizePalette(FALSE);
}

void CDIBStatic::OnPaletteChanged(CWnd* pFocusWnd) 
{
	DoRealizePalette(TRUE);
}

BOOL CDIBStatic::DoRealizePalette(BOOL bForceBackGround)
{
	if (IsValidDib())
	{
		CClientDC dc(this);
		if (!m_DIB.m_pPalette)
			return FALSE;
		HPALETTE hPal = (HPALETTE)m_DIB.m_pPalette->m_hObject;
		HPALETTE hOldPalette = SelectPalette(dc, hPal, bForceBackGround);
		UINT nChanged = dc.RealizePalette();
		SelectPalette(dc, hOldPalette, TRUE);

		if (nChanged == 0)	// no change to our mapping
			return FALSE;
		
		// some changes have been made; invalidate
		UpdateDib();
	}

	return TRUE;
}
*/
