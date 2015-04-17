// JPublicDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "JPublicDlg.h"


// CJPublicDlg 对话框

IMPLEMENT_DYNAMIC(CJPublicDlg, CDialog)

CJPublicDlg::CJPublicDlg(UINT nIDTemplate, CWnd* pParent /*=NULL*/)
	: CDialog(nIDTemplate, pParent)
	, m_lBtnState(0)
	, m_lWidth(-1)
	, m_lHeight(-1)
	, m_lLeft(0)
	, m_lBtnInterval(20)
	, m_strText(_T(""))
{
	m_font.CreateFont(13,0,0,0,FW_NORMAL,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,_T("宋体"));
	m_crBK = RGB(235, 236, 237);
	m_crText = RGB(230, 230, 230);
	m_strOKText = _T("确定");
	m_strCancel = _T("取消");
	
	JLoadImage("realtime\\cu_property_title.jpg", m_imgPropertyTitle);
	JLoadImage("realtime\\cu_property_border_left.jpg", m_imgLeftBorder);
	JLoadImage("realtime\\cu_property_border_right.jpg", m_imgRightBorder);
	JLoadImage("realtime\\cu_property_border_bottom.jpg", m_imgBottomBorder);
	JLoadImage("realtime\\cu_page_title.jpg", m_imgPageTitle);
	JLoadImage("realtime\\cu_title_close.jpg", m_imgClose);
	JLoadImage("realtime\\cu_sel_title_close.jpg", m_imgSelClose);
	JLoadImage("realtime\\cu_property_btn.jpg", m_imgBtn);
	JLoadImage("realtime\\cu_property_sel_btn.jpg", m_imgSelBtn);
}

CJPublicDlg::~CJPublicDlg()
{
}

void CJPublicDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CJPublicDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_NCPAINT()
	ON_WM_NCACTIVATE()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_NCMOUSEMOVE()
	ON_WM_NCCALCSIZE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_NCRBUTTONDOWN()
END_MESSAGE_MAP()

void CJPublicDlg::SetSize(long lWidth, long lHeight)
{
	m_lWidth = lWidth;
	m_lHeight = lHeight;
}

void CJPublicDlg::HideSysBtn()
{
	GetDlgItem(IDOK)->EnableWindow(FALSE);
	GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
	GetDlgItem(IDOK)->ShowWindow(SW_HIDE);
	GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);
}

void CJPublicDlg::SetBtnText(const CString& strOK, const CString& strCancel)
{
	m_strOKText = strOK;
	m_strCancel = strCancel;
}

// CJPublicDlg 消息处理程序

void CJPublicDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CDialog::OnPaint()
}

void CJPublicDlg::OnNcPaint()
{
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CDialog::OnNcPaint()
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

	CFont* pOldFont = pDC->SelectObject(&m_font);
	CString strTitle;GetWindowText(strTitle);
	int nOldBKMode = pDC->SetBkMode(TRANSPARENT);
	COLORREF crOldText = pDC->SetTextColor(m_crText);
	pDC->TextOut(rcWin.left + 10, rcWin.top + 5, strTitle);
	pDC->SetTextColor(crOldText);
	pDC->SetBkMode(nOldBKMode);
	pDC->SelectObject(pOldFont);

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

BOOL CJPublicDlg::OnNcActivate(BOOL bActive)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	//return CDialog::OnNcActivate(bActive);
	BOOL bRet = TRUE;
	if(bActive)
	{
		OnNcPaint();
	}
	return bRet;
}

void CJPublicDlg::OnNcLButtonDown(UINT nHitTest, CPoint point)
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
		OnCancel();
		return;
	}

	CDialog::OnNcLButtonDown(nHitTest, point);
}

void CJPublicDlg::OnNcMouseMove(UINT nHitTest, CPoint point)
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

	CDialog::OnNcMouseMove(nHitTest, point);
}

void CJPublicDlg::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialog::OnNcCalcSize(bCalcValidRects, lpncsp);
	if(bCalcValidRects)
	{
		CRect rc = (CRect&)lpncsp->rgrc[0];
		rc.top += m_imgPropertyTitle.GetHeight() - ::GetSystemMetrics(SM_CYCAPTION) - ::GetSystemMetrics(SM_CYBORDER) - 2;
		rc.left += m_imgLeftBorder.GetWidth() - ::GetSystemMetrics(SM_CXBORDER) - 2;
		rc.right -= m_imgRightBorder.GetWidth() - ::GetSystemMetrics(SM_CXBORDER) - 2;
		rc.bottom -= m_imgBottomBorder.GetHeight() - ::GetSystemMetrics(SM_CYBORDER) - 2;
		lpncsp->rgrc[0] = rc;
	}
}

BOOL CJPublicDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CRect rc;
	GetClientRect(&rc);
	pDC->FillSolidRect(&rc, m_crBK);
	return TRUE;//CDialog::OnEraseBkgnd(pDC);
}

BOOL CJPublicDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	pSysMenu->DeleteMenu(SC_CLOSE, MF_BYCOMMAND);
	//pSysMenu->DeleteMenu(SC_MOVE, MF_BYCOMMAND);
	//pSysMenu->EnableMenuItem(SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);

	HideSysBtn();

	CRect rc;
	GetClientRect(&rc);

	CRect rcCancel;
	rcCancel.left = rc.right - m_imgBtn.GetWidth() - 11 - m_lLeft;
	rcCancel.right = rcCancel.left + m_imgBtn.GetWidth();
	rcCancel.top = rc.bottom - m_imgBtn.GetHeight() - 11;
	rcCancel.bottom = rcCancel.top + m_imgBtn.GetHeight();
	m_btnCancel.SetImage(&m_imgBtn, &m_imgSelBtn);
	BOOL bRet = m_btnCancel.Create(NULL, WS_CHILD | WS_VISIBLE, rcCancel, this, IDC_BTN_CANCEL);
	m_btnCancel.SetWindowText(m_strCancel);
	m_btnCancel.ShowWindow(SW_SHOW);

	CRect rcOK(rcCancel);
	rcOK.right = rcOK.left - m_lBtnInterval;
	rcOK.left = rcOK.right - m_imgSelBtn.GetWidth();
	rcOK.top = rcOK.bottom - m_imgSelBtn.GetHeight();
	m_btnOK.SetImage(&m_imgBtn, &m_imgSelBtn);
	BOOL bRet2 = m_btnOK.Create(NULL, WS_CHILD | WS_VISIBLE, rcOK, this, IDC_BTN_OK);
	m_btnOK.SetWindowText(m_strOKText);
	m_btnOK.ShowWindow(SW_SHOW);
	

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CJPublicDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	m_lWidth = max(m_lWidth, cx);
	m_lHeight = max(m_lHeight, cy);
	MoveWindow(0, 0, m_lWidth + GetSystemMetrics(SM_CXBORDER), m_lHeight + GetSystemMetrics(SM_CYCAPTION));// + GetSystemMetrics(SM_CYBORDER));
}

void CJPublicDlg::OnNcRButtonDown(UINT nHitTest, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	return;
	//CDialog::OnNcRButtonDown(nHitTest, point);
}

void CJPublicDlg::PreSubclassWindow()
{
	// TODO: 在此添加专用代码和/或调用基类
	CDialog::PreSubclassWindow();
}
