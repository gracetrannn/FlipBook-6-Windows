
#if !defined(CMAYA_H)
#define CMAYA_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MAX_MAYA_FRAMES 100

class CSketchDoc;
class CScene;
class CMaya
{
public:
	CMaya();
	~CMaya();
	int Init(LPCSTR name);
	void Info(UINT &w, UINT &h, UINT &n, UINT &l);
	int DoFile(UINT Index, CSketchDoc * pDoc);
	int Close(CScene * pScene);
//	char m_status[100];
//	char m_rsp[328];
	int Count() { return m_count;};
	
protected:
	int ExportFile(CScene * pScene, char * path,UINT Frame, UINT Level);
	int InformMaya();
	UINT m_width;
	UINT m_height;
	UINT m_frames;
	UINT m_levels;
	UINT m_rate;
	UINT m_port;
	char m_host[20];
	char m_cmd[200];
	char m_name[300];
	int  m_index[MAX_MAYA_FRAMES];
	char m_names[MAX_MAYA_FRAMES][300];
	int  m_count;
};
#endif
