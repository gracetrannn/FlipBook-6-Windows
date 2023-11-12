#include "MyIO.h"
#include "CLevels.h"
#include "CLevel.h"


#pragma pack(push,2)

typedef struct {
        DWORD   dwLevel;
        DWORD   dwKey;
} LEVELENTRY;

typedef struct {
	DWORD   dwKey;
	DWORD   dwKind;
	char	name[24];
	DWORD	LevelCount;
	LEVELENTRY levels[1];
} LEVELS;

#pragma pack(pop)

CLevels::CLevels(CMyIO * pIO) : CMyObj(pIO)
{
	DPF2("levels construct");
	Size(0);
	m_pData = new BYTE[m_size];
	LEVELS * p = (LEVELS *)m_pData;
	m_kind = p->dwKind = KIND_LEVELS;
	for (int i = 0; i < sizeof(p->name);p->name[i++] = 0);
	p->LevelCount = 0;
}

CLevels::~CLevels()
{
	DPF2("levels destruct");
}
DWORD CLevels::Size(DWORD c)
{
	m_size = sizeof(LEVELS) - sizeof(LEVELENTRY);
	m_size += c * sizeof(LEVELENTRY);
	return m_size;
}

DWORD CLevels::Select(int Level)
{
	ASSERT(m_pData != NULL);
	DWORD i,c,k;
	k = 0;
	LEVELS * p = (LEVELS *)m_pData;
	c = p->LevelCount;
	for (i = 0; i < c; i++)
		{
		if (p->levels[i].dwLevel >= (DWORD)Level)
			break;
		}
	if ((i < c) && (p->levels[i].dwLevel == (DWORD)Level))
		k = p->levels[i].dwKey;
	return k;
}
//
//
//	there was a problem prior to 10/14/03 whereby level count was too high
//	this allows scene::check to detect it
//
bool  CLevels::Check()
{
	ASSERT(m_pData != NULL);
	DWORD i,c;
	LEVELS * p = (LEVELS *)m_pData;
	c = p->LevelCount;
DPF2("check levels,c:%d",c);
	for (i = 0; i < c; i++)
		{
		DPZ("i:%d,l:%d,k:%d",i,p->levels[i].dwLevel,p->levels[i].dwKey);
		}
	DWORD j = 0;
	for (i = 0; i < c;)
		{
//		DPZ("i:%d,l:%d,k:%d",i,p->levels[i].dwLevel,p->levels[i].dwKey);
		DWORD e = 0;
		if (!p->levels[i].dwKey)
			e = 1;
		else if (i && !p->levels[i].dwLevel)
			e = 1;
		else if (p->levels[i].dwLevel > 100) // garbage check
			e = 1;
		if (e)
			{
			c--;
			for (j = i; j < c ; j++)
				p->levels[j] = p->levels[j+1];
			}
		else
			i++;
		}
	if (c < p->LevelCount)
		{
		DPZ("new level count:%d",c);
		for (i = 0; i < c; i++)
			{
			DPZ("i:%d,l:%d,k:%d",i,p->levels[i].dwLevel,p->levels[i].dwKey);
			}
		p->LevelCount = c;
		Write();
		}
	return i < c ? 1 : 0;		
}

DWORD CLevels::Insert(int Level)
{
	ASSERT(m_pData != NULL);
	DWORD i,c,k,j;
	k = 0;
	LEVELS * p = (LEVELS *)m_pData;
	c = p->LevelCount;
	for (i = 0; i < c; i++)
		{
		DPZ("i:%d,l:%d,k:%d",i,p->levels[i].dwLevel,p->levels[i].dwKey);
		if (p->levels[i].dwLevel >= (DWORD)Level)
			break;
		ASSERT(p->levels[i].dwKey);
		ASSERT(!i || p->levels[i].dwLevel);
		}
	if ((i < c) && (p->levels[i].dwLevel == (DWORD)Level))
		{
DPF2("clevels insert already");
		return  p->levels[i].dwKey;
		}
	c++;
	Size(c);
	BYTE * tp = new BYTE[m_size];
	LEVELS * np = (LEVELS *)tp;
	np->dwKey = p->dwKey;
	np->dwKind = p->dwKind;
	for (j = 0; j < sizeof(np->name);j++)
		np->name[j] = p->name[j];
	np->LevelCount = c;
	for (j = c;--j > i;)
		{
		np->levels[j].dwLevel = p->levels[j-1].dwLevel;
		np->levels[j].dwKey = p->levels[j-1].dwKey;
		}
	np->levels[i].dwLevel = Level;
	k = m_pIO->NewKey();
	np->levels[i].dwKey = k;
	for (j = i;j > 0;)
		{
		j--;
		np->levels[j].dwLevel = p->levels[j].dwLevel;
		np->levels[j].dwKey = p->levels[j].dwKey;
		}
	delete [] m_pData;
	m_pData = tp;
	tp = 0;
	Write();
	return k;
}

int CLevels::InsertLevel(int Start, int Count)
{
	DWORD i,c;
	LEVELS * p = (LEVELS *)m_pData;
	c = p->LevelCount;
	for (i = 0; i < c; i++)
		{
		if (p->levels[i].dwLevel >= (DWORD)Start)
			break;
		}
	for (; i < c; i++)
		p->levels[i].dwLevel += Count;
	return 0;
}


int CLevels::Read()
{
	m_pIO->GetSize(m_size,m_key);
	delete [] m_pData;
	m_pData = new BYTE[m_size];
	CMyObj::Read();
	DOSWAPX(m_pData,8,24,m_size);
	ASSERT(((LEVELS *)m_pData)->dwKind == KIND_LEVELS);
	return 0;
}

int CLevels::Write()
{
	DWORD c;
	LEVELS * p = (LEVELS *)m_pData;
	c = p->LevelCount;
	p->dwKey = m_key;
	Size(c);
	DOSWAPX(m_pData,8,24,m_size);
	CMyObj::Write();
	DOSWAPX(m_pData,8,24,m_size);
	return 0;
}

DWORD CLevels::zCount(int v /* = -1 */)
{
	LEVELS * p = (LEVELS *)m_pData;
	if (!p)
		{
		DPF2("levels, null data pointer");
		return 0;
		}
	if ((v != -1) && ((int)p->LevelCount != v))
		{
		ASSERT(m_pData != NULL);
		ASSERT(v <= 100);
		ASSERT(v > 0);
		int c,i;
		c = p->LevelCount;
		if (c < v)
			{
			Size(v);
			BYTE * tp = new BYTE[m_size];
			LEVELS * np = (LEVELS *)tp;
			*np = *p;
//			memcpy(np, p, sizeof(LEVELS));
			for (i = 1; i < c;i++)
				np->levels[i] = p->levels[i];
			for (; i < v; i++)
				{
				np->levels[i].dwLevel = 0;
				np->levels[i].dwKey = 0;
				}
			delete [] m_pData;
			m_pData = tp;
			p = np;
			}
		else if (c > v)
			{
			ASSERT(0);
			Size(v);
			BYTE * tp = new BYTE[m_size];
			LEVELS * np = (LEVELS *)tp;
			*np = *p;
			for (i = 1; i < v;i++)
				np->levels[i] = p->levels[i];
			delete [] m_pData;
			m_pData = tp;
			p = np;
			}
		p->LevelCount = v;
		Write();
		}
	return p->LevelCount;
}
#ifdef ZMYBUG
void CLevels::Display()
{
	LEVELS * p = (LEVELS *)m_pData;
	if (!p)
		{
		DPF2("levels, null data pointer");
		return;
		}
	DPF2("Display Levels, key:%8lx,kind:%8lx",p->dwKey,p->dwKind);
	DPF2("Name:%s|",p->name);
	DWORD i, c;
	c = p->LevelCount;
	DPF2("Count:%ld",c);
	if (!c)
		return;
	for (i = 0; i < c; i++)
		{
		DPF2("i:%ld,level:%ld,key:%8lx",i,
			p->levels[i].dwLevel,p->levels[i].dwKey);
		}

	for (i = 0; i < c; i++)
		{
		CLevel * pl = new CLevel(m_pIO);
		pl->SetKey(p->levels[i].dwKey);
		pl->Read();
		pl->Display();
		delete pl;
		pl = 0;
		}

	DPF2("end of levels");
}
#endif
