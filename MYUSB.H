/*
	returns 0 if not found
	returns 1 if found at hub and port
	returns two if hub or pprt have changed
*/
UINT FindUSB(UINT & id,LPSTR hub,UINT & port);
/*
	searches root dirs for digicel.key
*/
BOOL FindUSBMsg(CString & file, CString & msg);
BOOL WriteUSBMsg(CString file, CString code);

