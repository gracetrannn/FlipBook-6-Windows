#ifndef _MYIO_H_
#define _MYIO_H_

#include "CommonDefs.h"

class CMyDB;
class CEmbedded;
class CMyIO
{
public:
	CMyIO();
	~CMyIO();
	int Open(LPCSTR name);
	int Test(LPCSTR name);
	int Create(LPCSTR name);
	int Merge(LPCSTR name);
	int Save(LPCSTR name = 0, bool bErase = 0);
	int Close(bool NoErase = 0);
	int Wipe(bool bErase);
	int GetRecord(void * pRec, DWORD dwSize, DWORD dwKey, DWORD dwOff = 0);
	int GetSwapRecord(void * pRec, DWORD dwSize, DWORD dwKey);
	int GetSize(DWORD& dwSize, DWORD dwKey);
	int PutSwapRecord(void * pRec, DWORD dwSize, DWORD dwKey);
//	int ModRecord(void * pRec, DWORD dwSize, DWORD dwKey, DWORD dwOff);
	int PutRecord(void * pRec, DWORD dwSize, DWORD dwKey,
			DWORD offset = 0,DWORD actual = 0);
	int DelRecord(DWORD dwKey);
	LPCSTR Name() { return m_main;};
	DWORD NewKey(DWORD kind = 0);
#define EMB_KIND_SOUND 0
#define EMB_KIND_PALETTE 1
#define EMB_KIND_MODEL 2
#define EMB_KIND_TEXTURE 3
//
// embedded stuff
//
	UINT EmbFind(LPCSTR name, UINT kind,
					bool bAllowDisabled = 0); // -1 is not found, else index
	bool EmbName(UINT index, LPSTR name);
	UINT EmbKind(UINT index);
	UINT EmbData(UINT index, LPBYTE pData);
// index == 999 -> reset all 
// index == 998 -> return true if any set
	#define EMB_RESET_ALL 999
	#define EMB_ANY_SET   998
	#define EMB_ANY_IN_MEMORY  997
	#define EMB_SET_ALL   996

	#define EMB_ENABLED 0    // if embedded and ne, else still external
	#define EMB_DO_SAVE 1	  // set by prompt when saving
	#define EMB_SEEN  2	    // used to avoid multiple prompts

	bool InitEmbedding();
	bool EmbFlag(UINT index, int which, int val = 0); // 0 is check, 1 is set, 2 is off
	bool EmbAdd(LPCSTR name, UINT kind);
	bool EmbAppend(bool bJustEraseDGY);
	UINT EmbUnHook(LPCSTR pName, bool bHaveSome); // make temp file, schrink dgc
//	DWORD MyKind() { return m_dwKind;};
//	DWORD NextId() { return m_dwNext;};
//
//	for testing
//
	bool ReadOnly();
	UINT RecordCount(UINT which);
	int RecordInfo(DWORD& key, DWORD& size, 
				DWORD& adr, DWORD& kind, UINT index, UINT which);
protected:
	int Combine();
	void MakeNames(LPCSTR name);
	int CopyRecords(bool bDelete);
	int AddDeleted(DWORD dwKey);
	int ChkDeleted(DWORD dwKey);
//	int	Append(void * pFormat, UINT fmtsize, void * pData, UINT size);
//	UINT Appended(LPBYTE pData = 0);
	char m_main[350];
	char m_back[350];
	bool m_bDontErase;
	bool m_bNewBack;
	CMyDB * m_pMain;
	CMyDB * m_pBack;
	DWORD m_dwDeleted;
	DWORD * m_pDeleted;
	UINT m_nEmbFiles;
	CEmbedded * m_pEmbedded;
};
#endif
