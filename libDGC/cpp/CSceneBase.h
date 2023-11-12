#ifndef _SCENE_H_
#define _SCENE_H_

#include "MyObj.h"
#include "CommonDefs.h"
#include "ResourceLoader.h"

#include <memory>

#define CHECKING
#ifdef CHECKING
typedef struct {
	DWORD   dwKey;
	DWORD   dwStat;
	DWORD   dwSize;
	DWORD   dwAdr;
	DWORD   dwKind;
	DWORD   dwCount;
	DWORD   dwLink;
} CHKENTRY;
#endif

typedef struct {
	DWORD   dwKey;
	DWORD   dwCount;
} LINKENTRY;


typedef struct {
	int offx;
	int offy;
	int blur;
	UINT flags;
} ACTLAY;

#define FEAT_STD  1
#define FEAT_LITE 2
#define FEAT_PT   3
#define FEAT_PRO  4


#define BIT_TELECINE 0x8000
#define BIT_DESKEW   0x4000
#define BIT_HIRES    0x2000
#define BIT_COLORKEY 0x1000
#define BIT_LAYERS   0x0800
#define BIT_PLUS     0x0400
#define BIT_BIT6     0x0200
#define BIT_CAMERA   0x0100
#define BIT_LIBRARY  0x0080
#define BITS_TYPE    0x000F

#define SCN_FLG_DIRTY  0
#define SCN_FLG_COLOR  1
#define SCN_FLG_REDBOX 2
#define SCN_FLG_NLARGE 3
#define SCN_FLG_AUTOCMP 4
#define SCN_FLG_GRID  5
#define SCN_FLG_FIT 6
#define SCN_FLG_GRID_CTR 7

#define SCN_FLAG_HOLD 1

#define NBR_PALS 100

class CCellEntry;
class CLevels;
class CCamera;
class CLevel;
class CCell;
class CLayers;
class CLevelTable;
class CNewPals;

class CMiniJpegEncoder;
class CMiniJpegDecoder;

class CSceneBase : public CMyObj
{
public:
	CSceneBase(CMyIO * pIO, std::unique_ptr<ResourceLoader> resourceLoader);
	~CSceneBase();
	int Read(bool bPreview, DWORD dwFeatures= 0, DWORD id = 0);
	int Write();
	int Check();
	BYTE * LoggedData(bool bClear);
	UINT ReadErrors(UINT i = NEGONE);
	UINT Make(DWORD id, DWORD features, 
//			UINT width, UINT height, UINT rate, UINT frames, UINT levels = 1);
			UINT width, UINT height, UINT rate, UINT frames, UINT levels,
			UINT factor, UINT preview, UINT jpeg, UINT broadcast);
	//HDIB GetBG();
	UINT FrameCount() { return m_frames;};
	UINT Selection_Start() { return m_start;};
	UINT Selection_Stop() { return m_stop;};
	UINT MaxFrameCount() {return m_max_frames;};
	UINT LevelCount() { return m_levels;};
//	void SetCamInfo(bool bPlaying = 1, UINT Frame=0, UINT state=0);
	bool CamCursor(UINT x, UINT y);
	UINT CamCurPeg() { return m_CamPeg;};
	void SelectPeg(UINT peg);
//	UINT CamPegSel() { return m_CamPegSel;};
//	UINT CamPeg() { return m_CamPeg;};
//	void CamInfo(UINT frame = -1, bool bPlaying = 1);
	UINT FrameRate();
	UINT ComW() { return 2 * m_width / m_factor;};
	UINT ComH() { return 2 * m_height / m_factor;};
	void PublishSizes(UINT & w, UINT & h);
	bool ColorMode(UINT Mode = NEGONE);
	bool RedBox(UINT Mode = NEGONE);
	UINT Flagger(UINT msk, UINT v = 2);
	bool OverHold(int v = 2) { return Flagger(SCN_FLAG_HOLD,v);};
	void SetSelection(UINT start, UINT stop);
	void SetLevelCount(UINT count, UINT def);
	void SetFrameRate(UINT rate);
	UINT JpegQuality(UINT v = NEGONE);
	UINT ScnChangeFrames(UINT Start, UINT End);
	UINT InsertLevel(UINT Start, UINT Count, UINT def);
	UINT DeleteLevel(UINT Start, UINT Count);
	UINT SlideCells(UINT StartF, UINT EndF, UINT StartL, UINT EndL, UINT Count=0);
	UINT BlankCell(UINT Frame, UINT Level);
	UINT DeleteCell(UINT Frame, UINT Level); // from level and links
	UINT DeleteCell(DWORD dwKey);
	UINT SwapCellKey(UINT Frame, UINT Level, UINT key);
	void UpdateLinks(UINT * pKeys, UINT cnt); // a list of keys to delete if unused
	DWORD SceneId() { return m_dwId;};
//
//	0 is release, 1 is release opened with demo, 2 is demo/demo
//
	UINT SceneState() { return m_uState;};
	UINT Width() { return m_width;};// / m_scale;};
	UINT Height() { return m_height;};// / m_scale;};
//	UINT QScale() { return m_scale;};
	UINT SetFactor(UINT Factor);
	UINT Zoom(UINT zoom = 1000);
	UINT ZFactor(int bOrig = 0) {
		if (bOrig > 1)
			return m_factor / m_origfactor;
		else if (bOrig)
			return m_origfactor;
		else
			return m_factor;};
	void SetThumbSize(UINT w, UINT h)
		{m_thumbw = w; m_thumbh = h;};
    UINT GetThumbW() { return m_thumbw; };
    UINT GetThumbH() { return m_thumbh; };
	bool GetBackground(HPBYTE hpDst,UINT Frame,UINT min);
	void GetGray(HPBYTE hpDst, UINT Frame, UINT Level);
	UINT GetLayer(HPBYTE hpDst, UINT Frame, UINT Level, UINT Which, DWORD key=0);
	HPBYTE GetCacheP(UINT Frame);
//	void PutFrame(HANDLE hMono, HANDLE hGray, UINT Frame, UINT grayoffset);
	void PutMono(HPBYTE hpMono, UINT Frame, UINT Level);
	void PutGray(HPBYTE hpGray, UINT Frame, UINT Level);
	void SetLayer(CLayers * pLayer);
	void PutLayer(HPBYTE hpGray, UINT Frame, UINT Level, UINT Which);
	void ProcessCellLabel(CString & label, UINT hold);
	void ApplyImprint(HPBYTE hpDst, UINT d = 0);
	
//	void CompositeFrame(BYTE * pBuf, 
//					UINT StartLevel, UINT EndLevel, UINT Frame, bool bBroadcast);
	void CombineLayers(BYTE * pBuf, UINT StartLevel, UINT EndLevel, UINT Frame);
	void CompositePiece(BYTE * pBuf, UINT Frame,
			UINT x1, UINT y1, UINT x2, UINT y2);
	UINT GetThumb(HPBYTE hpDst, UINT Frame, UINT Level,
				UINT w, UINT h, UINT bpp, bool bForge = 0);
	void FetchCell(HPBYTE hpDst, UINT Frame, UINT Level, 
				bool b32, bool bUseGray, bool bHold = 0);
	UINT CellInfo(HPBYTE hpDst, UINT Frame, UINT Level, bool bHold,
					UINT & w, UINT & h, UINT & key);
	UINT MakeThumb(HPBYTE hpDst, UINT Frame, UINT Level);
	UINT BlankThumb(UINT Frame, UINT Level);
	void LevelName(LPSTR name, UINT level, bool bPut = FALSE);
//	UINT ScenePalette(UINT v = NEGONE);
	void SetScenePalette(UINT index);
	UINT PalIndex(UINT Level, UINT v = NEGONE);
	bool PalLocked(UINT PalIndex);
	UINT PalRef(UINT PalIndex);
	bool PalUsed(UINT PalIndex) { return PalRef(PalIndex) ? 1 : 0;};
	bool PalShared(UINT PalIndex) { return PalRef(PalIndex) > 1 ? 1 : 0;};
	CNewPals * LevelPalette(UINT level) { return PalettePtr(PalIndex(level));};
	CNewPals * PalettePtr(UINT Index);//, CNewPals * pPals = 0);
	void LevelModelName(LPSTR name, UINT level, bool bPut = FALSE);
	DWORD LevelFlags(UINT Level,DWORD val = NEGONE);
	void PalChanged(UINT Index);
//	UINT PaletteIO(LPCSTR name, UINT Level, bool bPut = 0);
	void CellName(LPSTR name, UINT frame, UINT level, bool bPut = 0);
	bool Modified(bool bClear = FALSE);
	UINT Broadcast(UINT v = NEGONE);
	void Broadcasting(bool bBroadcast);
	bool Flag(UINT which, bool bSet = FALSE, bool bValue = FALSE);
	void SceneOptionLock(bool bLock);
	int	SceneOptionInt(int Id, int op = 0, int val = 0);
	int	SceneOptionStr(int Id, LPSTR arg, int op = 0);
	UINT UpdateCache(UINT Frame = 0, UINT Level = 9999, UINT Count = 1);
	DWORD	GetCellKey(UINT Frame, UINT Level, bool bHold = FALSE);
	UINT	ChangeCellKey(UINT Frame, UINT Level, bool bRemove);
	bool	FindNextCell(UINT & Frame, UINT Level);
	bool	FindPrevCell(UINT & Frame, UINT Level);
	UINT	DuplicateCell(UINT key);
//	UINT	SetCellKey(UINT Frame, UINT Level, DWORD key);
	int	 	GetImageKey(DWORD& key, UINT Frame,
					UINT Level, UINT Which, bool bMake = FALSE);
	int	 	GetImageKey(DWORD& key, DWORD cellkey, UINT Which);
	void SetMinBG(UINT min, bool bInit = 0);
	UINT  LinkCellRecord(DWORD key, int inc = 0, bool bForce = 0);
	DWORD LinkCell(UINT Frame, UINT Level);
	void	MakeModified() { m_bModified = TRUE;};
	void	LevelTable(UINT Level, CLevelTable * tbl, bool bPut = 0);
	void	PutLevelWidth(UINT Level,UINT width){PutLevelInfo(0,Level,width);};
	UINT	GetLevelWidth(UINT Level,UINT def)
					{ return GetLevelInfo(0,Level,def);};
	int		AVIInfo(void * pInfo, UINT size, bool bPut = FALSE);
	UINT PalWrite(UINT index);
	UINT NewPalette(LPCSTR name = 0, CNewPals * pPals = 0);
	void NewPalName(LPCSTR name, UINT Index);
	void DeletePalette(UINT index);
	UINT ForgePals(BYTE * pPals, BYTE * pName);
	int	Tools(void * pTools, UINT size, bool bPut = FALSE);
	UINT PegFindLevel(UINT Level);
	UINT PegAttach(UINT Level, UINT Peg = 0);	// 0 is detach
	UINT PegName(LPSTR Name, UINT Peg = 9999, bool bPut = 0); // 999 is add
	BYTE * MakeWireFrame(UINT& w, UINT& h, UINT frame, UINT peg);
	CCamera * Camera();
	void zApplyGray(HPBYTE hpDst,UINT factor,UINT Frame,UINT Level, bool bHold=1);
	UINT BlowCell(UINT key);
	UINT BlowCell(UINT Frame, UINT Level);
	UINT WrapCell(CFile &file, UINT Key);
	UINT UnWrapCell(CFile &file);
	UINT SaveCache(LPCSTR name);
	UINT LoadCache(LPCSTR name);
	int  	ReadImage(HPBYTE hpDst, DWORD key);
	int  	ImageInfo(UINT & w, UINT &h, UINT & d, DWORD key);
	void ThumbMinMax(UINT & min, UINT & max, UINT Frame, UINT Level);
	UINT Before(UINT * pList, UINT max, UINT Frame, UINT Level);
	int 	TopLevel(UINT frame);
	UINT CachedSize(UINT Frame);
	void LipRect(LPRECT rect, bool bPut = 0);
	void RelativeName(LPSTR name);
	double CamScale();

	UINT EmbFind(LPCSTR name, UINT kind); // -1 is not found, else index
	UINT EmbData(UINT index, LPBYTE pData);
	void SetEmbedded(bool bAppended);
	bool EmbAppend(bool bJustDeleteDGY);
#ifdef _DISNEY
	UINT DisPalIO(BYTE * p, UINT size, bool BPut);
#endif
#ifdef MYBUG
	void Display();
void ShowLinks();
#endif


//
//	clipboard functions
//
	UINT	ClipSet(UINT x, UINT y, UINT RecNbr);
	UINT	ClipGet(UINT x, UINT y);
	void	ClipInfo(UINT & w, UINT &h);
//	bool	ClipNoLinks() { return m_bNoLinks;};  // cut, not copied
	UINT	ClipFinish();
//	UINT *  ClipClear(bool bMakeList);
//	void 	ClipClear(UINT key);
//	void 	ClipClear(UINT *pKeys, UINT count);
	UINT	ClipCount(UINT key);
	bool	IsLinked(UINT key);
	UINT	ClipFrame() { return m_ClipFrame;};
	UINT	ClipLevel() { return m_ClipLevel;};
    UINT    ClipFrameSet(UINT frame) { m_ClipFrame = frame;};
    UINT    ClipLevelSet(UINT level) { m_ClipLevel = level;};
	UINT	RefCount(UINT key);// w/o clipboard
    bool    ClipEmpty(int how = 0);
    UINT    ClipStart(UINT w, UINT h, UINT FirstLevel);
    void    CutCopy(UINT f, UINT l, UINT w, UINT h, bool bCopy);
    
    bool    GetLevel0Data(BYTE* hpDst, UINT Frame,
                          bool bHold, UINT min, bool bCamera, bool bBroadcast);
    UINT    GetSizeForLevel0(bool bCamera);
    UINT    GetDepthImage();
    void    CompositeFrame(BYTE * pBuf, UINT StartLevel, UINT EndLevel, UINT Frame, bool bBroadcast);
    
    void    CheckInfoLevel();
protected:
	UINT 	SetRefCount(UINT key, UINT count);
	UINT	FindRef(UINT key);
//	BYTE *  PalAddr(UINT Level, bool bPut = FALSE);
//	UINT	Pals(BYTE * pPals, UINT Level, bool bPut = FALSE);
	void LogIt(int Id, UINT level, LPCSTR name=0);
	int  CheckExternals();
	void XThumb(HPBYTE hpDst, UINT w, UINT h, UINT bpp, UINT Level);
	void InitCellCache();
	void ScanCellCache() {}; // someday
	UINT GetLayer32(UINT Frame, UINT Level, DWORD key);
	UINT GetOverlay(UINT key);
	void GetCell(HPBYTE hpDst, UINT Frame, UINT Level);
	UINT GetCell32(UINT Frame, UINT Level, bool bSearch);
	void ApplyCell(HPBYTE hpDst, 
			UINT Frame, UINT Level, bool bCamera = FALSE, bool bBradcast = 0);
	void Apply32(BYTE * hpDst, BYTE * hpSrc, UINT iw, UINT ih);
	void Apply24(BYTE * hpDst, BYTE * hpSrc, UINT iw, UINT ih, 
					bool bCamera, bool bBroadcast);
	void Apply24c(BYTE * hpDst, BYTE * hpSrc, UINT iw, UINT ih);
	void Apply24g(BYTE * hpDst, BYTE * hpSrc, UINT iw, UINT ih, bool bBG);
	void ApplyCell32(HPBYTE hpDst, UINT Frame, UINT Level, bool bFill);
	void ApplyBuff(HPBYTE hpDst, HPBYTE hpSrc, UINT w, UINT h, UINT factor);
	bool GetLevel0(HPBYTE hpDst,UINT Frame,bool bHold, UINT min,
				bool bCamera, bool bBroadcast);

	inline void  Magic4(UINT off, UINT zz, UINT * s, BYTE *pBuf)
			{
			pBuf += off;
			if (pBuf[3])
				{
				s[0] += zz * *pBuf++;
				s[1] += zz * *pBuf++;
				s[2] += zz * *pBuf++;
				s[3] += zz * *pBuf++;
				s[4] += zz;
				}
			}
	inline void MagicColumn4(UINT x,UINT y1,UINT y2,
				UINT xf, UINT q, UINT yf1, UINT yf2,
				UINT ip, UINT * s, BYTE * pBuf)
			{
			UINT off1 = ip * y1 + 4 * x;
			UINT off2 = ip * y2 + 4 * x;
			Magic4(off1,xf*yf1,s,pBuf);
			for (off1 += ip; off1 < off2; off1 += ip)
				Magic4(off1,xf * q,s,pBuf);
			Magic4(off1,xf*yf2,s,pBuf);
			}
	inline void MagicLine4(UINT x1,UINT x2,UINT y,
				UINT xf1, UINT xf2, UINT yf, UINT q,
				UINT ip, UINT * s, BYTE * pBuf)
			{
			UINT off1 = ip * y + 4 * x1;
			UINT off2 = off1 + 4 * (x2 - x1);
			Magic4(off1,yf*xf1,s,pBuf);
			for (off1 += 4; off1 < off2;off1 += 4)
				Magic4(off1,yf*q,s,pBuf);
			Magic4(off1,yf*xf2,s,pBuf);
			}

	inline void  Magic3(UINT off, UINT zz, UINT * s, BYTE *pBuf)
			{
			pBuf += off;
			s[0] += zz * *pBuf++;
			s[1] += zz * *pBuf++;
			s[2] += zz * *pBuf++;
			}
	inline void MagicColumn3(UINT x,UINT y1,UINT y2,
				UINT xf1, UINT xf2, UINT yf1, UINT yf2,
				UINT ip, UINT * s, BYTE * pBuf)
			{
			UINT off1 = ip * y1 + 3 * x;
			UINT off2 = ip * y2 + 3 * x;
			Magic3(off1,xf1*yf1,s,pBuf);
			for (off1 += ip; off1 < off2; off1 += ip)
				Magic3(off1,xf1 * xf2,s,pBuf);
			Magic3(off1,xf1*yf2,s,pBuf);
			}
	inline void MagicLine3(UINT x1,UINT x2,UINT y,
				UINT xf1, UINT xf2, UINT yf, UINT q,
				UINT ip, UINT * s, BYTE * pBuf)
			{
			UINT off1 = ip * y + 3 * x1;
			UINT off2 = off1 + 3 * (x2 - x1);
			Magic3(    off1,yf*xf1,s,pBuf);
			for (off1 += 3; off1 < off2;off1 += 3)
				Magic3(off1,yf*  q,s,pBuf);
			Magic3(    off1,yf*xf2,s,pBuf);
			}

	inline void  Magic1(UINT off, UINT zz, UINT * s, BYTE *pBuf)
			{
			pBuf += off;
			s[0] += zz * *pBuf++;
//			s[1] += zz;
			}
	inline void MagicColumn1(UINT x,UINT y1,UINT y2,
				UINT xf1, UINT xf2, UINT yf1, UINT yf2,
				UINT ip, UINT * s, BYTE * pBuf)
			{
			UINT off1 = ip * y1 + x;
			UINT off2 = ip * y2 + x;
			Magic1(off1,xf1*yf1,s,pBuf);
			for (off1 += ip; off1 < off2; off1 += ip)
				Magic1(off1,xf1 * xf2,s,pBuf);
			Magic1(off1,xf1*yf2,s,pBuf);
			}
	inline void MagicLine1(UINT x1,UINT x2,UINT y,
				UINT xf1, UINT xf2, UINT yf1, UINT yf2,
				UINT ip, UINT * s, BYTE * pBuf)
			{
			UINT off1 = ip * y + x1;
			UINT off2 = off1 + (x2 - x1);
			Magic1(off1,yf1*xf1,s,pBuf);
			for (off1 += 1; off1 < off2;off1 += 1)
				Magic1(off1,yf1*yf2,s,pBuf);
			Magic1(off1,yf1*xf2,s,pBuf);
			}


	int		GetPutOptions(bool bPut = FALSE);
	int		GetNewOptions();
	int		PutNewOptions();
	int 	MakeImprint();
	void	SetupPalettes(int code);
	int		LoadDefaultPalette(BYTE * pPal);
	int 	ForgeImage(HPBYTE hpDst, UINT which);
	int		ForgeImprint(UINT ow, UINT oh);
	UINT 	InsertCache(UINT Start, UINT End);
	CLevel * GetLevelPtr(UINT Level, bool bMake = 0);
	bool		SelectLevel(UINT Level=9999, bool bMake = 0);
	CCell * GetCellPtr(CLevel * pLevel, UINT Frame, bool bMake);
	CMyObj * GetImagePtr(CCell * pCell, UINT Which, bool bMake);
	int  	WriteImage(HPBYTE hpDst, DWORD key, UINT which);
	int  	WriteOverlay(HPBYTE hpDst, UINT Frame, UINT Level,UINT w, UINT h);
	void  	PutImage(HPBYTE hpDst, UINT Frame, UINT Level, UINT which);
	UINT	SetupCache(bool bRead, bool bInit);
	void	MakeInfo(bool bRead);
	UINT	GetLevelInfo(UINT what, UINT level, UINT def);
	void	PutLevelInfo(UINT what, UINT level, UINT value);
	void 	DeleteLevels();
	void 	GetJpegImage(UINT Frame);
#ifdef CHECKING
	int Bump(DWORD key, int which = 0);
	CHKENTRY * m_pEntry;
	UINT	m_nEntries;
#endif
	UINT	m_depth;
	UINT	m_width;
	UINT	m_height;
	UINT	m_size;
	UINT 	m_start;
	UINT 	m_stop;
	UINT	m_thumbw;
	UINT	m_thumbh;
	UINT	m_x;
	UINT	m_y;
	UINT	m_w;
	UINT	m_h;
//	UINT	m_scale;
	UINT	m_factor;
	UINT	m_origfactor;
	UINT	m_zoom;
	UINT	m_frames;
	UINT	m_max_frames;
	UINT	m_snd_levels;
	UINT	m_levels;
	bool	m_bOptLock;	// if set don't write options to dbase
	UINT	m_flags;	// 0 is thumb dirty, 1 is color mode, 2 is red
	UINT	m_oflags;	// overrides
	UINT 	m_uState;
	bool	m_bModified;
	bool	m_bRepaired;
	DWORD	m_dwId;
	UINT	m_CurFrame;
	UINT	m_CurLevel;
	UINT	m_MinBG;

	UINT	m_rate;	
	UINT	m_snip;	
	bool	m_bStory;
	bool	m_bBlind;
	bool m_bLoading;
	UINT	m_nNewRecords;
	UINT	m_srate;
	bool	m_bSound;
	UINT	m_nBuddy0;
	UINT	m_nBuddy1;
	UINT	m_nBuddy2;
	UINT	m_nBuddy3;
	UINT	m_OptFlags; // 1 is Quiet, 2 is save cache, 
				  // 4 & 8 are telecine value, 
	char	m_xcellname[20];
	UINT	m_vmark0;	
	UINT	m_smark0;	
	UINT	m_vmark1;	
	UINT	m_smark1;	
	UINT	m_vmark2;	
	UINT	m_smark2;
	int		m_embed_out;
	int		m_embed_trim;
	int		m_embed_in;
	int 	m_volume0; // +- 100
	int 	m_volume1;
	int 	m_volume2;
	int 	m_mvolume; // master
	char	m_wave0[300];
	char	m_wave1[300];
	char	m_wave2[300];
	UINT	m_jpeg_quality;
	int		m_nLipx;
	int		m_nLipy;
	int		m_nLipw;
	int		m_nLiph;
	char	m_LipFile[300];
	DWORD	m_levelskey;
	UINT	m_cache_state;
	LPBYTE * m_pCache;
	CMiniJpegEncoder * m_pJEnc;
	CMiniJpegDecoder * m_pJDec;
	BYTE *  m_pJpeg;
	BYTE *  m_pCamBuf;
	bool	m_bDoCam;
	UINT	m_CurPeg;
	UINT	m_CamPeg;
	UINT	m_CamPegSel;
	UINT	m_CamState;
	BYTE	* m_pImprint;
	UINT	m_nErrors;
	UINT *	m_pErrors;
	BYTE * m_pLog;
	UINT		m_logsize;
	UINT	* m_pFlags;
	UINT	m_nCells;
	BYTE *  m_pBG;
	UINT	m_BGw;
	UINT	m_BGh;
	UINT	m_BGd;
	UINT	m_BGk;
	UINT	m_BGmin;
	CCellEntry *  m_pCellCache;
	CLayers *  m_pLayers;
	CLevels *  m_pLevels;
	CLevel * * m_pLevelArray;
	CNewPals * m_pPals[NBR_PALS];
	CCamera * m_pCamera;
	UINT *	m_pXY;
	UINT *	m_pInfo;
	UINT	m_info;
	LINKENTRY *	m_pLinks;
	UINT	m_links;
	UINT	m_linkz;  // size of pLink area
	bool    m_bAppended;
	UINT *	m_pClipBoard;
	UINT	m_ClipFrame;
	UINT	m_ClipLevel;

	std::unique_ptr<ResourceLoader> m_pResourceLoader;

friend class CLayers;
};
#endif
