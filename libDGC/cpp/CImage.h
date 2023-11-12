#pragma once
#include "CommonDefs.h"

#define BI_RGB        0L

class CImage {
public:
	HRESULT Load(LPCSTR pszFileName);
	int GetWidth() const;
	int GetHeight() const;
	int GetBPP() const;
	void Destroy();
	void* GetPixelAddress(int x, int y);
};