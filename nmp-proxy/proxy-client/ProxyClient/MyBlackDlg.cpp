// MyBlackDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ProxyClient3.h"
#include "MyBlackDlg.h"


// CMyBlackDlg 对话框

IMPLEMENT_DYNAMIC(CMyBlackDlg, CDialog)

CMyBlackDlg::CMyBlackDlg(UINT IDD1,CWnd* pParent /*=NULL*/)
	: CDialog(IDD1, pParent)
{

}

CMyBlackDlg::~CMyBlackDlg()
{
}

void CMyBlackDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMyBlackDlg, CDialog)
	ON_WM_NCPAINT()
	ON_WM_NCHITTEST()
	ON_WM_CTLCOLOR()
//	ON_WM_SIZE()
END_MESSAGE_MAP()


// CMyBlackDlg 消息处理程序

void CMyBlackDlg::DrawRangeImage(int i, CDC *pDC, CRect rc){



	CDC MemDC;
	BITMAP bm;
	bit[i].GetBitmap(&bm);

	int li_Width = bm.bmWidth;
	int li_Height = bm.bmHeight;


	MemDC.CreateCompatibleDC(pDC);
	CBitmap* pOldBitmap = MemDC.SelectObject(&bit[i]);

	int x=rc.left;
	int y=rc.top;

	while (y < (rc.Height()+rc.top)) 
	{
		while(x < (rc.Width()+rc.left)) 
		{
			pDC->BitBlt(x, y, li_Width, li_Height, &MemDC, 0, 0, SRCCOPY);
			x += li_Width;
		}
		x = rc.left;
		y += li_Height;
	}

	//pDC->StretchBlt(x, y, rc.right - rc.left, rc.bottom - rc.top, &MemDC, 0, 0,2,2 ,SRCCOPY);
	MemDC.SelectObject(pOldBitmap);
	MemDC.DeleteDC();
}
void CMyBlackDlg::SetTitle(CString title){
	m_title = title;
}

void CMyBlackDlg::ChangeWindowRgn(CDC* pDC)
{
	COLORREF col = RGB(255,0,255);
	CRect rcClient;
	GetClientRect (rcClient);
	CRgn rgn;
	rgn.CreateRoundRectRgn (1, 1, rcClient.Width(), rcClient.Height(),6,6);

	SetWindowRgn (rgn, TRUE);
}
void CMyBlackDlg::OnNcPaint()
{
	CDialog::OnNcPaint();	
	CPaintDC dc(this);
	CDC szMemDC;
	CRect rcClient;
	CRect rc;
	GetClientRect(&rcClient);

	CBitmap bitmap;
	bitmap.LoadBitmap(IDB_HEAD);
	CDC memdc;
	memdc.CreateCompatibleDC(&dc);
	BITMAP bit;
	bitmap.GetBitmap(&bit);
	memdc.SelectObject(&bitmap);
	// rcClient.InflateRect(1,1,1,1);
	dc.StretchBlt(0,0,rcClient.Width(),rcClient.Height(),&memdc,0,0,bit.bmWidth,bit.bmHeight,SRCCOPY);

	rc = rcClient;
	rc.bottom = 31;
	szMemDC.CreateCompatibleDC(&dc);
	DrawRangeImage(0,&dc,rc);//top

	rc = rcClient;
	rc.right = 1;
	rc.top = rcClient.top - 31;
	DrawRangeImage(1,&dc,rc);//left

	rc = rcClient;
	rc.left = rcClient.right - 5;
	rc.top = rcClient.top - 31;
	DrawRangeImage(1,&dc,rc);//right

	rc = rcClient;
	rc.top = rcClient.bottom - 5;
	DrawRangeImage(0,&dc,rc);//bottom

	ChangeWindowRgn(&dc);
	dc.SetTextColor(RGB(255,255,240));
	dc.SetBkMode(TRANSPARENT);
	dc.TextOut(10,5,m_title);


}

LRESULT CMyBlackDlg::OnNcHitTest(CPoint point)
{
	ScreenToClient(&point);

	RECT rtWindow;
	GetClientRect(&rtWindow);

	long wndHeight = rtWindow.bottom - rtWindow.top;
	long wndWidth = rtWindow.right - rtWindow.left;

	RECT rcW = {0,0,wndWidth,31};
	if(::PtInRect(&rcW,point))
	{  
		return HTCAPTION;  // 在拖动范围内
	}


	return CDialog::OnNcHitTest(point);
}

BOOL CMyBlackDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	HBITMAP hBitmap1 = NULL;
	HBITMAP hBitmap2 = NULL;
	hBitmap1 = (HBITMAP)::LoadImage(NULL,_T("images/top.bmp"), 
		IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE|LR_LOADFROMFILE);
	hBitmap2 = (HBITMAP)::LoadImage(NULL,_T("images/band.bmp"), 
		IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE|LR_LOADFROMFILE);
	if(hBitmap1 == NULL||hBitmap2 == NULL)
	{
		AfxMessageBox(_T("无法加载图片"));
		PostQuitMessage(0);
	}
	if (bit[0].m_hObject)
		bit[0].Detach();
	if (bit[1].m_hObject)
		bit[1].Detach();
	bit[0].Attach(hBitmap1);
	bit[1].Attach(hBitmap2);

	CRect rect;
	GetClientRect(&rect);
	m_close_btn.Create(_T(""), WS_CHILD | WS_VISIBLE|BS_OWNERDRAW, CRect(0 , 0, 22, 18), this, CLOSE_BTN);

	m_close_btn.MoveWindow(rect.right-30 , 5, 22, 18);
	m_close_btn.SetImage(_T("images/cu_title_close.bmp"),_T("images/cu_sel_title_close.bmp"),
		_T("images/cu_sel_title_close.bmp"),_T("images/cu_sel_title_close.bmp"));
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

HBRUSH CMyBlackDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if(nCtlColor==CTLCOLOR_STATIC)
	{  
		pDC->SetBkMode(TRANSPARENT);  
		//pDC->SetTextColor(RGB(68,129,2));
		return  (HBRUSH)GetStockObject(NULL_BRUSH); 

	}
	return hbr;
}


