#include "stdafx.h"
#include "sketch.h"
#include "direct.h"
#include "dialogs.h"
static char BASED_CODE szSettings[] = "Settings";


#if (MYVER > 599)
#define KEYMSG "KeyMsg6"
#define USBMSG "USBMsg6"
#else
#define KEYMSG "KeyMsg5"
#define USBMSG "USBMsg5"
#endif

//#define RED_BULL

#ifdef RED_BULL
#define ANIMAL1 "Ferret"
#else
#define ANIMAL1 "Anteater"
#endif
#define ANIMAL2 "Wombat"

#define KEY_SECTION "KeyLogic"
int mycopy (LPCSTR dst, LPCSTR src)
{
	CFile fdst;
	CFile fsrc;
	UINT mode;
	mode = CFile::modeRead;
	if (!fsrc.Open(src, mode))
		return 1;
	mode = CFile::modeCreate | CFile::modeReadWrite;
	if (!fdst.Open(dst, mode))
		{
		fsrc.Close();
		return 2;
		}
	BYTE buf[200];
	UINT c = sizeof(buf);
	for (;;)
		{
		UINT siz = fsrc.Read(buf, c);
		if (siz)
			fdst.Write(buf, siz);
		if (siz != c)
			break;
		}
	fdst.Close();
	fsrc.Close();
	return 0;
}

void CSketchApp::InitProfile()
{
#if !MAC
	char newname[300];
	char oldname[300];
	MyKeyKey(newname,0, 0, 91);

	CFile fnew;
	UINT mode;
	mode = CFile::modeRead;
	if (fnew.Open(newname, mode))
		fnew.Close();
	else
		{
		MyKeyKey(oldname,0, 0, 90);
		mycopy(newname,oldname);
		}
	DWORD q = ::GetEnvironmentVariable("APPDATA",newname,299);
	strcat(newname,NATIVE_SEP_STRING);
	strcat(newname,"DigiCel");
	_mkdir(newname);  // must have a folder
	strcat(newname,NATIVE_SEP_STRING);
	strcat(newname,m_pszProfileName);
	BOOL bNeedCopy = FALSE;
	int v1, v2;
	v2 = ::GetPrivateProfileInt("Settings", "Copied", 0, newname);
	v1 = GetProfileInt("Options", "Not Closed",9);
	if ((v1 != 9) && !v2)
		bNeedCopy = TRUE;
	BOOL bEnable = AfxEnableMemoryTracking(FALSE);
	free((void*)m_pszProfileName);
	m_pszProfileName = _tcsdup(newname);
	AfxEnableMemoryTracking(bEnable);
	if (bNeedCopy)
		{	// existing ini file at old location and old format
		::GetEnvironmentVariable("windir",oldname,299);
		strcat(oldname,NATIVE_SEP_STRING);
		strcat(oldname,"FlipBook.INI");
		int z = mycopy(newname,oldname);		// copy old ini into new folder
		if (z)
			AfxMessageBox(IDS_ERR_INI_COPY);

		char temp[50];
		if (::GetProfileString("Animals",ANIMAL1, "",temp,sizeof(temp)))
			PutKeyString(8,temp);
		if (::GetProfileString("Animals",ANIMAL2, "",temp,sizeof(temp)))
			PutKeyString(9,temp);
		int i;
		CString tval;
		for (i = 0; i < 11; i++)
			{
			if ((i == 4) || (i == 6) ||
							(i == 8) || (i == 9))
				continue;
			char section[100];
			char key[100];
			MyKeyKey(0,section, key, i);
			tval = GetProfileString(section,key);
			if (tval.GetLength())
				PutKeyString(i,(LPCSTR)tval);
			}
		}
	WriteProfileInt("Settings","Copied", 1);
#endif
}


/*
	0 is USBMSG
	1 is KEYMSG
	2 is TempMsg
	3 is hub
	4 is port
	5 is Lic Default
	6 is Lic Flags
	7 is system id
	8 is animal1
	9 is animal2
	10 is licensing
*/
void CSketchApp::MyKeyKey(LPSTR name,LPSTR section, LPSTR key, int which)
{
	if (name)
	{
#if !MAC
	DWORD q = ::GetEnvironmentVariable("ALLUSERSPROFILE",name,299);
	strcat(name,NATIVE_SEP_STRING);
	if (which != 90)   // 90 returns old name \programdata\digicel.ini
		{
		strcat(name,"DigiCel");
		if (which == 91)
			_mkdir(name);
		strcat(name,NATIVE_SEP_STRING);	// otherwise in digicel folder
		}
	strcat(name,"DigiCel.ini");
#endif
	}
	if (which >= 90)
		return;				// done 
	BOOL bAnimals = 0;
	switch (which) {
	case 0:
		strcpy(key, USBMSG);
		break;
	case 1:
		strcpy(key, KEYMSG);
		break;
	case 2:
		strcpy(key, "TempMsg");
		break;
	case 3:
		strcpy(key, "USBHub");
		break;
	case 4:
		strcpy(key, "USBPort");
		break;
	case 5:
		strcpy(key, "LicDefault");
		break;
	case 6:
		strcpy(key, "LicenseFlags");
		break;
	case 7:
		strcpy(key, "SystemId");
		break;
	case 8:
		bAnimals = 1;
		strcpy(key, ANIMAL1);
		break;
	case 9:
		bAnimals = 1;
		strcpy(key, ANIMAL2);
		break;
	case 10:
		strcpy(key, "Licensing");
		break;
	default:
		strcpy(key, "Unknown");
		break;
	}
	if (bAnimals)
		strcpy(section, "Animals");
	else
		strcpy(section, "KeyLogic");
}


int CSketchApp::GetKeyString(int which, LPSTR buf, int size)
{
	char name[300];
	char section[100];
	char key[100];
	MyKeyKey(name,section, key, which);
	DWORD l = ::GetPrivateProfileString(section, key, "#@#", buf, size, name);
	if ((l == 3) && (buf[0] == '#') && (buf[1] == '@') && (buf[2] == '#'))
		{
		l = ::GetProfileString(section,key, "",buf,size);//get backup fron user
		}
	return l;
}

void CSketchApp::PutKeyString(int which, LPCSTR txt)
{
	char name[300];
	char section[100];
	char key[100];
	MyKeyKey(name,section, key, which);
	int v = ::WritePrivateProfileString(section, key, txt,name);
	char test[300];
	GetKeyString(which,test,300);
	int i,j, c;
	c = strlen(txt);			// str from dlg sometimes has trailing blanks
	for (;c && (txt[c-1] <= ' ');c--);
	for (j = 0;(j < c) && (txt[j] <= ' ');j++);
	c -= j;
	for (i = 0; i < c; i++)
		if (test[i] != txt[i+j])
			break;
	if (i < c)
		{
#if !MAC
		char t[300];
		sprintf_s(t,"bad prof str |%s| != |%s|",test,txt);
#endif
//		AfxMessageBox(t);
		}
	WriteProfileString(section, key,txt);// backup copy in user
	//if (!v)
	//	LASTERR;
}

int CSketchApp::GetKeyInt(int which)
{
	char name[300];
	char section[100];
	char key[100];
	MyKeyKey(name,section, key, which);
	return ::GetPrivateProfileInt(section, key, 0, name);
}

void CSketchApp::PutKeyInt(int which, int val)
{
	char name[300];
	char section[100];
	char key[100];
	char buf[20];
	MyKeyKey(name,section, key, which);
#if MAC
	sprintf(buf, "%d", val);
#else
	_stprintf_s(buf, _countof(buf),"%d", val);
#endif
	::WritePrivateProfileString(section, key, buf, name);

//	int v = GetKeyInt(which);
	int v = ::GetPrivateProfileInt(section, key, 99, name);
	if (v != val)
		{
#if !MAC
		char t[300];
		sprintf_s(t,"bad prof int %d != %d",v,val);
#endif
//		AfxMessageBox(t);
		}
}

int CSketchApp::CheckProfile()
{
#if !MAC
	if (GetProfileInt("Settings","Still Running", 0))
		{
		WriteProfileInt("Settings","Still Running",0);
		if (MyDialog(IDD_CRASHED) == IDOK)
			{
			char temp[300];
			strcpy(temp,  m_pszProfileName);
			int l = strlen(temp);
			if (l > 10)
				{
				temp[l-1] = 'Z';
				RemoveFile(temp);
				CFile::Rename(m_pszProfileName, temp);
				return 1;
				}
			}
		}
	WriteProfileInt("Settings","Still Running",1);
	int z1 = GetProfileInt("Settings","Write Test", 0);
	WriteProfileInt("Settings","Write Test", z1 ^ 1);
	int z2 = GetProfileInt("Settings","Write Test", 0);
	if (z1 == z2)
		AfxMessageBox(IDS_NO_INI_WRITE);
#endif
	return 0;
}
