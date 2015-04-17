// CustomListCtrl.cpp : implementation file
//

#include "stdafx.h"

#include "CustomListCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCustomListCtrl

CCustomListCtrl::CCustomListCtrl()
{



}


CCustomListCtrl::~CCustomListCtrl()
{

}


BEGIN_MESSAGE_MAP(CCustomListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CCustomListCtrl)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCustomListCtrl message handlers

void CCustomListCtrl::PreSubclassWindow() 
{
	m_Header.SubclassWindow(GetHeaderCtrl()->m_hWnd);	
	CListCtrl::PreSubclassWindow();
}

int CCustomListCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CListCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;
	return 0;
}
