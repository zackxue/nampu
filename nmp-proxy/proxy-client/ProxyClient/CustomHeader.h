#if !defined(AFX_CUSTOMHEADER_H__374F660D_AAEB_4558_BD98_2ACF63E75273__INCLUDED_)
#define AFX_CUSTOMHEADER_H__374F660D_AAEB_4558_BD98_2ACF63E75273__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CustomHeader.h : header file
//

/*
 * @author: xiaohe
 * @date: 2003/03/04 
 * @detail: 在ChildView中实现的MyListCtrl(仿ListControl), 中显示名称的，带有图片的第一栏非方格的部分  
 *
 */

/////////////////////////////////////////////////////////////////////////////
// CCustomHeader window

class CCustomHeader : public CHeaderCtrl
{
// Construction
public:
	CCustomHeader();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCustomHeader)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CCustomHeader();

	// Generated message map functions
protected:
	//{{AFX_MSG(CCustomHeader)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CUSTOMHEADER_H__374F660D_AAEB_4558_BD98_2ACF63E75273__INCLUDED_)
