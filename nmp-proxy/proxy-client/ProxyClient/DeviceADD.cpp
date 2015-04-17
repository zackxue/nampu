// DeviceAdd.cpp : 实现文件
//

#include "stdafx.h"
#include "ProxyClient3.h"
#include "DeviceAdd.h"

#define DEFAULT_PF_PORT "9902"				/* 平台默认端口 */

// CDeviceAdd 对话框
IMPLEMENT_DYNAMIC(CDeviceAdd, CDialog)
extern CManage gManage;

CDeviceAdd::CDeviceAdd(UINT IDD1,CWnd* pParent /*=NULL*/)
: CDialog(IDD1, pParent)
, m_result(0)
, m_showtype(0)
, m_pParent(NULL)
, m_deviceport_edit(0)
, m_plport_edit(0)
{
// 	m_device_port= _T("");	//设备端口
// 	m_platform_port= _T("");	//平台端口
// 	m_device_id =  _T("");		//设备ID  无
// 
// 	m_sdk_version = _T("");	//软件版本
// 	m_device_ip = _T("");		//设备IP 地址
// 	m_pu_id = _T("");			//PUID
// 	m_platform_ip = _T("");	//平台IP 地址
// 	m_username = _T("");		//用户名
// 	m_password = _T("");	//密码
// 	m_type = _T("");//设备类型
//     m_factory(_T(""))

}

CDeviceAdd::~CDeviceAdd()
{
}

void CDeviceAdd::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FACTORY, m_factory_combox);
	DDX_Control(pDX, IDC_DEVICETYPE, m_devicetype_combox);
	DDX_Control(pDX, IDC_VERSION, m_version_combox);
	DDX_Control(pDX, IDC_CLOSE, m_close_btn);
	DDX_Control(pDX, IDCANCEL, m_cancel_btn);
	DDX_Control(pDX, IDOK, m_ok_btn);
	DDX_Text(pDX, IDC_DEVICEPORT, m_deviceport_edit);
	//DDV_MinMaxInt(pDX, m_deviceport_edit, 0, 65535);
	DDX_Text(pDX, IDC_PLPORT, m_plport_edit);
	//DDV_MinMaxInt(pDX, m_plport_edit, 0, 65535);
}


BEGIN_MESSAGE_MAP(CDeviceAdd, CDialog)
//	ON_EN_KILLFOCUS(IDC_DEVICEPORT, &CDeviceAdd::OnEnKillfocusDeviceport)
	ON_EN_KILLFOCUS(IDC_PLPORT, &CDeviceAdd::OnEnKillfocusPlport)
	ON_CBN_SELCHANGE(IDC_FACTORY, &CDeviceAdd::OnCbnSelchangeFactory)
	ON_CBN_SELCHANGE(IDC_DEVICETYPE, &CDeviceAdd::OnCbnSelchangeDevicetype)
	ON_WM_MOUSEMOVE()
	ON_WM_NCACTIVATE()
	ON_WM_NCLBUTTONDBLCLK()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_NCLBUTTONUP()
	ON_WM_NCMOUSEMOVE()
	ON_WM_NCPAINT()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_NCHITTEST()
	ON_BN_CLICKED(IDC_CLOSE, &CDeviceAdd::OnBnClickedClose)
	ON_EN_KILLFOCUS(IDC_DEVICEPORT, &CDeviceAdd::OnEnKillfocusDeviceport)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CDeviceAdd 消息处理程序

void CDeviceAdd::OnOK()
{
	BOOL bRet;
	//gManage.UnReg(gManage.m_devicelistID);
	bRet = UpdateData();
	if (bRet != TRUE)
	{
		return;
	}
	device_info device;
	CString temptype;
	GetDlgItemText(IDC_PLIP,device.platform_ip);
	GetDlgItemText(IDC_PLID,device.pu_id);
	GetDlgItemText(IDC_FACTORY,device.factoryname);
	GetDlgItemText(IDC_DEVICETYPE,temptype);
	GetDlgItemText(IDC_VERSION,device.sdk_version);
	GetDlgItemText(IDC_USERNAME,device.username);
	GetDlgItemText(IDC_PWD,device.password);
	GetDlgItemText(IDC_DEVICEIP,device.device_ip);
	GetDlgItemText(IDC_DEVICEPORT,device.device_port);
	GetDlgItemText(IDC_PLPORT,device.platform_port);
	device.type = gManage.GetDeviceNum(temptype);

	if(device.username.IsEmpty()){
		MessageBox(_T("请输入用户名！"), NULL, MB_OK);
		return ;
	}
	if(device.factoryname.IsEmpty()){
		MessageBox(_T("请选择设备厂家！"), NULL, MB_OK);
		return ;
	}

	if(temptype.IsEmpty()){
		MessageBox(_T("请选择设备类型！"), NULL, MB_OK);
		return ;
	}
	if(device.password.IsEmpty() 
		&& device.factoryname != "XiongMaiTech"){
		MessageBox(_T("请输入用户名密码！"), NULL, MB_OK);
		return ;
	}
	if(device.device_ip.IsEmpty()){
		MessageBox(_T("请输入设备IP！"), NULL, MB_OK);
		return ;
	}

	if(device.sdk_version.IsEmpty()){
		MessageBox(_T("请输入设备版本号"), NULL, MB_OK);
		return ;
	}

	if(device.device_port.IsEmpty())
	{
		MessageBox(_T("请输入设备端口号"), NULL, MB_OK);
		return ;
	}
	else
	{
		if (m_deviceport_edit>65535||m_deviceport_edit<0)
		{
			MessageBoxW(_T("设备端口号超过范围!"));
			CEdit* pEdt;
			pEdt=(CEdit*)GetDlgItem(IDC_DEVICEPORT);
			pEdt->SetFocus();
			return;
		}
	}

	if(device.platform_port.IsEmpty())
	{
		MessageBox(_T("请输入平台端口号"), NULL, MB_OK);
		return ;
	}
	else
	{
		if (m_plport_edit>65535||m_plport_edit<0)
		{

			MessageBoxW(_T("平台端口号超过范围!"));
			CEdit* pEdt;
			pEdt=(CEdit*)GetDlgItem(IDC_PLPORT);
			pEdt->SetFocus();
			return;
		}
	}
	if(device.pu_id.IsEmpty()){
		MessageBox(_T("请输入puID"), NULL, MB_OK);
		return ;
	}

	if(device.platform_ip.IsEmpty()){
		MessageBox(_T("请输入平台IP"), NULL, MB_OK);
		return ;
	}
	device.device_id = m_deviceinfo.device_id;
	if (m_showtype ==0)
	{
		int ret = gManage.AddDevice(device);
		if (-1 == ret)
		{
			MessageBox(_T("添加设备失败"), NULL, MB_OK);
		}
		else
		{

			 CDialog::OnOK();//关闭修改对话框
			 CString str;
			 str.Format(_T("%d"),ret);

			 int n = m_pParent->m_DeviceList.GetItemCount();
				 //device_info deviceinfo;
				// deviceinfo = gManage.GetDeviceInfo(str);
			 if (n<COUNT_PER_PAG)
			 {
				 int j =  m_pParent->m_DeviceList.InsertItem(n,str);
				 m_pParent->m_DeviceList.SetItemText(j,1,device.pu_id);
				 m_pParent->m_DeviceList.SetItemText(j,2,device.factoryname);
				 m_pParent->m_DeviceList.SetItemText(j,3,temptype);
				 m_pParent->m_DeviceList.SetItemText(j,4,device.sdk_version);
				 m_pParent->m_DeviceList.SetItemText(j,5,_T("未连接设备"));
				
				 // 当在最后页时, 发送限制上报状态的消息
				 CString startDeviceId, endDeivceId;
				 //startDeviceId.Format(_T("%d"), m_pParent->m_offset);
				 //endDeivceId.Format(_T("%d"), m_pParent->m_offset + COUNT_PER_PAG);
				 // 获取当页最大的设备ID, 其实就是最底部的ID
				 
				 m_pParent->GetLimitDeviceId(startDeviceId, endDeivceId);
				 gManage.LimitBroadcastDeviceStatus(startDeviceId, endDeivceId);  // 注意: 这是第二个地方, 增加这条消息, 第一条在Usermanage.cpp中的GetDeviceList中
			 }
			 else // n==COUNT_PER_PAG的情况
			 {
				 
				 CString page;
				 m_pParent->m_count++;
				 int totalpage = ( m_pParent->m_count-1)/COUNT_PER_PAG+1;
				
				 //totalpage++;
				 page.Format(_T("%d"),totalpage);
				 m_pParent->SetDlgItemText(IDC_PAGECOUNT,page);
				 m_pParent->OnBnClickedGoto();	 
			 }


			 MessageBox(_T("添加设备成功！"), NULL, MB_OK);
			
			// gManage.GetDeviceList(this,_T(""),-1,)
			
		}

	}
	else if (2 == m_showtype)
	{
		if (m_deviceinfo.device_port==device.device_port&&
			m_deviceinfo.platform_port==device.platform_port&&
			m_deviceinfo.device_id==device.device_id&&
			//m_deviceinfo.protocol==device.protocol&&  
			m_deviceinfo.type==device.type&&
			m_deviceinfo.pu_id==device.pu_id&&
			m_deviceinfo.factoryname==device.factoryname&&
			m_deviceinfo.sdk_version==device.sdk_version&&
			m_deviceinfo.device_ip==device.device_ip&&
			m_deviceinfo.platform_ip==device.platform_ip&&
			m_deviceinfo.username==device.username&&
			m_deviceinfo.password==device.password
			)
		{
			CDialog::OnOK();	
			return ;
		}
		if (MessageBox(_T("是否要修改设备信息"),_T("请确认"),MB_YESNO)==IDYES)
		{

			int ret = gManage.DeviceModify(device);
		    if (-1 == ret)
		    {
				MessageBox(_T("修改设备失败"), NULL, MB_OK);
		    }
			else
			{
				//添加
				CDialog::OnOK();		

				int n = m_pParent->m_DeviceList.GetItemCount();
				
                CString temp_deviceid;
				int nItem = m_pParent->m_DeviceList.GetNextItem(-1,LVNI_SELECTED);	
				temp_deviceid = m_pParent->m_DeviceList.GetItemText(nItem,0);
				if (-1!=nItem)
				{

					//m_pParent->m_DeviceList.DeleteItem(nItem);
					//int j;
					//j =m_pParent->m_DeviceList.InsertItem(nItem,temp_deviceid);
					//CString states = 	m_pParent->m_DeviceList.GetItemText(nItem,5);
					m_pParent->m_DeviceList.SetItemText(nItem,0,temp_deviceid);
					m_pParent->m_DeviceList.SetItemText(nItem,1,device.pu_id);
					m_pParent->m_DeviceList.SetItemText(nItem,2,device.factoryname);
					m_pParent->m_DeviceList.SetItemText(nItem,3,gManage.GetDeviceType(device.type));
					m_pParent->m_DeviceList.SetItemText(nItem,4,device.sdk_version);
					//m_pParent->m_DeviceList.SetItemText(nItem,5,states);
					

				}
			


				MessageBox(_T("修改设备成功"), NULL, MB_OK);		
			}
		}

		
	}
	//gManage.RegCallBackDeviceList(m_pParent);
	  
}

BOOL CDeviceAdd::OnInitDialog()
{


	CEdit*pEdt=(CEdit*)GetDlgItem(IDC_USERNAME);
	pEdt->SetLimitText(32);

	pEdt=(CEdit*)GetDlgItem(IDC_DEVICEPORT);
	pEdt->SetLimitText(5);

	pEdt=(CEdit*)GetDlgItem(IDC_PWD);
	pEdt->SetLimitText(32);

	pEdt=(CEdit*)GetDlgItem(IDC_DEVICEIP);
	pEdt->SetLimitText(32);


	pEdt=(CEdit*)GetDlgItem(IDC_PLID);
	pEdt->SetLimitText(16);

	pEdt=(CEdit*)GetDlgItem(IDC_PLIP);
	pEdt->SetLimitText(32);

	pEdt=(CEdit*)GetDlgItem(IDC_PLPORT);
	pEdt->SetLimitText(5);

	CDialog::OnInitDialog();
	factory *rLinkfactory = NULL;//厂家链表指针
	device *pLinkdevice = NULL;
	device *rLinkdevice = NULL;//设备链表指针
	sdkversion *pLinkversion = NULL;
	sdkversion *rLinkversion = NULL;//版本链表指针 
	// TODO:  在此添加额外的初始化
	if (1==m_showtype)
	{
		SetWindowText(_T("设备详细信息"));
		GetDlgItem(IDOK)->ShowWindow(SW_HIDE);
		GetDlgItem(IDCANCEL)->ShowWindow(SW_HIDE);

		CComboBox *pCombox;
		pCombox  = (CComboBox *)GetDlgItem(IDC_FACTORY);
		pCombox->AddString(m_deviceinfo.factoryname);
		pCombox->SetCurSel(0);


		pCombox  = (CComboBox *)GetDlgItem(IDC_DEVICETYPE);//类型 

		pCombox->AddString(gManage.GetDeviceType(m_deviceinfo.type));

		pCombox->SetCurSel(0);

		pCombox  = (CComboBox *)GetDlgItem(IDC_VERSION);
		pCombox->AddString(m_deviceinfo.sdk_version);
		pCombox->SetCurSel(0);

		SetDlgItemText(IDC_DEVICEPORT,m_deviceinfo.device_port);

		SetDlgItemText(IDC_USERNAME,m_deviceinfo.username);
		SetDlgItemText(IDC_PWD,m_deviceinfo.password);
		SetDlgItemText(IDC_DEVICEIP,m_deviceinfo.device_ip);
		SetDlgItemText(IDC_PLID,m_deviceinfo.pu_id);
		SetDlgItemText(IDC_PLIP,m_deviceinfo.platform_ip);
		SetDlgItemText(IDC_PLPORT,m_deviceinfo.platform_port);
	}
	else if(2==m_showtype)//修改设备信息
	{
		factory *rLinkfactory = NULL;
		device *pLinkdevice= NULL;
		device *rLinkdevice= NULL;
		sdkversion *pLinkversion= NULL;
		if (gManage.m_pFactory)
			rLinkfactory = gManage.m_pFactory;
		while(rLinkfactory){
			m_factory_combox.AddString(rLinkfactory->name);
			rLinkfactory = rLinkfactory->next;
		}   
		factory *p = gManage.m_pFactory;
		while (p&&(p->name!=m_deviceinfo.factoryname))
		{
			p=p->next;
		}
		device *pDvice=NULL;
		if (p)
			pDvice = p->pDevice;
		while(pDvice){
			m_devicetype_combox.AddString(gManage.GetDeviceType(pDvice->name));
			pDvice = pDvice->next;
		}
		if (p)
		{
			pDvice = p->pDevice;
			while (pDvice&&(pDvice->name!=m_deviceinfo.type))//
			{
				pDvice=pDvice->next;
			}

			if (pDvice)
			{
				sdkversion *pSdkversion;
				pSdkversion = pDvice->pVsersion;
				while (pSdkversion)
				{
					m_version_combox.AddString(pSdkversion->name);
					pSdkversion = pSdkversion->next;
				}
			}
		}
		CString temp;
		int n ; 

		n =  m_factory_combox.GetCount();
		for (int i = 0;i<n;i++)
		{
			m_factory_combox.GetLBText(i,temp);
			if (temp == m_deviceinfo.factoryname)
			{
				m_factory_combox.SetCurSel(i);
				break;

			}
		}
		n =  m_devicetype_combox.GetCount();
		for (int i = 0;i<n;i++)
		{
			m_devicetype_combox.GetLBText(i,temp);
			if (temp == gManage.GetDeviceType(m_deviceinfo.type))
			{
				m_devicetype_combox.SetCurSel(i);
				break;

			}
		}
		n =  m_version_combox.GetCount();
		for (int i = 0;i<n;i++)
		{
			m_version_combox.GetLBText(i,temp);
			if (temp == m_deviceinfo.sdk_version)
			{
				m_version_combox.SetCurSel(i);
				break;

			}
		}
		SetWindowText(_T("修改设备信息"));
		GetDlgItem(IDOK)->SetWindowText(_T("保存"));
		//SetWindowText(IDOK,_T("保存"));
		SetDlgItemText(IDC_FACTORY,m_deviceinfo.factoryname);
		((CComboBox*)GetDlgItem(IDC_FACTORY))->EnableWindow(FALSE);
		

		SetDlgItemText(IDC_DEVICETYPE,m_deviceinfo.type);
		((CComboBox*)GetDlgItem(IDC_DEVICETYPE))->EnableWindow(FALSE);


		SetDlgItemText(IDC_VERSION,m_deviceinfo.sdk_version);
		((CComboBox*)GetDlgItem(IDC_VERSION))->EnableWindow(FALSE);


		SetDlgItemText(IDC_DEVICEPORT,m_deviceinfo.device_port);
		SetDlgItemText(IDC_USERNAME,m_deviceinfo.username);
		SetDlgItemText(IDC_PWD,m_deviceinfo.password);
		SetDlgItemText(IDC_DEVICEIP,m_deviceinfo.device_ip);


		
		SetDlgItemText(IDC_PLID,m_deviceinfo.pu_id);
		((CComboBox*)GetDlgItem(IDC_PLID))->EnableWindow(FALSE);
		SetDlgItemText(IDC_PLIP,m_deviceinfo.platform_ip);
		SetDlgItemText(IDC_PLPORT,m_deviceinfo.platform_port);




	}
	else{//添加设备
		SetWindowText(_T("添加设备"));
		rLinkfactory = gManage.m_pFactory;

		while(rLinkfactory)
		{
			m_factory_combox.AddString(rLinkfactory->name);//向下拉列表框填相关内容
			rLinkfactory = rLinkfactory->next;
		}

		if (gManage.m_pFactory)
			pLinkdevice = gManage.m_pFactory->pDevice;
		rLinkdevice = pLinkdevice; 
// 写死设备类型 dvr, nvr, ipc -- 2013-5-3
#if 0 
		while(pLinkdevice)
		{
			m_devicetype_combox.AddString(gManage.GetDeviceType(pLinkdevice->name));
			pLinkdevice = pLinkdevice->next;
		}
#else
		m_devicetype_combox.AddString(_T("DVR"));
		m_devicetype_combox.AddString(_T("NVR"));
		m_devicetype_combox.AddString(_T("IPC"));
#endif

		if(rLinkdevice)
			pLinkversion = rLinkdevice->pVsersion; 

		while(pLinkversion)
		{

			m_version_combox.AddString(pLinkversion->name);
			pLinkversion = pLinkversion->next;

		}

		m_factory_combox.SetCurSel(0);
		m_devicetype_combox.SetCurSel(0);
		m_version_combox.SetCurSel(0);


		CString temp_factory;
		CString dev_type;
		CString puidString;
		GetDlgItemText(IDC_FACTORY,temp_factory);
		GetDlgItemText(IDC_DEVICETYPE,dev_type);


		if (!temp_factory.IsEmpty())
		{
#if 0
			if ("hikvision" == temp_factory )
				puidString =_T("HIK-");
			else
				puidString =_T("DAH-");
#else
			if ("Hikvision" == temp_factory )
			{
				puidString =_T("HIK-");
			}
			else if ("Dahua" == temp_factory)
			{
				puidString =_T("DAH-");
			}
			else if ("Hbgk" == temp_factory)
			{
				puidString =_T("HBN-");
			}
			else if ("Bosiming" == temp_factory)
			{
				puidString =_T("BSM-");
			}
			else if ("Sunell" == temp_factory)
			{
				puidString =_T("JNY-");
			}
			else if ("XiongMai" == temp_factory)
			{
				puidString =_T("XMT-");
			}
			else if ("TopSee" == temp_factory)
			{
				puidString = _T("TPS-");
			}
			else
			{
				puidString =_T("-");
			}
#endif
		}
		if (!dev_type.IsEmpty())
		{
			puidString+= dev_type;
			puidString+="-";
		}

		SetDlgItemText(IDC_PLID,puidString);  // 设置PUID前端部分;
		SetDlgItemText(IDC_PLPORT, _T(DEFAULT_PF_PORT));

		//UpdateData(FALSE);
	}
	m_close_btn.SetImage(_T("images/cu_title_close.bmp"),_T("images/cu_sel_title_close.bmp"),
		_T("images/cu_sel_title_close.bmp"),_T("images/cu_sel_title_close.bmp"));

	SetBackBitmap();



	m_ok_btn.SetImage(_T("images/normal_black_btn.bmp"),_T("images/sel_black_btn.bmp"),
		_T("images/sel_black_btn.bmp"),_T("images/sel_black_btn.bmp"));

	m_cancel_btn.SetImage(_T("images/normal_black_btn.bmp"),_T("images/sel_black_btn.bmp"),
		_T("images/sel_black_btn.bmp"),_T("images/sel_black_btn.bmp"));

	m_ok_btn.SetTextPos(CPoint(10,4));
	m_cancel_btn.SetTextPos(CPoint(10,4));
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}

//void CDeviceAdd::OnEnKillfocusDeviceport()
//{
//	UpdateData();
//	
//	if (m_deviceport_edit>65535||m_deviceport_edit<0)
//	{
//		MessageBoxW(_T("设备端口号超过范围!"));
//	}
//
//}

/*
J_SDK_DVR=0,
J_SDK_DVS,
J_SDK_IPC,
J_SDK_IPNC,
J_SDK_SDEC,                         //软解码器
J_SDK_DEC,                          //硬解码器
J_SDK_NVR,
J_SDK_HVR,
*/

void CDeviceAdd::OnCbnSelchangeFactory()
{ 
	if (m_showtype ==0)					//添加时候防止多次选择时候
		SetDlgItemText(IDC_PLID,_T(""));
	CString temp_factory;
	GetDlgItemText(IDC_FACTORY,temp_factory);
	factory *p = gManage.m_pFactory;
	while (p&&p->name!=temp_factory)
	{
		p=p->next;
	}
	device *pDvice = NULL;
	if (p)
	{
		pDvice = p->pDevice;
	}
	// 厂商改变, 删除所有的设备类型
	int mm = m_devicetype_combox.GetCount();
	for (int i=0;i < mm;i++)
	{
		m_devicetype_combox.DeleteString( 0 );
	}
// 写死设备类型 dvr, nvr, ipc -- 2013-5-3
#if 0
	while(pDvice)
	{
		m_devicetype_combox.AddString(gManage.GetDeviceType(pDvice->name));
		pDvice = pDvice->next;
	}
#else
	//while (pDvice)
	{
		m_devicetype_combox.AddString(_T("DVR"));
		m_devicetype_combox.AddString(_T("NVR"));
		m_devicetype_combox.AddString(_T("IPC"));
	}
#endif
	CString tempstr,rightString,puidString;

	GetDlgItemText(IDC_PLID,tempstr);
	rightString =	tempstr.Right(tempstr.GetLength() - tempstr.Find('-')-1);
	if ("Hikvision" == temp_factory )
	{
		puidString =_T("HIK-")+rightString;
	}
	else if ("Dahua" == temp_factory)
	{
		puidString =_T("DAH-")+rightString;
	}
	else if ("Hbgk" == temp_factory)
	{
		puidString =_T("HBN-")+rightString;
	}
	else if ("Bosiming" == temp_factory)
	{
		puidString =_T("BSM-")+rightString;
	}
	else if ("Sunell" == temp_factory)
	{
		puidString =_T("JNY-")+rightString;
	}
	else if ("XiongMai" == temp_factory)
	{
		puidString = _T("XMT-")+rightString;
	}
	else if ("TopSee" == temp_factory)
	{
		puidString = _T("TPS-")+rightString;
	}
	else
	{
		puidString =_T("-")+rightString;
	}

	SetDlgItemText(IDC_PLID,puidString);

	// TODO: 在此添加控件通知处理程序代码
}

void CDeviceAdd::OnCbnSelchangeDevicetype()
{
	CString devicename ;
	CString factoryname ,tempstr;
	GetDlgItemText(IDC_DEVICETYPE,devicename);
	GetDlgItemText(IDC_FACTORY,factoryname);

	if (m_showtype == 0)
	{
		GetDlgItemText(IDC_PLID,tempstr);
		tempstr = tempstr.Left(4);
		tempstr+=devicename;
		tempstr+=_T("-");
		SetDlgItemText(IDC_PLID,tempstr);//设置平台ID头
	}
	else{
//HIK-DVR-20000000
		CString leftString,rigthString,tempString;
		GetDlgItemText(IDC_PLID,tempString);
		leftString = tempString.Left(tempString.Find('-')+1);
		rigthString = tempString.Right(tempString.GetLength() - tempString.Find('-')-4);
		tempString = leftString+devicename+rigthString;
		SetDlgItemText(IDC_PLID,tempString);
	}
	factory *p = gManage.m_pFactory;
	while(p&&p->name!=factoryname){//设置型号列表
		p = p->next;
	}
	if (p)
	{
		device *pdevice = p->pDevice;
		while (pdevice&&(gManage.GetDeviceType(pdevice->name)!=devicename))
		{
			pdevice=pdevice->next;
		}
		if (pdevice)
		{
			sdkversion *pSdkversion;
			pSdkversion = pdevice->pVsersion;
			int mm = m_version_combox.GetCount();
			for (int i=0;i < mm;i++)    //清空列表框原来数据
			{
				m_version_combox.DeleteString( 0 );  
			}
			while (pSdkversion)
			{
				m_version_combox.AddString(pSdkversion->name);
				pSdkversion = pSdkversion->next;
			}
		}
	}

	




	// TODO: 在此添加控件通知处理程序代码
}

void CDeviceAdd::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialog::OnMouseMove(nFlags, point);
}

BOOL CDeviceAdd::OnNcActivate(BOOL bActive)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	return CDialog::OnNcActivate(bActive);
}

void CDeviceAdd::OnNcLButtonDblClk(UINT nHitTest, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialog::OnNcLButtonDblClk(nHitTest, point);
}

void CDeviceAdd::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialog::OnNcLButtonDown(nHitTest, point);
}

void CDeviceAdd::OnNcLButtonUp(UINT nHitTest, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialog::OnNcLButtonUp(nHitTest, point);
}

void CDeviceAdd::OnNcMouseMove(UINT nHitTest, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialog::OnNcMouseMove(nHitTest, point);
}

void CDeviceAdd::OnNcPaint()
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

	  ChangeWindowRgn(&dc);


	  dc.SetTextColor(RGB(255,255,240));
	  dc.SetBkMode(TRANSPARENT);
	  if(0 == m_showtype)
		  dc.TextOut(10,5,_T("设备添加"));
	  else if (2==m_showtype)
		  dc.TextOut(10,5,_T("设备修改"));
	  else if (1==m_showtype)		
		  dc.TextOut(10,5,_T("设备详细信息"));
		


		

		
}

void CDeviceAdd::OnPaint()
{
	//CPaintDC dc(this); // device context for painting

// 	CDialog::OnPaint();	
// 	CRect rcClient;
// 	GetClientRect(&rcClient);
// 
// 	CRect rcTop = rcClient;//头背景
// 	rcTop.bottom = 31;

	
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

	ChangeWindowRgn(&dc);


	dc.SetTextColor(RGB(255,255,240));
	dc.SetBkMode(TRANSPARENT);
	if(0 == m_showtype)
		dc.TextOut(10,5,_T("设备添加"));
	else if (2==m_showtype)
		dc.TextOut(10,5,_T("设备修改"));
	else if (1==m_showtype)		
		dc.TextOut(10,5,_T("设备详细信息"));

	
	//DrawRangeImage(0,&dc,rcTop);
}

void CDeviceAdd::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
}

BOOL CDeviceAdd::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类


	if (pMsg->message==WM_KEYDOWN&&pMsg->wParam==VK_RETURN)
	{
		CString str;
		GetClassName(pMsg->hwnd,str.GetBuffer(MAX_PATH),MAX_PATH);
		if ("Edit"==str)
		{
			pMsg->wParam = VK_TAB;
		}
	}



	return CDialog::PreTranslateMessage(pMsg);
}

void CDeviceAdd::SetBackBitmap(void)
{
	
	HBITMAP hBitmap1 = NULL;
	HBITMAP hBitmap2 = NULL;
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

void CDeviceAdd::DrawRangeImage(int i, CDC *pDC, CRect rc){
	
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

LRESULT CDeviceAdd::OnNcHitTest(CPoint point)
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

void CDeviceAdd::ChangeWindowRgn(CDC* pDC)
{
	COLORREF col = RGB(255,0,255);
	CRect rcClient;
	GetClientRect (rcClient);
	CRgn rgn;
	rgn.CreateRoundRectRgn (1, 1, rcClient.Width(), rcClient.Height(),6,6);

	SetWindowRgn (rgn, TRUE);
}

void CDeviceAdd::OnBnClickedClose()
{

	CString platFormPort,devicePort;


	GetDlgItemText(IDC_DEVICEPORT,devicePort);
	GetDlgItemText(IDC_PLPORT,platFormPort);

	if(devicePort.IsEmpty())
	{
		SetDlgItemText(IDC_DEVICEPORT,_T("0"));
		
	}
	if(platFormPort.IsEmpty())
	{
		SetDlgItemText(IDC_PLPORT,_T("0"));
		
	}
	EndDialog(2);
	//OnCancel();
	// TODO: 在此添加控件通知处理程序代码
}

void CDeviceAdd::OnEnKillfocusDeviceport()
{
	UpdateData();

	if (m_deviceport_edit>65535||m_deviceport_edit<0)
	{
		MessageBoxW(_T("设备端口号超过范围!"));
		CEdit* pEdt;
		pEdt=(CEdit*)GetDlgItem(IDC_DEVICEPORT);
		pEdt->SetFocus();
		
	}

	// TODO: 在此添加控件通知处理程序代码
}
void CDeviceAdd::OnEnKillfocusPlport()
{
	return;

	UpdateData();

	if (m_plport_edit>65535||m_plport_edit<0)
	{

		MessageBoxW(_T("平台端口号超过范围!"));
		CEdit* pEdt;
		pEdt=(CEdit*)GetDlgItem(IDC_PLPORT);
		pEdt->SetFocus();
	}

}

HBRUSH CDeviceAdd::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (pWnd->m_hWnd == GetDlgItem(IDC_PLID)->m_hWnd)
	{
		return hbr;
	}

	if(nCtlColor==CTLCOLOR_STATIC)
	{  
		pDC->SetBkMode(TRANSPARENT);  
		//pDC->SetTextColor(RGB(68,129,2));
		return  (HBRUSH)GetStockObject(NULL_BRUSH); 

	}
	return hbr;
}
