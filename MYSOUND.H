#ifndef _MYSOUND_H_
#define _MYSOUND_H_
#include "mmsystem.h"
#include "dsound.h"

#ifndef mmioFOURCC
#define mmioFOURCC( ch0, ch1, ch2, ch3 ) \
	( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |	\
	( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )
#endif

class CScene;
class CMyWave;

class CMySound
{
public:
	CMySound();
	~CMySound();
	int	 LoadWaveFile(int idd, LPCSTR name);
	int	 LoadWaveData(int idd, LPBYTE pData, UINT size);
	int  SaveWaveData(BYTE * p, UINT size, LPCSTR name);
	BOOL SoundSetup(UINT w, UINT h, UINT frames, UINT rate, UINT snip);
	void SetScene(CScene * pScene) { m_pScene = pScene;};
	void SetRate(UINT rate);
	UINT Snip(UINT Frames = 0);
	void SetAVMarks(int idd, double time, UINT Frame);
	double AudioMark(int idd, BOOL bMax = FALSE);
	UINT VideoMark(int idd);
	BYTE * Image(int idd, UINT Frame);
	int Play(UINT Frame, int which = 0);
//	UINT Bytes() {return m_bytesamp;};
	BYTE * WaveData(UINT Frame);
	UINT WaveLength(UINT Frames);
	UINT GetFormat( WAVEFORMATEX * ppcm);
	BOOL IsValid(int idd = -1) const;
	UINT Rate() {return m_sampersec;};
	UINT Bytes() {return m_bytesamp;};
	void SetFrameCount(UINT frames);
	void Allow(BOOL bRun) { if (m_bAllow = bRun) Resample();};
	BOOL Enabled(int which, int v = -1);
	int  Volume(int idd, int volume = 999); // if id == 99, then master
protected:
	int  m_nVolume;
	BOOL m_bAllow;
	BOOL m_bCanDo32;
	UINT WaveSize(int which);
	int SetupBuffer();
	int Resample();
	void SampleWave(CMyWave * pWave);
	int m_which;
	int CreateDSObject();
//	int GetWaveFile(LPCSTR name);
//	int SetWaveData(BYTE * pPCM, BYTE * pData, UINT size);
	BOOL SetSoundData(int which, LPVOID pSoundData, DWORD dwSoundSize);
	BOOL CreateSoundBuffer(int which, WAVEFORMATEX * pcmwf);
	BOOL m_bSingle;
	UINT m_width;
	UINT m_height;
	UINT m_pitch;
	UINT m_rate;
	UINT m_snip;
	UINT m_frame;
	UINT m_frames;
	UINT m_sampersec;
	UINT m_bytesamp;
	UINT m_channels;
	UINT m_bitsample;
	WAVEFORMATEX m_pcm;
	BYTE * m_pImage;
	BYTE * m_pWave;
	UINT	m_nWaves;
	CMyWave * * m_pWaves;
	CScene * m_pScene;
private:
	LPDIRECTSOUNDBUFFER MyDsb(int which)
	{return which ? m_pBigDsb : m_pSmallDsb;};
	LPDIRECTSOUNDBUFFER m_pSmallDsb;
	LPDIRECTSOUNDBUFFER m_pBigDsb;
	static LPDIRECTSOUND m_lpDirectSound;
	static DWORD m_dwInstances;
};
#endif
