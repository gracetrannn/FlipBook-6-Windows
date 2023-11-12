#ifndef _LEVELTBL_H_
#define _LEVELTBL_H_

enum LevelLayerFlags: WORD {
    COMPOSITE_WITHIN = 0x1,
    ACTIVE = 0x100,
    DISPLAYABLE = 0x200
};

#pragma pack(push,4)
typedef struct {
		char name[20];
		short dx;		
		short dy;
		WORD blur; // type is * 256
		WORD color; // default color and maybe tool and size
		WORD flags; // 1 is composite within
/*
in cell use top word for flags to indicate whether default values
		DWORD flags; // 1 is composite within
		// 100 is active
		// 200 is displayable
				// 10000 is use offset; from cell
				// 20000 is use blur
				// 40000 is use def color
				// 1000 is use 1
*/
} LEVLAY;

typedef struct tagLEVTBL {
	UINT	layer; // selected layer
//	BYTE pals[1024];
	LEVLAY table[11];
} LEVTBL;
#pragma pack(pop)

class CLevelTable : public tagLEVTBL
{
public:
//	CLevelTable() {memset(this,0,sizeof(LEVTBL));};
	CLevelTable();
};
#endif
