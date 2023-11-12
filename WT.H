//#include "stdafx.h"
#ifdef __cplusplus
extern "C" {
#endif
	BOOL	WtInit(HWND hWnd, HANDLE hInstance);
	VOID	WtExit(void);
	WORD	WtCheck(WORD * px, WORD * py, WORD * pp);
//	BOOL	WtChange(void);
	WORD	WtPressure(void);
	WORD	WtPositionx(void);
	WORD	WtPositiony(void);
#ifdef __cplusplus
}
#endif 
