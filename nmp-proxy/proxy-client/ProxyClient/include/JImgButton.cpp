// ImgButton.cpp : 实现文件
//

#include "stdafx.h"
#include "JImgButton.h"

#ifndef IMG_PATH
#define IMG_PATH "jxj\\picture\\"
#endif

JString GetModulePath(HMODULE hModule)
{
	char szModulePath[MAX_PATH] = {0};
	::GetModuleFileNameA(hModule, szModulePath, MAX_PATH);
	JString strPath(szModulePath);
	return strPath.substr(0, strPath.rfind("\\") + 1);
}

BOOL JLoadImageFile(const CString& strFile,CImage& img)
{
	img.Destroy();

	return (img.Load(strFile) == ERROR_SUCCESS);
}

BOOL JLoadImage(const char* szName,CImage& img)
{
	JString strPath = GetModulePath(NULL);
	JString strFile = strPath + JString(IMG_PATH) + szName;

	return JLoadImageFile(StringAsciiToUnicode(strFile).c_str(),img);
}

HRGN ImgToRegion(CImage& img, COLORREF crMask, COLORREF crTolerance)
{
	HRGN hRgn = CreateRectRgn(0,0,img.GetWidth(),img.GetHeight());

	int nRMask = GetRValue(crMask);
	int nGMask = GetGValue(crMask);
	int nBMask = GetBValue(crMask);

	int nRTol = GetRValue(crTolerance);
	int nGTol = GetGValue(crTolerance);
	int nBTol = GetBValue(crTolerance);

	for(int i = 0;i < img.GetWidth();i++)
	{
		for(int j = 0;j < img.GetHeight();j++)
		{
			COLORREF crCur = img.GetPixel(i,j);
			//if(crCur == crMask)

			int nR = (int)GetRValue(crCur) - nRMask;
			int nG = (int)GetGValue(crCur) - nGMask;
			int nB = (int)GetBValue(crCur) - nBMask;
			if(nR < 0)nR = -nR;
			if(nG < 0)nG = -nG;
			if(nB < 0)nB = -nB;
			if(nR <= nRTol && nG <= nGTol && nB <= nBTol)
			{
				HRGN hRgnTmp1 = CreateRectRgn(i,j,i+1,j+1);
				HRGN hRgnTmp2 = CreateRectRgn(0,0,0,0);
				CombineRgn(hRgnTmp2,hRgn,hRgnTmp1,RGN_XOR);				
				::DeleteObject(hRgnTmp1); 
				::DeleteObject(hRgn); 
				hRgn = hRgnTmp2;
			}
		}
	}

	return hRgn;
}

// CImgButton

//IMPLEMENT_DYNAMIC(CImgButton, CButton)

CImgButton::CImgButton(int nTxtHeight)
{
	m_pImgOuterNormal = NULL;
	m_pImgOuterSelect = NULL;
	m_nJpgLeft = 0;
	m_nJpgTop = 0;
	m_hRgn = NULL;
	m_nAdjustX = 0;
	m_nAdjustY = 0;

	m_bMoseHover = FALSE;
	m_nXPlace = -1;
	m_nYPlace = -1;
	m_crText = RGB(255,255,255);
	m_crSelText = -1;

	m_pToolTip = NULL;
	m_bCapture = FALSE;

	m_font.CreateFont(nTxtHeight,0,0,0,FW_NORMAL,FALSE,FALSE,0,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_SWISS,_T("宋体"));
}

CImgButton::~CImgButton()
{
	if(m_hRgn)
	{
		::DeleteObject(m_hRgn); 
		m_hRgn = NULL;
	}
}

CImage* CImgButton::GetImgNormal()
{
	if(m_pImgOuterNormal)
	{
		return m_pImgOuterNormal;
	}
	else
	{
		if(m_imgNormal.IsNull())
		{
			return NULL;
		}
		else
		{
			return &m_imgNormal;
		}
	}
}

CImage* CImgButton::GetImgSelect()
{
	if(m_pImgOuterSelect)
	{
		return m_pImgOuterSelect;
	}
	else
	{
		if(m_imgSelect.IsNull())
		{
			return NULL;
		}
		else
		{
			return &m_imgSelect;
		}
	}
}

void CImgButton::SetImage(const char* szImgNameNormal , const char* szImgNameSelect , int nJpgLeft, int nJpgTop)
{
	m_pImgOuterNormal = NULL;
	m_pImgOuterSelect = NULL;

	if(szImgNameNormal)
	{
		JLoadImage(szImgNameNormal,m_imgNormal);
	}
	else
	{
		m_imgNormal.Destroy();
	}
	if(szImgNameSelect)
	{
		JLoadImage(szImgNameSelect,m_imgSelect);
	}
	else
	{
		m_imgSelect.Destroy();
	}

	m_nJpgLeft = nJpgLeft;
	m_nJpgTop = nJpgTop;

	if(IsWindow(GetSafeHwnd()))
	{
		//Invalidate();
		CRect rc;
		GetWindowRect(&rc);
		GetParent()->ScreenToClient(&rc);
		rc.right = rc.left + GetImgNormal()->GetWidth() - m_nJpgLeft;
		rc.bottom = rc.top + GetImgNormal()->GetHeight() - m_nJpgTop;

		rc.OffsetRect(m_nAdjustX,m_nAdjustY);
		MoveWindow(rc,FALSE);
		Invalidate();
	}
}

void CImgButton::SetImage(CImage* pImgNormal , CImage* pImgSelect, int nJpgLeft, int nJpgTop)
{
	m_imgNormal.Destroy();
	m_imgSelect.Destroy();

	m_pImgOuterNormal = pImgNormal;
	m_pImgOuterSelect = pImgSelect;	

	m_nJpgLeft = nJpgLeft;
	m_nJpgTop = nJpgTop;

	if(IsWindow(GetSafeHwnd()))
	{
		//Invalidate();
		CRect rc;
		GetWindowRect(&rc);
		GetParent()->ScreenToClient(&rc);
		rc.right = rc.left + GetImgNormal()->GetWidth() - m_nJpgLeft;
		rc.bottom = rc.top + GetImgNormal()->GetHeight() - m_nJpgTop;

		rc.OffsetRect(m_nAdjustX,m_nAdjustY);
		MoveWindow(rc,FALSE);
		Invalidate();
	}
}

void CImgButton::SetRgn(COLORREF crTolerance)
{
	m_hRgn = ImgToRegion(*GetImgNormal(),GetImgNormal()->GetPixel(0,0),crTolerance);

	if(IsWindow(GetSafeHwnd()))
	{
		DWORD style = GetClassLong(GetSafeHwnd(), GCL_STYLE);
		SetClassLong(GetSafeHwnd(), GCL_STYLE, style & ~CS_PARENTDC);
		SetWindowRgn(m_hRgn,TRUE);
	}
}

void CImgButton::SetToolTip(CToolTipCtrl* pToolTip , LPCTSTR lpszTip)
{
	m_pToolTip = pToolTip;
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
		m_pToolTip->AddTool(this,LPSTR_TEXTCALLBACK,&rc,(UINT_PTR)m_hWnd);
	}
}

long CImgButton::GetImgWidth()
{
	if(GetImgNormal())
	{
		return GetImgNormal()->GetWidth();
	}
	else
	{
		return 0;
	}
}

long CImgButton::GetImgHeight()
{
	if(GetImgNormal())
	{
		return GetImgNormal()->GetHeight();
	}
	else
	{
		return 0;
	}
}

void CImgButton::SetTextPlace(int nX,int nY)
{
	m_nXPlace = nX;
	m_nYPlace = nY;
}

void CImgButton::SetTextColor(COLORREF crText,COLORREF crSelText)
{
	m_crText = crText;
	m_crSelText = crSelText;
}

void CImgButton::AdjustButtonPlace(int nX, int nY)
{
	m_nAdjustX = nX;
	m_nAdjustY = nY;

	if(IsWindow(GetSafeHwnd()))
	{
		//Invalidate();
		CRect rc;
		GetWindowRect(&rc);
		GetParent()->ScreenToClient(&rc);
		rc.right = rc.left + GetImgWidth() - m_nJpgLeft;
		rc.bottom = rc.top + GetImgHeight() - m_nJpgTop;

		rc.OffsetRect(m_nAdjustX,m_nAdjustY);
		MoveWindow(rc,FALSE);
		Invalidate();
	}
}

BEGIN_MESSAGE_MAP(CImgButton, CButton)
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_NOTIFY_EX( TTN_NEEDTEXT, 0, OnToolTipNotify )
	ON_MESSAGE(WM_MOUSELEAVE,OnMouseLeave)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()



// CImgButton 消息处理程序



void CImgButton::PreSubclassWindow()
{
	// TODO: 在此添加专用代码和/或调用基类

	CButton::PreSubclassWindow();

	this->ModifyStyle(0,BS_OWNERDRAW);
	m_bMoseHover = FALSE;

	if(GetImgNormal() == NULL)
	{
		return;
	}

	CRect rc;
	GetWindowRect(&rc);
	GetParent()->ScreenToClient(&rc);
	rc.right = rc.left + GetImgWidth() - m_nJpgLeft;
	rc.bottom = rc.top + GetImgHeight() - m_nJpgTop;

	rc.OffsetRect(m_nAdjustX,m_nAdjustY);
	MoveWindow(rc);

	OnSize(0,rc.Width(),rc.Height());

	if(m_hRgn)
	{
		DWORD style = GetClassLong(GetSafeHwnd(), GCL_STYLE);
		SetClassLong(GetSafeHwnd(), GCL_STYLE, style & ~CS_PARENTDC); 
		SetWindowRgn(m_hRgn,TRUE);
	}
}

LRESULT CImgButton::OnMouseLeave(WPARAM,LPARAM)
{
	m_bMoseHover = FALSE;
	Invalidate();
	return 0;
}

BOOL CImgButton::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(!m_bMoseHover)
	{
		CRect rc;
		CPoint pt;
		::GetCursorPos(&pt);
		GetWindowRect(&rc);
		if(rc.PtInRect(pt))
		{
			TRACKMOUSEEVENT evt;
			evt.cbSize = sizeof(evt);
			evt.dwFlags = TME_LEAVE;
			evt.hwndTrack = GetSafeHwnd();
			evt.dwHoverTime = HOVER_DEFAULT;

			TrackMouseEvent(&evt);
			
			m_bMoseHover = TRUE;
			Invalidate();
		}
	}

	return CButton::OnSetCursor(pWnd, nHitTest, message);
}

void CImgButton::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	NMHDR hdr;
	hdr.hwndFrom = GetSafeHwnd();
	hdr.idFrom = GetDlgCtrlID();
	hdr.code = NM_CMD_LBUTTONDOWN;
	if(GetOwner()->SendMessage(WM_NOTIFY,hdr.idFrom,(LPARAM)&hdr) == CAPTURE_LBUTTONUP)
	{
		//捕获消息
		m_bCapture = TRUE;
		SetCapture();
		//CButton::OnLButtonDown(nFlags, point);
		return;
	}

	if(m_pToolTip)
	{
		tagMSG msg;
		msg.hwnd= m_hWnd;
		msg.message= WM_LBUTTONDOWN;
		msg.wParam= nFlags;
		msg.lParam= MAKELPARAM(LOWORD(point.x), LOWORD(point.y));
		msg.time= 0;
		msg.pt.x= point.x;
		msg.pt.y= point.y;
		m_pToolTip->RelayEvent(&msg);
	}

	CButton::OnLButtonDown(nFlags, point);
}

void CImgButton::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(m_bCapture)
	{
		m_bCapture = FALSE;
		ReleaseCapture();
		NMHDR hdr;
		hdr.hwndFrom = GetSafeHwnd();
		hdr.idFrom = GetDlgCtrlID();
		hdr.code = NM_CMD_LBUTTONUP;
		GetOwner()->SendMessage(WM_NOTIFY,hdr.idFrom,(LPARAM)&hdr);
		//CButton::OnLButtonUp(nFlags, point);
		return;
	}

	if(m_pToolTip)
	 {
		tagMSG msg;
		msg.hwnd= m_hWnd;
		msg.message= WM_LBUTTONUP;
		msg.wParam= nFlags;
		msg.lParam= MAKELPARAM(LOWORD(point.x), LOWORD(point.y));
		msg.time= 0;
		msg.pt.x= point.x;
		msg.pt.y= point.y;
		m_pToolTip->RelayEvent(&msg);		
	 }

	CButton::OnLButtonUp(nFlags, point);
}

void CImgButton::OnSize(UINT nType, int cx, int cy)
{
	CButton::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	if(m_pToolTip)
	{
		//del
		m_pToolTip->DelTool(this,(UINT_PTR)m_hWnd);

		//add
		CRect rc(0,0,cx,cy);
		m_pToolTip->AddTool(this,LPSTR_TEXTCALLBACK,&rc,(UINT_PTR)m_hWnd);
	}
}

void CImgButton::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if(m_pToolTip)
	{
		tagMSG msg;
		msg.hwnd= m_hWnd;
		msg.message= WM_MOUSEMOVE;
		msg.wParam= nFlags;
		msg.lParam= MAKELPARAM(LOWORD(point.x), LOWORD(point.y));
		msg.time= 0;
		msg.pt.x= point.x;
		msg.pt.y= point.y;
		m_pToolTip->RelayEvent(&msg);
	}

	CButton::OnMouseMove(nFlags, point);
}

BOOL CImgButton::OnToolTipNotify( UINT id, NMHDR * pNMHDR, LRESULT * pResult )
{
	*pResult = 0;
	TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pNMHDR;

	pTTT->hdr.hwndFrom = GetSafeHwnd();
	pTTT->hdr.idFrom = GetDlgCtrlID();
	memset(pTTT->szText,0,sizeof(pTTT->szText));

	GetParent()->SendMessage(WM_NOTIFY,GetDlgCtrlID(),(LPARAM)pTTT);

	if(pTTT->szText[0] != _T('\0'))
	{
		return TRUE;
	}

	if(m_strTip.IsEmpty())
	{
		return FALSE;
	}

	_tcsncpy(pTTT ->szText,m_strTip,sizeof(pTTT ->szText)/sizeof(TCHAR) - 1);
	return TRUE;
}

BOOL CImgButton::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	return TRUE;
	//return CButton::OnEraseBkgnd(pDC);
}


void CImgButton::DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct )
{
	CRect rcSrc(m_nJpgLeft,m_nJpgTop,GetImgWidth(),GetImgHeight());
	CRect rcTag(lpDrawItemStruct->rcItem);

	COLORREF	crText = m_crText;

	if (m_bMoseHover || (lpDrawItemStruct->itemState & ODS_SELECTED))
	{
		if(m_crSelText != -1)
		{
			crText = m_crSelText;
		}

		if(GetImgSelect())
		{
			GetImgSelect()->Draw(lpDrawItemStruct->hDC,rcTag,rcSrc);
		}
		else
		{
			if(GetImgNormal())
			{
				GetImgNormal()->Draw(lpDrawItemStruct->hDC,rcTag,rcSrc);
			}
		}
	}
	else
	{
		if(GetImgNormal())
		{
			GetImgNormal()->Draw(lpDrawItemStruct->hDC,rcTag,rcSrc);
		}		
	}

	//输出文字
	CString strTitle;
	GetWindowText(strTitle);
	if(!strTitle.IsEmpty())
	{
		::SetBkMode(lpDrawItemStruct->hDC,TRANSPARENT);
		::SetTextColor(lpDrawItemStruct->hDC,crText);
		HFONT hFont = (HFONT)::SelectObject(lpDrawItemStruct->hDC,m_font);

		if(m_nXPlace >= 0 && m_nYPlace >= 0)
		{
			CRect rc(lpDrawItemStruct->rcItem);
			rc.left += m_nXPlace;
			rc.top += m_nYPlace;
			::DrawText(lpDrawItemStruct->hDC,strTitle,-1,&rc,DT_SINGLELINE);
		}
		else if(m_nXPlace >= 0)
		{
			CRect rc(lpDrawItemStruct->rcItem);
			rc.left += m_nXPlace;
			::DrawText(lpDrawItemStruct->hDC,strTitle,-1,&rc,DT_SINGLELINE|DT_VCENTER);
		}
		else if(m_nYPlace >= 0)
		{
			CRect rc(lpDrawItemStruct->rcItem);
			rc.top += m_nYPlace;
			::DrawText(lpDrawItemStruct->hDC,strTitle,-1,&rc,DT_SINGLELINE|DT_CENTER);
		}
		else
		{
			::DrawText(lpDrawItemStruct->hDC,strTitle,-1,&lpDrawItemStruct->rcItem,DT_SINGLELINE|DT_VCENTER|DT_CENTER);
		}

		::SelectObject(lpDrawItemStruct->hDC,hFont);
	}

}
