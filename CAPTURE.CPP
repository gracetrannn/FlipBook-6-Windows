#include "stdafx.h"
#include "sketch.h"
#include "myview.h"
#include "capture.h"
#include "math.h"
#include "fbqt.h"
#include "ccell.h"
#include "sceneopt.h"
#include "keyer.h"
#include "mydialog.h"
#define ALLOW_AUTO



#define STAT_DEAD 0
#define STAT_EMPTY 1
#define STAT_READY 2
#define STAT_NEW_SIZE 3
#define STAT_FULL 9


#define FLAG_ASPECT 1
#define FLAG_CURSOR 2
#define FLAG_WV1    4
#define FLAG_WV2    8  
#define FLAG_CROP  16
#define FLAG_SECRET 32
#define FLAG_ALL (FLAG_ASPECT|FLAG_CURSOR|FLAG_WV1|FLAG_CROP|FLAG_SECRET|448)
// key invert is 64, key kind is 128 and 256


typedef struct {
	UINT	frame;
	UINT	level;
	UINT	skip;
	CString * plabel;
	CColorKey * pKeyer;
	CSketchDoc * pDoc;
} RECINFO, * PRECINFO;

class CCapOptDlg : public CMyDialog
{
public:
	CCapOptDlg(CWnd* pParent = NULL);	// standard constructor
	enum { IDD = IDD_CAP_OPTIONS };
	CComboBox	m_devices;
	CListBox	m_options;
	PCAPPACKET m_pArgs;
	PCAPINFO m_pInfo;
	BOOL m_bBozo;
	void FillCombos();
	char m_szDevice[80];
	protected:
	CMySpin	m_spnavg;
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CWnd * m_pParent;
	// Generated message map functions
	virtual BOOL OnInitDialog();
//	afx_msg void OnDialog(UINT Id);
	afx_msg void OnCapRotate();
	afx_msg void OnCapAspect();
	afx_msg void OnCapAlternate();
	afx_msg void OnCapBozo();
//	afx_msg void OnCapNumbers();
	afx_msg void OnCapCrop();
	afx_msg void OnSelChangeDevice();
	afx_msg void OnSelChangeOption();
	afx_msg void OnChangeAvg();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

void white_key(BYTE * dstp, BYTE * srcp, UINT w,UINT h,UINT d,UINT white);
void onion_skin(BYTE * dstp, BYTE * srcp, UINT w,UINT h,UINT d);

void MyScaler1(BYTE * dstp, UINT ow, UINT oh, UINT op, 
			BYTE * srcp, UINT iw, UINT ih, UINT ip, UINT d, BYTE * pTable)
{
//DPF("myscal,iw:%d,ih:%d,ow:%d,oh:%d",iw,ih,ow,oh);
//	return;
	int vacc = 0;
	UINT y;
//	UINT op = 4 * ((d * ow + 3) / 4);
//	UINT ip = 4 * ((3 * iw + 3) / 4);
	for (y = 0; y < oh; y++)
		{
		int hacc = 0;
		UINT ox, ix;
		if (pTable || (d == 1))
			{
			for (ox = 0, ix = 0; ox < ow; ox++)
				{
				UINT q, r, g, b;
				b = srcp[3*ix+0];
				g = srcp[3*ix+1];
				r = srcp[3*ix+2];
#ifdef LUMMY
				q = (30 * r + 59 * g + 11 * b) / 100;
#else
				q = (r + g + b) / 3;
#endif
				if (pTable)
					q = pTable[q];
				if (d == 1)
					dstp[ox] = q;
				else
					{
					dstp[3*ox+0] = q;
					dstp[3*ox+1] = q;
					dstp[3*ox+2] = q;
					}
				for (hacc += iw;hacc >= (int)ow; hacc -= ow, ix++);
				}
			}
		else
			{
			if (ow == iw)
				memcpy(dstp, srcp, ip);
			else
				{
				for (ox = 0, ix = 0; ox < ow; ox++)
					{
					dstp[3*ox+0] = srcp[3*ix+0];
					dstp[3*ox+1] = srcp[3*ix+1];
					dstp[3*ox+2] = srcp[3*ix+2];
					for (hacc += iw;hacc >= (int)ow; hacc -= ow, ix++);
					}
				}
			}
		vacc += ih;
		for(;vacc >= (int)oh;)
			{
			vacc -= oh;
			srcp += ip;
			}
		dstp += op;
		}
}


void MyScaler2(BYTE * dstp, UINT ow, UINT oh, UINT op,
			BYTE * srcp, UINT iw, UINT ih, UINT ip, UINT d, BYTE * pTable)
{
	int vacc = 0;
	UINT y;
	srcp += ip * (ih - 1);
	for (y = 0; y < oh; y++)
		{
		int hacc = 0;
		UINT ox, ix;
		ix = iw - 1;
		if (pTable || (d == 1))
			{
			for (ox = 0; ox < ow; ox++)
				{
				UINT q, r, g, b;
				b = srcp[3*ix+0];
				g = srcp[3*ix+1];
				r = srcp[3*ix+2];
#ifdef LUMMY
				q = (30 * r + 59 * g + 11 * b) / 100;
#else
				q = (r + g + b) / 3;
#endif
				if (pTable)
					q = pTable[q];
				if (d == 1)
					dstp[ox] = q;
				else
					{
					dstp[3*ox+0] = q;
					dstp[3*ox+1] = q;
					dstp[3*ox+2] = q;
					}
				for (hacc += iw;hacc >= (int)ow; hacc -= ow, ix--);
				}
			}
		else
			{
			for (ox = 0; ox < ow; ox++)
				{
				dstp[3*ox+0] = srcp[3*ix+0];
				dstp[3*ox+1] = srcp[3*ix+1];
				dstp[3*ox+2] = srcp[3*ix+2];
				for (hacc += iw;hacc >= (int)ow; hacc -= ow, ix--);
				}
			}
		vacc += ih;
		for(;vacc >= (int)oh;)
			{
			vacc -= oh;
			srcp -= ip;
			}
		dstp += op;
		}
}

void CSketchView::OnMyPaint()
{
	if (!m_packet.command || !m_pTemp1)
		return;
	if (m_info.infStatus == STAT_NEW_SIZE)
		{
		delete [] m_info.pBI;
DPF("new size:%d",m_info.Size);
		UINT w = m_info.Width;
		UINT h = m_info.Height;
		UINT p = 4 * ((3 * w + 3) / 4);
DPF("w:%d,h:%d",w,h);
		m_info.pBI = new BYTE[9  * m_info.Size+100];
		m_info.pBits = m_info.pBI + 40;
		m_info.Counter = 0;
		m_info.infStatus = STAT_EMPTY;
		return;
		}
	if (m_info.infStatus != STAT_READY)
		{
		if (m_info.infStatus != STAT_FULL)
			{
			DPF("bad status:%d",m_info.infStatus);
			}
		return;
		}
/*
	UINT x, y;
	UINT w = m_info.Width;
	UINT h = m_info.Height;
	UINT p = 4 * ((3 * w + 3) / 4);
	for (y = 0; y < h; y++)
	for (x = 0; x < w; x++)
		{
		UINT v = 255 * x / w;
		m_info.pBits[y*p+3*x+0] = v;
		m_info.pBits[y*p+3*x+1] = v;
		m_info.pBits[y*p+3*x+2] = v;
		}
*/
	ProcessSample();
	if (m_packet.pParent && (m_packet.command == 2))
		{
		BYTE * srcp = m_info.pBits;					// capturing
//		if (m_packet.process && m_packet.align && !m_packet.gmode)		// if gray
		if (m_packet.process && !m_packet.gmode)		// if gray
			{										// convert color into gray
			UINT w = m_info.Width;
			UINT h = m_info.Height;
			UINT p = 4 * ((3 * w + 3) / 4);
#ifndef LUMMY
			UINT z,c;
			c = h * p;
			for (z = 0; z < c; z++)
				srcp[z] = m_table[srcp[z]];
#else
			UINT x,y;
			for (y = 0; y < h; y++)
				{
				for (x = 0; x < w; x++)
					{
					UINT q, r, g, b;
					b = srcp[3*x+0];
					g = srcp[3*x+1];
					r = srcp[3*x+2];
					q = (30 * r + 59 * g + 11 * b) / 100;
					q = m_table[q];
					srcp[3*x+0] = q;
					srcp[3*x+1] = q;
					srcp[3*x+2] = q;
					}
				srcp += p;
				}
#endif
			}
		m_packet.pParent->PostMessage(WM_COMMAND,ID_MY_PAINT,0);//onreceive
		}
	else
		{
		m_info.infStatus = STAT_EMPTY; // ack it
		InvalidateRect(0,0);
		}
}


void CSketchView::CapKeyLogic(CDC * pDC)
{
	CPoint point;
	GetCursorPos(&point);
//	DPF("apoint %d,%d",point.x,point.y);
	ScreenToClient(&point);
//	CClientDC dc(this);
	pDC->DPtoLP(&point);
//	DPF("bpoint %d,%d",point.x,point.y);
	XlatePoint(point);
//	DPF("cpoint %d,%d",point.x,point.y);
	if ((point.x && point.y &&
		((UINT)(point.x+1) < m_swidth) && 
		((UINT)(point.y+1) < m_sheight )))
		{
		BYTE *pp = m_pTemp1 + (m_sheight - 1 - point.y-1) * 
					m_spitch + 3 * point.x -3;
		int i,j;
		m_packet.blue = 0;
		m_packet.green= 0;
		m_packet.red  = 0;
		for (i = 0; i < 3; i++)
			{
			for (j = 0; j < 3; j++)
				{
				m_packet.blue += pp[3*j+0];
				m_packet.green+= pp[3*j+1];
				m_packet.red  += pp[3*j+2];
				}
			pp += m_spitch;
			}
		m_packet.red /= 9;
		m_packet.green /= 9;
		m_packet.blue /= 9;
//DPF("r:%d,g:%d,b:%d",kr,kg,kb);
		BOOL bStop = GetKeyState(VK_LBUTTON) < 0 ? 1 : 0;
		if (m_packet.pKeyDlg)
			((CColorKeyDlg *)m_packet.pKeyDlg)->SetColor(bStop);//pp[2],pp[1],pp[0]);
		}
}

void CSketchView::CapPaint(CDC * pDC)
{
	RECT rectClip;
	pDC->GetClipBox(&rectClip);
	if (!m_pBits || !m_hdd)
		{
		pDC->FillSolidRect(&rectClip, m_crBack);
DPF("cant paint");
		return;
		}
	UINT d = ((LPBITMAPINFOHEADER)m_pDIB)->biBitCount;
	d = (d + 7) / 8;
	if ((d == 3) && m_pTemp1 && m_packet.pKeyDlg)
		{
		CapKeyLogic(pDC);	
		}
	BYTE * pDIB = m_pTemp1;
	if (m_packet.process && pDIB)
//	if (pDIB)
		{
		if ((m_packet.flags & (FLAG_WV1|FLAG_WV2)) == (FLAG_WV1|FLAG_WV2))
			pDIB += (m_spitch * m_sheight);
//		else if (m_packet.align && m_packet.command && // m_packet.show && 
		else if (m_packet.command && // m_packet.show && 
					(m_info.infStatus == STAT_EMPTY))
			MergeSample();
		}
	UINT sw = m_offx + m_nNum * m_swidth / m_nDen;
	UINT sh = m_offy + m_nNum * m_sheight / m_nDen;
	if (m_offy)
		{
		CRect rectOff(0,0,rectClip.right,m_offy);
		rectClip.top = m_offy;
		pDC->FillSolidRect(rectOff, m_crBack);
		}
	if (m_offx)
		{
		CRect rectOff(0,rectClip.top,m_offx,rectClip.bottom);
		rectClip.left = m_offx;
		pDC->FillSolidRect(rectOff, m_crBack);
		}
	if ((DWORD)rectClip.right > sw)
		{
		CRect rectOff(sw,rectClip.top,rectClip.right,rectClip.bottom);
		rectClip.right = sw;
		pDC->FillSolidRect(rectOff, m_crBack);
		}
	if ((DWORD)rectClip.bottom > sh)
		{
		CRect rectOff(rectClip.left,sh,rectClip.right,rectClip.bottom);
		rectClip.bottom = sh;
		pDC->FillSolidRect(rectOff, m_crBack);
		}
	RECT src;
	XlateRect(rectClip,src,1);
	
//	DPR("my paint clip ",&rectClip);
//	DPR("src",&src);
	DrawDibDraw(m_hdd,
                pDC->m_hDC,
				rectClip.left, rectClip.top, 
				rectClip.right - rectClip.left, rectClip.bottom - rectClip.top,
                (LPBITMAPINFOHEADER)m_pDIB,	 pDIB,
				src.left, src.top, 
				src.right - src.left, src.bottom - src.top,
                0);
//	if ((m_packet.flags & (FLAG_WV1|FLAG_WV2)) == FLAG_WV1)
//	if (m_packet.process && (m_packet.flags & FLAG_WV1))
		{
		char buf[20];
		if (m_packet.flags & FLAG_WV2)
			strcpy(buf," cell image ");
		else
			strcpy(buf," camera image");
		CSize cs = pDC->GetTextExtent(buf);
		int old = pDC->SetROP2(R2_NOT);
		RECT rect;
		GetClientRect(&rect);
#ifdef FLIPBOOK_MAC
		CPoint pt1 = GetZeroScrollPosition();
#else
		CPoint pt1 = GetScrollPosition();
#endif
		rect.right += pt1.x;
		rect.bottom += pt1.y;
		rect.left = rect.right - cs.cx;
		rect.top = rect.bottom - cs.cy;
		UINT nFormat = DT_SINGLELINE | DT_LEFT | DT_VCENTER;
		COLORREF crOld,crNew;
		crNew = RGB(0,0,0);
		crOld = pDC->SetTextColor(crNew);
		CBrush br;
		br.CreateStockObject(WHITE_BRUSH);
		pDC->FillRect(&rect,&br);
    	pDC->DrawText(buf,-1, (LPRECT)&rect, nFormat);
		pDC->SetTextColor(crOld);
		pDC->SetROP2(old);
		}
	return;
}

void CSketchView::ProcessSample()
{
	UINT zcode = m_info.infStatus;
	UINT iw = m_info.Width;
	UINT ih = m_info.Height;
	UINT id = m_info.Depth;
	UINT ow = m_swidth;
	UINT oh = m_sheight;
	UINT d = ((LPBITMAPINFOHEADER)m_pDIB)->biBitCount;
	d = (d + 7) / 8;
	BYTE * pDst = m_pTemp1;
	BYTE * pTable = 0;
//	if (m_packet.process && m_packet.align && !m_packet.gmode)
	if (m_packet.process && !m_packet.gmode && !m_packet.align)
		{
		if ((m_white != m_packet.white) || (m_gamma != m_packet.gamma))
			BuildTable();
		pTable = m_table;
		}
	UINT op = 4 * ((d * m_swidth + 3) / 4);
	UINT ip = 4 * ((3 * iw + 3) / 4);
	if (m_packet.flags & FLAG_CROP)
		{
		
		memset(pDst,255,op * m_sheight);
		if ((DWORD)iw * oh > (DWORD)ih * ow)
			{
			iw = MulDiv(ih,ow,oh);
//			pDst += d * delta;
			}
		else
			{
			ih = MulDiv(iw,oh,ow);
//			pSrc += ip * delta;
			}
		}
	else if (m_packet.flags & FLAG_ASPECT)
		{
		memset(pDst,255,op * m_sheight);
		if ((DWORD)iw * oh > (DWORD)ih * ow)
			{
			UINT t = oh;
			oh = MulDiv(ih, ow, iw);
			pDst += op * ((t - oh) / 2);
			}
		else
			{
			UINT t = ow;
			ow = MulDiv(oh, iw, ih);
			pDst += d * ((t - ow) / 2);
			}
		}

//DPF("iw:%d,ih:%d,ow:%d,oh:%d",iw,ih,ow,oh);
	if (m_packet.rotate)
		{
		MyScaler2(pDst, ow, oh, op, m_info.pBits, iw,ih,ip,d,pTable);
		}
	else
		{
		MyScaler1(pDst, ow, oh, op, m_info.pBits, iw,ih,ip,d,pTable);
		}
}


void CSketchView::MergeSample()
{
	UINT w = m_swidth;
	UINT h = m_sheight;
	UINT d = ((LPBITMAPINFOHEADER)m_pDIB)->biBitCount;
	d = (d + 7) / 8;
	UINT p = 4 * ((d * w + 3) / 4);
	BYTE * srcp = m_pTemp1 + m_spitch * m_sheight;
	BYTE * dstp = m_pTemp1;
	UINT white;
	if (!m_packet.gmode)
		white = (255 * m_packet.white) / 100; // for gray
	else
		white = m_packet.whitekey;
//	DPF("merging:%d",m_packet.align);
	if (!m_packet.align) // keyed ?
		{
		if ((d == 3) && (m_packet.gmode == 1)) // color keying
			{
			(((CCapDlg*)m_packet.pParent)->Keyer())->Process(dstp, srcp, w,h,d,
				 m_packet.red, m_packet.green,m_packet.blue,
					m_packet.extent,m_packet.flags >> 6);
			}
		else if (white < 255)
			{
			white_key(dstp, srcp, w,h,d,white);
			}
		}

//	else
	if (m_packet.align == 11) // alternate lines
		{
		UINT y;
		for (y = 0; y < h; y++)
			{
			if (!(y % 2))
				memcpy(dstp, srcp, p);
			srcp += p;
			dstp += p;
			}
		}
	else if (m_packet.align == 1)
		{
		onion_skin(dstp, srcp, w,h,d);
		}
}

void CSketchView::BuildTable()
{
	m_white = m_packet.white;
	UINT white = (255 * m_white) / 100;
	m_gamma = m_packet.gamma;
DPF("building, %d, %d", white,m_gamma);
UINT m_black = 0;
	double c = (double)(white - m_black);
	double g = (double)m_gamma;
	g /= 100;
	UINT i;
	for (i = 0; i <= m_black; i++)
		m_table[i] = 0;
	for (; i < white; i++)
		{
		double a, b;
		a = (double)(i - m_black);
		a /= c;
		b = exp(log(a) * g);
		b *= 255;
		m_table[i] = (BYTE)b;
		}
	for (; i < 256; i++)
		m_table[i] = 255;
}


CCapDlg::CCapDlg(CWnd* pParent /*=NULL*/)
: CMyDialog(CCapDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCapDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_pParent = pParent;	
//	m_old_level = 99999;
//	m_old_frame = 99999;
	m_nAuto = 0;
	m_pKeyer = 0;
}

CCapDlg::~CCapDlg()
{
	FBVideoCaptureStop(1);
	delete m_pKeyer;
}

BOOL CCapDlg::DestroyWindow() 
{
	FBVideoCaptureStop(0);
    return CMyDialog::DestroyWindow();
}

void CCapDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCapDlg)
	
	DDX_Check(pDX, IDC_CAP_PREVIOUS, m_bPrev);
	#ifdef FLIPBOOK_MAC // the radio button order matters; it's reversed on Mac for this dialog
		m_gmode = !m_gmode;
		DDX_Radio(pDX, IDC_CAP_GRAY, m_gmode);
		m_gmode = !m_gmode;
	#else
		DDX_Radio(pDX, IDC_CAP_GRAY, m_gmode);
	#endif
	DDX_Control(pDX, IDC_CAP_WHITE, m_whiteq);
	DDX_Control(pDX, IDC_CAP_GAMMA, m_gamma);
	DDX_Control(pDX, IDC_CAP_WHTSLIDE, m_wslider);
	DDX_Control(pDX, IDC_CAP_GAMSLIDE, m_gslider);
	DDX_Text(pDX, IDC_LABEL, m_label);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCapDlg, CMyDialog)
	//{{AFX_MSG_MAP(CCapDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(IDC_CAP_CAPTURE, OnCapture)
	ON_COMMAND(ID_MY_PAINT, OnReceive)
//	ON_COMMAND(IDC_CAP_LIVE, OnChange1)
//	ON_COMMAND(IDC_CAP_NORMAL, OnChange)
	ON_COMMAND(IDC_CAP_KEYED, OnChange)
	ON_COMMAND(IDC_CAP_PREVIOUS, OnChange1)
	ON_COMMAND(IDC_CAP_GRAY, OnChange)
//	ON_COMMAND(IDC_CAP_WHITEZ, OnChange)
	ON_COMMAND(IDC_CAP_KEYED, OnChange)
	ON_COMMAND(IDC_CAP_OPTIONS, OnCapOptions)
	ON_COMMAND(IDC_CAP_KEY_OPT, OnCapColorKey)
	ON_COMMAND(IDC_CAP_SAVE, OnSaveSettings)
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_FRAME, OnChangeCapFrame)
	ON_EN_CHANGE(IDC_LEVEL, OnChangeCapLevel)
	ON_EN_CHANGE(IDC_HOLD,  OnChangeCapHold)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCapDlg message handlers

UINT VideoCallback(FBDataKind Code, void * pvData, int size, void * pClass)
{
	BYTE * pData = (BYTE *)pvData;
	PCAPINFO p = (PCAPINFO)pClass;//m_pInfo;
	if ((p->infStatus & 7) == STAT_EMPTY) // empty ?
		{
		int w,h,d;
		if (Code == kRawDataType)
			{
			FBVideoCaptureSize(w,h,d);
			}
		else if (Code == kBMPDataType)
			{
			LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)pData;
			w = lpBI->biWidth;
			h = lpBI->biHeight;
			d = lpBI->biBitCount;
			pData += lpBI->biSize;
			}
		else
			return 0;
		UINT dd,ss;
		if (d == 16)
			{
			dd = 24;
			ss = 3 * (size / 2);
			}
		else
			{
			ss = size;
			dd = d;
			}
		if (((int)p->Width != w) || ((int)p->Height != h) || 
					(p->Depth != dd) || (p->Size != ss)
					|| !p->pBI)
			{
			p->infStatus = STAT_NEW_SIZE;	// not ready 
			p->Width = w;		// need buffer adjustment
			p->Height = h;
			p->Depth = dd;
			p->Size = ss;
			}
		else
			{
//       	DPF("BufferCBSample size:%d", size);
			long i;
			BYTE * pbits = p->pBits;
			if (p->avg > 1)
				pbits += ss * (1 + (p->Counter++ % p->avg));
			if (d == 16)
				{
				WORD * pp = (WORD *)pData;
				long z = size / 2;
				for (i = 0; i < z;i++)
					{
					WORD v = *pp++;
					WORD r,g,b;
					r = (v / 1024) & 31;
					g = (v / 32) & 31;
					b = (v & 31);
					r = 33 * r / 4;
					g = 33 * g / 4;
					b = 33 * b / 4;
					pbits[3*i+0] = (BYTE)b;
					pbits[3*i+1] = (BYTE)g;
					pbits[3*i+2] = (BYTE)r;
					}
				}
			else
				{
				memcpy(pbits, pData, size);
				}
			if (p->avg>1)
				{
				UINT j,c;
				BYTE * pdst = p->pBits;
				BYTE * pavg = p->pBits + ss;
				c = p->avg;
				for (i = 0; i < (long)ss;i++)
					{
					UINT v,q;
					q = c * ss;
					for (v = 0,j = 0; j < q; j += ss)
						v += pavg[j+i];
					pdst[i] = v / c;
					}
				}
			p->infStatus = STAT_READY;
			}
		p->infParent->PostMessage(WM_COMMAND,ID_MY_PAINT,0);//myview.onmypaint
		}
	else
		{
      DPF("Busy,BufferCBSample,code:%d, size:%d", p->infStatus,size);
		}
    return S_OK;//E_NOTIMPL;
}

BOOL CCapDlg::OnInitDialog()
{
	m_bNeedCell = 1;
	m_pArgs->pParent = (CWnd *)this;
	m_bBozo = 0;
//	m_nAlign = m_pArgs->align;
//if (m_nAlign == 1) m_nAlign = 2;
	m_pArgs->process = 1; // live
	m_bPrev = m_pArgs->prev;
	m_gmode = m_pArgs->gmode;
	if (m_gmode > 1) m_gmode = 1;
//	whitek = m_pArgs->whitekey;
//	if (!level)
//		GetDlgItem(IDC_CAP_WHITEZ)->SetWindowText("Color");
	CMyDialog::OnInitDialog();
	frame++;
	maxframe++;
	m_nExistFlag = 1;
	m_frame.Create(this, IDC_FRAME, frame, 1, maxframe);
	m_level.Create(this, IDC_LEVEL, level, 0, maxlevel);
	m_skip.Create (this, IDC_HOLD,  hold, 1, 256);
//	whitek = m_pArgs->whitekey;
	white = m_pArgs->white;
//	if (m_gmode)
//		m_wslider.Setup(&m_whiteq, 6, whitek,1,255);
//	else
		m_wslider.Setup(&m_whiteq, 4, white,1,100);
	gamma = m_pArgs->gamma;
	m_gslider.Setup(&m_gamma, 5, gamma,1,100);
	int w = m_pInfo->Width;
	int h = m_pInfo->Height;
	m_pInfo->infParent= m_pParent;
//	int z = FBVideoCaptureBegin (VideoCallback, this, w, h);
	int z = FBVideoCaptureBegin (VideoCallback, m_pInfo, w, h);
	int q = FBVideoSetup(m_pParent->m_hWnd, this, m_szDevice);
DPF("z:%d,q:%d",z,q);
	if (!FBVideoCaptureDeviceCount())
		{
DPF("empty");
		PostMessage(WM_COMMAND, IDCANCEL,0);
		}
	m_pArgs->command = 1;	// start displying 
	m_pInfo->infStatus = STAT_EMPTY;
	UpdateControls();
	m_pKeyer = new CColorKey;
	//	SetWindowPos(&wndTopMost,winx,winy,0,0,SWP_NOSIZE|SWP_SHOWWINDOW);
#ifndef FLIPBOOK_MAC
	RECT rcDesk;
	RECT rcMe;
	GetDesktopWindow()->GetWindowRect(&rcDesk);
	GetWindowRect(&rcMe);
	if ((int)winx < rcDesk.left)
		winx = rcDesk.left;
	else if (((int)winx + rcMe.right - rcMe.left) > rcDesk.right)
		winx = rcDesk.right - rcMe.right + rcMe.left;
	if ((int)winy < rcDesk.top)
		winy = rcDesk.top;
	else if (((int)winy + rcMe.bottom - rcMe.top) > rcDesk.bottom)
		winy = rcDesk.bottom - rcMe.bottom + rcMe.top;
	SetWindowPos(&wndTop,winx,winy,0,0,SWP_NOSIZE|SWP_SHOWWINDOW);
#endif
	GetDlgItem(IDC_CAP_CAPTURE)->SetFocus();
	FBVideoCaptureActiveDeviceName(m_szDevice);
	UpdateDeviceInfo();
	GetCellImage();
	return FALSE;  // return TRUE  unless you set the focus to a control
}

void CCapDlg::OnChange()
{
	OnChange1();
	UpdateControls();
}

void CCapDlg::OnChange1() // no update of controls
{
	BOOL bprev = m_bPrev;
	UpdateData();
	int  zalign = 0;
	if (!level)
		{
		if (m_bPrev)
			zalign = 1;
		else
			zalign = 2;
		}
	if ((bprev != m_bPrev) || (zalign != m_pArgs->align))
		{
DPF("prev:%d",m_bPrev);
		m_pArgs->prev = m_bPrev;
		m_bNeedCell = TRUE;
		m_pArgs->align = zalign;
//		m_pArgs->process = 1;
		}
	m_pArgs->process = 1;//(level || m_bPrev) ? 1 : 0;
	m_pArgs->gmode = m_gmode; // gray or color
	m_pArgs->whitekey = whitek;
	if (m_bNeedCell)
		GetCellImage();
}

void CCapDlg::OnSaveSettings()
{
	RECT rect;
	GetWindowRect(&rect);
	winx = rect.left;
	winy = rect.top;
	UpdateData();	// get label contents
	white = m_pArgs->white;
//	gamma = m_pArgs->gamma;
	hold = m_skip.GetPos();
	CSketchDoc * pDoc = ((CSketchView *)m_pParent)->GetDocument();
	pDoc->OptionString(CAPTURE_DEVICE,m_szDevice,1);
	int opts =  0;//(1 + m_nAlign) & 3;
//		opts |=  (dlg.m_nAlign ) & 2;
//		opts |=  ((m_pArgs->align & 1) << 1);
	opts |=  ((m_pArgs->prev & 1) << 2);
	opts |=  ((m_pArgs->gmode & 3) << 4);
	opts |=  ((m_pArgs->rotate  & 1) << 6);
	opts |=  (((m_pArgs->flags ^ FLAG_ASPECT) & FLAG_ALL) << 7);
	opts |=  (m_pInfo->avg - 1) << 24;
	pDoc->Option(VIDCAP_OPTS,1,opts);
	pDoc->Option(VIDCAP_WHITE,1,m_pArgs->white);
	pDoc->Option(VIDCAP_GAMMA,1,m_pArgs->gamma);// + 20000*m_pArgs->whitekey);
	pDoc->Option(DEF_HOLD,1,hold);
	pDoc->Option(VIDCAP_X,1,winx);
	pDoc->Option(VIDCAP_Y,1,winy);
	pDoc->Option(VIDCAP_RED,1,m_pArgs->red);
	pDoc->Option(VIDCAP_GREEN,1,m_pArgs->green);
	pDoc->Option(VIDCAP_BLUE,1,m_pArgs->blue);
	pDoc->Option(VIDCAP_EXTENT,1,m_pArgs->extent);
	pDoc->Option(VIDCAP_WIDTH,1,m_pInfo->Width);
	pDoc->Option(VIDCAP_HEIGHT,1,m_pInfo->Height);
}



BOOL CCapDlg::PreTranslateMessage(MSG* pMsg) 
{
	WORD v = 0;
    if( pMsg->message == WM_KEYDOWN )
		{
//DPF("got key:%d",pMsg->wParam);
        if((pMsg->wParam >= '1') && ( pMsg->wParam <= '9') )
			v = pMsg->wParam & 15;
        else if((pMsg->wParam >= VK_NUMPAD1) && ( pMsg->wParam <= VK_NUMPAD9) )
			v = pMsg->wParam - (VK_NUMPAD1 - 1);
#ifdef ALLOW_AUTO
		else if (pMsg->wParam == 27)
			m_nAuto = 0;
		else if (pMsg->wParam == VK_F2)
			{
			m_skip.SetPos(1);
			m_nAuto = 1000;
			v = 1;
			}
#endif
		else if (pMsg->wParam == ' ')
			v = 99;
		else if (m_pArgs->flags & FLAG_CURSOR)
			{
			if ((pMsg->wParam == VK_DOWN) || (pMsg->wParam == VK_UP))
			{
			int frame = m_frame.GetPos();
			int newframe;
			if (pMsg->wParam == VK_DOWN)
				newframe = frame + 1;
			else
				newframe = frame - 1;
			if ((newframe > 0) && (newframe <= (int)maxframe))
				{
				m_frame.SetPos(newframe);
				PostMessage(EN_CHANGE, IDC_FRAME,0);
				}
			return TRUE;
			}
			if ((pMsg->wParam == VK_RIGHT) || (pMsg->wParam == VK_LEFT))
			{
			int level = m_level.GetPos();
			int newlevel;
			if (pMsg->wParam == VK_RIGHT)
				newlevel = level - 1;
			else
				newlevel = level + 1;
			if ((newlevel >= 0) && (newlevel <= (int)maxlevel))
				{
				m_level.SetPos(newlevel);
				PostMessage(EN_CHANGE, IDC_LEVEL,0);
				}
			return TRUE;
			}
			if ((pMsg->wParam == VK_END))
				{
				int frame = m_frame.GetPos();
	((CSketchView *)m_pParent)->CapKludge(frame);
				return TRUE;
				}
			}
		}
	if (v)
		{
		CWnd * pw = GetFocus();
		if ((pw == GetDlgItem(IDC_LEVEL)) ||
			(pw == GetDlgItem(IDC_FRAME)) ||
			(pw == GetDlgItem(IDC_LABEL)) ||
			(pw == GetDlgItem(IDC_HOLD)))
			{
DPF("level set");
			v = 0;
			}
		}
	if (v)
		{
DPF("v:%d",v);
		if (v != 99)
			m_skip.SetPos(v);
		PostMessage(WM_COMMAND, IDC_CAP_CAPTURE,0);
		return TRUE;
		}
    return CMyDialog::PreTranslateMessage(pMsg);
}	




void CCapDlg::OnCapOptions()
{
	CCapOptDlg dlg(this);
	if (GetKeyState(VK_CONTROL) < 0)
		m_pArgs->flags |= FLAG_SECRET;
	dlg.m_pArgs = m_pArgs;
	dlg.m_pInfo = m_pInfo;
	BOOL bRotate = m_pArgs->rotate;
	BOOL bAspect = m_pArgs->flags & FLAG_ASPECT;
//	BOOL bSheet = m_pArgs->flags & FLAG_CURSOR;
//	BOOL bWV = m_pArgs->flags & FLAG_WV1;
	BOOL bCrop= m_pArgs->flags & FLAG_CROP;
//	BOOL bOverlay = m_pArgs->overlay & 1;
	UINT avg = m_pInfo->avg;
	dlg.m_bBozo = m_bBozo;
//	dlg.m_bNumbers = m_bNumbers;
//	dlg.m_bColorKey = m_bColorKey; // can do color key
//	dlg.m_bOverlay = m_oldlevel ? 1 : 0;
	strcpy(dlg.m_szDevice, m_szDevice);
	int res = dlg.DoModal();
	if (res == IDOK)
		{
		m_bBozo = dlg.m_bBozo;
//		m_bNumbers = dlg.m_bNumbers;
//		m_bColorKey = dlg.m_bColorKey;
		strcpy(m_szDevice, dlg.m_szDevice);
		UpdateDeviceInfo();
		}
	else
		{
		m_pInfo->avg = avg;
		m_pArgs->rotate = bRotate;
		m_pArgs->flags |= (FLAG_ASPECT|FLAG_CURSOR|FLAG_WV1|FLAG_CROP);
		m_pArgs->flags ^= (FLAG_ASPECT|FLAG_CURSOR|FLAG_WV1|FLAG_CROP);
		if (bAspect) m_pArgs->flags |= FLAG_ASPECT;
		m_pArgs->flags |= FLAG_CURSOR;
		m_pArgs->flags |= FLAG_WV1;
		if (bCrop) m_pArgs->flags |= FLAG_CROP;
	//	m_pArgs->overlay &= 1;
	//	if (bOverlay)
	//		m_pArgs->overlay |= 1;
		if (strcmp(m_szDevice, dlg.m_szDevice))
			{
			int c = FBVideoCaptureDeviceCount();
			int i;
			for (i = 0; i < c;i++)
				{
				FBVideoCaptureDeviceName (dlg.m_szDevice, i);
				if (strcmp(m_szDevice, dlg.m_szDevice))
					break;
				}
			if (i < c)
				FBVideoCaptureSelectDevice (i);
					
			}
		}
}

void CCapDlg::UpdateDeviceInfo()
{
	char buf[80];
	int w, h,d;
	FBVideoCaptureSize(w,h,d);
	sprintf(buf, "%d x %d (%d)", w,h,d);
	GetDlgItem(IDC_CAP_RES)->SetWindowText(buf);
	GetDlgItem(IDC_CAP_DEVICE)->SetWindowText(m_szDevice);
}

BOOL CCapDlg::GetLevel0(UINT frame, BOOL bHold, BOOL bPrev)
{
	DWORD key = 0;
	CSketchView * pView = (CSketchView *)m_pParent;
	CSketchDoc * pDoc = pView->GetDocument();
	CScene  * pScene = pDoc->GetDocScene();
	UINT w = pView->Width();
	UINT h = pView->Height();
	UINT p = pView->Pitch();
	BYTE *pp = pView->CapBuffer() + h * p;
	if (!bHold || level)
		frame++;
	else
		{
		if (!bPrev)
			frame = 0; 
		}
	for (;frame;)
		{
		frame--;
		pScene->GetImageKey(key, frame, 0,CCell::LAYER_BG);
		if (key || !bHold || !frame)
			break;
		}
	if (!key)
		{
		if (bHold)
			{
			memset(pp,255,h*p);
			}
		return 0;
		}
	UINT iw, ih, id;
	if (pScene->ImageInfo(iw,ih,id, key))
		{
//DPZ("bad info");
		return 0;
		}
//DPZ("info iw:%d,ih:%d,id:%d",iw,ih,id);
	if (iw > 8192)
		return 1;
	if (id != 24)
		return 1;
	UINT tpp = 4 * ((3 + 3 * iw) / 4);
	BYTE * tp = new BYTE[ih * tpp];
	pScene->ReadImage(tp,key);
	UINT x, y;
	if (w > iw)
		w = iw;
	if (h > ih)
		h = ih;
	for (y = 0; y < h; y++)
		for (x = 0; x < w; x++)
			{
			if (pScene->ColorMode())
				{
				pp[y*p+3*x+0] = tp[y*tpp+3*x+0];
				pp[y*p+3*x+1] = tp[y*tpp+3*x+1];
				pp[y*p+3*x+2] = tp[y*tpp+3*x+2];
				}
			else
				{
				UINT v = tp[y*tpp+3*x+0]+tp[y*tpp+3*x+1]+tp[y*tpp+3*x+2];
				pp[y*p+x] = v / 3;
				}
			}
	delete [] tp;
	return 1;
}


BOOL CCapDlg::MyApply(UINT frame, BOOL bHold)
{
	CSketchView * pView = (CSketchView *)m_pParent;
	CSketchDoc * pDoc = pView->GetDocument();
	CScene  * pScene = pDoc->GetDocScene();
	UINT w = pView->Width();
	UINT h = pView->Height();
	UINT p = pView->Pitch();
	BYTE *pp = pView->CapBuffer() + h * p;
	if (bHold)	// bg and / or prev
		{
//		if (!m_pArgs->process || !m_pArgs->align)	// live or normal
//			{
//			memset(pp,255,h*p);
//			return 0;
//			}
		UINT ff = frame;
		if (!pDoc->Option(PEG_SHOWBG))
			memset(pp,255,h*p);
		else
			{
			GetLevel0(ff, 1,1);// bhold will dec
DPF("mi,prev:%d,frm:%d,ff:%d",m_bPrev,frame,ff);
			}
		if (!m_bPrev || !frame)
			return 1;
		frame--;
		}
	BOOL bRes = 0;
	UINT d = pScene->ColorMode() ? 4 : 1;
	BYTE * tp = new BYTE[d * w * h];
	UINT cw, ch,ck;
	if (pScene->CellInfo(tp,frame, level,bHold,cw,ch,ck))
		{
		UINT x, y;
		for (y = 0; y < h; y++)
			for (x = 0; x < w; x++)
				{
				if (pScene->ColorMode())
					{
					UINT v = tp[y*4*w+4*x+3];
					int j;
					for (j = 0; j < 3; j++)
						pp[y*p+3*x+j] =
							(v * tp[y*4*w+4*x+j] + pp[y*p+3*x+j] * (255 - v))
									/ 255;
					}
				else
					{
					UINT v = 255 ^ tp[y*w+x];
					UINT q = pp[y*p+x];
					pp[y*p+x] = (v * q) / 255;
					}
				}
		bRes = 1;
		}
	delete [] tp;
	return bRes;
}

void CCapDlg::GetCellImage()
{
	m_bNeedCell = 0;
DPF("ci,prev:%d",m_bPrev);
	UINT frame = m_frame.GetPos()-1;
	if (!level)
		{
		if (!frame)
			m_pArgs->align = 2;
		if (frame && m_bPrev)
			{
			if (!GetLevel0(frame, 1, m_bPrev)) // bhold will dec	
				{
				m_pArgs->align = 2;
				}
			}
		if (m_pArgs->flags & FLAG_WV1)
			{
			m_pArgs->flags |= FLAG_WV2;
			m_pArgs->flags ^= FLAG_WV2;
			if (GetLevel0(frame, 0,0))	
				m_pArgs->flags |= FLAG_WV2;
			}
		}
	else
		{
		MyApply(frame, 1);
		if (m_pArgs->flags & FLAG_WV1)
			{
			m_pArgs->flags |= FLAG_WV2;
			m_pArgs->flags ^= FLAG_WV2;
			if (MyApply(frame, 0))	
				m_pArgs->flags |= FLAG_WV2;
			}
		}
}

void CCapDlg::OnChangeCapFrame() 
{
	m_frame.Change();
	frame = m_frame.GetPos();
	if (frame > maxframe) m_frame.SetPos(maxframe);
//	GetCellImage();
DPF("frame:%d,lvl:%d",frame, level);
	((CSketchView *)m_pParent)->SetSelection(frame-1, level);
	m_bNeedCell = 1;
	OnChange();
//	UpdateControls();
}

void CCapDlg::OnChangeCapLevel() 
{
	BOOL oldBG = level ? FALSE : TRUE;
	m_level.Change();
	level = m_level.GetPos();
	if (level > maxlevel)
		m_level.SetPos(maxlevel);
	BOOL newBG = level ? FALSE : TRUE;
DPF("chg lvl,old:%d,new:%d",oldBG,newBG);
	if (oldBG != newBG)
		{
	//	if (m_oldlevel)
	//		GetDlgItem(IDC_CAP_WHITEZ)->SetWindowText("White");
	//	else
	//		GetDlgItem(IDC_CAP_WHITEZ)->SetWindowText("Color");
		UpdateControls();
		}
	m_bNeedCell = 1;
	OnChange();
	((CSketchView *)m_pParent)->SetSelection(frame-1, level);
}

void CCapDlg::OnChangeCapHold() 
{
	m_skip.Change();
	hold = m_skip.GetPos();
}

void CCapDlg::OnCapColorKey()
{
	DPF("color key dlg");
	CColorKeyDlg dlg;
	UINT red = m_pArgs->red;
	UINT green = m_pArgs->green;
	UINT blue = m_pArgs->blue;
	UINT extent = m_pArgs->extent;
	UINT flags = (m_pArgs->flags >> 6) & 7;
	dlg.m_pArgs = m_pArgs;
	int result = dlg.DoModal();
	m_pArgs->pKeyDlg = 0;
	if (result != IDOK)
		{
DPF("cancelling");
		m_pArgs->red = red;
		m_pArgs->green = green;
		m_pArgs->blue = blue;
		m_pArgs->extent = extent;
		m_pArgs->flags |= 64 + 128 + 256;
		m_pArgs->flags ^= 64 + 128 + 256;
		m_pArgs->flags |= (flags << 6);
		}
}

void CCapDlg::UpdateControls()
{
	BOOL bRadio;
	BOOL bGray;
	BOOL bKey = FALSE;
	BOOL bWhite = 0;
//	BOOL bWhitez = 0;
	if (level)
		{
		bRadio = TRUE;
		if (!m_pArgs->color)
			{
			m_pArgs->gmode = m_gmode = 0;
			CheckRadioButton(IDC_CAP_GRAY,IDC_CAP_KEYED,IDC_CAP_GRAY+m_gmode);	//forcegray
			}
		bGray = m_gmode ? 0 : 1;
		bWhite = bGray;//m_mode == 2 ? 0 : 1;
		}		
	else
		{
		bRadio = FALSE;
		bGray = 0;
//		m_pArgs->gmode = m_gmode = 1;
//		CheckRadioButton(IDC_CAP_GRAY,IDC_CAP_KEYED,IDC_CAP_GRAY+1);	// force color
	//	if (m_pArgs->align == 1)
	//		{
	//		m_pArgs->align = 2; // force alternate
	//		}
		GetDlgItem(IDC_CAP_KEYED)->EnableWindow(0);
		GetDlgItem(IDC_CAP_GRAY)->EnableWindow(0);
//		bWhitez = 1;
		}
	GetDlgItem(IDC_CAP_KEYED)->EnableWindow(bRadio);
	GetDlgItem(IDC_CAP_GRAY)->EnableWindow(bRadio);
//	if (m_gmode)
//		m_wslider.Setup(&m_whiteq, 6, whitek,1,255);
//	else
		m_wslider.Setup(&m_whiteq, 4, white,1,100);
	//GetDlgItem(IDC_CAP_WHITEZ)->EnableWindow(bWhitez);
//	GetDlgItem(IDC_CAP_KEYED)->EnableWindow(bKey);
	GetDlgItem(IDC_CAP_KEY_OPT)->EnableWindow(bRadio && m_gmode);
	GetDlgItem(IDC_CAP_WHT_TXT)->EnableWindow(bWhite);
	GetDlgItem(IDC_CAP_WHITE)->EnableWindow(bWhite);
	GetDlgItem(IDC_CAP_WHTSLIDE)->EnableWindow(bWhite);
	GetDlgItem(IDC_CAP_GAM_TXT)->EnableWindow(bGray);
	GetDlgItem(IDC_CAP_GAMMA)->EnableWindow(bGray);
	GetDlgItem(IDC_CAP_GAMSLIDE)->EnableWindow(bGray);
}

WORD Receiver(PRECINFO pRec, PCAPPACKET pArgs, PCAPINFO pInfo)
{
	int rot = 0;
	if (pArgs->rotate)
		rot = 2;
	DPF("got dib,rot:%d",rot);
	LPBITMAPINFOHEADER  lpBI;
	BYTE * pTemp = 0;
	if (pRec->level && pArgs->gmode)
		{
		rot |= 128; // blur alpha
		pTemp = new BYTE[40 + pInfo->Height * 4 * pInfo->Width];
		lpBI = (LPBITMAPINFOHEADER)pTemp;
		lpBI->biBitCount		= 32;
		BYTE * pDst = pTemp + 40;
		BYTE * pSrc = pInfo->pBI + 40;
		pRec->pKeyer->Process(pDst, pSrc,
			pInfo->Width,pInfo->Height,4,
			pArgs->red, pArgs->green,pArgs->blue, pArgs->extent,
						(pArgs->flags >> 6) & 7);
		}
	else
		{
		if (pRec->level && pArgs->gmode)
			rot += 128 + (pArgs->whitekey << 16);
		lpBI = (LPBITMAPINFOHEADER)pInfo->pBI;
		lpBI->biBitCount		= 24;
		}
	lpBI->biSize			= sizeof(BITMAPINFOHEADER);
	lpBI->biWidth			= pInfo->Width;
	lpBI->biHeight			= pInfo->Height;
	lpBI->biPlanes			= 1;
//	lpBI->biBitCount		= 24;
	lpBI->biCompression		= BI_RGB;
	lpBI->biSizeImage		= 0;
	lpBI->biXPelsPerMeter	= 0;
	lpBI->biYPelsPerMeter	= 0;
	lpBI->biClrUsed			= 0;
	lpBI->biClrImportant	= 0;
	
	UINT nFit;
	if (pArgs->flags & FLAG_CROP)
		nFit = 4;
	else if (pArgs->flags & FLAG_ASPECT)
		nFit = 1;
	else
		nFit = 3;
	if (pRec->plabel)
	pRec->pDoc->m_pScene->ProcessCellLabel(*pRec->plabel, pRec->skip);
	WORD z = pRec->pDoc->CreateCell(pRec->frame,pRec->level,lpBI,rot,1,nFit);
	delete[] pTemp;
	pArgs->command = 1;
	pInfo->infStatus = STAT_EMPTY;
	return z;
}

void CCapDlg::OnReceive()
{
	DPF("receive");
	RECINFO rec;
	rec.frame = m_frame.GetPos()-1;
	rec.level = m_level.GetPos();
	rec.skip = m_skip.GetPos();
	rec.plabel = &m_label;
	rec.pKeyer = Keyer();
	rec.pDoc = ((CSketchView *)m_pParent)->GetDocument();
	UpdateData();	// get label contents
	WORD z = Receiver(&rec, m_pArgs, m_pInfo);
	if (z)
		{
		m_nAuto = 0;
		DPF("make cell error:%d",z);
		}
	else
		{
		if (maxframe < (UINT)(m_frame.GetPos() + m_skip.GetPos()))
			maxframe = m_frame.GetPos() + m_skip.GetPos();
		m_frame.SetRange(1,maxframe);
		m_frame.SetPos(m_frame.GetPos() + m_skip.GetPos());
		UpdateData(0);
		}
	if (!z)
PlaySound(TEXT("FINISHED"),AfxGetApp()->m_hInstance,SND_RESOURCE | SND_ASYNC);
	if (!m_nAuto)
		{
	GetDlgItem(IDC_CAP_CAPTURE)->EnableWindow( 1);
	GetDlgItem(IDC_CAP_CAPTURE)->SetFocus();
	((CSketchView *)m_pParent)->Invalidate();
		}
#ifdef ALLOW_AUTO
	if (m_nAuto)
		{
		m_nAuto--;
//		MSG msg;
//		while (::PeekMessage(&msg,NULL,0,0,PM_NOREMOVE) && m_nAuto)
//			{
//			if (!AfxGetThread()->PumpMessage())
//				return;
//			}
		PostMessage(WM_COMMAND, IDC_CAP_CAPTURE,0);
//		PostMessage(WM_KEYDOWN,49,0);
	((CSketchView *)m_pParent)->Invalidate();
		}
#endif
	m_bNeedCell = 1;
	PostMessage(WM_COMMAND, IDC_CAP_PREVIOUS,0);
//	OnChange();
}

void CCapDlg::OnCapture()
{
	DPF("capture,status:%d",m_pInfo->infStatus);
	CSketchView * pView = (CSketchView *)m_pParent;
	CSketchDoc * pDoc = pView->GetDocument();
	if (pDoc->ExistCheck(m_frame.GetPos()-1,level,1,m_nExistFlag))
		return;
	if (!m_nAuto)
		GetDlgItem(IDC_CAP_CAPTURE)->EnableWindow( 0);
	m_pArgs->command = 2;
PlaySound(TEXT("SHUTTER"),AfxGetApp()->m_hInstance,SND_RESOURCE | SND_ASYNC);
	((CSketchView *)m_pParent)->Invalidate();

}

void CCapDlg::OnHScroll( UINT nCode, UINT nPos, CScrollBar* pScrollBar )
{
	CMySlider * pSlider = (CMySlider *)pScrollBar;

	UINT pos = pSlider->GetPos();
	DPF("hscroll, code:%d, pos:%d,act:%d",nCode, nPos,pos);
	pSlider->UpdateBuddy();
	if (pSlider->m_argoffset == 4)
		{
		m_pArgs->white  = white = pos;//(100 * pos+ 127) / 255;
		}
	else if (pSlider->m_argoffset == 6)
		m_pArgs->whitekey  = whitek = pos;
	else
		{
		double x = pos;
		x /= 50;
		x += 1;
		x = exp(log(10.0) * x);
		m_pArgs->gamma  = gamma = (UINT)x;
		}
	OnChange1();
}

void CMySlider::Setup(CWnd * pBuddy, UINT offset, UINT pos, UINT min, UINT max)
{
	SetRange(min,max);
	m_pBuddy = pBuddy;	
	m_argoffset = offset;
	if (m_argoffset == 5)
		{
		double x = pos;
		x = log(x) / log(10.0);
		x = x - 1;
		x *= 50.0;
		pos = (UINT)(x + 0.5);
		}
	SetPos(pos);
	UpdateBuddy();
}

void CMySlider::Assign(UINT v)
{
	SetPos(v);
	UpdateBuddy();
}

void CMySlider::UpdateBuddy()
{
	char buf[10];
	int 	z = GetPos();
	if (m_argoffset == 5)
		{
		double x = z;
		x /= 50;
		x += 1;
		x = exp(log(10.0) * x);
		z  = (int)x;
		}
	sprintf(buf,"%d",z);
	m_pBuddy->SetWindowText(buf);
}


CGrabDlg::CGrabDlg(CWnd* pParent /*=NULL*/)
: CMyDialog(CGrabDlg::IDD, pParent)
{
	m_pParent = pParent;	
	m_pKeyer = 0;
}

CGrabDlg::~CGrabDlg()
{
	FBVideoCaptureStop(1);
	delete m_pKeyer;
}

BOOL CGrabDlg::DestroyWindow() 
{
	FBVideoCaptureStop(0);
    return CMyDialog::DestroyWindow();
}

BEGIN_MESSAGE_MAP(CGrabDlg, CMyDialog)
	//{{AFX_MSG_MAP(CGrabDlg)
	ON_COMMAND(IDC_CAP_CAPTURE, OnCapture)
	ON_COMMAND(ID_MY_PAINT, OnReceive)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGrabDlg message handlers

BOOL CGrabDlg::OnInitDialog()
{
	m_pArgs->pParent = (CWnd *)this;
	CMyDialog::OnInitDialog();
	frame++;
	int w = m_pInfo->Width;
	int h = m_pInfo->Height;
	m_pInfo->infParent= m_pParent;
	int z = FBVideoCaptureBegin (VideoCallback, m_pInfo, w, h);
	FBVideoSetup(m_pParent->m_hWnd, this, m_szDevice);
	if (!FBVideoCaptureDeviceCount())
		{
		PostMessage(WM_COMMAND, IDCANCEL,0);
		}
	m_pArgs->command = 1;	// start displying 
	m_pInfo->infStatus = STAT_EMPTY;
	m_pKeyer = new CColorKey;
	PostMessage(WM_COMMAND,IDC_CAP_CAPTURE,0);
	return FALSE;  // return TRUE  unless you set the focus to a control
}

void CGrabDlg::OnCancel() 
{
	DPF("on cancel");
	CMyDialog::OnCancel();
}

void CGrabDlg::OnReceive()
{
	DPF("grab receive");
	RECINFO rec;
	rec.frame = frame-1;
	rec.level = level;
	rec.skip = 0;
	rec.plabel = 0;
	rec.pKeyer = m_pKeyer;
	rec.pDoc = ((CSketchView *)m_pParent)->GetDocument();
	WORD z = Receiver(&rec, m_pArgs, m_pInfo);
	if (z)
		{
		DPF("make cell error:%d",z);
		}
	if (!z)
PlaySound(TEXT("FINISHED"),AfxGetApp()->m_hInstance,SND_RESOURCE | SND_ASYNC);
//	GetDlgItem(IDC_CAP_CAPTURE)->EnableWindow( 1);
	((CSketchView *)m_pParent)->Invalidate();
	EndDialog(IDOK);
}

void CGrabDlg::OnCapture()
{
	DPF("capture");
	if (!FBVideoCaptureDeviceCount())
		EndDialog(IDCANCEL);
//	GetDlgItem(IDC_CAP_CAPTURE)->EnableWindow( 0);
	m_pArgs->command = 2;
PlaySound(TEXT("SHUTTER"),AfxGetApp()->m_hInstance,SND_RESOURCE | SND_ASYNC);

}


CColorKeyDlg::CColorKeyDlg(CWnd* pParent /*=NULL*/)
: CMyDialog(CColorKeyDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CColorKeyDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
}

void CColorKeyDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_CK_RED, m_red);
	DDX_Text(pDX, IDC_CK_GREEN, m_green);
	DDX_Text(pDX, IDC_CK_BLUE, m_blue);
//	DDX_Text(pDX, IDC_CK_EXTENT, m_extent);
	DDX_Control(pDX, IDC_CK_EXTENT, m_extentxt);
	DDX_Control(pDX, IDC_CK_SLIDE,m_slider);
	DDX_Check(pDX, IDC_CK_INVERT,m_bInvert);
	DDX_Radio(pDX, IDC_CK_RGB,m_kind);
}

BEGIN_MESSAGE_MAP(CColorKeyDlg, CMyDialog)
	//{{AFX_MSG_MAP(CColorKeyDlg)
	ON_COMMAND(IDC_CK_SELECT, OnSelect)
	ON_COMMAND(IDC_CK_STOP, OnStop)
	ON_WM_HSCROLL()
	ON_COMMAND(IDC_CK_INVERT, OnChange)
	ON_COMMAND(IDC_CK_RGB, OnChange)
	ON_COMMAND(IDC_CK_YIQ, OnChange)
	ON_COMMAND(IDC_CK_HLS, OnChange)
	ON_COMMAND(IDC_CK_LAB, OnChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorKeyDlg message handlers

BOOL CColorKeyDlg::OnInitDialog()
{
	m_red = m_pArgs->red;
	m_green = m_pArgs->green;
	m_blue = m_pArgs->blue;
	m_extent = m_pArgs->extent;
	m_kind = (m_pArgs->flags >> 7) & 3;
	m_bInvert = (m_pArgs->flags >> 6) & 1;
	CMyDialog::OnInitDialog();
	
	m_slider.Setup(&m_extentxt, 4, m_extent,1,100);
	OnStop();//OnSelect();
//	UpdateData(0);
	return FALSE;  // return TRUE  unless you set the focus to a control
}

void CColorKeyDlg::EnableControls(BOOL bSelect)
{
	BOOL bEnable = !bSelect;
	GetDlgItem(IDC_CK_SELECT)->EnableWindow(bEnable);
	GetDlgItem(IDC_CK_STOP)->EnableWindow(!bEnable);
	GetDlgItem(IDC_CK_RED)->EnableWindow(bEnable);
	GetDlgItem(IDC_CK_GREEN)->EnableWindow(bEnable);
	GetDlgItem(IDC_CK_BLUE)->EnableWindow(bEnable);
//	GetDlgItem(IDOK)->SetFocus();
	GetDlgItem(IDC_CK_SLIDE)->SetFocus();
	m_pArgs->pKeyDlg = bSelect ? this : 0;
}

void CColorKeyDlg::OnSelect()
{
	EnableControls(1);
}

void CColorKeyDlg::OnStop()
{
	EnableControls(0);
}

void CColorKeyDlg::SetColor(BOOL bStop)
{
	m_red = m_pArgs->red;
	m_green = m_pArgs->green;
	m_blue = m_pArgs->blue;
	m_extent = m_pArgs->extent;
//	m_extent = m_pArgs->white;
	if (bStop)
		EnableControls(0);
	UpdateData(0);
}

void CColorKeyDlg::OnHScroll( UINT nCode, UINT nPos, CScrollBar* pScrollBar )
{
	CMySlider * pSlider = (CMySlider *)pScrollBar;
	pSlider->UpdateBuddy();

	UINT pos = pSlider->GetPos();
	m_pArgs->extent = m_extent  = pos;
}

void CColorKeyDlg::OnChange()
{
	UpdateData();
	UINT v = m_bInvert + (m_kind << 1);
DPF("change:%d",v);
	m_pArgs->flags |= 64 + 128 + 256;
	m_pArgs->flags ^= 64 + 128 + 256;
	m_pArgs->flags |= v << 6;
}


BOOL CColorKeyDlg::PreTranslateMessage(MSG* pMsg) 
{
	WORD v = 0;
	BOOL bReturn = FALSE;
    if( pMsg->message == WM_KEYDOWN )
		{
//DPF("got key:%d",pMsg->wParam);
		if (pMsg->wParam == VK_ADD)
			{
DPF("got plus");
			if (m_pArgs->extent < 100)
				m_slider.Assign(m_extent = ++m_pArgs->extent);
			bReturn = TRUE;
			}
		else if (pMsg->wParam == VK_SUBTRACT)
			{
DPF("got minus");
			if (m_pArgs->extent)
				m_slider.Assign(m_extent = --m_pArgs->extent);
			bReturn = TRUE;
			}
		else if (pMsg->wParam == VK_ESCAPE)
			{
			PostMessage(WM_COMMAND, IDCANCEL);
			bReturn = TRUE;
			}
		}
	if (bReturn)
		return bReturn;
    return CMyDialog::PreTranslateMessage(pMsg);
}	


BYTE * CSketchView::ScanPacket(BOOL bClear, BOOL bStart)
{
	if (bClear)
		{
		delete [] m_scanpacket.pBed;
		m_scanpacket.pBed = 0;
		delete [] m_scanpacket.pPage;
		m_scanpacket.pPage = 0;
		m_scanpacket.flags = bStart ? 2 : 0;
		Invalidate(1);
		return 0;
		}
	else
		{
		m_scanpacket.flags = 2;
		return (BYTE *) (&m_scanpacket);
		}
}

void CSketchView::ScanPaint(CDC * pDC)
{
	RECT rectClip;
	pDC->GetClipBox(&rectClip);
	BYTE * pBits;
	if (m_hdd && m_scanpacket.pBed && (m_scanpacket.flags & 1))
		pBits = m_scanpacket.pBed;
	else if (m_hdd && m_scanpacket.pPage && !(m_scanpacket.flags & 1))
		pBits = m_scanpacket.pPage;
	else
		{
		pDC->FillSolidRect(&rectClip, m_crBack);
DPF("cant paint");
		return;
		}
	UINT sw = m_nNum * m_swidth / m_nDen;
	UINT sh = m_nNum * m_sheight / m_nDen;
	if ((DWORD)rectClip.right > sw)
		{
		CRect rectOff(sw,rectClip.top,rectClip.right,rectClip.bottom);
		rectClip.right = sw;
		pDC->FillSolidRect(rectOff, m_crBack);
		}
	if ((DWORD)rectClip.bottom > sh)
		{
		CRect rectOff(rectClip.left,sh,rectClip.right,rectClip.bottom);
		rectClip.bottom = sh;
		pDC->FillSolidRect(rectOff, m_crBack);
		}
	RECT src;
	XlateRect(rectClip,src,1);
	
//	DPR("my paint clip ",&rectClip);
//	DPR("src",&src);
	BYTE * pDIB = pBits + 1064;
	DrawDibDraw(m_hdd,
                pDC->m_hDC,
				rectClip.left, rectClip.top, 
				rectClip.right - rectClip.left, rectClip.bottom - rectClip.top,
                (LPBITMAPINFOHEADER)pBits,	 pDIB,
				src.left, src.top, 
				src.right - src.left, src.bottom - src.top,
                0);
	return;
}


void CSketchView::OnGrab()
{
	DPF("file grab");
	if (CheckModified(TRUE))
		return;
	CGrabDlg dlg(this);
	dlg.m_pArgs = &m_packet;
	dlg.m_pInfo = &m_info;
	if (!m_pFrame->GetSelection(dlg.frame,dlg.level))
		{
		dlg.frame = CurrentFrame();
		dlg.level = CurrentLevel();
		}
	if (m_pDoc->ExistCheck(dlg.frame, dlg.level,1,m_nWarnIfExist))
		return;
	m_pDoc->OptionString(CAPTURE_DEVICE, dlg.m_szDevice);
	CapSetup();
	m_info.avg = 1;
	int nResponse = dlg.DoModal();
	m_packet.command = 0;
	delete [] m_info.pBI;
	m_info.pBI = 0;
	m_info.Size = 0;
	delete [] m_pTemp1;
	m_pTemp1 = 0;
	dlg.frame--;
	dlg.frame += m_pDoc->Option(DEF_HOLD);
	if (dlg.frame >= m_pScene->FrameCount())
		dlg.frame -= m_pDoc->Option(DEF_HOLD);
	m_pFrame->SetSelection(dlg.frame,dlg.level,dlg.frame,dlg.level);
}

void CSketchView::OnCapture()
{
	DPF("file test");
	if (CheckModified(TRUE))
		return;
	CCapDlg dlg(this);
	dlg.m_pArgs = &m_packet;
	dlg.m_pInfo = &m_info;
	if (!m_pFrame->GetSelection(dlg.frame,dlg.level))
		{
		dlg.frame = CurrentFrame();
		dlg.level = CurrentLevel();
		}
	dlg.maxlevel = m_pScene->LevelCount() - 1;
	dlg.maxframe = m_pScene->MaxFrameCount() - 1;
	dlg.hold = m_pDoc->Option(DEF_HOLD);
	dlg.winx = m_pDoc->Option(VIDCAP_X);
	dlg.winy = m_pDoc->Option(VIDCAP_Y);
	m_pDoc->OptionString(CAPTURE_DEVICE, dlg.m_szDevice);
	CapSetup();
	int nResponse = dlg.DoModal();
	m_packet.command = 0;
	delete [] m_info.pBI;
	m_info.pBI = 0;
	m_info.Size = 0;
	delete [] m_pTemp1;
	m_pTemp1 = 0;
	Invalidate(0);
}


void CSketchView::CapSetup()
{
	UINT size = m_spitch * m_sheight;
	m_pTemp1 = new BYTE[3 * size]; // for refe and will vinton
//	display from 0
//	merge from 1
//	wv from 2
//	memcpy(m_pTemp1 + size, m_pBits, size);
	m_info.infStatus = STAT_DEAD;
	m_packet.command = 0;
	m_info.Width = m_pDoc->Option(VIDCAP_WIDTH);
	m_info.Height = m_pDoc->Option(VIDCAP_HEIGHT);
	m_packet.red = m_pDoc->Option(VIDCAP_RED);
	m_packet.green = m_pDoc->Option(VIDCAP_GREEN);
	m_packet.blue = m_pDoc->Option(VIDCAP_BLUE);
	m_packet.extent= m_pDoc->Option(VIDCAP_EXTENT);
	if (m_packet.extent > 100) m_packet.extent = 100;
	m_packet.white = m_pDoc->Option(VIDCAP_WHITE);
	if (m_packet.white > 100) m_packet.white = 100;
	m_packet.gamma = m_pDoc->Option(VIDCAP_GAMMA) % 20000;
	m_packet.whitekey = 0;//m_pDoc->Option(VIDCAP_GAMMA) / 20000;
	if (m_packet.whitekey > 255) m_packet.whitekey = 255;
	int opts = m_pDoc->Option(VIDCAP_OPTS);
//	m_packet.process = 1 + (opts & 1);
//	m_packet.align = (opts >> 1) & 1;
	m_packet.align = opts & 3;
	if (m_packet.align) m_packet.align--;
	if (m_packet.align == 1) m_packet.align = 2;
	m_packet.prev = (opts >> 2) & 1;
//	if (!m_packet.show)
//		m_packet.flags |= FLAG_WV1;
// extra bit here
	m_packet.gmode = (opts >> 4) & 3;
	if (!(m_packet.color = m_pScene->ColorMode()))
		m_packet.gmode = 0;
	m_packet.rotate =  (opts >> 6) & 1;
	m_packet.flags =  (opts >> 7) & FLAG_ALL;
	m_info.avg = 1 + (opts >> 24) & 3;	// extra flag bis
	m_packet.flags |= FLAG_WV1;
	m_packet.flags |= FLAG_CURSOR;
	m_packet.flags ^= FLAG_ASPECT;
	BuildTable();
}

CCapOptDlg::CCapOptDlg(CWnd* pParent /*=NULL*/)
: CMyDialog(CCapOptDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCapOptDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_pParent = pParent;	
}


void CCapOptDlg::DoDataExchange(CDataExchange* pDX)
{
	CMyDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCapOptDlg)
	DDX_Control(pDX, IDC_CAP_DEVICES, m_devices);
	DDX_Control(pDX, IDC_CAP_DEV_OPT, m_options);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCapOptDlg, CMyDialog)
	//{{AFX_MSG_MAP(CCapOptDlg)
	ON_COMMAND(IDC_CAP_ROTATE, OnCapRotate)
	ON_COMMAND(IDC_CAP_ASPECT, OnCapAspect)
	ON_COMMAND(IDC_CAP_CROP, OnCapCrop)
	ON_COMMAND(IDC_CAP_BOZO, OnCapBozo)
	ON_CBN_SELCHANGE(IDC_CAP_DEVICES, OnSelChangeDevice)
	ON_LBN_SELCHANGE(IDC_CAP_DEV_OPT, OnSelChangeOption)
	ON_EN_CHANGE(IDC_AVG, OnChangeAvg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCapOptDlg message handlers


void CCapOptDlg::FillCombos()
{
	m_devices.ResetContent();
	int c = FBVideoCaptureDeviceCount();
	int i,j;
	for (i = 0, j = 0; i < c; i++)
		{
		char dev[100];
		FBVideoCaptureDeviceName (dev, i);
		m_devices.AddString(dev);
		if (!strcmp(dev,m_szDevice))
			j = i;
		}
	m_devices.SetCurSel(j);

	m_options.ResetContent();
	for (i = 0, j = 0;i < 20 ; i++)
		{
		char opt[100];
		if (FBVideoCaptureOption(i,opt))
			break;
		m_options.AddString(opt);
		}
	m_options.SetCurSel(-1);

}

BOOL CCapOptDlg::OnInitDialog()
{
	CMyDialog::OnInitDialog();
	if (m_pArgs->flags & FLAG_SECRET)
		GetDlgItem(IDC_CAP_CROP)->ShowWindow(SW_SHOW);
	CheckDlgButton(IDC_CAP_ROTATE, m_pArgs->rotate & 1 ? 1 : 0);
	CheckDlgButton(IDC_CAP_ASPECT, m_pArgs->flags  & FLAG_ASPECT ? 1 : 0);
	CheckDlgButton(IDC_CAP_CROP  , m_pArgs->flags  & FLAG_CROP ? 1 : 0);
	CheckDlgButton(IDC_CAP_BOZO,   m_bBozo);
	GetDlgItem(IDC_CAP_ASPECT)->EnableWindow(m_pArgs->flags&FLAG_CROP ? 0 :1);
	FillCombos();
	m_spnavg.Create(this, IDC_AVG, m_pInfo->avg, 1, 8);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CCapOptDlg::OnSelChangeDevice()
{
	int index = m_devices.GetCurSel();
	FBVideoCaptureSelectDevice (index);
	FBVideoCaptureActiveDeviceName (m_szDevice);
	FillCombos();
}

void CCapOptDlg::OnSelChangeOption()
{
	int sel = m_options.GetCurSel();
	if (sel >= 0)
		FBVideoCaptureOption(sel + (m_bBozo ? 256 : 0));
	m_options.SetCurSel(-1); // turn off since it is a command
}

void CCapOptDlg::OnChangeAvg()
{
	m_spnavg.Change();
	UINT avg = m_spnavg.GetPos();
	if (avg > 8)
		avg = 8;
	if (avg < 1)
		avg = 1;
	m_pInfo->avg = avg;
}

void CCapOptDlg::OnCapRotate()
{
	m_pArgs->rotate ^= 1;
}


void CCapOptDlg::OnCapAspect()
{
	m_pArgs->flags ^= FLAG_ASPECT;
}

void CCapOptDlg::OnCapCrop()
{
	m_pArgs->flags ^= FLAG_CROP;
	GetDlgItem(IDC_CAP_ASPECT)->EnableWindow(m_pArgs->flags&FLAG_CROP ? 0 :1);
}
void CCapOptDlg::OnCapAlternate()
{
//	m_pArgs->flags ^= FLAG_CROP;
//	GetDlgItem(IDC_CAP_ASPECT)->EnableWindow(m_pArgs->flags&FLAG_CROP ? 0 :1);
}

void CCapOptDlg::OnCapBozo()
{
DPF("on bozo");
	m_bBozo ^= 1;
}
/*
void CCapOptDlg::OnCapNumbers()
{
DPF("on numbers");
	m_bNumbers ^= 1;
}
*/

void white_key(BYTE * dstp, BYTE * srcp, UINT w,UINT h,UINT d,UINT white)
{
	UINT p = 4 * ((d * w + 3) / 4);

	UINT x,y;
	for (y = 0; y < h; y++)
		{
		if (d == 1)
			{
DPF("aligning gray:%d",white);
			for (x = 0; x < w; x++)
				{
				if (dstp[x] >= 255)
					dstp[x] = srcp[x];
				}
			}
		else
			{
DPF("aligning white:%d",white);
			for (x = 0; x < w; x++)
				{
				if ((dstp[3*x+0] >= 255) &&
					(dstp[3*x+1] >= 255) &&
					(dstp[3*x+2] >= 255))
					{
					dstp[3*x+0] = srcp[3*x+0];
					dstp[3*x+1] = srcp[3*x+1];
					dstp[3*x+2] = srcp[3*x+2];
					}
				}
			}
		srcp += p;
		dstp += p;
		}
}

void onion_skin(BYTE * dstp, BYTE * srcp, UINT w,UINT h,UINT d)
{
	UINT p = 4 * ((d * w + 3) / 4);

	UINT x,y;
	for (y = 0; y < h; y++)
		{
		if (d == 1)
			{
			for (x = 0; x < w; x++)
				{
				UINT v = 2 * dstp[x] + srcp[x];
				dstp[x] = v / 3;
				}
			}
		else
			{
			for (x = 0; x < w; x++)
				{
				UINT v = 2 * dstp[3*x+0] + srcp[3*x+0];
				dstp[3*x+0] = v / 3;
				v = 2 * dstp[3*x+1] + srcp[3*x+1];
				dstp[3*x+1] = v / 3;
				v = 2 * dstp[3*x+2] + srcp[3*x+2];
				dstp[3*x+2] = v / 3;
				}
			}
		srcp += p;
		dstp += p;
		}
}
