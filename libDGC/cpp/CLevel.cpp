#include "MyIO.h"
#include "CLevel.h"
#include "CCell.h"
#include "CLevtbl.h"

#pragma pack(push,2)

typedef struct {
        DWORD   dwFrame;
        DWORD   dwKey;
} LEVELCELL;

typedef struct {
	DWORD   dwKey;
	DWORD   dwKind;
	char	name[20];
	DWORD	dwFlags;
	DWORD	FrameCount;
	LEVELCELL cells[1];
} LEVELOLD;

typedef struct {
	DWORD   dwKey;
	DWORD   dwKind;
	char	name[20];
	DWORD	dwFlags;
	DWORD	FrameCount;
	char	palname[256];
	BYTE	pals[1024];
	LEVELCELL cells[1];
} LEVELX;

typedef struct {
	DWORD   dwKey;
	DWORD   dwKind;
	char	name[20];
	DWORD	dwFlags;
	DWORD	FrameCount;
	char	palname[256];
	char	modelname[256];
	BYTE	pals[1024];
	LEVELCELL cells[1];
} LEVEL_MODEL;

typedef struct {
	DWORD   dwKey;
	DWORD   dwKind;
	char	name[20];
	DWORD	dwFlags;
	DWORD	FrameCount;
	char	palname[256];
	char	modelname[256];
	BYTE	pals[1024];
	UINT	layer;
	LEVLAY  table[11];
//	LEVTBL  table;
	LEVELCELL cells[1];
} LEVELV5;

typedef struct {
	DWORD   dwKey;
	DWORD   dwKind;
	char	name[20];
	DWORD	dwFlags;
	DWORD	FrameCount;
	UINT	index;
//	char	palname[256];
	char	modelname[256];
//	BYTE	pals[1024];
	LEVTBL  table;
	LEVELCELL cells[1];
} LEVEL;

#pragma pack(pop)

void SwapEm2(void * pp, UINT c)
{
	BYTE *p = (BYTE *)pp;
	BYTE t;
	for (;c>3;c-=4,p+=4)
		{
		t = p[0];
		p[0] = p[3];
		p[3] = t;
		t = p[1];
		p[1] = p[2];
		p[2] = t;
		}
	if( c > 1)
		{
		t = p[0];
		p[0] = p[1];
		p[1] = t;
		}
		}
	
/*
	we nned this to correct bad scenes created in fall of 2009 due
to bug in version 6 on powerpc priior to version 6.52
Bin == 2 reverses bad swap on powerpc
*/
void SwapLevel(BYTE * p, int bIn, DWORD c, DWORD k)
{
//	DPF2("sizeof level:%d",sizeof(LEVEL));
	LEVEL * lp = (LEVEL *)p;
	
//	DPF2("key:%d,kind:%d,flg:%d,c:%d",lp->dwKey,lp->dwKind,lp->dwFlags,lp->FrameCount);
	SwapEm2(p, 8);
	p += 8;
//	DPF2("name:%s",lp->name);
	p += 20;
	SwapEm2(p, 8);
	p += 8;
//	DPF2("key:%d,kind:%d,flg:%d,c:%d",lp->dwKey,k,lp->dwFlags,c);
	if (k == KIND_LEVEL_PAL)
		{
		p += 1280;
		}
	else if (k == KIND_LEVEL_MODEL)
		{
		p += 1536;
		}
	else if (k == KIND_LEVEL_MATTE)
		{
		p += 1536;		// to level table
		SwapEm2(p, 4);	// layer
		p += 4;
		int i;
//	SwapEm2(p, 11 * sizeof(LEVLAY) + 8 * c);
		for (i = 0; i < 11; i++)
			{
			p += 20;
			SwapEm2(p, 2);
			p += 2;
			SwapEm2(p, 2);
			p += 2;
			SwapEm2(p, 2);
			p += 2;
			SwapEm2(p, 2);
			p += 2;
			SwapEm2(p, 2);
			p += 2;
			}
		p += 2; // skip pad
		}
else if (k == KIND_LEVEL_NEW)
		{
		if (bIn != 2)
			{
		SwapEm2(p, 4);	// index
		p += 4;			// to model
			}
		p += 256;		// to level tabl
		SwapEm2(p, 4);	// layer
		p += 4;			// to table
		int i;
//	SwapEm2(p, 11 * sizeof(LEVLAY) + 8 * c);
		for (i = 0; i < 11; i++)
			{
			p += 20;
			SwapEm2(p, 2);
			p += 2;
			SwapEm2(p, 2);
			p += 2;
			SwapEm2(p, 2);
			p += 2;
			SwapEm2(p, 2);
			p += 2;
			SwapEm2(p, 2);
			p += 2;
			}
		}
	if (bIn != 2)
		p += 2;		// slip over pad
LEVELCELL * cp = (LEVELCELL *)p;
//	DPF("frame:%d,key:%x",cp->dwFrame,cp->dwKey);
	SwapEm2(p, 8 * c);
//	DPF("frame:%d,key:%x",cp->dwFrame,cp->dwKey);
//	DWORD i;
//	for (i = 0; i < c; i++, cp++)
//		{
//		DPF("i:%2d,frame:%d,key:%x",i,cp->dwFrame,cp->dwKey);
//		DPF("i:%2d,frame:%d,key:%x",i,lp->cells[i].dwFrame,lp->cells[i].dwKey);
//		}
}
//#endif


CLevel::CLevel(CMyIO * pIO) : CMyObj(pIO)
{
//	DPF2("level construct");
	Size(0);
	m_pData = new BYTE[m_size];
	LEVEL * p = (LEVEL *)m_pData;
	memset(m_pData,0,m_size);
	p->dwFlags = 1;
	p->index = 0;// scene pal
	m_kind = p->dwKind = KIND_LEVEL_NEW;
//	p->table.layer = 0;
//	memset(p->table.table, 0, sizeof(p->table));
	p->table.table[0].name[1] = (char)255;
}

DWORD CLevel::Size(DWORD c)
{
	m_size = sizeof(LEVEL) - sizeof(LEVELCELL);
	m_size += c * sizeof(LEVELCELL);
	return m_size;
}

DWORD CLevel::Select(int Frame, bool bHold /* = 0 */)
{
	ASSERT(m_pData != NULL);
	DWORD i,c,k,f;
	f = Frame;
	LEVEL * p = (LEVEL *)m_pData;
	k = 0;
	c = p->FrameCount;
/*
	for (i = 0; i < c; i++)
		{
		DPZ("i:%ld,frame:%ld,key:%8lx",i,p->cells[i].dwFrame,p->cells[i].dwKey);
		}
DPZ("lvl sel,c:%d,f:%d",c,Frame);
*/
	for (i = 0; i < c; i++)
		{
		if (p->cells[i].dwFrame >= f)
			break;
		}
	if ((i < c) && (p->cells[i].dwFrame == f))
		k = p->cells[i].dwKey;
	else if (i && bHold)
		k = p->cells[i-1].dwKey;
//DPF2("i:%d,k:%lx",i,k);
	return k;
}

void CLevel::MinMax(UINT & min, UINT & max, UINT Frame)
{
	ASSERT(m_pData != NULL);
	DWORD i,c,k,f;
	f = Frame;
	LEVEL * p = (LEVEL *)m_pData;
	k = 0;
	c = p->FrameCount;
	for (i = 0; i < c; i++)
		{
		if (p->cells[i].dwFrame >= f)
			break;
		}
	if (!c)
		{
		min = 0;
		return;
		}
	else if (i)
		min = p->cells[i-1].dwFrame;
	if (p->cells[i].dwFrame > f)
		max = p->cells[i].dwFrame;
	else if ((i + 1) < c) 
		max = p->cells[i+1].dwFrame;
}

UINT CLevel::Before(UINT * pList, UINT max, UINT Frame)
{
	ASSERT(m_pData != NULL);
	UINT q;
	DWORD i,c,k;
	LEVEL * p = (LEVEL *)m_pData;
	k = 0;
	c = p->FrameCount;
	if (Frame == NEGONE)
		{
		if (c > max)
			c = max;
		for (i = 0; i < c; i++)
			pList[i] = p->cells[i].dwFrame;
		return i;
		}
	for (i = 0; i < c; i++)
		{
		if (p->cells[i].dwFrame >= Frame)
			break;
		}
	if (!max)
		{
		UINT res = 1;
		if (i < c)
			{
			if (p->cells[i].dwFrame == Frame)
				pList[0] = p->cells[i].dwFrame;
			else if (i)
				pList[0] = p->cells[i-1].dwFrame;
			else
				res = 0;
			}
		else if (c)
			{
			pList[0] = p->cells[c-1].dwFrame;
			}
		else
			res = 0;
		return res;
		}
	for (q = 0;i-- && (q < max);)
		pList[q++] = p->cells[i].dwFrame;
	return q;
}

UINT CLevel::Next(UINT Frame)
{
	ASSERT(m_pData != NULL);
	DWORD i,c;
	LEVEL * p = (LEVEL *)m_pData;
	c = p->FrameCount;
	for (i = 0; i < c; i++)
		if (p->cells[i].dwFrame >= Frame)
			break;
	if (i < c)
		Frame = p->cells[i].dwFrame;
	else
		Frame = NEGONE;
	return Frame;
}

int	 CLevel::MoveFrames(UINT From, UINT To, UINT Count)
{
	ASSERT(m_pData != NULL);
	DWORD c;
	if (!Count)
		Count = 99999;
	DWORD Last = From + Count;
	UINT i;
	LEVEL * p = (LEVEL *)m_pData;
	c = p->FrameCount;
//DPF2("lvl move frames:c:%d,from:%d,To:%d,cnt::%d",c,From, To,Count);
	for (i = 0; i < c; i++)
		{
		if ((p->cells[i].dwFrame >= (DWORD)From) &&
			(p->cells[i].dwFrame < Last))
				p->cells[i].dwFrame += To - From;
		}
	Write();
	return 0;
	
}
void CLevel::setSceneClosures(std::function<UINT(BYTE*, BYTE*)> forgePalsClosure, std::function<CNewPals* (UINT)> palettePtrClosure)
{
	m_forgePalsClosure = forgePalsClosure;
	m_palettePtrClosure = palettePtrClosure;
}
DWORD CLevel::Insert(int Frame, DWORD key, bool bSwap)
{
	ASSERT(m_pData != NULL);
	DWORD c,k;
	UINT i,j;
	k = 0;
	LEVEL * p = (LEVEL *)m_pData;
	c = p->FrameCount;
/*
DPF2("insert cell into level,frm:%d,k:%d",Frame,key);
	for (i = 0; i < c; i++)
		{
DPF2("i:%ld,frame:%ld,key:%8lx",i,p->cells[i].dwFrame,p->cells[i].dwKey);
		}
*/
	for (i = 0; i < c; i++)
		{
		if (p->cells[i].dwFrame >= (DWORD)Frame)
			break;
		}
	if ((i < c) && (p->cells[i].dwFrame == (DWORD)Frame))
		{
//DPF2("clevel insert already");
		if (bSwap)
			{
			DWORD old = p->cells[i].dwKey;
			p->cells[i].dwKey = key;
			Write();
			return old;
			}
		return  p->cells[i].dwKey;
		}
//DPF2("lvel insrt, i:%d,c:%d,f:%d",i,c,Frame);
if (c)
	{
	//DPF2("i:%d,f:%d,k:%x",i,p->cells[c-1].dwFrame,p->cells[c-1].dwKey);
	}
	c++;
	Size(c);
	BYTE * tp = new BYTE[m_size];
	LEVEL * np = (LEVEL *)tp;
	memmove(np,p,sizeof(LEVEL) - sizeof(LEVELCELL));
	np->FrameCount = c;
	for (j = c;--j > i;)
		{
		np->cells[j].dwFrame = p->cells[j-1].dwFrame;
		np->cells[j].dwKey = p->cells[j-1].dwKey;
		}
	np->cells[i].dwFrame = Frame;
	if (!key)
		k = m_pIO->NewKey();
	else
		k = key;
	np->cells[i].dwKey = k;
	for (j = i;j > 0;)
		{
		j--;
		np->cells[j].dwFrame = p->cells[j].dwFrame;
		np->cells[j].dwKey = p->cells[j].dwKey;
		}
	delete [] m_pData;
	m_pData = tp;
	tp = 0;
	Write();
	return bSwap ? 0 : k;
}

DWORD CLevel::DeleteCell(int Frame)
{
	ASSERT(m_pData != NULL);
	DWORD c,k;
	UINT i,j;
	k = 0;
	LEVEL * p = (LEVEL *)m_pData;
	c = p->FrameCount;
//DPF2("delete cell from level,frm:%d",Frame);
/*
	for (i = 0; i < c; i++)
		{
//DPF2("i:%ld,frame:%ld,key:%8lx",i,p->cells[i].dwFrame,p->cells[i].dwKey);
		}
*/
	for (i = 0; i < c; i++)
		{
		if (p->cells[i].dwFrame >= (DWORD)Frame)
			break;
		}
	if ((i >= c) || (p->cells[i].dwFrame != (DWORD)Frame))
		{
//DPF2("clevel delete not found");
		return  0;
		}
	k = p->cells[i].dwKey;
//DPF2("lvel delete, i:%d,c:%d,f:%d",i,c,Frame);
	c--;
	Size(c);
	BYTE * tp = new BYTE[m_size];
	LEVEL * np = (LEVEL *)tp;
	memmove(np,p,sizeof(LEVEL) - sizeof(LEVELCELL));
	np->FrameCount = c;
	for (j = 0; j < i; j++)
		np->cells[j] = p->cells[j];
	for (; i < c; i++)
		np->cells[i] = p->cells[i+1];
	delete [] m_pData;
	m_pData = tp;
	tp = 0;
	Write();
	return k;
}

int CLevel::Read()
{
	BYTE pals[1024];
	BYTE name[256];
	m_pIO->GetSize(m_size,m_key);
	delete [] m_pData;
	m_pData = new BYTE[m_size];
	LEVEL * lp = (LEVEL *)m_pData;
	int result = CMyObj::Read();
	if (result)
		{
		memset(m_pData,0,m_size);
		return result;
		}
    
	bool bBadScene = FALSE; // detect fall 2009 powerpc scene
	UINT frameCount = SWAPV(lp->FrameCount);
	UINT kk = SWAPV(lp->dwKind);
	if (kk == KIND_LEVEL_NEW) {
        //
        // if a new level structure and at least one frame
        // then check for a null key in the first slot
        //
		if (frameCount > 0 && lp->cells[0].dwKey == 0)
            bBadScene = TRUE;
    }
#ifdef _NEEDSWAP
	if (bBadScene)
		SwapLevel(m_pData,2,fc,kk);
	else
		SwapLevel(m_pData,1,fc,kk);
#else
	if (bBadScene)
		{
		SwapLevel(m_pData,2,frameCount,kk);
		SwapLevel(m_pData,1,frameCount,kk);
		}
#endif
	LEVEL * np = 0;
	BYTE * pOld = m_pData;
	name[0] = 2; // default
	DWORD kind = lp->dwKind;
//	if ((size == m_size) && (kind != KIND_LEVEL_MATTE))
//		{
//		DPZ("size:%d.msize:%d",size,m_size);
//		kind = lp->dwKind = KIND_LEVEL_MATTE;
//		Write();
//		}
	if (kind == KIND_LEVEL)
		{
		LEVELOLD * op = (LEVELOLD *)pOld;
		UINT c = op->FrameCount;
		Size(c);
		m_pData = new BYTE[m_size];
		np = (LEVEL *)m_pData;
		np->dwKey = op->dwKey;
		UINT j;
		memcpy(np->name,op->name,sizeof(np->name));
		np->dwFlags = op->dwFlags;
		np->index = 0; // scene pals
		memset(&np->modelname,0,sizeof(np->modelname));
		np->FrameCount = c;
		for (j = 0; j < c; j++)
			np->cells[j] = op->cells[j];
		}
	else if (kind == KIND_LEVEL_PAL)
		{
		LEVELX * op = (LEVELX *)pOld;
		UINT c = op->FrameCount;
		Size(c);
		m_pData = new BYTE[m_size+4];// 8/19/08 I do not know why extra 4needed
		np = (LEVEL *)m_pData;
		np->dwKey = op->dwKey;
		UINT j;
		memcpy(np->name,op->name,sizeof(np->name));
		np->dwFlags = op->dwFlags;
		memcpy(pals,op->pals,sizeof(pals));
		memcpy(name,op->palname,sizeof(name));
		memset(&np->modelname,0,sizeof(np->modelname));
		np->FrameCount = c;
		for (j = 0; j < c; j++)
			np->cells[j] = op->cells[j];
		}
	else if (kind == KIND_LEVEL_MODEL)
		{
		LEVEL_MODEL * op = (LEVEL_MODEL *)pOld;
		UINT c = op->FrameCount;
		Size(c);
		m_pData = new BYTE[m_size+4];
		np = (LEVEL *)m_pData;
		np->dwKey = op->dwKey;
		UINT j;
		memcpy(np->name,op->name,sizeof(np->name));
		np->dwFlags = op->dwFlags;
		memcpy(pals,op->pals,sizeof(pals));
		memcpy(name,op->palname,sizeof(name));
		memcpy(np->modelname,op->modelname,sizeof(np->modelname));
		np->FrameCount = c;
		for (j = 0; j < c; j++)
			np->cells[j] = op->cells[j];
		}
	else if (kind == KIND_LEVEL_MATTE)
		{
		LEVELV5 * op = (LEVELV5 *)pOld;
		UINT c = op->FrameCount;
		Size(c);
		m_pData = new BYTE[m_size];
		np = (LEVEL *)m_pData;
		np->dwKey = op->dwKey;
		UINT j;
		memcpy(np->name,op->name,sizeof(np->name));
		np->dwFlags = op->dwFlags;
		memcpy(pals,op->pals,sizeof(pals));
		memcpy(name,op->palname,sizeof(name));
		memcpy(np->modelname,op->modelname,sizeof(np->modelname));
		np->table.layer = op->layer;
		memcpy(&np->table.table,&op->table, sizeof(np->table.table));
		np->FrameCount = c;
		for (j = 0; j < c; j++)
			np->cells[j] = op->cells[j];
		}
	else if (kind != KIND_LEVEL_NEW)
		{
		DPZ("bad kind:%d",kind);
		ASSERT(FALSE);
		lp->dwKind = KIND_LEVEL_NEW;
		Write();
	//	return -1;
		}
	if (np)
		{
		np->dwKind = KIND_LEVEL_NEW;
		np->index = m_forgePalsClosure(pals, name);
		if (kind < KIND_LEVEL_MATTE)
			{
			np->table.layer = 0;
			memset(&np->table.table, 0, sizeof(np->table));
			np->table.table[0].name[1] = (char)255;
			}
		np->name[sizeof(np->name) - 1] = 0;
		delete [] pOld;
		Write();
		}
	else
		{
		np = (LEVEL *)m_pData;
		np->name[sizeof(np->name) - 1] = 0;
		}
	return 0;
}

int CLevel::Write()
{
	LEVEL * np = (LEVEL *)m_pData;
	UINT c = np->FrameCount;
	Size(c);
	UINT index = np->index;
	CNewPals* pPals = m_palettePtrClosure(index);
#ifdef ZBACWARDS
	if (pPals->Simple()) // if no new pals then convert to old level format
		{
		m_size = sizeof(LEVELV5) - sizeof(LEVELCELL);
		m_size += c * sizeof(LEVELCELL);
		m_pData = new BYTE[m_size];
		LEVELV5 * op = (LEVELV5 *)m_pData;
		op->dwKey = np->dwKey;
		UINT j;
		memcpy(op->name,np->name,sizeof(np->name));
		op->dwFlags = np->dwFlags;
		BYTE * pp = (BYTE *)&op->pals;
		for (j = 0; j < 256; j++, pp+=4)
			pPals->Color(pp, j);
		memcpy(op->palname,pPals->PalName(),sizeof(op->palname));
		memcpy(op->modelname,np->modelname,sizeof(np->modelname));
		memcpy(&op->table,&np->table, sizeof(np->table));
		op->FrameCount = c;
		for (j = 0; j < c; j++)
			op->cells[j] = np->cells[j];
#ifdef _NEEDSWAP
		BYTE * pTemp = new BYTE[m_size];
		memmove(pTemp, m_pData, m_size);
		SwapLevel(m_pData,0,c, KIND_LEVEL_NEW);
#endif
		CMyObj::Write();
#ifdef _NEEDSWAP
		memmove(m_pData, pTemp, m_size);
		delete [] pTemp;
#endif

		delete [] m_pData;
		m_pData = (BYTE *)np;
		Size(c);
		}
	else
#endif
		{
		np->dwKey = m_key;
#ifdef _NEEDSWAP
		BYTE * pTemp = new BYTE[m_size];
		memmove(pTemp, m_pData, m_size);
		SwapLevel(m_pData,0,c,KIND_LEVEL_NEW);
#endif
		CMyObj::Write();
#ifdef _NEEDSWAP
		memmove(m_pData, pTemp, m_size);
		delete [] pTemp;
#endif
		}
	return 0;
}

void CLevel::Name(LPSTR name, bool bPut)
{
	LEVEL * p = (LEVEL *)m_pData;
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
	if (bPut)
		Write();
}

void CLevel::ModelName(LPSTR name, bool bPut)
{
	LEVEL * p = (LEVEL *)m_pData;
	if (bPut)
		strcpy(p->modelname, name);
	else
		strcpy(name, p->modelname);
	if (bPut)
		Write();
}

DWORD CLevel::Flags(DWORD Val /* = -1 */, bool bInit /* = 0 */)
{
	LEVEL * p = (LEVEL *)m_pData;
	if (bInit)
		{
		p->table.layer = 0;
		memset(&p->table, 0, sizeof(p->table));
		p->table.table[0].name[1] = (char)255;
		p->dwFlags = Val;
		}
	else if (Val != -1)
		{
		p->dwFlags = Val;
		Write();
		}
	return p->dwFlags;
}

UINT CLevel::PalIndex(UINT v)
{
	LEVEL * p = (LEVEL *)m_pData;
	if (v != NEGONE)
		{
		p->index = v;
		Write();
		}
	return p->index;
}
/*
UINT CLevel::Pals(BYTE * pPals, CScene * pScene)
{
	LEVEL * p = (LEVEL *)m_pData;
	if (!pScene)
		{
//		p->palname[0] = 1;
//		p->palname[1] = 0;
		memmove(&p->pals, pPals, 1024);
		Write();
		}
	else
		{
		if (!p->palname[0])
			memmove(pPals,pScene->ScenePals(),1024);
		else if (p->palname[0] == 2)
			memmove(pPals,pScene->DefPals(),1024);
		else
			memmove(pPals, &p->pals, 1024);
		}
	return 0;	
}
*/
CLevelTable * CLevel::Table()
{
	LEVEL * p = (LEVEL *)m_pData;
	return (CLevelTable *)&p->table;
}

UINT CLevel::Table(CLevelTable * pTbl, bool bCheck)
{
	LEVEL * p = (LEVEL *)m_pData;
	UINT Flag = 0;
	if (bCheck)
		{
		if (p->table.layer != pTbl->layer)
			{
			Flag |= 1;
			p->table.layer = pTbl->layer;
			}
		
		UINT i;
		for (i = 0; i < 11; i++)
			{
			if ((p->table.table[i].dx != pTbl->table[i].dx) ||
				(p->table.table[i].dy != pTbl->table[i].dy) ||
				(p->table.table[i].blur != pTbl->table[i].blur) ||
				(p->table.table[i].flags!= pTbl->table[i].flags) ||
				strcmp(p->table.table[i].name,pTbl->table[i].name))
				break;
			}
		if (i < 11)
			{
			Flag |= 4;
			memmove(&p->table.table, &pTbl->table, sizeof(p->table.table));
			}
		if (Flag)
			Write();
		if (Flag < 2)
			Flag = 0; // no need to recomposite
		}
	else
		{
//	pal name == 0 scene pals
//	1 is custom
//	2 is default (digicel)
//  else external file name
//
		pTbl->layer = p->table.layer;
//		Flag = p->palname[0];
//		if (!Flag)
//			memmove(&pTbl->pals,pScene->ScenePals(),1024);
//		else if (Flag == 2)
//			memmove(&pTbl->pals,pScene->DefPals(),1024);
//		else
//			memmove(&pTbl->pals, &p->pals, 1024);
		memmove(&pTbl->table, &p->table.table, sizeof(p->table.table));
		}
	return Flag;	

}

UINT CLevel::Cleanup(UINT frames)
{
	DWORD i,c;
	UINT result = 0;
	LEVEL * p = (LEVEL *)m_pData;
	c = p->FrameCount;
//	for (i = 0; i < 1024;i++)
//		if (p->pals[i]) break;
//	if ((i >= 1024) && (p->palname[0] == 1))
//		{
//		p->palname[0] = 0;
//		result++;
//		}
	for (i = 0; i < c; i++)
		{
		if (p->cells[i].dwFrame >= frames)
			{
DPZ("cleanup,i:%d,f:%d,c:%d,ff:%d",i,p->cells[i].dwFrame,c,frames);
			result++;
			p->FrameCount = i;
			}
		}
	if (result)
		Write();
	return result;
}
#ifdef ZMYBUG
void CLevel::Display()
{
	LEVEL * p = (LEVEL *)m_pData;
	if (!p)
		{
		DPF2("level, null data pointer");
		return;
		}
	DPF2("Display Level, key:%8lx,kind:%8lx",p->dwKey,p->dwKind);
	DPF2("Name:%s|",p->name);
	DWORD i, c;
	c = p->FrameCount;
	DPF2("Count:%ld",c);
	if (!c)
		return;
	for (i = 0; i < c; i++)
		{
		DPF2("i:%ld,frame:%ld,key:%8lx",i,
			p->cells[i].dwFrame,p->cells[i].dwKey);
		}
	for (i = 0; i < c; i++)
		{
		CCell * pc = new CCell(m_pIO);
		pc->SetKey(p->cells[i].dwKey);
		pc->Read();
		pc->Display();
		delete pc;
		pc = 0;
		}
	DPF2("end of level");
}
#endif
