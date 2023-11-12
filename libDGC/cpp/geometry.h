#pragma once

class CPoint
{
public:

    CPoint();
    CPoint(int inX, int inY);

    int x;
    int y;
};

class CSize
{
public:

    CSize();
    CSize(int inWidth, int inHeight);

    void SetWidth(int inWidth);
    void SetHeight(int inHeight);

    int cx;
    int cy;
};

class CRect
{
public:

    CRect();
    CRect(int inLeft, int inTop, int inRight, int inBottom);

    int Width();
    int Height();
    CPoint TopLeft();
    CPoint BottomRight();

    bool PtInRect(CPoint& inPoint);
    void InflateRect(int inXDelta, int inYDelta);
    void SetRect(int inLeft, int inTop, int inRight, int inBottom);
    void OffsetRect(int inXDelta, int inYDelta);

    int top;
    int left;
    int bottom;
    int right;
};