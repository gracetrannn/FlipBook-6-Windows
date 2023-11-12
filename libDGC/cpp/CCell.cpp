#include "MyIO.h"
#include "CCell.h"

#pragma pack(push,2)

typedef struct {
        DWORD   dwType;
        DWORD   dwKey;
} LAYER;

typedef struct {
	DWORD   dwKey;
	DWORD   dwKind;
	DWORD	dwFlags;
	char	name[24];
	DWORD	LayerCount;
	LAYER layers[1];
} CELL;

#pragma pack(pop)

CCell::CCell(CMyIO * pIO) : CMyObj(pIO)
{
	DPF2("cell construct");
	Size(0);
	m_pData = new BYTE[m_size];
	CELL * p = (CELL *)m_pData;
	m_kind = p->dwKind = KIND_CELL;
	p->dwFlags = 0;
	for (int i = 0; i < sizeof(p->name);p->name[i++] = 0);
	p->LayerCount = 0;
}

CCell::~CCell()
{
//	name[0] = 0;
//	LayerCount = 0;
}

DWORD CCell::Size(DWORD c)
{
	m_size = sizeof(CELL) - sizeof(LAYER);
	m_size += c * sizeof(LAYER);
	return m_size;
}

DWORD CCell::Flags(UINT v)
{
	ASSERT(m_pData != NULL);
	CELL * p = (CELL *)m_pData;
	if (v != NEGONE)
		p->dwFlags = v;
	return p->dwFlags;
}

DWORD CCell::Select(int Which)
{
	ASSERT(m_pData != NULL);
	DWORD i,k;
	CELL * pCell = (CELL *)m_pData;
	k = 0;
	if (Which == 99)
		{
		Which = LAYER_THUMB;
		k = 1;
		}
	for (i = 0; i < pCell->LayerCount; i++)
		{
		DWORD layerType = pCell->layers[i].dwType;
		if (Which == 98)
			{
			if (layerType == LAYER_INK)
				break;
			if (layerType == LAYER_GRAY)
				break;
			if (layerType == LAYER_PAINT)
				break;
			if ((layerType >= LAYER_MATTE0) && (layerType <= LAYER_MATTE9))
				break;
			}
#ifdef FBVER7
		else if (Which == 97)
			{
			if (layerType == LAYER_OLD_OVERLAY)
				break;
			if (layerType == LAYER_OLD_BG)
				break;
			}
#endif
		else if (layerType == (DWORD)Which)
			break;
		}
	if (i < pCell->LayerCount)
		k = pCell->layers[i].dwKey;  // found something
	else if (pCell->LayerCount && k)			// no thumb, but something
		k = 0;
	return k;
}

DWORD CCell::Insert(int Which)
{
	ASSERT(m_pData != NULL);
	DWORD i,c,k,j;
	k = 0;
	CELL * p = (CELL *)m_pData;
	c = p->LayerCount;
	for (i = 0; i < c; i++)
		{
		if (p->layers[i].dwType == (DWORD)Which)
			break;
		}
	if (i < c)
		{
DPF2("insert already");
		return  p->layers[i].dwKey;
		}
	c++;
	Size(c);
	BYTE * tp = new BYTE[m_size];
	CELL * np = (CELL *)tp;
	np->dwKey = p->dwKey;
	np->dwFlags = p->dwFlags;
	np->dwKind = p->dwKind;
	for (j = 0; j < sizeof(np->name);j++)
		np->name[j] = p->name[j];
	for (j = 0; j < (c - 1); j++)
		{
		np->layers[j] = p->layers[j];
		}
	delete [] m_pData;
	np->LayerCount = c;
	np->layers[c-1].dwType = Which;
	k = m_pIO->NewKey();
	np->layers[c-1].dwKey = k;
	m_pData = tp;
	Write();
	tp = 0;
	return k;
}
#ifdef FBVER7
DWORD CCell::ChangeKind(DWORD key, int which)
{
	ASSERT(m_pData != NULL);
	DWORD i,c;
	CELL * p = (CELL *)m_pData;
	c = p->LayerCount;
	for (i = 0; i < c; i++)
		{
		if (p->layers[i].dwKey == key)
			{
			p->layers[i].dwType = which;
			break;
			}
		}
	if (i < c)
		Write();
	return (i < c) ? 1 : 0;
}
#endif
DWORD CCell::DeleteLayer(int which)
{
	ASSERT(m_pData != NULL);
	DWORD i,c,k,j;
	k = 0;
	CELL * p = (CELL *)m_pData;
	c = p->LayerCount;
	if (which == 99) // if all
		{
		for (i = 0; i < c; i++)
			m_pIO->DelRecord(p->layers[i].dwKey);
		p->LayerCount = 0;
		Write();
		return 0;
		}
	for (i = 0; i < c; i++)
		{
		if (p->layers[i].dwType == (DWORD)which)
			break;
		}
	if (i >= c)
		{
DPF2("layer not found");
		return  1;
		}
	c--;
	Size(c);
	BYTE * tp = new BYTE[m_size];
	CELL * np = (CELL *)tp;
	np->dwKey = p->dwKey;
	np->dwFlags = p->dwFlags;
	np->dwKind = p->dwKind;
	for (j = 0; j < sizeof(np->name);j++)
		np->name[j] = p->name[j];
	for (j = 0; j < i; j++)
		np->layers[j] = p->layers[j];
	m_pIO->DelRecord(p->layers[i].dwKey);
	for (; i < c; i++)
		np->layers[i] = p->layers[i+1];
	delete [] m_pData;
	np->LayerCount = c;
	m_pData = tp;
	Write();
	tp = 0;
	return k;
}

int CCell::Read()
{
	m_pIO->GetSize(m_size,m_key);
	delete [] m_pData;
	m_pData = new BYTE[m_size];
	int z = CMyObj::Read();
	if (z )
		return z;
	DOSWAPX(m_pData,12,24,m_size);
	CELL * pCell = (CELL *)m_pData;
	ASSERT(pCell->dwKind == KIND_CELL);
	pCell->name[sizeof(pCell->name)-1] = 0; // truncate
	if (pCell->dwKind != KIND_CELL) return 99;
	return 0;
}

int CCell::Write()
{
	CELL * p = (CELL *)m_pData;
	p->dwKey = m_key;
	Size(p->LayerCount);
	DOSWAPX(m_pData,12,24,m_size);
	CMyObj::Write();
	DOSWAPX(m_pData,12,24,m_size);
	return 0;
}

void CCell::Name(LPSTR name, bool bPut)
{
	CELL * p = (CELL *)m_pData;
	int l = sizeof(p->name) - 1;
	if (bPut)
		{
		strncpy(p->name, name, l);
		p->name[l] = 0;
		}
	else
		{
		p->name[l] = 0;
		strcpy(name, p->name);
		}
}



DWORD CCell::Get(CFile & sf)
{
	CELL * cp = (CELL *)m_pData;
	LAYER lays[20]; 
	sf.Read(cp, m_size);
	UINT c = cp->LayerCount;
	sf.Read(&lays, c * sizeof(LAYER));
	DOSWAPC(&lays, c * sizeof(LAYER));
	m_key = cp->dwKey = m_pIO->NewKey();
	cp->LayerCount = 0;
	UINT i;
	for (i = 0; i < c; i++)
		lays[i].dwType = Insert(lays[i].dwType);
	DOSWAPC(m_pData,m_size);
	CMyObj::Write();
	DOSWAPC(m_pData,m_size);
	for (i = 0; i < c; i++)
		{
		UINT size = lays[i].dwKey; // actually size
		UINT * p = new UINT[(size+3) / 4];
		sf.Read(p,size);
		*p = SWAPV(lays[i].dwType);
		m_pIO->PutRecord(p,size,lays[i].dwType);
		delete [] p;
		}
	return m_key;
}

DWORD CCell::Put(CFile & sf)
{
	UINT pl[20];
	UINT i,c;
	DWORD size;
	CELL * p = (CELL *)m_pData;
	c = p->LayerCount;
	for (i = 0; i < c; i++)
		{
		pl[i] = p->layers[i].dwKey;
		m_pIO->GetSize(size,pl[i]); 
		p->layers[i].dwKey = size;	// replace key with size, leave typ alone
		}
	sf.Write(m_pData, m_size);
	for (i = 0; i < c; i++)
		{
		size = p->layers[i].dwKey;
		BYTE * p = new BYTE[size];
		m_pIO->GetRecord(p, size, pl[i]); // and these are keys
		sf.Write(p,size);
		delete [] p;
		}
	return p->dwKey;
}

DWORD CCell::Duplicate(CMyIO * pIO, DWORD key, bool bKeepName)
{
	DPF("ccell dup:%u",key);
	DWORD csize;
	pIO->GetSize(csize,key); 
	LPBYTE p1 = new BYTE[csize];
	pIO->GetRecord(p1, csize, key);
	CELL * pc = (CELL *)p1;
	ASSERT(pc->dwKey == key);
	ASSERT(pc->dwKind == KIND_CELL);
	ASSERT(csize == sizeof(CELL) + pc->LayerCount * sizeof(LAYER) - sizeof(LAYER)); 
	UINT max_size = 10000;
	LPBYTE pLayer = new BYTE[max_size];
	DWORD newkey;
	newkey = pIO->NewKey(); // get it first so sequence is the same as normal
	pc->dwKey = newkey;
	if (!bKeepName)
		pc->name[0] = 0;
	UINT i;
	for (i = 0; i < pc->LayerCount;i++)
		{
		DWORD lsize,lkey;
		lkey = pc->layers[i].dwKey; 
		pIO->GetSize(lsize, lkey); 
		if (lsize > max_size)
			{
			delete [] pLayer;
			max_size = lsize + 10000;
			pLayer = new BYTE[max_size];
			}
		DWORD * pdw = (DWORD *)pLayer;
		pIO->GetRecord(pLayer, lsize, lkey);
		ASSERT(pdw[0] == lkey);
		ASSERT(pdw[1] == KIND_IMAGE);
		DWORD newlkey;
		newlkey = pIO->NewKey();
		pc->layers[i].dwKey = newlkey; 
		pdw[0] = newlkey;
		pIO->PutRecord(pLayer, lsize, newlkey);
		}
	delete [] pLayer;
	pIO->PutRecord(p1, csize, newkey);
	delete [] p1;
	return newkey;
}


#ifdef QMYBUG
void CCell::Display()
{
	CELL * p = (CELL *)m_pData;
	if (!p)
		{
		DPF2("level, null data pointer");
		return;
		}
	DPF2("Display Cell, key:%8lx,kind:%8lx",p->dwKey,p->dwKind);
	DPF2("Name:%s|",p->name);
	DWORD i, c;
	c = p->LayerCount;
	DPF2("Count:%ld",c);
	if (!c)
		return;
	for (i = 0; i < c; i++)
		{
		DPF2("i:%ld,type:%ld,key:%8lx",i,
			p->layers[i].dwType,p->layers[i].dwKey);
		}
	DPF2("end of cell");
}
#endif
