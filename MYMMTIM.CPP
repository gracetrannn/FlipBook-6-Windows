#include	"stdafx.h"
#include	"mymmtim.h"


CMMTimer::CMMTimer(UINT resolution) : m_TimerRes(0), m_TimerId(0)
{
#ifdef FLIPBOOK_MAC
	// not sure why this is called resolution; it's really the timer ID
	fMacTimer = new WindowsEventTimer (resolution);
#else
	TIMECAPS	tc;

	if (TIMERR_NOERROR == timeGetDevCaps(&tc,sizeof(TIMECAPS)))
	{
		m_TimerRes = min(max(tc.wPeriodMin,resolution),tc.wPeriodMax);
		timeBeginPeriod(m_TimerRes);
	}
#endif
}


CMMTimer::~CMMTimer()
{
	StopTimer();
#ifdef FLIPBOOK_MAC
	delete fMacTimer;
#else
	if (0 != m_TimerRes)
	{
		timeEndPeriod(m_TimerRes);
		m_TimerRes = 0;
	}
#endif
}


extern "C"
void
CALLBACK
internalTimerProc(UINT id,UINT msg,DWORD dwUser,DWORD dw1,DWORD dw2)
{
//DPF("call back");
	CWnd * pWnd = (CWnd *)dwUser;
	pWnd->PostMessage(WM_COMMAND, ID_TIMER_FIRE);
}


BOOL CMMTimer::StartTimer(CWnd* pWnd, UINT period,BOOL oneShot)
{
#ifdef FLIPBOOK_MAC
	return fMacTimer->StartWndTimer (pWnd, period, oneShot);
#else
	bool		res = false;
	MMRESULT	result;

	result = timeSetEvent(period,m_TimerRes,internalTimerProc,(DWORD)pWnd,oneShot ? TIME_ONESHOT : TIME_PERIODIC);
	if (NULL != result)
	{
		m_TimerId = (UINT)result;
		res = true;
	}

	return res;
#endif
}


BOOL CMMTimer::StopTimer()
{
#ifdef FLIPBOOK_MAC
	return fMacTimer->StopTimer();
#else
	MMRESULT	result;

	result = timeKillEvent(m_TimerId);
	if (TIMERR_NOERROR == result)
		m_TimerId = 0;

	return TIMERR_NOERROR == result;
#endif
}
