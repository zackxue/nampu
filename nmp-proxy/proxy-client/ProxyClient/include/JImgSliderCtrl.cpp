// ImgSliderCtrl.cpp : 实现文件
//

#include "stdafx.h"
#include "JImgSliderCtrl.h"


// CJImgSliderCtrl

CJImgSliderCtrl::CJImgSliderCtrl()
{
	m_bCreate = FALSE;
	m_pImgOuter = NULL;
	m_lMarginPixel = 0;
	m_lThumbMarginPixel = 0;
	m_bVERT = TRUE;
	m_lRangeMin = 0;
	m_lRangeMax = 100;
	m_lCurPos = 0;
	m_lLineSize = 5;
	m_bMoving = FALSE;
	m_pbtThumb = new CJThumbButton(this);
	m_crProBKColor = RGB(0, 0, 0);
	m_bEnableProcBK = FALSE;
}

CJImgSliderCtrl::~CJImgSliderCtrl()
{
	delete m_pbtThumb;
}


BEGIN_MESSAGE_MAP(CJImgSliderCtrl, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_MESSAGE(WM_ENABLE,OnEnable)
	ON_NOTIFY(TTN_NEEDTEXT,100,OnToolTipNotify )
	ON_WM_CREATE()
END_MESSAGE_MAP()



// CJImgSliderCtrl 消息处理程序

BOOL CJImgSliderCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	m_bCreate = TRUE;
	return CWnd::Create(NULL,_T(""),dwStyle,rect,pParentWnd,nID);
}

void CJImgSliderCtrl::SetImage(const char* szImgName)
{
	m_pImgOuter = NULL;
	if(szImgName)
	{
		JLoadImage(szImgName,m_image);
	}
	else
	{
		m_image.Destroy();
	}
	
	if(IsWindow(GetSafeHwnd()))
	{
		//Invalidate();
		CRect rc;
		GetWindowRect(&rc);
		GetParent()->ScreenToClient(&rc);
		rc.right = rc.left + m_image.GetWidth();
		rc.bottom = rc.top + m_image.GetHeight();

		MoveWindow(rc,FALSE);
		Invalidate();
	}
}

void CJImgSliderCtrl::SetImage(CImage* pImg)
{
	m_image.Destroy();
	m_pImgOuter = pImg;

	if(IsWindow(GetSafeHwnd()))
	{
		//Invalidate();
		CRect rc;
		GetWindowRect(&rc);
		GetParent()->ScreenToClient(&rc);
		rc.right = rc.left + m_pImgOuter->GetWidth();
		rc.bottom = rc.top + m_pImgOuter->GetHeight();

		MoveWindow(rc,FALSE);
		Invalidate();
	}
}

CImage* CJImgSliderCtrl::GetImage()
{
	if(m_pImgOuter)
	{
		return m_pImgOuter;
	}
	else
	{
		if(m_image.IsNull())
		{
			return NULL;
		}
		else
		{
			return &m_image;
		}
	}
}

void CJImgSliderCtrl::SetThumbImage(const char* szImgNameNormal , const char* szImgNameSelect, int nJpgLeft, int nJpgTop)
{
	m_pbtThumb->SetImage(szImgNameNormal,szImgNameSelect,nJpgLeft,nJpgTop);
}

void CJImgSliderCtrl::SetThumbImage(CImage* pImgNormal, CImage* pImgSelect, int nJpgLeft, int nJpgTop)
{
	m_pbtThumb->SetImage(pImgNormal,pImgSelect,nJpgLeft,nJpgTop);
}

void CJImgSliderCtrl::SetVert(BOOL bVERT)
{
	m_bVERT = bVERT;
}

void CJImgSliderCtrl::SetToolTip(CToolTipCtrl* pToolTip)
{
	m_pbtThumb->SetToolTip(pToolTip);
}

long CJImgSliderCtrl::GetImgWidth()
{
	if(GetImage())
	{
		return GetImage()->GetWidth();
	}
	else
	{
		return 0;
	}
}

long CJImgSliderCtrl::GetImgHeight()
{
	if(GetImage())
	{
		return GetImage()->GetHeight();
	}
	else
	{
		return 0;
	}
}

void CJImgSliderCtrl::SetThumbRectMargin(long lPixel)
{
	m_lMarginPixel = lPixel;
	if(GetSafeHwnd())
	{
		UpdateThumbPlace();
	}
}

void CJImgSliderCtrl::SetThumbImgMargin(long lPixel)
{
	m_lThumbMarginPixel = lPixel;
	if(GetSafeHwnd())
	{
		UpdateThumbPlace();
	}
}

void CJImgSliderCtrl::SetRange(long lMin, long lMax)
{
	ASSERT(lMin < lMax);
	m_lRangeMin = lMin;
	m_lRangeMax = lMax;

}

void CJImgSliderCtrl::GetRange(long& lMin, long& lMax)
{
	lMin = m_lRangeMin;
	lMax = m_lRangeMax;
}

void CJImgSliderCtrl::SetPos(long lPos)
{
	m_lCurPos = lPos;

	if(m_lCurPos < m_lRangeMin)
	{
		m_lCurPos = m_lRangeMin;
	}
	if(m_lCurPos > m_lRangeMax)
	{
		m_lCurPos = m_lRangeMax;
	}

	if(GetSafeHwnd())
	{
		UpdateThumbPlace();
	}
}

long CJImgSliderCtrl::GetPos()
{
	return m_lCurPos;
}

long CJImgSliderCtrl::SetLineSize(long lSize)
{
	long lLineSize = m_lLineSize;
	m_lLineSize = lSize;
	return lLineSize;
}

BOOL CJImgSliderCtrl::IsMovingPos()
{
	return m_bMoving;
}

void CJImgSliderCtrl::UpdateThumbPlace()
{
	CRect rcSlider;
	GetClientRect(&rcSlider);
	CRect rcThumb;
	m_pbtThumb->GetWindowRect(&rcThumb);
	ScreenToClient(&rcThumb);

	int nThumbWidth = rcThumb.Width();
	int nThumbHeight = rcThumb.Height();
	//
	if(m_bVERT)
	{
		float fPos = float(m_lCurPos - m_lRangeMin)/(m_lRangeMax - m_lRangeMin);
		
		rcThumb.left = (long)(rcSlider.left + m_lMarginPixel + fPos*(float)(rcSlider.Width() - nThumbWidth - m_lMarginPixel*2));
		rcThumb.right = rcThumb.left + nThumbWidth;

		rcThumb.top = rcSlider.top + m_lThumbMarginPixel;// + (rcSlider.Height() - nThumbHeight)/2;
		rcThumb.bottom = rcThumb.top + nThumbHeight;
	}
	else
	{
		float fPos = (float)(m_lCurPos - m_lRangeMin)/(m_lRangeMax - m_lRangeMin);

		//rcThumb.top = rcSlider.top + m_lMarginPixel + fPos*(rcSlider.Height() - nThumbHeight - m_lMarginPixel*2);
		//rcThumb.bottom = rcThumb.top + nThumbHeight;
		rcThumb.bottom = (long)(rcSlider.bottom - m_lMarginPixel - fPos*(rcSlider.Height() - nThumbHeight - m_lMarginPixel*2));
		rcThumb.top = rcThumb.bottom - nThumbHeight;

		rcThumb.left = rcSlider.left + m_lThumbMarginPixel;// + (rcSlider.Width() - nThumbWidth)/2;
		rcThumb.right = rcThumb.left + nThumbWidth;

	}

	m_pbtThumb->MoveWindow(&rcThumb);

}

void CJImgSliderCtrl::OnThumbMove(const CPoint& ptPre, const CPoint& ptCur,BOOL bSendChangedMsg)
{
	CRect rcSlider;
	GetClientRect(&rcSlider);

	CRect rc;
	m_pbtThumb->GetWindowRect(&rc);
	ScreenToClient(&rc);

	if(m_bVERT)
	{
		rc.left += ptCur.x - ptPre.x;
		rc.right += ptCur.x - ptPre.x;

		if(rc.left < m_lMarginPixel)
		{
			rc.right = m_lMarginPixel + rc.Width();
			rc.left = m_lMarginPixel;
		}
		else if(rc.right > (rcSlider.right - m_lMarginPixel))
		{
			rc.left = rcSlider.right - m_lMarginPixel - rc.Width();
			rc.right = rcSlider.right - m_lMarginPixel;
		}
	}
	else
	{
		rc.top += ptCur.y - ptPre.y;
		rc.bottom += ptCur.y - ptPre.y;

		if(rc.top < m_lMarginPixel)
		{
			rc.bottom = m_lMarginPixel + rc.Height();
			rc.top = m_lMarginPixel;
		}
		else if(rc.bottom > (rcSlider.bottom - m_lMarginPixel))
		{
			rc.top = rcSlider.bottom - m_lMarginPixel - rc.Height();
			rc.bottom = rcSlider.bottom - m_lMarginPixel;
		}
	}

	m_pbtThumb->MoveWindow(&rc);

	//CRect rcSlider;
	GetClientRect(&rcSlider);
	CRect rcThumb;
	m_pbtThumb->GetWindowRect(&rcThumb);
	ScreenToClient(&rcThumb);

	float fPos = 0;
	if(m_bVERT)
	{
		fPos = float(rcThumb.left - rcSlider.left - m_lMarginPixel)/(rcSlider.Width() - m_lMarginPixel*2 - rcThumb.Width());
	}
	else
	{
		//fPos = (rcThumb.top - rcSlider.top - m_lMarginPixel)/(rcSlider.Height() - m_lMarginPixel*2);
		fPos = float(rcSlider.bottom - rcThumb.bottom - m_lMarginPixel)/(rcSlider.Height() - m_lMarginPixel*2 - rcThumb.Height());
	}

	m_lCurPos = (long)(m_lRangeMin + (m_lRangeMax - m_lRangeMin)*fPos);

	if(m_lCurPos < m_lRangeMin)
	{
		m_lCurPos = m_lRangeMin;
	}
	if(m_lCurPos > m_lRangeMax)
	{
		m_lCurPos = m_lRangeMax;
	}

	rcSlider.right = rcThumb.left;
	DrawProColor(rcSlider);

	if(bSendChangedMsg)
	{
		//发送通知
		NMHDR hdr;
		hdr.hwndFrom = GetSafeHwnd();
		hdr.idFrom = GetDlgCtrlID();
		hdr.code = NM_THEMECHANGED;
		GetParent()->SendMessage(WM_NOTIFY,GetDlgCtrlID(),(LPARAM)&hdr);
	}
}

void CJImgSliderCtrl::DrawProColor(CRect& rcProc)
{
	if(!m_bEnableProcBK)
	{
		return;
	}

	m_rcCurPro = rcProc;

	CRect rcRight(rcProc);
	rcRight.left = rcProc.right;
	InvalidateRect(&rcRight);
	CDC* pDC = GetDC();
	rcProc.top += 1;
	pDC->FillSolidRect(&rcProc, m_crProBKColor);
	ReleaseDC(pDC);
}

void CJImgSliderCtrl::OnThumbRelease(const CPoint& ptPre, const CPoint& ptCur)
{
	OnThumbMove(ptPre,ptCur,FALSE);

	//发送通知
	NMHDR hdr;
	hdr.hwndFrom = GetSafeHwnd();
	hdr.idFrom = GetDlgCtrlID();
	hdr.code = NM_RELEASEDCAPTURE;
	GetParent()->SendMessage(WM_NOTIFY,GetDlgCtrlID(),(LPARAM)&hdr);

}

BOOL CJImgSliderCtrl::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	return TRUE;

	//return CWnd::OnEraseBkgnd(pDC);
}

void CJImgSliderCtrl::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CWnd::OnPaint()
	if(GetImage())
	{
		GetImage()->Draw(dc,0,0);
	}

	if(m_lCurPos)
	{
		DrawProColor(m_rcCurPro);
	}

	m_pbtThumb->UpdateWindow();
}

int CJImgSliderCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	m_bCreate = TRUE;

	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码

	m_pbtThumb->Create(_T(""),WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,CRect(0,0,m_pbtThumb->GetImgWidth(),m_pbtThumb->GetImgHeight()),this,100);

	CRect rc;
	GetWindowRect(&rc);
	GetParent()->ScreenToClient(&rc);
	rc.right = rc.left + GetImage()->GetWidth();
	rc.bottom = rc.top + GetImage()->GetHeight();

	MoveWindow(rc);
	OnSize(0,rc.Width(),rc.Height());

	m_pbtThumb->EnableWindow(IsWindowEnabled());

	return 0;
}

void CJImgSliderCtrl::PreSubclassWindow()
{
	// TODO: 在此添加专用代码和/或调用基类
	CWnd::PreSubclassWindow();

	if(!m_bCreate)
	{

		m_pbtThumb->Create(_T(""),WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,CRect(0,0,m_pbtThumb->GetImgWidth(),m_pbtThumb->GetImgHeight()),this,100);

		CRect rc;
		GetWindowRect(&rc);
		GetParent()->ScreenToClient(&rc);
		rc.right = rc.left + GetImage()->GetWidth();
		rc.bottom = rc.top + GetImage()->GetHeight();

		MoveWindow(rc);
		OnSize(0,rc.Width(),rc.Height());

		m_pbtThumb->EnableWindow(IsWindowEnabled());
	}
	
}

void CJImgSliderCtrl::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	if(m_pbtThumb->GetSafeHwnd())
	{
		UpdateThumbPlace();
	}

}

void CJImgSliderCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	// TODO: 在此添加消息处理程序代码和/或调用默认值
	CRect rcSlider;
	GetClientRect(&rcSlider);
	CRect rcThumb;
	m_pbtThumb->GetWindowRect(&rcThumb);
	ScreenToClient(&rcThumb);

	CRect rcLeft,rcRight;
	if(m_bVERT)
	{
		rcLeft.left = rcSlider.left + m_lMarginPixel;
		rcLeft.right = rcThumb.left;
		rcLeft.top = rcThumb.top;
		rcLeft.bottom = rcThumb.bottom;
		//
		rcRight.left = rcThumb.right;
		rcRight.right = rcSlider.right - m_lMarginPixel;
		rcRight.top = rcThumb.top;
		rcRight.bottom = rcThumb.bottom;
	}
	else
	{
		rcLeft.left = rcThumb.left;
		rcLeft.right = rcThumb.right;
		rcLeft.top = rcThumb.bottom;
		rcLeft.bottom = rcSlider.bottom - m_lMarginPixel;
		//
		rcRight.left = rcThumb.left;
		rcRight.right = rcThumb.right;
		rcRight.top = rcSlider.top + m_lMarginPixel;
		rcRight.bottom = rcThumb.top;
	}

	if(rcLeft.PtInRect(point))
	{
		m_lCurPos -= m_lLineSize;
	}
	else if(rcRight.PtInRect(point))
	{
		m_lCurPos += m_lLineSize;
	}
	else
	{
		CWnd::OnLButtonDown(nFlags, point);
		return;
	}

	if(m_lCurPos < m_lRangeMin)
	{
		m_lCurPos = m_lRangeMin;
	}
	if(m_lCurPos > m_lRangeMax)
	{
		m_lCurPos = m_lRangeMax;
	}

	rcSlider.right = rcThumb.left;
	DrawProColor(rcSlider);

	UpdateThumbPlace();

	//发送通知
	NMHDR hdr;
	hdr.hwndFrom = GetSafeHwnd();
	hdr.idFrom = GetDlgCtrlID();
	hdr.code = NM_RELEASEDCAPTURE;
	GetParent()->SendMessage(WM_NOTIFY,GetDlgCtrlID(),(LPARAM)&hdr);

	CWnd::OnLButtonDown(nFlags, point);

}

LRESULT CJImgSliderCtrl::OnEnable(WPARAM wParam,LPARAM lParam)
{
	LRESULT lRet = DefWindowProc(WM_ENABLE,wParam,lParam);

	m_pbtThumb->EnableWindow(wParam);
	return lRet;
}

void CJImgSliderCtrl::OnToolTipNotify(NMHDR * pNMHDR, LRESULT * pResult)
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
}


// CJThumbButton
CJThumbButton::CJThumbButton(CJImgSliderCtrl* pSliderWnd)
{
	m_pSliderWnd = pSliderWnd;
	m_bCapture = FALSE;
}

CJThumbButton::~CJThumbButton()
{
}


BEGIN_MESSAGE_MAP(CJThumbButton, CImgButton)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()



// CJThumbButton 消息处理程序

void CJThumbButton::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	SetCapture();

	m_pSliderWnd->m_bMoving = TRUE;

	CPoint pt;
	::GetCursorPos(&pt);

	m_bCapture = TRUE;
	m_ptLastCapture = pt;


	CButton::OnLButtonDown(nFlags, point);
}

void CJThumbButton::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	m_pSliderWnd->m_bMoving = FALSE;

	if(m_bCapture)
	{
		::ReleaseCapture();

		m_bCapture = FALSE;

		CPoint pt;
		::GetCursorPos(&pt);
		m_pSliderWnd->OnThumbRelease(m_ptLastCapture,pt);
	}


	CImgButton::OnLButtonUp(nFlags, point);
}

void CJThumbButton::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(m_bCapture)
	{
		CPoint pt;
		::GetCursorPos(&pt);
		//
		m_pSliderWnd->OnThumbMove(m_ptLastCapture,pt,TRUE);

		m_ptLastCapture = pt;
	}
	
	//ShowToolTip();


	CImgButton::OnMouseMove(nFlags, point);
}


void CJThumbButton::ShowToolTip()
{
	if(m_pToolTip == NULL)
	{
		return;
	}
	//发送通知
	TOOLTIPTEXT tip;
	memset(&tip,0,sizeof(tip));
	tip.hdr.hwndFrom =  m_pSliderWnd->GetSafeHwnd();
	tip.hdr.idFrom = m_pSliderWnd->GetDlgCtrlID();
	tip.hdr.code = TTN_NEEDTEXT;
	m_pSliderWnd->GetParent()->SendMessage(WM_NOTIFY,tip.hdr.idFrom,(LPARAM)&tip);

	if(tip.szText[0] == _T('\0'))
	{
		CString strTmp;
		strTmp.Format(_T("%d"),m_pSliderWnd->GetPos());
		m_pToolTip->UpdateTipText(strTmp,this,(UINT_PTR)m_hWnd);
	}
	else
	{
		//强制显示
		m_pToolTip->UpdateTipText(tip.szText,this,(UINT_PTR)m_hWnd);
	}

}
