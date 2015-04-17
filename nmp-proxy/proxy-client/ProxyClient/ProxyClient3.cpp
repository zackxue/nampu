
// ProxyClient3.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "log.h"
#include "afxwinappex.h"
#include "ProxyClient3.h"
#include "MainFrm.h"
#include "Login.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CProxyClient3App

BEGIN_MESSAGE_MAP(CProxyClient3App, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CProxyClient3App::OnAppAbout)
END_MESSAGE_MAP()


// CProxyClient3App 构造

CProxyClient3App::CProxyClient3App()
: m_heart_elapse(15)
{

	m_bHiColorIcons = TRUE;

	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}

// 唯一的一个 CProxyClient3App 对象

CProxyClient3App theApp;
CManage  gManage;
int      gCleanList_tag = 0;
//IJDCUCommand *m_pCommand= JDCUCreateCommand(); ///TEST


// CProxyClient3App 初始化

CProxyClient3App:: ~CProxyClient3App()
{
	LOG("[INFO] Now Stop To Run ProxyClient\n");
}

BOOL CProxyClient3App::InitInstance()
{
	LOG("**************************************************************\n");
	LOG("[INFO] Now Start To Run ProxyClient\n");

	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	// 初始化 OLE 库
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));

	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// 若要创建主窗口，此代码将创建新的框架窗口
	// 对象，然后将其设置为应用程序的主窗口对象
	
	
	CLogin dlg;
	int response = dlg.DoModal();


	if(response!=IDOK)
		return FALSE;
 	
	
	CMainFrame* pFrame = new CMainFrame;
	if (!pFrame)
		return FALSE;
	m_pMainWnd = pFrame;
	pFrame->LoadFrame(IDR_MAINFRAME,
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL,
		NULL);
	
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();

	return TRUE;
}


// CProxyClient3App 消息处理程序




// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// 用于运行对话框的应用程序命令
void CProxyClient3App::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CProxyClient3App 自定义加载/保存方法

void CProxyClient3App::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
}

void CProxyClient3App::LoadCustomState()
{
}

void CProxyClient3App::SaveCustomState()
{
}

// CProxyClient3App 消息处理程序



