
// ChildView.cpp : CChildView 类的实现
//

#include "stdafx.h"
#include "ProxyClient3.h"
#include "ChildView.h"
#include "resource.h"
#include "MainFrm.h"
#include "Login.h"
#include "resource.h"
#include "ServerConfig.h"
#include "DataSafe.h"
#include "log.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#define VIEW_DEVICE  WM_USER+1
#define VIEW_USER    WM_USER+2
#define VIEW_LOGOUT  WM_USER+3
#define VIEW_CONFIG  WM_USER+4
#define VIEW_DATA	 WM_USER+5


// CChildView
extern CManage gManage;
CChildView::CChildView()
: m_timeout(0)
, m_heart_elapse(0)
{
	m_prameter = new S_Prameter;
	m_prameter->pMaindeviceDlg = &m_MainDevice;
	m_prameter->pMainUserDlg = &m_MainUser;
}

CChildView::~CChildView()
{
	delete m_prameter;
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(VIEW_DEVICE,OnMainDevice)
	ON_COMMAND(VIEW_USER,OnMainUser)
	ON_COMMAND(VIEW_LOGOUT,OnLogout)
	ON_COMMAND(VIEW_CONFIG,OnConfig)
	ON_COMMAND(VIEW_DATA,OnData)
	ON_WM_TIMER()
END_MESSAGE_MAP()



// CChildView 消息处理程序

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this); // 用于绘制的设备上下文
	
	CRect rcClient;
	CFont font;
	font.CreateFont(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("宋体"));
	dc.SelectObject(&font);
	
	GetClientRect(&rcClient);
	
	dc.SetTextColor(RGB(38,59,90));
	dc.SetBkMode(TRANSPARENT);
	
	//dc.TextOut(rcClient.right-120,rcClient.top+8,_T("当前用户: ")+ gManage.m_name);
    font.DeleteObject();


	CBitmap bitmap;
	bitmap.LoadBitmap(IDB_HEAD);
	CDC memdc;
	memdc.CreateCompatibleDC(&dc);
	BITMAP bit;
	bitmap.GetBitmap(&bit);
	memdc.SelectObject(&bitmap);
   // rcClient.InflateRect(1,1,1,1);
 	dc.StretchBlt(0,0,rcClient.Width(),40,&memdc,0,0,bit.bmWidth,bit.bmHeight,SRCCOPY);

	//dc.BitBlt(0,0,rcClient.Width(),rcClient.Height(),&memdc,0,0,SRCCOPY);
	// 不要为绘制消息而调用 CWnd::OnPaint()
}


int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码

	m_DeviceBtn.Create(_T(""), WS_CHILD | WS_VISIBLE|BS_OWNERDRAW, CRect(0, 0, 0, 0), this, VIEW_DEVICE);
	m_UserBtn.Create(_T(""), WS_CHILD | WS_VISIBLE|BS_OWNERDRAW, CRect(0, 0, 0, 0), this, VIEW_USER);
	m_config_btn.Create(_T("配置"), WS_CHILD | WS_VISIBLE|BS_OWNERDRAW, CRect(0, 0, 0, 0), this, VIEW_CONFIG);
	m_logout_btn.Create(_T("注销"), WS_CHILD | WS_VISIBLE|BS_OWNERDRAW, CRect(0, 0, 0, 0), this, VIEW_LOGOUT);
	m_data_btn.Create(_T("数据"), WS_CHILD | WS_VISIBLE|BS_OWNERDRAW, CRect(0, 0, 0, 0), this, VIEW_DATA);


    m_UserBtn.SetTextPos(CPoint(10,8));
	m_DeviceBtn.SetTextPos(CPoint(10,8));
	m_logout_btn.SetTextPos(CPoint(33,8));
	m_config_btn.SetTextPos(CPoint(33,8));
	m_data_btn.SetTextPos(CPoint(33,8));

	m_DeviceBtn.SetImage(_T("images/normal_device_btn.bmp"),_T("images/sel_device_btn.bmp"),
		_T("images/sel_device_btn.bmp"),_T("images/sel_device_btn.bmp"));
	
	m_UserBtn.SetImage(_T("images/normal_user_btn.bmp"),_T("images/sel_user_btn.bmp"),
		_T("images/sel_user_btn.bmp"),_T("images/sel_user_btn.bmp"));
	
	m_logout_btn.SetImage(_T("images/normal_logout.bmp"),_T("images/hot_loutout.bmp"),
		_T("images/hot_loutout.bmp"),_T("images/hot_loutout.bmp"));
	
	m_config_btn.SetImage(_T("images/normal_set.bmp"),_T("images/hot_set.bmp"),
		_T("images/hot_set.bmp"),_T("images/hot_set.bmp"));

	m_data_btn.SetImage(_T("images/normal_set.bmp"),_T("images/hot_set.bmp"),
		_T("images/hot_set.bmp"),_T("images/hot_set.bmp"));

	m_logout_btn.SetTextColor(RGB(0,0,0));
	m_config_btn.SetTextColor(RGB(0,0,0));
	m_data_btn.SetTextColor(RGB(0,0,0));
	m_DeviceBtn.ShowWindow(SW_SHOW);
	m_UserBtn.ShowWindow(SW_SHOW);

	m_DeviceBtn.EnableWindow(FALSE);
	

	m_MainDevice.Create(IDD_MAINDEVICE,this);
	m_MainUser.Create(IDD_MAINUSER,this);
	m_MainDevice.ShowWindow(SW_SHOW);
	m_MainUser.ShowWindow(SW_HIDE);



	gManage.RegCallBack(m_prameter);//注册回调函数
	gManage.GetBaseInfo();//获取支持的设备厂家，类型，版本
	return 0;
}

void CChildView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	
	int fX = cx;
	int fY = cy;
	CBitmap normalbmp ;
	normalbmp.LoadBitmap(IDB_BITMAP1);
	BITMAPINFO bmpinfo;
	normalbmp.GetObject(sizeof(BITMAPINFO),&bmpinfo);
	CRect rect;
	this->GetClientRect(&rect);
	m_MainDevice.MoveWindow(rect.left,rect.top + 40/*bmpinfo.bmiHeader.biHeight*/,rect.right - rect.left,rect.bottom - rect.top-15);
	m_MainUser.MoveWindow(rect.left,rect.top + 40/*bmpinfo.bmiHeader.biHeight*/,rect.right - rect.left,rect.bottom - rect.top-15);
	
	if(m_DeviceBtn.GetSafeHwnd())
	{
		m_DeviceBtn.MoveWindow(13 , 40-bmpinfo.bmiHeader.biHeight, bmpinfo.bmiHeader.biWidth, bmpinfo.bmiHeader.biHeight);
	}
	if(m_UserBtn.GetSafeHwnd())
	{
		m_UserBtn.MoveWindow(13+bmpinfo.bmiHeader.biWidth , 40-bmpinfo.bmiHeader.biHeight, bmpinfo.bmiHeader.biWidth, bmpinfo.bmiHeader.biHeight);
	}


	if(m_logout_btn.GetSafeHwnd())
	{
		m_logout_btn.MoveWindow(rect.right-70 , 15, 60, 25);
	}
	if(m_config_btn.GetSafeHwnd())
	{
		m_config_btn.MoveWindow(rect.right-132 , 15, 60, 25);
	}
	if (m_data_btn.GetSafeHwnd())
	{
		m_data_btn.MoveWindow(rect.right-194 , 15, 60, 25);
	}


	// TODO: 在此处添加消息处理程序代码
}

void CChildView::OnMainDevice(){
	m_MainDevice.ShowWindow(SW_SHOW);
	m_MainUser.ShowWindow(SW_HIDE);

	m_DeviceBtn.EnableWindow(FALSE);//选中的背景更改
	m_UserBtn.EnableWindow(TRUE);
	
}

void CChildView::OnMainUser(){
	if (0 == m_MainUser.m_tag)
	{
		m_MainUser.OnBnClickedFind();
		m_MainUser.m_tag = 1;
	}
	m_MainUser.ShowWindow(SW_SHOW);
	m_MainDevice.ShowWindow(SW_HIDE);

   	m_UserBtn.EnableWindow(FALSE);
   	m_DeviceBtn.EnableWindow(TRUE);
}
void CChildView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
    if (1 == nIDEvent)
    {
		m_timeout++;
		if (m_timeout>m_heart_elapse*3)
		{
			KillTimer(1);
			gManage.m_pCommand->JUninit();  //断开连接;
			LOG("[WANRING] CChileView Proxyclient reset the connection with Proxy Server\n");
			MessageBox(_T("服务器连接已断开！"));
		}
		TRACE("ViewWin%d\n",m_timeout);
    }
	
	CWnd::OnTimer(nIDEvent);
}

extern CProxyClient3App theApp;
void CChildView::OnLogout(void)
{
	TCHAR exeFullPath[MAX_PATH]; // MAX_PATH
	GetModuleFileName(NULL, exeFullPath, MAX_PATH);//得到程序模块名称，全路径
	STARTUPINFO si; 
	PROCESS_INFORMATION pi; 
	ZeroMemory(&si,sizeof(si)); 
	si.cb=sizeof(si);
	CreateProcess(NULL,exeFullPath,NULL, NULL,FALSE, NORMAL_PRIORITY_CLASS,NULL,NULL,&si,&pi);
	/*HANDLE hCurrentProcess;
	hCurrentProcess  = ::GetCurrentProcess();
	TerminateProcess(hCurrentProcess,0);*/
	GetParent()->PostMessage(WM_CLOSE);
}

void CChildView::OnConfig(void)
{
	CServerConfig dlg;
	dlg.SetTitle(_T("服务器配置"));
	if (dlg.DoModal()==IDOK)
	{
		OnLogout();
	}
}

void CChildView::OnData(void)
{
	CDataSafe dlg;
	dlg.SetTitle(_T("数据安全"));
	dlg.DoModal();
}