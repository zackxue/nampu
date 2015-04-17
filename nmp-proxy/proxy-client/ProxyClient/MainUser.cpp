// MainUser.cpp : 实现文件
//

#include "stdafx.h"
#include "ProxyClient3.h"
#include "MainUser.h"


// CMainUser 对话框
extern CManage gManage;

IMPLEMENT_DYNAMIC(CMainUser, CDialog)

CMainUser::CMainUser(CWnd* pParent /*=NULL*/)
	: CDialog(CMainUser::IDD, pParent)
	, m_offset(0)
	, m_count(0)
	, m_username(_T(""))
	, m_tag(0)
{

}

CMainUser::~CMainUser()
{
}

void CMainUser::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_ADD, m_add_btn);
	DDX_Control(pDX, IDC_DEL, m_del_btn);
	DDX_Control(pDX, IDC_MODIFY, m_modify_btn);
	DDX_Control(pDX, IDC_FIND, m_find_btn);
	DDX_Control(pDX, IDC_FINDNAME, m_find_edit);
	DDX_Control(pDX, IDC_FIND, m_find_btn);

	DDX_Control(pDX, IDC_BACK, m_back_btn);
	DDX_Control(pDX, IDC_NEXT, m_next_btn);
	DDX_Control(pDX, IDOK, m_goto_btn);
	DDX_Control(pDX, IDC_PAGECOUNT, m_page_edit);
	DDX_Control(pDX, IDC_SHOWPAGE, m_showpage_static);
	DDX_Control(pDX, IDC_USERLIST, m_UserList);
	DDX_Control(pDX, IDC_FIRST, m_first_btn);
	DDX_Control(pDX, IDC_TAIL, m_tail_btn);
}


BEGIN_MESSAGE_MAP(CMainUser, CDialog)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_ADD, &CMainUser::OnBnClickedAdd)
	ON_BN_CLICKED(IDC_FIND, &CMainUser::OnBnClickedFind)
	ON_BN_CLICKED(IDC_DEL, &CMainUser::OnBnClickedDel)
	ON_BN_CLICKED(IDC_MODIFY, &CMainUser::OnBnClickedModify)
	ON_BN_CLICKED(IDC_BACK, &CMainUser::OnBnClickedBack)
	ON_BN_CLICKED(IDC_NEXT, &CMainUser::OnBnClickedNext)
	//ON_BN_CLICKED(IDOK, &CMainUser::OnBnClickedGoto)
	ON_WM_PAINT()
	ON_NOTIFY(NM_RCLICK, IDC_USERLIST, &CMainUser::OnNMRClickUserlist)
	ON_COMMAND(ID_DEVICE_UPDATE, &CMainUser::OnUserUpdate)
	ON_COMMAND(ID_DEVICE_ADD, &CMainUser::OnUserAdd)
	ON_COMMAND(ID_DEVICE_DEL, &CMainUser::OnUserDel)
	ON_COMMAND(ID_DEVICE_MODIFY, &CMainUser::OnUserModify)
	ON_COMMAND(ID_DEVICE_QUERY, &CMainUser::OnUserQuery)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_FIRST, &CMainUser::OnBnClickedFirst)
	ON_BN_CLICKED(IDC_TAIL, &CMainUser::OnBnClickedTail)
	ON_NOTIFY(NM_DBLCLK, IDC_USERLIST, &CMainUser::OnNMDblclkUserlist)
END_MESSAGE_MAP()


// CMainUser 消息处理程序

int CMainUser::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
// 	m_UserList.Create(LVS_REPORT|LVS_SHOWSELALWAYS|WS_BORDER,
// 		CRect(0,0,0,0),this,IDC_USERLIST);
// 	m_UserList.InsertColumn(0,_T("用户名"));// 设置列
// 	m_UserList.InsertColumn(1,_T("备注"));
//	m_UserList.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);



//	m_UserList.ShowWindow(SW_SHOW);

// 	CFont m_font1,m_font2;
// 	m_font1.CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
// 	m_font2.CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
// 	m_UserList.SetFont(&m_font2);
// 	m_UserList.SetTextColor(RGB(38,59,90));
// 	m_UserList.GetHeaderCtrl()->SetFont(&m_font1);









	return 0;
}

void CMainUser::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	CRect rect;
	GetClientRect(&rect);

	if(m_UserList.GetSafeHwnd()==NULL)
		return;

	m_UserList.SetWindowPos(NULL, 0, 0, 
		rect.right - rect.left, rect.bottom - rect.top-50, NULL);

	if(m_find_btn.GetSafeHwnd())//查询
	{
		m_find_btn.MoveWindow(rect.left+425,rect.bottom-48+3/*statebar+*/,50,20);
	}

	if(m_find_edit.GetSafeHwnd())//查询名称
	{
		m_find_edit.MoveWindow(rect.left+320,rect.bottom-48+3/*statebar+*/,100,20);
	}
	

	if(m_add_btn.GetSafeHwnd())//添加
	{
		m_add_btn.MoveWindow(rect.left+80,rect.bottom-48+3/*statebar+*/,40,20);
	}

	if(m_del_btn.GetSafeHwnd())//删除
	{
		m_del_btn.MoveWindow(rect.left+80+42,rect.bottom-48+3/*statebar+*/,40,20);
	}
	if(m_modify_btn.GetSafeHwnd())//修改
	{
		m_modify_btn.MoveWindow(rect.left+80+42+42,rect.bottom-48+3/*statebar+*/,40,20);
	}

	if(m_first_btn.GetSafeHwnd())//首页
	{
		m_first_btn.MoveWindow(rect.right - 50-27-42-32-42-42-42-27,rect.bottom-48+3/*statebar+*/,40,20);
	}

	if(m_back_btn.GetSafeHwnd())//上一页
	{
		m_back_btn.MoveWindow(rect.right - 50-27-42-32-42-42-27,rect.bottom-48+3/*statebar+*/,40,20);
	}

	if(m_showpage_static.GetSafeHwnd())//1/4
	{
		m_showpage_static.MoveWindow(rect.right - 50-27-42-42-32-27,rect.bottom-48+4/*statebar+*/,55,20);

	}
	if(m_next_btn.GetSafeHwnd())//下一页
	{
		m_next_btn.MoveWindow(rect.right - 50-27-42-42-2,rect.bottom-48+3/*statebar+*/,40,20);
	}
	if(m_tail_btn.GetSafeHwnd())//尾页
	{
		m_tail_btn.MoveWindow(rect.right - 50-27-42-2,rect.bottom-48+3/*statebar+*/,40,20);
	}

	if(m_page_edit.GetSafeHwnd())//页面输入
	{
		m_page_edit.MoveWindow(rect.right - 50-27,rect.bottom-48+3/*statebar+*/,30,20);
	}

	if(m_goto_btn.GetSafeHwnd())//跳转
	{
		m_goto_btn.MoveWindow(rect.right - 45,rect.bottom-48+3/*statebar+*/,37,20);
	}


	int width = ( rect.right - rect.left-10)/3;
	for ( int i=0; i<3; i++ )
		m_UserList.SetColumnWidth( i, width );


}

void CMainUser::OnBnClickedAdd()
{


	CUserAddDlg dlg;
	dlg.m_pParent = this;
	dlg.DoModal();
	//MessageBox(_T("tt"));

}
extern int gCleanList_tag;
void CMainUser::OnBnClickedFind()
{
	CString find_name;
	GetDlgItemText(IDC_FINDNAME,find_name);
	m_UserList.DeleteAllItems();//清空。。。。


	gCleanList_tag = 0;

	for (int i=0;i<TIMES;i++)
	{
		gManage.GetUserList(find_name,i*5 + m_offset);
	}
	ShowPage();
	Invalidate();
	// TODO: 在此添加控件通知处理程序代码
}

void CMainUser::ShowUser(void)
{

}

void CMainUser::OnBnClickedDel()
{
	int nItem=m_UserList.GetNextItem(-1,LVNI_SELECTED);	
	CString temp_name = m_UserList.GetItemText(nItem,0);   	
	int ret = -1;
	if(-1 == nItem)
	{
		MessageBox(_T("请选择删除的用户！"));
		return;
	}
	if (MessageBox(_T("是否删除该用户"),_T("请确认"),MB_YESNO)==IDYES)
	{
		ret = gManage.DelUser(temp_name);
		
		if (ret == 0)
		{
			m_UserList.DeleteItem(nItem);

			MessageBox(_T("删除用户成功！"));

		}
		else
			MessageBox(_T("删除用户失败！"));

	}
	// TODO: 在此添加控件通知处理程序代码
}

void CMainUser::OnBnClickedModify()
{
	int nItem=m_UserList.GetNextItem(-1,LVNI_SELECTED);	
	if (-1==nItem)
	{
		MessageBox(_T("请选择要修改的用户"));
		return ;
	}

	if ("admin" == gManage.m_name)
	{
		int ret=-1;
		int nItem=m_UserList.GetNextItem(-1,LVNI_SELECTED);	
		CString temp_name = m_UserList.GetItemText(nItem,0);
		if(-1 == nItem)
		{
			MessageBox(_T("请选择修改的用户！"));
			return;
		}
		
		if ("admin" == temp_name)
		{
			CUserModify1 dlg;
			dlg.m_name = temp_name;
			dlg.DoModal();

		}
		else
		{
			CUserModify2 dlg;	
			dlg.m_name = temp_name;
			dlg.DoModal();

		}

	}
	else
	{
		CUserModify1 dlg;
		CString temp_name = m_UserList.GetItemText(nItem,0);
		dlg.m_name = gManage.m_name;
		if (temp_name == gManage.m_name)
		{
			dlg.DoModal();
		}
		else
		{
			MessageBox(_T("普通用户只能修改自己信息"), MB_OK);
		}
	}
	// TODO: 在此添加控件通知处理程序代码
}

//BOOL CMainUser::PreTranslateMessage(MSG* pMsg)
//{
//	// TODO: 在此添加专用代码和/或调用基类
//
//
//		if (pMsg->message==WM_KEYDOWN&&pMsg->wParam==VK_RETURN)
//			{
//				CString str;
//				GetClassName(pMsg->hwnd,str.GetBuffer(MAX_PATH),MAX_PATH);
//				if ("Edit"==str)
//				{
//
//
//
//					
//					pMsg->wParam = VK_TAB;
//				}
//			}
//		
//	return CDialog::PreTranslateMessage(pMsg);
//}

void CMainUser::OnOK()
{
	if(GetDlgItem(IDC_FINDNAME) == GetFocus())
	{

		OnBnClickedFind();
		return ;
	}

	if(GetDlgItem(IDC_PAGECOUNT) == GetFocus())
	{

		int page = GetDlgItemInt(IDC_PAGECOUNT);
		int total = m_count/COUNT_PER_PAG+1;
		if (page>total||page<1)
		{
			MessageBox(_T("没有该页码!"));
		}
		else{
			m_offset = (page-1)*COUNT_PER_PAG;
			m_UserList.DeleteAllItems();
			gCleanList_tag = 0;
			for (int i=0;i<TIMES;i++)
			{	
				gManage.GetUserList(m_username,i*5+m_offset);
			}
		}
		ShowPage();
	}

	//Invalidate();
}

void CMainUser::OnBnClickedBack()
{
	if (m_offset>0)
	{
		if (m_offset>=COUNT_PER_PAG)
			m_offset-=COUNT_PER_PAG;
		else
			m_offset = 0;
		m_UserList.DeleteAllItems();
		gCleanList_tag = 0;
		for (int i=0;i<TIMES;i++)
		{

			gManage.GetUserList(m_username,m_offset);
		}
	}
ShowPage();
}

void CMainUser::OnBnClickedNext()
{	
	if ((m_offset+COUNT_PER_PAG)<m_count)
	{
		m_offset+=COUNT_PER_PAG;
		m_UserList.DeleteAllItems();
		gCleanList_tag = 0;
		for (int i=0;i<TIMES;i++)
		{	
			gManage.GetUserList(m_username,i*5+m_offset);
		}
	}
ShowPage();
}

void CMainUser::OnBnClickedGoto()
{

}

void CMainUser::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CDialog::OnPaint()

	CRect rcClient;
	GetClientRect(&rcClient);
	CBitmap bitmap;
	bitmap.LoadBitmap(IDB_BOTTOMBK);
	CDC memdc;
	memdc.CreateCompatibleDC(&dc);
	BITMAP bit;
	bitmap.GetBitmap(&bit);
	memdc.SelectObject(&bitmap);
	// rcClient.InflateRect(1,1,1,1);
	dc.StretchBlt(0,0,rcClient.Width(),rcClient.Height(),&memdc,0,0,bit.bmWidth,bit.bmHeight,SRCCOPY);

}

void CMainUser::OnNMRClickUserlist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	DWORD dwPos = GetMessagePos();
	CPoint point( LOWORD(dwPos), HIWORD(dwPos) );

	CMenu menu;
	VERIFY( menu.LoadMenu(IDR_DEVICE));
	CMenu* popup = menu.GetSubMenu(0);

	ASSERT( popup != NULL );

	if (popup)
	{
		int nItem=m_UserList.GetNextItem(-1,LVNI_SELECTED);	
		CString temp_name = m_UserList.GetItemText(nItem,0);   	
		int ret = -1;
		if (-1==nItem)
		{
			popup->EnableMenuItem(ID_DEVICE_DEL, MF_GRAYED);
			popup->EnableMenuItem(ID_DEVICE_MODIFY, MF_GRAYED);
		}
		else
		{
			popup->EnableMenuItem(ID_DEVICE_DEL, MF_ENABLED);
			popup->EnableMenuItem(ID_DEVICE_MODIFY, MF_ENABLED);
		}
		popup->EnableMenuItem(ID_DEVICE_QUERY, MF_GRAYED);

		popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	}

	*pResult = 0;
}

void CMainUser::OnUserUpdate()
{
	m_offset = 0;
	SetDlgItemText(IDC_FINDNAME,_T(""));
	m_UserList.DeleteAllItems();//清空。。。。
	gCleanList_tag = 0;
	for (int i=0;i<TIMES;i++)
	{
		gManage.GetUserList(_T(""),i*5 + m_offset);
	}
}

void CMainUser::OnUserAdd()
{
	OnBnClickedAdd();
}

void CMainUser::OnUserDel()
{
	OnBnClickedDel();
}

void CMainUser::OnUserModify()
{
	OnBnClickedModify();
}

void CMainUser::OnUserQuery()
{
	OnBnClickedFind();
}

BOOL CMainUser::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_UserList.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_TWOCLICKACTIVATE);
	m_UserList.InsertColumn(0,_T("用户名"));// 设置列
	m_UserList.InsertColumn(1,_T("用户类型"));
	m_UserList.InsertColumn(2,_T("备注"));

	
// 	m_ImageList.Create(1,19,ILC_COLOR24,1,1); 
// 	m_UserList.SetImageList(&m_ImageList,LVSIL_SMALL); 
	m_imagelist.Create(19,19,TRUE,2,2);
	m_imagelist.Add(AfxGetApp()->LoadIcon(IDI_ICON2));
	m_UserList.SetImageList(&m_imagelist,LVSIL_SMALL);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

HBRUSH CMainUser::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

void CMainUser::OnBnClickedFirst()
{
	m_offset = 0;
	SetDlgItemText(IDC_FINDNAME,_T(""));
	m_UserList.DeleteAllItems();//清空。。。。
	gCleanList_tag = 0;
	for (int i=0;i<TIMES;i++)
	{
		gManage.GetUserList(_T(""),i*5 + m_offset);
	}
	ShowPage();
}

void CMainUser::OnBnClickedTail()
{
	int totaltcount = (m_count-1)/COUNT_PER_PAG+1;
	m_offset = (totaltcount-1)*COUNT_PER_PAG;
	SetDlgItemText(IDC_FINDNAME,_T(""));
	m_UserList.DeleteAllItems();//清空。。。。
	gCleanList_tag = 0;
	for (int i=0;i<TIMES;i++)
	{
		gManage.GetUserList(_T(""),i*5 + m_offset);
	}
	ShowPage();
}

void CMainUser::ShowPage(void)
{
	CString str;
	int totaltcount = (m_count-1)/COUNT_PER_PAG+1;
	int currentpage = m_offset/COUNT_PER_PAG+1;
	str.Format(_T("%d / %d"),currentpage,totaltcount);
	SetDlgItemText(IDC_SHOWPAGE,str);
}

void CMainUser::OnNMDblclkUserlist(NMHDR *pNMHDR, LRESULT *pResult)
{	
	int nItem=m_UserList.GetNextItem(-1,LVNI_SELECTED);	
	if (-1!=nItem)
	{
		OnBnClickedModify();
	}
}

BOOL CMainUser::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN 
		&& (pMsg->wParam ==VK_RETURN || pMsg->wParam == VK_ESCAPE))
	{
		TRACE("PreTranslateMessage wParam:%d lParam:%d\n", pMsg->wParam, pMsg->lParam);
		//MessageBox(_T("PreTranslateMessage"), NULL, MB_OK);
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}
