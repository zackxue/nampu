#if !defined(AFX_CUSTOMLISTCTRL_H__4A7CA523_F757_42BF_84C3_EC59665F393B__INCLUDED_)
#define AFX_CUSTOMLISTCTRL_H__4A7CA523_F757_42BF_84C3_EC59665F393B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CustomListCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCustomListCtrl window
#include "CustomHeader.h"

/*
 * @author: xiaohe
 * @date: 2003/03/04 
 * @detail: 在ChildView中实现的MyListCtrl(仿ListControl), 输入或输出信息带方格的部分  
 *
 */

class CCustomListCtrl : public CListCtrl
{
// Construction
public:
	CCustomListCtrl();
	CCustomHeader m_Header;
// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCustomListCtrl)
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CCustomListCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CCustomListCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CUSTOMLISTCTRL_H__4A7CA523_F757_42BF_84C3_EC59665F393B__INCLUDED_)
