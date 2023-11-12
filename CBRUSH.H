#ifndef _CBRUSHER_H_
#define _CBRUSHER_H_

typedef struct {
	int 	x;
	int		y;
	int 	d;
	int		r;
} ZPOINT;

class CLayers;
class CBrusher
{
public:
	CBrusher();
	~CBrusher();
	void StartBrush();
	void AppendBrush(CPoint point);
	void StopBrush();
	void Init(CLayers * pLayers);
	void DrawObject(UINT which, CPoint p1, CPoint p2, UINT rad, UINT den);
	void DrawCurve(CPoint p1, CPoint p2, CPoint p3, CPoint p4,
								UINT rad, UINT den);
	void SetDensity(UINT v) { m_density = v;};
	void SetRadius(UINT v);// { m_density = v;};
	void SetTexture(UINT v) { m_texture = v;};
	BYTE * CursorBits(UINT & extra, UINT radius);
	UINT Factor() { return m_factor;};
	void SetSmart(BOOL bOn);
	void Replay();
	void SetSolid(BOOL bSolid) { m_bSolid = bSolid;};
protected:
	void AppendIt(int x, int y);
	BOOL SetupCache(BOOL bCaching);
	void UpdateCache();
	UINT CheckCache(UINT x, UINT y);
	void SetMyRadius(UINT v);// { m_density = v;};
	void DumpSmart();
	CLayers * m_pLayers;
	void ApplyBrush(int x, int y);//, int r, int d);
	void MakeBrush(UINT r);
	void MakeBrushes();
	void MakeMasks();
	void DrawBegin(UINT x, UINT y, UINT radius, UINT density);
	void DrawAgain(UINT x, UINT y);
	void LineTo(UINT x, UINT y);
	void ZPoint(int i, int x, int y, int r, int d);
	void ZPoint(int i, int x, int y);
	void DoFront();
	void DoMiddle();
	void DoBack();
	void Smoother();
	BOOL m_bSolid;
	UINT m_ramp;
	BOOL m_bShow;
	UINT m_radius;
	UINT m_density;
	int m_d1;
	int m_d2;
	int m_d3;
	int m_r1;
	int m_r2;
	int m_r3;
	UINT m_texture;
	UINT m_factor;
	UINT m_width;
	UINT m_height;
	UINT m_brushw;
	UINT m_brushf;
	BOOL m_bCaching;
	UINT m_offsets[160];
	UINT m_masks[16];
	BYTE * m_pMasks;
	BYTE * m_pbz;
	BYTE * m_pbb;

	UINT m_cache_count;
	UINT m_first;
	UINT m_start;
	UINT m_stop;
	UINT * m_pq;
	UINT * m_pz;
	UINT * m_pc;
//
//	curve stuff
//
	BOOL	m_bFirst;
	int 	m_count;
	ZPOINT	m_p[4];
	int		m_px;
	int		m_py; //drawing previous
	int 	m_pdir; //previous direction
	int m_prevx;
	int m_prevy;
	int m_logc;
	ZPOINT  m_log[100];
	POINT *  m_pSmarts;
	int  * m_pInfo;
	UINT m_nSmarts;

};
#endif
