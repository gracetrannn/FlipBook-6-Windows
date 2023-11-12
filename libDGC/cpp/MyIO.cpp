//#include "resource.h"       // main symbols
#include "MyIO.h"
#include "MyDB.h"
#include "utils.h"

#define KEY_DELETED 20

#define mmioFOURCC( ch0, ch1, ch2, ch3 ) \
	( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |	\
	( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )
#define DGYID	mmioFOURCC('D', 'G', 'Y', 26)

typedef struct {
	DWORD   dwId;			// always a constant
	DWORD	version;
	DWORD	count;			// number of objects
} EMBED_HEADER;

typedef struct {			// in dgy file and scene
	DWORD   size;
	DWORD	kind;
	char	name[300];
} EMBED_OBJ_HEADER;

typedef struct embentry {	// in memory
	UINT	kind;
	UINT	flags;
	UINT	size;			// size of data
	UINT	position;		// to obj_header
	char   	name[300];
} echunk;

class CEmbedded : public embentry {};

typedef struct {
	char	id[8];
	UINT	version;
	UINT	size;
} APPENDEDHEADER;



CMyIO::CMyIO()
{
	DPF2("myio construct");
	m_pMain = new CMyDB();
	m_pBack = new CMyDB();
	m_dwDeleted = 0;
	m_pDeleted = 0;
	m_bDontErase = 0;
	m_bNewBack = 0;
	m_nEmbFiles = 0;
	m_pEmbedded = 0;
}

CMyIO::~CMyIO()
{
	DPF2("myio destruct");
	Close();
	delete m_pMain;
	delete m_pBack;
	delete [] m_pEmbedded;
}

bool CMyIO::ReadOnly()
{
	return m_pMain->ReadOnly();
}

int CMyIO::Close(bool NoErase /* = 0 */)
{
DPF2("myio close");
	if (NoErase)
		{
		m_bDontErase = TRUE;
		return 0;
		}
	DWORD dwCurKey1 = m_pMain->NextKey(FALSE);
	DWORD dwCurKey2 = m_pBack->NextKey(FALSE);
DPZ("cur key1:%d,key2:%d",dwCurKey1, dwCurKey2);
	m_pMain->Close();
	if (m_pBack->Opened())
		{
		m_pBack->Close();
//		if (!m_pBack->Modified())
		if (dwCurKey2 == dwCurKey1)
			m_pBack->Erase();
		else if (m_bNewBack && m_bDontErase) // dgx made by test or preview
			m_pBack->Erase();
		}
//	if (!m_bDontErase && (dwCurKey1 == 1024) && (dwCurKey2 == 1024))
//		m_pMain->Erase();
	delete [] m_pDeleted;
	m_dwDeleted = 0;
	m_pDeleted = 0;
	return 0;
}

void CMyIO::MakeNames(LPCSTR name)
{
	int i,j;
	for (i = 0, j = 0; m_main[i] = name[i]; i++)
		{
		m_back[i] = name[i];
		if (name[i] == '.')
			j = i + 1;
		}
	m_back[j] =0;
#ifdef _DISNEY
	strcat(m_back,"cbx");
#else
	strcat(m_back,"dgx");
#endif
	DPF2("main:%s",m_main);
	DPF2("back:%s",m_back);
}

int CMyIO::Test(LPCSTR name)
{
	return m_pMain->Test(name);
}

int CMyIO::Open(LPCSTR name)
{
DPF2("myio open:%s",name);
	int nResult;
	MakeNames(name);
	nResult = m_pMain->Open(m_main);
	DWORD dwOrigKey = m_pMain->NextKey(FALSE);
DPF2("result:%d",nResult,dwOrigKey);
//	if (nResult)
//		return 1;
	if (m_pMain->ReadOnly())
		{
		FBGetTempPath(sizeof(m_back),m_back);
		int i,j;
		for (i = 0, j = 0; m_main[i]; i++)
			{
//			if (m_main[i] == NATIVE_SEP_CHAR)
            if (m_main[i] == NATIVE_SEP_CHAR_WIN32 || m_main[i] == NATIVE_SEP_CHAR_NWIN32)
				j = i + 1;
			}
//		strcat(m_back,"\\");
		strcat(m_back,m_main+j);
		for (i = 0, j = 0; m_back[i]; i++)
			{
			if (m_back[i] == '.')
				j = i + 1;
			}
		m_back[j] = 0;
		strcat(m_back,"dgx");
		DPF2("new back:%s",m_back);
		}
	int nResult2 = m_pBack->Open(m_back);
	m_bNewBack = FALSE;
	if (nResult2 == ERR_NO_OPEN)
		{
		nResult2 = m_pBack->Open(m_back,dwOrigKey);
		m_bNewBack = TRUE;
		}
//
//	read delteted list
//
	if (nResult2)
		{
		return 2;
		}
	return nResult;
}

int CMyIO::Create(LPCSTR name)
{
	Close();
	MakeNames(name);
	int nResult = m_pMain->Open(m_main,2);
	if (nResult)
		return 1;
	nResult = m_pBack->Open(m_back,m_pMain->NextKey(FALSE));
	if (nResult)
		return 2;
	return nResult;
	
}

DWORD CMyIO::NewKey(DWORD kind)
{
	return m_pBack->NextKey();
}

int CMyIO::Save(LPCSTR lpName, bool bErase)
{
	int nResult;
//	DPF2("save,%lu",GetTickCount());
	if (lpName)
		{
DPF2("my io save:%s",lpName);
		MakeNames(lpName);
//		DPF2("after make:%lu",GetTickCount());
		if (nResult = Combine())
			return nResult;
		m_pMain->Close();
		if (bErase)
			{
//			m_pBack->Close();
//			m_pMain->Close();
//			m_pBack->Erase();
			m_pMain->Erase();
			}
//		DPF2("after maincopy:%lu",GetTickCount());
		if (nResult = m_pMain->Open(m_main))
			return nResult;
DPF2("now with new name");
		nResult = CopyRecords(0);	// already deleted
		}
	else
		{
		if (bErase)
			{
			m_pBack->Close();
			m_pMain->Close();
			m_pBack->Erase();
			m_pMain->Erase();
			return 0;
			}
		if (m_pMain->ReadOnly())
			return 99;
		nResult = CopyRecords(1);
		}
//	DPF2("after copy:%lu",GetTickCount());
//	nResult = CopyRecords();
DPF2("after copyrecs,result:%d",nResult);
//DPF2("ticks:%lu",GetTickCount());
	if (!nResult)
		nResult = Wipe(0);
//	DPF2("afterwipe:%lu",GetTickCount());
	return nResult;
}

int CMyIO::Wipe(bool bErase)
{
	int nResult;
	m_pBack->Close();
	if (!(nResult = m_pBack->Erase()))
		{
		if (!bErase)
		nResult = m_pBack->Open(m_back,m_pMain->NextKey(FALSE));
		}
	return nResult;
}


int CMyIO::Combine()
{
DPF("copying to %s",m_main);
	CMyDB * pNew = new CMyDB;
	int nResult = pNew->Open(m_main,2,0);
	if (nResult)
		return nResult;
	DWORD max = 2000;
	BYTE * pRec  = new BYTE[max];
	UINT nEntries = m_pMain-> RecordCount();
//PL("entries:%d", nEntries);
//	if (!nEntries) return 99;
	UINT i;
	for (i = 0; i < nEntries; i++)
		{
		DWORD dwKey;
		nResult = m_pMain->GetKey(dwKey, i);
DPF2("idx:%ld,key:%lx,res:%d",i,dwKey,nResult);
		if (nResult)
			break;
		if (!dwKey)
			continue;
		if (dwKey == KEY_DELETED)
			continue;
		if (dwKey == 13) // key clip from mydoc
			continue;
		if (!ChkDeleted(dwKey))
			{
DPF2("have a deleted rec:%d",dwKey);
			continue;
			}
		DWORD dwSize;
		nResult = m_pMain->GetIndexedSize(dwSize, i);
		if (nResult)
			break;
		if (!dwSize)
			{
			continue;
			}
		if (dwSize > max)
			{
			delete [] pRec;
			max = dwSize + 2000;
			pRec  = new BYTE[max];
			if (!pRec)
				{
				nResult = 99;
				break;
				}
			}
		nResult = m_pMain->GetRecord(pRec, dwSize, dwKey);
//DPF2("read:%d",nResult);
		if (!nResult)
			nResult = pNew->PutRecord(pRec, dwSize, dwKey);
		if (nResult)
			break;
//DPL("stat,sz:%d,rz:%d,wz:%d",sz,rz,wz);
		}
	delete [] pRec;
	pNew->Close();
	delete pNew;
	return 0;
}


int CMyIO::CopyRecords(bool bDelete)
{
	DWORD dwCount = m_pBack->RecordCount();
	int nResult = 0;
	DWORD i;
	DWORD maxrec = 2000;
	BYTE * pRecord = new BYTE[maxrec];
DPF2("myio copy rec, count:%ld", dwCount);
	for (i = 0; i < dwCount; i++)
		{
		DWORD dwKey;
//
//	if in deleted list 
//
		nResult = m_pBack->GetKey(dwKey, i);
DPF2("idx:%ld,key:%lx,res:%d",i,dwKey,nResult);
		if (nResult)
			break;
		if (!dwKey)
			continue;
		if (dwKey == KEY_DELETED)
			continue;
		if (dwKey == 13) // key clip from mydoc
			continue;
		if (!ChkDeleted(dwKey))
			{
DPF2("have a deleted rec:%d",dwKey);
			continue;
			}
		DWORD dwSize;
		nResult = m_pBack->GetIndexedSize(dwSize, i);
		if (nResult)
			break;
//DPF2("i:%ld,key:%8lx,size:%ld",i,dwKey,dwSize);
		if (dwSize > maxrec)
			{
			delete [] pRecord;
			maxrec = dwSize+2000;
			pRecord = new BYTE[maxrec];
			if (!pRecord)
				{
				nResult = 99;
				break;
				}
			}
		nResult = m_pBack->GetRecord(pRecord, dwSize, dwKey);
DPF2("read:%d",nResult);
		if (!nResult)
			nResult = m_pMain->PutRecord(pRecord, dwSize, dwKey);
		if (nResult)
			break;
		}
	delete [] pRecord;
	if (bDelete)
		{
DPF2("doing deleted:%d",m_dwDeleted);
		for (i = 0; i < m_dwDeleted;i++)
			{
DPF2("deleting:%d",m_pDeleted[i]);
			m_pMain->DelRecord(m_pDeleted[i]);
			}
		}
	return nResult;
}

int CMyIO::ChkDeleted(DWORD dwKey)
{
	if (!m_pDeleted) return ERR_NOT_FOUND;
	DWORD i;
	for (i = 0; i < m_dwDeleted; i++)
		if (m_pDeleted[i] == dwKey)
			break;
	if (i >= m_dwDeleted)
		return ERR_NOT_FOUND;
	else
		return 0;
}

int CMyIO::AddDeleted(DWORD dwKey)
{
	if (!(m_dwDeleted & 15))
		{
		DWORD * pTemp = new DWORD[m_dwDeleted + 16];
		DWORD i;
		for (i = 0; i < m_dwDeleted; i++)
			pTemp[i] = m_pDeleted[i];
		if (m_pDeleted)
			delete [] m_pDeleted;
		m_pDeleted = pTemp;
		}
	m_pDeleted[m_dwDeleted++] = dwKey;
	DOSWAPC(m_pDeleted, 4*m_dwDeleted);
	m_pBack->PutRecord(m_pDeleted, 4 * m_dwDeleted, KEY_DELETED);
	DOSWAPC(m_pDeleted, 4*m_dwDeleted);
	return 0;
}
int CMyIO::DelRecord(DWORD dwKey)
{
	DWORD size;
DPF2("deleting rec:%d",dwKey);
	if (!ChkDeleted(dwKey))
		return ERR_ALREADY_DELETED;
	m_pBack->DelRecord(dwKey);
	if (!m_pMain->GetSize(size,dwKey))
		AddDeleted(dwKey);
	return 0;
}

int CMyIO::GetSwapRecord(void * pRec, DWORD dwSize, DWORD dwKey)
{
	int res = GetRecord(pRec, dwSize,dwKey,0);
	DOSWAPC(pRec, dwSize);
	return res;
}

int CMyIO::GetRecord(void * pRec, DWORD dwSize, DWORD dwKey, DWORD dwOff)
{
	if (!ChkDeleted(dwKey))
		return ERR_DELETED;
//	check deleted list
	int nResult = m_pBack->GetRecord(pRec, dwSize,dwKey,dwOff);
	if (nResult == ERR_NOT_FOUND)
		{
DPF2("getting from main");
		nResult = m_pMain->GetRecord(pRec, dwSize,dwKey,dwOff);
		}
	else
		{
DPF2("got from back");
		}
DPF2("get record, id:%lx, res:%d",dwKey,nResult);
	return nResult;
}

int CMyIO::GetSize(DWORD& dwSize, DWORD dwKey)
{
//	check deleted list
//	return 
	int nResult = m_pBack->GetSize(dwSize,dwKey);
	if (nResult == ERR_NOT_FOUND)
		nResult = m_pMain->GetSize(dwSize,dwKey);
	return nResult;
}

//int CMyIO::PutRecord(void * pRec, DWORD dwSize, DWORD dwKey)
int CMyIO::PutRecord(void * pRec, DWORD dwSize, DWORD dwKey, DWORD off, DWORD act)
{
//	check deleted list
//	and remove it if there
	int nResult = m_pBack->PutRecord(pRec, dwSize, dwKey, off,act);
DPF2("putting to backup,id:%lx,result:%d",dwKey,nResult);
	return nResult;
}


int CMyIO::PutSwapRecord(void * pRec, DWORD dwSize, DWORD dwKey)
{
	DOSWAPC(pRec, dwSize);
	int res = PutRecord(pRec, dwSize,dwKey,0,0);
	DOSWAPC(pRec, dwSize);
	return res;
}


/*
int CMyIO::ModRecord(void * pRec, DWORD dwSize, DWORD dwKey, DWORD dwOffset)
{
//	check deleted list
//	and remove it if there
	int nResult = m_pBack->ModRecord(pRec, dwSize, dwKey, dwOffset);
DPF2("putting to backup,id:%lx,result:%d",dwKey,nResult);
	return nResult;
}
*/

UINT CMyIO::RecordCount(UINT which)
{
	if (which)
		return m_pBack->RecordCount();
	else
		return m_pMain->RecordCount();
}

int CMyIO::RecordInfo(DWORD& key, DWORD& size,
				DWORD& adr, DWORD& kind, UINT index, UINT which)
{
	DWORD buf[2];
	int z = m_pMain->RecordInfo( key, size, adr,index);
	if (z) return z;
	z = m_pMain->GetRecord(&buf, 8, key);
	DOSWAPC(&buf, 8);
	if (z) return z;
	kind = buf[1];
	if (which == 2) return 0;
	if (key != buf[0])
		return 99;
	return 0;
}

UINT CMyIO::EmbFind(LPCSTR name, UINT kind, 
							bool bAllowDisabled) // -1 is not found, else index
{
	UINT index;
	if (!name) return NEGONE;
	UINT mask = 1 << EMB_ENABLED;
	for(index = 0; index < m_nEmbFiles; index++)
		{
		if (!bAllowDisabled && !(m_pEmbedded[index].flags & mask))
			continue;
		if (!_stricmp(name,m_pEmbedded[index].name))
			{
			if ((kind == NEGONE) || (kind == m_pEmbedded[index].kind))
				break;
			}
		}
	if (index >= m_nEmbFiles)
		index = NEGONE;
	else
		EmbFlag(index,EMB_SEEN,1);
	return index;
}

bool CMyIO::EmbName(UINT index, LPSTR name)
{
	bool bResult = 0;
	if (index < m_nEmbFiles)
		{
		strcpy(name, m_pEmbedded[index].name);
		bResult = 1;
		}
	return bResult;
}

UINT CMyIO::EmbKind(UINT index)
{
	UINT res = NEGONE;
	if (index < m_nEmbFiles)
		res = m_pEmbedded[index].kind;
	return res;
}

UINT CMyIO::EmbData(UINT index, LPBYTE pData)
{
	UINT size = 0;
	if (index < m_nEmbFiles)
		{
		size = m_pEmbedded[index].size;
		if (pData)
			{
			UINT pos = m_pEmbedded[index].position;
			pos += sizeof(EMBED_OBJ_HEADER);
			m_pMain->Seek(pos);
			m_pMain->m_file.Read(pData, size);
			}
		}
	return size;
}
// index == 999 -> reset all 
// index == 996 -> set all 
// index == 998 -> return true if any set
// which 0 is active, 1 is used
bool CMyIO::EmbFlag(UINT index, int which, int val /*= 0*/) // 0 is check, 1 is set, 2 is off
{
	bool bResult = 0;
	UINT mask = 1 << which;
	UINT idx;
	if (index == EMB_RESET_ALL)
		{
		for (idx = 0; idx < m_nEmbFiles; idx++)
			{
			m_pEmbedded[idx].flags |= mask;
			m_pEmbedded[idx].flags ^= mask;
			}
		}
	else if (index == EMB_SET_ALL)
		{
		for (idx = 0; idx < m_nEmbFiles; idx++)
			{
			m_pEmbedded[idx].flags |= mask;
			}
		}
	else if (index == EMB_ANY_SET)
		{
		for (idx = 0; idx < m_nEmbFiles; idx++)
			{
//				TRACE(m_pEmbedded[idx].name);
			if (m_pEmbedded[idx].flags & mask)
				{
				bResult = 1;			
				break;
				}
			}
		}
	else if (index == EMB_ANY_IN_MEMORY)
		{
		for (idx = 0; idx < m_nEmbFiles; idx++)
			{
			if (m_pEmbedded[idx].position)
				{
				bResult = 1;			
				break;
				}
			}
		}
	else
		{
		UINT flags = m_pEmbedded[index].flags;
		if (val == 2) // clear flag
			{
			m_pEmbedded[index].flags |= mask;
			m_pEmbedded[index].flags ^= mask;
			}
		else if (val == 1) // set flag
			{
			m_pEmbedded[index].flags |= mask;
			bResult = 1;
			}
		else if (val == 0)
			{
			UINT flags = m_pEmbedded[index].flags;
			bResult = flags & mask ? 1 : 0;
			}			
		}
	return bResult;
}

bool CMyIO::EmbAdd(LPCSTR name, UINT kind)
{
	if (!(m_nEmbFiles % 8))
		{
		CEmbedded * tp = m_pEmbedded;
		m_pEmbedded = new CEmbedded[m_nEmbFiles + 8];
		if (tp)
			{
			UINT i;
			for (i = 0; i < m_nEmbFiles;i++)
				m_pEmbedded[i] = tp[i];
			}
		}
	m_pEmbedded[m_nEmbFiles].kind = kind;
	m_pEmbedded[m_nEmbFiles].flags = 0;
	m_pEmbedded[m_nEmbFiles].position = 0;
	strcpy(m_pEmbedded[m_nEmbFiles].name,name);
	return m_nEmbFiles++;
}

bool CMyIO::EmbAppend(bool bJustEraseDGY)
{
	char name[300];
	int i,j;
	for (i = 0, j = 0; name[i] = m_main[i]; i++)
		{
		if (name[i] == '.')
			j = i + 1;
		}
	name[j] = 0;
	strcat(name,"dgy");
	i = m_pMain->AppendFile(name,bJustEraseDGY);
	if (i)
		FBMessageBox("bad append");
	return 0;
}

//
//	creates a temp file with dgy suffix containing all embedded data
//
UINT CMyIO::EmbUnHook(LPCSTR pName, bool bHaveSome)
{
	if (!bHaveSome)
		{
		m_pMain->UnAppend();
		return 0;
		}
	#define MAXZ 50000000

	CFile file;
	EMBED_HEADER hdr;
	EMBED_OBJ_HEADER fhdr;
	UINT act_count = 0;
	hdr.dwId = DGYID;
	hdr.version = 0;
	hdr.count = 0; // update later SWAPV(count);
	char name[300];
	char errmsg[500];
	int i,j;
	for (i = 0, j = 0; name[i] = pName[i]; i++)
		{
		if (name[i] == '.')
			j = i + 1;
		}
	name[j] = 0;
	strcat(name,"dgy");
	DWORD mode = CFile::modeCreate | CFile::modeWrite;
	if (!file.Open(name, mode))
		{
		FBMessageBox("Could Not Create Embedded Temp File");
		return 3;
		}
	file.Write(&hdr, sizeof(hdr));
	UINT idx;
	BYTE * pData = 0;
	UINT dsize = 0;
	for (idx = 0; idx < m_nEmbFiles; idx++)
		{
		if (m_pEmbedded[idx].flags & (1 << EMB_DO_SAVE))
			{
	//		if (m_pEmbedded[idx].flags & (1 << EMB_ENABLED))
			if (m_pEmbedded[idx].position)
				{
				UINT size = m_pEmbedded[idx].size;
				size += sizeof(EMBED_OBJ_HEADER);
				if (dsize < size)
					{
					delete [] pData;
					pData = new BYTE[dsize = size];
					}
				m_pMain->Seek(m_pEmbedded[idx].position);
				m_pMain->m_file.Read(pData, size);
				file.Write(pData, size);
				act_count++;
				}
			else
				{
				memset(&fhdr,0,sizeof(fhdr));
				fhdr.kind = SWAPV(m_pEmbedded[idx].kind);
				fhdr.size =  0;
				strcpy(fhdr.name,m_pEmbedded[idx].name);
				if (dsize < 10000)
					{
					delete [] pData;
					pData = new BYTE[dsize = 10000];
					}
				CFile xfile;
				DWORD mode = CFile::modeRead;
				if (!xfile.Open(m_pEmbedded[idx].name, mode))
					{
					sprintf(errmsg,"Could Not Open %s", m_pEmbedded[idx].name);
					FBMessageBox(errmsg);
					continue;
					}
				ULONGLONG dwLength = xfile.GetLength();
				if (dwLength > MAXZ)
					{
					sprintf(errmsg,"Embedded File Too Big, %s, truncated",
									m_pEmbedded[idx].name);
					FBMessageBox(errmsg);
					dwLength = MAXZ;
					}
				fhdr.size =  SWAPV((UINT)dwLength);
				file.Write(&fhdr,sizeof(fhdr));
				UINT size, total;
				for (total = 0; total < MAXZ;total += size)
      				{
					size = xfile.Read(pData, dsize);
					file.Write(pData, size);
					if (!size) break;
					}
				xfile.Close();
				act_count++;
				}
			}
		}
	delete [] pData;
	if (act_count) // update hdr with correct count
		{
		hdr.count = SWAPV(act_count);
		file.Seek(0,CFile::begin);
		file.Write(&hdr, sizeof(hdr));
		}
	file.Close();
	m_pMain->UnAppend();
	return 0;
}

bool CMyIO::InitEmbedding()
{
	APPENDEDHEADER ahdr;
	EMBED_HEADER ehdr;
	EMBED_OBJ_HEADER fhdr;
	UINT dwLen = (UINT)m_pMain->m_file.SeekToEnd();
	UINT pos = dwLen;
	pos -= sizeof(ahdr);
	m_pMain->Seek(pos);
	DWORD siz;
	ahdr.version = 99; // force fail if no read
	siz = m_pMain->m_file.Read(&ahdr, sizeof(ahdr));
	ahdr.version = SWAPV(ahdr.version);
	ahdr.size = SWAPV(ahdr.size);
	if ((ahdr.id[0] != 'A') || (ahdr.id[1] != 'P') || (ahdr.id[2] != 'P') ||
		(ahdr.id[3] != 'E') || (ahdr.id[4] != 'N') || (ahdr.id[5] != 'D')	||
		(ahdr.id[6] != 'E') || (ahdr.id[7] != 'D') || (ahdr.version != 2))
		return 0;
	pos -= ahdr.size;
	m_pMain->Seek(pos);
	m_pMain->m_file.Read(&ehdr, sizeof(ehdr));
	DOSWAP(ehdr);
	if (ehdr.dwId != DGYID)
		return 0;
	if (ehdr.version)
		return 0;
	pos += sizeof(ehdr);
	UINT i;
	for (i = 0; i < ehdr.count;i++)
		{
		m_pMain->Seek(pos);
		m_pMain->m_file.Read(&fhdr, sizeof(fhdr));
		fhdr.size = SWAPV(fhdr.size);
		fhdr.kind = SWAPV(fhdr.kind);
		EmbAdd(fhdr.name, fhdr.kind);
		m_pEmbedded[m_nEmbFiles-1].flags = (1 << EMB_ENABLED);
		m_pEmbedded[m_nEmbFiles-1].position = pos;
		m_pEmbedded[m_nEmbFiles-1].size = fhdr.size;
		pos += sizeof(fhdr);
		pos += fhdr.size;
		}
	return ehdr.count ? 1 : 0;
}
