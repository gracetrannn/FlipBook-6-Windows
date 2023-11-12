#ifndef _CLAYERS_H_
#define _CLAYERS_H_
#include "MyObj.h"

class CLayer
{
public:
	UINT	kind;
	UINT	flags;	// 1 is modified, 2 is active
	BYTE* pData;
};

UINT TestModel(LPCSTR name, UINT width, UINT height);
class CSceneBase;
class CUndo;
class CLevel;
class CLevelTable;
//class CColors;
class CNewPals;
class CCell;
class CFloat;
class CLayers
{
public:
	CLayers();
	~CLayers();
	UINT Layer(int which = -1);
	UINT Setup(CSceneBase * pScene, bool bScene, UINT Level = NEGONE);
	UINT LoadModel(UINT Level, bool bCreate = 0);
	UINT SaveModel(LPCSTR name = 0);
	bool IsOverlay() { return m_pOverlay ? TRUE : FALSE;};
	BYTE * GetClip( RECT & rcSelect,UINT  code, BYTE * pMask);
	bool Crop(RECT & rcSelect);
	void ApplyFloat(CFloat * pFloat, bool bMapPalette = 0);
	void DupCell(UINT Frame, UINT Level);
	void ChangeFrame(UINT frame) { m_frame = frame;};
	bool	CanModel() { return (m_nLayers < 3) || m_pOverlay ? FALSE : TRUE;};
	UINT Select(UINT Frame, UINT Level);
	UINT SelectLayer(UINT Layer);
	void MakeDeltaMask(UINT newrad);
	UINT Get(UINT Frame, UINT Level);
	UINT Put(bool bForce = FALSE);
	bool Modified() { return m_bModified;};
	bool GetFakeFrame(UINT &FakeFrame, UINT Level);
	bool NeedFake(UINT Frame);
	bool UseFake(UINT cellkey);
	UINT HaveAlpha(UINT x, UINT y, UINT * pLayer = 0, BYTE * p = 0);
	UINT DisPaint(UINT x, UINT y);
	UINT GetRGB(int x, int y, UINT &r,UINT &g,UINT &b);
	UINT GetIndex(UINT x, UINT y);
	UINT GetAlpha(UINT x, UINT y);
	UINT GetDotAlpha(UINT x, UINT y);
	void PutDotAlpha(UINT x, UINT y, BYTE v);
	bool CanUndo();
	bool CanRedo();
	void Undo(UINT index = NEGONE);
	void Redo();
	void CleanUp(UINT code);
	BYTE * LastUndo();
	UINT  LastUndoIndex();
	void DrawInit(UINT color, UINT type, bool bErasing);
	void DrawMono(UINT x, UINT y);
	void MonoMask(int dx, int dy, UINT x, UINT y);
	void DrawDot(int x, int y, BYTE alpha);
	void Change(UINT which, UINT v);
	void Flipper(UINT mask);
	UINT FillOffset(int x, int y, UINT index, bool bErase, UINT kind);
	UINT Fill(int x, int y, UINT index, bool bErase, UINT kind);
	UINT Blend(int x, int y, UINT radius);
	void DoDot(int x, int y, int c1, int c2);
	UINT Flood(UINT index, int kind);
	UINT Clear();
	void StashUndo();
	int  CDespeckle(int count);
	int  UnCranny(int count);
	int  Speckle(int count);
	int	 Magical();
	int	 DeGapper(int count,UINT color);
	void ApplyCell(BYTE * pDst,  CSceneBase * pScene, 
				CNewPals * pPals, CLevelTable * pTable, CCell * pCell);
//	bool FakeIt(BYTE * hpDst, UINT Frame, UINT Level);
//	bool FakeIt(BYTE * hpDst, CNewPals * pPals, CLevelTable * pTab);
	bool FakeIt(BYTE * hpDst);
	void Update(BYTE * pBits, BYTE * pBg, UINT pitch, bool BAll = 0);
	void LoadControl(UINT Frame, UINT Level, bool bClear = FALSE);
    bool GetPointerToLayerImage(BYTE*& pData, UINT kind);
    void GetLayerImage(BYTE* pBits, UINT kind, bool premultipliedAlpha = true, bool swapRedBlue = false);
    void GetImage(BYTE* pBits, UINT pitch);
    
    UINT CurrentFrame() { return m_frame; }
    UINT CurrentLevel() { return m_level; }
    void UpdateLayer(void* data, size_t size, UINT kind);
    
	bool	m_bDirty; // needs update
	bool	m_bFirstDot;
	UINT	m_nFlags;// 1 is for paint, 2 is for buddy paint, 4 is for FG
	int		m_minx;
	int		m_miny;
	int		m_maxx;
	int		m_maxy;
	UINT 	Width() { return m_width;};
	UINT 	Height() { return m_height;};
//	BYTE * 	InitPalette();
	BYTE *  m_pControl;		// fill control
	CLevelTable * LevelTable(bool bPut = 0);
//	CNewPals * Pals() { return m_pPals;};
	LPCSTR LayerName(int layer);
	UINT FillStack(UINT pos);
	UINT	m_zx;
	UINT	m_zy;
	void	NewPalette();
	bool	Solid() { return m_bSolid;};
protected:
	BYTE * PushUndo(UINT Layer, bool bSkip = 0);
	void UpdateColor(BYTE * pBits, UINT pitch);
	void UpdateInk(BYTE * pBits, UINT pitch);
	void UpdateGray(BYTE * pBits, BYTE * pSkin,UINT pitch);
#ifdef DOMONO
	UINT MakeMono();
	void MakeMonoMask();
	void UpdateMono();//BYTE * pBits, BYTE * pSkin,UINT pitch);
	void InitMono();
	void MonoToInk();
	void MonoLine(UINT x, UINT y);
#endif
	void ApplyLayer(BYTE *pDst, BYTE * pSrc, BYTE * pPaint,BYTE * pInk,
					UINT kind, UINT qq, CNewPals * pPals);
	UINT CreateLayers();
	void BlurIt(UINT layer);
	int  Findruns(int x, int y, int z, int py);
	int  Findruns2(int x, int y, int z, int py);
	int  Try(int x, int y, int px, int py);
	void	EmptyUs();
	void MonoDots(UINT x, UINT y, UINT v);
	UINT m_nCurLayer;
	UINT	m_color;
	UINT	m_dot_type;
	UINT	m_width;
	UINT	m_height;
	UINT	m_pitch;
	BYTE *	m_pOverlay;
	bool	m_bModified;
	bool	m_bNeedFake;
	UINT	m_cellkey;
	UINT	m_frame;
	UINT	m_level;
	UINT	m_nLayers;
	UINT m_nInk;
	UINT m_nPaint;
	UINT 	m_nUndo;
	UINT 	m_nRedo;
	UINT 	m_nPushCount;
	bool	m_bErasing;
	bool	m_bSolid;
#ifdef DOMONO
	UINT	m_nMono;		// scale factor for mono
	BYTE * m_pMono;			// mono buffer
	bool	m_bMonoModified;
#endif
	//CColors * m_pColors;
	UINT	m_prevx;
	UINT	m_prevy;
	BYTE  * m_pFillStack;
	UINT	m_nFillMax;
	UINT	m_nFillCount;
	UINT	m_nFillIndex;
	CLayer * m_pLayers;
	UINT	m_nMaxUndo;
	CUndo * m_pUndo;
	CSceneBase * m_pScene;
	CLevelTable * m_pTable;
	CNewPals * m_pPals;
	bool 	m_bHasName;
	char 	m_cell_name[16];
	char	m_name[300];
};
#endif
