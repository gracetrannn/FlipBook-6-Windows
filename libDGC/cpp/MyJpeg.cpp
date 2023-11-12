
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

////////////////////////////////////////////////////////////////////////////////

#include "MyJpeg.h"
#include "CommonDefs.h"
#include <cstring>

////////////////////////////////////////////////////////////////////////////////

CMiniJpegEncoder::CMiniJpegEncoder( )
{
	m_nQuality = 50;
	InitEncoder( );	
}

CMiniJpegEncoder::CMiniJpegEncoder( int nQuality )
{
	m_nQuality = nQuality;
	InitEncoder( );
}

CMiniJpegEncoder::~CMiniJpegEncoder( )
{
	
}


////////////////////////////////////////////////////////////////////////////////
//	Prepare for all the tables needed, 
//	eg. quantization tables, huff tables, color convert tables
//	1 <= nQuality <= 100, is used for quantization scaling
//	Computing once, and reuse them again and again !!!!!!!

void CMiniJpegEncoder::InitEncoder( )
{
	//	prepare color convert table, from bgr to ycbcr
	InitColorTable( );

	//	prepare two quant tables, one for Y, and another for CbCr
	InitQuantTable( );

	//	prepare four huffman tables: 
	InitHuffmanTable( );
}


////////////////////////////////////////////////////////////////////////////////
//	Name:	CMiniJpegEncoder::InitColorTable()
//  Purpose:	
//			Save RGB->YCC colorspace conversion for reuse, only computing once
//			so dont need multiply in color conversion later

/* Notes:
 * 
 * YCbCr is defined per CCIR 601-1, except that Cb and Cr are
 * normalized to the range 0 .. 255 rather than -0.5 .. 0.5.
 * The conversion equations to be implemented are therefore
 *
 *	Y  =  0.29900 * R + 0.58700 * G + 0.11400 * B
 *	Cb = -0.16874 * R - 0.33126 * G + 0.50000 * B  + 128
 *	Cr =  0.50000 * R - 0.41869 * G - 0.08131 * B  + 128
 *
 * (These numbers are derived from TIFF 6.0 section 21, dated 3-June-92.)
 * To avoid floating-point arithmetic, we represent the fractional constants
 * as integers scaled up by 2^16 (about 4 digits precision); we have to divide
 * the products by 2^16, with appropriate rounding, to get the correct answer.
 */

void CMiniJpegEncoder::InitColorTable( void )
{
	int i;
	int nScale	= 1L << 16;		//equal to power(2,16)
	int CBCR_OFFSET = 128<<16;
	/*	
	*	nHalf is for (y, cb, cr) rounding, equal to (1L<<16)*0.5
	*	If (R,G,B)=(0,0,1), then Cb = 128.5, should round to 129
	*	Using these tables will produce 129 too: 
	*	Cb	= (int)((RToCb[0] + GToCb[0] + BToCb[1]) >> 16)
	*		= (int)(( 0 + 0 + 1L<<15 + 1L<<15 + 128 * 1L<<16 ) >> 16)
	*		= (int)(( 1L<<16 + 128 * 1L<<16 ) >> 16 )
	*		= 129
	*/
	int nHalf = nScale >> 1;	

	for( i=0; i<256; i++ )
	{
		m_RToY[ i ]	= (int)( 0.29900 * nScale + 0.5 ) * i;
		m_GToY[ i ]	= (int)( 0.58700 * nScale + 0.5 ) * i;
		m_BToY[ i ]	= (int)( 0.11400 * nScale + 0.5 ) * i + nHalf;

		m_RToCb[ i ] = (int)( 0.16874 * nScale + 0.5 ) * (-i);
		m_GToCb[ i ] = (int)( 0.33126 * nScale + 0.5 ) * (-i);
		m_BToCb[ i ] = (int)( 0.50000 * nScale + 0.5 ) * i + 
										CBCR_OFFSET + nHalf - 1;

		m_RToCr[ i ] = m_BToCb[ i ];
		m_GToCr[ i ] = (int)( 0.41869 * nScale + 0.5 ) * (-i);
		m_BToCr[ i ] = (int)( 0.08131 * nScale + 0.5 ) * (-i);
	}
}


////////////////////////////////////////////////////////////////////////////////
//	InitQuantTable will produce customized quantization table into:
//		m_tblYQuant[0..63] and m_tblCbCrQuant[0..63]

void CMiniJpegEncoder::InitQuantTable( void )
{
	// These are the sample quantization tables given in JPEG spec section K.1.
	// The spec says that the values given produce "good" quality, and
	// when divided by 2, "very good" quality.	

	static unsigned short std_luminance_quant_tbl[64] = 
	{
			16,  11,  10,  16,  24,  40,  51,  61,
			12,  12,  14,  19,  26,  58,  60,  55,
			14,  13,  16,  24,  40,  57,  69,  56,
			14,  17,  22,  29,  51,  87,  80,  62,
			18,  22,  37,  56,  68, 109, 103,  77,
			24,  35,  55,  64,  81, 104, 113,  92,
			49,  64,  78,  87, 103, 121, 120, 101,
			72,  92,  95,  98, 112, 100, 103,  99
	};
	static unsigned short std_chrominance_quant_tbl[64] = 
	{
			17,  18,  24,  47,  99,  99,  99,  99,
			18,  21,  26,  66,  99,  99,  99,  99,
			24,  26,  56,  99,  99,  99,  99,  99,
			47,  66,  99,  99,  99,  99,  99,  99,
			99,  99,  99,  99,  99,  99,  99,  99,
			99,  99,  99,  99,  99,  99,  99,  99,
			99,  99,  99,  99,  99,  99,  99,  99,
			99,  99,  99,  99,  99,  99,  99,  99
	};


	/*  For AA&N IDCT method, divisors are equal to quantization
	*	coefficients scaled by scalefactor[row]*scalefactor[col], where
	*		scalefactor[0] = 1
	*		scalefactor[k] = cos(k*PI/16) * sqrt(2)    for k=1..7
	*	We apply a further scale factor of 8.
	*/	
	static unsigned short aanscales[64] = {
			/* precomputed values scaled up by 14 bits */
			16384, 22725, 21407, 19266, 16384, 12873,  8867,  4520,
			22725, 31521, 29692, 26722, 22725, 17855, 12299,  6270,
			21407, 29692, 27969, 25172, 21407, 16819, 11585,  5906,
			19266, 26722, 25172, 22654, 19266, 15137, 10426,  5315,
			16384, 22725, 21407, 19266, 16384, 12873,  8867,  4520,
			12873, 17855, 16819, 15137, 12873, 10114,  6967,  3552,
			 8867, 12299, 11585, 10426,  8867,  6967,  4799,  2446,
			 4520,  6270,  5906,  5315,  4520,  3552,  2446,  1247
	};
	
	// Safety checking. Convert 0 to 1 to avoid zero divide. 
	m_nScale = m_nQuality;

	if (m_nScale <= 0) 
		m_nScale = 1;
	if (m_nScale > 100) 
		m_nScale = 100;
	
	//	Non-linear map: 1->5000, 10->500, 25->200, 50->100, 75->50, 100->0
	if (m_nScale < 50)
		m_nScale = 5000 / m_nScale;
	else
		m_nScale = 200 - m_nScale*2;

	//	Scale the Y and CbCr quant table, respectively
	ScaleQuantTable( m_qtblY,    std_luminance_quant_tbl,   aanscales );
	ScaleQuantTable( m_qtblCbCr, std_chrominance_quant_tbl, aanscales );
}

////////////////////////////////////////////////////////////////////////////////

void CMiniJpegEncoder::ScaleQuantTable(
			unsigned short* tblRst,		//result quant table
			unsigned short* tblStd,		//standard quant table
			unsigned short* tblAan		//scale factor for AAN dct
			)
{
	int i, temp, half = 1<<10;
	for (i = 0; i < 64; i++) 
	{
		// (1) user scale up
		temp = (int)(( m_nScale * tblStd[i] + 50 ) / 100 );

		// limit to baseline range 
		if (temp <= 0) 
			temp = 1;
		if (temp > 255)
			temp = 255;		

		// (2) scaling needed for AA&N algorithm
		tblRst[i] = (unsigned short)(( temp * tblAan[i] + half )>>11 );
	}
}


////////////////////////////////////////////////////////////////////////////////
//	Prepare four Huffman tables:
//		HUFFMAN_TABLE m_htblYDC, m_htblYAC, m_htblCbCrDC, m_htblCbCrAC;

void CMiniJpegEncoder::InitHuffmanTable( void )
{
	//	Y dc component
	static unsigned char bitsYDC[17] =
    { 0, 0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 };
	static unsigned char valYDC[] =
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
	

	//	CbCr dc
	static unsigned char bitsCbCrDC[17] =
    { 0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 };
	static unsigned char valCbCrDC[] =
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
	

	//	Y ac component
	static unsigned char bitsYAC[17] =
    { 0, 0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d };
	static unsigned char valYAC[] =
    { 0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
	0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
	0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
	0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
	0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
	0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
	0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
	0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
	0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
	0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
	0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
	0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
	0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
	0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
	0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
	0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
	0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
	0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
	0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
	0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
	0xf9, 0xfa };
	

	//	CbCr ac
	static unsigned char bitsCbCrAC[17] =
    { 0, 0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77 };
	static unsigned char valCbCrAC[] =
    { 0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
	0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
	0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
	0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
	0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
	0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
	0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
	0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
	0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
	0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
	0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
	0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
	0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
	0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
	0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
	0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
	0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
	0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
	0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
	0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
	0xf9, 0xfa };

	//	Compute four derived Huffman tables
	ComputeHuffmanTable( bitsYDC, valYDC, &m_htblYDC );
	ComputeHuffmanTable( bitsYAC, valYAC, &m_htblYAC );

	ComputeHuffmanTable( bitsCbCrDC, valCbCrDC, &m_htblCbCrDC );
	ComputeHuffmanTable( bitsCbCrAC, valCbCrAC, &m_htblCbCrAC );
}

////////////////////////////////////////////////////////////////////////////////

//	Compute the derived values for a Huffman table.	

/*
*	typedef struct {
*		unsigned int	code[256];	// code for each symbol 
*		char			size[256];	// length of code for each symbol 
*		//	If no code has been allocated for a symbol S, ehufsi[S] contains 0 
*	} HUFFMAN_TABLE;
*
*	HUFFMAN_TABLE m_htblYDC, m_htblYAC, m_htblCbCrDC, m_htblCbCrAC;
*/	

void CMiniJpegEncoder::ComputeHuffmanTable(
		unsigned char *	pBits, 
		unsigned char * pVal,
		HUFFMAN_TABLE * pTbl	)
{
	int p, i, l, lastp, si;
	char huffsize[257];
	unsigned int huffcode[257];
	unsigned int code;
	
	/* Figure C.1: make table of Huffman code length for each symbol */
	/* Note that this is in code-length order. */
	
	p = 0;
	for (l = 1; l <= 16; l++) {
		for (i = 1; i <= (int) pBits[l]; i++)
			huffsize[p++] = (char) l;
	}
	huffsize[p] = 0;
	lastp = p;
	
	/* Figure C.2: generate the codes themselves */
	/* Note that this is in code-length order. */
	
	code = 0;
	si = huffsize[0];
	p = 0;
	while (huffsize[p]) {
		while (((int) huffsize[p]) == si) {
			huffcode[p++] = code;
			code++;
		}
		code <<= 1;
		si++;
	}
	
	/* Figure C.3: generate encoding tables */
	/* These are code and size indexed by symbol value */
	
	/* Set any codeless symbols to have code length 0;
	* this allows EmitBits to detect any attempt to emit such symbols.
	*/
	memset( pTbl->size, 0, sizeof( pTbl->size ) );
	
	for (p = 0; p < lastp; p++) {
		pTbl->code[ pVal[p] ] = huffcode[p];
		pTbl->size[ pVal[p] ] = huffsize[p];
	}
}

////////////////////////////////////////////////////////////////////////////////
//	CMiniJpegEncoder::CompressImage(), the main function in this class !!
//	Don't ask me its purpose, parameter lists, and return value,       d:-)

bool CMiniJpegEncoder::CompressImage(	
	 unsigned char *pInBuf,	//source data, bgr format, 3 bytes per pixel
	 unsigned char *pOutBuf,//destination buffer, in jpg format
	 int nWidthPix,			//image width in pixels
	 int nHeight,			//height
	 int& nOutputBytes		//return number of bytes being written
	 )
{
	//	Error handling
	if(( pInBuf == 0 )||( pOutBuf == 0 ))
		return false;
	if (nHeight & 4096)
		m_bGray = 1;
	else
		m_bGray = 0;
	nHeight &= 4095;

	m_nWidth = nWidthPix;
	m_nHeight = nHeight;

	//	image width and height, 4 bytes
	memcpy ( pOutBuf, & m_nWidth, 2 );
	pOutBuf += 2;
	memcpy ( pOutBuf, & m_nHeight, 2 );
	pOutBuf += 2;

	//	Write quality factor, 2 byte
	if (m_bGray)
		m_nQuality |= 4096;
	memcpy ( pOutBuf, & m_nQuality, 2 );
	m_nQuality &= 4095;
	pOutBuf += 2;

	//	let pOutBuf rewind to the first byte of output buffer
	pOutBuf -= 6;
	nOutputBytes = 6;
	
	int bpp = m_bGray ? 1 : 3;
	int nRowBytes = ( m_nWidth * bpp + 3 ) / 4 * 4;		

	if(( nWidthPix <= 0 )||( nRowBytes <= 0 )||( nHeight <= 0 ))
		return false;

	//	declares
	int xPixel, yPixel, xTile, yTile, cxTile, cyTile, cxBlock, cyBlock;
	int x, y, nTrueRows, nTrueCols, nTileBytes;
	unsigned char byTile[768], *pTileRow, *pLastPixel, *pHolePixel;
		
	//	horizontal and vertical count of tile, macroblocks, 
	//	or MCU(Minimum Coded Unit), in 16*16 pixels
	cxTile = (nWidthPix + 15)>> 4;	
	cyTile = (nHeight + 15)	>> 4;

	//	horizontal and vertical count of block, in 8*8 pixels
	cxBlock = cxTile << 1;
	cyBlock = cyTile << 1;

	//	first set output bytes as zero
	nTileBytes = 0;
	//	three dc values set to zero, needed for compressing one new image
	m_dcY = m_dcCb = m_dcCr = 0;

	//	Initialize size (in bits) and value to be written out
	m_nPutBits = 0;
	m_nPutVal = 0;

	//	Run all the tiles, or macroblocks, or MCUs
	for( yTile = 0; yTile < cyTile; yTile++ )
	{
		for( xTile = 0; xTile < cxTile; xTile++ )
		{
			//	Get tile starting pixel position
			xPixel = xTile << 4;
			yPixel = yTile << 4;

			//	Get the true number of tile columns and rows
			nTrueRows = 16;
			nTrueCols = 16;			
			if( yPixel + nTrueRows > nHeight )
				nTrueRows = nHeight - yPixel;
			if( xPixel + nTrueCols > nWidthPix )
				nTrueCols = nWidthPix - xPixel;

			//	Prepare pointer to one row of this tile
//			pTileRow = pInBuf + (yPixel-1) * nRowBytes + xPixel * 3;
			pTileRow = pInBuf + yPixel * nRowBytes + xPixel * bpp;

			//	Get tile data from pInBuf into byTile. If not full, padding 
			//	byTile with the bottom row and rightest pixel
			for( y = 0; y < 16; y ++ )
			{
				if( y < nTrueRows )
				{	
					//	Get data of one row
//					pTileRow += nRowBytes;					
					memcpy( byTile + y * 16 * bpp, pTileRow, nTrueCols * bpp );
					
					//	padding to full tile with the rightest pixel
					if( nTrueCols < 16 )
					{
						pLastPixel = pTileRow + (nTrueCols - 1) * bpp;
						pHolePixel = byTile + y * 16 * bpp + nTrueCols * bpp;
						for( x = nTrueCols; x < 16; x ++ )
						{
							memcpy( pHolePixel, pLastPixel, bpp );
							pHolePixel += bpp;
						}
					}
				}
				else
				{
					//	padding the hole rows with the bottom row
					memcpy( byTile + y * 16 * bpp, 
							byTile + (nTrueRows - 1) * 16 * bpp,
							16 * bpp );
				}
					pTileRow += nRowBytes;					
			}
		
			//	Compress this full tile with jpeg algorithm here !!!!!
			//	The compressed data length for this tile is return by nTileBytes
			bool bresult;
			if (m_bGray)
			bresult = CompressOneGrayTile(	byTile, 
									pOutBuf + nOutputBytes, 
									nTileBytes );
			else
			bresult = CompressOneTile(	byTile, 
									pOutBuf + nOutputBytes, 
									nTileBytes );
			if (!bresult)
				return false;
			nOutputBytes += nTileBytes;
		}
	}
	
	//	Maybe there are some bits left, send them here
	if( m_nPutBits > 0 )
	{
		EmitLeftBits( pOutBuf + nOutputBytes, nTileBytes );
		nOutputBytes += nTileBytes;
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////
//	function Purpose:	compress one 16*16 pixels with jpeg

bool CMiniJpegEncoder::CompressOneTile(	
	unsigned char * pBgr,	//source data, in BGR format
	unsigned char * pJpg,	//destination, in jpg format
	int& nTileBytes			//return value, the length of compressed data
	)
{
	//	Three color components, 256 + 64 + 64 elements 
	int pYCbCr[384];

	//	The DCT outputs are returned scaled up by a factor of 8;
	//	they therefore have a range of +-8K for 8-bit source data 
	int coef[64];	

	//	Initialize to zero
	nTileBytes = 0;

	//	Color conversion and subsampling
	//	pY data is in block order, e.g. 
	//	block 0 is from pY[0] to pY[63], block 1 is from pY[64] to pY[127]
	BGRToYCbCrEx( pBgr, pYCbCr );
	
//	Do Y/Cb/Cr components, Y: 4 blocks; Cb: 1 block; Cr: 1 block
	int i, nBytes,blocks;
	blocks = m_bGray ? 4 : 6;
	for( i=0; i<blocks; i++ )
	{
		ForwardDct( pYCbCr + i*64, coef );
		
		Quantize( coef, i );	//coef is both in and out
		
		HuffmanEncode( coef, pJpg + nTileBytes, i, nBytes );
		nTileBytes += nBytes;
	}

	return true;
}


bool CMiniJpegEncoder::CompressOneGrayTile(	
	unsigned char * pGray,	//source data, in BGR format
	unsigned char * pJpg,	//destination, in jpg format
	int& nTileBytes			//return value, the length of compressed data
	)
{
	//	Three color components, 256 + 64 + 64 elements 
	int pYs[256];

	//	The DCT outputs are returned scaled up by a factor of 8;
	//	they therefore have a range of +-8K for 8-bit source data 
	int coef[64];	

	//	Initialize to zero
	nTileBytes = 0;

	int i,x,y,z;
	for( i = 0, z= 0; i < 4; i++)
		{
		for (y = 0; y < 8; y++)
		for (x = 0; x < 8; x++)
			{
			int sx = x + 8 * (i & 2);
			int sy = y + 8 * (i / 2);
			pYs[z++] = (int)(pGray[16 * sy + sx]) - 128;
			}
		}
//	memset(pYs,255,1024);
	int nBytes,blocks;
	blocks = m_bGray ? 4 : 6;
	for( i=0; i<blocks; i++ )
	{
		ForwardDct( pYs + i*64, coef );
		
		Quantize( coef, i );	//coef is both in and out
		
		HuffmanEncode( coef, pJpg + nTileBytes, i, nBytes );
		nTileBytes += nBytes;
	}

	return true;
}


////////////////////////////////////////////////////////////////////////////////
//	Color convertion from bgr to ycbcr for one tile, 16*16 pixels
//	Actually being not used for efficiency !!!!! Please use BGRToYCbCrEx()

void CMiniJpegEncoder::BGRToYCbCr(	
		unsigned char * pBgr,	//tile source data, in BGR format, 768 bytes
		unsigned char * pY,		//out, Illuminance, 256 bytes
		unsigned char * pCb,	//out, Cb, 256 bytes
		unsigned char * pCr		//out, Cr, 256 bytes
		)
{
	int i;
	unsigned char r, g, b, *pByte = pBgr, *py = pY, *pcb = pCb, *pcr = pCr;

	for( i=0; i<256; i++ )
	{
		b = *(pByte ++);
		g = *(pByte ++);
		r = *(pByte ++);

		*(py++)	 = (unsigned char)((m_RToY[r]  + m_GToY[g]  + m_BToY[b] )>>16);
		*(pcb++) = (unsigned char)((m_RToCb[r] + m_GToCb[g] + m_BToCb[b])>>16);
		*(pcr++) = (unsigned char)((m_RToCr[r] + m_GToCr[g] + m_BToCr[b])>>16);
	}
}

////////////////////////////////////////////////////////////////////////////////

//	(1) Color convertion from bgr to ycbcr for one tile, 16*16 pixels;
//	(2) Y has 4 blocks, with block 0 from pY[0] to pY[63], 
//		block 1 from pY[64] to pY[127], block 2 from pY[128] to pY[191], ...
//	(3) With Cb/Cr subsampling, i.e. 2*2 pixels get one Cb and one Cr
//		IJG use average for better performance; we just pick one from four
//	(4) Do unsigned->signed conversion, i.e. substract 128 

void CMiniJpegEncoder::BGRToYCbCrEx(	
		unsigned char * pBgr,	//in, tile data, in BGR format, 768 bytes
		int * pBlock	//out, Y: 256; Cb: 64; Cr: 64 
		)
{
	int x, y, *py[4], *pcb, *pcr;
	unsigned char r, g, b, *pByte;

	pByte = pBgr;
	for( x = 0; x < 4; x++ )
		py[ x ] = pBlock + 64 * x;
	pcb	  = pBlock + 256;
	pcr   = pBlock + 320;
	
	for( y=0; y<16; y++ )
	{
		for( x=0; x<16; x++ )
		{
			b = *(pByte ++);
			g = *(pByte ++);
			r = *(pByte ++);
			
			//	block number is ((y/8) * 2 + x/8): 0, 1, 2, 3
			*( py[((y>>3)<<1) + (x>>3)] ++ ) = 
				((m_RToY[ r ]  + m_GToY[ g ]  + m_BToY[ b ] )>>16) - 128;	
			
			//	Equal to: (( x%2 == 0 )&&( y%2 == 0 ))
			if( (!(y & 1L)) && (!(x & 1L)) )
			{
				*(pcb++) = 
					((m_RToCb[ r ] + m_GToCb[ g ] + m_BToCb[ b ])>>16) - 128;
				*(pcr++) = 
					((m_RToCr[ r ] + m_GToCr[ g ] + m_BToCr[ b ])>>16) - 128;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

/************************************************************************** 
 * (1)	Direct dct algorithms:
 *	are also available, but they are much more complex and seem not to 
 *  be any faster when reduced to code.
 *
 *************************************************************************
 * (2)  LL&M dct algorithm:
 *	This implementation is based on an algorithm described in
 *  C. Loeffler, A. Ligtenberg and G. Moschytz, "Practical Fast 1-D DCT
 *  Algorithms with 11 Multiplications", Proc. Int'l. Conf. on Acoustics,
 *  Speech, and Signal Processing 1989 (ICASSP '89), pp. 988-991.
 *	The primary algorithm described there uses 11 multiplies and 29 adds.
 *	We use their alternate method with 12 multiplies and 32 adds.
 *
 ***************************************************************************
 * (3)	AA&N DCT algorithm:
 * This implementation is based on Arai, Agui, and Nakajima's algorithm for
 * scaled DCT.  Their original paper (Trans. IEICE E-71(11):1095) is in
 * Japanese, but the algorithm is described in the Pennebaker & Mitchell
 * JPEG textbook (see REFERENCES section in file README).  The following 
 * code is based directly on figure 4-8 in P&M.
 *
 * The AA&N method needs only 5 multiplies and 29 adds. 
 *
 * The primary disadvantage of this method is that with fixed-point math,
 * accuracy is lost due to imprecise representation of the scaled
 * quantization values.  The smaller the quantization table entry, the less
 * precise the scaled value, so this implementation does worse with high-
 * quality-setting files than with low-quality ones.
 ***************************************************************************
 */

//	AA&N DCT algorithm implemention


void CMiniJpegEncoder::ForwardDct( 
		int* data,	//source data, length is 64 
		int* coef	//output dct coefficients
		)
{

////////////////////////////////////////////////////////////////////////////
//	define some macroes 
	
//	Scale up the float with 1<<8; so (int)(0.382683433 * 1<<8 ) = 98
#define FIX_0_382683433  ((int)98)		/* FIX(0.382683433) */
#define FIX_0_541196100  ((int)139)		/* FIX(0.541196100) */
#define FIX_0_707106781  ((int)181)		/* FIX(0.707106781) */
#define FIX_1_306562965  ((int)334)		/* FIX(1.306562965) */
	
//	This macro changes float multiply into int multiply and right-shift
//	MULTIPLY(a, FIX_0_707106781) = (short)( 0.707106781 * a )
#define MULTIPLY(var,cons)  (int)(((cons) * (var)) >> 8 )

////////////////////////////////////////////////////////////////////////////

	static const int DCTSIZE = 8;
	int x, y;
	int *dataptr;
	int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
	int tmp10, tmp11, tmp12, tmp13;
	int z1, z2, z3, z4, z5, z11, z13, *coefptr;
	
	/* Pass 1: process rows. */
	
	dataptr = data;		//input
	coefptr = coef;		//output	
	for( y = 0; y < 8; y++ ) 
	{
		tmp0 = dataptr[0] + dataptr[7];
		tmp7 = dataptr[0] - dataptr[7];
		tmp1 = dataptr[1] + dataptr[6];
		tmp6 = dataptr[1] - dataptr[6];
		tmp2 = dataptr[2] + dataptr[5];
		tmp5 = dataptr[2] - dataptr[5];
		tmp3 = dataptr[3] + dataptr[4];
		tmp4 = dataptr[3] - dataptr[4];
		
		/* Even part */
		
		tmp10 = tmp0 + tmp3;	/* phase 2 */
		tmp13 = tmp0 - tmp3;
		tmp11 = tmp1 + tmp2;
		tmp12 = tmp1 - tmp2;
		
		coefptr[0] = tmp10 + tmp11; /* phase 3 */
		coefptr[4] = tmp10 - tmp11;
		
		z1 = MULTIPLY(tmp12 + tmp13, FIX_0_707106781); /* c4 */
		coefptr[2] = tmp13 + z1;	/* phase 5 */
		coefptr[6] = tmp13 - z1;
		
		/* Odd part */
		
		tmp10 = tmp4 + tmp5;	/* phase 2 */
		tmp11 = tmp5 + tmp6;
		tmp12 = tmp6 + tmp7;
		
		/* The rotator is modified from fig 4-8 to avoid extra negations. */
		z5 = MULTIPLY(tmp10 - tmp12, FIX_0_382683433);	/* c6 */
		z2 = MULTIPLY(tmp10, FIX_0_541196100) + z5;		/* c2-c6 */
		z4 = MULTIPLY(tmp12, FIX_1_306562965) + z5;		/* c2+c6 */
		z3 = MULTIPLY(tmp11, FIX_0_707106781);			/* c4 */
		
		z11 = tmp7 + z3;		/* phase 5 */
		z13 = tmp7 - z3;
		
		coefptr[5] = z13 + z2;	/* phase 6 */
		coefptr[3] = z13 - z2;
		coefptr[1] = z11 + z4;
		coefptr[7] = z11 - z4;
		
		dataptr += 8;		/* advance pointer to next row */
		coefptr += 8;
	}
	
	/* Pass 2: process columns. */	

	coefptr = coef;		//both input and output
	for ( x = 0; x < 8; x++ ) 
	{
		tmp0 = coefptr[DCTSIZE*0] + coefptr[DCTSIZE*7];
		tmp7 = coefptr[DCTSIZE*0] - coefptr[DCTSIZE*7];
		tmp1 = coefptr[DCTSIZE*1] + coefptr[DCTSIZE*6];
		tmp6 = coefptr[DCTSIZE*1] - coefptr[DCTSIZE*6];
		tmp2 = coefptr[DCTSIZE*2] + coefptr[DCTSIZE*5];
		tmp5 = coefptr[DCTSIZE*2] - coefptr[DCTSIZE*5];
		tmp3 = coefptr[DCTSIZE*3] + coefptr[DCTSIZE*4];
		tmp4 = coefptr[DCTSIZE*3] - coefptr[DCTSIZE*4];
		
		/* Even part */
		
		tmp10 = tmp0 + tmp3;	/* phase 2 */
		tmp13 = tmp0 - tmp3;
		tmp11 = tmp1 + tmp2;
		tmp12 = tmp1 - tmp2;
		
		coefptr[DCTSIZE*0] = tmp10 + tmp11; /* phase 3 */
		coefptr[DCTSIZE*4] = tmp10 - tmp11;
		
		z1 = MULTIPLY(tmp12 + tmp13, FIX_0_707106781); /* c4 */
		coefptr[DCTSIZE*2] = tmp13 + z1; /* phase 5 */
		coefptr[DCTSIZE*6] = tmp13 - z1;
		
		/* Odd part */
		
		tmp10 = tmp4 + tmp5;	/* phase 2 */
		tmp11 = tmp5 + tmp6;
		tmp12 = tmp6 + tmp7;
		
		/* The rotator is modified from fig 4-8 to avoid extra negations. */
		z5 = MULTIPLY(tmp10 - tmp12, FIX_0_382683433); /* c6 */
		z2 = MULTIPLY(tmp10, FIX_0_541196100) + z5; /* c2-c6 */
		z4 = MULTIPLY(tmp12, FIX_1_306562965) + z5; /* c2+c6 */
		z3 = MULTIPLY(tmp11, FIX_0_707106781); /* c4 */
		
		z11 = tmp7 + z3;		/* phase 5 */
		z13 = tmp7 - z3;
		
		coefptr[DCTSIZE*5] = z13 + z2; /* phase 6 */
		coefptr[DCTSIZE*3] = z13 - z2;
		coefptr[DCTSIZE*1] = z11 + z4;
		coefptr[DCTSIZE*7] = z11 - z4;
		
		coefptr++;			/* advance pointer to next column */
	}
}


////////////////////////////////////////////////////////////////////////////////

void CMiniJpegEncoder::Quantize( 
		int* coef,	//coef is both in and out
		int iBlock	//block id; Y: 0,1,2,3; Cb: 4; Cr: 5
		)
{
	int temp;
	unsigned short qval, *pQuant;

	if( iBlock < 4 )
		pQuant = m_qtblY;
	else
		pQuant = m_qtblCbCr;

	for (int i = 0; i < 64; i++) 
	{
		qval = pQuant[i];
		temp = coef[i];
		
		/* Divide the coefficient value by qval, ensuring proper rounding.
		* Since C does not specify the direction of rounding for negative
		* quotients, we have to force the dividend positive for portability.
		*
		* In most files, at least half of the output values will be zero
		* (at default quantization settings, more like three-quarters...)
		* so we should ensure that this case is fast.  On many machines,
		* a comparison is enough cheaper than a divide to make a special test
		* a win.  Since both inputs will be nonnegative, we need only test
		* for a < b to discover whether a/b is 0.
		* If your machine's division is fast enough, define FAST_DIVIDE.
		*/

		// Notes: Actually we use the second expression !!
/*
#ifdef FAST_DIVIDE
#define DIVIDE_BY(a,b)	a /= b
#else
*/
#define DIVIDE_BY(a,b)	if (a >= b) a /= b; else a = 0
//#endif		
		
		if ( temp < 0) 
		{
			temp = -temp;
			temp += qval>>1;	/* for rounding */
			DIVIDE_BY(temp, qval);
			temp = -temp;
		} 
		else 
		{
			temp += qval>>1;	/* for rounding */
			DIVIDE_BY(temp, qval);
		}
		
		coef[i] = temp;		
    }
}



////////////////////////////////////////////////////////////////////////////////

bool CMiniJpegEncoder::HuffmanEncode( 
		int* pCoef,				//	DCT coefficients
		unsigned char* pOut,	//	Output byte stream
		int iBlock,				//	0,1,2,3:Y; 4:Cb; 5:Cr;
		int& nBytes				//	Out, Byte number of Output stream
		)
{	
	/*
	* jpeg_natural_order[i] is the natural-order position of the i'th element
	* of zigzag order.
	*
	* When reading corrupted data, the Huffman decoders could attempt
	* to reference an entry beyond the end of this array (if the decoded
	* zero run length reaches past the end of the block).  To prevent
	* wild stores without adding an inner-loop test, we put some extra
	* "63"s after the real entries.  This will cause the extra coefficient
	* to be stored in location 63 of the block, not somewhere random.
	* The worst case would be a run-length of 15, which means we need 16
	* fake entries.
	*/
	static const int jpeg_natural_order[64+16] = {
			 0,  1,  8, 16,  9,  2,  3, 10,
			17, 24, 32, 25, 18, 11,  4,  5,
			12, 19, 26, 33, 40, 48, 41, 34,
			27, 20, 13,  6,  7, 14, 21, 28,
			35, 42, 49, 56, 57, 50, 43, 36,
			29, 22, 15, 23, 30, 37, 44, 51,
			58, 59, 52, 45, 38, 31, 39, 46,
			53, 60, 61, 54, 47, 55, 62, 63,
			63, 63, 63, 63, 63, 63, 63, 63,//extra entries for safety
			63, 63, 63, 63, 63, 63, 63, 63
	};
	
	int temp, temp2, nbits, k, r, i, nWrite;
	int *block = pCoef;
	int *pLastDc = &m_dcY;
	HUFFMAN_TABLE *dctbl, *actbl;

	nBytes = 0;

	if( iBlock < 4 )
	{
		dctbl = & m_htblYDC;
		actbl = & m_htblYAC;
//		pLastDc = &m_dcY;	
	}
	else
	{
		dctbl = & m_htblCbCrDC;
		actbl = & m_htblCbCrAC;

		if( iBlock == 4 )
			pLastDc = &m_dcCb;
		else
			pLastDc = &m_dcCr;
	}
	
	/* Encode the DC coefficient difference per section F.1.2.1 */
	
	temp = temp2 = block[0] - (*pLastDc);
	*pLastDc = block[0];
	
	if (temp < 0) {
		temp = -temp;		/* temp is abs value of input */
		/* For a negative input, want temp2 = bitwise complement of abs(input) */
		/* This code assumes we are on a two's complement machine */
		temp2 --;
	}
	
	/* Find the number of bits needed for the magnitude of the coefficient */
	nbits = 0;
	while (temp) {
		nbits ++;
		temp >>= 1;
	}
	
	//	Write category number
	if (! EmitBits( pOut + nBytes, dctbl->code[nbits], dctbl->size[nbits], nWrite ))
		return FALSE;
	nBytes += nWrite;

	//	Write category offset
	if (nbits)			/* EmitBits rejects calls with size 0 */
	{
		if (! EmitBits( pOut + nBytes, (unsigned int) temp2, nbits, nWrite ))
			return FALSE;
		nBytes += nWrite;
	}
	
	////////////////////////////////////////////////////////////////////////////
	/* Encode the AC coefficients per section F.1.2.2 */
	
	r = 0;			/* r = run length of zeros */
	
	for (k = 1; k < 64; k++) 
	{
		if ((temp = block[jpeg_natural_order[k]]) == 0) 
		{
			r++;
		} 
		else 
		{
			/* if run length > 15, must emit special run-length-16 codes (0xF0) */
			while (r > 15) {
				if (! EmitBits( pOut + nBytes, actbl->code[0xF0], 
								actbl->size[0xF0], nWrite ))
					return FALSE;
				nBytes += nWrite;
				r -= 16;
			}
			
			temp2 = temp;
			if (temp < 0) {
				temp = -temp;		/* temp is abs value of input */
				/* This code assumes we are on a two's complement machine */
				temp2--;
			}
			
			/* Find the number of bits needed for the magnitude of the coefficient */
			nbits = 1;		/* there must be at least one 1 bit */
			while ((temp >>= 1))
				nbits++;
			
			/* Emit Huffman symbol for run length / number of bits */
			i = (r << 4) + nbits;
			if (! EmitBits( pOut + nBytes, actbl->code[i], 
							actbl->size[i], nWrite ))
				return FALSE;
			nBytes += nWrite;
			
			//	Write Category offset
			if (! EmitBits( pOut + nBytes, (unsigned int) temp2, 
							nbits, nWrite ))
				return FALSE;
			nBytes += nWrite;
			
			r = 0;
		}
	}
	
	//If all the left coefs were zero, emit an end-of-block code
	if (r > 0)
	{
		if (! EmitBits( pOut + nBytes, actbl->code[0], 
						actbl->size[0], nWrite ))
			return FALSE;
		nBytes += nWrite;
	}		
	
	return TRUE;
}


////////////////////////////////////////////////////////////////////////////////

/* Outputting bits to the file */

/* Only the right 24 bits of put_buffer are used; the valid bits are
 * left-justified in this part.  At most 16 bits can be passed to EmitBits
 * in one call, and we never retain more than 7 bits in put_buffer
 * between calls, so 24 bits are sufficient.
 */

inline bool CMiniJpegEncoder::EmitBits(
		unsigned char* pOut,	//Output byte stream
		unsigned int code,		//Huffman code
		int size,				//Size in bits of the Huffman code
		int& nBytes				//Out, bytes length 
		)
{
	/* This routine is heavily used, so it's worth coding tightly. */
	int put_buffer = (int) code;
	int put_bits = m_nPutBits;
	
	nBytes = 0;
	
	/* if size is 0, caller used an invalid Huffman table entry */
	if (size == 0)
		return false;
	
	put_buffer &= (((int)1)<<size) - 1; /* mask off any extra bits in code */
	
	put_bits += size;					/* new number of bits in buffer */
	
	put_buffer <<= 24 - put_bits;		/* align incoming bits */
	
	put_buffer |= m_nPutVal;			/* and merge with old buffer contents */
	
	//	If there are more than 8 bits, write it out
	while (put_bits >= 8) 
	{
		//	Write one byte out !!!!
		*(pOut++) = (unsigned char) ((put_buffer >> 16) & 0xFF);
		nBytes ++;
/*	
		if (uc == 0xFF) {		//need to stuff a zero byte?
			*(pOut++) = (unsigned char) 0;	//	Write one byte out !!!!
			nBytes ++;
		}
*/
		put_buffer <<= 8;
		put_bits -= 8;
	}
	
	m_nPutVal	= put_buffer; /* update state variables */
	m_nPutBits	= put_bits;
	
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

inline void CMiniJpegEncoder::EmitLeftBits(
		unsigned char* pOut,	//Output byte stream
		int& nBytes				//Out, bytes length 
		)
{
	
	unsigned char uc = (unsigned char) ((m_nPutVal >> 16) & 0xFF);
	*(pOut++) = uc;			//	Write one byte out !!!!
	nBytes = 1;

	m_nPutVal  = 0;
	m_nPutBits = 0;
}

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

////////////////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
//#include "MyJpeg.h"


////////////////////////////////////////////////////////////////////////////////

CMiniJpegDecoder::CMiniJpegDecoder( )
{
	
}

CMiniJpegDecoder::~CMiniJpegDecoder( )
{
	
}

////////////////////////////////////////////////////////////////////////////////

bool CMiniJpegDecoder::GetImageInfo(	
	unsigned char *pInBuf,	//in, source data, in jpp format
	int cbInBuf,			//in, count bytes for in buffer
	int& nWidth,			//out, image width in pixels
	int& nHeight,			//out, image height
	int& nHeadSize			//out, header size in bytes
	)
{
	//	image width and height, 4 bytes
	memcpy ( &m_nWidth, pInBuf, 2 );
	pInBuf += 2;
	memcpy ( &m_nHeight, pInBuf, 2 );
	pInBuf += 2;

	//	Write quality factor, 2 byte
	memcpy ( &m_nQuality, pInBuf, 2 );
	pInBuf += 2;
	m_bGray = (m_nQuality & 4096) ? 1 : 0;
	m_nQuality &= 4095;

	m_nDataBytes = cbInBuf - 6;

	nHeadSize = 6;

	if(( m_nWidth <= 0 )||( m_nHeight <= 0 ))
		return false;

	nWidth = m_nWidth;
	nHeight = m_nHeight;

	//	Prepare for decoding here !!!!
	InitDecoder( );

	return true;
}


////////////////////////////////////////////////////////////////////////////////
//	Prepare for all the tables needed, 
//	eg. quantization tables, huff tables, color convert tables
//	1 <= nQuality <= 100, is used for quantization scaling
//	Computing once, and reuse them again and again !!!!!!!

void CMiniJpegDecoder::InitDecoder( void )
{
	m_nGetBits = 0;
	m_nGetBuff = 0;

	m_dcY = m_dcCb = m_dcCr = 0;

	//	prepare range limiting table to limit idct outputs
	SetRangeTable( );

	//	prepare color convert table, from bgr to ycbcr
	InitColorTable( );

	//	prepare two quant tables, one for Y, and another for CbCr
	InitQuantTable( );

	//	prepare four huffman tables: 
	InitHuffmanTable( );
}


////////////////////////////////////////////////////////////////////////////////
//	prepare_range_limit_table(): Set m_tblRange[5*256+128 = 1408]
//	range table is used for range limiting of idct results
/*	On most machines, particularly CPUs with pipelines or instruction prefetch,
 *	a (subscript-check-less) C table lookup
 *			x = sample_range_limit[x];
 *	is faster than explicit tests
 *			if (x < 0)  x = 0;
 *			else if (x > MAXJSAMPLE)  x = MAXJSAMPLE;
 */

void CMiniJpegDecoder::SetRangeTable( void )
{
	unsigned char *tbl;

	//	m_tblRange[0, ..., 255], limit[x] = 0 for x < 0
	memset( m_tblRange, 0, 256 );

	//	m_tblRange[256, ..., 511], limit[x] = x
	tbl = m_tblRange + 256;
	int i;
	for( i=0; i<256; i++ )
		*(tbl++) = (unsigned char) i;

	// m_tblRange[512, ..., 895]: first half of post-IDCT table
	tbl = m_tblRange + 512;
	for (i = 128; i < 512; i++)
		*(tbl++) = 255;

	//	m_tblRange[896, ..., 1280]: Second half of post-IDCT table
	memset( m_tblRange + 896, 0, 384);

	// [1280, 1407] = [256, 384]
	memcpy( m_tblRange + 1280, m_tblRange + 256, 128);
}


////////////////////////////////////////////////////////////////////////////////

/**************** YCbCr -> RGB conversion: most common case **************/

/*
 * YCbCr is defined per CCIR 601-1, except that Cb and Cr are
 * normalized to the range 0..MAXJSAMPLE rather than -0.5 .. 0.5.
 * The conversion equations to be implemented are therefore
 *	R = Y                + 1.40200 * Cr
 *	G = Y - 0.34414 * Cb - 0.71414 * Cr
 *	B = Y + 1.77200 * Cb
 * where Cb and Cr represent the incoming values less CENTERJSAMPLE.
 * (These numbers are derived from TIFF 6.0 section 21, dated 3-June-92.)
 *
 * To avoid floating-point arithmetic, we represent the fractional constants
 * as integers scaled up by 2^16 (about 4 digits precision); we have to divide
 * the products by 2^16, with appropriate rounding, to get the correct answer.
 * Notice that Y, being an integral input, does not contribute any fraction
 * so it need not participate in the rounding.
 *
 * For even more speed, we avoid doing any multiplications in the inner loop
 * by precalculating the constants times Cb and Cr for all possible values.
 * For 8-bit JSAMPLEs this is very reasonable (only 256 entries per table);
 * for 12-bit samples it is still acceptable.  It's not very reasonable for
 * 16-bit samples, but if you want lossless storage you shouldn't be changing
 * colorspace anyway.
 * The Cr=>R and Cb=>B values can be rounded to integers in advance; the
 * values for the G calculation are left scaled up, since we must add them
 * together before rounding.
 */

void CMiniJpegDecoder::InitColorTable( void )
{
	int i, x;
	int nScale	= 1L << 16;		//equal to power(2,16)
	int nHalf	= nScale >> 1;

#define FIX(x) ((int) ((x) * nScale + 0.5))

	/* i is the actual input pixel value, in the range 0..MAXJSAMPLE */
    /* The Cb or Cr value we are thinking of is x = i - CENTERJSAMPLE */
    /* Cr=>R value is nearest int to 1.40200 * x */
	/* Cb=>B value is nearest int to 1.77200 * x */
	/* Cr=>G value is scaled-up -0.71414 * x */
	/* Cb=>G value is scaled-up -0.34414 * x */
    /* We also add in ONE_HALF so that need not do it in inner loop */
	for (i = 0, x = -128; i < 256; i++, x++) 
	{
		m_CrToR[i] = (int) ( FIX(1.40200) * x + nHalf ) >> 16;
		m_CbToB[i] = (int) ( FIX(1.77200) * x + nHalf ) >> 16;
		m_CrToG[i] = (int) (- FIX(0.71414) * x );
		m_CbToG[i] = (int) (- FIX(0.34414) * x + nHalf );
	}
}


////////////////////////////////////////////////////////////////////////////////
//	InitQuantTable will produce customized quantization table into:
//		m_tblYQuant[0..63] and m_tblCbCrQuant[0..63]

void CMiniJpegDecoder::InitQuantTable( void )
{
	// These are the sample quantization tables given in JPEG spec section K.1.
	// The spec says that the values given produce "good" quality, and
	// when divided by 2, "very good" quality.	

	static unsigned short std_luminance_quant_tbl[64] = 
	{
			16,  11,  10,  16,  24,  40,  51,  61,
			12,  12,  14,  19,  26,  58,  60,  55,
			14,  13,  16,  24,  40,  57,  69,  56,
			14,  17,  22,  29,  51,  87,  80,  62,
			18,  22,  37,  56,  68, 109, 103,  77,
			24,  35,  55,  64,  81, 104, 113,  92,
			49,  64,  78,  87, 103, 121, 120, 101,
			72,  92,  95,  98, 112, 100, 103,  99
	};
	static unsigned short std_chrominance_quant_tbl[64] = 
	{
			17,  18,  24,  47,  99,  99,  99,  99,
			18,  21,  26,  66,  99,  99,  99,  99,
			24,  26,  56,  99,  99,  99,  99,  99,
			47,  66,  99,  99,  99,  99,  99,  99,
			99,  99,  99,  99,  99,  99,  99,  99,
			99,  99,  99,  99,  99,  99,  99,  99,
			99,  99,  99,  99,  99,  99,  99,  99,
			99,  99,  99,  99,  99,  99,  99,  99
	};


	/*  For AA&N IDCT method, divisors are equal to quantization
	*	coefficients scaled by scalefactor[row]*scalefactor[col], where
	*		scalefactor[0] = 1
	*		scalefactor[k] = cos(k*PI/16) * sqrt(2)    for k=1..7
	*	We apply a further scale factor of 8.
	*/	
	static unsigned short aanscales[64] = {
			/* precomputed values scaled up by 14 bits */
			16384, 22725, 21407, 19266, 16384, 12873,  8867,  4520,
			22725, 31521, 29692, 26722, 22725, 17855, 12299,  6270,
			21407, 29692, 27969, 25172, 21407, 16819, 11585,  5906,
			19266, 26722, 25172, 22654, 19266, 15137, 10426,  5315,
			16384, 22725, 21407, 19266, 16384, 12873,  8867,  4520,
			12873, 17855, 16819, 15137, 12873, 10114,  6967,  3552,
			 8867, 12299, 11585, 10426,  8867,  6967,  4799,  2446,
			 4520,  6270,  5906,  5315,  4520,  3552,  2446,  1247
	};
	
	// Safety checking. Convert 0 to 1 to avoid zero divide. 
	m_nScale = m_nQuality;
	if (m_nScale <= 0) 
		m_nScale = 1;
	if (m_nScale > 100) 
		m_nScale = 100;
	
	//	Non-linear map: 1->5000, 10->500, 25->200, 50->100, 75->50, 100->0
	if (m_nScale < 50)
		m_nScale = 5000 / m_nScale;
	else
		m_nScale = 200 - m_nScale*2;

	//	Scale the Y and CbCr quant table, respectively
	ScaleQuantTable( m_qtblY,	 std_luminance_quant_tbl, aanscales );
	ScaleQuantTable( m_qtblCbCr, std_chrominance_quant_tbl, aanscales );
}

////////////////////////////////////////////////////////////////////////////////

void CMiniJpegDecoder::ScaleQuantTable(
			unsigned short* tblRst,		//result quant table
			unsigned short* tblStd,		//standard quant table
			unsigned short* tblAan		//scale factor for AAN dct
			)
{
	int i, temp, half = 1<<11;

	for (i = 0; i < 64; i++) 
	{
		// (1) user scale up
		temp = (int)(( m_nScale * tblStd[i] + 50 ) / 100 );

		// limit to baseline range 
		if (temp <= 0) 
			temp = 1;
		if (temp > 255)
			temp = 255;		

		// (2) scaling needed for AA&N algorithm
		tblRst[i] = (unsigned short)(( temp * tblAan[i] + half ) >> 12 );
	}
}


////////////////////////////////////////////////////////////////////////////////
//	Prepare four Huffman tables:
//		HUFFMAN_TABLE m_htblYDC, m_htblYAC, m_htblCbCrDC, m_htblCbCrAC;

void CMiniJpegDecoder::InitHuffmanTable( void )
{
	//	Y dc component
	static unsigned char bitsYDC[17] =
    { 0, 0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 };
	static unsigned char valYDC[] =
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };	

	//	CbCr dc
	static unsigned char bitsCbCrDC[17] =
    { 0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 };
	static unsigned char valCbCrDC[] =
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };	

	//	Y ac component
	static unsigned char bitsYAC[17] =
    { 0, 0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d };
	static unsigned char valYAC[] =
    { 0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
	0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
	0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
	0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
	0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
	0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
	0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
	0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
	0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
	0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
	0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
	0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
	0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
	0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
	0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
	0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
	0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
	0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
	0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
	0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
	0xf9, 0xfa };	

	//	CbCr ac
	static unsigned char bitsCbCrAC[17] =
    { 0, 0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77 };
	static unsigned char valCbCrAC[] =
    { 0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
	0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
	0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
	0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
	0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
	0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
	0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
	0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
	0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
	0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
	0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
	0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
	0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
	0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
	0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
	0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
	0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
	0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
	0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
	0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
	0xf9, 0xfa };

	//	Compute four derived Huffman tables

	ComputeHuffmanTable( bitsYDC, valYDC, &m_htblYDC );

	ComputeHuffmanTable( bitsYAC, valYAC, &m_htblYAC );

	ComputeHuffmanTable( bitsCbCrDC, valCbCrDC, &m_htblCbCrDC );

	ComputeHuffmanTable( bitsCbCrAC, valCbCrAC, &m_htblCbCrAC );
}

////////////////////////////////////////////////////////////////////////////////

//	Compute the derived values for a Huffman table.	

void CMiniJpegDecoder::ComputeHuffmanTable(
		unsigned char *	pBits, 
		unsigned char * pVal,
		HUFFTABLE * dtbl	
		)
{
	int p, i, l, si;
	int lookbits, ctr;
	char huffsize[257];
	unsigned int huffcode[257];
	unsigned int code;

	memcpy( dtbl->bits, pBits, 17 );
	memcpy( dtbl->huffval, pVal, 256 );
	
	/* Figure C.1: make table of Huffman code length for each symbol */
	/* Note that this is in code-length order. */
	p = 0;
	for (l = 1; l <= 16; l++) {
		for (i = 1; i <= (int) pBits[l]; i++)
			huffsize[p++] = (char) l;
	}
	huffsize[p] = 0;
	
	/* Figure C.2: generate the codes themselves */
	/* Note that this is in code-length order. */
	
	code = 0;
	si = huffsize[0];
	p = 0;
	while (huffsize[p]) {
		while (((int) huffsize[p]) == si) {
			huffcode[p++] = code;
			code++;
		}
		code <<= 1;
		si++;
	}
	
	/* Figure F.15: generate decoding tables for bit-sequential decoding */
	
	p = 0;
	for (l = 1; l <= 16; l++) {
		if (pBits[l]) {
			dtbl->valptr[l] = p; /* huffval[] index of 1st symbol of code length l */
			dtbl->mincode[l] = huffcode[p]; /* minimum code of length l */
			p += pBits[l];
			dtbl->maxcode[l] = huffcode[p-1]; /* maximum code of length l */
		} else {
			dtbl->maxcode[l] = -1;	/* -1 if no codes of this length */
		}
	}
	dtbl->maxcode[17] = 0xFFFFFL; /* ensures jpeg_huff_decode terminates */
	
	/* Compute lookahead tables to speed up decoding.
	 * First we set all the table entries to 0, indicating "too long";
	 * then we iterate through the Huffman codes that are short enough and
	 * fill in all the entries that correspond to bit sequences starting
	 * with that code.	 */
	
	memset( dtbl->look_nbits, 0, sizeof(int)*256 );
	
	int HUFF_LOOKAHEAD = 8;
	p = 0;
	for (l = 1; l <= HUFF_LOOKAHEAD; l++) 
	{
		for (i = 1; i <= (int) pBits[l]; i++, p++) 
		{
			/* l = current code's length, 
			p = its index in huffcode[] & huffval[]. Generate left-justified
			code followed by all possible bit sequences */
			lookbits = huffcode[p] << (HUFF_LOOKAHEAD-l);
			for (ctr = 1 << (HUFF_LOOKAHEAD-l); ctr > 0; ctr--) 
			{
				dtbl->look_nbits[lookbits] = l;
				dtbl->look_sym[lookbits] = pVal[p];
				lookbits++;
			}
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
//	CMiniJpegDecoder::DecompressImage(), the main function in this class !!
//	IMPORTANT: You should call GetImageInfo() to get image width and height,
//				Then allocate (m_nWidth * m_nHeight * 3) bytes for pOutBuf

bool CMiniJpegDecoder::DecompressImage(	
	unsigned char *pInBuf,	//in, source data, in jpg format
	unsigned char *pOutBuf	//out, bgr format, (m_nWidth * m_nHeight * 3) bytes
	)
{
	//	Error handling
	if(( pInBuf == 0 )||( pOutBuf == 0 ))
		return false;

	//	declares
	int xPixel, yPixel, xTile, yTile, cxTile, cyTile, cxBlock, cyBlock;
	int y, nTrueRows, nTrueCols, nRowBytes;
	unsigned char byTile[768], *pTileRow;
		
	//	horizontal and vertical count of tile, macroblocks, 
	//	or MCU(Minimum Coded Unit), in 16*16 pixels
	cxTile = (m_nWidth + 15) >> 4;	
	cyTile = (m_nHeight + 15) >> 4;

	//	horizontal and vertical count of block, in 8*8 pixels
	cxBlock = cxTile << 1;
	cyBlock = cyTile << 1;

	int bpp = m_bGray ? 1 : 3;
	//	BMP row width, must be divided by 4
	nRowBytes = (m_nWidth * bpp + 3) / 4 * 4;

	m_pData = pInBuf;
	
	//	Decompress all the tiles, or macroblocks, or MCUs
	for( yTile = 0; yTile < cyTile; yTile++ )
	{
		for( xTile = 0; xTile < cxTile; xTile++ )
		{
			//	Decompress one macroblock started from m_pData;
			//	This function will push m_pData ahead
			//	Result is storing in byTile
			bool bresult;
			if (m_bGray)
				bresult = DecompressOneGrayTile( byTile );
			else
				bresult = DecompressOneTile( byTile );
			if( ! bresult)
				return false;
//			memset(byTile, 255, 256);
			//	Get tile starting pixel position
			xPixel = xTile << 4;
			yPixel = yTile << 4;

			//	Get the true number of tile columns and rows
			nTrueRows = 16;
			nTrueCols = 16;			
			if( yPixel + nTrueRows > m_nHeight )
				nTrueRows = m_nHeight - yPixel;
			if( xPixel + nTrueCols > m_nWidth )
				nTrueCols = m_nWidth - xPixel;

			//	Write the output bgr data
			pTileRow = pOutBuf + yPixel * nRowBytes + xPixel * bpp;
			for( y = 0; y < nTrueRows; y ++ )
			{
				memcpy( pTileRow, byTile + y * 16 * bpp, nTrueCols * bpp );
				pTileRow += nRowBytes;			
			}		
		}
	}	
	return true;
}

////////////////////////////////////////////////////////////////////////////////
//	function Purpose:	decompress one 16*16 pixels
//	source is m_pData;
//	This function will push m_pData ahead for next tile

bool CMiniJpegDecoder::DecompressOneTile(	
	unsigned char * pBgr	//out, in BGR format, 16*16*3
	)
{
	unsigned char pYCbCr[384];//Three color components, 256 + 64 + 64 bytes 
	short coef[64];	
	
	//	Do Y/Cb/Cr components, Y: 4 blocks; Cb: 1 block; Cr: 1 block
	int blocks = m_bGray ? 4 : 6;
	for( int i=0; i<blocks; i++ )
	{
		HuffmanDecode( coef, i );	//source is m_pData; coef is result
		InverseDct( coef, pYCbCr + i*64, i );	//De-scale and inverse dct		
	}

	//	Color conversion and up-sampling
	YCbCrToBGREx( pYCbCr, pBgr );
	return true;
}

bool CMiniJpegDecoder::DecompressOneGrayTile(	
	unsigned char * pGray	//out, in gray format
	)
{
	unsigned char pYs[256];
	short coef[64];	
	int i;
	for( i=0; i<4; i++ )
		{
		HuffmanDecode( coef, i );	//source is m_pData; coef is result
		InverseDct( coef, pYs + i*64, i );	//De-scale and inverse dct		
		}

	int x,y,z;
	for( i = 0, z= 0; i < 4; i++ )
		{
		for (y = 0; y < 8; y++)
		for (x = 0; x < 8; x++)
			{
			int dx = x + 8 * (i & 2);
			int dy = y + 8 * (i / 2);
			pGray[16 * dy + dx] = pYs[z++];
			}
		}
	return true;
}


////////////////////////////////////////////////////////////////////////////////
//	Color conversion and up-sampling

void CMiniJpegDecoder::YCbCrToBGREx(	
		unsigned char * pYCbCr,	//in, Y: 256 bytes; Cb: 64 bytes; Cr: 64 bytes 
		unsigned char * pBgr	//out, BGR format, 16*16*3 = 768 bytes
		)
{
	int i, j;
	unsigned char y, cb, cr, *pByte, *py[4], *pcb, *pcr;

	pByte = pBgr;
	for( i = 0; i < 4; i++ )
		py[i] = pYCbCr + i * 64;
	pcb	  = pYCbCr + 256;
	pcr   = pYCbCr + 320;
	unsigned char * range_limit = m_tblRange + 256;
	
	for( j=0; j<16; j++ )//vertical axis
	{
		for( i=0; i<16; i++ )	//horizontal axis
		{			
			//	block number is ((j/8) * 2 + i/8): {0, 1, 2, 3}
			y = *( py[(j>>3) * 2 + (i>>3)] ++ );
		
			cb = pcb[j/2 * 8 + i/2];
			cr = pcr[j/2 * 8 + i/2]; 

			//	Blue
			*(pByte++) = range_limit[ y + m_CbToB[cb] ];

			//	Green
			*(pByte++) = range_limit[ y + ((m_CbToG[cb] + m_CrToG[cr])>>16) ];

			//	Red
			*(pByte++) = range_limit[ y + m_CrToR[cr] ];
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

//	AA&N DCT algorithm implemention

void CMiniJpegDecoder::InverseDct( 
		short* coef, 			//in, dct coefficients, length = 64
		unsigned char* data, 	//out, 64 bytes		
		int nBlock				//block index: 0~3:Y; 4:Cb; 5:Cr
		)
{

#define FIX_1_082392200  ((int)277)		/* FIX(1.082392200) */
#define FIX_1_414213562  ((int)362)		/* FIX(1.414213562) */
#define FIX_1_847759065  ((int)473)		/* FIX(1.847759065) */
#define FIX_2_613125930  ((int)669)		/* FIX(2.613125930) */
#ifndef MULTIPLY	
#define MULTIPLY(var,cons)  ((int) ((var)*(cons))>>8 )
#endif
	int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
	int tmp10, tmp11, tmp12, tmp13;
	int z5, z10, z11, z12, z13;
	int workspace[64];		/* buffers data between passes */

	short* inptr = coef;
	unsigned short* quantptr;	
	int* wsptr = workspace;
	unsigned char* outptr;
	unsigned char* range_limit = &(m_tblRange[256+128]);
	int ctr, dcval, DCTSIZE = 8;

	if( nBlock < 4 )
		quantptr = m_qtblY;
	else
		quantptr = m_qtblCbCr;
	
	//Pass 1: process columns from input (inptr), store into work array(wsptr)
	
	for (ctr = 8; ctr > 0; ctr--) {
    /* Due to quantization, we will usually find that many of the input
	* coefficients are zero, especially the AC terms.  We can exploit this
	* by short-circuiting the IDCT calculation for any column in which all
	* the AC terms are zero.  In that case each output is equal to the
	* DC coefficient (with scale factor as needed).
	* With typical images and quantization tables, half or more of the
	* column DCT calculations can be simplified this way.
	*/
		
		if ((inptr[DCTSIZE*1] | inptr[DCTSIZE*2] | inptr[DCTSIZE*3] |
			inptr[DCTSIZE*4] | inptr[DCTSIZE*5] | inptr[DCTSIZE*6] |
			inptr[DCTSIZE*7]) == 0) 
		{
			/* AC terms all zero */
			dcval = (int)( inptr[DCTSIZE*0] * quantptr[DCTSIZE*0] );
			
			wsptr[DCTSIZE*0] = dcval;
			wsptr[DCTSIZE*1] = dcval;
			wsptr[DCTSIZE*2] = dcval;
			wsptr[DCTSIZE*3] = dcval;
			wsptr[DCTSIZE*4] = dcval;
			wsptr[DCTSIZE*5] = dcval;
			wsptr[DCTSIZE*6] = dcval;
			wsptr[DCTSIZE*7] = dcval;
			
			inptr++;			/* advance pointers to next column */
			quantptr++;
			wsptr++;
			continue;
		}
		
		/* Even part */
		
		tmp0 = inptr[DCTSIZE*0] * quantptr[DCTSIZE*0];
		tmp1 = inptr[DCTSIZE*2] * quantptr[DCTSIZE*2];
		tmp2 = inptr[DCTSIZE*4] * quantptr[DCTSIZE*4];
		tmp3 = inptr[DCTSIZE*6] * quantptr[DCTSIZE*6];
		
		tmp10 = tmp0 + tmp2;	/* phase 3 */
		tmp11 = tmp0 - tmp2;
		
		tmp13 = tmp1 + tmp3;	/* phases 5-3 */
		tmp12 = MULTIPLY(tmp1 - tmp3, FIX_1_414213562) - tmp13; /* 2*c4 */
		
		tmp0 = tmp10 + tmp13;	/* phase 2 */
		tmp3 = tmp10 - tmp13;
		tmp1 = tmp11 + tmp12;
		tmp2 = tmp11 - tmp12;
		
		/* Odd part */
		
		tmp4 = inptr[DCTSIZE*1] * quantptr[DCTSIZE*1];
		tmp5 = inptr[DCTSIZE*3] * quantptr[DCTSIZE*3];
		tmp6 = inptr[DCTSIZE*5] * quantptr[DCTSIZE*5];
		tmp7 = inptr[DCTSIZE*7] * quantptr[DCTSIZE*7];
		
		z13 = tmp6 + tmp5;		/* phase 6 */
		z10 = tmp6 - tmp5;
		z11 = tmp4 + tmp7;
		z12 = tmp4 - tmp7;
		
		tmp7  = z11 + z13;		/* phase 5 */
		tmp11 = MULTIPLY(z11 - z13, FIX_1_414213562); /* 2*c4 */
		
		z5	  = MULTIPLY(z10 + z12, FIX_1_847759065); /* 2*c2 */
		tmp10 = MULTIPLY(z12, FIX_1_082392200) - z5; /* 2*(c2-c6) */
		tmp12 = MULTIPLY(z10, - FIX_2_613125930) + z5; /* -2*(c2+c6) */
		
		tmp6 = tmp12 - tmp7;	/* phase 2 */
		tmp5 = tmp11 - tmp6;
		tmp4 = tmp10 + tmp5;
		
		wsptr[DCTSIZE*0] = (int) (tmp0 + tmp7);
		wsptr[DCTSIZE*7] = (int) (tmp0 - tmp7);
		wsptr[DCTSIZE*1] = (int) (tmp1 + tmp6);
		wsptr[DCTSIZE*6] = (int) (tmp1 - tmp6);
		wsptr[DCTSIZE*2] = (int) (tmp2 + tmp5);
		wsptr[DCTSIZE*5] = (int) (tmp2 - tmp5);
		wsptr[DCTSIZE*4] = (int) (tmp3 + tmp4);
		wsptr[DCTSIZE*3] = (int) (tmp3 - tmp4);
		
		inptr++;			/* advance pointers to next column */
		quantptr++;
		wsptr++;
	}
	
	/* Pass 2: process rows from work array, store into output array. */
	/* Note that we must descale the results by a factor of 8 == 2**3, */
	/* and also undo the PASS1_BITS scaling. */

int RANGE_MASK = 1023; //2 bits wider than legal samples
#define PASS1_BITS  2
#define IDESCALE(x,n)  ((int) ((x)>>(n)) )
	
	wsptr = workspace;
	for (ctr = 0; ctr < DCTSIZE; ctr++) {
		outptr = data + ctr * 8;
	
		/* Rows of zeroes can be exploited in the same way as we did with columns.
		* However, the column calculation has created many nonzero AC terms, so
		* the simplification applies less often (typically 5% to 10% of the time).
		* On machines with very fast multiplication, it's possible that the
		* test takes more time than it's worth.  In that case this section
		* may be commented out.
		*/
		
		if ((wsptr[1] | wsptr[2] | wsptr[3] | wsptr[4] | wsptr[5] | wsptr[6] |
			wsptr[7]) == 0) {
			/* AC terms all zero */
			dcval = (int) range_limit[ (wsptr[0] >> 5) & RANGE_MASK];		
			outptr[0] = dcval;
			outptr[1] = dcval;
			outptr[2] = dcval;
			outptr[3] = dcval;
			outptr[4] = dcval;
			outptr[5] = dcval;
			outptr[6] = dcval;
			outptr[7] = dcval;
			
			wsptr += DCTSIZE;		/* advance pointer to next row */
			continue;
		}
		
		/* Even part */
		
		tmp10 = ((int) wsptr[0] + (int) wsptr[4]);
		tmp11 = ((int) wsptr[0] - (int) wsptr[4]);
		
		tmp13 = ((int) wsptr[2] + (int) wsptr[6]);
		tmp12 = MULTIPLY((int) wsptr[2] - (int) wsptr[6], FIX_1_414213562)
			- tmp13;
		
		tmp0 = tmp10 + tmp13;
		tmp3 = tmp10 - tmp13;
		tmp1 = tmp11 + tmp12;
		tmp2 = tmp11 - tmp12;
		
		/* Odd part */
		
		z13 = (int) wsptr[5] + (int) wsptr[3];
		z10 = (int) wsptr[5] - (int) wsptr[3];
		z11 = (int) wsptr[1] + (int) wsptr[7];
		z12 = (int) wsptr[1] - (int) wsptr[7];
		
		tmp7 = z11 + z13;		/* phase 5 */
		tmp11 = MULTIPLY(z11 - z13, FIX_1_414213562); /* 2*c4 */
		
		z5    = MULTIPLY(z10 + z12, FIX_1_847759065); /* 2*c2 */
		tmp10 = MULTIPLY(z12, FIX_1_082392200) - z5; /* 2*(c2-c6) */
		tmp12 = MULTIPLY(z10, - FIX_2_613125930) + z5; /* -2*(c2+c6) */
		
		tmp6 = tmp12 - tmp7;	/* phase 2 */
		tmp5 = tmp11 - tmp6;
		tmp4 = tmp10 + tmp5;
		
		/* Final output stage: scale down by a factor of 8 and range-limit */
		
		outptr[0] = range_limit[(IDESCALE(tmp0 + tmp7, PASS1_BITS+3))
			& RANGE_MASK];
		outptr[7] = range_limit[(IDESCALE(tmp0 - tmp7, PASS1_BITS+3))
			& RANGE_MASK];
		outptr[1] = range_limit[(IDESCALE(tmp1 + tmp6, PASS1_BITS+3))
			& RANGE_MASK];
		outptr[6] = range_limit[(IDESCALE(tmp1 - tmp6, PASS1_BITS+3))
			& RANGE_MASK];
		outptr[2] = range_limit[(IDESCALE(tmp2 + tmp5, PASS1_BITS+3))
			& RANGE_MASK];
		outptr[5] = range_limit[(IDESCALE(tmp2 - tmp5, PASS1_BITS+3))
			& RANGE_MASK];
		outptr[4] = range_limit[(IDESCALE(tmp3 + tmp4, PASS1_BITS+3))
			& RANGE_MASK];
		outptr[3] = range_limit[(IDESCALE(tmp3 - tmp4, PASS1_BITS+3))
			& RANGE_MASK];
		
		wsptr += DCTSIZE;		/* advance pointer to next row */
	}
}

////////////////////////////////////////////////////////////////////////////////

//	Below are difficult and complex HUFFMAN decoding !!!!!


////////////////////////////////////////////////////////////////////////////////
//	HuffmanDecode( coef, i ); //source is m_pData; coef is result

void CMiniJpegDecoder::HuffmanDecode( 
		short* coef,			//	out, DCT coefficients
		int iBlock				//	0,1,2,3:Y; 4:Cb; 5:Cr
		)
{	
/*
* jpeg_natural_order[i] is the natural-order position of the i'th 
* element of zigzag order.
*
* When reading corrupted data, the Huffman decoders could attempt
* to reference an entry beyond the end of this array (if the decoded
* zero run length reaches past the end of the block).  To prevent
* wild stores without adding an inner-loop test, we put some extra
* "63"s after the real entries.  This will cause the extra coefficient
* to be stored in location 63 of the block, not somewhere random.
* The worst case would be a run-length of 15, which means we need 16
* fake entries.
*/	
	static const int jpeg_natural_order[64+16] = {
			0,  1,  8, 16,  9,  2,  3, 10,
			17, 24, 32, 25, 18, 11,  4,  5,
			12, 19, 26, 33, 40, 48, 41, 34,
			27, 20, 13,  6,  7, 14, 21, 28,
			35, 42, 49, 56, 57, 50, 43, 36,
			29, 22, 15, 23, 30, 37, 44, 51,
			58, 59, 52, 45, 38, 31, 39, 46,
			53, 60, 61, 54, 47, 55, 62, 63,
			63, 63, 63, 63, 63, 63, 63, 63,//extra entries for safety
			63, 63, 63, 63, 63, 63, 63, 63
	};

	int* pLastDC;
	int s, k, r;

	HUFFTABLE *dctbl, *actbl;

	if( iBlock < 4 )
	{
		dctbl = &m_htblYDC;
		actbl = &m_htblYAC;
		pLastDC = &m_dcY;
	}
	else
	{
		dctbl = &m_htblCbCrDC;
		actbl = &m_htblCbCrAC;
		if( iBlock == 4 )
			pLastDC = &m_dcCb;
		else
			pLastDC = &m_dcCr;
	}

	memset( coef, 0, sizeof(short) * 64 );
	
    /* Section F.2.2.1: decode the DC coefficient difference */
	s = GetCategory( dctbl );		//get dc category number, s

	if (s) {
		r = GetBits(s);					//get offset in this dc category
		s = ValueFromCategory(s, r);	//get dc difference value
    }
	
    /* Convert DC difference to actual value, update last_dc_val */
    s += *pLastDC;
    *pLastDC = s;

    /* Output the DC coefficient (assumes jpeg_natural_order[0] = 0) */
    coef[0] = (short) s;	
    
	/* Section F.2.2.2: decode the AC coefficients */
	/* Since zeroes are skipped, output area must be cleared beforehand */
	for (k = 1; k < 64; k++) 
	{
		s = GetCategory( actbl );	//s: (run, category)
		r = s >> 4;			//	r: run length for ac zero, 0 <= r < 16
		s &= 15;			//	s: category for this non-zero ac
		
		if( s ) 
		{
			k += r;					//	k: position for next non-zero ac
			r = GetBits(s);			//	r: offset in this ac category
			s = ValueFromCategory(s, r);	//	s: ac value

			coef[ jpeg_natural_order[ k ] ] = (short) s;
		} 
		else // s = 0, means ac value is 0 ? Only if r = 15.  
		{
			if (r != 15)	//means all the left ac are zero
				break;
			k += 15;
		}
	}		
}



////////////////////////////////////////////////////////////////////////////////
//get category number for dc, or (0 run length, ac category) for ac

//	The max length for Huffman codes is 15 bits; so we use 32 bits buffer	
//	m_nGetBuff, with the validated length is m_nGetBits.
//	Usually, more than 95% of the Huffman codes will be 8 or fewer bits long
//	To speed up, we should pay more attention on the codes whose length <= 8

inline int CMiniJpegDecoder::GetCategory( HUFFTABLE* htbl )
{
	//	If left bits < 8, we should get more data
	if( m_nGetBits < 8 )
		FillBitBuffer( );

	//	Call special process if data finished; min bits is 1
	if( m_nGetBits < 8 )
		return SpecialDecode( htbl, 1 );

	//	Peek the first valid byte	
	int look = ((m_nGetBuff>>(m_nGetBits - 8))& 0xFF);
	int nb = htbl->look_nbits[look];

	if( nb ) 
	{ 
		m_nGetBits -= nb;
		return htbl->look_sym[look]; 
	} 
	else	//Decode long codes with length >= 9
		return SpecialDecode( htbl, 9 );
}

////////////////////////////////////////////////////////////////////////////////

void CMiniJpegDecoder::FillBitBuffer( void )
{
	unsigned char uc;
	while( m_nGetBits < 25 )	//#define MIN_GET_BITS  (32-7)
	{
		if( m_nDataBytes > 0 )//Are there some data?
		{
			uc = *m_pData++;
			m_nGetBuff = (m_nGetBuff << 8) | ((int) uc);
			m_nGetBits += 8;
			m_nDataBytes --;
		}
		else
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////

inline int CMiniJpegDecoder::GetBits(int nbits) 
{
	if( m_nGetBits < nbits )//we should read nbits bits to get next data
		FillBitBuffer();
	m_nGetBits -= nbits;
	return (int) (m_nGetBuff >> m_nGetBits) & ((1<<nbits)-1);
}

////////////////////////////////////////////////////////////////////////////////
//	Special Huffman decode:
//	(1) For codes with length > 8
//	(2) For codes with length < 8 while data is finished

int CMiniJpegDecoder::SpecialDecode( HUFFTABLE* htbl, int nMinBits )
{
	
	int l = nMinBits;
	int code;
	
	/* HUFF_DECODE has determined that the code is at least min_bits */
	/* bits long, so fetch that many bits in one swoop. */

	code = GetBits(l);
	
	/* Collect the rest of the Huffman code one bit at a time. */
	/* This is per Figure F.16 in the JPEG spec. */
	while (code > htbl->maxcode[l]) {
		code <<= 1;
		code |= GetBits(1);
		l++;
	}
	
	/* With garbage input we may reach the sentinel value l = 17. */
	if (l > 16) {
		return 0;			/* fake a zero as the safest result */
	}
	
	return htbl->huffval[ htbl->valptr[l] +	(int)(code - htbl->mincode[l]) ];
}

////////////////////////////////////////////////////////////////////////////////
//	To find dc or ac value according to category and category offset

inline int CMiniJpegDecoder::ValueFromCategory(int nCate, int nOffset)
{
/*	//Method 1: 
	//On some machines, a shift and add will be faster than a table lookup.
	#define HUFF_EXTEND(x,s) \
	((x)< (1<<((s)-1)) ? (x) + (((-1)<<(s)) + 1) : (x)) 
*/
	//Method 2: Table lookup
	
	//If (nOffset < half[nCate]), then value is below zero
	//Otherwise, value is above zero, and just the nOffset
	static const int half[16] =		/* entry n is 2**(n-1) */
	{ 0, 0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080,
    0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000 };
	
	//start[i] is the starting value in this category; surely it is below zero
	static const int start[16] =	/* entry n is (-1 << n) + 1 */
	{ 0, ((-1)<<1) + 1, ((-1)<<2) + 1, ((-1)<<3) + 1, ((-1)<<4) + 1,
    ((-1)<<5) + 1, ((-1)<<6) + 1, ((-1)<<7) + 1, ((-1)<<8) + 1,
    ((-1)<<9) + 1, ((-1)<<10) + 1, ((-1)<<11) + 1, ((-1)<<12) + 1,
    ((-1)<<13) + 1, ((-1)<<14) + 1, ((-1)<<15) + 1 };	

	return ( nOffset < half[nCate] ? nOffset + start[nCate] : nOffset);	
}
