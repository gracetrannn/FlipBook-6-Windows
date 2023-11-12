#ifndef _MYCLIP_H_
#define _MYCLIP_H_

class CMyClipBoard
{
	CMyClipBoard();
public:
	void	RemoveAll();
	void	Begin(UINT w, UINT h);
	UINT 	Width() { return m_width;};
	UINT 	Height() { return m_width;};
protected:
	UINT	m_width;
	UINT	m_height;
	UINT	m_count;
};
#endif

