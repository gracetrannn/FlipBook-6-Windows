#include "stdafx.h"
#include "myqt.h"

#ifdef USEQT
const int kBaseTimeScale = 60;

CMyQT::CMyQT()
{
	m_state = 0;
	m_imageDesc = NULL;
	m_compressedData = NULL;
	m_compressedDataPtr = NULL;
	m_resRefNum = 0;
	m_pMovie = NULL;
	m_savedPort = NULL;
	m_gWorld = NULL;
	m_savedGD = NULL;	
	m_pTrack = NULL;
	m_pMedia = NULL;
	m_hBackDC = NULL;
	m_hBackBitmap = NULL;
}

CMyQT::~CMyQT()
{
	CleanUp();
	ExitMovies();
//	TerminateQTML();
}

int CMyQT::Setup(const char * name,
						int width, int height, int depth, int rate, int frames)
{
	CleanUp();
	EnterMovies();

typedef struct tagMYBITMAPINFO { 
  BITMAPINFOHEADER bmiHeader; 
  RGBQUAD          bmiColors[256]; 
} MYBITMAPINFO; 
	MYBITMAPINFO	bmpInfo;


	m_width = width;
	m_height = height;
	m_depth = depth;
	m_rate = rate;
	depth = 24;
	ZeroMemory(&bmpInfo,sizeof(bmpInfo));
	bmpInfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	bmpInfo.bmiHeader.biBitCount = depth;
	bmpInfo.bmiHeader.biCompression = BI_RGB;
	bmpInfo.bmiHeader.biSizeImage = m_height*4* ((m_width * depth + 31) / 32);
	bmpInfo.bmiHeader.biPlanes = 1;
	bmpInfo.bmiHeader.biHeight = m_height;
	bmpInfo.bmiHeader.biWidth = m_width;
	bmpInfo.bmiHeader.biClrUsed = 256;
	bmpInfo.bmiHeader.biClrImportant = 256;
	if (depth == 8)
		{
		int i;
		for (i = 0; i < 256; i++)
			{
  			bmpInfo.bmiColors[i].rgbBlue = i; 
  			bmpInfo.bmiColors[i].rgbRed = i; 
  			bmpInfo.bmiColors[i].rgbGreen = i; 
  			bmpInfo.bmiColors[i].rgbReserved = 0; 
			}
		}
	
	m_hBackDC = CreateCompatibleDC(0);
	m_hBackBitmap = CreateDIBSection(m_hBackDC,(BITMAPINFO *)&bmpInfo,
								DIB_RGB_COLORS,(void **)&m_pBits,NULL,NULL);

	int res;
	if(m_hBackBitmap == NULL)
		res = 22;
	else
		res = InitGraphicsWorld(name);

	if (res)
		CleanUp();
	else
		m_state = 1;  // ready to start
	return res;
}

int CMyQT::InitGraphicsWorld(LPCTSTR lpszFileName)
{

	OSErr		err = noErr;
	FSSpec		fileSpec;
	char		szAbsFileName[MAX_PATH];

	if(_fullpath(szAbsFileName,lpszFileName,MAX_PATH)==NULL)
		return 33;
	if(NewGWorldFromHBITMAP(&m_gWorld,NULL,NULL,NULL,
						m_hBackBitmap,m_hBackDC)!= noErr)
		return 34;
	GetGWorld(&m_savedPort,&m_savedGD);
	SetGWorld(m_gWorld,NULL);

	c2pstrcpy(m_FileName,szAbsFileName);
	FSMakeFSSpec(0,0,m_FileName,&fileSpec);

	err = CreateMovieFile(&fileSpec,FOUR_CHAR_CODE('TVOD'),smCurrentScript,
			createMovieFileDeleteCurFile | createMovieFileDontCreateResFile,
				&m_resRefNum,&m_pMovie);
	if(err != noErr)
		return 35;
	m_pTrack = NewMovieTrack (m_pMovie, 
							FixRatio(m_width,1),//Do not cast to (short) - Overflow occurs
							FixRatio(m_height,1), 
							0);
	if(m_pTrack==NULL)
		return 36;
	m_trackFrame.left = 0;
	m_trackFrame.top = 0; 
	m_trackFrame.right = m_width;	//Do not cast to (short) - Overflow occurs
	m_trackFrame.bottom = m_height ;

	m_pMedia = NewTrackMedia (m_pTrack, VideoMediaType,
							kBaseTimeScale * m_rate,
							nil, 0);
	if(m_pMedia==NULL)
		return 37;
	return 0;

}

int CMyQT::Append(BYTE * bits, int size)
{
	int res;
	if (!m_state)
		return 1;
	if (m_depth == 8)
		{
		int ipitch = 4 * ((m_width * m_depth + 31) / 32);
		int opitch = 4 * ((m_width * 24 + 31) / 32);
		int x,y;
		BYTE * dp = m_pBits + m_height * opitch;
		for (y = 0; y < m_height; y++)
			{
			dp -= opitch;
			for (x = 0; x < m_width;x++)
				{
				BYTE v = bits[x];
				dp[3*x+0] = v;
				dp[3*x+1] = v;
				dp[3*x+2] = v;
				}
			bits += ipitch;
			}
		}
	else
		{
		int pitch = 4 * ((m_width * m_depth + 31) / 32);
		int y;
		BYTE * dp = m_pBits + m_height * pitch;
		for (y = 0; y < m_height; y++)
			{
			dp -= pitch;
			memmove(dp,bits,pitch);
			bits += pitch;
			}
		}
	if (m_state == 1)
		{
		if (res = InitialSetup())
			CleanUp();
		}
	else
		res = 0;
	if (!res)
		{
		OSErr err = noErr;
		err = CompressImage (m_gWorld->portPixMap, 
						&m_trackFrame, 
						VIDEO_CODEC_QUALITY,
						VIDEO_CODEC_TYPE,
						m_imageDesc, 
						m_compressedDataPtr );
		if(err != noErr)
			res = 2;
		else
			{
			err = AddMediaSample(m_pMedia, 
						m_compressedData,
						kNoOffset,	/* no offset in data */
						(**m_imageDesc).dataSize, 
						kBaseTimeScale,
						(SampleDescriptionHandle)m_imageDesc, 
						kAddOneVideoSample,	/* one sample */
						kSyncSample,	/* self-contained samples */
						nil);
			if(err != noErr)
				res = 3;
			else
				m_state = 2;
			}
		}
	return res;
}

void CMyQT::CleanUp()
{
	if (m_imageDesc)
		{
		DisposeHandle ((Handle)m_imageDesc);
		m_imageDesc = NULL;
		}
	if (m_compressedData)
		{
		DisposeHandle (m_compressedData);
		m_compressedData = NULL;
		}
	if(m_resRefNum)
		{
		CloseMovieFile(m_resRefNum);
		m_resRefNum=0;
		}		
	if(m_pMovie)
		{	
		DisposeMovie(m_pMovie);
		m_pMovie = NULL;
		}
	if(m_savedPort)
		{
		SetGWorld(m_savedPort,m_savedGD);
		m_savedPort = NULL;
		}
	if(m_gWorld)
		{
		DisposeGWorld(m_gWorld);
		m_gWorld = NULL;
		}
	if(m_hBackDC)
		{
		DeleteDC(m_hBackDC);
		m_hBackDC = NULL;
		}
	if(m_hBackBitmap)
		{
		DeleteObject(m_hBackBitmap);
		m_hBackBitmap = NULL;
		}
	m_state = 0;
}

int CMyQT::Finish()
{
	if(m_state == 2)   // had we started
		{
		short resId = movieInDataForkResID;
		if(m_pMedia)
			EndMediaEdits (m_pMedia);
		if(m_pTrack)
			InsertMediaIntoTrack (m_pTrack, kTrackStart,/* track start time */
									kMediaStart, /* media start time */
									GetMediaDuration (m_pMedia),
									fixed1);
		if(m_pMovie)
			AddMovieResource (m_pMovie, m_resRefNum, &resId, m_FileName);
		}
	CleanUp();
	return 0;
}

int CMyQT::InitialSetup()
{
	long maxCompressedSize;
	OSErr err = noErr;

	Rect rect;
	rect.left = m_trackFrame.left;
	rect.top = m_trackFrame.top;
	rect.right = m_trackFrame.right;
	rect.bottom = m_trackFrame.bottom;

	if(GetSystemMetrics(SM_CXSCREEN)>rect.right)
		rect.right=GetSystemMetrics(SM_CXSCREEN);
	if(GetSystemMetrics(SM_CYSCREEN)>rect.bottom)
		rect.bottom=GetSystemMetrics(SM_CYSCREEN);

	err = GetMaxCompressionSize(m_gWorld->portPixMap,
								&rect, 
								kMgrChoose, /* let ICM choose depth */
								VIDEO_CODEC_QUALITY, 
								VIDEO_CODEC_TYPE    , 
								(CompressorComponent) anyCodec,
								&maxCompressedSize);
	if(err != noErr)
		return 7;

	m_compressedData = NewHandle(maxCompressedSize);
	if(m_compressedData==NULL)
		return 8;

	MoveHHi( m_compressedData );
	HLock( m_compressedData );
	m_compressedDataPtr = StripAddress( *m_compressedData );

	m_imageDesc = (ImageDescriptionHandle)NewHandle(4);
	if(m_imageDesc==NULL)
		return 9;

	err = BeginMediaEdits (m_pMedia);
	if(err != noErr)
		return 10;

	return 0;
}

int CMyQT::AddSound(void * inData, int inSize, int inNumChannels,
			int inSampleSize, int inSampleRate,int inFrames)
{
	OSErr err;
	Track audio_track = NewMovieTrack (m_pMovie, 0, 0, kFullVolume);
 	Media media = NewTrackMedia (audio_track, SoundMediaType,
									inSampleRate, NULL, 0);
		
	Handle snd_data = NewHandle (inSize);
	HLock (snd_data);
	memcpy (*snd_data, inData, inSize);
	HUnlock (snd_data);
		
	SoundDescriptionHandle snd_desc = (SoundDescriptionHandle) NewHandle (4);
	SetHandleSize ((Handle)snd_desc, sizeof(SoundDescription));

	(*snd_desc)->descSize = sizeof(SoundDescription);
	(*snd_desc)->resvd1 = 0;
	(*snd_desc)->resvd2 = 0;
	(*snd_desc)->dataRefIndex = 1;
	(*snd_desc)->compressionID = 0;
	(*snd_desc)->packetSize = 0;
	(*snd_desc)->version = 0;
	(*snd_desc)->revlevel = 0;
	(*snd_desc)->vendor = 0;
	(*snd_desc)->numChannels = inNumChannels;
	(*snd_desc)->sampleSize = inSampleSize;
	(*snd_desc)->sampleRate = (inSampleRate << 16);//IntToFixed (inSampleRate);

	int sample_size_bytes = inSampleSize / 8;
	int num_samples = inSize / (sample_size_bytes * inNumChannels);
	int res = 0;
	if (inSampleSize == 8)
		{
		(*snd_desc)->dataFormat = k8BitOffsetBinaryFormat;
		}
	else if (inSampleSize == 16) {
			(*snd_desc)->dataFormat = k16BitLittleEndianFormat;
		}
	else
		res = 21;
	if (!res)
		{
		err = BeginMediaEdits (media);
		if (err != noErr)
			res = 22;
		}
	if (!res)
		{
		err = AddMediaSample (media, 
					snd_data, 
					0,			/* offset in data */ 
					inSize, 
					1,//sample_size_bytes,	/* duration of each sound sample */
					(SampleDescriptionHandle)snd_desc, 
					num_samples, 
					0,			/* self-contained samples */ 
					NULL); 
		if (err != noErr)
			res = 23;
		}
	if (!res)
		{
		err = EndMediaEdits (media); 
		if (err != noErr)
			res = 24;
		}
	if (!res)
		{
	    err = InsertMediaIntoTrack (audio_track, 
					0,          /* track start time */ 
					0,          /* media start time */ 
					GetMediaDuration (media), 
					fixed1); 
		if (err != noErr)
			res = 24;
		}
	DisposeHandle ((Handle)snd_desc);
	DisposeHandle (snd_data);
	return res;
}
#endif
