#ifndef _MYDB_H_
#define _MYDB_H_

#include "CommonDefs.h"

//
//	internal error codes
//
#define ERR_BAD_TYPE 1
#define ERR_NULL_INDEX 2
#define ERR_NO_OPEN 3
#define ERR_BAD_WRITE 4
#define ERR_BAD_READ 5
#define ERR_BAD_SEEK 6
#define ERR_BAD_HEADER 7
#define ERR_BIG_TYPE 8
#define ERR_BIG_INDEX 9
#define ERR_ALREADY_DELETED 10
#define ERR_REC_HEADER 11
#define ERR_BAD_SIZE 12
#define ERR_TOO_BIG 13
#define ERR_DELETED 14
#define ERR_BAD_KEY 15
#define ERR_NOT_FOUND 16
#define ERR_NOT_CLOSED 17
#define ERR_BAD_CHAIN 18
#define ERR_NO_MEMORY 19
#define ERR_BAD_FAT 20
#define ERR_NO_CREATE 21
#define ERR_BAD_CLUSTER 22
#define ERR_SHORT_FAT 28
#define ERR_FAT_FLINK 29
#define ERR_BAD_DELETE 30
#define ERR_BIG_DELETE 31


typedef struct {
	DWORD   dwKey;
	DWORD   dwSize;
	DWORD   dwCluster;
} KEYENTRY;

class CMyDB
{
public:
	CMyDB();
	~CMyDB();
	int Open(LPCSTR name,
			DWORD dwInitKey = 0, DWORD clusters = 0);
	int Test(LPCSTR name);
	int Close();
	int Erase();
	int GetRecord(void * pRec, DWORD dwSize, DWORD dwKey, DWORD dwOffset = 0);
	int GetSize(DWORD& dwSize, DWORD dwKey);
//	int ModRecord(void * pRec, DWORD dwSize, DWORD dwKey, DWORD dwOffset = 0);
	int PutRecord(void * pRec, DWORD dwSize, DWORD dwKey,
			DWORD offset = 0,DWORD actual = 0);
	int DelRecord(DWORD dwKey);
	DWORD MyKind() { return m_dwKind;};
//	DWORD NextId() { return m_dwNext;};
	DWORD NextKey(bool bIncrement = TRUE);
	DWORD RecordCount() { return m_count;};
	int	GetKey (DWORD& dwKey, DWORD Index)
			{ dwKey = m_table[Index].dwKey;return 0;};
	int	GetIndexedSize (DWORD& dwSize, DWORD Index)
			{ dwSize = m_table[Index].dwSize;return 0;};
	bool Modified() { return m_bModified;};
//	int Fragmentation();
	bool Opened() { return m_bOpened;};
	int RecordInfo(DWORD& key, DWORD& size, DWORD& adr, UINT index);
	DWORD ID() { return m_id;};
	bool ReadOnly() {return m_bReadOnly;};
	void UnAppend();
//	UINT Appended(LPBYTE pData=0);	// 0 is wave
	int	AppendFile(LPCSTR name, bool bErase);
protected:
	int ReadChain(void * pData, DWORD dwSize, DWORD dwOffset);
	int WriteChain(void * pData, DWORD dwSize, DWORD dwOffset, DWORD actual);
	int VerifyFile();
	int VerifyChain(DWORD Index, DWORD clust, DWORD c, DWORD * pFat);
	CFile m_file;
	bool	m_bModified;
	bool	m_bOpened;
	bool	m_bReadOnly;
	DWORD	m_id;
	bool		m_bCopying;
	DWORD	m_dwKind;
	DWORD	m_dwNext;
	DWORD	m_dwFlags;
	DWORD	m_dir;
	DWORD	m_count;
	DWORD	m_maxtable;
	DWORD	m_fatpos;
	DWORD	m_fatsize;
	KEYENTRY * m_table;
	DWORD * m_fat;
	int 	m_result;
	DWORD	m_index;
	DWORD	m_cluster;
	int Create(LPCSTR lpName, DWORD dwInitKey,DWORD clusters);
	int	UpdateHeader();
	int	UpdateTable();
	int	UpdateFAT();
	char m_szName[256];
	int Read(void * buf, DWORD size, DWORD pos = NEGONE);
	int Write(void * buf, DWORD size, DWORD Pos = NEGONE);
	int Seek( DWORD pos);
	int NextCluster();
	int Find(DWORD dwKey);
	int CheckChain(DWORD dwSize);
	int EnsureTable();
	int Ensure(DWORD size);
	int EnsureLinear(DWORD size);
	bool CheckLinear(DWORD cluster, DWORD size);
	int DeleteChain(DWORD cluster);
friend class CMyIO;
};
#endif
