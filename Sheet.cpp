#include "stdafx.h"
//#include "sketch.h"
#include "sheet.h"
#include "myview.h"
#include "mainfrm.h"


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define CYCAPTION 9     /* height of the caption */
#define EXTRA 0
#define HEIGHT 50

CSheet::CSheet(CWnd* pParent /*=NULL*/)
	: CDialog(CSheet::IDD, pParent)
{     
	DPF("sheet construct");
	m_pDoc = 0;
//	m_hThumb = 0;
	m_height = 1;
	m_width = 1;
//	Create(CSheet::IDD, pParent);
	m_nFixCols = 2;
	m_nFixRows = 1;
	m_nCols = 4;
	m_nRows = 20;
	m_bEditable = TRUE;
	m_bHorzLines = TRUE;
	m_bListMode = FALSE;
	m_bVertLines = TRUE;
	m_bSelectable = TRUE;
	m_bAllowColumnResize = TRUE;
	m_bAllowRowResize = TRUE;
	m_bHeaderSort = TRUE;
	m_bReadOnly = TRUE;
	m_bItalics = FALSE;//TRUE;
	m_btitleTips = TRUE;
	m_bSingleSelMode = FALSE;
	m_bCellURL = FALSE;
	//}}AFX_DATA_INIT
//	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CSheet::~CSheet()
{
//	DPF("sheet destruct:%8lx",(DWORD)m_hThumb);
//	if (m_hThumb)
//		{
//		::GlobalFree(m_hThumb);
//		m_hThumb = NULL;
//		}
}

void CSheet::OnClose()
{
	DPF("Sheet On Close");
	((CMainFrame *)m_pOwnerWnd)->SwitchSheet(TRUE);
//	((CMainFrame *)GetParent())->SwitchSheet(TRUE);
}

BOOL CSheet::MyCreate(CMainFrame* pOwnerWnd, BOOL bOnTop)
{
	m_pOwnerWnd = pOwnerWnd;
//	m_list.m_pSheet = this;
//	BOOL bResult = Create(CSheet::IDD, GetDesktopWindow());//pOwnerWnd);
	BOOL bResult = Create(CSheet::IDD, bOnTop ? pOwnerWnd : GetDesktopWindow());
	ShowWindow(SW_HIDE);
	DPF("create:%d",bResult);
	DPF("sheet w:%d,h:%d",m_width,m_height);
//	m_list.SetHorizontalExtent(100);
//	SetPosition();
	return bResult;
}

void CSheet::DoDataExchange(CDataExchange* pDX)
{
	DPF("Data Exchange");
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSheet)
//	DDX_Control(pDX, IDC_THUMBS, m_list);
	DPF("data done");
	//}}AFX_DATA_MAP
	DDX_GridControl(pDX, IDC_GRID, m_Grid);
}


BEGIN_MESSAGE_MAP(CSheet, CDialog)
	//{{AFX_MSG_MAP(CSheet)
	ON_WM_SIZE()
	ON_WM_CLOSE()
    ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
    ON_NOTIFY(NM_DBLCLK, IDC_GRID, OnGridDblClick)
    ON_NOTIFY(NM_CLICK, IDC_GRID, OnGridClick)
    ON_NOTIFY(NM_RCLICK, IDC_GRID, OnGridRClick)
    ON_NOTIFY(123, IDC_GRID, OnGridSlideUp)
    ON_NOTIFY(124, IDC_GRID, OnGridSlideDown)
    ON_NOTIFY(127, IDC_GRID, OnGridSoundSlide)
    ON_NOTIFY(128, IDC_GRID, OnGridDelete)
    ON_NOTIFY(129, IDC_GRID, OnGridInsert)
    ON_NOTIFY(130, IDC_GRID, OnGridCut)
    ON_NOTIFY(131, IDC_GRID, OnGridCopy)
    ON_NOTIFY(132, IDC_GRID, OnGridPaste)
    ON_NOTIFY(133, IDC_GRID, OnGridChar)
    ON_NOTIFY(134, IDC_GRID, OnGridPasteReverse)
    ON_NOTIFY(135, IDC_GRID, OnGridSpecialChar)
    ON_NOTIFY(136, IDC_GRID, OnGridMsg)
    ON_NOTIFY(137, IDC_GRID, OnGridDblClick)
    ON_NOTIFY(138, IDC_GRID, OnGridRClick)
    ON_NOTIFY(139, IDC_GRID, OnAltKeyUp)
END_MESSAGE_MAP()


void CSheet::OnGridMsg(NMHDR *pNotifyStruct, LRESULT* pResult)
{
	NM_GRIDVIEW * pnm = (NM_GRIDVIEW *)pNotifyStruct;
	DPF("grid msg,%d,%d,%d",pnm->hdr.code,pnm->iRow,pnm->iColumn);
	m_pOwnerWnd->GridMsg(pnm->hdr.code, pnm->iRow,pnm->iColumn);
}

void CSheet::OnGridChar(NMHDR *pNotifyStruct, LRESULT* pResult)
{
	NM_GRIDVIEW * pnm = (NM_GRIDVIEW *)pNotifyStruct;
	int nChar = pnm->iRow;
	int nRep = pnm->iColumn;
	DPF("notify char,%d,%d",nChar,nRep);
	m_pOwnerWnd->GridChar(nChar,nRep);
}

void CSheet::OnGridDelete(NMHDR *pNotifyStruct, LRESULT* pResult)
{
	DPF("notify delete");
	m_pOwnerWnd->InsertDelete(TRUE);
}

void CSheet::OnGridInsert(NMHDR *pNotifyStruct, LRESULT* pResult)
{
	DPF("notify insert");
	m_pOwnerWnd->InsertDelete(FALSE);
}

void CSheet::OnGridCut(NMHDR *pNotifyStruct, LRESULT* pResult)
{
	DPF("notify cut");
	m_pOwnerWnd->CutCopy(FALSE);
}

void CSheet::OnGridCopy(NMHDR *pNotifyStruct, LRESULT* pResult)
{
	DPF("notify copy");
	m_pOwnerWnd->CutCopy(TRUE);
}

void CSheet::OnGridPaste(NMHDR *pNotifyStruct, LRESULT* pResult)
{
	DPF("notify paste");
	m_pOwnerWnd->OnEditPaste();
}

void CSheet::OnGridPasteReverse(NMHDR *pNotifyStruct, LRESULT* pResult)
{
	DPF("notify paste rev");
	m_pOwnerWnd->OnEditPasteReverse();
}

void CSheet::OnGridSlideUp(NMHDR *pNotifyStruct, LRESULT* pResult)
{
//	DPF("notify slide up");
	m_pOwnerWnd->SlideIt(0);
}

void CSheet::OnGridSlideDown(NMHDR *pNotifyStruct, LRESULT* pResult)
{
//	DPF("notify slide");
	m_pOwnerWnd->SlideIt(1);
}
void CSheet::OnGridSoundSlide(NMHDR *pNotifyStruct, LRESULT* pResult)
{
	NM_GRIDVIEW * pnm = (NM_GRIDVIEW *)pNotifyStruct;
	int dx = pnm->iRow;
	int dy = pnm->iColumn;
	DPF("notify slide,dx:%d,dy:%d",dx,dy);
	m_pOwnerWnd->SlideSound(dy,dx);
}

void CSheet::OnGridSpecialChar(NMHDR *pNotifyStruct, LRESULT* pResult)
{
	NM_GRIDVIEW * pnm = (NM_GRIDVIEW *)pNotifyStruct;
	int nChar = pnm->iRow;
	UINT cmd = 0;
	if (nChar == VK_F8)
		cmd = ID_FILE_GRAB;
	else if (nChar == VK_F4)
		cmd = ID_KEY_F4;
	else if (nChar == VK_F5)
		cmd = ID_KEY_F5;
	if (cmd)
		m_pOwnerWnd->PostMessage(WM_COMMAND, cmd, 0);
}

void CSheet::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
DPF("sheet char:%d",nChar);
	switch (nChar)
        {
		case VK_UP:
DPF("up");
			break;
		case VK_DOWN:
DPF("down");
			break;
		default:
			CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
			break;
        }
}


void CSheet::GetCellText(CString& txt, int row, int col)
{
	char buf[20];
//	int cc = m_Grid.GetColumnCount();
//	m_pOwnerWnd->GetCellText(buf,row,cc-col-1);
//	int cc = m_Grid.GetColumnCount();
	m_pOwnerWnd->GetCellText(buf,row,col);
	txt = buf;
}

void CSheet::DrawCellImage(CDC * pDC, LPCRECT rect, int row, int col, DWORD state)
{
//	int cc = m_Grid.GetColumnCount();
//	m_pOwnerWnd->DrawThumb(pDC, rect,row,cc-col-1);
	DWORD code = 0;
	if (state & GVIS_SELECTED)
		code |= 1;
	if (state & GVIS_DROPHILITED)
		code |= 2;
	if (state & GVIS_FOCUSED)
		code |= 2;
	m_pOwnerWnd->DrawThumb(pDC, rect,row,col, code);
}

void CSheet::Drop(int row, int col, BOOL bCopy)
{
//	int cc = m_Grid.GetColumnCount();
DPF("sheet drop, row:%d,col:%d",row,col);
//	m_pOwnerWnd->Drop(row,cc-col-1);
	m_pOwnerWnd->Drop(row,col,bCopy);
}

void CSheet::DropFile(LPCSTR szName)
{
DPF("sheet drop file:%s",szName);
	m_pOwnerWnd->DropFile(szName);
}

void CSheet::GetSelection(LPRECT rect)
{
    CCellRange Selection = m_Grid.GetSelectedCellRange();
    if (!m_Grid.IsValid(Selection))
		{
		rect->top = 0;
		rect->left = 0;
		rect->right = 0;
		rect->bottom = 0;
		}
	else
		{
		rect->top = Selection.GetMinRow();
		rect->left = Selection.GetMinCol();
		rect->bottom = Selection.GetMaxRow() + 1;
		rect->right = Selection.GetMaxCol() + 1;
		}
}

void CSheet::SetSelection(RECT& rect)
{
	rect.top++;
	rect.bottom++;
	int cc = m_Grid.GetColumnCount();
	int w = rect.right - rect.left;
	rect.left = cc - rect.right;
	rect.right = cc + w - rect.right;
DPR("sheetsel",&rect);
	m_Grid.SetFocusCell(rect.top,rect.left);
	m_Grid.SetSelectedRange(rect.top,rect.left,rect.bottom-1,rect.right-1);
//	m_Grid.SetFocusCell(rect.top,rect.right);
//	m_Grid.SetFocusCell(rect.top,rect.left);
//	m_Grid.SetFocusCell(-1,-1);
}

void CSheet::DrawCellText(CDC * pDC, LPCRECT rect, int row, int col)
{
//	int cc = m_Grid.GetColumnCount();
//	m_pOwnerWnd->DrawText(pDC, rect,row,cc-col-1);
	m_pOwnerWnd->DrawText(pDC, rect,row,col);
}

void CSheet::ReDraw()
{
	return;
DWORD dwTextStyle = DT_RIGHT|DT_VCENTER|DT_SINGLELINE;
	// fill rows/cols with text
	int cc = m_Grid.GetColumnCount();
	for (int row = 0; row < m_Grid.GetRowCount(); row++)
		for (int col = 0; col < cc; col++)
		{ 
			GV_ITEM Item;
			Item.mask = GVIF_TEXT|GVIF_FORMAT;
			Item.row = row;
			Item.col = col;
			if (row < m_nFixRows)
            {
				Item.nFormat = DT_LEFT|DT_WORDBREAK;
				char buf[20];
				if (col)
					{
					m_pOwnerWnd->GetCellText(buf,row,col);
					}
				else
					strcpy(buf,"              ");
				Item.strText.Format(buf);
			}
            else if (col < m_nFixCols) 
            {
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("Frame %d"),row);
			}
            else 
            {
				char buf[10];
				Item.nFormat = dwTextStyle;
				m_pOwnerWnd->GetCellText(buf,row,col);
				Item.strText.Format(buf);
			//	Item.strText.Format(_T("%s-%d"),buf,row);
			}
			m_Grid.SetItem(&Item);

//			if (col == (m_Grid.GetFixedColumnCount()-1) )//&& row >= m_Grid.GetFixedRowCount())
//				m_Grid.SetItemImage(row, col, rand()%m_ImageList.GetImageCount());
			//else if (rand() % 10 == 1)
			//	m_Grid.SetItemImage(row, col, 0);
/*
			if (rand() % 10 == 1)
			{
                COLORREF clr = RGB(rand()%128 + 128, rand()%128 + 128, rand()%128 + 128);
				m_Grid.SetItemBkColour(row, col, clr);
				m_Grid.SetItemFgColour(row, col, RGB(255,0,0));
			}
*/
		}
}

BOOL CSheet::OnInitDialog()
{
	CDialog::OnInitDialog();

	CRect rect;
	GetClientRect(rect);
	m_OldSize = CSize(rect.Width(), rect.Height());

	OnListmode();

	/////////////////////////////////////////////////////////////////////////
	// initialise grid properties
	/////////////////////////////////////////////////////////////////////////

	m_Grid.SetEditable(m_bEditable);
	m_Grid.SetListMode(m_bListMode);
	m_Grid.EnableDragAndDrop(TRUE);
	m_Grid.SetTextBkColor(RGB(0xFF, 0xFF, 0xFF));
//	m_Grid.SetTextBkColor(RGB(0xFF, 0xFF, 0xE0));

	TRY {
		m_Grid.SetRowCount(m_pOwnerWnd->Rows());
		m_Grid.SetColumnCount(m_pOwnerWnd->Columns());
		m_Grid.SetFixedRowCount(1);
		m_Grid.SetFixedColumnCount(
					m_pOwnerWnd->Columns()-m_pOwnerWnd->Levels());
	}
	CATCH (CMemoryException, e)
	{
		e->ReportError();
		e->Delete();
		return FALSE;
	}
    END_CATCH

    DWORD dwTextStyle = DT_RIGHT|DT_VCENTER|DT_SINGLELINE;
#ifndef _WIN32_WCE
    dwTextStyle |= DT_END_ELLIPSIS;
#endif
	ReDraw();

	// Make cell 1,1 read-only
//    m_Grid.SetItemState(1,1, m_Grid.GetItemState(1,1) | GVIS_READONLY);

    OnItalics();
    OnTitletips();
    
//	m_Grid.AutoSize();
	m_Grid.SetRowHeight(0, 40);
	
#ifndef FLIPBOOK_MAC
	HICON hIcon;

	hIcon = (HICON)LoadImage(  AfxGetApp()->m_hInstance,
                           MAKEINTRESOURCE(IDR_MAINFRAME),
                           IMAGE_ICON,
                           GetSystemMetrics(SM_CXSMICON),
                           GetSystemMetrics(SM_CYSMICON),
                           0);
	if(hIcon)
		{
         SendMessage(WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		}
#endif

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSheet::SelectCell(UINT Frame, UINT Level)
{
	DPF("select cell,frm:%d,Lev:%d",Frame,Level);
	int cc = m_Grid.GetColumnCount();
	UINT row = Frame + 1;
	UINT col = cc - Level - 1;
//	m_pegbar.AddItem(Frame);
	m_Grid.EnableSelection(TRUE);
	m_Grid.SetFocusCell(row, col);
	m_Grid.SetSelectedRange(row,col,row,col);
	m_pOwnerWnd->SelectGCell(row,col);
//	CSketchView * pView = (CSketchView *)((CFrameWnd *)GetParent())->GetActiveView();
//	pView->VSelectCell(Frame,Level);
}

void CSheet::SetRange(UINT Frames, UINT Levels, BOOL bClear /* = 0 */)
{
	DPF("sheet set range,%d,%d",Frames,Levels);
	if (bClear)
		m_Grid.ClearFocusCell();
	m_Grid.SetRowCount(m_pOwnerWnd->Rows());
	m_Grid.SetColumnCount(m_pOwnerWnd->Columns());
	m_Grid.SetFixedRowCount(1);
	UINT FixedCount = m_pOwnerWnd->Columns()-m_pOwnerWnd->Levels();
	m_Grid.SetFixedColumnCount(FixedCount);
	m_Grid.DragAcceptFiles(1);//(FixedCount > 1);
	ReDraw();
}

/*
void CSheet::OnSysCommand(UINT nID, LPARAM lParam)
{
//	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
//	{
//		CAboutDlg dlgAbout;
//		dlgAbout.DoModal();
//	}
//	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

HCURSOR CSheet::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CSheet::OnUpdateEditCols() 
{
	if (!::IsWindow(m_Grid.m_hWnd)) return;
	UpdateData();

    int nOldNumCols = m_Grid.GetColumnCount();

	TRY { 
        m_Grid.SetColumnCount(m_nCols); 
    }
	CATCH (CMemoryException, e)
	{
		e->ReportError();
		e->Delete();
		return;
	}
    END_CATCH

	m_nCols = m_Grid.GetColumnCount();
	m_nFixCols = m_Grid.GetFixedColumnCount();
	UpdateData(FALSE);

    CString str;
    for (int i = nOldNumCols; i < m_nCols; i++)
    {
        str.Format(_T("Column %d"), i);
        m_Grid.SetItemText(0,i,str);
    }
}

void CSheet::OnUpdateEditFixcols() 
{
	if (!::IsWindow(m_Grid.m_hWnd)) return;
	UpdateData();

	TRY {
        m_Grid.SetFixedColumnCount(m_nFixCols); 
    }
	CATCH (CMemoryException, e)
	{
		e->ReportError();
		e->Delete();
		return;
	}
    END_CATCH

	m_nCols = m_Grid.GetColumnCount();
	m_nFixCols = m_Grid.GetFixedColumnCount();
	UpdateData(FALSE);
}

void CSheet::OnUpdateEditFixrows() 
{
	if (!::IsWindow(m_Grid.m_hWnd)) return;
	UpdateData();

	TRY {
        m_Grid.SetFixedRowCount(m_nFixRows); 
    }
	CATCH (CMemoryException, e)
	{
		e->ReportError();
		e->Delete();
		return;
	}
    END_CATCH

	m_nRows = m_Grid.GetRowCount();
	m_nFixRows = m_Grid.GetFixedRowCount();
	UpdateData(FALSE);
}

void CSheet::OnUpdateEditRows() 
{	
	if (!::IsWindow(m_Grid.m_hWnd)) return;
	UpdateData();

    int nOldNumRows = m_Grid.GetRowCount();

	TRY {
        m_Grid.SetRowCount(m_nRows); 
    }
	CATCH (CMemoryException, e)
	{
		e->ReportError();
		e->Delete();
		return;
	}
    END_CATCH

	m_nRows = m_Grid.GetRowCount();
	m_nFixRows = m_Grid.GetFixedRowCount();
	UpdateData(FALSE);

    CString str;
    for (int i = nOldNumRows; i < m_nRows; i++)
    {
        str.Format(_T("Row %d"), i);
        m_Grid.SetItemText(i,0,str);
    }
}

//void CSheet::OnGridSlideUp(NMHDR *pNotifyStruct, LRESULT* pResult)
//{
//	DPF("slide up");
//}

void CSheet::OnGridLines() 
{
	UpdateData();

	if (!m_bHorzLines && !m_bVertLines)
		m_Grid.SetGridLines(GVL_NONE);
	else if (m_bHorzLines && !m_bVertLines)
		m_Grid.SetGridLines(GVL_HORZ);
	else if (!m_bHorzLines && m_bVertLines)
		m_Grid.SetGridLines(GVL_VERT);
	else 
		m_Grid.SetGridLines(GVL_BOTH);
}
*/
void CSheet::OnListmode() 
{
	UpdateData();
	m_Grid.SetListMode(m_bListMode);

#ifndef FLIPBOOK_MAC // this doesn't really make sense for the Mac xsheet
	CWnd* pButton = GetDlgItem(100);//IDC_HEADERSORT);
	if (pButton) 
	{
		pButton->ModifyStyle(m_bListMode?WS_DISABLED:0, m_bListMode? 0:WS_DISABLED);
		pButton->Invalidate();
	}
	pButton = GetDlgItem(101);//IDC_SINGLESELMODE);
	if (pButton) 
	{
		pButton->ModifyStyle(m_bListMode?WS_DISABLED:0, m_bListMode? 0:WS_DISABLED);
		pButton->Invalidate();
	}
#endif
}
/*
void CSheet::OnHeaderSort() 
{
	UpdateData();
	m_Grid.SetHeaderSort(m_bHeaderSort);
}

void CSheet::OnSingleselmode() 
{
	UpdateData();
	m_Grid.SetSingleRowSelection(m_bSingleSelMode);
}

void CSheet::OnEditable() 
{
	UpdateData();
	m_Grid.SetEditable(m_bEditable);
}

void CSheet::OnAllowSelection() 
{
	UpdateData();
	m_Grid.EnableSelection(m_bSelectable);
}

void CSheet::OnRowResize() 
{
	UpdateData();
	m_Grid.SetRowResize(m_bAllowRowResize);
}

void CSheet::OnColResize() 
{
	UpdateData();
	m_Grid.SetColumnResize(m_bAllowColumnResize);
}

void CSheet::OnPrintButton() 
{
#if !defined(WCE_NO_PRINTING) && !defined(GRIDCONTROL_NO_PRINTING)
	m_Grid.Print();
#endif
}

void CSheet::OnFontButton() 
{
#ifndef _WIN32_WCE
	LOGFONT lf;
	m_Grid.GetFont()->GetLogFont(&lf);

	CFontDialog dlg(&lf);
	if (dlg.DoModal() == IDOK) {
		dlg.GetCurrentFont(&lf);

		CFont Font;
		Font.CreateFontIndirect(&lf);
		m_Grid.SetFont(&Font);
        OnItalics();	
		m_Grid.AutoSize();
		Font.DeleteObject();	
	}
#endif
}
*/
BOOL CALLBACK EnumProc(HWND hwnd, LPARAM lParam)
{
	CWnd* pWnd = CWnd::FromHandle(hwnd);
	CSize* pTranslate = (CSize*) lParam;

	CSheet* pDlg = (CSheet*) pWnd->GetParent();
	if (!pDlg) return FALSE;

	CRect rect;
	pWnd->GetWindowRect(rect);
	if (hwnd == pDlg->m_Grid.GetSafeHwnd())
		DPF("Wnd rect: %d,%d - %d,%d",rect.left,rect.top, rect.right, rect.bottom);
	pDlg->ScreenToClient(rect);
	if (hwnd == pDlg->m_Grid.GetSafeHwnd())
		DPF("Scr rect: %d,%d - %d,%d",rect.left,rect.top, rect.right, rect.bottom);


	if (hwnd == pDlg->m_Grid.GetSafeHwnd())
	{
		if (  ((rect.top >= 7 && pTranslate->cy > 0) || rect.Height() > 20) &&
			  ((rect.left >= 7 && pTranslate->cx > 0) || rect.Width() > 20)   )
			pDlg->m_Grid.MoveWindow(rect.left, rect.top, 
									rect.Width()+pTranslate->cx, 
									rect.Height()+pTranslate->cy, FALSE);
		else
			pWnd->MoveWindow(rect.left+pTranslate->cx, rect.top+pTranslate->cy, 
							 rect.Width(), rect.Height(), FALSE);
	}
	else 
	{
 //       if (::GetDlgCtrlID(hwnd) == IDC_INFO1 || ::GetDlgCtrlID(hwnd) == IDC_INFO2)
 //   		pWnd->MoveWindow(rect.left, rect.top+pTranslate->cy, 
//	    					 rect.Width(), rect.Height(), FALSE);
 //       else
       		pWnd->MoveWindow(rect.left+pTranslate->cx, rect.top+pTranslate->cy, 
        					 rect.Width(), rect.Height(), FALSE);
	}
	pDlg->Invalidate();

	return TRUE;
}

void CSheet::OnSize(UINT nType, int cx, int cy) 
{
	if (nType == SIZE_RESTORED)
		{
			DPF("sheet size");
//		m_pOwnerWnd->ShowWindow(SW_RESTORE);
		}
	CDialog::OnSize(nType, cx, cy);
	
	if (cx <= 1 || cy <= 1 ) 
        return;
	
#ifdef _WIN32_WCE
    m_Grid.MoveWindow(0,0, cx,cy, FALSE);
#else
	CSize Translate(cx - m_OldSize.cx, cy - m_OldSize.cy);

	::EnumChildWindows(GetSafeHwnd(), EnumProc, (LPARAM)&Translate);
	m_OldSize = CSize(cx,cy);
#endif
	return;
#ifndef FLIPBOOK_MAC
 if (::IsWindow(m_Grid.m_hWnd))
       m_Grid.ExpandToFit();
#endif
}
/*
#ifndef GRIDCONTROL_NO_CLIPBOARD
void CSheet::OnEditCopy() 
{
	m_Grid.OnEditCopy();	
}

void CSheet::OnEditCut() 
{
	m_Grid.OnEditCut();	
}

void CSheet::OnEditPaste() 
{
	m_Grid.OnEditPaste();	
}

void CSheet::OnUpdateEditCopyOrCut(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_Grid.GetSelectedCount() > 0);
}

void CSheet::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
    // Attach a COleDataObject to the clipboard see if there is any data
    COleDataObject obj;
    pCmdUI->Enable(obj.AttachClipboard() && obj.IsDataAvailable(CF_TEXT)); 
}
#endif

void CSheet::OnEditSelectall() 
{
	m_Grid.OnEditSelectAll();
}

void CSheet::OnAppAbout() 
{
//	CAboutDlg dlgAbout;
//	dlgAbout.DoModal();
}

void CSheet::OnReadOnly() 
{
	UpdateData();
    if (m_bReadOnly)
        m_Grid.SetItemState(1,1, m_Grid.GetItemState(1,1) | GVIS_READONLY);
    else
        m_Grid.SetItemState(1,1, m_Grid.GetItemState(1,1) & ~GVIS_READONLY);
}
*/
void CSheet::OnItalics() 
{
    UpdateData();
    
    // Set fixed cell fonts as italics
	for (int row = 0; row < m_Grid.GetRowCount(); row++)
		for (int col = 0; col < m_Grid.GetColumnCount(); col++)
		{ 
		    if (row < m_Grid.GetFixedRowCount() || col < m_Grid.GetFixedColumnCount())
		    {
#ifndef FLIPBOOK_MAC
		        LOGFONT* pLF = m_Grid.GetItemFont(row ,col);
		        if (!pLF) continue;
		        
		        LOGFONT lf;
		        memcpy(&lf, pLF, sizeof(LOGFONT));
		        lf.lfItalic = (BYTE) m_bItalics;
		        
		        m_Grid.SetItemFont(row, col, &lf);
#endif
		    }
		}
	
	m_Grid.Invalidate();
}

void CSheet::OnTitletips() 
{
    UpdateData();
    m_Grid.EnableTitleTips(m_btitleTips);
}

void CSheet::OnInsertRow() 
{
	//m_Grid.SetSelectedRange(-1,-1,-1,-1);
	int nRow = m_Grid.GetFocusCell().row;
    if (nRow >= 0)
    {
	    m_Grid.InsertRow(_T("Newest Row"), nRow);	
	    m_Grid.Invalidate();
    }
	//m_Grid.SetSelectedRange(-1,-1,-1,-1);
}

void CSheet::OnDeleteRow() 
{
	int nRow = m_Grid.GetFocusCell().row;
    if (nRow >= 0)
    {
	    m_Grid.DeleteRow(nRow);	
	    m_Grid.Invalidate();
    }
}

// (Thanks to Koay Kah Hoe for this)
BOOL CSheet::PreTranslateMessage(MSG* pMsg) 
{
    if( pMsg->message == WM_KEYDOWN )
    {
        if(pMsg->wParam == VK_RETURN
            || pMsg->wParam == VK_ESCAPE )
        {
#ifndef FLIPBOOK_MAC
            ::TranslateMessage(pMsg);
            ::DispatchMessage(pMsg);
#endif
            return TRUE;                    // DO NOT process further
        }
    }
    return CDialog::PreTranslateMessage(pMsg);
}	
/*
BOOL CSheet::OwnerModal()
{
	UINT mask = WS_DISABLED | WS_DLGFRAME | WS_CLIPSIBLINGS;
	UINT z = ::GetWindowLong(m_pOwnerWnd->m_hWnd,GWL_STYLE);
	DPF("modal:%x",z);
	return (z & mask) == mask ? 1 : 0;
}
*/
void CSheet::OnGridDblClick(NMHDR *pNotifyStruct, LRESULT* /*pResult*/)
{
//	if (OwnerModal())
//		return;
    NM_GRIDVIEW* pItem = (NM_GRIDVIEW*) pNotifyStruct;
    if (pItem->iRow < 0)
        return;
    DPF("Double Clicked on row %d, col %d", pItem->iRow, pItem->iColumn);
	if (pItem->iRow>=0 && pItem->iColumn>0)
		{
//		int cc = m_Grid.GetColumnCount();
		if (m_pOwnerWnd->Timing())
			m_pOwnerWnd->Timing(0);
		m_pOwnerWnd->SelectGCell(pItem->iRow, pItem->iColumn);
		}
}

void CSheet::OnGridClick(NMHDR *pNotifyStruct, LRESULT* /*pResult*/)
{
    NM_GRIDVIEW* pItem = (NM_GRIDVIEW*) pNotifyStruct;

    DPF("Clicked on row %d, col %d", pItem->iRow, pItem->iColumn);
    if ((pItem->iRow >= 0) && (pItem->iColumn >= 0))
		m_pOwnerWnd->LClick(pItem->iRow,pItem->iColumn);
}

void CSheet::OnGridRClick(NMHDR *pNotifyStruct, LRESULT* /*pResult*/)
{
    
    NM_GRIDVIEW* pItem = (NM_GRIDVIEW*) pNotifyStruct;
    DPF("Right Clicked on row %d, col %d", pItem->iRow, pItem->iColumn);
	if ((pItem->iRow >= 0) && (pItem->iColumn >= 0))
		m_pOwnerWnd->RClick(pItem->iRow,pItem->iColumn);
}


void CSheet::OnExpandToFit() 
{
#ifndef FLIPBOOK_MAC
    m_Grid.ExpandToFit();
#endif
}

void CSheet::OnAutoSize() 
{
#ifndef FLIPBOOK_MAC
    m_Grid.AutoSize();
#endif
}
/*
void CSheet::OnFill() 
{
    m_Grid.DeleteAllItems();
    m_Grid.AutoFill();
}
*/
void CSheet::OnAltKeyUp(NMHDR *pNotifyStruct, LRESULT* pResult)
{
	CSketchView * pView = 
				(CSketchView *)((CFrameWnd *)m_pOwnerWnd)->GetActiveView();
	pView->SendMessage(WM_SYSKEYUP,18,0);
}
