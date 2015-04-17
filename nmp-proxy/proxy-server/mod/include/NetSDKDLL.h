// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 NETSDKDLL_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// NETSDKDLL_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifndef		__NETSDKDLL_20110716_H
#define __NETSDKDLL_20110716_H

#include "common_head.h"
/*
#ifdef NETSDKDLL_EXPORTS


#if IPCCONFIGDLL
#define NETSDKDLL_API 
#else
#define NETSDKDLL_API __declspec(dllexport)
#endif

#else

#ifdef NETSDKDLL_IS_LIB	//使用lib版本时，全增加以下依赖库
	#pragma comment(lib,"libUsageEnvironment.lib")
	#pragma comment(lib,"libliveMedia.lib")
	#pragma comment(lib,"libgroupsock.lib")
	#pragma comment(lib,"libfaad.lib")
	#pragma comment(lib,"libfaac.lib")
	#pragma comment(lib,"libBasicUsageEnvironment.lib")
	#pragma comment(lib,"jthread.lib")
	#pragma comment(lib,"jrtplib.lib")
	#pragma comment(lib,"NetSDKDLL.lib")
	#pragma comment(linker, "/FORCE:MULTIPLE")
	#define NETSDKDLL_API
#else
	#define NETSDKDLL_API __declspec(dllimport)
	#pragma comment(lib, "NetSDKDLL.lib")
#endif
#endif
*/
#define MAX_ALARM_DATA 128


enum REPLAY_IPC_ACTION
{
ACTION_PLAY=0,
ACTION_PAUSE,
ACTION_RESUME,
ACTION_FAST,
ACTION_SLOW,
ACTION_SEEK,
ACTION_FRAMESKIP,
ACTION_STOP,
ACTION_CAPIMG=10,
ACTION_CHANGE_SOUND,
ACTION_RECV_DECODEPARAM,
};



#define MAX_IP_NAME_LEN 256
#define MAC_ADDRESS_LEN 256

typedef struct
{
	char  MACAddress[MAC_ADDRESS_LEN];	
	int	  dhcpEnable;
	char  IPAddress[MAX_IP_NAME_LEN];
	char  netMask[MAX_IP_NAME_LEN];
	char  gateWay[MAX_IP_NAME_LEN];
	char  DNS1[MAX_IP_NAME_LEN];
	char  DNS2[MAX_IP_NAME_LEN];
}LANConfig;

typedef struct
{
	int auth;
	int videoPort;
	int rtpoverrtsp;//added by johnnyling 20090323
	int ptzPort;
	int webPort;	
}StreamAccessConfig;

#define GROUP_NAME_MAX_LEN 32
#define ACCOUNT_STATUS_MAX_LEN 8
#define ACCOUNT_NAME_MAX_LEN 40
#define ACCOUNT_PASSWORD_MAX_LEN  40
#define MAX_ACCOUNT_COUNT 20

typedef struct
{
	char 	userName[ACCOUNT_NAME_MAX_LEN];
	char 	password[ACCOUNT_PASSWORD_MAX_LEN];
	char 	group[GROUP_NAME_MAX_LEN];
	char    status[ACCOUNT_STATUS_MAX_LEN];
}UserAccount;

typedef struct
{
	int count;
	UserAccount accounts[MAX_ACCOUNT_COUNT];
}UserConfig;

#define MAX_IPC_SERIALNUMBER	32
#define MAX_DEVICETYPE_LEN		32

typedef struct
{
	char				ipc_sn[MAX_IPC_SERIALNUMBER];
	char				deviceType[MAX_DEVICETYPE_LEN];
	UserConfig			userCfg;
	StreamAccessConfig	streamCfg;
	LANConfig			lanCfg;
}IPC_ENTRY;





//回放时数据头部信息
typedef struct _updpackhead
{
	unsigned long frame_timestamp;//此帧对应的时间戳，用于音视频同步，一帧中的不同包时间戳相同
	unsigned long keyframe_timestamp;//如果是非I帧，记录其前一I帧的timestamp，如果解码器没有收到前面那个I帧，所有非I帧丢掉丢包不解码
	unsigned short pack_seq;//包序号-65535，到最大后从开始
	unsigned short payload_size;//此包中包含有效数据的长度
	unsigned char pack_type;//0x01第一包，x10最后一包, 0x11第一包也是最后一包，x00中间包
	unsigned char frame_type;//帧类型：I帧，：非I帧
	unsigned char stream_type;//0: video, 1: audio
	unsigned char stream_index;//流ID号
	unsigned int  frame_index; //新增加字段 
}UpdPackHead;



//用到的结构定义
typedef struct _FRAMNE_INFO
{
	int bIsVideo;
	int bIsKeyFrame;
	double TimeStamp;
}FRAMNE_INFO;


typedef struct
{
	int year;
	int month;
	int day;
	int wday;
	int hour;
	int minute;
	int second;
}ALARM_TIME;

typedef struct
{
	ALARM_TIME time;	
	int code;
	int flag;
	int level;
	char data[MAX_ALARM_DATA];	
}ALARM_ITEM;



typedef struct
{
  LONG    lChannel;
  LONG    lLinkMode;
  HWND    hPlayWnd;
  char    *sMultiCastIP;
}*LPIP_NET_DVR_CLIENTINFO,IP_NET_DVR_CLIENTINFO;
//LPIP_NET_DVR_CLIENTINFO



#define			SERIALNO_LEN		48
typedef struct
{
  BYTE     sSerialNumber[SERIALNO_LEN];
  BYTE     byAlarmInPortNum;
  BYTE     byAlarmOutPortNum;
  BYTE     byDiskNum;
  BYTE     byDVRType;
  BYTE     byChanNum;
  BYTE     byStartChan;
}*LPIP_NET_DVR_DEVICEINFO,IP_NET_DVR_DEVICEINFO;


typedef struct 
{

}IP_NET_DVR_ALARMER;


#define MAX_IPADDR_LEN	64
typedef struct
{
	int  nVideoPort;
	int  bIsTcp;
	int  nVideoChannle;
	void *pUserData;
}USRE_VIDEOINFO,* LPUSRE_VIDEOINFO;


enum ERROR_CODE
{
	ERR_NOT_FIND_DEVICE=-9000002,
	ERR_OPEN_AUDIOCAPTURE_FAIL,
	ERR_START_AUDIOCAPTURE_FAIL,
	ERR_AUDIO_PARAM_ERROR,//对讲参数不一致
	ERR_AUDIO_NOT_START,//对讲未启动
	ERR_DEV_NOT_CONNECTED,
	ERR_DEV_NOT_LOGIN,
	ERR_MSGTYPE_ERROR,
	ERR_OUTOFF_MEMORY,
	ERR_INIT_SOCKET_ERROR,
	ERR_PARAM_ERROR,
	ERR_NOT_DEV_EXIST,
	ERR_START_THREADERROR,
	ERR_NOT_FIND_STREAM,
	ERR_ISUPLOADING_ERROR,
	ERR_ISDOWNLOADING_ERROR,
	ERR_IS_STARTAUDIO_ERROR,
	ERR_ISFINISH_ERROR,
	ERR_NOT_DOWNLOAD_MODE_ERROR,
	ERR_PTZCMD_ACTION_ERROR,
	ERR_LOC_FILE_ERROR,
	ERR_NOT_REPLAY_MODE_ERROR,
	ERR_PLAY_ACTION_ERROR,
	ERR_NOT_ALLOW_REPLAY_ERROR,
	ERR_MEMORY_SIZE_ERROR,
	ERR_XML_FORMAT_ERROR,
	ERR_CREATE_SOCKET_ERROR,
	ERR_SEND_MODIFYCMD_ERROR,
	ERR_NOT_STARTTALK_MODE_ERROR,
	ERR_RECORD_MEDIA_PARAM_ERROR,
	ERR_RECORD_CREATEERROR,
	ERR_RECORD_ISRECORDING,
	ERR_RECORD_FILEMAXSECONDS_ERROR,
	ERR_RECORD_ALLRECORDSECONDS_ERROR,
	ERR_RECORD_NOTRUNNING,
	ERR_RECORD_STREAMPARAM_ERROR,
	ERR_RECORD_WRITETEMPBUFFER_ERROR,
	ERR_RECORD_ISNOTRECORDSTREAM_MODE,
	ERR_RECORD_NOTINPUTSTREAM_MODE,
	ERR_RECORD_FILEPATH_ERROR,
};



enum enumNetSatateEvent
{
	EVENT_CONNECTING,   //0
	EVENT_CONNECTOK,
	EVENT_CONNECTFAILED,
	EVENT_SOCKETERROR,
	EVENT_LOGINOK,
	EVENT_LOGINFAILED,   //5
	EVENT_STARTAUDIOOK,
	EVENT_STARTAUDIOFAILED,
	EVENT_STOPAUDIOOK,
	EVENT_STOPAUDIOFAILED,
	EVENT_SENDPTZOK,    //10
	EVENT_SENDPTZFAILED,
	EVENT_SENDAUXOK,
	EVENT_SENDAUXFAILED,
	EVENT_UPLOADOK,
	EVENT_UPLOADFAILED,  //15
	EVENT_DOWNLOADOK,
	EVENT_DOWNLOADFAILED,
	EVENT_REMOVEOK,
	EVENT_REMOVEFAILED,
	EVENT_SENDPTZERROR,   //20
	EVENT_PTZPRESETINFO,  
	EVNET_PTZNOPRESETINFO,
	EVENT_PTZALARM,
	EVENT_RECVVIDEOPARAM,
	EVENT_RECVAUDIOPARAM,  //25
	EVENT_CONNECTRTSPERROR,
	EVENT_CONNECTRTSPOK,
	EVENT_RTSPTHREADEXIT,
	EVENT_URLERROR,
	EVENT_RECVVIDEOAUDIOPARAM,  //30
	EVENT_LOGIN_USERERROR,
	EVENT_LOGOUT_FINISH,			//登录线程已停止
	EVENT_LOGIN_RECONNECT,			//进行重新登录   33
	EVENT_LOGIN_HEARTBEAT_LOST,		//心跳丢失
	EVENT_STARTAUDIO_ISBUSY,		//  35
	EVENT_STARTAUDIO_PARAMERROR,
	EVENT_STARTAUDIO_AUDIODDISABLED,
	EVENT_CONNECT_RTSPSERVER_ERROR,
	EVENT_CREATE_RTSPCLIENT_ERROR,
	EVENT_GET_RTSP_CMDOPTION_ERROR,     //40
	EVENT_RTSP_AUTHERROR,
	EVNET_RECORD_FILEBEGIN,
	EVENT_RECORD_FILEEND,
	EVENT_RECORD_TASKEND,
	EVENT_RECORD_DISKFREESPACE_TOOLOW,  //45
	EVNET_RECORD_FILEBEGIN_ERROR,
	EVNET_RECORD_WRITE_FILE_ERROR,
	EVENT_RECORD_INITAVIHEAD_ERROR,
	EVENT_RECORD_MEDIA_PARAM_ERROR,	   //49
};








enum PTZ_CMD_TYPE
{
	LIGHT_PWRON=2,// 2 接通灯光电源 
	WIPER_PWRON,// 3 接通雨刷开关 
	FAN_PWRON,// 4 接通风扇开关 
	HEATER_PWRON,// 5 接通加热器开关 
	AUX_PWRON1,// 6 接通辅助设备开关 
	AUX_PWRON2,// 7 接通辅助设备开关 
	ZOOM_IN= 11,// 焦距变大(倍率变大) 
	ZOOM_OUT, //12 焦距变小(倍率变小) 
	FOCUS_NEAR, //13 焦点前调 
	FOCUS_FAR, //14 焦点后调 
	IRIS_OPEN, //15 光圈扩大 
	IRIS_CLOSE,// 16 光圈缩小 
	TILT_UP,// 21 云台上仰 
	TILT_DOWN, //22 云台下俯 
	PAN_LEFT,// 23 云台左转 
	PAN_RIGHT,// 24 云台右转 
	UP_LEFT,// 25 云台上仰和左转 
	UP_RIGHT,// 26 云台上仰和右转 
	DOWN_LEFT, //27 云台下俯和左转 
	DOWN_RIGHT,// 28 云台下俯和右转 
	PAN_AUTO,// 29 云台左右自动扫描 
	STOPACTION
};


enum PTZ_PRESET_TYPE
{
	SET_PRESET= 8 ,
	CLE_PRESET= 9,
	GOTO_PRESET= 39
};


typedef struct 
{
	char codec[256];
	int width;
	int height;
	int colorbits;
	int framerate;
	int bitrate;
	BYTE vol_data[256];
	int vol_length;
}VIDEO_PARAM;

typedef struct 
{
	char codec[256];
	int samplerate;
	int bitspersample;
	int channels;
	int framerate;
	int bitrate;
}AUDIO_PARAM;



typedef struct  __STREAM_AV_PARAM
{
	unsigned char	ProtocolName[32];	//==AV_FALG
	short  bHaveVideo;//0 表示没有视频参数
	short  bHaveAudio;//0 表示没有音频参数
	VIDEO_PARAM videoParam;//视频参数
	AUDIO_PARAM audioParam;//音频参数
}STREAM_AV_PARAM;




typedef struct
{
	int		bIsKey;
	double	timestamp;
	void    *pUserData;
}FRAME_EXTDATA,* LPFRAME_EXTDATA;

 

typedef LONG(CALLBACK *MSGCallBack)(LONG lCommand,IP_NET_DVR_ALARMER *pAlarmer,char *pAlarmInfo,DWORD BufLen,void *pUser);
typedef LONG(CALLBACK *StatusEventCallBack)(LONG lUser,LONG nStateCode,char *pResponse,void *pUser);
typedef LONG(CALLBACK *AUXResponseCallBack)(LONG lUser,LONG nType,char *pResponse,void *pUser);


typedef LONG(CALLBACK *fVoiceDataCallBack)(LONG lVoiceComHandle,char *pRecvDataBuffer,DWORD dwBufSize,BYTE byAudioFlag,LPFRAME_EXTDATA  pUser);
typedef LONG(CALLBACK *fRealDataCallBack)(LONG lRealHandle,DWORD dwDataType,BYTE *pBuffer,DWORD dwBufSize,LPFRAME_EXTDATA  pExtData);


typedef LONG(CALLBACK *fPlayActionEventCallBack)(LONG lUser,LONG nType,LONG nFlag,char * pData,void * pUser);


typedef LONG(CALLBACK *fExceptionCallBack)(DWORD dwType,LONG lUserID,LONG lHandle,void *pUser);




enum FILE_TYPE
{
	LOG_FILE,
	RECORD_FILE,
	CONFIG_FILE,
	UPDATE_FILE
};



#ifdef		IPCCONFIGDLL
#undef		IPCCONFIGDLL	
#define		IPCCONFIGDLL
#endif

NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_Init();
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_Cleanup();
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_SetAutoReconnect(LONG lUserID,int bAutoReconnect);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_Reconnect(LONG lUserID);

NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_Login(char *sDVRIP,WORD wDVRPort,char *sUserName,char *sPassword,LPIP_NET_DVR_DEVICEINFO lpDeviceInfo);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_Logout(LONG lUserID);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_LogoutAll();


//全局callback
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_SetDVRMessage(UINT nMessage,HWND hWnd);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_SetExceptionCallBack(UINT nMessage,HWND hWnd,fExceptionCallBack cbExceptionCallBack,void *pUser);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_SetAUXResponseCallBack(AUXResponseCallBack fAUXCallBack,void * pUser);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_SetStatusEventCallBack(StatusEventCallBack fStatusEventCallBack,void * pUser);




NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_GetSDKBuildVersion();
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_GetSDKVersion();
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_SetLogToFile(DWORD bLogEnable,char *strLogDir,BOOL bAutoDel);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_RealPlay(LONG lUserID,LPIP_NET_DVR_CLIENTINFO lpClientInfo,fRealDataCallBack cbRealDataCallBack,LPUSRE_VIDEOINFO pUser,BOOL bBlocked);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_RealPlayEx(char * serverip,char *user,char *pass,fRealDataCallBack cbRealDataCallBack,LPUSRE_VIDEOINFO pUser,BOOL bBlocked);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_StopRealPlay(LONG lRealHandle);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_StopAllRealPlay();
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_GetVideoParam(LONG  lRealHandle,VIDEO_PARAM *pParam);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_GetAudioParam(LONG lRealHandle,AUDIO_PARAM *pParam);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_SetRealDataCallBack(fRealDataCallBack cbRealDataCallBack,void * dwUser);


NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_SetDVRMessageCallBack(MSGCallBack fMessageCallBack,void *pUser);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_PTZControl(LONG lUser,DWORD dwPTZCommand,DWORD nTspeed,DWORD nSpeed);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_PTZPreset(LONG lUser,DWORD dwPTZPresetCmd,DWORD dwPresetIndex);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_PTZControlEx(LONG lUser,char *pXml);

NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_FormatDisk(LONG lUserID,LONG lDiskNumber);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_GetFormatProgress(LONG lFormatHandle,LONG *pCurrentFormatDisk,LONG *pCurrentDiskPos,LONG *pFormatStatic);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_Upgrade(LONG lUserID,char *sFileName);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_GetUpgradeProgress(LONG lUserID);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_GetUpgradeState(LONG lUserID);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_CloseUpgradeHandle(LONG lUserID);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_FindDVRLogFile(LONG lUserID);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_RestoreConfig(LONG lUserID);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_GetConfigFile(LONG lUserID,char *sFileName);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_SetConfigFile(LONG lUserID,char *sFileName);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_RebootDVR(LONG lUserID);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_ShutDownDVR(LONG lUserID);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_GetDVRConfig(LONG lUserID,DWORD dwCommand,LONG lChannel,LPVOID lpOutBuffer,DWORD dwOutBufferSize,LPDWORD lpBytesReturned);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_SystemControl(LONG lUserID,DWORD nCmdValue,LONG flag,char * pXml);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_SetDVRConfig(LONG lUserID,DWORD dwCommand,LONG lChannel,LPVOID pXml,DWORD dwInBufferSize);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_GetDeviceAbility(LONG lUserID);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_WriteAUXStringEx(LONG lUserID,char * pMsgType,LONG nCode,LONG flag,char * pXml);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_CreateIFrame(ULONG lUserId,int bIsSubStream);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_GetUserData(ULONG lUserId,char * pOutBuffer,int* nInOutLen);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_SetUserData(ULONG lUserId,char * pBuffer,int len);


//文件上传下载
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_GetFileByName(LONG lUserID,LONG nFileType,char *sDVRFileName,char *saveDir);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_StopGetFile(LONG lFileHandle);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_GetDownloadState(LONG lFileHandle);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_GetDownloadPos(LONG lFileHandle);

//ipc文件回放
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_ControlPlay(LONG lUserID,LONG Action,LONG param);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_GetReplayAblity(LONG lUserID);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_PlayDeviceFile(LONG lUserID,char * filenme);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_SetReplayDataCallBack(fRealDataCallBack cbReplayDataCallBack,void * dwUser);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_SetPlayActionEventCallBack(fPlayActionEventCallBack cbActionCallback,void * dwUser);


//设备搜索功能
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_StartSearchIPC();
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_StopSearchIPC();
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_GetSearchIPCCount();
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_GetIPCInfo(LONG index, IPC_ENTRY * pIPCInfo);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_ModifyIPC(LONG index, IPC_ENTRY * pIPCInfo);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_GetIPCInfoXML(LONG index, char * pXMLInfo,int maxLen);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_ModifyIPCXML(LONG index, const char * strXML);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_GetOneIPAddress(char * strResult,int nSize);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_GetNetworkParam(ULONG nParamIndex, char * strResult,int nSize);




//广播音频相关(对讲)
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_StartTalk(int audiotype, int samplerate, int bitspersample, int channels);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_StopTalk();
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_AddTalk(LONG lUserID);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_RemoveTalk(LONG lUserID);



NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_StartVoiceCom(LONG lUserID,DWORD dwVoiceChan,BOOL bNeedCBNoEncData,fVoiceDataCallBack cbVoiceDataCallBack,void *pUser);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_SetVoiceComClientVolume(LONG lVoiceComHandle,WORD wVolume);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_StopVoiceCom(LONG lVoiceComHandle);



//录像相关
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_StartRecord(LONG lRealHandle,const char * filePath,int nFileMaxSeconds,int nAllRecordMaxSeconds);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_StopRecord(LONG lRealHandle);

NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_GetLastErrorCode(int nType);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_StartRecordStream(STREAM_AV_PARAM * pAvParam,const char * filePath,int nFileMaxSeconds,int nAllRecordMaxSeconds);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_InputRecordStream(LONG lRealHandle,const void * pBuffer,int nSize,int isVideo,int iskey,double timestamp);
NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_StopRecordStream(LONG lRealHandle);

NETSDKDLL_API		LONG		__stdcall		IP_NET_DVR_wzwTest();

#endif


