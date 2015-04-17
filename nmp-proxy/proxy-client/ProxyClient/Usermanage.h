/************************************************************************ 
* 文件名：    Usermanage.h 
* 文件描述：  用于与设备通过网络库交互
* 创建人：    XiaoHe, 2013年03月2日
* 版本号：    1.0 
************************************************************************/ 


#ifndef XX_H
#define XX_H 

#pragma once
#define MAX_ID_LEN				32
#define MAX_NAME_LEN			64
#define MAX_PWD_LEN				64
#define MAX_VERSION_LEN			16
#define MAX_IP_LEN				64
#define COUNT_PER_XML           25   //服务器回复时每个XML 设备的条目
#define COUNT_PER_PAG           25  //每页显示的设备数
#define TIMES                   1   //5个XML, 一次发送1个请求, 每次获取COUNT_PER_XML个数
#include "MainDevice.h"
#include "MainUser.h"



enum DeviceStyle{DVR,DVS,IPC};
class CMainDevice;
struct S_Prameter{
	CMainDevice* pMaindeviceDlg;
	CMainUser* pMainUserDlg;
};

typedef struct _prx_device_info
{
	//int result;
	
	CString device_port;					//设备端口
	CString platform_port;					//平台端口
	CString device_id;						//设备ID 无用
	CString protocol;						//目前没用到.
	
	CString type;							//设备类型
	CString pu_id;				            //PUID
	CString factoryname;					//厂家
	CString sdk_version ;					//软件版本
	CString device_ip ;						//设备IP 地址
	CString platform_ip ;					//平台IP 地址
	CString username ;						//用户名
	CString password ;						//密码
	_prx_device_info()
	{	
		device_id = "";
		protocol ="";

	}
}device_info;


class sdkversion{//版本链表节点
public:
	sdkversion(void);
	CString name;
	sdkversion *next;
};

class device//设备列表节点
{
public:
	device(void);
	CString name;
	sdkversion *pVsersion;
	device  *next;
};
class factory//厂家链表节点
{
public:
	factory(void);
	CString name;
	device *pDevice;
	factory *next;

};
class CManage
{
public:
	CString m_name;//登录用户名
	CString m_pwd;//密码
	CString  m_port;//端口
	int m_type;
	char m_IP[MAX_IP_LEN];//IP
	IJXmlParser *m_pXmlParser;
	IJDCUCommand *m_pCommand;
	factory *m_pFactory;

	
	long m_infoID;
	long m_loginID;
	long m_userlistID;
	long m_devicelistID;
	long m_heartresponseID;
	long m_AddDevBrocastID;
	long m_DelDevBrocastID;
	long m_ModifyDevBrocastID;
	long m_AddUserBrocastID;
	long m_DelUserBrocastID;
	

	int m_devicecount;//设备总数
	int m_usercount;  //用户总数

	void* m_MaindeviceDlg;

	CManage(void);
	~CManage(void);
	int Login(void *);
	int SetConfig(CString,CString,CString);
	int GetConfig(CString&,CString&,CString&);
	
	int AddUser(const CString &name,const CString &pwd,void* wPram);
	
	int DelUser(const CString &name);
	
	int DelDevice(const CString &);
	

	
	
	void GetUserList(CString name = _T(""),int offet = 0);

	/*
	 * @Date 2013-7-29
	 * @Describe GetDeivceList与GetDeviceInfo的区别, GetDeviceList是异步的, GetDeivceInfo是同步的
	 *           但是他们的响应都是同一个xml("GetDeviceInfoResponse"), 而且GetDeviceList与GetDeviceInfo
	 *           处理"GetDeviceInfoResponse"的结果都不同, GetDeviceInfo必须同步返回 
	 *           所以, 为返回异步的方式去处理同步的结果, 必须在使用同步GetDeivceInfo的时候, 反注册异步
	 *           的GetDeivcelList。
	 */
	void GetDeviceList(void*,CString factory = _T(""),int mtype = -1,CString kversion = _T(""),int offset = 0,int deviceId = -1);
	device_info GetDeviceInfo(CString deviceid=_T(""));
	
	int AddDevice(device_info device);

	int DeviceModify(device_info device);

	int UserModify(const CString &name,const CString &pwd,const CString &oldpwd=_T(""));


	const char* CString2char(CString str);

	device_info Doc2Device(IJXmlDocument*);//解析单个设备 填充设备结构体

	BOOL WINAPI fun_callback(IJDCUCommand* pJDCUCommand,
		DWORD dwMsgID, IJXmlDocument* pXmlDoc,void* pParam);

	CString GetDeviceType(CString);//从枚举到设备名称
	CString GetDeviceNum(CString);//从设备名称到枚举值
	
	void SendHeart(void);//向服务端发送心跳

	IJXmlElement* AppendChile(IJXmlParser*,IJXmlElement*,const char*,CString);
	IJXmlElement* AppendChildEle(IJXmlParser* pXmlParser, IJXmlElement* pParent, const char* pszName, const char* pszValue);

	void GetBaseInfo(void);
	void RegCallBack(void*);

	//void GetCount(void);
	void UnReg(long);

	void RegCallBackDeviceList(void* pPram);

	// 下载数据
	int DownloadData(CString& port,CString& magic,CString& fileSize);   // 返回服务器的信息

	// 上传数据
	int UploadData(CString fileSize, CString& port, CString &magic); // info, 返回的服务器上传时的端口, magic	  
	
	// 限制广播设备状态
	int LimitBroadcastDeviceStatus(CString startDeviceId, CString endDeviceId);
};


#endif