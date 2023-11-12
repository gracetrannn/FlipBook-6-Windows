int RCapInit(HWND hwnd, LPCSTR VidName, LPCSTR AudName, LPCSTR CmpName,
			LPCSTR CaptureFile,BOOL bAudio, BOOL bPreview, UINT size);
int RCapDeviceCount(int which); // 0 is video, 1 is audio, 2 is compressor
int RCapGetName(char * ioDeviceName, int inDeviceIndex, int which);
int RCapSelectDevice(int inDeviceIndex, int which);
int RCapCurrentDevice(int which);
int RCapPreview(int v);
int RCapDoAudio(int v);
int RCapCapSize(int v);
int RCapOption (int index, int which, char * OptionName = 0);
int	RCapGetGUID(LPSTR szGUID);
//int RCapSize(int & w, int & h);
int RCapStop(BOOL bDestroy);
int RCapCopy(LPCSTR dst);
int RCapStartRecord();
int RCapStopRecord();
BOOL RCapStatus(LPSTR szStaus, int size, BOOL bFull);
