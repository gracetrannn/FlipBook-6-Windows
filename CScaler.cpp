#include "stdafx.h"
#include "CScaler.h"

int CSScaler::Custom(HPBYTE hpDst, UINT opitch,
	HPBYTE hpSrc, UINT ipitch, BYTE* pals, UINT d, UINT rot)
{
	m_dst = hpDst;
	m_src = hpSrc;
	m_ipitch = ipitch;
	m_opitch = opitch;
	m_depth = d;
	m_rot = rot;
	if (d < 9)
	{
		int i, c;
		c = 1 << d;
		for (i = 0; i < c; i++)
		{
			UINT v = 0;
			v += 11 * (UINT)pals[4 * i + 0];
			v += 59 * (UINT)pals[4 * i + 1];
			v += 30 * (UINT)pals[4 * i + 2];
			m_gray[i] = v / 100;
		}
	}
	return 0;
}

int CSScaler::get_line()
{
	if ((m_iy + m_ah) < m_ih)
	{
		memset(m_ibuf, 255, m_iw);
		return 0;
	}
	else if (m_aw < m_iw)
		memset(m_ibuf, 255, m_iw);
	if (m_depth == 8)
	{
		UINT x;
		for (x = 0; x < m_aw; x++)
			m_ibuf[x] = m_gray[m_src[x]];
	}
	else if (m_depth == 24)
	{
		UINT x;
		UINT v;
		for (x = 0; x < m_aw; x++)
		{
			v = 11 * m_src[3 * x + 0] + 59 * m_src[3 * x + 1] + 30 * m_src[3 * x + 2];
			m_ibuf[x] = v / 100;
		}
	}
	else if (m_depth == 32)
	{
		UINT x;
		//	UINT v;
		for (x = 0; x < m_aw; x++)
		{
			//	v = (11 * m_src[4*x+0] + 59 * m_src[4*x+1] + 30 * m_src[4*x+2] + 50)/ 100;
			m_ibuf[x] = 255 - m_src[4 * x + 3];// * (255 - v)) / 255;
		}
	}
	else if (m_depth == 16)
	{
		UINT x, v, q;
		for (x = 0; x < m_aw; x++)
		{
			v = m_src[2 * x + 0] + 256 * m_src[2 * x + 1];
			q = 11 * (v & 31);
			q += 58 * ((v >> 5) & 31);
			q += 30 * ((v >> 10) & 31);
			m_ibuf[x] = q / 12;
		}
	}
	else if (m_depth == 1)
	{
		UINT x, z, v;
		BYTE t = 0;
		BYTE m = 0;
		BYTE blk = m_gray[0];
		BYTE wht = m_gray[1];
		UINT qw = m_aw / 8;
		for (z = 0, x = 0; z < qw; z++)
		{
			if (!(t = m_src[z]))
			{
				v = m_gray[0];
				m_ibuf[x++] = blk;
				m_ibuf[x++] = blk;
				m_ibuf[x++] = blk;
				m_ibuf[x++] = blk;
				m_ibuf[x++] = blk;
				m_ibuf[x++] = blk;
				m_ibuf[x++] = blk;
				m_ibuf[x++] = blk;
			}
			else if (t == 255)
			{
				m_ibuf[x++] = wht;
				m_ibuf[x++] = wht;
				m_ibuf[x++] = wht;
				m_ibuf[x++] = wht;
				m_ibuf[x++] = wht;
				m_ibuf[x++] = wht;
				m_ibuf[x++] = wht;
				m_ibuf[x++] = wht;
			}
			else
			{
				for (m = 128; m;)
				{
					if (t & m)
						m_ibuf[x++] = wht;
					else
						m_ibuf[x++] = blk;
					m = m / 2;
				}
			}
		}
		for (; x < m_aw; x++)
		{
			if (!m)
			{
				t = m_src[z++];
				m = 128;
			}
			if (v = (t & m))
				v = 1;
			m_ibuf[x] = m_gray[v];
			m = m / 2;
		}
	}
	m_src += m_ipitch;
	return 0;
}

int CSScaler::put_line()
{
	if (m_rot == 1)
	{
		UINT y;
		UINT p = m_opitch;
		for (y = 0; y < m_ow; y++)
			m_dst[(m_offx + m_ow - 1 - y) * p + m_oy + m_offy] = m_obuf[y];
	}
	else if (m_rot == 3)
	{
		UINT y;
		UINT p = m_opitch;
		for (y = 0; y < m_ow; y++)
			m_dst[(y + m_offx) * p + m_oh - 1 - m_oy + m_offy] = m_obuf[y];
	}
	else if (m_rot == 2)
	{
		UINT x;
		for (x = 0; x < m_ow; x++)
			m_dst[m_opitch * (m_offy + m_oh - 1 - m_oy) +
			m_offx + m_ow - 1 - x] = m_obuf[x];
	}
	else
	{
		memmove(m_dst + m_opitch * (m_offy + m_oy) + m_offx, m_obuf, m_ow);
	}
	return 0;
}

int CCScaler::Custom(HPBYTE hpDst, UINT opitch,
	HPBYTE hpSrc, UINT ipitch, BYTE* pals, UINT id, UINT rot)
{
	m_dst = hpDst;
	m_src = hpSrc;
	m_ipitch = ipitch;
	m_opitch = opitch;
	m_depth = id;
	DPF("ccscaler,bpp:%d", m_id);
	m_bCvt24 = (m_id == 4) && (id == 24) && (rot & 128) ? 1 : 0;
	m_key = rot >> 16;
	m_rot = rot & 3;
	m_pals = pals;
	return 0;
}

int CCScaler::get_line()
{
	if ((m_iy + m_ah) < m_ih)
	{
		memset(m_ibuf, 255, m_id * m_iw);
		return 0;
	}
	else if (m_aw < m_iw)
		memset(m_ibuf, 255, m_id * m_iw);
	if (m_depth == 8)
	{
		UINT x, v, j;
		for (x = 0; x < m_aw; x++)
		{
			v = m_src[x];
			for (j = 0; j < m_id; j++)
				m_ibuf[m_id * x + j] = m_pals[4 * v + j];
		}
	}
	else if (m_depth == 1)
	{
		UINT x, z, v, j;
		BYTE t = 0;
		BYTE m = 0;
		//		UINT qw = m_aw / 8;
		for (x = 0, z = 0; x < m_aw; x++)
		{
			if (!m)
			{
				t = m_src[z++];
				m = 128;
			}
			if (v = (t & m))
				v = 1;
			for (j = 0; j < m_id; j++)
				m_ibuf[m_id * x + j] = m_pals[4 * v + j];
			m = m / 2;
		}
	}
	else if (m_bCvt24)
	{
		UINT x;
		if (m_key == 255)
		{
			for (x = 0; x < m_aw; x++)
			{
				m_ibuf[4 * x + 0] = m_src[3 * x + 0];
				m_ibuf[4 * x + 1] = m_src[3 * x + 1];
				m_ibuf[4 * x + 2] = m_src[3 * x + 2];
				m_ibuf[4 * x + 3] = 255;
			}
		}
		else
		{
			for (x = 0; x < m_aw; x++)
			{
				BYTE r = m_ibuf[4 * x + 0] = m_src[3 * x + 0];
				BYTE g = m_ibuf[4 * x + 1] = m_src[3 * x + 1];
				BYTE b = m_ibuf[4 * x + 2] = m_src[3 * x + 2];
				m_ibuf[4 * x + 3] = (r > m_key) &&
					(g > m_key) && (b > m_key) ? 0 : 255;
			}
		}
	}
	else if ((m_depth == 32) && (m_id == 3))
	{
		for (UINT x = 0; x < m_aw; x++)
		{
			m_ibuf[3 * x + 0] = m_src[4 * x + 0];
			m_ibuf[3 * x + 1] = m_src[4 * x + 1];
			m_ibuf[3 * x + 2] = m_src[4 * x + 2];
		}
	}
	else if (m_depth >= 24)
	{
		memmove(m_ibuf, m_src, m_id * m_aw);
	}
	else if (m_depth == 16)
	{
		UINT x, v;
		for (x = 0; x < m_aw; x++)
		{
			v = m_src[2 * x + 0] + 256 * m_src[2 * x + 1];
			m_ibuf[3 * x + 0] = (33 * (v & 31)) / 4;
			m_ibuf[3 * x + 1] = (33 * ((v >> 5) & 31)) / 4;
			m_ibuf[3 * x + 2] = (33 * ((v >> 10) & 31)) / 4;
		}
	}
	m_src += m_ipitch;
	return 0;
}

int CCScaler::put_line()
{
	if (m_rot == 1)
	{
		UINT y;
		UINT p = m_opitch;
		for (y = 0; y < m_ow; y++)
		{
			UINT i;
			for (i = 0; i < m_id; i++)
				m_dst[(m_offx + m_ow - 1 - y) * p + m_id * (m_oy + m_offy) + i] =
				m_obuf[m_id * y + i];
		}
	}
	else if (m_rot == 3)
	{
		UINT y;
		UINT p = m_opitch;
		for (y = 0; y < m_ow; y++)
		{
			UINT i;
			for (i = 0; i < m_id; i++)
				m_dst[(y + m_offx) * p + m_id * (m_oh - 1 - m_oy + m_offy) + i] =
				m_obuf[m_id * y + i];
		}
	}
	else if (m_rot == 2)
	{
		UINT x;
		for (x = 0; x < m_ow; x++)
		{
			UINT i;
			for (i = 0; i < m_id; i++)
				m_dst[m_opitch * (m_offy + m_oh - 1 - m_oy) +
				m_id * (m_offx + m_ow - 1 - x) + i] = m_obuf[m_id * x + i];
		}
	}
	else
	{
		memmove(m_dst + m_opitch * (m_offy + m_oy) + m_id * m_offx,
			m_obuf, m_id * m_ow);
	}
	return 0;
}