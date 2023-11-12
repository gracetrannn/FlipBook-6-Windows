/****************************************************************************
*	Author:			Dr. Tony Lin											*
*	Email:			lintong@cis.pku.edu.cn									*
*	Release Date:	Dec. 2002												*
*																			*
*	Name:			mini JPEG class, rewritten from IJG codes				*
*	Source:			IJG v.6a JPEG LIB										*
*	Purpose:		1. Readable, so reusable								*
*					2. Customized Jpeg format, with smallest overhead		*
*					3. Standard c++ types, for easily understood			*
*																			*
*	Acknowlegement:	Thanks for great IJG, and Chris Losinger				*
*																			*
*	Legal Issues:	(almost same as IJG with followings)					*
*																			*
*	1. We don't promise that this software works.							*
*	2. You can use this software for whatever you want.						*
*	You don't have to pay.													*
*	3. You may not pretend that you wrote this software. If you use it		*
*	in a program, you must acknowledge somewhere. That is, please			*
*	metion IJG, and Me, Dr. Tony Lin.										*
*																			*
*****************************************************************************/
#ifndef _MYJPEG
#define _MYJPEG
class CMiniJpegEncoder
{

private:
	
	////////////////////////////////////////////////////////////////////////////
	//	Following data members should be computed in initialization

	unsigned short m_nQuality, m_nScale;

	//	To speed up, we save RGB=>YCbCr color map tables, 
	//	with result scaled up by 2^16
	int m_RToY[256],	m_GToY[256],	m_BToY[256];
	int m_RToCb[256],	m_GToCb[256],	m_BToCb[256];
	int m_RToCr[256],	m_GToCr[256],	m_BToCr[256];

	//	To speed up, we precompute two DCT quant tables
	unsigned short m_qtblY[64], m_qtblCbCr[64];

	//	Derived data constructed for each Huffman table 	
	typedef struct tag_HUFFMAN_TABLE {
		unsigned int	code[256];	// code for each symbol 
		char			size[256];	// length of code for each symbol 
		//If no code has been allocated for a symbol S, size[S] is 0 
	}HUFFMAN_TABLE;
	HUFFMAN_TABLE m_htblYDC, m_htblYAC, m_htblCbCrDC, m_htblCbCrAC;

	////////////////////////////////////////////////////////////////////////////
	
	////////////////////////////////////////////////////////////////////////////
	//	Following are should be initialized for compressing every image

	unsigned short m_nWidth, m_nHeight;

	//	Three dc records, used for dc differentize for Y/Cb/Cr
	int m_dcY, m_dcCb, m_dcCr;

	//	The size (in bits) and value (in 4 byte buffer) to be written out
	int m_nPutBits, m_nPutVal;

	////////////////////////////////////////////////////////////////////////////

private:
	
	void InitEncoder( void );
	
	void InitColorTable( void );

	void InitQuantTable( void );

	void ScaleQuantTable(
		unsigned short* tblRst,		//result quant table
		unsigned short* tblStd,		//standard quant table
		unsigned short* tblAan		//scale factor for AAN dct
		);

	void InitHuffmanTable( void );

	void ComputeHuffmanTable(
		unsigned char *	pBits, 
		unsigned char * pVal,
		HUFFMAN_TABLE * pTbl	
		);

	bool CompressOneTile(	
		unsigned char * pBgr,	//source data, in BGR format
		unsigned char * pJpg,	//destination, in jpg format
		int& nTileBytes			//return value, the length of compressed data
	);

	bool CompressOneGrayTile(	
		unsigned char * pGray,	//source data, in gray format
		unsigned char * pJpg,	//destination, in jpg format
		int& nTileBytes			//return value, the length of compressed data
	);

	void BGRToYCbCr(	
		unsigned char * pBgr,	//tile source data, in BGR format, 768 bytes
		unsigned char * pY,		//out, Illuminance, 256 bytes
		unsigned char * pCb,	//out, Cb, 256 bytes
		unsigned char * pCr		//out, Cr, 256 bytes
		);
	
	void BGRToYCbCrEx(	
		unsigned char * pBgr,	//in, tile data, in BGR format, 768 bytes
		int * pBlock			//out, Y: 256; Cb: 64; Cr: 64 
		);
	
	void ForwardDct( 
		int* data,	//source data, length is 64 
		int* coef	//output dct coefficients
		);
	
	void Quantize( 
		int* coef,	//coef is both in and out
		int iBlock	//block id; Y: 0,1,2,3; Cb: 4; Cr: 5
		);

	bool HuffmanEncode( 
		int* pCoef,				//	DCT coefficients
		unsigned char* pOut,	//	Output byte stream
		int iBlock,				//	0,1,2,3:Y; 4:Cb; 5:Cr;
		int& nBytes				//	Out, Byte number of Output stream
		);

	bool EmitBits(
		unsigned char* pOut,	//Output byte stream
		unsigned int code,		//Huffman code
		int size,				//Size in bits of the Huffman code
		int& nBytes				//Out, bytes length 
		);

	void EmitLeftBits(
		unsigned char* pOut,	//Output byte stream
		int& nBytes				//Out, bytes length 
		);
	bool m_bGray;
public:
	
	CMiniJpegEncoder();			//default quality is 50

	CMiniJpegEncoder( int nQuality );

	~CMiniJpegEncoder();

	bool CompressImage(	
		unsigned char *pInBuf,	//source data, bgr format, 3 bytes per pixel
		unsigned char *pOutBuf,	//destination buffer, in jpg format
		int nWidthPix,			//image width in pixels
		int nHeight,			//height
		int& nOutputBytes		//return number of bytes being written
		);
};

/****************************************************************************
*	Author:			Dr. Tony Lin											*
*	Email:			lintong@cis.pku.edu.cn									*
*	Release Date:	Dec. 2002												*
*																			*
*	Name:			mini JPEG class, rewritten from IJG codes				*
*	Source:			IJG v.6a JPEG LIB										*
*	Purpose:		1. Readable, so reusable								*
*					2. Customized Jpeg format, with smallest overhead		*
*					3. Standard c++ types, for easily understood			*
*																			*
*	Acknowlegement:	Thanks for great IJG, and Chris Losinger				*
*																			*
*	Legal Issues:	(almost same as IJG with followings)					*
*																			*
*	1. We don't promise that this software works.							*
*	2. You can use this software for whatever you want.						*
*	You don't have to pay.													*
*	3. You may not pretend that you wrote this software. If you use it		*
*	in a program, you must acknowledge somewhere. That is, please			*
*	metion IJG, and Me, Dr. Tony Lin.										*
*																			*
*****************************************************************************/

class CMiniJpegDecoder
{

private:
	
	////////////////////////////////////////////////////////////////////////////
	//	Following are initialized when create a new decoder

	unsigned short m_nQuality, m_nScale;

	unsigned char m_tblRange[5*256+128];
	
	//	To speed up, we save YCbCr=>RGB color map tables
	int m_CrToR[256], m_CrToG[256],	m_CbToB[256], m_CbToG[256];

	//	To speed up, we precompute two DCT quant tables
	unsigned short m_qtblY[64], m_qtblCbCr[64];

	// Derived data constructed for each Huffman table 
	typedef struct{
		int				mincode[17];	// smallest code of length k 
		int				maxcode[18];	// largest code of length k (-1 if none) 
		int				valptr[17];		// huffval[] index of 1st symbol of length k
		unsigned char	bits[17];		// bits[k] = # of symbols with codes of 
		unsigned char	huffval[256];	// The symbols, in order of incr code length 
		int				look_nbits[256];// # bits, or 0 if too long
		unsigned char	look_sym[256];	// symbol, or unused
	} HUFFTABLE;

	HUFFTABLE m_htblYDC, m_htblYAC, m_htblCbCrDC, m_htblCbCrAC;
	
	////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////
	//	Following data are initialized for decoding every image

	unsigned short m_nWidth, m_nHeight;

	int m_dcY, m_dcCb, m_dcCr;

	int m_nGetBits, m_nGetBuff, m_nDataBytes;

	unsigned char * m_pData;

	////////////////////////////////////////////////////////////////////////////

private:

	void InitDecoder( void );
	
	void SetRangeTable( void );

	void InitColorTable( void );

	void InitQuantTable( void );

	void ScaleQuantTable(
			unsigned short* tblRst,		//result quant table
			unsigned short* tblStd,		//standard quant table
			unsigned short* tblAan		//scale factor for AAN dct
			);

	void InitHuffmanTable( void );

	void ComputeHuffmanTable(
		unsigned char *	pBits, 
		unsigned char * pVal,
		HUFFTABLE * dtbl	
		);

	bool DecompressOneTile(	unsigned char * pBgr );
	
	bool DecompressOneGrayTile(	unsigned char * pBgr );

	void YCbCrToBGREx(	
		unsigned char * pYCbCr,	//in, Y: 256 bytes; Cb: 64 bytes; Cr: 64 bytes 
		unsigned char * pBgr	//out, BGR format, 16*16*3 = 768 bytes
		);

	void InverseDct( 
		short* coef, 			//in, dct coefficients, length = 64
		unsigned char* data, 	//out, 64 bytes		
		int nBlock				//block index: 0~3:Y; 4:Cb; 5:Cr
		);
	
	void HuffmanDecode( 
		short* coef,			//	out, DCT coefficients
		int iBlock				//	0,1,2,3:Y; 4:Cb; 5:Cr
		);

	int GetCategory( HUFFTABLE* htbl );

	void FillBitBuffer( void );

	int GetBits(int nbits);
	
	int SpecialDecode( HUFFTABLE* htbl, int nMinBits );

	int ValueFromCategory(int nCate, int nOffset);

	bool m_bGray;

public:	
	CMiniJpegDecoder();	
	~CMiniJpegDecoder();
	
	bool GetImageInfo(	
		unsigned char *pInBuf,	//in, source data, in jpp format
		int cbInBuf,			//in, count bytes for in buffer
		int& nWidth,			//out, image width in pixels
		int& nHeight,			//out, image height
		int& nHeadSize			//out, header size in bytes
		);
	
	bool DecompressImage(	
		unsigned char *pInBuf,	//in, source data, in jpg format
		unsigned char *pOutBuf	//out, destination buffer, bgr format
		);
};

#endif
