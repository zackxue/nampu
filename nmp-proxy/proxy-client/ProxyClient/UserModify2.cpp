// UserModify2.cpp : 实现文件
//

#include "stdafx.h"
#include "ProxyClient3.h"
#include "UserModify2.h"


// CUserModify2 对话框
extern CManage gManage;
IMPLEMENT_DYNAMIC(CUserModify2, CDialog)

CUserModify2::CUserModify2(CWnd* pParent /*=NULL*/)
	: CDialog(CUserModify2::IDD, pParent)
	, m_name(_T(""))
{

}

CUserModify2::~CUserModify2()
{
}

void CUserModify2::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CLOSE, m_close_btn);
	DDX_Control(pDX, IDCANCEL, m_cancel_btn);
	DDX_Control(pDX, IDOK, m_ok_btn);
}


BEGIN_MESSAGE_MAP(CUserModify2, CDialog)
	ON_BN_CLICKED(IDOK, &CUserModify2::OnBnClickedOk)
	ON_WM_NCPAINT()
	ON_BN_CLICKED(IDC_CLOSE, &CUserModify2::OnBnClickedClose)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CUserModify2 消息处理程序

void CUserModify2::OnBnClickedOk()
{
	CString newpwd;
	CString verifypwd;
	GetDlgItemText(IDC_PWD,newpwd);
	GetDlgItemText(IDC_VERIFYPWD,verifypwd);
	
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
		MessageBox(_T("两次输入密码不一致"));
		return;
	}
	
	int ret ;
	ret = gManage.UserModify(m_name,newpwd ,gManage.m_pwd);
	if (ret == 0)
	{
		OnOK();
		MessageBox(_T("修改用户密码成功！"));
		return ; 

	}
	else
		MessageBox(_T("修改用户密码失败！"));
	OnOK();
}

void CUserModify2::DrawRangeImage(int i, CDC *pDC, CRect rc){
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

BOOL CUserModify2::OnInitDialog()
{
	CDialog::OnInitDialog();
	SetDlgItemText(IDC_NAME,m_name);
	//
	//GetDlgItem(IDC_NAME)->EnableWindow(FALSE);
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

void CUserModify2::OnNcPaint()
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

void CUserModify2::OnBnClickedClose()
{

	OnCancel();
	// TODO: 在此添加控件通知处理程序代码
}

HBRUSH CUserModify2::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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
