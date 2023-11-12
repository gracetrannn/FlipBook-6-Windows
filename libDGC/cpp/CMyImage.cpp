#include "MyIO.h"
#include "CMyImage.h"
#include "zlib.h"

#pragma pack(push,2)

typedef struct {
	DWORD   dwKey;
	DWORD   dwKind;
	DWORD	dwFlags;
	DWORD	dwWidth;
	DWORD	dwHeight;
	DWORD	dwDepth;
} IMAGE;

#pragma pack(pop)

CMyImage::CMyImage(CMyIO * pIO) : CMyObj(pIO)
{
	DPF2("cimage construct");
	m_size = sizeof(IMAGE);
	m_pData = new BYTE[m_size];
	IMAGE * p = (IMAGE *)m_pData;
	m_kind = p->dwKind = KIND_IMAGE;
	p->dwFlags = 0;
}

CMyImage::~CMyImage()
{
	DPF2("cimage destruct");
}

int CMyImage::DeCompress(HPBYTE buf, DWORD size)
{
//	UINT p = (m_width + 7) / 8;
	DWORD isize;
	if (m_pIO->GetSize(isize, m_key))
		{
DPF2("decompress key failure ");
		return 1;
		}
	isize -= sizeof(IMAGE);
	BYTE * tbuf = new BYTE[isize];
	if (tbuf == NULL)
		{
DPF2("decompress mem failure ");
		return 1;
		}
	int result = m_pIO->GetRecord(tbuf, isize, m_key, sizeof(IMAGE));
	if (result)
		{
DPF2("dec read error");
		delete [] tbuf;
		return result;
		}
	unsigned long vc = size;
	UINT q = uncompress(buf,&vc,tbuf,isize);
	if (q)
		{
DPF2("decompress error:%d",q);
		delete [] tbuf;
		return 5;
		}
	delete [] tbuf;
	return 0;
}
int CMyImage::Read(HPBYTE buf, DWORD size)
{
	CMyObj::Read();
	IMAGE * p = (IMAGE *)m_pData;
	DOSWAPC(m_pData, sizeof(IMAGE));
	ASSERT(p->dwKind == KIND_IMAGE);
	if (p->dwKind != KIND_IMAGE)
		return 17;
	m_width = p->dwWidth;
	m_height = p->dwHeight;
	m_depth = p->dwDepth;
	m_flags = p->dwFlags;
DPF2("image read,key:%d,w:%d,h:%d,d:%d,f:%x",m_key,m_width,m_height,m_depth,m_flags);
	if (buf == NULL)
		return 0;
	if (!size)
		{
		if (m_depth == 16)
			size = 2 * m_height * 4 * ((m_width + 3) / 4);
		else if (m_depth == 24)
			size = m_height * 4 * ((3 * m_width + 3) / 4);
		else if (m_depth == 32)
			size = m_height * 4 * m_width;
		else
			size = m_height * 4 * ((m_width + 3) / 4);
		}
	int result;
	if (m_flags & 1)
		result = DeCompress(buf, size);
	else
		result = m_pIO->GetRecord(buf, size, m_key, sizeof(IMAGE));
	return result;
}

int CMyImage::Compress(HPBYTE buf, DWORD size)
{
//	UINT p = (m_width + 7) / 8;
//	DWORD dsize = 20 + (size * 102) / 100;
	unsigned long dsize = 20 + size + (size + 50) / 50; // avoid math overflow
	BYTE * tbuf = new BYTE[dsize];
	if (tbuf == NULL)
		{
DPF2("compress mem failure ");
		return 1;
		}
	UINT q = compress(tbuf,&dsize,buf,size);
	if (q)
		{
DPF2("compression failure:%d",q);
		delete [] tbuf;
		return 1;
		}
	int result;
	if (!(result = m_pIO->PutRecord(m_pData, m_size, m_key, 0, m_size + dsize)))
	result = m_pIO->PutRecord(tbuf, dsize, m_key, m_size);
	delete [] tbuf;
	return result;
}

int CMyImage::Write(HPBYTE buf)
{
DPF2("image write,w:%d,h:%d,d:%d",m_width,m_height,m_depth);
	IMAGE * p = (IMAGE *)m_pData;
	p->dwKey = m_key;
	p->dwKind = KIND_IMAGE;
	p->dwFlags = m_flags;
	p->dwWidth = m_width;
	p->dwHeight = m_height;
	p->dwDepth = m_depth;
	m_size = sizeof(IMAGE);
	DOSWAPC(m_pData, sizeof(IMAGE));
	DWORD size;
	if (m_depth == 16)
		size = 2 * m_height * 4 * ((m_width + 3) / 4);
	else if (m_depth == 24)
		size = m_height * 4 * ((3 * m_width + 3) / 4);
	else if (m_depth == 32)
		size = m_height * 4 * m_width;
	else
		size = m_height * 4 * ((m_width + 3) / 4);
//	CMyObj::Write();
	int result;
	if (m_flags & 1)
		result = Compress(buf, size);
	else
		{
		if (!(result = m_pIO->PutRecord(m_pData, m_size, m_key, 0, m_size + size)))
			result = m_pIO->PutRecord(buf, size, m_key, m_size);
		}
	return result;
}

#ifdef MYBUG
void CMyImage::Display()
{
	IMAGE * p = (IMAGE *)m_pData;
	if (!p)
		{
		DPF2("image, null data pointer");
		return;
		}
	DPF2("Display Image, key:%8lx,kind:%8lx",p->dwKey,p->dwKind);
	DPF2("w:%ld,h:%ld,d:%ld",m_width,m_height,m_depth);
}
#endif
