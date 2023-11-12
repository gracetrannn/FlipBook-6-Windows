#if MAC

#include "FBQuickTime.h"
#include "FBQuickTimeAudio.h"
#import "FBFlashVideo.h"

#else

#ifndef _FBQuickTime
#define _FBQuickTime

#include "ImageUtils.h"

class CMySound;
/*
	FBQuickTime.h

	by Manton Reece
	Copyright 2005 Digicel.
	
	Changes:
	
		5/24/2005: Updated based on Rick's notes.
		4/18/2005: First version.
*/

typedef enum {
	kNoError = 0, 
	kNoCreateFile = 10,
	kNoOpenFile = 11,
	kBadFormat = 12,
	kBadInfo = 13,
	kNoMemory = 14,
	kMemoryError = 98,
	kUnknownError = 99
} FBResult;

typedef enum {
	kBMPFileFormat = 0, 
	kTARGAFileFormat = 1,
	kPNGFileFormat = 2, 
	kJPEGFileFormat = 3, 
	kPhotoshopFileFormat = 4,
	kTIFFFileFormat = 5,
	kPICTFileFormat = 6
} FBFileFormat;

/*
	gets info about file type
	if pExt is non zero, copies in file extension
	returns 1 if 24 bit support, 2 for 32, 4 for 8, ored together,
	zero if bad format
*/
int FBQTGetExt( char * pExt, FBFileFormat which);

/*
	FBQuickTimeInit

	Description:
		Should be called once after the application starts up to initialize the FBQuickTime library.
*/
void FBQuickTimeInit (void );
/*
	FBQuickTime();
	return non-zero if quicktime library is present
*/
int FBQuickTime(void);
/*
	FBExportMovieSetup

	Parameters:
		inFilePath: The native filepath to save the movie to (delimited by forward-slashes).
		inWidth: Width for the movie.
		inHeight: Height for the movie.
		inBitDepth: Depth for the movie (usually 24).
		inRate: Frames per second (e.g. 15).
		inNumFrames: Total number of frames that will be in the exported movie.
*/
void* FBExportMovieSetup (const char* inFilePath, int inWidth, int inHeight, int inBitDepth, int inRate, int inNumFrames);

/*
	FBExportMoviePut

	Parameters:
		inUserObject: Pointer returned from FBExportMovieSetup.
		inBitmapData: The bitmap data for one frame.
		inBitmapSize: Size of the data.
*/
int FBExportMoviePut (void *inUserObject, void *inBitmapData, int inBitmapSize);
int FBExportMovieSound (void *inUserObject, CMySound* inSound,
						UINT start_frame, UINT endframe);

/*
	FBExportMovieClose

	Parameters:
		inUserObject: Pointer returned from FBExportMovieSetup.
*/
int FBExportMovieClose (void* inUserObject);

/*
	FBVideoCaptureDeviceCount
*/
int FBVideoCaptureDeviceCount ();

/*
	FBVideoCaptureDeviceName
*/
int FBVideoCaptureDeviceName (char* ioDeviceName, int inDeviceIndex);

/*
	FBVideoCaptureSelectDevice
*/
int FBVideoCaptureSelectDevice (int inDeviceIndex);

/*
	FBVideoCaptureActiveDeviceName
*/
int FBVideoCaptureActiveDeviceName (char* ioDeviceName);

/*
	FBVideoCaptureOption
	if name is null then execute option
	if index & 256 then stop stream before dialog, then resume
	return zero if index is >= than options
*/
int FBVideoCaptureOption (int index, char * OptionName = 0);

/*
	FBVideoCaptureBegin
*/
int FBVideoCaptureBegin (ImageDataCallback inFunction, void* inUserObject, int inWidth, int inHeight);

int FBVideoSetup( HWND m_hWndView, CWnd * pParent, LPCSTR szDevice);
/*
	FBVideoCaptureStop
*/
int FBVideoCaptureStop (BOOL bDestroy);


int FBVideoCaptureSize(int & w, int & h, int &d);
/*
	FBExportStill

	Description:
		Save the bitmap data to the full filepath specified, using the file format specified.
	Parameters:
		inFilePath: The native filepath (delimited by forward-slashes).
		inBitmapData: Pointer to the bitmap data.
		inBitmapSize: Length of the data.
		inFileFormat: The format to export the data to (one of FBFileFormat enums above).
	Result:
		0 if successful, error code otherwise.
*/
int FBExportStill (const char* inFilePath, void *inBitmapData, int inBitmapSize, FBFileFormat inFileFormat);

/*
	FBImportMovieOpen
*/
void* FBImportMovieOpen (const char* inFilePath,
							int* outNumFrames, bool * bHasAudio);

/*
	FBImportMovieFrame

	Parameters:
		inMovieUserObject: The object returned from FBImportMovieOpen.
		inFunction: Function to call for each frame that is imported.
		inFrame: Frame number to fetch (0-based).
		inCallbackUserObject: Passed to the callback function.
*/
int FBImportMovieFrame (void* inMovieUserObject,
				ImageDataCallback inFunction, int inFrame,
						void* inCallbackUserObject);

/*
	FBImportMovieClose

	Parameters:
		inMovieUserObject: The object returned from FBImportMovieOpen.
*/
int FBImportMovieClose (void* inMovieUserObject);

/*!
    @function FBAudioProgressCallback
    @abstract Callback used for audio conversion progress and final audio data.
    @param inProgress Percentage complete from 0..100.
	@param inAudioData The WAV data or NULL if not ready yet.
	@param inAudioSize The WAV data size or 0 if not ready yet.
	@param inUserObject Same pointer passed to FBAudioExtractBegin().
    @result Return kNoError (0) if everything is okay. Call FBAudioExtractStop() if user cancelled.
*/
typedef unsigned int (*FBAudioProgressCallback)(int inProgress, void *inUserObject);

/*!
    @function FBAudioNeedsExtracting
    @abstract Returns TRUE if the file needs to be converted to work in FlipBook.
	@param inFilePath Full path to the file that  might need conversion.
*/

BOOL FBAudioNeedsExtracting (const char* inFilePath);


/*!
    @function FBAudioExtractBegin
	@param inFunction Callback of type FBAudioProgressCallback. If not NULL this will be called as the file is processed.
	@param inFilePath Full path to the file (usually MP3 or AIFF) to be converted
	@param inUserObject This object will be passed to the progress callback.
	@param inAudioData Pointer to the WAV data.
	@param inAudioSize Pointer to the WAV data size.
*/
int  FBAudioExtractBegin (FBAudioProgressCallback inFunction,
					const char* inFilePath, void* inUserObject,
					BYTE** outAudioData, int* outAudioSize);


/*!
    @function FBAudioExtractCancel
	@abstract If the user cancelled the conversion, call this inside the progress callback.
*/
int FBAudioExtractCancel ();

/*!
    @function FBAudioExtractCleanup
	@abstract Call this after FBAudioExtractBegin() returns to cleanup the audio data if successful.
	@param inAudioData Data to be disposed of.
*/
int  FBAudioExtractCleanup (BYTE * inAudioData);



/*
		new recorder stuff
		very similar to old capture
*/
int FBVideoRecordBegin (CWnd * pVideo, LPCSTR szVidDevice, LPCSTR szAudDevice,
			LPCSTR szCompressor, LPCSTR CaptureFile, 
					BOOL bAudio, BOOL bPreview, UINT size);

int FBVideoRecordCopy(LPCSTR dst);

int FBVideoRecordDeviceCount (int which);

int FBVideoRecordDeviceName (char* ioDeviceName, int inDeviceIndex, int which);

int FBVideoRecordSelectDevice (int inDeviceIndex, int which);

int FBVideoRecordCurrentDevice (int which);

/*
	FBVideoCaptureOption
	if name is null then execute option
	if index & 256 then stop stream before dialog, then resume
	return zero if index is >= than options
*/
int FBVideoRecordOption (int index, int which, char * OptionName = 0);

//int FBVideoRecordSetup( HWND m_hWndView, CWnd * pParent,
//			LPCSTR szVidDevice, LPCSTR szAudDevicer);
int FBVideoRecordStop (BOOL bDestroy);
int FBVideoRecordPreview(BOOL bOn);
int FBVideoRecordDoAudio(BOOL bOn);
int FBVideoRecordCapSize(UINT MBs);

int FBVideoRecordCapture(BOOL bStart);

int FBVideoRecordGetGUID(LPSTR szGUID);

BOOL FBVideoRecordStatus(LPSTR szStatus, int size, BOOL bAll = 0);


#endif

#endif
