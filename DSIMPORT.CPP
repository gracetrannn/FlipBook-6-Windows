#include "stdafx.h"
#include "dsimport.h"
#include <dshow.h>
#include "qedit.h"
#include <streams.h>
#include <initguid.h>
#include "dvdmedia.h"
//#include "mydoc.h"
#include "dshelp.h"

struct __declspec(  uuid("{71771540-2017-11cf-ae26-0020afd79767}")  )
							CLSID_Sampler;

struct MySampler : public CBaseVideoRenderer {
    MySampler( IUnknown* unk, HRESULT *hr ) :
						CBaseVideoRenderer(__uuidof(CLSID_Sampler),
						NAME("Frame Sampler"), unk, hr) {
							m_pVClass = 0;
							m_hWnd = 0;
							m_index = 0;
							m_rate = 0;
			};
    HRESULT CheckMediaType(const CMediaType *media ) {


//		VIDEOINFOHEADER2 * vi;
//		VIDEOINFO* vi;
		void * pvi = media->Format();
		if(!IsEqualGUID( *media->Subtype(), MEDIASUBTYPE_RGB24) || !pvi)
			return E_FAIL;	
		long nTenMillion    = 10000000;
		long lAvgFrameDuration  = 0;
		if (IsEqualGUID( *media->FormatType(), FORMAT_VideoInfo2))
			{
			lAvgFrameDuration = (long)((VIDEOINFOHEADER2 *)pvi)->AvgTimePerFrame;
			bmih = ((VIDEOINFOHEADER2 *)pvi)->bmiHeader;
			}
		else if (IsEqualGUID( *media->FormatType(), FORMAT_VideoInfo))
			{
			lAvgFrameDuration = (long)((VIDEOINFO *)pvi)->AvgTimePerFrame;
			bmih = ((VIDEOINFO *)pvi)->bmiHeader;
			}
		else
			return E_FAIL;
		DPF("w:%d,h:%d",bmih.biWidth, bmih.biHeight);
		if (lAvgFrameDuration)
			m_rate = (long)( (nTenMillion + lAvgFrameDuration / 2) / lAvgFrameDuration );
DPF("rate:%d",m_rate);
		m_VCB(1,&bmih,m_pVClass);
//		SetWindowPos(hwnd,0,0,0,20+(w=bmih.biWidth),60+(h=bmih.biHeight),SWP_NOZORDER|SWP_NOMOVE);
        return  S_OK;
    }

    HRESULT DoRenderSample(IMediaSample *sample)
		{
		BYTE* data;
		sample->GetPointer( &data ); 
		if (m_front)		// if skipping, ignore front frames
			m_front--;
		else
			{
			if (m_count)	// stop after so many
				{
				m_count--;
				if (m_pVClass)
					m_VCB(0,data,m_pVClass);
				}
			}
		m_index++;
		if (!m_count)
			m_index = NEGONE;
        return  S_OK;
    	}
    HRESULT ShouldDrawSampleNow(IMediaSample *sample, REFERENCE_TIME *start,
								REFERENCE_TIME *stop) {
       return S_OK; // disable droping of frames
    }

//	HRESULT GetSampleTimes( IMediaSample *pMediaSample,
//    				REFERENCE_TIME *pStartTime, REFERENCE_TIME *pEndTime)
//		{
//		return S_OK;
//		}
// 	HRESULT WaitForRenderTime() { return S_OK;}

	void SetRange(UINT Front, UINT Count)
		{ m_front = Front; m_count = Count;};
	UINT Current() { return m_index;};

	void * m_pVClass;
	VideoCallback m_VCB;	

	HWND m_hWnd;
	int m_rate;
	UINT m_front;
	UINT m_count;
	UINT m_index;
	UINT m_zarg; // in this case level
	BITMAPINFOHEADER bmih;
};


class CSampleGrabberCB : public ISampleGrabberCB {

public:
	CSampleGrabberCB() { m_nCount = m_nTotal = m_max = m_index = 0 ;};
	UINT GetCount() { return m_nCount;};
	UINT GetTotal() { return m_nTotal;};
	void SetBuffer(BYTE * pBuf, UINT nmax)
		{
		m_index = 0;
		m_pBuf = pBuf;
		m_max = nmax;
		};
 STDMETHODIMP_(ULONG) AddRef() { return 2; }
 STDMETHODIMP_(ULONG) Release() { return 1; }
 STDMETHODIMP QueryInterface(REFIID riid, void ** ppv) {
	 DPF("samp query");
 CheckPointer(ppv,E_POINTER);
 if( riid == IID_ISampleGrabberCB || riid == IID_IUnknown ) {
		*ppv = static_cast <void *> (this);
return NOERROR;
 }
 return E_NOINTERFACE;
 }

    HRESULT CheckMediaType(const CMediaType *media ) {
		DPF("grab check format");
		if (!IsEqualGUID( *media->FormatType(), FORMAT_WaveFormatEx))
			{
			return E_FAIL;	
			}
//		m_wf = *((WAVEFORMATEX *)media->pbFormat);
        return  S_OK;
    }

	STDMETHODIMP SampleCB( double SampleTime, IMediaSample * pSample ) {
		DPF("sampler");		
		AM_MEDIA_TYPE * pmtype;
		if (pSample->GetMediaType(&pmtype) == S_OK)
			{
			m_wf = *((WAVEFORMATEX *)pmtype->pbFormat);
			}
		return 0;}

 STDMETHODIMP BufferCB( double SampleTime, BYTE * pBuffer, long BufferSize ) {
	// DPF("audio sample,%d",BufferSize);
	if ((m_index + BufferSize) > m_max)
		return 0;
	m_nCount++;
	memcpy(m_pBuf+m_index, pBuffer, BufferSize);
	m_index += BufferSize;
	m_nTotal += BufferSize;
// _tprintf(TEXT("Found a sample at %f s\t[%ld]\r\n"), SampleTime , BufferSize );
 return 0;
 }
	BOOL GetFormat(WAVEFORMATEX * pwf) {
				*pwf = m_wf;return 1;};
 UINT m_nCount;
	UINT m_nTotal;
	UINT m_index;
	BYTE * m_pBuf;
	UINT m_max;
    WAVEFORMATEX m_wf;
};


CDSImport::CDSImport()
{
	m_pSrc = 0;
	m_pSampler = 0;
	m_pGraph = 0;
	m_pCB = 0;
	m_pCtrl = 0;
//	m_pAvi = 0;
	m_bStopping = 0;
	m_bHasSound = 0;
	m_nFrames = 0;
	m_nRate = 0;
	m_nWidth = 0;
	m_nHeight = 0;
	m_pGrabber = 0;
	m_pGrabberF = 0;
	m_pNull1F = 0;
	m_pNull2F = 0;
	m_pAudio = 0;
}

CDSImport::~CDSImport()
{
	m_bStopping = 1;
	if (m_pSampler)
		m_pSampler->m_pVClass = 0;
	DPF("-2");
	DPF("-1");
	if (m_pCtrl)
		{
//		m_pCtrl->Stop();
		m_pCtrl->Release();
		}
	DPF("0");
	if (m_pGrabber)
		m_pGrabber->SetCallback(0,0);
	if (m_pCB)
	{
		DPF("count:%u,%u",m_pCB->GetCount(),m_pCB->GetTotal());
		delete m_pCB;
	//	m_pCB->Release();
	}
 	
	if (m_pSrc)
		m_pSrc->Release();
	if (m_pGrabber)
		m_pGrabber->Release();
	if (m_pGrabberF)
		m_pGrabberF->Release();
	if (m_pNull1F)
		m_pNull1F->Release();
	if (m_pNull2F)
		m_pNull2F->Release();
	if (m_pBuilder)
		m_pBuilder->Release();
	DPF("1");
	DPF("2");
	if (m_pGraph)
		m_pGraph->Release();
	DPF("3");
//	delete m_pSampler;
	delete [] m_pAudio;
	DPF("4");
}

BOOL hrcheck(HRESULT hr, LPCSTR errtext)//TCHAR* errtext)
{
    if (hr >= S_OK)
        return FALSE;
  //  TCHAR szErr[MAX_ERROR_TEXT_LEN];
	char szErr[80];
	char szMsg[180];
	DWORD res = 0;
#ifdef _DEBUG
	    res = AMGetErrorText(hr, szErr, MAX_ERROR_TEXT_LEN);
#else
		szErr[0] = 0;
#endif
 //       if (res == 0)
  //      {
			sprintf_s(szMsg,"hr err:%x,%s\n%s",hr,errtext,szErr);
     //       StringCchPrintf(szErr, MAX_ERROR_TEXT_LEN, L"Unknown Error: 0x%2x", hr);
   //     }
        MessageBox(0, szMsg, TEXT("Error!"), MB_OK | MB_ICONERROR);

 //   DWORD res = 0;//AMGetErrorText(hr, szErr, MAX_ERROR_TEXT_LEN);
  //  if (res)
    //    printf("Error %x: %s\n%s\n",hr, errtext,szErr);
 //   else
 //       printf("Error %x: %s\n", hr, errtext);
    return TRUE;
}

//change this macro to fit your style of error handling
#define CHECK_HR(hr, msg) if (hrcheck(hr, msg)) return hr;

CComPtr<IPin> GetPin(IBaseFilter *pFilter, LPCOLESTR pinname)
{
    CComPtr<IEnumPins>  pEnum;
    CComPtr<IPin>       pPin;

    HRESULT hr = pFilter->EnumPins(&pEnum);
    if (hrcheck(hr, "Can't enumerate pins."))
        return NULL;

    while(pEnum->Next(1, &pPin, 0) == S_OK)
    {
        PIN_INFO pinfo;
        pPin->QueryPinInfo(&pinfo);
        BOOL found = !_wcsicmp(pinname, pinfo.achName);
        if (pinfo.pFilter) pinfo.pFilter->Release();
        if (found)
            return pPin;
        pPin.Release();
    }
    printf("Pin not found!\n");
    return NULL;  
}


HRESULT GetPin( IBaseFilter * pFilter, PIN_DIRECTION dirrequired, int iNum,
IPin **ppPin) {
 CComPtr< IEnumPins > pEnum;
 *ppPin = NULL;

 HRESULT hr = pFilter->EnumPins(&pEnum);
 if(FAILED(hr)) return hr;

 ULONG ulFound;
 IPin *pPin;
 hr = E_FAIL;

 while (S_OK == pEnum->Next(1, &pPin, &ulFound)) {
 PIN_DIRECTION pindir = (PIN_DIRECTION)3;
 pPin->QueryDirection(&pindir);
 if (pindir == dirrequired) {
 if (iNum == 0) {
 *ppPin = pPin; // Return the pin's interface
 hr = S_OK; // Found requested pin, so clear error
 break;
 }
 iNum--;
 } 
pPin->Release();
 }

 return hr;
}


IPin * GetInPin( IBaseFilter * pFilter, int nPin ) {
	IPin * pComPin=0;
	GetPin(pFilter, PINDIR_INPUT, nPin, &pComPin);
 return pComPin;
}

IPin * GetOutPin( IBaseFilter * pFilter, int nPin ) {
 IPin * pComPin=0;
 GetPin(pFilter, PINDIR_OUTPUT, nPin, &pComPin);
 return pComPin;
}


int CDSImport::SelectFile(LPCSTR name, UINT flags) // 1 is video, 2 is audio
{
	WCHAR wname[400];
	int result = ::MultiByteToWideChar(

        CP_UTF8,                // convert from UTF-8

        MB_ERR_INVALID_CHARS,   // error on invalid chars

        name,	            // source UTF-8 string

        1 + strlen(name),                 // total length of source UTF-8 string,

                             // in CHAR's (= bytes), including end-of-string\0

        wname,               // destination buffer

        500                // size of destination buffer, in WCHAR's

        );

	DPF("10");

	HRESULT hr;
	m_nFrames = 0;
	m_nWidth = 0;
	m_nHeight = 0;
    m_pGraph= 0;
    m_pCtrl = 0;
	m_pSampler = 0;
	m_pGrabber = 0;
	m_pBuilder = 0;
	hr = CoCreateInstance( CLSID_FilterGraph, 0,
				CLSCTX_INPROC,IID_IGraphBuilder, (void **)&m_pGraph );
	CHECK_HR(hr,"no graph");
	if (!m_pGraph) return 11;
	hr = m_pGraph->QueryInterface( IID_IMediaControl, (void **)&m_pCtrl );
	CHECK_HR(hr,"no interface");
	if (!m_pCtrl) return 12;

	DPF("11");
    m_pSampler      = new MySampler(0,&hr); 

	m_pSampler->m_VCB = m_pVCB;
	m_pSampler->m_pVClass = m_pVClass;

	hr = m_pGraph->AddFilter((IBaseFilter*)m_pSampler, L"VidSampler");

    hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, 
        IID_PPV_ARGS(&m_pNull1F));
    if (FAILED(hr))
    {
        return 56;
    }

    hr = m_pGraph->AddFilter(m_pNull1F, L"NULL Renderer 1");
    if (FAILED(hr))
    {
        return 57;
    }

    hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, 
        IID_PPV_ARGS(&m_pNull2F));
    if (FAILED(hr))
    {
        return 58;
    }

    hr = m_pGraph->AddFilter(m_pNull2F, L"NULL Renderer 2");
    if (FAILED(hr))
    {
        return 59;
    }

	SetupAudio();
	m_pSrc = 0;
	if (TryAVI(wname))
		return 13;

	hr = CoCreateInstance (CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER,
                       IID_ICaptureGraphBuilder2,  (void **)&m_pBuilder);
	CHECK_HR(hr,"builder2");
	hr = m_pBuilder->SetFiltergraph (m_pGraph);

	
	GUID majorType;
//	GUID           majorType;
	majorType = MEDIATYPE_Video;
	hr = m_pBuilder->RenderStream (NULL, NULL, m_pSrc, NULL, m_pSampler);
	
	if (hr != S_OK)
		{
		IPin * pVidOut = GetOutPin(m_pSrc,0);
	
		if (!pVidOut)
			return 2;
  		hr = m_pGraph->Render( pVidOut );
		}
	


	CHECK_HR(hr,"render v1");
	IPin * pGrabIn = 0;
	
	majorType = MEDIATYPE_Audio;
//	majorType = MFMediaType_Audio;
	hr = m_pBuilder->RenderStream (NULL, NULL, m_pSrc, NULL, m_pGrabberF);
	if (hr == S_OK)
		{
//	CHECK_HR(hr,"render a1");
		hr = m_pBuilder->RenderStream (NULL, &majorType, m_pGrabber, NULL,m_pNull2F);
	//	CHECK_HR(hr,"render a2");
		if (hr == S_OK)
		pGrabIn = GetInPin(m_pGrabberF,0);
		}
		if (pGrabIn)
		{
		m_bHasSound = TRUE;
		CMediaType mt;
		hr = pGrabIn->ConnectionMediaType(&mt);
		CHECK_HR(hr,"grab pin");
		m_wf = *((WAVEFORMATEX *)mt.pbFormat);
		pGrabIn->Release();
		}





	CComQIPtr<IMediaSeeking> pSeek = m_pCtrl;
	if (pSeek == NULL)
		return 7;// E_NOINTERFACE;
	LONGLONG Duration = 0;
	hr = pSeek->GetDuration(&Duration);
//	if (!Duration) return 3;
	DPF("14");

	if (m_bHasSound)
		{
		UINT s = (UINT)(Duration / 10000000);
		s++;
		s *= m_wf.nAvgBytesPerSec;
		s = 120 * s / 100; 
		m_pAudio = new BYTE[s];
		m_pCB->SetBuffer(m_pAudio,s); 
		}
	m_nRate = m_pSampler->m_rate;
//	if (!m_nRate) return 4;

	m_nFrames = (int)(Duration * m_nRate / 10000000);
	m_nWidth = m_pSampler->bmih.biWidth;
	m_nHeight = m_pSampler->bmih.biHeight;
	m_pSampler->SetRange(0,m_nFrames);
	IMediaFilter *pMediaFilter = 0;
	hr = m_pGraph->QueryInterface(IID_IMediaFilter, (void**)&pMediaFilter);
	hr = pMediaFilter->SetSyncSource(NULL);
	pMediaFilter->Release();
	
 	//hr = m_pBuilder->SetSyncSource(NULL);
	CHECK_HR(hr, "setclock");
	return 0;
}

void CDSImport::SetCallback( VideoCallback pVCB, void * pVClass)
{
	m_pVCB = pVCB;
	m_pVClass = pVClass;
}

BOOL CDSImport::TryWMV()
{
	return 1;
}
#define OLDY
BOOL CDSImport::TryAVI(WCHAR * wname)
{
	HRESULT hr;
//	hr = m_pGraph->RenderFile(wname,0);
//	CHECK_HR(hr,"render file");
//	if (hr != S_OK)
//		return 113;
//	else
//		return 0;
#ifdef OLDY
//	hr = m_pGraph->AddSourceFilter (wname, L"File Source", &m_pSrc);
	hr = m_pGraph->AddSourceFilter (wname, L"WM ASF Reader", &m_pSrc);
	if (!m_pSrc)
		return 1;
	DPF("111");
#else
    hr = CoCreateInstance(CLSID_AsyncReader, NULL, CLSCTX_INPROC_SERVER,
        				IID_PPV_ARGS(&m_pSrc));
    CHECK_HR(hr, "Can't create File Source (Async.)");
	hr = m_pGraph->AddFilter(m_pSrc, L"File Source (Async.)");
    CHECK_HR(hr, "Can't add File Source (Async.) to graph");

    CComQIPtr<IFileSourceFilter, &IID_IFileSourceFilter>
						pFileSourceAsync_src(m_pSrc);
    if (!pFileSourceAsync_src)
        CHECK_HR(E_NOINTERFACE, "Can't get IFileSourceFilter");
    hr = pFileSourceAsync_src->Load(wname, NULL);
    CHECK_HR(hr, "Can't load file");
#endif
	IPin * pVidOut = 0;

//	pVidOut = m_pSrc->FindPin(L"Output", &pVidOut);
	pVidOut = GetOutPin(m_pSrc,0);
	
	if (!pVidOut)
		return 2;


//	SetupAudio();

	//CBasePin * pBPin;

//	CBaseFilter * pFilt = pBPin->m_pFilter;

//    hr = FindUnconnectedPin(m_pSplit, PINDIR_OUTPUT, &pVidOut);


	DPF("12");
	DPF("13, hr:%u",hr);
//	SetupAudio();
//	IPin * pWavOut = GetPin(m_pWavDest, L"In");
//	IPin * pFileIn = GetPin(m_pSrc,L"Output");
//	hr = m_pGraph->Connect(pWavOut,pVidOut);
//	CHECK_HR(hr, "no connect");
	DPF("14, hr:%u",hr);

  //  hr = m_pGraph->Render( pVidOut );
	//CHECK_HR(hr,"render");
	return 0;
}


BOOL CDSImport::SetupAudio()
{
	HRESULT hr;
 //  BYTE *pBuffer = NULL;

    // Create the Sample Grabber filter.
    hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&m_pGrabber));
    if (FAILED(hr))
    {
        return 51;
    }

   
	m_pGrabberF = 0;
//	hr = m_pGrabber->QueryInterface(IID_ISampleGrabber, (void **)&m_pGrabberF );
	hr = m_pGrabber->QueryInterface(IID_IBaseFilter, (void **)&m_pGrabberF);
    if (FAILED(hr))
    {
        return 53;
	}
    AM_MEDIA_TYPE mt;
    ZeroMemory(&mt, sizeof(mt));
    mt.majortype = MEDIATYPE_Audio;
    mt.subtype = MEDIASUBTYPE_PCM;
    mt.formattype = FORMAT_WaveFormatEx;

    hr = m_pGrabber->SetMediaType(&mt);
    if (FAILED(hr))
    {
        return 54;
    }
   
	hr = m_pGraph->AddFilter(m_pGrabberF, L"Grabber");
    if (FAILED(hr))
    {
        return 52;
    }

    hr = m_pGrabber->SetOneShot(FALSE);
    if (FAILED(hr))
    {
        return 59;
    }

    hr = m_pGrabber->SetBufferSamples(FALSE);
    if (FAILED(hr))
    {
	return 60;
    }
	m_pCB = new CSampleGrabberCB;
	if (!m_pCB)
		return 77;
 	hr = m_pGrabber->SetCallback(m_pCB, 1);
	CHECK_HR(hr,"setcb");
 //	CComQIPtr pFilter(m_pGraph);
 //	hr = m_pGraph->SetSyncSource(NULL);
//	CHECK_HR(hr, "setclock");
	return 0;
}

void CDSImport::SetRange(UINT first, UINT last, UINT arg)
{
	CComQIPtr<IMediaSeeking> pSeek = m_pCtrl;

	if (pSeek == NULL)
		return;// 7;// E_NOINTERFACE;
	DWORD caps = 0;
	pSeek->GetCapabilities(&caps);
//	if (caps & WORD caps = AM_SEEKING_CanSeekAbsolute | AM_SEEKING_CanGetDuration;
	if (caps & AM_SEEKING_CanSeekAbsolute) // | AM_SEEKING_CanGetDuration;
		{
	#define ONE_SECOND 10000000
	REFERENCE_TIME rtNow  = first * m_nRate * ONE_SECOND, 
               	rtStop = 	(last + 1) * m_nRate * ONE_SECOND;

	HRESULT hr = pSeek->SetPositions(
    		&rtNow,  AM_SEEKING_AbsolutePositioning, 
    		&rtStop, AM_SEEKING_AbsolutePositioning
 		   );
		}
	m_pSampler->SetRange(first,last+1-first);
//	m_pSampler->SetRange(first,last);
}


int CDSImport::Function(int code)
{
	int res = 0;
	if (code == 0)
		{
		HRESULT hr = m_pCtrl->Run();
		return 0;
		}
	else if (code == 1)
		{
		HRESULT hr = m_pCtrl->Stop();
		return 0;
		}
	long ev=0, p1=0, p2=0;
	CComQIPtr<IMediaEvent, &IID_IMediaEvent> mediaEvent(m_pGraph);
	if (mediaEvent->GetEvent(&ev, &p1, &p2, 0)==S_OK)
		{
		if (ev == EC_COMPLETE)
			res = -1;
		else if ( ev == EC_USERABORT)
			{
			m_pCtrl->Stop();
			res = -2;
			}
		mediaEvent->FreeEventParams(ev, p1, p2);
		}
	if (!res && m_pSampler && m_nFrames)
		{
		res = (int)m_pSampler->Current();
		}
	return res;
}

int CDSImport::WriteWave(LPCSTR name)
{
	if (!m_pCB) return 9;
	UINT nTotal = m_pCB->GetTotal();
	DWORD buff[200];
 //   WAVEFORMATEX wf;
//	m_pCB->GetFormat(&wf);
    long l = sizeof(m_wf);
//   AVIStreamReadFormat(paudio, 0, &wf, &l);
    if (!l)
		return 0;
	DWORD offset = 0;
	UINT nHeader = 0;
//	long nBytes;
//	nBytes = dwTotal;
//	dwTotal += 4 + 4 + 4 + 4 + 4 + sizeof(WAVEFORMATEX) + 4 + 10;
	DWORD dwTotal = 0;
	DWORD * pData = (DWORD *)buff;
	*pData++ = mmioFOURCC('R', 'I', 'F', 'F');
	*pData++ = 0;//dwTotal; // fill in below
	*pData++ = mmioFOURCC('W', 'A', 'V', 'E');
	*pData++ = mmioFOURCC('f', 'm', 't', ' ');
	DWORD dwSize = sizeof(WAVEFORMATEX);
	*pData++ = dwSize;
	* ((WAVEFORMATEX *)pData) = m_wf;
	pData = (DWORD *)((BYTE *)pData + ((dwSize+1)&~1));
	*pData++ = mmioFOURCC('d', 'a', 't', 'a');
	*pData++ = nTotal;;
	nHeader = (BYTE *)pData - (BYTE *)buff;
	pData = (DWORD *)buff;
//	nHeader += 2;
	pData[1] = nHeader + nTotal - 8;
//	pData = (DWORD *)((BYTE *)pData + ((dwSize+1)&~1));

	CFile file;
	DWORD mode = CFile::modeCreate | CFile::modeWrite;
	if (!file.Open(name, mode))
		return 1;
	file.Write(buff,nHeader);
	file.Write(m_pAudio,nTotal);
	file.Close();
	return 0;
}



