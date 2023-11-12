#ifndef _DSIMPORT

#include "dshow.h"
struct IGraphBuilder;
struct  MySampler;
struct IBaseFilter;
struct  IMediaControl; 
struct  IPin;

class CSampleGrabberCB;
struct ISampleGrabber;
typedef int (*VideoCallback)(int code, void * buf, void *userdata);

class CSketchDoc;

class CDSImport
{
public:
	CDSImport();
	~CDSImport();
	int SelectFile(LPCSTR name, UINT flags); // 1 is video, 2 is audio
	void SetCallback( VideoCallback pVCB, void * pVClass);
	int Function(int code); // 0 is run, 1 is stop, 2 is percent
	BOOL HasSound() { return m_bHasSound;};
	int Frames() {return m_nFrames;};
	int Rate() {return m_nRate;};
	int Width() {return m_nWidth;};
	int Height() {return m_nHeight;};
	int WriteWave(LPCSTR name);
	void SetRange(UINT First, UINT Last, UINT arg);
	int CBHandler(int code, void * pBuf);
protected:
	BOOL TryAVI(WCHAR * wname);
	BOOL TryWMV();
	BOOL SetupAudio();
	BOOL m_bStopping;
	BOOL m_bHasSound;
	UINT m_nFrames;
	UINT m_nRate;
	UINT m_nWidth;
	UINT m_nHeight;
    IGraphBuilder*  m_pGraph;
    ICaptureGraphBuilder2*  m_pBuilder;
	IBaseFilter*    m_pSrc;
	IBaseFilter*   m_pSplit;
	IBaseFilter*   m_pWavDest;
	IBaseFilter*   m_pGrabberF;
    IBaseFilter *  m_pNull1F;
    IBaseFilter *  m_pNull2F;
	ISampleGrabber * m_pGrabber;
	MySampler * m_pSampler;
	CSampleGrabberCB * m_pCB;
	void * m_pVClass;
	VideoCallback m_pVCB;	
	IMediaControl * m_pCtrl;
	BYTE * m_pAudio;
	WAVEFORMATEX m_wf;
};

#define _DSIMPORT
#endif
