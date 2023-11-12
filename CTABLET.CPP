#include "stdafx.h"
#ifndef FLIPBOOK_MAC // Mac implementation is in CFC_Events.h.
#include "ctablet.h"
#include "wtutils.h"
#include <msinkaut.h>
#include "TpcConpt.h"
#include <msinkaut_i.c>

BOOL CTablet::HaveWinTab()
{
	if ( !LoadWintab( ) )
	{
	//	ShowError( "Wintab not available" );
		return FALSE;
	}

	/* check if WinTab available. */
	if (!gpWTInfoA(0, 0, NULL)) {
//		::MessageBox(NULL, "WinTab Services Not Available.", gpszProgramName,
//					MB_OK | MB_ICONHAND);
		UnloadWintab();
		return FALSE;
	}
	return TRUE;
}

void CTablet::Unload()
{

	UnloadWintab();
}

class CMyInkEvents : public InkCollectorEvents
{
public:
    CMyInkEvents();
    HRESULT Init( HWND hWnd, CTablet * pTablet);

    // Event: Stroke
    virtual void Stroke( IInkCursor* Cursor, IInkStrokeDisp* Stroke,
        						VARIANT_BOOL *Cancel);
    // Event: Packet
    virtual void NewPackets( IInkCursor* Cursor, IInkStrokeDisp* Stroke,
    							long count, VARIANT * pData);

	virtual void CursorDown( IInkCursor* Cursor, IInkStrokeDisp* Stroke);

	virtual void CursorInRange(IInkCursor* Cursor,VARIANT_BOOL NewCursor,
							VARIANT ButtonsState);
private:
	void SetupCursor(IInkCursor * Cursor, UINT check);
	void CheckTablet(UINT check, IInkCursor * pCursor, IInkTablet * pTablet);
    HWND m_hWnd;
	CTablet * m_pTablet;
	UINT m_prange;
	UINT m_flags;
	long m_id;
};

/////////////////////////////////////////////////////////
//                                          
// class CMyInkCollector
// 
// The CMyInkCollector class handles the connection between
// the window and the Ink Collector. It sets up the connection
// at start up and tears it down at exit.
//
/////////////////////////////////////////////////////////
class CMyInkCollector
{
public:
    CMyInkCollector();
    ~CMyInkCollector();
    HRESULT Init( HWND hWnd, CTablet * pTablet);
private:
    IInkCollector *m_pInkCollector;
    CMyInkEvents m_InkEvents;
};


#define DOING_POINTS

#ifdef DOING_POINTS
	#define PACKETDATA	(PK_X|PK_Y|PK_CURSOR | PK_BUTTONS | PK_NORMAL_PRESSURE)
#else
	#define PACKETDATA	(PK_CURSOR | PK_BUTTONS | PK_NORMAL_PRESSURE)
#endif

//#define PACKETDATA	(PK_X | PK_Y | PK_BUTTONS | PK_NORMAL_PRESSURE)
//#define PACKETDATA	(PK_CURSOR | PK_BUTTONS | PK_NORMAL_PRESSURE)
#define PACKETMODE	0
#include "pktdef.h"

CTablet::CTablet()
{
	m_pmic = 0;
	wActiveCsr = 0;
	m_cursor = 0;
	m_pressure = 0;
	wOldCsr = (UINT)-1;
	m_pWTMutex = new CMutex( TRUE, NULL, NULL );
	hCtx = 0;
	m_bFirst = 0;
	m_bDown = 0;
	m_icount = m_ocount = 0;
}

CTablet::~CTablet()
{
	delete m_pWTMutex;
	if( hCtx )
		gpWTClose( hCtx );
	Unload();
	if (m_pmic)
		m_pmic->Init(0,0); // unlink window
	delete m_pmic;
	m_pmic = NULL;
}

UINT CTablet::Open(HWND hWnd, int res, BOOL bPrimary, BOOL bWinTab)
{
	m_hWnd = hWnd;
	m_factor = res;
	if (hCtx)
		{
		gpWTClose(hCtx);
		hCtx = 0;
		}
	if (bWinTab)
		{
		LOGCONTEXT lc;
	gpWTInfoA( WTI_DEFCONTEXT, 0, &lc );
//	WTInfo( WTI_DEFSYSCTX,  0, &lc );
	DPF("tablet dev:%d",lc.lcDevice);
	DPF("x:%d,y:%d",lc.lcOutExtX,lc.lcOutExtY);
//	m_ExtX = lc.lcOutExtX;
//	m_ExtY = lc.lcOutExtY;
	DPF("sysmode:%d",lc.lcSysMode);
	DPF("rate:%d",lc.lcPktRate);
	DPF("xsens:%d",lc.lcSensX);
	DPF("xsyssens:%d",lc.lcSysSensX);
//	WTInfo(WTI_DEVICES,DVC_X,&TabletX);
//	WTInfo(WTI_DEVICES,DVC_Y,&TabletY);
//	DPF("axis,x:%d,y:%d",TabletX.axMax,TabletY.axMax);
//	lc.lcInOrgX = 0;
//	lc.lcInOrgY = 0;
//	lc.lcInExtX = TabletX.axMax;
//	lc.lcInExtY = TabletY.axMax;
	// Open the context
//	lc.lcSysMode = 0;
	lc.lcPktData = PACKETDATA;
	lc.lcPktMode = 0;//PACKETMODE;
//	lc.lcOptions = CXO_MESSAGES | CXO_CSRMESSAGES;
	lc.lcOptions = CXO_SYSTEM | CXO_MESSAGES | CXO_CSRMESSAGES;
//	lc.lcOptions = CXO_MESSAGES | CXO_CSRMESSAGES;
	lc.lcMoveMask = PACKETDATA;
	lc.lcOutOrgX = 0;
	lc.lcOutOrgY = 0;
	/* the negative sign here reverses the sense of the Y axis. */
	/* WinTab uses a lower-left origin; MM_TEXT uses upper left. */
	if (bPrimary)
		{
		lc.lcOutExtX = res * GetSystemMetrics(SM_CXSCREEN);
 		lc.lcOutExtY = -res * GetSystemMetrics(SM_CYSCREEN);
		lc.lcSysExtX = GetSystemMetrics(SM_CXSCREEN);
		lc.lcSysExtY = GetSystemMetrics(SM_CYSCREEN);
		}
	else
		{
		lc.lcOutExtX = res * GetSystemMetrics(SM_CXVIRTUALSCREEN);
 		lc.lcOutExtY = -res * GetSystemMetrics(SM_CYVIRTUALSCREEN);
		}
	hCtx = gpWTOpenA( hWnd, &lc, TRUE );
	if (hCtx) // && ((int)lc.lcDevice >= 0)) 
		{
//		if (m_pmic)
//			m_pmic->Init(0,0); // unlink window
		delete m_pmic;
		m_pmic = NULL;
		return 1;
		}
	}
	if (m_pmic)
		return 2;
	m_pmic = new CMyInkCollector();
	    
	if (m_pmic != NULL)
		{
		DPF("have pmic");
// 	Real initialization. This consists of creating
	     //  an ink collector object and attaching it to
	        //  the current window. 
		if (SUCCEEDED(m_pmic->Init(m_hWnd,this)))
			{
			HDC hDC = GetDC(m_hWnd);
			m_xdpi =  GetDeviceCaps(hDC, LOGPIXELSX);
			m_ydpi =  GetDeviceCaps(hDC, LOGPIXELSY);
			ReleaseDC(m_hWnd, hDC);
	           return 2;//0;
	        }
	 // Failure free resources.
		 delete m_pmic;
		 m_pmic = NULL;
		}
	return 0;
}

void CTablet::PrsInit(void)
{
	/* browse WinTab's many info items to discover pressure handling. */
	AXIS np;
	LOGCONTEXT lc;
	BYTE logBtns[32];
	UINT btnMarks[2];
	UINT size;

	/* discover the LOGICAL button generated by the pressure channel. */
	/* get the PHYSICAL button from the cursor category and run it */
	/* through that cursor's button map (usually the identity map). */
	wPrsBtn = (BYTE)-1;
	gpWTInfoA(WTI_CURSORS + wActiveCsr, CSR_NPBUTTON, &wPrsBtn);
	size = gpWTInfoA(WTI_CURSORS + wActiveCsr, CSR_BUTTONMAP, &logBtns);
	if ((UINT)wPrsBtn < size)
		wPrsBtn = logBtns[wPrsBtn];

	/* get the current context for its device variable. */
	gpWTGetA(hCtx, &lc);

	/* get the size of the pressure axis. */
	gpWTInfoA(WTI_DEVICES + lc.lcDevice, DVC_NPRESSURE, &np);
	prsNoBtnOrg = (UINT)np.axMin;
	prsNoBtnExt = (UINT)np.axMax - (UINT)np.axMin;

	/* get the button marks (up & down generation thresholds) */
	/* and calculate what pressure range we get when pressure-button is down. */
	btnMarks[1] = 0; /* default if info item not present. */
	gpWTInfoA(WTI_CURSORS + wActiveCsr, CSR_NPBTNMARKS, btnMarks);
	prsYesBtnOrg = btnMarks[1];
	prsYesBtnExt = (UINT)np.axMax - btnMarks[1];
}
/* -------------------------------------------------------------------------- */
UINT CTablet::PrsAdjust(void * pp)
{
	UINT wResult = 0;
	PACKET * p = (PACKET *) pp;

//	wActiveCsr = p->pkCursor;
	if (wActiveCsr != wOldCsr) {
DPF("tab cursor change");
		/* re-init on cursor change. */
		PrsInit();
		wOldCsr = wActiveCsr;
	}

	/* scaling output range is 0-255 */
	if (p->pkButtons & (1 << m_wPrsBtn)) {
		/* if pressure-activated button is down, */
		/* scale pressure output to compensate btn marks */
		wResult = p->pkNormalPressure - prsYesBtnOrg;
		wResult = MulDiv(wResult, 255, prsYesBtnExt);
	}
	else {
		/* if pressure-activated button is not down, */
		/* scale pressure output directly */
		wResult = p->pkNormalPressure - prsNoBtnOrg;
		wResult = MulDiv(wResult, 255, prsNoBtnExt);
	}
	if (wResult > 255)
		{
		DPF("bad result:%d",wResult);
		wResult = 255;
		}
	return wResult;
}
/*
	called by view when message from wintab is received
	we poll wintab for x,y,pressure and cursor
	add the data to our point array and pass the cursor
	and pressure back to view
*/
UINT CTablet::Packet(WPARAM wSerial, LPARAM hCtx)
{
	UINT code = 0;
	// Read the packet
	PACKET pkt;
	gpWTPacket( (HCTX)hCtx, wSerial, &pkt );

	// Process packets in order, one at a time
	CSingleLock lock( m_pWTMutex, TRUE );
	if (m_bDown && (pkt.pkButtons & 1))//(1 << m_wPrsBtn)))
		{
		UINT prsNew = PrsAdjust(&pkt);
		code = AddPoint(pkt.pkX-m_xbase,pkt.pkY-m_ybase,prsNew);
		}
	UINT curNew = pkt.pkCursor;
	if (m_act_cursor != curNew)
		{
		char txt[100];
		m_act_cursor = pkt.pkCursor;
		gpWTInfoA(WTI_CURSORS + m_act_cursor, CSR_NAME, txt);
		int i;
		UINT typ = 0;
		for (i = 0; txt[i]; i++)
			if (((txt[i+0] | 32) == 'e') &&
				((txt[i+1] | 32) == 'r') &&
				((txt[i+2] | 32) == 'a') &&
				((txt[i+3] | 32) == 's') &&
				((txt[i+4] | 32) == 'e'))
			{
			typ = 1;
			break;
			}
		if (typ != m_cursor)
			{
			code |= 4;
			m_cursor = typ;
			DPF("cursor:%x,%x,%s",m_act_cursor,typ,txt);
			}
		}
	return code + (m_cursor << 8) + (m_pressure << 16);
}

void CTablet::ButtonDown(BOOL bDown, UINT minpress, UINT maxpress,
							int winx, int winy)
{
	m_bDown = bDown;
	if (bDown)
		{
		m_xbase = (int)m_factor * winx;
		m_ybase = (int)m_factor * winy;
		m_minpress = minpress;
		if (maxpress)
			m_maxpress = maxpress;
		m_ocount = m_icount;
		}
}

UINT CTablet::AddPoint(int x, int y, UINT pressure)
{
	if (pressure < m_minpress)
		pressure = 0;
	else if (pressure > m_maxpress)
		pressure = 255;
	else
		{
		UINT d1 = pressure - m_minpress;
		UINT d2 = m_maxpress - m_minpress;
		if (!d2)
			return 14;
		pressure = (d1 * 255) / d2;
		}
	if ((m_ocount + MAX_BUF) <= m_icount)
		{
		DPF("toverflow");
		return 15;
	//	ASSERT(0);
		}
	else if (!m_icount ||
					(x != m_past[(m_icount - 1) % MAX_BUF].x) ||
					(y != m_past[(m_icount - 1) % MAX_BUF].y) ||
					(pressure != m_press[(m_icount - 1) % MAX_BUF]))
		{
DPZ("i:%d,x:%d,y:%d",m_icount,x,y);
		m_past[m_icount % MAX_BUF].x = x;
		m_past[m_icount % MAX_BUF].y = y;
		m_press[m_icount % MAX_BUF] = pressure;
		m_icount++;
		}
	UINT code = 0;
	if (pressure != m_pressure)
		{
		m_pressure = pressure;
		code |= 1;
		}
	return code;
}

UINT CTablet::InkPacket(int x, int y, int pressure)
{
	if (m_bDown || m_bFirst)
		{
		x = (x * m_factor * m_xdpi ) / 2539;
		y = (y * m_factor * m_ydpi ) / 2539;
		UINT code = AddPoint(x,y,pressure);
		if (code > 7)
			return 0;
		if (code || m_bFirst)
			{
			if (m_bFirst) code = 64;
			m_bFirst = 0;
			UINT x = m_past[(m_icount + MAX_BUF - 1) % MAX_BUF].x;
			UINT y = m_past[(m_icount + MAX_BUF - 1) % MAX_BUF].y;
			UINT v = (y << 16) + x;
			::PostMessage(m_hWnd, TABLET_MESSAGE, code, v);
			}
		}
	return 0;
}

/*
typedef struct FARSTRUCT tagSAFEARRAY
	{
	unsigned short cDims;       // Count of dimensions in this array.
	unsigned short fFeatures;   // Flags used by the SafeArray
                        // routines documented below.
#if defined(WIN32)
	unsigned long cbElements;   // Size of an element of the array.
                        // Does not include size of
                        // pointed-to data.
   	unsigned long cLocks;      // Number of times the array has been 
                        // locked without corresponding unlock.
#else
	unsigned short cbElements;
	unsigned short cLocks;
	unsigned long handle;      // Used on Macintosh only.
#endif
	void HUGEP* pvData;             // Pointer to the data.
	SAFEARRAYBOUND rgsabound[1];      // One bound for each dimension.
	} SAFEARRAY;
*/
void CMyInkEvents::Stroke(
        IInkCursor* Cursor,
        IInkStrokeDisp* Stroke,
        VARIANT_BOOL *Cancel)
{
DPF("stroke");
        // Demonstrate that we received the event notification.
        MessageBox(m_hWnd, "Stroke Event", "Event Received", MB_OK);
}


void CMyInkEvents::CursorDown( IInkCursor *Cursor, IInkStrokeDisp *Stroke )
{
	SetupCursor(Cursor,2);//3);
	m_pTablet->Begin();
//	::PostMessage(m_hWnd, TABLET_MESSAGE, 64, 0);
}

void CMyInkEvents::CursorInRange(IInkCursor* Cursor,
						VARIANT_BOOL NewCursor, VARIANT ButtonsState)
{
	DPF("In range");
	SetupCursor(Cursor,3);
}

void CMyInkEvents::NewPackets(
        IInkCursor* Cursor,
        IInkStrokeDisp* Stroke,
    	long count, VARIANT * pData)
{
	SetupCursor(Cursor,0);
	int * p = (int *) pData;
	p += 2;
	SAFEARRAY * pa = (SAFEARRAY *)p[0];
		
//DPF("packet,count:%d", count);
//DPF("type:%x,dim:%d,elem:%d",pData->vt,pa->cDims,pa->cbElements);
//DPF("lb:%d,cnt:%d",pa->rgsabound[0].lLbound,pa->rgsabound[0].cElements);
	p = (int *)pa->pvData;
//	DPF("x:%d,y:%d,z:%d,q:%d",(int)p[0],(int)p[1],p[2],p[3]);
	for (;count--;)
	{
//	char buf[100];
	int x = (int)*p++;// * 7 * GetDeviceCaps(hDC, LOGPIXELSX)) / 2540;
	int y = (int)*p++;// * 7 * GetDeviceCaps(hDC, LOGPIXELSY)) / 2540;
	UINT pressure;
	if (m_prange)
		pressure = (255 * *p++) / m_prange;
	else
		pressure = 255;
	//wsprintf(buf,"x:%5d,y:%5d,z:%d",x, y, pressure);
	//	strcpy(buf,"Hello");
	m_pTablet->InkPacket(x,y,pressure);
	}
}
    
CMyInkEvents::CMyInkEvents()
{
	m_hWnd = NULL;
	m_id = -1;
	m_prange = 0;
	m_flags = 0;
}
    
HRESULT CMyInkEvents::Init(
        HWND hWnd, CTablet * pTablet)
{
	m_hWnd = hWnd;
	m_pTablet = pTablet;
	return InkCollectorEvents::Init();
}    
    
void CMyInkEvents::CheckTablet(UINT check,
					IInkCursor * pCursor, IInkTablet * pTablet)
{
	BSTR bstr;
	if (SUCCEEDED(pTablet->get_Name(&bstr)))
		{
		TRACE("The name of tablet is %s.\n", bstr);
		}
	m_flags = 0;
	VARIANT_BOOL bInverted;
	HRESULT hr = pCursor->get_Inverted (&bInverted);
	if ((hr == S_OK) && (check & 1))
		{
		m_flags |= bInverted & 1;
		}
	if (!(check & 2))
		return;
//	if (!wcscmp(bstr, L"\\\\.\\DISPLAY1"))
//		flags |= 16;
//	m_flags = flags;
	return;
	TabletHardwareCapabilities hc;
	hr = pTablet->get_HardwareCapabilities (&hc);
	if (SUCCEEDED(hr))
		{
		TRACE("hc:%x",hc);
		}
	TabletDeviceKind  Kind;
	IInkTablet2 * pTablet2;
	hr = pTablet->QueryInterface(IID_IInkTablet2,
						reinterpret_cast<void**>(&pTablet2));
	if (hr == S_OK)
		{
		hr = ((IInkTablet2 *)pTablet)->get_DeviceKind( &Kind);
		if (hr == S_OK)
			{
			DPF("kind:%d",Kind);
			if (Kind == TDK_Touch) //TabletDeviceKind.Touch)
				{
				m_flags |= 16;
				DPF("touch");
				}
			else if (Kind == TDK_Pen)//TabletDeviceKind.Pen)
				{
				DPF("pen");
				}
			else if (Kind == TDK_Mouse)
				{
				DPF("mouse");
				}
			else
				{
				DPF("Bad Kind:%d",Kind);
				}
			}
		}

	VARIANT_BOOL bMultiTouch = 0;
	ULONG nMaxCursors;
	IInkTablet3 * pTablet3;
	hr = pTablet->QueryInterface(IID_IInkTablet3,
						reinterpret_cast<void**>(&pTablet3));
	if (hr == S_OK)
		{
		hr = ((IInkTablet3 *)pTablet)->get_IsMultiTouch(&bMultiTouch);
		if (hr == S_OK)
			{
			DPF("multi:%u",bMultiTouch);
			}
		hr = ((IInkTablet3 *)pTablet)->get_MaximumCursors(&nMaxCursors);
		if (hr == S_OK)
			{
			DPF("max cursors:%u",nMaxCursors);
			}
		}
}

void CMyInkEvents::SetupCursor(IInkCursor * Cursor, UINT Check)
{
// get prop for tablet
	long iid;
	if (Cursor->get_Id(&iid) != S_OK)
		return;
	if (iid == m_id)
		return;
	if (Check & 2)
		iid = iid;
	DPF("new cursor:%d",m_id);
	IInkTablet * pTablet = 0;
	if (Cursor->get_Tablet( &pTablet) != S_OK)
		return;
	UINT flags = m_flags;
	if (Check)
		CheckTablet(Check, Cursor, pTablet);
	long min, max;
	float  res;
	m_prange = 0;
  	TabletPropertyMetricUnit units;
	int i;
	for (i = 2; i < 3; i++) // pressure only
		{
		BSTR bstr;
		if (i == 2)
			 bstr = SysAllocString(STR_GUID_NORMALPRESSURE);
		else if (i == 1)
			 bstr = SysAllocString(STR_GUID_Y);
		else
			 bstr = SysAllocString(STR_GUID_X);
		HRESULT hr = pTablet->GetPropertyMetrics(bstr,
		 				&min,&max,&units,&res);
		if ((i == 2) && (hr == S_OK))
			{
			m_prange = max;
			}
		if ((i == 0) && (hr == S_OK))
			{
//			m_pTablet->m_xrange = max;
			}
		if ((i == 1) && (hr == S_OK))
			{
//			m_pTablet->m_yrange = max;
			}
TRACE("i:%d,min:%d,max:%d,units:%d,res:%d,hr:%d\n",	\
					i,min,max,(int)units,(int)(1000*res),(int)hr);
DPF("i:%d,min:%d,max:%d,units:%d,res:%d,hr:%d",i,min,max,(int)units,(int)(1000*res),(int)hr);
		SysFreeString(bstr);
		}
	m_id = iid;	// different cursor
	if (flags != m_flags)
		{	
	//	m_flags = flags;
		::PostMessage(m_hWnd,TABLET_MESSAGE,4 + (m_flags << 8),0);
		}
}

    
/////////////////////////////////////////////////////////
//                                          
// class CMyInkCollector
// 
// The CMyInkCollector class handles the connection between
// the window and the Ink Collector. It sets up the connection
// at start up and tears it down at exit.
//
/////////////////////////////////////////////////////////
CMyInkCollector::CMyInkCollector()
{
	m_pInkCollector = NULL;
}
    
// Destructor: if there is an InkCollector, then release
//  all resources connected with that InkCollector.
CMyInkCollector::~CMyInkCollector()
{
	if (m_pInkCollector != NULL)
        {
		m_InkEvents.Init(0,0);
        m_InkEvents.UnadviseInkCollector();
        m_pInkCollector->put_Enabled(VARIANT_FALSE);
        m_pInkCollector->Release();
        }
}
    
HRESULT CMyInkCollector::Init(HWND hWnd, CTablet * pTablet)
{
        // Initialize event sink. This consists of setting
        //  up the free threaded marshaler.
        HRESULT hr = m_InkEvents.Init(hWnd, pTablet);
        
        if (FAILED(hr))
        {
            return hr;
        }
        
        // Create the ink collector
        hr = CoCreateInstance(CLSID_InkCollector, NULL, CLSCTX_ALL,
            IID_IInkCollector, (void **) &m_pInkCollector);

        if (FAILED(hr))
        {
            return hr;
        }

		VARIANT  valDesc ;
		
		valDesc.vt = VT_BSTR | VT_ARRAY ;

		
   		SAFEARRAYBOUND    sAryBound ;
   		BSTR*             bstrData ;

   		sAryBound.cElements   = 3;
   		sAryBound.lLbound     = 0;

   valDesc.parray= SafeArrayCreate( VT_BSTR, 1, &sAryBound ) ;
   SafeArrayAccessData( valDesc.parray, ( void** )&bstrData ) ;

	bstrData[  0 ] = STR_GUID_X ;
	bstrData[  1 ] = STR_GUID_Y ;
	bstrData[  2 ] = STR_GUID_NORMALPRESSURE ;
//   bstrData[  3 ] = STR_GUID_PAKETSTATUS ;
  
   	SafeArrayUnaccessData( valDesc.parray ) ;
	
hr = m_pInkCollector->put_DesiredPacketDescription( valDesc) ;
hr = m_pInkCollector->get_DesiredPacketDescription( &valDesc) ;
//LASTERR;
_ASSERTE( SUCCEEDED( hr )) ;





        // Since this sample only handles the Stroke event, it is
        // not necessary to set the event interest (Stroke, CursorInRange,
        // CursorOutOfRange are the only event interests that are on by 
        // default).  In order to handle an event that is not on by default, 
        // it is necessary to set the event interest at this point.  The
        // following code illustrates how to listen for the NewPackets
        // event:
        // 
        hr = m_pInkCollector->SetEventInterest(ICEI_Stroke, VARIANT_FALSE);
        if (FAILED(hr))
        {
            return hr;
        }

        hr = m_pInkCollector->SetEventInterest(ICEI_NewPackets, VARIANT_TRUE);
        if (FAILED(hr))
        {
            return hr;
        }

        hr = m_pInkCollector->SetEventInterest(ICEI_CursorDown, VARIANT_TRUE);
        if (FAILED(hr))
        {
            return hr;
        }

        hr = m_pInkCollector->SetEventInterest(ICEI_CursorInRange,VARIANT_TRUE);
        if (FAILED(hr))
        {
            return hr;
        }

        // Set up connection between Ink Collector and our event sink        
        hr = m_InkEvents.AdviseInkCollector(m_pInkCollector);

        if (FAILED(hr))
        {
            return hr;
        }

        // Attach Ink Collector to window
        hr = m_pInkCollector->put_hWnd((long) hWnd);
        
        if (FAILED(hr))
        {
            return hr;
        }
	m_pInkCollector->put_MousePointer(IMP_Custom);
   	m_pInkCollector->put_DynamicRendering(0); 
//	m_pInkCollector->CollectingInk(0);
  	m_pInkCollector->put_AutoRedraw(0);     
        // Allow Ink Collector to receive input.
    hr = m_pInkCollector->put_Enabled(VARIANT_TRUE);
	if (FAILED(hr))
	{
		return hr;
	}
	return hr;
}
    
/*
HRESULT GetPacketDescriptionPropertyMetrics(
  [in]   BSTR id,
  [out]  long *min,
  [out]  long *max,
  [out]  TabletPropertyMetricUnit *units,
  [out]  float *resolution
);

*/


#endif
