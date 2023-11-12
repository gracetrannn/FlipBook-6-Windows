#pragma once

#include <cstdint>

typedef uint32_t        DWORD;
typedef uint16_t        WORD;
typedef int32_t         LONG;
typedef const char*     LPCSTR;
typedef char*           LPSTR;
typedef uint32_t        UINT;
typedef unsigned char   BYTE;
typedef BYTE*           LPBYTE;
typedef uint32_t        COLORREF;
typedef int32_t         HRESULT;
typedef uint64_t        ULONGLONG;
typedef double          DOUBLE;
typedef int32_t         INT;

#define S_OK    ((HRESULT)0L)

#define TRUE    1
#define FALSE   0

#define LOBYTE(w)           ((BYTE)(((size_t)(w)) & 0xff))
#define HIBYTE(w)           ((BYTE)((((size_t)(w)) >> 8) & 0xff))

#define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
#define GetRValue(rgb)      (LOBYTE(rgb))
#define GetGValue(rgb)      (LOBYTE(((WORD)(rgb)) >> 8))
#define GetBValue(rgb)      (LOBYTE((rgb)>>16))

/* Global Memory Flags */
#define GMEM_FIXED          0x0000
#define GMEM_MOVEABLE       0x0002
#define GMEM_NOCOMPACT      0x0010
#define GMEM_NODISCARD      0x0020
#define GMEM_ZEROINIT       0x0040
#define GMEM_MODIFY         0x0080
#define GMEM_DISCARDABLE    0x0100
#define GMEM_NOT_BANKED     0x1000
#define GMEM_SHARE          0x2000
#define GMEM_DDESHARE       0x2000
#define GMEM_NOTIFY         0x4000
#define GMEM_LOWER          GMEM_NOT_BANKED
#define GMEM_VALID_FLAGS    0x7F72
#define GMEM_INVALID_HANDLE 0x8000

#define GHND                (GMEM_MOVEABLE | GMEM_ZEROINIT)
#define GPTR                (GMEM_FIXED | GMEM_ZEROINIT)

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

struct BITMAPINFOHEADER {
    uint32_t biSize; // 40
    int32_t biWidth; // width of image
    int32_t biHeight; // height of image
    int16_t biPlanes; // 0, number of planes
    int16_t biBitCount; // 8, bits per pixel
    uint32_t biCompression; // 0, no compression
    uint32_t biSizeImage; // 0, image data size
    int32_t biXPelsPerMeter; // 0, horizontal pixels per meter
    int32_t biYPelsPerMeter; // 0, vertical pixels per meter
    uint32_t biClrUsed; // 0, number of colors if not in 8bit
    uint32_t biClrImportant; // 0, number of important colors
};

typedef BITMAPINFOHEADER* LPBITMAPINFOHEADER;

typedef struct tagBITMAPFILEHEADER {
    WORD    bfType;
    DWORD   bfSize;
    WORD    bfReserved1;
    WORD    bfReserved2;
    DWORD   bfOffBits;
} BITMAPFILEHEADER, *LPBITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagRECT
{
    int32_t    left;
    int32_t    top;
    int32_t    right;
    int32_t    bottom;
} RECT, * PRECT, *LPRECT;

typedef struct tagPOINT
{
    int32_t  x;
    int32_t  y;
} POINT, *PPOINT, *LPPOINT;
