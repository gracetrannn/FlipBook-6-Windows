#pragma once
#ifndef _MYQT_H_
#define _MYQT_H_
#undef DEBUG

#ifdef USEQT
#include <qtml.h>
#include <movies.h>
#endif

#define		kNoOffset 			0
#define		kMgrChoose			0
#define		kSyncSample 		0
#define		kAddOneVideoSample	1
#define		kTrackStart			0
#define		kMediaStart			0

#ifndef		VIDEO_CODEC_TYPE
#define		VIDEO_CODEC_TYPE	kJPEGCodecType
#endif
#ifndef		VIDEO_CODEC_QUALITY
#define		VIDEO_CODEC_QUALITY	codecNormalQuality
#endif

#ifdef USEQT
class CMyQT
{
public:
	CMyQT();
	~CMyQT();
	int Setup(const char * name,int width, int height, int depth, int rate,int frames);
	int Append(BYTE * bits, int size);
	int AddSound(void * pData,int size, int channels,int bits,int rate,int frames);
	int Finish();
protected:
	int 		m_state;
	int			m_width;
	int 		m_height;
	int 		m_depth;
	int 		m_rate;
	BYTE * 		m_pBits;
	HDC			m_hBackDC;
	HBITMAP		m_hBackBitmap;
	CGrafPtr	m_savedPort;
	GDHandle	m_savedGD;
	GWorldPtr	m_gWorld;
	Track		m_pTrack;
	Media		m_pMedia;
	Movie		m_pMovie;
	Str255		m_FileName;
	short		m_resRefNum;
	Rect		m_trackFrame ;
	Handle		m_compressedData;
	Ptr			m_compressedDataPtr;
	ImageDescriptionHandle m_imageDesc;

	TCHAR	m_szErrMsg[256];

	void	CleanUp();
	int 	InitialSetup();
	int		InitGraphicsWorld(LPCTSTR lpszFileName);
};
#endif
#endif
