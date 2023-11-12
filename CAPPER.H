#ifndef CAPPERDEF

#ifndef FLIPBOOK_MAC
#include <streams.h>
#include "qedit.h"
#include "fbqt.h"
#endif

class CGrabCB;
class CCrossbar;
class CCapDlg;

class CCapper
{
public:
	CCapper();
	~CCapper();
	BOOL Setup(HWND hView, CWnd * pParent, LPCSTR pDevName);
	BOOL FillDeviceList();
	void UpdateUI();
	void SetRes();
	BOOL SelectDevice(int which);
	BOOL	Dialog(int which);
	void Destroy();
	HWND m_hWndView;
	int m_nDVSize;
	int Devices() { return m_nDevices;};
	int Options() { return m_Options;};
	UINT m_width;
	UINT m_height;
	UINT m_depth;
	UINT Width();
	UINT Height();
	ImageDataCallback m_callback;
	CCapDlg * m_pCallObj;
	LPCSTR CurName() { return m_szDevice;};
	CWnd * m_pParent;
	char m_szDevices[20][50];
	char m_szOptions[20][50];
	UINT m_nOptions[20];
	int m_Options;
protected:
	void ResizeWindow(int w, int h, BOOL bRecurse = 0);
	void ErrMsg(LPCSTR txt);
	BOOL StopPreview();
	BOOL StartPreview();
	void ChooseDevice(int which);
	void ReleaseMonikers();
	BOOL MakeBuilder();
	BOOL MakeGraph();
	void NukeDownstream(IBaseFilter *pf);
	void TearDownGraph();
	BOOL InitCapFilters();
	void FreeCapFilters();
	BOOL BuildPreviewGraph();
	BOOL SetGrabber();
	UINT SetGrabber2(UINT depth);
	void VideoFormat();


	BOOL FormatDialog();
	BOOL SourceDialog();
	BOOL DisplayDialog();
	BOOL VidCapDialog();
	BOOL CapturePinDialog();
	BOOL PreviewPinDialog();
	BOOL CrossbarDialog();
	BOOL Crossbar2Dialog();
	BOOL TVTunerDialog();
	BOOL DVDialog();
	int  DVSize(int val = -1);

    ICaptureGraphBuilder2 *m_pBuilder;
    ISampleGrabber * m_pGrab;
    IMediaControl  * m_pControl;
    IAMVfwCaptureDialogs * m_pDlg;
    IAMStreamConfig *pVSC;      // for video cap
    IBaseFilter *pVCap;
    IBaseFilter *pDVCodec;
    IGraphBuilder *pFg;
    BOOL m_bPreviewGraphBuilt;
    BOOL m_bPreviewing;
	BOOL	m_bHaveDevices;
	int m_current;
    IMoniker * m_pMonikers[10];
	int	m_nDevices;
    BOOL m_bWantPreview;
	char m_szDevice[80];
	WCHAR wachFriendlyName[120];
    CCrossbar * m_pCrossbar;
   LONG NumberOfVideoInputs;
	int	m_nFrameRate;
	CGrabCB * m_pCB;
};

#define CAPPERDEF
#endif
