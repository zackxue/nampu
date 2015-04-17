#if !defined(AFX_MYSLIDERCONTROL_H__C76FA857_51CC_4EB6_A8E2_8323BBEF10BD__INCLUDED_)
#define AFX_MYSLIDERCONTROL_H__C76FA857_51CC_4EB6_A8E2_8323BBEF10BD__INCLUDED_
#pragma once   // speed up compiling with MSVC++, file will only be read once
#if _MSC_VER > 1000
#pragma once
#endif 

/////////////////////////////////////////////////////////////////////////////////////////
class CJTranSliderCtrl : public CSliderCtrl
{


// Construction
public:
	CJTranSliderCtrl();
	virtual ~CJTranSliderCtrl();

public:
	void SetToolTip(CToolTipCtrl* pToolTip , LPCTSTR lpszTip = NULL);
	void SetThumbColors(COLORREF face, COLORREF highlight);
	void SetllPos(LONGLONG nPos);
	void DrawTransparent(BOOL bRepaint);

private:
	BOOL		m_bRedraw;
	HDC			m_dcBk;
	HBITMAP		m_bmpBk;
	HBITMAP     m_bmpBkOld;
	COLORREF	m_crThumbColor;
	COLORREF	m_crThumbColorSelected;
	CString		m_strTip;
	CToolTipCtrl* m_pToolTipCtrl;

protected:
	void DrawChannel(CDC *pDC, LPNMCUSTOMDRAW lpcd);
	void DrawThumb(CDC *pDC, LPNMCUSTOMDRAW lpcd);

	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnToolTipNotify(UINT id, NMHDR * pNMHDR, LRESULT * pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYSLIDERCONTROL_H__C76FA857_51CC_4EB6_A8E2_8323BBEF10BD__INCLUDED_)
