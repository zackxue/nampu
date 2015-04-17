// DrawEdit.cpp : implementation file
//

#include "stdafx.h"
#include "DrawEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDrawEdit

CDrawEdit::CDrawEdit()
{
	m_BoundColor = RGB(140, 158, 176);	//默认为黑色
}

CDrawEdit::~CDrawEdit()
{
}


BEGIN_MESSAGE_MAP(CDrawEdit, CEdit)
	//{{AFX_MSG_MAP(CDrawEdit)
	ON_WM_CTLCOLOR_REFLECT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDrawEdit message handlers

HBRUSH CDrawEdit::CtlColor(CDC* pDC, UINT nCtlColor) 
{
	CDC* dc = GetDC(); //获取画布对象
	CRect rect;
	GetClientRect(rect); //获取客户区域
	rect.InflateRect(1, 1, 1, 1);//将客户区域增大一个像素
	CPen pen(PS_SOLID, 1, m_BoundColor); //创建画笔
	dc->SelectObject(&pen);
	CBrush brush(m_BoundColor);//创建画刷
	dc->FrameRect(rect, &brush);//绘制边框
	return NULL;
}

void CDrawEdit::PreSubclassWindow() 
{	
	CEdit::PreSubclassWindow();
	ModifyStyleEx(WS_EX_CLIENTEDGE, 0);
	SetWindowPos(NULL, 0, 0, 0, 0, SWP_DRAWFRAME|SWP_NOSIZE|SWP_NOMOVE|SWP_NOZORDER);
}

BOOL CDrawEdit::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.dwExStyle=cs.dwExStyle+~WS_BORDER;	
	return CEdit::PreCreateWindow(cs);
}

void CDrawEdit::SetBoundColor(COLORREF color)
{
	m_BoundColor = color;
}
