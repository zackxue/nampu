// MyEdit.cpp : 实现文件
//

#include "stdafx.h"
#include "ProxyClient3.h"
#include "MyEdit.h"


// CMyEdit

IMPLEMENT_DYNAMIC(CMyEdit, CEdit)

CMyEdit::CMyEdit()
{
	m_Colour = RGB(240, 245, 249);

}

CMyEdit::~CMyEdit()
{
}


BEGIN_MESSAGE_MAP(CMyEdit, CEdit)
//	ON_WM_PAINT()
ON_WM_CTLCOLOR()
ON_WM_DRAWITEM()
ON_WM_PAINT()
ON_WM_NCPAINT()
END_MESSAGE_MAP()



// CMyEdit 消息处理程序



//void CMyEdit::OnPaint()
//{
//	CPaintDC dc(this); // device context for painting
//
//	COLORREF col = RGB(255,0,255);
//	CRect rcClient;
//	GetClientRect (rcClient);
//	CRgn rgn;
//	rgn.CreateRoundRectRgn (1, 1, rcClient.Width(), rcClient.Height(),6,6);
//
//	SetWindowRgn (rgn, TRUE);
//
//}

HBRUSH CMyEdit::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	//HBRUSH hbr = CEdit::OnCtlColor(pDC, pWnd, nCtlColor);

		  CDC* dc = GetDC(); //获取画布对象
	      CRect rect;
	      GetClientRect(rect); //获取客户区域
	      rect.InflateRect(1,1,1,1);//将客户区域增大一个像素
	      CBrush brush (m_Colour);//创建画刷
	      dc->FrameRect(rect,&brush);//绘制边框
	
	      return NULL;

	//return hbr;
}

void CMyEdit::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值


	CDC* dc = GetDC(); //获取画布对象
	CRect rect;
	GetClientRect(rect); //获取客户区域
	rect.InflateRect(1,1,1,1);//将客户区域增大一个像素
	CBrush brush (m_Colour);//创建画刷
	dc->FrameRect(rect,&brush);//绘制边框
	CEdit::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

void CMyEdit::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	    CRect rect; 
		GetWindowRect(rect); 
		rect.left += 1; 
		rect.right -= 1; 
		rect.top += 1; 
		rect.bottom -= 1; 
        rect.InflateRect(1,1,1,1);
		ScreenToClient(rect); 
		dc.Draw3dRect(rect, RGB(68,129,2), RGB(68,129,2)); 
		Invalidate(); 
		Default(); 

}

void CMyEdit::PreSubclassWindow()
{
	// TODO: 在此添加专用代码和/或调用基类

	CEdit::PreSubclassWindow();

	//ModifyStyle(0, BS_OWNERDRAW);
}

void CMyEdit::OnNcPaint()
{
	CWindowDC dc(this); // device context for painting
	CRect rect; 
	GetWindowRect(rect); 
	rect.left += 1; 
	rect.right -= 1; 
	rect.top += 1; 
	rect.bottom -= 1; 

	ScreenToClient(rect); 
	dc.Draw3dRect(rect, RGB(68,129,2), RGB(68,129,2)); 
	Invalidate(); 
	Default(); 
}
