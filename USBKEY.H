//
//	header file for usb access routines
//
	UINT FindKeyFile(CString & name, CString & msg);
	UINT FindMyDevice(UINT & Id);
	UINT MyUSB_GetInfo(LPSTR buf,UINT size, UINT &port);
	UINT MyUSB_GetRawNumber(LPSTR buf);
	UINT AccessMyDevice(UINT & Id, LPCSTR name, UINT port);
