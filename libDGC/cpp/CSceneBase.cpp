#include "CSceneBase.h"
#include "CLevels.h"
#include "CLevel.h"
#include "CCell.h"
#include "CMyImage.h"
#include "MyIO.h"
#include "NScaler.h"
#include "zlib.h"
#include "SceneOpt.h"
#include "Camera.h"
#include "CLayers.h"
#include "CLevtbl.h"
#include "CNewPals.h"
#include "MyJpeg.h"
#include "utils.h"
#include "Resource.h"
#include "CommonDefs.h"

//#define MAGIC_FACTOR_FIX

//#define FIXSCENES

//#define NOCOMPRESS
//#define CELLCACHESIZE 5
#define XTHUMB 40
//#define MAGIC_COLOR 254
//#define NEWTHUMBS
//#define DOBROADCAST
#define KEY_CLIP 13
#define KEY_SCENE 0x398
#define KEY_LEVELS 0x397
#define KEY_LINKS 0x396
#define KEY_OPTIONS 0x395
#define KEY_LEVELINFO 0x394 // for level widths in xsheet
#define KEY_PALETTE 0x393
#define KEY_TOOLS 0x392
#define KEY_AVI 0x391
#define KEY_CAMERA 0x390
#ifdef _DISNEY
#define KEY_DISPAL 0x389
#endif
#define KEY_PAL_BASE 0x300 // pals are 0 to 99, 0 is scene pal


void BlurX(BYTE * pDst, BYTE * pSrc, UINT w, UINT h, 
		UINT r, UINT f, UINT z, UINT pitch);

//#define MAKEWATERMARK
#define IMPRINT_ALPHA 140 // over 100
#define TGA
//#define THE_DISC
//#undef MYBUG
#define AUTONAME
#pragma pack(push,2)
typedef struct {
	DWORD   dwId;
	DWORD   dwKind;
	WORD	wWidth;
	WORD    wHeight;
	WORD	wScale;
	WORD	wFlags;
	DWORD	dwMyId;
	WORD	wFrameCount;
	WORD	wLevelCount;
} SCENEHEADER;

typedef struct {
	DWORD   dwId;
	WORD	wWidth;
	WORD    wHeight;
	WORD	wDepth;
	WORD	wFrameCount;
	DWORD	dwCode;
} CACHEHEADER;


typedef struct tagCEntry {
	DWORD   dwKey;
	UINT	iw;
	UINT	ih;
	UINT	id;
	UINT	level;
	UINT	palindex;
//	UINT	minx;
//	UINT	maxx;
//	UINT	miny;
//	UINT	maxy;
	BYTE *	pData;
} CELLENTRY;
class CCellEntry : public tagCEntry {};


 
#pragma pack(pop)
#ifndef mmioFOURCC
#define mmioFOURCC( ch0, ch1, ch2, ch3 ) \
	( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |	\
	( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )
#endif
#define DGCID	mmioFOURCC('D', 'G', 'C', 26)
#define DGQID	mmioFOURCC('D', 'G', 'Q', 26)

#ifdef _NEEDSWAP
void SwapHdr(BYTE * p)
{
	SwapEm(p, 8);
	p += 8;
	SwapEm(p, 2);
	p += 2;
	SwapEm(p, 2);
	p += 2;
	SwapEm(p, 2);
	p += 2;
	SwapEm(p, 2);
	p += 2;
	SwapEm(p, 4);
	p += 4;
	SwapEm(p, 2);
	p += 2;
	SwapEm(p, 2);
}
#endif

UINT TestSound(LPCSTR name)
{
	CFile file;
	DWORD mode = CFile::modeRead;
	if (!file.Open(name, mode))
		return 1;
	DWORD zz[3];
	UINT size = file.Read(&zz, 12);
	file.Close();
	if (size != 12)
		return 2;
	/*
		DWORD dwRiff = SWAPV(zz[0]);
		DWORD dwType = SWAPV(zz[2]);
		if( dwRiff != mmioFOURCC('R', 'I', 'F', 'F') )
			return 4;
		else if( dwType != mmioFOURCC('W', 'A', 'V', 'E') )
			return  5;      // not a WAV
	*/
	return 0;
}

//
//	Flag()
//	0 is thumb dirty (historical)
//	1 is color
//	2 is red box mode
//	3 is enlarge
//	4 is disable auto composite
//	5 is grid
//	6 is zoom fit
//	14 is 00 is always preview
//	15 is 01 is broadcast when publish, 10 is broadcast always

UINT CSceneBase::Flagger(UINT mask, UINT v)
{
	UINT q = mask ^ 0xffffffff;
	q = q + 1;		// clear bottom bits
	q = q & mask;	// just the one bit
	if (v < 2)
		{
		q = v * q;
		q = v & mask;
		m_oflags |= mask;
		m_oflags ^= mask;
		m_oflags |= q;
		}
	else
		v = (m_oflags & mask) / q;
	return v;
}


CCamera * CSceneBase::Camera()
{ return m_pCamera;}

CSceneBase::CSceneBase(CMyIO * pIO, std::unique_ptr<ResourceLoader> resourceLoader) : CMyObj(pIO)
{
DPF("scene construct");
	m_pEntry = 0;
	m_nEntries = 0;
//	memset(this,0,sizeof(this));// cant.cause m_pio
	m_origfactor = m_factor = 2;
//	m_scale = 1;//5;
	m_width = 640;
	m_height = 480;
	m_frames = 0;
	m_levels = 0;
	m_CurLevel = 1;
	m_CurFrame = 0;
	m_bOptLock = 0;
	m_flags = 0x4002;
	m_pLevels = 0;
	m_pLevelArray = 0;
//	m_nLevel = -1;
	m_pCamera = 0;
	m_pInfo = 0;
	m_pCache = 0;
	m_pCamBuf = 0;
	m_pJpeg = 0;
	m_pJEnc = 0;
//	m_pJEnc = new CMiniJpegEncoder(BJPEG);
	m_pJDec = 0;
//	m_pJDec = new CMiniJpegDecoder;
//	m_CamFrame = -1;
	m_CamPeg = 255;
	m_CamPegSel = -1;
	m_CamState = 0;
	m_pBG = 0;
//	m_nStack = 0;
//	m_pSaveLevel = 0;
	m_pCellCache = 0;
	m_pLayers = 0;
	m_pLinks = 0;
	m_links = 0;
	m_pImprint = 0;
	m_pLog = 0;
	m_pErrors = 0;
	m_pXY = 0;
	m_pFlags = 0;
	m_thumbw = 71;
	m_thumbh = 53;
	m_MinBG = 100;
	m_snip = 3;
	m_bBlind = 0;
	m_kind = KIND_SCENE;
	m_key = KEY_SCENE;
	m_info = 0;
	m_depth = 3;
	m_OptFlags = 0x10;	// starightahead
	m_xcellname[0] = 0;
	m_wave0[0] = m_wave1[0] = m_wave2[0] = 0;
	m_nBuddy0 = m_nBuddy1 = m_nBuddy2 = m_nBuddy3 = 0;
	m_mvolume = m_volume0 = m_volume1 = m_volume2 = 0;
	m_embed_out = m_embed_trim = m_embed_in = 0;
	m_jpeg_quality = 0;
	m_nLipx = m_nLipy = 0;
	m_nLipw = m_nLiph = 0;
	m_LipFile[0] = 0;
	m_bAppended = 0;
	UINT i;
	for (i = 0; i < 100; i++)
		{
		m_pPals[i] = 0;
		}
	m_pClipBoard = 0;
	m_pResourceLoader = std::move(resourceLoader);
}

CSceneBase::~CSceneBase()
{
DPF("scene destruct");
	UINT i;
	DeleteLevels();
	delete [] m_pInfo;
	m_pInfo = 0;
	if (m_pImprint)
		delete [] m_pImprint;
	if (m_pFlags)
		delete [] m_pFlags;
	if (m_pLinks)
		delete [] m_pLinks;
	if (m_pCache)
		{
		for (i = 0; i < m_frames; i++)
			delete [] m_pCache[i];
		delete [] m_pCache;
		}
	delete [] m_pCamBuf;
	delete [] m_pJpeg;
	if (m_pCellCache)
		{
		for (i = 0; i < m_nCells; i++)
			delete [] m_pCellCache[i].pData;
		delete [] m_pCellCache;
		}
	delete [] m_pBG;
	delete m_pCamera;
	delete [] m_pXY;
	delete [] m_pLog;
	delete [] m_pErrors;
	delete m_pJEnc;
	delete m_pJDec;
	delete [] m_pClipBoard;
}

void CSceneBase::DeleteLevels()
{
	delete m_pLevels;
	m_pLevels = 0;
	UINT i;
	if (m_pLevelArray)
		{
		for (i = 0; i < m_levels;i++)
			{
			CLevel * pLevel = m_pLevelArray[i];
			delete pLevel;
			}
		delete [] m_pLevelArray;
		m_pLevelArray = 0;
		}
	for (i = 0; i < 100; i++)
		{
		delete m_pPals[i];
		m_pPals[i] = 0;
		}
}

void CSceneBase::SelectPeg(UINT peg)
{
	m_CamPegSel = peg;
//	m_pFlags[m_CamFrame] = 1;
}

bool CSceneBase::CamCursor(UINT x, UINT y)
{
	UINT peg;
//	if (!m_CamState)
//		return 0;
	if (!m_pCamera)
		return 0;
	x = 2 * x / m_factor;
	y = 2 * y / m_factor;
	if (x < ComW() && y < ComH())
		peg = m_pCamBuf[ComW() * (ComH() - 1 - y) + x];
	else
		peg = 255;
DPZ("lvl:%d,cl:%d,st:%d,x:%d,y:%d",peg,m_CamPeg,m_CamState,x,y);
DPZ("comw:%d,m_w:%d,fact:%d",ComW(),m_width,m_factor);
	if (peg == m_CamPeg)
		return 0;
	m_CamPeg = peg;
	return 1;
}

void CSceneBase::CompositePiece(BYTE * pBuf, UINT Frame,
			UINT x1, UINT y1, UINT x2, UINT y2)
{
	UINT x = m_x;
	UINT y = m_y;
	UINT w = m_w;
	UINT h = m_h;
	m_x = x1;
	m_y = y1;
	m_w = x2 + 1 - m_x;
	m_h = y2 + 1 - m_y;

	UINT ow = ComW();
	UINT oh = ComH();
	UINT op = 4 * ((m_depth * ow + 3) / 4);
	UINT oy, ox;
	for (oy = m_y; oy < (m_y + m_h);oy++)
	for (ox = m_x; ox < (m_x + m_w);ox++)
		{
		UINT ty = oh - 1 - oy;
		pBuf[ty*op+3*ox+0] = 128;
		pBuf[ty*op+3*ox+1] = 128;
		pBuf[ty*op+3*ox+2] = 128;
		}
//	CompositeFrame(pBuf, 0, m_levels - 1, Frame,0);

	m_x = x;
	m_y = y;
	m_w = w;
	m_h = h;
}

void CSceneBase::PublishSizes(UINT & w, UINT & h)
{
	UINT f = m_factor;
	if (Broadcast())
		m_factor = m_origfactor;
	w = ComW();
	h = ComH();
	m_factor = f;
}

double CSceneBase::CamScale()
{
	if (m_pCamera)
		return m_pCamera->CamScale();
	else
		return 0.0;
}

bool CSceneBase::Modified(bool bClear)
{
	if (bClear)
		m_bModified = FALSE;
	return m_bModified;
}
/*
	needed to allocate larger buffers
*/
void CSceneBase::Broadcasting(bool bBroadcast)
{
	if (Broadcast() != 1)
		return;			// not necessary
	if (!m_uState)
		return;
	UINT ow,oh;
	if (bBroadcast) 
		{
		ow = m_width;
		oh = m_height;
		}
	else
		{
		ow = ComW();
		oh = ComH();
		}
	int z = ForgeImprint(ow,oh);
	if (z)	// create gauze
		{
		UINT i,c;
		c = ow * oh * m_depth;
		for (i = 0; i < c;i++)
			m_pImprint[i] = 128 + (i & 127);
		}
	DPF("imprintz:%d",z);
}

UINT CSceneBase::Broadcast(UINT v /*=-1*/)
{
	if (v != -1)
		{
		UINT old = (m_flags >> 14) & 3;
		if (v != old)
			{
			if ((v == 2) || (old == 2))
				{
				SetupCache(FALSE, FALSE);
//				for (UINT Frame = 0; Frame < m_frames; m_pFlags[Frame++] = 1);
				}
			m_flags = (m_flags & 0x3fff) | (v << 14);
			Write();
			}
		}
	return (m_flags >> 14) & 3;
}

bool CSceneBase::Flag(UINT zwhich, bool bSet /*=FALSE */ , bool bValue /*=FALSE */)
{
	UINT which = 1 << zwhich;
	if (bSet)
		{
		bool bCur = (m_flags & which) ? 1 : 0;
		if (bCur != bValue)
			{
			m_flags ^= which;
			Write();
			if ((zwhich == 1) || (zwhich == 5))
				UpdateCache();
			}
		}
	return m_flags & which ? 1 : 0;
}

UINT CSceneBase::Zoom(UINT zoom )
{
	if ((zoom < 8) && (zoom != m_zoom))
		{
		m_zoom = zoom;
		Write();
		}
	return m_zoom;
}

UINT CSceneBase::SetFactor(UINT Factor)
{
	if ((Factor * m_origfactor) != m_factor)
		{
		m_factor = m_origfactor * Factor;
		Write();
		}
	SetupCache(FALSE, FALSE);
	return m_factor;
}

void CSceneBase::InitCellCache()
{
	UINT i;
	if (m_pCellCache)
		{
		for (i = 0; i < m_nCells; i++)
			delete [] m_pCellCache[i].pData;
		delete [] m_pCellCache;
		m_pCellCache = 0;
		}
	UINT level;
	m_nCells = 1; // extra for luck
	for (level = 0; level < m_levels; level++)
		if (LevelFlags(level) & 1)
			m_nCells++;
	
	m_pCellCache = new CCellEntry[m_nCells];
	for (i = 0; i < m_nCells; i++)
		{
		m_pCellCache[i].dwKey = -1;
		m_pCellCache[i].pData = 0;//new BYTE[siz];
		}
}

UINT CSceneBase::SetupCache(bool bRead, bool bInit )
{
	UINT i;
	bool bFactoring = 0;
//	if (!bRead && !bInit)
		{
		bFactoring = TRUE;
		bRead = TRUE;
		}
DPZ("setup cache:%d",bFactoring);
	m_w = ComW();
	m_h = ComH();
	m_x = 0;
	m_y = 0;
	m_size = m_h * 4 * ((m_depth*m_w+3)/4);
	if (m_pCache)
		{
		for (i = 0; i < m_frames; i++)
			{
			if (m_pCache[i])
				delete [] m_pCache[i];
			m_pCache[i] = 0;
			}
		delete [] m_pCache;
		m_pCache = 0;
		}
	delete [] m_pCamBuf;
	m_pCamBuf = 0;
	delete [] m_pJpeg;
	m_pJpeg = 0;
	if (m_pCellCache && !bFactoring)
		{
		for (i = 0; i < m_nCells; i++)
			delete [] m_pCellCache[i].pData;
		delete [] m_pCellCache;
		m_pCellCache = 0;
		}
	delete [] m_pFlags;
	m_pFlags = new UINT[m_frames];
	m_cache_state = 0;
	m_pCache = new BYTE * [m_frames];
	for (i = 0; i < m_frames; i++)
		m_pCache[i] = 0;
	bool bBad = 0;
	if (!m_jpeg_quality)
		{
    	try
 	   	{
		for (i = 0; i < m_frames; i++)
			{
			if (bRead)
				m_pFlags[i] = 1;
			else
				m_pFlags[i] = 0;
			m_pCache[i] = new BYTE[m_size];
			}
		m_pCamBuf = new BYTE[m_width * m_height+ m_size];
		}
 	   catch(std::exception&)
    		{
    //	    e->ReportError();
     //   e->Delete();
      //  return TRUE;
			bBad = 1;
    		}
//	UINT siz = m_depth == 1 ? 1 : 4;
		}
	if (bBad)
		{
		delete [] m_pCamBuf;
		for (i = 0; i < m_frames; i++)
			{
			delete [] m_pCache[i];
			m_pCache[i] = 0;
			}
		m_jpeg_quality = 80;
		m_pJEnc = new CMiniJpegEncoder(m_jpeg_quality);
		m_pJDec = new CMiniJpegDecoder;
		}
	if (bBad || m_jpeg_quality)
    try
 	   {
		for (i = 0; i < m_frames; i++)
			{
			if (bRead)
				m_pFlags[i] = 1;
			else
				m_pFlags[i] = 0;
			m_pCache[i] = new BYTE[m_size/10];
			*((UINT *)m_pCache[i]) = m_size/10; // start with an estimate
			}
		m_pCamBuf = new BYTE[m_width * m_height+ m_size];
		m_pJpeg = new BYTE[m_width * m_height * 6];// m_size];
		}
    catch (std::exception&)
    	{
        return TRUE;
    	}
//		m_pCamBuf = new BYTE[m_width * m_height+ m_size];
//	UINT siz = m_depth == 1 ? 1 : 4;
//	siz *= m_height * m_width;
	InitCellCache();
	delete [] m_pXY;
//	UINT pw, ph;
//	PublishSizes(pw,ph);
	UINT w = 2 * m_width / m_origfactor;
	UINT h = 2 * m_height / m_origfactor;	// full size just to be safe
	m_pXY = new UINT[2 * w * h]; // for export w or w/o preview
	if (m_uState)
		{
		int z = ForgeImprint(m_w,m_h);
		if (z)	// create gauze
			{
			UINT i;
			for (i = 0; i < m_size;i++)
				m_pImprint[i] = 128 + (i & 127);
			}
		DPF("imprintz:%d",z);
		}
	if (bBad)
		FBMessageBox("Switched on JPEG"); 
	return 0;
}

UINT CSceneBase::Make(DWORD idd,  DWORD features,
		UINT width, UINT height, UINT rate, UINT frames, UINT levels,
				UINT Factor, UINT Preview, UINT jpeg, UINT broadcast)
{
	DPF("making scene");
	m_bAppended = 0;
	m_bLoading = FALSE;
	m_flags = (m_flags & 0x3fff) | ((broadcast & 3) << 14);
	m_jpeg_quality = jpeg;
	m_origfactor = 2 + Factor;
	m_factor = m_origfactor * (1 + Preview);
	m_width = width * m_origfactor / 2;
	m_height = height * m_origfactor / 2;
	m_frames = frames;
	m_levels = levels;
	m_oflags = features >> 16;	// overrides from open doc
	m_start = 0;
	m_stop = m_frames - 1;
	m_dwId = idd;
	m_uState = features & 15 ? 0 : 2;
#ifdef _THEDISC
	m_max_frames = 300;
#else
	m_max_frames = 1500;
	if ((features & 15) == FEAT_LITE)
		{
		if (features & BIT_PLUS)
			m_max_frames = 1000;
		else
			m_max_frames = 300;
		}
#endif
	m_links = 0;
	m_smark0 = m_smark1 = m_smark2 = 0;
	m_vmark0 = m_vmark1 = m_vmark2 = 0;
	m_mvolume = m_volume0 = m_volume1 = m_volume2 = 0;
	m_rate = rate;
	m_zoom = 0;
	m_wave0[0] = m_wave1[0] = m_wave2[0] = 0;
	delete [] m_pLinks;
	m_pLinks = 0;
	DeleteLevels();
	SetupPalettes(2);
	delete m_pCamera;
	delete [] m_pInfo;
	m_info = 0;
	m_pLevels = new CLevels(m_pIO);
	if (!m_pLevels) return 7;
	if (((features & 15) == FEAT_PRO) || !(features & 15) || (features & 0x100))
		{
		m_pCamera = new CCamera(m_pIO, KEY_CAMERA);
		if (!m_pCamera) return 8;
		m_pCamera->Setup(this);
		}
	else
		m_pCamera = 0;
	m_pLevels->SetKey(KEY_LEVELS);
	int nResult = Write();
	if (!nResult)
		m_pLevels->Write();
	m_bOptLock = 0;
	GetPutOptions(TRUE);
	if (m_pCamera)
		m_pCamera->Flush();
	if (m_jpeg_quality)
		{
		m_pJEnc = new CMiniJpegEncoder(m_jpeg_quality);
		m_pJDec = new CMiniJpegDecoder;
		}
//	DPF("insert frames:%d",wResult);
	SetupCache(FALSE, TRUE);
	m_bModified = TRUE;
	return nResult;
}
#ifdef _DEBUG
#define CHECKLOG
#endif
#ifdef CHECKING

int CSceneBase::Bump(DWORD key, int which /* = 0 */)
{
	UINT i;
	m_pEntry[m_nEntries].dwKey = key;
	for(i = 0;m_pEntry[i].dwKey != key;i++);
	if (i >= m_nEntries)
		{
dpc("nofnd key:%x, for bump,which:%d",key,which);
		return 1;
		}
	else if (which == 0)
		m_pEntry[i].dwCount++;
	else if (which == 1)
		m_pEntry[i].dwLink++;
	else if (which == 2)
		return i + 2; // to skip over error rteurn
	else if (which == 3)
		m_pEntry[i].dwCount = 1; // force single
	return 0;
}
int CSceneBase::Check()
{
	UINT Level, Frame, Layer;
	bool bDeleted = 0;
	int result = 0;
DPF("checking");
	m_nEntries = m_pIO->RecordCount(0);
	if (!m_nEntries) return 0;
dpc("count:%d",m_nEntries);
	DWORD key,adr,size,kind;
	m_pEntry = new CHKENTRY[m_nEntries+1];
	if (!m_pEntry)
		return 1;
	UINT i;
	for (i = 0; i < m_nEntries; i++)
		{
		m_pEntry[i].dwCount = 0;
		if (m_pEntry[i].dwStat = m_pIO->RecordInfo(key,size,adr,kind,i,2))
			result++;
		m_pEntry[i].dwKey= key;
		m_pEntry[i].dwAdr = adr;
		m_pEntry[i].dwSize = size;
		m_pEntry[i].dwKind = kind;
		UINT zz = RefCount(key);
		m_pEntry[i].dwLink = zz ? zz : 1;
#ifdef CHECKLOG
dpc("i:%4d,key:%5d(%4x),stat:%3d,size:%5d,adr:%5d,kind:%8X,link:%d",i,
		m_pEntry[i].dwKey,
		m_pEntry[i].dwKey,m_pEntry[i].dwStat,
		m_pEntry[i].dwSize,m_pEntry[i].dwAdr,
		m_pEntry[i].dwKind, m_pEntry[i].dwLink);
#endif
//DPF("i:%4d,key:%5d,link:%d",i,key,link);
		}
	result += Bump(KEY_SCENE);
	result += Bump(KEY_LEVELS);
	if (result)
	{
		return result;
	}
	Bump(KEY_CLIP);
	Bump(KEY_LINKS);
	Bump(KEY_OPTIONS);
	Bump(KEY_LEVELINFO);
	Bump(KEY_PALETTE);
	Bump(KEY_TOOLS);
	Bump(KEY_AVI);
	Bump(KEY_CAMERA);
#ifdef _DISNEY
	Bump(KEY_DISPAL);
#endif
	bool bNew = Bump(KEY_PAL_BASE+0) ? 0 : 1;
//	Bump(KEY_PAL_BASE+1);
//	Bump(KEY_PAL_BASE+2);
	if (m_pLinks && m_links)
		{
		dpc("links:%d",m_links);
		UINT i;
		for (i = 0; i < m_links; i++)
			{
			dpc("%3i %5x, %d",i,m_pLinks[i].dwKey, m_pLinks[i].dwCount);
			}
		}
	if (m_pLevels->Check())
		{
		result |= 0x800;
		dpc("Level Check failure");
		m_bModified = TRUE;
		}

	for (Level = 0; Level < m_levels; Level++)
		{
		CLevel * pLevel = GetLevelPtr(Level);
		if (!pLevel)
			{
dpc("missing level:%3d",Level);
//if (Level == m_levels - 1)
//	m_levels--;
//		result += 999;
//		m_bModified = TRUE;
			continue;
			}
		result += Bump(pLevel->GetKey());
		if (bNew)
		{
		dpc("lvl:%d,key:%4X,pal:%d",Level,pLevel->GetKey(),pLevel->PalIndex());
		result += Bump(pLevel->PalIndex()+KEY_PAL_BASE,3);
		}
		if (pLevel->Cleanup(m_frames))
			{
			result += 0x1000;
			dpc("Level(%d) Cleanup",Level);
			m_bModified = TRUE;
			}
		for (Frame = 0; Frame < m_frames; Frame++)
			{
			CCell * pCell = (CCell *)GetCellPtr(pLevel, Frame, 0);
			if (!pCell )
				{
//				pLevel->DeleteCell(Frame);
//				m_bRepaired = TRUE;
				continue;
				}
			DWORD cellkey = pCell->GetKey();
	//		UINT cellcnt = RefCount(cellkey);
//			UINT cellcnt = LinkCellRecord(cellkey);
			result += Bump(cellkey);
dpc("frm:%5d,lvl:%5d,cell key:%4X,res:%d",Frame,Level,cellkey,result);
/*
			if (cellcnt)
				{
				int j = Bump(cellkey,2);
				if (j >= 2)
					{
					if (m_pEntry[j-2].dwCount != 1) // if not first
						cellcnt = 0;
					}
				}
*/
			for (Layer = 0; Layer < 20; Layer++)
				{
				DWORD key = pCell->Select(Layer);
				if (key)
					{
dpc("   layer:%d,key:%4X",Layer,key);
					DWORD size;
					int zresult = m_pIO->GetSize(size,key);
//DPF("lvl:%3d,frm:%3d,lay:%d,key:%4X,res:%d",Level,Frame,Layer,key,zresult);
					if (zresult)
						{
pCell->DeleteLayer(Layer);
						result += 0x2000;
dpc("bad layer, lvl:%3d,frm:%3d,lay:%d,key:%4X,res:%d",Level,Frame,Layer,key,zresult);
						
						}
					result += Bump(key);
			//		UINT q;
			//		for (q = 0; q < cellcnt;q++)
			//			result += Bump(key,1);
					}
				}
			delete pCell;
			}
		}
dpc("cleanup:%d,res:%d",m_nEntries,result);
for (i = 0; i < m_nEntries;i++)
	if ((m_pEntry[i].dwCount != m_pEntry[i].dwLink) ||
		m_pEntry[i].dwStat)
		{
//result = 2;
#ifdef CHECKLOG
dpc("i:%4d,key:%5x,stat:%3d,size:%5d,adr:%5d,kind:%8X,cnt:%3d,link:%d",i,
		m_pEntry[i].dwKey,m_pEntry[i].dwStat,
		m_pEntry[i].dwSize,m_pEntry[i].dwAdr,
		m_pEntry[i].dwKind, m_pEntry[i].dwCount, m_pEntry[i].dwLink);
#endif
if (!m_pEntry[i].dwCount && !m_pEntry[i].dwStat)
	{
	if ((m_pEntry[i].dwKey < KEY_PAL_BASE) ||
					(m_pEntry[i].dwKey > KEY_PAL_BASE + NBR_PALS))
		{
dpc("i:%4d,key:%5x, deleting",i,m_pEntry[i].dwKey);
	m_pIO->DelRecord(m_pEntry[i].dwKey);
	bDeleted = TRUE;
	m_bModified = TRUE;
	result = 2;
		}
	}
else if ((m_pEntry[i].dwCount != m_pEntry[i].dwLink) 
					&& (m_pEntry[i].dwKind == 4))
	{
	result = 3;
dpc("setting link count,key:%5x,%u",m_pEntry[i].dwKey, m_pEntry[i].dwCount);
	SetRefCount(m_pEntry[i].dwKey, m_pEntry[i].dwCount);
	}
		}
		for (i = 0; i < m_links; i++)
			{
	//		dpc("%3i %5x, %d",i,m_pLinks[i].dwKey, m_pLinks[i].dwCount);
			UINT key = m_pLinks[i].dwKey;
			m_pEntry[m_nEntries].dwKey = key;
			UINT j;
			for(j = 0;m_pEntry[j].dwKey != key;j++);
			if (j >= m_nEntries)
				{
			result = 3;
				dpc("link not found,j:%d,k:%d(%X)",j,key,key);
				for (j = i + 1;j < m_links;j++)
					m_pLinks[j-1] = m_pLinks[j];
				m_links--;
				}
			}
	delete [] m_pEntry;
	if (result == 3)
		{
		if (!m_pLinks)
			{
			ASSERT(m_links == 0);
			m_linkz = m_links + 100;
			m_pLinks = new LINKENTRY[m_linkz];
			}
//		UpdateLinks(0,0);
m_pIO->PutSwapRecord(m_pLinks, (m_links + 1) * sizeof(LINKENTRY), KEY_LINKS);
		m_bModified = TRUE;
		}
	m_pEntry = 0;
	if (bDeleted)
		m_nNewRecords++;	// we made a record for deleted stuff
	return result;
}
#endif

BYTE * CSceneBase::LoggedData(bool bClear)
{
	if (bClear)
		{
		delete [] m_pLog;
		m_pLog = 0;
		m_logsize = 0;
		}
	return m_pLog;
}

void CSceneBase::LogIt(int Id, UINT level, LPCSTR name /*=0 */)
{
	CString txt;
	txt.LoadString(Id);
	char buf[300];
	if (level == -1)
		{
		if (name)
			sprintf(buf,"%s : %s", (LPCSTR)txt, name);
		else
			sprintf(buf,"%s", (LPCSTR)txt);
		}
	else if (!Id && name)
		strcpy(buf, name);
	else if (name)
		sprintf(buf,"lvl:%d,%s : %s", level, (LPCSTR)txt, name);
	else
		sprintf(buf,"lvl:%d,%s", level, (LPCSTR)txt);
	UINT c = strlen(buf);
	UINT i;
	if (((5 + c + m_logsize+999) / 1000) > ((m_logsize+999) / 1000))
		{
DPZ("grow log,size:%d,c:%d",m_logsize,c);
		i = 1000 * ((5 + c + m_logsize+999) / 1000);
		BYTE * tp = new BYTE[i];
		for (i = 0; i < m_logsize; i++)
			tp[i] = m_pLog[i];
		delete m_pLog;
		m_pLog = tp;
		}
	for (i = 0; i < c; i++)
		m_pLog[m_logsize++] = buf[i];
	m_pLog[m_logsize++] = 13;
	m_pLog[m_logsize++] = 10;
	m_pLog[m_logsize] = 0;
}

UINT CSceneBase::ReadErrors(UINT code)
{
	if (code == NEGONE)
		{
		delete [] m_pErrors;
		m_pErrors = 0;
		m_nErrors = 0;
		}
	else if (code != NEGTWO)
		{
		ASSERT(code < m_nErrors);
		return m_pErrors[code];
		}
	return m_nErrors;
}

void CSceneBase::RelativeName(LPSTR Name)
{
	char buf[350];
	strcpy(buf, m_pIO->Name());
	int i, j;
	for (i = 0, j = 0; buf[i]; i++)
//		if (buf[i] == NATIVE_SEP_CHAR) // find trailing slash
        if (buf[i] == NATIVE_SEP_CHAR_WIN32 || buf[i] == NATIVE_SEP_CHAR_NWIN32) // find trailing slash
			j = i;
	if (j)
		buf[j+1] = 0;	//term path at slash
	for (i = 0, j = 0; Name[i]; i++)
//		if (Name[i] == NATIVE_SEP_CHAR)
        if (Name[i] == NATIVE_SEP_CHAR_WIN32 || Name[i] == NATIVE_SEP_CHAR_NWIN32)
			j = i;
	strcat(buf,Name+j+1);
	strcpy(Name, buf);
}

UINT CSceneBase::EmbFind(LPCSTR name, UINT kind) // -1 is not found, else index
			{ return m_pIO->EmbFind(name,kind);};
UINT CSceneBase::EmbData(UINT index, LPBYTE pData)
			{ return m_pIO->EmbData(index, pData);};

void CSceneBase::SetEmbedded(bool bAppended)
{
	m_bAppended = bAppended;
	Write();
}

bool CSceneBase::EmbAppend(bool bJustEraseDGY)
{
	m_pIO->EmbAppend(bJustEraseDGY);
	Write();
	return m_bAppended;
}

int CSceneBase::CheckExternals()
{
	UINT level;
	char name[350];
	delete [] m_pErrors;
	m_pErrors = new UINT[200];
	m_nErrors = 0;
	for (level = 0; level < m_levels; level++)
		{
		LevelModelName(name, level);
		if (name[0])
			{
			if (m_pIO->EmbFind(name,EMB_KIND_MODEL) != NEGONE)
				continue;
			if (TestModel(name, m_width, m_height))
				{
				char temp[350];
				strcpy(temp, name); // save for error msg
				RelativeName(name);
				if (TestModel(name, m_width, m_height))
					{
					LogIt(IDS_EXT_MODEL,level,temp);
					}
				else
					{
					LevelModelName(name,1);
					LogIt(IDS_EXT_MODEL_CHANGE,level,name);
					}
				}
			}
		}
	int i;
	for (i = 0; i < NBR_PALS; i++)
		{
		CNewPals * pPals = m_pPals[i];
		if (pPals)
			{
			if (i && !PalUsed(i))
				m_pErrors[m_nErrors++] = i + (4 << 16);
			int j;
			char temp[350];
			if (j = pPals->Linked(1))
				{
				if (j == 1)
					{
					strcpy(name, pPals->GetFileName());
				//	if (m_pIO->EmbFind(name,EMB_KIND_PALETTE) != NEGONE)
				//		{
				//		j = 0;
				//		}
				//	else
				//		{
					strcpy(temp, name); // save for error msg
					RelativeName(name);
					pPals->SetFileName(name);
					j = pPals->Linked(1);
					if (j != 1)
						{
						LogIt(IDS_EXT_PAL_CHANGE,-1,name);
						}
					else
						pPals->SetFileName(temp);
					}
			//		}
				if (j == 1)
					
					m_pErrors[m_nErrors++] = i + (1 << 16);
				else
					m_pErrors[m_nErrors++] = i + (2 << 16);
/*
				if (j == 1)
					LogIt(IDS_EXT_PALS,-1,pPals->GetFileName());
				else
					LogIt(IDS_EXT_PALDIFF,-1,pPals->GetFileName());
*/
				}
			for (j = 0; j < 256; j++)
				{
	//			if (m_pIO->EmbFind(pPals->FileName(j),EMB_KIND_TEXTURE)
	//						!= NEGONE)
	//				continue;
				if (pPals->FileName(j,1)) // just test for exsitence
					{
					strcpy(name, pPals->FileName(j));
					RelativeName(name);
					if (!pPals->LoadTexture(j,name))
						{
						LogIt(IDS_EXT_TEXTURE_CHANGE,-1,name);
						}
					else
						{
						m_pErrors[m_nErrors++] = i + (3 << 16) + (j << 8);
						}
//					LogIt(IDS_NO_TEXTURE_FILE,-1,pPals->FileName(j));
					}
				}
			}
		}

	DPZ("q");
	for (i = 0; i < 3; i++)
		{
		SceneOptionStr(SCOPT_WAVE0+i,name);
		if (name[0])
			{
			if (i >= (int)m_snd_levels)
				{
				LogIt(IDS_EXT_SND_LEVELS,-1,name);
				continue;
				}
			if (m_pIO->EmbFind(name,EMB_KIND_SOUND) != NEGONE)
				continue;
			if (TestSound(name))
				{
				char temp[350];
				strcpy(temp, name); // save for error msg
				RelativeName(name);
				if (TestSound(name))
					{
					LogIt(IDS_EXT_WAVE,-1,temp);
					}
				else
					{
					SceneOptionStr(SCOPT_WAVE0+i,name,1);
					LogIt(IDS_EXT_WAVE_CHANGE,-1,name);
					}
				}
			}
		}
	return 0;
}

int CSceneBase::Read(bool bPreview , DWORD dwFeatures, DWORD idd)
{
	m_bAppended = 0;
	m_nCells = 0;
	m_nNewRecords = 0;
	m_bRepaired = 0;
	m_oflags = dwFeatures >> 16;	// overrides from open doc
	m_stop = m_start = 0;
	LoggedData(TRUE);
	ReadErrors();
	DPZ("reading scene,features:%x,preview:%d",dwFeatures,bPreview);
	SCENEHEADER header;
	if (m_pIO->GetRecord(&header, sizeof(header), KEY_SCENE))
		return 1;
	#ifdef _NEEDSWAP
	SwapHdr((BYTE *)&header);
	#endif
	if (header.dwId != DGCID)
		return 2;
DPZ("header kind:%d",header.dwKind);
dpc("Zheader kind:%d",header.dwKind);
	if (header.dwKind < 5)
		return 2;
	if (header.dwKind > 15)
		return 3;
// 15 is ianimate with appended data
//
// 12 and 13 are rel 6 kinds with new palette structure
// since we convert old ones, adjust numbers
//

	bool bChanged = FALSE;
	m_uState = 2;
	UINT aptype = dwFeatures & BITS_TYPE;
DPZ("aptype:%d",aptype);
dpc("aptype:%d",aptype);

	if (aptype)	// release version of ap
		{
		if (header.dwKind == 6)
			{
			header.dwKind = 7 + aptype;
			header.dwMyId = idd;
			bChanged = TRUE;
			m_uState = 0;
			}
		else if (header.dwKind == 8)  // rel 5 demo
			m_uState = 0;
		else if (header.dwKind == 13)  // rel 6 demo
			m_uState = 0;
		else if (header.dwKind == 15)  // rel 6 embedded
			m_uState = 0;
#ifdef FIXSCENES
		else
			{
			header.dwKind = 8;
			bChanged = TRUE;
			m_uState = 0;
			}
#endif
/* per Kent 6/20/01
		else if ((header.dwMyId == idd) || !header.dwMyId)
			{
			header.dwKind = 7 + aptype;
			bChanged = TRUE;
			m_bDemo = FALSE;
			}
*/
		}
	else			// restricted ap
		{
		if (header.dwKind == 6)
			{
			bChanged = TRUE;
			header.dwKind = 7;
			header.dwMyId = idd;
			m_uState = 1;
			}
		else if (header.dwKind == 8) // rel 5 release
			m_uState = 1;
		else if (header.dwKind == 13) // rel 6 release
			m_uState = 1;
		else if (header.dwKind == 15) // rel 6 embedded
			m_uState = 1;
		}
	if (header.dwKind < 12)
		bChanged = TRUE;			// nned new palette stuff
DPZ("new kind:%d, state:%d,changed:%d",header.dwKind,m_uState,bChanged);
	UINT scale  = header.wScale % 256;
	m_factor = header.wScale / 256;
	m_origfactor = m_factor & 7;
	if (!m_origfactor)
		m_origfactor = 1;
	else if (m_origfactor > 5)
		m_origfactor = 5;
	else if (m_origfactor > 4)
		m_origfactor = 3;
	else 
		m_origfactor *= 2;
	m_zoom = m_factor >> 5;
	m_factor = (m_factor >> 3) & 3;
	m_factor = m_origfactor * (1 + m_factor);
#ifdef MAGIC_FACTOR_FIX
	m_factor = 2;
	bChanged = TRUE;
#endif
//	m_origfactor = 2;
//	m_factor = 8;
//	bChanged = TRUE;
	m_width = header.wWidth / scale;
	m_height = header.wHeight / scale;
	m_flags = header.wFlags;
	m_dwId = header.dwMyId;
	if (m_flags & 2)
		m_depth = 3;
	else
		m_depth = 1;
	m_rate = 24;
//	m_nScenePalette = header.wLevelCount / 256;
	m_frames = header.wFrameCount;
	m_levels = header.wLevelCount;// & 255; // kludge
								// I will change scene header next time
	m_start = 0;
	m_stop = m_frames - 1;
	m_bModified = FALSE;
	m_max_frames = 1500;
	UINT max_levels = 6;
#ifdef _THEDISC
	m_max_frames = 300;
	max_levels = 2;
	m_snd_levels = 1;
#else
	m_snd_levels = 3;
	if (aptype == FEAT_LITE)
		{
// perKent 1/29/02 final answer
		m_max_frames = 300;
		max_levels = 2;
		m_snd_levels = 1;
		if (dwFeatures & BIT_PLUS)  // ianimate
			{
			m_max_frames = 1000;
			max_levels += 2;
			}
		}
	else if (aptype == FEAT_PT)
		{
		m_snd_levels = 2;
		m_max_frames = 1000;
		if (m_depth != 1)
			{
			bChanged = TRUE;
			m_depth = 1;
			LogIt(0,0,"Cannot Do Color");
			}
		}
	else if (aptype == FEAT_STD)
		{
		m_snd_levels = 2;
		m_max_frames = 1000;
		}
	else if ((aptype == FEAT_PRO) || bPreview)
		{
		max_levels = 100;
		}
#endif
	if (m_frames > m_max_frames)
		return 21; // too many frames;
	if (m_levels > max_levels)
		{
		LogIt(IDS_TOO_MANY_LEVELS,m_levels);
		header.wLevelCount = m_levels = max_levels;
		bChanged = TRUE;
		}
	if (bChanged)
		Write();
//m_levels = 3;
//	WORD	wLevelCount;
//	if (header.wDepth != 8)
//		return 3;
	WORD wResult = 0;
	DeleteLevels();
	SetupPalettes((header.dwKind < 12) ? 1 : 0);
	m_pLevels = new CLevels(m_pIO);
	if (!m_pLevels) return 7;
	m_pLevels->SetKey(KEY_LEVELS);
	int result = m_pLevels->Read();
//	m_pLevels->Display();
//	m_levels = m_pLevels->Count();
DPF("frames:%d,levels:%d,hdr:%d",m_frames,m_levels,header.wLevelCount);
//	if (m_levels < header.wLevelCount)
//		m_levels = header.wLevelCount;
	delete m_pCamera;
//	if (bHaveCamera)
	if ((aptype == FEAT_PRO) || !aptype || (dwFeatures & BIT_CAMERA))
		{
DPZ("read camera");
		m_pCamera = new CCamera(m_pIO, KEY_CAMERA);
		if (!m_pCamera) return 8;
		m_pCamera->Setup(this);
		result = m_pCamera->Read();
		}
	else
		{
		m_pCamera = 0;
		result = 0;
		}
	delete [] m_pInfo;
	m_pInfo = 0;
	m_info = 0;
	m_links = 0;
	delete [] m_pLinks;
	DWORD size;
	m_pLinks = 0;
	if (!m_pIO->GetSize(size,KEY_LINKS))
		{
//		m_links = size / sizeof(LINKENTRY);
//		m_pLinks = new LINKENTRY[m_links];
//		m_pIO->GetSwapRecord(m_pLinks, m_links * sizeof(LINKENTRY),KEY_LINKS);
//		if (m_links) m_links--;
		m_links = size / sizeof(LINKENTRY);
		m_linkz = m_links + 100;
		m_pLinks = new LINKENTRY[m_linkz+1];
		m_pIO->GetSwapRecord(m_pLinks, m_links * sizeof(LINKENTRY),KEY_LINKS);
		m_links--;
		}
	GetPutOptions();
	if ((m_width * m_height > 500000) && !m_jpeg_quality)
		{
		m_jpeg_quality = 80;
		}
	if (!bPreview)
		m_pIO->InitEmbedding();
	int cres = Check();
DPZ("cres:%d",cres);
	if (bPreview)
		return cres;
	m_bLoading = TRUE;
	if (m_jpeg_quality)
		{
		m_pJEnc = new CMiniJpegEncoder(m_jpeg_quality);
		m_pJDec = new CMiniJpegDecoder;
		}
	SetupCache(TRUE, TRUE);
	CheckExternals();
	m_bLoading = FALSE;
	if (cres)
		LogIt(IDS_EXT_CHECK,-1);
	cres = 0;
//	m_nNewRecords += 1; // scene 
	if (header.dwKind < 12)
		{
		m_nNewRecords += 1; // new scene header
//		m_nNewRecords += 1; // new scene palette
		m_nNewRecords += m_levels;
		}
	if (m_bRepaired)
		cres |= 0x4000;
	UINT rc = m_pIO->RecordCount(1);
	if (rc != m_nNewRecords)
		cres |= 0x1000;
	if (header.dwKind < 12)
		{
//		bChanged = TRUE;
		cres |= 0x8000;
		}
	DPZ("qq,v:%d,rc:%d",m_nNewRecords,rc);
	return cres;
}

bool CSceneBase::ColorMode(UINT mode /* = -1 */)
{
	if (mode != -1)
		{
		UINT d,oldd;
		if (mode)
			d = 3;
		else
			d = 1;
		if (d != m_depth)
			{
			oldd = m_depth;
			Flag(SCN_FLG_COLOR,TRUE, mode);
			m_depth = d;
			delete [] m_pBG;
			m_pBG = 0;
			if (SetupCache(TRUE, FALSE))
				return oldd > 1 ? FALSE : TRUE;
			}
		}
	return m_depth > 1 ? TRUE : FALSE;
}

bool CSceneBase::RedBox(UINT mode /* = -1 */)
{
	bool bWas = Flag(SCN_FLG_REDBOX);
	if (mode != -1)
		{
		bool bIs =  mode ? TRUE : FALSE; 
		if (bWas != bIs)
			{
			bWas = bIs;
			Flag(SCN_FLG_REDBOX,TRUE, bIs); 
//			UpdateCache();	// nw if flag(
			}
		}
	return bWas;
}

void CSceneBase::SetFrameRate(UINT rate)
{
	if (rate != m_rate)
		m_bModified = TRUE;
	m_rate = rate;
}

UINT CSceneBase::FrameRate()
{
	return m_rate;
	if (m_bStory)
		return SceneOptionInt(SCOPT_SRATE);
	else
		return SceneOptionInt(SCOPT_RATE);
}

void CSceneBase::LipRect(LPRECT rect, bool bPut /* = 0 */)
{
	if (bPut)
		{
		if ((m_nLipx != rect->left) ||
				(m_nLipw != (rect->right - m_nLipx)) ||
				(m_nLipy != rect->top) ||
				(m_nLiph = (rect->bottom - m_nLipy)))
			{
			m_nLipx = rect->left;
			m_nLipw = rect->right - m_nLipx;
			m_nLipy = rect->top;
			m_nLiph = rect->bottom - m_nLipy;
			}
		GetPutOptions(TRUE);
		}
	else
		{
		rect->left = m_nLipx;
		rect->right = m_nLipx + m_nLipw;
		rect->top = m_nLipy;
		rect->bottom = m_nLipy + m_nLiph;
		}
}

UINT CSceneBase::JpegQuality(UINT v /* = NEGONE */)
{
	if ((v != NEGONE) && (v != m_jpeg_quality))
		{
		delete m_pJEnc; 
		delete m_pJDec;
		m_pJEnc = 0;
		m_pJDec = 0;		// just to be safe
//		ASSERT(v != m_jpeg_quality);
		if (m_jpeg_quality = v)
			{
			m_pJEnc = new CMiniJpegEncoder(m_jpeg_quality);
			m_pJDec = new CMiniJpegDecoder;
			}
		SetupCache(FALSE, FALSE);
		m_bModified = TRUE;
		}
	return m_jpeg_quality;
}

void CSceneBase::SetSelection(UINT start, UINT stop)
{
	if ((start >= m_frames) || (stop >= m_frames))
		return;
	m_start = start;
	m_stop  = stop;
}

void CSceneBase::SetLevelCount(UINT count, UINT def)
{
	if (m_pInfo && (count > m_levels))
		{
		UINT * tp = new UINT[count+1];
		UINT i;
		for (i = 0; i <= m_levels; i++)
			tp[i] = m_pInfo[i];
		for (; i <= count; i++)
			tp[i] = def;
		delete [] m_pInfo;
		m_pInfo = tp;
		}
	if (count > m_levels)
		{
		CLevel ** tlp = m_pLevelArray;
		m_pLevelArray = new CLevel * [count];
		UINT i;
		for (i = 0; i < m_levels;i++)		// copy level pointers
			m_pLevelArray[i] = tlp[i];
		for (; i < count;i++)
			m_pLevelArray[i] = 0;
		delete [] tlp;
		}
	m_levels = count;
//	m_pLevels->Count(m_levels);
	if (m_pCamera)
		m_pCamera->Update();
	m_bModified = TRUE;
	m_info = 1;
	PutLevelInfo(0,9999,0);
	Write();
}

int CSceneBase::Write()
{
	SCENEHEADER header;
	header.dwId = DGCID;
//	header.dwKind = m_uState ? 7 : 8;
	if (m_bAppended)
		header.dwKind = 15;
	else
		header.dwKind = m_uState ? 12 : 13;
	header.wWidth = m_width;
	header.wHeight = m_height;
	UINT zz;
	if (m_origfactor == 3)
		zz = 5;
	else if (m_origfactor == 5)
		zz = 6;
	else
		zz = m_origfactor / 2;
	UINT ff = (m_factor / m_origfactor) - 1;
	zz += (m_zoom << 5) + (ff << 3);
	header.wScale = 256 * zz + 1;// + m_scale;
	header.wFlags = m_flags;
	header.dwMyId = m_dwId;
	header.wFrameCount = FrameCount();
	header.wLevelCount = LevelCount();// + 256 * m_nScenePalette;
	#ifdef _NEEDSWAP
	SwapHdr((BYTE *)&header);
	#endif
	if (m_pIO->PutRecord(&header, sizeof(header), KEY_SCENE))
		return 1;
	m_bModified = TRUE;
	return 0;
}

void CSceneBase::LevelName(LPSTR name, UINT Level, bool bPut)
{
	if (!bPut)
		name[0] = 0;
	CLevel * pLevel = GetLevelPtr(Level, bPut);
	if (pLevel)
		{
		pLevel->Name(name,bPut);
		if (bPut)
			m_bModified = TRUE;
		}
	else
		{
		if (Level)
			sprintf(name,"%d",Level);
		else
			strcpy(name,"BG");
		}
}


void CSceneBase::LevelModelName(LPSTR name, UINT Level, bool bPut /* = 0 */)
{
	if (!bPut)
		name[0] = 0;
	CLevel * pLevel = GetLevelPtr(Level, bPut);
	if (pLevel)
		{
		pLevel->ModelName(name,bPut);
		if (bPut)
			m_bModified = TRUE;
		}
}

DWORD CSceneBase::LevelFlags(UINT Level, DWORD val /* = -1 */)
{
	bool bMake = val != -1 ? TRUE : FALSE;
	DWORD v = 1;	// default to enabled
	CLevel * pLevel = GetLevelPtr(Level, bMake);
	if (pLevel)
		{
		v = pLevel->Flags(val);
		if (bMake)
			m_bModified = TRUE;
		}
	return v;
}

//
//	the pallete has changed
//  decide if what should be recomposited
//
void CSceneBase::PalChanged(UINT index)
{
	m_bModified = TRUE;
	if (!m_bLoading)
		{
		UINT i;
		for (i = 0; i < m_levels; i++)
			{
			CLevel * pLevel = GetLevelPtr(i);
			if (!pLevel)
				continue;
			if ((pLevel->PalIndex() == index) && (pLevel->Flags() & 1))
				break;
			}
		if (i < m_levels)
			UpdateCache();
		}
	BlowCell(-3,index);
	m_pPals[index]->ReadWrite(m_pIO, KEY_PAL_BASE+index,1);
}

DWORD CSceneBase::GetCellKey(UINT Frame, UINT Level, bool bHold /* = 0 */)
{
	CLevel * pLevel = GetLevelPtr(Level);
	if (!pLevel)
		return 0;
DPF("getcellkey,f:%d,l:%d",Frame,Level);
	DWORD key = pLevel->Select(Frame,bHold);
DPF("getcellkey,f:%d,l:%d,k:%d",Frame,Level,key);
	return key;
}

void CSceneBase::UpdateLinks(UINT * pKeys, UINT cnt)
{
	delete [] m_pLinks;
	m_links = 0;
	m_linkz = m_links + 100;
	m_pLinks = new LINKENTRY[m_linkz+1];
	UINT Level, Frame;
	for (Level = 0; Level < m_levels; Level++)
		{
		CLevel * pLevel = GetLevelPtr(Level);
		if (!pLevel)
			continue;
		for (Frame = 0; Frame < m_frames; Frame++)
			{
			UINT cellkey = ((CLevel *)pLevel)->Select(Frame);
			if (!cellkey)
				continue;
			UINT i;
			m_pLinks[m_links].dwKey = cellkey;
			for (i = 0; m_pLinks[i].dwKey != cellkey;i++);
			if (i >= m_links)
				{
				if (i >= m_linkz)
					{
					LINKENTRY * pTemp = m_pLinks;
					ASSERT(pTemp != 0);
					if (!pTemp)
						return;
					m_linkz += 100;
					m_pLinks = new LINKENTRY[m_linkz+1];
					for (i = 0; i < m_links;i++)
						m_pLinks[i] = pTemp[i];
					delete [] pTemp;
					}
				m_pLinks[i].dwKey = cellkey;
				m_pLinks[i].dwCount = 0;
				m_links++;
				}
			else
				m_pLinks[i].dwCount++;
			}
		}
	UINT j = 0;
	if (pKeys && cnt)
		{
		UINT z = 0;
		if (m_pClipBoard)
			z = m_pClipBoard[0] * m_pClipBoard[1];
		for (j = 0; j < cnt; j++)
			{
			UINT key = pKeys[j];
			UINT i;
			for (i = 0; i < m_links; i++)
				if (key == m_pLinks[i].dwKey)
					break;
			if ((i < m_links) || !z)    // key is referenced
				continue;
			for (i = 0; i < z; i++)
				{
				if (key == m_pClipBoard[i + 3])
					break;
				}
			if (i < z)
				continue;			// or in clipboard
			DeleteCell(key);  // all its layers, and its record
			}
		}
	UINT c = m_links;
	m_links = 0;
	for (j = 0 ;j < c ; j++)
		{
		if (m_pLinks[j].dwCount > 0)
			{
			m_pLinks[m_links] = m_pLinks[j];
			m_pLinks[m_links++].dwCount -= 1;
			}
		}
	m_pIO->PutSwapRecord(m_pLinks, (m_links + 1) * sizeof(LINKENTRY), KEY_LINKS);
	m_bModified = TRUE;
}

UINT CSceneBase::DuplicateCell(UINT key)
{
	ASSERT(key);
	if (!key) return 0;
	m_bModified = TRUE;
	UINT newkey = CCell::Duplicate(m_pIO, key);
	return newkey;
}
#ifdef ZZZZZ
DWORD CSceneBase::LinkCell(UINT Frame, UINT Level)
{
	DWORD key = GetCellKey(Frame, Level);
	LinkCellRecord(key, 1);
	return key;
}

UINT CSceneBase::LinkCellRecord(DWORD key, int inc, bool bForce /* = 0 */)
{
	UINT i;
	UINT result;
//DPF("linkcellrec,k:%d,inc:%d",key,inc);
	for (i = 0; i < m_links; i++)
		{
//ASSERT(m_pLinks[i].dwCount);
		if (m_pLinks[i].dwKey == key)
			break;
		}
	if (inc)
		i = 0;
	if (bForce)
		{
		if (i < m_links)
			{
			if (inc > 0)
				m_pLinks[i].dwCount = inc - 1;
			else
				{
				m_links--;
				for (; i < m_links;i++)
					m_pLinks[i] = m_pLinks[i+1];
				}
			m_pIO->PutSwapRecord(m_pLinks, (m_links+1) * sizeof(LINKENTRY), KEY_LINKS);
			m_bModified = TRUE;
			}
		else if (inc > 0)
			{
			LINKENTRY * pTemp = new LINKENTRY[m_links+2];
			if (!pTemp)
				return 0;
			for (i = 0; i < m_links;i++)
				pTemp[i] = m_pLinks[i];
			delete [] m_pLinks;
			m_pLinks = pTemp;
			m_links++;
			m_pLinks[i].dwKey = key;
			m_pLinks[i].dwCount = inc;
			m_pIO->PutSwapRecord(m_pLinks, (m_links+1) * sizeof(LINKENTRY), KEY_LINKS);
			m_bModified = TRUE;
			}
		return 0;
		}
	if (i >= m_links)
		{
//ASSERT(inc >= 0);
		if (inc !=  1)
			return 0;
	//	ASSERT(inc == 1);
	//	(inc != 1) && (!bForce))
	//		return 0;
		LINKENTRY * pTemp = new LINKENTRY[m_links+2];
		if (!pTemp)
			return 0;
		for (i = 0; i < m_links;i++)
			pTemp[i] = m_pLinks[i];
		delete [] m_pLinks;
		m_pLinks = pTemp;
		m_links++;
		m_pLinks[i].dwKey = key;
		m_pLinks[i].dwCount = 0;
		result = 1;
		}
	else if (inc == 1)
		result = ++m_pLinks[i].dwCount;
	else if (inc < 0)
		{
		result = m_pLinks[i].dwCount--;
		if (!result)
			{
			m_links--;
			for (; i < m_links;i++)
				m_pLinks[i] = m_pLinks[i+1];
			}
	//	else
		result++;
		}
	else
		result = m_pLinks[i].dwCount + 1;
	if (inc)
		{
		m_pIO->PutSwapRecord(m_pLinks, (m_links+1) * sizeof(LINKENTRY), KEY_LINKS);
		m_bModified = TRUE;
		}
	return result;
}

UINT CSceneBase::SetCellKey(UINT Frame, UINT Level, DWORD key)
{
	CLevel * pLevel = GetLevelPtr(Level);
	if (!pLevel)
		return 0;
	DWORD cellkey = pLevel->Select(Frame);
ASSERT(key);
ASSERT(cellkey == 0);
	UINT result = 0;
	if (!cellkey && key)
		{
		m_bModified = TRUE;
		LinkCellRecord(key, 1);
		result = pLevel->Insert(Frame, key);
		}
ASSERT(Frame < m_frames);
	m_pFlags[Frame] = 1;
	m_cache_state = 0;
	return result;
}
#endif

void CSceneBase::ProcessCellLabel(CString & alabel, UINT hold)
{
	if (!alabel.GetLength())
		return;
	char label[20];
	strncpy(label,(LPCSTR)alabel,19);
	label[19] = 0;
	int j,l,v,f;
	l = 99;
	for (j = 0; label[j]; j++)
		if ((label[j] >= '0' ) && (label[j] <= '9'))
			l = j;
	if (l == 99)
		{
		alabel = "";
		return;
		}
	strcpy(m_xcellname,label);
	f = 1;
	v = 0;
	for (j = l;;j--)
		{
		if ((label[j] < '0' ) || (label[j] > '9'))
			{
			j++;
			break;
			}
		v = v + f * (label[j] & 15);
		f *= 10;
		if (!j)
			break;
		}
	sprintf(label+j,"%d",v + hold);
	label[19] = 0;
	alabel = label;
}

void CSceneBase::CellName(LPSTR name, UINT Frame, UINT Level, bool bPut)
{
	if (!bPut)
		name[0] = 0;
	CLevel * pLevel = GetLevelPtr(Level, bPut);
	if (!pLevel)
		return;
	CCell * pCell = (CCell *)GetCellPtr(pLevel, Frame, 0);
	if (pCell)
		{
		if (!bPut)
			{
			pCell->Name(name,bPut);
			if (!name[0])
				{
				pLevel->Name(name);

				bPut = 1;
				}
			}
		if (bPut)
			{
			pCell->Name(name,bPut);
			m_bModified = TRUE;
			pCell->Write();
			}
		delete pCell;
		}
}

UINT CSceneBase::InsertCache(UINT Start, UINT End)
{
	UINT i;
	m_cache_state = 0;
	if (End > Start)
		{
//ASSERT(Start == m_frames);
		bool bAppend = Start == m_frames ? 1 : 0;
		UINT count = End - Start;
		m_frames += count;
		if ((m_start + 1) >= Start)
			m_start += count;
		if ((m_stop + 1) >= Start)
			m_stop += count;
		UINT * pFlags = new UINT[m_frames];
		LPBYTE * pCache = new BYTE * [m_frames];
		for (i = 0; i < Start; i++)
			{
			pFlags[i] = m_pFlags[i];
			pCache[i] = m_pCache[i];
			}
		for (; i < End; i++)
			{
			if (m_jpeg_quality)
				{
				if ((count > i) || bAppend)
					{
					pFlags[i] = 1;
					UINT size = m_size / 10;
					pCache[i] = new BYTE[size];
					*((UINT *)pCache[i]) = size; // start with an estimate
					}
				else
					{
					UINT size =  *((UINT *)m_pCache[i-count]);
					pCache[i] = new BYTE[size+4];
					pFlags[i] = m_pFlags[i-count];
					memcpy(pCache[i], m_pCache[i-count], size+4);
					}
				}
			else
				{
				pCache[i] = new BYTE[m_size];
				if ((count > i) || bAppend)
					{
					pFlags[i] = 1;
					memset(pCache[i], 255, m_size);
					}
				else
					{
					pFlags[i] = m_pFlags[i-count];
					memcpy(pCache[i], m_pCache[i-count], m_size);
					}
				}
			}
		for (; i < m_frames; i++)
			{
			pFlags[i] = m_pFlags[i - count];
			pCache[i] = m_pCache[i - count];
			}
		delete [] m_pFlags;
		delete [] m_pCache;
		m_pFlags = pFlags;
		m_pCache = pCache;
		Write();
		}
	else
		{
		UINT count = Start - End;
		m_frames -= count;
		if (m_start >= Start)
			m_start -= count;
		if (m_stop >= m_frames)
			m_stop = m_frames - 1;
		else if (m_stop >= Start)
			m_stop -= count;
		UINT * pFlags = new UINT[m_frames];
		LPBYTE * pCache = new BYTE * [m_frames];
		for (i = 0; i < End; i++)
			{
			pFlags[i] = m_pFlags[i];
			pCache[i] = m_pCache[i];
			}
		for (; i < Start; i++)
			delete [] m_pCache[i];
		i = End;
		for (; i < m_frames; i++)
			{
			pFlags[i] = m_pFlags[i + count];
			pCache[i] = m_pCache[i + count];
			}
		delete [] m_pFlags;
		delete [] m_pCache;
		m_pFlags = pFlags;
		m_pCache = pCache;
		Write();
		}
	if (m_pCamera)
		m_pCamera->Update();
	return 0;
}

UINT CSceneBase::ScnChangeFrames(UINT Start, UINT End)
{
DPZ("chg frm,%d,%d",Start,End);
	UINT Level;
	if (Start == End)
		Start = m_frames;
//	else if ((Start + 1) < m_frames)
	else if (Start < m_frames)
		{
		for (Level = 0; Level < m_levels; Level++)
			{
			CLevel * pLevel = GetLevelPtr(Level,0);
			if (pLevel)
				{
				pLevel->MoveFrames(Start,End);
				}
			}
		}
	InsertCache(Start, End);
	if (m_pCamera)
		m_pCamera->Update();
	return 0;
}

UINT CSceneBase::InsertLevel(UINT Start, UINT Count, UINT def)
{
	if (!Count) return 0;

	if (m_pInfo) {
// first element is not for level
		UINT * tp = new UINT[m_levels + Count+1];
		UINT i;
		tp[0] = m_pInfo[0];
		for (i = 0; i < Start; i++)
			tp[i+1] = m_pInfo[i+1];
		for (i = 0; i < Count; i++)
			tp[i+Start+1] = def;
		for (i = Start; i < m_levels; i++)
			tp[i+Count+1] = m_pInfo[i+1];
		delete [] m_pInfo;
		m_pInfo = tp;
	}
	CLevel ** tlp = m_pLevelArray;
	m_pLevelArray = new CLevel * [m_levels+Count];
	UINT i;
	for (i = 0; i < Start;i++)		// copy level pointers
		m_pLevelArray[i] = tlp[i];
	for (i = 0; i < Count;i++)
		m_pLevelArray[Start+i] = 0;
	for (i = Start; i < m_levels;i++)		// copy level pointers
		m_pLevelArray[i+Count] = tlp[i];
	delete [] tlp;
	m_levels += Count;
	for (i = 0; i < Count;i++)
		m_pLevels->InsertLevel(Start,Count);
	if (m_pCamera)
		m_pCamera->InsertLevel(Start,Count);

	for (i = 0; i < m_nCells; i++) {
		if (m_pCellCache[i].level >= Start)
			m_pCellCache[i].level += Count;
	}

	m_bModified = TRUE;
	m_info = 1;
	PutLevelInfo(0,9999,0);
	Write();
	return 0;
}

UINT CSceneBase::DeleteLevel(UINT Start, UINT Count)
{
	if (!Count) return 0;

	if (m_pInfo) {
// first element is not for level
		UINT i;
		for (i = Start; i < m_levels; i++)
			m_pInfo[i+1] = m_pInfo[i+1+Count];
	}
	UINT i;
	for (i = 0; i < Count; i++) {
		CLevel * pLevel = m_pLevelArray[Start+i];
		UINT f;
		for (f = 0; f < m_frames; f++) {
			DWORD cellkey = pLevel->Select(f);
			if (!cellkey)
				continue;
			DeleteCell(cellkey);
		}
		delete pLevel;
	}
	m_levels -= Count;
	for (i = Start; i < m_levels;i++)
		m_pLevelArray[i] = m_pLevelArray[i+Count];

	for (i = 0; i < m_nCells; i++) {
		if ((m_pCellCache[i].level >= Start) && (m_pCellCache[i].level < (Start + Count)))
			m_pCellCache[i].level = 9999;
	}

	m_bModified = TRUE;
	m_info = 1;
	PutLevelInfo(0,9999,0);
	Write();
	UpdateLinks(0,0);
	return 0;
}


UINT CSceneBase::SlideCells(UINT From, UINT To, 
			UINT StartL, UINT EndL, UINT Count)
{
	UINT Level, Low, High;
	if (From < To)
		{
		Low = From;
		High = To;
		High = To + Count;
		}
	else
		{
		Low = To;
		High = From;
		}
	if (!Count)
		Count = m_frames - High;
	else if ((High + Count) >= m_frames)
		Count = m_frames - High;
	for (Level = StartL; Level <= EndL; Level++)
		{
		CLevel * pLevel = GetLevelPtr(Level);
		if (pLevel)
			{
			pLevel->MoveFrames(From, To, Count);
			UpdateCache(Low, Level, High + Count - Low);
			}
		}
	m_bModified = TRUE;
	return 0;
}

UINT CSceneBase::BlankCell(UINT Frame, UINT Level)
{
DPF("blank cell,frm:%d,lvl:%d",Frame,Level);
	CLevel * pLevel = GetLevelPtr(Level);
	if (!pLevel)
		return 1;
DPF("modified");
	m_bModified = TRUE;
	CCell * pCell = (CCell *)GetCellPtr(pLevel, Frame, TRUE);
	if (!pCell)
		return 2;
	BlowCell(Frame,Level);	// from cell cache
//	pCell->DeleteLayer(CCell::LAYER_MONO);
	pCell->DeleteLayer();
	delete pCell;
/*
	UINT size = 4 * m_height * ((m_width +3) / 4);
	BYTE * tbuf = new BYTE[size];
	memset(tbuf, 255, size);		// this will stop composite
	if (Level == -1)
		Level = m_CurLevel;
	PutImage(tbuf, Frame,Level,CCell::LAYER_GRAY);
	UpdateCache(Frame, Level);
	delete [] tbuf;
*/
//	BlankThumb(Frame, Level);
	return 0;
}

UINT CSceneBase::ChangeCellKey(UINT Frame, UINT Level, bool bRemove)
{
	CLevel * pLevel = GetLevelPtr(Level);
	if (!pLevel)
		return 0;
DPF("modified");
	UINT cellkey = ((CLevel *)pLevel)->Select(Frame);
	if (cellkey && bRemove)
		{
		m_bModified = TRUE;
		pLevel->DeleteCell(Frame);
		}
	return cellkey;
}

UINT CSceneBase::SwapCellKey(UINT Frame, UINT Level, UINT key)
{
	CLevel * pLevel = GetLevelPtr(Level);
	if (!pLevel)
		return 0;
DPF("modified");
	m_bModified = TRUE;

	DWORD oldkey = ((CLevel *)pLevel)->Insert(Frame, key, 1);
	return oldkey;	
}

UINT CSceneBase::DeleteCell(UINT Frame, UINT Level)
{
DPF("delete cell,frm:%d,lvl:%d",Frame,Level);
	CLevel * pLevel = GetLevelPtr(Level);
	if (!pLevel)
		return 1;
DPF("modified");
	m_bModified = TRUE;
	UINT k = pLevel->DeleteCell(Frame);
	if (k)
		UpdateLinks(0,0);
	return 0;
}

UINT CSceneBase::DeleteCell(DWORD dwKey)
{
	DPF("deleting cell:%d",dwKey);
	if (!dwKey) return 0;
//	if (LinkCellRecord(dwKey, -1))
//		return 0;
	CCell * pCell = new CCell(m_pIO);
	if (pCell == NULL)
		{
DPF("new failure");
		return 1;
		}
	pCell->SetKey(dwKey);
	if (pCell->Read())
		{
DPF("read failure");
		delete pCell;
		return 2;
		}
	m_bModified = TRUE;
	pCell->DeleteLayer();
	delete pCell;
	m_pIO->DelRecord(dwKey);
	return 0;
}

void CSceneBase::CompositeFrame(BYTE * pBuf, UINT StartLevel, UINT EndLevel, UINT Frame, bool bBroadcast)
{
    UINT Level;
    if (!Broadcast())
        bBroadcast = 0;
    if (StartLevel == 0 && (LevelFlags(StartLevel) & 1))
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
//    if (m_uState)
//        ApplyImprint(pBuf);
//    UINT Mask = bBroadcast ? 0x20000000 : 0x10000000;
//    if ((m_OptFlags & Mask) && (m_depth != 4))
//        ApplyFrameCount(pBuf, Frame);
}

bool CSceneBase::GetBackground(HPBYTE hpDst, UINT Frame, UINT min)
{
	if (m_depth != 1)
		min = 100;
	return GetLevel0(hpDst,Frame,1, min,0,1);// using broad cast to thwart fill
}

bool CSceneBase::GetLevel0Data(BYTE* hpDst, UINT Frame,
                               bool bHold, UINT min, bool bCamera, bool bBroadcast) {
    return GetLevel0(hpDst, Frame, bHold, min, bCamera, bBroadcast);
}

UINT CSceneBase::GetDepthImage() {
    switch (m_depth) {
        case 1:
            return 8;
        case 2:
            return 16;
        case 3:
            return 24;
        case 4:
            return 32;
        case 5:
            return 40;
        default:
            return 0;
    }
}

UINT CSceneBase::GetSizeForLevel0(bool bCamera) {
    UINT size;
    if (bCamera)
        size = m_size;
    else
        size = m_height * 4 * ((m_depth*m_width+3) / 4);
    return size;
}

bool CSceneBase::GetLevel0(HPBYTE hpDst, UINT Frame,
						bool bHold, UINT min, bool bCamera, bool bBroadcast)
{
	UINT OrigFrame = Frame; 
	BYTE * hpTemp = 0;
	BYTE * hpSrc = 0;
	UINT w = ComW();
	UINT h = ComH();
	UINT siz;
	if (bCamera)
		siz = m_size;
	else
		siz = m_height * 4 * ((m_depth*m_width+3) / 4);
memset(hpDst,255,siz);
	if (!min)
		{
//		memset(hpDst,255,siz);
		return TRUE;
		}
	UINT which;
	DWORD key;
	for (;;)
		{
		which = CCell::LAYER_BG;
		GetImageKey(key, Frame, 0, which);
		if (key)
			break;
//		which = CCell::LAYER_GRAY;
//		GetImageKey(key, Frame, 0, which);
//		if (key)
//			break;
		if (!bHold)
			break;
		if (!Frame)
			break;
		Frame--;
		}
	if (!key)
		{
		if (bCamera || !bBroadcast)	// from edit
			memset(hpDst,255,siz);
		return 0;
		}
	UINT iw, ih, idd;
	if (m_pBG && (m_BGk == key) && (m_BGmin == min) && (m_BGd == m_depth))
		{
		iw = m_BGw;
		ih = m_BGh;
		idd = 24;//m_BGd;
		}
	else
		{
		UINT idd;
		if (ImageInfo(iw,ih,idd, key))
			{
DPZ("bad info");
		return 0;
			}
//DPZ("info iw:%d,ih:%d,id:%d",iw,ih,id);
		if (iw > 8192)
			return 0;
		if (idd != 24)
			return 0;
		UINT size = ih;
		if (!m_pBG || (m_BGh != ih) || (m_BGw != iw) || (m_BGd != m_depth))
			{
			int ip;//,tp;
			delete [] m_pBG;
////			ip = 4 * ((m_depth * iw + 3) / 4);
			ip = 4 * ((3 * iw + 3) / 4);
			size *= ip;
			m_pBG = new BYTE[size];
			m_BGw = iw;
			m_BGh = ih;
			m_BGmin = min;
			m_BGd = m_depth;
			}
		m_BGk = key;
		m_BGmin = min;
		if (m_depth == 1)
			{
			UINT ip = 4 * ((3 * iw + 3) / 4);
			UINT op = 4 * ((iw + 3) / 4);
			UINT x, y;
			BYTE * pTemp = new BYTE[ih * ip];
			ReadImage(pTemp, key);
			for (y = 0; y < ih; y++)
			for (x = 0; x < iw; x++)
				{
				UINT v = 30 * pTemp[y*ip+3*x+2] +
						 59 * pTemp[y*ip+3*x+1] +
						 11 * pTemp[y*ip+3*x+0];
				v /= 100;
				v = 255 - ((255 - v) * min) / 100;
				m_pBG[y*op+x] = v;
				}
			delete [] pTemp;
			}
		else
			ReadImage(m_pBG, key);
		}
	if (!bCamera)	// just fetch for edit, or thumb
		{
		UINT x,y;
		 if ((m_depth == 3) || (m_depth== 1))
			{
			w = m_width;
			h = m_height;
			UINT op = 4 * ((m_depth * w + 3) / 4);
			UINT ip = 4 * ((m_depth * iw + 3) / 4);
			UINT cx = m_depth * min(w, iw);
			UINT cy = min(h, ih);
			BYTE * pDst = hpDst;
			BYTE * pSrc = m_pBG;

			if (h > ih)
				pDst += op * ((h - ih) / 2);
			else
				pSrc += ip * ((ih - h) / 2);
			if (w > iw)
				pDst += m_depth * ((w - iw) / 2);
			else
				pSrc += m_depth * ((iw - w) / 2);

			for (y = 0; y < cy; y++)
				{
				memmove(pDst, pSrc, cx);
				pDst += op;
				pSrc += ip;
				}
			}
		else if (m_depth == 4)
			{
			w = m_width;
			h = m_height;
			UINT op = 4 * ((4 * w + 3) / 4);
			UINT ip = 4 * ((3 * iw + 3) / 4);
			UINT cx = min(w, iw);
			UINT cy = min(h, ih);
			BYTE * pDst = hpDst;
			BYTE * pSrc = m_pBG;
			if (h > ih)
				pDst += op * (h - ih) / 2;
			else
				pSrc += ip * (ih - h) / 2;
			if (w > iw)
				pDst += 4 * ((w - iw) / 2);
			else
				pSrc += 4 * ((iw - w) / 2);
			for (y = 0; y < cy; y++)
				{
				for (x = 0; x < cx; x++)
					{
					pDst[4*x+0] = pSrc[3*x+0];
					pDst[4*x+1] = pSrc[3*x+1];
					pDst[4*x+2] = pSrc[3*x+2];
					pDst[4*x+3] = 255;
					}
				pDst += op;
				pSrc += ip;
				}
			}
		return 1;
		}
	if (m_pCamera)
		bCamera = m_pCamera->SetupCell(OrigFrame, 0);
	else
		bCamera = 0;
	Apply24(hpDst, m_pBG, iw, ih, bCamera, bBroadcast);
	return 1;
}

CLevel * CSceneBase::GetLevelPtr(UINT Level, bool bMake /* = 0 */)
{
ASSERT(Level < m_levels);
	if (!m_pLevelArray)
		{
		m_pLevelArray = new CLevel * [m_levels];
		for (UINT i = 0; i < m_levels;m_pLevelArray[i++] = 0);
		}
	CLevel * pLevel = m_pLevelArray[Level];
	if (pLevel)
		return pLevel;
	DWORD levelkey = m_pLevels->Select(Level);

	if (!levelkey && !bMake)
		{
//ASSERT(0);
//		return 0;
		}

	pLevel = new CLevel(m_pIO);
	m_pLevelArray[Level] = pLevel;
	pLevel->setSceneClosures([this](BYTE* pPals, BYTE* pName) { return ForgePals(pPals, pName); }, [this](UINT index) { return PalettePtr(index); }); //pLevel->SetScene(this);
	if (!levelkey)
		{
		levelkey = m_pLevels->Insert(Level);
		if (!levelkey)
			{
DPZ("insert level failure");
ASSERT(0);
			return 0;
			}
		char buf[20];
		if (Level)
			sprintf(buf,"%d", Level);
		else
			strcpy(buf,"BG");
		pLevel->SetKey(levelkey);
		pLevel->Name(buf,TRUE);
		pLevel->Flags(1,1);		// enable it
		m_bModified = TRUE;
		}
	else
		{
		pLevel->SetKey(levelkey);
		if (pLevel->Read())
			{
DPZ("sl read failure:%d",levelkey);
			m_bModified = TRUE;
			if (pLevel->Write())
				{
				//SelectLevel();	// pop stack
				ASSERT(0);
			//	return 0;
				}
	//		m_nNewRecords++;
			}
#ifdef AUTONAME
		char buf[20];
		pLevel->Name(buf);
		if (!buf[0])
			{
			sprintf(buf,"%c", 'A'+Level);
DPF("making level:%s",buf);
			pLevel->Name(buf,TRUE);
			m_bModified = TRUE;
			}
#endif
		}
	UINT index = pLevel->PalIndex();
	if (!m_pPals[index])
		{
ASSERT(0);
		m_pPals[index] = new CNewPals;
		m_pPals[index]->ReadWrite(m_pIO, KEY_PAL_BASE+index,0);
		}
	return pLevel;
}


CCell * CSceneBase::GetCellPtr(CLevel * pLevel, UINT Frame, bool bMake)
{
	DWORD cellkey = ((CLevel *)pLevel)->Select(Frame);
	if (!cellkey && !bMake)
		return 0;
	CCell * pCell = new CCell(m_pIO);
	if (pCell == NULL)
		{
DPF("new failure");
ASSERT(0);
		return 0;
		}
	if (!cellkey)
		{
		cellkey = pLevel->Insert(Frame);
		if (!cellkey)
			{
DPF("insert failure");
			delete pCell;
			return 0;
			}
		char buf[30];
		char lvl[30];
		((CLevel*)pLevel)->Name(lvl);
		if (m_xcellname[0])
			sprintf(buf,"%s - %s", lvl, m_xcellname);
		else
			sprintf(buf,"%s - %d", lvl, Frame+1);
		m_xcellname[0] = 0; //disable further use
		pCell->Name(buf, TRUE);
		pCell->SetKey(cellkey);
		pCell->Write();
		}
	else
		{
		pCell->SetKey(cellkey);
		if (pCell->Read())
			{
DPF("read failure");
//ASSERT(0);
			delete pCell;
			pLevel->DeleteCell(Frame);
			m_bRepaired = TRUE;
			return 0;
			}
#ifdef AUTONAME
		char buf[30];
		pCell->Name(buf);
		if (!buf[0])
			{
			char lvl[30];
			((CLevel*)pLevel)->Name(lvl);
			((CLevel*)pLevel)->Name(lvl);
			if (m_xcellname[0])
				sprintf(buf,"%s - %s", lvl, m_xcellname);
			else
				sprintf(buf,"%s - %d", lvl, Frame+1);
			m_xcellname[0] = 0; //disable further use
			pCell->Name(buf, TRUE);
			pCell->Write();
			}
#endif
		}
	return pCell;
}

int	 CSceneBase::GetImageKey(DWORD& key, UINT Frame,
					UINT Level, UINT Which, bool bMake)
{
	key = 0;
	CLevel * pLevel = GetLevelPtr(Level, bMake);
	if (!pLevel)
		{
//DPF("no level");
		if (bMake)
			{
ASSERT(0);
DPF("error 1");
			}
//		key = 0;
		return 0;
		}
	CCell * pCell = GetCellPtr(pLevel, Frame, bMake);
	if (pCell == NULL)
		{
//DPF("no cell");
		if (bMake)
			{
DPF("error 2");
ASSERT(0);
			}
//		key = 0;
		return 0;
		}
//	CImage * pImage = (CImage *)GetImagePtr(pCell, Which, bMake);
	key = pCell->Select(Which);
	if ((key < 2) && !bMake)
		{
		delete pCell;
		return 0;
		}
	if (!key)
		{
		key = pCell->Insert(Which);
		if (!key)
			{
DPF("insert image failure");
ASSERT(0);
			return 0;
			}
		}
	delete pCell;
	return 0;
}

int	 CSceneBase::GetImageKey(DWORD& key, DWORD cellkey, UINT Which)
{
	key = 0;
	CCell * pCell = new CCell(m_pIO);
	if (!cellkey || pCell == NULL)
		{
DPF("new failure");
ASSERT(0);
		return 0;
		}
	pCell->SetKey(cellkey);
	if (pCell->Read())
		{
DPF("read failure");
ASSERT(0);
		delete pCell;
		return 0;
		}
	key = ((CCell *)pCell)->Select(Which);
	if (!key)
		{
		delete pCell;
		return 0;
		}
	delete pCell;
	return 1;
}

int  CSceneBase::ForgeImage(HPBYTE hpDst, UINT which)
{
	WORD x, y, p,w,h,s,v;
	if (which == CCell::LAYER_GRAY)
		s = 1;//m_scale;
	else
		s = 1;
	w = m_width / s;
	h = m_height /s;
	p = w;
	if (which == CCell::LAYER_INK)
		{
		p = 4 * (( w + 3) / 4);
		h *= 2;
		v = 0;
		}
	else
		v = 255;
	DPF("forge,w:%d,h:%d,which:%d",w,h,which);
	for (y = 0; y < h; y++)
		for (x = 0; x < w;x++)
			hpDst[p * y + x] = (BYTE)v;
	return 0;
}

int  CSceneBase::ImageInfo(UINT & w, UINT &h, UINT & d, DWORD key)
{
	CMyImage * pImage = new CMyImage(m_pIO);
	if (pImage == NULL)
		{
DPZ("new read failure");
		return 0;
		}
	pImage->SetKey(key);
	int result = pImage->Read(0);
	w = pImage->Width();
	h = pImage->Height();
	d = pImage->Depth();
	delete pImage;
	return result;
}

int  CSceneBase::ReadImage(HPBYTE hpDst, DWORD key)
{
	CMyImage * pImage = new CMyImage(m_pIO);
	if (pImage == NULL)
		{
DPF("new read failure");
ASSERT(0);
		return 0;
		}
	pImage->SetKey(key);
	int result = pImage->Read(hpDst);
	delete pImage;
	return result;
}

int  CSceneBase::WriteOverlay(HPBYTE hpDst, UINT Frame, UINT Level, UINT w, UINT h)
{
	DWORD key;
//	DeleteCell(Frame,Level,TRUE);
	GetImageKey(key, Frame, Level, 
			Level ? CCell::LAYER_OVERLAY : CCell::LAYER_BG, TRUE);
	if (key == 0)
		{
DPF("wrt overly, null key after bmake");
		return 9;
		}
	CMyImage * pImage = new CMyImage(m_pIO);
	if (pImage == NULL)
		{
DPF("new write failure");
ASSERT(0);
		return 2;
		}
	pImage->SetKey(key);
	int result;
	UINT d = Level ? 32 : 24;
#ifdef NOCOMPRESS
	pImage->Setup(w, h, d, 0);
#else
	pImage->Setup(w, h, d, 1);
#endif
	result = pImage->Write(hpDst);
	m_bModified = TRUE;
	delete pImage;
	return result;
}

int  CSceneBase::WriteImage(HPBYTE hpDst, DWORD key, UINT which)
{
	DWORD w,h,f,d;
	f = 0;
	d = 8;
	switch (which) {
	case CCell::LAYER_MONO:
		f = 1;
		w = m_width;
		h = m_height;
		break;
	case CCell::LAYER_THUMB:
		w = m_thumbw;
		h = m_thumbh;
		break;
	case CCell::LAYER_GRAY:
		w = m_width;// / m_scale;
		h = m_height;// / m_scale;
		f = 1;
		break;
	case CCell::LAYER_INK:
	case CCell::LAYER_PAINT:
		d = 16;
		f = 1;
		w = m_width;// / m_scale;
		h = m_height;// / m_scale;
		break;
	case CCell::LAYER_MATTE0:
	case CCell::LAYER_MATTE1:
	case CCell::LAYER_MATTE2:
	case CCell::LAYER_MATTE3:
	case CCell::LAYER_MATTE4:
	case CCell::LAYER_MATTE5:
	case CCell::LAYER_MATTE6:
	case CCell::LAYER_MATTE7:
	case CCell::LAYER_MATTE8:
	case CCell::LAYER_MATTE9:
		d = 16;
		f = 1;
		w = m_width;// / m_scale;
		h = m_height;// / m_scale;
		break;
	case CCell::LAYER_BG:
		f = 1;
		d = 24;
		w = m_width;// / m_scale;
		h = m_height;// / m_scale;
		break;
	default:
DPF("doing blank, which:%d",which);
		f = 0;
		w = 1;
		h = 1;
		break;
	}
#ifdef NOCOMPRESS
	f = 0;
#endif
	CMyImage * pImage = new CMyImage(m_pIO);
	if (pImage == NULL)
		{
DPF("new write failure");
ASSERT(0);
		return 2;
		}
	pImage->SetKey(key);
	int result;
	pImage->Setup(w, h, d, f);
	result = pImage->Write(hpDst);
	m_bModified = TRUE;
	delete pImage;
	return result;
}


UINT CSceneBase::CachedSize(UINT Frame)
{
	UINT size = 0;
	if (m_jpeg_quality)
		{
		BYTE * sp = m_pCache[Frame];
		size = *((int *)sp);
		}
	else
		size = m_size;
	return size;
}


void CSceneBase::GetJpegImage(UINT Frame)
{
	BYTE * sp = m_pCache[Frame];
	sp += 4;	// over buffer size
	int w, h, hsize,isize;
	isize = *((int *)sp);
	if (isize < 0)
		return;
	int z = m_pJDec->GetImageInfo(sp+4,
				isize, w, h, hsize);
	if (w != ComW())
		return;
	int zz = m_pJDec->DecompressImage(sp+4+hsize, m_pJpeg);	

	if (m_depth == 1)
		{
		int x,y;
		for (y = 0; y < h;y++)
			{
			for (x=0; x < w; x++)
				{
				m_pJpeg[y*w+x] = m_pJpeg[w*y*3+x*3];
				}
			}
		}
}

HPBYTE CSceneBase::GetCacheP(UINT Frame)
{
	BYTE * dp;
	if (m_CamState)// && (Frame == m_CamFrame))
		{
		UINT w = ComW();
		UINT h = ComH();
		dp = m_pCamBuf + w * h;
		UINT x, y, peg;
		UINT p = 4 * ((m_depth * w + 3) / 4);
//		BYTE * sp = m_pCamBuf + w * h;//Cache[Frame];
		peg = m_CamPeg;
		if (peg == 255)
			peg = 254;// not dots if no BG
		if (m_depth == 1)
			{
			for (y = 0; y < h;y++)
			for (x = 0; x < w;x++)
				{
				if ((m_pCamBuf[w * y + x] == peg) &&
							!(x % 3) && !(y % 3))
					{
					dp[y*p+x] ^= 255;//= sp[y*p+x] ^ 255;
					}
				else
					{
//					dp[y*p+x] = sp[y*p+x];
					}
				}
			}
		else
			{
			UINT which;
			which = 0;		// 0 is blue, 1 is green, 2 is red
			UINT f = 100;	// 0..255, smaller is more tint
			UINT g = 128;
			UINT z = 255 * (255 - f);
			for (y = 0; y < h;y++)
			for (x = 0; x < w;x++)
				{
				if (m_pCamBuf[w * y + x] == peg)
					{
					int j;
					for (j = 0; j < 3; j++)
						{
						UINT v = dp[y*p+3*x+j];
						if (j == which)				// if blue
							v = (v * f + z) / 255;	// boost
						else
							v = (v * g) / 255;		// reduce
						dp[y*p+3*x+j] = v;
						}
					}
				}
			}
		m_CamState = 0;
		return dp;
		}
	if (m_jpeg_quality)
		{
		if (!m_pFlags[Frame])
			GetJpegImage(Frame);
		dp = m_pJpeg;
		}
	else
		{
		dp = m_pCache[Frame];
		}

	if (m_pFlags[Frame] && (m_depth == 3))
		{
		UINT w = ComW();
		UINT h = ComH();
		UINT y;
		UINT cx, cy;
		cx = w / 2;
		cy = h / 2;
		UINT hh = h / 3;
		UINT pp = 4 * ((3 * w + 3) / 4);
		BYTE * p = dp;//m_pCache[Frame];
	//	memset(p,255,pp*h);
		for (y = 0; y < hh; y++)
			{
			p[pp * (cy - y) + 3 * (cx - y)+2] = 255;
			p[pp * (cy - y) + 3 * (cx - y)+1] = 0;
			p[pp * (cy - y) + 3 * (cx - y)+0] = 0;
			p[pp * (cy - y) + 3 * (cx + y)+2] = 255;
			p[pp * (cy - y) + 3 * (cx + y)+1] = 0;
			p[pp * (cy - y) + 3 * (cx + y)+0] = 0;
			p[pp * (cy + y) + 3 * (cx - y)+2] = 255;
			p[pp * (cy + y) + 3 * (cx - y)+1] = 0;
			p[pp * (cy + y) + 3 * (cx - y)+0] = 0;
			p[pp * (cy + y) + 3 * (cx + y)+2] = 255;
			p[pp * (cy + y) + 3 * (cx + y)+1] = 0;
			p[pp * (cy + y) + 3 * (cx + y)+0] = 0;
			}
		}
	return dp;//m_pCache[Frame];
}

void CSceneBase::GetGray(HPBYTE hpDst, UINT Frame, UINT Level)
{
	if (Level == -1)
		Level = m_CurLevel;
//	GetImage(hpDst,Frame,Level,CCell::LAYER_GRAY);
	if (GetCell32(Frame, Level, TRUE))
		return;
	BYTE * hpTmp = m_pCellCache[0].pData;
	UINT x,y,p,z;
	ASSERT(m_depth == 1);
	UINT w = m_width;
	UINT h = m_height;
	z = 255;
	p = 4 * ((w + 3) / 4);
	for (y = 0; y < h; y++)
		{
		for (x = 0; x < w; x++)
			{
			hpDst[x] = z ^ hpTmp[x];
			}
		hpDst += p;
		hpTmp += p;
		}
}

UINT CSceneBase::CellInfo(HPBYTE hpDst, UINT Frame, UINT Level, bool bHold,
				UINT & w, UINT & h, UINT & kkey)
{
	if (Level)
		{
		if (GetCell32(Frame, Level,bHold))
			return 0;
		w = m_pCellCache[0].iw;
		h = m_pCellCache[0].ih;
		UINT d = m_depth == 3 ? 4 : 1;
		kkey = 0;
		if (hpDst)
			{
			BYTE * hpTmp = m_pCellCache[0].pData;
			memmove(hpDst, hpTmp, d * w * h);
			}
		return 1;
		}
	UINT which;
	DWORD key;
	for (;;)
		{
		which = CCell::LAYER_BG;
		GetImageKey(key, Frame, 0, which);
		if (key)
			break;
		if (!bHold)
			break;
		if (!Frame)
			break;
		Frame--;
		}
	if (!key)
		return 0;
	UINT iw,ih,id;
	if (ImageInfo(iw,ih,id, key))
		{
DPZ("bad info");
		return 0;
		}
	if (iw > 8192)
		return 0;
	if ((id != 24) && !Level)
		return 0;
	w = iw;
	h = ih;
	kkey = key;
	return 1;
}

void CSceneBase::FetchCell(HPBYTE hpDst, UINT Frame, UINT Level, bool b32,
						bool bUseGray, bool bHold /* = 0 */)
{
	if (Level == -1)
		Level = m_CurLevel;
	if (!Level)
		{
		if (!b32)
			{
			GetLevel0(hpDst,Frame,1, 100,0,0);
			}
		return;
		}
	if (GetCell32(Frame, Level,bHold))
		{
		UINT op;
		if (m_depth == 1)
			{
			op = 4 * ((m_width+3) / 4);
			memset(hpDst, 0, m_height * op);
			}
		else
			{
			UINT ox,oy;
			op = 4 * m_width;
			for (oy = 0; oy < m_height; oy++)
			for (ox = 0; ox < m_width ; ox++)
				{
				hpDst[op*oy+4*ox+0] = 255;
				hpDst[op*oy+4*ox+1] = 255;
				hpDst[op*oy+4*ox+2] = 255;
				hpDst[op*oy+4*ox+3] = 0;
				}
			}
		return;
		}
	BYTE * hpTmp = m_pCellCache[0].pData;
	UINT w = m_width;
	UINT h = m_height;
	UINT iw = m_pCellCache[0].iw;
	UINT ih = m_pCellCache[0].ih;
	UINT ow,oh,offx, offy;
	ow = w;
	oh = h;
	if (w * ih > h * iw)
		{
		offy = 0;
		ow = MulDiv(h, iw, ih);
		offx = (w - ow) / 2;
		}
	else
		{
		offx = 0;
		oh = MulDiv(ih, ow, iw);
		offy = (h - oh) / 2;
		}

	UINT op,ip;
	UINT d;
	if (m_depth == 1)
		{
		d = 1;
		UINT x, y;
		ip = 4 * ((iw + 3) / 4);
		op = 4 * ((w + 3) / 4);
		for (y = 0; y < oh; y++)
		for (x = 0; x < ow; x++)
			{
			UINT ix, iy;
			ix = (x * iw) / ow;
			iy = (y * ih) / oh;
			if (bUseGray)
				hpDst[(y+offy)*op+x+offx] = hpTmp[iy*ip+ix];
			else
				hpDst[(y+offy)*op+x+offx] = 255 - hpTmp[iy*ip+ix];
			}
		}
	else
		{
		UINT x, y;
		if (b32)
			d = 4;
		else
			d = 3;
		op = 4 * ((d * w + 3) / 4);
		ip = 4 * iw;
		for (y = 0; y < oh; y++)
		for (x = 0; x < ow; x++)
			{
			UINT ix, iy;
			ix = (x * iw) / ow;
			iy = (y * ih) / oh;
			hpDst[(y+offy)*op+d*(x+offx)+0] = hpTmp[iy*ip+4*ix+0];
			hpDst[(y+offy)*op+d*(x+offx)+1] = hpTmp[iy*ip+4*ix+1];
			hpDst[(y+offy)*op+d*(x+offx)+2] = hpTmp[iy*ip+4*ix+2];
			if (b32)
				hpDst[(y+offy)*op+d*(x+offx)+3] = hpTmp[iy*ip+4*ix+3];
			}
		}
	if (m_uState && !bUseGray)
		ApplyImprint(hpDst,d);
}

void CSceneBase::GetCell(HPBYTE hpDst, UINT Frame, UINT Level)
{
	if (Level == -1)
		Level = m_CurLevel;
	if (!Level) return;
	ApplyCell32(hpDst, Frame,Level,0);
}

void CSceneBase::SetLayer(CLayers * pLayer)
{
	m_pLayers = pLayer;
}

UINT CSceneBase::GetLayer(HPBYTE hpDst, UINT Frame, UINT Level, 
			UINT Which, DWORD kkey /* = 0 */)
{
	if (Level == -1)
		Level = m_CurLevel;
	DWORD key;
//	if (m_pLayers && m_pLayers->Fetch(hpDst, Frame, Level, Which))
//		return 0;
	if (kkey)
		GetImageKey(key, kkey, Which);
	else
		GetImageKey(key, Frame, Level, Which);
	if (key)
		{
		if (!ReadImage(hpDst, key))
			return 0;
		}
		/*
	if (!bForge)
		{
		GetImageKey(key, Frame, Level,CCell::LAYER_GRAY);
		if (!key)
			return 1;
		}
		*/
	UINT w = m_width;// / m_scale;
	UINT p = 4 * (( w + 3)/ 4);
	UINT h = m_height;// / m_scale;
	UINT s = p * h;
	UINT x,y;
	if ((Which == CCell::LAYER_PAINT) || (Which == CCell::LAYER_INK))
		{
		for (y = 0; y < h; y++)
			for (x = 0; x < w; x++)
				{
				hpDst[p*y+x] = 0;	// alpha
				hpDst[s + p*y+x] = 0; // index
				}
		return 0;
		}
	return 1;
	if (Which != CCell::LAYER_INK)
		return 1;
	GetGray(hpDst,Frame,Level);
	for (y = 0; y < h; y++)
		for (x = 0; x < w; x++)
			{
			hpDst[p*y+x] ^= 255;
			hpDst[s + p*y+x] = 0;
			}
	return 0;
}
/*
void CSceneBase::ScanCellCache()
{
	BYTE * p = m_pCellCache[0].pData;
	UINT h = m_pCellCache[0].ih;
	UINT w = m_pCellCache[0].iw;
	UINT minx = w;
	UINT miny = h;
	UINT maxx = 0;
	UINT maxy = 0;
	if (m_depth == 1)
		{
		UINT x, y;
		UINT exx = 4 * ((w + 3) / 4) - w;
		for (y = 0; y < h; y++, p += exx)
			{
			UINT ix = minx;
			UINT ax = maxx;
			for (x = 0; x < w; x++)
				if (*p++)
					{
					if (x < minx) 
						minx = x;
					if (x > maxx)
						maxx = x;
					}
			if ((ix != minx) || (ax != maxx))
				{
				if (y < miny) 
					miny = y;
				if (y > maxy)
					maxy = y;
				}
			}
		}
	else
		{
		UINT x,y;
		UINT pitch = 4 * ((3 * w + 3) / 4);
		for (y = 0; y < h; y++, p += pitch)
			{
			UINT ix = minx;
			UINT ax = maxx;
			for (x = 0; x < w; x++)
				if (p[4*x+3])
					{
					if (x < minx) 
						minx = x;
					if (x > maxx)
						maxx = x;
					}
			if ((ix != minx) || (ax != maxx))
				{
				if (y < miny) 
					miny = y;
				if (y > maxy)
					maxy = y;
				}
			}
		}
	m_pCellCache[0].minx = minx;
	m_pCellCache[0].miny = miny;
	m_pCellCache[0].maxx = maxx;
	m_pCellCache[0].maxy = maxy;
}
*/

UINT CSceneBase::GetOverlay(UINT Key)
{
	UINT iw,ih,id;
	BYTE * hpDst = m_pCellCache[0].pData;
	if (ImageInfo(iw,ih,id, Key))
		return 9;
	if (id != 32)
		return 8;
	ASSERT(iw == m_pCellCache[0].iw);
	ASSERT(ih == m_pCellCache[0].ih);
	if (m_depth == 1)
		{
		UINT op = 4 * ((iw + 3) / 4);
		UINT ip = 4 * iw;
		BYTE * tp = new BYTE[ih * ip];
		ReadImage(tp, Key);
		UINT y,x;
		for (y = 0; y < ih; y++)
		for (x = 0; x < iw ; x++)
			{
			UINT z;
			if (z = tp[y * ip + 4 * x + 3])
				{
				UINT v = 30 * tp[y*ip + 4 * x + 2]
						+ 59 * tp[y*ip + 4 * x + 1]
						+ 11 * tp[y*ip + 4 * x + 0];
				v /= 100;
				v = 255 - v;
				hpDst[op*y+x] = v*z/255;
				}
			else
				hpDst[op*y+x] = 0;
			}
		delete [] tp;
		}
	else
		ReadImage(hpDst, Key);
	return 0;
}

UINT CSceneBase::GetLayer32(UINT Frame, UINT Level, DWORD cellkey)
{
	CLevel * pLevel = GetLevelPtr(Level);
	if (!pLevel)
		return 1;
	BYTE * hpDst = m_pCellCache[0].pData;

	COLORREF * pColor = (COLORREF *)hpDst;
	UINT ip = 4 * ((m_width + 3) / 4);
	UINT op;
	if (m_depth == 1)
		op = ip;
	else
		op = 4 * m_width;
	UINT size = m_height * op;
	UINT ox,oy;
	if (m_depth == 1)
		{
		memset(hpDst, 0, size);
		}
	else
		{
		memset(hpDst, 255, size);
		for (oy = 0; oy < m_height; oy++)
		for (ox = 0; ox < m_width ; ox++)
			{
			hpDst[op*oy+4*ox+3] = 0;
			}
		}
//	if (!m_pLayers || m_pLayers-FakeIt(hpDst,Frame, Level))
		{
		CCell * pCell = new CCell(m_pIO);
		CLayers * pLayers = new CLayers;
		pCell->SetKey(cellkey);
		if (!pCell->Read())
			{
DPF("read failure");
			CLevelTable * pTable = pLevel->Table();
pTable->table[0].name[1] = (char)255;
			pLayers->ApplyCell(hpDst,this,
					PalettePtr(pLevel->PalIndex()),pTable, pCell);
			}
		delete pLayers;
		delete pCell;
		}
	return 0;
}

void CSceneBase::ApplyBuff(HPBYTE hpDst, HPBYTE hpSrc, UINT w, UINT h, UINT factor)
{
//	ASSERT(m_depth == 1);
	if (m_depth == 1)
		{
		UINT x,y,p,v,q,z;
		if (factor >= 1000)
			{
			z = 255;
			factor -= 1000;
			}
		else
			z = 0;
		p = 4 * ((w + 3) / 4);
		for (y = 0; y < h; y++)
			{
			for (x = 0; x < w; x++)
				{
				v = z ^ hpSrc[x];
				if (factor == 100)
					{
					q = hpDst[x];
					v = (v * q) / 255;
					hpDst[x] = (BYTE)v;
					}
				else  if (v < 255)
					{
					q = hpDst[x];
					v = 255 - (factor * (255 - v)) / 100;
					v = (v * q) / 255;
					hpDst[x] = (BYTE)v;
					}
				}
			hpDst += p;
			hpSrc += p;
			}
		}	
	else
		{
		UINT x,y,sp,dp;
		dp = 4 * ((3 * w + 3) / 4);
		sp = 4 * w;
		for (y = 0; y < h; y++)
			{
			for (x = 0; x < w; x++)
				{
				UINT z;
				if (z = hpSrc[4*x+3])
					{
					hpDst[3*x+0] = hpSrc[4*x+0];
					hpDst[3*x+1] = hpSrc[4*x+1];
					hpDst[3*x+2] = hpSrc[4*x+2];
					}
				}
			hpDst += dp;
			hpSrc += sp;
			}
		}
}

bool CSceneBase::FindNextCell(UINT & Frame, UINT Level)
{
	CLevel * pLevel = GetLevelPtr(Level);
	if (!pLevel)
		return 0;
	bool bResult = 1;
	UINT nf = pLevel->Next(Frame);
	if (nf == NEGONE)
		bResult = 0;
	else
		Frame = nf;
/*
	UINT start = Frame;
	for (;Frame < m_frames;Frame++)
		{
		if (pLevel->Select(Frame))
			break;
		}
	if (Frame >= m_frames)
		{
		bResult = 0;
		Frame = start;
		}
*/
	return bResult;
}

bool CSceneBase::FindPrevCell(UINT & Frame, UINT Level)
{
	bool bResult = 1;
	CLevel * pLevel = GetLevelPtr(Level);
	if (!pLevel)
		return 0;
	UINT start = Frame;
	for (;;Frame--)
		{
		if (pLevel->Select(Frame))
			break;
		if (!Frame)
			{
			Frame = start;
			bResult = 0;
			break;
			}
		}
	return bResult;
}

void CSceneBase::Apply24(BYTE * hpDst, BYTE * hpSrc, UINT iw, UINT ih, 
				bool bCamera, bool bBroadcast)
{
	UINT w,h,dp,ip;
//	bBroadcast = 0;
//	BYTE * hpTmp;
	UINT code = 0;
	UINT ialpha = 255;
	if (bCamera && m_pCamera)
		ialpha = m_pCamera->m_alpha;
	w = ComW();
	h = ComH();
	dp = 4 * ((m_depth *  w + 3)/4);
	ip = 4 * ((m_depth * iw + 3) / 4);
	UINT r, q, fact;
	int yy,oy,ox, offx, offy, scale;
	memset(hpDst, 255, dp * h);
	offx = 0;
	offy = 0;
	m_bDoCam = 0;
	if (m_CamState && m_pCamera)
		{
		m_CurPeg = m_pCamera->PegFindLevel(0);
		if ((m_CamPegSel == -1) || (m_CurPeg == m_CamPegSel))
			m_bDoCam = 1;
		}
	if (bCamera && m_pCamera)
		{
		code = m_pCamera->Table(m_pXY,w,h,iw,ih,bBroadcast);
		if (code == 1)
			{
			offx = m_pCamera->m_offx;
			offy = m_pCamera->m_offy;
			scale = (int)(100.0 * m_pCamera->Scale());
			}
		else
			{
			q = m_pCamera->m_factor;
			r = m_pCamera->m_radius;
			}
		}
	else if (m_factor != 2)
		{
		fact = m_factor;
		code = 3;
		offx = 0;
		offy = 0;
		}
	if ((code == 2) && bBroadcast)
		{
		if (m_depth == 1)
			Apply24g(hpDst,hpSrc,iw,ih,1);
		else
			Apply24c(hpDst,hpSrc,iw,ih);
		return;
		}
	BYTE * hpBlur = 0;
	if (Broadcast() && bCamera && m_pCamera && m_pCamera->Blur())
		{
		UINT blur = (UINT)((0.0 + m_pCamera->Blur()) / 2);
		if (blur)
			{
			hpBlur = new BYTE[ip * ih];
			BlurX(hpBlur, hpSrc,iw,ih,blur,1,m_depth == 1 ? 1 : 3,ip);
			hpSrc = hpBlur;
			}
		}
	UINT z;
	for (yy = m_y; yy < (int)(m_y+m_h);yy++)
		{
		oy = h - 1 - yy;
		for (ox = m_x; ox < (int)(m_x+m_w); ox++)
			{
			int ax, ay;
			if (code > 2)
				{
				ax = offx + ox * fact;
				ay = offy + oy * fact;
				}
			else if (code > 1)
				{
				ax = m_pXY[yy*w+ox] / q;
				ay = m_pXY[w*h+yy*w+ox] / q;
				}
			else if (code)
				{
				ax = (10000 * ox + offx);
				ay = (10000 * oy + offy);
				ax /= scale;
				ay /= scale;
				}
			else
				{
				ay = oy;
				ax = ox;
				}
			if (((UINT)ax >= iw) || ((UINT)ay >= ih))
				continue;
			z = ialpha;
			if (m_bDoCam)
				m_pCamBuf[w*oy+ox] = m_CurPeg;
			if (m_depth == 1)
				{
				if (z == 255)
					hpDst[dp*oy+ox] = hpSrc[ip*ay+ax];
				else
					{
					WORD v = (255 - z) * hpDst[dp*oy+ox];
					v += z * hpSrc[ip*ay+ax];
					hpDst[dp*oy+ox] = v / 255;
					}
				}
			else
				{
				int j;
				if (z == 255)
					{
					for (j = 0; j < 3; j++)
						hpDst[dp*oy+3*ox+j] = hpSrc[ip*ay+3*ax+j];
					}
				else if (z)
					for (j = 0; j < 3; j++)
					{
					WORD v = (255 - z) * hpDst[dp*oy+3*ox+j];
					v += z * hpSrc[ip*ay+3*ax+j];
					hpDst[dp*oy+3*ox+j] = v / 255;
					}
				}
			}	
		}
	delete [] hpBlur;
}


void CSceneBase::Apply24c(BYTE * hpDst, BYTE * pBuf, UINT iw, UINT ih)
{
	ASSERT(m_depth == 3);
	UINT j,w,h;
	UINT dp;
	UINT s[4];
	UINT yy,oy,ox;
	UINT z;
//	UINT x1,x2,y1,y2,f;
	int x1, y1, x2, y2;
	int xx1, xx2, yy1, yy2;
	int maxx, maxy;
	UINT f, xf, yf, xf1, xf2,yf1,yf2;
	UINT q = m_pCamera ? m_pCamera->m_factor : 1;
	UINT R = m_pCamera ? m_pCamera->m_radius : 1;
//	UINT D = R + R + 1;
	UINT ialpha = m_pCamera ? m_pCamera->m_alpha : 255;
	if (!ialpha)
		return;
	maxx = iw * q - 1;
	maxy = ih * q - 1;
	w = ComW();
	h = ComH();
	UINT ip = 4 * ((3 * iw+3) / 4);
	dp = 4 * ((m_depth*w+3)/4);
	for (yy = m_y; yy < (int)(m_y+m_h);yy++)
		{
		oy = h - 1 - yy;
		for (ox = m_x; ox < (int)(m_x+m_w); ox++)
			{
			int ax, ay;
			s[0] = 0;
			s[1] = 0;
			s[2] = 0;
			ax = m_pXY[yy*w+ox];
			ay = m_pXY[w*h+yy*w+ox];
			x1 = ax - R;
			x2 = ax + R;
			y1 = ay - R;
			y2 = ay + R;
			if ((ax > maxx) || (ay > maxy) || (x2 < 0) || (y2 < 0)
				|| (x1 > maxx) || (y1 > maxy))
				{
				hpDst[dp*oy+3*ox+0] = 255;
				hpDst[dp*oy+3*ox+1] = 255;
				hpDst[dp*oy+3*ox+2] = 255;
				continue;
				}
			if (x1 < 0) x1 = 0;
			if (y1 < 0) y1 = 0;
			if (x2 > maxx) x2 = maxx;
			if (y2 > maxy) y2 = maxy;
			xx1 = x1 / q;
			xx2 = x2 / q;
			yy1 = y1 / q;
			yy2 = y2 / q;
			xf = x2 + 1 - x1;
			yf = y2 + 1 - y1;
			f = xf * yf;
			if ((xx1 == xx2) && (yy1 == yy2))
				{
				Magic3(ip*yy1+3*xx1,f,s,pBuf);
				}
			else if (xx1 == xx2)
				{
				yf1 = q - (y1 % q);
				yf2 = 1 + y2  % q;
				MagicColumn3(xx1,yy1,yy2,xf,q,yf1,yf2,ip,s,pBuf);
				}
			else if (yy1 == yy2)
				{
				xf1 = q - (x1 % q);
				xf2 =  1 + x2 % q;
				MagicLine3(xx1,xx2,yy1,xf1,xf2,yf,q,ip,s,pBuf);
				}
			else
				{
				xf1 = q - (x1 % q);
				xf2 = 1 + x2 % q;
				yf1 = q - (y1 % q);
				yf2 = 1 + y2 % q;
				MagicLine3(    xx1,xx2,yy1,xf1,xf2,yf1,q,ip,s,pBuf);
				for (yy1++; yy1 < yy2; yy1++)
					MagicLine3(xx1,xx2,yy1,xf1,xf2,q  ,q,ip,s,pBuf);
				MagicLine3(    xx1,xx2,yy2,xf1,xf2,yf2,q,ip,s,pBuf);
				}
			s[0] /= f;
			s[1] /= f;
			s[2] /= f;

			if (m_bDoCam)
				m_pCamBuf[w*oy+ox] = m_CurPeg;
			z = ialpha;
			if (z == 255)
				{
				hpDst[dp*oy+3*ox+0] = s[0];
				hpDst[dp*oy+3*ox+1] = s[1];
				hpDst[dp*oy+3*ox+2] = s[2];
				}
			else
				{
				for (j = 0; j < 3; j++)
					{
					WORD v = (255 - z) * hpDst[dp*oy+3*ox+j];
					v += z * s[j];
					hpDst[dp*oy+3*ox+j] = v / 255;
					}
				}
			}
		}
}

void CSceneBase::Apply24g(BYTE * hpDst, BYTE * pBuf, UINT iw, UINT ih,
					bool bBG)
{
	UINT w,h;
	UINT dp;
//	UINT s[2];
	UINT s[1];
	UINT yy,oy,ox;
	UINT z;
	int x1,x2,y1,y2,xx1,xx2,yy1,yy2,maxx,maxy;
	UINT f,xf,yf,xf1, xf2,yf1,yf2;
	UINT q = m_pCamera ? m_pCamera->m_factor : 1;
	UINT R = m_pCamera ? m_pCamera->m_radius : 1;
//	UINT D = R + R + 1;
	maxx = q * iw - 1;
	maxy = q * ih - 1;
	UINT ialpha = m_pCamera ? m_pCamera->m_alpha : 255;
	w = ComW();
	h = ComH();
	UINT ip = 4 * ((iw+3) / 4);
	dp = 4 * ((m_depth*w+3)/4);
	for (yy = m_y; yy < (int)(m_y+m_h);yy++)
		{
		oy = h - 1 - yy;
		for (ox = m_x; ox < (int)(m_x+m_w); ox++)
			{
			int ax, ay;
			s[0] = 0;
//			s[1] = 0;
			
			ax = m_pXY[yy*w+ox];
			ay = m_pXY[w*h+yy*w+ox];
			x1 = ax - R;
			y1 = ay - R;
			x2 = ax + R;
			y2 = ay + R;
			if ((ax > maxx) || (ay > maxy) || (x2 < 0) || (y2 < 0)
				|| (x1 > maxx) || (y1 > maxy))
				{
				if (bBG)
					hpDst[dp*oy+ox] = 0;
				continue;
				}
			if (x1 < 0) x1 = 0;
			if (y1 < 0) y1 = 0;
			if (x2 > maxx) x2 = maxx - 1;
			if (y2 > maxy) y2 = maxy - 1;
			xf = x2 + 1 - x1;
			yf = y2 + 1 - y1;
			xx1 = x1 / q;
			yy1 = y1 / q;
			xx2 = x2 / q;
			yy2 = y2 / q;
			xf = x2 + 1 - x1;
			yf = y2 + 1 - y1;
			f = xf * yf;
			if ((xx1 == xx2) && (yy1 == yy2))
				{
				Magic1(ip*yy1+xx1,f,s,pBuf);
				}
			else if (xx1 == xx2)
				{
				yf1 = q - (y1 % q);
				yf2 = 1 + y2 % q;
				MagicColumn1(xx1,yy1,yy2,xf,q,yf1,yf2,ip,s,pBuf);
				}
			else if (yy1 == yy2)
				{
				xf1 = q - (x1 % q);
				xf2 = 1 + x2 % q;
				MagicLine1(xx1,xx2,yy1,xf1,xf2,yf,q,ip,s,pBuf);
				}
			else
				{
				xf1 = q - (x1 % q);
				xf2 = 1 + x2 % q;
				yf1 = q - (y1 % q);
				yf2 = 1 + y2 % q;
				MagicLine1(    xx1,xx2,yy1,xf1,xf2,yf1,q,ip,s,pBuf);
				for (yy1++; yy1 < yy2; yy1++)
					MagicLine1(xx1,xx2,yy1,xf1,xf2,q  ,q,ip,s,pBuf);
				MagicLine1(    xx1,xx2,yy2,xf1,xf2,yf2,q,ip,s,pBuf);
				}
//			UINT qq = s[1];
//			if (!qq)
//				{
//continue;
//				}
//			z = s[0] / qq;
			z = s[0] / f;
			if (m_bDoCam)
				m_pCamBuf[w*oy+ox] = m_CurPeg;
			if (bBG)
				hpDst[dp*oy+ox] = (BYTE)z;
			else
				{
				z = (UINT)(ialpha * z) / 255;
				if (!z)
					continue;
				UINT q = hpDst[dp*oy+ox];
				q = ((255-z) * q) / 255;
				hpDst[dp*oy+ox] = (BYTE)q;
				}
			}
		}
}

void CSceneBase::Apply32(BYTE * hpDst, BYTE * pBuf, UINT iw, UINT ih)
{
	ASSERT(m_depth > 2);
	UINT j,w,h;
	UINT dp;
	UINT s[5];
	UINT yy,oy,ox;
	UINT z;
	UINT f;
	int x1, x2, y1, y2;
	int xx1, xx2, yy1, yy2;
	UINT xf1, xf2,yf1,yf2;
	UINT xf, yf;
	int q = m_pCamera ? m_pCamera->m_factor : 1;
	int maxy, maxx;
	maxx = q * iw - 1;
	maxy = q * ih - 1;
	int R = m_pCamera ? m_pCamera->m_radius:1;
//	int d = R + R + 1;
	UINT ialpha = m_pCamera ? m_pCamera->m_alpha : 255;
	w = ComW();
	h = ComH();
	UINT ip = 4 * iw;
	dp = 4 * ((m_depth*w+3)/4);
	for (yy = m_y; yy < (int)(m_y+m_h);yy++)
		{
		oy = h - 1 - yy;
		for (ox = m_x; ox < (int)(m_x+m_w); ox++)
			{
			int ax, ay;
			s[0] = 0;
			s[1] = 0;
			s[2] = 0;
			s[3] = 0;
			s[4] = 0;
			ax = m_pXY[yy*w+ox];
			ay = m_pXY[w*h+yy*w+ox];
			x1 = ax - R;
			y1 = ay - R;
			x2 = ax + R;
			y2 = ay + R;
			if ((ax > maxx) || (ay > maxy) || (x2 < 0) || (y2 < 0)
				|| (x1 > maxx) || (y1 > maxy))
					continue;
			if (x1 < 0) x1 = 0;
			if (y1 < 0) y1 = 0;
			if (x2 > maxx) x2 = maxx - 1;
			if (y2 > maxy) y2 = maxy - 1;
			xf = x2 + 1 - x1;
			yf = y2 + 1 - y1;
			xx1 = x1 / q;
			yy1 = y1 / q;
			xx2 = x2 / q;
			yy2 = y2 / q;
			xf = x2 + 1 - x1;
			yf = y2 + 1 - y1;
			f = xf * yf;
			if ((xx1 == xx2) && (yy1 == yy2))
				{
				Magic4(ip*yy1+4*xx1,f,s,pBuf);
				}
			else if (xx1 == xx2)
				{
				yf1 = q - (y1  % q);
				yf2 = 1 + y2 % q;
				MagicColumn4(xx1,yy1,yy2,xf,q,yf1,yf2,ip,s,pBuf);
				}
			else if (yy1 == yy2)
				{
				xf1 = q - (x1 % q);
				xf2 = 1 + x2 % q;
				MagicLine4(xx1,xx2,yy1,xf1,xf2,yf,q,ip,s,pBuf);
				}
			else
				{
				xf1 = q - (x1 % q);
				xf2 = 1 + x2 % q;
				yf1 = q - (y1 % q);
				yf2 = 1 + y2 % q;
				MagicLine4(    xx1,xx2,yy1,xf1,xf2,yf1,q,ip,s,pBuf);
				for (yy1++; yy1 < yy2; yy1++)
					MagicLine4(xx1,xx2,yy1,xf1,xf2,q  ,q,ip,s,pBuf);
				MagicLine4(    xx1,xx2,yy2,xf1,xf2,yf2,q,ip,s,pBuf);
				}
			UINT qq = s[4];
			if (!qq)
				continue;
			s[0] /= qq;
			s[1] /= qq;
			s[2] /= qq;
			s[3] /= f;

			z = ialpha * s[3] / 255;
			if (!z)
				continue;
			if (m_bDoCam)
				m_pCamBuf[oy*w+ox] = m_CurPeg;
			bool bFirst = (m_depth == 4) && !hpDst[dp*oy+4*ox+3] ? 1 : 0;
			if (z == 255 || (bFirst))
				{
				hpDst[dp*oy+m_depth*ox+0] = s[0];
				hpDst[dp*oy+m_depth*ox+1] = s[1];
				hpDst[dp*oy+m_depth*ox+2] = s[2];
				if (m_depth == 4)
					hpDst[dp*oy+m_depth*ox+3] = z;
				}
			else
				{
				if (m_depth != 4)
					{
					for (j = 0; j < 3; j++)
						{
						WORD v = (255 - z) * hpDst[dp*oy+m_depth*ox+j];
							v += z * s[j];
						hpDst[dp*oy+m_depth*ox+j] = v / 255;
						}
					}
				else
					{
					int t;
					UINT b_alpha = hpDst[dp*oy+4*ox+3];
					if (!b_alpha)
						{
						for (j = 0; j < 4; j++)
							hpDst[dp*oy+4*ox+j] = s[j];
						}
					else
						{
						for (j = 0; j < 3; j++)
							{
							UINT back = (UINT)hpDst[dp*oy+4*ox+j] * b_alpha;
							t = (UINT)s[j] * z;
							if (back)
								t +=  back - back * z / 255;
							t /= 255;
							hpDst[dp*oy+4*ox+j] = t;
							}
						t = z + b_alpha - (z * b_alpha) / 255;
						hpDst[dp*oy+4*ox+3] = t;
						}
					}
				}
			}
		}
}

UINT CSceneBase::GetCell32(UINT Frame, UINT Level, bool bSearch)
{
	CLevel * pLevel = GetLevelPtr(Level);
	if (!pLevel)
		return 1;
	UINT code = 0;
	UINT cellkey = 0;
	UINT FakeFrame;
	if (m_oflags & 1)		// no hold
		bSearch = 0;
	UINT Flags = pLevel->Flags();
	if (!(Flags & 1) || !m_pLayers) // not enabled or no layer info
		code = 0;		// normal use image key
	else
		{
			if ((Frame == 1) && (Level == 2))
				code = 0;
		if (m_pLayers->GetFakeFrame(FakeFrame, Level))
			code = 0;
		else if (Frame < FakeFrame)
			code = 0;
		else if (Frame == FakeFrame)
			code = 1;
		else if (bSearch)
			{
			UINT nf = pLevel->Next(FakeFrame+1);
			if ((nf == NEGONE) || (Frame < nf))
				code = 1;
			}
		}
	if (code == 0)
		{
		cellkey = pLevel->Select(Frame,bSearch);
		if (!cellkey)
			return 2;
		}
	else
		cellkey = NEGTWO;
	UINT palindex = 0;
	if (m_depth != 1)
		palindex = pLevel->PalIndex();
DPF("getcell32,b:%d,f:%d,l:%d,k:%d",bSearch,Frame,Level,cellkey);

	UINT i;
	if (code == 0)
		{
		for (i = 0; i < m_nCells; i++)
			{
			if (m_pCellCache[i].dwKey != cellkey)
				continue;
			if (m_pCellCache[i].id != m_depth)
				continue;
			if (m_depth == 1)
				break;				// no need if gray
			if (m_pCellCache[i].palindex != palindex)
				continue; // different palette
			if (m_pCellCache[i].level == Level)
				break;
			}
		if (i < m_nCells)	// found it, move it to front
			{
			if (i)
				{
				CCellEntry t = m_pCellCache[i];
				for (; i > 0; i--)
					m_pCellCache[i] = m_pCellCache[i-1];
				m_pCellCache[0] = t;
				}
//DPZ("found");
DPZ("found, key:%u",m_pCellCache[0].dwKey);
//DPZ("found, minxy:%d,%d",m_pCellCache[0].minx,m_pCellCache[0].miny);
			return 0;	// found and at position 0
			}
		}
	i = m_nCells - 1;
	CCellEntry t = m_pCellCache[i];
	for (; i > 0; i--)
		m_pCellCache[i] = m_pCellCache[i-1];
	m_pCellCache[0] = t;	// make slot at zero

	UINT iw,ih,id;
	DWORD ikey = 0;
	if (code == 0)
		GetImageKey(ikey, cellkey, CCell::LAYER_OVERLAY);
	if (ikey)
		{
		DPZ("overlay");
		if (ImageInfo(iw,ih,id, ikey))
				return 9;
		if (id != 32)
			return 8;
		}
	else
		{
		iw = m_width;
		ih = m_height;
		}
	m_pCellCache[0].dwKey = cellkey;
	if (!m_pCellCache[0].pData ||
				(m_pCellCache[0].id != m_depth) ||
				(m_pCellCache[0].iw != iw) ||
				(m_pCellCache[0].ih != ih))
		{
		delete [] m_pCellCache[0].pData;
		m_pCellCache[0].iw = iw;
		m_pCellCache[0].ih = ih;
		m_pCellCache[0].id = m_depth;
		UINT siz;
		if (m_depth == 1)
			siz = 4 * ((m_pCellCache[0].iw +3) / 4);
		else
			siz = 4 * m_pCellCache[0].iw;
		siz *= m_pCellCache[0].ih;
		m_pCellCache[0].pData = new BYTE[siz];
		}
	if (m_depth == 1)
		m_pCellCache[0].palindex = NEGONE;
	else
		{
		m_pCellCache[0].palindex = palindex;
		m_pCellCache[0].level = Level;
		}
	if (code == 1)
		{
		m_pLayers->FakeIt(m_pCellCache[0].pData);
		}
	else if (ikey)
		GetOverlay(ikey);
	else
		GetLayer32(Frame, Level, cellkey);
DPZ("not found, key:%u",m_pCellCache[0].dwKey);
//DPZ("not found, minxy:%d,%d",m_pCellCache[0].minx,m_pCellCache[0].miny);
	ScanCellCache();
	return 0;
}


UINT CSceneBase::BlowCell(UINT cellkey)
{
	UINT i;
	UINT result = 0;
	for (i = 0; i < m_nCells; i++)
		{
		if (m_pCellCache[i].dwKey == cellkey)
			{
			result = cellkey;
			m_pCellCache[i].dwKey = -1;
			}
		}
	return result;
}

UINT CSceneBase::BlowCell(UINT Frame, UINT Level)
{
	if (Frame == -1)
		{
		UINT i;
		for (i = 0; i < m_nCells; i++)
			m_pCellCache[i].dwKey = -1;
		return 0;
		}
	if (Frame == -2)
		{
		UINT i;
		for (i = 0; i < m_nCells; i++)
			{
			if (m_pCellCache[i].level == Level)
				m_pCellCache[i].level = NEGONE;
			}
		return 0;
		}
	if (Frame == -3)
		{
		UINT i;
		for (i = 0; i < m_nCells; i++)
			{
			if (m_pCellCache[i].palindex == Level)
				m_pCellCache[i].palindex = NEGONE;
			}
		return 0;
		}
	DWORD cellkey = GetCellKey(Frame, Level);
DPZ("blowcell,f:%d,l:%d,k:%d",Frame,Level,cellkey);
	if (cellkey)
		BlowCell(cellkey);
	return cellkey;
}

void CSceneBase::ApplyCell(HPBYTE hpDst,
			UINT Frame, UINT Level,
				bool bCamera /* = FALSE */, bool bBroadcast/* = 0 */)
{
//DPF("apply cell,frm:%d,lev:%d,fac:%d",Frame,Level,factor);
	UINT w,h,dp,ip;
//	bBroadcast = 0;
//	bCamera = 0;
	if (GetCell32(Frame, Level, TRUE))
		return;
	BYTE * hpTmp = m_pCellCache[0].pData;
	UINT code = 0;
	UINT ialpha = 255;
	if (bCamera && m_pCamera)
		bCamera = m_pCamera->SetupCell(Frame, Level);
	if (bCamera && m_pCamera)
		ialpha = m_pCamera->m_alpha;
	UINT iw = m_pCellCache[0].iw;
	UINT ih = m_pCellCache[0].ih;
	if (m_depth == 1)
		ip = 4 * ((iw +3)/4);
	else
		ip = 4 * iw;
	w = ComW();
	h = ComH();
	dp = 4 * ((m_depth*w+3)/4);
	UINT r, q, fact;
	int yy,oy,ox, offx, offy, scale;
//	UINT ww, hh;
//			COLORREF * pColor = (COLORREF *)hpTmp;
	offx = 0;
	offy = 0;
	m_bDoCam = 0;
	if (m_CamState && m_pCamera)
		{
		m_CurPeg = m_pCamera->PegFindLevel(Level);
		if ((m_CamPegSel == -1) || (m_CurPeg == m_CamPegSel))
			m_bDoCam = 1;
		}
	if (bCamera && m_pCamera)
		{
		code = m_pCamera->Table(m_pXY,w,h,iw,ih,bBroadcast);
		if (code == 1)
			{
			offx = m_pCamera->m_offx;
			offy = m_pCamera->m_offy;
			scale = (int)(100.0 * m_pCamera->Scale());
//if (!scale)
//	{
//	scale = 10;
//	}
			}
		else
			{
			q = m_pCamera->m_factor;
			r = m_pCamera->m_radius;
			}
		}
	else if (m_factor != 2)
		{
		fact = m_factor;
		code = 3;
		offx = 0;
		offy = 0;
		}
	if ((code == 2) && bBroadcast)
		{
		if (m_depth == 1)
			Apply24g(hpDst,hpTmp,iw,ih,0);
		else
			Apply32(hpDst,hpTmp,iw,ih);
		return;
		}
DPZ("applying");
	BYTE * hpBlur = 0;
	if (Broadcast() && bCamera && m_pCamera && m_pCamera->Blur())
		{
		UINT blur = (UINT)((0.0 + m_pCamera->Blur()) / 2);
		if (blur)
			{
			hpBlur = new BYTE[ip * ih];
			BlurX(hpBlur, hpTmp,iw,ih,blur,1,m_depth == 1 ? 1 : 4,ip);
			hpTmp = hpBlur;
			}
		}
	UINT z;
	for (yy = m_y; yy < (int)(m_y+m_h);yy++)
		{
		oy = h - 1 - yy;
		for (ox = m_x; ox < (int)(m_x+m_w); ox++)
			{
			int ax, ay;
			if (code > 2)
				{
				ax = offx + ox * fact;
				ay = offy + oy * fact;
				}
			else if (code > 1)
				{
				ax = m_pXY[yy*w+ox] / q;
				ay = m_pXY[w*h+yy*w+ox] / q;
				}
			else if (code)
				{
				ax = (10000 * ox + offx);
				ay = (10000 * oy + offy);
				ax /= scale;
				ay /= scale;
				}
			else
				{
				ay = oy;
				ax = ox;
				}
			if (((UINT)ax >= iw) || ((UINT)ay >= ih))
				continue;
			if (m_depth == 1)
				{
				z = (UINT)(ialpha * hpTmp[ip*ay+ax]) / 255;
				if (!z)
					continue;
				UINT q = hpDst[dp*oy+ox];
				q = ((255-z) * q) / 255;
						hpDst[dp*oy+ox] = (BYTE)q;
//						hpDst[dp*oy+ox] = 255-z;
				if (m_bDoCam)
					m_pCamBuf[w*oy+ox] = m_CurPeg;
				}
			else
				{
				int j;
				z = (UINT)(ialpha * hpTmp[ip*ay+4*ax+3]) / 255;
				if (!z)
					continue;
				bool bFirst = (m_depth == 4) && !hpDst[dp*oy+4*ox+3] ? 1 : 0;
				if ((z == 255) || bFirst)
					{
//					for (j = 0; j < 3; j++)
//						hpDst[dp*oy+m_depth*ox+j] = hpTmp[ip*ay+4*ax+j];
					hpDst[dp*oy+m_depth*ox+0] = hpTmp[ip*ay+4*ax+0];
					hpDst[dp*oy+m_depth*ox+1] = hpTmp[ip*ay+4*ax+1];
					hpDst[dp*oy+m_depth*ox+2] = hpTmp[ip*ay+4*ax+2];
					if (m_depth == 4)
						hpDst[dp*oy+m_depth*ox+3] = z;
					}
				else
					{
					for (j = 0; j < 3; j++)
						{
						WORD v = (255 - z) * hpDst[dp*oy+m_depth*ox+j];
							v += z * hpTmp[ip*ay+4*ax+j];
							hpDst[dp*oy+m_depth*ox+j] = v / 255;
						}
					}
				if (m_bDoCam)
					m_pCamBuf[w*oy+ox] = m_CurPeg;
				}
			}
		}	
	delete [] hpBlur;
}

void CSceneBase::CombineLayers(BYTE * pBuf, 
				UINT StartLevel, UINT EndLevel, UINT Frame)
{
	if (!StartLevel)
		StartLevel++;
	bool bFill = 1;
	for (; StartLevel <= EndLevel;StartLevel++)
		if (LevelFlags(StartLevel) & 1)
			{
			ApplyCell32(pBuf, Frame, StartLevel,bFill);
			bFill = 0;
			}
	if (m_uState)
		ApplyImprint(pBuf,m_depth == 1 ? 1 : 4);
}

void CSceneBase::ApplyCell32(HPBYTE hpDst, UINT Frame, UINT Level, bool bFill)
{
	UINT w,h,op,ip,x,y;
	if (GetCell32(Frame, Level, TRUE))
		{
		if (bFill)
			{
			UINT op;
			if (m_depth == 1)
				{
				op = 4 * ((m_width+3)/4);
				memset(hpDst,0,op * m_height);
				}
			else
				{
				UINT ox,oy;
				op = 4 * m_width;
				for (oy = 0; oy < m_height; oy++)
				for (ox = 0; ox < m_width ; ox++)
					{
					hpDst[op*oy+4*ox+0] = 255;
					hpDst[op*oy+4*ox+1] = 255;
					hpDst[op*oy+4*ox+2] = 255;
					hpDst[op*oy+4*ox+3] = 0;
					}
				}
			}
		return;
		}
	BYTE * hpTmp = m_pCellCache[0].pData;
	UINT iw = m_pCellCache[0].iw;
	UINT ih = m_pCellCache[0].ih;
	w = m_width;
	h = m_height;
	if (m_depth == 1)
		{
		ip = 4 * ((iw+3)/4);
		op = 4 * ((w+3)/4);
		UINT z,q;
		for (y = 0; y < h; y++)
		for (x = 0; x < w; x++)
			{
			if ((y >= ih) || (x >= iw))
				continue;
			z = hpTmp[ip*y+x];
			if (!z)
				continue;
			q = hpDst[op*y+x];
			q = ((255-z) * q) / 255;
			hpDst[op*y+x] = (BYTE)q;
			}
		}
	else
		{
		ip = 4 * iw;
		op = 4 * w;
//bFill = 0;
		for (y = 0; y < h;y++)
		for (x = 0; x < w; x++)
			{
if ((x >= iw) || (y >= ih))
	continue;
			UINT f_alpha;
			if (bFill)
				{
				if(hpTmp[ip*y+4*x+3])
					{
					hpDst[op*y+4*x+0] = hpTmp[ip*y+4*x+0];
					hpDst[op*y+4*x+1] = hpTmp[ip*y+4*x+1];
					hpDst[op*y+4*x+2] = hpTmp[ip*y+4*x+2];
					hpDst[op*y+4*x+3] = hpTmp[ip*y+4*x+3];
					}
				else
					{
					hpDst[op*y+4*x+0] = 255;
					hpDst[op*y+4*x+1] = 255;
					hpDst[op*y+4*x+2] = 255;
					hpDst[op*y+4*x+3] = 0;
					}
				}
			else if (f_alpha = hpTmp[ip*y+4*x+3])
				{
				int j,t;
				UINT b_alpha = hpDst[op*y+4*x+3];
				if (!b_alpha)
					{
					for (j = 0; j < 4; j++)
						hpDst[op*y+4*x+j] = hpTmp[ip*y+4*x+j];
					}
				else
					{
					for (j = 0; j < 3; j++)
						{
						UINT back = (UINT)hpDst[op*y+4*x+j] * b_alpha;
						t = (UINT)hpTmp[ip*y+4*x+j] * f_alpha;
						if (back)
							t +=  back - back * f_alpha / 255;
						t /= 255;
						hpDst[op*y+4*x+j] = t;
						}
					t = f_alpha + b_alpha - (f_alpha * b_alpha) / 255;
					hpDst[op*y+4*x+3] = t;
					}
				}
			}
		}
}

void CSceneBase::PutImage(HPBYTE hpDst, UINT Frame, UINT Level, UINT Which)
{
	if (Level == -1)
		Level = m_CurLevel;
DPF("put image,f:%d,l:%d,w:%d",Frame,Level,Which);
	DWORD key;
	GetImageKey(key, Frame, Level,Which,TRUE);
	if (key == 0)
		{
DPF("null key after bmake");
		return;
		}
	else
		WriteImage(hpDst, key, Which);
	return ;
}

void CSceneBase::PutMono(HPBYTE hpDst, UINT Frame, UINT Level)
{
	PutImage(hpDst, Frame,Level,CCell::LAYER_MONO);
}


void CSceneBase::SetMinBG(UINT min, bool bInit)
{
	m_MinBG = min;
	if ((LevelFlags(0) & 1) && !bInit)
		UpdateCache();
}
UINT CSceneBase::UpdateCache(UINT Frame /* = 0 */, UINT Level /* = 9999 */,
			UINT Count /* = 1 */)
{
	if (Level == 9999)
		{
		m_cache_state = 0;
DPF("update entire cache");
		for (; Frame < m_frames; m_pFlags[Frame++] = 1);
		return Frame;
		}
	if (Level == 9998)
		{
		Count++;
		if (Count	 >= m_frames)
			Count = m_frames;
		m_cache_state = 0;
		for (; Frame < Count; m_pFlags[Frame++] = 1);
		return Frame;
		}
	if ((Count != 9999) && !(LevelFlags(Level) & 1))
		return Frame;
	if ((Frame + Count) >= m_frames)
		Count = m_frames - Frame;
	UINT i;
	m_cache_state = 0;
	for (i = 0; i < Count;i++)
		m_pFlags[Frame++] = 1;
	DWORD key;
	for (;Frame < m_frames;Frame++)
		{
		GetImageKey(key, Frame, Level, CCell::LAYER_GRAY);
		if (key)
			break;
		GetImageKey(key, Frame, Level, CCell::LAYER_INK);
		if (key)
			break;
		m_pFlags[Frame] = 1;	// dirty
		m_cache_state = 0;
		}
	return Frame;
}

void CSceneBase::PutLayer(HPBYTE hpDst, UINT Frame, UINT Level, UINT Which)
{
	PutImage(hpDst, Frame,Level,Which);
}

void CSceneBase::XThumb(HPBYTE hpDst, UINT w, UINT h, UINT bpp, UINT Level)
{
	UINT j,i,p,v,q;
	DPZ("got blank,w:%d,h:%d",w,h);
	p = 4 * ((w * bpp + 3) / 4);
	if (bpp == 1 || !Level)
		v = 255;
	else
		v = 0;
	memset(hpDst, v, h*p);
	for (q = 0; q < 4; q++)
		{
		int x,y,dx, dy;
		dx = 1 - 2 * (q & 1);
		dy = 1 - (q & 2);
		x = w / 2;
		y = h / 2;
		for (i = 0; i < (h / 3); i++)
			{
			for (j = 0; j < bpp; j++)
				{
				if (j == 3)
					v = 255;
				else
					v = 0;
				hpDst[p * y + bpp*x + j] = v;
				hpDst[p * y + bpp*(x+dx) + j] = v;
				}
			x += dx;
			y += dy;
			}
		}
	return;
}

void CSceneBase::ThumbMinMax(UINT & min, UINT & max, UINT Frame, UINT Level)
{
	min = -1;
	max = m_frames;
	CLevel * pLevel = GetLevelPtr(Level);
	if (pLevel)
		{
		pLevel->MinMax(min, max, Frame);
		}
}


/*
	routine for finds cells for lightbox in straight ahead mode
*/
UINT CSceneBase::Before(UINT * pList, UINT max, UINT Frame, UINT Level)
{
	UINT res = 0;
	CLevel * pLevel = GetLevelPtr(Level);
	if (pLevel)
		{
		res = pLevel->Before(pList,max,Frame);
		}
	return res;
}

int CSceneBase::TopLevel(UINT Frame)
{
	int level = m_levels;
	for (;level-- > 0;)
		{
		CLevel * pLevel = GetLevelPtr(level);
		if (pLevel)
			{
			DWORD key = pLevel->Select(Frame,0);
			if (key)
				break;
			}
		}
	return level;
}


UINT CSceneBase::GetThumb(HPBYTE hpDst, UINT Frame, UINT Level, 
			UINT w, UINT h, UINT bpp, bool bForge /* = 0 */)
{
//return 0;
	bool bNoHold = m_oflags & 1;
	if (Level == -1)
		Level = m_CurLevel;
//	Level = 1;
DPF("get thumb,f:%d,l:%d,w:%d,h:%d,bpp:%d,forge:%d",Frame,Level,w,h,bpp, bForge);
	DWORD key;
#ifdef _DEBUG
	if (Frame == 1 && Level == 0)
	{
		w = w;
	}
#endif
//	GetImageKey(key, Frame, Level,CCell::LAYER_THUMB);
	GetImageKey(key, Frame, Level,99);
	if (key == 1)
		{
		XThumb(hpDst, w, h, bpp, Level);
		return 0;
		}
	if ((key == 0) && bForge)
		{
		if (Level)
			{
//			GetImageKey(key, Frame, Level,CCell::LAYER_INK);
			GetImageKey(key, Frame, Level,98);
			if (!key)
				GetImageKey(key,Frame,Level,CCell::LAYER_OVERLAY);
			}
		else
			GetImageKey(key, Frame, Level,CCell::LAYER_BG);
		if (!key && GetCellKey(Frame, Level))
			{
			XThumb(hpDst, w, h, bpp, Level);
			return 0;
			}
		if (key)
			GetImageKey(key, Frame, Level,CCell::LAYER_THUMB,1);
		}
	if (key == 0)
		{
		UINT z = 0;
		UINT p = bpp > 1 ? 3 : 1;
		UINT y,v;
		UINT min, max;
/*
		min = max = Frame;
		for (;min--;)
			{
			GetImageKey(key, min, Level,CCell::LAYER_THUMB);
			if (key)
				break;
			}
		if (key)
			{
			UINT w,h,d;
			ImageInfo(w,h,d,key);
			if (w > 1)
				z = 1;
			}
		if (z)
			{
			key = 0;
			for (;++max < m_frames;)
				{
				GetImageKey(key, max, Level,CCell::LAYER_THUMB);
				if (key)
					break;
				}
			if (key)
				max--;
			}
*/

		z = bNoHold ? 0 : 1;
		ThumbMinMax(min, max, Frame, Level);
		if (bpp == 1 || !Level)
			{
			p = 4 * ((w * p + 3) / 4);
			v = 255; // white for bg
			}
		else
			{
			v = 0;
			p = 4 * w;
			}
		if (Frame == (min+1))
			min = h / 3;
		else
			min = 0;
		if ((Frame + 1) == max)
			max = h - h / 3;
		else
			max = h;
//return 0;
		UINT ww = w / 2;
		for (y = 0; y < h; y++)
			{
			memset(hpDst, v, p);
			if (z && ((h-1-y) >= min) && ((h-1-y) < max))
				{
				if (!v)
					hpDst[4 * ww + 3] = XTHUMB;
				else if (bpp > 1)
					{
					hpDst[3 * ww + 0] = 255-XTHUMB; // bg line
					hpDst[3 * ww + 1] = 255-XTHUMB;
					hpDst[3 * ww + 2] = 255-XTHUMB;
					}
				else
					hpDst[ww + 0] = XTHUMB;
				}
			hpDst += p;
			}
		return 0;
		}
DPF("reading thumb");
	CMyImage * pImage = new CMyImage(m_pIO);
	if (pImage == NULL)
		{
DPF("new thumb read failure");
		return 0;
		}
	pImage->SetKey(key);
	if (pImage->Read(NULL))
		{
DPF("read thumb header failure");
		delete pImage;
		return 0;
		}
#ifndef NEWTHUMBS
	if ((pImage->Width() == w) && (pImage->Height() == h) &&
			(pImage->Depth() == 8*bpp) && !bForge)
		{
DPF("reading existing thumb");
		int result = pImage->Read(hpDst);
		delete pImage;
		return result;
		}
	else
#endif
		{
DPF("no match,w:%d,h:%d",pImage->Width(),pImage->Height());
		}

/*
	read header
	if w and h match read it
	else
		get gray
		make thumb
*/
	UINT gw = m_width;// / m_scale;
	UINT gh = m_height;// / m_scale;
	UINT size = gh * 4 * ((bpp*gw+3)/ 4);
	BYTE * buf = new BYTE[size];
UINT zize = m_size;
UINT zpp = m_depth;
m_depth = bpp;
m_size = size;
	if (!Level)
		GetLevel0(buf,Frame,0,100,0,0);
	else
		{
		memset(buf, bpp == 1 ? 255 : 0, m_size);
		GetCell(buf,Frame, Level);
		}
m_depth = zpp;
m_size = zize;

	CGScaler scale;
	if (scale.Init(gw,gh,8*bpp,w,h))
		{
DPF("scale failure");
		delete pImage;
		return 0;
		}
	int p = 4 * ((bpp*w + 3) / 4);
//	int p = bpp*w;
	int q = scale.Custom(hpDst, buf, p,4 * ((bpp*m_width+3)/4));
DPF("after custom:%d",q);
	int z = scale.Copy();
DPF("after scale:%d",z);
	pImage->Setup(w, h, 8*bpp,1); // compress thumb
	pImage->Write(hpDst);
	m_bModified = TRUE;

	delete [] buf;
	delete pImage;
	return 0;

}
/*
UINT CSceneBase::MakeThumb(HPBYTE hpSrc, UINT Frame, UINT Level)
{
DPF("making thumb,f:%d,l:%d",Frame,Level);
	UINT p = 4 * ((m_thumbw + 3) / 4);
	UINT size = m_thumbh * p;
	BYTE * buf = new BYTE[size];
	HPBYTE hpTmp = buf;
	CGScaler scale;
	UINT w = m_width;// / m_scale;
	UINT h = m_height;// / m_scale;
	if (scale.Init(w, h,1,m_thumbw,m_thumbh))
		{
DPF("scale failure");
		delete [] buf;
		return 0;
		}
	int q = scale.Custom(buf, hpSrc, p);
DPF("after custom:%d",q);
	int z = scale.Copy();
DPF("after scale:%d",z);
	PutImage(buf, Frame,Level,CCell::LAYER_THUMB);
	delete [] buf;
	return 0;
}
*/
UINT CSceneBase::BlankThumb(UINT Frame, UINT Level)
{
	return 0;
DPF("blanking thumb,f:%d,l:%d",Frame,Level);
	BYTE  buf[10];
	DWORD key;
	GetImageKey(key, Frame, Level,CCell::LAYER_THUMB,TRUE);
	if (key == 0)
		{
DPF("null key after bmake");
		return 1;
		}
	else
		WriteImage(buf, key, 99);	// kludge for blank
	return 0;
}

#ifdef MAKEWATERMARK
int CSceneBase::MakeImprint()
{
	CFile file;

	DWORD mode = CFile::modeRead;
#ifdef TGA
//	if (!file.Open("c:\\flipbook\\camskt\\watermark432.tga", mode))
#ifdef THE_DISC
	if (!file.Open("c:\\flipbook\\camskt\\Th DISC! 2007RP_startup.tga", mode))
#else
	if (!file.Open("c:\\flipbook\\camskt\\newwatermark.tga", mode))
#endif
#else
	if (!file.Open("h:\\newskt\\watermar\\watermark.bmp", mode))
#endif
		{
DPF("no open");
		return 88;
		}
	DWORD ss = file.GetLength();
	BYTE * fp = new BYTE[ss];
	if (!fp)
		{
		file.Close();
DPF("no mem");
		return 87;
		}
	UINT osiz = file.ReadHuge(fp, ss);
	file.Close();
#ifndef TGA
	osiz -= 14;
#endif
	DWORD dsize = 20 + (osiz * 102) / 100;
	BYTE * tbuf = new BYTE[dsize];
	if (tbuf == NULL)
		{
		delete [] fp;
DPF("compress mem failure ");
		return 85;
		}
	DWORD * dp = (DWORD *)tbuf;
	dp[0] = 0;
	dp[1] = osiz;
#ifdef TGA
	UINT cq = compress(tbuf+12,&dsize,fp,osiz);
#else
	UINT cq = compress(tbuf+12,&dsize,fp+14,osiz);
#endif
	delete fp;
	if (cq)
		{
DPF("compression failure:%d",cq);
		delete tbuf;
		return 1;
		}
	dp[2] = dsize;
DPF("compressed size:%d",dsize);
	mode = CFile::modeWrite | CFile::modeCreate;
	if (!file.Open("c:\\flipbook\\camskt\\watermk4.xyz", mode))
		{
DPF("no create");
		delete [] tbuf;
		return 86;
		}
	file.WriteHuge(tbuf, dsize+12);
	file.Close();
	delete [] tbuf;
	return 66;
}
#endif

int CSceneBase::ForgeImprint(UINT ow, UINT oh)
{
//	UINT ow = ComW();
//	UINT oh = ComH();

	if (m_pImprint)
		delete [] m_pImprint;
#ifdef TGA
	if (m_depth == 1)
		m_pImprint = new BYTE[oh* 4 * ((ow + 3) / 4)];
	else
		m_pImprint = new BYTE[oh * ow * 4];
#else
	m_pImprint = new BYTE[oh*ow*m_depth;
#endif
#ifdef MAKEWATERMARK
	MakeImprint();
	return 99;
#endif
	int result = 0;

	auto resourcePtr = m_pResourceLoader->LoadImprint();
	if (!resourcePtr) {
		DPF("cannot find imprint");
		return 11;
	}

	UINT * pData = (UINT *)resourcePtr.get();
	UINT t = SWAPV(pData[0]);
	UINT ins = SWAPV(pData[1]);
	UINT outs = SWAPV(pData[2]);
	if (t)
		{
		return 14;
		}
	DPF("imprint,ins:%d,outs:%d",ins,outs);
	BYTE * hpDst = new BYTE[ins];
	if (!hpDst)
		{
		return 15;
		}
	unsigned long vc = ins;
	UINT qq = uncompress(hpDst, &vc, (BYTE*)(resourcePtr.get()) + 12, outs);
	if (qq)
		{
DPF("decompress error:%d",qq);
		delete [] hpDst;
		return 16;
		}
	UINT iw;
	UINT ih;
	UINT Depth;
#ifdef TGA
//	WORD * pw = (WORD *)hpDst;
	Depth = 0;
	iw = hpDst[12] + 256 * hpDst[13];//pw[6];
	ih = hpDst[14] + hpDst[15] * 256;//pw[7];
//	if ((pw[1] == 2) && ((pw[8] / 256) == 8))
	if ((hpDst[2] == 2) && (hpDst[17] == 8))
		Depth = hpDst[16];//pw[8] & 255;
#else
	LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)hpDst;
	iw = lpBI->biWidth;
	ih = lpBI->biHeight;
	Depth = lpBI->biBitCount;
#endif
	DPF("src %d,%d,%d", iw,ih,Depth);
	if ((Depth != 8) && (Depth != 24) && (Depth != 32))
		{
DPF("bad depth:%d",Depth);
		delete [] hpDst;
		return 19;
		}
	UINT bpp = Depth / 8;
	CGScaler scale;
	BYTE * pTemp;
	if (m_depth != 3)
#ifdef TGA
		pTemp = new BYTE[oh * 4 * ow];
#else
		pTemp = new BYTE[oh * 4 * ((3 * ow + 3) / 4)];
#endif
	else
		pTemp = m_pImprint;
	if (scale.Init(iw,ih,Depth,ow,oh))
		{
DPF("scale failure");
		delete [] hpDst;
		return 17;
		}
	int p = 4 * ((bpp*ow + 3) / 4);
	BYTE * pdst = hpDst;
#ifdef TGA
	pdst += 18;
#else
	pdst += 40;
	if (bpp == 1)
		pdst += 1024;
#endif
	int q = scale.Custom(pTemp, pdst, p, 4 * ((bpp*iw+3)/ 4));
DPF("after custom:%d",q);
	int z = scale.Copy();
DPF("after scale:%d",z);
	delete [] hpDst;
	UINT ip;
#ifdef TGA
		ip = 4 * ow;
#else
		ip = 4 * ((3 * ow + 3) / 4);
#endif
	if (m_depth != 3)
		{
		UINT x, y, op;
		op = 4 * ((m_depth * ow + 3) / 4);
		for (y = 0; y < oh; y++)
		for (x = 0; x < ow; x++)
			{
#ifdef TGA
			m_pImprint[y*op+x] = 255 - 
					pTemp[y*ip+4*x+3] * IMPRINT_ALPHA / 100;
#else
			UINT v = 30 * pTemp[y*ip+3*x+1] +
					59 * pTemp[y*ip+3*x+0] +
					11 * pTemp[y*ip+3*x+2];
			m_pImprint[y*op+x] = v / 100;
#endif
			}
		delete [] pTemp;
		}
	else
		{
		UINT x, y;
		for (y = 0; y < oh; y++)
		for (x = 0; x < ow; x++)
			{
			m_pImprint[y*ip+4*x+3] = 
				m_pImprint[y*ip+4*x+3] * IMPRINT_ALPHA / 100;
			}
		}
	return result;
}

//
// 0 is read new, 1 read old, 2 is make
//
void CSceneBase::SetupPalettes(int code)
{
	UINT i;
	for (i = 0; i < NBR_PALS; i++)
		{
		delete m_pPals[i];
		m_pPals[i] = 0;
		}
	m_pPals[0] = new CNewPals; // scene palette
	if (!code)
		{
		bool bBad = 0;
		m_pPals[0]->ReadWrite(m_pIO, KEY_PAL_BASE+0,0);
		for (i = 1; i < NBR_PALS; i++)
			{
			DWORD size;
			if (!m_pIO->GetSize(size, KEY_PAL_BASE+i))
				{
				m_pPals[i] = new CNewPals; // scene palette
				m_pPals[i]->ReadWrite(m_pIO, KEY_PAL_BASE+i,0);
				if ((m_snd_levels < 2) && !m_pPals[i]->Simple(1))
					{
					bBad = TRUE;
					m_pPals[i]->ReadWrite(m_pIO, KEY_PAL_BASE+i,1);
					}
				}
			}
		if (bBad)
			LogIt(IDS_PAL_READ_LITE,-1);
		}
	else
		{
		m_pPals[0]->SetPalName("Scene Palette");
		BYTE * pTemp = new BYTE[1024];
		if (code == 2)
			LoadDefaultPalette(pTemp);
		else if (m_pIO->GetRecord(pTemp, 1024, KEY_PALETTE))
			LoadDefaultPalette(pTemp);
		m_pPals[0]->Legacy(pTemp);
		m_nNewRecords++;
		m_pPals[0]->ReadWrite(m_pIO, KEY_PAL_BASE+0,1);
		delete [] pTemp;
		}
}

int CSceneBase::LoadDefaultPalette(BYTE * pPal)
{
	int result = 0;
	int i;
	for (;;)
	{
	result = 11;

	auto resourcePtr = m_pResourceLoader->LoadDefaultPalette();
	if (!resourcePtr) {
		DPF("cannot find palette");
		break;
	}

	BYTE* hpSrc = (BYTE*)resourcePtr.get();

	result = 0;
	for (i = 0; i < 1024; i++)
		pPal[i] = hpSrc[i+2];

	for (i = 240; i < 255; i++)
		pPal[4*i+3] = 128;

	pPal[1020] = 255;
	pPal[1021] = 255;
	pPal[1022] = 255;
	pPal[1023] = 255;
	break;
	}
	if (result)
		{
		for (i = 0; i < 256; i++)
			{
			pPal[4*i+0] = i;
			pPal[4*i+1] = i;
			pPal[4*i+2] = i;
			pPal[4*i+3] = 255;
			}
		}
	return result;
}
/*
void CSceneBase::SetScenePalette(UINT index)
{
	if (!index)
		return;
	UINT i;
	for (i = 0; i < m_levels; i++)
		{
		CLevel * pLevel = GetLevelPtr(i);
		if (!pLevel)
			continue;
		if (pLevel->PalIndex() == index)
			pLevel->PalIndex(0);
		}
	delete m_pPals[0];
	m_pPals[0] = m_pPals[index];
	m_pPals[index] = 0;
	BlowCell(-3,0);			// any using old scene palette
	BlowCell(-3,index);		// and old index, not really required
	m_pPals[0]->ReadWrite(m_pIO, KEY_PAL_BASE+0,1);
	m_pIO->DelRecord(KEY_PAL_BASE+index);
}
*/
void CSceneBase::DeletePalette(UINT index)
{
	ASSERT(index != 0);
	if (!index) return;
	UINT i;
	for (i = 1; i < m_levels; i++)
		{
		if (PalIndex(i) == index)
			break;
		}
	ASSERT(i >= m_levels);
	CNewPals * pPals = m_pPals[index];
	delete pPals;
	m_pPals[index] = 0;
	m_pIO->DelRecord(KEY_PAL_BASE+index);
	m_bModified = TRUE;
}

void CSceneBase::NewPalName(LPCSTR Name, UINT index)
{
	char buf[80];
	UINT j;
	for (j = 0; j < 100; j++)
		{
		CNewPals * pPal = m_pPals[j];
		if (pPal && (index != j))
			{
			if (!strcmp(pPal->GetPalName(),Name))
				break;
			}
		}
	if (j < 100)
		sprintf(buf,"%s-%d",Name,index);
	else
		strcpy(buf, Name);
	m_pPals[index]->SetPalName(buf);
	m_pPals[index]->ReadWrite(m_pIO, KEY_PAL_BASE+index,1);
	m_bModified = TRUE;
}

UINT CSceneBase::PalWrite(UINT index)
{
	m_pPals[index]->ReadWrite(m_pIO, KEY_PAL_BASE+index,1);
	m_bModified = TRUE;
	return 0;
}


UINT CSceneBase::NewPalette(LPCSTR Name /* = 0 */, CNewPals * pPals /*= 0 */)
{
	UINT i;
	char buf[40];
	for (i = 0; m_pPals[i] && (i < 100); i++);
	ASSERT(i < 100);
	if (i >= 100) i = 99; // very unlikely, I know
	CNewPals * pNewPals;
	if (pPals)
		pNewPals = pPals->Clone(Name);
	else
		pNewPals = new CNewPals;
	if (Name)
		{
		UINT j;
		for (j = 0; j < 100; j++)
			{
			CNewPals * pPal = m_pPals[j];
			if (pPal)
				{
				if (!strcmp(pPal->GetPalName(),Name))
					break;
				}
			}
		if (j < 100)
			{
			sprintf(buf,"%s-%d",Name,i);
			}
		else
			strcpy(buf, Name);
		}
	else
		{
		sprintf(buf,"Pal-%d",i);
		}
	pNewPals->SetPalName(buf);
	m_pPals[i] = pNewPals;
	m_pPals[i]->ReadWrite(m_pIO, KEY_PAL_BASE+i,1);
	m_bModified = TRUE;
	return i;
}

UINT CSceneBase::ForgePals(BYTE * pPals, BYTE * pName)
{
	if ((pName[0] == 0) || (pName[0] == 2))
		return 0;//m_nScenePalette;
	UINT i;
	if (pName[0] == 1)
		{
		pName = 0;
		i = NewPalette("Custom");
		}
	else
		i = NewPalette("External");
	m_pPals[i]->Legacy(pPals,(LPCSTR)pName);
	m_pPals[i]->ReadWrite(m_pIO, KEY_PAL_BASE+i,1);
	m_nNewRecords++;
	return i;
}

int	 CSceneBase::Tools(void * pTools, UINT size, bool bPut/*= FALSE*/)
{
	UINT stat;
	if (bPut)
		{
		stat = m_pIO->PutSwapRecord(pTools, size, KEY_TOOLS);
		m_bModified = TRUE;
		}
	else
		stat = m_pIO->GetSwapRecord(pTools, size, KEY_TOOLS);
	return stat;
}

CNewPals * CSceneBase::PalettePtr(UINT index) //, CNewPals * pPals /* = 0 */)
{
	return m_pPals[index];
}

bool CSceneBase::PalLocked(UINT index)
{
	CNewPals * pPals = m_pPals[index];
	ASSERT(pPals != 0);
	if (pPals->Linked(0))// && !pPals->Dirty())
		return TRUE;
	return PalShared(index);
}

UINT CSceneBase::PalRef(UINT PalIndex)
{
	UINT Lev;
//
//	if more than one level, pal 0 (scene pal) is always shared
//
	UINT Count = (PalIndex || (m_levels < 3)) ? 0 : 1; // pal zero is always there
	for (Lev = 1; Lev < m_levels; Lev++) // dont use bg
		{
		CLevel * pLevel = GetLevelPtr(Lev);
		if (pLevel && (pLevel->PalIndex() == PalIndex))
			{
			Count++;
			if (Count > 1)
				break;
			}
		}
//	ASSERT(Count != 0);
	return Count; 
}

UINT CSceneBase::PalIndex(UINT Level, UINT v)
{
	CLevel * pLevel = GetLevelPtr(Level);
	if (!pLevel) return 0;
	if (v != NEGONE)
		{
		m_bModified = TRUE;
		if (!m_bLoading && (pLevel->Flags() & 1))
			UpdateCache();
		BlowCell(-2,Level);
//		m_pPals[v]->ReadWrite(m_pIO, KEY_PAL_BASE+v,1);
		}
	return pLevel->PalIndex(v);
}

#ifdef QQQ
UINT   CSceneBase::Pals(BYTE * pPals, UINT Level, bool bPut /* = 0 */)
{
	CLevel * pLevel = GetLevelPtr(Level);
	if (pLevel && bPut)
		{
		bool bUpdate = 0;
		char name[300];
		pLevel->PalName(name);
DPZ("palname0:%d",name[0]);
		if (!name[0])
			memmove(m_pScenePalette, pPals, 1024);
		pLevel->Pals(pPals,0);
		m_bModified = TRUE;
		if (!m_bLoading)
			{
			if (name[0] != 1) // see if other levels have the same palette name
				{
 				for (UINT Lev = 0; Lev < m_levels; Lev++)
					{
					if ((Lev != Level) && !SelectLevel(Lev))
						{
						char name2[300];
						pLevel->PalName(name2);
						if (!strcmp(name,name2))
							{
							pLevel->Pals(pPals,0); // write its pals, also
							m_bModified = TRUE;
							if (m_pLevel->Flags() & 1)
								bUpdate = 1;
							}
						}
					}
				m_pIO->PutRecord(m_pScenePalette, 1024, KEY_PALETTE);
				}
			else if (pLevel->Flags() & 1)
				bUpdate = 1;
			if (bUpdate)
				UpdateCache();
			}
		}
	else if (pLevel)
		{
		pLevel->Pals(pPals,this);
		}
	return 0;
}
#endif

void	CSceneBase::SceneOptionLock(bool bLock)
{
	if (bLock)
		m_bOptLock = 1;
	else
		{
ASSERT(m_bOptLock);
		m_bOptLock = 0;			// remove lock
		GetPutOptions(TRUE);	// write options
		}
}

#define DOIT(a) oldval=a;if (op == 2) a ^= 1; else if (op == 1) a=val; val=a
int	CSceneBase::SceneOptionInt(int Id, int op, int val)
{
	int oldval;
	UINT bit, shift, mask;
	switch (Id) {
	case SCOPT_VMARK0:
		DOIT(m_vmark0);
		break;
	case SCOPT_SMARK0:
		DOIT(m_smark0);
		break;
	case SCOPT_VMARK1:
		DOIT(m_vmark1);
		break;
	case SCOPT_SMARK1:
		DOIT(m_smark1);
		break;
	case SCOPT_VMARK2:
		DOIT(m_vmark2);
		break;
	case SCOPT_SMARK2:
		DOIT(m_smark2);
		break;
	case SCOPT_VOL0:
		DOIT(m_volume0);
		break;
	case SCOPT_VOL1:
		DOIT(m_volume1);
		break;
	case SCOPT_VOL2:
		DOIT(m_volume2);
		break;
	case SCOPT_MVOL:
		DOIT(m_mvolume);
		break;
	case SCOPT_EMBED_OUT:
		DOIT(m_embed_out);
		break;
	case SCOPT_EMBED_TRIM:
		DOIT(m_embed_trim);
		break;
	case SCOPT_EMBED_IN:
		DOIT(m_embed_in);
		break;
//	case SCOPT_JPEG:
//		DOIT(m_jpeg_quality);
//		break;
	case SCOPT_RATE:
		DOIT(m_rate);
		break;
	case SCOPT_SOUND:
		DOIT(m_bSound);
		break;
	case SCOPT_BUD0:
		DOIT(m_nBuddy0);
		break;
	case SCOPT_BUD1:
		DOIT(m_nBuddy1);
		break;
	case SCOPT_BUD2:
		DOIT(m_nBuddy2);
		break;
	case SCOPT_BUD3:
		DOIT(m_nBuddy3);
		break;
	case SCOPT_QUIET:
	case SCOPT_SCACHE:
	case SCOPT_NEXT:
	case SCOPT_PREV:
	case SCOPT_NEXT1:
	case SCOPT_PREV1:
	case SCOPT_PAINT:
	case SCOPT_LIGHT_PAINT:
	case SCOPT_AVI_SCALE:
		if (Id == SCOPT_PREV)
			bit = 64;
		else if (Id == SCOPT_NEXT)
			bit = 128;
		else if (Id == SCOPT_PREV1)
			bit = 0x10000;
		else if (Id == SCOPT_NEXT1)
			bit = 0x20000;
		else if (Id == SCOPT_PAINT)
			bit = 0x40000;
		else if (Id == SCOPT_LIGHT_PAINT)
			bit = 0x80000;
		else if (Id == SCOPT_AVI_SCALE)
			bit = 0x100000;
		else if (Id == SCOPT_QUIET)
			bit = 1;
		else 
			bit = 2;
		oldval = m_OptFlags;
		if (op == 2)
			m_OptFlags ^= bit;
		else if (op)
			{
				m_OptFlags |= bit;
			if (!val)
				m_OptFlags ^= bit;
			val = m_OptFlags;
			}
		else
			{
			if (m_OptFlags & bit)
				val = 1;
			else
				val = 0;
			}
		break;
	case SCOPT_MRU:
	case SCOPT_TELECINE:
	case SCOPT_APPLY_FRAME:
		if (Id == SCOPT_APPLY_FRAME)
			{
			shift = 25;
			mask = 31;
			}
		else if (Id == SCOPT_MRU)
			{
			shift = 4;
			mask = 3;
			}
		else
			{
			shift = 2;
			mask = 3;
			}
		oldval = m_OptFlags;
		if (op == 2)
			val = 0;
		else if (op)
			{
			val = val << shift;
			mask = mask << shift;
			m_OptFlags |= mask;	// all on
			m_OptFlags ^= mask;	// all off
			m_OptFlags |= val & mask;
			val = m_OptFlags;
			}
		else
			{
			val = (m_OptFlags >> shift) & mask;
			}
		break;
	case SCOPT_HOLD:
		oldval = m_OptFlags;
		if (op == 2)
			break;
		else if (op)
			{
			if (val) val--;	// normalize from 1..256 to 0..255
			m_OptFlags &= 0xffff00ff;
			m_OptFlags |= ((val & 255) << 8);
			val = m_OptFlags;
			}
		else
			{
			val = 1 + ((m_OptFlags >> 8) & 255);
			}
		break;
	case SCOPT_SNIP:
		DOIT(m_snip);
		break;
	case SCOPT_BLIND:
		DOIT(m_bBlind);
		break;
	}
	if (op && !m_bOptLock && (val != oldval))
		GetPutOptions(TRUE);
	return val;
}

int	CSceneBase::SceneOptionStr(int Id, LPSTR arg, int op)
{
	if (Id == SCOPT_WAVE0)
		{
		if (op)
			{
			strcpy(m_wave0, arg);
			GetPutOptions(TRUE);
			}
		else
			strcpy(arg, m_wave0);
		}
	else if (Id == SCOPT_WAVE1)
		{
		if (op)
			{
			strcpy(m_wave1, arg);
			GetPutOptions(TRUE);
			}
		else
			strcpy(arg, m_wave1);
		}
	else if (Id == SCOPT_WAVE2)
		{
		if (op)
			{
			strcpy(m_wave2, arg);
			GetPutOptions(TRUE);
			}
		else
			strcpy(arg, m_wave2);
		}
	return 0;
}

int CSceneBase::GetPutOptions(bool bPut)
{
//	history
//	version 1 oct 6, 2000
//	version 2 oct 8, 2000
//
//	Kludge with capital K, July 10, 2006
//	need 40 bits for 4 buddy levels in lightbox
//	using top 20 bits of bgmin and bSound
//	please forgive me
//

typedef struct {
	UINT	version;
	UINT	rate;
	UINT	vmark;
	UINT	smark;
	UINT	bgmin;
	UINT	bSound;
	UINT	OptFlags;
// 0x1 is quiet
// 0x2 is save cache
// 0xc	is	telecine
// 0x30 is mru
// 0x40 is prev
// 0x80 is next
// 0xff00 is hold
// 0x10000 is prev1
// 0x20000 is next1
// 0x40000 is paint ref
// 0x80000 is light paint
//0x0100000 is scale avi 
//0x0200000 is apply frame comp
//0x0400000 is apply export
//0x0800000 is start at 1
//0x1000000 is right
//0x2000000 is lower

	char	wave[300];
	UINT	snip;
	bool	blind;				// version 2

/*
	bool	bStory;				// version 3
	UINT	StoryRate;
	UINT	vc_black;
	UINT	vc_white;
	UINT	vc_gamma;

	UINT	tool;
	UINT	pen_radius;
	UINT	pen_density;
	UINT	pen_color;
	UINT	pencil_radius;
	UINT	pencil_density;
	UINT	pencil_color;
	UINT	eraser_radius;
	UINT	eraser_density;
	UINT	eraser_color;
*/
} OPTIONSREC;
	int res = 0;
	OPTIONSREC opt;
	if (bPut)
		{
		res = PutNewOptions();
/*
		opt.version = 3;
		opt.rate = m_rate;
		opt.vmark = m_vmark;
		opt.smark = m_smark;
		opt.OptFlags = m_OptFlags ^ 192; //SCOPT_PREV | SCOPT_NEXT, defaults
		opt.bgmin = m_MinBG + (m_nBuddy0 << 8) + ( m_nBuddy1 << 18);
		opt.bSound= m_bSound+ (m_nBuddy2 << 8) + ( m_nBuddy3 << 18);
		opt.snip = m_snip;
		opt.blind = m_bBlind;
		strcpy(opt.wave, m_wave);
		DOSWAPX((BYTE*)&opt,28,300,sizeof(opt));
		if (m_pIO->PutRecord(&opt, sizeof(opt), KEY_OPTIONS))
			res = 1;
*/
		m_bModified = TRUE;
		}
	else
		{
		opt.version = 1;
		opt.rate = 24; 
		opt.vmark = 0;
		opt.smark = 0;
		opt.snip = 3;
		opt.bSound = 0;
		opt.OptFlags = 0;
		opt.blind = 0;
		opt.bgmin = 0;
		opt.wave[0] = 0;
		m_vmark0 = m_vmark1 = m_vmark2 = 0;
		m_smark0 = m_smark1 = m_smark2 = 0;
		m_wave0[0] = m_wave1[0] = m_wave2[0] = 0;
		m_nBuddy0 = m_nBuddy1 = m_nBuddy2 = m_nBuddy3 = 0;
		if (!GetNewOptions())
			return 0;
		if (m_pIO->GetRecord(&opt, sizeof(opt), KEY_OPTIONS))
			res = 1;
		DOSWAPX((BYTE *)&opt,28,300,sizeof(opt));
		if ((opt.version == 1) || ((UINT)opt.blind > 1))
			opt.blind = 0;		// ignore bad values
		else if (opt.version > 3)
			return 2;
		m_rate = opt.rate; 
		m_vmark0 = opt.vmark;
		m_smark0 = opt.smark;
		m_vmark1 = m_vmark2 = 0;
		m_smark1 = m_smark2 = 0;
		m_nBuddy0 = min((opt.bgmin >> 8) & 0x3ff,m_levels-1);
		m_nBuddy1 = min((opt.bgmin >> 18) & 0x3ff,m_levels-1);
		m_nBuddy2 = min((opt.bSound >> 8) & 0x3ff,m_levels-1);
		m_nBuddy3 = min((opt.bSound >> 18) & 0x3ff,m_levels-1);
		m_bSound = opt.bSound & 1;
		m_OptFlags = opt.OptFlags ^ 192;//(SCOPT_PREV | SCOPT_NEXT); // defaults
		m_MinBG = opt.bgmin & 0xff;
		m_snip = opt.snip;
		m_bBlind = opt.blind;
		strcpy(m_wave0, opt.wave);
		}
	return res;
}

typedef enum {
	first,
	rate, 
	flags,
	snip,
	bsound,
	blind,
	bgmin,
	wave0,
	smark0,
	vmark0,
	wave1,
	smark1,
	vmark1,
	wave2,
	smark2,
	vmark2,
	buddy0,
	buddy1,
	buddy2,
	buddy3,
	vol0,
	vol1,
	vol2,
	mvol,
	jpeg,
	lipfile,
	lipx,
	lipy,
	lipw,
	liph,
	embo,
	embi,
	embt,
	last,
} SCNOPT;


#define LAZY(a,b) case a: b = v; break

int CSceneBase::GetNewOptions()
{
	DWORD rsize;
	if (m_pIO->GetSize(rsize,KEY_OPTIONS))
		return 98;
	if (rsize > 30000)
		return 99;
	BYTE * pOpt = new BYTE[rsize];
	if (m_pIO->GetRecord(pOpt, rsize, KEY_OPTIONS))
		{
		delete [] pOpt;
		return 1;
		}
	UINT * p = (UINT *)pOpt;
	if ((SWAPV(p[0]) != 4) || p[1]) // version follwed by first
		{
		delete [] pOpt;
		return 2;
		}
	p += 2;			// skip over version and first to first option/ tag
	for (;;)
		{
		UINT v = 0;
		BYTE * pd = nullptr;
		UINT lng = SWAPV(*p++);		// code
		UINT k = lng &0xffff;	// kind is lower 16
		lng = lng >> 16;		// lng is upper
        if (lng) {				// not a dword
			pd = (BYTE *) p;	// save a pointer to the data
			p += (lng + 3) / 4;	// bump to next UINT
        }
        else {
            if(k != SCNOPT::last) {
                v = SWAPV(*p++); // get value and mump to next
            }
        }
DPF("opt:%d, lng:%d,v:%d",k,lng,v);
		if ((k == last) || ((BYTE *)p >= (pOpt + rsize)))
			break;
		switch (k) {
		LAZY(rate, m_rate);
		LAZY(flags, m_OptFlags);
		LAZY(snip,m_snip);
		case bsound:
			m_bSound = v & 1;
			break;
		LAZY(blind,m_bBlind);
		LAZY(bgmin, m_MinBG);
		LAZY(smark0,m_smark0);
		LAZY(vmark0,m_vmark0);
		LAZY(smark1,m_smark1);
		LAZY(vmark1,m_vmark1);
		LAZY(smark2,m_smark2);
		LAZY(vmark2,m_vmark2);
		LAZY(vol0,m_volume0);
		LAZY(vol1,m_volume1);
		LAZY(vol2,m_volume2);
		LAZY(mvol,m_mvolume);
		LAZY(jpeg,m_jpeg_quality);
		case wave0: memmove(m_wave0, pd, lng); m_wave0[lng] = 0; break;
		case wave1: memmove(m_wave1, pd, lng); m_wave1[lng] = 0; break;
		case wave2: memmove(m_wave2, pd, lng); m_wave2[lng] = 0; break;
		LAZY(buddy0,m_nBuddy0);
		LAZY(buddy1,m_nBuddy1);
		LAZY(buddy2,m_nBuddy2);
		LAZY(buddy3,m_nBuddy3);
		LAZY(lipx,m_nLipx);
		LAZY(lipy,m_nLipy);
		LAZY(lipw,m_nLipw);
		LAZY(liph,m_nLiph);
		LAZY(embo,m_embed_out);
		LAZY(embi,m_embed_in);
		LAZY(embt,m_embed_trim);
		case lipfile: memmove(m_LipFile, pd, lng); m_LipFile[lng] = 0; break;
		case last:
			
		default:
			break;
	}
	}
	delete [] pOpt;
	return 0;	
}


UINT * StrOpt(UINT k, char * q, UINT * p)
{
	int lng;
	if (lng = strlen(q))
		{
		*p++ = SWAPV(k + (lng << 16));
		strcpy((char *)p, q);
		lng = (lng + 3) / 4;
		}
	return p + lng;
}

#define PLAZY(a,b) case a: *p++ = SWAPV(a); *p++ = SWAPV(b); break
#define ZLAZY(a,b) case a: p = StrOpt(a,b,p); break
int CSceneBase::PutNewOptions()
{
	BYTE * pOpt = new BYTE[30000];
	UINT * p = (UINT *)pOpt;
	*p++ = SWAPV(4); // version
	UINT k;
	ASSERT((int)m_snip >= 0);
	for (k = first; k <= last; k++)
		{
		switch(k) {
		case first:
			*p++ = SWAPV(0); // yeh, I know
			break;
		PLAZY(rate, m_rate);
		PLAZY(flags, m_OptFlags);
		PLAZY(snip,m_snip);
		PLAZY(bsound,(m_bSound & 1));
		PLAZY(blind,m_bBlind);
		PLAZY(bgmin, m_MinBG);
		PLAZY(smark0,m_smark0);
		PLAZY(vmark0,m_vmark0);
		PLAZY(smark1,m_smark1);
		PLAZY(vmark1,m_vmark1);
		PLAZY(smark2,m_smark2);
		PLAZY(vmark2,m_vmark2);
		PLAZY(vol0,m_volume0);
		PLAZY(vol1,m_volume1);
		PLAZY(vol2,m_volume2);
		PLAZY(mvol,m_mvolume);
		PLAZY(jpeg,m_jpeg_quality);
		ZLAZY(wave0, m_wave0);
		ZLAZY(wave1, m_wave1);
		ZLAZY(wave2, m_wave2);
		PLAZY(buddy0,m_nBuddy0);
		PLAZY(buddy1,m_nBuddy1);
		PLAZY(buddy2,m_nBuddy2);
		PLAZY(buddy3,m_nBuddy3);
		PLAZY(lipx,m_nLipx);
		PLAZY(lipy,m_nLipy);
		PLAZY(lipw,m_nLipw);
		PLAZY(liph,m_nLiph);
		PLAZY(embo,m_embed_out);
		PLAZY(embi,m_embed_in);
		PLAZY(embt,m_embed_trim);
		ZLAZY(lipfile,m_LipFile);
		case last:
			*p++ = last;
		default:
			break;
		}
		}
	UINT size = (BYTE *)p - pOpt;
	int res = 0;
	if (m_pIO->PutRecord(pOpt, size, KEY_OPTIONS))
		res = 1;
	delete [] pOpt;
	m_bModified = TRUE;
	return res;	
}

void CSceneBase::MakeInfo(bool bRead)
{
	if (!m_info)
		{
		m_info = 9999;
		delete [] m_pInfo;
		m_pInfo = new UINT[m_levels + 1];
		if (!m_pInfo)
			return;
		if (bRead)
			{
			if (!m_pIO->GetSwapRecord(m_pInfo,4*(1 + m_levels),KEY_LEVELINFO))
				{
DPF("info rec:%d",m_pInfo[0]);
				if (m_pInfo[0] == 1)
					m_info = 1;
				}
		}
		if (m_info != 1)
			{
			UINT i;
			for (i = 0; i < m_levels; m_pInfo[++i] = 9999);
			m_info = 1;
			}
		}
}

void CSceneBase::CheckInfoLevel() {
    if (m_info != 1 || !m_pInfo) {
        MakeInfo(true);
    }
}

UINT CSceneBase::GetLevelInfo(UINT what, UINT level, UINT def)
{
	if (m_info != 1)
		MakeInfo(TRUE);
DPF("get info,%d,lvl:%d,v:%d",m_info,level,m_pInfo[1+level]);
	if (m_pInfo[1+level] == 9999)
		m_pInfo[1+level] = def;
	return m_pInfo[1+level];
}

void CSceneBase::PutLevelInfo(UINT what, UINT level, UINT value)
{
DPF("put info,%d,%d",level,value);
	if (level == 9999)
		{
DPF("info:%d",m_info);
		if ((m_info == 1) && m_bModified)
			{
			m_pInfo[0] = 1;
			UINT z=m_pIO->PutSwapRecord(m_pInfo, 4 * (1 + m_levels), KEY_LEVELINFO);
DPF("z:%d",z);
			}
		return;
		}
	if (m_info != 1)
		{
		MakeInfo(0);
		}
	if (m_info != 1)
		return;
	if (value != m_pInfo[level+1])
		{
		m_bModified = TRUE;
		m_pInfo[level+1] = value;
		}
}

int	 CSceneBase::AVIInfo(void * pInfo, UINT size, bool bPut/*= FALSE*/)
{
	UINT stat;
	if (bPut)
		{
		stat = m_pIO->PutRecord(pInfo, size, KEY_AVI);
		m_bModified = TRUE;
		}
	else
		stat = m_pIO->GetRecord(pInfo, size, KEY_AVI);
	return stat;
}

void CSceneBase::ApplyImprint(HPBYTE pBuf, UINT d)
{
	UINT ow,oh;
	if (!d)
		{
		ow = ComW();
		oh = ComH();
		d = m_depth;
		}
	else
		{
		ow = m_width;
		oh = m_height;
		}
	if (m_depth == 1)
		d = 1;
	UINT f = ow / ComW();
	UINT x,y,z,op,zp;
	UINT ix,iy;
	op = 4 * ((d * ow + 3) / 4);
	if (d == 1)
		{
		zp = 4 * (((ComW()) + 3) / 4);
		for (y = 0; y < oh; y++)
		for (x = 0; x < ow; x++)
			{
			ix = x / f;
			iy = y / f;
			UINT v = m_pImprint[iy*zp+ix];
			UINT q = pBuf[op*y+x];
			v = (v * q) / 255;
			pBuf[op*y+x] = (BYTE)v;
			}
		return;
		}
	zp = 4 * ComW();
	for (y = 0; y < oh; y++)
		{
		for (x = 0; x < ow; x++)
			{
			ix = x / f;
			iy = y / f;
			UINT f_alpha = m_pImprint[iy*zp+4*ix+3];
			if (f_alpha)
				{
				if (d == 4)
					{
					UINT j,t;		
					UINT b_alpha = pBuf[op*y+4*x+3];
					for (j = 0; j < 3; j++)
						{
						UINT back = (UINT)pBuf[op*y+4*x+j] * b_alpha;
						t  = ((long)m_pImprint[zp*iy+4*ix+j] * f_alpha + 
									back - back * f_alpha / 255) / 255;
						pBuf[op*y+d*x+j] = t;
						}
					t = f_alpha + b_alpha - (f_alpha * b_alpha) / 255;
					pBuf[op*y+d*x+3] = t;
					}
				else
					{
					for (z = 0; z < 3; z++)
						pBuf[y*op+d*x+z] = 
								((UINT)pBuf[y*op+d*x+z] * (255 - f_alpha) +
							(UINT)m_pImprint[iy*zp+4*ix+z] * f_alpha) / 255;
					}
				}
			}
	}
}

UINT CSceneBase::UnWrapCell(CFile &file)
{
	CCell * pCell = new CCell(m_pIO);
	UINT key = pCell->Get(file);
	delete pCell;
	return key;
}

UINT CSceneBase::WrapCell(CFile &file, UINT Key)
{
	CCell * pCell = new CCell(m_pIO);
	pCell->SetKey(Key);
	if (!pCell->Read())
		{
		pCell->Put(file);
		}
	delete pCell;
	return 0;
}

UINT CSceneBase::LoadCache(LPCSTR pname)
{
	DPZ("load cache");
	CACHEHEADER header;
	char name[300];
	strcpy(name,pname);
	int c = strlen(name);
	if (!c) return 1;
	name[c - 1] = 'Q';
	CFile file;
	DWORD mode = CFile::modeRead;
	if (!file.Open(name, mode))
		return 1;
	UINT w = ComW();
	UINT h = ComH();
	UINT d = m_depth;


	file.Read(&header,sizeof(header));
	UINT hc = header.dwCode;
	if (m_jpeg_quality)
		hc = 2;
	if (header.dwId != DGQID ||
		header.wWidth != w ||
		header.wHeight != h ||
		header.wDepth != d ||
		header.dwCode != hc ||
		header.wFrameCount != m_frames)
		{
DPZ("hdr mismatch");
		file.Close();
		return 2;
		}
	if (header.dwCode == 2)
		{
		for (UINT frame = 0; frame < m_frames; frame++)
			{
			BYTE * buf = m_pCache[frame];
			int size;
			file.Read(&size,4);
			int cur_size = *((int *)buf);
			if ((size > (cur_size - 8)) || (size < (9 * cur_size / 10))) 
				{
				delete [] m_pCache[frame];
				cur_size = 11 * size / 10;
				m_pCache[frame] = new BYTE[cur_size];
				buf = m_pCache[frame];
				*((int *)buf) = cur_size;
				}
			buf += 4;
			*((int *)buf) = size;
			file.Read(buf,size);
			m_pFlags[frame] = 0;
			}
		}
	else
		{
		UINT pp = h * w;
		UINT p = 4 * ((w * d + 3) / 4);
		UINT size = h * p;
		UINT maxsize = (11 * d * pp) / 10;
		BYTE * temp = new BYTE[maxsize];
		BYTE * rgb = new BYTE[d * pp];
		if (!temp || !rgb)
			{
			delete []temp;
			delete []rgb;
			file.Close();
			return 3;
			}
		BYTE * buf = nullptr;
		BYTE * ref = nullptr;
		for (UINT frame = 0; frame < m_frames; frame++)
			{
			buf = m_pCache[frame];
			if (header.dwCode == 1)
				{
				UINT insize;
				file.Read(&insize,4);
				file.Read(temp,insize);
				unsigned long vc = d * pp;
				UINT qq = uncompress(rgb,&vc,temp,insize);
				if (qq)
					{
DPZ("decompress error:%d",qq);
					break;
					}
				}
			else
				{
				file.Read(rgb,d*pp);
				}
			UINT x,y,j;
			for (y = 0; y < h; y++)
				{
				for (x = 0; x < w; x++)
					{
					if (frame)
						for (j = 0; j < d; j++)
							buf[y*p+d*x+j] = ref[y*p+d*x+j] + rgb[pp*j+y*w+x];		
					else
						for (j = 0; j < d; j++)
							buf[y*p+d*x+j] = rgb[pp*j+y*w+x];		
					}
				}
			m_pFlags[frame] = 0;
			ref = buf;
			}
	delete [] rgb;
	delete [] temp;
}
	file.Close();
	return 0;
}

#define COMPRESSCACHE
UINT CSceneBase::SaveCache(LPCSTR pname)
{
	DPZ("save cache");
	CACHEHEADER header;
	UINT w = ComW();
	UINT h = ComH();
	UINT d = m_depth;
	UINT pp = h * w;
	UINT p = 4 * ((w * d + 3) / 4);
	UINT size = h * p;
	UINT maxsize = (11 * d * pp) / 10;
	BYTE * temp = new BYTE[maxsize];
	BYTE * rgb = new BYTE[d * pp];
	if (!temp || !rgb)
		{
		delete temp;
		delete rgb;
		return 3;
		}
	CFile file;
	DWORD mode = CFile::modeWrite | CFile::modeCreate;
	if (!file.Open(pname, mode))
		{
		delete [] temp;
		delete [] rgb;
		return 2;
		}
	header.dwId = DGQID;
	header.wWidth = w;
	header.wHeight = h;
	header.wDepth = d;
	header.wFrameCount = m_frames;
	if (m_jpeg_quality)
		header.dwCode = 2;
	else
		{
#ifdef COMPRESSCACHE
	header.dwCode = 1;
#else
	header.dwCode = 0;
#endif
		}
	file.Write(&header,sizeof(header));
	if (m_jpeg_quality)
		{
		for (UINT frame = 0; frame < m_frames; frame++)
			{
			BYTE * buf = m_pCache[frame];
			int size;
			size = *((int *)buf);
			file.Write(buf,size+4);
			}
		}
	else
	{
	BYTE * buf = nullptr;
	BYTE * ref = nullptr;
	for (UINT frame = 0; frame < m_frames; frame++)
		{
		UINT x,y,j;
		buf = m_pCache[frame];
		for (y = 0; y < h; y++)
			{
			for (x = 0; x < w; x++)
				{
				if (frame)
					for (j = 0; j < d; j++)
						rgb[pp*j+y*w+x] = buf[y*p+d*x+j] - ref[y*p+d*x+j];
				else
					for (j = 0; j < d; j++)
						rgb[pp*j+y*w+x] = buf[y*p+d*x+j];
				}
			}
		ref = buf;
		unsigned long dsize = maxsize;
#ifdef COMPRESSCACHE
		UINT cq = compress(temp,&dsize,rgb,d* pp);
		if (cq)
			{
DPZ("compress failure");
			break;
			}
		file.Write(&dsize,4);
		file.Write(temp,dsize);
#else
		file.Write(rgb,d * pp);
#endif
		}
	}
	file.Close();
	delete [] rgb;
	delete [] temp;
	return 0;
}

void CSceneBase::LevelTable(UINT Level, CLevelTable * tbl, bool bPut /*= 0 */)
{
	CLevel * pLevel = GetLevelPtr(Level, 1);
	if (pLevel && bPut)
		{
		UINT flags;
		if (flags = pLevel->Table(tbl,1))
			m_bModified = TRUE;
		if (flags && !m_bLoading && (pLevel->Flags() & 1))
			UpdateCache();
		BlowCell(-2,Level);
		}
	else if (pLevel)
		{
		pLevel->Table(tbl,0);
		if ((tbl->table[0].name[0] == 0 ) &&
				(tbl->table[0].name[1] == (char)255))
			{
			for (int i = 0; i < 11; i++)
				{
				if (i == 5)
					sprintf(tbl->table[i].name,"Ink & Paint");
				else if (i > 5)
					sprintf(tbl->table[i].name,"Above %d",i-5);
				else
					sprintf(tbl->table[i].name,"Below %d",i+1);
				tbl->table[i].color = 0;
				tbl->table[i].blur = 0;
				tbl->table[i].flags = 0x300;//i == 5 ? 0x300 : 0;
				tbl->table[i].dx = 0;
				tbl->table[i].dy = 0;
				}
			}

		}
}


CLevelTable::CLevelTable()
{
	memset(this,0,sizeof(LEVTBL));
//	table[0].name[1] = (char)255; // make look like old 
	for (int i = 0; i < 11; i++)
		{
		if (i == 5)
			sprintf(table[i].name,"Ink & Paint");
		else if (i > 5)
			sprintf(table[i].name,"Above %d",i-5);
		else
			sprintf(table[i].name,"Below %d",i+1);
		table[i].color = 0;
		table[i].blur = 0;
		table[i].flags = 0x300;//i == 5 ? 0x300 : 0;
		table[i].dx = 0;
		table[i].dy = 0;
		}
}

#ifdef _DISNEY
UINT CSceneBase::DisPalIO(BYTE * p, UINT size, bool bPut)
{
	if (bPut)
		return m_pIO->PutRecord(p, size, KEY_DISPAL);
	else
		return m_pIO->GetRecord(p,size,KEY_DISPAL);
}
#endif

UINT CSceneBase::FindRef(UINT key)
{
	UINT i;
	if (m_pLinks)
	{
		m_pLinks[m_links].dwKey = key;
		for (i = 0; m_pLinks[i].dwKey != key; i++);
	}
	else
		i = m_links;
	return i;
}

UINT CSceneBase::RefCount(UINT key)
{
	UINT c = 0;
	UINT i = FindRef(key);
	if (i < m_links)
		c = m_pLinks[i].dwCount + 2;
	return c;
}

bool CSceneBase::IsLinked(UINT key)
{
  UINT i = FindRef(key);
  return i >= m_links ? 0 : 1;
}

void CSceneBase::ClipInfo(UINT & w, UINT &h)
{
    w = m_pClipBoard[0];
    h = m_pClipBoard[1];
}

UINT CSceneBase::ClipSet(UINT x, UINT y, UINT RecNbr)
{
DPF("clipset,x:%d,y:%d,k:%d",x,y,RecNbr);
    if (!m_pClipBoard) return 1;
    if (x >= m_pClipBoard[0]) return 2;
    if (y >= m_pClipBoard[1]) return 3;
    m_pClipBoard[y * m_pClipBoard[0] + x + 3] = RecNbr;
    return 0;
}

UINT CSceneBase::ClipGet(UINT x, UINT y)
{
    if (!m_pClipBoard) return 0;
    if (x >= m_pClipBoard[0]) return 0;
    if (y >= m_pClipBoard[1]) return 0;
    return m_pClipBoard[y * m_pClipBoard[0] + x + 3];
}

UINT CSceneBase::ClipFinish()
{
//    UINT w = m_pClipBoard[0];
//    UINT h = m_pClipBoard[1];
//    if (m_pIO)
//        m_pIO->PutRecord(m_pClipBoard,(3 + w * h) * sizeof(UINT), KEY_CLIP);
  return 0;
}

bool CSceneBase::ClipEmpty(int how) {
    if (how && m_pClipBoard) {
        UINT w = m_pClipBoard[0];
        UINT h = m_pClipBoard[1];
        UINT* pKeys = new UINT[1 + w * h];
        UINT kCount = 0;
        UINT x, y;
        DPF("deleting clip recs, w:%d,h:%d", w, h);
        for (x = 0; x < w; x++)
            for (y = 0; y < h; y++) {
                if ((pKeys[kCount] = m_pClipBoard[y * w + x + 3]))
                    kCount++;
            }
        //        m_pIO->DelRecord(KEY_CLIP);
        delete[] m_pClipBoard;
        m_pClipBoard = 0;
        UpdateLinks(pKeys, kCount);
        delete[] pKeys;
    }
    return m_pClipBoard ? true : false;
}

UINT CSceneBase::ClipStart(UINT w, UINT h, UINT FirstLevel) {
    ClipEmpty(1);
    //    m_bNoLinks = 1;  // cut, not copied, also cross scene
    m_pClipBoard = new UINT[3 + w * h];
    UINT i;
    for (i = 0; i < 3 + w * h; m_pClipBoard[i++] = 0);
    m_pClipBoard[0] = w;
    m_pClipBoard[1] = h;
    m_pClipBoard[2] = FirstLevel;
    m_ClipLevel = FirstLevel;
    return 0;
}

void CSceneBase::CutCopy(UINT f, UINT l, UINT w, UINT h, bool bCopy) {
    UINT cnt = 0;
    UINT* pKeys = 0;
    if (m_pClipBoard) {
        UINT w = m_pClipBoard[0];
        UINT h = m_pClipBoard[1];
        pKeys = new UINT[1 + w * h];
        UINT x, y;
        for (x = 0; x < w; x++)
            for (y = 0; y < h; y++) {
                UINT keyClip = m_pClipBoard[y * w + x + 3];
                if (keyClip)
                    pKeys[cnt++] = keyClip;
            }
    }
    m_ClipFrame = f;
    m_ClipLevel = l;
    ClipStart(w, h, m_ClipLevel);
    //    if (bCopy)
    //        m_bNoLinks = 0;
    DWORD i, j, key;
    for (i = 0; i < w; i++)
        for (j = 0; j < h; j++) {
            key = ChangeCellKey(f + j, l + i, bCopy ? false : true);
            ClipSet(i, j, key);
        }
    ClipFinish();
    UpdateLinks(pKeys, cnt);
    delete[] pKeys;
}

UINT CSceneBase::SetRefCount(UINT key, UINT count)
{
	if (count < 2)
	{
		UINT i = FindRef(key);
		if (i < m_links)	// oops
		{
			for (i++; i < m_links; i++)
				m_pLinks[i - 1] = m_pLinks[i];
			m_links--;
		}
		return count;
	}

	UINT i = FindRef(key);
	if (i >= m_links)
	{
		if (i >= m_linkz)
		{
			LINKENTRY* pTemp = m_pLinks;
			if (!pTemp)
				return 0;
			m_linkz += 100;
			m_pLinks = new LINKENTRY[m_linkz];
			for (i = 0; i < m_links; i++)
				m_pLinks[i] = pTemp[i];
			delete[] pTemp;
		}
		m_links++;
		m_pLinks[i].dwKey = key;
	}
	m_pLinks[i].dwCount = count - 2;
	return count;
}
