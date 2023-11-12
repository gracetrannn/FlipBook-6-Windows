//#define NOBUG
#include "Camera.h"
#include "CSceneBase.h"
#include "MyIO.h"
#include "math.h"


#define NBR_GROUPS 5
#define MAX_PEGS 100
#define NAME_SIZE 30


#pragma pack(push,2)

typedef struct {
	DWORD   dwKey;
	DWORD   dwKind;
	DWORD	dwFlags;
	UINT	Groups;
	UINT	Pegs;
	UINT	Frames;
	UINT	Levels;
	UINT	count;
	UINT	limit;
} CAMERAHDR;

#pragma pack(pop)



typedef struct {
	UINT	attr;
	UINT 	frame;
	UINT	peg;
#ifdef FBMAC
	UINT	qq;	// spacer since double aligned on double
#endif
	double	value;
	UINT	kind;
#ifdef FBMAC
	UINT	zz; // extra for size
#endif
} ATTRLIST;


typedef struct {
	UINT 	frame;
	UINT	kind;
	double	value;
} TEMPLIST;


typedef struct {
	UINT 	group;			// which
#ifdef FBMAC
	UINT	qq;				// spacer for alignment;
#endif
	double	pegx;			// peg center
	double	pegy;
	double	levelx;			// level center
	double	levely;			// 
	UINT 	ppeg;			// used for level list
							// low word is level, high word is peg flags
							// 0x10000 is field composite
	char	name[NAME_SIZE];
#ifdef FBMAC
	UINT morepadding;
#endif
} PEGLIST;

/*
typedef struct {
	double xoffset;
	double yoffset;
	double scale;
	double rotation;
	double dalpha;
} INFO;
*/


typedef struct {
	PEGLIST pl;
	UINT	count;
	BYTE *  pAttr;
} MOVLIST;


bool CheckCamera(CMyIO * pIO, DWORD Key)
{
	CAMERAHDR hdr;
#ifdef FBMAC
	int result = pIO->GetSwapRecord(&hdr, sizeof(hdr), Key);
#else
	int result = pIO->GetRecord(&hdr, sizeof(hdr), Key);
#endif
	if (result) return FALSE;
	ASSERT(hdr.dwKey == Key);
	ASSERT(hdr.dwKind == KIND_CAMERA);
	ASSERT(hdr.dwFlags == 0);
//	ASSERT(hdr.Groups == m_Groups);
//	ASSERT(hdr.Pegs == m_Pegs);
	return hdr.count ? TRUE : FALSE;
}

CCamera::CCamera(CMyIO * pIO, UINT key)
{
	m_pIO = pIO;
	m_Key = key;
	m_count = 0;
	m_limit = 0;
	m_pList = 0;
	m_pData = 0;
	m_pScene = 0;
	m_Groups = NBR_GROUPS;
	m_Pegs = MAX_PEGS;
	m_Levels = 0;
	m_Frames = 0;
	m_bDeferWrite = 1;
	UINT size = (m_Pegs + m_Groups) * sizeof(PEGLIST);
	m_pPegs = new BYTE[size];
	memset(m_pPegs, 0, size);
	double z = 0;
	UINT i;
	for (i = CATTR_Y; i < CATTR_EXTRA; m_dirty[i++] = TRUE);
	for (i = 0; i < m_Groups; i++)
		{
		char buf[40];
		if (i > 1)
			sprintf(buf,"Group %d", i-1);
		else if (i)
			sprintf(buf,"Table Top");
		else
			sprintf(buf,"Camera");
		GroupName(buf,i,TRUE);
		PegCenter(z,z,i+1000,TRUE);
		}
	for (i = 0; i < m_Pegs; i++)
		{
		char buf[40];
		sprintf(buf,"Peg %d", i);
		PegName(buf,i,TRUE);
		PegCenter(z,z,i,TRUE);
		LevelCenter(z,z,i,TRUE);
		PegAttach(i,i);
		GroupAttach(i,1);
		}

	DPF("grp:%d",GroupFindPeg(0));
	m_bDeferWrite = 0;
	if (m_bModified)
		MakeModified();
}

CCamera::~CCamera()
{
	delete [] m_pList;
	m_pList = 0;
	delete [] m_pData;
	m_pData = 0;
	delete [] m_pPegs;
	m_pPegs = 0;
}

void CCamera::Mover(bool bRead, BYTE * tp)
{
	UINT tsize = sizeof(PEGLIST) * (m_Pegs + m_Groups);
	if (bRead)
		{
		m_pPegs = new BYTE [tsize];
		}
	PEGLIST * dpp;
	PEGLIST * spp;
	if (bRead)
		{
		dpp = (PEGLIST *)m_pPegs; 
		spp = (PEGLIST *)tp; 
		}
	else
		{
		spp = (PEGLIST *)m_pPegs; 
		dpp = (PEGLIST *)tp; 
		}
#ifdef FBMAC
	UINT t;
	for (t = 0; t < (m_Pegs + m_Groups); t++,spp++,dpp++)
		{
		dpp->group = SWAPV(spp->group);
		dpp->pegx = SWAPDBL(spp->pegx);
		dpp->pegy = SWAPDBL(spp->pegy);
		dpp->levelx = SWAPDBL(spp->levelx);
		dpp->levely = SWAPDBL(spp->levely);
		dpp->ppeg = SWAPV(spp->ppeg);
		strcpy(dpp->name,spp->name);
		}
#else
	memmove(dpp,spp,tsize);
#endif
	tp += tsize;
	tsize = sizeof(ATTRLIST) * m_limit;
	if (bRead)
		m_pList = new BYTE [tsize];
	ATTRLIST * dp;
	ATTRLIST * sp;
	if (bRead)
		{
		dp = (ATTRLIST *)m_pList; 
		sp = (ATTRLIST *)tp; 
		}
	else
		{
		sp = (ATTRLIST *)m_pList; 
		dp = (ATTRLIST *)tp; 
		}
#ifdef FBMAC
	for (tsize = 0; tsize < m_count; tsize++,sp++,dp++)
		{
		dp->attr = SWAPV(sp->attr);
		dp->frame = SWAPV(sp->frame);
		dp->peg = SWAPV(sp->peg);
		dp->value = SWAPDBL(sp->value);
		dp->kind = SWAPV(sp->kind);
		}
#else
	tsize = sizeof(ATTRLIST) * m_count;
	memmove(dp,sp, tsize);
#endif
//	for (tsize = 0; tsize < m_count; tsize++,sp++,dp++)
//		{
//DPF("i:%3d,peg:%d,atr:%d,frm:%3d,v:%d",tsize,dp->peg,dp->attr,dp->frame,(int)(1000*dp->value));
//		}
}

int CCamera::Read()
{
	CAMERAHDR hdr;
DPF("camera read,%d,%x",m_Key,m_Key);
#ifdef FBMAC
	int result = m_pIO->GetSwapRecord(&hdr, sizeof(hdr), m_Key);
#else
	int result = m_pIO->GetRecord(&hdr, sizeof(hdr), m_Key);
#endif
DPF("read:%d", result);
	if (result) return result;
	ASSERT(hdr.dwKey == m_Key);
	ASSERT(hdr.dwKind == KIND_CAMERA);
	ASSERT(hdr.dwFlags == 0);
	ASSERT(hdr.Groups == m_Groups);
	ASSERT(hdr.Pegs == m_Pegs);
//	ASSERT(hdr.Frames == m_Frames);
//	ASSERT(hdr.Levels == m_Levels);
	m_count = hdr.count;
	m_limit = hdr.limit;
	delete [] m_pPegs;
	delete [] m_pList;
	delete [] m_pData;
	m_pData = 0;
	UINT size = sizeof(PEGLIST) * (m_Pegs + m_Groups);
	size += sizeof(ATTRLIST) * m_count;
	BYTE * tp = new BYTE[size];
ASSERT(tp != 0);
	if (!tp)
		return 1;
	result = m_pIO->GetRecord(tp, size, m_Key, sizeof(hdr));
DPF("read:%d", result);
	if (result) return result;
	Mover(TRUE,tp);
	delete [] tp;
	m_bModified = FALSE;
DPF("cam,m_count:%d",m_count);
	return 0;
}

int CCamera::Write()
{
	CAMERAHDR hdr;
DPF("camera write");
	hdr.dwKey = m_Key;
	hdr.dwKind = KIND_CAMERA;
	hdr.dwFlags = 0;
	hdr.Groups = m_Groups;
	hdr.Pegs = m_Pegs;
	hdr.Frames = m_Frames;
	hdr.Levels = m_Levels;
	hdr.count = m_count;
	hdr.limit = m_limit;
	UINT size = sizeof(hdr);
	size += sizeof(PEGLIST) * (m_Pegs + m_Groups);
	size += sizeof(ATTRLIST) * m_count;
	BYTE * tp = new BYTE[size];
	if (!tp)
		return 1;
	BYTE * p = tp;
	UINT tsize = sizeof(hdr);
	memmove(tp, &hdr, tsize);
#ifdef FBMAC
	DOSWAPC(p,tsize);
#endif
	p += tsize;
	Mover(FALSE,p);
	int result = m_pIO->PutRecord(tp, size, m_Key);
DPF("put cam rec:%d",result);
	delete [] tp;
	m_bModified = FALSE;
	return result;
}


bool CCamera::GroupActive(UINT which)
{
	UINT i;
	which += 1000;
	ATTRLIST * p = (ATTRLIST * )m_pList;
	for (i = 0; i < m_count;i++)
		if (p[i].peg == which)
			break;
	return i < m_count ? TRUE : FALSE;
}

UINT CCamera::GroupFindPeg(UINT Peg)	// returns group with peg
{
	PEGLIST * pPegs = (PEGLIST *)m_pPegs;
	if (!pPegs[m_Groups + Peg].group)
		pPegs[m_Groups + Peg].group++;
	return pPegs[m_Groups + Peg].group;
}


UINT CCamera::GroupAttach(UINT Peg, UINT Group)
{
	PEGLIST * pPegs = (PEGLIST *)m_pPegs;
	if (pPegs[m_Groups + Peg].group != Group)
		{
		pPegs[m_Groups + Peg].group = Group;
		MakeModified();
		}
	return 0;
}

bool CCamera::GroupName(LPSTR Name, UINT Which, bool bPut/*= 0*/)
{
	if (Which >= m_Groups)
		return 1;
	PEGLIST * pPegs = (PEGLIST *)m_pPegs;
	LPSTR p = (LPSTR)&pPegs[Which].name;
	if (bPut)
		{
		strcpy(p, Name);
		MakeModified();
		}
	else
		{
		if (!p[0])
			sprintf(Name,"Group %03d", Which);
		else
			strcpy(Name,p);
		}
	return 0;
}


bool CCamera::PegActive(UINT which)
{
	UINT i;
	ATTRLIST * p = (ATTRLIST * )m_pList;
	for (i = 0; i < m_count;i++)
		if (p[i].peg == which)
			break;
	return i < m_count ? TRUE : FALSE;
}


UINT CCamera::PegFindLevel(UINT Level)
{
	if (Level >= m_Levels)		// major kludge, using groups and pegs
		return NEGONE;				// for level info
	PEGLIST * pPegs = (PEGLIST *)m_pPegs;
	return pPegs[Level].ppeg & 0xffff;
}

bool CCamera::IsKey(UINT Frame, UINT Level)
{
	if ((Level >= m_Levels) || !m_pPegs)
		return FALSE;
	PEGLIST * pPegs = (PEGLIST *)m_pPegs;
	UINT Peg = pPegs[Level].ppeg & 0xffff;
	UINT i;
	ATTRLIST * p = (ATTRLIST * )m_pList;
	for (i = 0; i < m_count;i++)
		if ((p[i].frame == Frame) && (p[i].peg == Peg))
			break;
	return i < m_count ? TRUE : FALSE;
}


UINT CCamera::PegFlags(UINT Peg, UINT which, UINT Value /* = NEGONE */)
{
	if (Peg >= m_Pegs)
		return NEGONE;
	PEGLIST * pPegs = (PEGLIST *)m_pPegs;
	if (Value != NEGONE)
		{
		Value = Value << 16;
		Value = Value + (pPegs[Peg].ppeg & 0xffff);
		pPegs[Peg].ppeg = Value;
		}
	return pPegs[Peg].ppeg >> 16;
}

UINT CCamera::PegAttach(UINT Level, UINT Peg)
{
	if ((Level >= m_Levels) || (Level >= m_Pegs) || (Peg >= m_Pegs))
		return 1;
	PEGLIST * pPegs = (PEGLIST *)m_pPegs;
	if ((pPegs[Level].ppeg & 0xffff) != Peg)
		{
		pPegs[Level].ppeg &= 0xffff0000;
		pPegs[Level].ppeg |= Peg;
		MakeModified();
		}
	return 0;
}

UINT CCamera::PegCenter(double& cx, double& cy, UINT Peg, bool bPut/*=FALSE*/)
{
	if (Peg >= 1000)
		Peg -= 1000;
	else
		Peg += m_Groups;
//	if (which >= m_Pegs)
//		return 1;
	PEGLIST * pPegs = (PEGLIST *)m_pPegs;
	pPegs += Peg;
	if (bPut)
		{
		if ((pPegs->pegx != cx) || (pPegs->pegy != cy))
			{
			pPegs->pegx = cx;
			pPegs->pegy = cy;
			Compute();
			MakeModified();
			}
		}
	else
		{
		cx = pPegs->pegx;
		cy = pPegs->pegy;
		}
	return 0;
}

UINT CCamera::LevelCenter(double& cx, double& cy, UINT which, bool bPut/*=FALSE*/)
{
	if (which >= m_Pegs)
		return 1;
	PEGLIST * pPegs = (PEGLIST *)m_pPegs;
	pPegs += m_Groups + which;
	LPSTR p = (LPSTR)&pPegs[m_Groups + which].name;
	if (bPut)
		{
		if ((pPegs->levelx != cx) || ( pPegs->levely != cy))
			{
			pPegs->levelx = cx;
			pPegs->levely = cy;
			MakeModified();
			}
		}
	else
		{
		cx = pPegs->levelx;
		cy = pPegs->levely;
		}
	return 0;
}

UINT CCamera::PegName(LPSTR Name, UINT Which, bool bPut/*= 0*/)
{
	if (Which >= m_Pegs)
		return 1;
	PEGLIST * pPegs = (PEGLIST *)m_pPegs;
	LPSTR p = (LPSTR)&pPegs[m_Groups + Which].name;
	if (bPut)
		{
		strcpy(p, Name);
		MakeModified();
		}
	else
		{
		if (!p[0])
			sprintf(Name,"Peg %03d", Which);
		else
			strcpy(Name,p);
		}
	return 0;
}

void CCamera::Update()
{
	if (!m_pScene)
		return;
	if (m_Frames != m_pScene->FrameCount())
		{
		m_Frames = m_pScene->FrameCount();
		delete [] m_pData;
		m_pData = 0;
		}
	if (m_Levels != m_pScene->LevelCount())
		{
		UINT i = m_Levels;
		m_Levels = m_pScene->LevelCount();
		for (; i < m_Levels;i++)
			PegAttach(i,i);
		}
}

void CCamera::InsertLevel(UINT start, UINT count)
{
	if (!m_pScene)
		return;
	if ((start >= m_Levels) || (start >= m_Pegs))
		return;
	m_Levels = m_pScene->LevelCount();
	UINT i;
	for (i = m_Levels; i-- > (start+count);) 
		{
		PEGLIST * pPegs = (PEGLIST *)m_pPegs;
		pPegs[i] = pPegs[i - count];
		}
	MakeModified();
}

void CCamera::Setup(CSceneBase * pScene)
{
	m_pScene = pScene;
	m_Groups = NBR_GROUPS;
	delete [] m_pData;
	m_pData = 0;
	m_Frames = m_pScene->FrameCount();
	m_Levels = pScene->LevelCount();
	m_Pegs = MAX_PEGS;	// for now
	m_bModified = FALSE;
	double z = 0;
	UINT i;
	m_bDeferWrite = TRUE;
	for (i = CATTR_Y; i < CATTR_EXTRA; m_dirty[i++] = TRUE);
	for (i = 0; i < m_Groups; i++)
		{
		char buf[40];
		if (i > 1)
			sprintf(buf,"Group %d", i-1);
		else if (i)
			sprintf(buf,"Table Top");
		else
			sprintf(buf,"Camera");
		GroupName(buf,i,TRUE);
		PegCenter(z,z,i+1000,TRUE);
		}
	for (i = 0; i < m_Pegs; i++)
		{
		char buf[40];
		sprintf(buf,"Peg %d", i);
		PegName(buf,i,TRUE);
		PegCenter(z,z,i,TRUE);
		LevelCenter(z,z,i,TRUE);
		GroupAttach(i,1);
		}
	for (i = 0; i < m_Levels; i++)
		{
		PegAttach(i,i);
		}
	m_bDeferWrite = FALSE;
//	if (m_bModified)
//		MakeModified();

}

UINT CCamera::GetAttr(UINT Attr,UINT Frame,UINT Peg,
			double& Value,UINT & Kind)
{
	if (m_dirty[Attr] || !m_pData)
		Compute(Attr);
	UINT i;
	ATTRLIST * p = (ATTRLIST * )m_pList;
	for (i = 0; i < m_count;i++)
		if ((p[i].attr == Attr) && (p[i].frame == Frame) && (p[i].peg == Peg))
			break;
	if (i < m_count)
		Kind = p[i].kind;
	else
		Kind = CCamAttr::CKIND_COMPUTE;
	if (Peg >= 1000)
		Peg -= 1000;
	else
		Peg += m_Groups;
	UINT offset = Frame + m_Frames*Peg + m_Frames * (m_Pegs + m_Groups) * Attr; 
	Value = m_pData[offset];
	return 0;
}

double CCamera::GetValue(UINT Attr,UINT Frame,UINT Peg)
{
	if (m_dirty[Attr] || !m_pData)
		Compute(Attr);
	if (Peg >= 1000)
		Peg -= 1000;
	else
		Peg += m_Groups;
	UINT offset = Frame + m_Frames*Peg + m_Frames * (m_Pegs + m_Groups) * Attr; 
	return m_pData[offset];
}


UINT CCamera::PutAttr(UINT Attr,UINT Frame,UINT Peg, double Value,UINT  Kind)
{
	UINT i;
	ATTRLIST * p = (ATTRLIST * )m_pList;
	if (Attr == 99)
		{
		for (i = CATTR_Y; i < CATTR_EXTRA; m_dirty[i++] = TRUE);
		m_count = 0;
		MakeModified();
		return 0;
		}
	if (Attr >= 90)  // clear all with this attr
		{
		Attr -= 90;
		for (i = 0; i < m_count;i++)	// fnd the first one
			if (p[i].attr == Attr && p[i].peg == Peg)
				break;
		if (i >= m_count)		// none found
			return 0;
		UINT j = i++;			// step over it
		for (; i < m_count;i++)
			{
			if ((p[i].attr != Attr) || (p[i].peg != Peg))
				p[i] = p[j++];	// copy all others
			}
		m_count = j;		
		m_dirty[Attr] = TRUE;
		MakeModified();
		return 0;
		}
	for (i = 0; i < m_count;i++)
		if (p[i].attr == Attr && p[i].frame == Frame && p[i].peg == Peg)
			break;
	if (i < m_count) // found
		{
		if (Kind == CCamAttr::CKIND_COMPUTE)	// removing it
			{
			m_count--;
			for (;i < m_count;i++)
				p[i] = p[i+1]; 
			m_dirty[Attr] = TRUE;
			MakeModified();
			}
		if ((Kind != p[i].kind) || (Value != p[i].value))
			{
			p[i].kind = Kind;
			p[i].value = Value;
			m_dirty[Attr] = TRUE;
			MakeModified();
			}
		return 0;
		}
	else
		{
		if (Kind == CCamAttr::CKIND_COMPUTE)
			return 1;				// compute should not be in list
		if (m_count >= m_limit)
			{
			m_limit += 50;
			BYTE * bp = new BYTE[m_limit * sizeof(ATTRLIST)];
			ATTRLIST * pp = (ATTRLIST * )bp;
			UINT i;
			for (i = 0;i < m_count;i++)
				pp[i] = p[i];
			delete [] m_pList;
			m_pList = (BYTE *)pp;
			p = pp;
			pp = 0;
			}
		p[m_count].value = Value;
		p[m_count].attr = Attr;
		p[m_count].frame = Frame;
		p[m_count].peg = Peg;
		p[m_count].kind = Kind;
		m_count++;
		m_dirty[Attr] = TRUE;
		MakeModified();
		}
	return 0;
}

UINT CCamera::FindKey(UINT peg, UINT frame, bool bUp)
{
	if (!m_pData)
		return NEGONE;
	ATTRLIST * p = (ATTRLIST * )m_pList;
	UINT i, val;
	if (bUp)
		{
		val = m_Frames;
		for (i = 0; i < m_count;i++)
			if ((p[i].frame > frame) && (p[i].frame < val) && (peg==p[i].peg))
				{
				val = p[i].frame;
				if (val == (frame + 1))
					break;
				}
		if (val == m_Frames)
			val = NEGONE;
		}
	else
		{
		val = 0;
		for (i = 0; i < m_count;i++)
			if ((p[i].frame < frame) && ((p[i].frame+1) > val) && (peg==p[i].peg))
				{
				val = p[i].frame+1;
				if (val == frame)
					break;
				}
		if (val == 0)
			val = NEGONE;
		else
			val--;
		}
	return val;
}

void CCamera::Simple1(double * pt, int x1, double y1, int x2, double y2, double slope)
{
//DPF("simple1,%d,%d\n",x1,x2);
	double a = slope / (x2 - x1);
	int x;
	for (x = x1 + 1; x <= x2; x++)
		{
		double t = x - x1;
		*pt++ = y1 + (t * t * a) / 2.0;
//DPF("x:%d,v:%d",x,(int)(100 * pt[-1]));
		}
}

void CCamera::Simple2(double * pt, int x1, double y1, int x2, double y2, double slope)
{
//DPF("simple2,%d,%d\n",x1,x2);
	double a = slope / (x2 - x1);
	int x;
	for (x = x1 + 1; x <= x2; x++)
		{
		double t = x2 - x;
		*pt++ = y2 - (t * t * a) / 2.0;
//DPF("x:%d,v:%d",x,(int)(100 * pt[-1]));
		}
}


/*
	hermite, point slope
*/
static double her[4][4] = { 2,-2, 1, 1,
					-3, 3,-2,-1,
					 0, 0, 1, 0,
					 1, 0, 0, 0};
void CCamera::Curve(double * pt, int x1, double y1, int x2, double y2, 
					double slope1, double slope2)
{
//DPF("curve,%d,%d\n",x1,x2);
	double coef[4];
	double pp[4];
	pp[0] = y1;
	pp[1] = y2;
	pp[2] = slope1;
	pp[3] = slope2;
	int ii,jj;
	for (ii = 0; ii < 4; ii++)
		{
		for (coef[ii] = 0, jj = 0; jj < 4; jj++)
			coef[ii] += her[ii][jj] * pp[jj];
		}
	int x;
	for (x = x1+ 1; x <= x2; x++)
		{
		double t = ((double)x - x1) / (x2 - x1);
		double qq = 0;
		double tq = 1;
		for (ii = 0; ii < 4; ii++, tq *= t)
			qq += coef[3 - ii] * tq;
		*pt++ = qq;	
//DPF("x:%d,v:%d",x,(int)(100 * pt[-1]));
		}
}

void CCamera::Linear(double * pt,int x1, double y1, int x2, double y2, int easein, int easeout)
{
/*

given
frame1
frame2
easein at frame 1
easout at frame 2
value1 at frame 1
value2 at frame 2

theory is that a constant acceleration is employed 
for a period called easein to reach a speed
a constant deceleration for easeout will slow it down
the speed in between these points is constant
*/
	int	min = 1;
	if (easein)
		min++;
	if (easeout)
		min++;
	int count = x2 - x1;
	if (count < min)
		{
		easein = 0;
		easeout = 0;
		}
	int delta;

	if ((easein + easeout) > count)
		{
		easein = 0;
		easeout = 0;
		delta = count;
		}
	else
		delta = count - easein - easeout;
	double a;
	double slope;
	double d;
	double step = y1;

	d = 0;
	if (!easeout && !easein)
		{
		a = 0;
		slope = (y2 - y1 ) / delta;
		}
	else if (!easeout)
		{
		a = (2 * (y2 - y1)) / (easein * (easein + 2 * delta));
		slope = a * easein;
		step += (slope * easein) / 2;
		}
	else if (!easein)
		{
		d = (2 * (y2 - y1)) / (easeout * (easeout + 2 * delta));
		a = 0;
		slope = d * easeout;
		}
	else
		{
		a = (2 * (y2 - y1)) /
				(easein * (easein + easeout + 2 * delta));
		slope = a * easein; // also d * easeout
		d = slope / easeout;
		step += (a * easein * easein) / 2;
		}
	int f;
	for (f = 1; f <= count; f++)
		{
		double value = y1;

		if (f < easein)
			{
			value = y1 + (a * f * f) / 2;
			}
		else if (f > (easein + delta))
			{
			value = y2 - (d * (count - f) * (count - f)) / 2;
			}
		else
			{
			value = step + (f - easein) * slope;
			}
		*pt++ = value;
//DPF("x:%d,v:%d",f+x1,(int)(100 * pt[-1]));
		}
}

void CCamera::Compute(UINT attr /* = 9999 */)
{
	if (!m_pData)
		{
		m_pData = new double[ m_Frames * (m_Pegs + m_Groups) * CATTR_EXTRA]; 
		for (int i = CATTR_Y; i < CATTR_EXTRA; m_dirty[i++] = TRUE);
		}
	if (attr == 9999)
		{
		int i;
		for (i = CATTR_Y; i < CATTR_EXTRA; i++)
			if (m_dirty[i])
				Compute(i);
//				Compute(attr);
		return;
		}
	double * pSave;
	pSave = new double[m_Frames];
	ATTRLIST * p = (ATTRLIST * )m_pList;
	BYTE * bp = new BYTE[m_Frames * sizeof(TEMPLIST)];
	TEMPLIST * tp = (TEMPLIST * )bp; // temp for sorting frame entries
	UINT peg;
	UINT frame;
	UINT z;
	double minv, maxv, initialv;
	initialv = 0;
	switch (attr)
	{
	case CATTR_SCALE:
		minv = 10;
		maxv = 1000;
		initialv = 100;
		break;
	case CATTR_X:
	case CATTR_Y:
		minv = -150;
		maxv = 150;
		break;
	case CATTR_ROT:
		minv = -3600;
		maxv = 3600;
		break;
	case CATTR_BLUR:
		minv = 0;
		maxv = 20;
		break;
	case CATTR_ALPHA:
		minv = 0;
		maxv = 100;
		initialv = 100;
		break;
	default:
		minv = 0;
		maxv = 100;
	}
//	for (peg = 0; peg < m_Pegs; peg++)
	for (z = 0; z < (m_Groups + m_Pegs); z++)
		{
		if (z >= m_Groups)
			peg = z - m_Groups;
		else
			peg = z + 1000;
		UINT tcount = 0;
		UINT i,j;
		for (i = 0; i < m_count;i++)
			if (p[i].attr == attr && p[i].peg == peg)
				{
				frame = p[i].frame;
				if (frame >= m_Frames)
					continue;
				for (j = 0; j < tcount; j++)
					if (frame <= tp[j].frame)
						break;
				if (j < tcount)
					{			// inserting
					ASSERT(frame < tp[j].frame);
					UINT z;
					for (z = tcount; z > j; z--)
						tp[z] = tp[z-1];
					}
				tp[j].frame = p[i].frame;
				if (p[i].value < minv)
					p[i].value = minv;
				if (p[i].value > maxv)
					p[i].value = maxv;
				tp[j].value = p[i].value;
				tp[j].kind  = p[i].kind;
//DPF("i:%d,j:%d,f:%d,k:%d,v:%d",i,j,tp[j].frame,tp[j].kind,(int)100*tp[j].value);
				tcount++;
				}
/*
if (tcount)
{
DPF("attr:%d,peg:%d",attr,peg);
for (j = 0; j < tcount;j++)
{
DPF("j:%d,f:%d,k:%d,v:%d",j,tp[j].frame,tp[j].kind,(int)(100*tp[j].value));
}
}
*/
//
//	at this point tp has sorted list of all attr/pegs by frame
//
		if (!tcount)
			{
			tp[tcount].value = initialv;
			tp[tcount].kind = 0;
			tp[tcount].frame = m_Frames - 1;
			tcount++;
			}
		else if ((tp[tcount-1].frame != (m_Frames -1)))
			{
			tp[tcount].value = tp[tcount-1].value;
			tp[tcount].kind = 0;
			tp[tcount].frame = m_Frames - 1;
			tcount++;
			}
		if (tp[0].frame != 0)
			{
			UINT z;
			for (z = tcount++; z; z--)
				tp[z] = tp[z-1];
			tp[0].value = tp[1].value;
			tp[0].frame = 0;
			tp[0].kind  = 0;
			}
		
//		double v1 = initialv;
//		UINT frame = 0;
		UINT easein = 0;
		UINT easeout = 0;
		UINT kind;
		UINT index;
		UINT offset = m_Frames * z + m_Frames * (m_Pegs + m_Groups)* attr; 
		double * pt = m_pData;
		pt += offset;
		for (index = 0; index < m_Frames; index++)
			pSave[index] = pt[index];	// save attr vales
		*pt++ = tp[0].value;
		for (index = 0; (index + 1)  < tcount; index++)
		  {
		  kind = tp[index].kind & 2;
		  easein  = (tp[index].kind >> 2) & 32767;
		  easeout = (tp[index+1].kind >> 17) & 32767;
		  UINT x1 = tp[index].frame;
		  double y1 = tp[index].value;
		  UINT x2 = tp[index+1].frame;
		  double y2 = tp[index+1].value;
		  if (!index || !kind)
			{
			if (((index+1) < tcount) && (tp[index+1].kind & 2))
				{
				double slope = (y2 - y1) / (x2 - x1); 
				double zz = (slope * (double)easein) / 2.0;
				int mx = x1 + easein;
				double my = y1 + zz;
				if (easein)
					Simple1(pt,x1,y1,mx,my,slope);
				double slope2=(tp[index+2].value-y1)/(tp[index+2].frame-x1);
				Curve(pt+mx-x1,mx,my,x2,y2,slope,slope2);
				}
			else
				{
				Linear(pt,x1, y1, x2, y2, easein,easeout);
				}
		  }
		else
			{
			double slope1 = (y2-tp[index-1].value) /
						(x2-tp[index-1].frame);
			if (((index+2) >= tcount) || !(tp[index+1].kind & 2))
				{
				double slope2 = (y2-y1) / (x2 - x1);
				double zz = (slope2 * (double)easeout) / 2.0;
				int mx = x2 - easeout;
				double my = y2 - zz;
				Curve(pt,x1,y1,mx,my,slope1,slope2);
//				pt += mx - x1;
				if (easeout)
					Simple2(pt+mx-x1,mx,my,x2,y2,slope2);
				}
			else
				{
				double slope2 = (tp[index+2].value - y1) / 
							(tp[index+2].frame-x1);
				Curve(pt,x1,y1,x2,y2,slope1,slope2);
				}
			}
#define POWER 1.3
#ifdef POWER
			if (attr == CATTR_SCALE && (y2 != y1))
				{
				UINT i;
				for (i = x1 + 1; i < x2; i++)
					{
					double v = pt[i-x1-1];
					double f = (v - y1) / (y2 - y1);
					v = y1 + (y2 - y1) * exp(	POWER * log(f));
					pt[i-x1-1] = v;
					}
				}
#endif
/*
		if (attr == CATTR_SCALE && (y2 != y1))
			{
	DPF("x1:%d,x2:%d,y1:%d,y2:%d",x1,x2,(int)(1000*y1),(int)(1000*y2));
			int i;
			for (i = x1+1; i < x2; i++)
				DPF("%2d,%3d,%d",i-x1-1,x1,(int)(1000*pt[i-x1-1]));
			i = x1;
			}
*/
		pt += x2 - x1;
		}
	pt = m_pData + offset;
	for (index = 0; index < m_Frames; index++)
		if (pSave[index] != pt[index])
			{
			if (index < m_min_diff)
				m_min_diff = index;
			if (index >= m_max_diff)
				m_max_diff = index + 1;
			}
	}
	delete [] pSave;
	delete [] tp;
	m_dirty[attr] = FALSE;
}

void CCamera::MakeModified()
{
	m_bModified = TRUE;
	if (m_bDeferWrite)
		return;
	if (m_pScene)
		{
		m_pScene->MakeModified();
		Flush(TRUE);
		}
}

void CCamera::Flush(bool bForce /* = 0 */)
{
//	return;
	if (m_bModified || bForce)
		Write();
}
//
//	0 is no camera, 1 is simple, 2 is complex
//
UINT CCamera::SetupCell(UINT Frame, UINT Level)
{
	UINT result = 2;
	UINT Peg = PegFindLevel(Level);
	UINT group = GroupFindPeg(Peg);
	m_Frame = Frame;		// stash for table processor
	m_Level = Level;
	m_bFieldComp = PegFlags(Peg,1);

	m_cyoffset = 0;
	m_cxoffset = 0;
	m_cscale = 100;
	m_crotation = 0;
	m_cblur = 0;
	m_cdalpha = 100;

	m_gyoffset = 0;
	m_gxoffset = 0;
	m_gscale = 100;
	m_grotation = 0;
	m_gblur = 0;
	m_gdalpha = 100;
	m_gpegx = 0;
	m_gpegy = 0;

	m_yoffset = 0;
	m_xoffset = 0;
	m_scale = 100;
	m_rotation = 0;
	m_blur = 0;
	m_dalpha = 100;
	m_pegx = 0;
	m_pegy = 0;
	result = 1;
//	result = 0;
	m_bAttached = TRUE;
	m_cyoffset = GetValue(CCamera::CATTR_Y, Frame,1000);
	m_cxoffset = GetValue(CCamera::CATTR_X, Frame,1000);
	m_cscale = GetValue(CCamera::CATTR_SCALE, Frame,1000);
	m_crotation = GetValue(CCamera::CATTR_ROT, Frame,1000);
	m_cblur = GetValue(CCamera::CATTR_BLUR, Frame,1000);
	m_cdalpha = GetValue(CCamera::CATTR_ALPHA, Frame,1000);
	m_bGroup = TRUE;//FALSE;
	if (group && GroupActive(group))
//	if (GroupActive(group))
		{
		m_bGroup = TRUE;
		m_gyoffset = GetValue(CCamera::CATTR_Y, Frame,group+1000);
		m_gxoffset = GetValue(CCamera::CATTR_X, Frame,group+1000);
		m_gscale = GetValue(CCamera::CATTR_SCALE, Frame,group+1000);
		m_grotation = GetValue(CCamera::CATTR_ROT, Frame,group+1000);
		m_gblur = GetValue(CCamera::CATTR_BLUR, Frame,group+1000);
		m_gdalpha = GetValue(CCamera::CATTR_ALPHA, Frame,group+1000);
		PegCenter(m_gpegx, m_gpegy, group+1000);
		}
	if (PegActive(Peg))
		{
		m_yoffset = GetValue(CCamera::CATTR_Y, Frame,Peg);
		m_xoffset = GetValue(CCamera::CATTR_X, Frame,Peg);
		m_scale = GetValue(CCamera::CATTR_SCALE, Frame,Peg);
		m_rotation = GetValue(CCamera::CATTR_ROT, Frame,Peg);
		m_blur = GetValue(CCamera::CATTR_BLUR, Frame,Peg);
		m_dalpha = GetValue(CCamera::CATTR_ALPHA, Frame,Peg);
		PegCenter(m_pegx, m_pegy, Peg);
		}
	if (m_pScene->RedBox())
		{
		result = 2;
		m_cscale /= 2;
		m_cxoffset /= 2;
		m_cyoffset /= 2;
		}
//	m_cscale /= (double)m_pScene->Factor();
//	m_gscale /= (double)m_pScene->Factor();
	m_scale = (2 * m_scale) / (double)m_pScene->ZFactor();
	if (result && (m_scale == 100.0) && (m_rotation == 0) && (m_blur == 0)
		&& (m_gscale == 100.0) && (m_grotation == 0) && (m_gblur == 0)
		&& (m_cscale == 100.0) && (m_crotation == 0) && (m_cblur == 0))
		result = 1;
	if (result)
		{
		double alpha = m_cdalpha * m_gdalpha * m_dalpha;
		alpha /= 100 * 100;
		if (alpha < 0)
			m_alpha = 0;
		else if (alpha >= 100)
			m_alpha = 255;
		else
			m_alpha = (UINT)(2.55 * alpha);
		m_blur += m_cblur + m_gblur;
		}
	else
		m_alpha = 255;
	return result;
}


UINT CCamera::Table(UINT * pTable, UINT w, UINT h, UINT iw, UINT ih,
			bool bBroadcast /* = 0 */)
{
	UINT q = m_pScene->RedBox() ? 2 : 1;
	bool bFactor = 0;
	if (m_scale == (200.0 / (double)m_pScene->ZFactor()))
		bFactor = 1;
	UINT y0;
	UINT dy;
	UINT flag = 0;
	if (m_bFieldComp && ((m_Frame+1) < m_Frames ))
		{
		y0 = 0;
		dy = 2;
		flag = 1;
		bBroadcast = 1;
		}
	else
		{
		y0 = 0;
		dy = 1;
		}
	double cyoffset;
	double cxoffset;
	double cscale;
	double crotation;
	double cblur;
	double cdalpha;


	double gyoffset;
	double gxoffset;
	double gscale;
	double grotation;
	double gblur;
	double gdalpha;
	double gpegx;
	double gpegy;

	double yoffset;
	double xoffset;
	double scale;
	double rotation;
	double blur;
	double dalpha;
	double pegx;
	double pegy;
	for (;;)
	{
	double twopi = 8.0 * atan(1.0);
	double angle;
	double ccx = (double)((w * 12 - 12) / 24.0);
	double ccy = (double)((h * 12 - 12) / 24.0);

	angle = (m_rotation * twopi) / 360;
	double	ssin = sin(angle);
	double	ccos = cos(angle);
	double	cssin,cccos;
	double	gssin,gccos;
	double csx, csy;
	double cqx, cqy,cqqx,cqqy;
	double gsx, gsy;
	double gx, gy;
	double ggx, ggy;
	bool bSimple = TRUE;
//		bSimple = FALSE;
	if (bBroadcast)
		bSimple = FALSE;
	if (m_bAttached)
		{
		double angle = (m_crotation * twopi) / 360;
		cssin = sin(angle);
		cccos = cos(angle);
		csx = 100.0 / m_cscale;
		csy = csx;
		if ((m_crotation != 0) || (m_cscale != 100.0))
			bSimple = FALSE;	// no blur check, caues only if preview
		cqqx = (double)(((w) * (m_cxoffset)) / 24.0);
		cqqy = (double)(((h) * (m_cyoffset)) / 24.0);
		cqx = (double)(((w) * (0.0 - m_cxoffset)) / 24.0);
		cqy = (double)(((h) * (0.0 - m_cyoffset)) / 24.0);
		}
	if (m_bGroup)
		{
		angle = (m_grotation * twopi) / 360;
		gssin = sin(angle);
		gccos = cos(angle);
		gsx = 100.0 / m_gscale;
		gsy = gsx;
		gx = (double)(((w) * (m_gpegx - m_gxoffset)) / 24.0);
		gy = (double)(((h) * (m_gpegy - m_gyoffset)) / 24.0);
		ggx = (double)(((w) * (m_gpegx )) / 24.0);
		ggy = (double)(((h) * (m_gpegy )) / 24.0);
		if ((m_grotation != 0) || (m_gscale != 100.0))
			bSimple = FALSE;
		m_rotation += m_grotation;
		}
	double sx = 100.0 / m_scale;
	double sy = sx;
	double px = (double)(((w) * (m_pegx - m_xoffset)) / 24.0);
	double py = (double)(((h) * (m_pegy - m_yoffset)) / 24.0);
	double ppx = (double)(((w) * (m_pegx - 12 )) / 24.0);
	double ppy = (double)(((h) * (m_pegy - 12 )) / 24.0);
	ppx = (ppx * m_pScene->ZFactor()) / 2;
	ppy = (ppy * m_pScene->ZFactor()) / 2;
	if ((m_rotation != 0) || !bFactor || (m_blur != 0))
//	if ((m_rotation != 0) || (m_scale != 100.0) || (m_blur != 0))
//	if ((m_rotation != 0))
		bSimple = FALSE;

	m_factor = (UINT)((35. * (m_scale + 1)) / (1.0 + m_blur));
//	m_factor = (UINT)((35. * (11)) / (1.0 + m_blur));
	m_radius = (UINT)(50 * (1 + m_blur) * m_factor / m_scale);
//DPF("factor :%d, radius:%d",m_factor,m_radius);
	
//DPF("cx:%d,cy:%d",(int)cx,(int)cy);
//DPF("cdx:%d,cdy:%d",(int)cdx,(int)cdy);
//DPF("tx:%d,ty:%d",(int)tx,(int)ty);

	if (bSimple)
		{
		double x,y;
		//double xx,yy;
		int ox = 0;
		int oy = 0;
		y = ccy - (double)oy;
		y = (double)oy - ccy;
		x = (double)ox - ccx;
		x += cqx;
		y += cqy;
//		if (m_bAttached)
//			{
//			yy = y * csy;
//			xx = x * csx;
//			x = xx * cccos - yy * cssin;
//			y = xx * cssin + yy * cccos;
//			}
	//	x -= cqx;
	//	y -= cqy;
		if (m_bGroup)
			{
			x += gx;
			y += gy;
//			x *= gsx;
//			y *= gsy;
//			xx = x * gccos - y * gssin;
//			yy = x * gssin + y * gccos;
			x = x - ggx;
			y = y - ggy;
			}
		x += px;
		y += py;
		m_offx = (int)(10000 * x);
		m_offy = (int)(10000 * y);
		m_offx -= (int)(100.0 * m_scale * ppx);
		m_offy -= (int)(100.0 * m_scale * ppy);
		return 1;
		}

	UINT yyy,ox,oy;
	for (yyy = y0; yyy < h;yyy += dy)
		{
		oy = h - 1 - yyy;
		for (ox = 0; ox < w; ox++)
			{
			double x,y;
			double xx,yy;
			
			y = ccy - (double)oy;
			x = (double)ox - ccx;
			if (m_bAttached)
				{
				x += cqx;
				y += cqy;
				yy = y * csy;
				xx = x * csx;
				x = xx * cccos - yy * cssin;
				y = xx * cssin + yy * cccos;
//				x += cqx;
//				y += cqy;
				}
			if (m_bGroup)
				{
				x += gx;
				y += gy;
				x *= gsx;
				y *= gsy;
				xx = x * gccos - y * gssin;
				yy = x * gssin + y * gccos;
				x = xx - ggx;
				y = yy - ggy;
				}
			x += px;
			y += py;
			xx = x * sx;
			yy = y * sy;
			x = xx * ccos - yy * ssin;
			y = xx * ssin + yy * ccos;
			x -= ppx;
			y -= ppy;

			x *= (double)m_factor;
			y *= (double)m_factor;
			pTable[oy*w+ox] = (int)x;
			pTable[w*h+oy*w+ox] = (int)y;
			}
		}
	if (m_bFieldComp && (flag == 1))
		{
		y0 ^= 1;
		flag = 0;

		cyoffset = m_cyoffset;
		cxoffset = m_cxoffset;
		cscale = m_cscale;
		crotation = m_crotation;
		cblur = m_cblur;
		cdalpha = m_cdalpha;

		gyoffset = m_gyoffset;
		gxoffset = m_gxoffset;
		gscale = m_gscale;
		grotation = m_grotation;
		gblur = m_gblur;
		gdalpha = m_gdalpha;
		gpegx = m_gpegx;
		gpegy = m_gpegy;

		yoffset = m_yoffset;
		xoffset = m_xoffset;
		scale = m_scale;
		rotation = m_rotation;
		blur = m_blur;
		dalpha = m_dalpha;
		pegx = m_pegx;
		pegy = m_pegy;
		SetupCell(m_Frame+1, m_Level);
		m_cyoffset = (cyoffset + m_cyoffset) / 2;
		m_cxoffset = (cxoffset + m_cxoffset) / 2;
		m_cscale = (cscale + m_cscale) / 2;
		m_crotation = (crotation + m_crotation) / 2;
		m_cblur = (cblur + m_cblur) / 2;
		m_cdalpha = (cdalpha + m_cdalpha) / 2;

		m_gyoffset = (gyoffset + m_gyoffset) / 2;
		m_gxoffset = (gxoffset + m_gxoffset) / 2;
		m_gscale = (gscale + m_gscale) / 2;
		m_grotation = (grotation + m_grotation) / 2;
		m_gblur = (gblur + m_gblur) / 2;
		m_gdalpha = (gdalpha + m_gdalpha) / 2;
		m_gpegx = (gpegx + m_gpegx) / 2;
		m_gpegy = (gpegy + m_gpegy) / 2;

		m_yoffset = (yoffset + m_yoffset) / 2;
		m_xoffset = (xoffset + m_xoffset) / 2;
		m_scale = (scale + m_scale) / 2;
		m_rotation = (rotation + m_rotation) / 2;
		m_blur = (blur + m_blur) / 2;
		m_dalpha = (dalpha + m_dalpha) / 2;
		m_pegx = (pegx + m_pegx) / 2;
		m_pegy = (pegy + m_pegy) / 2;
		}
	else
		break;
	}
	return 2;
}

void CCamera::PegXY(int &pegx, int & pegy, UINT Peg, UINT Frame, 
						UINT ww, UINT hh)
{
	if (Peg >= 1000)
		Peg -= 1000;
	else
		Peg += m_Groups;
	double w = ww;
	double h = hh;
	double ccx = (w * 12.0 - 12.0) / 24.0;
	double ccy = (h * 12.0 - 12.0) / 24.0;
	double cyoffset = GetValue(CCamera::CATTR_Y, Frame,1000);
	double cxoffset = GetValue(CCamera::CATTR_X, Frame,1000);
	double cqx = (w * (0.0 - cxoffset)) / 24.0;
	double cqy = (h * (0.0 - cyoffset)) / 24.0;
	double twopi = 8.0 * atan(1.0);
	UINT group;
	double x = 0;
	double y = 0;
	if (Peg >= m_Groups)
		{
		group = GroupFindPeg(Peg - m_Groups);
		double gscale = GetValue(CCamera::CATTR_SCALE, Frame,group+1000);
		double grotation = GetValue(CCamera::CATTR_ROT, Frame,group+1000);
		double yoffset = GetValue(CCamera::CATTR_Y, Frame,Peg-m_Groups);
		double xoffset = GetValue(CCamera::CATTR_X, Frame,Peg-m_Groups);
		double ppegx, ppegy;
		PegCenter(ppegx, ppegy, Peg-m_Groups);
		double gangle = (grotation * twopi) / 360;
		double gssin = sin(gangle);
		double gccos = cos(gangle);
		double gsx = 100.0 / gscale;
		double gsy = gsx;
		double px = (double)(((w) * (ppegx - xoffset)) / 24.0);
		double py = (double)(((h) * (ppegy - yoffset)) / 24.0);
		double gpegx, gpegy;
		PegCenter(gpegx, gpegy, group+1000);
		double ggx = (w * gpegx ) / 24.0;
		double ggy = (h * gpegy ) / 24.0;
		double xx = ggx - px;
		double yy = ggy - py;
		x = xx * gccos  + yy * gssin;
		y = - xx * gssin + yy * gccos;
		x /= gsx;
		y /= gsy;
		}
	else
		{
		group = Peg;
		}
	if (Peg)
		{
		double cscale = GetValue(CCamera::CATTR_SCALE, Frame,1000);
		if (m_pScene->RedBox())
			cscale /= 2;
		double crotation = GetValue(CCamera::CATTR_ROT, Frame,1000);
		double angle = (crotation * twopi) / 360;
		double cssin = sin(angle);
		double cccos = cos(angle);
		double csx = 100.0 / cscale;
		double csy = csx;
		double gyoffset = GetValue(CCamera::CATTR_Y, Frame,group+1000);
		double gxoffset = GetValue(CCamera::CATTR_X, Frame,group+1000);
		double gpegx, gpegy;
		PegCenter(gpegx, gpegy, group+1000);
		double gx = (w * (gpegx - gxoffset)) / 24.0;
		double gy = (h * (gpegy - gyoffset)) / 24.0;
		double xx = x - gx;
		double yy = y - gy;
		x = xx * cccos  + yy * cssin;
		y = - xx * cssin + yy * cccos;
		x /= csx;
		y /= csy;
		}
	x -= cqx;
	y -= cqy;
	pegx = (int)(ccx + x);
	pegy = (int)(ccy - y); 
}

void CCamera::PegVXY(double &pegx, double &pegy,
			int vx, int vy, UINT Peg, UINT Frame, UINT ww, UINT hh)
{
	if (Peg >= 1000)
		Peg -= 1000;
	else
		Peg += m_Groups;
	double w = ww;
	double h = hh;
	double ccx = (w * 12.0 - 12.0) / 24.0;
	double ccy = (h * 12.0 - 12.0) / 24.0;
	double cyoffset = GetValue(CCamera::CATTR_Y, Frame,1000);
	double cxoffset = GetValue(CCamera::CATTR_X, Frame,1000);
	double cqx = (w * (0.0 - cxoffset)) / 24.0;
	double cqy = (h * (0.0 - cyoffset)) / 24.0;
	double twopi = 8.0 * atan(1.0);
	double csx, csy;
	UINT group;
	UINT ox,oy;
	oy = hh - 1 - vy;
	ox = vx;
	double x,y;
	double xx,yy;
		
	y = ccy - (double)oy;
	x = (double)ox - ccx;
	if (Peg >= m_Groups)
		{
		group = GroupFindPeg(Peg - m_Groups);
		}
	else
		group = Peg;
	if (!group)
		{
		pegx = (24.0 * x) / w;
		pegy = (24.0 * y) / h;
		return;
		}
	if (m_bAttached)
		{
		x += cqx;
		y += cqy;
		double cscale = GetValue(CCamera::CATTR_SCALE, Frame,1000);
		if (m_pScene->RedBox())
			cscale /= 2;
		double crotation = GetValue(CCamera::CATTR_ROT, Frame,1000);
		double angle = (crotation * twopi) / 360;
		double cssin = sin(angle);
		double cccos = cos(angle);
		csx = 100.0 / cscale;
		csy = csx;
		yy = y * csy;
		xx = x * csx;
		x = xx * cccos - yy * cssin;
		y = xx * cssin + yy * cccos;
		}

	double gyoffset = GetValue(CCamera::CATTR_Y, Frame,group+1000);
	double gxoffset = GetValue(CCamera::CATTR_X, Frame,group+1000);
	if (Peg < m_Groups)
		{
		pegx = gxoffset - (24.0 * x) / w;
		pegy = (24.0 * y) / h + gyoffset;
		return;
		}
	double gscale = GetValue(CCamera::CATTR_SCALE, Frame,group+1000);
	double grotation = GetValue(CCamera::CATTR_ROT, Frame,group+1000);
	double angle = (grotation * twopi) / 360;
	double gssin = sin(angle);
	double gccos = cos(angle);
	double gsx = 100.0 / gscale;
	double gsy = gsx;
	double gpegx, gpegy;
	PegCenter(gpegx, gpegy, group+1000);
	double gx = (double)(((w) * (gpegx - gxoffset)) / 24.0);
	double gy = (double)(((h) * (gpegy - gyoffset)) / 24.0);
	double ggx = (double)(((w) * (gpegx )) / 24.0);
	double ggy = (double)(((h) * (gpegy )) / 24.0);

	x += gx;
	y -= gy;
	x *= gsx;
	y *= gsy;
	xx = x * gccos - y * gssin;
	yy = x * gssin + y * gccos;

	x = xx - ggx;
	y = yy + ggy;

	double yoffset = GetValue(CCamera::CATTR_Y, Frame,Peg-m_Groups);
	double xoffset = GetValue(CCamera::CATTR_X, Frame,Peg-m_Groups);
	double ppegx, ppegy;
	PegCenter(ppegx, ppegy, Peg-m_Groups);

	pegx = xoffset - (24.0 * x) / w;
	pegy = (24.0 * y) / h + yoffset;
}

CCamMoves::CCamMoves()
{
	m_pFile = 0;
	m_pPegs = 0;
	m_pItems = 0;
}

CCamMoves::~CCamMoves()
{
	delete m_pFile;
	if (m_pPegs)
		{
		MOVLIST * pList = (MOVLIST *)m_pPegs;
		UINT i;
		UINT total = NBR_GROUPS + MAX_PEGS;
		for (i = 0; i < total;i++)
			{
			if (pList[i].pAttr)
				delete [] pList[i].pAttr;
			}
		delete [] m_pPegs;
		}
	delete [] m_pItems;
}

int CCamMoves::WriteLine(LPCSTR txt, bool bComment)
{
	char buf[80];
	if (bComment)
		strcpy(buf, "#   ");
	else
		buf[0] = 0;
	strcat(buf, txt);
	strcat(buf,"\n");
	m_pFile->WriteString(buf);
	return 0;
}

static char attrlets[] = "YXZRBAQ";
int CCamMoves::WritePeg(UINT gpeg, UINT StartFrame, UINT EndFrame, UINT flags)
{
	char pg;
	UINT peg, grp;
	char name[50];
	if (gpeg >= 1000)
		{
		peg = gpeg - 1000;
		pg = 'G';
		grp = 0;
		m_pCamera->GroupName(name,peg);
		}
	else
		{
		peg = gpeg;
		pg = 'P';
		grp = m_pCamera->GroupFindPeg(peg);
		m_pCamera->PegName(name,peg);
		}
	WriteLine("",1);
	CString str;
//	if (gpeg > 1000)
//		str.Format("Group %d", peg);
//	else
//		str.Format("Peg %d", peg);
//	WriteLine(str,1);
	double cx, cy;
	m_pCamera->PegCenter(cx, cy, gpeg);

	str.Format("%c%02d %03d %d %8.3f %8.3f %s",
				pg,peg,0,grp,cx,cy,name);
	WriteLine(str);

	WriteLine("",1);

	UINT attr, mask;
	UINT frame;
	for (frame = StartFrame; frame <= EndFrame; frame++)
	{
		for (attr = CCamera::CATTR_Y, mask = 1; attr < CCamera::CATTR_EXTRA; attr++, mask += mask)
		{
		if (!(flags & mask))
			continue;
		double v;
		UINT kind;
		UINT z = m_pCamera->GetAttr(attr, frame,gpeg,v,kind);
		if (!kind && !(flags & 64))
			continue;
		str.Format("%c%02d %03d %c %8.3f %d",
				pg,peg,frame+1,attrlets[attr],v,kind);
		WriteLine(str);
		}
	}
	return 0;
}

int CCamMoves::Write(CCamera * pCamera, LPCSTR name, LPCSTR SceneName,
				CSceneBase * pScene , UINT flags,
			UINT StartFrame, UINT StartLevel, UINT EndFrame, UINT EndLevel)
{
	m_pCamera = pCamera;
	int nGroups = 0;
	int nPegs = 0;
	UINT groups[10];
	UINT pegs[100];
	UINT level;
	groups[nGroups++] = 0; // always have camera
	for (level = StartLevel; level <= EndLevel; level++)
		{
		if ((flags & 256) && !(pScene->LevelFlags(level) & 1))
			continue;
		UINT peg = pCamera->PegFindLevel(level);
		pegs[nPegs] = peg;
		int i;
		for (i = 0;pegs[i] != peg;i++);
		if (i >= nPegs)	// new one
			{
			nPegs++;
			UINT grp = pCamera->GroupFindPeg(peg);
			groups[nGroups] = grp;
			int i;
			for (i = 0; groups[i] != grp;i++);
			if (i >= nGroups)	// new one
				nGroups++;
			}
		}
	if (!nPegs)
		return 2;	// nothing to do
	m_pFile = new CStdioFile;
	DWORD mode = CFile::modeCreate | CFile::modeWrite | CFile::typeText;
	if (!m_pFile->Open(name, mode))
		return 1;
	WriteLine("",1);
	WriteLine("Digicel Camera Moves",1);
	WriteLine("",1);
	CString scene, str;
	str.Format("Scene : %s",SceneName);
	WriteLine(str,1);
    CTime t = CTime::GetCurrentTime();
	str.Format("Time  : %s", (const char*)t.Format("%B %d, %Y %I:%M%p"));
	WriteLine(str,1);
	WriteLine("",1);
	int i;
	if ((flags & 128) && nGroups)
		{
//		WriteLine("Groups",1);
		for (i = 0; i < nGroups; i++)
			WritePeg(1000 + groups[i], StartFrame, EndFrame,flags);
		}
//	WriteLine("Pegs",1);
	for (i = 0; i < nPegs; i++)
		WritePeg(pegs[i], StartFrame, EndFrame,flags);
	m_pFile->Close();
	return 0;
}

int CCamMoves::Initialize()
{
	UINT total = NBR_GROUPS + MAX_PEGS;
	m_pPegs = new BYTE[total * sizeof(MOVLIST)];
	MOVLIST * pList = (MOVLIST *)m_pPegs;
	UINT i;
	for (i = 0; i < total;i++)
		{
		pList[i].count = -1;
		pList[i].pAttr = 0;
		}
	m_pItems = new UINT[total];
	return 0;
}


int CCamMoves::Read(CCamera * pCamera, LPCSTR name)
{
	m_pCamera = pCamera;
	m_nPegs = 0;
	m_pFile = new CStdioFile;
	DWORD mode = CFile::modeRead | CFile::typeText;
	if (!m_pFile->Open(name, mode))
		return 1;
	if (Initialize())
		{
		m_pFile->Close();
		return 7;
		}
	int result = 0;
	for (;;)
		{
		char buf[100];
		int cc;
		if (!m_pFile->ReadString(buf,99))
			break;
		cc = strlen(buf);
		if (cc < 10)
			continue;
		if (buf[cc-1] < ' ')
			buf[cc-1] = 0;
DPF("buf:%s|",buf);
		if (buf[0] == '#')
			continue;
		char pg,at;
		UINT peg;
		UINT frame;
		UINT grp;
		UINT kind;
		double v1, v2;
		char name[100];
		int q = sscanf(buf,"%c%02d %03d %c %lf %d",
				&pg,&peg,&frame,&at,&v1,&kind);
		if ((q != 6) || !frame)
			{
	//		q = sscanf(buf,"%c%02d %03d %lf %lf %s",
			q = sscanf(buf,"%c%02d %03d %d %lf %lf %[^\0]",
				&pg,&peg,&frame,&grp,&v1,&v2,&name);
			if (!frame && (q == 7))
				q = 6;
			else
				q = 9;
			}
		else if (!frame)
			q = 8;
		if (q != 6)
			{
DPF("bad q:%d,%s|",q,buf);
			break;
			}
		if (pg == 'P')
			{
			if (peg >= MAX_PEGS)
				result = 4;
			else
				peg += NBR_GROUPS;
			}
		else if (pg == 'G')
			{
			if (peg >= NBR_GROUPS)
				result = 5;
			else if (grp)
				result = 9;
			}
		else
			result = 3;
		UINT attr;
		for (attr = 5; attr > 0 && attrlets[attr] != at; attr--); 
		if (result)
			break;
		if (!frame)
			result = AddPeg(peg,grp,v1,v2,name);
		else
			result = AddAttr(peg,frame,attr,v1,kind);
		if (result)
			break;
		}
	m_pFile->Close();
	return 0;
}

bool CCamMoves::IsPeg(UINT index)
{
	if (index >= m_nPegs)
		return FALSE;
	if (m_pItems[index] >= NBR_GROUPS)
		return TRUE;
	else
		return FALSE;
}

bool CCamMoves::HasGroup(UINT index)
{
	ASSERT(index < m_nPegs);
	UINT i;
	UINT peg = m_pItems[index];
	ASSERT(peg >= NBR_GROUPS);
	UINT group = m_pCamera->GroupFindPeg(peg - NBR_GROUPS);
	ASSERT(group < NBR_GROUPS);

	for (i = 0; i < m_nPegs; i++)
		if (m_pItems[i] == group)
			break;
	if (i >= m_nPegs)
		return FALSE;
	return TRUE;
}

void CCamMoves::Name(UINT index, CString & idd, CString & name)
{
	if (index >= m_nPegs)
		{
		name.Format("bad:%d",index);
		}
	else
		{
		MOVLIST * pList = (MOVLIST *)m_pPegs;
		UINT peg = m_pItems[index];
		if (peg >= NBR_GROUPS)
			idd.Format("P%02d",peg-NBR_GROUPS);
		else
			idd.Format("G%02d",peg);
		name = pList[peg].pl.name;
		}
}

int CCamMoves::Apply(UINT index, UINT flags, UINT FrameOffset, UINT StartLevel)
{
	int result = 0;
	if (index >= m_nPegs)
		{
		UINT i;
		for (i = 0; i < m_nPegs; i++)
			{
			if (result = Apply(i, flags, 0,999))
				break;
			}
		return result;
		}
//	m_pCamera->ClearPeg(mask, startframe, endfra,e)
	UINT peg = m_pItems[index];
	MOVLIST * pList = (MOVLIST *)m_pPegs;
	pList += peg;

	UINT actpeg;
	if (StartLevel > 99)
		{
		if (peg >= NBR_GROUPS)
			actpeg = peg - NBR_GROUPS;
		else
			actpeg = peg + 1000;
		}
	else
		{
		if (peg >= NBR_GROUPS)
			{
			actpeg = m_pCamera->PegFindLevel(StartLevel);
			if (flags & 128)
				{
				UINT group = m_pCamera->GroupFindPeg(actpeg);
				UINT i;
				for (i = 0; i < m_nPegs; i++)
					{
					if (m_pItems[i] == group)
						break;
					}
				if (i < m_nPegs)
					{
					if (result = Apply(i, flags,FrameOffset,StartLevel))
						return result;
					}
				}
			}
		else
			actpeg = peg + 1000;
		}

//	if (flags & 1024)
		m_pCamera->PegCenter(pList->pl.pegx, pList->pl.pegy,actpeg,TRUE);

//	if (flags & 512)
		{
		if (actpeg >= 1000)
			m_pCamera->GroupName(pList->pl.name,actpeg-1000,TRUE);
		else
			m_pCamera->PegName(pList->pl.name,actpeg,TRUE);
		}


	UINT count = pList->count;
	if (!count)
		return 0;
	ATTRLIST * pAttr = (ATTRLIST *)pList->pAttr;
	UINT i;
	UINT firsts = 0;
	for (i = 0; i < count; i++, pAttr++)
		{
		ASSERT(peg == pAttr->peg);	
		UINT mask = 1 << pAttr->attr;
		if (!(mask & flags))
			continue;
		UINT kind = pAttr->kind;
		if (!(kind & 1))		// not computed
			{
			if (!(flags & 64))	// but ...
				continue;
			kind = 1;		// make it a key
			}
		if (!(firsts & mask))
			{
			firsts |= mask;
			m_pCamera->PutAttr(90+pAttr->attr, 0, actpeg,0,0); //clear attr
			}			
		m_pCamera->PutAttr(pAttr->attr, pAttr->frame +FrameOffset - 1,
					actpeg, pAttr->value, kind);
		}
	return 0;
}

#define ATTR_GROW 50

int CCamMoves::AddPeg(UINT peg, UINT grp, double pcx,double pcy, LPCSTR name)
{
	MOVLIST * pList = (MOVLIST *)m_pPegs;
	if (pList[peg].count != -1)
		{
		DPF("hdr,non empty peg:%d",peg);
		return 13;
		}
	pList[peg].count = 0;
	pList[peg].pl.group = grp;
	pList[peg].pl.pegx = pcx;
	pList[peg].pl.pegy = pcy;
//	UINT 	ppeg;			// used for level list
	strcpy(pList[peg].pl.name, name);
	ASSERT(pList[peg].pAttr == 0);
//	pList[peg].pAttr = new BYTE[ATTR_GROW * sizeof(ATTRLIST)];
	m_pItems[m_nPegs++] = peg;
	return 0;
}

int CCamMoves::AddAttr(UINT peg,UINT frame,UINT at,double v, UINT kind)
{
	if (peg >= (MAX_PEGS + NBR_GROUPS))
		return 4;
	MOVLIST * pList = (MOVLIST *)m_pPegs;
	UINT Count = pList[peg].count;
	if (Count == -1)
		{
		DPF("peg not initi:%d",peg);
		return 8;
		}
	if (!(Count % ATTR_GROW))
		{
		BYTE * old = pList[peg].pAttr; 
		pList[peg].pAttr = new BYTE[(Count+ATTR_GROW) * sizeof(ATTRLIST)];
		if (!old)
			{
			ASSERT(pList[peg].count == 0);
			}
		else
			{
			memcpy(pList[peg].pAttr, old, Count * sizeof(ATTRLIST));
			delete [] old;
			}
		}
	ATTRLIST * pAttr = (ATTRLIST *)pList[peg].pAttr;
	pAttr += pList[peg].count++;
	pAttr->attr = at;
	pAttr->frame = frame;
	pAttr->peg = peg;
	pAttr->value = v;
	pAttr->kind = kind;
	return 0;
}
