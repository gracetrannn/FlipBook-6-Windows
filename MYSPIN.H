
class CMySpin : public CSpinButtonCtrl
{
public:
	BOOL Create(CWnd * pParent, UINT Id, UINT val, UINT min, UINT max);
	void Change();
//	UINT	m_value;
//	CSpinButtonCtrl m_spin;
};
