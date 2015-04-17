// JPropertySheet.cpp : 实现文件
//

#include "stdafx.h"
#include "JPropertySheet.h"


// CJPropertySheet

//IMPLEMENT_DYNAMIC(CJPropertySheet, CPropertySheet)
//
//CJPropertySheet::CJPropertySheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
//	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
//{
//
//}

CJPropertySheet::CJPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
	, m_lSelectPage(iSelectPage)
{
	m_psh.dwFlags |= PSP_USETITLE;
	m_psh.pszCaption = m_strTitle[0] = pszCaption;
	m_font.CreatePointFont(90, _T("宋体"));

	m_pJLanguage = JDCUCreateLanguage();
	JLoadImage("realtime\\cu_property_title.jpg", m_imgPropertyTitle);
	JLoadImage("realtime\\cu_property_border_left.jpg", m_imgLeftBorder);
	JLoadImage("realtime\\cu_property_border_right.jpg", m_imgRightBorder);
	JLoadImage("realtime\\cu_property_border_bottom.jpg", m_imgBottomBorder);
	JLoadImage("realtime\\cu_page_title.jpg", m_imgPageTitle);
	JLoadImage("realtime\\cu_title_close.jpg", m_imgClose);
	JLoadImage("realtime\\cu_sel_title_close.jpg", m_imgSelClose);
	JLoadImage("realtime\\cu_property_btn.jpg", m_imgBtn);
	JLoadImage("realtime\\cu_property_sel_btn.jpg", m_imgSelBtn);
	m_strOKTitle = m_pJLanguage->JGetTextW(_T("public"), _T("p_OK"));
	m_strCancelTitle = m_pJLanguage->JGetTextW(_T("public"), _T("p_cancel"));
	m_lBtnState = 0;
}

CJPropertySheet::~CJPropertySheet()
{
	if(m_pJLanguage)
	{
		m_pJLanguage->JDelete();
	}
}


BEGIN_MESSAGE_MAP(CJPropertySheet, CPropertySheet)
	ON_WM_PAINT()
	ON_MESSAGE(WM_LSTCTR_MSG, OnListCtrlMsg)
	ON_BN_CLICKED(IDC_SHEET_BTN_OK, &CJPropertySheet::OnFunBtnOK)
	ON_BN_CLICKED(IDC_SHEET_BTN_CANCEL, &CJPropertySheet::OnFunBtnCancel)
	ON_WM_ERASEBKGND()
	ON_WM_NCPAINT()
	ON_WM_NCACTIVATE()
	ON_WM_NCCALCSIZE()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_NCMOUSEMOVE()
END_MESSAGE_MAP()

void CJPropertySheet::WidenSheet(int nWide)
{
	CRect rcSheet;
	GetWindowRect(&rcSheet);
	rcSheet.right += nWide;
	MoveWindow(&rcSheet);
}

void CJPropertySheet::MovePage(UINT uPage, int nToLeft)
{
	CRect rcSheet;
	GetWindowRect(&rcSheet);

	CRect rcPage;
	CPropertyPage* pPage = GetPage(uPage);
	pPage->GetWindowRect(&rcPage);
	int nWidthPage = rcPage.Width();
	rcPage.right = rcSheet.right - nToLeft;
	rcPage.left = rcPage.right - nWidthPage;
	ScreenToClient(&rcPage);
	m_rcPageClient = rcPage;
	pPage->MoveWindow(rcPage);
}

void CJPropertySheet::DrawPageCaption(CString& strCaption, CDC* pDC, COLORREF crCaption)
{
	CRect rcSheet, rcList, rcPage;
	GetClientRect(&rcSheet);
	m_lstSheet.GetWindowRect(&rcList);
	GetPage(0)->GetWindowRect(&rcPage);
	ScreenToClient(rcList);
	ScreenToClient(&rcPage);

	CRect rcCaption(rcPage);
	rcCaption.left = rcPage.left - 1;
	rcCaption.top = rcList.top - 1;
	rcCaption.right = rcPage.right + 1;
	rcCaption.bottom = rcPage.top - 1;
	//pDC->FillSolidRect(&rcCaption, RGB(255, 0, 0));

	CDC dcMem;
	CBitmap bmp;
	dcMem.CreateCompatibleDC(pDC);
	bmp.CreateCompatibleBitmap(pDC, rcCaption.right, rcSheet.Height());
	dcMem.SelectObject(&bmp);

	//渐变色调
	if(m_imgPageTitle.IsNull())
	{
		DrawGradualChange(&dcMem, rcCaption, RGB(92, 132, 255));
	}
	else
	{
		DrawGradualChangeEx(&dcMem, rcCaption, RGB(92, 132, 255));
	}
	
	//文本
	CRect rcText(rcCaption);
	rcText.left = rcCaption.left + 26;
	rcText.top = rcCaption.top + 5;
	DrawCaptionText(&dcMem, strCaption, rcText);

	//图标
	::DrawIconEx(dcMem.GetSafeHdc(), rcCaption.left + 4, rcCaption.top + 3\
		, m_lstSheet.GetItemIcon(m_lSelectPage), 16, 16, NULL, NULL, DI_NORMAL);

	pDC->BitBlt(rcCaption.left, rcCaption.top, rcCaption.Width(), rcCaption.Height(), &dcMem, rcCaption.left, rcCaption.top, SRCCOPY);
}

void CJPropertySheet::DrawGradualChange(CDC* pDC, const CRect& rc, COLORREF crChange)
{
	int nRBase = GetRValue(crChange);
	int nGBase = GetGValue(crChange);
	int nBBase = GetBValue(crChange);

	int nRCur = nRBase;
	int nGCur = nGBase;
	int nBCur = nBBase;

	double dIncR = (double)(255 - nRBase) / (double)rc.Width();
	double dIncG = (double)(255 - nGBase) / (double)rc.Width();
	double dIncB = (double)(255 - nBBase) / (double)rc.Width();

	CRect rcCaption(rc);
	int nLeft = rcCaption.left;
	int nRight = rcCaption.left + 1;
	for(; nLeft < rc.right; nLeft++, nRight++)
	{
		rcCaption.left = nLeft;
		rcCaption.right = nRight;
		pDC->FillSolidRect(&rcCaption, RGB(nRCur, nGCur, nBCur));

		nRCur = (int)((double)(nLeft - rc.left) * dIncR + nRBase);
		nGCur = (int)((double)(nLeft - rc.left) * dIncG + nGBase);
		nBCur = (int)((double)(nLeft - rc.left) * dIncB + nBBase);
	}
}

void CJPropertySheet::DrawGradualChangeEx(CDC* pDC, const CRect& rc, COLORREF crChange)
{
	for(int nX = rc.left; nX < rc.right;)
	{
		m_imgPageTitle.Draw(pDC->GetSafeHdc(), nX, rc.top);
		nX += m_imgPageTitle.GetWidth();
	}
}

void CJPropertySheet::DrawCaptionText(CDC* pDC, CString& strText, const CRect& rc)
{
	int nBKMode = pDC->SetBkMode(TRANSPARENT);
	CFont ft;
	ft.CreatePointFont(100, _T("宋体"), pDC);
	pDC->SelectObject(&ft);
	pDC->SetTextColor(RGB(100, 100, 100));
	pDC->TextOut(rc.left, rc.top, (LPCTSTR)strText, strText.GetLength());
	pDC->SetTextColor(RGB(200, 200, 200));
	pDC->TextOut(rc.left - 1, rc.top - 1, (LPCTSTR)strText, strText.GetLength());
}

void CJPropertySheet::HideSysBtn()
{
	GetDlgItem(IDOK)->ShowWindow(SW_HIDE);
	GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);
	GetDlgItem(ID_APPLY_NOW)->ShowWindow(SW_HIDE);
	GetDlgItem(IDHELP)->ShowWindow(SW_HIDE);
}

void CJPropertySheet::SetBtnText(LPCTSTR lpOKText, LPCTSTR lpCancelText)
{
	m_btnOK.SetWindowText(lpOKText);
	m_btnCancel.SetWindowText(lpCancelText);
}
// CJPropertySheet 消息处理程序

BOOL CJPropertySheet::OnInitDialog()
{
	BOOL bResult = CPropertySheet::OnInitDialog();

	// TODO:  在此添加您的专用代码
	CRect rcTable;
	CTabCtrl* pTable = GetTabControl();
	pTable->GetWindowRect(&rcTable);
	ScreenToClient(&rcTable);
	if(pTable)
	{
		pTable->ModifyStyle(TCS_MULTILINE, TCS_SINGLELINE);//, SWP_FRAMECHANGED|SWP_DRAWFRAME);//单行模式
		SetActivePage(m_lSelectPage); 
		pTable->ShowWindow(SW_HIDE);
	}

	WidenSheet();
	MovePage(m_lSelectPage);
	HideSysBtn();

	int nLeft = m_imgLeftBorder.GetWidth() + 5;
	m_lstSheet.Create(WS_CHILD, CRect(nLeft, rcTable.top, INC_SHEET_WIDE - 10 - nLeft, m_rcPageClient.bottom), this, 0xFFFF);
	int nPageCount = GetPageCount();
	for(int nPage = 0; nPage < nPageCount; nPage++)
	{
		PROPSHEETPAGE stProPage;stProPage = GetPage(nPage)->GetPSP();
		m_lstSheet.AddItemText(nPage, (LPWSTR)stProPage.pszTitle);
		if(nPage == m_lSelectPage)
		{
			m_strActiveTitle = stProPage.pszTitle;
		}
	}
	for(int nItem = 0; nItem < (int)m_vtPageIcon.size(); nItem++)
	{
		m_lstSheet.AddItemIcon(nItem, m_vtPageIcon[nItem]);
	}

	CRect rcBtn;
	rcBtn.left = m_rcPageClient.right / 2;
	rcBtn.right = rcBtn.left + 70;
	rcBtn.top = m_rcPageClient.bottom + 5;
	rcBtn.bottom = rcBtn.top + 23;
	m_btnOK.Create(NULL, WS_CHILD | WS_VISIBLE, rcBtn, this, IDC_SHEET_BTN_OK);
	m_btnOK.SetImage(&m_imgBtn, &m_imgSelBtn);
	m_btnOK.SetWindowText(m_strOKTitle);
	m_btnOK.ShowWindow(SW_SHOW);

	rcBtn.left = rcBtn.right + 12;
	rcBtn.right = rcBtn.left + 70;
	m_btnCancel.Create(NULL, WS_CHILD | WS_VISIBLE, rcBtn, this, IDC_SHEET_BTN_CANCEL);
	m_btnCancel.SetImage(&m_imgBtn, &m_imgSelBtn);
	m_btnCancel.SetWindowText(m_strCancelTitle);
	m_btnCancel.ShowWindow(SW_SHOW);

	return bResult;
}

void CJPropertySheet::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CPropertySheet::OnPaint()
	CBrush br(RGB(144, 144, 144));
	CRect rcPage;
	GetPage(0)->GetWindowRect(rcPage);
	ScreenToClient(&rcPage);
	rcPage.left		-= 1;
	rcPage.top		-= 1;
	rcPage.right	+= 1;
	rcPage.bottom	+= 1;
	dc.FrameRect(&rcPage, &br);

	//
	CRect rc;
	m_lstSheet.GetWindowRect(&rc);
	ScreenToClient(&rc);
	rc.left		-= 1;
	rc.top		-= 1;
	rc.right	+= 1;
	rc.bottom	+= 1;
	dc.FrameRect(&rc, &br);

	DrawPageCaption(m_strActiveTitle, &dc, RGB(0, 0, 0));
}

LRESULT CJPropertySheet::OnListCtrlMsg(WPARAM wParam, LPARAM lParam)
{
	PST_LSTCTR_MSG pMsg = (PST_LSTCTR_MSG)lParam;
	if(NULL != pMsg)
	{
		pMsg->lResult = 0;
		if(pMsg->uMsg == WM_LSTCTR_CLICK)
		{
			if(pMsg->nSelectItem >= 0 && m_lSelectPage != pMsg->nSelectItem)
			{
				AfxGetApp()->BeginWaitCursor();
				m_strActiveTitle = m_lstSheet.GetItemText(pMsg->nSelectItem, 0);
				SetActivePage(pMsg->nSelectItem);
				MovePage(pMsg->nSelectItem);
				m_rcPageClient.left = 0;
				m_rcPageClient.top -= 20;
				InvalidateRect(&m_rcPageClient);
				m_lSelectPage = pMsg->nSelectItem;
				AfxGetApp()->EndWaitCursor();
			}
		}
		m_lstSheet.SetFocus();
		return pMsg->lResult;
	}
	return -1;
}

void CJPropertySheet::AddPage(CPropertyPage* pPage, HICON hIcon)
{
	m_vtPageIcon.push_back(hIcon);
	CPropertySheet::AddPage(pPage);
}

void CJPropertySheet::OnFunBtnOK()
{
	SendMessage(WM_CLOSE);
}

void CJPropertySheet::OnFunBtnCancel()
{
	SendMessage(WM_CLOSE);
}

BOOL CJPropertySheet::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	RECT rc;
	GetClientRect(&rc);
	CBrush br(RGB(235, 236, 237));
	pDC->FillRect(&rc, &br);
	return TRUE;//CPropertySheet::OnEraseBkgnd(pDC);
}

void CJPropertySheet::OnNcPaint()
{
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CPropertySheet::OnNcPaint()
	CDC* pDC = GetWindowDC();
	CRect rcWin;
	GetWindowRect(&rcWin);
	rcWin.right = rcWin.right - rcWin.left;
	rcWin.bottom = rcWin.bottom - rcWin.top;
	rcWin.top = 0;
	rcWin.left = 0;
	for(int nX = 0; nX < rcWin.right;)
	{
		m_imgPropertyTitle.Draw(pDC->GetSafeHdc(), nX, rcWin.top, m_imgPropertyTitle.GetWidth(), m_imgPropertyTitle.GetHeight());
		nX += m_imgPropertyTitle.GetWidth();
	}

	CRect rcLeftBorder(rcWin);
	rcLeftBorder.right = rcLeftBorder.left + m_imgLeftBorder.GetWidth();
	rcLeftBorder.top = rcWin.top + m_imgPropertyTitle.GetHeight();
	for(int nY = rcLeftBorder.top; nY < rcLeftBorder.bottom;)
	{
		m_imgLeftBorder.Draw(pDC->GetSafeHdc(), rcLeftBorder.left, nY, m_imgLeftBorder.GetWidth(), m_imgLeftBorder.GetHeight());
		nY += m_imgLeftBorder.GetHeight();
	}

	CRect rcRightBorder(rcWin);
	rcRightBorder.left = rcRightBorder.right -  m_imgRightBorder.GetWidth();
	rcRightBorder.top += m_imgPropertyTitle.GetHeight();
	for(int nY = rcRightBorder.top; nY < rcRightBorder.bottom;)
	{
		m_imgRightBorder.Draw(pDC->GetSafeHdc(), rcRightBorder.left, nY, m_imgRightBorder.GetWidth(), m_imgRightBorder.GetHeight());
		nY += m_imgRightBorder.GetHeight();
	}

	CRect rcBottomBorder(rcWin);
	rcBottomBorder.left += m_imgLeftBorder.GetWidth();
	rcBottomBorder.top = rcBottomBorder.bottom - m_imgBottomBorder.GetHeight();
	rcBottomBorder.right -= m_imgRightBorder.GetWidth();
	for(int nX = rcBottomBorder.left; nX < rcBottomBorder.right;)
	{
		m_imgBottomBorder.Draw(pDC->GetSafeHdc(), nX, rcBottomBorder.top, m_imgBottomBorder.GetWidth(), m_imgBottomBorder.GetHeight());
		nX += m_imgBottomBorder.GetWidth();
	}

	CRect rcCloseBtn(rcWin);
	rcCloseBtn.left = rcCloseBtn.right - m_imgClose.GetWidth() - 3;
	rcCloseBtn.top += 2;
	rcCloseBtn.bottom = rcCloseBtn.top + m_imgClose.GetHeight();
	if(m_lBtnState == 0)
	{
		m_imgClose.Draw(pDC->GetSafeHdc(), rcCloseBtn.left, rcCloseBtn.top, rcCloseBtn.Width(), rcCloseBtn.Height());
	}
	else if(m_lBtnState == 1)
	{
		m_imgSelClose.Draw(pDC->GetSafeHdc(), rcCloseBtn.left, rcCloseBtn.top);
	}

	ReleaseDC(pDC);
}

BOOL CJPropertySheet::OnNcActivate(BOOL bActive)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	BOOL bRet = TRUE;//CPropertySheet::OnNcActivate(bActive);
	if(bActive)
	{
		OnNcPaint();
	}
	return bRet;
}

void CJPropertySheet::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CPropertySheet::OnNcCalcSize(bCalcValidRects, lpncsp);
	if(bCalcValidRects)
	{
		CRect rc = (CRect&)lpncsp->rgrc[0];
		rc.top += m_imgPropertyTitle.GetHeight() - GetSystemMetrics(SM_CYCAPTION) - GetSystemMetrics(SM_CYBORDER) - 2;
		rc.left += m_imgLeftBorder.GetWidth() - GetSystemMetrics(SM_CYBORDER) - 2;
		rc.right -= m_imgRightBorder.GetWidth() - GetSystemMetrics(SM_CYBORDER) - 2;
		rc.bottom -= m_imgBottomBorder.GetHeight() - GetSystemMetrics(SM_CYBORDER) - 2;
		lpncsp->rgrc[0] = rc;
	}
}

void CJPropertySheet::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	long lPreState = m_lBtnState;
	CRect rcWin;
	GetWindowRect(&rcWin);
	
	CRect rcCloseBtn;
	rcCloseBtn.left = rcWin.right - m_imgClose.GetWidth() - 3;
	rcCloseBtn.right = rcCloseBtn.left + m_imgClose.GetWidth();
	rcCloseBtn.top = rcWin.top;
	rcCloseBtn.bottom = rcCloseBtn.top + m_imgClose.GetHeight();
	if(rcCloseBtn.PtInRect(point))
	{
		m_lBtnState = 0;
		SendMessage(WM_CLOSE);
		return;
	}

	CPropertySheet::OnNcLButtonDown(nHitTest, point);
}

void CJPropertySheet::OnNcMouseMove(UINT nHitTest, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	long lPreState = m_lBtnState;
	CRect rcWin;
	GetWindowRect(&rcWin);
	
	CRect rcCloseBtn;
	rcCloseBtn.left = rcWin.right - m_imgClose.GetWidth() - 3;
	rcCloseBtn.right = rcCloseBtn.left + m_imgClose.GetWidth();
	rcCloseBtn.top = rcWin.top;
	rcCloseBtn.bottom = rcCloseBtn.top + m_imgClose.GetHeight();
	if(rcCloseBtn.PtInRect(point))
	{
		m_lBtnState = 1;
	}
	else
	{
		m_lBtnState = 0;
	}
	if(m_lBtnState == lPreState)
	{
		return;
	}

	CDC* pDC = GetWindowDC();

	if(m_lBtnState == 1)
	{
		m_imgSelClose.Draw(pDC->GetSafeHdc(), rcCloseBtn.left - rcWin.left + 1, rcCloseBtn.top - rcWin.top + 2);
	}
	if(lPreState == 1)
	{
		m_imgClose.Draw(pDC->GetSafeHdc(), rcCloseBtn.left - rcWin.left + 1, rcCloseBtn.top - rcWin.top + 2);
	}

	ReleaseDC(pDC);

	CPropertySheet::OnNcMouseMove(nHitTest, point);
}
