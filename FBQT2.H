
/*
	FBRecordVidDeviceCount
	number of devices
*/
int FBRecordVidDeviceCount ();
int FBRecordAudDeviceCount ();

/*
	FBRecordVidDeviceName
*/
int FBRecordVidDeviceName (char* ioDeviceName, int inDeviceIndex);
int FBRecordAudDeviceName (char* ioDeviceName, int inDeviceIndex);

/*
	FBRecordVidSelectDevice
*/
int FBRecordVidSelectDevice (int inDeviceIndex);
int FBRecordAudSelectDevice (int inDeviceIndex);

/*
	FBRecordVidActiveDeviceName
*/
int FBRecordVidActiveDeviceName (char* ioDeviceName);
int FBRecordAudActiveDeviceName (char* ioDeviceName);

/*
	FBRecordVidOption
	if name is null then execute option
	if index & 256 then stop stream before dialog, then resume
	return zero if index is >= than options
*/
int FBRecordVidOption (int index, char * OptionName = 0);
int FBRecordAudOption (int index, char * OptionName = 0);

/*
	FBRecordVidBegin
	start recording, stop after so many secods
*/
int FBRecordVidBegin (UINT secords);


int FBVideoSetup( HWND m_hWndView, CWnd * pParent,
			LPCSTR szVidDevice);


/*
	FBRecordVidStop
	release all resources if bDestroy
*/
int FBRecordVidStop (BOOL bDestroy);

