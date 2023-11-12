
#ifndef _MYID_H_
#define _MYID_H_
class CMyId
{
public:
	CMyId();
	int  GetOsStr(CString &strVersion);
	void GetMac(LPSTR pMac);
	UINT GetNewMac( DWORD  & umac); // returns count
	BOOL ChkNewMac(UINT uMac, UINT count);// true if found
	void GetDiskNbr(CString & serNbr, DWORD & dwId);
	DWORD GetVolume();
	DWORD MemoryLoad() { return m_dwLoad;};
	DWORD TotalPhysical() { return m_dwTotalPhys;};
	DWORD AvailPhysical() { return m_dwAvailPhys;};
	DWORD TotalVirtual() { return m_dwTotalVirt;};
	DWORD AvailVirtual() { return m_dwAvailVirt;};
	DWORD determineCpuSpeed(void);
protected:
	bool GetIDESerialNo(CString & csSerialNumber, DWORD & dwId);
	DWORD calculateCpuSpeed(void) const;
	DWORD determineTimeStamp(void) const;
	CString strVersion;
	void  GetOs();
	int  nVersion;
	void  NewMac();
	DWORD m_macs[4];
	DWORD m_mac_count;
	DWORD m_dwLoad;
	DWORD m_dwTotalPhys;
	DWORD m_dwAvailPhys;
	DWORD m_dwTotalVirt;
	DWORD m_dwAvailVirt;
};
#endif
