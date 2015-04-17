// JPropertPage.cpp : 实现文件
//

#include "stdafx.h"
#include "JPropertyPage.h"


// CJPropertyPage 对话框

IMPLEMENT_DYNAMIC(CJPropertyPage, CPropertyPage)

CJPropertyPage::CJPropertyPage(UINT nIDTemplate, UINT nIDCaption, DWORD dwSize)
	: CPropertyPage(nIDTemplate)
{

}

CJPropertyPage::~CJPropertyPage()
{
}

void CJPropertyPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CJPropertyPage, CPropertyPage)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// CJPropertyPage 消息处理程序

BOOL CJPropertyPage::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	RECT rc;
	GetClientRect(&rc);
	CBrush br(RGB(235, 236, 237));
	pDC->FillRect(&rc, &br);
	return TRUE;//CPropertyPage::OnEraseBkgnd(pDC);
}
