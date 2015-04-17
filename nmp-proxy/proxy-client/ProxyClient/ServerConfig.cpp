// ServerConfig.cpp : 实现文件
//

#include "stdafx.h"
#include "ProxyClient3.h"
#include "ServerConfig.h"


// CServerConfig 对话框

IMPLEMENT_DYNAMIC(CServerConfig, CDialog)

CServerConfig::CServerConfig(UINT IDD1,CWnd* pParent /*=NULL*/)
	: CMyBlackDlg(IDD1, pParent)
{

}

CServerConfig::~CServerConfig()
{
}

void CServerConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
		DDX_Control(pDX, IDCANCEL, m_cancel_btn);
	DDX_Control(pDX, IDOK, m_ok_btn);
}


BEGIN_MESSAGE_MAP(CServerConfig, CDialog)
//	ON_EN_CHANGE(IDC_EDIT2, &CServerConfig::OnEnChangeEdit2)
ON_WM_NCPAINT()
ON_WM_NCHITTEST()
ON_WM_CTLCOLOR()
ON_COMMAND(CLOSE_BTN, OnClose)
END_MESSAGE_MAP()


// CServerConfig 消息处理程序

//void CServerConfig::OnEnChangeEdit2()
//{
//	// TODO:  如果该控件是 RICHEDIT 控件，它将不
//	// 发送此通知，除非重写 CDialog::OnInitDialog()
//	// 函数并调用 CRichEditCtrl().SetEventMask()，
//	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。
//
//	// TODO:  在此添加控件通知处理程序代码
//}
extern CManage gManage;
void CServerConfig::OnOK()
{
	CString ip,listenport,rtspport;
	GetDlgItemText(IDC_IP,ip);
	GetDlgItemText(IDC_LISTENPORT,listenport);
	GetDlgItemText(IDC_RTSPPORT,rtspport);
	
	if(ip.IsEmpty())
	{
		MessageBox(_T("请输入服务器IP！"), NULL, MB_OK);
		return ;
	}	

	if(listenport.IsEmpty())
	{
		MessageBox(_T("请输入监听端口！"), NULL, MB_OK);
		return ;
	}

	if(rtspport.IsEmpty())
	{
		MessageBox(_T("请输入流媒体端口！"), NULL, MB_OK);
		return ;
	}
	//CString ip2,port1,port2;
	if (ip==ip2&&listenport == port1&&rtspport==port2)
	{
		EndDialog(2);
		return ;
	}
	int ret = gManage.SetConfig( ip, listenport, rtspport);
	
	if (0==ret)
	{
		CDialog::OnOK();
		MessageBox(_T("配置成功! 请重新登录"));
		return;
	}
	else{
      MessageBox(_T("配置失败!"));
	  return ;
	}
		EndDialog(2);
}

void CServerConfig::OnNcPaint()
{
	CMyBlackDlg::OnNcPaint();
}

LRESULT CServerConfig::OnNcHitTest(CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	return CMyBlackDlg::OnNcHitTest(point);
}

HBRUSH CServerConfig::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CMyBlackDlg::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何属性

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}


void CServerConfig::OnClose()
{
	EndDialog(2);
}

BOOL CServerConfig::OnInitDialog()
{
	CMyBlackDlg::OnInitDialog();


	CEdit*pEdt=(CEdit*)GetDlgItem(IDC_IP);
	pEdt->SetLimitText(32);

	pEdt=(CEdit*)GetDlgItem(IDC_LISTENPORT);
	pEdt->SetLimitText(32);

	pEdt=(CEdit*)GetDlgItem(IDC_RTSPPORT);
	pEdt->SetLimitText(32);


	m_ok_btn.SetImage(_T("images/normal_black_btn.bmp"),_T("images/sel_black_btn.bmp"),
		_T("images/sel_black_btn.bmp"),_T("images/sel_black_btn.bmp"));

	m_cancel_btn.SetImage(_T("images/normal_black_btn.bmp"),_T("images/sel_black_btn.bmp"),
		_T("images/sel_black_btn.bmp"),_T("images/sel_black_btn.bmp"));

	m_ok_btn.SetTextPos(CPoint(12,4));
	m_cancel_btn.SetTextPos(CPoint(12,4));


	int ret = gManage.GetConfig(ip2,port1,port2);
	SetDlgItemText(IDC_IP,ip2);
	SetDlgItemText(IDC_LISTENPORT,port1);
	SetDlgItemText(IDC_RTSPPORT,port2);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}
