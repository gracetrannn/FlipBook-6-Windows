#include "NScaler.h"
#include "utils.h"


CNScaler::CNScaler()
{
	m_ibuf = 0;
	m_obuf = 0;
	m_acc = 0;
}

CNScaler::~CNScaler()
{
	Clear();
}

void CNScaler::Clear()
{
	if (m_ibuf)
		delete [] m_ibuf;
	m_ibuf = 0;
	if (m_obuf)
		delete [] m_obuf;
	m_obuf = 0;
	if (m_acc)
		delete [] m_acc;
	m_acc = 0;
}

bool CNScaler::Init(UINT iw, UINT ih, UINT idd, UINT ow, UINT oh, UINT nHold)
{
	m_bMono = FALSE;
	if (nHold & 8)
		{
		m_offy = (oh - ih) / 2;
		m_offx = (ow - iw) / 2;
		ow = iw;
		oh = ih;
		nHold &= 7;
		}
	else if (nHold > 1)
		{
		if ((DWORD)iw * oh > (DWORD)ih * ow)
			{
			m_offx = 0;
			UINT t = oh;
			iw = MulDiv(ih,ow,oh);
			m_offy = 0;//(t - oh) / 2;
			}
		else
			{
			m_offy = 0;
			UINT t = ow;
			ih = MulDiv(iw,oh,ow);
			m_offx = 0;//(t - ow) / 2;
			}
		}
	else if (nHold)
		{
		if ((DWORD)iw * oh > (DWORD)ih * ow)
			{
			m_offx = 0;
			UINT t = oh;
			oh = MulDiv(ih, ow, iw);
			m_offy = (t - oh) / 2;
			}
		else
			{
			m_offy = 0;
			UINT t = ow;
			ow = MulDiv(oh, iw, ih);
			m_offx = (t - ow) / 2;
			}
		}
	else
		{
		m_offx = 0;
		m_offy = 0;
		}
	if (idd == 1)
		{
		m_bMono = TRUE;
		idd = 8;
		}
	m_iw = iw;
	m_ih = ih;
	m_id = (idd + 7) / 8;
	m_ow = ow;
	m_oh = oh;
	m_ip = iw * m_id;
	m_op = ow * m_id;
DPF2("scale init, iw:%d,ih:%d,id:%d,ow:%d,oh:%d",m_iw,m_ih,m_id,m_ow,m_oh);
DPF2("offx:%d,offy:%d",m_offx,m_offy);
	m_cnt = 0;
	m_iy = 0;
	m_oy = 0;
	m_acty = 0;
	m_adj = 1;
	Clear();
	m_divisor = (DWORD)m_iw * (DWORD)m_ih;
	if (m_bMono)
		m_divisor /= 255;
	else
		{
		for (;m_divisor >= 0x1000000;)
			{
			m_adj *= 2;
			m_divisor /= 2;
			}
		}
	m_ibuf = new BYTE[m_ip+10];
	m_obuf = new BYTE[m_op+10];
	m_acc = new DWORD[m_op+10];
	if (m_ibuf && m_obuf && m_acc)
		{
		clearacc();
		return 0;
		}
	else
		return 1;
}
//
//	should be overridden by parent class
//
//
int CNScaler::get_line()
{
DPF2("raw getline:%d",m_iy);
	return 99;
}
int CNScaler::put_line()
{
DPF2("raw putline:%d",m_oy);
	return 99;
}

int CNScaler::make_line()
{
DPF2("raw makeline:%d",m_oy);
	for (WORD i = 0; i < m_op; i++)
		m_obuf[i] = (BYTE)(m_acc[i] / m_divisor);
	return put_line();
}

int CNScaler::PullLine(BYTE * buffer)
{
	int nResult = 0;
DPF2("pull line:%d",m_oy);
	for (;;)
		{
		m_cnt += m_oh;
		if (m_cnt >= m_ih)
			break;
		if (nResult = hline(m_oh))
			return nResult;
		m_iy++;
		}
	WORD f = m_ih + m_oh - m_cnt;
	if (nResult = hline(f))
		return nResult;
	for (WORD i = 0; i < m_op; i++)
		buffer[i] = (BYTE)(m_acc[i] / m_divisor);
	m_oy++;
	clearacc();
	m_cnt -= m_ih;
	if (m_cnt)
		if (nResult = hline(m_cnt))
			return nResult;
	m_iy++;
	return nResult;
}

int CNScaler::Copy()
{
	int nResult = 0;
	if (m_ih >= m_oh)
	{
	for (; m_oy < m_oh;)
		{
//DPF2("csale loop:oy:%d",m_oy);
		m_cnt += m_oh;
		if (m_cnt < m_ih)
			{
			if (nResult = hline(m_oh))
				break;
			}
		else
			{
			int f = m_ih + m_oh - m_cnt;
			if (nResult = hline(f))
				break;
			for (WORD i = 0; i < m_op; i++)
				m_obuf[i] = (BYTE)(m_acc[i] / m_divisor);
			if (nResult = put_line())
				break;
			m_oy++;
			clearacc();
			m_cnt -= m_ih;
			if (m_cnt)
				if (nResult = hline(m_cnt))
					break;
			}
		m_iy++;
		}
	}
	else
	{
	m_cnt = m_oh;
	if (nResult = get_line())
		return nResult;
	m_acty++;
	m_iy++;
	bool bSkip = 0;
	for (; m_oy < m_oh;m_oy++)
		{
		clearacc();
		if (m_cnt > m_ih)
			{
			if (nResult = hline(m_ih, 1))
				break;
			}
		else
			{
			if (nResult = hline(m_cnt,1))
				break;
			if (m_ih > m_cnt)
				{
				if (nResult = hline(m_ih-m_cnt))
					break;
				m_iy++;
				m_cnt += m_oh;
				}
			}
		m_cnt -= m_ih;
		for (WORD i = 0; i < m_op; i++)
			m_obuf[i] = (BYTE)(m_acc[i] / m_divisor);
		if (nResult = put_line())
			break;
		}
	}
	return nResult;
}

void CNScaler::clearacc()
{
	for (UINT i = 0; i < m_op; m_acc[i++] = 0);
}

int CNScaler::hline(UINT f, bool bSkip /* = 0 */)
{
//DPF2("hline,iy:%d,ay:%d",m_iy,m_acty);
	int nResult;
	f = (f + m_adj - 1) / m_adj;
	if (m_ih >= m_oh)
		{
		if ((m_iy + 1) > m_acty)
			{
			if (nResult = get_line())
				return nResult;
			m_acty++;
			}
		}
	else
		{
		if (m_iy < m_ih)
			{
			if (!bSkip)
				{
				if (nResult = get_line())
					return nResult;
				m_acty++;
				}
			}
		}
	if (m_bMono)
		hline0(f);
	else if (m_id == 1)
		hline1(f);
	else if (m_id == 3)
		hline3(f);
	else
		hline4(f);
	return 0;
}

void CNScaler::hline0(UINT f)
{
	WORD ox;
	WORD cnt = 0;
	DWORD f1 = (DWORD)f * (DWORD)m_ow;
	BYTE *sp = m_ibuf;
	for (ox = 0; ox < m_ow;)
		{
		cnt += m_ow;
		if (cnt < m_iw)
			{
			if (*sp++)
				m_acc[ox] += f1;
			}
		else
			{
			WORD ff = m_iw + m_ow - cnt;
			if (ff == m_ow)
				{
				cnt = 0;
				if (*sp++)
					m_acc[ox++] += f1;
				else
					ox++;
				}
			else
				{
				cnt -= m_iw;
				if (*sp++)
					{
					m_acc[ox++] += (DWORD)f * (DWORD)ff;
					m_acc[ox] += (DWORD)f * (DWORD)cnt;
					}
				else
					ox++;
				}
			}
		}
}

void CNScaler::hline1(UINT f)
{
	if (m_ow <= m_iw)
	{
	WORD ox;
	WORD cnt = 0;
	DWORD f1 = (DWORD)f * (DWORD)m_ow;
	BYTE *sp = m_ibuf;
	for (ox = 0; ox < m_ow;)
		{
		cnt += m_ow;
		if (cnt < m_iw)
			{
			m_acc[ox] += f1 * (DWORD)*sp++;
			}
		else
			{
			WORD ff = m_iw + m_ow - cnt;
			if (ff == m_ow)
				{
				cnt = 0;
				m_acc[ox++] += f1 * (DWORD)*sp++;
				}
			else
				{
				cnt -= m_iw;
				m_acc[ox++] += (DWORD)f * (DWORD)ff * (DWORD)*sp;
				m_acc[ox] += (DWORD)f * (DWORD)cnt * (DWORD)*sp++;
				}
			}
		}
	}
	else // if (m_ow > m_iw)
	{
	WORD ox;
	WORD cnt = m_ow;
	DWORD f1 = (DWORD)f * (DWORD)m_iw;
	BYTE *sp = m_ibuf;
	for (ox = 0; ox < m_ow;ox++)
		{
		if (cnt > m_iw)
			{
			m_acc[ox] += f1 * (DWORD)*sp;
			cnt -= m_iw;
			}
		else
			{
			DWORD f2 = (DWORD)f * (DWORD)cnt;
			m_acc[ox] += f2 * (DWORD)*sp++;
			if (m_iw > cnt)
			{
			f2 = (DWORD)f * (DWORD)(m_iw - cnt);
			m_acc[ox] += f2 * (DWORD)*sp;
			}
			cnt = m_ow - m_iw + cnt;
			}
		}
	}
}

void CNScaler::hline3(UINT f)
{
	if (m_ow <= m_iw)
	{
	BYTE *sp = m_ibuf;
	WORD ox;
	WORD cnt = 0;
	DWORD f1 = (DWORD)f * (DWORD)m_ow;
	for (ox = 0; ox < m_ow;)
		{
		cnt += m_ow;
		if (cnt < m_iw)
			{
			m_acc[3 * ox + 0] += f1 * (DWORD)*sp++;
			m_acc[3 * ox + 1] += f1 * (DWORD)*sp++;
			m_acc[3 * ox + 2] += f1 * (DWORD)*sp++;
			}
		else
			{
			DWORD f2 = (DWORD)f * (DWORD)(m_iw + m_ow - cnt);
			m_acc[3 * ox + 0] += f2 * (DWORD)*sp++;
			m_acc[3 * ox + 1] += f2 * (DWORD)*sp++;
			m_acc[3 * ox + 2] += f2 * (DWORD)*sp++;
			ox++;
			cnt -= m_iw;
			if (cnt)
				{
				sp -= 3;//m_id;
				DWORD f3 = (DWORD)f * (DWORD)cnt;
				m_acc[3 * ox + 0] += f3 * (DWORD)*sp++;
				m_acc[3 * ox + 1] += f3 * (DWORD)*sp++;
				m_acc[3 * ox + 2] += f3 * (DWORD)*sp++;
				}
			}
		}
	}
	else // if (m_ow > m_iw)
	{
	BYTE *sp = m_ibuf;
	WORD ox;
	WORD cnt = m_ow;
	DWORD f1 = (DWORD)f * (DWORD)m_iw;
	for (ox = 0; ox < m_ow;ox++)
		{
		if (cnt > m_iw)
			{
			m_acc[3 * ox + 0] += f1 * (DWORD)*sp++;
			m_acc[3 * ox + 1] += f1 * (DWORD)*sp++;
			m_acc[3 * ox + 2] += f1 * (DWORD)*sp++;
			sp -= 3;
			cnt -= m_iw;
			}
		else
			{
			DWORD f2 = (DWORD)f * (DWORD)cnt;
			m_acc[3 * ox + 0] += f2 * (DWORD)*sp++;
			m_acc[3 * ox + 1] += f2 * (DWORD)*sp++;
			m_acc[3 * ox + 2] += f2 * (DWORD)*sp++;
			if (m_iw > cnt)
			{
			f2 = (DWORD)f * (DWORD)(m_iw - cnt);
			m_acc[3 * ox + 0] += f2 * (DWORD)*sp++;
			m_acc[3 * ox + 1] += f2 * (DWORD)*sp++;
			m_acc[3 * ox + 2] += f2 * (DWORD)*sp++;
			sp -= 3;
			}
			cnt = m_ow - m_iw + cnt;
			}
		}
	}
}

void CNScaler::hline4(UINT f)
{
	if (m_ow <= m_iw)
	{
	BYTE *sp = m_ibuf;
	WORD ox;
	WORD cnt = 0;
	DWORD f1 = (DWORD)f * (DWORD)m_ow;
	for (ox = 0; ox < m_ow;)
		{
		cnt += m_ow;
		if (cnt < m_iw)
			{
			m_acc[4 * ox + 0] += f1 * (DWORD)*sp++;
			m_acc[4 * ox + 1] += f1 * (DWORD)*sp++;
			m_acc[4 * ox + 2] += f1 * (DWORD)*sp++;
			m_acc[4 * ox + 3] += f1 * (DWORD)*sp++;
			}
		else
			{
			DWORD f2 = (DWORD)f * (DWORD)(m_iw + m_ow - cnt);
			m_acc[4 * ox + 0] += f2 * (DWORD)*sp++;
			m_acc[4 * ox + 1] += f2 * (DWORD)*sp++;
			m_acc[4 * ox + 2] += f2 * (DWORD)*sp++;
			m_acc[4 * ox + 3] += f2 * (DWORD)*sp++;
			ox++;
			cnt -= m_iw;
			if (cnt)
				{
				sp -= 4;//m_id;
				DWORD f3 = (DWORD)f * (DWORD)cnt;
				m_acc[4 * ox + 0] += f3 * (DWORD)*sp++;
				m_acc[4 * ox + 1] += f3 * (DWORD)*sp++;
				m_acc[4 * ox + 2] += f3 * (DWORD)*sp++;
				m_acc[4 * ox + 3] += f3 * (DWORD)*sp++;
				}
			}
		}
	}
	else // if (m_ow > m_iw)
	{
	BYTE *sp = m_ibuf;
	WORD ox;
	WORD cnt = m_ow;
	DWORD f1 = (DWORD)f * (DWORD)m_iw;
	for (ox = 0; ox < m_ow;ox++)
		{
		if (cnt > m_iw)
			{
			m_acc[4 * ox + 0] += f1 * (DWORD)*sp++;
			m_acc[4 * ox + 1] += f1 * (DWORD)*sp++;
			m_acc[4 * ox + 2] += f1 * (DWORD)*sp++;
			m_acc[4 * ox + 3] += f1 * (DWORD)*sp++;
			sp -= 4;
			cnt -= m_iw;
			}
		else
			{
			DWORD f2 = (DWORD)f * (DWORD)cnt;
			m_acc[4 * ox + 0] += f2 * (DWORD)*sp++;
			m_acc[4 * ox + 1] += f2 * (DWORD)*sp++;
			m_acc[4 * ox + 2] += f2 * (DWORD)*sp++;
			m_acc[4 * ox + 3] += f2 * (DWORD)*sp++;
			if (m_iw > cnt)
			{
			f2 = (DWORD)f * (DWORD)(m_iw - cnt);
			m_acc[4 * ox + 0] += f2 * (DWORD)*sp++;
			m_acc[4 * ox + 1] += f2 * (DWORD)*sp++;
			m_acc[4 * ox + 2] += f2 * (DWORD)*sp++;
			m_acc[4 * ox + 3] += f2 * (DWORD)*sp++;
			sp -= 4;
			}
			cnt = m_ow - m_iw + cnt;
			}
		}
	}
}

int CGScaler::Custom(HPBYTE hpDst, HPBYTE hpSrc, UINT pitch, UINT ip)
{
	m_dst = hpDst;
	m_src = hpSrc;
	m_pitch = pitch;
	if (ip)
		m_ipitch = ip;
	else
		m_ipitch = m_iw * m_id;
	m_cnt = 0;
	m_iy = 0;
	m_oy = 0;
	m_acty = 0;
	return 0;
}

int CGScaler::get_line()
{
	memmove(m_ibuf, m_src,m_iw*m_id);
	m_src += m_ipitch;
	return 0;
}

int CGScaler::put_line()
{
	memmove(m_dst, m_obuf, m_ow*m_id);
	m_dst += m_pitch;
	return 0;
}
