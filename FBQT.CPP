#include "stdafx.h"
#include "sketch.h"
#include "fbqt.h"
//#include "string.h"
#include "capper.h"
#include <atlimage.h>
#ifdef USEQT
#include "myqt.h"
#include "mysound.h"
#include <windowsx.h>
#include <Gdiplusimaging.h>
//#include <strsafe.h>
	#include <QuickTimeComponents.h>
#endif
#include "rcapdef.h"
#include "vfw.h"


DEFINE_GUID(ImageFormatGIF,
		0xb96b3cb0,0x0728,0x11d3,0x9d,0x7b,0x00,0x00,0xf8,0x1e,0xf3,0x2e);
DEFINE_GUID(ImageFormatEXIF,
		0xb96b3cb2,0x0728,0x11d3,0x9d,0x7b,0x00,0x00,0xf8,0x1e,0xf3,0x2e);
DEFINE_GUID(ImageFormatPNG,
		0xb96b3caf,0x0728,0x11d3,0x9d,0x7b,0x00,0x00,0xf8,0x1e,0xf3,0x2e);
DEFINE_GUID(ImageFormatJPEG,
		0xb96b3cae,0x0728,0x11d3,0x9d,0x7b,0x00,0x00,0xf8,0x1e,0xf3,0x2e);
DEFINE_GUID(ImageFormatTIFF,
		0xb96b3cb1,0x0728,0x11d3,0x9d,0x7b,0x00,0x00,0xf8,0x1e,0xf3,0x2e);

#ifdef USEQT
int bQT = 0;
int FBQuickTime()
{
	if (!bQT)
		FBQuickTimeInit();
	return bQT > 1 ? 1 : 0;
}

void FBQuickTimeInit ()
{
	if(InitializeQTML(0) == noErr)
		bQT = 2;
	else
		{
		TerminateQTML();
		bQT = 1;
		}
}
/*
	returns 1 for 24, 2 for 32 , 4 for 8 bit
*/
int FBQTGetExt( char * pExt, FBFileFormat which)
{
	int res = 0;
	switch (which) {
	case kBMPFileFormat:
		res = 5;
		if(pExt)
			strcpy_s(pExt,4,"BMP");
		break;
	case kTARGAFileFormat:
		res = 3;
		if(pExt)
			strcpy_s(pExt,4,"TGA");
		break;
	case kPNGFileFormat: 
		res = 3;
		if(pExt)
			strcpy_s(pExt,4,"PNG");
		break;
	case kJPEGFileFormat:
		res = 1;
		if(pExt)
			strcpy_s(pExt,4,"JPG");
		break;
	case kPhotoshopFileFormat:
		if(pExt)
			strcpy_s(pExt,4,"PSD");
		break;
	case kTIFFFileFormat:
		res = 3;
		if(pExt)
			strcpy_s(pExt,4,"TIF");
		break;
	default:
		res = 0;
		if(pExt)
			strcpy_s(pExt,4,"???");
		break;
	}
	return res;
}

void* FBExportMovieSetup (const char* inFilePath, int inWidth, int inHeight,
				int inBitDepth, int inRate, int inNumFrames)
{
	CMyQT * pQT = new CMyQT;
	int res;
	if (res = pQT->Setup(inFilePath,
					inWidth,inHeight,inBitDepth,inRate,inNumFrames))
		{
		delete pQT;
		pQT = 0;
		DPF("myqt setup:%d",res);
		FormattedMsg(IDS_ERR_QT_INIT,res);
		}
	return pQT;
}

int FBExportMoviePut (void *inUserObject, void *inBitmapData, int inBitmapSize)
{
	CMyQT * pQT = (CMyQT *)inUserObject;
	return pQT->Append((BYTE *)inBitmapData,inBitmapSize);
}

int FBExportMovieSound (void *inUserObject, CMySound* inSound, UINT f1, UINT f2)
{
	CMyQT * pQT = (CMyQT *)inUserObject;
	if (pQT && inSound) {
		WAVEFORMATEX format;
		inSound->GetFormat (&format);
		int num_frames = 1 + f2 - f1;
		BYTE* wav_data = inSound->WaveData (f1);
		int wav_size = inSound->WaveLength (num_frames);
		pQT->AddSound(wav_data,wav_size,format.nChannels,format.wBitsPerSample,
					format.nSamplesPerSec,num_frames);
		}
	return 0;
}

int FBExportMovieClose (void* inUserObject)
{
	CMyQT * pQT = (CMyQT *)inUserObject;
	int z = pQT->Finish();
	delete pQT;
	return z;
}
#endif

CCapper * pCap = 0;
int FBVideoCaptureBegin (ImageDataCallback inFunction, void* inUserObject, int inWidth, int inHeight)
{
	delete pCap;
	pCap = new CCapper();
	pCap->m_callback = inFunction;
	pCap->m_pCallObj = (CCapDlg *)inUserObject;
	pCap->m_width = inWidth;
	pCap->m_height = inHeight;
	return 0;
}

int FBVideoCaptureStop(BOOL bDestroy)
{
	if (!bDestroy)
		pCap->Destroy();
	else
		{
		delete pCap;
		pCap = 0;
		}
	return 0;
}

int FBVideoSetup( HWND hWndView, CWnd * pParent, LPCSTR szDevice)
{
	ASSERT(pCap);
	return pCap->Setup(hWndView, pParent,szDevice);
}

int FBVideoCaptureSize(int & w, int & h, int &d)
{
	ASSERT(pCap);
	w = pCap->m_width;
	h = pCap->m_height;
	d = pCap->m_depth;
	return 0;
}

int FBVideoCaptureDeviceCount ()
{
	ASSERT(pCap);
	return pCap->Devices();
}

int FBVideoCaptureDeviceName (char* ioDeviceName, int inDeviceIndex)
{
	ASSERT(pCap);
	strcpy_s(ioDeviceName,80,pCap->m_szDevices[inDeviceIndex]);
	return 1;
}

int FBVideoCaptureSelectDevice (int inDeviceIndex)
{
	ASSERT(pCap);
	pCap->SelectDevice(inDeviceIndex);
	return 1;
}

int FBVideoCaptureActiveDeviceName (char* ioDeviceName)
{
	ASSERT(pCap);
	strcpy_s(ioDeviceName,80,pCap->CurName());
	return 1;
}

int FBVideoCaptureOption (int which, char * OptionName)
{
	ASSERT(pCap);
	if (!OptionName)
		{
		pCap->Dialog(which);
		}
	else
		{
		if (which >= pCap->Options())
			return -1;
		strcpy_s(OptionName, 80,pCap->m_szOptions[which]);
		}
	return 0;
}



#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')
int PutBMP(CFile &file, void * BitmapData)
{
	BITMAPFILEHEADER bmfHdr; // Header for Bitmap file
	LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)BitmapData;

	UINT w = lpBI->biWidth;
	UINT h = lpBI->biHeight;
	UINT d = lpBI->biBitCount;
	UINT p = 4 * ((w * d + 31) / 32);
	if ((d != 24) && (d != 8) && (d != 1))
		return kBadInfo;
	bmfHdr.bfType = DIB_HEADER_MARKER;  // "BM"

	DWORD dwExtra = sizeof(BITMAPINFOHEADER);
	if (d < 24)
		dwExtra += 4 << d;
	DWORD dwSizeImage = h * p;

	lpBI->biSizeImage = dwSizeImage;

	bmfHdr.bfSize = dwExtra + dwSizeImage + sizeof(BITMAPFILEHEADER);
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;

	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + dwExtra;

	// Write the file header
	file.Write((LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER));
	file.Write(lpBI, dwExtra + dwSizeImage);
	return 0;
}

int PutTGA(CFile &file, void * BitmapData)
{
	BOOL bCompressed = 1;
	BYTE hdr[18];
	DWORD dwBytesSaved = 0;
	BYTE * dst;
	BYTE * src;
	src = (BYTE *)BitmapData;
	LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)BitmapData;
	src += lpBI->biSize;
	UINT w = lpBI->biWidth;
	UINT h = lpBI->biHeight;
	UINT dd = lpBI->biBitCount;
	UINT ip = 4 * ((w * dd + 31) / 32);
	UINT d = (dd + 7) / 8;
	if ((d != 3) && (d!=4)) return kBadInfo;
	UINT op = d * w;
	DWORD dsize = 20 + h * op * 2;
	dst = new BYTE[dsize];
	BYTE * dptr = dst;
	if (!dst)
		return kNoMemory;
	DWORD y;
	UINT x;
	UINT bpp = d;
	hdr[0] = 0;
	hdr[1] = 0;
	hdr[2] = bCompressed ? 10 : 2;
	hdr[3] = 0;
	hdr[4] = 0;
	hdr[5] = 0;
	hdr[6] = 0;
	hdr[7] = 0;
	hdr[8] = 0;
	hdr[9] = 0;
	hdr[10] = 0;
	hdr[11] = 0;
	hdr[12] = w % 256;
	hdr[13] = w / 256;
	hdr[14] = h % 256;
	hdr[15] = h / 256;
	hdr[16] = 8 * d;
	hdr[17] = 0;// ((d == 4) ? 2048 : 0);
	dwBytesSaved += 18;
	file.Write(&hdr, 18);
	UINT outcnt = 0;
	if (bCompressed)
		{
		for (y = 0; y < h; y++, src += ip)
			{
			for (x = 0;x < w;)
				{
				UINT i,cnt;
				BYTE * tp = src + x * bpp;
				UINT max;
				max = w - x;
				if (max > 128)
					max = 128;
				for (i = bpp; (i < max * bpp) && (tp[i-bpp] == tp[i]); i++);
				cnt = i / bpp;
				if (cnt > 1)
					{
					*dptr++ = 0x80 + cnt - 1;
					for (i = 0; i < bpp; i++)
						*dptr++ = tp[i];
					outcnt += 1 + bpp;
					x += cnt;
					continue;
					}
				for (cnt = 1; cnt < max; cnt++)
					{
					for (i = 0; i < bpp; i++)
						if (tp[(cnt-1)*bpp+i] != tp[(cnt*bpp)+i])
							break;
					if (i >= bpp)
						break;
					}
				*dptr++ = cnt - 1;
				for (i = 0; i < cnt * bpp; i++)
					*dptr++ = tp[i];
				outcnt += 1 + cnt * bpp;
				x += cnt;
				}
			}
		}
	else
		{
		for (y = 0; y < h; y++)
			memmove(dst+y*op,src+y*ip,op);
		outcnt = h * op;
		}
	file.Write(dst, outcnt);
	dwBytesSaved += outcnt;
	delete [] dst;
//	return dwBytesSaved;
	return 0;
}

int PutCimage(LPCSTR pName, void * BitmapData, int which)
{
	CImage image;
	LPBITMAPINFOHEADER lpBitmap = (LPBITMAPINFOHEADER)BitmapData;
	int nWidth = (int)lpBitmap->biWidth;
	int nHeight = (int)lpBitmap->biHeight;
	int bits = (int)lpBitmap->biBitCount;
	BOOL bRes;
	if (bits == 32)
		bRes = image.CreateEx(nWidth, nHeight,32,
					BI_RGB,0,CImage::createAlphaChannel);
	else
		bRes = image.Create(nWidth, nHeight,bits);
	if (!bRes)
	{
//		LASTERR;
		return 99;
	}
	BYTE * lpBits = (BYTE *)(lpBitmap + 1);
	int pitch = 4 * ((bits * nWidth+31) / 32);
	int x, y,d;
	d = bits / 8;
	for (y = 0; y < nHeight;y++, lpBits += pitch)
		{
		for (x = 0; x < nWidth; x++)
			{
			BYTE * p = (BYTE *)image.GetPixelAddress(x,nHeight - 1 - y);
			p[0] = lpBits[x*d+0];
			p[1] = lpBits[x*d+1];
			p[2] = lpBits[x*d+2];
			if (d == 4)
				p[3] = lpBits[x*d+3];
			}
		}
	int res;
	if (which == 0)
		res = image.Save(pName,ImageFormatPNG) == 0 ? 0 : 1;
	else if (which == 1)
		res = image.Save(pName,ImageFormatJPEG) == 0 ? 0 : 1;
	else
		res = image.Save(pName,ImageFormatTIFF) == 0 ? 0 : 1;
	image.Destroy();
	return res;
}
int FBExportStill (const char* inFilePath, void *inBitmapData, 
				int inBitmapSize, FBFileFormat inFileFormat)
{
	int res;
	if (inFileFormat ==  kPNGFileFormat)
		res = PutCimage(inFilePath,inBitmapData,0);
	else if (inFileFormat ==  kJPEGFileFormat)
		res = PutCimage(inFilePath,inBitmapData,1);
	else if (inFileFormat ==  kTIFFFileFormat)
		res = PutCimage(inFilePath,inBitmapData,2);
//	else if (inFileFormat ==  kPhotoshopFileFormat)
//		res = PutCimage(inFilePath,inBitmapData,3);
	else
		{
		CFile file;
		if (file.Open(inFilePath, CFile::modeCreate | CFile::modeWrite))
			{
			if (inFileFormat == kBMPFileFormat)
				res = PutBMP(file,inBitmapData);
			else if (inFileFormat == kTARGAFileFormat)
				res = PutTGA(file,inBitmapData);
			else
				res = kBadFormat;
			file.Close();
			}
		else
			res = kNoCreateFile;
		}
	return res;
}

#ifdef USEQT
void* FBImportMovieOpen (const char* inFilePath, int* outNumFrames,
					bool * bHasAudio)
{
	*outNumFrames = 0;
	*bHasAudio = 0;
	*outNumFrames = 24;
	return &pCap;
}

int FBImportMovieFrame (void* inMovieUserObject, ImageDataCallback inFunction, 
			int inFrame, void* inUserObject)
{
	return 0;
//HRESULT IMediaDet::GetBitmapBits(double StreamTime, long *pBufferSize, char
//*pBuffer, long Width, long Height);

//	long size;
//	char buf[200];
//HRESULT hres = IMediaDet::GetBitmapBits((double)inFrame, & size,
//				buf, 640, 480);
/*
	char name[300];
#ifdef _VC2005
	sprintf_s(name,sizeof(name),
			"c:\\flipbook\\scenes\\L001F00%d.BMP",(inFrame % 4)+1);
#else
	sprintf(name,"c:\\flipbook\\scenes\\L001F00%d.BMP",(inFrame % 4)+1);
#endif
	return FBImportStill (name,inFunction, inUserObject);
*/
}

int FBImportMovieClose (void* inMovieUserObject)
{
	return 0;
}


#define kFBAudioTempFilename "FlipBookTemp.wav"
#define kFBAudioComponentType 'spit'
#define kFBAudioComponentSubType 'WAVE'
#define kFBAudioComponentManufacturer 'soun'

FBAudioProgressCallback gFBAudioCallback = NULL;
BOOL gFBAudioCancel = FALSE;

static pascal OSErr FBMovieExportProgressProc (Movie inMovie, short inMessage, short inOperation, Fixed inPercentDone, long inRefCon)
{
	OSErr err = noErr;
	
	switch (inMessage) {
		case movieProgressOpen:
			break;

		case movieProgressUpdatePercent:
//			int percentage = Fix2Long(FixMul(inPercentDone, Long2Fix(100)));
//			if (gFBAudioCallback) {
//				(*gFBAudioCallback)(percentage, (void *)inRefCon);
//			}
			break;

		case movieProgressClose:
			break;
	}
	
	if (gFBAudioCancel) {
		err = userCanceledErr;
	}
	
	return err;
}

#pragma mark -

BOOL FBAudioNeedsExtracting (const char* inFilePath)
{
	// TODO: could also use CanQuickTimeOpenDataRef()
	int len = strlen (inFilePath);
	if ((len > 5) && (tolower(inFilePath[len-3]) == 'w') && (tolower(inFilePath[len-2]) == 'a') && (tolower(inFilePath[len-1]) == 'v')) {
		return FALSE;
	}
	else {
		return TRUE;
	}
}

int FBAudioExtractBegin (FBAudioProgressCallback inFunction, const char* inFilePath, void* inUserObject, BYTE** outAudioData, int* outAudioSize)
{
	OSErr err;
	
	gFBAudioCancel = FALSE;
	gFBAudioCallback = inFunction;
	EnterMovies();
	// save WAV to temp directory
	const char* tmp_name = kFBAudioTempFilename;
	char tmp_path[MAX_PATH];
	int len = GetTempPath (MAX_PATH, tmp_path);
	if (len + strlen (tmp_name) + 1 > MAX_PATH) {
		return kNoCreateFile;
	}

	strcat (tmp_path, tmp_name);

	// just create the file
	CFile* dest_file = new CFile (tmp_path, CFile::modeCreate);
	delete dest_file;

	// setup source and destination datarefs
	Handle source_dataref = NULL;
	OSType source_datatype = 0;
	CFStringRef source_cf = CFStringCreateWithCString (NULL, inFilePath, kCFStringEncodingUTF8);
 
	Handle dest_dataref = NULL;
	OSType dest_datatype = kQTFileTypeWave;
	CFStringRef dest_cf = CFStringCreateWithCString (NULL, tmp_path, kCFStringEncodingUTF8);
	
	MovieProgressUPP proc = NewMovieProgressUPP (FBMovieExportProgressProc);
	
	err = QTNewDataReferenceFromFullPathCFString (source_cf, (UInt32)kQTNativeDefaultPathStyle, 0, &source_dataref, &source_datatype);
	if (err == noErr) {

		err = QTNewDataReferenceFromFullPathCFString (dest_cf, (UInt32)kQTNativeDefaultPathStyle, 0, &dest_dataref, &dest_datatype);
		if (err == noErr) {

			// signature fo the WAV export component
			ComponentDescription desc;
			desc.componentType = kFBAudioComponentType;
			desc.componentSubType = kFBAudioComponentSubType;
			desc.componentManufacturer = kFBAudioComponentManufacturer;
			desc.componentFlags = 0;
			desc.componentFlagsMask = 0;
			
			Component comp = FindNextComponent(NULL, &desc);

			ComponentInstance comp_instance;
			err = OpenAComponent (comp, &comp_instance);
			if (err == noErr) {
			
				MovieExportSetProgressProc (comp_instance, proc, (long)inUserObject);

				Movie source_movie;
				
				err = NewMovieFromDataRef (&source_movie, 0, NULL, source_dataref, source_datatype);
				if (err == noErr) {

					err = (OSErr)MovieExportToDataRef (comp_instance, dest_dataref, dest_datatype, source_movie, NULL, 0, GetMovieDuration (source_movie));
					if (err == noErr) {
					
						// load data to return
						CFile file;
						DWORD mode = CFile::modeRead;
						if (file.Open (tmp_path, mode)) {
							UINT maxc = (UINT)file.GetLength();
							BYTE * p = new BYTE[maxc];
							UINT size = file.Read(p,maxc);
							file.Close();
							if (size != maxc) {
								err = ioErr;
							}

							*outAudioData = p;
							*outAudioSize = size;
						}
						else {
							err = ioErr;
						}				
					}

					DisposeMovie (source_movie);
				}
				
				CloseComponent (comp_instance);
			}			

			DisposeHandle (dest_dataref);
		}
		
		DisposeHandle (source_dataref);
	}
	
	// more cleanup
	DeleteFile (tmp_path);
	DisposeMovieProgressUPP (proc);
	
	int result;
	switch (err) {
		case noErr:
			result = kNoError;
			break;
			
		case ioErr:
		case fnfErr:
			result = kNoOpenFile;
			break;
			
		case badFileFormat:
		default:
			result = kUnknownError;
	}
	
	return result;
}

int FBAudioExtractCancel ()
{
	gFBAudioCancel = TRUE;
	return kNoError;
}

int FBAudioExtractCleanup (BYTE* inAudioData)
{
	delete[] inAudioData;
	ExitMovies();
	return kNoError;
}
#endif

//Make sure you are calling InitializeQTML and EnterMovies beforehand to
//initialize QuickTime and the Movie Toolbox


//
// new record logic
//

int FBVideoRecordBegin (CWnd * pVideo,LPCSTR szVidDevice, LPCSTR szAudDevice,
			LPCSTR szCompressor, LPCSTR szCaptureFile,
						BOOL bAudio, BOOL bPreview, UINT size)
{
	BOOL bRes = RCapInit(pVideo->m_hWnd,szVidDevice, szAudDevice,szCompressor,
				szCaptureFile, bAudio, bPreview, size);
	return bRes;
}

int FBVideoRecordCopy(LPCSTR dst)
{
	return RCapCopy(dst);
}

int FBVideoRecordDeviceCount (int which)
{
	return RCapDeviceCount(which);
}

int FBVideoRecordDeviceName (char* ioDeviceName, int inDeviceIndex, int which)
{
	return RCapGetName(ioDeviceName, inDeviceIndex, which);
}

int FBVideoRecordSelectDevice (int inDeviceIndex, int which)
{
	return RCapSelectDevice(inDeviceIndex, which);
}

int FBVideoRecordCurrentDevice(int which)
{
	return RCapCurrentDevice(which);
}

/*
	FBVideoCaptureOption
	if name is null then execute option
	if index & 256 then stop stream before dialog, then resume
	return zero if index is >= than options
*/
int FBVideoRecordOption (int index, int which, char * OptionName)
{
	return RCapOption(index, which, OptionName);
}

//int FBVideoRecordSetup( HWND m_hWndView, CWnd * pParent,
//			LPCSTR szVidDevice, LPCSTR szAudDevice)
//{
//	return 0;
//}

int FBVideoRecordStop (BOOL bDestroy)
{
	return RCapStop(bDestroy);
}

//int FBVideoRecordSize(int & w, int & h)
//{
//	return RCapCapSize(w,h);
//}

int FBVideoRecordCapture(BOOL bStart)
{
	int v;
	if (bStart)
		v = RCapStartRecord();
	else
		v = RCapStopRecord();
	return v;
}

int FBVideoRecordPreview(BOOL bPreview)
{
	return RCapPreview(bPreview);
}

int FBVideoRecordDoAudio(BOOL bAudio)
{
	return RCapDoAudio(bAudio);
}

/*
int FBVideoRecordOptions(FBRECORD_OPT * pOptions, BOOL bPut)
{
	return RCapOptions(pOptions, bPut);
}
*/

BOOL FBVideoRecordStatus(LPSTR szStatus, int size, BOOL bFull)
{
	return RCapStatus(szStatus, size, bFull);
}

int FBVideoRecordGetGUID(LPSTR szGUID)
{
	RCapGetGUID(szGUID);
	return 0;
}

