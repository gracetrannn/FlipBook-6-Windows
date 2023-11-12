#include "stdafx.h"
#include "CScene.h"
#include "MYJPEG.H"
#include "CLAYERS.H"
#include "CScaler.h"
#include "CCell.h"

void BlurOver(BYTE* pTmp, BYTE* pSrc, UINT w, UINT h, UINT pitch)
{
	UINT x, y, x1, y1;
	int i, j, r;
	r = 4;
	for (y = 0; y < h; y++)
		for (x = 0; x < w; x++)
		{
			if (pSrc[pitch * y + 4 * x + 3])
			{
				UINT d = 0;
				UINT s = 0;
				for (i = -r; i <= r; i++)
					for (j = -r; j <= r; j++)
					{
						x1 = x + j;
						y1 = y + i;
						if ((x1 < w) && (y1 < h))
						{
							UINT q;
							if (q = pSrc[pitch * y1 + 4 * x1 + 3])
							{
								s += 255; // force full alpha
								d++;
							}
							else
								d += 2 * r;
						}
					}
				pTmp[y * w + x] = s / d;
			}
			else
				pTmp[y * w + x] = 0;
		}

	for (y = 0; y < h; y++)
		for (x = 0; x < w; x++)
			pSrc[pitch * y + 4 * x + 3] = pTmp[y * w + x];
}

void CScene::ApplyFrameCount(HPBYTE pBuf, UINT frame)
{
	CDC MemDC;
	MemDC.CreateCompatibleDC(0);
	CString str;
	str.Format("Frame: %04d", 0);
	CSize cc = MemDC.GetTextExtent(str);
	if (m_OptFlags & 0x8000000)
		frame++;
	str.Format("Frame :%4d", frame);
	UINT nFormat = DT_SINGLELINE | DT_LEFT | DT_BOTTOM;
	// using 24 bit text for simplicity

	UINT textw = cc.cx;
	UINT texth = cc.cy;
	UINT bits = 24;//8 * m_depth;
	UINT pitch = 4 * ((bits * textw + 31) / 32);
	UINT size = pitch * texth;
	BITMAPINFO bmi;
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = textw;
	bmi.bmiHeader.biHeight = texth;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = bits;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = size;
	bmi.bmiHeader.biXPelsPerMeter = 0;
	bmi.bmiHeader.biYPelsPerMeter = 0;
	bmi.bmiHeader.biClrUsed = 0;
	bmi.bmiHeader.biClrImportant = 0;
	LPBYTE pBits = 0;
	HBITMAP  hBitmap =
		CreateDIBSection(0, &bmi, DIB_RGB_COLORS, (void**)&pBits, NULL, NULL);
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(MemDC, hBitmap);

	CRect rect(0, 0, textw, texth);

	MemDC.FillSolidRect(&rect, RGB(255, 255, 255));
	MemDC.DrawText(str, -1, (LPRECT)rect, nFormat);
	MemDC.SelectObject(hOldBitmap);
	int x, y;
	UINT i, j;
	x = 0;
	y = 0;
	if (!(m_OptFlags & 0x4000000))
		y = m_height - texth; // upper
	if (m_OptFlags & 0x2000000)
		x = m_width - textw; // right
	UINT dpitch = 4 * ((m_depth * m_width + 3) / 4);
	for (i = 0; i < texth; i++)
		for (j = 0; j < textw; j++)
		{
			UINT k;
			for (k = 0; k < m_depth; k++)
				pBuf[dpitch * (y + i) + m_depth * (x + j) + k] = pBits[pitch * i + 3 * j + k];
		}
	DeleteObject(hBitmap);
}

void CScene::CompositeFrame(BYTE * pBuf, UINT StartLevel, UINT EndLevel,
	UINT Frame, BOOL bBroadcast)
{
	UINT Level;
	if (!Broadcast())
		bBroadcast = 0;
	//	if ((m_CamState ) && (m_CamFrame == Frame))
	//		memset(m_pCamBuf, 255, ComW() * ComH()); 
	if (!StartLevel && (LevelFlags(0) & 1))
	{
		GetLevel0(pBuf, Frame, 1, m_MinBG, 1, bBroadcast);
		StartLevel++;
	}
	else
	{
		if (m_depth == 1) // no bg so fill with white and no alpha
			memset(pBuf, 255, m_size);
		else if (m_depth == 3)
			memset(pBuf, 255, m_size);
		else
		{
			UINT w = ComW();
			UINT h = ComH();
			UINT x, y;
			for (y = 0; y < h; y++)
				for (x = 0; x < w; x++)
				{
					pBuf[4 * (y * w + x) + 0] = 255;
					pBuf[4 * (y * w + x) + 1] = 255;
					pBuf[4 * (y * w + x) + 2] = 255;
					pBuf[4 * (y * w + x) + 3] = 0;
				}
		}
	}
	for (Level = StartLevel; Level <= EndLevel; Level++)
		if (LevelFlags(Level) & 1)
		{
			ApplyCell(pBuf, Frame, Level, 1, bBroadcast);
		}
	if (m_uState)
		ApplyImprint(pBuf);
	UINT Mask = bBroadcast ? 0x20000000 : 0x10000000;
	if ((m_OptFlags & Mask) && (m_depth != 4))
		ApplyFrameCount(pBuf, Frame);
}

void CScene::CompositeFrame32(BYTE* pBuf, UINT StartLevel, UINT EndLevel, UINT Frame, BOOL b32)
{
	/*
		if (StartLevel == 9999)
			{
			CheckComposite(Frame);
			memmove(pBuf, m_pCache[Frame],m_size);
			return;
			}
	*/
	UINT zfact = m_factor;
	UINT zw = ComW();
	UINT zh = ComH();
	UINT zize = m_size;
	UINT zpp = m_depth;
	if (Broadcast())
	{
		m_factor = m_origfactor;
		m_w = ComW();
		m_h = ComH();
	}
	if (b32)
		m_depth = 4;
	m_size = m_h * m_depth * m_w;
	CompositeFrame(pBuf, StartLevel, EndLevel, Frame, TRUE);
	m_depth = zpp;
	m_size = zize;
	m_factor = zfact;
	m_w = ComW();
	m_h = ComH();
}

void CScene::CompositeCacheFrame(UINT Frame, BOOL bBroadcast)
{
	UINT StartLevel = 0;
	UINT EndLevel = m_levels - 1;
	BYTE* pDest = m_pCache[Frame];
	if (m_jpeg_quality)
	{
		UINT w = ComW();
		UINT h = ComH();
		CompositeFrame(m_pJpeg, StartLevel, EndLevel, Frame, TRUE);
		int size;
		if (m_depth == 1)
		{
			UINT x, y;
			for (y = h; y-- > 0;)   // for insitu
			{
				for (x = 0; x < w; x++)
				{
					m_pJpeg[y * w * 3 + 3 * x + 0] = m_pJpeg[w * y + x];
					m_pJpeg[y * w * 3 + 3 * x + 1] = m_pJpeg[w * y + x];
					m_pJpeg[y * w * 3 + 3 * x + 2] = m_pJpeg[w * y + x];
				}
			}
		}
		BYTE* pTemp = m_pJpeg + 3 * w * h;
		int  z = m_pJEnc->CompressImage(
			m_pJpeg,
			pTemp,
			w, h,// + (m_depth == 1 ? 4096 : 0),
			size);
		if (!z)
			return;//size = -1;
		int cur_size = *((int*)pDest);
		if ((size > (cur_size - 8)) || (size < (9 * cur_size / 10)))
		{
			delete[] m_pCache[Frame];
			cur_size = 11 * size / 10;
			m_pCache[Frame] = new BYTE[cur_size];
			pDest = m_pCache[Frame];
			*((int*)pDest) = cur_size;
		}
		pDest += 4;
		*((int*)pDest) = size;
		memmove(pDest + 4, pTemp, size);
	}
	else
	{
		CompositeFrame(pDest, StartLevel, EndLevel, Frame, bBroadcast);
	}
}

int CScene::CheckComposite(UINT zFrame, UINT Force /* = 0 */, BOOL bBroadcast /* = 0 */)
{
	UINT Frame;
	int nResult = 0;
	if (!Force && (Broadcast() == 2))
		bBroadcast = 1;
	if (Flag(SCN_FLG_REDBOX))
		bBroadcast = 0;
	if (Force == 3)
	{
		ASSERT(zFrame < m_frames);
		//		m_pScene->SetCamInfo(m_bPlaying,m_Frame,m_CamState);
		//		m_CamState = state;
		//		m_pFlags[frame] = 1;
		//		m_CamFrame = frame;
		m_CamState = 1;
		UINT w = ComW();
		UINT h = ComH();
		BYTE* dp = m_pCamBuf + w * h;
		memset(m_pCamBuf, 255, ComW() * ComH());
		CompositeFrame(dp, 0, m_levels - 1, zFrame,
			bBroadcast);
		//		m_CamState = 0;
		return 0;
	}
	if (Force == 2)
	{
		for (Frame = 0; Frame < m_frames; m_pFlags[Frame++] = 1);
		if (zFrame == -1)
			return 0;
	}
	if (zFrame != -1)
	{
		//if (!m_pFlags[zFrame]) return 0;
		ASSERT(zFrame < m_frames);
		if (zFrame >= m_frames)
			return nResult;
		if (m_pLayers && m_pLayers->NeedFake(zFrame))
			Force = 1;
		DPF("got zframe:%d", zFrame);
		if (m_pFlags[zFrame] || Force)
		{
			CompositeCacheFrame(zFrame, bBroadcast);
			m_pFlags[zFrame] = 0;
			nResult = 50;
		}
		return nResult;
	}
	for (Frame = m_start; Frame <= m_stop; Frame++)
	{
		if (m_pFlags[Frame])
		{
			DPZ("dirty:%d", Frame);
			CompositeCacheFrame(Frame, bBroadcast);
			m_pFlags[Frame] = 0;
			nResult = MulDiv(100, m_frames - Frame, m_frames);
			break;
		}
	}
	if (Flag(SCN_FLG_REDBOX))
		return nResult;		// no sense doing broadcast if red box mode
#ifdef DOBROADCAST
	if (!nResult && !m_cache_state)
	{
		for (Frame = m_start; Frame <= m_stop; Frame++)
			m_pFlags[Frame] = 1;
		m_cache_state = 1;
		nResult = 100;
	}
#endif
	//DPF("cache clean:%d",Frame);
	return nResult;
}

UINT CScene::ConvertGray(HPBYTE hpGray, LPBITMAPINFOHEADER  lpbi,
	UINT Rotation, UINT nFit)
{
	UINT result;
	DPF("cvting gray,rot:%d", Rotation);
	DWORD w, h, dd;
	w = lpbi->biWidth;
	h = lpbi->biHeight;
	dd = lpbi->biBitCount;
	DPF("w:%d,h:%d,d:%d", w, h, dd);
	if ((dd != 8) && (dd != 16) && (dd != 24) && (dd != 1) && (dd != 32))
		return 1;
	DWORD gw = m_width;
	DWORD gh = m_height;
	DWORD gpitch = 4 * ((gw + 3) / 4);
	DWORD dwSize = gpitch * gh;
	DWORD ipitch = 4 * ((w * dd + 31) / 32);
	HPBYTE hpBuf = (HPBYTE)lpbi;
	hpBuf += 40;	// for header
	BYTE* pals = hpBuf;
	if (dd < 9)
	{
		hpBuf += 4 << dd;	// for palette
	}
	CSScaler scale;
	DWORD ww, hh;
	if (Rotation & 1)
	{
		ww = gh;
		hh = gw;
	}
	else
	{
		ww = gw;
		hh = gh;
	}
	scale.m_aw = w;
	scale.m_ah = h;
	//	if (w < ww)
	//		w = ww;
	//	if (h < hh)
	//		h = hh;
	UINT dq = (dd > 1) ? 8 : 1;
	if (Rotation & 4)
	{
		Rotation = 0;
		dq = 1;
	}
	if (nFit < 2)
		nFit = 1;
	else if (nFit <= 3)
		nFit = 0;
	if (scale.Init(w, h, dq, ww, hh, nFit))
	{
		DPF("scale failure");
		return 1;
	}
	memset(hpGray, 255, gpitch * gh);
	int q = scale.Custom(hpGray, gpitch, hpBuf, ipitch, pals, dd, Rotation);
	DPF("after custom:%d", q);
	//DPF("offx:%d,offy:%d",scale.m_offx,scale.m_offy);
	int z = scale.Copy();
	DPF("after scale:%d", z);
	result = 0;
	return result;
}

UINT CScene::zCreateCell(UINT Frame, UINT Level,
	LPBITMAPINFOHEADER  lpbi, UINT Rotation,
	bool bMakeMono, UINT nHold)
{
	//	bScaleColor = TRUE;
	//	nHold is 2 for no apsect hold, 1 for scale to fit
	//	> 2 is crop flag plus offset
	//
	UINT alpha_key = Rotation >> 16;
	UINT layer = Rotation / 256;	// low byte for roation
	bool bCvt24 = Rotation & 128 ? 1 : 0;	// flag to create alpha
	Rotation &= 7;
	UINT result;
	if (Frame >= m_frames)
		InsertCache(m_frames, Frame + 1);
	//	DeleteCell(Frame, Level,1);
	BlowCell(Frame, Level);

	bMakeMono = FALSE;
	UINT w = lpbi->biWidth;
	UINT h = lpbi->biHeight;
	UINT d = lpbi->biBitCount;
	if (Level && (d == 24) && bCvt24)
		d = 32;
	else if ((d == 32) && !bCvt24)
		d = 8;
	else
		bCvt24 = 0;
	if (!Level || (d == 32))
	{
		//	DeleteCell(Frame, Level,1);
		DeleteCell(Frame, Level);
		if (nHold & 5)
		{
			w = m_width;
			h = m_height;
		}
		else if (Rotation & 1)
		{
			h = lpbi->biWidth;
			w = lpbi->biHeight;
		}
		UINT c, od;
		if (w > h)
			c = w;
		else
			c = h;
		if (Level)
			od = 32;
		else
			od = 24;
		//		UINT size = h * 4 * ((d * w + 31) / 32);
		UINT size = c * 4 * ((od * c + 31) / 32);
		BYTE* temp = new BYTE[size];
		memset(temp, 0, size);
		ConvertColor(temp, lpbi, od,
			(alpha_key << 16) + (Rotation | (bCvt24 ? 128 : 0)), nHold);
		WriteOverlay(temp, Frame, Level, w, h);
		delete[] temp;
		//		UpdateCache(Frame, Level);
		return 0;
	}
	//	nHold &= 3; // remove aspect hold flag
	DWORD gw = m_width;// / m_scale;
	DWORD gh = m_height;// / m_scale;

	BYTE* hpDst = 0;
	DWORD bw = lpbi->biWidth;
	DWORD bh = lpbi->biHeight;
	//
	//	the following is disabled temporarily, until
	//	 we decide about the postponed scaling
	//
	UINT p;
	if (hpDst && bMakeMono && (bw > gw) && (bh > gh))
	{
		p = 4 * ((m_width + 3) / 4);
		h = 4 * ((m_height + 3) / 4);	// for rotation
		hpDst = new BYTE[p * h];
		if (!(result = ConvertGray(hpDst, lpbi, Rotation, nHold)))
		{
			PutMono(hpDst, Frame, Level);
			MakeGray(hpDst);
			//			PutGray(hpDst,Frame,Level);
		}
	}
	else
	{
		if (bMakeMono)
		{
			p = 4 * ((m_width + 3) / 4);
			h = 4 * ((m_height + 3) / 4);	// for rotation
		}
		else
		{
			p = 4 * ((gw + 3) / 4);
			h = 4 * ((gh + 3) / 4);	// for rotation
		}
		hpDst = new BYTE[2 * p * h];
		//		if (layer)
		//			memset(hpDst,MAGIC_COLOR,2* p * h);
		if (!(result = ConvertGray(hpDst, lpbi, Rotation, nHold)))
		{
			UINT i;
			if (!layer)
			{
				for (i = 0; i < p * gh; i++)
				{
					hpDst[i] ^= 255;
					hpDst[i + p * gh] = 0;
				}
				PutLayer(hpDst, Frame, Level, CCell::LAYER_INK);
				memset(hpDst, 0, 2 * p * h);
				PutLayer(hpDst, Frame, Level, CCell::LAYER_PAINT);
			}
			else
			{
				for (i = 0; i < p * gh; i++)
				{
					if (hpDst[i] != 255)
						hpDst[i] = 255;
					else
						hpDst[i] = 0;
					hpDst[i + p * gh] = 0;//MAGIC_COLOR;
				}
				PutLayer(hpDst, Frame, Level, layer - 1 + CCell::LAYER_MATTE0);
			}
			//			UpdateCache(Frame, Level);
			//			PutGray(hpDst,Frame,Level);
			if (bMakeMono)
			{
				MakeMono(hpDst);
				PutMono(hpDst, Frame, Level);
			}
		}
	}
	if (hpDst)
		delete[] hpDst;
	return result;
}

void CScene::MakeMono(HPBYTE hpDst)
{
	DWORD gw = m_width;// / m_scale;
//	DWORD gh = m_height / m_scale;
	DWORD gpitch = 4 * ((gw + 3) / 4);
	DWORD mpitch = 4 * ((m_width + 3) / 4);
	UINT x, y, z, gx, gy, f;
	UINT m_scale = 5;
	f = m_scale * m_scale - 1;

	DPF("making mono");
	for (y = m_height; y-- > 0;)
	{
		gy = y / m_scale;
		UINT gp = gpitch * gy;
		UINT mp = mpitch * y;
		z = m_scale * (y % m_scale);
		for (x = m_width; x-- > 0;)
		{
			gx = x / m_scale;
			UINT v = hpDst[gx + gp];
			if (v != 255)
			{
				v *= f;
				UINT d = (x % m_scale) + z;
				if (v >= (250 * d))
					v = 255;
				else
					v = 0;
			}
			hpDst[mp + x] = v;
		}
	}
	DPF("mono done");
}

void CScene::MakeGray(HPBYTE hpDst)
{
	DWORD gw = m_width;// / m_scale;
	DWORD gh = m_height;// / m_scale;
	DWORD gpitch = 4 * ((gw + 3) / 4);
	DWORD mpitch = 4 * ((m_width + 3) / 4);
	BYTE* src = hpDst;
	BYTE* dst = hpDst;
	UINT x, y, f, i, j, sx;
	UINT m_scale = 5;
	f = m_scale * m_scale;
	DPF("making gray");
	UINT* temp = new UINT[gw];
	if (!temp) return;
	for (y = 0; y < gh; y++)
	{
		for (x = 0; x < gw; temp[x++] = 0);
		for (i = 0; i < m_scale; i++)
		{
			for (x = 0, sx = 0; x < gw; x++)
				for (j = 0; j < m_scale; j++)
					temp[x] += src[sx++];
			src += mpitch;
		}
		for (x = 0; x < gw; x++)
			dst[x] = temp[x] / f;
		dst += gpitch;
	}
	delete[] temp;
	DPF("gray done");
}

int CScene::CloseIt()
{
	// any stuff to do at close
	ClipEmpty(1);
	return 0;
}

// 
// rotation & 128 for cvt 24 to 32
//
UINT CScene::ConvertColor(HPBYTE hpColor, LPBITMAPINFOHEADER  lpbi, UINT od,
	UINT Rotation, UINT nFit)
{
	UINT result;
	UINT nCenter = nFit & 8;
	nFit &= 7;
	DPF("cvting color,key:%d,rot:%d", (Rotation >> 16), Rotation & 0xffff);
	DWORD iw, ih, idd;
	iw = lpbi->biWidth;
	ih = lpbi->biHeight;
	idd = lpbi->biBitCount;
	DPF("w:%d,h:%d,d:%d", iw, ih, idd);
	if ((idd != 8) && (idd != 16) && (idd != 24) && (idd != 1) && (idd != 32))
		return 1;
	DWORD ow, oh;
	if (nFit & 5)
	{
		ow = m_width;
		oh = m_height;
		if (Rotation & 1)
		{
			UINT t = ow;
			ow = oh;
			oh = t;
		}
	}
	else
	{
		ow = iw;
		oh = ih;
	}
	DWORD opitch, dwSize;
	if (Rotation & 1)
	{
		opitch = 4 * ((od * oh + 31) / 32);
		dwSize = opitch * ow;
	}
	else
	{
		opitch = 4 * ((od * ow + 31) / 32);
		dwSize = opitch * oh;
	}
	DWORD ipitch = 4 * ((idd * iw + 31) / 32);
	HPBYTE hpBuf = (HPBYTE)lpbi;
	hpBuf += 40;	// for header
	BYTE* pals = hpBuf;
	if (idd < 9)
	{
		hpBuf += 4 << idd;	// for palette
	}
	CCScaler scale;
	scale.m_aw = iw;
	scale.m_ah = ih;
	if (nFit < 2)
		nFit = 1;
	else if (nFit <= 3)
		nFit = 0;
	//DPF("w:%d,h:%d,d:%d,nfit:%d",iw,ih,idd,nFit);
	if (scale.Init(iw, ih, od, ow, oh, nFit + nCenter))
	{
		DPF("scale failure");
		return 1;
	}
	//DPF("ip:%d,op:%d",ipitch,opitch);
	if (Rotation & 128)
	{
		DWORD* pe = (DWORD*)(hpColor + dwSize);
		DWORD* pd = (DWORD*)hpColor;
		for (; pd < pe; *pd++ = 0x00ffffff);
	}
	else
		memset(hpColor, 255, dwSize);
	int q = scale.Custom(hpColor, opitch, hpBuf, ipitch, pals, idd, Rotation);
	DPF("after custom:%d", q);
	//DPF("offx:%d,offy:%d",scale.m_offx,scale.m_offy);
	int z = scale.Copy();
	DPF("after scale:%d", z);
	if (Rotation & 128)
	{
#ifndef XYZ
		BYTE* tt = new BYTE[oh * ow];
		BlurOver(tt, hpColor, ow, oh, opitch);
#else
		BYTE* tt = new BYTE[oh * opitch];
		BlurXX(tt + 3, hpColor + 3, ow, oh, 2, 5, 4, opitch);

		UINT x, y, v;
		for (y = 0; y < oh; y++)
			for (x = 0; x < ow; x++)
				if (hpColor[opitch * y + 4 * x + 3])
				{
					v = tt[opitch * y + 4 * x + 3];
					if (v < 128)
						v = 0;
					else
						v = 1 + 2 * (v - 128);
					hpColor[opitch * y + 4 * x + 3] = v;
				}
#endif
		delete[] tt;
	}
	result = 0;
	return result;
}
