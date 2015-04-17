#include "StdAfx.h"
#include "JMenuEx.h"

CMap<UINT, UINT, UINT, UINT> JMenuEx::m_mpCheckItem;
JMenuEx::JMenuEx(void)
{
	m_crMenuLeft = RGB(33, 32, 32);
	m_crMenuBK = RGB(235, 236, 237);
	m_crMenuSelect = RGB(132, 132, 130);
	m_crMenuText = RGB(0, 0, 0);
	m_crMenuTextSelect = RGB(255, 0, 0);
	m_crSpliterbar = RGB(170, 170, 170);
	m_crCheck = RGB(0, 0, 0);
}

JMenuEx::~JMenuEx(void)
{
	while(!m_lstMenuItem.IsEmpty())
	{
		delete m_lstMenuItem.RemoveHead();
	}
}


BOOL JMenuEx::AppendMenu(UINT nFlags, UINT_PTR nIDNewItem, LPCTSTR lpszNewItem )
{
	m_mpCheckItem.SetAt(nIDNewItem, nFlags);
	return CMenu::AppendMenu(nFlags, nIDNewItem, lpszNewItem);
}

void JMenuEx::InitMenu()
{
	ChangeItemType(this);
}

void JMenuEx::ChangeItemType(CMenu* pMenu)
{
	UINT uCount = pMenu->GetMenuItemCount();
	for(UINT i = 0; i < uCount; i++)
	{
		PJMENUITEM pItem = new JMENUITEM;
		pItem->uID = pMenu->GetMenuItemID(i);
		pMenu->GetMenuString(i, pItem->strText, MF_BYPOSITION);
		pItem->uIndex = -1;
		pItem->iLeftPos = -1;
		if(pItem->uID > 0)
		{
			CMenu* pSubMenu = pMenu->GetSubMenu(i);
			if(pSubMenu)
			{
				ChangeItemType(pSubMenu);
			}
		}
		UINT nFlag = MF_BYPOSITION | MF_OWNERDRAW | m_mpCheckItem[pItem->uID];
		pMenu->ModifyMenu(i, nFlag, pItem->uID, (LPCTSTR)pItem);
		m_lstMenuItem.AddTail(pItem);
	}
}

void JMenuEx::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	// TODO:  添加您的代码以绘制指定项
	//TRACE("---%d\n", lpDrawItemStruct->itemID);
	CDC dc;
	CRect rc(lpDrawItemStruct->rcItem);
	dc.Attach(lpDrawItemStruct->hDC);
	PJMENUITEM pJMenuItem = (PJMENUITEM)(lpDrawItemStruct->itemData);
	//文本颜色
	if(lpDrawItemStruct->itemState & ODS_SELECTED)
	{
		dc.SetTextColor(m_crMenuTextSelect);
	}
	else
	{
		dc.SetTextColor(m_crMenuText);
	}
	//背景色
	CBrush brBK(m_crMenuBK);
	dc.FillRect(&rc, &brBK);

	int iOldBKMode = dc.SetBkMode(TRANSPARENT);

	//左边颜色
	CBrush brLeft(m_crMenuLeft);
	RECT rcLeft = lpDrawItemStruct->rcItem;
	rcLeft.right = rcLeft.left + LEFT_BORDER_MENUE;
	dc.FillRect(&rcLeft, &brLeft);

	//分隔条
	if(pJMenuItem->uID == 0)
	{
		CRect rcSpliter = lpDrawItemStruct->rcItem;
		rcSpliter.top += rcSpliter.Height() / 2;
		rcSpliter.bottom = rcSpliter.top + 2;
		rcSpliter.left += LEFT_BORDER_MENUE + 2;
		rcSpliter.right -= 1;
		CBrush br(m_crSpliterbar);
		dc.Draw3dRect(&rcSpliter, m_crSpliterbar, m_crMenuBK);
	}
	else
	{
		//画文字
		BOOL bSelected = lpDrawItemStruct->itemState & ODS_SELECTED;
		BOOL bGrayed = lpDrawItemStruct->itemState & ODS_GRAYED;
		BOOL bCheck = lpDrawItemStruct->itemState & ODS_CHECKED;
		CRect rcText(rc.left + LEFT_BORDER_MENUE + 5, rc.top, rc.right, rc.bottom);
		DrawMenuText(&dc, rc, rcText, bSelected, bGrayed, pJMenuItem);

		//勾选
		if(bCheck)
		{


			CRect rcCheckRect = lpDrawItemStruct->rcItem;
			rcCheckRect.left += 3;
			rcCheckRect.right = LEFT_BORDER_MENUE - 3;
			dc.FillSolidRect(rcCheckRect, RGB(186, 186, 186));
			dc.Draw3dRect(rcCheckRect, RGB(0, 100, 190), RGB(0, 180, 200));			
			
			dc.SetTextColor(m_crCheck);
			CRect rcCheck = lpDrawItemStruct->rcItem;
			rcCheck.left += LEFT_BORDER_MENUE / 2 - 3;
			dc.DrawText(_T("√"), &rcCheck, DT_EXPANDTABS|DT_VCENTER|DT_SINGLELINE);
		}
	}
	dc.SetBkMode(iOldBKMode);
	dc.Detach();
}

void JMenuEx::DrawMenuText(CDC *pDC, CRect &rect, CRect rcText, BOOL bSelected, BOOL bGrayed, PJMENUITEM pItem)
{
	if(bSelected)
	{
		CBrush brSelected(m_crMenuSelect);
		pDC->FillRect(&rect, &brSelected);
	}

	if(bGrayed)
	{
		
	}
	else
	{
		pDC->DrawText(pItem->strText, rcText, DT_LEFT | DT_EXPANDTABS | DT_VCENTER);
	}
}

void JMenuEx::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	// TODO:  添加您的代码以确定指定项的大小
	PJMENUITEM pJMenuItem = (PJMENUITEM)lpMeasureItemStruct->itemData;
	CDC* pDC = AfxGetMainWnd()->GetDC();
	CSize size = pDC->GetTextExtent(pJMenuItem->strText);
	if(pJMenuItem->uID == 0)
	{
		lpMeasureItemStruct->itemHeight = SEPLIT_STATUS_MENUE;
		lpMeasureItemStruct->itemWidth = size.cx;
	}
	else
	{
		lpMeasureItemStruct->itemWidth = size.cx + LEFT_BORDER_MENUE;
		lpMeasureItemStruct->itemHeight = size.cy;
	}

	//OutputDebugString(pJMenuItem->strText);
	AfxGetMainWnd()->ReleaseDC(pDC);
}

BOOL JMenuEx::TrackPopupMenu(UINT nFlags, int x, int y, CWnd* pWnd, LPCRECT lpRect)
{
	InitMenu();
	return CMenu::TrackPopupMenu(nFlags, x, y, pWnd, lpRect);
}
