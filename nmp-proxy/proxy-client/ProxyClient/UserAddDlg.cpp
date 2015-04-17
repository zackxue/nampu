// UserAddDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ProxyClient3.h"
#include "UserAddDlg.h"


// CUserAddDlg 对话框
extern CManage gManage;
IMPLEMENT_DYNAMIC(CUserAddDlg, CDialog)

CUserAddDlg::CUserAddDlg(CWnd* pParent /*=NULL*/)
: CDialog(CUserAddDlg::IDD, pParent)
, m_name(_T(""))
, m_pwd(_T(""))
, m_pwd_confirm(_T(""))
, m_pParent(NULL)
{

}

CUserAddDlg::~CUserAddDlg()
{
}

void CUserAddDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_idok_btn);
	DDX_Control(pDX, IDCANCEL, m_idcancel_btn);
	DDX_Control(pDX, IDC_CLOSE, m_close_btn);
}


BEGIN_MESSAGE_MAP(CUserAddDlg, CDialog)

//	ON_BN_CLICKED(IDOK, &CUserAddDlg::OnBnClickedOk)
ON_WM_NCPAINT()
ON_WM_NCHITTEST()
ON_BN_CLICKED(IDC_CLOSE, &CUserAddDlg::OnBnClickedClose)
ON_WM_CTLCOLOR()
ON_WM_PAINT()
END_MESSAGE_MAP()


// CUserAddDlg 消息处理程序

//void CUserAddDlg::OnBnClickedOk()
//{
//	// TODO: 在此添加控件通知处理程序代码
//
//	//OnOK();
//}

//void CUserAddDlg::OnOK()
//{
//	// TODO: 在此添加专用代码和/或调用基类
//
//	CDialog::OnOK();
//}

BOOL CUserAddDlg::OnInitDialog()
{
	CDialog::OnInitDialog();



	CEdit*pEdt=(CEdit*)GetDlgItem(IDC_NAME);
	pEdt->SetLimitText(32);

	pEdt=(CEdit*)GetDlgItem(IDC_PWD);
	pEdt->SetLimitText(32);

	pEdt=(CEdit*)GetDlgItem(IDC_PWDCONFIRM);
	pEdt->SetLimitText(32);
	// TODO:  在此添加额外的初始化


	m_close_btn.SetImage(_T("images/cu_title_close.bmp"),_T("images/cu_sel_title_close.bmp"),
		_T("images/cu_sel_title_close.bmp"),_T("images/cu_sel_title_close.bmp"));


	m_idok_btn.SetImage(_T("images/normal_black_btn.bmp"),_T("images/sel_black_btn.bmp"),
		_T("images/sel_black_btn.bmp"),_T("images/sel_black_btn.bmp"));

	m_idcancel_btn.SetImage(_T("images/normal_black_btn.bmp"),_T("images/sel_black_btn.bmp"),
		_T("images/sel_black_btn.bmp"),_T("images/sel_black_btn.bmp"));

	m_idok_btn.SetTextPos(CPoint(10,4));
	m_idcancel_btn.SetTextPos(CPoint(10,4));
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CUserAddDlg::OnClose(void)
{
}



void CUserAddDlg::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类
	GetDlgItemText(IDC_NAME,m_name);
	GetDlgItemText(IDC_PWD,m_pwd);
	GetDlgItemText(IDC_PWDCONFIRM,m_pwd_confirm);
	if(m_name.IsEmpty())
	{
		MessageBox(_T("请输入用户名！"), NULL, MB_OK);
		return ;
	}

	if(m_pwd_confirm.IsEmpty())
	{
		MessageBox(_T("请输入确认密码！"), NULL, MB_OK);
		return ;
	}
	if(m_pwd.IsEmpty())
	{
		MessageBox(_T("请输入密码！"), NULL, MB_OK);
		return ;
	}
	if (m_pwd != m_pwd_confirm)
	{
		MessageBox(_T("输入的密码和确认密码不相同！"));
		return ;

	}

	int ret = gManage.AddUser(m_name,m_pwd,this);
	if (ret == 0)
	{
		CDialog::OnOK();
		CMainUser* pMainUser = (CMainUser*)m_pParent;
		int n = pMainUser->m_UserList.InsertItem(0,m_name);
		if (m_name == _T("admin"))
			pMainUser->m_UserList.SetItemText(n,1,_T("超级用户"));
		else
			pMainUser->m_UserList.SetItemText(n,1,_T("普通用户"));

		
		MessageBox(_T("添加用户成功！"));
		return;
	
	}
	else
		MessageBox(_T("添加用户失败！"));

	CDialog::OnOK();

}
void CUserAddDlg::DrawRangeImage(int i, CDC *pDC, CRect rc){
	HBITMAP hBitmap1 = NULL;
	HBITMAP hBitmap2 = NULL;
	CBitmap bit[2];
	if(hBitmap1 ==NULL||hBitmap2 ==NULL){

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
	}


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
void CUserAddDlg::OnNcPaint()
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


		COLORREF col = RGB(255,0,255);//修改对话框外形

		CRgn rgn;
		rgn.CreateRoundRectRgn (1, 1, rcClient.Width(), rcClient.Height(),6,6);

		SetWindowRgn (rgn, TRUE);

		dc.SetTextColor(RGB(255,255,240));
		dc.SetBkMode(TRANSPARENT);
		dc.TextOut(10,5,_T("用户添加"));
}

LRESULT CUserAddDlg::OnNcHitTest(CPoint point)
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

void CUserAddDlg::OnBnClickedClose()
{

	OnCancel();
	// TODO: 在此添加控件通知处理程序代码
}

HBRUSH CUserAddDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

void CUserAddDlg::OnPaint()
{

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


	COLORREF col = RGB(255,0,255);//修改对话框外形

	CRgn rgn;
	rgn.CreateRoundRectRgn (1, 1, rcClient.Width(), rcClient.Height(),6,6);

	SetWindowRgn (rgn, TRUE);

	dc.SetTextColor(RGB(255,255,240));
	dc.SetBkMode(TRANSPARENT);
	dc.TextOut(10,5,_T("用户添加"));
}
