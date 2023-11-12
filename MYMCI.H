#ifndef _MYMCI_DEF
#define _MYMCI_DEF
class CMyMCI
{
public:
	CMyMCI();
	~CMyMCI();
HWND ThisWnd() { return m_Video;};
int Create(HWND hwnd, LPCSTR name);
int Open(LPCSTR name);
int SetSize(int width, int height);
int Close();
int Destroy();
int Play();
int Pause();
int Resume();
int Repeat(BOOL bLoop);
int Stop();
int Seek(UINT pos);
UINT Where();	// gets postion
UINT Length();	// gets pnbr of frames
int CanPlay();

protected:
	BOOL m_bOpened;
	HWND m_Video;
};
#endif
