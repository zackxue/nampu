#if !defined(AFX_CHECKBOX_H__38E72D24_A4C7_11D5_B914_000000000000__INCLUDED_)
#define AFX_CHECKBOX_H__38E72D24_A4C7_11D5_B914_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CJCheckBox.h : header file
//
#define WM_CK_LDOWNCLICK			(WM_USER + 12)
/////////////////////////////////////////////////////////////////////////////
// CJCheckBox window

class CJCheckBox : public CButton
{
	// Construction
public:
	CJCheckBox();
	virtual ~CJCheckBox();
	
	
	// Attributes
public:
	int GetCheck() const{return m_bCheckBtn;}
	void SetCheck(int nCheck){m_bCheckBtn = nCheck;}

	void DrawCheck(CDC* pDC, CRect m_rcTemp);	
	void DrawOrange(CDC* pDC, CRect m_rcTemp);
	
public:
	BOOL         m_bCheckBtn;
	BOOL         m_bPressBtn;
	BOOL         m_bOldTemp;
	COLORREF     m_clrHigh;
	COLORREF     m_clrCheck;
	BOOL         m_bSelected;
	
public:
	// Overrides
	//{{AFX_VIRTUAL(CJCheckBox)
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL
	
protected:
	//{{AFX_MSG(CJCheckBox)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg int	 OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point); 
	//}}AFX_MSG
	
	DECLARE_MESSAGE_MAP()
};

////////////////////////////////////////////
/////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHECKBOX_H__38E72D24_A4C7_11D5_B914_000000000000__INCLUDED_)
