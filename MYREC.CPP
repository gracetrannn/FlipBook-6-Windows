//------------------------------------------------------------------------------
// File: AMCap.cpp
//
// Desc: Audio/Video Capture sample for DirectShow
//
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------
#include "stdafx.h"
#include "rcapdef.h"

#include <streams.h>
#include "DShowUtil.h"
#include "smartptr.h"   // smart pointer class

#include <dbt.h>
#include <mmreg.h>
#include <msacm.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
//#include <commdlg.h>
#include <strsafe.h>
//#include "stdafx.h"
//#include "myrec.h"
//#include "status.h"
#include "crossbar.h"
#include "Sampler.h"

//#define _COMPRESS

//------------------------------------------------------------------------------
// Macros
//------------------------------------------------------------------------------
#define ABS(x) (((x) > 0) ? (x) : -(x))

TCHAR mytemp[300];
LPTSTR MySub(const WCHAR * pw)
{
		size_t v;
		wcstombs_s (&v,(LPSTR)mytemp, 300,pw, 300);
		mytemp[v] = 0;
	//	wcstombs ( (LPSTR)mytemp,pw,300);
//		wchar_t * wcstr, const char * mbstr, size_t max );
//		mbstowcs ( mytemp, pw, 100 )	;
//		WideToMultiByte(mytemp, pw);
		return mytemp;
}
wchar_t mywide[400];
wchar_t * MyWide(const TCHAR * txt)
{
	size_t v;
	mbstowcs_s(&v,mywide,(LPCSTR)txt,400);//mblen(txt));
	return mywide;	
}
//------------------------------------------------------------------------------
// Global data
//------------------------------------------------------------------------------
HINSTANCE ghInstApp=0;
HACCEL ghAccel=0;
HFONT  ghfontApp=0;
TEXTMETRIC gtm={0};
TCHAR gszAppName[]=TEXT("AMCAP");
HWND ghwndApp=0, ghwndStatus=0;
//HDEVNOTIFY ghDevNotify=0;
//PUnregisterDeviceNotification gpUnregisterDeviceNotification=0;
//PRegisterDeviceNotification gpRegisterDeviceNotification=0;
DWORD g_dwGraphRegister=0;

struct _capstuff
{
    TCHAR szCaptureFile[_MAX_PATH];
    WORD wCapFileSize;  // size in Meg
    ISampleCaptureGraphBuilder *pBuilder;
    IVideoWindow *pVW;
    IMediaEventEx *pME;
    IAMDroppedFrames *pDF;
	IAMVideoCompression *pVC;
    IAMVfwCaptureDialogs *pDlg;
    IAMStreamConfig *pASC;      // for audio cap
    IAMStreamConfig *pVSC;      // for video cap
    IBaseFilter *pMux;
    IBaseFilter *pVCap;
	IBaseFilter *pACap;
	IBaseFilter *pVCmp;
    IGraphBuilder *pFg;
    IFileSinkFilter *pSink;
//    int  iiMasterStream;
    BOOL fCaptureGraphBuilt;
    BOOL fPreviewGraphBuilt;
    BOOL fCapturing;
    BOOL fPreviewing;
    BOOL fMPEG2;
    BOOL fCapAudio;
    BOOL fCapCC;
    BOOL fCCAvail;
	BOOL fCapAudioIsRelevant;
    bool fDeviceListPopulated;
    IMoniker *rgpmVideoList[10];
    IMoniker *rgpmAudioList[10];
    IMoniker *rgpmCompressList[30];
    IMoniker *pmVideo;
    IMoniker *pmAudio;
    IMoniker *pmCompress;
	WCHAR VidNames[10][120];
	WCHAR AudNames[10][120];
	WCHAR CmpNames[30][120];
	int  VDialogs[10];
	int  ADialogs[10];
	char szDialogs[20][80];
    BOOL fWantPreview;
    long lCapStartTime;
    long lCapStopTime;
//    WCHAR wachVidFriendlyName[120];
//    WCHAR wachAudFriendlyName[120];
//    WCHAR wachCmpFriendlyName[120];
	GUID myfmt;
	UINT myw;
	UINT myh;
	UINT myd;
	UINT myc;
int capw;
int caph;
    int iFormatDialogPos;
    int iSourceDialogPos;
    int iDisplayDialogPos;
    int iVCapDialogPos;
    int iVCrossbarDialogPos;
    int iTVTunerDialogPos;
    int iACapDialogPos;
    int iACrossbarDialogPos;
    int iTVAudioDialogPos;
    int iVCapCapturePinDialogPos;
    int iVCapPreviewPinDialogPos;
    int iACapCapturePinDialogPos;
    long lDroppedBase;
    long lNotBase;
    BOOL fPreviewFaked;
    CCrossbar *pCrossbar;
    int iVideoInputMenuPos;
    LONG NumberOfVideoInputs;
    HMENU hMenuPopup;
    int iNumVCapDevices;
    int iNumACapDevices;
    int iNumCompressors;
    int iNumVCapDialogs;
    int iNumACapDialogs;
	int iVidDevice;
	int iAudDevice;
	int iCompressor;
	BOOL bStatus; // 1 if not polled yet
	char zstatus[100]; // polled by parent
} gcap;

#define RAW "UnCompressed"

int  zinfo();
void AddDevicesToList();
int EnumVideo( ICreateDevEnum *pCreateDevEnum);
int EnumCompress( ICreateDevEnum *pCreateDevEnum);
int EnumAudio( ICreateDevEnum *pCreateDevEnum);


void  statusUpdateStatus(HWND hwnd, LPCTSTR lpsz)
{
	gcap.bStatus = 1;	// we have something new
	strcpy_s(gcap.zstatus, lpsz);
}
//------------------------------------------------------------------------------
// Function Prototypes
//------------------------------------------------------------------------------
void ErrMsg(LPTSTR sz,...);

//BOOL SetCaptureFile(HWND hWnd);
BOOL SaveCaptureFile(HWND hWnd);
//BOOL AllocCaptureFile(HWND hWnd);
void TearDownGraph(void);
BOOL BuildCaptureGraph();
BOOL BuildPreviewGraph();
void SetupCaptureFile(LPCSTR CapName);
void UpdateStatus(BOOL fAllStats);
void ChooseDevices(int vid_id, int aud_id, int cmp_id);
void ChooseFrameRate();

BOOL InitCapFilters();
void FreeCapFilters();

BOOL StopPreview();
BOOL StartPreview();
BOOL StopCapture();

DWORDLONG GetSize(LPCTSTR tach);
void MakeMenuOptions();
void OnClose();

void IMonRelease(IMoniker *&pm)
{
    if(pm)
    {
        pm->Release();
        pm = 0;
    }
}

// Make a graph builder object we can use for capture graph building
//
BOOL MakeBuilder()
{
    // we have one already
    if(gcap.pBuilder)
        return TRUE;

    gcap.pBuilder = new ISampleCaptureGraphBuilder( );
    if( NULL == gcap.pBuilder )
    {
        return FALSE;
    }

    return TRUE;
}


// Make a graph object we can use for capture graph building
//
BOOL MakeGraph()
{
    // we have one already
    if(gcap.pFg)
        return TRUE;

    HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
                                  IID_IGraphBuilder, (LPVOID *)&gcap.pFg);

    return (hr == NOERROR) ? TRUE : FALSE;
}


// make sure the preview window inside our window is as big as the
// dimensions of captured video, or some capture cards won't show a preview.
// (Also, it helps people tell what size video they're capturing)
// We will resize our app's window big enough so that once the status bar
// is positioned at the bottom there will be enough room for the preview
// window to be w x h
//
int gnRecurse = 0;

//int statusGetHeight() { return 16;}

void ResizeWindow(int w, int h)
{
	if (!gnRecurse)
		{
		gcap.capw = 320;
		gcap.caph = 240;
		}
return;
    RECT rcW, rcC;
    int xExtra, yExtra;
    int cyBorder = GetSystemMetrics(SM_CYBORDER);

    gnRecurse++;

    GetWindowRect(ghwndApp, &rcW);
    GetClientRect(ghwndApp, &rcC);
    xExtra = rcW.right - rcW.left - rcC.right;
    yExtra = rcW.bottom - rcW.top - rcC.bottom + cyBorder;// + statusGetHeight();

    rcC.right = w;
    rcC.bottom = h;
    SetWindowPos(ghwndApp, NULL, 0, 0, rcC.right + xExtra,
        rcC.bottom + yExtra, SWP_NOZORDER | SWP_NOMOVE);

    // we may need to recurse once.  But more than that means the window cannot
    // be made the size we want, trying will just stack fault.
    //
    if(gnRecurse == 1 && ((rcC.right + xExtra != rcW.right - rcW.left && w > GetSystemMetrics(SM_CXMIN)) ||
        (rcC.bottom + yExtra != rcW.bottom - rcW.top)))
        ResizeWindow(w,h);

    gnRecurse--;
}


// Tear down everything downstream of a given filter
void RemoveDownstream(IBaseFilter *pf)
{
    IPin *pP=0, *pTo=0;
    ULONG u;
    IEnumPins *pins = NULL;
    PIN_INFO pininfo;

    if (!pf)
        return;

    HRESULT hr = pf->EnumPins(&pins);
    pins->Reset();

    while(hr == NOERROR)
    {
        hr = pins->Next(1, &pP, &u);
        if(hr == S_OK && pP)
        {
            pP->ConnectedTo(&pTo);
            if(pTo)
            {
                hr = pTo->QueryPinInfo(&pininfo);
                if(hr == NOERROR)
                {
                    if(pininfo.dir == PINDIR_INPUT)
                    {
                        RemoveDownstream(pininfo.pFilter);
                        gcap.pFg->Disconnect(pTo);
                        gcap.pFg->Disconnect(pP);
                        gcap.pFg->RemoveFilter(pininfo.pFilter);
                    }
                    pininfo.pFilter->Release();
                }
                pTo->Release();
            }
            pP->Release();
        }
    }

    if(pins)
        pins->Release();
}


// Tear down everything downstream of the capture filters, so we can build
// a different capture graph.  Notice that we never destroy the capture filters
// and WDM filters upstream of them, because then all the capture settings
// we've set would be lost.
//
void TearDownGraph()
{
    SAFE_RELEASE(gcap.pSink);
    SAFE_RELEASE(gcap.pMux);
	SAFE_RELEASE(gcap.pME);
    SAFE_RELEASE(gcap.pDF);

    if(gcap.pVW)
    {
        // stop drawing in our window, or we may get wierd repaint effects
        gcap.pVW->put_Owner(NULL);
        gcap.pVW->put_Visible(OAFALSE);
        gcap.pVW->Release();
        gcap.pVW = NULL;
    }

    // destroy the graph downstream of our capture filters
    if(gcap.pVCap)
        RemoveDownstream(gcap.pVCap);
    if(gcap.pACap)
        RemoveDownstream(gcap.pACap);
    if(gcap.pVCmp)
        RemoveDownstream(gcap.pVCmp);
    if(gcap.pVCap)
        gcap.pBuilder->ReleaseFilters();

    // potential debug output - what the graph looks like
    // if (gcap.pFg) DumpGraph(gcap.pFg, 1);

    gcap.fCaptureGraphBuilt = FALSE;
    gcap.fPreviewGraphBuilt = FALSE;
    gcap.fPreviewFaked = FALSE;
}

void SetupCaptureFile(LPCSTR CapName)
{
	strcpy_s(gcap.szCaptureFile,CapName);	
}

// create the capture filters of the graph.  We need to keep them loaded from
// the beginning, so we can set parameters on them and have them remembered
//
BOOL InitCapFilters()
{
    HRESULT hr=S_OK;
    BOOL f;

    gcap.fCCAvail = FALSE;  // assume no closed captioning support

    f = MakeBuilder();
    if(!f)
    	{
        ErrMsg(TEXT("Cannot instantiate graph builder"));
        return FALSE;
    	}

    DWORDLONG dwlSize = 0;
	CFile ff;
	if (ff.Open(gcap.szCaptureFile, CFile::modeReadWrite))
		{
		dwlSize = ff.GetLength();
		dwlSize /= 1024;
		dwlSize /= 1024;
		ff.Close();
		}
	if (dwlSize && (!gcap.wCapFileSize ||
				((WORD)dwlSize  != gcap.wCapFileSize))) 
		{
		try
			{
    		CFile::Remove(gcap.szCaptureFile);
			}
		catch (CFileException* pEx)
			{
		   	pEx->Delete();
			}
		}

	if (gcap.wCapFileSize && ((WORD)dwlSize < gcap.wCapFileSize))
		{
        if(gcap.pBuilder->AllocCapFile(MyWide(gcap.szCaptureFile),
            (DWORDLONG)gcap.wCapFileSize * 1024L * 1024L) != NOERROR)
        	{
            MessageBox(ghwndApp, TEXT("Error"),
                       TEXT("Failed to pre-allocate capture file space"),
                       MB_OK | MB_ICONEXCLAMATION);
            return FALSE;
        	}
		}	
    //
    // First, we need a Video Capture filter, and some interfaces
    //
	ASSERT(gcap.pVCap == 0);
    gcap.pVCap = NULL;

    if(gcap.pmVideo != 0)
    	{
    	hr = gcap.pmVideo->BindToObject(0, 0, IID_IBaseFilter,
						(void**)&gcap.pVCap);
    	}

    if(gcap.pVCap == NULL)
    {
        ErrMsg(TEXT("Error %x: Cannot create video capture filter"), hr);
        goto InitCapFiltersFail;
    }

	ASSERT(gcap.pVCmp == 0);
    gcap.pVCmp = NULL;

    if(gcap.pmCompress != 0)
    	{
	    hr = gcap.pmCompress->BindToObject(0, 0, IID_IBaseFilter,
						(void**)&gcap.pVCmp);
		if(gcap.pVCmp == NULL)
			{
        	ErrMsg(TEXT("Error %x: Cannot create video compress filter"), hr);
        	goto InitCapFiltersFail;
			}
		}

    //
    // make a filtergraph, give it to the graph builder and put the video
    // capture filter in the graph
    //


    f = MakeGraph();
    if(!f)
    {
        ErrMsg(TEXT("Cannot instantiate filtergraph"));
        goto InitCapFiltersFail;
    }

    hr = gcap.pBuilder->SetFiltergraph(gcap.pFg);
    if(hr != NOERROR)
    {
        ErrMsg(TEXT("Cannot give graph to builder"));
        goto InitCapFiltersFail;
    }

    // Add the video capture filter to the graph with its friendly name
	hr = gcap.pFg->AddFilter(gcap.pVCap, gcap.VidNames[gcap.iVidDevice]);
    if(hr != NOERROR)
    	{
        ErrMsg(TEXT("Error %x: Cannot add vidcap to filtergraph"), hr);
        goto InitCapFiltersFail;
    	}
	if (gcap.pVCmp)
		{
		hr = gcap.pFg->AddFilter(gcap.pVCmp,gcap.CmpNames[gcap.iCompressor]);
    	if(hr != NOERROR)
    		{
        	ErrMsg(TEXT("Error %x: Cannot add compressor to filtergraph"), hr);
        		goto InitCapFiltersFail;
    		}
		}

    // Calling FindInterface below will result in building the upstream
    // section of the capture graph (any WDM TVTuners or Crossbars we might
    // need).

    // we use this interface to get the name of the driver
    // Don't worry if it doesn't work:  This interface may not be available
    // until the pin is connected, or it may not be available at all.
    // (eg: interface may not be available for some DV capture)
	
	hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
                                     &MEDIATYPE_Interleaved, gcap.pVCap,
		                         IID_IAMVideoCompression, (void **)&gcap.pVC);
	if(hr != S_OK)
		{
		hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
                                          &MEDIATYPE_Video, gcap.pVCap,
                                 IID_IAMVideoCompression, (void **)&gcap.pVC);
		}

    // !!! What if this interface isn't supported?
    // we use this interface to set the frame rate and get the capture size
    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
                                      &MEDIATYPE_Interleaved, gcap.pVCap,
								IID_IAMStreamConfig, (void **)&gcap.pVSC);

    if(hr != NOERROR)
    {
        hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
                                          &MEDIATYPE_Video, gcap.pVCap,
                                     IID_IAMStreamConfig, (void **)&gcap.pVSC);
        if(hr != NOERROR)
        {
            // this means we can't set frame rate (non-DV only)
            ErrMsg(TEXT("Error %x: Cannot find VCapture:IAMStreamConfig"), hr);
        }
    }

//	if (gcap.pVC)
//		{
//		int zz = zinfo();
//		DPF("zinfo:%d",zz);
//		}
    gcap.fCapAudioIsRelevant = TRUE;

    AM_MEDIA_TYPE *pmt;

    // default capture format
    if(gcap.pVSC && gcap.pVSC->GetFormat(&pmt) == S_OK)
    {
        // DV capture does not use a VIDEOINFOHEADER
        if(pmt->formattype == FORMAT_VideoInfo)
        {
            // resize our window to the default capture size
            ResizeWindow(HEADER(pmt->pbFormat)->biWidth,
                         ABS(HEADER(pmt->pbFormat)->biHeight));
        }
        if(pmt->majortype != MEDIATYPE_Video)
        {
            // This capture filter captures something other that pure video.
            // Maybe it's DV or something?  Anyway, chances are we shouldn't
            // allow capturing audio separately, since our video capture
            // filter may have audio combined in it already!
            gcap.fCapAudioIsRelevant = FALSE;
            gcap.fCapAudio = FALSE;
        }
        DeleteMediaType(pmt);
    }

    // we use this interface to bring up the 3 dialogs
    // NOTE:  Only the VfW capture filter supports this.  This app only brings
    // up dialogs for legacy VfW capture drivers, since only those have dialogs
    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
                                      &MEDIATYPE_Video, gcap.pVCap,
                                IID_IAMVfwCaptureDialogs, (void **)&gcap.pDlg);

    // Use the crossbar class to help us sort out all the possible video inputs
    // The class needs to be given the capture filters ANALOGVIDEO input pin
    {
        IPin        *pP = 0;
        IEnumPins   *pins=0;
        ULONG        n;
        PIN_INFO     pinInfo;
        BOOL         Found = FALSE;
        IKsPropertySet *pKs=0;
        GUID guid;
        DWORD dw;
        BOOL fMatch = FALSE;

        gcap.pCrossbar = NULL;

        if(SUCCEEDED(gcap.pVCap->EnumPins(&pins)))
        {
            while(!Found && (S_OK == pins->Next(1, &pP, &n)))
            {
                if(S_OK == pP->QueryPinInfo(&pinInfo))
                {
                    if(pinInfo.dir == PINDIR_INPUT)
                    {
                        // is this pin an ANALOGVIDEOIN input pin?
                        if(pP->QueryInterface(IID_IKsPropertySet,
                            (void **)&pKs) == S_OK)
                        {
                            if(pKs->Get(AMPROPSETID_Pin,
                                AMPROPERTY_PIN_CATEGORY, NULL, 0,
                                &guid, sizeof(GUID), &dw) == S_OK)
                            {
                                if(guid == PIN_CATEGORY_ANALOGVIDEOIN)
                                    fMatch = TRUE;
                            }
                            pKs->Release();
                        }

                        if(fMatch)
                        {
                            HRESULT hrCreate=S_OK;
                            gcap.pCrossbar = new CCrossbar(pP, &hrCreate);
                            if (!gcap.pCrossbar || FAILED(hrCreate))
                                break;

                            hr = gcap.pCrossbar->GetInputCount(&gcap.NumberOfVideoInputs);
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

    // there's no point making an audio capture filter
    if(gcap.fCapAudioIsRelevant == FALSE)
        goto SkipAudio;

    // create the audio capture filter, even if we are not capturing audio right
    // now, so we have all the filters around all the time.

    //
    // We want an audio capture filter and some interfaces
    //

    if(gcap.pmAudio == 0)
    {
        // there are no audio capture devices. We'll only allow video capture
        gcap.fCapAudio = FALSE;
        goto SkipAudio;
    }
    gcap.pACap = NULL;


    hr = gcap.pmAudio->BindToObject(0, 0, IID_IBaseFilter, (void**)&gcap.pACap);

    if(gcap.pACap == NULL)
    {
        // there are no audio capture devices. We'll only allow video capture
        gcap.fCapAudio = FALSE;
        ErrMsg(TEXT("Cannot create audio capture filter"));
        goto SkipAudio;
    }

    //
    // put the audio capture filter in the graph
    //

    {

        WCHAR wachAudioFriendlyName[256];
        IPropertyBag *pBag;

        wachAudioFriendlyName[0] = 0;

        // Read the friendly name of the filter to assist with remote graph
        // viewing through GraphEdit
        hr = gcap.pmAudio->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
        if(SUCCEEDED(hr))
        {
            VARIANT var;
            var.vt = VT_BSTR;

            hr = pBag->Read(L"FriendlyName", &var, NULL);
            if(hr == NOERROR)
            {
                hr = StringCchCopyW(wachAudioFriendlyName, 256, var.bstrVal);
                SysFreeString(var.bstrVal);
            }

            pBag->Release();
        }

        // We'll need this in the graph to get audio property pages
		hr = gcap.pFg->AddFilter(gcap.pACap, wachAudioFriendlyName);
        if(hr != NOERROR)
        {
            ErrMsg(TEXT("Error %x: Cannot add audio capture filter to filtergraph"), hr);
            goto InitCapFiltersFail;
        }
    }

    // Calling FindInterface below will result in building the upstream
    // section of the capture graph (any WDM TVAudio's or Crossbars we might
    // need).

    // !!! What if this interface isn't supported?
    // we use this interface to set the captured wave format
    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Audio, gcap.pACap,
                                      IID_IAMStreamConfig, (void **)&gcap.pASC);

    if(hr != NOERROR)
    {
        ErrMsg(TEXT("Cannot find ACapture:IAMStreamConfig"));
    }

SkipAudio:

    // Can this filter do closed captioning?
    IPin *pPin;
    hr = gcap.pBuilder->FindPin(gcap.pVCap, PINDIR_OUTPUT, &PIN_CATEGORY_VBI,
                                NULL, FALSE, 0, &pPin);
    if(hr != S_OK)
        hr = gcap.pBuilder->FindPin(gcap.pVCap, PINDIR_OUTPUT, &PIN_CATEGORY_CC,
                                    NULL, FALSE, 0, &pPin);
    if(hr == S_OK)
    {
        pPin->Release();
        gcap.fCCAvail = TRUE;
    }
    else
    {
        gcap.fCapCC = FALSE;    // can't capture it, then
    }

    // potential debug output - what the graph looks like
    // DumpGraph(gcap.pFg, 1);

    return TRUE;

InitCapFiltersFail:
    FreeCapFilters();
    return FALSE;
}


// all done with the capture filters and the graph builder
//
void FreeCapFilters()
{
    SAFE_RELEASE(gcap.pFg);
    if( gcap.pBuilder )
    {
        delete gcap.pBuilder;
        gcap.pBuilder = NULL;
    }
    SAFE_RELEASE(gcap.pVCap);
    SAFE_RELEASE(gcap.pVCmp);
    SAFE_RELEASE(gcap.pACap);
    SAFE_RELEASE(gcap.pASC);
    SAFE_RELEASE(gcap.pVSC);
	SAFE_RELEASE(gcap.pVC);
    SAFE_RELEASE(gcap.pDlg);

    if(gcap.pCrossbar)
    {
        delete gcap.pCrossbar;
        gcap.pCrossbar = NULL;
    }
}

void AspectRect(RECT &rc)
{
	int iw = rc.right - rc.left;
	int ih = rc.bottom - rc.top;
	int t;
	if (iw * 3 > 4 * ih)
		{
		t = 4 * ih / 3;
		rc.left += (iw - t) / 2;
		rc.right = rc.left + t;
		}
	else
		{
		t =  3 * iw / 4;
		rc.top += t;
		rc.bottom = rc.top + t;
		}
}

void SetMode(BOOL bInterleave)
{
	IConfigInterleaving *pInterleave = NULL;
	HRESULT hr = gcap.pMux->QueryInterface(IID_IConfigInterleaving,
					(void**)&pInterleave);
	if (SUCCEEDED(hr))
		{
		if (bInterleave)
			pInterleave->put_Mode(INTERLEAVE_CAPTURE);
		else
			pInterleave->put_Mode(INTERLEAVE_NONE);
		pInterleave->Release();
		}
	else
		{
		DPF("cannot set mode");
		}
}

 
void SetMasterStream()
{
	IConfigAviMux *pConfigMux = NULL;
	HRESULT hr = gcap.pMux->QueryInterface(IID_IConfigAviMux, (void**)&pConfigMux);
	if (SUCCEEDED(hr))
		{
//		hr = pConfigMux->SetOutputCompatibilityIndex(1);
		if (SUCCEEDED(hr) &&  gcap.fCapAudio)
			hr = pConfigMux->SetMasterStream(1);
		pConfigMux->Release();
		}
	if (!SUCCEEDED(hr))
		{
        ErrMsg(TEXT("SetMasterStream failed!"));
		DPF("cannot set stream");
		}
}


// build the capture graph
//
BOOL BuildCaptureGraph()
{
	HRESULT hr;
	AM_MEDIA_TYPE *pmt=0;

    if(gcap.fCaptureGraphBuilt)
        return TRUE;			// if already , done

	if(gcap.fCapturing || gcap.fPreviewing)
        return FALSE; // No rebuilding while we're running

    if(gcap.pVCap == NULL)
        return FALSE; // We don't have the necessary capture filters
    if(gcap.pACap == NULL && gcap.fCapAudio)
        return FALSE;
	ASSERT(gcap.szCaptureFile[0]);
    if(gcap.fPreviewGraphBuilt)		// we already have another graph built...
        TearDownGraph();			// tear down the old one

    //
    // We need a rendering section that will write the capture file out in AVI
    // file format
    //

    GUID guid;
    if( gcap.fMPEG2 )
        guid = MEDIASUBTYPE_Mpeg2;
    else
        guid = MEDIASUBTYPE_Avi;

    hr = gcap.pBuilder->SetOutputFileName(&guid, MyWide(gcap.szCaptureFile),
                                          &gcap.pMux, &gcap.pSink);
    if(hr != NOERROR)
		{
        ErrMsg(TEXT("Cannot set output file"));
        goto SetupCaptureFail;
		}

    // Now tell the AVIMUX to write out AVI files that old apps can read properly.
    // If we don't, most apps won't be able to tell where the keyframes are,
    // slowing down editing considerably
    // Doing this will cause one seek (over the area the index will go) when
    // you capture past 1 Gig, but that's no big deal.
    // NOTE: This is on by default, so it's not necessary to turn it on

    // Also, set the proper MASTER STREAM

    if( !gcap.fMPEG2 )
    	{
	//	SetMasterStream(gcap.fCapAudio ? 1 : 0, 1);
		SetMasterStream();
	    }

	SetMode(TRUE);

    //
    // Render the video capture and preview pins - even if the capture filter only
    // has a capture pin (and no preview pin) this should work... because the
    // capture graph builder will use a smart tee filter to provide both capture
    // and preview.  We don't have to worry.  It will just work.
    //

    // NOTE that we try to render the interleaved pin before the video pin, because
    // if BOTH exist, it's a DV filter and the only way to get the audio is to use
    // the interleaved pin.  Using the Video pin on a DV filter is only useful if
    // you don't want the audio.

    if( !gcap.fMPEG2 )
    {
        hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_CAPTURE,
                                             &MEDIATYPE_Video,
                                             gcap.pVCap, gcap.pVCmp, gcap.pMux);
            if(hr != NOERROR)
            {
                ErrMsg(TEXT("Cannot render video capture stream"));
                goto SetupCaptureFail;
            }
       

        if(gcap.fWantPreview)
        {
            hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_PREVIEW,
							&MEDIATYPE_Interleaved, gcap.pVCap, NULL, NULL);
            if(hr == VFW_S_NOPREVIEWPIN)
            {
                // preview was faked up for us using the (only) capture pin
               gcap.fPreviewFaked = TRUE;
           }
            else if(hr != S_OK)
            {
                hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_PREVIEW,
								&MEDIATYPE_Video, gcap.pVCap, NULL, NULL);
                if(hr == VFW_S_NOPREVIEWPIN)
                {
                    // preview was faked up for us using the (only) capture pin
                    gcap.fPreviewFaked = TRUE;
                }
                else if(hr != S_OK)
                {
                    ErrMsg(TEXT("Cannot render video preview stream"));
                    goto SetupCaptureFail;
                }
            }
        }
    }
    else
    {
        SmartPtr< IBaseFilter > sink;
        if( &gcap.pSink )
        {
            gcap.pSink->QueryInterface( IID_IBaseFilter,
							reinterpret_cast<void **>( &sink ) );
        }

        hr = gcap.pBuilder->RenderStream(NULL,
                                         &MEDIATYPE_Stream,
                                         gcap.pVCap, NULL, sink);
    }

//		int zz = zinfo();
//		DPF("zinfo:%d",zz);


    //
    // Render the audio capture pin?
    //

    if(!gcap.fMPEG2 && gcap.fCapAudio)
    {
        hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Audio,
                                         gcap.pACap, NULL, gcap.pMux);
        if(hr != NOERROR)
        {
            ErrMsg(TEXT("Cannot render audio capture stream"));
            goto SetupCaptureFail;
        }
    }

    //
    // Render the closed captioning pin? It could be a CC or a VBI category pin,
    // depending on the capture driver
    //
/*
    if(!gcap.fMPEG2  && gcap.fCapCC)
    {
        hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_CC, NULL,
                                         gcap.pVCap, NULL, gcap.pMux);
        if(hr != NOERROR)
        {
            hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_VBI, NULL,
                                             gcap.pVCap, NULL, gcap.pMux);
            if(hr != NOERROR)
            {
                ErrMsg(TEXT("Cannot render closed captioning"));
                // so what? goto SetupCaptureFail;
            }
        }
        // To preview and capture VBI at the same time, we can call this twice
        if(gcap.fWantPreview)
        {
  //          hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_VBI, NULL,
  //                                           gcap.pVCap, NULL, NULL);
        }
    }
*/
    //
    // Get the preview window to be a child of our app's window
    //

    // This will find the IVideoWindow interface on the renderer.  It is
    // important to ask the filtergraph for this interface... do NOT use
    // ICaptureGraphBuilder2::FindInterface, because the filtergraph needs to
    // know we own the window so it can give us display changed messages, etc.

    if(!gcap.fMPEG2 && gcap.fWantPreview)
    {
        hr = gcap.pFg->QueryInterface(IID_IVideoWindow, (void **)&gcap.pVW);
        if(hr != NOERROR && gcap.fWantPreview)
        {
            ErrMsg(TEXT("This graph cannot preview"));
        }
        else if(hr == NOERROR)
        {
            RECT rc;
            gcap.pVW->put_Owner((OAHWND)ghwndApp);    // We own the window now
            gcap.pVW->put_WindowStyle(WS_CHILD);    // you are now a child

            // give the preview window all our space but where the status bar is
            GetClientRect(ghwndApp, &rc);
         //   cyBorder = GetSystemMetrics(SM_CYBORDER);
         //   cy = cyBorder;
         //   cy = statusGetHeight() + cyBorder;
         //   rc.bottom -= cy;

		AspectRect(rc);
        gcap.pVW->SetWindowPosition(rc.left,rc.top,rc.right-rc.left, rc.bottom-rc.top);
     //       gcap.pVW->SetWindowPosition(0, 0, 640,480);//rc.right, rc.bottom); // be this big
            gcap.pVW->put_Visible(OATRUE);
        }
    }

/*
    // now tell it what frame rate to capture at.  Just find the format it
    // is capturing with, and leave everything alone but change the frame rate
    if( !gcap.fMPEG2 )
    {
        hr = NOERROR;//gcap.fUseFrameRate ? E_FAIL : NOERROR;
        if(gcap.pVSC)
        {
            hr = gcap.pVSC->GetFormat(&pmt);

            // DV capture does not use a VIDEOINFOHEADER
            if(hr == NOERROR)
            {
                if(pmt->formattype == FORMAT_VideoInfo)
                {
                    VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)pmt->pbFormat;
                    pvi->AvgTimePerFrame = (LONGLONG)(10000000 / 30);
                    hr = gcap.pVSC->SetFormat(pmt);
                }
                DeleteMediaType(pmt);
            }
        }
        if(hr != NOERROR)
            ErrMsg(TEXT("Cannot set frame rate for capture"));
    }
*/
    // now ask the filtergraph to tell us when something is completed or aborted
    // (EC_COMPLETE, EC_USERABORT, EC_ERRORABORT).  This is how we will find out
    // if the disk gets full while capturing
    hr = gcap.pFg->QueryInterface(IID_IMediaEventEx, (void **)&gcap.pME);
	if(hr == NOERROR)
		{
		gcap.pME->SetNotifyWindow((OAHWND)0, 0, 0);
		}

    // potential debug output - what the graph looks like
    // DumpGraph(gcap.pFg, 1);

    // Add our graph to the running object table, which will allow
    // the GraphEdit application to "spy" on our graph
    // All done.
//	if (gcap.pVC)
//		zinfo();
    gcap.fCaptureGraphBuilt = TRUE;
    return TRUE;

SetupCaptureFail:
    TearDownGraph();
    return FALSE;
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
BOOL BuildPreviewGraph()
{
    int cy, cyBorder;
    HRESULT hr;
//    AM_MEDIA_TYPE *pmt;

    // we have one already
    if(gcap.fPreviewGraphBuilt)
        return TRUE;

    // No rebuilding while we're running
    if(gcap.fCapturing || gcap.fPreviewing)
        return FALSE;

    // We don't have the necessary capture filters
    if(gcap.pVCap == NULL)
        return FALSE;
    if(gcap.pACap == NULL && gcap.fCapAudio)
        return FALSE;

    // we already have another graph built... tear down the old one
    if(gcap.fCaptureGraphBuilt)
        TearDownGraph();

    //
    // Render the preview pin - even if there is not preview pin, the capture
    // graph builder will use a smart tee filter and provide a preview.
    //
    // !!! what about latency/buffer issues?

    // NOTE that we try to render the interleaved pin before the video pin, because
    // if BOTH exist, it's a DV filter and the only way to get the audio is to use
    // the interleaved pin.  Using the Video pin on a DV filter is only useful if
    // you don't want the audio.

    if( gcap.fMPEG2 )
    {
        hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_PREVIEW,
                                         &MEDIATYPE_Stream, gcap.pVCap, NULL, NULL);
        if( FAILED( hr ) )
        {
            ErrMsg(TEXT("Cannot build MPEG2 preview graph!"));
        }

    }
    else
    {
        hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_PREVIEW,
                              &MEDIATYPE_Interleaved, gcap.pVCap, NULL, NULL);
        if(hr == VFW_S_NOPREVIEWPIN)
       {
            // preview was faked up for us using the (only) capture pin
            gcap.fPreviewFaked = TRUE;
        }
       else if(hr != S_OK)
        {
            // maybe it's DV?
            hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_PREVIEW,
                                 &MEDIATYPE_Video, gcap.pVCap, NULL, NULL);
            if(hr == VFW_S_NOPREVIEWPIN)
            {
                // preview was faked up for us using the (only) capture pin
                gcap.fPreviewFaked = TRUE;
            }
            else if(hr != S_OK)
            {
                ErrMsg(TEXT("This graph cannot preview!"));
                gcap.fPreviewGraphBuilt = FALSE;
                return FALSE;
            }
        }

        //
        // Render the closed captioning pin? It could be a CC or a VBI category pin,
        // depending on the capture driver
        //

        if(gcap.fCapCC)
        {
            hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_CC, NULL,
                                             gcap.pVCap, NULL, NULL);
            if(hr != NOERROR)
            {
                hr = gcap.pBuilder->RenderStream(&PIN_CATEGORY_VBI, NULL,
                                                 gcap.pVCap, NULL, NULL);
                if(hr != NOERROR)
                {
                    ErrMsg(TEXT("Cannot render closed captioning"));
                }
            }
        }
    }

    //
    // Get the preview window to be a child of our app's window
    //

    // This will find the IVideoWindow interface on the renderer.  It is
    // important to ask the filtergraph for this interface... do NOT use
    // ICaptureGraphBuilder2::FindInterface, because the filtergraph needs to
    // know we own the window so it can give us display changed messages, etc.

    hr = gcap.pFg->QueryInterface(IID_IVideoWindow, (void **)&gcap.pVW);
    if(hr != NOERROR)
    {
        ErrMsg(TEXT("This graph cannot preview properly"));
    }
    else
    {
        //Find out if this is a DV stream
        AM_MEDIA_TYPE * pmtDV;

        if(gcap.pVSC && SUCCEEDED(gcap.pVSC->GetFormat(&pmtDV)))
        {
            if(pmtDV->formattype == FORMAT_DvInfo)
            {
                // in this case we want to set the size of the parent window to that of
                // current DV resolution.
                // We get that resolution from the IVideoWindow.
                SmartPtr<IBasicVideo> pBV;

				// If we got here, gcap.pVW is not NULL 
				ASSERT(gcap.pVW != NULL);
				hr = gcap.pVW->QueryInterface(IID_IBasicVideo, (void**)&pBV);

                if(SUCCEEDED(hr))
                {
                    HRESULT hr1, hr2;
                    long lWidth, lHeight;

                    hr1 = pBV->get_VideoHeight(&lHeight);
                    hr2 = pBV->get_VideoWidth(&lWidth);
                    if(SUCCEEDED(hr1) && SUCCEEDED(hr2))
                    {
   //                     ResizeWindow(lWidth, abs(lHeight));
                    }
                }
            }
        }

        RECT rc;
        gcap.pVW->put_Owner((OAHWND)ghwndApp);    // We own the window now
        gcap.pVW->put_WindowStyle(WS_CHILD);    // you are now a child

        // give the preview window all our space but where the status bar is
        GetClientRect(ghwndApp, &rc);
        cyBorder = GetSystemMetrics(SM_CYBORDER);
      //  cy = statusGetHeight() + cyBorder;
        cy = cyBorder;
        rc.bottom -= cy;
//	gcap.pVW->put_Width(800);
		AspectRect(rc);
        gcap.pVW->SetWindowPosition(rc.left,rc.top,rc.right-rc.left, rc.bottom-rc.top);
        gcap.pVW->put_Visible(OATRUE);
    }

    // make sure we process events while we're previewing!
	hr = gcap.pFg->QueryInterface(IID_IMediaEventEx, (void **)&gcap.pME);
	if(hr == NOERROR)
		{
		gcap.pME->SetNotifyWindow((OAHWND)0, 0, 0);
		}

    // potential debug output - what the graph looks like
    // DumpGraph(gcap.pFg, 1);

    // Add our graph to the running object table, which will allow
    // the GraphEdit application to "spy" on our graph
    // All done.
    gcap.fPreviewGraphBuilt = TRUE;
    return TRUE;
}


// Start previewing
//
BOOL StartPreview()
{
    // way ahead of you
    if(gcap.fPreviewing)
        return TRUE;

    if(!gcap.fPreviewGraphBuilt)
        return FALSE;

    // run the graph
    IMediaControl *pMC = NULL;
    HRESULT hr = gcap.pFg->QueryInterface(IID_IMediaControl, (void **)&pMC);
    if(SUCCEEDED(hr))
    {
        hr = pMC->Run();
        if(FAILED(hr))
        {
            // stop parts that ran
            pMC->Stop();
        }
        pMC->Release();
    }
    if(FAILED(hr))
    {
        ErrMsg(TEXT("Error %x: Cannot run preview graph"), hr);
        return FALSE;
    }

    gcap.fPreviewing = TRUE;
    return TRUE;
}


// stop the preview graph
//
BOOL StopPreview()
{
    // way ahead of you
    if(!gcap.fPreviewing)
    {
        return FALSE;
    }

    // stop the graph
    IMediaControl *pMC = NULL;
    HRESULT hr = gcap.pFg->QueryInterface(IID_IMediaControl, (void **)&pMC);
    if(SUCCEEDED(hr))
    {
        hr = pMC->Stop();
        pMC->Release();
    }
    if(FAILED(hr))
    {
        ErrMsg(TEXT("Error %x: Cannot stop preview graph"), hr);
        return FALSE;
    }

    gcap.fPreviewing = FALSE;

    // get rid of menu garbage
    InvalidateRect(ghwndApp, NULL, TRUE);

    return TRUE;
}


// start the capture graph
//
BOOL StartCapture()
{
    BOOL fHasStreamControl;
    HRESULT hr;

    // way ahead of you
    if(gcap.fCapturing)
        return TRUE;

    // or we'll get confused
    if(gcap.fPreviewing)
        StopPreview();

    // or we'll crash
    if(!gcap.fCaptureGraphBuilt)
        return FALSE;
    SAFE_RELEASE(gcap.pDF);
	gcap.pDF = 0;
    // This amount will be subtracted from the number of dropped and not
    // dropped frames reported by the filter.  Since we might be having the
    // filter running while the pin is turned off, we don't want any of the
    // frame statistics from the time the pin is off interfering with the
    // statistics we gather while the pin is on
    gcap.lDroppedBase = 0;
    gcap.lNotBase = 0;

    REFERENCE_TIME start = MAXLONGLONG, stop = MAXLONGLONG;

    // don't capture quite yet...
    hr = gcap.pBuilder->ControlStream(&PIN_CATEGORY_CAPTURE, NULL,
                                      NULL, &start, NULL, 0, 0);

    // Do we have the ability to control capture and preview separately?
    fHasStreamControl = SUCCEEDED(hr);

    // prepare to run the graph
    IMediaControl *pMC = NULL;
    hr = gcap.pFg->QueryInterface(IID_IMediaControl, (void **)&pMC);
    if(FAILED(hr))
    {
        ErrMsg(TEXT("Error %x: Cannot get IMediaControl"), hr);
        return FALSE;
    }

    // If we were able to keep capture off, then we can
    // run the graph now for frame accurate start later yet still showing a
    // preview.   Otherwise, we can't run the graph yet without capture
    // starting too, so we'll pause it so the latency between when they
    // press a key and when capture begins is still small (but they won't have
    // a preview while they wait to press a key)

    if(fHasStreamControl)
        hr = pMC->Run();
    else
        hr = pMC->Pause();
    if(FAILED(hr))
    {
        // stop parts that started
        pMC->Stop();
        pMC->Release();
        ErrMsg(TEXT("Error %x: Cannot start graph"), hr);
        return FALSE;
    }
    // Start capture NOW!
    if(fHasStreamControl)
    {
        // we may not have this yet
        if(!gcap.pDF)
        {
            hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
                                      &MEDIATYPE_Interleaved, gcap.pVCap,
                                     IID_IAMDroppedFrames, (void **)&gcap.pDF);
            if(hr != NOERROR)
                hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
                                      &MEDIATYPE_Video, gcap.pVCap,
                                      IID_IAMDroppedFrames, (void **)&gcap.pDF);
        }

        // turn the capture pin on now!
        hr = gcap.pBuilder->ControlStream(&PIN_CATEGORY_CAPTURE, NULL,
            NULL, NULL, &stop, 0, 0);
        // make note of the current dropped frame counts

        if(gcap.pDF)
        {
            gcap.pDF->GetNumDropped(&gcap.lDroppedBase);
            gcap.pDF->GetNumNotDropped(&gcap.lNotBase);
DPF("dropbase:%d, notbase:%d",gcap.lDroppedBase, gcap.lNotBase);
        }
    }
    else
    {
        hr = pMC->Run();
        if(FAILED(hr))
        {
            // stop parts that started
            pMC->Stop();
            pMC->Release();
            ErrMsg(TEXT("Error %x: Cannot run graph"), hr);
            return FALSE;
        }
    }

    pMC->Release();

    // when did we start capture?
    gcap.lCapStartTime = timeGetTime();

    // update status bar 30 times per second - #captured, #dropped
//    SetTimer(ghwndApp, 1, 33, NULL);

    gcap.fCapturing = TRUE;
    return TRUE;
}


// stop the capture graph
//
BOOL StopCapture()
{
    // way ahead of you
    if(!gcap.fCapturing)
    {
        return FALSE;
    }

    // stop the graph
    IMediaControl *pMC = NULL;
    HRESULT hr = gcap.pFg->QueryInterface(IID_IMediaControl, (void **)&pMC);
    if(SUCCEEDED(hr))
    {
        hr = pMC->Stop();
        pMC->Release();
    }
    if(FAILED(hr))
    {
        ErrMsg(TEXT("Error %x: Cannot stop graph"), hr);
        return FALSE;
    }

    // when the graph was stopped
    gcap.lCapStopTime = timeGetTime();

    // no more status bar updates
//    KillTimer(ghwndApp, 1);

    // one last time for the final count and all the stats
    UpdateStatus(TRUE);

    gcap.fCapturing = FALSE;

    // get rid of menu garbage
    InvalidateRect(ghwndApp, NULL, TRUE);

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


void MakeMenuOptions()
{
    HRESULT hr;
    int zz = 0;
    gcap.iFormatDialogPos = -1;
    gcap.iSourceDialogPos = -1;
    gcap.iDisplayDialogPos = -1;
    gcap.iVCapDialogPos = -1;
    gcap.iVCrossbarDialogPos = -1;
    gcap.iTVTunerDialogPos = -1;
    gcap.iACapDialogPos = -1;
    gcap.iACrossbarDialogPos = -1;
    gcap.iTVAudioDialogPos = -1;
    gcap.iVCapCapturePinDialogPos = -1;
    gcap.iVCapPreviewPinDialogPos = -1;
    gcap.iACapCapturePinDialogPos = -1;

	gcap.iNumVCapDialogs = 0;
	gcap.iNumACapDialogs = 0;
    // If this device supports the old legacy UI dialogs, offer them
    if(gcap.pDlg)
    {

        hr = gcap.pDlg->HasDialog(VfwCaptureDialog_Format);
        if(SUCCEEDED(hr) && S_FALSE != hr)
        {
            strcpy_s(gcap.szDialogs[zz], TEXT("Video Format..."));
			gcap.VDialogs[gcap.iNumVCapDialogs++] = zz;
            gcap.iFormatDialogPos = zz++;
        }
        hr = gcap.pDlg->HasDialog(VfwCaptureDialog_Source);
        if(SUCCEEDED(hr) && S_FALSE != hr)
        {
            strcpy_s(gcap.szDialogs[zz], TEXT("Video Source..."));
			gcap.VDialogs[gcap.iNumVCapDialogs++] = zz;
            gcap.iSourceDialogPos = zz++;
        }
        hr = gcap.pDlg->HasDialog(VfwCaptureDialog_Display);
        if(SUCCEEDED(hr) && S_FALSE != hr)
        {
            strcpy_s(gcap.szDialogs[zz], TEXT("Video Display..."));
			gcap.VDialogs[gcap.iNumVCapDialogs++] = zz;
            gcap.iDisplayDialogPos = zz++;
        }
    }
    // Also check the audio capture filter at this point, since even non wdm devices
    // may support an IAMAudioInputMixer property page (we'll also get any wdm filter
    // properties here as well). We'll get any audio capture pin property pages just
    // a bit later.
    if(gcap.pACap != NULL)
    {
        ISpecifyPropertyPages *pSpec;
        CAUUID cauuid;

        hr = gcap.pACap->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
        if(SUCCEEDED(hr))
        {
            hr = pSpec->GetPages(&cauuid);
            if(SUCCEEDED(hr) && cauuid.cElems > 0)
            {
            	strcpy_s(gcap.szDialogs[zz], TEXT("Audio Capture Filter..."));
				gcap.ADialogs[gcap.iNumACapDialogs++] = zz;
                gcap.iACapDialogPos = zz++;
                CoTaskMemFree(cauuid.pElems);
            }
            pSpec->Release();
        }
    }

    // don't bother looking for new property pages if the old ones are supported
    // or if we don't have a capture filter
    if(gcap.pVCap == NULL || gcap.iFormatDialogPos != -1)
        return;

    // New WDM devices support new UI and new interfaces.
    // Your app can use some default property
    // pages for UI if you'd like (like we do here) or if you don't like our
    // dialog boxes, feel free to make your own and programmatically set
    // the capture options through interfaces like IAMCrossbar, IAMCameraControl
    // etc.

    // There are 9 objects that might support property pages.
    // Let's go through them.

    ISpecifyPropertyPages *pSpec;
    CAUUID cauuid;

    // 1. the video capture filter itself

    hr = gcap.pVCap->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
    if(SUCCEEDED(hr))
    {
        hr = pSpec->GetPages(&cauuid);
        if(SUCCEEDED(hr) && cauuid.cElems > 0)
        {
            strcpy_s(gcap.szDialogs[zz], TEXT("Video Capture Filter..."));
			gcap.VDialogs[gcap.iNumVCapDialogs++] = zz;
            gcap.iVCapDialogPos = zz++;
            CoTaskMemFree(cauuid.pElems);
        }
        pSpec->Release();
    }

    // 2.  The video capture capture pin

    IAMStreamConfig *pSC;

//    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
//                                      &MEDIATYPE_Interleaved,
//                                      gcap.pVCap, IID_IAMStreamConfig, (void **)&pSC);
//    if(FAILED(hr))
        hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
                                      &MEDIATYPE_Video, gcap.pVCap,
                                      IID_IAMStreamConfig, (void **)&pSC);

    if(SUCCEEDED(hr))
    {
        hr = pSC->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
        if(SUCCEEDED(hr))
        {
            hr = pSpec->GetPages(&cauuid);
            if(SUCCEEDED(hr) && cauuid.cElems > 0)
            {
	            strcpy_s(gcap.szDialogs[zz], TEXT("Video Capture Pin..."));
				gcap.VDialogs[gcap.iNumVCapDialogs++] = zz;
                gcap.iVCapCapturePinDialogPos = zz++;
                CoTaskMemFree(cauuid.pElems);
            }
            pSpec->Release();
        }
        pSC->Release();
    }

    // 3.  The video capture preview pin.
    // This basically sets the format being previewed.  Typically, you
    // want to capture and preview using the SAME format, instead of having to
    // enter the same value in 2 dialog boxes.  For a discussion on this, see
    // the comment above the MakePreviewGraph function.

//    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_PREVIEW,
//                                      &MEDIATYPE_Interleaved, gcap.pVCap,
///                                      IID_IAMStreamConfig, (void **)&pSC);
//    if(FAILED(hr))
        hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_PREVIEW,
                                          &MEDIATYPE_Video, gcap.pVCap,
                                          IID_IAMStreamConfig, (void **)&pSC);
    if(SUCCEEDED(hr))
    {
        hr = pSC->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
        if(SUCCEEDED(hr))
        {
            hr = pSpec->GetPages(&cauuid);
            if(SUCCEEDED(hr) && cauuid.cElems > 0)
            {
	            strcpy_s(gcap.szDialogs[zz], TEXT("Video Preview Pin..."));
				gcap.VDialogs[gcap.iNumVCapDialogs++] = zz;
                gcap.iVCapPreviewPinDialogPos = zz++;
                CoTaskMemFree(cauuid.pElems);
            }
            pSpec->Release();
        }
        pSC->Release();
    }

    // 4 & 5.  The video crossbar, and a possible second crossbar

    IAMCrossbar *pX, *pX2;
    IBaseFilter *pXF;

//    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
//                                    &MEDIATYPE_Interleaved, gcap.pVCap,
//                                    IID_IAMCrossbar, (void **)&pX);
//    if(FAILED(hr))
        hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
                                    &MEDIATYPE_Video, gcap.pVCap,
                                    IID_IAMCrossbar, (void **)&pX);

    if(SUCCEEDED(hr))
    {
        hr = pX->QueryInterface(IID_IBaseFilter, (void **)&pXF);
        if(SUCCEEDED(hr))
        {
            hr = pX->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
            if(SUCCEEDED(hr))
            {
                hr = pSpec->GetPages(&cauuid);
                if(SUCCEEDED(hr) && cauuid.cElems > 0)
                {
	            	strcpy_s(gcap.szDialogs[zz], TEXT("Video Crossbar..."));
					gcap.VDialogs[gcap.iNumVCapDialogs++] = zz;
                    gcap.iVCrossbarDialogPos = zz++;
                    CoTaskMemFree(cauuid.pElems);
                }
                pSpec->Release();
            }

            hr = gcap.pBuilder->FindInterface(&LOOK_UPSTREAM_ONLY, NULL, pXF,
                                              IID_IAMCrossbar, (void **)&pX2);
            if(SUCCEEDED(hr))
            {
                hr = pX2->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
                if(SUCCEEDED(hr))
                {
                    hr = pSpec->GetPages(&cauuid);
                    if(SUCCEEDED(hr) && cauuid.cElems > 0)
                    {
	            		strcpy_s(gcap.szDialogs[zz], TEXT("Second Crossbar..."));
						gcap.VDialogs[gcap.iNumVCapDialogs++] = zz;
                        gcap.iACrossbarDialogPos = zz++;
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

//    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
//                                      &MEDIATYPE_Interleaved, gcap.pVCap,
//                                      IID_IAMTVTuner, (void **)&pTV);
///    if(FAILED(hr))
        hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
                                          &MEDIATYPE_Video, gcap.pVCap,
                                          IID_IAMTVTuner, (void **)&pTV);
    if(SUCCEEDED(hr))
    {
        hr = pTV->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
        if(SUCCEEDED(hr))
        {
            hr = pSpec->GetPages(&cauuid);
            if(SUCCEEDED(hr) && cauuid.cElems > 0)
            {
	            strcpy_s(gcap.szDialogs[zz], TEXT("TV Tuner..."));
				gcap.VDialogs[gcap.iNumVCapDialogs++] = zz;
                gcap.iTVTunerDialogPos = zz++;
                CoTaskMemFree(cauuid.pElems);
            }
            pSpec->Release();
        }
        pTV->Release();
    }

    // no audio capture, we're done
    if(gcap.pACap == NULL)
        return;

    // 7.  The Audio capture filter itself... Thanks anyway, but we got these already

    // 8.  The Audio capture pin

    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
                                      &MEDIATYPE_Audio, gcap.pACap,
                                      IID_IAMStreamConfig, (void **)&pSC);
    if(SUCCEEDED(hr))
    {
        hr = pSC->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
        if(SUCCEEDED(hr))
        {
            hr = pSpec->GetPages(&cauuid);
            if(SUCCEEDED(hr) && cauuid.cElems > 0)
            {
	            strcpy_s(gcap.szDialogs[zz], TEXT("Audio Capture Pin..."));
				gcap.ADialogs[gcap.iNumACapDialogs++] = zz;
                gcap.iACapCapturePinDialogPos = zz++;
                CoTaskMemFree(cauuid.pElems);
            }
            pSpec->Release();
        }
        pSC->Release();
    }

    // 9.  The TV Audio filter

    IAMTVAudio *pTVA;
    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
                                      &MEDIATYPE_Audio, gcap.pACap,
                                      IID_IAMTVAudio, (void **)&pTVA);
    if(SUCCEEDED(hr))
    {
        hr = pTVA->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
        if(SUCCEEDED(hr))
        {
            hr = pSpec->GetPages(&cauuid);
            if(SUCCEEDED(hr) && cauuid.cElems > 0)
            {
	            strcpy_s(gcap.szDialogs[zz], TEXT("TV Audio..."));
				gcap.ADialogs[gcap.iNumACapDialogs++] = zz;
                gcap.iTVAudioDialogPos = zz++;
                CoTaskMemFree(cauuid.pElems);
            }
            pSpec->Release();
        }
        pTVA->Release();
    }
//#ifdef CCCC
    // 10.  Crossbar class helper menu item, to let you choose an input

    if(gcap.pCrossbar && gcap.NumberOfVideoInputs)
    {
        gcap.hMenuPopup = CreatePopupMenu();
        LONG j;
        LONG  PhysicalType;
        TCHAR buf[MAX_PATH];
        LONG InputToEnable = -1;

   //     gcap.iVideoInputMenuPos = zz++;
   //     AppendMenu(hMenuSub, MF_SEPARATOR, 0, NULL);

        for(j = 0; j < gcap.NumberOfVideoInputs; j++)
        {
            hr = gcap.pCrossbar->GetInputType(j, &PhysicalType);
			ASSERT(hr == S_OK);

            hr = gcap.pCrossbar->GetInputName(j, buf, sizeof (buf));
			ASSERT(hr == S_OK);

	       strcpy_s(gcap.szDialogs[zz], buf);
		gcap.ADialogs[gcap.iNumACapDialogs++] = zz;
           gcap.iTVAudioDialogPos = zz++;
       //     AppendMenu(gcap.hMenuPopup,MF_STRING,MENU_DIALOG0+zz, buf);
            zz++;

            // Route the first TVTuner by default
            if((PhysicalType == PhysConn_Video_Tuner) && InputToEnable == -1)
            {
                InputToEnable = j;
            }
        }

     //   AppendMenu(hMenuSub, MF_STRING | MF_POPUP, (UINT_PTR)gcap.hMenuPopup, TEXT("Video Input"));

      //  if(InputToEnable == -1)
       // {
        //    InputToEnable = 0;
      //  }
        CheckMenuItem(gcap.hMenuPopup, InputToEnable, MF_BYPOSITION | MF_CHECKED);

        gcap.pCrossbar->SetInputIndex(0);//InputToEnable);
    }
    // !!! anything needed to delete the popup when selecting a new input?
//#endif
}


// how many captured/dropped so far
//
void UpdateStatus(BOOL fAllStats)
{
    HRESULT hr;
    LONG lDropped, lNot=0, lAvgFrameSize;
    TCHAR tach[160];

    // we use this interface to get the number of captured and dropped frames
    // NOTE:  We cannot query for this interface earlier, as it may not be
    // available until the pin is connected
    if(!gcap.pDF)
    {
        hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
                                          &MEDIATYPE_Interleaved, gcap.pVCap,
                                          IID_IAMDroppedFrames, (void **)&gcap.pDF);
        if(hr != S_OK)
            hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
                                              &MEDIATYPE_Video, gcap.pVCap,
                                              IID_IAMDroppedFrames, (void **)&gcap.pDF);
    }

    // this filter can't tell us dropped frame info.
    if(!gcap.pDF)
    {
        statusUpdateStatus(ghwndStatus, TEXT("Filter cannot report capture information"));
        return;
    }

    hr = gcap.pDF->GetNumDropped(&lDropped);
    if(hr == S_OK)
        hr = gcap.pDF->GetNumNotDropped(&lNot);
    if(hr != S_OK)
        return;
DPF("drop:%d,not:%d",lDropped, lNot);
    lDropped -= gcap.lDroppedBase;
    lNot -= gcap.lNotBase;
DPF("drop:%d,not:%d",lDropped, lNot);

    if(!fAllStats)
    {
        LONG lTime = timeGetTime() - gcap.lCapStartTime;
		lTime /= 10;
	hr = sprintf_s(tach,"Captured %d frames (%d dropped) %d.%dsec",
				lNot, lDropped, lTime / 100, lTime % 100);
        statusUpdateStatus(ghwndStatus, tach);
        return;
    }

    // we want all possible stats, including capture time and actual acheived
    // frame rate and data rate (as opposed to what we tried to get).  These
    // numbers are an indication that though we dropped frames just now, if we
    // chose a data rate and frame rate equal to the numbers I'm about to
    // print, we probably wouldn't drop any frames.

    // average size of frame captured
    hr = gcap.pDF->GetAverageFrameSize(&lAvgFrameSize);
    if(hr != S_OK)
        return;

    // how long capture lasted
    LONG lDurMS = gcap.lCapStopTime - gcap.lCapStartTime;
    double flFrame;     // acheived frame rate
    LONG lData;         // acheived data rate
	LONGLONG drate;
DPF("Dur:%d",lDurMS);
    if(lDurMS > 0)
    {
        flFrame = (double)(LONGLONG)lNot * 1000. /
            (double)(LONGLONG)lDurMS;
    //    lData = (LONG)(LONGLONG)(lNot / (double)(LONGLONG)lDurMS *
    //        1000. * (double)(LONGLONG)lAvgFrameSize);
			drate = lNot * lAvgFrameSize;
//			drate *= 1000;
			drate /= 10 * lDurMS;
    }
    else
    {
        flFrame = 0.;
        lData = 0;
		drate = 0;
    }
DPF("frame rate:%d",(UINT)(flFrame*100));
	lDurMS /= 10;
	UINT frate = (UINT)(flFrame * 100);
    hr = sprintf_s(tach,
"Captured %d frames in %d.%d sec (%d dropped): %d.%d fps %d.%d MBsec",
             lNot, lDurMS / 100, lDurMS % 100,
             lDropped, frate / 100, frate % 100,
			drate / 100,
			drate % 100);
      //       lData / 1000000,
      //       lData / 1000 - (lData / 1000000 * 1000));
    statusUpdateStatus(ghwndStatus, tach);
}


void ChooseDevices(int video_id, int audio_id, int comp_id)
{
#define VERSIZE 40
#define DESCSIZE 80

    int versize = VERSIZE;
    int descsize = DESCSIZE;
    WCHAR wachVer[VERSIZE]={0}, wachDesc[DESCSIZE]={0};
    TCHAR tachStatus[VERSIZE + DESCSIZE + 5]={0};

	IMoniker *pmVideo = gcap.rgpmVideoList[video_id];
	IMoniker *pmAudio = gcap.rgpmAudioList[audio_id];
	IMoniker *pmComp  = comp_id ? gcap.rgpmCompressList[comp_id-1] : 0;
    // they chose a new device. rebuild the graphs
    if((gcap.pmVideo != pmVideo) ||
			(gcap.pmAudio != pmAudio) || (gcap.pmCompress != pmComp))
		{
        if(pmVideo)
            pmVideo->AddRef();
        if(pmAudio)
            pmAudio->AddRef();
        if(pmComp)
            pmComp->AddRef();

        IMonRelease(gcap.pmVideo);
        IMonRelease(gcap.pmAudio);
        IMonRelease(gcap.pmCompress);
        gcap.pmVideo = pmVideo;
        gcap.pmAudio = pmAudio;
        gcap.pmCompress = pmComp;
		gcap.iVidDevice = video_id;
		gcap.iAudDevice = audio_id;
		gcap.iCompressor = comp_id;
        if(gcap.fPreviewing)
            StopPreview();
        if(gcap.fCaptureGraphBuilt || gcap.fPreviewGraphBuilt)
            TearDownGraph();

        FreeCapFilters();
        InitCapFilters();

        if(gcap.fWantPreview)   // were we previewing?
			{
            BuildPreviewGraph();
            StartPreview();
			}

        MakeMenuOptions();      // the UI choices change per device
		zinfo();
    }
/*
    if(gcap.pVC)
    {
        HRESULT hr = gcap.pVC->GetInfo(wachVer, &versize, wachDesc, &descsize,
                                       NULL, NULL, NULL, NULL);
        if(hr == S_OK)
        {
            // It's possible that the call succeeded without actually filling
            // in information for description and version.  If these strings
            // are empty, just display the device's friendly name.
            if(wcslen(wachDesc) && wcslen(wachVer))
            {
                hr = StringCchPrintf(tachStatus, VERSIZE + DESCSIZE + 5, TEXT("%s - %s\0"), wachDesc, wachVer);
                statusUpdateStatus(ghwndStatus, tachStatus);
                return;
            }
        }
    }

    // Since the GetInfo method failed (or the interface did not exist),
    // display the device's friendly name.
 //   statusUpdateStatus(ghwndStatus, MySub(gcap.wachFriendlyName));

*/
}


// put all installed video and audio devices in the menus
//
void AddDevicesToList()
{
    if(gcap.fDeviceListPopulated)
    {
        return;
    }
    gcap.fDeviceListPopulated = true;
    gcap.iNumVCapDevices = 0;
    gcap.iNumACapDevices = 0;
    gcap.iNumCompressors = 0;

    UINT    uIndex = 0;
    HRESULT hr;
    BOOL bCheck = FALSE;

    for(int i = 0; i < NUMELMS(gcap.rgpmVideoList); i++)
    {
        IMonRelease(gcap.rgpmVideoList[i]);
    }
    for(int i = 0; i < NUMELMS(gcap.rgpmAudioList); i++)
    {
        IMonRelease(gcap.rgpmAudioList[i]);
    }
    for(int i = 0; i < NUMELMS(gcap.rgpmCompressList); i++)
    {
        IMonRelease(gcap.rgpmCompressList[i]);
    }

    ICreateDevEnum *pCreateDevEnum = 0;
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
                          IID_ICreateDevEnum, (void**)&pCreateDevEnum);
    if((hr != NOERROR) || !pCreateDevEnum)
    	{
        ErrMsg(TEXT("Error Creating Device Enumerator"));
        return;
    	}
	EnumVideo( pCreateDevEnum);
	if (!gcap.iNumVCapDevices)
		{
        ErrMsg(TEXT("Sorry, you have no video capture hardware.\r\n\r\n")
               TEXT("Video capture will not function properly."));
        return;
		}

	EnumCompress( pCreateDevEnum);
	EnumAudio( pCreateDevEnum);
    pCreateDevEnum->Release();
}


int DoDialog(int id)
{
	HRESULT hr;
	if(id == gcap.iFormatDialogPos)
			{
                // this dialog will not work while previewing
                if(gcap.fWantPreview)
                    StopPreview();
                HRESULT hrD;
                hrD = gcap.pDlg->ShowDialog(VfwCaptureDialog_Format, ghwndApp);

                // Sometimes bringing up the FORMAT dialog can result
                // in changing to a capture format that the current graph
                // can't handle.  It looks like that has happened and we'll
                // have to rebuild the graph.
                if(hrD == VFW_E_CANNOT_CONNECT)
                {
                    TearDownGraph();    // now we need to rebuild
                    // !!! This won't work if we've left a stranded h/w codec
                }

                // Resize our window to be the same size that we're capturing
                if(gcap.pVSC)
                {
                    AM_MEDIA_TYPE *pmt;
                    // get format being used NOW
                    hr = gcap.pVSC->GetFormat(&pmt);

                    // DV capture does not use a VIDEOINFOHEADER
                    if(hr == NOERROR)
                    {
                        if(pmt->formattype == FORMAT_VideoInfo)
                        {
                            // resize our window to the new capture size
                            ResizeWindow(HEADER(pmt->pbFormat)->biWidth,
                                abs(HEADER(pmt->pbFormat)->biHeight));
                        }
                        DeleteMediaType(pmt);
                    }
                }

                if(gcap.fWantPreview)
                {
                    BuildPreviewGraph();
                    StartPreview();
                }
            }
            else if(id == gcap.iSourceDialogPos)
            {
                // this dialog will not work while previewing
                if(gcap.fWantPreview)
                    StopPreview();

                gcap.pDlg->ShowDialog(VfwCaptureDialog_Source, ghwndApp);
                if(gcap.fWantPreview)
                    StartPreview();
            }
            else if(id  == gcap.iDisplayDialogPos)
            {
                // this dialog will not work while previewing
                if(gcap.fWantPreview)
                    StopPreview();

                gcap.pDlg->ShowDialog(VfwCaptureDialog_Display, ghwndApp);
                if(gcap.fWantPreview)
                    StartPreview();

                // now the code for the new dialogs
            }
            else if(id == gcap.iVCapDialogPos)
            {
                ISpecifyPropertyPages *pSpec;
                CAUUID cauuid;

                hr = gcap.pVCap->QueryInterface(IID_ISpecifyPropertyPages,
                    (void **)&pSpec);
                if(hr == S_OK)
                {
                    hr = pSpec->GetPages(&cauuid);

                    hr = OleCreatePropertyFrame(ghwndApp, 30, 30, NULL, 1,
                        (IUnknown **)&gcap.pVCap, cauuid.cElems,
                        (GUID *)cauuid.pElems, 0, 0, NULL);

                    CoTaskMemFree(cauuid.pElems);
                    pSpec->Release();
                }
            }
            else if(id  == gcap.iVCapCapturePinDialogPos)
            {
                // You can change this pin's output format in these dialogs.
                // If the capture pin is already connected to somebody who's
                // fussy about the connection type, that may prevent using
                // this dialog(!) because the filter it's connected to might not
                // allow reconnecting to a new format. (EG: you switch from RGB
                // to some compressed type, and need to pull in a decoder)
                // I need to tear down the graph downstream of the
                // capture filter before bringing up these dialogs.
                // In any case, the graph must be STOPPED when calling them.
                if(gcap.fWantPreview)
                    StopPreview();  // make sure graph is stopped

                // The capture pin that we are trying to set the format on is connected if
                // one of these variable is set to TRUE. The pin should be disconnected for
                // the dialog to work properly.
                if(gcap.fCaptureGraphBuilt || gcap.fPreviewGraphBuilt)
                {
                    TearDownGraph();    // graph could prevent dialog working
                }

                IAMStreamConfig *pSC;
            //    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
            //        &MEDIATYPE_Interleaved, gcap.pVCap,
            //        IID_IAMStreamConfig, (void **)&pSC);

             //   if(hr != NOERROR)
                    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
                        &MEDIATYPE_Video, gcap.pVCap,
                        IID_IAMStreamConfig, (void **)&pSC);

                ISpecifyPropertyPages *pSpec;
                CAUUID cauuid;

                hr = pSC->QueryInterface(IID_ISpecifyPropertyPages,
                    (void **)&pSpec);

                if(hr == S_OK)
                {
                    hr = pSpec->GetPages(&cauuid);
                    hr = OleCreatePropertyFrame(ghwndApp, 30, 30, NULL, 1,
                        (IUnknown **)&pSC, cauuid.cElems,
                        (GUID *)cauuid.pElems, 0, 0, NULL);

                    // !!! What if changing output formats couldn't reconnect
                    // and the graph is broken?  Shouldn't be possible...

                    if(gcap.pVSC)
                    {
                        AM_MEDIA_TYPE *pmt;
                        // get format being used NOW
                        hr = gcap.pVSC->GetFormat(&pmt);

                        // DV capture does not use a VIDEOINFOHEADER
                        if(hr == NOERROR)
                        {
                            if(pmt->formattype == FORMAT_VideoInfo)
                            {
//	gcap.myfmt = pmt->subtype;
       //                         // resize our window to the new capture size
       //                         ResizeWindow(HEADER(pmt->pbFormat)->biWidth,
                         //           abs(HEADER(pmt->pbFormat)->biHeight));
                            }
                            DeleteMediaType(pmt);
                        }
                    }

                    CoTaskMemFree(cauuid.pElems);
                    pSpec->Release();
                }

                pSC->Release();
                if(gcap.fWantPreview)
                {
                    BuildPreviewGraph();
                    StartPreview();
                }
            }
            else if(id == gcap.iVCapPreviewPinDialogPos)
            {
                // this dialog may not work if the preview pin is connected
                // already, because the downstream filter may reject a format
                // change, so we better kill the graph. (EG: We switch from
                // capturing RGB to some compressed fmt, and need to pull in
                // a decompressor)
                if(gcap.fWantPreview)
                {
                    StopPreview();
                    TearDownGraph();
                }

                IAMStreamConfig *pSC;

                // This dialog changes the preview format, so it might affect
                // the format being drawn.  Our app's window size is taken
                // from the size of the capture pin's video, not the preview
                // pin, so changing that here won't have any effect. All in all,
                // this probably won't be a terribly useful dialog in this app.
            //    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_PREVIEW,
            //                                      &MEDIATYPE_Interleaved, gcap.pVCap,
            //                                      IID_IAMStreamConfig, (void **)&pSC);
            //    if (hr != NOERROR)
                {
                    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_PREVIEW,
                        &MEDIATYPE_Video, gcap.pVCap,
                        IID_IAMStreamConfig, (void **)&pSC);
                }

                ISpecifyPropertyPages *pSpec;
                CAUUID cauuid;

                hr = pSC->QueryInterface(IID_ISpecifyPropertyPages,
                    (void **)&pSpec);
                if(hr == S_OK)
                {
                    hr = pSpec->GetPages(&cauuid);

                    hr = OleCreatePropertyFrame(ghwndApp, 30, 30, NULL, 1,
                        (IUnknown **)&pSC, cauuid.cElems,
                        (GUID *)cauuid.pElems, 0, 0, NULL);

                    CoTaskMemFree(cauuid.pElems);
                    pSpec->Release();
                }

                pSC->Release();
                if(gcap.fWantPreview)
                {
                    BuildPreviewGraph();
                    StartPreview();
                }
            }
            else if(id  == gcap.iVCrossbarDialogPos)
            {
                IAMCrossbar *pX;

       //         hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
       //             &MEDIATYPE_Interleaved, gcap.pVCap,
       //             IID_IAMCrossbar, (void **)&pX);
       //         if(hr != NOERROR)
                    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
                        &MEDIATYPE_Video, gcap.pVCap,
                        IID_IAMCrossbar, (void **)&pX);

                ISpecifyPropertyPages *pSpec;
                CAUUID cauuid;

                hr = pX->QueryInterface(IID_ISpecifyPropertyPages,
                    (void **)&pSpec);
                if(hr == S_OK)
                {
                    hr = pSpec->GetPages(&cauuid);

                    hr = OleCreatePropertyFrame(ghwndApp, 30, 30, NULL, 1,
                        (IUnknown **)&pX, cauuid.cElems,
                        (GUID *)cauuid.pElems, 0, 0, NULL);

                    CoTaskMemFree(cauuid.pElems);
                    pSpec->Release();
                }
                pX->Release();
            }
            else if(id  == gcap.iTVTunerDialogPos)
            {
                IAMTVTuner *pTV;
         //       hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
          //          &MEDIATYPE_Interleaved, gcap.pVCap,
          //          IID_IAMTVTuner, (void **)&pTV);
          //      if(hr != NOERROR)
                    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
                        &MEDIATYPE_Video, gcap.pVCap,
                        IID_IAMTVTuner, (void **)&pTV);

                ISpecifyPropertyPages *pSpec;
                CAUUID cauuid;

                hr = pTV->QueryInterface(IID_ISpecifyPropertyPages,
                    (void **)&pSpec);
                if(hr == S_OK)
                {
                    hr = pSpec->GetPages(&cauuid);

                    hr = OleCreatePropertyFrame(ghwndApp, 30, 30, NULL, 1,
                        (IUnknown **)&pTV, cauuid.cElems,

                        (GUID *)cauuid.pElems, 0, 0, NULL);
                    CoTaskMemFree(cauuid.pElems);
                    pSpec->Release();
                }
                pTV->Release();
            }
            else if(id == gcap.iACapDialogPos)
            {
                ISpecifyPropertyPages *pSpec;
                CAUUID cauuid;

                hr = gcap.pACap->QueryInterface(IID_ISpecifyPropertyPages,
                    (void **)&pSpec);
                if(hr == S_OK)
                {
                    hr = pSpec->GetPages(&cauuid);

                    hr = OleCreatePropertyFrame(ghwndApp, 30, 30, NULL, 1,
                        (IUnknown **)&gcap.pACap, cauuid.cElems,
                        (GUID *)cauuid.pElems, 0, 0, NULL);

                    CoTaskMemFree(cauuid.pElems);
                    pSpec->Release();
                }
            }
            else if(id == gcap.iACapCapturePinDialogPos)
            {
                // this dialog will not work while previewing - it might change
                // the output format!
                if(gcap.fWantPreview)
                    StopPreview();

                IAMStreamConfig *pSC;
                hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
                    &MEDIATYPE_Audio, gcap.pACap,
                    IID_IAMStreamConfig, (void **)&pSC);

                ISpecifyPropertyPages *pSpec;
                CAUUID cauuid;

                hr = pSC->QueryInterface(IID_ISpecifyPropertyPages,
                    (void **)&pSpec);
                if(hr == S_OK)
                {
                    hr = pSpec->GetPages(&cauuid);

                    hr = OleCreatePropertyFrame(ghwndApp, 30, 30, NULL, 1,
                        (IUnknown **)&pSC, cauuid.cElems,
                        (GUID *)cauuid.pElems, 0, 0, NULL);

                    CoTaskMemFree(cauuid.pElems);
                    pSpec->Release();
                }
                pSC->Release();

                if(gcap.fWantPreview)
                    StartPreview();
            }
            else if(id == gcap.iACrossbarDialogPos)
            {
                IAMCrossbar *pX, *pX2;
                IBaseFilter *pXF;
                // we could use better error checking here... I'm assuming
                // this won't fail
         //       hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
         //           &MEDIATYPE_Interleaved, gcap.pVCap,
         //           IID_IAMCrossbar, (void **)&pX);
         //       if(hr != NOERROR)
                    hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
                        &MEDIATYPE_Video, gcap.pVCap,
                        IID_IAMCrossbar, (void **)&pX);

                hr = pX->QueryInterface(IID_IBaseFilter, (void **)&pXF);

                hr = gcap.pBuilder->FindInterface(&LOOK_UPSTREAM_ONLY, NULL,
                    pXF, IID_IAMCrossbar, (void **)&pX2);

                ISpecifyPropertyPages *pSpec;
                CAUUID cauuid;

                hr = pX2->QueryInterface(IID_ISpecifyPropertyPages,
                    (void **)&pSpec);
                if(hr == S_OK)
                {
                    hr = pSpec->GetPages(&cauuid);

                    hr = OleCreatePropertyFrame(ghwndApp, 30, 30, NULL, 1,
                        (IUnknown **)&pX2, cauuid.cElems,
                        (GUID *)cauuid.pElems, 0, 0, NULL);

                    CoTaskMemFree(cauuid.pElems);
                    pSpec->Release();
                }
                pX2->Release();
                pXF->Release();
                pX->Release();
            }
            else if(id == gcap.iTVAudioDialogPos)
            {
                IAMTVAudio *pTVA;
                hr = gcap.pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE,
                    &MEDIATYPE_Audio, gcap.pACap,
                    IID_IAMTVAudio, (void **)&pTVA);

                ISpecifyPropertyPages *pSpec;
                CAUUID cauuid;

                hr = pTVA->QueryInterface(IID_ISpecifyPropertyPages,
                    (void **)&pSpec);
                if(hr == S_OK)
                {
                    hr = pSpec->GetPages(&cauuid);

                    hr = OleCreatePropertyFrame(ghwndApp, 30, 30, NULL, 1,
                        (IUnknown **)&pTVA, cauuid.cElems,
                        (GUID *)cauuid.pElems, 0, 0, NULL);

                    CoTaskMemFree(cauuid.pElems);
                    pSpec->Release();
                }
                pTVA->Release();
            }
            else if(((id ) >  gcap.iVideoInputMenuPos) &&
                (id ) <= gcap.iVideoInputMenuPos + gcap.NumberOfVideoInputs)
            {
                // Remove existing checks
             //   for(int j = 0; j < gcap.NumberOfVideoInputs; j++)
              // {
               //     CheckMenuItem(gcap.hMenuPopup, j, MF_BYPOSITION |
               //         ((j == (id - MENU_DIALOG0) - gcap.iVideoInputMenuPos - 1) ?
               //         MF_CHECKED : MF_UNCHECKED ));
              //  }

                if(gcap.pCrossbar)
                {
                    hr = gcap.pCrossbar->SetInputIndex((id) - gcap.iVideoInputMenuPos - 1);
					ASSERT(hr == S_OK);
                }
            }

	return 0;
    }
/*----------------------------------------------------------------------------*\
|   ErrMsg - Opens a Message box with a error message in it.  The user can     |
|            select the OK button to continue                                  |
\*----------------------------------------------------------------------------*/
void ErrMsg(LPTSTR szFormat,...)
{
    static TCHAR szBuffer[2048]={0};
    const size_t NUMCHARS = sizeof(szBuffer) / sizeof(szBuffer[0]);
    const int LASTCHAR = NUMCHARS - 1;

    // Format the input string
    va_list pArgs;
    va_start(pArgs, szFormat);

    // Use a bounded buffer size to prevent buffer overruns.  Limit count to
    // character size minus one to allow for a NULL terminating character.
    HRESULT hr = StringCchVPrintf(szBuffer, NUMCHARS - 1, szFormat, pArgs);
    va_end(pArgs);

    // Ensure that the formatted string is NULL-terminated
    szBuffer[LASTCHAR] = TEXT('\0');

    MessageBox(ghwndApp, szBuffer, NULL,
               MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL);
}

BOOL MySaveCaptureFile(LPCSTR dst, LPCSTR src)
{
	HRESULT hr;
	wchar_t tmpwide[400];
	memcpy(tmpwide, MyWide(dst), 780);
	hr = gcap.pBuilder->CopyCaptureFile(MyWide(src),tmpwide,0,0);
	return (hr == NOERROR ? TRUE : FALSE);
}


DWORDLONG GetSize(LPCTSTR tach)
{
    HANDLE hFile = CreateFile(tach, GENERIC_READ, FILE_SHARE_READ, 0,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if(hFile == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    DWORDLONG dwlSize = 0;
    DWORD dwSizeHigh = 0;

    DWORD dwSizeLow = GetFileSize(hFile, &dwSizeHigh);
    if (INVALID_FILE_SIZE != dwSizeLow)
    {
        dwlSize = dwSizeLow + ((DWORDLONG)dwSizeHigh << 32);
    }

    if(!CloseHandle(hFile))
    {
        dwlSize = 0;
    }

    return dwlSize;
}

void OnClose()
{
 //   TCHAR szBuf[512];
    WCHAR *wszDisplayName = NULL;

    // Unregister device notifications
//   if(ghDevNotify != NULL)
//    {
//        ASSERT(gpUnregisterDeviceNotification);
//        gpUnregisterDeviceNotification(ghDevNotify);
//        ghDevNotify = NULL;
//    }

    // Destroy the filter graph and cleanup
    StopPreview();
    StopCapture();
    TearDownGraph();
    FreeCapFilters();
}

void MyGUIDtoString(LPSTR txt, GUID & guid)
{
	int c = 2 * sizeof(guid);
	int i;
	BYTE * p = (BYTE *)&guid;
	for (i = 0; i < c; i++)
		{
		UINT v = *p;
		if (i & 1)
			v = v & 15,p++;
		else
			v = v / 16;
		if (v > 9)
			txt[i] ='A' + v - 10;
		else
			txt[i] ='0' + v;
		}
	txt[i] = 0;
}

void MyGUIDfromString(LPCSTR txt, GUID & guid)
{
	int c = 2 * sizeof(guid);
	int i;
	BYTE * p = (BYTE *)&guid;
	UINT q;
	for (i = 0; i < c; i++)
		{
		UINT v = txt[i];
	ASSERT(v);
		if (v >= 'A')
			v = v + 10 - 'A';
		else
			v -= '0';
		if (i & 1)
			{
			q = q + v;
			*p++ = q;
			}
		else
			q = v * 16;
		}
}

int RCapInit(HWND hwnd, LPCSTR VidName, LPCSTR AudName, LPCSTR CmpName,
				LPCSTR CapName, BOOL bAudio, BOOL bPreview, UINT size)
{
	memset(&gcap,0, sizeof(gcap));
	ghwndApp = hwnd;
    gcap.pmVideo = 0;
    gcap.pmAudio = 0;
    gcap.fMPEG2  = 0;//TRUE;
    gcap.fCapAudio = bAudio;
    gcap.fCapCC    = 0;
    gcap.fWantPreview = bPreview;
//	if (szGUID[0])
//	MyGUIDfromString(szGUID, gcap.myfmt);
    gcap.wCapFileSize = size;//pOptions->nCapSize;
	SetupCaptureFile(CapName);
    AddDevicesToList();


// which stream should be the master? NONE(-1) means nothing special happens
// AUDIO(1) means the video frame rate is changed before written out to keep
// the movie in sync when the audio and video capture crystals are not the
// same (and therefore drift out of sync after a few minutes).  VIDEO(0)
// means the audio sample rate is changed before written out//
//	if (!gcap.fCapAudio || !gcap.fCapAudioIsRelevant)
//		gcap.iiMasterStream = 0;
//	else
//		gcap.iiMasterStream = 1;
//	gcap.iiMasterStream = -1;

    // Instantiate the capture filters we need to do the menu items.
    // This will start previewing, if desired
    //
    // Make these the official devices we're using

	int vid = 0;
	int aid = 0;
	int cid = 0;
	int i;
    for (i = 0; i < gcap.iNumVCapDevices;i++)
		{
		if (!strcmp(MySub(gcap.VidNames[i]), VidName))
			vid = i;
		}
    for (i = 0; i < gcap.iNumACapDevices;i++)
		{
		if (!strcmp(MySub(gcap.AudNames[i]), AudName))
			aid = i;
		}
	if (!strcmp(RAW, CmpName))
		cid = 0;
    else for (i = 0; i < gcap.iNumCompressors;i++)
		{
		if (!strcmp(MySub(gcap.CmpNames[i]), CmpName))
			cid = i+1;
		}
//	cid = 6;
    ChooseDevices(vid,aid,cid);
	return 0;
}

int RCapCopy(LPCSTR dst)
{
	return MySaveCaptureFile(dst,gcap.szCaptureFile);
}

int RCapDeviceCount(int which)
{
	if (which > 1)
    	which = gcap.iNumCompressors + 1; // for raw
	else if (which == 1)
    	which = gcap.iNumACapDevices;
	else
		which = gcap.iNumVCapDevices;
	return which;
}

int RCapGetName(char * ioDeviceName, int inDeviceIndex, int which)
{
	int nName = 80;
	if (which > 1)
		{
		if (!inDeviceIndex)
			strcpy_s(ioDeviceName,nName,RAW);
		else
			strcpy_s(ioDeviceName, nName,MySub(gcap.CmpNames[inDeviceIndex-1]));
		}
	else if (which == 1)
		strcpy_s(ioDeviceName, nName, MySub(gcap.AudNames[inDeviceIndex]));
	else
		strcpy_s(ioDeviceName, nName,MySub(gcap.VidNames[inDeviceIndex]));
	return 0;
}

int RCapCurrentDevice(int which)
{
	if (which > 1)
		which = gcap.iCompressor;
	else if (which == 1)
		which = gcap.iAudDevice;
	else
		which = gcap.iVidDevice;
	return which;
}

int RCapSelectDevice(int inDeviceIndex, int which)
{
	if (which > 1)
		ChooseDevices(gcap.iVidDevice, gcap.iAudDevice, inDeviceIndex);
	else if (which == 1)
		ChooseDevices(gcap.iVidDevice, inDeviceIndex, gcap.iCompressor);
	else
		ChooseDevices(inDeviceIndex, gcap.iAudDevice, gcap.iCompressor);
	return 0;
}

int RCapSize(int & w, int & h)
{
	w = gcap.capw;//640;
	h = gcap.caph;//480;
	return 0;
}

int RCapGetGUID(LPSTR szGUID)
{
	MyGUIDtoString(szGUID, gcap.myfmt);
//RPC_STATUS hr = UuidToString(&gcap.myfmt, (RPC_CSTR *)szGUID);
	return 0;
}

int RCapStop(BOOL bDestroy)
{
	OnClose();
	return 0;
}

int RCapOption(int index, int which, char * OptionName)
{
	if (!which)
		{
		if (index >= gcap.iNumVCapDialogs)
			return -1;
		index = gcap.VDialogs[index];
		
		if (!OptionName)
			{
			DoDialog(index);
			}
		else
			{
			strcpy_s(OptionName, 80,gcap.szDialogs[index]);
			}
		}
	else
		{
		if (index >= gcap.iNumACapDialogs)
			return -1;
		index = gcap.ADialogs[index];
		
		if (!OptionName)
			{
			DoDialog(index);
			}
		else
			{
			strcpy_s(OptionName, 80,gcap.szDialogs[index]);
			}
		}
	return 0;
}
int RCapPause()
{
	if(gcap.fPreviewing)
		StopPreview();
	if(gcap.fPreviewGraphBuilt)
		TearDownGraph();
	return 0;
}
   
int RCapPreview(int v)
{
	int res;
	if (v)
		res = StartPreview();
	else
		res = StopPreview();
	return res;
}

int RCapDoAudio(int v)
{
    gcap.fCapAudio = v & 1;
	return 0;
}

int RCapCapSize(int v)
{
    gcap.wCapFileSize = v;
	return 0;
}

int RCapStartRecord()
{
	RCapPause();
	BuildCaptureGraph();
	StartCapture();
	return 0;
}

int RCapResume()
{
	if(gcap.fWantPreview)
		{
		BuildPreviewGraph();
		StartPreview();
		}
	return 0;
}

int RCapStopRecord()
{
	StopCapture();

//	SaveCaptureFile(ghwndApp);

	RCapResume();

	return 0;
}

BOOL RCapStatus(LPSTR szStatus, int size, BOOL bFull)
{
	if (!bFull)
	UpdateStatus(bFull);
	strcpy_s(szStatus, size, gcap.zstatus);
	return 1;
	BOOL bRes; 
	if (bRes = gcap.bStatus)
		{
		strcpy_s(szStatus, size, gcap.zstatus);
		gcap.bStatus = 0;
		}
	return bRes;
}


int zinfo()
{
	return 0;
	HRESULT hr;
	IEnumPins *enumpin; 
    IPin        *pPin = 0;

    //find pin that suppor IAMVideoCompression 
    gcap.pVCmp->EnumPins(&enumpin); 
	IAMVideoCompression *pVC; 
	hr = -1;
    while (S_OK == enumpin->Next(1, &pPin, NULL)) 
		{ 
		hr = pPin->QueryInterface(IID_IAMVideoCompression, (void**)&pVC); 
		pPin->Release(); 
		if (SUCCEEDED(hr)) // Found the interface. 
			{ 
            break; 
			} 
		} 
	if(SUCCEEDED(hr)) 
		{ 
DPF("got pvc");
//ASSERT(0);
		long lCap; 
        //Real Values 
        long lKeyFrame, lPFrame; 
        double Quality; 
        //Default values 
        long lKeyFrameDef, lPFrameDef;  
        double QualityDef;

        int cbVersion, cbDesc; // Size in bytes, not characters! 
        //setting up 
        hr = pVC->GetInfo(0L, &cbVersion, 0, &cbDesc,0,0,0,0); 
        if (SUCCEEDED(hr)) 
        	{ 
            WCHAR *pszVersion = new WCHAR[cbVersion / sizeof(WCHAR)];
            WCHAR *pszDesc = new WCHAR[cbDesc / sizeof(WCHAR)]; 
            hr = pVC->GetInfo(pszVersion,&cbVersion,pszDesc,&cbDesc,0,0,0,0);
			delete [] pszVersion;
			delete [] pszDesc;
        	} 
        else
			return 6; 

        // Get default values and capabilities. 
        hr = pVC->GetInfo(0, 0, 0, 0, &lKeyFrameDef, &lPFrameDef, &QualityDef,
					&lCap); 
		
DPF("defs key frm:%d, frame:%d",lKeyFrameDef, lPFrameDef);
DPF("def quality:%d", (int)(1000.0 * QualityDef));
DPF("caps:%x",lCap);
		if (SUCCEEDED(hr)) 
	        { 
            // Get actual values where possible. 
            if (lCap & CompressionCaps_CanKeyFrame) 
            	{ 
                hr = pVC->get_KeyFrameRate(&lKeyFrame); 
                if (FAILED(hr) || lKeyFrame < 0) 
                    lKeyFrame = lKeyFrameDef; 
DPF("actual key frm:%d",lKeyFrame);
            	} 
            if (lCap & CompressionCaps_CanBFrame) 
            	{ 
                hr = pVC->get_PFramesPerKeyFrame(&lPFrame); 
                if (FAILED(hr) || lPFrame < 0) 
                    lPFrame = lPFrameDef; 
DPF("actual frame def:%d",lPFrame);
            	} 
            if (lCap & CompressionCaps_CanQuality) 
            	{ 
                hr = pVC->get_Quality(&Quality); 
                if (FAILED(hr) || Quality < 0) 
                    Quality = QualityDef; 
DPF("actual quality:%d",(int)(1000.0 * Quality));
            	} 
        	} 
        else return 5; 
    } 
	else
			return 1; 

    return 0; 
}

int EnumVideo( ICreateDevEnum *pCreateDevEnum)
{
    UINT    uIndex = 0;
	gcap.iNumVCapDevices = uIndex;
    HRESULT hr;
    IEnumMoniker *pEm = 0;
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,
					&pEm, 0);
    if(hr != NOERROR)
		return 1;
	pEm->Reset();
    ULONG cFetched;
    IMoniker *pM;
    while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK)
    	{
        IPropertyBag *pBag=0;

        hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
        if(SUCCEEDED(hr))
        	{
            VARIANT var;
            var.vt = VT_BSTR;
            hr = pBag->Read(L"FriendlyName", &var, NULL);
            if(hr == NOERROR)
            	{
				StringCchCopyW(gcap.VidNames[uIndex], 120, var.bstrVal);
                SysFreeString(var.bstrVal);

                ASSERT(gcap.rgpmVideoList[uIndex] == 0);
                gcap.rgpmVideoList[uIndex] = pM;
        		uIndex++;
                pM->AddRef();
            	}
            pBag->Release();
        	}
        pM->Release();
		if (uIndex >= NUMELMS(gcap.rgpmVideoList))
			break;
 	   }
    pEm->Release();

    gcap.iNumVCapDevices = uIndex;
	return 0;
}

int EnumCompress( ICreateDevEnum *pCreateDevEnum)
{
    UINT    uIndex = 0;
    gcap.iNumCompressors = uIndex;
    HRESULT hr;
	IEnumMoniker *pEm = 0;
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoCompressorCategory,
					&pEm, 0);
    if(hr != NOERROR)
		return 1;
    uIndex = 0;
    pEm->Reset();
	ULONG cFetched;
    IMoniker *pM;
    while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK)
    	{
        IPropertyBag *pBag=0;

        hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
        if(SUCCEEDED(hr))
        	{
            VARIANT var;
            var.vt = VT_BSTR;
            hr = pBag->Read(L"FriendlyName", &var, NULL);
            if(hr == NOERROR)
            	{
         StringCchCopyW(gcap.CmpNames[uIndex], 120, var.bstrVal);
                SysFreeString(var.bstrVal);
DPF("compressor %d:%s",uIndex,MySub(gcap.CmpNames[uIndex]));
                ASSERT(gcap.rgpmCompressList[uIndex] == 0);
                gcap.rgpmCompressList[uIndex] = pM;
        		uIndex++;
                pM->AddRef();
            	}
            pBag->Release();
        	}
        pM->Release();
		if (uIndex >= NUMELMS(gcap.rgpmCompressList))
			break;
 	   }
    pEm->Release();
    gcap.iNumCompressors = uIndex;
	return 0;
}

int EnumAudio( ICreateDevEnum *pCreateDevEnum)
{
    UINT    uIndex = 0;
    gcap.iNumACapDevices = uIndex;
    HRESULT hr;
    IEnumMoniker *pEm = 0;
    hr = pCreateDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory,
									&pEm, 0);
    if(hr != NOERROR)
        return 1;
    pEm->Reset();
	ULONG cFetched;
    IMoniker *pM;

    while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK)
    	{
        IPropertyBag *pBag;
        hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
        if(SUCCEEDED(hr))
        	{
            VARIANT var;
            var.vt = VT_BSTR;
            hr = pBag->Read(L"FriendlyName", &var, NULL);
            if(hr == NOERROR)
            	{
         StringCchCopyW(gcap.AudNames[uIndex], 120, var.bstrVal);
                SysFreeString(var.bstrVal);
                ASSERT(gcap.rgpmAudioList[uIndex] == 0);
                gcap.rgpmAudioList[uIndex] = pM;
                pM->AddRef();
        		uIndex++;
            	}
            pBag->Release();
        	}
        pM->Release();
		if (uIndex >= NUMELMS(gcap.rgpmAudioList))
			break;
    	}
    pEm->Release();
    gcap.iNumACapDevices = uIndex;
	return 0;
}


