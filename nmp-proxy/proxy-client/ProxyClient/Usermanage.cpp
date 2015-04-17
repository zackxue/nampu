#include "StdAfx.h"
#include "Usermanage.h"
#include "ProxyClient3.h"
#include "./include/IJDCUCommand.h"
#include "Login.h"
#include "DeviceADD.h"
#include "ChildView.h"
#include "MainFrm.h"

#include "log.h"
#include "ping.h"

extern CProxyClient3App theApp;
extern int g_msgID;
extern CManage gManage;

extern int gCleanList_tag;
BOOL WINAPI Call_back_devicelist(IJDCUCommand* pJDCUCommand,DWORD dwMsgID, IJXmlDocument* pXmlDoc,void* pParam)
{
	CMainDevice* pMaindeviceDlg = (CMainDevice*)pParam;

	if (false == pMaindeviceDlg->m_showFlag)
	{
		return TRUE;
	}
	if (gCleanList_tag == 0)
	{
		pMaindeviceDlg->m_DeviceList.DeleteAllItems();
		pMaindeviceDlg->PostMessage(WM_PAINT);       
	}
	if (gCleanList_tag>=TIMES)
	{
		return TRUE;
	}
	gCleanList_tag++;
	//pXmlDoc->SaveFile("C:\\test_status2\\GetProxyDeviceInfoResponse.xml");  
	IJXmlElement* pEleMsg = pXmlDoc->FirstChild(XMLMSG);
	IJXmlElement* pDeviceListEle = NULL;
	IJXmlElement* pDeviceIdEle= NULL;
	IJXmlElement* pConditionEle= NULL;
	IJXmlElement* pTotalCountEle = NULL;
	IJXmlElement* pOffsetEle = NULL;
	IJXmlElement* pResultEle = NULL;
	CString deviceid;
	pResultEle = pEleMsg->FirstChild("resultCode");
	if (pResultEle)
	{
		pConditionEle = pResultEle->NextSibling();
		if(pConditionEle)
		{
			deviceid = pConditionEle->FirstChild("deviceId")->GetText();
		}		
		if (0!=StringToLong(pResultEle->GetText()))
			return TRUE;	
	}
	else
		return TRUE;
	if (_T("-1")==deviceid)//列表
	{
		if (pConditionEle)//conditon节点
		{
			if(pConditionEle->FirstChild("factoryName")){
				pMaindeviceDlg->m_factoryName = pConditionEle->FirstChild("factoryName")->GetText();

			}
			if(pConditionEle->FirstChild("machineType")){
				pMaindeviceDlg->m_machineType = StringToLong(pConditionEle->FirstChild("machineType")->GetText());

			}
			if(pConditionEle->FirstChild("sdkVersion"))
			{
				pMaindeviceDlg->m_sdkVersion = pConditionEle->FirstChild("sdkVersion")->GetText();
			}
		}
		if(pEleMsg->FirstChild("totalCount")){
			pMaindeviceDlg->m_count = StringToLong(pEleMsg->FirstChild("totalCount")->GetText()); // 设备的总数
		}
		pDeviceListEle = pEleMsg->FirstChild("deviceList");
		if (pDeviceListEle)
		{
			CString deviceId,puId,factoryname,machineType,sdkVersion;
			CString deviceState;//设备状态;
			CString errCode;//错误码
			int i = 0 ;
			if(0!=pMaindeviceDlg->m_DeviceList.GetItemCount())
				i =pMaindeviceDlg->m_DeviceList.GetItemCount()-1 ;

			pDeviceIdEle = pDeviceListEle->FirstChild("device");
			while (pDeviceIdEle)
			{
				deviceId = pDeviceIdEle->GetAttribute("id");
				puId = pDeviceIdEle->FirstChild("puId")->GetText();
				factoryname = pDeviceIdEle->FirstChild("factoryName")->GetText();
				CString str;
				str=pDeviceIdEle->FirstChild("machineType")->GetText(); 
				machineType = gManage.GetDeviceType(str);
				sdkVersion = pDeviceIdEle->FirstChild("sdkVersion")->GetText();

				deviceState = pDeviceIdEle->FirstChild("status")->GetText();  //设备状态;
				errCode = pDeviceIdEle->FirstChild("errCode")->GetText(); //主动获取设备信息---错误码
				
				int j =  pMaindeviceDlg->m_DeviceList.InsertItem(i,deviceId);
				pMaindeviceDlg->m_DeviceList.SetItemText(j,1,puId);
				pMaindeviceDlg->m_DeviceList.SetItemText(j,2,factoryname);
				pMaindeviceDlg->m_DeviceList.SetItemText(j,3,machineType);
				pMaindeviceDlg->m_DeviceList.SetItemText(j,4,sdkVersion);
				pMaindeviceDlg->SetDevceState(j,deviceState, _T(":")+errCode);
				pDeviceIdEle = pDeviceIdEle->NextSibling();
				i++;
			}

		}
	}
	
	// 获取到设备列表, 且填充MainDevice这后, 此时前获取到DeviceList的最后的设备的ID了
	CString startDeviceId, endDeviceId;
	pMaindeviceDlg->GetLimitDeviceId(startDeviceId, endDeviceId);
	gManage.LimitBroadcastDeviceStatus(startDeviceId, endDeviceId);  // 所有的获取设备列表的后面

	return TRUE;
}
BOOL WINAPI Call_back_userlistret(IJDCUCommand* pJDCUCommand,DWORD dwMsgID, IJXmlDocument* pXmlDoc,void* pParam)
{

	S_Prameter* p_prameter = (S_Prameter*)pParam;

	if (gCleanList_tag == 0)
	{
		p_prameter->pMainUserDlg->m_UserList.DeleteAllItems();
	}

	if (gCleanList_tag>=TIMES)
	{
		return TRUE;
	}
	gCleanList_tag++;
//	pXmlDoc->SaveFile("c:\\userlist.xml");
	IJXmlElement* pEleMsg = pXmlDoc->FirstChild(XMLMSG);
	IJXmlElement* pUserListEle = NULL;
	IJXmlElement* pUserNameEle= NULL;
	IJXmlElement* pTotalCountEle = NULL;
	IJXmlElement* pOffsetEle = NULL;
	CString temp_name;
	int num;
	pUserNameEle = pEleMsg->FirstChild("userName");
	if (pUserNameEle)
	{
		p_prameter->pMainUserDlg->m_username = pUserNameEle->GetText();
		pTotalCountEle = pUserNameEle->NextSibling();

	}
	if (pTotalCountEle)
	{
		p_prameter->pMainUserDlg->m_count = StringToLong(pTotalCountEle->GetText());
		pOffsetEle = pTotalCountEle->NextSibling();
		CString str;
		int totaltcount = p_prameter->pMainUserDlg->m_count/COUNT_PER_PAG+1;
		int currentpage = p_prameter->pMainUserDlg->m_offset/COUNT_PER_PAG+1;
		str.Format(_T("%d / %d"),currentpage,totaltcount);
		p_prameter->pMainUserDlg->SetDlgItemText(IDC_SHOWPAGE,str);
	}
	if (pOffsetEle)
	{
		//p_prameter->pMainUserDlg->offset = StringToLong(pOffsetEle->GetText());

	}
	pUserListEle = pEleMsg->LastChild("userList");
	pUserNameEle=NULL;
	if (pUserListEle)
	{

		num = StringToLong(pUserListEle->GetAttribute("count"));
		pUserNameEle = pUserListEle->FirstChild("userName");
		int i = 0 ;
		if(0!=p_prameter->pMainUserDlg->m_UserList.GetItemCount())
			i =p_prameter->pMainUserDlg->m_UserList.GetItemCount()-1 ;


		int j = 0;///test
		while (pUserNameEle)
		{
			temp_name = pUserNameEle->GetText();
			j = p_prameter->pMainUserDlg->m_UserList.InsertItem(i,temp_name);
		  if (_T("admin") == temp_name)
			p_prameter->pMainUserDlg->m_UserList.SetItemText(j,1,_T("超级用户"));
		  else
			  p_prameter->pMainUserDlg->m_UserList.SetItemText(j,1,_T("普通用户"));


			pUserNameEle = pUserNameEle->NextSibling();
			i++;
		}
	}
	return TRUE;
}
BOOL WINAPI Call_back_heart(IJDCUCommand* pJDCUCommand,DWORD dwMsgID, IJXmlDocument* pXmlDoc,void* pParam)
{
	IJXmlElement* pTimeEle = NULL;
	IJXmlElement* pEleMsg = pXmlDoc->FirstChild(XMLMSG);
	//TRACE("rev heart!!\n\n");
	CMainFrame* pMain = (CMainFrame *)(AfxGetApp()->m_pMainWnd);
	LOG("[INFO] ProxyClient recv heart call back from Proxy Server, timeout:%d, msgId=%d\n", pMain->m_timeout, dwMsgID);
	pMain->m_timeout = 0;
	CString strTime;
	if (pEleMsg)
		pTimeEle = pEleMsg->FirstChild("serverTime");

	if (pTimeEle)
		strTime = pTimeEle->GetText();
	return TRUE;
}
BOOL WINAPI Call_back_AddDevBrocast(IJDCUCommand* pJDCUCommand,DWORD dwMsgID, IJXmlDocument* pXmlDoc,void* pParam)
{
	CMainDevice* pMaindeviceDlg = (CMainDevice*)pParam;
	IJXmlElement* pDeviceIdEle = NULL;
	CString deviceId;
	IJXmlElement* pEleMsg = pXmlDoc->FirstChild(XMLMSG);
	if (pEleMsg)
	{
		pDeviceIdEle = pEleMsg->FirstChild("deviceId");
	}
	if (pDeviceIdEle)
	{
		deviceId = pDeviceIdEle->GetText();
	}
	LOG("[INFO] Brocast from Proxy Server AddDev deviceId:%d\n", StringToLong(StringUnicodeToAscii((LPCTSTR)deviceId).c_str()));
	device_info tempdevice = gManage.GetDeviceInfo(deviceId);

	if (!tempdevice.device_id.IsEmpty())
	{
		int n = pMaindeviceDlg->m_DeviceList.GetItemCount();
		if (n<COUNT_PER_PAG)   //如果是最后一页而且没满才加入；
		{
			int tag = 0;
			CString str;
			for (int i = 0;i<n;i++)//如果列表已经有了 就不用添加进去
			{
				str = pMaindeviceDlg->m_DeviceList.GetItemText(i,0);
				if (str== tempdevice.device_id)
				{
					tag = 1;
					break;
				}
			}
			if (tag == 0)
			{
				LOG("[INFO] Succeed to AddDev deviceId:%d\n", StringToLong(StringUnicodeToAscii((LPCTSTR)deviceId).c_str()));
				int j =pMaindeviceDlg->m_DeviceList.InsertItem(n,tempdevice.device_id);
				pMaindeviceDlg->m_DeviceList.SetItemText(j,1,tempdevice.pu_id);
				pMaindeviceDlg->m_DeviceList.SetItemText(j,2,tempdevice.factoryname);
				pMaindeviceDlg->m_DeviceList.SetItemText(j,3,gManage.GetDeviceType(tempdevice.type));
				pMaindeviceDlg->m_DeviceList.SetItemText(j,4,tempdevice.sdk_version);
			}

		}
	}
	// 获取到设备列表, 且填充MainDevice这后, 此时前获取到DeviceList的最后的设备的ID了
	CString startDeviceId, endDeviceId;
	pMaindeviceDlg->GetLimitDeviceId(startDeviceId, endDeviceId);
	gManage.LimitBroadcastDeviceStatus(startDeviceId, endDeviceId);  // 所有的获取设备列表的后面

	return TRUE;
}

BOOL WINAPI Call_back_DelDevBrocast(IJDCUCommand* pJDCUCommand,DWORD dwMsgID, IJXmlDocument* pXmlDoc,void* pParam)
{
	CMainDevice* pMaindeviceDlg = (CMainDevice*)pParam;
	IJXmlElement* pDeviceIdEle = NULL;
	CString deviceId;
	IJXmlElement* pEleMsg = pXmlDoc->FirstChild(XMLMSG);
	if (pEleMsg)
	{
		pDeviceIdEle = pEleMsg->FirstChild("deviceId");
	}
	if (pDeviceIdEle)
	{
		deviceId = pDeviceIdEle->GetText();
	}
	CString temp_listid;
	int count = pMaindeviceDlg->m_DeviceList.GetItemCount();
	for (int m = 0;m<count;m++)
	{
		temp_listid = pMaindeviceDlg->m_DeviceList.GetItemText(m,0);
		if (deviceId ==temp_listid)
		{
			pMaindeviceDlg->m_DeviceList.DeleteItem(m);
		}	
	} 
	// 获取到设备列表, 且填充MainDevice这后, 此时前获取到DeviceList的最后的设备的ID了
	CString startDeviceId, endDeviceId;
	pMaindeviceDlg->GetLimitDeviceId(startDeviceId, endDeviceId);
	gManage.LimitBroadcastDeviceStatus(startDeviceId, endDeviceId);  // 所有的获取设备列表的后面

	return TRUE;
}

BOOL WINAPI Call_back_ModifyDevBrocast(IJDCUCommand* pJDCUCommand,DWORD dwMsgID, IJXmlDocument* pXmlDoc,void* pParam)
{

	CMainDevice* pMaindeviceDlg = (CMainDevice*)pParam;
	IJXmlElement* pDeviceIdEle = NULL;
	CString deviceId;
	IJXmlElement* pEleMsg = pXmlDoc->FirstChild(XMLMSG);
	if (pEleMsg)
	{
		pDeviceIdEle = pEleMsg->FirstChild("deviceId");
	}
	if (pDeviceIdEle)
	{
		deviceId = pDeviceIdEle->GetText();
	}
	CString temp_listid;
	int count = pMaindeviceDlg->m_DeviceList.GetItemCount();
	for (int m = 0;m<count;m++)
	{
		temp_listid = pMaindeviceDlg->m_DeviceList.GetItemText(m,0);
		if (deviceId ==temp_listid)
		{
			device_info tempdevice = gManage.GetDeviceInfo(deviceId);
			if (!tempdevice.device_id.IsEmpty())
			{
				//pMaindeviceDlg->m_DeviceList.DeleteItem(m);	
				//int j =pMaindeviceDlg->m_DeviceList.InsertItem(m,tempdevice.device_id);
				pMaindeviceDlg->m_DeviceList.SetItemText(m,0,tempdevice.device_id);
				pMaindeviceDlg->m_DeviceList.SetItemText(m,1,tempdevice.pu_id);
				pMaindeviceDlg->m_DeviceList.SetItemText(m,2,tempdevice.factoryname);
				pMaindeviceDlg->m_DeviceList.SetItemText(m,3,gManage.GetDeviceType(tempdevice.type));
				pMaindeviceDlg->m_DeviceList.SetItemText(m,4,tempdevice.sdk_version);

			}
		}	
	} 
	return TRUE;
}

BOOL WINAPI Call_back_AddUserBrocast(IJDCUCommand* pJDCUCommand,DWORD dwMsgID, IJXmlDocument* pXmlDoc,void* pParam)
{
	CMainUser* pMainUserDlg = (CMainUser*)pParam;
	IJXmlElement* pUserEle = NULL;
	CString userName;
	IJXmlElement* pEleMsg = pXmlDoc->FirstChild(XMLMSG);
	if (pEleMsg)
	{
		pUserEle = pEleMsg->FirstChild("userName");
	}
	if (pUserEle)
	{
		userName = pUserEle->GetText();
	}

	if (!userName.IsEmpty())
	{
		int n = pMainUserDlg->m_UserList.GetItemCount();
		if (n<COUNT_PER_PAG)
		{
			int tag = 0;
			CString str;
			for (int i = 0;i<n;i++)//如果列表已经有了 就不用添加进去
			{
				str = pMainUserDlg->m_UserList.GetItemText(i,0);
				if (str== userName)
				{
					tag = 1;
					break;
				}
			}
			if (tag == 0)
			{
				int m = pMainUserDlg->m_UserList.InsertItem(n,userName);
				if (userName == _T("admin"))
					pMainUserDlg->m_UserList.SetItemText(m,1,_T("超级用户"));
				else
					pMainUserDlg->m_UserList.SetItemText(m,1,_T("普通用户"));
			}
			
		}

	}

	return TRUE;
}

BOOL WINAPI Call_back_DelUserBrocast(IJDCUCommand* pJDCUCommand,DWORD dwMsgID, IJXmlDocument* pXmlDoc,void* pParam)
{
	CMainUser* pMainUserDlg = (CMainUser*)pParam;
	IJXmlElement* pUserEle = NULL;
	CString userName;
	IJXmlElement* pEleMsg = pXmlDoc->FirstChild(XMLMSG);
	if (pEleMsg)
	{
		pUserEle = pEleMsg->FirstChild("userName");
	}
	if (pUserEle)
	{
		userName = pUserEle->GetText();
	}

	if (!userName.IsEmpty())
	{
		int n = pMainUserDlg->m_UserList.GetItemCount();
		CString tempName;
		for (int i = 0;i<n;i++)
		{
			tempName = pMainUserDlg->m_UserList.GetItemText(i,0);
			if (tempName == userName)
			{
				 pMainUserDlg->m_UserList.DeleteItem(i);
			}

		}
	}

	return TRUE;
}
BOOL WINAPI Call_back_login(IJDCUCommand* pJDCUCommand,DWORD dwMsgID, IJXmlDocument* pXmlDoc,void* pParam)
{

	CLogin *pDlg = (CLogin*)pParam;
	CProxyClient3App* pView = (CProxyClient3App*)AfxGetApp();
	pDlg->KillTimer(0);//关闭登录服务器超时定时器
	int lRet;
	IJXmlElement* pEleMsg = pXmlDoc->FirstChild(XMLMSG);
	if(pEleMsg)
	{
		lRet = gManage.m_pXmlParser->JGetRetCode(pEleMsg,"resultCode");
		if (0==lRet)
		{
			IJXmlElement* pEle = NULL;
			pEle = pEleMsg->FirstChild("keepAliveTime");
			if (pEle)
			{
				pView->m_heart_elapse = StringToLong(pEle->GetText());
			}

		}

	}
	if(0==lRet)
		pDlg->Close();
	else if(-1==lRet)
	{
		pDlg->m_waiting_static.ShowWindow(FALSE);
		pDlg->SendMessage(UM_ERR);
		//AfxMessageBox(_T("用户名密码错误!"));

	}
	else if (1==lRet)
	{
		pDlg->m_waiting_static.ShowWindow(FALSE);
		AfxMessageBox(_T("此用户已经登录!"));;
	}
	gManage.UnReg(gManage.m_loginID);
	return TRUE;
}

BOOL WINAPI Call_back_info(IJDCUCommand* pJDCUCommand,DWORD dwMsgID, IJXmlDocument* pXmlDoc,void* pParam)
{
	IJXmlElement* pEleMsg = pXmlDoc->FirstChild("message");
	IJXmlElement* pfactory = NULL;
	IJXmlElement* pfactoryname= NULL;
	IJXmlElement* pmachinelist = NULL;
	IJXmlElement* pmachine= NULL;
	IJXmlElement* pmachinetype = NULL;
	IJXmlElement* pmachinename= NULL;
	IJXmlElement* pversion= NULL;

	factory *pLinkfactory,*rLinkfactory = NULL;//厂家链表指针
	device *pLinkdevice = NULL;
	device *rLinkdevice = NULL;//设备链表指针
	sdkversion *pLinkversion = NULL;
	sdkversion *rLinkversion = NULL;//版本链表指针 
	CString temp ;
	if (pEleMsg)
	{
		pfactory = pEleMsg->FirstChild("factory");

	}
	
	rLinkfactory = gManage.m_pFactory;	 
	while(pfactory){//factory循环 

		pLinkfactory = new factory;
		if (!gManage.m_pFactory)
		{
			gManage.m_pFactory = pLinkfactory;
			rLinkfactory = pLinkfactory;
		}
		else
		{
			rLinkfactory->next = pLinkfactory;//新建节点插入链表
			rLinkfactory = pLinkfactory;      //标记指针后移}
		}

		pfactoryname = pfactory->FirstChild("factoryName");
		pLinkfactory->name = pfactoryname->GetText();
		pmachinelist = pfactory->FirstChild("machineList");
		if (pmachinelist) {
			pmachine = pmachinelist->FirstChild("machineType");
			while(pmachine){   //设备循环
				pLinkdevice = new device;
				if (!rLinkdevice)
				{
					rLinkfactory->pDevice = pLinkdevice;
					rLinkdevice = pLinkdevice;
				}else{
					rLinkdevice->next = pLinkdevice;
					rLinkdevice = pLinkdevice;

				}

				pmachinename = pmachine;
				//pmachinename = pmachine->FirstChild();
				temp = pmachinename->GetText();
				pLinkdevice->name = temp;
				pversion = pmachinename->NextSibling();
				pversion = pversion->FirstChild("sdkVersion");
				while (pversion)//SDK版本循环
				{
					temp = pversion->GetText();
					pLinkversion = new sdkversion;
					pLinkversion->name = temp;
					if (!rLinkversion)
					{
						rLinkdevice->pVsersion = pLinkversion;
						rLinkversion = pLinkversion;
					}
					else
					{
						rLinkversion->next = pLinkversion;
						rLinkversion = pLinkversion;
					}
					pversion = pversion->NextSibling();		 
				}
				rLinkversion = NULL;
				pmachine = pmachine->NextSibling()->NextSibling();
				//pmachine = pmachine->NextSibling();

			}
		}
	
		rLinkdevice = NULL;
		pfactory = pfactory->NextSibling();
	}
	gManage.UnReg(gManage.m_infoID);
	return TRUE;
}


BOOL WINAPI SendHeartCB(IJDCUCommand* pJDCUCommand, void* pParam)
{
	gManage.SendHeart();
	return TRUE;
}

BOOL WINAPI ReconnServerCB(IJDCUCommand* pJDCUCommand, void* pParam)
{
	return TRUE;
}

BOOL WINAPI OfflineCB(IJDCUCommand* pJDCUCommand, void* pParam)
{
	CMainDevice* pMaindeviceDlg = (CMainDevice*)pParam;
	//pMaindeviceDlg->MessageBox(_T("服务器已断开连接, 请重新登陆"), NULL, MB_OK);
	AfxMessageBox(_T("服务器已断开连接, 请重新登陆!"));
	LOG("[WARNING] ProxyClient offline with Proxy Server\n");

	TCHAR exeFullPath[MAX_PATH]; // MAX_PATH
	GetModuleFileName(NULL, exeFullPath, MAX_PATH);//得到程序模块名称，全路径
	STARTUPINFO si; 
	PROCESS_INFORMATION pi; 
	ZeroMemory(&si,sizeof(si)); 
	si.cb=sizeof(si);
	CreateProcess(NULL,exeFullPath,NULL, NULL,FALSE, NORMAL_PRIORITY_CLASS,NULL,NULL,&si,&pi);
	HANDLE hCurrentProcess;
	hCurrentProcess  = ::GetCurrentProcess();
	TerminateProcess(hCurrentProcess,0);

	return TRUE;
}

CManage::CManage(void)
: m_devicecount(0)
, m_usercount(0)
{
	m_infoID = -1;
	m_loginID = -1;
	m_userlistID = -1;
	m_heartresponseID = -1;
	m_AddDevBrocastID = -1;

	m_DelDevBrocastID = -1;
	m_ModifyDevBrocastID = -1;
	m_AddUserBrocastID = -1;
	m_DelUserBrocastID = -1;

	m_devicelistID = -1;
	m_pXmlParser = NULL;
	m_pCommand = NULL; 
	m_name = _T("");
	m_pwd = _T("");
	m_port = _T("");
	m_pXmlParser = JDCUCreateParser();
	m_pCommand = JDCUCreateCommand();
	m_pFactory = NULL;
}

CManage::~CManage(void)
{

	m_pCommand->UnRegCmd(m_infoID);

	m_pCommand->UnRegCmd(m_loginID);

	m_pCommand->UnRegCmd(m_userlistID);
	m_pCommand->UnRegCmd(m_AddDevBrocastID);

	m_pCommand->UnRegCmd(m_heartresponseID);
	m_pCommand->UnRegCmd(m_DelDevBrocastID);
	m_pCommand->UnRegCmd(m_ModifyDevBrocastID);
	m_pCommand->UnRegCmd(m_AddUserBrocastID);
	m_pCommand->UnRegCmd(m_DelUserBrocastID);

	m_pCommand->UnRegCmd(m_devicelistID);

	m_pCommand->JUninit();
	m_pCommand->JDelete();
	m_pXmlParser->JDelete();


	while (m_pFactory)//释放链表
	{
		device *pDevice = m_pFactory->pDevice;
		while (pDevice)
		{
			sdkversion *pVersion = pDevice->pVsersion;
			while (pVersion)
			{
				sdkversion *pd_version = pVersion;
				pVersion = pVersion->next;
				delete pd_version;
			}


			device *pd_device = pDevice;
			pDevice = pDevice->next;
			delete pd_device;
		}
		factory *pd_factory = m_pFactory;
		m_pFactory = m_pFactory->next;
		delete pd_factory;
	}

}
int CManage::AddUser(const CString &name,const CString &pwd,void* wPram){

	int ret = -1;
	if(m_pXmlParser)
	{
		IJXmlDocument* pDoc = NULL;
		IJXmlElement* pMsgEle = m_pXmlParser->JCreateMsgDoc(pDoc, "AddUser");
		if(pMsgEle)
		{
			AppendChildEle(m_pXmlParser, pMsgEle, "userName", StringUnicodeToUtf8((LPCTSTR)name).c_str());
			AppendChildEle(m_pXmlParser, pMsgEle, "password", StringUnicodeToUtf8((LPCTSTR)pwd).c_str());

			IJXmlDocument* pRecDoc = NULL;
			DWORD dwMsgID = GetMsgNumber();
			m_pCommand->SendCmdMsg(dwMsgID,pDoc,pRecDoc,4000);
			ReleaseNumber(dwMsgID);

			if(NULL != pRecDoc)
			{
				IJXmlElement* pEleMsg = pRecDoc->FirstChild(XMLMSG);
				if(pEleMsg)
				{
					ret = m_pXmlParser->JGetRetCode(pEleMsg,"resultCode");
					pRecDoc->JDelete();
					return ret;

				}
			}
		}
	}

	return ret;
}
int CManage::Login(void *pWram){
	long lRet = -1;		
	char port[32] = {0};
	//m_pCommand = JDCUCreateCommand();

	WideCharToMultiByte(CP_ACP, 0, m_port.GetBuffer(), -1, port, sizeof(port), 0, 0);

	//if (Ping(m_IP) == FALSE)
	//{
	//	LOG("[ERR] Failed to PING server %s:%d(%s)\n", m_IP, _wtol(m_port), port);
	//	return -2;
	//}
	//else
	//{
	//	LOG("[INFO] Succeed to PING server %s:%d(%s)\n", m_IP, _wtol(m_port), port);
	//}
	
	if (m_pCommand)
	{
		lRet = m_pCommand->JInit(m_IP,_wtol(m_port), 1500);// 返回 0 为成功
		
		LOG("[INFO] Begin to connect server %s:%d(%s), result:%d\n", m_IP, _wtol(m_port), port, lRet);
	}
	if (0!=lRet)
	{
		return -2;
	}
	else
	{

		//m_loginID = m_pCommand->RegCmdCallback("UserLoginResponse",Call_back_login,pWram);	
		lRet = -3;
	
		IJXmlDocument* pDoc = NULL;
		IJXmlElement* pMsgEle = m_pXmlParser->JCreateMsgDoc(pDoc, "UserLogin");
		if(pMsgEle)
		{
			AppendChildEle(m_pXmlParser, pMsgEle, "userName", StringUnicodeToUtf8((LPCTSTR)gManage.m_name).c_str());
			AppendChildEle(m_pXmlParser, pMsgEle, "password", StringUnicodeToUtf8((LPCTSTR)gManage.m_pwd).c_str());
		}

		IJXmlDocument* pRecDoc = NULL;
		DWORD dwMsgID = CSerialLong::Instance().RequestSerialLong();
		//long ret = m_pCommand->SendCmdMsg(dwMsgID,pDoc,pRecDoc,4000);	//异步登录
		long ret = m_pCommand->SendCmdMsg(dwMsgID,pDoc,pRecDoc,10000);	
		CSerialLong::Instance().ReleaseSerialLong(dwMsgID);

		
		if(NULL != pRecDoc)
		{
			pRecDoc->SaveFile("c://t.xml");
			IJXmlElement* pEleMsg = pRecDoc->FirstChild(XMLMSG);

			lRet = gManage.m_pXmlParser->JGetRetCode(pEleMsg,"resultCode");
			if (0==lRet)
			{
				IJXmlElement* pEle = NULL;
				pEle = pEleMsg->FirstChild("keepAliveTime");
				if (pEle)
				{
					CProxyClient3App* pApp = (CProxyClient3App*)AfxGetApp(); 
					pApp->m_heart_elapse = StringToLong(pEle->GetText());
				}

			}
		
		}

		return lRet;

	}
}
int CManage::DelUser(const CString &name){

	long ret=-1;
	IJXmlDocument* pDoc=NULL;
	if(m_pXmlParser)
	{
		IJXmlDocument* pDoc = NULL;
		IJXmlElement* pMsgEle = m_pXmlParser->JCreateMsgDoc(pDoc, "DelUser");
		if(pMsgEle)
		{
			AppendChildEle(m_pXmlParser, pMsgEle, "userName", StringUnicodeToUtf8((LPCTSTR)name).c_str());

			IJXmlDocument* pRecDoc = NULL;
			DWORD dwMsgID = GetMsgNumber();
			m_pCommand->SendCmdMsg(dwMsgID,pDoc,pRecDoc,4000);
			ReleaseNumber(dwMsgID);

			if(NULL != pRecDoc)
			{
				IJXmlElement* pEleMsg = pRecDoc->FirstChild(XMLMSG);
				if(pEleMsg)
				{
					ret = m_pXmlParser->JGetRetCode(pEleMsg,"resultCode");
					pRecDoc->JDelete();
					return ret;

				}
			}
		}
	}
	return ret;
}

int CManage::DelDevice(const CString &deviceID){
	long ret=-1;
	IJXmlDocument* pDoc=NULL;
	if(m_pXmlParser)
	{
		IJXmlDocument* pDoc = NULL;
		IJXmlElement* pMsgEle = m_pXmlParser->JCreateMsgDoc(pDoc, "DelProxyDevice");
		if(pMsgEle)
		{
			AppendChildEle(m_pXmlParser, pMsgEle, "deviceId", StringUnicodeToUtf8((LPCTSTR)deviceID).c_str());

			IJXmlDocument* pRecDoc = NULL;
			DWORD dwMsgID = GetMsgNumber();
			m_pCommand->SendCmdMsg(dwMsgID,pDoc,pRecDoc,4000);
			ReleaseNumber(dwMsgID);

			if(NULL != pRecDoc)
			{
				IJXmlElement* pEleMsg = pRecDoc->FirstChild(XMLMSG);
				if(pEleMsg)
				{
					ret = m_pXmlParser->JGetRetCode(pEleMsg,"resultCode");
					pRecDoc->JDelete();
					return ret;
				}
			}
		}
	}
	return ret;
}
int CManage::UserModify(const CString &name,const CString &pwd,const CString &oldpwd){

	int  ret = -1;
	if(m_pXmlParser)
	{
		IJXmlDocument* pDoc = NULL;
		IJXmlElement* pMsgEle = m_pXmlParser->JCreateMsgDoc(pDoc, "ModifyPassword");
		if(pMsgEle)
		{
			AppendChildEle(m_pXmlParser, pMsgEle, "userName", StringUnicodeToUtf8((LPCTSTR)name).c_str());
			AppendChildEle(m_pXmlParser, pMsgEle, "oldPassword", StringUnicodeToUtf8((LPCTSTR)oldpwd).c_str());
			AppendChildEle(m_pXmlParser, pMsgEle, "newPassword", StringUnicodeToUtf8((LPCTSTR)pwd).c_str());

			IJXmlDocument* pRecDoc = NULL;
			DWORD dwMsgID = CSerialLong::Instance().RequestSerialLong();
			m_pCommand->SendCmdMsg(dwMsgID,pDoc,pRecDoc,4000);
			CSerialLong::Instance().ReleaseSerialLong(dwMsgID);

			if(NULL != pRecDoc)
			{
				IJXmlElement* pEleMsg = pRecDoc->FirstChild(XMLMSG);
				if(pEleMsg)
				{
					ret = m_pXmlParser->JGetRetCode(pEleMsg,"resultCode");
					pRecDoc->JDelete();
					return ret;

				}
			}
		}
	}

	return ret;

}
void CManage::GetUserList(CString name,int offset){

	if(m_pXmlParser)
	{

		CString str;
		CString str2;
		str2.Format(_T("%d"),COUNT_PER_XML);
		str.Format(_T("%d"),offset);

		IJXmlDocument* pDoc = NULL;
		IJXmlElement* pMsgEle = m_pXmlParser->JCreateMsgDoc(pDoc, "GetUserInfo");
		if(pMsgEle)
		{
			AppendChildEle(m_pXmlParser, pMsgEle, "userName", StringUnicodeToUtf8((LPCTSTR)name).c_str());
			AppendChildEle(m_pXmlParser, pMsgEle, "offset", StringUnicodeToUtf8((LPCTSTR)str).c_str());
			AppendChildEle(m_pXmlParser, pMsgEle, "count", StringUnicodeToUtf8((LPCTSTR)str2).c_str());
		}

		//	pDoc->SaveFile("c:\\testtttttt.xml");
		DWORD dwMsgId = GetMsgNumber();
		m_pCommand->SendCmdMsg(dwMsgId, pDoc);		
		ReleaseNumber(dwMsgId);
	}


}

void CManage::GetDeviceList(void* p,CString factoryName,int machineType,CString sdkVersion,int offset,int deviceId)
{
	CString str;
	CString str2;
	CString str3;
	CString deviceid;
	str2.Format(_T("%d"),COUNT_PER_XML);
	str.Format(_T("%d"),offset);
	str3.Format(_T("%d"),machineType);
	deviceid.Format(_T("%d"),deviceId);

	if(m_pXmlParser)
	{	
		IJXmlDocument* pDoc = NULL;
		IJXmlElement* pMsgEle = m_pXmlParser->JCreateMsgDoc(pDoc, "GetProxyDeviceInfo");
		if(pMsgEle)
		{
			IJXmlElement* pConditionEle = AppendChildEle(m_pXmlParser, pMsgEle, "condition", "");

			AppendChildEle(m_pXmlParser, pConditionEle, "factoryName", StringUnicodeToUtf8((LPCTSTR)factoryName).c_str());
			AppendChildEle(m_pXmlParser, pConditionEle, "machineType", StringUnicodeToUtf8((LPCTSTR)str3).c_str());
			AppendChildEle(m_pXmlParser, pConditionEle, "sdkVersion", StringUnicodeToUtf8((LPCTSTR)sdkVersion).c_str());
			AppendChildEle(m_pXmlParser, pConditionEle, "deviceId", StringUnicodeToUtf8((LPCTSTR)deviceid).c_str());
			AppendChildEle(m_pXmlParser, pMsgEle, "offset", StringUnicodeToUtf8((LPCTSTR)str).c_str());
			AppendChildEle(m_pXmlParser, pMsgEle, "count", StringUnicodeToUtf8((LPCTSTR)str2).c_str());
		}

		DWORD dwMsgID = GetMsgNumber();
		m_pCommand->SendCmdMsg(dwMsgID,pDoc);
		ReleaseNumber(dwMsgID);
	}
	//CString endDeviceId;
	//endDeviceId.Format(_T("%d"), offset + COUNT_PER_XML);
	//LimitBroadcastDeviceStatus(str, endDeviceId);  // 所有的获取设备列表的后面
}

int CManage::AddDevice(device_info device)
{

	int ret = -1;
	if(m_pXmlParser)
	{
		IJXmlDocument* pDoc = NULL;
		IJXmlElement* pMsgEle = m_pXmlParser->JCreateMsgDoc(pDoc, "AddProxyDevice");
		if(pMsgEle)
		{
			AppendChildEle(m_pXmlParser, pMsgEle, "deviceId", "");
			AppendChildEle(m_pXmlParser, pMsgEle, "puId", StringUnicodeToUtf8((LPCTSTR)device.pu_id).c_str());
			AppendChildEle(m_pXmlParser, pMsgEle, "machineType", StringUnicodeToUtf8((LPCTSTR)device.type).c_str());
			AppendChildEle(m_pXmlParser, pMsgEle, "factoryName", StringUnicodeToUtf8((LPCTSTR)device.factoryname).c_str());
			AppendChildEle(m_pXmlParser, pMsgEle, "sdkVersion", StringUnicodeToUtf8((LPCTSTR)device.sdk_version).c_str());
			AppendChildEle(m_pXmlParser, pMsgEle, "userName", StringUnicodeToUtf8((LPCTSTR)device.username).c_str());
			AppendChildEle(m_pXmlParser, pMsgEle, "password", StringUnicodeToUtf8((LPCTSTR)device.password).c_str());
			AppendChildEle(m_pXmlParser, pMsgEle, "deviceIp", StringUnicodeToUtf8((LPCTSTR)device.device_ip).c_str());
			AppendChildEle(m_pXmlParser, pMsgEle, "devicePort", StringUnicodeToUtf8((LPCTSTR)device.device_port).c_str());
			AppendChildEle(m_pXmlParser, pMsgEle, "platformIp", StringUnicodeToUtf8((LPCTSTR)device.platform_ip).c_str());
			AppendChildEle(m_pXmlParser, pMsgEle, "platformPort", StringUnicodeToUtf8((LPCTSTR)device.platform_port).c_str());

			IJXmlDocument* pRecDoc = NULL;
			DWORD dwMsgID = GetMsgNumber();
			m_pCommand->SendCmdMsg(dwMsgID,pDoc,pRecDoc,4000);
			ReleaseNumber(dwMsgID);

			if(NULL != pRecDoc)
			{
				IJXmlElement* pEleMsg = pRecDoc->FirstChild(XMLMSG);
				if(pEleMsg)
				{
					ret = m_pXmlParser->JGetRetCode(pEleMsg,"resultCode");
					pRecDoc->JDelete();
					return ret;

				}
			}
		}
	}
	return ret;
}
int CManage::DeviceModify(device_info device){
	int ret = -1;
	if(m_pXmlParser)
	{
		IJXmlDocument* pDoc = NULL;
		IJXmlElement* pMsgEle = m_pXmlParser->JCreateMsgDoc(pDoc, "SetProxyDeviceInfo");
		if(pMsgEle)
		{
			AppendChildEle(m_pXmlParser, pMsgEle, "deviceId", StringUnicodeToUtf8((LPCTSTR)device.device_id).c_str());
			AppendChildEle(m_pXmlParser, pMsgEle, "puId", StringUnicodeToUtf8((LPCTSTR)device.pu_id).c_str());
			AppendChildEle(m_pXmlParser, pMsgEle, "machineType", StringUnicodeToUtf8((LPCTSTR)device.type).c_str());
			AppendChildEle(m_pXmlParser, pMsgEle, "factoryName", StringUnicodeToUtf8((LPCTSTR)device.factoryname).c_str());
			AppendChildEle(m_pXmlParser, pMsgEle, "sdkVersion", StringUnicodeToUtf8((LPCTSTR)device.sdk_version).c_str());
			AppendChildEle(m_pXmlParser, pMsgEle, "userName", StringUnicodeToUtf8((LPCTSTR)device.username).c_str());
			AppendChildEle(m_pXmlParser, pMsgEle, "password", StringUnicodeToUtf8((LPCTSTR)device.password).c_str());
			AppendChildEle(m_pXmlParser, pMsgEle, "deviceIp", StringUnicodeToUtf8((LPCTSTR)device.device_ip).c_str());
			AppendChildEle(m_pXmlParser, pMsgEle, "devicePort", StringUnicodeToUtf8((LPCTSTR)device.device_port).c_str());
			AppendChildEle(m_pXmlParser, pMsgEle, "platformIp", StringUnicodeToUtf8((LPCTSTR)device.platform_ip).c_str());
			AppendChildEle(m_pXmlParser, pMsgEle, "platformPort", StringUnicodeToUtf8((LPCTSTR)device.platform_port).c_str());

			IJXmlDocument* pRecDoc = NULL;
			DWORD dwMsgID = GetMsgNumber();
			m_pCommand->SendCmdMsg(dwMsgID,pDoc,pRecDoc,4000);
			ReleaseNumber(dwMsgID);

			if(NULL != pRecDoc)
			{
				IJXmlElement* pEleMsg = pRecDoc->FirstChild(XMLMSG);
				if(pEleMsg)
				{
					ret = m_pXmlParser->JGetRetCode(pEleMsg,"resultCode");
					pRecDoc->JDelete();
					return ret;

				}
			}
		}
	}
	return ret;
}
device_info CManage::GetDeviceInfo(CString deviceid){
	IJXmlDocument* pRecDoc = NULL;
	device_info deviceinfo;
	if(m_pXmlParser)
	{	
		CString str;
		CString str2;
		CString str3;
		str2.Format(_T("%d"),COUNT_PER_XML);
		str.Format(_T("%d"),0);
		str3.Format(_T("%d"),-1);

		IJXmlDocument* pDoc = NULL;
		IJXmlElement* pMsgEle = m_pXmlParser->JCreateMsgDoc(pDoc, "GetProxyDeviceInfo");
		if(pMsgEle)
		{
			IJXmlElement* pConditionEle = AppendChildEle(m_pXmlParser, pMsgEle, "condition", "");

			AppendChildEle(m_pXmlParser, pConditionEle, "factoryName", "");
			AppendChildEle(m_pXmlParser, pConditionEle, "machineType", StringUnicodeToUtf8((LPCTSTR)str3).c_str());
			AppendChildEle(m_pXmlParser, pConditionEle, "sdkVersion",  "");
			AppendChildEle(m_pXmlParser, pConditionEle, "deviceId", StringUnicodeToUtf8((LPCTSTR)deviceid).c_str());
			AppendChildEle(m_pXmlParser, pMsgEle, "offset", StringUnicodeToUtf8((LPCTSTR)str).c_str());
			AppendChildEle(m_pXmlParser, pMsgEle, "count", StringUnicodeToUtf8((LPCTSTR)str2).c_str());
			
			//m_devicelistID = gManage.m_pCommand->RegCmdCallback("GetProxyDeviceInfoResponse",Call_back_devicelist,pPram);
			
			//UnReg(m_devicelistID);

			DWORD dwMsgID = GetMsgNumber();
			m_pCommand->SendCmdMsg(dwMsgID, pDoc, pRecDoc, 4000);
			ReleaseNumber(dwMsgID);
			
			//RegCallBackDeviceList(m_MaindeviceDlg);  //重新注册
		}
	}

	if (pRecDoc)
	{
		deviceinfo = Doc2Device(pRecDoc);
	}
	else
	{
		LOG("[WARNING] Failed to receive XML GetProxyDeviceInfoResponse.\n");
	}

	return deviceinfo;
}


const char *CManage::CString2char(CString str){

	int n=str.GetLength();
	char *c = (char*)malloc(n+1);
	if (c)
	{
		for (int i=0;i<=n;i++)
		{
			c[i]=(char)str.GetAt(i);
		}
		return c;
	}
	return NULL;
}

device_info CManage::Doc2Device(IJXmlDocument* pDevice){
	device_info device;
	IJXmlElement* pItem = NULL;
	IJXmlElement* pDeviceListEle = NULL;
	IJXmlElement* pDeviceEle = NULL;
	IJXmlElement* pEleMsg = NULL;

	if (pDevice)
	{
		pEleMsg = pDevice->FirstChild(XMLMSG);////////
	}

	if (pEleMsg)
	{
		pDeviceListEle = pEleMsg->FirstChild("deviceList");
	}

	if (pDeviceListEle)
	{
		pDeviceEle =pDeviceListEle->FirstChild("device");
		device.device_id =pDeviceEle->GetAttribute("id");
	}

	if (pDeviceEle)
	{
		pItem = pDeviceEle->FirstChild("puId");
	}

	if (pItem)
	{		
		device.pu_id =pDeviceEle->FirstChild("puId")->GetText();//平台ID
		device.type = pDeviceEle->FirstChild("machineType")->GetText();//machineType;
		device.protocol =  pDeviceEle->FirstChild("protocol")->GetText();//protocol;
		device.factoryname = pDeviceEle->FirstChild("factoryName")->GetText();//factoryName;
		device.sdk_version = pDeviceEle->FirstChild("sdkVersion")->GetText();//版本
		device.username =pDeviceEle->FirstChild("userName")->GetText();//用户名
		device.password = pDeviceEle->FirstChild("password")->GetText();//密码
		device.device_ip =pDeviceEle->FirstChild("deviceIp")->GetText();//设备Ip
		device.device_port =pDeviceEle->FirstChild("devicePort")->GetText();//设备端口
		device.platform_ip =pDeviceEle->FirstChild("platformIp")->GetText();//平台IP
		device.platform_port =pDeviceEle->FirstChild("platformPort")->GetText();//平台端口
	}

	return device;
}

sdkversion::sdkversion(void){
	next = NULL;
}

device::device(void){
	pVsersion =NULL;
	next = NULL;
}

factory::factory(void){

	pDevice = NULL;
	next = NULL;
}
/*
J_SDK_SDEC,                         //软解码器
J_SDK_DEC,                          //硬解码器

*/
CString CManage::GetDeviceType(CString n)
{
/*
	CString strDeviceType;
	switch(n)
	{
	case _T("0"):
		strDeviceType = _T("DVR");
		break;
	case _T("1"):
		strDeviceType = _T("DVS");
		break;
	case _T("2"):
		strDeviceType = _T("IPC");		
		break;
	case _T("3"):
		strDeviceType = _T("IPNC");
		break;	
	case _T("4"):
		strDeviceType = _T("SDEC");
		break;	
	case _T("5"):
		strDeviceType = _T("DEC");
		break;	
	case _T("6"):
		strDeviceType = _T("NVR");
		break;	
	case _T("7"):
		strDeviceType = _T("HVR");
		break;
	default:
		break;
	}
*/
	//return _T("未知设备类型");

	if(_T("0")==n)
		return _T("DVR");
	else if (_T("1")==n)
	{
		return _T("DVS");
	}
	else if (_T("2")==n)
		return _T("IPC");
	else if (_T("3")==n)
	{
		return _T("IPNC");
	}
	else if (_T("4")==n)
		return _T("SDEC");
	else if (_T("5")==n)
		return _T("DEC");
	else if (_T("6")==n)
		return _T("NVR");
	else if (_T("7")==n)
		return _T("HVR");	

	return _T("未知设备类型");

	
}

CString CManage::GetDeviceNum(CString n)
{
	if (_T("DVR")==n)//类型转换成枚举 。。。
	{
		return _T("0");

	}
	else if(_T("DVS")==n){
		return _T("1");

	}
	else if (_T("IPC")==n)
	{
		return _T("2");
	}
	else if (_T("IPNC")==n)
	{
		return _T("3");
	}
	else if (_T("SDEC")==n)
		return _T("4");
	else if (_T("DEC")==n)
		return _T("5");
	else if (_T("NVR") ==n)
		return _T("6");
	else if (_T("HVR"))
		return _T("7");
	return 0;

}

IJXmlElement* CManage::AppendChile(IJXmlParser *m_pXmlParser,IJXmlElement *pEle,const char *type,CString str){

	IJXmlElement* pRet;
	const char *p = NULL;
	p = CString2char(str);

	pRet = m_pXmlParser->JAppendChildEle(pEle, type, p);
	//m_pXmlParser->JAppendChildEle(pEle, type, StringAsciiToUtf8(p).c_str());
	free((void*)p);
	return pRet;

}

IJXmlElement* CManage::AppendChildEle(IJXmlParser* pXmlParser, IJXmlElement* pParent, const char* pszName, const char* pszValue)
{
	return pXmlParser->JAppendChildEle(pParent, pszName, pszValue);
}

void CManage::SendHeart(void)//发心跳包
{
	IJXmlDocument* pDoc = NULL;
	IJXmlElement* pMsgEle = m_pXmlParser->JCreateMsgDoc(pDoc, "UserHeart");
	if(pMsgEle)
	{
		AppendChildEle(m_pXmlParser, pMsgEle, "userName", StringUnicodeToUtf8((LPCTSTR)gManage.m_name).c_str());
	}
	DWORD dwMsgId = GetMsgNumber();
	m_pCommand->SendCmdMsg(dwMsgId, pDoc);
	
	LOG("[INFO] SendHeart to Proxy Server, msgId=%d\n", dwMsgId);
	TRACE("SendHeart\n\n");
}

void CManage::GetBaseInfo(void)
{
	IJXmlDocument* pDoc = NULL;
	IJXmlElement* pMsgEle = m_pXmlParser->JCreateMsgDoc(pDoc, "GetFactoryInfo");
	if(pMsgEle)
	{
		DWORD dwMsgId = GetMsgNumber();
		m_pCommand->SendCmdMsg(dwMsgId, pDoc);
		ReleaseNumber(dwMsgId);		
	}
}

void CManage::RegCallBack(void* wPram)
{
	S_Prameter* p_prameter = (S_Prameter*)wPram;

	m_MaindeviceDlg = p_prameter->pMaindeviceDlg;  //保存该参数, 用于设备列表回调函数的重新注册
	//厂家，设备，版本基本信息回调函数
	m_infoID = m_pCommand->RegCmdCallback("GetFactoryInfoResponse",Call_back_info,wPram);
	//设备列表回调函数
	m_devicelistID = m_pCommand->RegCmdCallback("GetProxyDeviceInfoResponse",Call_back_devicelist,p_prameter->pMaindeviceDlg);
	//用户列表回调函数
	m_userlistID = m_pCommand->RegCmdCallback("GetUserInfoResponse",Call_back_userlistret,wPram);
	//心跳回调函数
	m_heartresponseID  =  m_pCommand->RegCmdCallback("UserHeartResponse",Call_back_heart,wPram);
	//添加设备回调函数
	m_AddDevBrocastID  =  m_pCommand->RegCmdCallback("BroadcastAddDevice",Call_back_AddDevBrocast,p_prameter->pMaindeviceDlg);
    //删除设备回调
	m_DelDevBrocastID  =  m_pCommand->RegCmdCallback("BroadcastDelDevice",Call_back_DelDevBrocast,p_prameter->pMaindeviceDlg);
    //修改设备信息回调
	m_ModifyDevBrocastID  =  m_pCommand->RegCmdCallback("BroadcastModifyDevice",Call_back_ModifyDevBrocast,p_prameter->pMaindeviceDlg);
	//添加用户回调
	m_AddUserBrocastID  =  m_pCommand->RegCmdCallback("BroadcastAddUser",Call_back_AddUserBrocast,p_prameter->pMainUserDlg);
    //删除用户回调
	m_DelUserBrocastID  =  m_pCommand->RegCmdCallback("BroadcastDelUser",Call_back_DelUserBrocast,p_prameter->pMainUserDlg);

	CProxyClient3App* pApp = (CProxyClient3App*)AfxGetApp(); 
	int heart_elapse = pApp->m_heart_elapse;

	m_pCommand->SetHeartTime((heart_elapse<=0) ? 15 : heart_elapse);
	DWORD dwRegHeadID  =  m_pCommand->RegHeartCallback(SendHeartCB,p_prameter->pMainUserDlg);
	//DWORD dwRegReconnID = m_pCommand->RegReconnCallback(ReconnServerCB, p_prameter->pMaindeviceDlg);
	DWORD dwRetOfflineID = m_pCommand->RegOfflineCallback(OfflineCB, p_prameter->pMaindeviceDlg);
}

void CManage::UnReg(long id)
{
	m_pCommand->UnRegCmd(id);
}
void CManage::RegCallBackDeviceList(void* pPram)
{
	m_devicelistID = gManage.m_pCommand->RegCmdCallback("GetProxyDeviceInfoResponse",Call_back_devicelist,pPram);
}

int CManage::SetConfig(CString ip,CString listenport,CString rtspport)
{
	int ret = -1;
	if(m_pXmlParser)
	{
		IJXmlDocument* pDoc = NULL;
		IJXmlElement* pMsgEle = m_pXmlParser->JCreateMsgDoc(pDoc, "SetServerConfig");
		if(pMsgEle)
		{
			IJXmlElement *ConfigEle = m_pXmlParser->JAppendChildEle(pMsgEle, "serverConfig", "");

			AppendChildEle(m_pXmlParser, ConfigEle, "serverIp", StringUnicodeToUtf8((LPCTSTR)ip).c_str());
			AppendChildEle(m_pXmlParser, ConfigEle, "listenPort", StringUnicodeToUtf8((LPCTSTR)listenport).c_str());
			AppendChildEle(m_pXmlParser, ConfigEle, "rtspPort", StringUnicodeToUtf8((LPCTSTR)rtspport).c_str());
		}

		IJXmlDocument* pRecDoc = NULL;
		DWORD dwMsgID = GetMsgNumber();
		m_pCommand->SendCmdMsg(dwMsgID, pDoc, pRecDoc,4000);
		ReleaseNumber(dwMsgID);

		if(NULL != pRecDoc)
		{
			IJXmlElement* pEleMsg = pRecDoc->FirstChild(XMLMSG);
			if(pEleMsg)
			{
				ret = m_pXmlParser->JGetRetCode(pEleMsg,"resultCode");
				pRecDoc->JDelete();
				return ret;
			}
		}
	}
	return ret;


}

int CManage::GetConfig(CString& ip,CString& listenport,CString& rtspport){


	int ret = -1;
	if(m_pXmlParser)
	{
		IJXmlDocument* pDoc = NULL;
		IJXmlElement* pMsgEle = m_pXmlParser->JCreateMsgDoc(pDoc, "GetServerConfig");
		if(pMsgEle)
		{
			IJXmlDocument* pRecDoc = NULL;
			DWORD dwMsgID = GetMsgNumber();
			m_pCommand->SendCmdMsg(dwMsgID, pDoc, pRecDoc,4000);
			ReleaseNumber(dwMsgID);

			if(NULL != pRecDoc)
			{
				IJXmlElement* pEleMsg = pRecDoc->FirstChild(XMLMSG);
				if(pEleMsg)
				{
					ret = m_pXmlParser->JGetRetCode(pEleMsg,"resultCode");
					if (ret != 0)
					{
						pRecDoc->JDelete();
						return ret;
					}
					else
					{
						IJXmlElement* serverEle;
						serverEle = pEleMsg->FirstChild("serverConfig");
						ip = serverEle->FirstChild("serverIp")->GetText();
						listenport = serverEle->FirstChild("listenPort")->GetText();
						rtspport = serverEle->FirstChild("rtspPort")->GetText();
						pRecDoc->JDelete();
						return ret;
					}
					pRecDoc->JDelete();
					return ret;
				}
			}
		}
	}
	return ret;


}

int CManage::DownloadData(CString& port,CString& magic,CString& fileSize)
{
	int ret = -1;
	if(m_pXmlParser)
	{
		IJXmlDocument* pDoc = NULL;
		IJXmlElement* pMsgEle = m_pXmlParser->JCreateMsgDoc(pDoc, "DownloadData");
		if(pMsgEle)
		{
			IJXmlDocument* pRecDoc = NULL;
			DWORD dwMsgID = GetMsgNumber();
			m_pCommand->SendCmdMsg(dwMsgID, pDoc, pRecDoc, 4000);
			ReleaseNumber(dwMsgID);

			if(NULL != pRecDoc)
			{
				IJXmlElement* pEleMsg = pRecDoc->FirstChild(XMLMSG);
				if(pEleMsg)
				{
					ret = m_pXmlParser->JGetRetCode(pEleMsg,"resultCode");
					if (ret!=0)
					{
						pRecDoc->JDelete();
						return ret;
					}
					else
					{	
						port = pEleMsg->FirstChild("port")->GetText();
						magic = pEleMsg->FirstChild("magic")->GetText();
						fileSize = pEleMsg->FirstChild("fileSize")->GetText();
						pRecDoc->JDelete();
						return ret;
					}
				}
				pRecDoc->JDelete();
			}
		}
	}
	return ret;

}

int CManage::UploadData(CString fileSize, CString& port, CString &magic)
{
	int ret = -1;
	if(m_pXmlParser)
	{
		IJXmlDocument* pDoc = NULL;
		IJXmlElement* pMsgEle = m_pXmlParser->JCreateMsgDoc(pDoc, "UploadData");
		if(pMsgEle)
		{
			AppendChildEle(m_pXmlParser, pMsgEle, "fileSize", StringUnicodeToUtf8((LPCTSTR)fileSize).c_str());

			IJXmlDocument* pRecDoc = NULL;
			DWORD dwMsgID = GetMsgNumber();
			m_pCommand->SendCmdMsg(dwMsgID, pDoc, pRecDoc, 4000);
			ReleaseNumber(dwMsgID);

			if(NULL != pRecDoc)
			{
				IJXmlElement* pEleMsg = pRecDoc->FirstChild(XMLMSG);
				if(pEleMsg)
				{
					ret = m_pXmlParser->JGetRetCode(pEleMsg, "resultCode");
					if (ret != 0)
					{
						pRecDoc->JDelete();
						return ret;
					}
					else
					{//
						magic = pEleMsg->FirstChild("magic")->GetText();
						port = pEleMsg->FirstChild("port")->GetText();
						pRecDoc->JDelete();
						return ret;
					}
				}
				pRecDoc->JDelete();
			}
		}
	}
	return ret;
}


int CManage::LimitBroadcastDeviceStatus(CString startDeviceId, CString endDeviceId)
{
	if(m_pXmlParser)
	{
		IJXmlDocument* pDoc = NULL;
		IJXmlElement* pMsgEle = m_pXmlParser->JCreateMsgDoc(pDoc, "LimitBroadcastDeviceStatus");
		if(pMsgEle)
		{
			AppendChildEle(m_pXmlParser, pMsgEle, "startDeviceId", StringUnicodeToUtf8((LPCTSTR)startDeviceId).c_str());
			AppendChildEle(m_pXmlParser, pMsgEle, "endDeviceId", StringUnicodeToUtf8((LPCTSTR)endDeviceId).c_str());

			DWORD dwMsgID = GetMsgNumber();
			m_pCommand->SendCmdMsg(dwMsgID, pDoc);
			ReleaseNumber(dwMsgID);
		}
	}
	return 0;
}