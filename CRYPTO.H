#ifndef _MYCRYPTO_H_
#define _MYCRYPTO_H_
class CMyCrypto
{
public:
	CMyCrypto(int nBump = 0);
	void Encrypt(void * pDst, void * pSrc, void * pKey = 0);
	void Decrypt(void * pDst, void * pSrc, void * pKey = 0);
protected:
	int m_nKeyBump;
	void Idea(void *in, void  *out, void *ks);
};
#endif
