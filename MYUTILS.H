#ifndef _MYUTILS_H_
#define _MYUTILS_H_
class MyUINT
{
public:
	MyUINT() { m_count = 0; m_pData = 0; };
	~MyUINT() { delete [] m_pData;};
	UINT Count() { return m_count;};
	UINT Get(UINT index) {if (index < m_count) return m_pData[index];
							else return -1; };
	void Append(UINT v) { if (!(m_count % 2000) {
								UINT * pTemp = m_pData;
								m_pData = new UINT [m_count + 2000];
								if (m_count)
									memmove(m_pData, pTemp, 4 * m_count);
								delete [] pTemp;
								}
							m_pData[m_count++] = v;
							};
protected:
	UINT m_count;
	UINT * m_pData;
};

class MyTemp
{
public:
	MyTemp(UINT size = 0) { if (m_size = size) m_pData = new BYTE[size];
								else m_pData = 0;};
	~MyTemp() { delete [] m_pData; };
	void * Ensure(UINT size) { if (size > m_size)
								{
								delete [] m_pData;
								m_pData = new BYTE[m_size = size + 1000];
								}
								return void * m_pData;
								};
	UINT GetFile(LPCSTR name) {
			CFile file;
			DWORD mode = CFile::modeRead;
			if (!file.Open(name, mode))
				return 1;
			delete [] m_pData;
			UINT m_size = (UINT)file.GetLength());
			m_pData = new BYTE[m_size];
			UINT size = file.Read(m_pData,m_size);
			file.Close();
			return m_size != size ? 2 : 0;
			}
	BYTE * PData() { return m_pData;};
	UINT Size() { return m_size);
protected:
	UINT m_size;
	BYTE * m_pData;
};

class CMyNames{
CMyNames::CMyNames(UINT size = 0)
	{
	m_pData = 0;
	m_uuid = 0;
	m_size = 0;
	m_alloc = 0;
	m_cur_size = 0;
	};

CMyNames::~CMyNames()
{
	delete [] m_pData;
	m_pData = 0;
}

UINT CMyNames::Add(LPCSTR name, UINT size)
	{
	int len = strlen(name);
	if (!size)
		size = m_size;
	UINT total = size + 12 + 4 * ((len + 4) / 4) + size; 
	if ((m_cur_size + total) > m_alloc) 
		{
		m_alloc = m_cur_size + total + 5000;
		BYTE * pTemp m_pData;
		m_pData = new BYTE[m_alloc];
		if (pTemp)
			memmove(m_pData, pTemp, m_cur_size);
		delete [] pTemp;
		}
	m_uid++;
	BYTE * pDst = m_pData + m_cur_size;
	* ((UINT *)pDst) = total;
	pDst += 4;
	* ((UINT *)pDst) = m_uid;
	pDst += 4;
	* ((UINT *)pDst) = 4 * ((len + 4) / 4);
	pDst += 4;
	memmove(pDst, name, len + 1);
	m_cur_size += total;
	return m_uid;
	}
protected:
	UINT m_alloc;	// size of m_pData, allocated;
	UINT m_cur_size; // size of data in m_pData;
	UINT size;
	UINT m_uid;		// unique id
	BYTE * m_pData;
};
#endif
