#if !defined(FBRECOPT_H)
#define FBRECOPT_H
struct FBRECORD_OPT
{
	int mask; // 1 is preview, 2 is useframerate,
				// 4 is framerate, 8 is master, 16 is cap video
				// 32 is cap audio, 64 is cap cc
	BOOL bPreview;
	BOOL bUseRate;
	UINT FrameRate;
	UINT Master;
	BOOL bCaptureVideo;
	BOOL bCaptureAudio;
	BOOL bCaptureCC;
};

#endif
