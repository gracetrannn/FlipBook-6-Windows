
class CAviInDlg : public CMyDialog
{
// Construction
public:
	CAviInDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CAviInDlg)
	enum { IDD = IDD_AVIINDLG };
	BOOL	m_bForeGround;
	int	m_start;
	int	m_stop;
	int	m_max;
	int	m_width;
	int	m_height;
	//}}AFX_DATA
// Implementation
	void Init(WORD start, WORD stop, WORD max, WORD level, WORD w, WORD h);
	WORD Start() { return m_start;};
	WORD Stop() { return m_stop;};
	BOOL Fore() { return m_bForeGround;};
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	// Generated message map functions
	//{{AFX_MSG(monitor)
//	virtual BOOL OnInitDialog();
//	virtual void OnCancel();
//	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
