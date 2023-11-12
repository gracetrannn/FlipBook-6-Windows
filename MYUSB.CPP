#include "stdafx.h"
#include "myusb.h"
#ifndef FLIPBOOK_MAC
#include "usbkey.h"
#else
#include "IOKit/usb/IOUSBLib.h"
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>

int nSerializedKey = 0;
/*
	checks vendor id and product id
	returns true if bad
*/
BOOL CheckKeyInfo(UINT vid, UINT pid)
{
	nSerializedKey = 0;
	if (vid == 0x0EA0)
		{
		if ((pid != 0x6803) && (pid != 0x6828) && (pid != 0x2168))
			return TRUE;
		}
	else if (vid == 0x090C)
		{
		if (pid != 0x1000)
			return TRUE;
		}
	else if (vid == 0x058f)
		{
		if (pid != 0x6387)
			return TRUE;
		}
	else if (vid == 0x1516)
		{
		if (pid != 0x8628)
			return TRUE;
		}
	else if (vid == 0x1307)
		{
		if ((pid != 0x0163) && (pid != 0x0165))
			return TRUE;
		}
	else if (vid == 0x1E3d)
		{
		if (pid == 0x2095)
			{
			nSerializedKey = 1;
			}
		else if ((pid != 0x2092) && (pid != 0x2093))
			return TRUE;
		}
	else
		return TRUE;
	return FALSE;
}

UINT myhex(char c)
{
	if ((c >= '0') && (c <= '9'))
		return c - '0';
	c |= 32;
	if ((c >= 'a') && (c <= 'f'))
		return 9 + (c & 15);
	return 0;
}
UINT ProcessSerialNum(LPCSTR sernbr)
{
	UINT outID;
	int offset = 0;
	int l = strlen(sernbr);
	if (l < 8)
		return 0;
	UINT snum1 = 0;
	UINT snum2 = 0;
	UINT v;
	int i;
	for (i = 0, v = 1; i < 8; i++, v*=16)
		snum1 += v * myhex(sernbr[l-1-i]);
	if (nSerializedKey == 1)
		{
		if ((snum1 < 0x99900000) || (snum1 > 0x99909999))
			return 0;
		}
	for (v = 1; i < l; i++, v*=16)
		snum2 += v * myhex(sernbr[l-1-i]);
	outID = snum1 ^ snum2;
	return outID;
}



/*
	returns 0 if not found
	returns 1 if found at hub and port
	returns two if hub or pprt have changed
*/
UINT FindUSB(UINT & outID,LPSTR hub,UINT & port)
{
	UINT res = 0;
#ifndef FLIPBOOK_MAC
	if (port)
		res = AccessMyDevice(outID, (LPCSTR)hub, port);
	if (!res)
		{
		if (FindMyDevice(outID) == 1)
			{
			MyUSB_GetInfo(hub, 140, port);
			res = 2;
			}
		else
			res = 0;
		}
#else
	// we don't use, clear it
	hub[0] = '\0';

	// mac version always returns 0 (not found) or 2 (found, ignoring hub/port)

	CFMutableDictionaryRef myUSBMatchDictionary = IOServiceMatching (kIOUSBDeviceClassName);

	io_iterator_t myIterator;

#if 0
	CFNumberRef cfValue;
	cfValue = CFNumberCreate (kCFAllocatorDefault, kCFNumberSInt32Type, &kDeviceVendorID);
	if (cfValue) {
		CFDictionaryAddValue (myUSBMatchDictionary, CFSTR("VendorID"), cfValue);
		CFRelease (cfValue);
	}
	
	cfValue = CFNumberCreate (kCFAllocatorDefault, kCFNumberSInt32Type, &kDeviceProductID);
	if (cfValue) {
		CFDictionaryAddValue (myUSBMatchDictionary, CFSTR("idProduct"), cfValue);
		CFRelease (cfValue);
	}
#endif

	IOServiceGetMatchingServices (kIOMasterPortDefault, myUSBMatchDictionary, &myIterator);
	
	io_object_t iot;
	while (iot = IOIteratorNext (myIterator)) {
		SInt32 vendor_id = 0;
		CFNumberRef vendor_ref = static_cast<CFNumberRef>(IORegistryEntryCreateCFProperty (iot, CFSTR("idVendor"), kCFAllocatorDefault, 0));
		if (vendor_ref) {
			CFNumberGetValue (vendor_ref, kCFNumberSInt32Type, &vendor_id);
			CFRelease (vendor_ref);
		}

		SInt32 product_id = 0;
		CFNumberRef product_ref = static_cast<CFNumberRef>(IORegistryEntryCreateCFProperty (iot, CFSTR("idProduct"), kCFAllocatorDefault, 0));
		if (product_ref) {
			CFNumberGetValue (product_ref, kCFNumberSInt32Type, &product_id);
			CFRelease (product_ref);
		}
		
		if ( !CheckKeyInfo(vendor_id, product_id))
			{
			CFStringRef serialnum_ref = static_cast<CFStringRef>(IORegistryEntryCreateCFProperty (iot, CFSTR("USB Serial Number"), kCFAllocatorDefault, 0));			
			if (serialnum_ref) {
				char hex_full[33];
				if (CFStringGetCString (serialnum_ref, hex_full, 33, kCFStringEncodingMacRoman)) {
					outID = ProcessSerialNum(hex_full);
					res = 2;
				}
				CFRelease (serialnum_ref);
				break;
			}
		}		
	}
	
#endif
	return res;
}

UINT CheckKeyFile(CString & name, CString & msg)
{
	FILE * stream;
	stream = fopen(name,"r");
	if (stream == NULL)
		return 1;
#ifndef FLIPBOOK_MAC
DPF("found on:%s",name);
#endif
	char buf[100];
	char mmsg[100];
	for (;;)
		{
		if (!fgets(buf,80,stream))
			break;
		if (buf[0] == '#')
			continue;
		int v = sscanf(buf,"FlipBook=%s",&mmsg);
		if (v == 1)
			{
			msg = mmsg;
			break;
			}
		}
	fclose(stream);
	return 0;
}


/*
	searches root dirs for digicel.key
*/
BOOL FindUSBMsg(CString & name, CString & msg)
{
	UINT result = 0;
#ifndef FLIPBOOK_MAC
    DWORD     dwDrives = GetLogicalDrives ();
    char     szCheck[16];
	char	drive;
	DWORD bit = 8;
	msg = "";
DPF("dwDrives:%8X", dwDrives);
	for (drive = 'D'; drive <= 'Z'; drive++, bit += bit)
		{
		if (!( dwDrives & bit ))
			continue;
		sprintf (szCheck,"%c:\\", drive);
		UINT v = GetDriveType( szCheck );
	DPF("bit:%d,type:%d,%s",bit,v,szCheck);
		if (GetDriveType( szCheck ) != 2)
			continue;
		CString name1;
		strcat(szCheck,"digicel.ini");
		name1 = szCheck;
		if (CheckKeyFile(name1, msg))
			continue;
		result = 1;
		name = szCheck;
		break;
		}
#else
	NSArray* volumes = [[NSWorkspace sharedWorkspace] mountedRemovableMedia];
	for (int i = 0; i < [volumes count]; i++) {
		NSString* volume_path = [volumes objectAtIndex:i];
		NSString* file_path = [volume_path stringByAppendingPathComponent:@"digicel.ini"];
		if ([[NSFileManager defaultManager] fileExistsAtPath:file_path]) {
			CString s ([file_path cStringUsingEncoding:NSUTF8StringEncoding]);
			if (CheckKeyFile (s, msg)) {
				continue;
			}
			result = 1;
			name = [file_path cStringUsingEncoding:NSUTF8StringEncoding];
			break;
		}
	}
#endif
	return result;
}

BOOL WriteUSBMsg(CString  name,CString xcode)
{
	CString code;
	code = "FlipBook=";
	code += xcode;
	FILE * stream;
#ifndef FLIPBOOK_MAC
	_chmod(name,_S_IWRITE);
#else
	NSFileManager* fm = [NSFileManager defaultManager];
	NSString* fullpath = [NSString stringWithCString:name.c_str()];
	if (![fm fileExistsAtPath:fullpath]) {
		return 1;
	}
	[fm changeFileAttributes:[NSDictionary dictionaryWithObject:[NSNumber numberWithBool:NO] forKey:NSFileImmutable] atPath:fullpath];	
#endif
	stream = fopen(name,"w");
	if (stream == NULL)
		return 1;
	fputs("#\n", stream);
	fputs("#	Digicel Key File for USB storage\n",stream);
	fputs("#	Do Not Delete\n",stream);
	fputs("#\n", stream);
	if (code.GetLength())
		{
		fputs((LPCSTR)code, stream);
		fputs("\n", stream);
		}
	fclose(stream);
#ifdef FLIPBOOK_MAC
	[fm changeFileAttributes:[NSDictionary dictionaryWithObject:[NSNumber numberWithBool:YES] forKey:NSFileImmutable] atPath:fullpath];	
#endif
	return 0;
}

