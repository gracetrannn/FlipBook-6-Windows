#include "ImageUtils.h"
#include "CImage.h"
#include "utils.h"
#include <exception>

#ifndef IMPLEMENT_WINDOWS_APIS
	#include <windowsx.h>
#endif

void* GetCImage(LPCSTR pName, UINT& isize)
{
	CImage image;
	HRESULT res = image.Load(pName);
	if (res != S_OK)
		return 0;
	// feeble attempt to get actual DPI
	//CBitmap * pBitmap = CBitmap::FromHandle(image);
	//CSize ss = pBitmap->GetBitmapDimension();
	int nWidth = image.GetWidth();
	int nHeight = image.GetHeight();
	int bits = image.GetBPP();
	int pitch = 4 * ((bits * nWidth + 31) / 32);
	DWORD dwLength = pitch * nHeight;
	isize = sizeof(BITMAPINFOHEADER) + dwLength;
	isize += 1024; // safe
	LPBITMAPINFOHEADER lpBI;
	if ((bits == 24) || (bits == 32) || (bits == 8) || (bits == 1))
		lpBI = (LPBITMAPINFOHEADER)new BYTE[isize];
	else
		lpBI = 0;
	if (lpBI == 0)
	{
		image.Destroy();
		return 0;
	}
	lpBI->biSize = sizeof(BITMAPINFOHEADER);
	lpBI->biWidth = nWidth;
	lpBI->biHeight = nHeight;
	lpBI->biPlanes = 1;
	lpBI->biBitCount = bits;
	lpBI->biCompression = BI_RGB;
	lpBI->biSizeImage = dwLength;
	lpBI->biXPelsPerMeter = 0;//(50 + dpi * 3939 / 100);
	lpBI->biYPelsPerMeter = 0;//(50 + dpi * 3939 / 100);
	lpBI->biClrUsed = (bits > 8) ? 0 : (1 << bits);
	lpBI->biClrImportant = 0;
	BYTE* lpBits = (BYTE*)lpBI;
	lpBits += lpBI->biSize;
	int x, y, d;
	d = bits / 8;
	if (bits < 9)
	{
		int c = 1 << (2 + bits);
		int f = 257 - (1 << bits);
		for (x = 0; x < c; x++)
			*lpBits++ = ((x & 3) == 3) ? 0 : (x / 4) * f;
	}
	if (bits < 9)
		memset(lpBits, 0, dwLength);
	for (y = 0; y < nHeight; y++, lpBits += pitch)
	{
		for (x = 0; x < nWidth; x++)
		{
			BYTE* p = (BYTE*)image.GetPixelAddress(x, nHeight - 1 - y);
			if (d > 1)
			{
				lpBits[x * d + 0] = p[0];
				lpBits[x * d + 1] = p[1];
				lpBits[x * d + 2] = p[2];
				if (d == 4)
					lpBits[x * d + 3] = p[3];
			}
			else if (d == 0)
			{
				BYTE v = p[0] & (1 << (x & 7));
				lpBits[x / 8] |= v;
			}
			else
				lpBits[x * d + 0] = p[0];
		}
	}
	image.Destroy();
	return lpBI;
}

void* GetBMP(CFile& file, UINT& size)
{
	BITMAPFILEHEADER bmfHeader;
	if (file.Read((LPSTR)&bmfHeader, sizeof(bmfHeader)) != sizeof(bmfHeader))
		return 0;
	if (bmfHeader.bfType != DIB_HEADER_MARKER)
		return 0;
	DWORD dwReadBytes = sizeof(bmfHeader);
	DWORD dwLength = (DWORD)file.GetLength() - sizeof(bmfHeader);
	if (!dwLength)
		return 0;
	LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)(new BYTE[dwLength]);
	if (lpBI == 0)
		return 0;
	// Read header.
	if (file.Read(lpBI, dwLength) != dwLength)
	{
#pragma warning(suppress: 6387)
		delete[](LPBYTE)lpBI;
		lpBI = 0;
	}
	size = dwLength;
	return lpBI;
}

void* GetTGA(CFile& file, UINT& isize)
{
	BYTE hdr[18];
	file.Seek(0, CFile::begin);
	if (file.Read((LPSTR)&hdr, sizeof(hdr)) != sizeof(hdr))
		return 0;
	if (hdr[0] || hdr[1])
		return 0;
	bool bFlip = (hdr[17] & 32) ? 1 : 0;
	UINT k = hdr[2];
	UINT d = hdr[16] & 255;
	UINT w = hdr[12] + 256 * hdr[13];
	UINT h = hdr[14] + 256 * hdr[15];
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
	isize = sizeof(BITMAPINFOHEADER) + dwLength;
	if (d == 8)
		isize += 1024;
	LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)MyAlloc(isize);
	if (lpBI == 0)
		return 0;
	lpBI->biSize = sizeof(BITMAPINFOHEADER);
	lpBI->biWidth = w;
	lpBI->biHeight = h;
	lpBI->biPlanes = 1;
	lpBI->biBitCount = d;
	lpBI->biCompression = BI_RGB;
	lpBI->biSizeImage = dwLength;
	lpBI->biXPelsPerMeter = 0;
	lpBI->biYPelsPerMeter = 0;
	lpBI->biClrUsed = d > 8 ? 0 : 256;
	lpBI->biClrImportant = 0;
	BYTE* pBits = (BYTE*)lpBI;
	pBits += lpBI->biSize;
	if (d == 8)
	{
		int i;
		for (i = 0; i < 256; i++)
		{
			*pBits++ = i;
			*pBits++ = i;
			*pBits++ = i;
			*pBits++ = 0;
		}
	}
	BYTE* p = pBits;
	if (bFlip)
		p += op * h;
	UINT depth = d / 8;
	UINT ip = depth * w;
	UINT y;
	for (y = 0; y < h; y++)
	{
		if (bFlip)
			p -= op;
		if ((k == 2) || (k == 3))
		{
			if (file.Read(p, ip) != ip)
				break;
		}
		else
		{
			BYTE* pp = p;
			UINT x;
			for (x = 0; x < w;)
			{
				BYTE code;
				file.Read(&code, 1);
				if (code & 128)
				{
					code = code & 127;
					if ((x + code) > w)
						code = w - x;
					BYTE* zp = pp;
					file.Read(zp, depth);
					pp += depth;
					x++;
					if ((x + code) > w)
						code = w - x;
					x += code;
					for (; code--; pp += depth)
						memcpy(pp, zp, depth);
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
		if (!bFlip)
			p += op;
	}
	if (y < h)
		return 0;
	//	CreatePalette();
	return lpBI;
}

int FBImportStill(const char* inFilePath,
	ImageDataCallback inFunction, void* inUserObject)
{
	int res = 0;
	UINT size;
	void* p = 0;
	try
	{
		CFile file(inFilePath, CFile::modeRead);
		p = GetBMP(file, size);
		if (!p)
		{
			file.Seek(0, CFile::begin);
			p = GetTGA(file, size);
		}
	}
	catch (std::exception& e)
	{
		return 1;
	}
	if (!p)
	{
		p = GetCImage(inFilePath, size);
	}
	if (p)
	{
		res = inFunction(kBMPDataType, p, size, inUserObject);
		delete[](LPBYTE)p;
	}
	return res;
}
