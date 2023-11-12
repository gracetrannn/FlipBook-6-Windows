#include "stdafx.h"
#include "sketch.h"
#include "mydoc.h"
#include "nscaler.h"
#include "vfw.h"
#include "mysound.h"
void extern Interlace(BYTE * tp, BYTE * hp, UINT pitch, UINT height, UINT off);

void CSketchDoc::DoFlv(CString name, UINT w, UINT h, UINT BPP,
					UINT origw, UINT origh,
					UINT f1, UINT l1, UINT f2, UINT l2,
					UINT rate, int Telecine, CMySound * pSound, CString encoder)
{
#ifndef FLIPBOOK_MAC
	UINT i;
	HRESULT hr;
//	WORD wVer;
	UINT bits = 8 * BPP;
	CGScaler scale;
	char tname[400];
	if (!GetTempPath(380, tname))
		tname[0] = 0;
	strcat(tname,"fbflvsrc.avi");
//	AfxMessageBox(encoder);
	BeginBusy("Writing AVI");
	AVISTREAMINFO strhdr;
	PAVIFILE pfile = NULL;
	PAVISTREAM ps = NULL;
	UINT wLineLen = 4 * (( bits * w + 31) / 32);
	DWORD wColSize = sizeof(RGBQUAD)*((bits <= 8) ? 1<<bits : 0);
	DWORD dwSize = (DWORD)(UINT)wLineLen*(DWORD)(UINT)h;

	BYTE * hpBuf1 = new BYTE[dwSize];
	BYTE * hpBuf2;
	BYTE * hpBuf;
	BYTE * hpTBuf = 0;
	if ((origw != w) || (origh != h))
		{
		UINT owLineLen = 4 * (( BPP * origw+ 3) / 4);
		hpBuf2 = new BYTE[owLineLen * origh];
		hpBuf = hpBuf2;
		}
	else
		{
		hpBuf = hpBuf1;
		hpBuf2 = 0;
		}
	int toffset, tfactor;
	if (Telecine)
		{
		if (Telecine > 1)
			toffset = 1;
		else
			toffset = 0;
		tfactor = 4;
		hpTBuf = new BYTE[dwSize];
DPF("t:%d,off:%d,fac:%d",Telecine,toffset,tfactor);
		}
	else
		tfactor = 1;
	LPBITMAPINFOHEADER lpbi = (LPBITMAPINFOHEADER)new BYTE[40+1024];
	lpbi->biSize = sizeof(BITMAPINFOHEADER) ;
	lpbi->biWidth = w;
	lpbi->biHeight = h;
	lpbi->biPlanes = 1 ;
	lpbi->biBitCount = (WORD) bits ;
	lpbi->biCompression = BI_RGB ;
	lpbi->biSizeImage = dwSize;
	lpbi->biXPelsPerMeter = 0 ;
	lpbi->biYPelsPerMeter = 0 ;
	lpbi->biClrUsed = (bits <= 8) ? 1<<bits : 0;
	lpbi->biClrImportant = 0 ;
	for (i = 0; i < 1024; i++)
		((BYTE *)lpbi)[i+40] = i / 4;
	Progress(1,1 + f2 - f1);

	CloseHandle(CreateFile((LPCSTR)tname,GENERIC_WRITE | DELETE,
								0,0,TRUNCATE_EXISTING,FILE_ATTRIBUTE_NORMAL,0));
	AVIFileInit();
	hr = AVIFileOpen(&pfile,		    // returned file pointer
		       (LPCSTR)tname,		            // file name
		       OF_CREATE,	    // mode to open file with
		       NULL);			    // use handler determined
						    // from file extension....
DPF("aviopen:%lx",hr);
	if (hr != AVIERR_OK)
		goto error;
	// Fill in the header for the video stream....

	memset(&strhdr, 0, sizeof(strhdr));
	strhdr.fccType                = streamtypeVIDEO;// stream type
	strhdr.fccHandler             = 0;
	strhdr.dwScale                = 1;
	strhdr.dwRate                 = Telecine ? (5 * rate) / 4 : rate;
	strhdr.dwSuggestedBufferSize  = lpbi->biSizeImage;
	SetRect(&strhdr.rcFrame, 0, 0,		    // rectangle for stream
	    (int) lpbi->biWidth,
	    (int) lpbi->biHeight);

	// And create the stream;
	hr = AVIFileCreateStream(pfile,		    // file pointer
			         &ps,		    // returned stream pointer
			         &strhdr);	    // stream header
DPF("create stream:%lx",hr);
	if (hr != AVIERR_OK) {
		goto error;
	}
	hr = AVIStreamSetFormat(ps, 0,
			       lpbi,	    // stream format
			       lpbi->biSize +   // format size
			       lpbi->biClrUsed * sizeof(RGBQUAD));
DPF("setstreamformat:%lx",hr);
	if (hr != AVIERR_OK) {
		goto error;
	}
	UINT Frame;
	UINT OFrame;
	if (hpBuf2)
		{
		if (scale.Init(origw,origh,8*BPP,w,h))
			{
DPF("scale failure");
			goto error;
			}
		}
	for (OFrame = 0, Frame = f1; Frame <= f2;Frame++)
		{
		m_pScene->CompositeFrame32(hpBuf,
				l1, l2, Frame,FALSE);
		if (hpBuf2)
			{
	int q = scale.Custom(hpBuf1, hpBuf2, 4 * ((BPP*w+3)/4));
DPF("after custom:%d",q);
	int z = scale.Copy();
DPF("after scale:%d",z);
			}
		if ((BPP == 1) && (bits == 24))
			{
			int x,y,p1,p2,p3,p4;
			p1 = 4 * ((w+3)/4);
			p2 = 4 * ((3*w+3)/4);
			p3 = p1 * (h - 1);
			p4 = p2 * (h - 1);
			for (y = h - 1; y >= 0; y--)
				{
				for (x = w -1 ; x >= 0; x--)
					{
					hpBuf1[p4+3*x+2] = hpBuf1[p3+x];
					hpBuf1[p4+3*x+1] = hpBuf1[p3+x];
					hpBuf1[p4+3*x+0] = hpBuf1[p3+x];
					}
				p4 -= p2;
				p3 -= p1;
				}
			}
//		m_pScene->GetGray(hpBuf, Frame, dlg.m_StartLevel);
//		for (Level = dlg.m_StartLevel+1; Level <= dlg.m_EndLevel;Level++)
//			m_pScene->ApplyGray(hpBuf, 100, Frame,Level);
DPF("putting:%d",Frame);
		UINT pitch = wLineLen;
		UINT height = h;
	
		switch((Frame - f1) % tfactor)
		{
		case 1:
			Interlace(hpTBuf, hpBuf1, pitch,height, toffset);
		case 0:
			hr = AVIStreamWrite(ps, OFrame++,
			1, hpBuf1, lpbi->biSizeImage, AVIIF_KEYFRAME, NULL, NULL);
			break;
		case 2:
			Interlace(hpTBuf, hpBuf1, pitch,height, toffset ^ 1);
			hr = AVIStreamWrite(ps, OFrame++,
			1, hpTBuf, lpbi->biSizeImage, AVIIF_KEYFRAME, NULL, NULL);
			Interlace(hpTBuf, hpBuf1, pitch,height, toffset);
			break;
		case 3:
			Interlace(hpTBuf, hpBuf1, pitch,height, toffset ^ 1);
			hr = AVIStreamWrite(ps, OFrame++,
			1, hpTBuf, lpbi->biSizeImage, AVIIF_KEYFRAME, NULL, NULL);
			hr = AVIStreamWrite(ps, OFrame++,
			1, hpBuf1, lpbi->biSizeImage, AVIIF_KEYFRAME, NULL, NULL);
			break;
		}
		if (hr != AVIERR_OK)
			break;
		Progress(2, Frame + 1 - f1);
	}
	DPF("last frame:%d",Frame);
	
	error:

DPF("hr:%lx",hr);
	Progress();
	EndBusy();
	BeginBusy("Writing Sound");
	//
	// Now close the file
	//
	if (ps)
		AVIStreamClose(ps);

	
	if (hr == AVIERR_OK && pSound)
		{
//    AVISTREAMINFO   strhdr;
		UINT nFormat;
		UINT nAudio = pSound->WaveLength(1 + f2 - f1);
		BYTE * pAudio = pSound->WaveData(f1);
   // Fill in the header for the audio stream....
		memset(&strhdr, 0, sizeof(strhdr));
		strhdr.fccType                = streamtypeAUDIO;  // stream type
		strhdr.dwScale                = 1;                
		strhdr.dwRate                 = pSound->Rate();
		strhdr.dwSuggestedBufferSize  = pSound->Bytes();                    

   // And create the stream;
		hr = AVIFileCreateStream(pfile,             // file pointer
                              &ps,             // returned stream pointer
                              &strhdr);            // stream header
		if (hr != AVIERR_OK) 
			goto error2;
		WAVEFORMATEX pcm;
		nFormat = pSound->GetFormat(&pcm);
		hr = AVIStreamSetFormat(ps, 0, 
                              &pcm,       // stream format
                              nFormat);      // format size
		if (hr == AVIERR_OK) 

		hr = AVIStreamWrite(ps,                // stream pointer
                       0,                     // starting sample
                       nAudio / pSound->Bytes(),                     // number to write
                       pAudio,   // pointer to data
                       nAudio,     // size of the data
                       AVIIF_KEYFRAME,        // flags....
                       NULL, NULL);
error2:
		AVIStreamClose(ps);            // close the audio stream
		}

	if (pfile)
		AVIFileClose(pfile);

	AVIFileExit();
	delete lpbi;
	delete hpBuf1;
	delete hpTBuf;
	if (hpBuf2)
		delete hpBuf2;
	EndBusy();

	if (hr != NOERROR) {
		CloseHandle(CreateFile((LPCSTR)tname,GENERIC_WRITE | DELETE,
								0,0,TRUNCATE_EXISTING,FILE_ATTRIBUTE_NORMAL,0));
//		LoadString(hInstance, IDS_APPERR, szTitle, BUFSIZE );
//		LoadString(hInstance, IDS_WRITEERR, szMessage, BUFSIZE );
		
//		MessageBox(NULL, szMessage, szTitle, MB_OK);
		MyError(IDS_BAD_WRITE, MB_OK);
		return;
	}
	BeginBusy("Converting AVI");
	CloseHandle(CreateFile((LPCSTR)name,GENERIC_WRITE | DELETE,
								0,0,TRUNCATE_EXISTING,FILE_ATTRIBUTE_NORMAL,0));
// the -y should pre erase the flv file
//
	char args[900];
	sprintf(args,"%c%s%c -o %c%s%c -ovc lavc -of lavf -oac pcm -srate 11025 -lavcopts ",
	34,tname,34,34,name,34);
	strcat(args,"vcodec=flv:vbitrate=7200");
//	sprintf(args,"-y -i %s -vcodec flashsv %s",tname,name);
//	sprintf(args,"-y -i %s %s",tname,name);
	SHELLEXECUTEINFO ShellInfo; // Name structure
	memset(&ShellInfo, 0, sizeof(ShellInfo)); // Set up memory block
	ShellInfo.cbSize = sizeof(ShellInfo); // Set up structure size
	ShellInfo.lpVerb = "open";
	ShellInfo.lpFile = encoder;
	ShellInfo.lpParameters = args;
	ShellInfo.nShow = SW_HIDE;
	ShellInfo.fMask = SEE_MASK_NOASYNC |
						SEE_MASK_WAITFORINPUTIDLE |
						SEE_MASK_NOCLOSEPROCESS;
	int z = 32;
	if (!ShellExecuteEx(&ShellInfo))
		z = 1;
	else
		WaitForSingleObject(ShellInfo.hProcess, 120000);
	RemoveFile(tname);
	CFileStatus status;
	if (!CFile::GetStatus( name, status )) 
		z = 31;
	else if (!status.m_size)
		z = 30;
	EndBusy();
	if (z == 31)
		AfxMessageBox(IDS_ERR_FLV_CREATE);
	else if (z == 30)
		AfxMessageBox(IDS_ERR_FLV_CONVERT);
	else if (z < 32)
		FormattedMsg(IDS_ERR_FLV_ERROR,z);
#endif
}
