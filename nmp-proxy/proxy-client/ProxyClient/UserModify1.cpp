// UserModify1.cpp : 实现文件
//

#include "stdafx.h"
#include "ProxyClient3.h"
#include "UserModify1.h"


// CUserModify1 对话框
extern CManage gManage;
IMPLEMENT_DYNAMIC(CUserModify1, CDialog)

CUserModify1::CUserModify1(CWnd* pParent /*=NULL*/)
	: CDialog(CUserModify1::IDD, pParent)
	, m_name(_T(""))
{

}

CUserModify1::~CUserModify1()
{
}

void CUserModify1::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CLOSE, m_close_btn);
	DDX_Control(pDX, IDCANCEL, m_cancel_btn);
	DDX_Control(pDX, IDOK, m_ok_btn);
}

void CUserModify1::DrawRangeImage(int i, CDC *pDC, CRect rc){
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


BEGIN_MESSAGE_MAP(CUserModify1, CDialog)
	ON_BN_CLICKED(IDOK, &CUserModify1::OnBnClickedOk)
	ON_WM_NCPAINT()
	ON_WM_NCHITTEST()
	ON_BN_CLICKED(IDC_CLOSE, &CUserModify1::OnBnClickedClose)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CUserModify1 消息处理程序

void CUserModify1::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CString oldpwd;
	CString newpwd;
	CString verifypwd;
	GetDlgItemText(IDC_OLDPWD,oldpwd);
	GetDlgItemText(IDC_NEWPWD,newpwd);
	GetDlgItemText(IDC_VERIFYPWD,verifypwd);
	if (oldpwd.IsEmpty())
	{
		MessageBox(_T("请输入原密码"));
		return;
	}
	if (newpwd.IsEmpty())
	{
		MessageBox(_T("请输入新密码"));
		return;
	}
	if (verifypwd.IsEmpty())
	{
		MessageBox(_T("请再次填入密码"));
		return;
	}
	if (newpwd != verifypwd)
	{
		MessageBox(_T("两次输入的密码不一致"));
		return;
	}

	if (oldpwd == gManage.m_pwd)
	{
		int ret ;
		ret = gManage.UserModify(m_name,newpwd ,oldpwd);
		if (ret == 0)
		{
			OnOK();
			gManage.m_pwd = newpwd;  // 修改成功密码后，改变程序记录下的原始密码
			MessageBox(_T("修改用户密码成功！"));
			return ;
		}
		else
			MessageBox(_T("修改用户密码失败！"));

	}
	else
	{
		MessageBox(_T("原始密码不正确！"));
		return ;
	}
	OnOK();
}

void CUserModify1::OnNcPaint()
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
	dc.TextOut(10,5,_T("修改用户密码"));
}

LRESULT CUserModify1::OnNcHitTest(CPoint point)
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

BOOL CUserModify1::OnInitDialog()
{
	CDialog::OnInitDialog();
	if (_T("admin")!=m_name)
	{
		SetDlgItemText(IDC_TIP,_T("普通用户只能修改自己信息"));
	}
	else
	{
		SetDlgItemText(IDC_TIP,_T("修改admin信息"));
	}

	m_close_btn.SetImage(_T("images/cu_title_close.bmp"),_T("images/cu_sel_title_close.bmp"),
		_T("images/cu_sel_title_close.bmp"),_T("images/cu_sel_title_close.bmp"));


	m_ok_btn.SetImage(_T("images/normal_black_btn.bmp"),_T("images/sel_black_btn.bmp"),
		_T("images/sel_black_btn.bmp"),_T("images/sel_black_btn.bmp"));

	m_cancel_btn.SetImage(_T("images/normal_black_btn.bmp"),_T("images/sel_black_btn.bmp"),
		_T("images/sel_black_btn.bmp"),_T("images/sel_black_btn.bmp"));

	m_ok_btn.SetTextPos(CPoint(10,4));
	m_cancel_btn.SetTextPos(CPoint(10,4));
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CUserModify1::OnBnClickedClose()
{
	OnCancel();
	// TODO: 在此添加控件通知处理程序代码
}

HBRUSH CUserModify1::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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
