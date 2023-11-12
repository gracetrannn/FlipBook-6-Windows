// GridCell.cpp : implementation file
//
// MFC Grid Control - Grid cell base class
//
// Written by Chris Maunder <cmaunder@mail.com>
// Copyright (c) 1998-2000. All Rights Reserved.
//
// Eric Woodruff - 02/20/2000 - Added PrintCell() plus other minor changes
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GridCell.h"
#include "InPledit.h"
#include "GridCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define SELECTED_CELL_FONT_WEIGHT 600    // weight of text for selected items


IMPLEMENT_DYNCREATE(CGridCell, CObject)

/////////////////////////////////////////////////////////////////////////////
// GridCell

CGridCell::CGridCell()
{
    Reset();
}

CGridCell::~CGridCell()
{
}

/////////////////////////////////////////////////////////////////////////////
// GridCell Attributes

void CGridCell::Reset()
{
    m_nState  = 0;
    m_nFormat = 0;
    m_strText.Empty();
    m_nImage  = -1;
    m_crBkClr = CLR_DEFAULT;
    m_crFgClr = CLR_DEFAULT;
    m_lParam  = 0;
    m_nMargin = 3;
    m_pGrid   = NULL;

    m_bEditing = FALSE;
    m_pEditWnd = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// GridCell Operations

// EFW - Various changes to make it draw cells better when using alternate
// color schemes.  Also removed printing references as that's now done
// by PrintCell() and fixed the sort marker so that it doesn't draw out
// of bounds.
BOOL CGridCell::Draw(CDC* pDC, int nRow, int nCol, CRect rect,
    BOOL bEraseBkgnd /*=TRUE*/)
{
    if (!m_pGrid || !pDC)
        return FALSE;


    BOOL bDrawFixed = ((GetState() & GVIS_FIXED) == GVIS_FIXED);

//    TRACE3("Drawing %s cell %d, %d\n", bDrawFixed? _T("Fixed") : _T(""), nRow, nCol);

    int nSavedDC = pDC->SaveDC();
    pDC->SetBkMode(TRANSPARENT);
//#ifdef QQQQ
    // Set up text and background colours
    COLORREF TextClr, TextBkClr;
    if (m_crFgClr == CLR_DEFAULT)
        TextClr = (bDrawFixed)? m_pGrid->GetFixedTextColor() : m_pGrid->GetTextColor();
    else
        TextClr = m_crFgClr;

    if (m_crBkClr == CLR_DEFAULT)
        TextBkClr = (bDrawFixed)? m_pGrid->GetFixedBkColor() : m_pGrid->GetTextBkColor();
    else
    {
        bEraseBkgnd = TRUE;
        TextBkClr = m_crBkClr;
    }

    // Draw cell background and highlighting (if necessary)
    if((m_nState & GVIS_FOCUSED) || (m_nState & GVIS_DROPHILITED))
    {
        // Always draw even in list mode so that we can tell where the
        // cursor is at.  Use the highlight colors though.
        if(m_nState & GVIS_SELECTED)
        {
            TextBkClr = RGB(0,0,0);//::GetSysColor(COLOR_HIGHLIGHT);
            TextClr = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
            bEraseBkgnd = TRUE;
        }

        rect.right++; rect.bottom++;    // FillRect doesn't draw RHS or bottom
        if (bEraseBkgnd)
        {
            CBrush brush(TextBkClr);
            pDC->FillRect(rect, &brush);
        }

        // Don't adjust frame rect if no grid lines so that the
        // whole cell is enclosed.
        if(m_pGrid->m_nGridLines != GVL_NONE)
        {
            rect.right--;
            rect.bottom--;
        }

        // Use same color as text to outline the cell so that it shows
        // up if the background is black.
        CBrush brush(TextClr);
//        pDC->FrameRect(rect, &brush);
        pDC->SetTextColor(TextClr);

        // Adjust rect after frame draw if no grid lines
        if(m_pGrid->m_nGridLines == GVL_NONE)
        {
            rect.right--;
            rect.bottom--;
        }

        rect.DeflateRect(1,1);
    }
    else if ((m_nState & GVIS_SELECTED))
    {
        rect.right++; rect.bottom++;    // FillRect doesn't draw RHS or bottom
//        pDC->FillSolidRect(rect, ::GetSysColor(COLOR_HIGHLIGHT));
        rect.right--; rect.bottom--;
        pDC->SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
    }
    else
    {
        if (bEraseBkgnd)
        {
            rect.right++; rect.bottom++;    // FillRect doesn't draw RHS or bottom
            CBrush brush(TextBkClr);
            pDC->FillRect(rect, &brush);
            rect.right--; rect.bottom--;
        }
        pDC->SetTextColor(TextClr);
    }

    // Setup font and if fixed, draw fixed cell highlighting
//#endif
    CFont Font;
    static LOGFONT lf;
    memcpy(&lf, &m_lfFont, sizeof(LOGFONT));

    // Draw lines only when wanted
    if (bDrawFixed && m_pGrid->m_nGridLines != GVL_NONE)
    {
        CCellID FocusCell = m_pGrid->GetFocusCell();

        // As above, always show current location even in list mode so
        // that we know where the cursor is at.
        BOOL bHiliteFixed = m_pGrid->IsValid(FocusCell) &&
            (FocusCell.row == nRow || FocusCell.col == nCol);

        // If this fixed cell is on the same row/col as the focus cell,
        // highlight it.
        if (bHiliteFixed)
        {
            lf.lfWeight = SELECTED_CELL_FONT_WEIGHT;

            rect.right++; rect.bottom++;
            pDC->DrawEdge(rect, BDR_SUNKENINNER /*EDGE_RAISED*/, BF_RECT);
            rect.DeflateRect(1,1);
        }
        else
        {
            CPen lightpen(PS_SOLID, 1,  ::GetSysColor(COLOR_3DHIGHLIGHT)),
                darkpen(PS_SOLID,  1, ::GetSysColor(COLOR_3DDKSHADOW)),
                *pOldPen = pDC->GetCurrentPen();

            pDC->SelectObject(&lightpen);
            pDC->MoveTo(rect.right, rect.top);
            pDC->LineTo(rect.left, rect.top);
            pDC->LineTo(rect.left, rect.bottom);

            pDC->SelectObject(&darkpen);
            pDC->MoveTo(rect.right, rect.top);
            pDC->LineTo(rect.right, rect.bottom);
            pDC->LineTo(rect.left, rect.bottom);

            pDC->SelectObject(pOldPen);
            rect.DeflateRect(1,1);
        }
    }
    Font.CreateFontIndirect(&lf);
    pDC->SelectObject(&Font);

    // Draw Text and image

//    rect.DeflateRect(m_nMargin, m_nMargin);
  //  rect.DeflateRect(m_nMargin, 0);

	m_pGrid->DrawCellImage(pDC, &rect, nRow, nCol,m_nState);
//    rect.DeflateRect(m_nMargin, m_nMargin);
#ifdef ZZZZ
    if (m_pGrid->GetImageList() && m_nImage >= 0)
    {
        IMAGEINFO Info;
        if (m_pGrid->GetImageList()->GetImageInfo(m_nImage, &Info))
        {
            //CRgn rgn;
            //rgn.CreateRectRgnIndirect(rect);
            //pDC->SelectClipRgn(&rgn);
            //rgn.DeleteObject();

            int nImageWidth = Info.rcImage.right-Info.rcImage.left+1;
            m_pGrid->GetImageList()->Draw(pDC, m_nImage, rect.TopLeft(), ILD_NORMAL);
            rect.left += nImageWidth+m_nMargin;
        }
    }

    // Draw sort arrow
    if (m_pGrid->m_nSortColumn == nCol && nRow == 0)
    {
        CSize size = pDC->GetTextExtent(_T("M"));
        int nOffset = 2;

        // Base the size of the triangle on the smaller of the column
        // height or text height with a slight offset top and bottom.
        // Otherwise, it can get drawn outside the bounds of the cell.
        size.cy -= (nOffset * 2);

        if(size.cy >= rect.Height())
            size.cy = rect.Height() - (nOffset * 2);

        size.cx = size.cy;      // Make the dimensions square

        CPen penShadow(PS_SOLID, 0, ::GetSysColor(COLOR_3DSHADOW));
        CPen penLight(PS_SOLID, 0, ::GetSysColor(COLOR_3DHILIGHT));
        if (m_pGrid->m_bAscending)
        {
            // Draw triangle pointing upwards
            CPen *pOldPen = (CPen*) pDC->SelectObject(&penLight);
            pDC->MoveTo( rect.right - size.cx + 1, rect.top + nOffset + size.cy + 1);
            pDC->LineTo( rect.right - (size.cx / 2) + 1, rect.top + nOffset + 1 );
            pDC->LineTo( rect.right + 1, rect.top + nOffset + size.cy + 1);
            pDC->LineTo( rect.right - size.cx + 1, rect.top + nOffset + size.cy + 1);

            pDC->SelectObject(&penShadow);
            pDC->MoveTo( rect.right - size.cx, rect.top + nOffset + size.cy );
            pDC->LineTo( rect.right - (size.cx / 2), rect.top + nOffset );
            pDC->LineTo( rect.right, rect.top + nOffset + size.cy );
            pDC->LineTo( rect.right - size.cx, rect.top + nOffset + size.cy );
            pDC->SelectObject(pOldPen);
        }
        else
        {
            // Draw triangle pointing downwards
            CPen *pOldPen = (CPen*) pDC->SelectObject(&penLight);
            pDC->MoveTo( rect.right - size.cx + 1, rect.top + nOffset + 1 );
            pDC->LineTo( rect.right - (size.cx / 2) + 1, rect.top + nOffset + size.cy + 1 );
            pDC->LineTo( rect.right + 1, rect.top + nOffset + 1 );
            pDC->LineTo( rect.right - size.cx + 1, rect.top + nOffset + 1 );

            pDC->SelectObject(&penShadow);
            pDC->MoveTo( rect.right - size.cx, rect.top + nOffset );
            pDC->LineTo( rect.right - (size.cx / 2), rect.top + nOffset + size.cy );
            pDC->LineTo( rect.right, rect.top + nOffset );
            pDC->LineTo( rect.right - size.cx, rect.top + nOffset );
            pDC->SelectObject(pOldPen);
        }

        rect.right -= size.cy;
    }
#endif
    // We want to see '&' characters so use DT_NOPREFIX
	if (m_pGrid)
		m_pGrid->DrawCellText(pDC, rect, nRow, nCol);
//		m_pGrid->GetText(m_strText, nRow, nCol);
//    DrawText(pDC->m_hDC, m_strText, -1, rect, m_nFormat | DT_NOPREFIX);

    pDC->RestoreDC(nSavedDC);
    Font.DeleteObject();

    return TRUE;
}

BOOL CGridCell::Edit(int nRow, int nCol, CRect rect, UINT nID, UINT nChar)
{
    DWORD dwStyle = ES_LEFT;
    if (m_nFormat & DT_RIGHT) 
        dwStyle = ES_RIGHT;
    else if (m_nFormat & DT_CENTER) 
        dwStyle = ES_CENTER;

    m_bEditing = TRUE;
    
    // InPlaceEdit auto-deletes itself
    m_pEditWnd = new CInPlaceEdit(m_pGrid, rect, dwStyle, nID, nRow, nCol, m_strText, nChar);
    
    return TRUE;
}

void CGridCell::EndEdit()
{
    if (m_pEditWnd)
        ((CInPlaceEdit*)m_pEditWnd)->EndEdit();
}

void CGridCell::OnEndEdit()
{
    m_bEditing = FALSE;
    m_pEditWnd = NULL;
}

// Not yet implemented
void CGridCell::OnMouseEnter()
{
    TRACE0("Mouse entered cell\n");
}

void CGridCell::OnMouseOver()
{
    //TRACE0("Mouse over cell\n");
}

// Not Yet Implemented
void CGridCell::OnMouseLeave()
{
    TRACE0("Mouse left cell\n");
}

void CGridCell::OnClick()
{
    TRACE0("Mouse clicked in cell\n");
}

void CGridCell::OnDblClick()
{
    TRACE0("Mouse double-clicked in cell\n");
}

// Return TRUE if you set the cursor
BOOL CGridCell::OnSetCursor()
{
#ifndef _WIN32_WCE_NO_CURSOR
    SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
#endif
    return TRUE;
}

// Gets the extent of the text pointed to by str (no CDC needed)
// By default this uses the selected font (which is a bigger font)
CSize CGridCell::GetTextExtent(LPCTSTR str)
{
    CDC* pDC = m_pGrid->GetDC();
    if (!pDC) return CSize(0,0);

    CFont *pOldFont, font;

    if (GetState() & GVIS_FIXED)
    {
        LOGFONT lf;
        memcpy(&lf, GetFont(), sizeof(LOGFONT));
        lf.lfWeight = SELECTED_CELL_FONT_WEIGHT;

        font.CreateFontIndirect(&lf);

        pOldFont = pDC->SelectObject(&font);
    }
    else
	{
        font.CreateFontIndirect( GetFont() );
        pOldFont = pDC->SelectObject(&font);
    }

    CSize size;

    // EFW - If null, use current cell text
    if(!str)
        size = pDC->GetTextExtent(m_strText);
    else
        size = pDC->GetTextExtent(str);

    pDC->SelectObject(pOldFont);
    m_pGrid->ReleaseDC(pDC);

    return size + CSize(2*m_nMargin, 2*m_nMargin);
}

CSize CGridCell::GetCellExtent(CDC* pDC)
{
    LOGFONT *pLF = GetFont();

    // use selected font since it's thicker
    LOGFONT lf;
    memcpy(&lf, pLF, sizeof(LOGFONT));

    if (GetState() & GVIS_FIXED)
        lf.lfWeight = SELECTED_CELL_FONT_WEIGHT;

    CFont font;
    font.CreateFontIndirect(&lf);

    CSize size;
    int nFormat = GetFormat();

    // If the cell is a multiline cell, then use the width of the cell
    // to get the height
    CFont* pOldFont = pDC->SelectObject(&font);
    if ((nFormat & DT_WORDBREAK) && !(nFormat & DT_SINGLELINE))
    {
        CString str = m_strText;
        int nMaxWidth = 0;
        while (TRUE)
        {
            int nPos = str.Find(_T('\n'));
            CString TempStr = (nPos < 0)? str : str.Left(nPos);
            int nTempWidth = pDC->GetTextExtent(TempStr).cx;
            if (nTempWidth > nMaxWidth)
                nMaxWidth = nTempWidth;

            if (nPos < 0)
                break;
            str = str.Mid(nPos + 1);    // Bug fix by Thomas Steinborn
        }
        
        CRect rect;
        rect.SetRect(0,0, nMaxWidth, 0);
        pDC->DrawText(m_strText, rect, nFormat | DT_CALCRECT);
        size = rect.Size();
    }
    else
        size = pDC->GetTextExtent(m_strText);
    pDC->SelectObject(pOldFont);
    
    size += CSize(4*m_nMargin, 2*m_nMargin);
    
    CSize ImageSize(0,0);
    if (m_pGrid->GetImageList()) 
    {
        int nImage = GetImage();
        if (nImage >= 0) 
        {
            IMAGEINFO Info;
            if (m_pGrid->GetImageList()->GetImageInfo(nImage, &Info))
                ImageSize = CSize(Info.rcImage.right-Info.rcImage.left+1, 
                                  Info.rcImage.bottom-Info.rcImage.top+1);
        }
    }
    
    return CSize(size.cx + ImageSize.cx, max(size.cy, ImageSize.cy));
}

// EFW - Added to print cells so that grids that use different colors are
// printed correctly.
BOOL CGridCell::PrintCell(CDC* pDC, int /*nRow*/, int /*nCol*/, CRect rect)
{
    COLORREF crFG, crBG;
    GV_ITEM Item;

    if (!m_pGrid || !pDC)
        return FALSE;

    int nSavedDC = pDC->SaveDC();

    BOOL bDrawFixed = ((m_nState & GVIS_FIXED) == GVIS_FIXED);

    pDC->SetBkMode(TRANSPARENT);

    if(m_pGrid->m_bShadedPrintOut)
    {
        // Use custom color if it doesn't match the default color and the
        // default grid background color.  If not, leave it alone.
        if(bDrawFixed)
            crBG = (m_crBkClr != CLR_DEFAULT) ? m_crBkClr : m_pGrid->GetFixedBkColor();
        else
            crBG = (m_crBkClr != CLR_DEFAULT && m_crBkClr != m_pGrid->GetTextBkColor()) ?
                m_crBkClr : CLR_DEFAULT;

        // Use custom color if the background is different or if it doesn't
        // match the default color and the default grid text color.  If not,
        // use black to guarantee the text is visible.
        if(bDrawFixed)
            crFG = (m_crBkClr != CLR_DEFAULT) ? m_crFgClr : m_pGrid->GetFixedTextColor();
        else
            crFG = (m_crBkClr != CLR_DEFAULT ||
                (m_crFgClr != CLR_DEFAULT && m_crFgClr != m_pGrid->GetTextColor())) ?
                    m_crFgClr : RGB(0, 0, 0);

        // If not printing on a color printer, adjust the foreground color
        // to a gray scale if the background color isn't used so that all
        // colors will be visible.  If not, some colors turn to solid black
        // or white when printed and may not show up.  This may be caused by
        // coarse dithering by the printer driver too (see image note below).
        if(pDC->GetDeviceCaps(NUMCOLORS) == 2 && crBG == CLR_DEFAULT)
            crFG = RGB(GetRValue(crFG) * 0.30, GetGValue(crFG) * 0.59,
                GetBValue(crFG) * 0.11);

        // Only erase the background if the color is not the default
        // grid background color.
        if(crBG != CLR_DEFAULT)
        {
            CBrush brush(crBG);
            rect.right++; rect.bottom++;
            pDC->FillRect(rect, &brush);
            rect.right--; rect.bottom--;
        }
    }
    else
    {
        crBG = CLR_DEFAULT;
        crFG = RGB(0, 0, 0);
    }

    pDC->SetTextColor(crFG);

    // Create the appropriate font and select into DC.
    CFont Font;

    // Bold the fixed cells if not shading the print out.  Use italic
    // font it it is enabled.
    if(bDrawFixed && !m_pGrid->m_bShadedPrintOut)
    {
        Font.CreateFont(m_lfFont.lfHeight, 0, 0, 0, FW_BOLD, m_lfFont.lfItalic, 0, 0,
            ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
#ifndef _WIN32_WCE
            PROOF_QUALITY,
#else
            DEFAULT_QUALITY,
#endif
            VARIABLE_PITCH | FF_SWISS, m_lfFont.lfFaceName);
    }
    else
        Font.CreateFontIndirect(&m_lfFont);

    pDC->SelectObject(&Font);

    // Draw lines only when wanted on fixed cells.  Normal cell grid lines
    // are handled in OnPrint.
    if(m_pGrid->m_nGridLines != GVL_NONE && bDrawFixed)
    {
        CPen lightpen(PS_SOLID, 1,  ::GetSysColor(COLOR_3DHIGHLIGHT)),
             darkpen(PS_SOLID,  1, ::GetSysColor(COLOR_3DDKSHADOW)),
            *pOldPen = pDC->GetCurrentPen();

        pDC->SelectObject(&lightpen);
        pDC->MoveTo(rect.right, rect.top);
        pDC->LineTo(rect.left, rect.top);
        pDC->LineTo(rect.left, rect.bottom);

        pDC->SelectObject(&darkpen);
        pDC->MoveTo(rect.right, rect.top);
        pDC->LineTo(rect.right, rect.bottom);
        pDC->LineTo(rect.left, rect.bottom);

        rect.DeflateRect(1,1);
        pDC->SelectObject(pOldPen);
    }

    rect.DeflateRect(m_nMargin, 0);

    if(m_pGrid->GetImageList() && m_nImage >= 0)
    {
        // NOTE: If your printed images look like fuzzy garbage, check the
        //       settings on your printer driver.  If it's using coarse
        //       dithering and/or vector graphics, they may print wrong.
        //       Changing to fine dithering and raster graphics makes them
        //       print properly.  My HP 4L had that problem.

        IMAGEINFO Info;
        if(m_pGrid->GetImageList()->GetImageInfo(m_nImage, &Info))
        {
            int nImageWidth = Info.rcImage.right-Info.rcImage.left;
            m_pGrid->GetImageList()->Draw(pDC, m_nImage, rect.TopLeft(), ILD_NORMAL);
            rect.left += nImageWidth+m_nMargin;
        }
    }

    // Draw without clipping so as not to lose text when printed for real
    DrawText(pDC->m_hDC, m_strText, -1, rect,
        m_nFormat | DT_NOCLIP | DT_NOPREFIX);

    pDC->RestoreDC(nSavedDC);
    Font.DeleteObject();

    return TRUE;
}
