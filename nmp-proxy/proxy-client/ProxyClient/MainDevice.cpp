// MainDevice.cpp : 实现文件
//

#include "stdafx.h"
#include "ProxyClient3.h"
#include "MainDevice.h"
#include "Login.h"
#include "DeviceADD.h"
#include "ChildView.h"
#include "DeviceFind.h"
#include "resource.h"
#include "log.h"


// CMainDevice 对话框
extern CManage gManage;
IMPLEMENT_DYNAMIC(CMainDevice, CDialog)

CMainDevice::CMainDevice(CWnd* pParent /*=NULL*/)
	: CDialog(CMainDevice::IDD, pParent)
	, m_tag_firstpage(1)
	, m_factoryName(_T(""))
	, m_machineType(-1)
	, m_sdkVersion(_T(""))
	, m_offset(0)
	, m_count(0)
{


	m_showFlag = false;


}

CMainDevice::~CMainDevice()
{
}

void CMainDevice::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ADD, m_add_btn);
	DDX_Control(pDX, IDC_DEL, m_del_btn);
	DDX_Control(pDX, IDC_MODIFY, m_modify_btn);
	//DDX_Control(pDX, IDC_DEVICELIST, m_DeviceList);
	DDX_Control(pDX, IDC_FIND, m_find_btn);
	DDX_Control(pDX, IDC_BACK, m_back_btn);
	DDX_Control(pDX, IDC_NEXT, m_next_btn);
	DDX_Control(pDX, IDC_GOTO, m_goto_btn);
	DDX_Control(pDX, IDC_PAGECOUNT, m_page_edit);

	DDX_Control(pDX, IDC_SHOWPAGE, m_showpage_static);
	DDX_Control(pDX, IDC_DEVICELIST, m_DeviceList);
	DDX_Control(pDX, IDC_FIRST, m_first_btn);
	DDX_Control(pDX, IDC_TAIL, m_tail_btn);
}


BEGIN_MESSAGE_MAP(CMainDevice, CDialog)
	ON_WM_SIZE()
//	ON_WM_CREATE()
//	ON_BN_CLICKED(IDC_BUTTON2, &CMainDevice::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_ADD, &CMainDevice::OnBnClickedAdd)
	ON_BN_CLICKED(IDC_FIND, &CMainDevice::OnBnClickedFind)
	ON_BN_CLICKED(IDC_DEL, &CMainDevice::OnBnClickedDel)
	ON_BN_CLICKED(IDC_MODIFY, &CMainDevice::OnBnClickedModify)
	ON_NOTIFY(NM_DBLCLK, IDC_DEVICELIST, &CMainDevice::OnNMDblclkList1)
	ON_BN_CLICKED(IDC_BACK, &CMainDevice::OnBnClickedBack)
	ON_BN_CLICKED(IDC_NEXT, &CMainDevice::OnBnClickedNext)
	ON_BN_CLICKED(IDC_GOTO, &CMainDevice::OnBnClickedGoto)
	ON_WM_PAINT()
	ON_NOTIFY(NM_RCLICK, IDC_DEVICELIST, &CMainDevice::OnNMRClickDevicelist)
	ON_COMMAND(ID_DEVICE_UPDATE, &CMainDevice::OnDeviceUpdate)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_FIRST, &CMainDevice::OnBnClickedFirst)
	ON_BN_CLICKED(IDC_TAIL, &CMainDevice::OnBnClickedTail)
	ON_COMMAND(ID_DEVICE_ADD, &CMainDevice::OnDeviceAdd)
	ON_COMMAND(ID_DEVICE_DEL, &CMainDevice::OnDeviceDel)
	ON_COMMAND(ID_DEVICE_MODIFY, &CMainDevice::OnDeviceModify)
	ON_COMMAND(ID_DEVICE_QUERY, &CMainDevice::OnDeviceQuery)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CMainDevice 消息处理程序

void CMainDevice::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	CRect rect;
	GetClientRect(&rect);
	
	if(m_DeviceList.GetSafeHwnd()==NULL)
		return;

	m_DeviceList.SetWindowPos(NULL, 0, 0, 
		rect.right - rect.left, rect.bottom - rect.top-50, NULL);


	m_DeviceList.SetColumnWidth( 0, 60 );
	int width = ( rect.right - rect.left-65)/5;
	for ( int i=1; i<6; i++ )
		m_DeviceList.SetColumnWidth( i, width );
	TRACE("RECT: %d, %d\n", rect.Width(), rect.Height());



	//////
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
	/////




	if(m_add_btn.GetSafeHwnd())
	{
		m_add_btn.MoveWindow(rect.left+80,rect.bottom-48+3/*statebar+*/,40,20);
	}

	if(m_del_btn.GetSafeHwnd())
	{
		m_del_btn.MoveWindow(rect.left+80+42,rect.bottom-48+3/*statebar+*/,40,20);
	}
	if(m_modify_btn.GetSafeHwnd())
	{
		m_modify_btn.MoveWindow(rect.left+80+42+42,rect.bottom-48+3/*statebar+*/,40,20);
	}

	if(m_find_btn.GetSafeHwnd())
	{
		m_find_btn.MoveWindow(rect.left+80+42+42+42,rect.bottom-48+3/*statebar+*/,60,20);
	}


	// TODO: 在此处添加消息处理程序代码
}

//int CMainDevice::OnCreate(LPCREATESTRUCT lpCreateStruct)
//{
//	if (CDialog::OnCreate(lpCreateStruct) == -1)
//		return -1;
//     m_DeviceList.Create(LVS_REPORT|LVS_SHOWSELALWAYS,
// 		CRect(0,0,0,0),this,IDC_DEVICELIST);
//
//	//m_DeviceList.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);
//
//
//
//	//m_DeviceList.ShowWindow(SW_SHOW);
//
//
// 	CFont m_font1,m_font2;
// 	m_font1.CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("微软雅黑"));
// 	m_font2.CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("微软雅黑"));
// 	m_DeviceList.SetFont(&m_font2);
// 	
// 	m_DeviceList.SetTextColor(RGB(38,59,90));
//
//	//m_DeviceList.GetHeaderCtrl()->SetFont(&m_font1);
//	//m_DeviceList.GetHeaderCtrl()->SetTextColor(RGB(38,59,90));
//
//
//
//	
//		return 0;
//}

void CMainDevice::OnBnClickedAdd()
{
	CDeviceAdd dlg;
	dlg.m_pParent = this;
    dlg.DoModal();
	//m_device = dlg.device;
}

//BOOL CMainDevice::PreTranslateMessage(MSG* pMsg)
//{
//	// TODO: 在此添加专用代码和/或调用基类
//
//		if (pMsg->message==WM_KEYDOWN&&pMsg->wParam==VK_RETURN)
//		{
//			CString str;
//			GetClassName(pMsg->hwnd,str.GetBuffer(MAX_PATH),MAX_PATH);
//			if ("Edit"==str)
//			{
//				pMsg->wParam = VK_TAB;
//			}
//		}
//	
//
//	return CDialog::PreTranslateMessage(pMsg);
//}

void CMainDevice::OnOK()
{
	int page = GetDlgItemInt(IDC_PAGECOUNT);
	int total = (m_count-1)/COUNT_PER_PAG+1;
	if (page>total||page<1)
	{
		MessageBox(_T("没有该页码!"));
	}
	else{
		m_offset = (page-1)*COUNT_PER_PAG;
		m_DeviceList.DeleteAllItems();
		ShowDeviceList();
	}

	ShowPage();
}



void CMainDevice::OnBnClickedFind()
{

	CDeviceFind dlg;
	if (IDOK == dlg.DoModal())
          m_DeviceList.DeleteAllItems();
}

/*
 * OnBnClickedDel
 *
 * @Describe 删除一个设备, 当删除一页中的一个设备时, 这页的设备列表是如何补全的
 * @Date 2013-7-6
 */
void CMainDevice::OnBnClickedDel()
{
	int nItem=m_DeviceList.GetNextItem(-1,LVNI_SELECTED);	
	CString temp_name = m_DeviceList.GetItemText(nItem,0);   	
	int ret = -1;
	if (-1==nItem)
	{
		MessageBox(_T("请选择要删除的设备"));
		return ;
	}
	if (MessageBox(_T("是否要删除设备"),_T("请确认"),MB_YESNO)==IDYES)
	{
		ret = gManage.DelDevice(temp_name);
		if (0==ret)
		{			
			int currentpage = m_offset/COUNT_PER_PAG+1;    // 根据m_offset获取当前的页码, m_offset的维护相当重要
			int itemcount = m_DeviceList.GetItemCount();
			m_DeviceList.DeleteItem(nItem);				   // 删除该条设备信息
			if (1==itemcount)							   //当只有一条时候 删除了退回前页；
			{
				CString strCurrent;
				strCurrent.Format(_T("%d"),currentpage-1); // 页码减1
				m_offset-=COUNT_PER_PAG;				   // m_offset是当页的第一设备, 可能是设备ID
				SetDlgItemText(IDC_PAGECOUNT,strCurrent);
				if (currentpage!=1)						   // 当前页只有一个设备时, 并只有一页, 删除这个设备后,不用管了
				{										   // 如果不只一个设备, 就去到strCurrent, 并获取设备列表
				     OnBnClickedGoto();
				}
			}
			else	
			{
				// 不只一条, 删除以后, 页码不变, m_offset也不变, 通过m_offset与count重新获取设备列表, 
				// 获取的还得以m_offset为起点的那一页设备
				CString strCurrent;
				strCurrent.Format(_T("%d"),currentpage);
				SetDlgItemText(IDC_PAGECOUNT,strCurrent);
				OnBnClickedGoto();
			}
			MessageBox(_T("删除设备成功！"));
		}
		else
			MessageBox(_T("删除设备失败！"));
	}
	// TODO: 在此添加控件通知处理程序代码
}

void CMainDevice::OnBnClickedModify()
{
    gManage.UnReg(gManage.m_devicelistID);
	
	int nItem=m_DeviceList.GetNextItem(-1,LVNI_SELECTED);	

	if (-1==nItem)
	{
		MessageBox(_T("请选择要修改的设备"));
		return ;
	}
	CString deviceid = m_DeviceList.GetItemText(nItem,0); 


	device_info deviceinfo;
	deviceinfo = gManage.GetDeviceInfo(deviceid);

    CDeviceAdd dlg;
	dlg.m_pParent = this ; 
	dlg.m_deviceinfo = deviceinfo;
	dlg.m_showtype = 2;
	dlg.DoModal();
	gManage.RegCallBackDeviceList(this);//重写注册设备列表回调  因为只提供一个》》。。  所以只能这样做了

}

void CMainDevice::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{

	int nItem=m_DeviceList.GetNextItem(-1,LVNI_SELECTED);	
	if (-1!=nItem)
	{
		OnBnClickedModify();
	}
	
	
}

BOOL CMainDevice::OnInitDialog()
{
	CDialog::OnInitDialog();
	m_DeviceList.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_TWOCLICKACTIVATE|LVS_EX_SUBITEMIMAGES);
	m_DeviceList.InsertColumn(0,_T("设备编号"),LVCFMT_LEFT,80,-1);
	m_DeviceList.InsertColumn(1,_T("PUID"),LVCFMT_LEFT,80,-1);
	m_DeviceList.InsertColumn(2,_T("设备厂家"),LVCFMT_LEFT,80,-1);
	m_DeviceList.InsertColumn(3,_T("设备类型"),LVCFMT_LEFT,80,-1);
	m_DeviceList.InsertColumn(4,_T("设备版本号"),LVCFMT_LEFT,80,-1);
	m_DeviceList.InsertColumn(5,_T("设备状态"),LVCFMT_LEFT,80,-1);

	CImageList   m_ImageList; 
	m_ImageList.Create(1,19,ILC_COLOR24,1,1); 
	m_DeviceList.SetImageList(&m_ImageList,LVSIL_SMALL); 

	gManage.m_pCommand->RegCmdCallback("BroadcastDeviceStatus",Call_back_deviceState,this);
	
	m_Timer = SetTimer(1, 10, NULL);   // 每10ms取一次设备状态

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

void CMainDevice::OnBnClickedBack()
{
	if (m_offset>0)
	{
		if (m_offset>=COUNT_PER_PAG)
			m_offset-=COUNT_PER_PAG;
		else
			m_offset = 0;
		m_DeviceList.DeleteAllItems();
        ShowDeviceList();
	}
    ShowPage();
}

void CMainDevice::OnBnClickedNext()
{
	if ((m_offset+COUNT_PER_PAG)<m_count)		// 该情况说明不是在最后一页
	{
		m_offset+=COUNT_PER_PAG;
		m_DeviceList.DeleteAllItems();
        ShowDeviceList();
	}
ShowPage();
}

void CMainDevice::OnBnClickedGoto()
{
	int page = GetDlgItemInt(IDC_PAGECOUNT);
	int total = (m_count-1)/COUNT_PER_PAG+1;
	if (page>total||page<1)
	{
	   MessageBox(_T("没有该页码!"));
	}
	else
	{
		m_offset = (page-1)*COUNT_PER_PAG;
		m_DeviceList.DeleteAllItems();
        ShowDeviceList();
	}

	ShowPage();

	// TODO: 在此添加控件通知处理程序代码
}

void CMainDevice::OnPaint()
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

	if (m_showFlag == FALSE)
	{
		m_DeviceList.DeleteAllItems();//清空。。。。
		ShowDeviceList();     // 第一次获取设备列表, 此时m_offset==0, 所以获取的是第一页
		
		m_showFlag = TRUE;
	}
	
	
	ShowPage();

}

void CMainDevice::OnNMRClickDevicelist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码

	DWORD dwPos = GetMessagePos();
	CPoint point( LOWORD(dwPos), HIWORD(dwPos) );

	CMenu menu;
	VERIFY( menu.LoadMenu(IDR_DEVICE));
	CMenu* popup = menu.GetSubMenu(0);

	ASSERT( popup != NULL );

	if (popup)
	{
		int nItem=m_DeviceList.GetNextItem(-1,LVNI_SELECTED);	
		CString temp_name = m_DeviceList.GetItemText(nItem,0);   	
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

		popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	}

	*pResult = 0;
}

void CMainDevice::OnDeviceUpdate()
{

    m_factoryName = _T("");
	m_machineType = -1;

	m_sdkVersion = _T("");


	m_DeviceList.DeleteAllItems();//清空。。。。
	m_offset = 0;
    ShowDeviceList();
	

}

extern int gCleanList_tag;
void CMainDevice::ShowDeviceList(void)
{

	gCleanList_tag = 0;
	for (int i=0;i<TIMES;i++)
	{
		gManage.GetDeviceList(this,m_factoryName,m_machineType,m_sdkVersion,i*5+m_offset);
	}
}

HBRUSH CMainDevice::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

void CMainDevice::OnBnClickedFirst()
{
	m_factoryName = _T("");
	m_machineType = -1;

	m_sdkVersion = _T("");


	m_DeviceList.DeleteAllItems();//清空。。。。
	m_offset = 0;
	ShowDeviceList();

}

void CMainDevice::OnBnClickedTail()
{
	int totaltcount = (m_count-1)/COUNT_PER_PAG+1;
	m_offset = (totaltcount-1)*COUNT_PER_PAG;
	m_DeviceList.DeleteAllItems();
	ShowDeviceList();

	ShowPage();
}

void CMainDevice::ShowPage(void)
{
	CString str;
	int totaltcount = (m_count-1)/COUNT_PER_PAG+1;
	int currentpage = m_offset/COUNT_PER_PAG+1;
	str.Format(_T("%d / %d"),currentpage,totaltcount);
	SetDlgItemText(IDC_SHOWPAGE,str);
}

void  CMainDevice::SetDevceState(int  index,CString StateId,CString errCode)
{
	if (StateId == "0")
	{
		if (errCode == ":0")
			m_DeviceList.SetItemText(index, 5, _T("未连接设备"));
		else
			m_DeviceList.SetItemText(index, 5, _T("连接设备失败") + errCode);
	}
	else if (StateId == "1")
	{
		if (errCode = ":0")
			m_DeviceList.SetItemText(index, 5, _T("正在连接设备"));
		else
			m_DeviceList.SetItemText(index, 5, _T("正在登出设备"));
	}
	else if (StateId == "2")
	{
		if (errCode == ":0")
			m_DeviceList.SetItemText(index, 5, _T("已连接设备"));
		else
		{
			CString csRet = m_DeviceList.GetItemText(index, 5);
			if (csRet == "正在注册CMS")
				m_DeviceList.SetItemText(index, 5, _T("注册CMS失败") + errCode);
			else
				m_DeviceList.SetItemText(index, 5, _T("连接CMS失败") + errCode);	
		}
	}
	else if (StateId == "3")
	{
		m_DeviceList.SetItemText(index, 5, _T("正在注册CMS"));
	}
	else if (StateId == "4")
	{
		m_DeviceList.SetItemText(index, 5, _T("注册CMS成功"));
	}
}

BOOL WINAPI CMainDevice::Call_back_deviceState(IJDCUCommand* pJDCUCommand,DWORD dwMsgID, IJXmlDocument* pXmlDoc,void* pParam)
{
//return TRUE;
	CMainDevice* pMainDevice = (CMainDevice*)pParam;

	if (pMainDevice->m_showFlag == false)
	{
		return TRUE;
	}

	IJXmlElement* pEleMsg = pXmlDoc->FirstChild(XMLMSG);
	IJXmlElement* pDeviceEle= NULL;
	IJXmlElement* pEle= NULL;
	pDeviceEle = pEleMsg->FirstChild("device");
	CString deviceId,status,errCode;
	if (pDeviceEle)
	{
		deviceId =  pDeviceEle->GetAttribute("id");
		status = pDeviceEle->FirstChild("status")->GetText();
		errCode = pDeviceEle->FirstChild("errCode")->GetText();
	}

	DeviceState *state = new DeviceState(deviceId, status, errCode);
	pMainDevice->m_DeviceStateList.m_lock.Lock();
	pMainDevice->m_DeviceStateList.m_list.AddTail(state);
	pMainDevice->m_DeviceStateList.m_lock.Unlock();
/*
	int iCount = pMainDevice->m_DeviceList.GetItemCount();

	CString strId;
	for (int i = 0;i<iCount;i++)
	{
		strId = pMainDevice->m_DeviceList.GetItemText(i,0);
		if (strId == deviceId)
		{
			pMainDevice->SetDevceState(i,status,_T(":")+errCode);
		}		
	}
*/
	
	return TRUE;
}

void CMainDevice::OnDeviceAdd()
{
	OnBnClickedAdd();
}

void CMainDevice::OnDeviceDel()
{
	OnBnClickedDel();
}

void CMainDevice::OnDeviceModify()
{
	OnBnClickedModify();
}

void CMainDevice::OnDeviceQuery()
{
	OnBnClickedFind();
}

BOOL CMainDevice::PreTranslateMessage(MSG* pMsg)
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

int CMainDevice::GetLimitDeviceId(CString &startDeviveId, CString &endDeviceId)
{
	int nItemCount;
	nItemCount = m_DeviceList.GetItemCount();
	startDeviveId = m_DeviceList.GetItemText(0, 0);
	endDeviceId = m_DeviceList.GetItemText(nItemCount - 1, 0);
	return 0;
}
void CMainDevice::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)
	{
		m_DeviceStateList.m_lock.Lock();
		for (int i = 0; i < m_DeviceStateList.m_list.GetCount(); i++)
		{
			
			DeviceState *state = m_DeviceStateList.m_list.GetHead();
			CString deviceId = state->m_id;
			CString status = state->m_status;
			CString errCode = state->m_errCode;

			int iCount = m_DeviceList.GetItemCount();

			CString strId;
			for (int i = 0;i<iCount;i++)
			{
				strId = m_DeviceList.GetItemText(i,0);
				if (strId == deviceId)
				{
					SetDevceState(i,status,_T(":")+errCode);
				}		
			}
			m_DeviceStateList.m_list.RemoveHead();
		}
		m_DeviceStateList.m_lock.Unlock();
	}

	CDialog::OnTimer(nIDEvent);
}
