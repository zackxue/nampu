#include "stdafx.h"
#include "JTranSliderCtrl.h"
#include "windows.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CJTranSliderCtrl

CJTranSliderCtrl::CJTranSliderCtrl()
{
	m_dcBk = NULL;
	m_crThumbColor = RGB(0, 0, 0);
	m_crThumbColorSelected = RGB(186, 186, 186);
	m_bRedraw = FALSE;
	m_pToolTipCtrl = NULL;
}

CJTranSliderCtrl::~CJTranSliderCtrl()
{	
	::SelectObject(m_dcBk, m_bmpBkOld);
	::DeleteObject(m_bmpBk);
	::DeleteDC(m_dcBk);			
}


BEGIN_MESSAGE_MAP(CJTranSliderCtrl, CSliderCtrl)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTipNotify)
END_MESSAGE_MAP()

// CJTranSliderCtrl 消息处理
void CJTranSliderCtrl::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LPNMCUSTOMDRAW lpcd = (LPNMCUSTOMDRAW)pNMHDR;
	CDC *pDC = CDC::FromHandle(lpcd->hdc);
	DWORD dwStyle = this->GetStyle();
	switch(lpcd->dwDrawStage)
	{
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		break;
	case CDDS_ITEMPREPAINT:
		{
			switch(lpcd->dwItemSpec)
			{
			case TBCD_TICS:
				*pResult = CDRF_DODEFAULT;
				break;
			case TBCD_THUMB:
				DrawThumb(pDC, lpcd);
				*pResult = CDRF_SKIPDEFAULT;
				break;
			case TBCD_CHANNEL:
				DrawChannel(pDC, lpcd);
				*pResult = CDRF_SKIPDEFAULT;
				break;
			}
			break;
		}
	}
}

void CJTranSliderCtrl::DrawChannel(CDC *pDC, LPNMCUSTOMDRAW lpcd)
{
	CClientDC clientDC(GetParent());
	CRect crect;
	CRect wrect;		
	GetClientRect(crect);
	GetWindowRect(wrect);
	GetParent()->ScreenToClient(wrect);
	if (m_dcBk == NULL)
	{
		m_dcBk = CreateCompatibleDC(clientDC.m_hDC);
		m_bmpBk = CreateCompatibleBitmap(clientDC.m_hDC, crect.Width(), crect.Height());
		m_bmpBkOld = (HBITMAP)::SelectObject(m_dcBk, m_bmpBk);
		::BitBlt(m_dcBk, 0, 0, crect.Width(), crect.Height(), clientDC.m_hDC, wrect.left, wrect.top, SRCCOPY);
	}

	HDC hSaveHDC;
	HBITMAP hSaveBmp;
	int iWidth = crect.Width();
	int iHeight = crect.Height();
	hSaveHDC = ::CreateCompatibleDC(pDC->m_hDC);
	hSaveBmp = ::CreateCompatibleBitmap(hSaveHDC, iWidth, iHeight);
	HBITMAP hSaveCBmpOld = (HBITMAP)::SelectObject(hSaveHDC, hSaveBmp);			
	//
	COLORREF crOldBack = ::SetBkColor(pDC->m_hDC, RGB(0,0,0));
	COLORREF crOldText = ::SetTextColor(pDC->m_hDC, RGB(255,255,255));		
	::BitBlt(hSaveHDC, 0, 0, iWidth, iHeight, pDC->m_hDC, crect.left, crect.top, SRCCOPY);
	::BitBlt(pDC->m_hDC, 0, 0, iWidth, iHeight, m_dcBk, 0, 0, SRCCOPY);
	::BitBlt(pDC->m_hDC, 0, 0, iWidth, iHeight, hSaveHDC, 0, 0, SRCAND);
	//
	::SetBkColor(pDC->m_hDC, crOldBack);
	::SetTextColor(pDC->m_hDC, crOldText);
	::SelectObject(hSaveHDC, hSaveCBmpOld);
	::DeleteObject(hSaveBmp);
	::DeleteDC(hSaveHDC);
	crect = lpcd->rc;
	if((crect.bottom - crect.top) > (crect.right - crect.left))
	{
		crect.InflateRect(1, 0, 1, 0);
	}
	else
	{
		crect.InflateRect(0, 2, 0, 2);
	}
	crect.top += 1;
	crect.bottom -= 1;
	DrawEdge(pDC->m_hDC, &crect, EDGE_SUNKEN, BF_RECT | BF_ADJUST);
}

void CJTranSliderCtrl::DrawThumb(CDC *pDC, LPNMCUSTOMDRAW lpcd)
{
	CRect crect;
	GetThumbRect(&crect);
	COLORREF col;
	if(lpcd->uItemState & CDIS_SELECTED)
	{
		col = m_crThumbColorSelected;
	}
	else
	{
		col = m_crThumbColor;
	}
	if(col == NULL && lpcd->uItemState & CDIS_SELECTED)
	{
		col = GetSysColor(COLOR_3DHIGHLIGHT);
	}
	else if(col == NULL && !(lpcd->uItemState & CDIS_SELECTED))
	{
		col = GetSysColor(COLOR_3DFACE);
	}
	HBRUSH hbrush = CreateSolidBrush(col);
    HBRUSH hbOld = (HBRUSH)SelectObject(pDC->m_hDC, hbrush);
    Ellipse(pDC->m_hDC, crect.left, crect.top, crect.right, crect.bottom);
    SelectObject(pDC->m_hDC, hbOld);
    DeleteObject(hbrush);
}

BOOL CJTranSliderCtrl::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;
}


void CJTranSliderCtrl::SetThumbColors(COLORREF face, COLORREF highlight)
{
	m_crThumbColor = face;
	m_crThumbColorSelected = highlight;
}

void CJTranSliderCtrl::DrawTransparent(BOOL bRepaint)
{
	if(m_dcBk != NULL && m_bmpBkOld != NULL)
	{
		::SelectObject(m_dcBk, m_bmpBkOld);
	}
	::DeleteObject(m_bmpBk);
	::DeleteDC(m_dcBk);
	m_dcBk = NULL;
	m_bmpBk = NULL;
	if(bRepaint == TRUE) 
	{
		Invalidate();
		EnableWindow(FALSE);
		EnableWindow(TRUE);
	}
}

void CJTranSliderCtrl::SetToolTip(CToolTipCtrl* pToolTip , LPCTSTR lpszTip)
{
	m_pToolTipCtrl = pToolTip;
	if(lpszTip)
	{
		m_strTip = lpszTip;
	}
	else
	{
		m_strTip.Empty();
	}

	if(::IsWindow(GetSafeHwnd()))
	{
		EnableToolTips(TRUE);
		CRect rc;
		GetClientRect(&rc);
		m_pToolTipCtrl->AddTool(this,LPSTR_TEXTCALLBACK,&rc,(UINT_PTR)m_hWnd);
	}
}

BOOL CJTranSliderCtrl::OnToolTipNotify(UINT id, NMHDR * pNMHDR, LRESULT * pResult)
{
	*pResult = 0;
	TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pNMHDR;


	//转发通知	
	pTTT->hdr.hwndFrom =  GetSafeHwnd();
	pTTT->hdr.idFrom = GetDlgCtrlID();
	//pTTT->hdr.code = TTN_NEEDTEXT;
	GetParent()->SendMessage(WM_NOTIFY,pTTT->hdr.idFrom,(LPARAM)pTTT);

	if(pTTT->szText[0] == _T('\0'))
	{
		CString strTmp;
		strTmp.Format(_T("%d"),GetPos());
		_tcsncpy(pTTT ->szText,strTmp,sizeof(pTTT ->szText)/sizeof(TCHAR) - 1);
	}

	return TRUE;
}

void CJTranSliderCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(m_pToolTipCtrl)
	{
		tagMSG msg;
		msg.hwnd= m_hWnd;
		msg.message= WM_MOUSEMOVE;
		msg.wParam= nFlags;
		msg.lParam= MAKELPARAM(LOWORD(point.x), LOWORD(point.y));
		msg.time= 0;
		msg.pt.x= point.x;
		msg.pt.y= point.y;
		m_pToolTipCtrl->RelayEvent(&msg);
	}

	CSliderCtrl::OnMouseMove(nFlags, point);
}

