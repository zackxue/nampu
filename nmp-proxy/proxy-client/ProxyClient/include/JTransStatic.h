#pragma once


// CJTransStatic

class CJTransStatic : public CStatic
{
	DECLARE_DYNAMIC(CJTransStatic)

public:
	CJTransStatic();
	virtual ~CJTransStatic();

	void SetTxtColor(COLORREF cr);
	void JRefresh();
private:
	CBrush				m_brNULL;
	COLORREF			m_crText;
	CString				m_strText;
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg LRESULT OnSetText(WPARAM wParam, LPARAM lParam);
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
};


