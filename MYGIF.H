
#ifndef BYTE
	typedef unsigned char BYTE;
	typedef unsigned char * LPBYTE;
#endif

class CTreeNode;

class CMyGif
{
public:
	CMyGif();
	~CMyGif();
//
//	options 
//	0 compute palette or use supplied
//	1 optipal
//	2 dither 2 x2
//	3 dither 4 x 4
//  4 dither 8 x 8
//  5 dither 4 x 4 growth dither
//
	UINT Encode(CFile * pFile,  
			UINT w, UINT h, UINT frames, UINT depth, UINT rate, BOOL bLoop);
	UINT  SetPalette(BYTE * pPalette);
	UINT AddFrame(BYTE * pBuf, UINT pitch = 0);
	UINT FinishIt();
	void SetOption(UINT o) { m_option = o;};
protected:

	void AddCodeToBuffer(int code,short n = 0);
	void InitIndices(int cc);
	CTreeNode * AddNode(int,int);
	int SearchTreeForCode(CTreeNode * ,LPBYTE,int,int,int,int);
	void ClearTree(CTreeNode *,int);
	void FlushBuffer();
	UINT Write(UINT size);
	CFile * m_pFile;
	BOOL m_bLoop;
	UINT m_flags;
	BOOL m_bWriting;
	UINT m_width;
	UINT m_height;
	UINT m_rate;
	UINT m_dacc1;// for delay
	UINT m_dacc2; //for delay
	UINT m_fact1;
	UINT	m_option;
	UINT	m_frames;
	UINT m_frame;
	UINT m_depth;
	BOOL m_bGray;
	UINT m_bpp;
	UINT m_clear;
	UINT m_nBytes;
	UINT m_nBits;
	WORD m_acc;
	WORD m_cnt;
	BOOL m_bNewPals;
	BYTE * m_pPalette;
	BYTE * m_pBuffer;
	BYTE * m_pTemp;
	CTreeNode * m_pTree;
};
