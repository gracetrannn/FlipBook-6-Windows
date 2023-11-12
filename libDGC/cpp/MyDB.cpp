//#include "resource.h"       // main symbols
#include "MyDB.h"
#include <exception>

//#define LOGGING

//
//	the following macro is used to generate a 4 byte id field ala MCI
//
#define mmioFOURCC( ch0, ch1, ch2, ch3 ) \
	( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |	\
	( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )
#define DGCID	mmioFOURCC('D', 'G', 'C', 26)

//
//	header version
//
#ifdef _qDISNEY
#define STUFF 255
#else
#define STUFF 5
#endif
#define FAT_KEY 1
#define DIR_KEY 2
#define FIRST_KEY 1024
#define CLUSTER_SIZE 1024
#define INITIAL_CLUSTERS 1024
#define TABLEPAD 16		// extra space in table
//
//
typedef struct {
	DWORD   dwId;			// always a constant
	DWORD	dwStuff;		// the above version
	DWORD	dwFlags;		// e.g. closed
	DWORD	dwKind;			// unique id for this kind of file
	DWORD	dwNext;			// next record id
	DWORD   dwDir;			// cluster of directory
	DWORD   dwKeys;			// how many entries
	DWORD	dwFat;			// cluster of FAT
	DWORD	dwFatSize;		// size of FAT
} MYHEADER;


#ifdef LOGGING
CMyDB * zz;
DWORD which = 0;
bool bLogging = 0;
//char name[256];
void FARCDECL dpl( LPSTR szFormat, ...)
{
	if (!bLogging) return;
	char amsg[280];
	CFile file;
	DWORD mode = CFile::modeWrite;// | CFile::modeCreate;
//	strcpy(amsg,name);
	strcpy(amsg,"c:\\programdata\\DigiCel");
	strcat(amsg,"\\log.txt");
	if (!file.Open(amsg, mode))
		{
		mode |= CFile::modeCreate;
		if (!file.Open(amsg, mode))
			return;
		}
	wsprintf(amsg,"%3d %10u:",zz->ID(),GetTickCount());
	wvsprintf(amsg+15, szFormat,(LPSTR)(&szFormat+1));
DPF2("log:%s",amsg);
	strcat(amsg, "\r\n");
	file.SeekToEnd();
	file.Write(amsg, strlen(amsg));
	file.Close();
}
	#define DPL zz = this; dpl
#else
#endif
#define DPG(a,b) dpt(a,b,m_table[b].dwPos,m_table[b].dwKey,m_table[b].dwSize)
#ifdef MYBUG
	#define DPT DPG
#else
#endif
#ifdef MYBUG
void dpt(LPSTR sztext, DWORD index, DWORD p, DWORD k, DWORD s)
{
DPF2("%s,idx:%ld,pos:%ld,key:%lx,siz:%ld",sztext,index, p,k,s);
}
#endif

//
//	the following macro is used to generate a 4 byte id field ala MCI
//
#define mmioFOURCC( ch0, ch1, ch2, ch3 ) \
	( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |	\
	( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )
#define DGCID	mmioFOURCC('D', 'G', 'C', 26)

//
//	construct clears essential variables
//
CMyDB::CMyDB()
{
	m_bModified = FALSE;
	m_bOpened = FALSE;
	m_dir = 0;
	m_count = 0;
	m_fat = 0;
	m_table = 0;
	m_bCopying = 0;
}
//
//	destruct releases resources
//
CMyDB::	~CMyDB()
{
DPF2("my db destruct");
	Close();
	delete [] m_table;
	delete [] m_fat;
}

int CMyDB::Close()
{
DPF2("closing,%d,mod:%d",m_bOpened,m_bModified);
	if (m_bCopying)
		{
		m_bCopying = 0;
		UpdateHeader();
		UpdateTable();
		UpdateFAT();
		}
	if (m_bOpened)
		m_file.Close();
	m_bOpened = FALSE;
	return 0;
}

int CMyDB::Erase()
{
DPF2("erase:%d,%s|",m_bOpened,m_szName);
	if (m_bOpened)
		return ERR_NOT_CLOSED;

	try {
		m_file.Remove(m_szName);
	} catch(std::exception&) {
		DPF2("bad erase:%s|", m_szName);
	}

	return 0;
}

int CMyDB::Test(LPCSTR lpName)
{
	MYHEADER header;
	DPF2("test,name:%s",lpName);
	m_result = ERR_NO_OPEN;
	m_bOpened = FALSE;
	DWORD mode = CFile::modeRead;
	if (!m_file.Open(lpName, mode))
		return m_result;
	m_result = ERR_BAD_READ;
	for (;;)
		{
		if (Read(&header, sizeof(header)))
			break;
		DOSWAP(header);
		m_result = ERR_BAD_HEADER;
		if (header.dwId != DGCID)
			break;
		if (header.dwStuff != STUFF)
			break;
		m_result = 0;
		break;
		}
	m_file.Close();
	DPF2("test:%d",m_result);
	return m_result;
}

int CMyDB::Open(LPCSTR lpName, DWORD dwInitKey /* = 0 */, 
					DWORD clusters /* = 0 */)
{
	MYHEADER header;
#ifdef LOGGING
	m_id = which++;
//	strcpy(name,lpName);
#endif
	strcpy((LPSTR)&m_szName,lpName);
	DPF2("open,initkey:%ld,name:%s",dwInitKey,m_szName);
	DPL("open,initkey:%ld,name:%s",dwInitKey,lpName);
	m_bReadOnly = FALSE;
	if (dwInitKey)
		return Create(lpName,dwInitKey,clusters);
	m_result = ERR_NO_OPEN;
	DWORD mode = CFile::modeReadWrite;
	if (!m_file.Open(lpName, mode))
		{
		m_bReadOnly = TRUE;
		mode = CFile::modeRead;
		if (!m_file.Open(lpName, mode))
            //printf("Error opening file: %s, %s\n", strerror(errno), lpName);
			return m_result;
		}
	m_bOpened = TRUE;
	m_result = ERR_BAD_READ;
	for (;;)
		{
		if (Read(&header, sizeof(header)))
			break;
		DOSWAP(header);
		m_result = ERR_BAD_HEADER;
		if (header.dwId != DGCID)
			break;
		if (header.dwStuff != STUFF)
			break;
		m_dwFlags = header.dwFlags;
		m_dwKind = header.dwKind;
		m_dwNext = header.dwNext;
		m_dir = header.dwDir;
		m_count = header.dwKeys;
		m_fatpos = header.dwFat;
		m_fatsize = header.dwFatSize;
		DWORD tblclusters = 
			(sizeof(KEYENTRY) * (m_count + 2) + CLUSTER_SIZE - 1) / CLUSTER_SIZE;
		m_maxtable = (tblclusters * CLUSTER_SIZE) / 12;
		m_result = ERR_NO_MEMORY;
		delete [] m_table;
		m_table = new KEYENTRY[m_maxtable+1];
		if (!m_table)
			break;
		delete [] m_fat;
		m_fat = new DWORD[m_fatsize];
		if (!m_fat)
			break;
		if (Read(m_fat, 4 * m_fatsize, CLUSTER_SIZE * m_fatpos))
			break;
		DOSWAPC(m_fat, 4 * m_fatsize);
DPL("tbl pos:%ld,count:%ld",header.dwDir,m_count);
		m_result = ERR_BAD_READ;
		DWORD dwSize = m_count * sizeof(KEYENTRY);
		m_cluster = m_dir;
		memset (m_table,0, dwSize);
		m_result = ReadChain(m_table, dwSize,0);
		DOSWAPC(m_table, dwSize);
//		
//		fix next key bug
//
		DPL("next key:%lu,%lx",m_dwNext,m_dwNext);
		UINT i,q;
		q = m_dwNext;
		for (i = 0; i < m_count;i++)
			if (m_table[i].dwKey >= m_dwNext)
				m_dwNext = m_table[i].dwKey + 1;
		DPL("after fix next key:%lu,%lu,%lx",q,m_dwNext,m_dwNext);
#ifdef _DEBUG
		if (q != m_dwNext)
			DPF2("after fix next key:%lu,%lu,%lx",q,m_dwNext,m_dwNext);
#endif
		m_result = 0;
		break;
		}
	if (!m_result)
		m_result = VerifyFile();
	m_bModified = FALSE;
	DPL("open result:%ld",m_result);
	return m_result;
}

int CMyDB::Create(LPCSTR lpName, DWORD dwInitKey, DWORD clusters)
{
//bLogging = 1;
	m_result = ERR_NO_CREATE;
	DWORD mode = CFile::modeCreate | CFile::modeReadWrite;
	if (!m_file.Open(lpName, mode))
		return m_result;
	m_bOpened = TRUE;
	m_dwKind = 0;
	if (dwInitKey > 1)
		m_dwNext = dwInitKey;
	else
		m_dwNext = FIRST_KEY;
	m_dwFlags = 0;
	for (;;)
		{
//m_dir = header.dwDir;
		m_count = 0;
		if (!clusters)
			clusters = INITIAL_CLUSTERS;
		else
			m_bCopying = TRUE;
DPL("clusters for fat:%d",clusters);
		clusters *= 4;	// byte size
		clusters  = (clusters + CLUSTER_SIZE - 1) / CLUSTER_SIZE;
		m_fatsize = clusters * (CLUSTER_SIZE / 4);
DPL("fat size:%d,%d",m_fatsize,clusters);
//		m_fatsize = clusters;
		DWORD tblclusters = 
			(sizeof(KEYENTRY) * (m_count + 2) + CLUSTER_SIZE - 1) / CLUSTER_SIZE;
		m_maxtable = (tblclusters * CLUSTER_SIZE) / 12;
//		m_maxtable = m_count+TABLEPAD;
		m_result = ERR_NO_MEMORY;
		delete [] m_table;
		m_table = new KEYENTRY[m_maxtable+1];
		if (!m_table)
			break;
		delete [] m_fat;
		m_fat = new DWORD[m_fatsize];
		if (!m_fat)
			break;
		UINT i;
		for (i = 0; i < m_fatsize; m_fat[i++] = 0);
		m_fat[0] = -1; 			// the header, gone forever
		m_fat[m_dir = 1] = -1;	// the table
		m_fatpos = 2;
		UINT j = m_fatpos + clusters - 1;
		for (i = m_fatpos; i < j; i++)
			m_fat[i] = i + 1;
		m_fat[i] = -1;
		if (UpdateFAT())
			break;
		break;
		}
	UpdateHeader();
	m_bModified = FALSE;
	DPL("create result:%d",m_result);
	return m_result;	
}


//
//	writes a new header, usally with an updated "next key"
//
int CMyDB::UpdateHeader()
{
	if (m_bCopying)
		return m_result = 0;
	MYHEADER header;
	header.dwId = DGCID;
	header.dwStuff = STUFF;
	header.dwFlags = 0;
	header.dwKind = m_dwKind;
	header.dwNext = m_dwNext;
//	if (m_dir)
//		header.dwFlags |= 1;
	header.dwDir = m_dir;
	header.dwKeys = m_count;
	header.dwFat = m_fatpos;
	header.dwFatSize = m_fatsize;
	DOSWAP(header);
	m_result = Write(&header, sizeof(header),0);
	DOSWAP(header);
	DPL("update result:%d,next:%d",m_result,m_dwNext);
	return m_result;
}

int CMyDB::Find(DWORD dwKey)
{
	m_table[m_count].dwKey = dwKey;
	for (m_index = 0; m_table[m_index].dwKey != dwKey;m_index++);
	if (m_index >= m_count)
		m_result = ERR_NOT_FOUND;
	else
		{
/*
		if (m_table[m_index].dwKey >= m_dwNext)
			{
DPL("found but new key:%x,%d",m_table[m_index].dwKey,m_table[m_index].dwKey);
			m_dwNext = m_table[m_index].dwKey + 1;
			UpdateHeader();
			}
*/
		m_result = 0;
		}
	return m_result;
}

DWORD CMyDB::NextKey(bool bIncrement)
{
	if (!bIncrement)
		return m_dwNext;
	DWORD res =	m_dwNext++;
	UpdateHeader();
DPL("new key:%08X",res);
	return res;
}

int CMyDB::RecordInfo(DWORD& key, DWORD& size, DWORD& adr, UINT index)
{
	if (index >= m_count)
		return (m_result = ERR_NOT_FOUND);
	key =  m_table[index].dwKey;
	size = m_table[index].dwSize;
	adr  = m_table[index].dwCluster;
	return 0;
}
int CMyDB::GetSize(DWORD &dwSize, DWORD dwKey)
{
	DPL("get size,key:%8lx",dwKey);
	if (Find(dwKey))
		return m_result;
	dwSize = m_table[m_index].dwSize;
	return 0;
}

int CMyDB::ReadChain(void * pChain, DWORD dwSize, DWORD dwOffset)
{
DPL("read chain, cluster:%d,size:%d,off:%d",m_cluster,dwSize,dwOffset);
ASSERT(dwOffset < CLUSTER_SIZE);
	if (dwOffset >= CLUSTER_SIZE)
		return ERR_TOO_BIG;
ASSERT(m_cluster < m_fatsize);
	if (m_cluster >= m_fatsize)
		return ERR_BAD_CLUSTER;
	BYTE * pData = (BYTE *)pChain;
	bool bLinear = CheckLinear(m_cluster, dwOffset + dwSize);
	for (;;)
		{
		DWORD size;
		if (!bLinear && ( (dwOffset + dwSize) > CLUSTER_SIZE))
			size = CLUSTER_SIZE - dwOffset;
		else
			size = dwSize;
		if (m_result = Seek(dwOffset + CLUSTER_SIZE * m_cluster))
			break;
		if (m_result = Read(pData,size))
			break;
		if (size == dwSize)
			break;
		if (NextCluster())
			break;
		pData += size;
		dwSize -= size;
		dwOffset = 0;
		}
ASSERT(m_result == 0);
	return m_result;
}
int CMyDB::GetRecord(void * pRec, DWORD dwSize, DWORD dwKey, DWORD dwOffset)
{
	DPL("get rec,key:%8lx,siz:%ld,off:%ld",dwKey,dwSize,dwOffset);
	if (Find(dwKey))
		return m_result;
	m_cluster = m_table[m_index].dwCluster;
	return ReadChain(pRec, dwSize, dwOffset);
}


int CMyDB::WriteChain(void * pChain, DWORD dwSize, DWORD dwOffset, DWORD actual)
{
DPL("write chain, cluster:%d,size:%d,off:%d,act:%d",m_cluster,dwSize, dwOffset,actual);
ASSERT(dwOffset < CLUSTER_SIZE);
	if (dwOffset >= CLUSTER_SIZE)
		return ERR_TOO_BIG;
ASSERT(m_cluster < m_fatsize);
	if (m_cluster >= m_fatsize)
		return ERR_BAD_CLUSTER;
	BYTE * pData = (BYTE *)pChain;
	bool bLinear = CheckLinear(m_cluster, actual);
DPL("cluster:%d,linear:%d",m_cluster,bLinear);
	for (;;)
		{
		DWORD size;
		if (!bLinear && (dwOffset + dwSize) > CLUSTER_SIZE)
			size = CLUSTER_SIZE - dwOffset;
		else
			size = dwSize;
		if (m_result = Seek(dwOffset + CLUSTER_SIZE * m_cluster))
			break;
//DPL("cls:%ld,siz:%ld",m_cluster,size);
		if (m_result = Write(pData,size))
			break;
		if (size == dwSize)
			break;
		if (NextCluster())
			break;
		pData += size;
		dwSize -= size;
		dwOffset = 0;
		}
ASSERT(m_result == 0);
	return m_result;
}

int CMyDB::PutRecord(void * pRec, DWORD dwSize, DWORD dwKey,
			DWORD dwOffset /* = 0 */ , DWORD actual /* = 0 */)
{
	DPL("put rec,key:%8lx,siz:%ld,off:%ld,act:%ld",dwKey,dwSize,dwOffset,actual);
	if (dwOffset >= CLUSTER_SIZE)
		return ERR_TOO_BIG;
	if (!actual)
		actual = dwSize + dwOffset;
	DWORD oldcluster;
	if (Find(dwKey))
		{
		int j = EnsureTable();
ASSERT(j == 0);
		m_index = m_count++;
		m_table[m_index].dwKey = dwKey;
		oldcluster = 0;
		if (dwKey >= m_dwNext)
			m_dwNext = dwKey + 1;
DPL("inserting new key:%d, next:%d",dwKey, m_dwNext);
		}
	else
		{
		oldcluster = m_table[m_index].dwCluster;
		}
	if (!dwOffset)
		{
		if (Ensure(actual))		// m_cluster has start of new chain
			return m_result;
DPL("new cluster:%ld,old:%ld",m_cluster,oldcluster);
		}
	else
		{
		if (!oldcluster)
			{
			return m_result = ERR_TOO_BIG;
DPL("updating non existent");
			}
		m_cluster = oldcluster;
		oldcluster = 0;
		}
	m_table[m_index].dwCluster = m_cluster;
	m_table[m_index].dwSize = actual;
	if (UpdateTable())
		return m_result;
	m_result = WriteChain(pRec, dwSize, dwOffset, actual);
	if (m_result)
		{
DPL("m_result:%d",m_result);
//		delete new chain;
		}
	else
		{
		if (oldcluster)
			DeleteChain(oldcluster);
		}
ASSERT(m_result == 0);
	return m_result;
}

int CMyDB::EnsureTable()
{
DPL("ensure count:%ld,max:%ld",m_count,m_maxtable);
	if ((m_count + 2) < m_maxtable)
		return 0;
DPL("ensure count:%ld,max:%ld",m_count,m_maxtable);
	DWORD olds = (sizeof(KEYENTRY) * m_maxtable + CLUSTER_SIZE - 1) / CLUSTER_SIZE;
	m_maxtable = ((olds + 1) * CLUSTER_SIZE) / sizeof(KEYENTRY);
DPL("olds:%ld,newt:%ld",olds,m_maxtable);
	KEYENTRY * oldtable = m_table;
	m_table = new KEYENTRY[m_maxtable+1];
	if (!m_table)
		{
	DPL("no mem");
		m_table = oldtable;
		return ERR_NO_MEMORY;
		}
	DWORD i;
	for (i = 0; i < m_count; i++)
		{
		m_table[i] = oldtable[i];
		}
	delete [] oldtable;
	if (!Ensure(sizeof(KEYENTRY)* m_maxtable))
		{
		DeleteChain(m_dir);
		m_dir = m_cluster;
DPL("new tbl pos:%ld",m_dir);
		}
	return 0;
}

int CMyDB::UpdateTable()
{
	if (m_bCopying) return m_result = 0;
	DWORD dwSize = m_count*sizeof(KEYENTRY);
	DPL("update Table:%ld,siz:%ld",m_count,dwSize);
	DWORD temp = m_cluster;		// save it
	m_cluster = m_dir;
	DOSWAPC(m_table, dwSize);
	m_result = WriteChain(m_table, dwSize, 0, dwSize);
	DOSWAPC(m_table, dwSize);
	if (!m_result)
		UpdateHeader();
	m_cluster = temp;		// restore it
	return m_result;
}

int CMyDB::UpdateFAT()
{
	DPL("update fat,siz:%ld,pos:%ld",m_fatsize,m_fatpos);
	if (m_bCopying) return m_result = 0;
	DOSWAPC(m_fat, 4*m_fatsize);
	m_result = Write(m_fat,4 * m_fatsize,CLUSTER_SIZE * m_fatpos);
	DOSWAPC(m_fat, 4*m_fatsize);
	return m_result;
}

int CMyDB::DelRecord(DWORD dwKey)
{
	DPL("del record(key:%8lx",dwKey);
	if (Find(dwKey))
		return m_result;
	DWORD cluster = m_table[m_index].dwCluster;
	DWORD i;
	m_count--;
	for (i = m_index; i < m_count; i++)
		m_table[i] = m_table[i+1];
	UpdateTable();
	return DeleteChain(cluster);
}


//
//	utilty routines for read and write which check for errors
//
int CMyDB::Read(void * buf, DWORD size, DWORD pos)
{
	if (pos != -1 && Seek(pos))
		return ERR_BAD_SEEK;
	DWORD siz;
	siz = m_file.Read(buf, size);
	if (siz == size)
		return 0;
#ifdef _DEBUG
	FBMessageBox("bad read");
#endif
	DPL("Read Error,siz:%d,size:%d",siz,size);
	return ERR_BAD_READ;
}

int CMyDB::Write(void * buf, DWORD size, DWORD pos)
{
	if (pos != -1 && Seek(pos))
		return ERR_BAD_SEEK;
DPL("write,size:%ld,pos:%ld",size,pos);
	DWORD siz = size;
	m_file.Write(buf, size);
	m_bModified = TRUE;
	if (siz == size)
		{
//		m_file.Flush();
		return 0;
		}
#ifdef _DEBUG
	FBMessageBox("bad write");
#endif
	DPF2("Write Error");
	return ERR_BAD_WRITE;
}

int CMyDB::Seek( DWORD pos)
{
DPL("seek:%ld",pos);
	m_file.Seek(pos,CFile::begin);
	return 0;
}
int CMyDB::NextCluster()
{
#ifdef _DEBUG
	DWORD old = m_cluster;
#endif
	if (m_cluster >= m_fatsize)
		m_result = ERR_BAD_CHAIN;
	else
		{
		m_cluster = m_fat[m_cluster];
		if (m_cluster == -1)
			m_result = ERR_SHORT_FAT;
		else if (m_cluster == 0)
			m_result = ERR_FAT_FLINK;
		else if (m_cluster >= m_fatsize)
			m_result = ERR_BAD_CHAIN;
		else
			m_result = 0;
		}
#ifdef _DEBUG
	if (m_result)
		{
char buf[80];
	sprintf(buf,"bad next cluster,%x,%x,%d",m_cluster,old,m_result);
	FBMessageBox(buf);
		}
#endif
	return m_result;
}

bool CMyDB:: CheckLinear(DWORD cluster, DWORD size)
{
	DWORD clusters = (size + CLUSTER_SIZE - 1) / CLUSTER_SIZE;
	UINT i, j;
	for (i = cluster,j = cluster + clusters; i < j; i++)
		{
		if (m_fat[i] == -1)
			{
			if ((i + 1) == j)
				return TRUE;
			break;
			}
		if (!m_fat[i])
			{
			DPF2("zero chain in check lin,i:%d",i);
			}
		if (m_fat[i] != (i + 1))
			break;
		}
	return FALSE;
}

int CMyDB::EnsureLinear(DWORD size)
{
	DWORD clusters = (size + CLUSTER_SIZE - 1) / CLUSTER_SIZE;
	DPL("ensurelinear:%ld,cls:%ld",size,clusters);
	UINT i, j;
	m_cluster = 0;
	m_result = ERR_BAD_FAT;
	for (j =0, i = clusters - 1; i < m_fatsize;)
		{
		for (j = 0; j < clusters;j++)
			if (m_fat[i-j])
				break;
		if (j >= clusters)
			break;
		i += clusters - j;
		}
	if (j >= clusters)
		{
		m_cluster = i + 1 - j;
//		for (j = m_cluster; j <= i;j++)
//			m_fat[j] = j+1;
//		m_fat[j-1] = -1;
		for (j = m_cluster; j < i;j++)
			m_fat[j] = j+1;
		m_fat[j] = -1;
		UpdateFAT();
		m_result = 0;
		}
	return m_result;
}
//
//	ensures and creates a chain of length size
// and sets m_cluster to it
//
int CMyDB::Ensure(DWORD size)
{
	if (!size)
		return m_result = ERR_BAD_FAT;
	if (!EnsureLinear(size))
		return m_result;
	DWORD clusters = (size + CLUSTER_SIZE - 1) / CLUSTER_SIZE;
	DPL("Ensure:%ld,cls:%ld",size,clusters);
	UINT i, j;
	m_cluster = 0;
	for (i = 0,j = clusters; j && (i < m_fatsize); i++)
		if (m_fat[i] == 0)
			{
			if (!m_cluster)
				m_cluster = i;
			j--;
			}
	if (!j)
		{
		i = j = m_cluster;
		if (clusters-- == 1)
			{
			m_fat[i] = -1;
			}
		i++;
		for (; clusters && (i < m_fatsize); i++)
			if (m_fat[i] == 0)
				{
				m_fat[j] = i;
//DPL("chianing,j:%ld,i:%ld",j,i);
				if (clusters-- == 1)
					{
					m_fat[i] = -1;
					break;
					}
				j = i;
				}
		if (i >= m_fatsize)
			{
			DPL("fat err");
			FBMessageBox("fat err 1");
			m_result = ERR_BAD_FAT;
			}
		else
			UpdateFAT();
		return m_result;
		}
	DWORD old = (4 * m_fatsize + CLUSTER_SIZE - 1) / CLUSTER_SIZE;
//	DWORD exp = old + (4 * clusters + CLUSTER_SIZE - 1) / CLUSTER_SIZE;
//
//	douboled extra
//
	DWORD exp = (4 * clusters + CLUSTER_SIZE - 1) / CLUSTER_SIZE;
	exp += old;
	exp += (4 * exp * clusters + CLUSTER_SIZE - 1) / CLUSTER_SIZE;

//	DWORD exp = old + 1 + (old / 256) + (4 * clusters + CLUSTER_SIZE - 1) / CLUSTER_SIZE;
	

DPL("expanding fat,was:%ld,willbe:%ld",old,exp);
	DWORD * oldfat = m_fat;
	DWORD newsize = (exp * CLUSTER_SIZE) / 4;
	m_fat = new DWORD[newsize];
	for (i = 0; i < m_fatsize; i++)
		m_fat[i] = oldfat[i];
	DeleteChain(m_fatpos);
	m_fatpos = m_fatsize;
	for (;i < (m_fatsize + exp - 1);i++)
		m_fat[i] = i+1;
	m_fat[i++] = -1;	// allocate fat
#define SOMEDAY
	m_fatsize = newsize;
#ifdef SOMEDAY
	m_cluster = i;
	j = m_cluster + clusters - 1;
	for (;i < j; i++)
		m_fat[i] = i+1;
	m_fat[i++] = -1;
#endif
	for (;i < m_fatsize;m_fat[i++] = 0);
	delete [] oldfat;
	UpdateFAT();
#ifdef SOMEDAY
return 0;
#endif
	return Ensure(size);
}
int CMyDB::DeleteChain(DWORD cluster)
{
DPL("delete chain:%ld",cluster);
	m_result = 0;
	for (;;)
		{
		if (cluster >= m_fatsize)
			{
DPZ("delete err");
ASSERT(0);
			m_result = ERR_BIG_DELETE;
			break;
			}
		DWORD next = m_fat[cluster];
//DPL("cls:%ld,nxt:%ld",cluster,next);
		if (!next)
			{
			m_result = ERR_BAD_DELETE;
DPZ("delete err 2");
ASSERT(0);
			break;
			}
		m_fat[cluster] = 0;
		cluster = next;
		if (cluster == -1)
			break;
		}
DPL("deletechain:%d",m_result);
	if (!m_result)
		UpdateFAT();
	return m_result;
}

int CMyDB::VerifyChain(DWORD index, DWORD clust, DWORD size, DWORD * pFat)
{
	UINT j,n;
	UINT c = (size + CLUSTER_SIZE - 1) / CLUSTER_SIZE;
	if (clust >= m_fatsize)
		{
		DPL("bad first,%d,%x",clust,clust);
		return 1;
		}
	for (j = 0; j < c; j++)
		{
		if (clust >= m_fatsize)
			{
			DPL("big clust,%d,%x",clust,m_fatsize);
			break;
			}
		if (pFat[clust] == -2)
			{
			pFat[clust] = index;
			}
		else
			{
DPL("cross link at clust:%d,%d", clust,pFat[clust]);
			break;
			}	
		n = m_fat[clust];
		if (!n)
			{
			DPL("zero chain,clust:%d, link:%d",clust,n);
			break;
			}
		if (n == -1)
			{
			if ((j+1) < c)
				{
				DPL("premature end");
				break;
				}
			}
		clust = n;
		}
	if (j < c)
		{
DPL("on index:%d",index);
		return 3;
		}
	return 0;
}
int CMyDB::VerifyFile()
{
DPL("verify file");
	m_result = 0;
	UINT i;
	DWORD * pFat = new DWORD [m_fatsize];
	for (i = 0; i < m_fatsize;i++)
		pFat[i] = -2;
	VerifyChain(9998,m_dir,12 * m_count,pFat);
	VerifyChain(9999,m_fatpos,4 * m_fatsize,pFat);
	for (i = 0; i < m_count;i++)
		{
		UINT size = m_table[i].dwSize;
		UINT key =  m_table[i].dwKey;
		UINT clust= m_table[i].dwCluster;
		if (VerifyChain(i,clust,size,pFat))
			m_result = ERR_BAD_CLUSTER;
		}
	delete [] pFat;
#ifdef _DEBUG
	if (m_result)
		FBMessageBox("bad verify");
#endif
	return m_result;
}
/*
bool CMyDB::ReadOnly()
{
	if (m_bReadOnly)
		return TRUE;
	CFileStatus status;
	m_file.GetStatus(status);
	if (status.m_attribute & CFile::readOnly)
		return 1;
	else
		return 0;
}
*/


typedef struct {
	char	id[8];
	UINT	version;
	UINT	size;
} APPENDEDHEADER;

void CMyDB::UnAppend()
{
	APPENDEDHEADER hdr;
	UINT dwLen = (UINT)m_file.SeekToEnd();
	UINT pos = dwLen;
	pos -= sizeof(hdr);
	Seek(pos);
	DWORD siz;
	
	siz = m_file.Read(&hdr, sizeof(hdr));
#ifdef _NEEDSWAP
	hdr.version = SWAPV(hdr.version);
	hdr.size = SWAPV(hdr.size);
#endif
	if ((hdr.id[0] != 'A') || (hdr.id[1] != 'P') || (hdr.id[2] != 'P') ||
		(hdr.id[3] != 'E') || (hdr.id[4] != 'N') || (hdr.id[5] != 'D')	||
		(hdr.id[6] != 'E') || (hdr.id[7] != 'D'))
		return;
	if ((hdr.version != 1) && (hdr.version != 2))
		return;
	pos -= hdr.size;
	m_file.SetLength((ULONGLONG)pos);
	return;
}

int	CMyDB::AppendFile(LPCSTR name, bool bJustErase)
{
	int v = 0;
	UINT total = 0;
	if (!bJustErase)
	{
		m_file.SeekToEnd();
		DWORD mode = CFile::modeRead;
		CFile file;
		if (!file.Open(name, mode))
		{
			FBMessageBox("Cannot Open DGY File");
			return 1;
		}
		UINT size;
		total = 0;
		BYTE* pData = new BYTE[10000];
		do {
			size = file.Read(pData, 10000);
			m_file.Write(pData, size);
			total += size;
		} while (size > 0);
		file.Close();
	}

	try {
		CFile::Remove(name);
	} catch(std::exception&) {
		v = 2;
	}

	if (!bJustErase)
		{		
		APPENDEDHEADER hdr;
		strcpy(hdr.id,"APPENDED");
		hdr.version = SWAPV(2);
		hdr.size = SWAPV(total);
		m_file.Write(&hdr, sizeof(hdr));
		}	
	return v;
}
