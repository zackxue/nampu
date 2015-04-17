#pragma once


// CJComboBox

class CJComboBox : public CComboBox
{
	DECLARE_DYNAMIC(CJComboBox)

public:
	CJComboBox();
	virtual ~CJComboBox();

	void DrawComboBox(CDC* pDC, CImage& img, COLORREF cr);

	void SetImage(const char* szNormalName, const char* szSelectName);
	void SetImage(const CImage* pImgNormal, const CImage* pImgSel);
private:
	long			m_lArrowWidth;
	BOOL			m_bSelect;
	BOOL			m_bMoseHover;
	CImage			m_imgNormal;
	CImage			m_imgSelect;
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnMouseLeave();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};


