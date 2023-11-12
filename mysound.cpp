#include "stdafx.h"
#include "mysound.h"
#include "math.h"
#ifdef USEQT
#include "fbqt.h"
#endif
#include "mmsystem.h"
#include "mmreg.h"
#include "cscene.h"
#ifndef DSBLOCK_ENTIREBUFFER
	#define DSBLOCK_ENTIREBUFFER        0x00000002
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

BOOL extern RemoveFile(LPCSTR file_name); // true if could not

#define FACTOR 100000


static void DSError( HRESULT hRes ) {
//DXTRACE_ERR("error",hRes);
	DPF("DSError:");
	switch(hRes) {
		case DS_OK: DPF("NO ERROR"); break;
		case DSERR_ALLOCATED: DPF("ALLOCATED"); break;
		case DSERR_INVALIDPARAM: DPF("INVALIDPARAM"); break;
		case DSERR_OUTOFMEMORY: DPF("OUTOFMEMORY"); break;
		case DSERR_UNSUPPORTED: DPF("UNSUPPORTED"); break;
		case DSERR_NOAGGREGATION: DPF("NOAGGREGATION"); break;
		case DSERR_UNINITIALIZED: DPF("UNINITIALIZED"); break;
		case DSERR_BADFORMAT: DPF("BADFORMAT"); break;
		case DSERR_ALREADYINITIALIZED: DPF("ALREADYINITIALIZED"); break;
		case DSERR_BUFFERLOST: DPF("BUFFERLOST"); break;
		case DSERR_CONTROLUNAVAIL: DPF("CONTROLUNAVAIL"); break;
		case DSERR_GENERIC: DPF("GENERIC"); break;
		case DSERR_INVALIDCALL: DPF("INVALIDCALL"); break;
		case DSERR_OTHERAPPHASPRIO: DPF("OTHERAPPHASPRIO"); break;
		case DSERR_PRIOLEVELNEEDED: DPF("PRIOLEVELNEEDED"); break;
		default: DPF("%lu\n",hRes);
			break;
	}
}

LPDIRECTSOUND CMySound::m_lpDirectSound;
DWORD CMySound::m_dwInstances;


class CMyWave
{
public:
	CMyWave();
	~CMyWave();
//	int	 LoadWaveFile(LPCSTR name);
//	int	 LoadWaveData(LPBYTE pData, UINT size);
	BOOL Changed(BOOL bClear = 0){if (bClear) m_bDirty = 0;return m_bDirty;};
	void SetAVMarks(UINT time, UINT index);
	UINT AudioMark(UINT factor = 0);
	UINT VideoMark() { return m_vmark;};
//	UINT Rate() {return m_sampersec;};
//	UINT Bytes() {return m_bytesamp;};
protected:
	void Image(BYTE * pDst, UINT frame, UINT rate, UINT w, UINT h);
	int GetWaveFile(LPCSTR name);
	int ProcessWaveFileData(BYTE* p, UINT size);
	int SetWaveData(BYTE * pPCM, BYTE * pData, UINT size);
	int PutWaveFileData(BYTE * pData, UINT size);
	BOOL m_bDirty;		// whenever marks or data change
	BOOL m_bEnabled;
	BOOL m_bFloating;
	int  m_nVolume;
	UINT m_width;
	UINT m_height;
	UINT m_pitch;
	UINT m_vmark;
	UINT m_smark;
	UINT m_sampersec;
	UINT m_bytesamp;
	UINT m_channels;
	UINT m_maxsample;
	UINT m_bitsample;
	WAVEFORMATEX m_pcm;
	UINT m_OrigWaveSize;		// original size
	BYTE * m_pWave;

friend class CMySound;
};

CMySound::CMySound()
{
	++m_dwInstances;
	m_lpDirectSound = 0;
	m_pBigDsb = 0;
	m_pSmallDsb = 0;
	m_pImage = 0;
	m_pWave = 0;
	m_bAllow = 0;
	m_rate = 24;
	m_snip = 3;
	m_sampersec = 0;
	m_bytesamp = 0;
	m_bSingle = 0;
	m_nWaves = 3;
	m_bCanDo32 = 0;
	UINT i;
	m_pWaves = new CMyWave * [m_nWaves];
	for (i = 0; i < m_nWaves;i++)
		m_pWaves[i] = new CMyWave;
	int	res = CreateDSObject();
	DPF("res:%d",res);
}

CMySound::~CMySound()
{
	if( m_pSmallDsb )
		m_pSmallDsb->Release();
	if (m_pBigDsb)
		m_pBigDsb->Release();

	if( !--m_dwInstances && m_lpDirectSound ) {
		m_lpDirectSound->Release();
		m_lpDirectSound = 0;
	}
	delete [] m_pWave;
	m_pWave = 0;
	delete [] m_pImage;
	m_pImage = 0;
	if (m_pWaves)
		{
		UINT i;
		for (i = 0; i < m_nWaves;i++)
			delete m_pWaves[i];
		delete [] m_pWaves;
		m_pWaves = 0;
		}
}

BOOL CMySound::Enabled(int which, int code /* = -1 */)
{
	CMyWave * pW = m_pWaves[which];
	BOOL bOld = pW->m_bEnabled;
	if (!pW->m_pWave)
		return 0;
	if (code == 1)
		pW->m_bEnabled = TRUE;
	else if (!code)
		pW->m_bEnabled = FALSE;
	else if (code == 2)
		pW->m_bEnabled ^= 1;
	if (bOld != pW->m_bEnabled)
		Resample();
	return pW->m_bEnabled;
}

int CMySound::Volume(int idd, int volume /* = 999 */)
{
	if (volume != 999)
		{
		if (idd == 99)
			{
			UINT vv = 32768 + 327 * volume;
			waveOutSetVolume(0,(vv << 16) + vv);
			m_nVolume = volume;
			}
		else
			{
			m_pWaves[idd]->m_nVolume = volume;
			Resample();
			}
		}
	if (idd == 99)
		return m_nVolume;
	else
		return m_pWaves[idd]->m_nVolume;
}

int CMySound::Resample()
{
	UINT i;
	UINT channels = 1;
	UINT sampersec = 8000;
	UINT bitssamp = 8;
	UINT cnt = 0;
	BOOL bFloating = 1;
	for (i = 0; i < m_nWaves; i++)
		{
		if (m_pWaves[i]->m_bEnabled)
			{
			cnt++;
			if (m_pWaves[i]->m_bitsample > bitssamp)
				bitssamp = m_pWaves[i]->m_bitsample;
			if (m_pWaves[i]->m_channels > channels)
				channels = m_pWaves[i]->m_channels;
			if (m_pWaves[i]->m_sampersec > sampersec)
				sampersec = m_pWaves[i]->m_sampersec;
			if (!m_pWaves[i]->m_bFloating)
				bFloating = 0;
			}
		}
	if (!cnt)
		{
		bFloating = 0;
		DPF("nothing enabled");
		}
	m_bSingle = (cnt == 1 ? 1 : 0);
	if (bFloating)
		m_bSingle = 0;		// must convert floating samples
	if (!m_bCanDo32 && (bitssamp > 16)) 
		{
		m_bSingle = 0;		// xp acnnot do > 16 bit
		bitssamp = 16;
		}
	m_sampersec = sampersec;
	m_bytesamp = channels * ((bitssamp + 7) / 8);
	m_bitsample = bitssamp;
	m_channels = channels;

	m_pcm.wFormatTag = WAVE_FORMAT_PCM;
	m_pcm.nChannels = channels;
	m_pcm.nSamplesPerSec = sampersec;
	m_pcm.nBlockAlign = m_bytesamp;
	m_pcm.wBitsPerSample = bitssamp;
    m_pcm.nAvgBytesPerSec = sampersec * m_pcm.nBlockAlign; 
	m_pcm.cbSize = 0;
	delete [] m_pWave;

	UINT size = WaveSize(1);
	m_pWave = new BYTE[size];
	int res = CreateSoundBuffer(0, &m_pcm);
DPF("small:%d",res);
	if (!res)
		res = CreateSoundBuffer(1, &m_pcm);
DPF("big:%d",res);
	if (!res)
		res = SetupBuffer();
	return res;
}

void  CMySound::SetFrameCount(UINT frames)
{
	if (m_frames != frames)
		{
		m_frames = frames;
		if (m_bytesamp)
			Resample();
		}
}

#define MAXVALUE 0x7fffffff
#define FACTOR8 0x1020480      // 0x7fffffff / 127
#define FACTOR16 0x10002		// 0x7fffffff / 32767
#define FACTOR24 0x100			// 0x7fffffff / 
#define SILENCE 128				// factor 8 only
typedef struct {
	union {
		BYTE bit[4];
		int act;
	};} MYSAMP;
int mysamp (UINT c, BYTE *pp)
	{
	int z;
	if (c > 100)
		{
		z = (int)(MAXVALUE * *((float *)pp));
		}
	else if (c == 8)
		z = (int)*pp - SILENCE;
	else if (c == 16)
		z = *((short *)pp);
	else if (c == 24)
		{
		MYSAMP q;
		q.bit[0] = 0;
		q.bit[1] = pp[0];
		q.bit[2] = pp[1];
		q.bit[3] = pp[2];
		z = q.act;
		}
	else
		{
		z = *((int *)pp);
		}
	return z;
}

void putsamp(BYTE * op,UINT oc, int acc)
{
	if (oc == 8)
		*op = SILENCE + (acc / FACTOR8);
	else if (oc == 16)
		*((short *)op) = (short)(acc / FACTOR16);
	else if (oc == 24)
		{
		MYSAMP q;
		q.act = acc;
		op[0] = q.bit[1];
		op[1] = q.bit[2];
		op[2] = q.bit[3];
		}
	else
		*((int *)op) = acc;
}

//#define MYSAMP(z) if ((c == 9) ||(c == 10)){ z = (int)*pp - 127;} \
//		else if ((c == 17) || (c == 18)) { z = *((short *)pp);}\
//		else if ((c == 33) || (c == 34)) { z = *((int *)pp);}

void CMySound::SampleWave(CMyWave * pWave)
{
	UINT i, mint;
	if (!pWave->m_bEnabled)
		return;
	BYTE * ip = pWave->m_pWave;	
	BYTE * op = m_pWave;
	UINT isamp = pWave->m_OrigWaveSize / pWave->m_bytesamp;
	_int64 iit = isamp;
	iit = iit * FACTOR / pWave->m_sampersec;
	UINT maxt = (UINT)iit - pWave->m_smark + pWave->m_vmark;
	if (pWave->m_smark >= pWave->m_vmark)
		{
		mint = 0;
		}
	else
		{
		mint = pWave->m_vmark - pWave->m_smark;
		}
	UINT idelta, odelta;
	if (m_bSingle && !pWave->m_nVolume)
		{
		ASSERT(m_bytesamp == pWave->m_bytesamp);
		_int64 st;
		if (mint)
			{
			st = mint;
			st *= m_sampersec;
			st /= FACTOR;
			odelta = m_bytesamp * (UINT)st;
			idelta = 0;
			}
		else
			{
			st = pWave->m_smark - pWave->m_vmark;
			st *= m_sampersec;
			st /= FACTOR;
			odelta = 0;
			idelta = m_bytesamp * (UINT)st;
			}
		ip += idelta;
		op += odelta;
		UINT lng = pWave->m_OrigWaveSize - idelta;
		if (lng > (WaveSize(1)-odelta))
			lng = WaveSize(1)-odelta;
		memcpy(op, ip, lng);
		return;
		}
	UINT osamples = WaveSize(1) / m_bytesamp;
    UINT oc = m_bitsample;
    UINT ic = pWave->m_bitsample;
	if (pWave->m_channels > 1)
		idelta = pWave->m_bytesamp / pWave->m_channels;
	else
		idelta = 0;
	if (m_channels > 1)
		odelta = m_bytesamp / m_channels;
	else
		odelta = 0;

	_int64 ifactor;
	_int64 ofactor;
	if (oc == 8)
		ofactor = FACTOR8;
	else if (oc == 16)
		ofactor = FACTOR16;
	else
		ofactor = 1;
	if (ic == 8)
		ifactor = FACTOR8;
	else if (ic == 16)
		ifactor = FACTOR16;
	else
		ifactor = 1;
	if (pWave->m_nVolume )
		{
		double ratio;
		ratio = pWave->m_nVolume;
		//ratio = 100.0;
		ratio /= 100.0;
		ratio = exp(log(2.0) * ratio); // DB to ratio
		//ratio = 1.2;
		ratio *= ifactor;
		ifactor = (int)ratio;
		}
	if (pWave->m_bFloating)
		ic += 100;
	for (i = 0; i < osamples; i++, op += m_bytesamp)
		{
		_int64 tt = i;
		tt = (tt * FACTOR + m_sampersec/ 2) / m_sampersec;
		UINT t = (UINT)tt;
		if (t < mint)
			continue;
		if (t > maxt)
			break;
		tt -= mint;
		tt *= pWave->m_sampersec;
		tt +=  FACTOR / 2;
		tt /= FACTOR;
		UINT offset =  pWave->m_bytesamp * (UINT)tt;
		if (offset > pWave->m_OrigWaveSize)
			break;
		BYTE * p = ip + offset;
		_int64 acc = ofactor * mysamp(oc,op);
		_int64 s1 = ifactor * mysamp(ic,p);

		if ((acc < 0) && (s1 < 0)) {
			acc = (acc + s1) + ((acc * s1) / MAXVALUE);
		}
		else if ((acc > 0) && (s1 > 0)) {
			acc = (acc + s1) - ((acc * s1) / MAXVALUE);
		}
		else {
			acc = acc + s1;
		}
		
//		acc = 3 * acc / 4;
			
/*
 if ( bufferA[i] < 0 && bufferB[i] < 0 ) {
	 // If both samples are negative, mixed signal must have an amplitude between 
	 // the lesser of A and B, and the minimum permissible negative amplitude
	 outputBuffer[i] = (bufferA[i] + bufferB[i]) - ((bufferA[i] * bufferB[i])/INT16_MIN);
 } else if ( bufferA[i] > 0 && bufferB[i] > 0 ) {
	 // If both samples are positive, mixed signal must have an amplitude between the greater of
	 // A and B, and the maximum permissible positive amplitude
	 outputBuffer[i] = (bufferA[i] + bufferB[i]) - ((bufferA[i] * bufferB[i])/INT16_MAX);
 } else {
	 // If samples are on opposite sides of the 0-crossing, mixed signal should reflect 
	 // that samples cancel each other out somewhat
	 outputBuffer[i] = bufferA[i] + bufferB[i];
 }
 */
			
//		NSLog (@"samples: x + %f = %f | %llu, %llu", new_acc, new_s1, acc, s1);
		putsamp(op,oc, (int)acc);
		if (odelta)	// stereo ?
			{
			acc = ofactor * mysamp(oc,op+odelta);
			if (idelta)
				s1 = ifactor * mysamp(ic,p+idelta);
			if ((acc < 0) && (s1 < 0)) {
				acc = (acc + s1) + ((acc * s1) / MAXVALUE);
			}
			else if ((acc > 0) && (s1 > 0)) {
				acc = (acc + s1) - ((acc * s1) / MAXVALUE);
			}
			else {
				acc = acc + s1;
			}
//			acc = 3 * acc / 4;
			putsamp(op+odelta,oc, (int)acc);
			}
		}
	int res = SetSoundData(1, WaveData(0), WaveSize(1));
	DPF("res:%d",res);
	return; //res;
}

int CMySound::SetupBuffer()
{
	m_frame = 99999; // force creation of little buffer later
	BYTE silence;
	if ((m_bytesamp / m_channels) == 1)
		silence = SILENCE;
	else
		silence = 0;
//	m_bFloating = 0;
	UINT size = WaveSize(1);
	memset(m_pWave,silence,size);		// clear it

	UINT i;
	for (i = 0; i < m_nWaves; i++)
		{
		CMyWave * pWave = m_pWaves[i];
		if (pWave->m_pWave && pWave->m_bytesamp && pWave->m_bEnabled)
			SampleWave(pWave);
		}
	int res = SetSoundData(1, m_pWave, size);
	DPF("res:%d",res);
	return res;
}

int	 CMySound:: LoadWaveFile(int idd, LPCSTR name)
{
	DPF("loading wave,id:%d,name:%s",idd,name);
	int res = 0;
#ifdef USEQT
	FBQuickTimeInit();
	if (FBAudioNeedsExtracting (name))
		{
		BYTE* d = NULL;
		int d_size = 0;
		UINT index = m_pScene->EmbFind(name,0);//EMB_KIND_SOUND);
		if (index != NEGONE)  /// make a temp file for extraction
			{
			CFile file;
			char temp[300];
			int i,j;
			for (i = 0, j = 0; name[i]; i++)
				if (name[i] == '.') j = i + 1;
			strcpy(temp,"embedded.");
			if (j)
				strcat(temp,name+j);
			else
				strcat(temp,"tmp");
			DWORD mode = CFile::modeCreate | CFile::modeWrite;
  			CFileException ex;
			if (!file.Open(temp, mode, &ex))
				{
		//		ex.GetErrorMessage(name,300);
				char temp[500];
				sprintf(temp,"Could Not Create Temp File,%s",name);
				AfxMessageBox(temp,MB_OK);
				return 3;
				}
			UINT size = m_pScene->EmbData(index, 0);
			BYTE * pData = new BYTE[size];
			m_pScene->EmbData(index, pData);
			file.Write(pData, size);
			delete [] pData;
			file.Close();
			res = FBAudioExtractBegin (NULL, temp, this, &d, &d_size);
    		RemoveFile(temp);
			}
		else
			res = FBAudioExtractBegin (NULL, name, this, &d, &d_size);
		if (d) {
			res = m_pWaves[idd]->PutWaveFileData(d, d_size);
			FBAudioExtractCleanup (d);
		}
	}
	 else
#endif
		{
		UINT index = m_pScene->EmbFind(name,0);//EMB_KIND_SOUND);
		UINT size;
		if ((index != NEGONE) && (size = m_pScene->EmbData(index, 0)))
			{
			BYTE * pData = new BYTE[size];
			m_pScene->EmbData(index, pData);
			res = m_pWaves[idd]->PutWaveFileData(pData, size);
			delete [] pData;
			}
		else
			{
			res = m_pWaves[idd]->GetWaveFile(name);
			}
		}
	
	DPF("get wave:%d",res);
	if (!res)
		res = Resample();
	return res;
}

int	 CMySound:: LoadWaveData(int idd, LPBYTE pData, UINT size)
{
	DPF("loading wave data,id:%d",idd);
	UINT psize = sizeof(m_pcm);
	int res = m_pWaves[idd]->SetWaveData(pData,pData+psize,size-psize);
	if (!res)
		res = Resample();
	return res;
}

#define THRESH 20
#define RANGE (255 - THRESH)

BOOL CMySound::SoundSetup(UINT w, UINT h, UINT frames, UINT rate, UINT snip)
{
	m_width = w;
	m_height = h;
	m_pitch = 4 * ((w +3) / 4);
	m_frames = frames;
	m_rate = rate;
	m_snip = snip;
	delete [] m_pImage;
	m_pImage = new BYTE[m_pitch * m_height];
	if (!m_pImage)
		return 0;
	int res = Resample();
	return res ? 0 : 1;
}

void CMySound::SetRate(UINT rate)
{
	m_rate = rate;
	Resample();
}

UINT CMySound::Snip(UINT frames)
{
	if (!frames || (m_snip == frames)) return m_snip;
	m_snip = frames;
	if (m_pWave)
	{
	int res = CreateSoundBuffer(0, &m_pcm);
DPF("snip,small:%d",res);
	}
	return m_snip;
}

void CMySound::SetAVMarks(int idd, double time, UINT frame)
{
	UINT index = ((frame-1) * FACTOR + (m_rate / 2)) / m_rate;
	m_pWaves[idd]->SetAVMarks((UINT)(FACTOR * time), index);
	Resample();
}

double CMySound::AudioMark(int idd, BOOL bMax)
{
	double t;
	if (bMax)
		t = (double)m_pWaves[idd]->AudioMark(FACTOR);
	else
		t = (double)m_pWaves[idd]->AudioMark();
	return t / FACTOR;
}

UINT CMySound::VideoMark(int idd)
{
	return 1 + (m_pWaves[idd]->VideoMark() * m_rate + (FACTOR / 2 - 1)) / FACTOR;
}

BYTE * CMySound::Image(int idd,UINT Frame)
{
	if (m_pImage)
		{
		memset (m_pImage, 255, m_height * m_pitch);
		m_pWaves[idd]->Image(m_pImage,Frame, m_rate, m_width, m_height);
		}
	return m_pImage;
}

UINT CMySound::WaveSize(int which)
{
	if (which == 99)
		return m_bytesamp * ((m_frames * m_sampersec) / m_rate);
	else if (which)
		return m_bytesamp * (((m_frames + m_snip) * m_sampersec) / m_rate);
	else
		return m_bytesamp * ((m_snip * m_sampersec) / m_rate);
}

UINT CMySound::WaveLength(UINT Frames)
{
	return m_bytesamp * ((Frames * m_sampersec) / m_rate);
}

UINT CMySound::GetFormat( WAVEFORMATEX * ppcm)
{
	*ppcm = m_pcm;
	return sizeof(WAVEFORMATEX);
}

BYTE * CMySound::WaveData( UINT Frame)
{
	return m_pWave + m_bytesamp * ((Frame * m_sampersec) / m_rate);	
}

BOOL CMySound::CreateSoundBuffer( int which, WAVEFORMATEX * pcmwf)
{
	if (!m_lpDirectSound)
		return 1;
	DSBUFFERDESC dsbdesc;

	// Set up DSBUFFERDESC structure.
	memset(&dsbdesc, 0, sizeof(DSBUFFERDESC)); // Zero it out.
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	// Need no controls (pan, volume, frequency).
	dsbdesc.dwFlags = 
					DSBCAPS_STATIC |
//					DSBCAPS_STICKYFOCUS |
//					DSBCAPS_PRIMARYBUFFER
//					DSBCAPS_GLOBALFOCUS |
					DSBCAPS_CTRLVOLUME
//					|DSBCAPS_LOCDEFER
//					DSBCAPS_GLOBALFOCUS
					;
	dsbdesc.dwBufferBytes = WaveSize(which);
	dsbdesc.lpwfxFormat = pcmwf;    // Create buffer.
	HRESULT hRes;
DPF("createsndbuf:%d,%d",which,dsbdesc.dwBufferBytes);
	if (which)
		{
		if( m_pBigDsb )
			m_pBigDsb->Release();
		hRes = m_lpDirectSound->CreateSoundBuffer(&dsbdesc, &m_pBigDsb, 0);
		}
	else
		{
		if( m_pSmallDsb )
			m_pSmallDsb->Release();
		hRes = m_lpDirectSound->CreateSoundBuffer(&dsbdesc, &m_pSmallDsb, 0);
		}
	if( DS_OK != hRes)
		{
		// Failed.
		DSError(hRes);
		if (which)
			m_pBigDsb = 0;
		else
			m_pSmallDsb = 0;
		return 1;
	}

	return 0;
}

//#define MYDSBLOCK DSBLOCK_FROMWRITECURSOR
#define MYDSBLOCK DSBLOCK_ENTIREBUFFER
BOOL CMySound::SetSoundData(int which, void * pSoundData, DWORD dwSoundSize) {
	LPDIRECTSOUNDBUFFER pDsb = MyDsb(which);
	if (!pDsb) return 9; 
	LPVOID lpvPtr1, lpvPtr2;
	DWORD dwBytes1, dwBytes2;
	// Obtain write pointer.
	HRESULT hr = pDsb->Lock(0, WaveSize(which), &lpvPtr1, &dwBytes1,
				&lpvPtr2, &dwBytes2, MYDSBLOCK);    
    // If DSERR_BUFFERLOST is returned, restore and retry lock.
	if(DSERR_BUFFERLOST == hr) {
DPF("buffer lost");
		pDsb->Restore();
		hr = pDsb->Lock(0, 0, &lpvPtr1, &dwBytes1, 
				&lpvPtr2, &dwBytes2, MYDSBLOCK);
	}
DPF("Set Sound,size:%d,%d",dwBytes1,WaveSize(which));
DSError(hr);
	if(DS_OK == hr) {
if (lpvPtr2)
	{
DPF("got ptr,%d",dwBytes2);
	}
		// Write to pointers.
		memmove(lpvPtr1, pSoundData, dwBytes1);
		// Release the data back to DirectSound.
		hr = pDsb->Unlock(lpvPtr1, dwBytes1, 0, 0);
		if(DS_OK == hr)
            return 0;
	}
	// Lock, Unlock, or Restore failed.
	return 1;
}

int CMySound::Play(UINT Frame, int which)
{
	BOOL bSaved = FALSE;
	long volume;
DPF("play,frame:%d,which:%d",Frame, which);
	if( !IsValid())
		return 1;		// no chance to play the sound ...
	LPDIRECTSOUNDBUFFER pDsb;
	if (Frame == 9999)
		{
DPF("stopping:%d",m_which);
		pDsb = MyDsb(m_which);
		pDsb->Stop();
		return 0;
		}
	m_which = which;
	pDsb = MyDsb(which);
	pDsb->Stop();
	UINT offset;
	if (which)
		{
		offset = m_bytesamp * ((Frame * m_sampersec) / m_rate);	
		}
	else
		{
		HRESULT hhh;
		if ((hhh=pDsb->GetVolume(&volume)) == DS_OK)
			{
			pDsb->SetVolume(-10000);
			bSaved = TRUE;
			}
		else
			{
			DSError(hhh);
			DPF("no volume set");
			pDsb->SetVolume(-10000);
			}
		offset = 0;
		if (Frame != m_frame)
			{
			m_frame = Frame;
DPF("new frame");
		int res = SetSoundData(0, WaveData(m_frame), WaveSize(0));
//DPF("res:%d",res);
//get new data
			}
		}
//	pDsb->Stop();
DPF("Playing:%d",offset);
	pDsb->SetCurrentPosition(offset);
	HRESULT hres = pDsb->Play(0, 0, 0);
//	HRESULT hres = pDsb->Play(0, 0, DSBPLAY_TERMINATEBY_TIME );
DSError(hres);
	if ( DSERR_BUFFERLOST == hres) {
		// another application had stolen our buffer
		// Note that a "Restore()" is not enough, because
		// the sound data is invalid after Restore().
//		SetSoundData(which);
//m_pWave, m_WaveSize);
DPF("again");
		// Try playing again
		pDsb->Play(0, 0, 0);
	}
	if (bSaved)
		pDsb->SetVolume(volume);
	return 0;
}

BOOL CMySound::IsValid(int idd) const
{
	if (idd >= 0)
		{
		if (idd >= (int)m_nWaves)
			return FALSE;
		if (!m_pWaves[idd]->m_pWave)
			return FALSE;
		}
	else
		{		// check if any channels have been loaded
		for (idd = 0; idd < (int)m_nWaves;idd++)
			{
			if (m_pWaves[idd]->m_pWave)
				break;
			}
		if (idd >= (int)m_nWaves)
			return FALSE;
		}
	return (m_lpDirectSound &&
			m_pBigDsb && m_pSmallDsb && m_pWave) ? TRUE : FALSE;
}

int CMySound::CreateDSObject()
{
	m_bCanDo32 = 0;
	CWnd * pWnd = 0;
	if(pWnd == 0)
		pWnd = AfxGetApp()->GetMainWnd();

	ASSERT(pWnd != 0);
#ifndef FLIPBOOK_MAC
	ASSERT(::IsWindow(pWnd->GetSafeHwnd()));
#endif

//	ASSERT(m_pWave != 0);

	//////////////////////////////////////////////////////////////////
	// create direct sound object
	
	if( m_lpDirectSound == 0 ) {
		// Someone might use sounds for starting apps. This may cause
		// DirectSoundCreate() to fail because the driver is used by
		// anyone else. So wait a little before starting with the work ...
		HRESULT hRes = DS_OK;
		short nRes = 0;

		do {
			if( nRes )
				::Sleep(500);
			hRes = ::DirectSoundCreate(0, &m_lpDirectSound, 0);
			++nRes;
		} while( nRes < 10 && (hRes == DSERR_ALLOCATED || hRes == DSERR_NODRIVER) );

		if( hRes != DS_OK )
			return 1;
	
//	m_lpDirectSound->SetCooperativeLevel(pWnd->GetSafeHwnd(), DSSCL_NORMAL);
	if (m_lpDirectSound->SetCooperativeLevel(pWnd->GetSafeHwnd(),
			DSSCL_NORMAL) != DS_OK)
//			DSSCL_PRIORITY) != DS_OK)
		return 1;
	}

	ASSERT(m_lpDirectSound != 0);

#if MAC
		m_bCanDo32 = 1;
#else
	OSVERSIONINFO osinfo;
	osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	if (GetVersionEx(&osinfo))
		{
	//WORD dwPlatformId   = osinfo.dwPlatformId;
	//DWORD dwMinorVersion = osinfo.dwMinorVersion;
	DWORD dwMajorVersion = osinfo.dwMajorVersion;
	//DWORD dwBuildNumber  = osinfo.dwBuildNumber & 0xFFFF;	// Win 95 needs this
	
	if (dwMajorVersion > 5)
		{
		m_bCanDo32 = 1;	// vista and w7 can do 32bit
		}
	}
#endif
	return 0;
}

int CMySound::SaveWaveData(BYTE * sp, UINT size, LPCSTR name)
{
	int res = 0;
	CFile file;
	if (!file.Open(name, CFile::modeCreate | CFile::modeWrite))
		return 1;
	WAVEFORMATEX * spcm = (WAVEFORMATEX *)sp;
	BYTE hdr[100];
	BYTE * op = (BYTE *)&hdr;
	DWORD * p = (DWORD *)op;
	UINT lng = size + 28;
	sp += sizeof(m_pcm);
	size -= sizeof(m_pcm);	
	*p++ = SWAPV(mmioFOURCC('R', 'I', 'F', 'F') );
	*p++ = SWAPV(lng);
	*p++ = SWAPV(mmioFOURCC('W', 'A', 'V', 'E') );
	*p++ = SWAPV(mmioFOURCC('f', 'm', 't', ' ') );
	*p++ = SWAPV(sizeof(m_pcm));

	WAVEFORMATEX * ppcm = (WAVEFORMATEX *)p;
 
	*ppcm++ = *spcm;

	p = (DWORD*)ppcm;

//	*p++ = SWAPV(mmioFOURCC('f', 'a', 'c', 't') );
//	*p++ = SWAPV(4);
//	*p++ = SWAPV(0);
	*p++ = SWAPV(mmioFOURCC('d', 'a', 't', 'a') );
	*p++ = SWAPV(size);
	UINT siz = (UINT)((BYTE *)p - op);
	file.Write(op, siz);
	file.Write(sp,size);

	file.Close();
	return res;
}


CMyWave::CMyWave()
{
	m_pWave = 0;
	m_vmark = 0;
	m_smark = 0;
	m_sampersec = 0;
	m_bytesamp = 0;
	m_maxsample = 1;
	m_bEnabled = 0;
	m_nVolume = 0;
	m_bFloating = 0;
}

CMyWave::~CMyWave()
{
	delete [] m_pWave;
	m_pWave = 0;
}
/*
int	CMyWave::LoadWaveFile(LPCSTR name)
{
	DPF("loading wave:%s",name);
	int res = GetWaveFile(name);
	DPF("get wave:%d",res);
	m_bEnabled = res ? 0 : 1;
	return res;
}
int	CMyWave::LoadWaveData(LPBYTE pData, UINT size)
{
	return 0;
}
*/
void CMyWave::SetAVMarks(UINT time, UINT index)
{
 DPF("setav, time:%d,frm:%d",time,index);
	if ((time != m_smark) || (index != m_vmark))
		m_bDirty = TRUE;
	m_vmark = index;
	m_smark = time;
DPF("marks,%d,%d",m_smark,m_vmark);
}

UINT CMyWave::AudioMark(UINT factor)
{
	UINT q;
	if (factor)
		{
		q = m_OrigWaveSize / m_bytesamp;
		q *= factor;
		q /= m_sampersec;
		}
	else
		q = m_smark;
	return q;
}

int CMyWave::PutWaveFileData(BYTE * pData, UINT size)
{
	m_bFloating = 0;
	int res = ProcessWaveFileData(pData, size);
	m_bEnabled = res ? 0 : 1;
	return res;
}

int CMyWave::GetWaveFile(LPCSTR name)
{
	m_bEnabled = 0;
	m_bFloating = 0;
	CFile file;
	DWORD mode = CFile::modeRead;
	if (!file.Open(name, mode))
		return 1;
	UINT maxc = (UINT)file.GetLength();
	BYTE * p = new BYTE[maxc];
	UINT size = file.Read(p,maxc);
	file.Close();
	if (size != maxc)
		return 2;

	int res = ProcessWaveFileData(p, size);
	delete [] p;
	m_bEnabled = res ? 0 : 1;
	return res;
}

int CMyWave::ProcessWaveFileData(BYTE* p, UINT size)
{
	BYTE * pWaveData = 0;
	BYTE * pPCM = 0;
	UINT nWaveSize = 0;
	BOOL bDone = FALSE;
	DWORD * pdw = (DWORD *)p;
	DWORD dwRiff = SWAPV(*pdw++);
	DWORD dwLength = SWAPV(*pdw++);
	if (dwLength > size)
		{
//		delete [] p;
		return 3;
		}
	DWORD dwType = SWAPV(*pdw++);
	int result;
	if( dwRiff != mmioFOURCC('R', 'I', 'F', 'F') )
		result = 4;
	else if( dwType != mmioFOURCC('W', 'A', 'V', 'E') )
		result = 5;      // not a WAV
	else
		result = 0;
	if (result)
		{
		delete [] p;
		return result;
		}
	DWORD * pdwEnd = (DWORD *)((BYTE *)pdw + dwLength-4);
	result = 99;
	while((result==99) && (pdw < pdwEnd )) {
		dwType = SWAPV(*pdw++);
		dwLength = SWAPV(*pdw++);

		switch( dwType ) {
			case mmioFOURCC('f', 'm', 't', ' '):
				if( !pPCM ) {
					if( dwLength < sizeof(WAVEFORMAT) )
						result = 6;
					else
						{

						pPCM = (BYTE *) pdw;
	WAVEFORMATEX * ppPCM=(WAVEFORMATEX *)pPCM;
	if (ppPCM->wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
		m_bFloating = 1;
//						m_pcm = * ((WAVEFORMATEX *)pdw);
						if( pWaveData && nWaveSize )
							result = 0;
						}
					}
				break;

			case mmioFOURCC('d', 'a', 't', 'a'):
				pWaveData = (BYTE *)pdw;
				nWaveSize = dwLength;
				if( pPCM )
					result = 0;
				break;
			default:
				DPF("wierd type:%d",dwType);
				break;
		}
		pdw = (DWORD *)((BYTE *)pdw + ((dwLength+1)&~1));
	}
	if (result == 99)
		{
		}
	if (!result)
		SetWaveData(pPCM, pWaveData, nWaveSize);
	return result;
}

int CMyWave::SetWaveData(BYTE * pPCM, BYTE * pWaveData, UINT nWaveSize)
{
	m_pcm = * ((WAVEFORMATEX *)pPCM);
#ifdef _NEEDSWAP
	DOSWAPC(&m_pcm.nChannels, sizeof(WORD));
	DOSWAPC(&m_pcm.nSamplesPerSec, sizeof(DWORD));
	DOSWAPC(&m_pcm.nAvgBytesPerSec, sizeof(DWORD));
	DOSWAPC(&m_pcm.nBlockAlign, sizeof(WORD));
	DOSWAPC(&m_pcm.wBitsPerSample, sizeof(WORD));
	DOSWAPC(&m_pcm.cbSize, sizeof(WORD));
#endif
DPF("format:%d",m_pcm.wFormatTag);
    if (m_pcm.wFormatTag == 3) //WAVE_FORMAT_IEEE_FLOAT;
		m_bFloating = 1;
DPF("channels:%d",m_pcm.nChannels);
DPF("samp/sec:%d",m_pcm.nSamplesPerSec);
DPF("byte/sec:%d",m_pcm.nAvgBytesPerSec);
DPF("blkalign:%d",m_pcm.nBlockAlign);
DPF("bits/smp:%d",m_pcm.wBitsPerSample);
DPF(" cbsize :%d",m_pcm.cbSize);
DPF("wavsize :%d",nWaveSize);
	m_sampersec = m_pcm.nSamplesPerSec;
	m_bytesamp = m_pcm.nBlockAlign;
	m_bitsample = m_pcm.wBitsPerSample;
	m_channels = m_pcm.nChannels;
	delete [] m_pWave;
	m_OrigWaveSize = nWaveSize;
	m_pWave = new BYTE[m_OrigWaveSize];
	if (!m_pWave)
		return 7;
	memcpy(m_pWave, pWaveData, nWaveSize);
    UINT c = m_bitsample;
	BYTE * pp = m_pWave;
	UINT count = m_OrigWaveSize / m_bytesamp;
DPF("doing max,count:%d,c:%d",count,c);
	m_maxsample = 1;
	c += m_bFloating ? 100 : 0;
	for (;count--;pp += m_bytesamp)
		{
		int z = abs(mysamp(c,pp));
		if (z > (int)m_maxsample)
			m_maxsample = z;
		}
DPF("max:%d",m_maxsample);
	return 0;
}

void CMyWave::Image(BYTE * pDst, UINT frame, UINT rate, UINT ww, UINT h)
{
	if (!m_bytesamp)
		return;
//	return;
	UINT pitch = 4 * ((ww +3) / 4);
    UINT c = m_bitsample;
	if (!c) return;
DPF("snd image,c:%d",c);
	UINT mint;
	if (m_smark >= m_vmark)
		mint = 0;
	else
		mint = m_vmark - m_smark;
//	UINT xx = w / 2;
	UINT y,yy;
	int px = 10000;
	int minx = 3;
	int maxx = ww - minx;
	int midx = ww / 2;
	int range = maxx - midx;
	c += m_bFloating ? 100 : 0;
	for (y = 0; y < h;y++)
		{
		_int64 tt = y;
		tt += frame * h;
		tt *= FACTOR;
		tt /= h * rate;
		UINT t = (UINT)tt;
		if (t < mint)
			continue;
		tt -= mint;
		tt *= m_sampersec;
		tt /= FACTOR;
		UINT offset = m_bytesamp * (UINT)tt;
		if (offset >= m_OrigWaveSize)
			break;
		BYTE * pp = m_pWave + offset;

		int z = mysamp(c,pp);
		if (abs(z) > (int)m_maxsample)
			z = 0;
		yy = h - 1 - y;
		int x;
		_int64 q = z;
		q *= range;
		q /= m_maxsample;
		z = (int)q;
//		z = z * 10 * (int)w / (int)m_maxsample;
//		z = z * (int)w / (2 * (int)m_maxsample);
		x = midx + z;
		if (px < 10000)
			{
			if (px <= x)
				{
				for (;px < x; pDst[yy * pitch + px++] = 0);
				}
			else
				{
				for (;px > x; pDst[yy * pitch + px--] = 0);
				}
			}
		else
			px = x;
		pDst[yy * pitch + px] = 0;
		}
}
