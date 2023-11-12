#include "stdafx.h"
#include "resource.h"
#include "clicense.h"
#include <afxinet.h>

#define ERR_LICENSE_UNAVAILABLE  -1
#define ERR_CUSTOMER_NOT_FOUND  -2
#define ERR_IP_ADDRESS_NOT_FOUND  -3
#define ERR_IP_TABLE_EMPTY  -4
#define ERR_PRODUCT_MISSING  -5
#define ERR_BAD_HANDLE  -6
#define ERR_BAD_MAC  -7
#define ERR_BAD_CRC  -8
#define ERR_BAD_RSP09  -9

#define ERR_NO_INTERNET -11
//
// if both sites are down, same as no internet
//
#define ERR_NO_DIGICEL -12 


#define ERR_BAD_RSP20  -20
#define ERR_BAD_RSP21  -21
#define ERR_BAD_RSP22  -22
#define ERR_BAD_RSP23  -23
#define ERR_BAD_RSP24  -24

#define DOIT_OBTAIN  0
#define DOIT_RENEW   1
#define DOIT_RELEASE 2
#define DOIT_INFO    3
#define DOIT_VERSION 4


// #define USE_PORT

DWORD MyCRC(DWORD crc, LPCSTR buf, DWORD len);

CLicense::CLicense()
{
	memset(this,0, sizeof(*this));
#ifdef USE_PORT
	m_nPort = 80;
#endif
	m_nTryExec = 1;
	m_nTryTarget = 0;	// both
}

CLicense::~CLicense()
{
	Close();
}

void CLicense::Close()
{
	if (m_bOpened)
		{
		m_bOpened = FALSE;
		DoIt(DOIT_RELEASE);
		}
}
/*
	do version if using modern php code
	do info to get product list
*/
int CLicense::Setup(LPCSTR ip, LPCSTR mac)
{
	m_nExec = m_nTryExec;
	m_nTarget = m_nTryTarget;
	strcpy(m_ip, ip);
	strcpy(m_mac, mac);			// get parameters for later
	for (;;)
		{
		if (m_nExec)
			{
			DoIt(DOIT_VERSION);// get version
			if (m_answer >= 0)
				{
				if (!m_nTarget)
					m_nTarget = 1;	// if site found lock it in
				m_nVersion = m_answer;
				m_status[0] = 0;
				DoIt(DOIT_INFO);
				}
			}
		else
			{
			m_nVersion = 99;
			m_status[0] = 0;
			DoIt(DOIT_INFO);
			if (m_answer >= 0)
				{
				if (!m_nTarget)
					m_nTarget = 1;	// lock it in
				}
			}
		if (m_nTarget)
			break;
		if ((m_answer == ERR_NO_DIGICEL) ||
				(m_answer == ERR_NO_INTERNET))
			m_nTarget = 2;
		else
			break;
		}
	return m_answer;
}
/*
	the product is from list returned by info
*/
int CLicense::Obtain(int product)
{
	m_nRetries = 0;
	m_handle = product;			// use shared variable
	DoIt(DOIT_OBTAIN);
	if (m_answer > 0)
		m_bOpened = TRUE;		// we have a license
	else
		{
		m_answer = m_answer;
		}
	return m_answer;
}

BOOL CLicense::Renew()
{
	if (!m_bOpened)
		return 1;
//m_handle = 99;
	DoIt(DOIT_RENEW);
	if (!m_answer)
		m_nRetries = 0;
	if ((m_answer == ERR_NO_INTERNET) || (m_answer == ERR_NO_DIGICEL))
		{
		if (m_nRetries++ < 3)
			m_answer = 0;
		}
	if (m_answer)
		m_bOpened = 0;			// no further need for a close
	return m_answer ? 1 : 0;
}

BOOL CLicense::Release()
{
	if (!m_bOpened)
		return 1;
	Close();
	return m_answer;
}

void CLicense::Status(LPCSTR msg)
{
	strcpy(m_status,msg);
}

/*
	general purpose routine for sending a page and getting result
*/
void CLicense::SendIt(LPCSTR url, LPCSTR objs, LPCSTR args)
{
	m_nRetCode = 0;
	m_answer = ERR_NO_INTERNET;
	DWORD   dwAccessType = PRE_CONFIG_INTERNET_ACCESS;

	DWORD dwHttpRequestFlags =
		INTERNET_FLAG_EXISTING_CONNECT | INTERNET_FLAG_NO_AUTO_REDIRECT;

	CInternetSession session(_T("DigiCel License Manager"), dwAccessType);
	CHttpConnection* pServer = NULL;
	CHttpFile* pFile = NULL;

	CString page = "";
	try
	{
CString strHeaders =
      _T("Content-Type: application/x-www-form-urlencoded");
	 CString strFormData = args;
#ifdef USE_PORT
	pServer =   session.GetHttpConnection(url,m_nPort,NULL,NULL,NULL);
#else
	pServer =   session.GetHttpConnection(url);
#endif
	pFile =  pServer->OpenRequest(CHttpConnection::HTTP_VERB_POST,
					objs);
	DWORD dwRet;
	if (pFile)
		{
   	BOOL result = pFile->SendRequest(strHeaders,
      (LPVOID)(LPCTSTR)strFormData, strFormData.GetLength());
		pFile->QueryInfoStatusCode(dwRet);
		}
		if (dwRet != 200)
			{
			m_answer = ERR_NO_DIGICEL;
			char buf[200];
			sprintf(buf,"Status code is%d",dwRet);
			Status(buf);
			}
		else
			m_answer = ERR_BAD_RSP20;
		TCHAR sz[1024];
		int index = 0;
		int state = 0;
		int j = 0;
		sz[0] = 0;
		for (;;)
			{
			if (!sz[index])
				{
				if (!pFile->ReadString(sz, sizeof(sz) - 1))
					break;
				if (m_bCopy)
					page += sz;
				index = 0;
				}
			char c = sz[index++];
			if (state == 0)
				{
				if (c == '{')
					state = 1;
				}
			else if (state == 1)
				{
				if (c == '}')
					state = 2;
				else if ((j + 1) < sizeof(m_rsp))
					m_rsp[j++] = c;
				}
			}
		m_rsp[j] = 0;
		if (state == 2)
			m_answer = 0;
		pFile->Close();
		if (pServer)
		pServer->Close();
	}
	catch (CInternetException* pEx)
	{
		// catch errors from WinINet

		TCHAR szErr[1024];
		pEx->GetErrorMessage(szErr, 1024);

		char buf[200];
		sprintf(buf,"Error:%s",szErr);
		Status(buf);

		m_nRetCode = 2;
		pEx->Delete();
	}


	if (pFile != NULL)
	{
		pFile->Close();
		delete pFile;
	}
	if (pServer != NULL)
	{
		pServer->Close();
		delete pServer;
	}
	session.Close();
	if (m_bCopy)
		CopyHTML((LPCSTR)page);
}

long CLicense::DoIt(int code)
{
	char temp[80];
	char url[328];
	char obj[328];
	char args[328];
	
	if (m_nTarget != 2)
		strcpy(url,"www.digicelinc.com");
//		strcpy(url,"www.digicel.net");
	//	strcpy_s(url,"www.easyanimationsoftware.com");
	else
		strcpy(url,"www.flipbookpro.com");
	if (m_nExec > 1)
		strcpy(obj,"dexec1.php");
	else if (m_nExec)
		strcpy(obj,"exec1.php");
	else
		strcpy(obj,"exec.php");
	args[0] = 0;
	strcpy(args,"cmd=");
	BOOL bMac = 0;
	BOOL bIp = 0;
	switch (code) {
	case DOIT_OBTAIN:
		strcat(args,"Obtain");
		bMac = 1;
		bIp = 1;
		break;
	case DOIT_RENEW:
		strcat(args,"Renew");
		bMac = 1;
		break;
	case DOIT_RELEASE:
		bMac = 1;
		strcat(args,"Release");
		break;
	case DOIT_INFO:
		strcat(args,"Info");
		bIp = 1;
		break;
	default:
		strcat(args,"Version");
		break;
	}
	if (bMac)
		{
		sprintf(temp,"&MACAddress=%s",m_mac);
		strcat(args,temp);
		}
	if (bIp && m_ip[0])
		{
		strcat(args,"&IP_Override=on");
		strcat(args,"&Base_IP=");
		strcat(args,m_ip);
		}
	if (bMac)
		{
		sprintf(temp,"&Handle=%d",m_handle);
		strcat(args,temp);
		}
	m_rsp[0] = 0;
	SendIt(url, obj, args);
	if (m_answer)
		return 1;
	if (code == DOIT_INFO)
		{
		sscanf(m_rsp,"%d",&m_answer);
		}
	else
		{
		if ((code == DOIT_OBTAIN) && (m_nVersion >= 100))
			{
			UINT crc;
			if (sscanf(m_rsp,"%d,%u",&m_answer,&crc) == 2)
				{
				UINT v = MyCRC(0,m_mac, strlen(m_mac));
				v ^= m_answer;
				if (v != crc)
					m_answer = ERR_BAD_CRC;
				else
					m_handle = v;
				}
			else
				{
				if (sscanf(m_rsp,"%d",&m_answer) != 1)
					m_answer = ERR_BAD_RSP21;
				else if (m_answer != ERR_LICENSE_UNAVAILABLE)
					m_answer = ERR_BAD_RSP22;
				}
			}
		else if (sscanf(m_rsp,"%d",&m_answer) != 1)
			m_answer = ERR_BAD_RSP23;
		}
	if (m_answer >= 0)
		strcpy(m_status, "Okay");
	return 0;	
}

// CopyHtml() - Copies given HTML to the clipboard.
// The HTML/BODY blanket is provided, so you only need to
// call it like CallHtml("<b>This is a test</b>");

void CLicense::CopyHTML(LPCSTR html)
	{
#ifndef FLIPBOOK_MAC
    // Create temporary buffer for HTML header...
    char *buf = new char [400 + strlen(html)];
    if(!buf) return;

    // Get clipboard id for HTML format...
    static int cfid = 0;
    if(!cfid) cfid = RegisterClipboardFormat("HTML Format");
	//cfid = CF_TEXT;
    // Create a template string for the HTML header...
    strcpy(buf, "Version:0.9\r\n");
	strcat(buf, "StartHTML:-1\r\n");
	strcat(buf, "EndHTML:-1\r\n");
	strcat(buf, "StartFragment:------\r\n");
	strcat(buf,   "EndFragment:++++++\r\n");
	int qq = strlen(buf);
    strcat(buf, html);
	strcat(buf, "\r\n");
	int zz = strlen(buf);

	char * ptr;
	ptr = strstr(buf, "------");
	wsprintf(ptr, "%06u", qq);
	*(ptr+6) = '\r';	// erase null
	ptr = strstr(buf, "++++++");
	wsprintf(ptr, "%06u", zz);
	*(ptr+6) = '\r';	// erase null

    // Now you have everything in place ready to put on the
    // clipboard.

    // Open the clipboard...
    if(::OpenClipboard(0)) {

        // Empty what's in there...
        EmptyClipboard();

        // Allocate global memory for transfer...
        HGLOBAL hText = GlobalAlloc(GMEM_MOVEABLE |GMEM_DDESHARE,
strlen(buf)+4);

        // Put your string in the global memory...
        char *ptr = (char *)GlobalLock(hText);
        strcpy(ptr, buf);
        GlobalUnlock(hText);

        ::SetClipboardData(cfid, hText);

        CloseClipboard();
        // Free memory...
        GlobalFree(hText);
		delete [] buf;
    }

#endif
}
/*
	sans table
*/
DWORD MyCRC(DWORD crc, LPCSTR buf, DWORD len)
{
	DWORD poly = 0xedb88320;
    crc = crc ^ 0xffffffff;
	for(;len--;)
		{
		DWORD c = (crc ^ (*buf++)) & 0xff;
		int k;
		for (k = 0; k < 8; k++)
            c = c & 1 ? poly ^ (c >> 1) : c >> 1;
		crc = c ^ (crc >> 8);
	    } 
    return crc ^ 0xffffffff;
}
