//#define NOBUG
#include "stdafx.h"
#ifndef FLIPBOOK_MAC
#include <atlbase.h>
extern CComModule _Module;
#endif
//#include <streams.h>
//#include "stdafx.h"
#include "capper.h"
#include "crossbar.h"
#include "resource.h"
#include "mydialog.h"
#define CAP_DLG_FORMAT 0
#define CAP_DLG_SOURCE 1
#define CAP_DLG_DISPLAY 2
#define CAP_DLG_DLG 3
#define CAP_DLG_CAPTPIN 4
#define CAP_DLG_PREVPIN 5
#define CAP_DLG_CROSS 6
#define CAP_DLG_CROSS2 7
#define CAP_DLG_TUNER 8



class CGrabCB: public CUnknown, public ISampleGrabberCB
{
public:
    DECLARE_IUNKNOWN;

    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void **ppv)
    {
        if( riid == IID_ISampleGrabberCB )
        {
            return GetInterface((ISampleGrabberCB*)this, ppv);
        }
        return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }

    // ISampleGrabberCB methods

    STDMETHODIMP SampleCB(double SampleTime, IMediaSample *pSample)
    {
        DPF("Sample time: %f", SampleTime);
        return E_NOTIMPL;//S_OK;
    }

    STDMETHODIMP BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen)
    {
//       DPF("BufferCBSample size:%d", BufferLen);
		m_pCapper->m_width = m_w;
		m_pCapper->m_height= m_h;
		m_pCapper->m_depth = m_d;
		m_pCapper->m_callback(kRawDataType,pBuffer,BufferLen,m_pCapper->m_pCallObj);
        return S_OK;//E_NOTIMPL;
    }
    
    // Constructor
    CGrabCB(CCapper * pCapper ) : CUnknown("SGCB", NULL)
    {m_pCapper = pCapper; m_w = 0;}
	CCapper * m_pCapper;
	UINT m_w, m_h, m_d;
};


class CDVSizeDlg : public CMyDialog
{
// Construction
public:
	CDVSizeDlg(CWnd* pParent = NULL);	// standard constructor
// Dialog Data
	//{{AFX_DATA(CDVSizeDlg)
	enum { IDD = IDD_DV_DIALOG };
	int	m_nSize;
	//}}AFX_DATA
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDVSizeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL
};

CDVSizeDlg::CDVSizeDlg(CWnd* pParent /*=NULL*/)
		: CMyDialog(CDVSizeDlg::IDD,pParent)
{
}

void CDVSizeDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_720x480, m_nSize);
}


CCapper::CCapper()
{
	memset(this,0,sizeof(CCapper));
	m_current = -1;
	m_nFrameRate = 0;
	m_nDVSize = 0;
	m_pCB = new CGrabCB(this);
	m_pGrab = 0;
	m_pCrossbar = 0;
}

CCapper::~CCapper()
{
	Destroy();
	delete m_pCB;
}

void CCapper::Destroy()
{
	StopPreview();
	TearDownGraph();
	FreeCapFilters();
	ReleaseMonikers();
	delete m_pCB;
	m_pCB = 0;
}

//BOOL CCapper::Setup(CWnd * pView, CWnd * pParent, LPCSTR pName, PCAPINFO pInfo)
BOOL CCapper::Setup(HWND hView, CWnd * pParent, LPCSTR pName)
{
DPF("setup");
//	m_pInfo = pInfo;
	m_pParent = pParent;
	m_hWndView = hView;
	memset(&m_pMonikers, 0, sizeof(m_pMonikers));
	m_current = -1;
    m_bHaveDevices = 0;
	strcpy_s(m_szDevice, sizeof(m_szDevice),pName);
	FillDeviceList();
    m_bWantPreview = 1;
	m_nFrameRate = 5;
	ChooseDevice(99);
//	m_pInfo->Status = 1;
    return TRUE;
}

// Make a graph builder object we can use for capture graph building
//
BOOL CCapper::MakeBuilder()
{
    // we have one already
    if (m_pBuilder)
		return TRUE;

    HRESULT hr = CoCreateInstance((REFCLSID)CLSID_CaptureGraphBuilder2,
			NULL, CLSCTX_INPROC, (REFIID)IID_ICaptureGraphBuilder2,
			(void **)&m_pBuilder);
    return (hr == NOERROR) ? TRUE : FALSE;
}


// Make a graph object we can use for capture graph building
//
BOOL CCapper::MakeGraph()
{
    // we have one already
    if (pFg)
	return TRUE;

    HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
			       IID_IGraphBuilder, (LPVOID *)&pFg);
    return (hr == NOERROR) ? TRUE : FALSE;
}


// make sure the preview window inside our window is as big as the
// dimensions of captured video, or some capture cards won't show a preview.
// (Also, it helps people tell what size video they're capturing)
// We will resize our app's window big enough so that once the status bar
// is positioned at the bottom there will be enough room for the preview
// window to be w x h
//

void CCapper::ResizeWindow(int w, int h, BOOL bRecurse)
{
	DPF("resizing,w:%d,h:%d,r:%d",w,h,bRecurse);
return;
    RECT rcW, rcC;
    int xExtra, yExtra;
    int cyBorder = GetSystemMetrics(SM_CYBORDER);

	
	GetWindowRect(m_hWndView, &rcW);
    GetClientRect(m_hWndView, &rcC);
    xExtra = rcW.right - rcW.left - rcC.right;
    yExtra = rcW.bottom - rcW.top - rcC.bottom + cyBorder;// + statusGetHeight();
    
    rcC.right = w;
    rcC.bottom = h;
    SetWindowPos(m_hWndView, NULL, 0, 0, rcC.right + xExtra,
				rcC.bottom + yExtra, SWP_NOZORDER | SWP_NOMOVE);

    // we may need to recurse once.  But more than that means the window cannot
    // be made the size we want, trying will just stack fault.
    //
    if (!bRecurse && 
			((rcC.right + xExtra != rcW.right - rcW.left &&
				w > GetSystemMetrics(SM_CXMIN)) ||
		(rcC.bottom + yExtra != rcW.bottom - rcW.top)))
	ResizeWindow(w,h,1);
}


// Tear down everything downstream of a given filter
void CCapper::NukeDownstream(IBaseFilter *pf)
{
	DPF("Nuking...");

    IPin *pP, *pTo;
    ULONG u;
    IEnumPins *pins = NULL;
    PIN_INFO pininfo;
    HRESULT hr = pf->EnumPins(&pins);
    pins->Reset();
    while (hr == NOERROR)
		{
        hr = pins->Next(1, &pP, &u);
		if (hr == S_OK && pP)
			{
	    		pP->ConnectedTo(&pTo);
	    		if (pTo)
				{
	        		hr = pTo->QueryPinInfo(&pininfo);
	        		if (hr == NOERROR)
					{
		    			if (pininfo.dir == PINDIR_INPUT)
						{
		        			NukeDownstream(pininfo.pFilter);
		        			pFg->Disconnect(pTo);
		        			pFg->Disconnect(pP);
	                		pFg->RemoveFilter(pininfo.pFilter);
		    				}
				
	            		pininfo.pFilter->Release();
					}
				pTo->Release();
	    			}
	    		pP->Release();
//			pFg->Disconnect(pP);
			}
    	}
    if (pins)
        pins->Release();
}



// Tear down everything downstream of the capture filters, so we can build
// a different capture graph.  Notice that we never destroy the capture filters
// and WDM filters upstream of them, because then all the capture settings
// we've set would be lost.
//
void CCapper::TearDownGraph()
{
	DPF("teardopwn");
    // destroy the graph downstream of our capture filters
    if (pVCap)
		NukeDownstream(pVCap);
//	pVCap = NULL;
    m_bPreviewGraphBuilt = FALSE;
}


// create the capture filters of the graph.  We need to keep them loaded from
// the beginning, so we can set parameters on them and have them remembered
//
BOOL CCapper::InitCapFilters()
{
//	IBaseFilter     *pF = NULL;
//	IBaseFilter     *pNullRenderer = NULL;
	IAMExtDevice            *pExtDev              = NULL;

	pDVCodec = 0;
    HRESULT hr;
    BOOL f;
    UINT uIndex = 0;

//    fCCAvail = FALSE;	// assume no closed captioning support

    f = MakeBuilder();
    if (!f) {
	ErrMsg("Cannot instantiate graph builder");
	return FALSE;
    }

//
// First, we need a Video Capture filter, and some interfaces
//

	pVCap = NULL;
    if(m_current >= 0)
    {
		IPropertyBag *pBag = 0;
		wachFriendlyName[0] = 0;
    	IMoniker *pm = m_pMonikers[m_current];
        hr = pm->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
        if(SUCCEEDED(hr))
        {
            VARIANT var;
            var.vt = VT_BSTR;
            hr = pBag->Read(L"FriendlyName", &var, NULL);
            if (hr == NOERROR) {
                strcpy_s((char *)wachFriendlyName,sizeof(wachFriendlyName),
								(char *)var.bstrVal);
                SysFreeString(var.bstrVal);
            }
		}
            pBag->Release();
         hr = pm->BindToObject(0, 0, IID_IBaseFilter, (void**)&pVCap);
    }

    if (pVCap == NULL) {
//	ErrMsg("Error %x: Cannot create video capture filter", hr);
	ErrMsg("Error, Cannot create video capture filter");
	goto InitCapFiltersFail;
    }

    //
    // make a filtergraph, give it to the graph builder and put the video
    // capture filter in the graph
    //

    f = MakeGraph();
    if (!f) {
		ErrMsg("Cannot instantiate filtergraph");
		goto InitCapFiltersFail;
    	}
    hr = m_pBuilder->SetFiltergraph(pFg);
    if (hr != NOERROR) {
		ErrMsg("Cannot give graph to builder");
		goto InitCapFiltersFail;
    	}

    hr = pFg->AddFilter(pVCap, NULL);
    if (hr != NOERROR) {
//	ErrMsg("Error %x: Cannot add vidcap to filtergraph", hr);
		ErrMsg("Error, Cannot add vidcap to filtergraph");
		goto InitCapFiltersFail;
	    }


	

	hr = m_pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
			&MEDIATYPE_Video,   pVCap,IID_IAMExtDevice, 
					reinterpret_cast<PVOID*>(&pExtDev));
	if (hr == S_OK)
		{
		hr = CoCreateInstance(CLSID_DVVideoCodec, NULL, CLSCTX_INPROC_SERVER, 
			IID_IBaseFilter, reinterpret_cast<PVOID *>(&pDVCodec));
		hr = pFg->AddFilter(pDVCodec, L"DV Codec");
		pExtDev->Release();	
		}




		if (pDVCodec)
			DVSize(m_nDVSize);

#define XXXX
#ifdef XXXX
    // !!! What if this interface isn't supported?
    // we use this interface to set the frame rate and get the capture size
    hr = m_pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
			&MEDIATYPE_Interleaved,
			pVCap, IID_IAMStreamConfig, (void **)&pVSC);
    if (hr != NOERROR) {
        hr = m_pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Video, pVCap,
				IID_IAMStreamConfig, (void **)&pVSC);
        if (hr != NOERROR) {
	    // this means we can't set frame rate (non-DV only)
	  //  ErrMsg("Error %x: Cannot find VCapture:IAMStreamConfig", hr);
	    ErrMsg("Error,Cannot find VCapture:IAMStreamConfig");
		}

    }
#endif
if (!pVSC) 
        goto InitCapFiltersFail;
SetRes();
if (SetGrabber())
        goto InitCapFiltersFail;


    // we use this interface to bring up the 3 dialogs
    // NOTE:  Only the VfW capture filter supports this.  This app only brings
    // up dialogs for legacy VfW capture drivers, since only those have dialogs
    hr = m_pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Video, pVCap,
				IID_IAMVfwCaptureDialogs, (void **)&m_pDlg);


    // Use the crossbar class to help us sort out all the possible video inputs
    // The class needs to be given the capture filters ANALOGVIDEO input pin
    {
        IPin        *pP = 0;
        IEnumPins   *pins;
        ULONG        n;
        PIN_INFO     pinInfo;
        BOOL         Found = FALSE;
	IKsPropertySet *pKs;
	GUID guid;
	DWORD dw;
	BOOL fMatch = FALSE;
    delete m_pCrossbar;
	m_pCrossbar = NULL;

        if(SUCCEEDED(pVCap->EnumPins(&pins))) {            
            while (!Found && (S_OK == pins->Next(1, &pP, &n))) {
                if (S_OK == pP->QueryPinInfo(&pinInfo)) {
                    if (pinInfo.dir == PINDIR_INPUT) {
			
			// is this pin an ANALOGVIDEOIN input pin?
    			if (pP->QueryInterface(IID_IKsPropertySet,
						(void **)&pKs) == S_OK) {
			    if (pKs->Get(AMPROPSETID_Pin,
					AMPROPERTY_PIN_CATEGORY, NULL, 0,
					&guid, sizeof(GUID), &dw) == S_OK) {
	    			if (guid == PIN_CATEGORY_ANALOGVIDEOIN)
					fMatch = TRUE;
			    }
			    pKs->Release();
    			}

			if (fMatch) {
                            HRESULT hrCreate=S_OK;
                            m_pCrossbar = new CCrossbar (pP,&hrCreate);
                            if (!m_pCrossbar || FAILED(hrCreate))
                                break;

                            hr = m_pCrossbar->GetInputCount(&NumberOfVideoInputs);
                            Found = TRUE;
			}
                    }
                    pinInfo.pFilter->Release();
                }
                pP->Release();
            }
            pins->Release();
        }
    }
    // potential debug output - what the graph looks like
    // DumpGraph(pFg, 1);
	//if (m_pCrossbar)
	//	m_pCrossbar->SetInputIndex (1);

    return TRUE;

InitCapFiltersFail:
    FreeCapFilters();
    return FALSE;
	
}


// all done with the capture filters and the graph builder
//
void CCapper::FreeCapFilters()
{
	DPF("freeing filters");
    if (pFg)
		pFg->Release();
    pFg = NULL;
    if (m_pBuilder)
		m_pBuilder->Release();
    m_pBuilder = NULL;
    if (pVCap)
		pVCap->Release();
    pVCap = NULL;
	if (pDVCodec)
		pDVCodec->Release();
	pDVCodec = 0;
   if (pVSC)
		pVSC->Release();
   pVSC = NULL;
//    if (pVC)
//		pVC->Release();
 //   pVC = NULL;
    if (m_pDlg)
		m_pDlg->Release();
    m_pDlg = NULL;
    if (m_pCrossbar)
		delete m_pCrossbar;
	m_pCrossbar = NULL;
}



// build the preview graph!
//
// !!! PLEASE NOTE !!!  Some new WDM devices have totally separate capture
// and preview settings.  An application that wishes to preview and then 
// capture may have to set the preview pin format using IAMStreamConfig on the
// preview pin, and then again on the capture pin to capture with that format.
// In this sample app, there is a separate page to set the settings on the 
// capture pin and one for the preview pin.  To avoid the user
// having to enter the same settings in 2 dialog boxes, an app can have its own
// UI for choosing a format (the possible formats can be enumerated using
// IAMStreamConfig) and then the app can programmatically call IAMStreamConfig
// to set the format on both pins.
//
BOOL CCapper::BuildPreviewGraph()
{
    HRESULT hr;
    if (m_bPreviewGraphBuilt)
		return TRUE;
	if (m_bPreviewing)
		return FALSE;

    // We don't have the necessary capture filters
    if (pVCap == NULL)
		return FALSE;


    // now tell it what frame rate to capture at.  Just find the format it
    // is capturing with, and leave everything alone but change the frame rate
    // No big deal if it fails.  It's just for preview
    // !!! Should we then talk to the preview pin?

    if (pVSC && m_nFrameRate)
		{
DPF("setting rate");
    	AM_MEDIA_TYPE *pmt;
		hr = pVSC->GetFormat(&pmt);
	// DV capture does not use a VIDEOINFOHEADER
		if (hr == NOERROR)
			{
		    if (pmt->formattype == FORMAT_VideoInfo)
				{
				VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)pmt->pbFormat;
				pvi->AvgTimePerFrame = (LONGLONG)(10000000 / m_nFrameRate);
				hr = pVSC->SetFormat(pmt);
				if (hr != NOERROR)
					ErrMsg("Cannot set frame rate for preview");
	    		}
			DeleteMediaType(pmt);
			}
    	}
// All done.

    // potential debug output - what the graph looks like
    // DumpGraph(pFg, 1);

    m_bPreviewGraphBuilt = TRUE;
    return TRUE;
}





// Let's talk about UI for a minute.  There are many programmatic interfaces
// you can use to program a capture filter or related filter to capture the
// way you want it to.... eg:  IAMStreamConfig, IAMVideoCompression,
// IAMCrossbar, IAMTVTuner, IAMTVAudio, IAMAnalogVideoDecoder, IAMCameraControl,
// IAMVideoProcAmp, etc.
//
// But you probably want some UI to let the user play with all these settings.
// For new WDM-style capture devices, we offer some default UI you can use.
// The code below shows how to bring up all of the dialog boxes supported 
// by any capture filters.
//
// The following code shows you how you can bring up all of the
// dialogs supported by a particular object at once on a big page with lots
// of thumb tabs.  You do this by starting with an interface on the object that
// you want, and using ISpecifyPropertyPages to get the whole list, and
// OleCreatePropertyFrame to bring them all up.  This way you will get custom
// property pages a filter has, too, that are not one of the standard pages that
// you know about.  There are at least 9 objects that may have property pages.
// Your app already has 2 of the object pointers, the video capture filter and
// the audio capture filter (let's call them pVCap and pACap)
// 1.  The video capture filter - pVCap
// 2.  The video capture filter's capture pin - get this by calling
//     FindInterface(&PIN_CATEGORY_CAPTURE, pVCap, IID_IPin, &pX);
// 3.  The video capture filter's preview pin - get this by calling
//     FindInterface(&PIN_CATEGORY_PREVIEW, pVCap, IID_IPin, &pX);
// 4.  The audio capture filter - pACap
// 5.  The audio capture filter's capture pin - get this by calling
//     FindInterface(&PIN_CATEGORY_CAPTURE, pACap, IID_IPin, &pX);
// 6.  The crossbar connected to the video capture filter - get this by calling
//     FindInterface(NULL, pVCap, IID_IAMCrossbar, &pX);
// 7.  There is a possible second crossbar to control audio - get this by 
//     looking upstream of the first crossbar like this:
//     FindInterface(&LOOK_UPSTREAM_ONLY, pX, IID_IAMCrossbar, &pX2);
// 8.  The TV Tuner connected to the video capture filter - get this by calling
//     FindInterface(NULL, pVCap, IID_IAMTVTuner, &pX);
// 9.  The TV Audio connected to the audio capture filter - get this by calling
//     FindInterface(NULL, pACap, IID_IAMTVAudio, &pX);
// 10. We have a helper class, CCrossbar, which makes the crossbar issue less
//     confusing.  In fact, although not supported here, there may be more than
//     two crossbars, arranged in many different ways.  An application may not
//     wish to have separate dialogs for each crossbar, but instead hide the
//     complexity and simply offer the user a list of inputs that can be chosen.
//     This list represents all the unique inputs from all the crossbars.
//     The crossbar helper class does this and offers that list as #10.  It is
//     expected that an application will either provide the crossbar dialogs
//     above (#6 and #7) OR provide the input list (this #10), but not both.
//     That would be confusing because if you select an input using dialog 6 or
//     7 the input list here in #10 won't know about your choice.
//
// Your last choice for UI is to make your own pages, and use the results of 
// your custom page to call the interfaces programmatically.



void CCapper::UpdateUI()
{
	if (!m_pParent)
		return;	// grabber, no dialog
	BOOL bOldWay = FALSE;
	m_Options = 0;
    // If this device supports the old legacy UI dialogs, offer them

    if (m_pDlg && !m_pDlg->HasDialog(VfwCaptureDialog_Format))
		{
		strcpy_s(m_szOptions[m_Options],sizeof(m_szOptions[0]),
					TEXT("Video Format..."));
		m_nOptions[m_Options++] = CAP_DLG_FORMAT;
		bOldWay = TRUE;
    	}
    if (m_pDlg && !m_pDlg->HasDialog(VfwCaptureDialog_Source))
		{
		strcpy_s(m_szOptions[m_Options],sizeof(m_szOptions[0]),
				TEXT("Video Source..."));
		m_nOptions[m_Options++] = CAP_DLG_SOURCE;
    	}
    if (m_pDlg && !m_pDlg->HasDialog(VfwCaptureDialog_Display))
		{
		strcpy_s(m_szOptions[m_Options],sizeof(m_szOptions[0]),
					TEXT("Video Display..."));
		m_nOptions[m_Options++] = CAP_DLG_DISPLAY;
    	}


    // don't bother looking for new property pages if the old ones are supported
    // or if we don't have a capture filter
    if (pVCap == NULL || bOldWay)
		return;


    // New WDM devices support new UI and new interfaces.
    // Your app can use some default property
    // pages for UI if you'd like (like we do here) or if you don't like our
    // dialog boxes, feel free to make your own and programmatically set 
    // the capture options through interfaces like IAMCrossbar, IAMCameraControl
    // etc.

    // There are 9 objects that might support property pages.  Let's go through
    // them.


    ISpecifyPropertyPages *pSpec;
    CAUUID cauuid;

    // 1. the video capture filter itself
	HRESULT hr;
    hr = pVCap->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
    if (hr == S_OK) {
        hr = pSpec->GetPages(&cauuid);
        if (hr == S_OK && cauuid.cElems > 0)
			{
			strcpy_s(m_szOptions[m_Options],sizeof(m_szOptions[0]),
						TEXT("Brightness..."));
		    m_nOptions[m_Options++] = CAP_DLG_DLG;
	    	CoTaskMemFree(cauuid.pElems);
			}
		pSpec->Release();
    	}

    // 2.  The video capture capture pin

    IAMStreamConfig *pSC;
    hr = m_pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
			&MEDIATYPE_Interleaved,
			pVCap, IID_IAMStreamConfig, (void **)&pSC);
    if (hr != S_OK)
        hr = m_pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Video, pVCap,
				IID_IAMStreamConfig, (void **)&pSC);
    if (hr == S_OK) {
        hr = pSC->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
        if (hr == S_OK) {
            hr = pSpec->GetPages(&cauuid);
            if (hr == S_OK && cauuid.cElems > 0) {
			strcpy_s(m_szOptions[m_Options],sizeof(m_szOptions[0]),
						TEXT("Format..."));
	        m_nOptions[m_Options++] = CAP_DLG_CAPTPIN;
	        CoTaskMemFree(cauuid.pElems);
	    }
	    pSpec->Release();
        }
	pSC->Release();
    }
#ifdef DOINGPREV
    // 3.  The video capture preview pin.
    // This basically sets the format being previewed.  Typically, you
    // want to capture and preview using the SAME format, instead of having to
    // enter the same value in 2 dialog boxes.  For a discussion on this, see
    // the comment above the MakePreviewGraph function.

    hr = m_pBuilder->FindInterface(&PIN_CATEGORY_PREVIEW,
				&MEDIATYPE_Interleaved, pVCap,
				IID_IAMStreamConfig, (void **)&pSC);
    if (hr != NOERROR)
        hr = m_pBuilder->FindInterface(&PIN_CATEGORY_PREVIEW,
				&MEDIATYPE_Video, pVCap,
				IID_IAMStreamConfig, (void **)&pSC);
    if (hr == S_OK) {
        hr = pSC->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
        if (hr == S_OK) {
            hr = pSpec->GetPages(&cauuid);
            if (hr == S_OK && cauuid.cElems > 0) {
//	        AppendMenu(hMenuSub,MF_STRING,MENU_DIALOG0+m_Options,TEXT("Video Preview Pin..."));
	    m_bDialogs[CAP_DLG_PREVPIN] = 1;//    iVCapPreviewPinDialogPos = m_Options++;
		CoTaskMemFree(cauuid.pElems);
	    }
	    pSpec->Release();
        }
	pSC->Release();
    }
#else
	if (pDVCodec)
		{
		strcpy_s(m_szOptions[m_Options],sizeof(m_szOptions[0]),
					TEXT("Digital Video Size..."));
		m_nOptions[m_Options++] = CAP_DLG_PREVPIN;
		}
#endif
    // 4 & 5.  The video crossbar, and a possible second crossbar

    IAMCrossbar *pX, *pX2;
    IBaseFilter *pXF;
    hr = m_pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Interleaved, pVCap,
				IID_IAMCrossbar, (void **)&pX);
    if (hr != S_OK)
        hr = m_pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Video, pVCap,
				IID_IAMCrossbar, (void **)&pX);
    if (hr == S_OK) {
		hr = pX->QueryInterface(IID_IBaseFilter, (void **)&pXF);
		if (hr == S_OK)
			{
            hr = pX->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
			if (hr == S_OK)
				{
				hr = pSpec->GetPages(&cauuid);
				if (hr == S_OK && cauuid.cElems > 0)
					{

					strcpy_s(m_szOptions[m_Options],sizeof(m_szOptions[0]),
							TEXT("Source..."));
	       			m_nOptions[m_Options++] = CAP_DLG_CROSS;//     iVCrossbarDialogPos = m_Options++;
		    			CoTaskMemFree(cauuid.pElems);
	        			}
	        		pSpec->Release();
            		}
            hr = m_pBuilder->FindInterface(&LOOK_UPSTREAM_ONLY, NULL, pXF,
				IID_IAMCrossbar, (void **)&pX2);
            if (hr == S_OK)
				{
                	hr = pX2->QueryInterface(IID_ISpecifyPropertyPages,
							(void **)&pSpec);
                	if (hr == S_OK)
					{
                    	hr = pSpec->GetPages(&cauuid);
                    	if (hr == S_OK && cauuid.cElems > 0)
						{
						strcpy_s(m_szOptions[m_Options],
							sizeof(m_szOptions[0]),TEXT("Second Crossbar..."));
	  					m_nOptions[m_Options++] = CAP_DLG_CROSS2;
		        			CoTaskMemFree(cauuid.pElems);
	            			}
	            		pSpec->Release();
                		}
	        		pX2->Release();
	    			}
 	    		pXF->Release();
        		}
		pX->Release();
    	}

    // 6.  The TVTuner

    IAMTVTuner *pTV;
    hr = m_pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Interleaved, pVCap,
				IID_IAMTVTuner, (void **)&pTV);
    if (hr != S_OK)
        hr = m_pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Video, pVCap,
				IID_IAMTVTuner, (void **)&pTV);
    if (hr == S_OK) {
        hr = pTV->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
        if (hr == S_OK) {
            hr = pSpec->GetPages(&cauuid);
            if (hr == S_OK && cauuid.cElems > 0) {
		strcpy_s(m_szOptions[m_Options],sizeof(m_szOptions[0]),
					TEXT("TV Tuner..."));
		m_nOptions[m_Options++] = CAP_DLG_TUNER;
		CoTaskMemFree(cauuid.pElems);
	    }
	    pSpec->Release();
        }
 	pTV->Release();
    }

    // no audio capture, we're done
 //   if (pACap == NULL)
//	return;
}

// Check the devices we're currently using and make filters for them
//
void CCapper::ChooseDevice(int which)
{
DPF("choose 1,dev:%d",which);
	USES_CONVERSION;
//    #define VERSIZE 40
//    #define DESCSIZE 80
//    int versize = VERSIZE;
//    int descsize = DESCSIZE;
//    WCHAR wachVer[VERSIZE], wachDesc[DESCSIZE];
//    TCHAR tachStatus[VERSIZE + DESCSIZE + 5];


	if ((which == m_current) || !m_nDevices)
		return;
	int old = -1;
	if (which == 99)
		{
		if (m_current == -1)
			m_current = 0;
    	m_pMonikers[m_current]->AddRef();
		}
	else
		{
		old = m_current;
		if (m_bPreviewing)
	        StopPreview();
		if (m_current >= 0)
    		m_pMonikers[m_current]->Release();
    	m_pMonikers[which]->AddRef();
	    if (m_bPreviewGraphBuilt)
	        TearDownGraph();
	    FreeCapFilters();
		m_current = which;
		}
		
	IPropertyBag *pBag = 0;
	HRESULT hr =  
m_pMonikers[m_current]->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
	if(SUCCEEDED(hr)) {
	    VARIANT var;
	    var.vt = VT_BSTR;
	    hr = pBag->Read(L"FriendlyName", &var, NULL);
	    if (hr == NOERROR) {
DPF("dev:%s,%s",var.bstrVal,W2T(var.bstrVal));
		strcpy_s(m_szDevice,sizeof(m_szDevice),(LPCSTR)W2T(var.bstrVal));
		SysFreeString(var.bstrVal);
		}
	}
            pBag->Release();

	    InitCapFilters();
	    if (m_bWantPreview) { // were we previewing?
	        BuildPreviewGraph();
	        StartPreview();
	    		}
//if (pVW)
//		pVW->put_Visible(OAFALSE);
	    UpdateUI();	// the UI choices change per device

 }



BOOL CCapper::FillDeviceList()
{
   USES_CONVERSION;

	if (m_bHaveDevices)
        return 0;
    HRESULT hr;
	BOOL bCheck = FALSE;

    m_bHaveDevices = true;
    m_nDevices = 0;
    m_current = -1;
    UINT    uIndex = 0;
	ReleaseMonikers();
    // enumerate all video capture devices
    ICreateDevEnum *pCreateDevEnum;
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
			  IID_ICreateDevEnum, (void**)&pCreateDevEnum);
    if (hr != NOERROR)
		{
		ErrMsg("Error Creating Device Enumerator");
		return 1;
		}

    IEnumMoniker *pEm;
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,
								&pEm, 0);
    if (hr != NOERROR) {
//	ErrMsg("Sorry, you have no video capture hardware");
	ErrMsg("No Video Capture Hardware Detected");
DPF("done");
	goto EnumAudio;
    }
    pEm->Reset();
    ULONG cFetched;
    IMoniker *pM;
	
    while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK)
    {
	IPropertyBag *pBag = 0;
	hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
	if(SUCCEEDED(hr))
		{
	    VARIANT var;
	    var.vt = VT_BSTR;
	    hr = pBag->Read(L"FriendlyName", &var, NULL);
	    if (hr == NOERROR) {
DPF("dev:%s,%s",var.bstrVal,W2T(var.bstrVal));
		if (!strcmp(m_szDevice, W2T(var.bstrVal)))
			m_current = uIndex;
		strcpy_s(m_szDevices[uIndex],sizeof(m_szDevices[0]),W2T(var.bstrVal));
		SysFreeString(var.bstrVal);
 		ASSERT(m_pMonikers[uIndex] == 0);
        m_pMonikers[uIndex] = pM;
        pM->AddRef();
	    }
	    pBag->Release();
	}
//	pM->Release();
	uIndex++;
    }
    pEm->Release();

    m_nDevices = uIndex;

EnumAudio:
//	m_pDevice->SetCurSel(cursel);//m_nDevice);
DPF("returning 0");
	return 0;
}


// Start previewing
//
BOOL CCapper::StartPreview()
{
	if (m_pGrab)
		{
//		m_pGrab->SetCallback(m_pCB, 1);
//		m_pGrab->AddRef();
		}
//	m_pInfo->Status = 1;
//	return 0;
    BOOL f = TRUE;
DPF("start prev:%d,%d",m_bPreviewing,m_bPreviewGraphBuilt);
    // way ahead of you
    if (m_bPreviewing)
		return TRUE;

    if (!m_bPreviewGraphBuilt)
	return FALSE;

    // run the graph
    IMediaControl *pMC = NULL;
    HRESULT hr = pFg->QueryInterface(IID_IMediaControl, (void **)&pMC);
    if (SUCCEEDED(hr)) {
		hr = pMC->Run();
		if (FAILED(hr))
			{
DPF("partial run");
	    		// stop parts that ran
	    		pMC->Stop();
			}
		pMC->Release();
    	}
    if (FAILED(hr)) {
//	ErrMsg("Error %x: Cannot run preview graph", hr);
	ErrMsg("Error, Cannot run preview graph");


		return FALSE;
    	}
	//	pVW->put_Visible(OAFALSE);
DPF("started");
    m_bPreviewing = TRUE;
    return TRUE;
}


BOOL CCapper::StopPreview()
{
//	m_pInfo->Status = 0; // stopped
//	return 0;
	if (m_pGrab)
		{
//		if (m_pGrab->SetCallback(0,0))
//			{
//DPF("cant clear callback");
//			}
//		m_pCB->Release();
		}
	if (!m_bPreviewing)
		return FALSE;
    // stop the graph
    IMediaControl *pMC = NULL;
    HRESULT hr = pFg->QueryInterface(IID_IMediaControl, (void **)&pMC);
    if (SUCCEEDED(hr))
		{
		hr = pMC->Stop();
		pMC->Release();
		}
    if (FAILED(hr))
		{
//	ErrMsg("Error %x: Cannot stop preview graph", hr);
	ErrMsg("Error,Cannot stop preview graph");
		return FALSE;
    	}
    m_bPreviewing = FALSE;

//    InvalidateRect(m_hWndView, NULL, TRUE);
//		pVW->put_Visible(OAFALSE);

    return TRUE;
}

void CCapper::ReleaseMonikers()
{
	DPF("realeasing, n:%d, cur:%d",m_nDevices, m_current);
	if (m_nDevices)
		{
		ASSERT(m_current >= 0);
		m_pMonikers[m_current]->Release();
		for(int i = 0; i < m_nDevices; i++)
			{
    		IMoniker *pm = m_pMonikers[i];
			ASSERT(pm != 0);
			if (pm)
				pm->Release();
			m_pMonikers[i] = 0;
			}
		m_current = -1;
		m_nDevices = 0;
		}
}

BOOL CCapper::FormatDialog()
{
	if (m_bWantPreview)
	    StopPreview();
	if ( m_bPreviewGraphBuilt)
		{
				DPF("Tear down graph for dialog");
		TearDownGraph();	// graph could prevent dialog working
		}
	HRESULT hr;
	hr = m_pDlg->ShowDialog(VfwCaptureDialog_Format, m_pParent->m_hWnd);
	// Oh uh!  Sometimes bringing up the FORMAT dialog can result
	// in changing to a capture format that the current graph 
	// can't handle.  It looks like that has happened and we'll
	// have to rebuild the graph.
	if (hr == VFW_E_CANNOT_CONNECT)
		{
		DPF("DIALOG CORRUPTED GRAPH!");
		TearDownGraph();	// now we need to rebuild
	    // !!! This won't work if we've left a stranded h/w codec
		}

		// Resize our window to be the same size that we're capturing
	        if (pVSC) {
		    AM_MEDIA_TYPE *pmt;
		    // get format being used NOW
		    hr = pVSC->GetFormat(&pmt);
	    	    // DV capture does not use a VIDEOINFOHEADER
            	    if (hr == NOERROR) {
	 		if (pmt->formattype == FORMAT_VideoInfo) {
		            // resize our window to the new capture size
		            ResizeWindow(HEADER(pmt->pbFormat)->biWidth,
					abs(HEADER(pmt->pbFormat)->biHeight));
			}
		        DeleteMediaType(pmt);
		    }
	        }
	SetGrabber();
	if (m_bWantPreview)
		{
		BuildPreviewGraph();
		StartPreview();
		}
	return 1;
}

BOOL CCapper::SourceDialog()
{
	if (m_bWantPreview)
		StopPreview();
	m_pDlg->ShowDialog(VfwCaptureDialog_Source, m_pParent->m_hWnd);
	if (m_bWantPreview)
		StartPreview();
	return 1;
}

BOOL CCapper::DisplayDialog()
{
	if (m_bWantPreview)
		StopPreview();
	m_pDlg->ShowDialog(VfwCaptureDialog_Display, m_pParent->m_hWnd);
	if (m_bWantPreview)
		StartPreview();
	return 1;
}

BOOL CCapper::VidCapDialog()
{
	HRESULT hr;
	ISpecifyPropertyPages *pSpec;
	CAUUID cauuid;
	hr = pVCap->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
	if (hr == S_OK)
		{
		hr = pSpec->GetPages(&cauuid);
		hr = OleCreatePropertyFrame(m_pParent->m_hWnd, 30, 30, NULL, 1,
    	     		           (IUnknown **)&pVCap, cauuid.cElems,
		    				(GUID *)cauuid.pElems, 0, 0, NULL);
		CoTaskMemFree(cauuid.pElems);
		pSpec->Release();
		}
	return 1;
}


BOOL CCapper::CapturePinDialog()
{
DPF("cap pin dlg,%d,%d",m_bPreviewing,m_bPreviewGraphBuilt);
	// You can change this pin's output format in these dialogs.
	// If the capture pin is already connected to somebody who's 
	// fussy about the connection type, that may prevent using 
	// this dialog(!) because the filter it's connected to might not
	// allow reconnecting to a new format. (EG: you switch from RGB
              // to some compressed type, and need to pull in a decoder)
	// I need to tear down the graph downstream of the
	// capture filter before bringing up these dialogs.
	// In any case, the graph must be STOPPED when calling them.
	HRESULT hr;
	if (m_bWantPreview)
	    StopPreview();	// make sure graph is stopped
// The capture pin that we are trying to set the format on is connected if
// one of these variable is set to TRUE. The pin should be disconnected for
// the dialog to work properly.
	if ( m_bPreviewGraphBuilt)
		{
				DPF("Tear down graph for dialog");
		TearDownGraph();	// graph could prevent dialog working
		}
//	    FreeCapFilters();
//	    InitCapFilters();
#define ZZZZ
#ifdef ZZZZ		

    IAMStreamConfig *pSC;
    hr = m_pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Interleaved, pVCap,
				IID_IAMStreamConfig, (void **)&pSC);
	if (hr != NOERROR)
        hr = m_pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Video, pVCap,
				IID_IAMStreamConfig, (void **)&pSC);
	if (hr != NOERROR)
		{
DPF("no cap pn dlg");
		return 0;
		}

	ISpecifyPropertyPages *pSpec;
	CAUUID cauuid;
	hr = pSC->QueryInterface(IID_ISpecifyPropertyPages,
							(void **)&pSpec);
    if (hr == S_OK)
		{
        	   hr = pSpec->GetPages(&cauuid);
            hr = OleCreatePropertyFrame(m_pParent->m_hWnd, 30, 30, NULL, 1,
                    (IUnknown **)&pSC, cauuid.cElems,
		    (GUID *)cauuid.pElems, 0, 0, NULL);

		    // !!! What if changing output formats couldn't reconnect
		    // and the graph is broken?  Shouldn't be possible...
		
	     if (pSC) {
		        AM_MEDIA_TYPE *pmt;
		        // get format being used NOW
		        hr = pSC->GetFormat(&pmt);
	    	        // DV capture does not use a VIDEOINFOHEADER
            	   if (hr == NOERROR) {
	 		   	 if (pmt->formattype == FORMAT_VideoInfo) {
		                // resize our window to the new capture size
		                ResizeWindow(HEADER(pmt->pbFormat)->biWidth,
					  abs(HEADER(pmt->pbFormat)->biHeight));
			    		}
		            DeleteMediaType(pmt);
		        		}
	            }

		    CoTaskMemFree(cauuid.pElems);
		    pSpec->Release();
		}
DPF("restarting");
		pSC->Release();
#endif
	SetGrabber();
	        if (m_bWantPreview) {
		    BuildPreviewGraph();
		    StartPreview();
			}
//	SetGrabber();
//	UpdateUI();
	return 1;
}


BOOL CCapper::CrossbarDialog()
{
	HRESULT hr;
	IAMCrossbar *pX;
	hr = m_pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
			&MEDIATYPE_Interleaved, pVCap,
				IID_IAMCrossbar, (void **)&pX);
	if (hr != NOERROR)
		hr = m_pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Video, pVCap,
				IID_IAMCrossbar, (void **)&pX);
	ISpecifyPropertyPages *pSpec;
	CAUUID cauuid;
	hr = pX->QueryInterface(IID_ISpecifyPropertyPages,
							(void **)&pSpec);
	if (hr == S_OK)
		{
		hr = pSpec->GetPages(&cauuid);
		hr = OleCreatePropertyFrame(m_pParent->m_hWnd, 30, 30, NULL, 1,
                    	(IUnknown **)&pX, cauuid.cElems,
							(GUID *)cauuid.pElems, 0, 0, NULL);
		CoTaskMemFree(cauuid.pElems);
		pSpec->Release();
		}
	pX->Release();
	return 1;
}

BOOL CCapper::Crossbar2Dialog()
{
	HRESULT hr;
	IAMCrossbar *pX = 0;
	IAMCrossbar *pX2 = 0;
	IBaseFilter *pXF = 0;
		// we could use better error checking here... I'm assuming
		// this won't fail
	hr = m_pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Interleaved, pVCap,
				IID_IAMCrossbar, (void **)&pX);
	if (!SUCCEEDED(hr))
		hr = m_pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Video, pVCap,
				IID_IAMCrossbar, (void **)&pX);
	hr = pX->QueryInterface(IID_IBaseFilter, (void **)&pXF);
	hr = m_pBuilder->FindInterface(&LOOK_UPSTREAM_ONLY, NULL,
				pXF, IID_IAMCrossbar, (void **)&pX2);
	ISpecifyPropertyPages *pSpec;
	CAUUID cauuid;
	hr = pX2->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
	if (hr == S_OK)
		{
		hr = pSpec->GetPages(&cauuid);
		hr = OleCreatePropertyFrame(m_pParent->m_hWnd, 30, 30, NULL, 1,
                    (IUnknown **)&pX2, cauuid.cElems,
		    (GUID *)cauuid.pElems, 0, 0, NULL);
		CoTaskMemFree(cauuid.pElems);
		pSpec->Release();
		}
	pX2->Release();
	pXF->Release();
	pX->Release();
	return 1;
}


BOOL CCapper::TVTunerDialog()
{
	HRESULT hr;
	IAMTVTuner *pTV;
	hr = m_pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Interleaved, pVCap,
				IID_IAMTVTuner, (void **)&pTV);
	if (hr != NOERROR)
		hr = m_pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
				&MEDIATYPE_Video, pVCap,
				IID_IAMTVTuner, (void **)&pTV);
	ISpecifyPropertyPages *pSpec;
	CAUUID cauuid;
	hr = pTV->QueryInterface(IID_ISpecifyPropertyPages,
							(void **)&pSpec);
	if (hr == S_OK)
		{
		hr = pSpec->GetPages(&cauuid);
		hr = OleCreatePropertyFrame(m_pParent->m_hWnd,30, 30, NULL, 1,
                    (IUnknown **)&pTV, cauuid.cElems,
		    (GUID *)cauuid.pElems, 0, 0, NULL);
		CoTaskMemFree(cauuid.pElems);
		pSpec->Release();
		}
	pTV->Release();
	return 1;
}

BOOL CCapper::Dialog(int index)
{
	int which;
	BOOL bStop = index & 256;
	which = m_nOptions[index & 255];
	DPF("dialog,index:%d,which:%d",index,which);

	if (bStop)
		{
		if (m_bWantPreview)
	    	StopPreview();	// make sure graph is stopped
// The capture pin that we are trying to set the format on is connected if
// one of these variable is set to TRUE. The pin should be disconnected for
// the dialog to work properly.
		if ( m_bPreviewGraphBuilt)
			{
			DPF("Tear down graph for dialog");
			TearDownGraph();	// graph could prevent dialog working
			}
		}
	BOOL bResult;// = m_nDialogs[which];
//	if (!doit || !bResult)
//		return bResult;
	switch (which) {
	case CAP_DLG_FORMAT:
		bResult = FormatDialog();
		break;
	case CAP_DLG_SOURCE:
		bResult = SourceDialog();
		break;
	case CAP_DLG_DISPLAY:
		bResult = DisplayDialog();
		break;
	case CAP_DLG_DLG:
		bResult = VidCapDialog();
		break;
	case CAP_DLG_CAPTPIN:
		bResult = CapturePinDialog();
		break;
	case CAP_DLG_PREVPIN:
//		bResult = PreviewPinDialog();
		bResult = DVDialog();
		break;
	case CAP_DLG_CROSS:
		bResult = CrossbarDialog();
		break;
	case CAP_DLG_CROSS2:
		bResult = Crossbar2Dialog();
		break;
	case CAP_DLG_TUNER:
		bResult = TVTunerDialog();
		break;
	default:
		bResult = -1;
	}
	if (bStop)
		{
		SetGrabber();
	    if (m_bWantPreview)
			{
			BuildPreviewGraph();
			StartPreview();
			}
		}
	return bResult;
}

void CCapper::ErrMsg(LPCSTR txt)
{
	DPF("err:%s",txt);
//	AfxMessageBox(txt,"FlipBook Video Capture Program",MB_OK);
	AfxMessageBox(txt);
	DPF("after");
}

BOOL CCapper::SetGrabber()
{
	UINT result;
	if ((result = SetGrabber2(24)) == 6)
		result = SetGrabber2(16);
	return result ? TRUE : FALSE;
}

UINT CCapper::SetGrabber2(UINT depth)
{
	HRESULT hr;
	AM_MEDIA_TYPE mmt;
	IBaseFilter     *pF = NULL;
	IBaseFilter     *pNullRenderer = NULL;

//	if (m_pGrab)
//		m_pGrab->Release();
	{
    hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC,
                          IID_IBaseFilter, (LPVOID *)&pF);
    if (hr != NOERROR)
    	{
        ErrMsg("Creation of the sample grabber failed.");
		return 1;
    	}

    hr = pF->QueryInterface(IID_ISampleGrabber, (void **)&m_pGrab);
    if (hr != NOERROR)
    	{
        ErrMsg("Sample grabber interface could not be found.");
		return 2;
    	}

    hr = pFg->AddFilter(pF, L"Grabber");
    if (hr != NOERROR)
    	{
        ErrMsg("Adding the sample grabber to the filter graph failed.");
		return 3;
    	}

	AM_MEDIA_TYPE   mt;
    ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
    mt.majortype = MEDIATYPE_Video;
	if (depth == 24)
    	mt.subtype = MEDIASUBTYPE_RGB24;
	else
    	mt.subtype = MEDIASUBTYPE_RGB555;
    mt.formattype = FORMAT_VideoInfo;
    hr = m_pGrab->SetMediaType(&mt);
    if (FAILED(hr))
    	{
        ErrMsg("Setting the media type of the sample grabber failed.");
        return 5;
           }

    hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC,
                          IID_IBaseFilter, (LPVOID *)&pNullRenderer);
    hr = pFg->AddFilter(pNullRenderer, L"Nuller");
	
    hr = m_pBuilder->RenderStream(
            &PIN_CATEGORY_CAPTURE, // changed from ..._PREVIEW to ..._CAPTURE
            &MEDIATYPE_Video,
            pVCap, // src
            pF,            // via
            pNullRenderer
        );

	pNullRenderer->Release();

    pF->Release();
    if (FAILED(hr))
    	{
		if (depth == 24)
    		hr = pFg->RemoveFilter(pF);
		else
        	ErrMsg("Capture Graph could not be created.");
      return 6;//  goto InitCapFiltersFail;
	    }
	}
    hr = m_pGrab->GetConnectedMediaType( &mmt );
    if ( FAILED( hr) )
 	   {
        ErrMsg("Could not read the connected media type");
        return hr;
    	}

    VIDEOINFOHEADER * vih = (VIDEOINFOHEADER*) mmt.pbFormat;
  	m_pCB->m_w = vih->bmiHeader.biWidth;
  	m_pCB->m_h = vih->bmiHeader.biHeight;
  	m_pCB->m_d = vih->bmiHeader.biBitCount;
DPF("grabbing,w:%d,h:%d,d:%d",m_pCB->m_w,m_pCB->m_h,m_pCB->m_d);
    FreeMediaType( mmt );
	m_pGrab->SetOneShot(FALSE);
	m_pGrab->SetBufferSamples(0);
	m_pGrab->SetCallback(m_pCB, 1);
	return 0;
}


int CCapper::DVSize(int val /* -1 */)
{
	int iDisplayPix = -1;
	BOOL bResult = 0;
	IIPDVDec  *pDvDec               = NULL;
	HRESULT hr;
	hr = pDVCodec->QueryInterface(IID_IIPDVDec, 
					reinterpret_cast<PVOID *>(&pDvDec));
	if (SUCCEEDED(hr))
		{
		if (val >= 0)
			{
			val += DVRESOLUTION_FULL;
DPF("setting size:%d",val);
			hr = pDvDec->put_IPDisplay(val);
			}
		hr = pDvDec->get_IPDisplay(&iDisplayPix);
DPF("beforepix:%d",iDisplayPix);
		if (iDisplayPix < DVRESOLUTION_FULL)
			iDisplayPix = DVRESOLUTION_FULL;
		if (iDisplayPix > DVRESOLUTION_DC)
			iDisplayPix = DVRESOLUTION_DC;
		iDisplayPix -= DVRESOLUTION_FULL;
		pDvDec->Release();
		}
	else
		{
        ErrMsg("Could not get pvdec in size");
		}
	return iDisplayPix;
}


BOOL CCapper::DVDialog()
{
DPF("dv size dlg,%d,%d",m_bPreviewing,m_bPreviewGraphBuilt);
//	HRESULT hr;
	if (m_bWantPreview)
	    StopPreview();	// make sure graph is stopped
	if ( m_bPreviewGraphBuilt)
		TearDownGraph();	// graph could prevent dialog working
	    FreeCapFilters();

	CDVSizeDlg dlg;
	dlg.m_nSize = m_nDVSize;
	if (dlg.DoModal() == IDOK)
		{
		m_nDVSize = dlg.m_nSize;
		}

	InitCapFilters();
	if (m_bWantPreview)
		{
		BuildPreviewGraph();
		StartPreview();
		}
	return 1;
}


BOOL CCapper::SelectDevice(int which)
{
	ChooseDevice(which);
	return 0;
}

UINT CCapper::Width()
{ return m_pCB->m_w;};
UINT CCapper::Height()
{ return m_pCB->m_h;};


void CCapper::SetRes()
{
	HRESULT hr;
    AM_MEDIA_TYPE *pmt;
	hr = pVSC->GetFormat(&pmt);
	// DV capture does not use a VIDEOINFOHEADER
	if (hr == NOERROR)
		{
	    if (pmt->formattype == FORMAT_VideoInfo) {
			VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)pmt->pbFormat;
DPR("Src",&pvi->rcSource);
DPR("Tar",&pvi->rcTarget);
DPF("w:%d,h:%d,%d",pvi->bmiHeader.biWidth,pvi->bmiHeader.biHeight,pvi->bmiHeader.biBitCount);
//   pmt->subtype = MEDIASUBTYPE_RGB24;
//pvi->bmiHeader.biBitCount = 16;
pvi->bmiHeader.biWidth = m_width;//m_pInfo->Width;
pvi->bmiHeader.biHeight = m_height;//m_pInfo->Height;
			hr = pVSC->SetFormat(pmt);
			if (hr != NOERROR)
			{
DPF("err:%x",(UINT)hr);
				ErrMsg("Cannot set resolution for preview");
			}
	    	}
		DeleteMediaType(pmt);

		}
}
