#include "CNewPals.h"
#include "math.h"
//#include "fbqt.h"
#include "MyObj.h"
#include "MyIO.h"

#include "ImageUtils.h"
#include "utils.h"

enum palkinds {
		PKIND_SIMPLE,
		PKIND_GRAD,
		PKIND_TEXTURE};


typedef struct {
	DWORD   dwKey;
	DWORD   dwKind;
	DWORD	dwNameSize;
	DWORD	flags;
	char	pal_name[30];
	char 	file_name[300];
} PALETTEHDR;


//#define PAL_FLAG_PROTECTED 1
//#define PAL_FLAG_LINKED  2  // jan 13, 09, changed to use file name[0]
//#define PAL_FLAG_IF_SHARED  4
//#define PAL_FLAG_DIRTY  8



#ifndef FLIPBOOK_MAC
#define RGBA(r,g,b,a) ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))| \
					(((DWORD)(BYTE)(b))<<16)) | (((DWORD)(BYTE)(a))<<24))
#endif

CNewPals::CNewPals()
{
	memset(this,0,sizeof(*this));
}

CNewPals::~CNewPals()
{
	CleanUp();
}

void CNewPals::Assign(UINT index, BYTE r, BYTE g, BYTE b, BYTE o, bool bKeepKind)
{
	if (!bKeepKind)
		{
		CleanUpIndex(index);
		pals[index].kind = PKIND_SIMPLE;
		}
	pals[index].rgbo[0] = r;
	pals[index].rgbo[1] = g;
	pals[index].rgbo[2] = b;
	pals[index].rgbo[3] = o;
}

void CNewPals::Assign(UINT index, COLORREF crColor)
{
	CleanUpIndex(index);
	pals[index].kind = PKIND_SIMPLE;
	pals[index].rgbo[0] = GetRValue(crColor);
	pals[index].rgbo[1] = GetGValue(crColor);
	pals[index].rgbo[2] = GetBValue(crColor);
	pals[index].rgbo[3] = ((UINT)crColor >> 24);
}

/*
	inital entry, set rgb and default grad stuff
*/
void CNewPals::Initial(UINT index, BYTE r, BYTE g, BYTE b, BYTE o)
{
	pals[index].kind = 0;
	pals[index].rgbo[0] = r;
	pals[index].rgbo[1] = g;
	pals[index].rgbo[2] = b;
	pals[index].rgbo[3] = o;
	pals[index].grad.kind = 1;
	pals[index].grad.p1.x = 160;
	pals[index].grad.p1.y = 359;
	pals[index].grad.p2.x = 479;
	pals[index].grad.p2.y = 120;
	pals[index].grad.c1 = pals[index].rgb;
	pals[index].grad.c2 = pals[index].rgb;
//	pals[index].grad.rgb2[0] ^= 255;
//	pals[index].grad.rgb2[1] ^= 255;
//	pals[index].grad.rgb2[2] ^= 255;
	pals[index].cname = 0;
	pals[index].fname = 0;
	pals[index].w = 1;
	pals[index].h = 1;
	pals[index].flags = 0;
	pals[index].size = 0;
	pals[index].pTexture = 0;
}


void CNewPals::Legacy(BYTE * pOldPals, LPCSTR Name)
{
	UINT i;
	for (i = 0; i < 256; i++)
		{
		Initial(i, pOldPals[4*i+0],
				pOldPals[4*i+1], pOldPals[4*i+2], pOldPals[4*i+3]);
		}
	if (Name)
		strcpy(m_file_name,Name);
}

/*
void CNewPals::Init(BYTE * pOldPals, UINT kind, BYTE * pName)
{
	UINT i;
	for (i = 0; i < 256; i++)
		Initial(i, pOldPals[4*i+0],
				pOldPals[4*i+1], pOldPals[4*i+2], pOldPals[4*i+3]);
	if (kind < 3)
		{
//		m_external_name[0] = kind;
//		m_external_name[1] = 0;
		}
	else
		{
		for (i = 0; i < 299; i++)
			if (!(m_pal_name[i] = pName[i]))
				break;
		m_pal_name[i] = 0;
		}		
		
}

void CNewPals::Init(BYTE * pOldPals, LPCSTR Name)
{
	UINT i;
	for (i = 0; i < 256; i++)
		Initial(i, pOldPals[4*i+0],
				pOldPals[4*i+1], pOldPals[4*i+2], pOldPals[4*i+3]);
	if (Name[0] < 3)
		{
	//	m_external_name[0] = Name[0];
	//	m_external_name[1] = 0;
		}
	else
		{
		for (i = 0; i < 299; i++)
			if (!(m_pal_name[i] = Name[i]))
				break;
		m_file_name[i] = 0;
		}		
}
*/
/*
UINT CNewPals::PalKind()
{
//	if (m_external_name[0] < 3)
//		return m_external_name[0];
//	else
		return 3;
}
*/
bool CNewPals::Simple(bool bForce)
{
	UINT i;
	if (bForce)
		{
		bool bBad = 0;
		for (i = 0; i < 256; i++)
			if (pals[i].kind)
				{
				CleanUpIndex(i);
				pals[i].kind = 0;
				bBad = 1;
				}
		if (bBad)
			return false;
		}
	else
		{
		for (i = 0; i < 256; i++)
			if (pals[i].kind)
				return false;
		}
	return TRUE;
}

/*
bool CNewPals::Dirty(int v)
{
	if (v != -1)
		{
		m_flags |= PAL_FLAG_DIRTY;
		if (!v)
			m_flags ^= PAL_FLAG_DIRTY;
		}
	return (m_flags & PAL_FLAG_DIRTY) ? 1 : 0; 
}
*/
void CNewPals::CleanUpIndex(UINT index)
{
	if (pals[index].kind == PKIND_TEXTURE)
		delete [] pals[index].pTexture;
	pals[index].pTexture = 0;
}

void CNewPals::CleanUp()
{
	for (int i = 0; i < 256; i++)
		CleanUpIndex(i);
	delete m_pNames;
	m_pNames = 0;
}
/*
void CNewPals::SetPalInfo(UINT kind, LPSTR name)
{
}
*/
COLORREF CNewPals::Color(UINT index, UINT x, UINT y)
{
	BYTE col[4];
	Color(col,index,x,y);
	return RGBA(col[0],col[1],col[2],col[3]);
}
void CNewPals::Color(BYTE * Pals, UINT index, UINT x, UINT y)
{
	UINT kind = pals[index].kind;
	if (kind == ColorKind::SIMPLE) {
		*((DWORD*)Pals) = (DWORD)pals[index].rgb;
	}
	else if (kind == ColorKind::GRADIENT) {
		DoGrad(Pals,index, x,y);
	}
	else if(kind == ColorKind::TEXTURE) {
		if (!pals[index].pTexture) {
			*((DWORD*)Pals) = (DWORD)pals[index].rgb;
			return;
		}
		UINT sx,sy;
		int zq = 0;
		UINT w = pals[index].w;
		UINT h = pals[index].h;
		UINT f = pals[index].flags;
		UINT d = f & NPAL_FLAG_HAS_ALPHA ? 4 : 3;
		if (f & NPAL_FLAG_HSYM) // h reflect
			{
			sx = x % (w + w - 1);
			if (sx >= w)
				sx = w + w - 2 - sx;
			}
		else
			sx = x % w;
		if (f & NPAL_FLAG_VSYM) // v reflect
			{
			sy = y % (h + h - 1);
			if (sy >= h)
				sy = h + h - 2 - sy;
			}
		else
			sy = y % h;
		UINT p = 4 * ((d * w + 3) / 4);
		BYTE * pColor = pals[index].pTexture+sy*p;
		Pals[0] = pColor[d*sx+2];
		Pals[1] = pColor[d*sx+1];
		Pals[2] = pColor[d*sx+0];
		if (f & NPAL_FLAG_NORGB)
			{
			if (f & NPAL_FLAG_HAS_ALPHA)
				Pals[3] = pColor[4*sx+3];
			else
				Pals[3] = (30 * (UINT)Pals[0] + 59 * (UINT)Pals[1] + 11 *
						(UINT)Pals[2]) / 100;
			Pals[0] = pals[index].rgbo[0];
			Pals[1] = pals[index].rgbo[1];
			Pals[2] = pals[index].rgbo[2];
			}
		else
			Pals[3] = f & NPAL_FLAG_HAS_ALPHA ? pColor[4*sx+3] : 255;
		Pals[3] = (Pals[3] * pals[index].rgbo[3] + 127) / 255;
	}
}

int CNewPals::Write(LPCSTR name)
{
	CFile file;
	DWORD mode = CFile::modeCreate | CFile::modeWrite;
	if (!file.Open(name, mode))
		return 1;
	char buf[80];
	sprintf(buf,"DGC-PAL\r\n");
	file.Write(buf,strlen(buf));
	int i;
	for (i = 0; i < 256; i++)
		{
		sprintf(buf,"%3d,%3d,%3d,%3d,%3d\r\n", i,
			pals[i].rgbo[0], pals[i].rgbo[1], pals[i].rgbo[2],pals[i].rgbo[3]);
		file.Write(buf,strlen(buf));
		}
	file.Close();
	return 0;
}
int CNewPals::Read(LPCSTR name)
{
	CFile file;
	DWORD mode = CFile::modeRead;
	if (!file.Open(name, mode))
		return 1;
	CArchive arPal(&file, CArchive::load);
	char buf[80];
	int i,kind;
	for(i = -1; i < 256; i++)
		{
		arPal.ReadString(buf, 80);
		DPF("i:%d,%s",i,buf);
		if (i == -1)
			{
			if (buf[0] == 'D' && buf[1] == 'G' || buf[2] == 'C')
				kind = 0;
			else if (buf[0] == 'D' && buf[1] == 'G' || buf[2] == 'K')
				kind = 1;
			else
				break;
			}
		else if (kind == 0)
			{
			int j,r,g,b,o;
			if (sscanf(buf,"%d,%d,%d,%d,%d\n",&j,&r,&g,&b,&o) != 5)
				break;
			if (j != i)
				break;
			Assign(i,r,g,b,o);		
			}
		}
	arPal.Close();
	file.Close();
	DPF("i:%d",i);
	return i < 256 ? 2 : 0;
}

bool CNewPals::GetSetGrad(UINT Index, GRADENTRY & grad, bool bSet)
{
//	PALENTRY * pPal = (PALENTRY *)m_pData;
//	pPal += Index;
	if (!bSet)
		{
		if (pals[Index].kind != 1)
			return 1;
		grad = pals[Index].grad;
		}
	else
		{
		pals[Index].kind = 1;
		pals[Index].grad = grad;
		}
	return 0;
}

void CNewPals::SaveEntry(UINT index)
{
	ASSERT(m_save.pTexture == 0);
	m_save_index = index;
	m_save = pals[index];
	if (m_save.pTexture)
		{
		m_save.pTexture = new BYTE[m_save.size];
		memmove(m_save.pTexture,pals[index].pTexture, m_save.size);
		}
}
//
// 0 is just empty, 1 is rest, 2 is both
//
void CNewPals::RestEntry(int code)
{
	if (code)
		{
		UINT index = m_save_index;
		delete [] pals[index].pTexture;
		pals[index] = m_save;
		if (m_save. pTexture)
			{
			pals[index].pTexture = new BYTE[m_save.size];
			memmove(pals[index].pTexture, m_save.pTexture,m_save.size);
			}
		}
	if (code != 1)
		{
		delete [] m_save.pTexture;
		m_save.pTexture = 0;
		}
}

bool CNewPals::CompareOne(NPALENTRY & pal,UINT Index)
{
	NPALENTRY * pPal = &pals[Index];
	if (pPal->kind != pal.kind)
		return TRUE;
	if (pPal->kind == 2)
		{
		return TRUE;
		}
	else if (pPal->kind == 1)
		{
		if (pPal->grad.kind != pal.grad.kind)
			return TRUE;
		if (pPal->grad.c1 != pal.grad.c1)
			return TRUE;
		if (pPal->grad.c2 != pal.grad.c2)
			return TRUE;
		if (pPal->grad.p1.x != pal.grad.p1.x)
			return TRUE;
		if (pPal->grad.p1.y != pal.grad.p1.y)
			return TRUE;
		if (pPal->grad.p2.x != pal.grad.p2.x)
			return TRUE;
		if (pPal->grad.p2.y != pal.grad.p2.y)
			return TRUE;
		}
	else
		{
		if (pPal->rgb != pal.rgb)
			return TRUE;
		}
	return FALSE;
}

bool CNewPals::Compare(CNewPals * pPals)
{
	int i;
	bool bResult = 0;
	for (i = 0; !bResult && (i < 256); i++)
		{
		bResult = 1;
		if (pals[i].kind != pPals->pals[i].kind)
			break;
		switch (pals[i].kind) {
		case 0:
			if (pals[i].rgb == pPals->pals[i].rgb)
				bResult = 0;
			break;
		default:
			break;
		}
		}
	return bResult;
}
 
CNewPals * CNewPals::Clone(LPCSTR Name)
{
	CNewPals * pPals = new CNewPals;
	memmove(pPals, this, sizeof(*this));
	if (Name)
		strcpy(pPals->m_pal_name,Name);
	pPals->m_pNames = new BYTE[m_nNameSize];
	pPals->m_nNameSize = m_nNameSize;
	int i;
	for (i = 0; i < 256; i++)
		{
		if ((pals[i].kind == PKIND_TEXTURE) &&
						(pals[i].size))
			pPals->pals[i].pTexture = new BYTE[pals[i].size];
		}
	return pPals;
}

UINT CNewPals::Kind(UINT index, UINT kind)
{
	if (kind != pals[index].kind)
		{
		CleanUpIndex(index);
		}
	return pals[index].kind = kind;
}

void CNewPals::DoGrad(BYTE * grads, UINT Index, int xx, int yy)
{
	GRADENTRY grad = pals[Index].grad;
	int x3 = xx;
	int y3 = yy;
	int dx = grad.p2.x - grad.p1.x;
	int dy = grad.p2.y - grad.p1.y;
//	int d = (int)sqrt((double)(dx * dx) + (double)(dy * dy));
	double dd = sqrt((double)(dx * dx) + (double)(dy * dy));
	UINT f;

	if (!dx && !dy)
		return;

	if (grad.kind == GradientKind::LINEAR) {
		double cc = dx * dx + dy * dy;
		double tx = (xx - grad.p1.x);
		double ty = (yy - grad.p1.y);
		double bb = tx * tx + ty * ty;
		tx = (xx - grad.p2.x);
		ty = (yy - grad.p2.y);
		double aa = tx * tx + ty * ty;
//		double a = sqrt(aa);
//		double c = sqrt(cc);
//		double b = sqrt(bb);
		if ((cc + bb - aa) < 0) // angle a > 90
			f = 0;
		else if ((cc + aa - bb) < 0) // angle b > 90
			f = 255;
		else
			f = (UINT)(255 * (cc + bb - aa) / (2 * cc));
	}
	else if(grad.kind == GradientKind::RADIAL)
		{
		int ddx = x3 - grad.p1.x;
		int ddy = y3 - grad.p1.y;
		int d1 = (int)sqrt((double)(ddx * ddx) + (double)(ddy * ddy));
		int d = (int)dd;
		if (d1 > d)
			f = 255;
		else
			f = 255 * d1 / d;
		}
	UINT f2 = 255 - f;
	grads[0] = (f2 * GetRValue(grad.c1) + f * GetRValue(grad.c2)) / 255;
	grads[1] = (f2 * GetGValue(grad.c1) + f * GetGValue(grad.c2)) / 255;
	grads[2] = (f2 * GetBValue(grad.c1) + f * GetBValue(grad.c2)) / 255;
	grads[3] = (BYTE)((f2 * (grad.c1 >> 24) + f * (grad.c2 >> 24)) / 255);
}

int CNewPals::ProcessTextureData(void * pData, UINT ssize)
{
	LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)pData;
	if (!lpBI) return 97;
	UINT w = lpBI->biWidth;
	UINT h = lpBI->biHeight;
	UINT bits = lpBI->biBitCount;
	if ((bits != 24) && (bits != 32))
		return 94;
	UINT pitch = 4 * (( bits * w + 31) / 32);
	UINT size  = h * pitch;
	pals[m_ctindex].size = size;
	BYTE * pTemp = new BYTE[size];
	if (!pTemp)
		return 98;
	delete [] pals[m_ctindex].pTexture;
	pals[m_ctindex].kind = 2;
	pals[m_ctindex].w = w;
	pals[m_ctindex].h = h;
//	pals[m_ctindex].flags = 0;
//	pals[m_ctindex].flags |= 128;
	pals[m_ctindex].flags |= 64;
	if (bits == 24)
		pals[m_ctindex].flags ^= 64;
	pals[m_ctindex].pTexture = pTemp;
	memcpy(pTemp, (BYTE *)pData+40, size);
	return 0;
}



UINT TextureLoadCB(FBDataKind Code, void * pData, int size, void * pClass)
{
	if (Code != kBMPDataType)
		return 95;
	return ((CNewPals*)pClass)->ProcessTextureData(pData, size);
}

bool CNewPals::LoadTexture(UINT index,LPCSTR name)
{
	m_ctindex = index;
	int res = FBImportStill (name, &TextureLoadCB, (void*)this);
	if (!res)
		{
//		pals[index].flags |= NPAL_FLAG_INTERNAL;
		pals[index].fname = AddName(name);
		}
	return res;
}

UINT CNewPals::AddName(LPCSTR name)
{
	int l = strlen(name) + 1;				// name size
	BYTE * p = new BYTE[m_nNameSize + l];	// bigger name area
	memmove(p, m_pNames, m_nNameSize);		// copy old names
	memmove(p+m_nNameSize, name, l);		// append new name
	delete [] m_pNames;						// blow old data
	m_pNames = p;							// now have new
	UINT z = m_nNameSize;					// index to addition
	m_nNameSize += l;						// new name size
	return z+1;
}

UINT CNewPals::GetTextureSize()
{
	int i;
	UINT size = 0;
	for (i = 0; i < 256; i++)
		{
		if ((pals[i].kind == PKIND_TEXTURE) &&
						(pals[i].flags & NPAL_FLAG_INTERNAL))
			{
			size += pals[i].size;
			}
		}
	return size;
}



void CNewPals::SwapEntries(NPALENTRY * pp)
{
#ifdef FBMAC
	
	int i;
	for (i = 0; i < 256; i++, pp++)
		{
/*
		UINT	kind;			// color kind, 0 is simple, 1 is grad
		union {
			BYTE rgbo[4];
			COLORREF rgb;
		};
		GRADENTRY grad;
		UINT	cname;			// index to color name
		UINT	fname;			// index to file name
		UINT 	w,h,flags;			// width and height of pattern or texture
		UINT	size;
		BYTE *  pTexture;		// data assigned to texture or pattern
*/
		}
#endif
}
int CNewPals::ReadWrite(CMyIO * pIO, DWORD key, bool bPut)
{
	PALETTEHDR hdr;
    size_t xxx = sizeof(PALETTEHDR);
    size_t yyy = sizeof(NPALENTRY);
    size_t zzz = sizeof(GRADENTRY);
	int result = 0;
    const int oldPalEntrySize = sizeof(NPALENTRY) - (sizeof(void*) - 4); // 4 (bytes) is size of 32 bit pointer
    
	if (!bPut)
		{
		DWORD rsize;
		result = pIO->GetSize(rsize,key);
		if (result)
			return result;
		BYTE * tp = new BYTE[rsize];
		if (!tp)
			return 1;
		result = pIO->GetRecord(tp, rsize, key);
		DPF("read:%d", result);
		if (result)
			return result;

		PALETTEHDR * ph = (PALETTEHDR *)tp;
		ASSERT(ph->dwKey == key);
		ASSERT(ph->dwKind == KIND_PALETTE);
		m_flags = ph->flags;
		strcpy(m_pal_name,ph->pal_name);
		strcpy(m_file_name,ph->file_name);
		m_nNameSize = ph->dwNameSize;
		BYTE * p = tp;
		p += sizeof(hdr);
            
        if(sizeof(void*) > 4) {
            // Manually copy all pals to 64 bit versions of NPALENTRY
            for(int i = 0; i < 256; ++i) {
                memcpy(&pals[i], p + i*oldPalEntrySize, oldPalEntrySize);
            }
        } else {
            memmove(&pals, p, 256*sizeof(NPALENTRY));
        }

		p += 256 * oldPalEntrySize;
		if (m_nNameSize)
			{
			m_pNames = new BYTE[m_nNameSize];
			memmove(m_pNames, p, m_nNameSize);
			}
		p += m_nNameSize;
		int i;
		for (i = 0; i < 256; i++)
			{
			pals[i].pTexture = 0;
			if (pals[i].kind == PKIND_TEXTURE)
				{
				if (pals[i].flags & NPAL_FLAG_INTERNAL)
					{
ASSERT(0);
					pals[i].pTexture = new BYTE[pals[i].size];
					memmove(pals[i].pTexture, p, pals[i].size);
					p += pals[i].size;
					}
				else 
					{
		//			ASSERT(pals[i].fname);
					if (!pals[i].fname)
						{
						pals[i].kind = PKIND_SIMPLE;
						pals[i].rgbo[0] = 128;
						pals[i].rgbo[1] = 128;
						pals[i].rgbo[2] = 128;
						pals[i].rgbo[3] = 128;
						}
					else
						{
						m_ctindex = i;
		//			pals[i].pTexture = 0;
						char * pName = (char *)m_pNames;
						int res = FBImportStill (pName+pals[i].fname-1,
										&TextureLoadCB, (void*)this);
						}
					}
				}
			}
		delete [] tp;
		}
	else
		{
		DPF("palette write");
		memset(&hdr,0,sizeof(hdr));
		hdr.dwKey = key;
		hdr.dwKind = KIND_PALETTE;
		hdr.dwNameSize = m_nNameSize;
		hdr.flags = m_flags;
		strcpy(hdr.pal_name,m_pal_name);
		strcpy(hdr.file_name,m_file_name);
		UINT rsize = sizeof(hdr);
		rsize += 256 * oldPalEntrySize;
		rsize += m_nNameSize;
		rsize += GetTextureSize();
		BYTE * tp = new BYTE[rsize];
		if (!tp)
			return 1;
		BYTE * p = tp;
		UINT tsize = sizeof(hdr);
		memmove(p, &hdr, tsize);

		p += tsize;
            
            if(sizeof(void*) > 4) {
                for(int i = 0; i < 256; ++i) {
                    memcpy(p + i*oldPalEntrySize, &pals[i], oldPalEntrySize);
                }
            } else {
                memmove(p, &pals, 256*sizeof(NPALENTRY));
            }

		p += 256 * oldPalEntrySize;
		if (m_nNameSize)
			memmove(p, m_pNames, m_nNameSize);
		p += m_nNameSize;
		int i;
		for (i = 0; i < 256; i++)
			if ((pals[i].kind == PKIND_TEXTURE) &&
						(pals[i].flags & NPAL_FLAG_INTERNAL))
				{
ASSERT(0);
				memmove(p, pals[i].pTexture, pals[i].size);
				p += pals[i].size;
				}
		int result = pIO->PutRecord(tp, rsize, key);
DPF("put pal rec:%d",result);
		delete [] tp;
		}
	return result;
}

void CNewPals::SetPalName(LPCSTR Name)
{
	strcpy(m_pal_name, Name);
}

void CNewPals::SetFileName(LPCSTR Name)
{
	strcpy(m_file_name, Name);
}

LPCSTR CNewPals::FileName(UINT index, int code)
{
	UINT z = pals[index].fname;
	if (!z)
		return 0;
	LPCSTR name = (LPCSTR)m_pNames+z-1;
	if (code)
		{
		CFile file;
		DWORD mode = CFile::modeRead;
		if (!file.Open(name, mode))
			return (LPCSTR)1;
		DWORD zz[4];
		UINT size = file.Read(&zz,12);
		file.Close();
		if (size != 12)
			return (LPCSTR)2;
		return 0;
		}
	else
		return name;
}


int CNewPals::Linked(int v)  // if 1 then ret 1 for no open, 2 for mismatch
{
	if (v && m_file_name[0])
		return CompareFile();
	return m_file_name[0] ? 1 : 0;
}
#ifdef NONO
bool CNewPals::Protected(int v /* = -1 */)
{
	if (v != -1)
		{
		m_flags |= PAL_FLAG_PROTECTED;
		m_flags ^= (v ? 0 : PAL_FLAG_PROTECTED);
		}
	return m_flags & PAL_FLAG_PROTECTED ? 1 : 0;
}


bool CNewPals::IfShared(int v /* = -1 */)
{
	if (v != -1)
		{
		m_flags |= PAL_FLAG_IF_SHARED;
		m_flags ^= (v ? 0 : PAL_FLAG_IF_SHARED);
		}
	return m_flags & PAL_FLAG_IF_SHARED ? 1 : 0;
}
#endif
UINT CNewPals::PaletteIO(LPCSTR name, bool bWrite)
{
	if (!bWrite)
		return GetPaletteFile(name);
	else
		return PutPaletteFile(name);
}

UINT CNewPals::PutPaletteFile(LPCSTR name)
	{
	CFile file;
	DWORD mode = CFile::modeCreate | CFile::modeWrite;
	if (!file.Open(name, mode))
		return 1;
	int j;
	for (j = 0; j < 256; j++)
		if (pals[j].kind)
			break;
	char buf[300];
	if (j >= 256)		// no new stuff , use old format
		{
		sprintf(buf,"DGC-PAL\r\n");
		file.Write(buf,strlen(buf));
		int i;
		for (i = 0; i < 256; i++)
			{
			sprintf(buf,"%3d,%3d,%3d,%3d,%3d\r\n", i,
					pals[i].rgbo[0],pals[i].rgbo[1],
					pals[i].rgbo[2],pals[i].rgbo[3]);
			file.Write(buf,strlen(buf));
			}
//
//	put new stuff at end so that version 5 flipbook can read this
//	it will ignore stuff after pals
//
		sprintf(buf,"Flags:%d\r\n",m_flags);
		file.Write(buf,strlen(buf));
		sprintf(buf,"Name:%s\r\n",m_pal_name);
		file.Write(buf,strlen(buf));
		file.Close();
		return 0;
		}
//
//	new format
//
	sprintf(buf,"DGK-PAL\r\n");
	file.Write(buf,strlen(buf));
	sprintf(buf,"Flags:%d\r\n",m_flags);
	file.Write(buf,strlen(buf));
	sprintf(buf,"Name:%s\r\n",m_pal_name);
	file.Write(buf,strlen(buf));
	int i;
	char * pName = (char *)m_pNames;
	for (i = 0; i < 256; i++)
		{
		sprintf(buf,"%3d,%3d,%3d,%3d,%3d,%3d\r\n", i,pals[i].kind,
						pals[i].rgbo[0],pals[i].rgbo[1],
						pals[i].rgbo[2],pals[i].rgbo[3]);
		file.Write(buf,strlen(buf));
		sprintf(buf,"    %3d,%3d,%3d,%3d,%3d\r\n", pals[i].grad.kind,
						pals[i].grad.rgb1[0],pals[i].grad.rgb1[1],
						pals[i].grad.rgb1[2],pals[i].grad.rgb1[3]);
		file.Write(buf,strlen(buf));
		sprintf(buf,"        %3d,%3d,%3d,%3d\r\n",
						pals[i].grad.rgb2[0],pals[i].grad.rgb2[1],
						pals[i].grad.rgb2[2],pals[i].grad.rgb2[3]);
		file.Write(buf,strlen(buf));
		sprintf(buf,"        %3d,%3d,%3d,%3d\r\n",
						pals[i].grad.p1.x,pals[i].grad.p1.y,
						pals[i].grad.p2.x,pals[i].grad.p2.y);
		file.Write(buf,strlen(buf));
		if (pals[i].cname)
			sprintf(buf,"        color:%s\r\n",pName+pals[i].cname-1);
		else
			sprintf(buf,"        color:\r\n");
		file.Write(buf,strlen(buf));
		if (pals[i].fname)
			sprintf(buf,"        file :%s\r\n",pName+pals[i].fname-1);
		else
			sprintf(buf,"        file :\r\n");
		file.Write(buf,strlen(buf));
		}
	file.Close();
	return 0;
}

UINT CNewPals::GetPaletteFile(LPCSTR name)
{
	CFile file;
	DWORD mode = CFile::modeRead;
	if (!file.Open(name, mode))
		return 1;
	CArchive arPal(&file, CArchive::load);
	char buf[300];
	char tmp[300];
	int i,kind;
	kind = 0;
	int result = 0;
	for(i = -1; i < 256; i++)
		{
		arPal.ReadString(buf, 80);
		DPF("i:%d,%s",i,buf);
		if (i < 0)
			{
			if (buf[0] != 'D' || buf[1] != 'G')
				break;
			if (buf[2] == 'C')
				kind = 0;
			else if (buf[2] == 'K')
				{
				kind = 1;
				UINT f;
				arPal.ReadString(buf, 299);
				if (sscanf(buf,"Flags:%u",&f) != 1)
					break;
				m_flags = f;
				arPal.ReadString(buf,299);
		//		tmp[0] = 0;
		//		if (sscanf(buf,"Name:%[^\n]",tmp) != 1)
		//			break;
		//		strcpy(m_pal_name, tmp);
				}
			else
				break;
			}
		else if (kind == 0)
			{
			int j,r,g,b,o;
			if (sscanf(buf,"%d,%d,%d,%d,%d\n",&j,&r,&g,&b,&o) != 5)
				break;
			if (j != i)
				break;
			pals[i].kind = 0;
			pals[i].rgbo[0] = r;
			pals[i].rgbo[1] = g;
			pals[i].rgbo[2] = b;
			pals[i].rgbo[3] = o;
			pals[i].grad.kind = 1;
			pals[i].grad.p1.x = 160;
			pals[i].grad.p1.y = 359;
			pals[i].grad.p2.x = 479;
			pals[i].grad.p2.y = 120;
			pals[i].grad.c1 = pals[i].rgb;
			pals[i].grad.c2 = pals[i].rgb;
			pals[i].cname = 0;
			pals[i].fname = 0;
			pals[i].w = 1;
			pals[i].h = 1;
			pals[i].flags = 0;
			pals[i].size = 0;
			pals[i].pTexture = 0;
			}
		else
			{
			int j,k,r,g,b,o,pk;
			if (sscanf(buf,"%d,%d,%d,%d,%d,%d\n",&j,&k,&r,&g,&b,&o) != 6)
				break;
			if (j != i)
				break;
			pals[i].kind = k;
			pals[i].rgbo[0] = r;
			pals[i].rgbo[1] = g;
			pals[i].rgbo[2] = b;
			pals[i].rgbo[3] = o;
			arPal.ReadString(buf, 299);
			if (sscanf(buf,"    %3d,%3d,%3d,%3d,%3d", &pk,&r,&g,&b,&o) != 5)
				break;
			pals[i].grad.kind = pk;
			pals[i].grad.rgb1[0] = r;
			pals[i].grad.rgb1[1] = g;
			pals[i].grad.rgb1[2] = b;
			pals[i].grad.rgb1[3] = o;

			arPal.ReadString(buf, 299);
			if (sscanf(buf,"        %3d,%3d,%3d,%3d",&r,&g,&b,&o) != 4)
				break;
			pals[i].grad.rgb2[0] = r;
			pals[i].grad.rgb2[1] = g;
			pals[i].grad.rgb2[2] = b;
			pals[i].grad.rgb2[3] = o;

			arPal.ReadString(buf, 299);
			if (sscanf(buf,"        %3d,%3d,%3d,%3d",&r,&g,&b,&o) != 4)
				break;
			pals[i].grad.p1.x = r;
			pals[i].grad.p1.y = g;
			pals[i].grad.p2.x = b;
			pals[i].grad.p2.y = o;

			arPal.ReadString(buf, 299);
			tmp[0] = 0;
			if (buf[14])
				pals[i].cname = AddName(buf+14);
			arPal.ReadString(buf, 299);
			if (buf[14])
				pals[i].fname = AddName(buf+14);
			}
		}
	if ((i == 256) && (kind == 0))
		{
		UINT f;
		arPal.ReadString(buf, 299);
		if (sscanf(buf,"Flags:%u",&f) == 1)
			m_flags = f;
/* // per kent, use file name
		arPal.ReadString(buf, 299);
		if (sscanf(buf,"Name:%[^\n]",tmp) == 1)
			strcpy(m_pal_name, tmp);
*/
		}
	arPal.Close();
	if (i < 256)
		result = 2;
	else
		strcpy(m_file_name,name);
/*		// do it before import so scene can dup names
	if (!result && !m_pal_name[0])
		{
		int j = 0;
		for (i = 0; name[i]; i++)
			{
			if (name[i] == NATIVE_SEP_CHAR)
				j = i + 1;
			if (name[i] == '.')
				break;
			}
		int z;
		for (z = 0; ((z + 2) < sizeof(m_pal_name)) && (j <  i);)
			m_pal_name[z++] = m_file_name[j++];
		m_pal_name[z] = 0;
		}
*/
	return result;
}

UINT CNewPals::CompareFile()
{
	CFile file;
	DWORD mode = CFile::modeRead;
	if (!file.Open(m_file_name, mode))
		return 1;
	CArchive arPal(&file, CArchive::load);
	char buf[300];
	char tmp[300];
	int i,kind;
	kind = 0;
	int result = 0;
	for(i = -1; i < 256; i++)
		{
		arPal.ReadString(buf, 80);
		DPF("i:%d,%s",i,buf);
		if (i < 0)
			{
			if (buf[0] != 'D' || buf[1] != 'G')
				break;
			if (buf[2] == 'C')
				kind = 0;
			else if (buf[2] == 'K')
				{
				kind = 1;
				UINT f;
				arPal.ReadString(buf, 299);
				if (sscanf(buf,"Flags:%u",&f) != 1)
					break;
				if (m_flags != f)
					break;
				arPal.ReadString(buf,299);
				tmp[0] = 0;
				if (sscanf(buf,"Name:%[^\n]",tmp) != 1)
					break;
				if (_stricmp(m_pal_name, tmp))
					break;
				}
			else
				break;
			}
		else if (kind == 0)
			{
			int j,r,g,b,o;
			if (sscanf(buf,"%d,%d,%d,%d,%d\n",&j,&r,&g,&b,&o) != 5)
				break;
			if (j != i)
				break;
			if ((pals[i].kind != 0) ||
				(pals[i].rgbo[0] != r) ||
				(pals[i].rgbo[1] != g) ||
				(pals[i].rgbo[2] != b) ||
				(pals[i].rgbo[3] != o))
					break;
			}
		else
			{
			int j,k,r,g,b,o,pk;
			if (sscanf(buf,"%d,%d,%d,%d,%d,%d\n",&j,&k,&r,&g,&b,&o) != 6)
				break;
			if (j != i)
				break;
			if (j == 15)
			{
				i = j;
			}
			if ((pals[i].kind != k) ||
				(pals[i].rgbo[0] != r ) ||
				(pals[i].rgbo[1] != g ) ||
				(pals[i].rgbo[2] != b ) ||
				(pals[i].rgbo[3] != o ))
					break;
			arPal.ReadString(buf, 299);
			if (sscanf(buf,"    %3d,%3d,%3d,%3d,%3d", &pk,&r,&g,&b,&o) != 5)
				break;
			if ((pals[i].grad.kind != pk) ||
				(pals[i].grad.rgb1[0] != r) ||
				(pals[i].grad.rgb1[1] != g ) ||
				(pals[i].grad.rgb1[2] != b ) ||
				(pals[i].grad.rgb1[3] != o ))
				break;
			arPal.ReadString(buf, 299);
			if (sscanf(buf,"        %3d,%3d,%3d,%3d",&r,&g,&b,&o) != 4)
				break;
			pals[i].grad.rgb2[0] = r;
			pals[i].grad.rgb2[1] = g;
			pals[i].grad.rgb2[2] = b;
			pals[i].grad.rgb2[3] = o;

			arPal.ReadString(buf, 299);
			if (sscanf(buf,"        %3d,%3d,%3d,%3d",&r,&g,&b,&o) != 4)
				break;
			pals[i].grad.p1.x = r;
			pals[i].grad.p1.y = g;
			pals[i].grad.p2.x = b;
			pals[i].grad.p2.y = o;

			arPal.ReadString(buf, 299);
			tmp[0] = 0;
//			if (sscanf(buf,"        color:%[^\n]",tmp) != 1)
//				break;
//			pals[i].cname = AddName(tmp);
			if (buf[14])
				pals[i].cname = AddName(buf+14);
			arPal.ReadString(buf, 299);
//			if (sscanf(buf,"        file :%[^\n]",tmp) != 1)
//				break;
//			pals[i].fname = AddName(tmp);
			if (buf[14])
				pals[i].fname = AddName(buf+14);
			}
		}
	if ((i == 256) && (kind == 0))
		{
		UINT f;
		arPal.ReadString(buf, 299);
		if (sscanf(buf,"Flags:%u",&f) == 1)
			m_flags = f;
		arPal.ReadString(buf, 299);
		if (sscanf(buf,"Name:%[^\n]",tmp) == 1)
			strcpy(m_pal_name, tmp);
//		arPal.ReadString(buf, 299);
//		if (sscanf(buf,"FileName:%s",&tmp) == 1)
//			{
//			if (tmp[0])
//				strcpy(m_file_name, tmp);
//			}
		}
	arPal.Close();
	if (i < 256)
		result = 2;
	/*
	else if (!m_file_name[0])
		strcpy(m_file_name,name);
		if (!result && !m_pal_name)
			{
			int j = 0;
			int z = 0;
			for (i = 0; m_file_name[i] = name[i]; i++)
				{
				if (name[i] == NATIVE_SEP_CHAR)
					j = i + 1;
				if (name[i] == '.')
					z = i;
				}
			if (z)
				i = z;
			if ((i-j) >= sizeof(m_pal_name))
				j = i - 1 -sizeof(m_pal_name); // limit size of name
			for (z = 0; j <  i;)
				m_pal_name[z++] = m_file_name[j++];
			m_pal_name[z++] = 0;
			}
	*/	
	return result;
}
