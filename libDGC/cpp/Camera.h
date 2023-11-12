#ifndef _CAMERA_H_
#define _CAMERA_H_
#include "MyObj.h"

class CMyIO;
class CSceneBase;

bool CheckCamera(CMyIO * pIO, DWORD key);

class CCamAttr
{
public:
enum kinds { CKIND_COMPUTE, CKIND_LINEAR, CKIND_SMOOTH};
	kinds  m_kind;
	double m_value;
};

class CCamAttrs
{
	CCamAttr m_x;
	CCamAttr m_y;
	CCamAttr m_scale;
	CCamAttr m_rotation;
	CCamAttr m_blur;
	CCamAttr m_alpha;
};

class CCamera
{
public:
	CCamera(CMyIO * pIO, UINT key);
	~CCamera();
	int Read();
	bool Modified() { return m_bModified;};
enum attrs {
	CATTR_Y, CATTR_X, CATTR_SCALE, CATTR_ROT,
			CATTR_BLUR, CATTR_ALPHA,CATTR_EXTRA};
//
//	group stuff
//
	bool GroupActive(UINT which);
	UINT GroupCount() { return m_Groups;};
	bool GroupName(LPSTR Name, UINT which, bool bPut = FALSE);
	UINT GroupAttach(UINT Peg, UINT Group);
	UINT GroupFindPeg(UINT Peg);	// returns group with peg
	bool IsKey(UINT Frame, UINT Level);
//
//	peg stuff
//
	bool PegActive(UINT which);
	UINT PegCount() { return m_Pegs;};
	UINT PegName(LPSTR Name, UINT which, bool bPut = FALSE);
	UINT PegCenter(double& cx, double& cy, UINT which, bool bPut = FALSE);
	UINT LevelCenter(double& cx, double& cy, UINT which, bool bPut = FALSE);
	UINT PegAttach(UINT Level, UINT Peg);
	UINT PegFindLevel(UINT Level);	// returns peg with level
	UINT PegFlags(UINT Peg, UINT Which, UINT Value = NEGONE);
	void Setup(CSceneBase * pScene);
	void Update();
	void InsertLevel(UINT start, UINT count);
	void DeletetLevel(UINT start, UINT count);
	UINT GetAttr(UINT attr, UINT Frame,UINT Peg,double& value,UINT & kind);
	double GetValue(UINT attr, UINT Frame, UINT Peg);
	UINT PutAttr(UINT attr, UINT Frame,UINT Peg,double value,UINT kind);
	UINT FindKey(UINT Peg, UINT frame, bool bUp);
	void Flush(bool bForce = 0);
	UINT SetupCell(UINT Frame, UINT Level);
	UINT Table(UINT * pTable, UINT ow, UINT oh, UINT iw, UINT ih, bool bBroadcast = 0);
	double Scale() { return m_scale;};
	double CamScale() { return m_cscale;};
	double Blur() { return m_blur;};
	UINT 	m_alpha;
	int	m_offx;
	int 	m_offy;
	UINT	m_factor;
	UINT	m_radius;
	UINT m_min_diff;
	UINT m_max_diff;
	void	Compute(UINT attr = 9999);
	void PegXY(int &pegx, int & pegy, UINT peg, UINT frame, UINT w, UINT h);
	void PegVXY(double &pegx, double &pegy, int x, int y, 
					UINT peg, UINT frame, UINT w, UINT h);

protected:

	void Linear(double * pt, int x1, double y1, int x2, double y2, int ei, int eo);
//	void Linear(double * pt, UINT count, UINT easein, UINT easeout,
//			double value1, double value2);
	void Simple1(double * pt, int x1, double y1, int x2, double y2, double slope);
	void Simple2(double * pt, int x1, double y1, int x2, double y2, double slope);
	void Curve( double* pt, int x1, double y1, int x2, double y2, 
				double slope1, double slope2);
	int Write();
	void Mover(bool bRead, BYTE * tp);
	void 	MakeModified();
	CMyIO	*m_pIO;
	CSceneBase * m_pScene;
	bool	m_bDeferWrite;
	UINT	m_Key;
	UINT	m_Frame;
	bool	m_bFieldComp;
	bool	m_bAttached;
	bool	m_bGroup;
	UINT	m_Level;	// for field composite
	UINT 	m_Frames;
	UINT	m_Levels;
	UINT	m_Groups;
	UINT	m_Pegs;
	bool	m_bModified;
	BYTE *  m_pPegs;
	BYTE *  m_pList;
//
//	peg settings
//
	double	m_xoffset;
	double 	m_yoffset;
	double	m_scale;
	double	m_rotation;
	double	m_dalpha;
	double 	m_blur;
	double 	m_pegx;
	double 	m_pegy;
//
//	group settings
//
	double	m_gxoffset;
	double 	m_gyoffset;
	double	m_gscale;
	double	m_grotation;
	double	m_gdalpha;
	double 	m_gblur;
	double 	m_gpegx;
	double 	m_gpegy;

//
//	camera settings
//
	double	m_cxoffset;
	double 	m_cyoffset;
	double	m_cscale;
	double	m_crotation;
	double	m_cdalpha;
	double 	m_cblur;

	double *  m_pData;
	UINT	m_count;
	UINT	m_limit;
	bool	m_dirty[CATTR_EXTRA];
};

class CCamMoves
{
public:
	CCamMoves();
	~CCamMoves();
	int Write(CCamera * pCamera, LPCSTR name, LPCSTR SceneName, 
				CSceneBase * pScene , UINT flags,
			UINT StartFrame, UINT StartLevel, UINT EndFrame, UINT EndLevel);
	int Read(CCamera * pCamera, LPCSTR name);
	int Apply(UINT peg, UINT flags, UINT StartFrame, UINT StartLevel);
	UINT Pegs() { return m_nPegs;};
	bool IsPeg(UINT index);
	bool HasGroup(UINT peg);
	void Name(UINT index, CString & idd, CString & name);
	bool NeedComposite() { return m_bNeedComposite;};
protected:
	int WritePeg(UINT peg, UINT sf, UINT ef, UINT flags);
//	int WriteAttr(CStdioFile & file, UINT peg, UINT frame, UINT which); 
	int WriteLine(LPCSTR buf, bool bComment = 0);
//	int ReadAttr(CStdioFile & file); 
//	int WriteLine(CStdioFile & file, LPCSTR buf);
	int Initialize();
	int AddPeg(UINT peg,UINT grp, double pcx,double pcy, LPCSTR name);
	int AddAttr(UINT peg,UINT frame,UINT attr,double v, UINT kind);
	CStdioFile * m_pFile;
	CCamera * m_pCamera;
	CSceneBase * m_pScene;
	bool m_bNeedComposite;
	UINT m_nPegs;
	UINT * m_pItems;
	BYTE * m_pPegs;
};

#endif
