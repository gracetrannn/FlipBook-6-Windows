#include "mydoc.h"


typedef struct {
	int flags;
	UINT frame;
	UINT level;
	int xx;
	int yy;
	int rot;
	int num;
} SHFTBL;


class CShiftTrace {
public:
	void Init(CSketchView * pView);
	BOOL Toggle(); // return new edit state
	void Draw();
	void Start(CPoint point);
	BOOL Stop(CPoint point);
	void Drag(CPoint point);
	void Cursor(CPoint point);
	void Add(UINT frame, UINT level, UINT code);
	UINT Test(UINT frame, UINT level);

	void Get(UINT which,int & xx,int & yy,
				int & rot,int & num);

protected:
	void ShiftCursor(UINT code);
	BOOL m_bActive;
	BOOL m_bEditing;
	CSketchView * m_pView;
	CPoint m_center;
	CPoint m_start;
	int m_cx;
	int m_cy;
	UINT m_hit;
	int m_xx;
	int m_yy;
	int m_rot;
	int m_num;
	BOOL m_bDown;
	UINT Hit(CPoint point);
	SHFTBL m_shift_tbl[10];
};
