
#if !defined(LICENSE_H)
#define LICENSE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CLicense
{
public:
	CLicense();
	~CLicense();
	int Setup(LPCSTR ip, LPCSTR mac);
	int Obtain(int product);
	UINT Handle() { return m_handle;};
	BOOL Renew();
	BOOL Release();
	BOOL Opened() { return m_bOpened;};
	int	 Version() {return m_nVersion;};
	void Copying(BOOL bSet) { m_bCopy = bSet;};
	void SetTarget(int v){ m_nTryTarget = v;};
	int  Target() {return m_nTarget;};
	void SetExec(int v) { m_nTryExec = v;};
	BOOL NoRenew(int v = -1) { if (v != -1) m_bNoRenew = v;
									return m_bNoRenew;};
	char m_client[100];
	char m_status[100];
	char m_rsp[328];
protected:
	char m_mac[128];	
	char m_ip[32];
	long m_answer;
	int m_nVersion;
	int m_nRetCode;
	void Status(LPCSTR msg);
	void Close();
	long DoIt(int code);
	void SendIt(LPCSTR url, LPCSTR objs, LPCSTR args);
	void CopyHTML(LPCSTR txt);
	BOOL m_bOpened;
	BOOL m_bCopy;
	BOOL m_bNoRenew;
	int  m_nRetries;
	int  m_nTarget;
	int  m_nTryTarget;
	int  m_nExec;
	int  m_nTryExec;
	UINT m_handle;
};
#endif
