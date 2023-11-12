#ifndef	__mm_timer__
#define	__mm_timer__
#include	<mmsystem.h>

class CMMTimer
{
public:
	CMMTimer(UINT resolution);
	virtual ~CMMTimer();

	UINT	GetTimerRes() { return m_TimerRes; };

	BOOL	StartTimer(CWnd * pWnd, UINT period,BOOL oneShot);
	BOOL	StopTimer();
protected:
	UINT	m_TimerRes;
	UINT	m_TimerId;

#ifdef FLIPBOOK_MAC
	WindowsEventTimer* fMacTimer;
#endif
};

#endif
//		return;
