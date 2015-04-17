#if !defined(AFX_DRAWEDIT_H__0D778788_B651_4CA0_AC46_323DB8BBF966__INCLUDED_)
#define AFX_DRAWEDIT_H__0D778788_B651_4CA0_AC46_323DB8BBF966__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DrawEdit.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDrawEdit window

//×Ô¶¨Òå±à¼­¿ò

class CDrawEdit : public CEdit
{
private:
	COLORREF  m_BoundColor;

// Construction
public:
	CDrawEdit();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDrawEdit)
	protected:
	virtual void PreSubclassWindow();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetBoundColor(COLORREF color);
	virtual ~CDrawEdit();

	// Generated message map functions
protected:
	//{{AFX_MSG(CDrawEdit)
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DRAWEDIT_H__0D778788_B651_4CA0_AC46_323DB8BBF966__INCLUDED_)
