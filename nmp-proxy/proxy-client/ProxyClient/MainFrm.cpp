
// MainFrm.cpp : CMainFrame 类的实现
//

#include "stdafx.h"
#include "ProxyClient3.h"

#include "log.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame
extern CManage gManage;
IMPLEMENT_DYNAMIC(CMainFrame, CFrameWndEx)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_VIEW_CUSTOMIZE, &CMainFrame::OnViewCustomize)
	ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, &CMainFrame::OnToolbarCreateNew)
	ON_COMMAND_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_OFF_2007_AQUA, &CMainFrame::OnApplicationLook)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_OFF_2007_AQUA, &CMainFrame::OnUpdateApplicationLook)
//	ON_COMMAND_RANGE(ID_SEPARATOR,ID_INDICATOR_SCRL,NULL)
	ON_WM_NCPAINT()
	ON_WM_TIMER()
	ON_WM_SIZE()
//	ON_WM_GETMINMAXINFO()
ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()

// CMainFrame 构造/析构
static UINT indicators[] =
{
	ID_SEPARATOR,           // 状态行指示器
	ID_INDICATOR_NUM,
	ID_INDICATOR_NUM,
	ID_INDICATOR_NUM,
};

CMainFrame::CMainFrame()
: m_heart_elapse(0)
, m_timeout(0)
{
	// TODO: 在此添加成员初始化代码
	theApp.m_nAppLook = theApp.GetInt(_T("ApplicationLook"), ID_VIEW_APPLOOK_OFF_2007_BLACK);
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;	
	// 基于持久值设置视觉管理器和样式
	//OnApplicationLook(theApp.m_nAppLook);


	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("未能创建状态栏\n");
		return -1;      // 未能创建
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
	// 设置视觉管理器使用的视觉样式
	CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);

	if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW, CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
	{
		TRACE0("未能创建视图窗口\n");
		return -1;
	}
	
	SetMenu(NULL);
	//ModifyStyle(WS_SYSMENU, 0);

	HICON hIcon = ::LoadIcon(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDR_MAINFRAME));
	SetIcon(hIcon, TRUE);

	//刷新窗口
	CRect rc;
	GetWindowRect(&rc);
	SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_FRAMECHANGED|SWP_NOZORDER);

    CProxyClient3App* pApp = (CProxyClient3App*)AfxGetApp(); 
	m_heart_elapse = pApp->m_heart_elapse;
	SetTimer(2,m_heart_elapse*1000,NULL);   // m_heart_elapse是proxy server发过来的心跳周期
	SetTimer(1,1000,NULL);
	
	LOGFONT logfont;
	GetObject(::GetStockObject(DEFAULT_GUI_FONT),sizeof(logfont),&logfont); 
	logfont.lfCharSet=GB2312_CHARSET; 
	logfont.lfWeight = FW_NORMAL; 
	logfont.lfHeight = 18; 
	lstrcpy(logfont.lfFaceName, _T("微软雅黑"));
	static CFont sfontStatusBar;
	sfontStatusBar.CreateFontIndirect(&logfont);
	
	m_wndStatusBar.SetFont(&sfontStatusBar);

	if ("admin"==gManage.m_name)
	{
		m_wndStatusBar.SetPaneText(1,_T("超级用户"));
	}
	else
		m_wndStatusBar.SetPaneText(1,_T("普通用户"));

	m_wndStatusBar.SetPaneWidth(2,50);
	m_wndStatusBar.SetPaneText(2,gManage.m_name);
	m_wndStatusBar.SetPaneWidth(3,50);
	m_wndStatusBar.SetPaneWidth(1,50);
	
	//gManage.SendHeart();
	
	//MoveWindow(0,0,1000,650,0);
	//CenterWindow();



	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWndEx::PreCreateWindow(cs) )
		return FALSE;
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式


 	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;

 	cs.style&=~WS_MAXIMIZEBOX; 
// 	cs.style&=~WS_THICKFRAME; 
//  	cs.cx=940; 
//  	cs.cy=632; 


	cs.lpszClass = AfxRegisterWndClass(0);
	return TRUE;
}

// CMainFrame 诊断

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// CMainFrame 消息处理程序

void CMainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
	// 将焦点前移到视图窗口
	m_wndView.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// 让视图第一次尝试该命令
	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// 否则，执行默认处理
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainFrame::OnViewCustomize()
{
	CMFCToolBarsCustomizeDialog* pDlgCust = new CMFCToolBarsCustomizeDialog(this, TRUE /* 扫描菜单*/);
	pDlgCust->EnableUserDefinedToolbars();
	pDlgCust->Create();
}

LRESULT CMainFrame::OnToolbarCreateNew(WPARAM wp,LPARAM lp)
{
	LRESULT lres = CFrameWndEx::OnToolbarCreateNew(wp,lp);
	if (lres == 0)
	{
		return 0;
	}

	CMFCToolBar* pUserToolbar = (CMFCToolBar*)lres;
	ASSERT_VALID(pUserToolbar);

	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
	return lres;
}

void CMainFrame::OnApplicationLook(UINT id)
{
	CWaitCursor wait;

	theApp.m_nAppLook = id;

	switch (theApp.m_nAppLook)
	{
	case ID_VIEW_APPLOOK_WIN_2000:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
		break;

	case ID_VIEW_APPLOOK_OFF_XP:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
		break;

	case ID_VIEW_APPLOOK_WIN_XP:
		CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
		break;

	case ID_VIEW_APPLOOK_OFF_2003:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_VS_2005:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	default:
		switch (theApp.m_nAppLook)
		{
		case ID_VIEW_APPLOOK_OFF_2007_BLUE:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_BLACK:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_SILVER:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_AQUA:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
			break;
		}

		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CDockingManager::SetDockingMode(DT_SMART);
	}

	RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

	theApp.WriteInt(_T("ApplicationLook"), theApp.m_nAppLook);
}

void CMainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_nAppLook == pCmdUI->m_nID);
}

void CMainFrame::OnNcPaint()
{

	CFrameWndEx::OnNcPaint();  
	CDC* pDC = GetWindowDC();  
	int x = GetSystemMetrics(SM_CXSIZE)+GetSystemMetrics(SM_CXBORDER)+GetSystemMetrics(SM_CXDLGFRAME);  
	int y = GetSystemMetrics(SM_CYDLGFRAME);  
	CRect CapRct;  
	GetWindowRect(&CapRct);  
	int y1 = GetSystemMetrics(SM_CYICON)-GetSystemMetrics(SM_CYDLGFRAME)-GetSystemMetrics(SM_CYBORDER);  
	int x1 = CapRct.Width ()- GetSystemMetrics(SM_CXSIZE)-GetSystemMetrics(SM_CXBORDER)-GetSystemMetrics(SM_CXDLGFRAME);  

	CapRct.left = x;  
	CapRct.top = y;  
	CapRct.right = x1;  
	CapRct.bottom = y1;  
	//pDC->FillSolidRect(&CapRct,RGB(0,0,0));  
	pDC->SetBkMode (TRANSPARENT);  
	pDC->SetTextColor (RGB(255,255,255));  
	pDC->DrawText (_T("ProxyClient"),&CapRct, DT_SINGLELINE | DT_LEFT | DT_VCENTER); 
	SetWindowText(_T("ProxyClient"));
	ReleaseDC(pDC);
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CFrameWndEx::OnNcPaint()
}

void CMainFrame::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//if (2==nIDEvent)
	//{// 该定时器每m_heart_elapse秒发送一次心跳, 
	//	TRACE("MainWin\n");
	//	gManage.SendHeart();
	//}

	if (1 == nIDEvent)
	{//@{每1s执行一次}
		m_timeout++;
		//TRACE("time%d\n",m_timeout);
		CTime time;
		time = CTime::GetCurrentTime();
		CString str = time.Format("%H:%M:%S");
		m_wndStatusBar.SetPaneText(3,str);
		if (m_timeout > m_heart_elapse*5)
		{//@{当m_timeout大于3*心跳周期时, 提示用户连接断后, 说明proxy server 在3*心跳周期内没有响应}
			KillTimer(1);
			KillTimer(2);
			gManage.m_pCommand->JUninit();  //断开连接;  
			LOG("[WARNING] Timeout ProxyClient reset the connection with Proxy Server, timeout:%d, heart time:%d\n", m_timeout, m_heart_elapse);			
			MessageBox(_T("服务器连接已断开！"));
		}
	}


	CFrameWndEx::OnTimer(nIDEvent);
}

void CMainFrame::ShowTime(CString strTime)
{
	m_wndStatusBar.SetPaneText(3,strTime);
}

void CMainFrame::OnSize(UINT nType, int cx, int cy)
{
	CFrameWndEx::OnSize(nType, cx, cy);



	// TODO: 在此处添加消息处理程序代码
}

//void CMainFrame::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
//{
//	
//
//	lpMMI->ptMinTrackSize.x = 900;
//	lpMMI->ptMinTrackSize.y = 600;
//
//	CFrameWndEx::OnGetMinMaxInfo(lpMMI);
//}

void CMainFrame::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	

	lpMMI->ptMinTrackSize.x = 940;
	lpMMI->ptMinTrackSize.y = 660;

	lpMMI->ptMaxTrackSize.x = 940;
	lpMMI->ptMaxTrackSize.y = 660;
	

	CFrameWndEx::OnGetMinMaxInfo(lpMMI);
}


BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN 
		&& (pMsg->lParam ==VK_RETURN || pMsg->lParam == VK_ESCAPE))
	{
		MessageBox(_T("PreTranslateMessage"), NULL, MB_OK);
		return TRUE;
	}

	return CFrameWndEx::PreTranslateMessage(pMsg);
}
