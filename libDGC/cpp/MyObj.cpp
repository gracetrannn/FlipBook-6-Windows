#include "MyIO.h"
#include "MyObj.h"
CMyObj::CMyObj(CMyIO * pIO)
//CMyObj::CMyObj(DWORD key)
{
//	DPF2("myobj construct,key:%8x",key);
	DPF2("myobj construct");
	m_pIO = pIO;
	m_kind = KIND_BAD;
	m_key = 0;
	m_pData = 0;
	m_size = 0;
}

CMyObj::~CMyObj()
{
	DPF2("myobj destruct,key:%8x,kind:%8lx",m_key,m_kind);
	delete [] m_pData;	
	m_pData = 0;
	m_size = 0;
	m_kind = 0;
}

int CMyObj::Read()
{
	ASSERT(m_pData != NULL);
	int result = m_pIO->GetRecord(m_pData, m_size, m_key);
//	ASSERT(result == 0);
	if (result) return result;
//	ASSERT(*((DWORD *)m_pData) == SWAPV(m_key));
	if (*((DWORD *)m_pData) != SWAPV(m_key))
		return 97;
	return result;
}

int CMyObj::Write()
{
	ASSERT(m_pData != NULL);
	int result = m_pIO->PutRecord(m_pData, m_size, m_key);
	return result;
}
