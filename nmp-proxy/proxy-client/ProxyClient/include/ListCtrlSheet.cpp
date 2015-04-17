// ListCtrlSheet.cpp : 实现文件
//

#include "stdafx.h"
#include "ListCtrlSheet.h"


// CListCtrlSheet

IMPLEMENT_DYNAMIC(CListCtrlSheet, CListCtrl)

CListCtrlSheet::CListCtrlSheet():m_hWndRecMsg(NULL)
, m_uMsg(WM_LSTCTR_MSG)
{
	//图标列表
	m_ImgList.Create(16, 16, ILC_COLOR32, 0, 20);
	m_crTextBKSelect = RGB(0, 132, 255);
	m_crSplitItem = RGB(0, 132, 255);
	m_ftList.CreatePointFont(90, _T("宋体"));
}

CListCtrlSheet::~CListCtrlSheet()
{
}


BEGIN_MESSAGE_MAP(CListCtrlSheet, CListCtrl)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CListCtrlSheet::OnNMCustomdraw)
	ON_NOTIFY_REFLECT(NM_CLICK, &CListCtrlSheet::OnNMClick)
	ON_WM_PAINT()
END_MESSAGE_MAP()

BOOL CListCtrlSheet::Create(DWORD dwStyle, const RECT& rc, CWnd* pParent, UINT uID)
{
	BOOL bRet = CListCtrl::Create(WS_CHILD | WS_VISIBLE |  LVS_REPORT | LVS_NOCOLUMNHEADER | dwStyle, rc, pParent, uID);
	if(bRet)
	{
		SetExtendedStyle(LVS_EX_FULLROWSELECT);
		SetImageList(&m_ImgList, LVSIL_SMALL);
		CFont font;
		font.CreatePointFont(210, _T("宋体"));
		SetFont(&font);
		InitList();
	}
	return bRet;
}

void CListCtrlSheet::InitList()
{
	CHeaderCtrl* pHeadCtrl = GetHeaderCtrl();
	if(pHeadCtrl)pHeadCtrl->ShowWindow(SW_HIDE);
	CRect rc;
	GetClientRect(&rc);
	InsertColumn(0, _T(""), LVCFMT_CENTER, rc.Width(), 0);
}

BOOL CListCtrlSheet::AddItemText(int nItem, LPWSTR szText)
{
	LVITEM lvText;
	ZeroMemory(&lvText, sizeof(lvText));
	lvText.iItem	= nItem;
	lvText.iSubItem = 0;
	lvText.iImage	= nItem;
	lvText.mask		= LVIF_TEXT | LVIF_IMAGE;
	lvText.pszText	= szText;
	int nRet = InsertItem(&lvText);
	return nRet == -1 ? FALSE : TRUE;
}

BOOL CListCtrlSheet::AddItemIcon(int nItem, HICON hIcon)
{
	int nRet = m_ImgList.Add(hIcon);
	return nRet == -1 ? FALSE : TRUE;
}

void CListCtrlSheet::DrawGradientLine(CDC* pDC, COLORREF crLine, POINT ptStart, POINT ptEnd)
{
	int iRBase = GetRValue(crLine);
	int iGBase = GetGValue(crLine);
	int iBBase = GetBValue(crLine);

	int nRCur = 255;
	int nGCur = 255;
	int nBCur = 255;

	double dRInc = double(255 - iRBase) / double(abs(ptEnd.x - ptStart.x));
	double dGInc = double(255 - iGBase) / double(abs(ptEnd.x - ptStart.x));
	double dBInc = double(255 - iBBase) / double(abs(ptEnd.x - ptStart.x));

	for(POINT ptCur = ptStart; ptCur.x < ptEnd.x; ptCur.x++)
	{
		pDC->SetPixel(ptCur.x, ptCur.y - 1, RGB(nRCur, nGCur, nBCur));
		pDC->SetPixel(ptCur, RGB(nRCur, nGCur, nBCur));
		nRCur = iRBase + int((ptCur.x - ptStart.x) * dRInc);
		nGCur = iGBase + int((ptCur.x - ptStart.x) * dGInc);
		nBCur = iBBase + int((ptCur.x - ptStart.x) * dBInc);
	}
}

HWND CListCtrlSheet::SetOwnWnd(HWND hWnd)
{
	HWND hOldWnd = m_hWndRecMsg;
	m_hWndRecMsg = hWnd;
	return hOldWnd;
}

UINT CListCtrlSheet::SetOwnMsg(UINT uMsg)
{
	UINT uOldMsg = m_uMsg;
	m_uMsg = uMsg;
	return uOldMsg;
}

HICON CListCtrlSheet::GetItemIcon(int nItem)
{
	return m_ImgList.ExtractIcon(nItem);
}

// CListCtrlSheet 消息处理程序
void CListCtrlSheet::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVCUSTOMDRAW pLVCD = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CRect rcList, rcItem;
	GetClientRect(&rcList);
	GetItemRect(0, &rcItem, LVIR_BOUNDS);
	int nItemTop	= rcItem.top;
	int nItemHeight = rcItem.Height();

	CDC* pDC = CDC::FromHandle(pLVCD->nmcd.hdc);
	pDC->SelectObject(&m_ftList);
	LOGFONT lFont;
	pDC->GetCurrentFont()->GetLogFont(&lFont);

	switch(pLVCD->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		{
			*pResult = CDRF_NOTIFYITEMDRAW;
			break;
		}
	case CDDS_ITEMPREPAINT:
		{
			*pResult = CDRF_NOTIFYSUBITEMDRAW;
			break;
		}
	case (CDDS_ITEMPREPAINT | CDDS_SUBITEM):
		{
			int iCol = pLVCD->iSubItem;
			int iRow = (int)pLVCD->nmcd.dwItemSpec;

			//SubItem大小
			CRect rcCell(pLVCD->nmcd.rc);
			rcCell.top = nItemTop + iRow * nItemHeight;
			rcCell.bottom = rcCell.top + nItemHeight;
			rcCell.left += 3;
			rcCell.right -= 3;
			if(iRow == 0)rcCell.top += 3;

			//ICON大小
			CRect rcIcon;
			GetSubItemRect(iRow, 0, LVIR_ICON, rcIcon);

			//
			COLORREF crBlack = RGB(0, 0, 0);
			COLORREF crWhite = RGB(255, 255, 255);
			UINT uState = CDIS_FOCUS | CDIS_SELECTED;
			if(uState == (pLVCD->nmcd.uItemState & uState))
			{
				pDC->FillSolidRect(&rcCell, m_crTextBKSelect);
				pDC->SetTextColor(crWhite);
				CString strText = GetItemText(iRow, iCol);
				int nX = rcCell.left + rcIcon.Width() + 8;
				int nY = ((iRow == 0) ? rcCell.top - 3 : rcCell.top) + (nItemHeight - abs(lFont.lfHeight)) / 2;
				pDC->TextOut(nX, nY, (LPCTSTR)strText, strText.GetLength());
				::DrawIconEx(pDC->GetSafeHdc(), rcIcon.left, rcIcon.top + (nItemHeight - 16) / 2\
					, m_ImgList.ExtractIcon(iRow), 16, 16, NULL, NULL, DI_NORMAL);
				pDC->SetTextColor(crBlack);

				//分隔线
				DrawGradientLine(pDC, m_crSplitItem, CPoint(rcCell.left - 1, rcCell.bottom - 1), CPoint(rcCell.right + 1, rcCell.bottom - 1));
			}
			else
			{
				pDC->FillSolidRect(&rcCell, crWhite);
				CString strText = GetItemText(iRow, iCol);
				int nX = rcCell.left + rcIcon.Width() + 8;
				int nY = ((iRow == 0) ? rcCell.top - 3 : rcCell.top) + (nItemHeight - abs(lFont.lfHeight)) / 2;
				pDC->TextOut(nX, nY, (LPCTSTR)strText, strText.GetLength());
				::DrawIconEx(pDC->GetSafeHdc(), rcIcon.left, rcIcon.top + (nItemHeight - 16) / 2\
					, m_ImgList.ExtractIcon(iRow), 16, 16, NULL, NULL, DI_NORMAL);

				//分隔线
				DrawGradientLine(pDC, m_crSplitItem, CPoint(rcCell.left - 1, rcCell.bottom - 1), CPoint(rcCell.right + 1, rcCell.bottom - 1));
			}

			*pResult = CDRF_SKIPDEFAULT;
			break; 
		}
	default:
		{
			*pResult = CDRF_SKIPDEFAULT;
			break;
		}
	}
}

void CListCtrlSheet::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	ST_LSTCTR_MSG stMsg;
	::ZeroMemory(&stMsg, sizeof(stMsg));
	stMsg.uMsg = WM_LSTCTR_CLICK;
	stMsg.nSelectItem = pNMLV->iItem;
	stMsg.pt = pNMLV->ptAction;
	if(NULL != m_hWndRecMsg)
	{
		*pResult = ::SendMessage(m_hWndRecMsg, WM_LSTCTR_MSG, 0, (LPARAM)&stMsg);
	}
	else
	{
		*pResult = GetParent()->SendMessage(WM_LSTCTR_MSG, 0, (LPARAM)&stMsg);
	}
}
