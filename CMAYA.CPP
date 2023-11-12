#include "stdafx.h"
#include "sketch.h"
#include "resource.h"
#include "cmaya.h"
#include "mydoc.h"
#include "cscene.h"
#include <windowsx.h>
#include "dib.h"
#include "utils.h"
//#include <afxsock.h>		// MFC socket extensions

#include <winsock2.h>



CMaya::CMaya()
{
	m_count = 0;
	m_frames = 0;
	m_levels = 0;
}

CMaya::~CMaya()
{
}


int CMaya::DoFile(UINT Index, CSketchDoc * pDoc)
{
	CFile file;
//	sprintf(msg,"opening:%s",m_names[Index]);
//	AfxMessageBox(msg);

	if (!file.Open(m_names[Index],
      CFile::modeRead | CFile::shareDenyWrite))
   		{
		FormattedMsg(IDS_ERR_NO_OPEN,m_names[Index]);
		return 1;
		}
	BYTE hdr[18];
	int res;
	for (;;)
	{
	res = 2;
	if (file.Read((LPSTR)&hdr, sizeof(hdr)) != sizeof(hdr))
		break;
	res++;
	if (hdr[0] || hdr[1])
		break;
	UINT k = hdr[2];
	UINT d = hdr[16] & 255;
	UINT w = hdr[12] + 256 * hdr[13];
	UINT h = hdr[14] + 256 * hdr[15];
	res++;
	if ((k != 2) && (k != 10) && (k != 3))
		break;
	res++;
	if ((d != 24) && (d != 32))
		break;
	if (!pDoc)
		{
		if (w > m_width) m_width = w;
		if (h > m_height) m_height = h;
		res = 0;
		break;
		}
	res++;
	UINT op = 4 * ((d * w + 31) / 32);
	DWORD dwLength = op * h;
//	if (dwLength > 16000000)
//		return 0;
	UINT isize = sizeof(BITMAPINFOHEADER)+dwLength;
	if (d == 8)
		isize += 1024;
	LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)MyAlloc(isize);
	if (lpBI == 0)
		return res;
	res++;
	lpBI->biSize			= sizeof(BITMAPINFOHEADER);
	lpBI->biWidth			= w;
	lpBI->biHeight			= h;
	lpBI->biPlanes			= 1;
	lpBI->biBitCount		= d;
	lpBI->biCompression		= BI_RGB;
	lpBI->biSizeImage		= dwLength;
	lpBI->biXPelsPerMeter	= 0;
	lpBI->biYPelsPerMeter	= 0;
	lpBI->biClrUsed			= d > 8 ? 0 : 256;
	lpBI->biClrImportant	= 0;
	BYTE * pBits = (BYTE *)lpBI;
	pBits += lpBI->biSize;
	if (d == 8)
		{
		int i;
		for (i = 0; i < 256; i++)
			{
			*pBits++ = i;
			*pBits++ = i;
			*pBits++ = i;
			*pBits++ = 0;
			}
		}
	BYTE * p = pBits;
	UINT depth = d / 8;
	UINT ip = depth * w;
	UINT y;
	for (y = 0; y < h; y++)
		{
		if ((k == 2) || (k == 3))
			{
			if (file.Read(p, ip) != ip)
				break;
			}	
		else
			{
			BYTE * pp = p;
			UINT x;
			for (x = 0;x < w;)
				{
				BYTE code;
				file.Read(&code, 1);
				if (code & 128)
					{
					code = code & 127;
					if ((x + code) > w)
						code = w - x;
					BYTE * zp = pp;
					file.Read(zp, depth);
					pp += depth;
					x++;
					if ((x + code) > w)
						code = w - x;
					x += code;
					for (;code--; pp+=depth)
						memcpy(pp,zp,depth);
					}
				else
					{
					code = 1 + (code & 127);
					file.Read(pp, depth * code);
					x += code;
					pp += depth * code;
					}
				}
			}
		p += op;
		}
	if (y < h)
		res = 8;
	else
		{
		UINT code = 0;
		WORD z = pDoc->CreateCell(m_index[Index]-1,0,lpBI,code,0,0);
		if (z)
			res = 9;
		res = 0;
		}
	MyFree(lpBI);
	break;
	}
	file.Close();
	return res;
}

int CMaya::Init(LPCSTR cname)
{
	m_width = 0;
	m_height = 0;
	m_frames = 0;
	m_levels = 0;
	char buf[300];
//	char msg[300];
	strcpy(m_name,cname);
	FILE * stream;
	sprintf(buf,"name:%s|",m_name);
//	AfxMessageBox(buf);
	stream = fopen(m_name,"r");
	if (stream == NULL)
		return 1;
//	AfxMessageBox("opened");
	m_count = 0;
	int res = 0;
	m_rate = 24;
	BOOL bFirst = 1;
	for (;!res && (m_count < MAX_MAYA_FRAMES);)
		{
		if (!fgets(buf,sizeof(buf)-1,stream))
			break;
		if (buf[0] == '#')
			continue;
//		sprintf(msg,"line:%s",buf);
//		AfxMessageBox(msg);
		if (bFirst)
			{
			int a,b,c,d;
			int v = sscanf(buf,"0:%d.%d.%d.%d,%d,%s",
						&a,&b,&c,&d,&m_port,&m_cmd);
			if (v != 6)
				{
				res = 6;
				v = sscanf(buf,"0:%d.%d.%d.%d,%d,%d,%s",
						&a,&b,&c,&d,&m_port,&m_rate,&m_cmd);
				if (v != 7)
					res = 7;
				else
					res = 0;
				}
			if (!res)
				sprintf(m_host,"%d.%d.%d.%d",a,b,c,d);
			bFirst = 0;
//			sprintf(msg,"host:%s",m_host);
//			AfxMessageBox(msg);
			}
		else
			{
			UINT frame;
			char iname[300];
			int i,j;
			frame = 0;
			for (i = 0; buf[i] && buf[i] != ':';i++)
				frame = 10 * frame + (buf[i] & 15);
			i++; // over :
			int a = 0;
			int b = 0;
			for (j = 0; iname[j] = buf[i++];j++) // ugly I know
				if (iname[j] == '.')
					{
					a = b;
					b = j;
					}
			int z = 0;
			if (a && b)
				{
				for (i = a+1; i < b; i++)
					z = 10* z+ (iname[i] & 15);
				}
			char q[10];
			iname[a] = 0;
			sprintf(q,".%04d.tga",z);
			strcat(iname,q);
			strcpy(m_names[m_count],iname);
			m_index[m_count] = frame;
			if (frame > m_frames)
				m_frames = frame;
			m_count++;
//sprintf(msg,"res:%d,f:%d,n:%s",res,frame,iname);
//			AfxMessageBox(msg);
			}
		}
	fclose(stream);
	if (res)
		return res;
	m_levels = 2;
	int i;
	if (m_count >= MAX_MAYA_FRAMES)
		AfxMessageBox("Too Many Keys");

	for (i = 0; i < m_count; i++)
		{
		res = DoFile(i,0);
		if (res)
			{
			char msg[300];
			sprintf(msg,"error:%d:%s",res,m_names[i]);
			AfxMessageBox(msg);
			break;
			}
		}
	if (!res)
		{
		if (!m_width && !m_count)
			{
			m_width = 256;
			m_height = 256;
			m_rate = 24;
			m_frames = 10;
			AfxMessageBox("No TGA files");
			}
		}
	return res;
		
}

void CMaya::Info(UINT &w, UINT &h, UINT &n, UINT &l)
{
	w = m_width;
	h = m_height;
	n = m_frames;
	l = m_levels;
}

int CMaya::ExportFile(CScene * pScene, char * path,UINT Frame, UINT Level)
{
	CDib dib;
	BOOL bColor = pScene->ColorMode();
	UINT ww = pScene->Width();
	UINT hh = pScene->Height();
	dib.Create(ww,hh,bColor ? 32 : 8);
	pScene->FetchCell(dib.m_pBits, Frame, Level, 1,0,0);
	if (!bColor)
		dib.Convert(32);
//DPF("filename:%s",fileName);
	char name[300];
	sprintf(name,"%sframe%d.tga",path,Frame+1);
	CFile file;
	if (!file.Open(name, CFile::modeCreate | CFile::modeWrite))
		return 15;
	BOOL bCompressed = 0;
	BYTE hdr[18];
	DWORD dwBytesSaved = 0;
	BYTE * dst;
	BYTE * src;
	src = dib.m_pBits;
	LPBITMAPINFOHEADER lpBI = (LPBITMAPINFOHEADER)dib.m_pBMI;
//	src += lpBI->biSize;
	UINT w = lpBI->biWidth;
	UINT h = lpBI->biHeight;
	UINT dd = lpBI->biBitCount;
	UINT ip = 4 * ((w * dd + 31) / 32);
	UINT d = (dd + 7) / 8;
//	if ((d != 3) && (d!=4)) return kBadInfo;
	UINT op = d * w;
	DWORD dsize = 20 + h * op * 2;
	dst = new BYTE[dsize];
	BYTE * dptr = dst;
	if (!dst)
		{
		file.Close();
		return 17;
		}
	DWORD y;
//	UINT x;
	UINT bpp = d;
	hdr[0] = 0;
	hdr[1] = 0;
	hdr[2] = bCompressed ? 10 : 2;
	hdr[3] = 0;
	hdr[4] = 0;
	hdr[5] = 0;
	hdr[6] = 0;
	hdr[7] = 0;
	hdr[8] = 0;
	hdr[9] = 0;
	hdr[10] = 0;
	hdr[11] = 0;
	hdr[12] = w % 256;
	hdr[13] = w / 256;
	hdr[14] = h % 256;
	hdr[15] = h / 256;
	hdr[16] = 8 * d;
	hdr[17] = 0;// ((d == 4) ? 2048 : 0);
	dwBytesSaved += 18;
	file.Write(&hdr, 18);
	UINT outcnt = 0;
	for (y = 0; y < h; y++)
		memmove(dst+y*op,src+y*ip,op);
	outcnt = h * op;
	file.Write(dst, outcnt);
	file.Close();
	delete [] dst;
	return 0;

}

int CMaya::Close(CScene * pScene)
{
/*
	get scsne pointer
	determine new frame ids
	get extra file stuff
	export to tga files
	create timing file name
	dump timing file
1:2
4:7
etc
*/
	UINT frames[MAX_MAYA_FRAMES];
	UINT levels[MAX_MAYA_FRAMES];
	UINT c = pScene->Before(frames,MAX_MAYA_FRAMES,-1,0);
	UINT i,j;
	char path[300];
	char name[300];
	for (i = 0, j = 0; name[i] = m_name[i]; i++)
		if (name[i] == '.')
			j = i;
	if (j)
		name[j] = 0;
	for (i = 0, j = 0; path[i] = m_name[i]; i++)
		if ((path[i] == '/') || (path[i] == '\\'))
			j = i;
	if (j)
		path[j+1] = 0;
	strcat(name,"_retimed.txt");
	FILE * stream;
	stream = fopen(name,"w");
	if (stream == NULL)
		return 1;
	fputs("[timing],\n", stream);
	for (i = 0; i < c;i++)
		{
		char buf[100];
		sprintf(buf,"%d:%d,\n",m_index[i],frames[i]+1);
		fputs((LPCSTR)buf, stream);
		}
	fputs("-,\n", stream);
	fputs("[images],\n", stream);
	for (i = 0; i < c;i++)
		{
		char buf[100];
		levels[i]  = pScene->TopLevel(frames[i]);
		if (levels[i] > 0)
			{
			sprintf(buf,"\"%sframe%d.tga\",\n",path,frames[i]+1);
//			sprintf(buf,"%d,%d\n",i,levels[i]);
			fputs((LPCSTR)buf, stream);
			}
		}
	fputs("-,\n", stream);
	fclose(stream);
	for (i = 0; i < c; i++)
		{
		if (levels[i] > 0)
			ExportFile(pScene,path,frames[i],levels[i]);
		}
	return InformMaya();
}

int CMaya::InformMaya()
{	
/*
//	return 0;
	SOCKET clisock;
	sockaddr_in cli;
//	int count,ee;

	cli.sin_addr.s_addr=inet_addr(m_host);
	cli.sin_family=AF_INET;
	cli.sin_port=m_port;//htons(m_port);
	clisock=socket(AF_INET,SOCK_STREAM,0);	
	char msg[200];
	sprintf(msg,"sending:%s|",m_cmd);
	AfxMessageBox(msg);
	sprintf(msg,"%s\n",m_cmd);
	send(clisock,msg,100,0);

	AfxMessageBox("after send");

	closesocket(clisock);
	
	return 0;
*/


#ifdef FLIPBOOK_MAC
	Socket* s = [[Socket alloc] init];
	[s connectToHostName:[NSString stringWithCString:m_host] port:m_port];
#else
    WSADATA WsaData;

    WSAStartup( MAKEWORD(1,1), &WsaData);

    SOCKET s; 

    struct sockaddr_in server;

    struct hostent *h_ent;

    h_ent = gethostbyname( m_host);

    server.sin_addr.s_addr = *(u_long *)h_ent->h_addr;

    server.sin_family = AF_INET;

    server.sin_port = htons(m_port);

    WSASetLastError(0);

    s = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP);

    connect( s, (struct sockaddr*)&server, sizeof(server));
#endif

	char msg[200];
	sprintf(msg,"sending:%s|",m_cmd);
//	AfxMessageBox(msg);
	sprintf(msg,"%s\n",m_cmd);

#ifdef FLIPBOOK_MAC
	[s writeString:[NSString stringWithCString:msg]];
	[s release];
#else
    int rc = send(s, msg,strlen(msg),0);
	sprintf(msg,"send rc:%d",rc);
//	AfxMessageBox(msg);

//    TCHAR szMessage[256];
/*
    char buffer[256] = {0}, *p = buffer;
	
    do{

        rc = recv(s, p++, 1, 0);

    }while(rc == 1);

    if( !buffer[0])

    {

      AfxMessageBox("No response!");

    }

    else

    {
	buffer[100] = 0; //termintae
	sprintf(msg,"rsp:%s",buffer);
	AfxMessageBox(msg);

    }
*/
    closesocket(s); 
#endif
    return 0;

}
