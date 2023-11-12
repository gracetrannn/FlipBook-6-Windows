#include "stdafx.h"
#include "shiftrc.h"
#include "clayers.h"
#include "math.h"
#include "myview.h"


#define RADIUS1 30
#define RADIUS2 50
#define RADIUS3 70

void CShiftTrace::Init(CSketchView * pView)
{
	DPZ("init shift trace");
	memset(this,0,sizeof( *this));
	m_pView = pView;
}

BOOL CShiftTrace::Toggle()
{
	m_bEditing = 0;
	m_bActive = !m_bActive;
	return 0;
}

/*
BOOL CShiftTrace::Engage(BOOL bActive, BOOL bEdit)
{
	BOOL bWasEditing = m_bEditing;
	m_bEditing = bEdit;
	m_bActive = bActive;
	if (m_bActive)
		{
		m_cx = m_pView->Width() / 2;
		m_cy = m_pView->Height() / 2;
		}
	return !m_bActive && bWasEditing;
}
*/
UINT CShiftTrace::Test(UINT frame, UINT level)
{
	if (!m_bActive) return 0;
	UINT i;
	UINT c = 10;
	for (i = 0; i < c; i++)
		{
		if ((m_shift_tbl[i].frame == frame)
				&& (m_shift_tbl[i].level == level))
			return i + 1;
		}
	return 0;
}

void CShiftTrace::Add(UINT frame, UINT level, UINT code)
{
	if (m_bEditing && !code)
		return;
	if (code > 1)
		{
		m_shift_tbl[0].flags = 1;
		m_shift_tbl[0].frame = frame;
		m_shift_tbl[0].level = level;
		m_shift_tbl[0].xx = 0;
		m_shift_tbl[0].yy = 0;
		m_shift_tbl[0].rot = 0;
		return;
		}
	UINT i;
	UINT c = 10;
	for (i = 0; i < c; i++)
		{
		if ((m_shift_tbl[i].frame == frame)
				&& (m_shift_tbl[i].level == level))
				break;
		}
	SHFTBL tmp;
	UINT j;
	if (i < c)
		{
		tmp = m_shift_tbl[i];
		j = i;
		}
	else
		j = c - 1;
	for (; j > 0; j--)
		m_shift_tbl[j] = m_shift_tbl[j-1];
	if (i < c)
		m_shift_tbl[0] = tmp;
	else
		{
		m_shift_tbl[0].flags = 1;
		m_shift_tbl[0].frame = frame;
		m_shift_tbl[0].level = level;
		m_shift_tbl[0].xx = 0;
		m_shift_tbl[0].yy = 0;
		m_shift_tbl[0].rot = 0;
		m_shift_tbl[0].num = 0;
		}
	if (code)
		{
		m_bActive = 1;
		m_bEditing = 1;
		m_cx = m_pView->Width() / 2;
		m_cy = m_pView->Height() / 2;
		}
}

void CShiftTrace::Get(UINT which, int & x, int & y, int & rot, int & num)
{
	x = m_shift_tbl[which].xx;
	y = m_shift_tbl[which].yy;
	rot = m_shift_tbl[which].rot;
	num = m_shift_tbl[which].num;
}

void CShiftTrace::Draw()
{
	if (!m_bEditing) return;
	m_center.x = m_cx + m_shift_tbl[0].xx;
	m_center.y = m_cy + m_shift_tbl[0].yy;
	int x = m_center.x;
	int y = m_center.y;
	CClientDC ddc(m_pView);
	CPoint p1, p2;
	p1.x = x - RADIUS1;
	p1.y = y - RADIUS1;
	p2.x = x + RADIUS1;
	p2.y = y + RADIUS1;
	m_pView->XlateCPoint(p1,1);
	m_pView->XlateCPoint(p2,1);
	m_pView->MyDrawObj(&ddc,2,p1,p2);

	p1.x = x - RADIUS2;
	p1.y = y - RADIUS2;
	p2.x = x + RADIUS2;
	p2.y = y + RADIUS2;
	m_pView->XlateCPoint(p1,1);
	m_pView->XlateCPoint(p2,1);
	m_pView->MyDrawObj(&ddc,2,p1,p2);

	p1.x = x - RADIUS3;
	p1.y = y - RADIUS3;
	p2.x = x + RADIUS3;
	p2.y = y + RADIUS3;
	m_pView->XlateCPoint(p1,1);
	m_pView->XlateCPoint(p2,1);
	m_pView->MyDrawObj(&ddc,2,p1,p2);
}

void CShiftTrace::Start(CPoint point)
{
	m_hit = Hit(point);
	if (!m_hit)
		return; 
	m_bDown = 1;
	m_start = point;
	m_xx = m_shift_tbl[0].xx;
	m_yy = m_shift_tbl[0].yy;
	m_rot= m_shift_tbl[0].rot;
	m_num= m_shift_tbl[0].num;
	ShiftCursor(m_hit);
}

void CShiftTrace::Drag(CPoint point)
{
	if (m_hit == 1)
		{
		m_shift_tbl[0].xx = m_xx + point.x - m_start.x;
		m_shift_tbl[0].yy = m_yy + point.y - m_start.y;
		}
	else if (m_hit == 2)
		{
		int dx = point.x - m_start.x;
		int dy = point.y - m_start.y;
		int v = m_num + 100 + (dx - dy) / 2;
		if (v > 500) v = 500;
		if (v < 20) v = 20;
		m_shift_tbl[0].num = v - 100;
		}
	else if (m_hit == 3)
		{
		double angle1 = atan2((double)(point.x - m_cx - m_xx),
					(double)(point.y - m_cy - m_yy));
			double angle2 = atan2((double)(m_start.x - m_cx - m_xx),
					(double)(m_start.y - m_cy - m_yy));
			double twopi = 8.0 * atan(1.0);
			int ang1 = (int)((360.0 * angle1) / twopi);
			int ang2 = (int)((360.0 * angle2) / twopi);
			int angle = (m_rot + 360 + ang2 - ang1) % 360;
			DPF("ang1:%d,ang2:%d,ang:%d",ang1,ang2,angle);
DPZ("mx:%d,my:%d,ang:%d",m_xx,m_yy, angle);
			m_shift_tbl[0].rot = angle;
		}
}

BOOL CShiftTrace::Stop(CPoint point)
{
	BOOL bRet = m_hit ? TRUE : FALSE;
	m_hit = 0;
	return bRet;
}

UINT CShiftTrace::Hit(CPoint point)
{
	int dx = point.x - m_center.x;
	int dy = point.y - m_center.y;
	int radq = dx * dx + dy * dy;
	if (radq <= RADIUS1 * RADIUS1)
		return 1;
	if (radq <= RADIUS2 * RADIUS2)
		return 2;
	if (radq <= RADIUS3 * RADIUS3)
		return 3;
	return 0;
}

void CShiftTrace::ShiftCursor(UINT code)
{
	LPSTR idd;
	UINT cid = 0;
	switch(code) {
	case 1:
		cid = IDC_HANDCUR;
		break;
	case 2:
		idd = IDC_SIZEALL;
		break;
	case 3:
		cid = IDC_ROTATE_CUR;
		break;
	default:
	//	ASSERT(0);
		idd = IDC_ARROW;
		break;
	}
	if (cid)
		SetCursor(AfxGetApp()->LoadCursor(cid));
	else
		SetCursor(AfxGetApp()->LoadStandardCursor(idd));
}

void CShiftTrace::Cursor(CPoint point)
{
	ASSERT(m_hit == 0);
	UINT code;
	if (point.x < 0)
		code = 0;
	else
		code = Hit(point);
	//DPF("cursor, m_hit:%d, hit:%d",m_hit,code);
	ShiftCursor(code);
}

