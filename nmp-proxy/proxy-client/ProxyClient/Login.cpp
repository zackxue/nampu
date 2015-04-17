// Login.cpp : 实现文件
//

#include "stdafx.h"
#include "log.h"
#include "ProxyClient3.h"
#include "Login.h"
#include "resource.h"
#include "DeviceADD.h"


// CLogin 对话框

IMPLEMENT_DYNAMIC(CLogin, CDialog)
extern CManage gManage;
extern CProxyClient3App theApp;
CLogin::CLogin(UINT IDD1,CWnd* pParent /*=NULL*/)
: CDialog(IDD1, pParent)
, m_IP(_T(""))
, m_port(_T(""))
{


}

CLogin::~CLogin()
{
	
}

void CLogin::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDOK, m_ok_btn);
	DDX_Control(pDX, IDCANCEL, m_cancel_btn);
	DDX_Control(pDX, IDC_WAITING, m_waiting_static);
	DDX_Control(pDX, IDC_NAME, m_name_edit);
	DDX_Control(pDX, IDC_IP, m_ip_edit);
	DDX_Control(pDX, IDC_PORT, m_port_edit);
	DDX_Control(pDX, IDC_PWD, m_pwd_edit);
}


BEGIN_MESSAGE_MAP(CLogin, CDialog)
	ON_WM_NCHITTEST()
	ON_WM_PAINT()
	//	ON_WM_MOUSEMOVE()
	//	ON_WM_NCACTIVATE()
	//	ON_WM_NCLBUTTONDBLCLK()
	//	ON_WM_NCLBUTTONDOWN()
	//	ON_WM_NCLBUTTONUP()
	//	ON_WM_NCMOUSEMOVE()
	//	ON_WM_NCPAINT()
	//	ON_WM_SIZE()
	ON_MESSAGE(UM_ERR,OnResult)
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CLogin 消息处理程序

void CLogin::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类
	GetDlgItemText(IDC_NAME,m_name);
	GetDlgItemText(IDC_PWD,m_pwd);
	GetDlgItemText(IDC_IP,m_IP);
	GetDlgItemText(IDC_PORT,m_port);
	if(m_name.IsEmpty())
	{
		MessageBox(_T("请输入用户名！"), NULL, MB_OK);
		return ;
	}	

	if(m_pwd.IsEmpty())
	{
		MessageBox(_T("请输入密码！"), NULL, MB_OK);
		return ;
	}

	if(m_IP.IsEmpty())
	{
		MessageBox(_T("请输入服务器IP！"), NULL, MB_OK);
		return ;
	}

	if(m_port.IsEmpty())
	{
		MessageBox(_T("请输入端口号！"), NULL, MB_OK);
		return ;
	}
	gManage.m_name=m_name;
	gManage.m_pwd=m_pwd;
	gManage.m_port =m_port;
	int n=m_IP.GetLength();

	for (int i=0;i<=n;i++)
	{
		gManage.m_IP[i]=(char)m_IP.GetAt(i);
	}


	//SetTimer(0,3000,NULL);  
	//DWORD dwThreadId;
	//CreateThread(NULL,0,LoginThread,this,0,&dwThreadId);
	//return ; 

	AfxGetApp()->BeginWaitCursor();
	int lRet = gManage.Login(this);

	//m_waiting_static.ShowWindow(TRUE);

	if(0==lRet)
	{
		WriteLoginInf(m_name,m_IP,m_port);
		CDialog::OnOK();
	}
	else if (-2==lRet)
	{
		//pDlg->m_waiting_static.ShowWindow(FALSE);
		MessageBox(_T("无法连接服务器!"));
		
	}
	else if(-1==lRet)
	{
		//pDlg->SendMessage(UM_ERR);
		MessageBox(_T("用户名密码错误!"));
		

	}
	else if (1==lRet)
	{
		//pDlg->m_waiting_static.ShowWindow(FALSE);
		MessageBox(_T("此用户已经登录!"));
		
	}
	else if (-3 == lRet)
	{
		MessageBox(_T("登录服务器超时!"));
	}
	else
	{
		CString str;
		str.Format(_T("登录服务器失败!"));
		MessageBox(str);
	}

	AfxGetApp()->EndWaitCursor();

}
void CLogin::Close(){

	WriteLoginInf(m_name,m_IP,m_port);
	EndDialog(1);
}
BOOL CLogin::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString name;
	CString ip;
	CString port;
	CString loginBmp;
	GetLoginInf(name,ip,port);
	SetDlgItemText(IDC_NAME,name);
	SetDlgItemText(IDC_IP,ip);
	SetDlgItemText(IDC_PORT,port);
	
	HICON m_hIcon1;
	m_hIcon1=AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	SetIcon(m_hIcon1, TRUE); //设置为大图标

	HBITMAP hBitmap = NULL;
	
#ifndef NEUTRAL_VERSION
	loginBmp.Format(_T("images/login.bmp"));
#else
	loginBmp.Format(_T("images/login_neutral.bmp"));
#endif
	hBitmap = (HBITMAP)::LoadImage(NULL,loginBmp, 
		IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE|LR_LOADFROMFILE);
	if(hBitmap == NULL)
	{
		AfxMessageBox(_T("加载图片失败!"));
		PostQuitMessage(0);
	}
	if (m_bkbitmap.m_hObject)
		m_bkbitmap.Detach();

	m_bkbitmap.Attach(hBitmap);

	BITMAP bm;
	m_bkbitmap.GetBitmap(&bm);
    SetWindowPos(NULL,0,0,bm.bmWidth,bm.bmHeight,NULL);

	CenterWindow();

	m_ok_btn.SetImage(_T("images/botton.bmp"),_T("images/botton.bmp"),
		_T("images/botton.bmp"),_T("images/botton.bmp"));
	
	m_cancel_btn.SetImage(_T("images/botton.bmp"),_T("images/botton.bmp"),
		_T("images/botton.bmp"),_T("images/botton.bmp"));
    

	m_ok_btn.SetTextPos(CPoint(13,4));
	m_cancel_btn.SetTextPos(CPoint(13,4));

	SetWindowText(_T("ProxyClient"));
	return TRUE; 
}

void CLogin::WriteLoginInf(const CString &name,const CString &ip,const CString &port)
{
	::WritePrivateProfileString(_T("CU_LOGIN"), _T("IP"), (LPCTSTR)ip, _T("./config.ini"));
	::WritePrivateProfileString(_T("CU_LOGIN"), _T("PORT"), (LPCTSTR)port, _T("./config.ini"));
	::WritePrivateProfileString(_T("CU_LOGIN"), _T("USER"), (LPCTSTR)name, _T("./config.ini"));

}

void CLogin::GetLoginInf(CString &name,CString &ip,CString &port)
{
	::GetPrivateProfileString(_T("CU_LOGIN"), _T("IP"), _T(""), ip.GetBufferSetLength(1024), 1024, _T("./config.ini"));
	::GetPrivateProfileString(_T("CU_LOGIN"), _T("PORT"), _T(""), port.GetBufferSetLength(1024), 1024, _T("./config.ini"));
	::GetPrivateProfileString(_T("CU_LOGIN"), _T("USER"), _T(""), name.GetBufferSetLength(1024), 1024, _T("./config.ini"));
	name.ReleaseBuffer();
	ip.ReleaseBuffer();
	port.ReleaseBuffer();
}

LRESULT CLogin::OnNcHitTest(CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	ScreenToClient(&point);

	RECT rtWindow;
	GetClientRect(&rtWindow);

	long wndHeight = rtWindow.bottom - rtWindow.top;
	long wndWidth = rtWindow.right - rtWindow.left;

	RECT rcW = {0,0,wndWidth,100};
	if(::PtInRect(&rcW,point))
	{  
		return HTCAPTION;  // 在拖动范围内
	}

	return CDialog::OnNcHitTest(point);
}

void CLogin::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	ChangeWindowRgn(&dc);

	//    CBitmap bgbitmap;
	//    bgbitmap.LoadBitmap(IDB_LOGINBG);
	CDC MemDC;
	BITMAP bm;
	m_bkbitmap.GetBitmap(&bm);

	int li_Width = bm.bmWidth;
	int li_Height = bm.bmHeight;

	MemDC.CreateCompatibleDC(&dc);
	CBitmap* pOldBitmap = MemDC.SelectObject(&m_bkbitmap);

	CRect rc;
	GetClientRect(&rc);
	int x=rc.left;
	int y=rc.top;
	while (y < (rc.Height()+rc.top)) 
	{
		while(x < (rc.Width()+rc.left)) 
		{
			dc.BitBlt(x, y, li_Width, li_Height, &MemDC, 0, 0, SRCCOPY);
			x += li_Width;
		}
		x = rc.left;
		y += li_Height;
	}


	MemDC.SelectObject(pOldBitmap);
	MemDC.DeleteDC();

// 	dc.SetTextColor(RGB(38,59,90));
// 	dc.SetBkMode(TRANSPARENT);
// 	dc.TextOut(20,8,_T("ProxyClient"));
	CDialog::OnPaint();
}

void CLogin::ChangeWindowRgn(CDC* pDC)
{
	COLORREF col = RGB(255,0,255);
	CRect rcClient;
	GetClientRect (rcClient);
	CRgn rgn;
	rgn.CreateRoundRectRgn (1, 1, rcClient.Width(), rcClient.Height(),6,6);

	SetWindowRgn (rgn, TRUE);
}


HBRUSH CLogin::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if(nCtlColor==CTLCOLOR_STATIC)
	{  
		pDC->SetBkMode(TRANSPARENT);  
		pDC->SetTextColor(RGB(68,129,2));
		return  (HBRUSH)GetStockObject(NULL_BRUSH); 

	}
	return hbr;
}

void CLogin::OnTimer(UINT_PTR nIDEvent)
{
    KillTimer(nIDEvent);
	gManage.m_pCommand->JUninit();//断开连接
	
	//m_waiting_static.ShowWindow(FALSE);//
	
	//MessageBox(_T("无法连接服务器"));
	CDialog::OnTimer(nIDEvent);
}


LRESULT CLogin::OnResult(WPARAM wParam, LPARAM lParam)
{

   MessageBox(_T("用户名密码错误!"));
	return 0;
}


DWORD CLogin::LoginThread(void* pParam){

	CLogin*	pDlg	= (CLogin*)pParam;
	int lRet = gManage.Login(pDlg);

	//m_waiting_static.ShowWindow(TRUE);

	if(0==lRet)
	{
		pDlg->Close();
	}
	else if (-2==lRet)
	{
		//pDlg->m_waiting_static.ShowWindow(FALSE);
		pDlg->MessageBox(_T("无法连接服务器!"));
		return 0;
	}
	else if(-1==lRet)
	{
		//pDlg->SendMessage(UM_ERR);
		pDlg->MessageBox(_T("用户名密码错误!"));
		return 0;;

	}
	else if (1==lRet)
	{
		//pDlg->m_waiting_static.ShowWindow(FALSE);
		pDlg->MessageBox(_T("此用户已经登录!"));
		return 0;;
	}
	else
	{
		pDlg->MessageBox(_T("登录失败!"));
	}
	return 0;
}
