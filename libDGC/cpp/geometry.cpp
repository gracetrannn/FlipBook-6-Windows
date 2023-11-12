#ifdef IMPLEMENT_WINDOWS_APIS

#include "geometry.h"

CPoint::CPoint()
{
    x = 0;
    y = 0;
}

CPoint::CPoint(int inX, int inY)
    : x(inX), y(inY)
{
}

#pragma mark -

CSize::CSize()
    : cx(0), cy(0)
{
}

CSize::CSize(int inWidth, int inHeight)
    : cx(inWidth), cy(inHeight)
{
}

void CSize::SetWidth(int inWidth)
{
    cx = inWidth;
}

void CSize::SetHeight(int inHeight)
{
    cy = inHeight;
}

#pragma mark -

CRect::CRect()
{
    top = 0;
    left = 0;
    bottom = 0;
    right = 0;
}

CRect::CRect(int inLeft, int inTop, int inRight, int inBottom)
    : top(inTop), left(inLeft), right(inRight), bottom(inBottom)
{
}

int CRect::Width()
{
    return (right - left);
}

int CRect::Height()
{
    return (bottom - top);
}

CPoint CRect::TopLeft()
{
    return CPoint(left, top);
}

CPoint CRect::BottomRight()
{
    return CPoint(right, bottom);
}

bool CRect::PtInRect(CPoint& inPoint)
{
    return ((inPoint.x > left) && (inPoint.x < right) && (inPoint.y > top) && (inPoint.y < bottom));
}

void CRect::InflateRect(int inXDelta, int inYDelta)
{
    top -= inYDelta;
    left -= inXDelta;
    bottom += inYDelta;
    right += inXDelta;
}

void CRect::SetRect(int inLeft, int inTop, int inRight, int inBottom)
{
    top = inTop;
    left = inLeft;
    bottom = inBottom;
    right = inRight;
}

void CRect::OffsetRect(int inXDelta, int inYDelta)
{
    left += inXDelta;
    bottom += inYDelta;
}

#endif // IMPLEMENT_WINDOWS_APIS