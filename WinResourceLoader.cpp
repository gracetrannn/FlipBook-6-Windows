#include "stdafx.h"
#include "WinResourceLoader.h"
#include <cstdint>

ResourcePtr WinResourceLoader::LoadImprint()
{
	HRSRC hRes = FindResource(AfxGetApp()->m_hInstance, "IMPRINT", "DGCRES");
	if (!hRes) {
		return ResourcePtr(nullptr);
	}

	HGLOBAL hand = LoadResource(AfxGetApp()->m_hInstance, hRes);
	if (!hand)
	{
		return ResourcePtr(nullptr);
	}
	BYTE* hpSrc = (BYTE*)LockResource(hand);
	if (!hpSrc)
	{
		return ResourcePtr(nullptr);
	}

	ResourceHeader* pHeader = (ResourceHeader*)hpSrc;
	size_t bufferSize = sizeof(ResourceHeader) + pHeader->outSize;

	auto result = std::make_unique<unsigned char[]>(bufferSize);
	memcpy(result.get(), hpSrc, bufferSize);
	UnlockResource(hand);

	return result;
}

ResourcePtr WinResourceLoader::LoadDefaultPalette()
{
	HRSRC hRes = FindResource(AfxGetApp()->m_hInstance, "PALETTE", "DGCRES");
	if (!hRes)
	{
		return ResourcePtr(nullptr);
	}

	HGLOBAL hand = LoadResource(AfxGetApp()->m_hInstance, hRes);
	if (!hand)
	{
		return ResourcePtr(nullptr);
	}

	BYTE* hpSrc = (BYTE*)LockResource(hand);
	if (!hpSrc)
	{
		return ResourcePtr(nullptr);
	}

	const size_t paletteSize = 1026; // initpal.dat file size
	auto result = std::make_unique<unsigned char[]>(paletteSize);
	memcpy(result.get(), hpSrc, paletteSize);
	UnlockResource(hand);

	return result;
}
