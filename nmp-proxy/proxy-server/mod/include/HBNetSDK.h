////////////////////////////////////////////////////////////////////////////////
// Copyright (C), 2009-2012, Beijing Hanbang Technology, Inc.
////////////////////////////////////////////////////////////////////////////////
//
// File Name:   NetSDK.h
// Author:      
// Version:     1.00
// Date:        
// Description: Header file of NetSDK.so.
//              Linux Net SDK
// History:
//              2013/4/15: Add Active mode Interface
//
////////////////////////////////////////////////////////////////////////////////

#ifndef HB_NETSDK_H
#define HB_NETSDK_H

typedef int             BOOL;
typedef short           SHORT;
typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned short  USHORT;
typedef unsigned long   DWORD;
typedef unsigned int    UINT;
typedef void*           HWND;
typedef void*           LPVOID;
typedef DWORD*          LPDWORD;
typedef long long       LONGLONG;
typedef long            LONG;
typedef unsigned long   ULONG;
typedef ULONG           *PULONG;
typedef unsigned long long  ULONGLONG;

#define CALLBACK

//////////////////////////////////////////////////////////////////////////
// 错误码
#define HB_NET_NOERROR                  0           // 没有错误
#define HB_NET_PASSWORD_ERROR           -1          // 用户名密码错误
#define HB_NET_NOENOUGHPRI              -2          // 权限不足
#define HB_NET_NOINIT                   -3          // 没有初始化
#define HB_NET_CHANNEL_ERROR            -4          // 通道号错误
#define HB_NET_OVER_MAXLINK             -5          // 连接到DVR的客户端个数超过最大
#define HB_NET_VERSIONNOMATCH           -6          // 版本不匹配
#define HB_NET_NETWORK_FAIL_CONNECT     -7          // 连接服务器失败
#define HB_NET_NETWORK_SEND_ERROR       -8          // 向服务器发送失败
#define HB_NET_NETWORK_RECV_ERROR       -9          // 从服务器接收数据失败
#define HB_NET_NETWORK_RECV_TIMEOUT     -10         // 从服务器接收数据超时
#define HB_NET_NETWORK_ERRORDATA        -11         // 传送的数据有误
#define HB_NET_ORDER_ERROR              -12         // 调用次序错误
#define HB_NET_OPERNOPERMIT             -13         // 无此权限
#define HB_NET_COMMANDTIMEOUT           -14         // DVR命令执行超时
#define HB_NET_ERRORSERIALPORT          -15         // 串口号错误
#define HB_NET_ERRORALARMPORT           -16         // 报警端口错误
#define HB_NET_PARAMETER_ERROR          -17         // 参数错误
#define HB_NET_CHAN_EXCEPTION           -18         // 服务器通道处于错误状态
#define HB_NET_NODISK                   -19         // 没有硬盘
#define HB_NET_ERRORDISKNUM             -20         // 硬盘号错误
#define HB_NET_DISK_FULL                -21         // 服务器硬盘满
#define HB_NET_DISK_ERROR               -22         // 服务器硬盘出错
#define HB_NET_NOSUPPORT                -23         // 服务器不支持
#define HB_NET_BUSY                     -24         // 服务器忙
#define HB_NET_MODIFY_FAIL              -25         // 服务器修改不成功
#define HB_NET_PASSWORD_FORMAT_ERROR    -26         // 密码输入格式不正确
#define HB_NET_DISK_FORMATING           -27         // 硬盘正在格式化，不能启动操作
#define HB_NET_DVRNORESOURCE            -28         // DVR资源不足
#define	HB_NET_DVROPRATEFAILED          -29         // DVR操作失败
#define HB_NET_OPENHOSTSOUND_FAIL       -30         // 打开PC声音失败
#define HB_NET_DVRVOICEOPENED           -31         // 服务器语音对讲被占用
#define	HB_NET_TIMEINPUTERROR           -32         // 时间输入不正确
#define	HB_NET_NOSPECFILE               -33         // 回放时服务器没有指定的文件
#define HB_NET_CREATEFILE_ERROR         -34         // 创建文件出错
#define	HB_NET_FILEOPENFAIL             -35         // 打开文件出错
#define	HB_NET_OPERNOTFINISH            -36         // 上次的操作还没有完成
#define	HB_NET_GETPLAYTIMEFAIL          -37         // 获取当前播放的时间出错
#define	HB_NET_PLAYFAIL                 -38         // 播放出错
#define HB_NET_FILEFORMAT_ERROR         -39         // 文件格式不正确
#define HB_NET_DIR_ERROR                -40         // 路径错误
#define HB_NET_ALLOC_RESOUCE_ERROR      -41         // 资源分配错误
#define HB_NET_AUDIO_MODE_ERROR         -42         // 声卡模式错误
#define HB_NET_NOENOUGH_BUF             -43         // 缓冲区太小
#define HB_NET_CREATESOCKET_ERROR       -44         // 创建SOCKET出错
#define HB_NET_SETSOCKET_ERROR          -45         // 设置SOCKET出错
#define HB_NET_MAX_NUM                  -46         // 个数达到最大
#define HB_NET_USERNOTEXIST             -47         // 用户不存在
#define HB_NET_WRITEFLASHERROR          -48         // 写FLASH出错
#define HB_NET_UPGRADEFAIL              -49         // DVR升级失败
#define HB_NET_CARDHAVEINIT             -50         // 解码卡已经初始化过
#define HB_NET_PLAYERFAILED             -51         // 播放器中错误
#define HB_NET_MAX_USERNUM              -52         // 用户数达到最大
#define HB_NET_GETLOCALIPANDMACFAIL     -53         // 获得客户端的IP地址或物理地址失败
#define HB_NET_NOENCODEING              -54         // 该通道没有编码
#define HB_NET_IPMISMATCH               -55         // IP地址不匹配
#define HB_NET_MACMISMATCH              -56         // MAC地址不匹配
#define HB_NET_UPGRADELANGMISMATCH      -57         // 升级文件语言不匹配
#define HB_NET_USERISALIVE              -58         // 用户已登录
#define HB_NET_UNKNOWNERROR             -59         // 未知错误

#define HB_NET_IPERR                    -101        // IP地址不匹配
#define HB_NET_MACERR                   -102        // MAC地址不匹配
#define HB_NET_PSWERR                   -103        // 密码不匹配
#define HB_NET_USERERR                  -104        // 用户名不匹配
#define HB_NET_USERISFULL               -105        // 主机用户满
#define NO_PERMISSION                   0xf0        // 用户没有权限




#define HB_REC_MANUAL                   1
#define HB_REC_SCHEDULE                 2
#define HB_REC_MOTION                   4
#define HB_REC_ALARM                    8
#define HB_REC_ALL                      0xff

// 查找文件和日志函数返回值
#define HB_NET_FILE_SUCCESS             1000        // 获得文件信息
#define HB_NET_FILE_NOFIND              1001        // 没有文件
#define HB_NET_ISFINDING                1002        // 正在查找文件
#define HB_NET_NOMOREFILE               1003        // 查找文件时没有更多的文件
#define HB_NET_FILE_EXCEPTION           1004        // 查找文件时异常

#pragma pack(8)

//////////////////////////////////////////////////////////////////////////
// 初始化/释放
BOOL HB_NET_Init();

BOOL HB_NET_Cleanup();

DWORD HB_NET_GetLastError();

//////////////////////////////////////////////////////////////////////////
// 登录/登出
BOOL HB_NET_Logout(LONG lUserID);

#define HB_SERIALNO_LEN             48
#define HB_NAME_LEN                 32
#define HB_PWD_LEN                  16
#define HB_MAX_CHANNUM              128 

// 协议版本
#define HB_NET_PROTOCOL_V1          0x10
#define HB_NET_PROTOCOL_V2          0x20

typedef struct ST_HB_NET_DEVICEINFO 
{
    DWORD   dwSize;
    BYTE    sSerialNumber[HB_SERIALNO_LEN];         // 保留
    char    sDvrName[HB_NAME_LEN];				    // 主机名
    char    sChanName[HB_MAX_CHANNUM][HB_NAME_LEN]; // 通道名称
    BYTE    byAlarmInPortNum;					    // DVR报警输入个数
    BYTE    byAlarmOutPortNum;					    // DVR报警输出个数
    BYTE    byDiskNum;		 					    // DVR 硬盘个数
    BYTE    byProtocol;							    // 协议版本: 
                                                    // 0x20, 协议二;
                                                    // 小于0x20, 协议一;
    BYTE    byChanNum;							    // DVR 通道个数
    BYTE    byEncodeType;						    // 主机编码格式:
                                                    // 1, ANSI,中文GB2312; 
                                                    // 2, UTF-8
    BYTE    byRes[10];                              // 保留
}HB_NET_DEVICEINFO, *LPHB_NET_DEVICEINFO;

LONG HB_NET_Login(char* sDevIP,
                  WORD wDevPort,
                  char* sUserName,
                  char* sPassword, 
                  LPHB_NET_DEVICEINFO lpDeviceInfo);

//////////////////////////////////////////////////////////////////////////
// 预览模块
#define HB_IP_LEN                   16

typedef enum
{
    HB_NET_TCP = 0,                                 // TCP
    HB_NET_UDP,                                     // UDP
    HB_NET_MULTICAST,                               // 多播
}HB_NET_LINKMODE_E;

typedef enum
{
    HB_NET_STREAMDATA = 2,	                        // 汉邦格式流数据
    HB_NET_BACKUPEND = 3,	                        // 备份结束
    HB_NET_SYSHEAD = 64,	                        // 系统头数据
}HB_NET_DATATYPE_E;


typedef struct ST_HB_NET_STREAMCALLBACKDATA 
{
    DWORD   dwSize;
    HB_NET_DATATYPE_E   dwDataType;
    char*   pFrame;                                 // 指向数据指针
    DWORD   dwDataSize;                             // 数据长度
    void*   pContext;                               // 用户上下文
}HB_NET_STREAMCALLBACKDATA, *LPHB_NET_STREAMCALLBACKDATA;

// 使用结构体参数
typedef void (CALLBACK* PHB_NET_STREAMDATA_PROC)(long lRealHandle,
                                                 LPHB_NET_STREAMCALLBACKDATA pStreamData);

typedef struct ST_HB_NET_CLIENTINFO
{
    DWORD                   dwSize;
    LONG                    lChannel;               // 通道号
    LONG                    lStreamType;            // 码流，主0，子1，子2，...
    HB_NET_LINKMODE_E       lLinkMode;              // 连接模式
    char                    sMultiCastIP[HB_IP_LEN];
    PHB_NET_STREAMDATA_PROC pfnCallback;
    void*                   pContext;
    DWORD                   dwRes[2];
}HB_NET_CLIENTINFO, *LPHB_NET_CLIENTINFO;

LONG HB_NET_RealPlay(LONG lUserID, LPHB_NET_CLIENTINFO lpClientInfo);

BOOL HB_NET_StopRealPlay(LONG lRealHandle);

// lStreamType: 码流类型; 0, 主码流; 1, 子码流1;...
BOOL HB_NET_MakeKeyFrame(LONG lUserID, LONG lChannel, LONG lStreamType);

// 显示
// typedef void (CALLBACK* PH_NET_DC_RENDER_PROC)(long lRealHandle,
//                                                HDC hDc,
//                                                void* pContext);
// 
// BOOL HB_NET_RigisterDrawFun(long lRealHandle, 
//                             PH_NET_DC_RENDER_PROC pfnDCRender, 
//                             void* pContext);

// 声音控制
BOOL HB_NET_OpenSound(LONG lRealHandle);

BOOL HB_NET_CloseSound(LONG lRealHandle);

BOOL HB_NET_SetVolume(LONG lRealHandle, WORD wVolume);

BOOL HB_NET_GetVolume(LONG lRealHandle, WORD* pVolume);

// 抓图
// BOOL HB_NET_CapturePicture(LONG lRealHandle, char* sPicFileName);   // bmp

// 云台控制


// 云台控制CMD，命令号
#define TILT_UP			    0x0001000c	            // 云台以SS的速度上仰
#define TILT_DOWN		    0x0001000d	            // 云台以SS的速度下俯 
#define PAN_LEFT		    0x0001000e 	            // 云台以SS的速度左转 
#define PAN_RIGHT		    0x0001000f	            // 云台以SS的速度右转 

#define FOCUS_NEAR		    0x00010014              // 焦点以速度SS前调 
#define FOCUS_FAR		    0x00010015 	            // 焦点以速度SS后调 

#define ZOOM_IN			    0x00010016	            // 焦距以速度SS变大(倍率变大) 
#define ZOOM_OUT		    0x00010017 	            // 焦距以速度SS变小(倍率变小) 

#define IRIS_OPEN		    0x00010018 	            // 光圈以速度SS扩大 
#define IRIS_CLOSE		    0x00010019	            // 光圈以速度SS缩小 

#define PAN_AUTO		    0x0001001c 	            // 云台以SS的速度左右自动扫描 

#define LIGHT_PWRON		    0x00010024	            // 接通灯光电源 
#define WIPER_PWRON		    0x00010025	            // 接通雨刷开关 
#define TM_COM_GUI_BRUSH    0x0001002e              // 雨刷

#define SET_PRESET		    0x0001001a 	            // 设置预置点 
#define CLE_PRESET		    18	                    // 清除预置点 
#define GOTO_PRESET		    0x0001001b 	            // 快球转到预置点 

#define ALL_STOP            0x00010028              // 云台命令发送后需要再发送本命令以停止前一命令

typedef struct ST_HB_NET_PTZCTRL
{
    DWORD   dwSize;
    DWORD   dwChannel;                              // 云台号(对应通道号)
    DWORD   dwPTZCmd;                               // 控制命令见以上宏定义
    DWORD   dwIndex;                                // 预置点号(受球机有效值范围影响)[0,255]
    DWORD   dwSpeed;                                // 控制速度[0,255], 0,速度最慢
    DWORD   dwRes;
}HB_NET_PTZCTRL, *LPHB_NET_PTZCTRL;

BOOL HB_NET_PTZControl(LONG lUserID, LPHB_NET_PTZCTRL lpCtrl);

typedef struct ST_HB_NET_PTZ_TRANS
{
    DWORD   dwSize;
    DWORD   dwChannel;                              // 云台号
    char*   pPTZCodeBuf;                            // 控制码指针
    DWORD   dwBufSize;                              // 控制码有效长度
    DWORD   dwRes;
}HB_NET_PTZ_TRANS, *LPHB_NET_PTZ_TRANS;

// 透明云台
BOOL HB_NET_TransPTZ(LONG lUserID, LPHB_NET_PTZ_TRANS lpTrans);

// 自定义云台协议
#define MAX_KEYNUM                  19
#define MAX_KEYSIZE                 24

typedef struct ST_HB_NET_DECODERCUSTOMIZE
{
    DWORD   dwSize;
    BYTE    CommandKey[MAX_KEYNUM][MAX_KEYSIZE];
    BYTE    CheckSum;                               // 效验码位置
    BYTE    PortNum;                                // 地址码位置
    BYTE    PreSet;                                 // 预制点位置
    BYTE    CheckSumMode;                           // 效验码计算模式
    BYTE    KeyLen;                                 // 码长度
    BYTE    KeyNum;                                 // 发码数
    BYTE    byRes[2];                          // 保留
    DWORD   dwRes;
}HB_NET_DECODERCUSTOMIZE,*LPHB_NET_DECODERCUSTOMIZE;

BOOL HB_NET_SetUserPTZProtocol(LONG lUserID, LPHB_NET_DECODERCUSTOMIZE lpPTZProtocol);

BOOL HB_NET_GetUserPTZProtocol(LONG lUserID, LPHB_NET_DECODERCUSTOMIZE lpPTZProtocol);

// 获取云台协议类型列表
#define MAX_PTZ_NUM                 100
#define MAX_PTZ_NAMELEN             10

typedef struct ST_HB_NET_PTZTYPE
{
    DWORD   dwSize;
    BYTE    byType;                                 // 0-本地 1-前端设备（NVR有效）
    BYTE    byChannel;                              // 通道号(NVR有效)
    BYTE    byRes[6];
    DWORD   dwPTZnum;                                 // 云台协议总数
    char    ptztype[MAX_PTZ_NUM][MAX_PTZ_NAMELEN];  // 云台协议名列表
    DWORD   dwRes;
}HB_NET_PTZTYPE,*LPHB_NET_PTZTYPE;

BOOL HB_NET_GetPTZProtocolList(LONG lUserID, LPHB_NET_PTZTYPE lpPTZType);

// 实时数据流回调和录像
typedef struct ST_HB_NET_DATACALLBACKPARAM
{
    DWORD dwSize;
    PHB_NET_STREAMDATA_PROC pfnCallback;
    void* pContext;
    DWORD dwRes;
}HB_NET_DATACALLBACKPARAM, *LPHB_NET_DATACALLBACKPARAM;

BOOL HB_NET_SetStreamDataCallback(LONG lStreamHandle, LPHB_NET_DATACALLBACKPARAM lpCallbackParam);

// 实时流强制I帧
typedef struct ST_HB_NET_STREAMDATA_SAVE_PARAM
{
    DWORD   dwSize;
    DWORD   dwTransType;                            // 保留
    char*   pFileName;                              // 存储目标文件名
    DWORD   dwRes;
}HB_NET_STREAMDATA_SAVE_PARAM, *LPHB_NET_STREAMDATA_SAVE_PARAM;

BOOL HB_NET_SaveStreamData(LONG lStreamHandle, LPHB_NET_STREAMDATA_SAVE_PARAM lpParam);

BOOL HB_NET_StopSaveStreamData(LONG lStreamHandle);

//////////////////////////////////////////////////////////////////////////
// 回放和下载模块
#define HB_MAX_CARD_NUMLEN             32

typedef struct ST_HB_NET_TIME
{
    DWORD   dwYear;
    DWORD   dwMonth;
    DWORD   dwDay;
    DWORD   dwHour;
    DWORD   dwMinute;
    DWORD   dwSecond;
    DWORD   dwMillisecond ;
    DWORD   dwRes;
}HB_NET_TIME, *LPHB_NET_TIME;

typedef enum
{
    HB_NET_RECMANUAL = 1,                           // 手动录像
    HB_NET_RECSCHEDULE = 2,                         // 定时录像
    HB_NET_RECMOTION = 4,                           // 移动侦测录像
    HB_NET_RECALARM = 8,                            // 报警录像
    HB_NET_REC_ALL = 0xff,                          // 所有录像
}HB_NET_RECTYPE_E;

typedef struct ST_HB_NET_FILEFINDCOND
{
    DWORD           dwSize;
    DWORD           dwChannel;                      // 通道名
    HB_NET_RECTYPE_E dwFileType;                    // 文件类型
    DWORD           dwUseCardNO;                    // 是否使用卡号
    HB_NET_TIME     struStartTime;                  // 开始时间
    HB_NET_TIME     struStopTime;                   // 结束时间
    BYTE            sCardNumber[HB_MAX_CARD_NUMLEN]; // 卡号
    DWORD           dwRes;
}HB_NET_FILEFINDCOND, *LPHB_NET_FILEFINDCOND;

typedef struct ST_HB_NET_FINDDATA
{
    DWORD           dwSize;
    HB_NET_TIME     struStartTime;                  // 文件开始时间
    HB_NET_TIME     struStopTime;                   // 文件结束时间
    DWORD           dwFileSize;                     // 文件大小
    DWORD           dwChannel;                      // 通道号
    DWORD           dwFileType;                     // 文件类型
    DWORD           dwRes;
}HB_NET_FINDDATA, *LPHB_NET_FINDDATA;

// 查找
LONG HB_NET_FindFile(LONG lUserID, LPHB_NET_FILEFINDCOND lpFindCond);

LONG HB_NET_FindNextFile(LONG lFindHandle, LPHB_NET_FINDDATA lpFindData);

BOOL HB_NET_FindFileClose(LONG lFindHandle);

// 回放
// 当结束时间为0时，从开始时间进行连续回放
// 结束时间大于开始时间，回放时间段内录像
typedef struct ST_HB_NET_PLAYBACKCOND
{
    DWORD                   dwSize;
    DWORD                   dwChannel;              // 通道号
    HB_NET_TIME             struStartTime;          // 请求开始时间
    HB_NET_TIME             struStopTime;           // 请求结束时间
    PHB_NET_STREAMDATA_PROC pfnDataProc;            // 设置数据回调
    void*                   pContext;
    DWORD                   dwType;                 // 文件类型
    DWORD                   dwRes;
}HB_NET_PLAYBACKCOND, *LPHB_NET_PLAYBACKCOND;

// by time
// by name ---->将文件名解析成time/channel
long HB_NET_PlayBack(long lUserID, LPHB_NET_PLAYBACKCOND lpPlayBackCond);

// setcallback
// save
// stop save

//播放控制命令宏定义 HB_NET_PlayBackControl,HB_NET_PlayControlLocDisplay,HB_NET_DecPlayBackCtrl的宏定义
#define HB_NET_PLAYSTART		    1               // 开始播放
#define HB_NET_PLAYSTOP		        2               // 停止播放
#define HB_NET_PLAYPAUSE		    3               // 暂停播放
#define HB_NET_PLAYRESTART		    4               // 恢复播放
#define HB_NET_PLAYFAST		        5               // 快放
#define HB_NET_PLAYSLOW		        6               // 慢放
#define HB_NET_PLAYNORMAL		    7               // 正常速度
#define HB_NET_PLAYFRAME		    8               // 单帧放
#define HB_NET_PLAYSTARTAUDIO		9               // 打开声音
#define HB_NET_PLAYSTOPAUDIO		10              // 关闭声音
//#define HB_NET_PLAYAUDIOVOLUME		11              // 调节音量
//#define HB_NET_PLAYGETPOS		    13              // 获取文件回放的进度   当等于100时候表示结束
#define HB_NET_PLAYBACK		        21              // 后退
#define HB_NET_PLAYBYSLIDER	        22              // 按进度条播放

typedef struct ST_HB_NET_PLAYBACKCTRL
{
    DWORD   dwSize;
    DWORD   dwCmd;
    char*   pInBuffer;
    DWORD   dwInLen;
    char*   pOutBuffer;
    DWORD*  lpOutLen;
    DWORD   dwRes;
}HB_NET_PLAYBACKCTRL, *LPHB_NET_PLAYBACKCTRL;

BOOL HB_NET_PlayBackControl(long lPlayHandle, LPHB_NET_PLAYBACKCTRL lpPlayBackCtrl);

BOOL HB_NET_StopPlayBack(long lPlayHandle);

// 下载
// 按文件名下载实际是将文件名解析为时间/通道/类型，所以取消
typedef struct ST_HB_NET_FILEGETCOND
{
    DWORD           dwSize;
    DWORD           dwChannel;              // 通道号
    HB_NET_RECTYPE_E    dwFileType;             // 文件类型
    HB_NET_TIME     struStartTime;          // 下载时间段开始时间
    HB_NET_TIME     struStopTime;           // 结束时间
    PHB_NET_STREAMDATA_PROC pfnDataProc;
    void*                   dwContext;
    char*           pSaveFilePath;          // 保存路径，按服务器文件保存
                                            // 即，分文件存储
    DWORD           dwRes;
}HB_NET_FILEGETCOND, *LPHB_NET_FILEGETCOND;

long HB_NET_GetFile(long lUserID, LPHB_NET_FILEGETCOND pGetFile);

// callback
// save
// stop save

LONGLONG HB_NET_GetDownloadTotalSize(long lFileHandle);

LONGLONG HB_NET_GetDownloadBytes(long lFileHandle);

// BOOL HB_NET_PlayBackControl(long lPlayHandle, DWORD dwControlCode, DWORD dwInValue, DWORD *lpOutValue);

BOOL HB_NET_StopGetFile(long lFileHandle);

//////////////////////////////////////////////////////////////////////////
// 参数配置模块

// CMD
#define HB_NET_GET_VEFF             0x04
#define HB_NET_SET_VEFF             0x05

typedef struct ST_HB_NET_VIDEOPARAM
{
    DWORD	dwBrightValue;		                    // 亮度  1-127
    DWORD	dwContrastValue;	                    // 对比度1-127
    DWORD	dwSaturationValue;	                    // 饱和度1-127
    DWORD	dwHueValue;			                    // 色度	1-127
}HB_NET_VIDEOPARAM, *LPHB_NET_VIDEOPARAM;

typedef struct ST_HB_NET_SCHEDULE_VIDEOPARAM
{
    BYTE    byStartMin;                             // 开始分
    BYTE    byStartHour;                            // 开始时
    BYTE    byEndMin;                               // 结束分
    BYTE    byEndHour;                              // 结束时
    DWORD   dwRes;
    HB_NET_VIDEOPARAM VideoParam;
}HB_NET_SCHEDULE_VIDEOPARAM, *LPHB_NET_SCHEDULE_VIDEOPARAM;

typedef struct  ST_HB_NET_VIDEOEFFECT
{
    DWORD   dwSize;
    DWORD	dwChannel;                              // 通道号
    HB_NET_SCHEDULE_VIDEOPARAM Schedule_VideoParam[2];  // 一天包含2个时间段
    HB_NET_VIDEOPARAM Default_VideoParam;           // 不在时间段内就使用默认
    DWORD   dwRes[2];
}HB_NET_VIDEOEFFECT,*LPHB_NET_VIDEOEFFECT; 

//-----------------------------------------------------------------------------
// 兼容NVR报警输出状态
#define HB_NET_GET_ALARM            0x19            // NVR 0xCF 
#define HB_NET_SET_ALARM            0x1A            // NVR 0xD1

#define HB_MAX_ALARMOUT             128

// dwStatus，仅在设置时有效
// 设置时，
// dwOpType为0xFF，则对所有报警输出强制执行dwStatus
// dwOpType为1，则dwChannel有效，对前端设备进行设置
// dwOpType为0，仅对当前设备进行设置
// byOutput中，为0，状态不变；为1，强制执行dwStatus
// DVR/IPC不对dwOpType做区分，NVR会对该字段做区分
//
// 获取时，需对dwOpType/dwChannel字段进行赋值
// dwOpType为1时dwChannel有效
// byOutput中，为1，有报警输出；为0，无报警输出；
typedef struct ST_HB_NET_ALARMOUTSTAT
{
    DWORD   dwSize;
    DWORD   dwStat;                                 // 0-不报警 1-报警
    DWORD   dwOpType;                               // 0-本地 1-前端
    DWORD   dwChannel;                              // 前端设备,dwOpType为1时有效
    BYTE    byOutput[HB_MAX_ALARMOUT];                 // 对应报警输出
    DWORD   dwRes;
}HB_NET_ALARMOUTSTAT, *LPHB_NET_ALARMOUTSTAT;

//-----------------------------------------------------------------------------
#define HB_NET_GET_MANURECORD_STAT  0x21            // 获取手动录像状态
#define HB_NET_SET_MANURECORD_STAT  0x20            // 设置手动录像状态

typedef struct ST_HB_NET_MANURECORD
{
    DWORD   dwSize;
    BYTE    byChannel[HB_MAX_CHANNUM];              // 0停止录像 1录像
    DWORD   dwRes;
}HB_NET_MANURECORD, *LPHB_NET_MANURECORD;

//-----------------------------------------------------------------------------
// 兼容NVR工作状态获取
#define HB_NET_GET_WORK_STAT        0x22            // 获取主机工作状态

typedef struct ST_HB_NET_DISKSTAT
{
    DWORD   dwVolume;                               // 硬盘总容量(MB)
    DWORD   dwFreeSpace;                            // 剩余容量(MB)
    DWORD   dwStat;                                 // 硬盘状态(dwVolume非0时有效)
}HB_NET_DISKSTAT, *LPHB_NET_DISKSTAT;

typedef struct ST_HB_NET_CHANNELSTAT
{
    BYTE    byRecordStat;                           // 通道是否在录像,0不录像,1录像
    BYTE    bySignalStat;                           // 连接的信号状态,0在线,1信号丢失,2不在线
    BYTE    byHardWareStat;                         // 保留
    BYTE    byLinkNum;                              // 客户端连接的个数
    DWORD   dwBitRate;                              // 实际码流
}HB_NET_CHANNELSTAT, *LPHB_NET_CHANNELSTAT;

#define HB_MAX_HARDDISK             16
#define HB_MAX_ALARMIN              128

typedef struct ST_HB_NET_WORKSTAT
{
    DWORD   dwSize;
    HB_NET_DISKSTAT     struHardDiskStat[HB_MAX_HARDDISK];  // 硬盘状态
    HB_NET_CHANNELSTAT  struChanStat[HB_MAX_CHANNUM];       // 通道状态
    BYTE    byAlarmInStat[HB_MAX_ALARMIN];                  // 本地报警输入端口状态
    BYTE    byAlarmOutStat[HB_MAX_ALARMOUT];                // 本地报警输出端口状态
    DWORD*  pBytesRet;
    DWORD   dwRes;
}HB_NET_WORKSTAT, *LPHB_NET_WORKSTAT;

//-----------------------------------------------------------------------------
// 设备信息---支持NVR
// 设置时，仅可设置sDevName/dwRecycleRecord
#define HB_NET_GET_DEVICECFG        0x26
#define HB_NET_SET_DEVICECFG        0x27

#define HB_SOFTWAREVER_LEN          64
#define HB_SOFTBUILDDATE_LEN        32
#define HB_PANELVER_LEN             32
#define HB_HARDWAREVER_LEN          32

typedef struct ST_HB_NET_DEVICECFG
{
    DWORD   dwSize;
    DWORD   dwDevID;                                // 保留
    char    sDevName[HB_NAME_LEN];                  // 设备名称
    char    sSerialNumber[HB_SERIALNO_LEN];         // 序列号
    char    sSoftwareVer[HB_SOFTWAREVER_LEN];       // 软件版本号
    char    sSoftwareBuildDate[HB_SOFTBUILDDATE_LEN];   //软件生成日期
    char    sPanelVer[HB_PANELVER_LEN];             // 前面板版本
    char    sHardwareVer[HB_HARDWAREVER_LEN];       // 硬件版本号
    DWORD   dwRecycleRecord;                        // 0循环覆盖, 1提示覆盖
    DWORD   dwDSPSoftwareVer;                       // DSP软件版本
    DWORD   dwDevType;                              // DVR类型
    BYTE    byAlarmInPortNum;                       // 报警输入个数,Nvr只取本地
    BYTE    byAlarmOutPortNum;                      // 报警输出
    BYTE    byDiskNum;                              // 硬盘个数
    BYTE    byChanNum;                              // 通道个数(0,128]
    BYTE    byRS232Num;
    BYTE    byRS485Num;
    BYTE    byNetworkPortNum;
    BYTE    byDiskCtrlNum;
    BYTE    byStartChan;                            //
    BYTE    byDecodeChans;
    BYTE    byVGANum;
    BYTE    byUSBNum;
    DWORD   dwRes;
}HB_NET_DEVICECFG, *LPHB_NET_DEVICECFG;

//-----------------------------------------------------------------------------
// 网络参数
#define HB_NET_GET_NETCFG       0x28
#define HB_NET_SET_NETCFG       0x29

#define HB_NET_MACADDR_LEN      6
#define HB_NET_ETHERNET_NUM     2
#define HB_NET_PPPOEUSER_LEN    32
#define HB_NET_PPPOEPWD_LEN     16

#define HB_NET_PATHNAME_LEN 	128

typedef struct ST_HB_NET_ETHERNET
{
    DWORD   dwDevIP;                                // 设备IP
    DWORD   dwDevIPMask;                            // 掩码
    DWORD   dwNetInterface;                         // 网络接口
                                                    // 1-10MBase-T ...
    WORD   wDevPort;                                // 端口
    BYTE    byMACAddr[HB_NET_MACADDR_LEN];
    DWORD   dwRes;
}HB_NET_ETHERNET, *LPHB_NET_ETHERNET;

// IP/GateWay/Mask为0或0xFFFFFFFF为非法
// 此处pppoe与pppoe设置命令中pppoe无区别
typedef struct ST_HB_NET_NETCFG
{
    DWORD   dwSize;
    DWORD   dwManageHostIP;                         // 远程管理主机地址
    DWORD   dwManageHostPort;                       // 远程管理主机端口
    DWORD   dwDNSIP;                                // DNS服务器地址
    DWORD   dwMultiCastIP;                          // 多播组地址
    DWORD   dwGateWayIP;                            // 网关地址
    DWORD   dwPPPOE;                                // 是否启用pppoe,0不启用,1启用
                                                    // 获取时为pppoe状态
    DWORD   dwPPPOEIP;                              // 只读
    char    sPPPOEUser[HB_NET_PPPOEUSER_LEN];
    char    sPPPOEPwd[HB_NET_PPPOEPWD_LEN];
    HB_NET_ETHERNET struEtherNet[HB_NET_ETHERNET_NUM]; // 以太网口
                                                    // (T/IPC仅使用了struEtherNet[0]
    DWORD   dwHttpPort;
    DWORD   dwRes;
}HB_NET_NETCFG, *LPHB_NET_NETCFG;

//-----------------------------------------------------------------------------
// 通道参数 ---NVR不支持
#define HB_NET_GET_PICCFG       0x2A
#define HB_NET_SET_PICCFG       0x2B

#define HB_MAX_DAYS             8
#define HB_MAX_TIMESEGMENT      2
#define HB_MAX_SHELTERNUM       4

typedef struct ST_HB_NET_HANDLEEXCEPTION
{
    DWORD   dwHandleType;       // 按位 2-声音报警 5-监视器最大化 6-邮件上传
    DWORD   dwRes;
    BYTE    byAlarmOut[HB_MAX_ALARMOUT];
}HB_NET_HANDLEEXCEPTION, *LPHB_NET_HANDLEEXCEPTION;

typedef struct ST_HB_NET_SCHEDTIME
{
    BYTE    byEnable;                               // 布放时间使能 0-撤防 1-布放
    BYTE    byStartHour;                            // 开始小时 0～23
    BYTE    byStartMin;                             // 开始分钟 0～59
    BYTE    byStartSec;                             // 开始秒 0～59 （保留）
    BYTE    byStopHour;
    BYTE    byStopMin;
    BYTE    byStopSec;
    BYTE    byRes;
}HB_NET_SCHEDTIME, *LPHB_NET_SCHEDTIME;

// 移动侦测区域，分22*18个小宏块
#define HB_MOTION_SCOPE_WIDTH   22
#define HB_MOTION_SCOPE_HIGHT   18

typedef struct ST_HB_NET_MOTION
{
    BYTE    byMotionScope[HB_MOTION_SCOPE_HIGHT][HB_MOTION_SCOPE_WIDTH]; // 侦测区域,共22*18个小宏块
                                                    // 1表示该宏块是移动侦测区域
    BYTE    byMotionSensitive;                      // 移动侦测灵敏度，0～5，5最灵敏
    BYTE    byEnable;                               // 移动侦测布放使能
    BYTE    byRes[2];
    HB_NET_HANDLEEXCEPTION struProc;                // 报警联动策略
    HB_NET_SCHEDTIME struTime[HB_MAX_DAYS][HB_MAX_TIMESEGMENT]; // 布放时间
                                                    // [0][0,1]表示每天，有被使能则不关注星期几
                                                    // [1~7][0,1]表示星期
    BYTE    byRecordChannel[HB_MAX_CHANNUM];        // 联动录像通道
}HB_NET_MOTION, *LPHB_NET_MOTION;

typedef struct ST_HB_NET_VILOST
{
    BYTE    byEnable;
    BYTE    byRes[7];
    HB_NET_HANDLEEXCEPTION struProc;
    HB_NET_SCHEDTIME struTime[HB_MAX_DAYS][HB_MAX_TIMESEGMENT];
}HB_NET_VILOST, *LPHB_NET_VILOST;

typedef struct ST_HB_NET_RECT
{
    WORD    wTopLeftX;                              // 0~704
    WORD    wTopLeftY;                              // 0~576
    WORD    wWidth;
    WORD    wHeight;
}HB_NET_RECT, *LPHB_NET_RECT;

typedef struct ST_HB_NET_SHELTER
{
    HB_NET_RECT rect;
}HB_NET_SHELTER, *LPHB_NET_SHELTER;

typedef struct ST_HB_NET_HIDEALARM
{
    DWORD   dwEnable;
    DWORD   dwRes;
    HB_NET_RECT rect;
    HB_NET_HANDLEEXCEPTION struProc;
    HB_NET_SCHEDTIME struTime[HB_MAX_DAYS][HB_MAX_TIMESEGMENT];
}HB_NET_HIDEALARM, *LPHB_NET_HIDEALARM;

typedef struct ST_HB_NET_PICCFG
{
    DWORD   dwSize;
    DWORD   dwChannel;                              // 通道号

    char    sChanName[HB_NAME_LEN];                 // 通道名
    DWORD   dwShowChanName;                         // 是否显示通道名
    WORD    wNameTopLeftX;                          // 通道名x坐标
    WORD    wNameTopLeftY;

    DWORD   dwShowTime;                             // 是否显示时间
    WORD    wOSDTopLeftX;
    WORD    wOSDTopLeftY;
    BYTE    byDispWeek;                             // 是否显示星期
    BYTE    byOSDAttrib;                            // OSD是否透明，1-不透明 2-透明
    BYTE    byOSDOverlay;                           // 最高位 0 解码后叠加 1-前端叠加
    BYTE    byShelter;                              // 视频遮挡使能

    BYTE    byBright;                               // 亮度 0~255
    BYTE    byContrast;                             // 对比度 0~255
    BYTE    bySaturation;                           // 饱和度 0~255
    BYTE    byHue;                                  // 色度 0~255
    
    HB_NET_SHELTER struShelter[HB_MAX_SHELTERNUM];
    HB_NET_VILOST struVILost;
    HB_NET_MOTION struMotion;
    HB_NET_HIDEALARM struHide;
    DWORD   dwVideoFormat;                          // 保留
    DWORD   dwRes;
}HB_NET_PICCFG, *LPHB_NET_PICCFG;

//-----------------------------------------------------------------------------
// 压缩参数
#define HB_NET_GET_COMPRESSCFG  0x2C
#define HB_NET_SET_COMPRESSCFG  0x2D

// 获取需填充dwChannel/dwRecordType字段

typedef struct ST_HB_NET_COMPRESSINFO
{
    BYTE    byAudio;                                // 0-无音频 1-有音频
    BYTE    byResolution;   // 0-CIF, 1-HD1, 2-D1, 3-QCIF, 4-720P, 5-1080P
    BYTE    byBitRateType;                          // 0-变码率, 1-定码率
    BYTE    byPicQuality;   // 图像质量 1-最好, 2-次好, 3, 4, 5, 6-差
    DWORD   dwBitRate;      // 0-100K, 1-128K, 2-256K, 3-512K, 4-1M, 5-1.5M, 6-2M,
                            // 7-3M, 8-4M ...  
                            // 其他:码率值[32~2^32] 以K为单位
    DWORD   dwFrameRate;     // 帧率:[2,30], 与主机相关
    DWORD   dwRes;
}HB_NET_COMPRESSINFO, *LPHB_NET_COMPRESSINFO;

typedef struct ST_HB_NET_COMPRESSCFG
{
    DWORD   dwSize;
    DWORD   dwChannel;
    HB_NET_COMPRESSINFO struRecord;                 // 录像流(主码流)
    HB_NET_COMPRESSINFO struNet;                     // 子码流(子码流)
    DWORD   dwRecordType;   // 0x0:手动录像，0x1:定时录象，0x2:移动侦测，0x3:报警，0x0f:所有类型
    DWORD   dwRes;
}HB_NET_COMPRESSCFG, *LPHB_NET_COMPRESSCFG;

//-----------------------------------------------------------------------------
// 录像参数
#define HB_NET_GET_RECORDCFG    0x2E
#define HB_NET_SET_RECORDCFG    0x2F

typedef struct ST_HB_NET_RECORDSCHED
{
    HB_NET_SCHEDTIME strRecordTime;
    DWORD   dwRecordType;
    DWORD   dwRes;
}HB_NET_RECORDSCHED, *LPHB_NET_RECORDSCHED;

typedef struct ST_HB_NET_RECORDDAY
{
    WORD    bAllDayRecord;      // 是否全天录像
    WORD    byRecordType;       // 录像类型
    DWORD   dwRes;
}HB_NET_RECORDDAY, *LPHB_NET_RECORDDAY;

typedef struct ST_HB_NET_RECORDCFG
{
    DWORD   dwSize;
    DWORD   dwChannel;
    HB_NET_RECORDDAY struRecAllDay[HB_MAX_DAYS];
    HB_NET_RECORDSCHED struRecSched[HB_MAX_DAYS][HB_MAX_TIMESEGMENT];
    DWORD   dwRecord;                               // 是否录像
    DWORD   dwRes;                                  // 保留
}HB_NET_RECORDCFG, *LPHB_NET_RECORDCFG;

//-----------------------------------------------------------------------------
// 云台解码器
#define HB_NET_GET_DECODERCFG   0x30
#define HB_NET_SET_DECODERCFG   0x31

typedef struct ST_HB_NET_DECODERCFG
{
    DWORD   dwSize;
    DWORD   dwChannel;                              // 通道号
    DWORD   dwBaudRate; // 波特率 0-2400, 1-2400, 2-4800, 3-9600, 4-19200, 5-38400,
                        // 自定义[300, 115200]
    BYTE    byDataBit;                              // 数据位 5 6 7 8
    BYTE    byStopBit;                              // 停止位 1 2
    BYTE    byParity;   // 校验位 0-NONE, 1-ODD, 2-EVEN, 3-SPACE
    BYTE    byFlowCtrl; // 流控 0-无, 1-Xon/Xoff, 2-硬件
    WORD    wDecoderType;       // 云台协议值 HB_NET_GetPTZProtocolList获取
    WORD    wDecoderAddr;       // 解码器地址[0-255]
    BYTE    bySetPreset[128];
    BYTE    bySetCruise[128];
    BYTE    bySetTrack[128];
    DWORD   dwRes;
}HB_NET_DECODERCFG, *LPHB_NET_DECODERCFG;

//-----------------------------------------------------------------------------
// 报警输入参数
#define HB_NET_GET_ALARMINCFG   0x32
#define HB_NET_SET_ALARMINCFG   0x33

typedef struct ST_HB_NET_ALARMINCFG
{
    DWORD   dwSize;
    DWORD   dwAlarmInPort;                          // 报警输入端口号
    char    sAlarmInName[HB_NAME_LEN];
    BYTE    byAlarmType;                            // 探头类型 0-常闭 1-常开
    BYTE    byAlarmInHandle;                        // 是否处理
    BYTE    byRecordTime;                           // 报警录像时间 1-99秒
    BYTE    byRes[5];
    HB_NET_HANDLEEXCEPTION  struAlarmProc;          // 处理策略
    HB_NET_SCHEDTIME    struTime[HB_MAX_DAYS][HB_MAX_TIMESEGMENT]; // 布放时间
    BYTE    byRelRecordChan[HB_MAX_CHANNUM];        // 报警触发录像通道, 1表示触发该通道
    BYTE    byEnablePreset[HB_MAX_CHANNUM];         // 是否调用预置点，仅用[0]判断
    BYTE    byPresetNO[HB_MAX_CHANNUM];             // 调用的云台预置点序号，一个报警输入
                                                    // 可以调用多个通道的云台预置点，
                                                    // 0xff表示不调用 [1,254]
    BYTE    byEnableCruise[HB_MAX_CHANNUM];
    BYTE    byCruiseNO[HB_MAX_CHANNUM];
    BYTE    byEnablePTZTrack[HB_MAX_CHANNUM];
    BYTE    byPTZTrack[HB_MAX_CHANNUM];
}HB_NET_ALARMINCFG, *LPHB_NET_ALARMINCFG;

//-----------------------------------------------------------------------------
// 报警输出参数
#define HB_NET_GET_ALARMOUTCFG  0x34
#define HB_NET_SET_ALARMOUTCFG  0x35

typedef struct ST_HB_NET_ALARMOUTCFG
{
    DWORD   dwSize;
    BYTE    byAlarmOutPort;                         // 报警输出端口号
    BYTE    byEnSchedule;                           // 报警输出布放时间激活
    WORD    wRes;
    char    sAlarmOutName[HB_NAME_LEN];             // 报警输出端口名
    HB_NET_SCHEDTIME    struTime[HB_MAX_DAYS][HB_MAX_TIMESEGMENT];
    DWORD   dwAlarmOutDelay;                        // 输出保持时间，单位秒[2,300]
    DWORD   dwRes;
}HB_NET_ALARMOUTCFG, *LPHB_NET_ALARMOUTCFG;

//-----------------------------------------------------------------------------
// 主机时间
#define HB_NET_GET_TIMECFG      0x36
#define HB_NET_SET_TIMECFG      0x37

// HB_NET_TIME

//-----------------------------------------------------------------------------
// 用户权限
#define HB_NET_GET_USERCFG      0x38
#define HB_NET_SET_USERCFG      0x39

#define HB_MAX_USER             16
#define HB_MAX_RIGHT            32

// 本地权限 数组0未使用; 取值: 0-无权限 1-有权限
// 1.常规设置
// 2.录像设置
// 3.输出设置
// 4.报警设置
// 5.串口设置
// 6.网络设置
// 7.录像回放
// 8.系统管理
// 9.系统信息
// 10.报警清除
// 11.云台控制
// 12.关机重启
// 13.USB升级
// 14.备份

// 远程权限
// 1.远程预览
// 2.参数设置
// 3.远程回放
// 4.远程备份
// 5.查看日志
// 6.语音对讲
// 7.远程升级
// 8.远程重启

typedef struct ST_HB_NET_USER_INFO
{
    char    sUser[HB_NAME_LEN];                 // 用户名
    char    sPWD[HB_PWD_LEN];                   // 密码
    BYTE    byLocalRight[HB_MAX_RIGHT];         // 本地权限
    BYTE    byLocalChannel[HB_MAX_CHANNUM];
    BYTE    byRemoteRight[HB_MAX_RIGHT];
    BYTE    byRemoteChannel[HB_MAX_CHANNUM];
    DWORD   dwUserIP;
    BYTE    byMACAddr[6];
    BYTE    byRes[2];
}HB_NET_USER_INFO, *LPHB_NET_USER_INFO;

typedef struct ST_HB_NET_USERCFG
{
    DWORD   dwSize;
    HB_NET_USER_INFO struUser[HB_MAX_USER];
}HB_NET_USERCFG, *LPHB_NET_USERCFG;

//-----------------------------------------------------------------------------
// 串口参数
#define HB_NET_GET_SERIALCFG    0x3A
#define HB_NET_SET_SERIALCFG    0x3B

#define HB_TELNUM_LEN           32                  // 电话号码长度

typedef struct ST_HB_NET_PPPCFG
{
    BYTE    sRemoteIP[HB_IP_LEN];                   // 远程IP地址
    BYTE    sLocalIP[HB_IP_LEN];                    // 本地IP地址
    BYTE    sLocalIPMask[HB_IP_LEN];                // 本地IP地址掩码
    BYTE    sUser[HB_NAME_LEN];                     // 用户名
    BYTE    sPWD[HB_PWD_LEN];                       // 密码
    BYTE    byPPPMode;                              // PPP模式, 0-主动, 1-被动
    BYTE    byRedial;                               // 是否回拨: 0-否, 1-是
    BYTE    byRedialMode;                           // 回拨模式: 0-由拨入者指定
                                                    // 1-预置回拨号码
    BYTE    byDataEncrypt;                          // 数据加密: 0-否, 1-是
    DWORD   dwMTU;                                  // MTU
    BYTE    sTelNum[HB_TELNUM_LEN];                 // 电话号码
}HB_NET_PPPCFG, *LPHB_NET_PPPCFG;

typedef struct ST_HB_NET_SERIALCFG
{
    DWORD   dwSize;
    DWORD   dwSerialType;                           // 串口类型: 1-232, 2-485
    DWORD   dwBaudRate;                             // 波特率(bps)
    BYTE    byDataBit;                              // 数据位 5 6 7 8
    BYTE    byStopBit;                              // 停止位 1 2
    BYTE    byParity;   // 校验位 0-NONE, 1-ODD, 2-EVEN, 3-SPACE
    BYTE    byFlowCtrl; // 流控 0-无, 1-Xon/Xoff, 2-硬件
    HB_NET_PPPCFG struPPPCfg;                       // 保留
    DWORD   dwWorkMode;                             // 保留
    DWORD   dwRes;                                  // ..
}HB_NET_SERIALCFG, *LPHB_NET_SERIALCFG;

//-----------------------------------------------------------------------------
// DDNS
#define HB_NET_GET_DDNSCFG      0x60
#define HB_NET_SET_DDNSCFG      0x61

typedef struct ST_HB_NET_DDNSCFG
{
    DWORD   dwSize;
    DWORD   dwRes1;
    char    sUser[HB_NAME_LEN];                 // DDNS帐号
    char    sPWD[32];                           // DDNS密码
    char    sAddr[4][32];                       // DDNS解析的IP
    char    sName[128];                         // 域名服务器名
    BYTE    byAddrs;                            // sAddr中指定解析地址的行数
    BYTE    byAutoCon;                          // 是否自动重连
    BYTE    bybyStat;                           // 获取时: 1-解析成功, 0 失败或未解析
                                                // 设置时: 1-需要解析域名, 0-不需要
    BYTE    bySave;                             // 获取时无意义
                                                // 设置时表示是否参数:1-保存 0-不保存
    WORD    wServer;                            // 0 hanbang.org.cn
                                                // 1 oray.net
                                                // 2 dyndns.com
                                                // 3 no-ip.com
                                                // 4 ddns.hbgk.net
    WORD    wReboot;
    DWORD   dwRes2;
}HB_NET_DDNSCFG, *LPHB_NET_DDNSCFG;

//-----------------------------------------------------------------------------
// PPPOE
#define HB_NET_GET_PPPOE        0x62
#define HB_NET_SET_PPPOE        0x63

typedef struct ST_HB_NET_PPPOECFG
{
    DWORD   dwSize;
    BYTE    byAutoCon;                          // 自动重连
    union
    {
        BYTE    byStat;                         // 获取时: 0-断开, 1-已连接
        BYTE    byCon;                          // 设置时: byAutoCon为1时无效
                                                // 手动命令: 0-断开, 1-连接
    }u;
    BYTE    bySave;                             // PPPOE信息保存: 0-不保存 1-保存
    BYTE    byRes;
    char    sUser[HB_NAME_LEN];                 // PPPOE用户名
    char    sPWD[32];                           // PPPOE密码
    DWORD   dwRes[2];
}HB_NET_PPPOECFG, *LPHB_NET_PPPOECFG;

//-----------------------------------------------------------------------------
// 平台服务器参数 ---T系列未实现，暂不考虑实现
#define HB_NET_GET_SERVERCFG    0x64
#define HB_NET_SET_SERVERCFG    0x65

//-----------------------------------------------------------------------------
// 序列号
#define HB_NET_GET_SERIALID     0x68
#define HB_NET_SET_SERIALID     0x67

//-----------------------------------------------------------------------------
// 设备类型
#define HB_NET_GET_DEVTYPE      0x71
// #define HB_NET_SET_DEVTYPE          0x72

typedef enum
{
    HB_7000T = 0,
    HB_8000T,
    HB_8200T,
    HB_8000ATM,
    HB_8600T,
    HB_6200T,
    HB_8004AH,
    HB_8004AI,
    HB_7000H,
    HB_7200H,
    HB_7000M = 12,
    HB_8000M,
    HB_8200M,
    HB_7000L,
    HB_2201TL = 16,
    HB_2600T,
    HB_2600TB,                                      // 客流统计智能分析盒
    HB_2600TC,                                      // 车牌识别智能分析盒
    HB_9300,
    HB_9400,

    HB_9824N16H = 1000,
    HB_9832N16H,
    HB_9904,
    HB_9908,
    HB_9912,
    HB_9916,
    HB_9932,
    HB_7904,
    HB_7908,
    HB_7912,
    HB_7916,
}HB_NET_DEVTYPE_E;

typedef enum
{
    HB_M8 = 0,                                      // 8M
    HB_M16,                                         // 16M
    HB_M32,
    HB_M64,
    HB_M128,
    HB_M256,
    HB_M512,
    HB_M1024,
}HB_NET_MEMSIZE_E;

typedef struct ST_HB_NET_DEVINFO
{
    DWORD   dwSize;
    DWORD   dwDevNO;                                // 7004/8004/NVRs/1030/1031
                                                    // 用于解码库初始化识别
    HB_NET_DEVTYPE_E eDevType;
    HB_NET_MEMSIZE_E eMemSize;
    DWORD   dwRes[2];
}HB_NET_DEVINFO, *LPHB_NET_DEVINFO;

//-----------------------------------------------------------------------------
// 多预置点轮巡
#define HB_NET_GET_PRESETPOLL   0x74
#define HB_NET_SET_PRESETPOLL   0x73

typedef struct ST_HB_NET_PRESET
{
    WORD    wPreset;                                // 预置点
    WORD    wTime;                                  // 预置点停留时间[1,99]
}HB_NET_PRESET, *LPHB_NET_PRESET;

typedef struct ST_HB_NET_PRESETPOLLCFG
{
    DWORD   dwSize;
    WORD    wChannel;                               // 设置通道
    WORD    wPoll;                                  // 轮巡开关: 0-关闭, 1-开启
    HB_NET_PRESET struPreset[256];                  // 目前最多支持16个预置点
                                                    // 预置点时间不可单独设置
                                                    // 预置点值为255表示无效
    DWORD   dwRes[2];
}HB_NET_PRESETPOLLCFG, *LPHB_NET_PRESETPOLLCFG;

//-----------------------------------------------------------------------------
// 前端视频制式
#define HB_NET_GET_VIDEOSYS     0x76
#define HB_NET_SET_VIDEOSYS     0x75

typedef struct ST_HB_NET_VIDEOSYS
{
    DWORD   dwSize;
    WORD    wSys;                                   // 1-PAL, 2-NTSC4.43, 3-NTSC3.58
    WORD    wRes;
}HB_NET_VIDEOSYS, *LPHB_NET_VIDEOSYS;

//-----------------------------------------------------------------------------
// 实时布放                         0x7A/0x7B---暂不支持

//-----------------------------------------------------------------------------
// 延时抓拍                         0x7C ---暂不支持

//-----------------------------------------------------------------------------
// 前端蜂鸣状态
#define HB_NET_GET_BUZSTAT      0x81 
#define HB_NET_SET_BUZSTAT      0x80

typedef struct ST_HB_NET_BUZSTAT
{
    DWORD   dwSize;
    DWORD   dwStat;                                 // 0-关, 1-开
}HB_NET_BUZSTAT, *LPHB_NET_BUZSTAT;

//-----------------------------------------------------------------------------
// I帧间隔
#define HB_NET_GET_IFRAMERATE   0x84
#define HB_NET_SET_IFRAMERATE   0x89

typedef struct ST_HB_NET_IFRAMERATE
{
    DWORD   dwSize;
    WORD    wChannel;                               // 通道号
    WORD    wStreamType;                            // 码流类型 0-主码流, 1-子码流1
    DWORD   dwIFrameRate;                           // I帧间隔
    DWORD   dwRes;
}HB_NET_IFRAMERATE, *LPHB_NET_IFRAMERATE;

//-----------------------------------------------------------------------------
// 无线参数配置(IPC)
#define HB_NET_GET_WIRELESSCFG  0x88
#define HB_NET_SET_WIRELESSCFG  0x87

typedef struct ST_HB_NET_WEP
{
    BYTE    bySafeOption;                           // 安全选项设置, 0-自动选择
                                                    // 1-开放系统, 2-共享密钥
    BYTE    byPWDFormat;                            // 密钥格式, 0-16进制, 1-ASCII码
    BYTE    byPWDType;                              // 密钥类型 0-禁用, 1-64位, 
                                                    // 2-128位, 3-152位
    BYTE    sPWD[62];                               // 密码
    BYTE    byRes[7];
}HB_NET_WEP, *LPHB_NET_WEP;

typedef struct ST_HB_NET_WPAPSK
{
    BYTE    bySafeOption;       // 安全选项设置 0-自动选择, 1-WPA-PSK, 2-WPA2-PSK
    BYTE    byPWDMod;           // 加密方式设置, 0-自动选择, 1-TKIP, 2-AES
    BYTE    byPWD[64];          // psk密码, 8到63个字符
    BYTE    byRes[6];
}HB_NET_WPAPSK, *LPHB_NET_WPAPSK;

typedef struct ST_HB_NET_WIRELESSCFG
{
    DWORD   dwSize;
    BYTE    sSSID[50];                              // SSID号
    BYTE    bySafeType;                             // 安全类型设置, 0-WEB, 1-WPA-PSK/WPA2-PSK
                                                    // 2-无加密
    BYTE    byRes;
    BYTE    sWirelessIP[HB_IP_LEN];                 // 无线IP
    union
    {
        HB_NET_WEP  struWep;
        HB_NET_WPAPSK struWpapsk;
    }u;
    DWORD   dwRes;
}HB_NET_WIRELESSCFG, *LPHB_NET_WIRELESSCFG;

//-----------------------------------------------------------------------------
// NTP配置
#define HB_NET_GET_NTPCFG       0x91
#define HB_NET_SET_NTPCFG       0x90

typedef struct ST_HB_NET_NTPCFG
{
    DWORD   dwSize;
    DWORD   dwPort;                                 // 端口
    DWORD   dwServerIndex;                          // 服务器索引号
    DWORD   dwSyncPeriod;                           // 同步周期[1,60]
    DWORD   dwSyncUnit;                             // 同步周期, 0-分钟, 1-小时
                                                    // 2-天, 3-星期, 4-月
    DWORD   dwSyncTime;                             // 保留
    int     nTimeZone;                              // 时区[-12, 13]
    DWORD   dwAuto;                                 // 开启ntp服务, 0-手动, 1-自动
    char    sServer[HB_NAME_LEN];                   // 服务器
    DWORD   dwRes[2];
}HB_NET_NTPCFG, *LPHB_NET_NTPCFG;

//-----------------------------------------------------------------------------
// SMTP配置
#define HB_NET_GET_SMTPCFG      0x93
#define HB_NET_SET_SMTPCFG      0x92

typedef struct ST_HB_NET_SMTPCFG
{
    DWORD   dwSize;
    DWORD   dwPort;                                 // 服务器端口，默认值25
    char    sHost[128];                             // 发送邮件的SMTP服务器
    char    sUser[HB_NAME_LEN];
    char    sPwd[32];
    char    sSendAddr[128];                         // FROM:邮件地址
    char    sRecvAddr[256];                         // TO:邮件地址,多个以';'分隔
    DWORD   dwSendPeriod;                           // 上传周期
    DWORD   dwSnapEnable;                           // 是否抓拍上传
    BYTE    byUseSSL;                               // 启用SSL
    BYTE    byUseStartTLS;                          // 启用STARTTLS
    BYTE    byRes[30];
}HB_NET_SMTPCFG, *LPHB_NET_SMTPCFG;

//-----------------------------------------------------------------------------
// 轮巡配置
#define HB_NET_GET_POLLCFG      0x95
#define HB_NET_SET_POLLCFG      0x94

typedef struct ST_HB_NET_POLLCFG
{
    DWORD   dwSize;
    DWORD   dwPollType;                             // 轮巡类型: 0-普通轮巡, 1-SPOT轮巡
    DWORD   dwEnable;                               // 是否启用轮巡
    DWORD   dwPeriod;                               // 轮巡间隔, [1,99]秒
    DWORD   dwFormat;                               // 画面格式, 0-off, 1-1画面
                                                    // 2-2画面, 4-2*2画面, 9-3*3画面...
    BYTE    byChList[HB_MAX_CHANNUM];               // 轮巡通道, 0-该通道不轮巡,
                                                    // 1-该通道轮巡
    DWORD   dwRes;
}HB_NET_POLLCFG, *LPHB_NET_POLLCFG;

//-----------------------------------------------------------------------------
// 视频矩阵
#define HB_NET_GET_VIDEOMATRIX  0x97
#define HB_NET_SET_VIDEOMATRIX  0x96

typedef struct ST_HB_NET_VIDEOMATRIX
{
    DWORD   dwSize;
    BYTE    byMatixChl[HB_MAX_CHANNUM];             // 视频矩阵对应通道, 从1开始, 0xFF表示关闭
    DWORD   dwRes;
}HB_NET_VIDEOMATRIX, *LPHB_NET_VIDEOMATRIX;

//-----------------------------------------------------------------------------
// 遮挡报警
#define HB_NET_GET_VCOVERDETECT 0x98
#define HB_NET_SET_VCOVERDETECT 0x99

typedef struct ST_HB_NET_VCOVERCFG
{
    DWORD   dwSize;
    DWORD   dwChannel;                              // 通道号
    DWORD   dwEnable;                               // 遮挡报警使能
    BYTE    bySensorOut[HB_MAX_ALARMOUT];           // 联动报警输出
    DWORD   dwRes;
}HB_NET_VCOVERCFG, *LPHB_NET_VCOVERCFG;

//-----------------------------------------------------------------------------
// 实时布防撤防状态
#define HB_NET_GET_REALDEFENCECFG   0xA5
#define HB_NET_SET_REALDEFENCECFG   0xA6

typedef struct ST_HB_NET_REALDEFENCECFG
{
    DWORD   dwSize;
    WORD    wEnable;                                // 0-撤防, 1-布防
    WORD    wTime;                                  // 布防延时时间
    DWORD   dwRes[4];
}HB_NET_REALDEFENCECFG, *LPHB_NET_REALDEFENCECFG;

//-----------------------------------------------------------------------------
// 零通道参数配置
#define HB_NET_GET_ZEROVENCCFG  0xA8
#define HB_NET_SET_ZEROVENCCFG  0xA9

typedef struct ST_HB_NET_VENCCFG
{
    BYTE    byStreamType;                           // 码流类型: 0-无音频, 1-有音频
    BYTE    byResolution;                           // 分辨率: 0-CIF, 1-HD1, 2-D1,
                                                    // 3-QCIF, 4-720P, 5-1080P
    BYTE    byBitRateType;                          // 码率类型: 0-变码率, 1-定码流
    BYTE    byPicQuality;                           // 图像质量, 1~6, 1最好, 6最差
    DWORD   dwBitRate;      // 码率 0-100K, 1-128K, 2-256K, 3-512K, 4-1M, 5-1.5M, 6-2M,
                            // 7-3M, 8-4M ...  
                            // 其他:码率值[32~2^32] 以K为单位
    WORD    wFrame;                                 // 帧率 wFrame/wTimeBase,一般为2～30
    WORD    wTimeBase;                              // 
    DWORD   dwQuant;                                // 量化因子 1~31
}HB_NET_VENCCFG, *LPHB_NET_VENCCFG;

typedef struct ST_HB_NET_ZEROVENCCFG
{
    DWORD   dwSize;
    DWORD   dwEnable;                               // 是否启用复合通道
    BYTE    byChlList[HB_MAX_CHANNUM];              // 选择通道
    HB_NET_VENCCFG struVencCfg;                     // 复合通道视频参数
    BYTE    byFormat;                               // 画面格式, 0-off, 1-1画面
                                                    // 4-2*2画面, 9-3*3画面, 16-4*4画面
    BYTE    bySwitchTime;                           // 切换时间
    BYTE    byRes[2];
}HB_NET_ZEROVENCCFG, *LPHB_NET_ZEROVENCCFG;

//-----------------------------------------------------------------------------
// 夏令时
#define HB_NET_GET_DSTTIME      0xAA
#define HB_NET_SET_DSTTIME      0xAB

typedef struct ST_HB_NET_DSTWEEKTIME
{
    BYTE    byMon;                                  // 月[1,12]
    BYTE    byWeeks;                                // 周[1, 5]
    BYTE    byWeek;                                 // 星期[0, 6]
    BYTE    byHour;                                 // 时[0, 23]
    BYTE    byMin;                                  // 分[0, 59]
    BYTE    bySec;                                  // 秒[0, 59]
    BYTE    byRes[2];
}HB_NET_DSTWEEKTIME, *LPHB_NET_DSTWEEKTIME;

typedef struct ST_HB_NET_DSTTIME
{
    DWORD   dwSize;
    WORD    wEanble;                                // 夏令时使能
    WORD    wType;                                  // 0-按周, 1按日期
    HB_NET_TIME struStartDate;                      // 按日期设置的开始时间
    HB_NET_TIME struEndDate;                        // 按日期设置的结束时间
    HB_NET_DSTWEEKTIME struStartTime;               // 按周设置的开始时间
    HB_NET_DSTWEEKTIME struEndTime;                 // ...
    DWORD   dwRes[2];
}HB_NET_DSTTIME, *LPHB_NET_DSTTIME;

//-----------------------------------------------------------------------------
// ftp升级配置
#define HB_NET_GET_FTPUPGRADECFG    0xAC
#define HB_NET_SET_FTPUPGRADECFG    0xAD

typedef struct ST_HB_NET_FTPUPGRADECFG
{
    DWORD   dwSize;
    DWORD   dwPort;                                 // ftp 端口号
    char    sServer[128];                           // ftp服务器地址,目前只支持IP地址
    char    sUser[HB_NAME_LEN];                     // ftp用户名
    char    sPwd[32];                               // ftp密码
    char    sPath[128];                             // ftp文件路径，升级文件的路径
    BYTE    byUpgrade;                              // 是否升级，设置时有效
    BYTE    byRes[7];
}HB_NET_FTPUPGRADECFG, *LPHB_NET_FTPUPGRADECFG;

//-----------------------------------------------------------------------------
// ftp上传录像文件配置
#define HB_NET_GET_FTPRECORDCFG 0xAE
#define HB_NET_SET_FTPRECORDCFG 0xAF

typedef struct ST_HB_NET_SCHE_ITEM
{
    DWORD   dwTime_seg[3];                          // 时间段，15分钟,96段
    BYTE    byEnable;                               // 
    BYTE    byRes[3];
}HB_NET_SCHE_ITEM, *LPHB_NET_SCHE_ITEM;

typedef struct ST_HB_NET_PAGE_SCHE
{
    HB_NET_SCHE_ITEM struItem[HB_MAX_DAYS];         // 0-everyday, 1-monday...7-sunday
    DWORD   dwRes[2];
}HB_NET_PAGE_SCHE, *LPHB_NET_PAGE_SCHE;

typedef struct ST_HB_NET_FTPRECORDCFG
{
    DWORD   dwSize;
    BYTE    byChl;                                  // 通道号, 0xFF表示所有通道
    BYTE    byEnPlan;                               // 定时录像ftp上传开关
    BYTE    byEnMotion;                             // 移动侦测
    BYTE    byEnAlarm;                              // 探头报警
    char    sServer[128];                           // ftp服务器地址
    char    sUser[32];                              // ftp用户名
    char    sPwd[32];                               // ftp密码
    char    sPath[128];                             // 文件上传路径
    HB_NET_PAGE_SCHE struPlanRec;                   // 定时录像监控时间段
    DWORD   dwPort;                                 // ftp端口
    DWORD   dwRes;
}HB_NET_FTPRECORDCFG, *LPHB_NET_FTPRECORDCFG;

//-----------------------------------------------------------------------------
// IPC工作参数
#define HB_NET_GET_IPCWORKPARAM 0xB0
#define HB_NET_SET_IPCWORKPARAM 0xB1
typedef struct 
{ 
   BYTE  cbStreamType; //码流类型 1-主流2-子流 3-第三码流 
   BYTE  cbReserve[3]; //保留 
}HB_NET_REQIPCWORKPARAM; 

typedef struct ST_HB_NET_ICRTIME
{
    DWORD    wLightRange;                           // ICR亮度切换临界值
    DWORD    wEnable;                               // 0-不支持, 1-亮度值有效, 2-时间段有效
    HB_NET_SCHEDTIME struTime[2];
}HB_NET_ICRTIME, *LPHB_NET_ICRTIME;

typedef struct ST_HB_NET_FRAMERATE
{
    BYTE    byMin;                                  // 该机型支持的最小帧率
    BYTE    byMax;                                  // 最大帧率
    BYTE    byCur;                                  // 当前帧率
    BYTE    byRes;
    DWORD   dwRes;
}HB_NET_FRAMERATE, *LPHB_NET_FRAMERATE;

// 快门时间参数
typedef struct ST_HB_NET_SHUTTERVAL
{
    DWORD   dwIndex;
    DWORD   dwRes;
    DWORD   dwVal[32];
}HB_NET_SHUTTERVAL, *LPHB_NET_SHUTTERVAL;

typedef struct ST_HB_NET_SCENEVAL
{
    DWORD   dwIndex;
    DWORD   dwRes;
    BYTE    bySceneVal[8][32];
}HB_NET_SCENEVAL, *LPHB_NET_SCENEVAL;

typedef struct ST_HB_NET_RESOLUTION
{
    DWORD   dwIndex;
    DWORD   dwRes;
    DWORD   dwResolution[16];
}HB_NET_RESOLUTION, *LPHB_NET_RESOLUTION;

typedef struct ST_HB_NET_AGCVAL
{
    DWORD   dwIndex;
    DWORD   dwRes;
    BYTE    byAgcVal[32];
}HB_NET_AGCVAL, *LPHB_NET_AGCVAL;

typedef struct ST_HB_NET_IPCWORKPARAM
{
    DWORD   dwSize;
    BYTE    byStreamType;                           // 码流类型 0-不支持 1-主码流
    BYTE    byStreamEnable;                         // 是否开启当前码流
    BYTE    byAudioEnable;                          // 音频使能
    BYTE    byAntiFlicker;                          // 抗闪烁设置
    HB_NET_FRAMERATE struFrameRate;
    HB_NET_SHUTTERVAL struShutter;
    HB_NET_SCENEVAL struScene;                   // 镜头相关参数
    HB_NET_RESOLUTION struResolution;               // 解析度相关
    HB_NET_AGCVAL struAgc;
    HB_NET_ICRTIME struICR;
    DWORD   dwBitRate;                              // 码率, 0-不支持, 1-128K...
    BYTE    byFoucusSpeed;                          
    BYTE    byDigitalFoucus;
    BYTE    byImageTurn;
    BYTE    byBlackWhiteCtrl;
    BYTE    byIRISCtrl;
    BYTE    byAutoFoucus;
    BYTE    byAWB;
    BYTE    byA3Ctrl;
    BYTE    byFNRSupp;
    BYTE    byStreamKind;
    BYTE    byVideoOutKind;
    BYTE    byWDR;
    BYTE    byColorMode;
    BYTE    bySharpNess;
    BYTE    byPlatformType;
    BYTE    byRes[17];
    DWORD   dwRes;
}HB_NET_IPCWORKPARAM, *LPHB_NET_IPCWORKPARAM;

//-----------------------------------------------------------------------------
// NVR报警输入输出端口属性
#define HB_NET_GET_ALARMPORTATTR        0xC9

typedef struct ST_HB_NET_ALARMPORTATTR
{
    DWORD   dwSize;
    BYTE    byOpType;                               // 0-本地, 1-前端
    BYTE    byChannel;                              // 通道号
    BYTE    byAlarmInNum;                           // 报警输入个数
    BYTE    byAlarmOutNum;                          // 报警输出个数
    char    sAlarmInPortName[HB_MAX_ALARMIN][HB_NAME_LEN];
    char    sAlarmOutPortName[HB_MAX_ALARMOUT][HB_NAME_LEN];
    char    sDevName[HB_NAME_LEN];                  // 通道对应设备名
    DWORD   dwIP;                                   // 设备IP
    DWORD   dwRes;
}HB_NET_ALARMPORTATTR, *LPHB_NET_ALARMPORTATTR;

//-----------------------------------------------------------------------------
// NVR报警输入参数
#define HB_NET_GET_NVRALARMINCFG        0xCA
#define HB_NET_SET_NVRALARMINCFG        0xCB

typedef struct ST_HB_NET_NVRALARMINCFG
{
    DWORD   dwSize;
    BYTE    byOpType;                               // 0-本地, 1-前端
    BYTE    byChannel;                              // 操作前端某通道设备
    BYTE    byAlarmType;                            // 探头类型
    BYTE    byEnSchedule;                           // 报警输入布防时间激活
    BYTE    byWeekEnable;                           // 每天使能
    BYTE    byAlarmInPort;                          // 报警输入端口号
    BYTE    byRes[2];
    char    sAlarmInName[HB_NAME_LEN];              // 报警输入端口名
    BYTE    byAllDayEnable[8];
    HB_NET_SCHEDTIME struAlarmTime[8][8];

    // 联动报警输出
    BYTE    byAlarmOutLocal[16];                    // 联动本地报警输出端口
    BYTE    byAlarmOutRemote[HB_MAX_CHANNUM][16];   // 联动前端设备报警输出端口
    
    // 联动录像
    BYTE    byRelRecordChnl[HB_MAX_CHANNUM];        // 报警触发录像通道, 1-触发该通道

    // 联动其他
    BYTE    byEnablePreset[128];                    // 是否调用预置点, 仅用byEnablePreset[0]来判断
    BYTE    byPresetNO[HB_MAX_CHANNUM]; // 调用云台预置点序号, 一个报警输入可调用多个通道的云台预置点
                                        // 0xFF表示不调用预置点[1,254]            
    DWORD   dwHandleType;                           // 按位 2-声音报警 5-监视器最大化
    DWORD   dwRes[9];
}HB_NET_NVRALARMINCFG, *LPHB_NET_NVRALARMINCFG;

//-----------------------------------------------------------------------------
// NVR报警输出参数
#define HB_NET_GET_NVRALARMOUTCFG       0xCC
#define HB_NET_SET_NVRALARMOUTCFG       0xCD

typedef struct ST_HB_NET_NVRALARMOUTCFG
{
    DWORD   dwSize;
    BYTE    byOpType;                               // 0-本地 1-前端
    BYTE    byChnl;                                 // 前端通道号
    BYTE    byPort;                                 // 报警输出通道号
    BYTE    byType;                                 // 探头类型 0-常闭 1-常开
    BYTE    byEnShcedule;                           // 报警输出布防时间激活 0-屏蔽 1-激活
    BYTE    byWeekEnable;                           // 每天使能
    BYTE    byRes[2];
    char    sName[HB_NAME_LEN];                     // 名称
    BYTE    byFullDayEnable[8];                     // 完整天录像
    HB_NET_SCHEDTIME struTime[8][8];
    DWORD   dwDelay;                                // 输出保持时间[2,300]秒
    DWORD   dwRes[9];
}HB_NET_NVRALARMOUTCFG, *LPHB_NET_NVRALARMOUTCFG;

//-----------------------------------------------------------------------------
// 获取NVR报警输入状态
#define HB_NET_GET_NVRALARMINSTAT       0xCE

typedef struct ST_HB_NET_NVRALARMINSTAT
{
    DWORD   dwSize;
    BYTE    byOpType;                               // 0-本地 1-前端
    BYTE    byChnl;                                 // 前端通道
    BYTE    byAlarm;
    BYTE    byRes;
    BYTE    byAlarmIn[HB_MAX_ALARMIN];
    DWORD   dwRes[4];
}HB_NET_NVRALARMINSTAT, *LPHB_NET_NVRALARMINSTAT;

//-----------------------------------------------------------------------------
// NVR通道参数
#define HB_NET_GET_NVRCHLCFG    0xD2
#define HB_NET_SET_NVRCHLCFG    0xD3

typedef struct ST_HB_NET_VIDEOINFO
{
    DWORD   dwBrightness;
    DWORD   dwConstrast;
    DWORD   dwSaturation;
    DWORD   dwHue;
    DWORD   dwSharp;
    DWORD   dwRes;
}HB_NET_VIDEOINFO, *LPHB_NET_VIDEOINFO;

typedef struct ST_HB_NET_NVRCHLCFG
{
    DWORD   dwSize;
    DWORD   dwChnl;                                 // 通道号
    char    sChnlName[HB_NAME_LEN];                 // 通道名
    DWORD   dwShowChnlName;                         // 是否显示通道名
    DWORD   dwOSDAttrib;                            // 通道名 1-不透明 2-透明
    WORD    wShowNameTopLeftX;                      // 通道名显示x坐标
    WORD    wShowNameTopLeftY;                      // 通道名显示y坐标
    
    // 日期相关
    DWORD   dwShowTime;                             // 是否显示时间
    WORD    wOSDTopLeftX;                           // 时间OSD坐标x
    WORD    wOSDTopLeftY;                           // 时间OSD坐标y
    DWORD   dwDataFormat;                           // 日期格式 0-YYYY-MM-DD 1-MM-DD-YYYY
                                                    // 2-YYYY年MM月DD日 3-MM月DD日YYYY年
    // 星期相关
    DWORD   dwDispWeek;                             // 是否显示星期
    DWORD   dwOSDLanguage;                          // 星期语言 0-中文 1-英文

    HB_NET_VIDEOINFO struVideoInfo;                 // 视频信息

    HB_NET_SHELTER struShelter[16];                 // 遮挡区域
    DWORD   dwEnableHide;                           // 视频遮挡使能
    DWORD   dwOSDOverType;                          // 1-前端叠加 2-后端叠加
    DWORD   dwRes[32];
}HB_NET_NVRCHLCFG, *LPHB_NET_NVRCHLCFG;

//-----------------------------------------------------------------------------
// NVR录像参数
#define HB_NET_GET_NVRRECORDCFG 0xD4
#define HB_NET_SET_NVRRECORDCFG 0xD5

typedef struct ST_HB_NET_RECORDPARAM
{
    BYTE    byStreamType;                           // 码流类型: 0-变码流 1-定码流
    BYTE    byQuality;                              // 视频质量: 1最高 6最低
    BYTE    byResolution;                           // 主码流: 0-CIF 1-D1 2-720P 3-1080P
                                                    // 子码流: 0-CIF 1-D1
    BYTE    byFrameRate;                            // 帧率
    BYTE    byMaxBitRate;                           // 0-100K 1-128K 2-256K 3-512K 4-1M 5-1.5M 
                                                    // 6-2M, 7-3M, 8-4M 
    BYTE    byAudio;                                // 0-无音频 1-有音频
    BYTE    byRes[2];
    DWORD   dwRes[2];
}HB_NET_RECORDPARAM, *LPHB_NET_RECORDPARAM;

typedef struct ST_HB_NET_RECTIMEPERIOD
{
    BYTE    byStartH;
    BYTE    byStartM;
    BYTE    byStopH;
    BYTE    byStopM;
    BYTE    byRecType;                              // 录像类型 0-无 1-手动 2-定时 3-移动 4-报警
    BYTE    byRes[3];
}HB_NET_RECTIMEPERIOD, *LPHB_NET_RECTIMEPERIOD;

typedef struct ST_HB_NET_RECFULLDAY
{
    BYTE    byEnable;
    BYTE    byRecType;
    BYTE    byRes[2];
}HB_NET_RECFULLDAY,*LPHB_NET_RECFULLDAY;

typedef struct ST_HB_NET_RECTIMESCHED
{
    DWORD   dwEnable;
    DWORD   dwWeekEnable;
    HB_NET_RECFULLDAY struFullDayEnable[8];
    HB_NET_RECTIMEPERIOD struRecTime[8][8];
    DWORD   dwRes[2];
}HB_NET_RECTIMESCHED, *LPHB_NET_RECTIMESCHED;

typedef struct ST_HB_NET_NVRRECORDCFG
{
    DWORD   dwSize;
    DWORD   dwChnl;
    DWORD   dwPreRec;                               // 预录时间[5,30]秒
    DWORD   dwDelayRec;                             // 录像持续时间[0, 180]
    HB_NET_RECTIMESCHED struSched;
    HB_NET_RECORDPARAM struManu;
    HB_NET_RECORDPARAM struTime;
    HB_NET_RECORDPARAM struMotion;
    HB_NET_RECORDPARAM struAlarm;
    HB_NET_RECORDPARAM struMTOrAlarm;
    HB_NET_RECORDPARAM struMTAndAlarm;
    HB_NET_RECORDPARAM struRes[4];
    DWORD   dwLinkMode;
    DWORD   dwRes[31];
}HB_NET_NVRRECORDCFG, *LPHB_NET_NVRRECORDCFG;

//-----------------------------------------------------------------------------
// NVR移动侦测参数
#define HB_NET_GET_NVRMTDTTCFG  0xD6
#define HB_NET_SET_NVRMTDTTCFG  0xD7

typedef struct ST_HB_NET_NVRMTDTTCFG
{
    DWORD   dwSize;
    DWORD   dwChnl;                                 // 通道号
    BYTE    byScope[18][22];                        // 侦测区域
    DWORD   dwSensitive;                            // 灵敏度 0～5，5最灵敏

    // 联动输出
    BYTE    byRelLocal[16];                         // 报警输出端口(本地)
    BYTE    byRelRemote[HB_MAX_CHANNUM][16];        // 前端设备

    // 联动录像
    BYTE    byRecordChnl[HB_MAX_CHANNUM];

    // 联动其他
    BYTE    byEnablePreset[128];
    BYTE    byPresetNO[128];

    // 时间
    DWORD   dwEnable;                               // 移动侦测布放使能 0-撤防 1-布防
    DWORD   dwWeekEnable;                           // 设置每天 0-不使能 1-使能
    BYTE    byFullDay[8];                           // 完整天录像 0-不使能 1-使能
    HB_NET_SCHEDTIME struTime[8][8];
    DWORD   dwHandleType;                           // 按位 2-声音报警 5 6
    DWORD   dwRes[33];
}HB_NET_NVRMTDTTCFG, *LPHB_NET_NVRMTDTTCFG;

//-----------------------------------------------------------------------------
// NVR异常报警参数
#define HB_NET_GET_ABNORALARMCFG        0xD8
#define HB_NET_SET_ABNORALARMCFG        0xD9

typedef struct ST_HB_NET_ABNORALARMCFG
{
    DWORD   dwSize;
    DWORD   dwType; // 1-视频丢失 2-网络断开 3-温度过高 4-遮挡报警 5-磁盘报警
    DWORD   dwChnl;                                 // 通道号(对遮挡报警有效)
    DWORD   dwEnable;                               // 异常报警使能 0-不支持 1-报警 2-不报警
    HB_NET_SHELTER struShelter;                     // 遮挡报警区域

    //联动报警输出
    BYTE    byRelLocal[16];
    BYTE    byRelRemote[HB_MAX_CHANNUM][16];
    DWORD   dwHandleType;
    DWORD   dwWeekEnable;
    BYTE    byFullDayEnable[8];
    HB_NET_SCHEDTIME struTime[8][8];
    DWORD   dwRes[32];
}HB_NET_ABNORALARMCFG, *LPHB_NET_ABNORALARMCFG;

//-----------------------------------------------------------------------------
// NVR主机分辨率
#define HB_NET_GET_NVRRESOLUTION        0xDC
#define HB_NET_SET_NVRRESOLUTION        0xDD

// 0 HDMI 1080Px60HZ
// 1 HDMI 1080Px50HZ
// 2 HDMI 720Px60HZ
// 3 HDMI 720Px50HZ
// 4 VGA  1024x768

typedef struct ST_HB_NET_NVRRESOLUTION
{
    DWORD   dwSize;
    BYTE    bySupport[32];
    BYTE    byCur;
    BYTE    byRes[7];
}HB_NET_NVRRESOLUTION, *LPHB_NET_NVRRESOLUTION;

//-----------------------------------------------------------------------------
// 远程图片设置
#define HB_NET_GET_SNAPSHOTCFG          0xE4
#define HB_NET_SET_SNAPSHOTCFG          0xE5

typedef struct ST_HB_NET_SNAPSHOTCFG
{
    DWORD   dwSize;
    DWORD   dwChnl;
    BYTE    byType;                                 // 截图类型 1-手动 2-定时 4-移动 8-探头报警
    BYTE    byFormat;                               // 图片格式 0-jpg 1-bmp
    BYTE    byResolution;                           // 0-CIF 1-HD1 2-D1 3-QCIF 4-720P 5-1080P
    BYTE    byQuality;                              // 质量1～6 1-最好 6-差
    BYTE    byPeriod;                               // 截图间隔[1,5]秒
    BYTE    byNum;                                  // 每秒截图张数 byPeriod为1时有效
    BYTE    byEnablePlan;                           // 定时截图开关
    BYTE    byEnableMotion;                         // 移动...
    BYTE    byEnableAlarm;                          // 报警...
    BYTE    byRes[7];
    BYTE    byLink[HB_MAX_CHANNUM];                 // 联动截图通道
    HB_NET_PAGE_SCHE struPlan;
    DWORD   dwRes[2];
}HB_NET_SNAPSHOTCFG, *LPHB_NET_SNAPSHOTCFG;

//-----------------------------------------------------------------------------
// 主机通道参数支持范围获取
#define HB_NET_GET_CHLPARAMSUPPORT      0xE6

//-----------------------------------------------------------------------------
// 用户信息扩展
#define HB_NET_GET_USERINFO             0xE8
#define HB_NET_SET_USERINFO             0xE9

//-----------------------------------------------------------------------------
// 平台参数
#define HB_NET_GET_PLATFORMCFG          0xF0
#define HB_NET_SET_PLATFORMCFG          0xF1

typedef struct ST_HB_NET_GETDEVCONFIG
{
    DWORD   dwSize;
    DWORD   dwCommand;
    DWORD   dwChannel;
    void*   pOutBuffer;                             // 接收数据的缓冲指针
    DWORD   dwOutBufSize;                           // 接收数据的缓冲长度(以字节为单位)，不能为0 
    void*   pBytesRet;                              // 实际收到的数据长度指针，不能为NULL 
    DWORD   dwRes;
}HB_NET_GETDEVCONFIG, *LPHB_NET_GETDEVCONFIG;

BOOL HB_NET_GetDevConfig(long lUserID, LPHB_NET_GETDEVCONFIG lpGet); 

typedef struct ST_HB_NET_SETDEVCONFIG
{
    DWORD   dwSize;
    DWORD   dwCommand;
    DWORD   dwChannel;
    void*   pInBuffer;
    DWORD   dwInBufSize;
    DWORD   dwRes;
}HB_NET_SETDEVCONFIG, *LPHB_NET_SETDEVCONFIG;

BOOL HB_NET_SetDevConfig(long lUserID, LPHB_NET_SETDEVCONFIG lpSet);

//////////////////////////////////////////////////////////////////////////
// 远程设备维护

// 获取主机掉线期间的报警信息
// 与<<实时布防撤防状态>>配置配合使用
#define HB_MAX_ALARMCOUNT           16

typedef struct ST_HB_NET_ALARMINFO
{
    DWORD    wChl;                                  // 报警通道
    DWORD    wType;                                 // 报警类型
    HB_NET_TIME strTime;                            // 报警发生时间
}HB_NET_ALARMINFO, *LPHB_NET_ALARMINFO;

typedef struct ST_HB_NET_DISCONALARM
{
    DWORD   dwSize;
    DWORD   dwCount;                                // 掉线期间报警总数
    HB_NET_ALARMINFO struInf[HB_MAX_ALARMCOUNT];    // 最多,最近的16次报警
    DWORD*  pBytesRet;                              // 实际收到的数据长度指针，不能为NULL 
    DWORD   dwRes;
}HB_NET_DISCONALARM, *LPHB_NET_DISCONALARM;

BOOL HB_NET_GetDisconAlarmInfo(LONG lUserID, LPHB_NET_DISCONALARM lpInfo);

// 清除报警
BOOL HB_NET_ClearAlarm(LONG lUserID);

// 设备工作状态
BOOL HB_NET_GetDevWorkStat(LONG lUserID, LPHB_NET_WORKSTAT lpWorkStat);

typedef struct ST_HB_NET_CLIENTIPINFO
{
    DWORD   dwSize;
    DWORD*  pBytesRet;
    DWORD   dwChannel;                              // 通道号
    DWORD   dwClientIPNum;                          // 连接次数(包括预览/回放/下载)
    DWORD   dwClientIP[64];                         // 连接此通道的IP列表
    DWORD   dwRes[2];
}HB_NET_CLIENTIPINFO, *LPHB_NET_CLIENTIPINFO;

BOOL HB_NET_GetChnClientIP(LONG lUserID, LPHB_NET_CLIENTIPINFO lpInfo);

// 远程升级
typedef struct ST_HB_NET_UPGRADESTAT
{
    DWORD   dwSize;
    DWORD   dwStat;                                 // HB_NET_UPGRADE_XXX 
    DWORD   dwRes[2];
}HB_NET_UPGRADESTAT, *LPHB_NET_UPGRADESTAT;

LONG HB_NET_Upgrade(LONG lUserID, char* sFileName);

//int HB_NET_GetUpgradeProgress(LONG lUpgradeHandle);

#define HB_NET_UPGRADE_FAIL     0
#define HB_NET_UPGRADE_VER_ERR  -1                  // 版本不对升级失败
#define HB_NET_UPGRADE_TRAN_SUC 100                 // 数据传输完毕，等待主机更新
#define HB_NET_UPGRADE_SUC      101                 // 更新成功

BOOL HB_NET_GetUpgradeState(LONG lUpgradeHandle, LPHB_NET_UPGRADESTAT lpStat);

BOOL HB_NET_CloseUpgrade(LONG lUpgradeHandle);

typedef enum //主类型
{
    LOG_PRI_ALL = -1, //全部
    LOG_PRI_ERROR, // 异常
    LOG_PRI_ALARM, // 报警
    LOG_PRI_OPERATE, // 操作
    LOG_PRI_MAX
} LOG_PRI_TYPE;

typedef enum //操作次类型
{
    LOG_OP_ALL = -1,
    LOG_OP_POWERON, // 开机
    LOG_OP_SHUTDOWN, // 关机
    LOG_OP_EXC_SHUTDOWN, //异常关机
    LOG_OP_REBOOT, // 重启
    LOG_OP_LOGIN, // 登陆
    LOG_OP_LOGOUT, // 注销
    LOG_OP_SETCONFIG, // 配置
    LOG_OP_FRONTEND_SETCONFIG, //前端设备配置
    LOG_OP_UPGRADE, // 升级
    LOG_OP_FRONTEND_UPGRADE, //前端设备升级
    LOG_OP_RECORD_START, // 本地启动手动录像
    LOG_OP_RECORD_STOP, // 本地停止手动录像
    LOG_OP_PTZ, // 云台控制
    LOG_OP_MANUAL_ALARM, //本地手动报警
    LOG_OP_FORMAT_DISK, // 格式化硬盘
    LOG_OP_FILE_PLAYBACK, // 本地回放文件
    LOG_OP_EXPORT_CONFIGFILE, //导出本地配置文件
    LOG_OP_LMPORT_CONFIGFILE, //导入本地配置文件
    LOG_OP_FREXPORT_CONFIGFILE, //导出前端设备配置文件
    LOG_OP_FRLMPORT_CONFIGFILE, //导入前端设备配置文件 
    LOG_OP_BACKUP_REC, //本地备份录像文件
    LOG_OP_DEFAULT, //本地恢复缺省
    LOG_OP_SETTIME, // 本地设置系统时间
    LOG_OP_TRANSCOM_OPEN, // 建立透明通道
    LOG_OP_TRANSCOM_CLOSE, // 断开透明通道
    LOG_OP_TALKBACK_START, // 对讲开始
    LOG_OP_TALKBACK_STOP, // 对讲结束

    LOG_OP_TYPE_NONE,    // 无录像
    LOG_OP_TYPE_MANUAL,  // 手动录像
    LOG_OP_TYPE_TIME,    // 定时录像
    LOG_OP_TYPE_MOTION,  // 移动录像
    LOG_OP_TYPE_SENSOR,  // 探头报警
    LOG_OP_TYPE_MOTION_OR_SENSOR,	// 移动或报警录像
    LOG_OP_TYPE_MOTION_AND_SENSOR,	// 移动与探头报警
    LOG_OP_REMOTE_LOGIN,	// 前端设备登陆
    LOG_OP_REMOTE_LOGOUT,	// 前端设备注销

    LOG_OP_TYPE_MAX
} LOG_OPERATE_TYPE;

typedef enum //报警次类型
{
    LOG_ALM_ALL = -1,
    LOG_ALM_LOCAL_SENSORIN, // 本地报警输入
    LOG_ALM_LOCAL_SENSOROUT,// 本地报警输出
    LOG_ALM_FRONTEND_SENSORIN,//前端设备报警输入  
    LOG_ALM_FRONTEND_SENSOROUT,//前端设备报警输出
    LOG_ALM_MOTION_START, // 移动侦测开始
    LOG_ALM_MOTION_STOP, // 移动侦测结束
    LOG_ALM_MAIL_UPLOAD, // 邮件上传
    LOG_ALM_TYPE_MAX
} LOG_ALARM_TYPE;

typedef enum //错误次类型
{
    LOG_ERR_ALL = -1,
    LOG_ERR_VIDEOLOST, // 视频丢失
    LOG_ERR_HD_ERROR, // 磁盘错误
    LOG_ERR_HD_FULL, // 磁盘满
    LOG_ERR_LOGIN_FAIL, // 登陆失败
    LOG_ERR_TEMP_HI, // 温度过高
    LOG_ERR_HD_PFILE_INDEX, // 磁盘主索引错误
    LOG_ERR_HD_DEV_FATAL, // 磁盘设备致命错误
    LOG_ERR_IP_CONFLICT, //ip冲突
    LOG_ERR_NET_EXCEPTION, //网络异常
    LOG_ERR_REC_EXCEPTION, //录像异常
    LOG_ERR_FRONTEND_EXCEPTION, //前端设备异常
    LOG_ERR_TIME_EXCEPTION, //时间异常
    LOG_ERR_FRONTBOARD_EXCEPTION, //前面板异常
    LOG_ERR_TYPE_MAX
} LOG_ERROR_TYPE;

typedef struct ST_HB_NET_LOGFINDCOND
{
    DWORD   dwSize;
    WORD    wPriType;
    WORD    wSecType;
    HB_NET_TIME struBegin;
    HB_NET_TIME struEnd;
    DWORD   dwRes[2];
}HB_NET_LOGFINDCOND, *LPHB_NET_LOGFINDCOND;

// 日志
LONG HB_NET_FindDevLog(LONG lUserID, LPHB_NET_LOGFINDCOND lpFind);

typedef struct ST_HB_NET_FINDLOG
{
    DWORD   dwSize;
    DWORD   dwLanguage;                         // 语言(0-中文 1-英文) ---仅T系列可设置
    DWORD*  pEncType;                           // 返回编码格式(1-UTF-8 2-gb2312)---仅NVR有返回
    char*   pLogData;
    DWORD   dwLogBufSize;
    DWORD*  pBytesRet;
    DWORD   dwRes;
}HB_NET_FINDLOG, *LPHB_NET_FINDLOG;

LONG HB_NET_FindNextLog(LONG lLogHandle, LPHB_NET_FINDLOG lpLog);

BOOL HB_NET_CloseFindLog(LONG lLogHandle);

// 恢复默认设置
BOOL HB_NET_RestoreConfig(LONG lUserID);

// 保存参数
BOOL HB_NET_SaveConfig(LONG lUserID);

// 重启
BOOL HB_NET_Reboot(LONG lUserID);

// 关闭
BOOL HB_NET_ShutDown(LONG lUserID);

// 远程格式化硬盘
LONG HB_NET_FormatDisk(LONG lUserID, LONG lDiskNum);

typedef struct ST_HB_NET_FORMATINFO
{
    DWORD   dwSize;
    DWORD*  pCurFormatDisk;
    DWORD*  pCurDiskPos;
    DWORD*  pFormatStat;
    DWORD   dwRes;
}HB_NET_FORMATINFO, *LPHB_NET_FORMATINFO;

BOOL HB_NET_GetFormatProgress(LONG lFormatHandle, LPHB_NET_FORMATINFO lpInfo);

BOOL HB_NET_CloseFormat(LONG lFormatHandle);

// 导入导出配置文件

#define HB_IMPORT_OK           0x01     // 导入成功
#define HB_TRANS_FILE_ERR      0x02     // 数据传输错误
#define HB_FILE_VERSION_ERR    0x03     // 文件版本错误

typedef struct ST_HB_NET_PARAMFILE_STAT
{
    DWORD dwSize;
    DWORD dwFileSize;
    DWORD dwGotSize;
    DWORD dwStatus;     // 导入状态码，仅导入时有效
    DWORD dwReserve[4];
}HB_NET_PARAMFILE_STAT, *LPHB_NET_PARAMFILE_STAT;

LONG HB_NET_ExpParamFile(LONG lUserID, char* pSaveFile);

BOOL HB_NET_GetParamFileExpStat(LONG lExpHandle, LPHB_NET_PARAMFILE_STAT lpExpStat);

BOOL HB_NET_CloseExpParamFile(LONG lExpHandle);

LONG HB_NET_ImpParamFile(LONG lUserID, char* pImpFile);

BOOL HB_NET_GetParamFileImpStat(LONG lImpHandle, LPHB_NET_PARAMFILE_STAT lpImpStat);

BOOL HB_NET_CloseImpParamFile(LONG lImpHandle);

//////////////////////////////////////////////////////////////////////////
// 语音对讲

typedef struct ST_HB_NET_VOICECOMCALLBACKDATA
{
    DWORD   dwSize;
    char*   pBuffer;
    DWORD   dwDataSize;
    DWORD   dwAudioFlag;
    void*   pContext;
    DWORD   dwRes;
}HB_NET_VOICECOMCALLBACKDATA, *LPHB_NET_VOICECOMCALLBACKDATA;

typedef void (CALLBACK* PHB_NET_VOICEDATA_PROC)(LONG lVoiceComHandle, 
                                                LPHB_NET_VOICECOMCALLBACKDATA lpData);

typedef struct ST_HB_NET_VOICECOMPARAM
{
    DWORD   dwSize;
    PHB_NET_VOICEDATA_PROC pfnCallback;
    void*   pContext;
    DWORD   dwRes;
}HB_NET_VOICECOMPARAM, *LPHB_NET_VOICECOMPARAM;

LONG HB_NET_StartVoiceCom(LONG lUserID, LPHB_NET_VOICECOMPARAM lpParam);

LONG HB_NET_StartVoiceComMR(LONG lUserID, LPHB_NET_VOICECOMPARAM lpParam);

BOOL HB_NET_VoiceComSendData(LONG lVoiceComHandle, char* pBuffer, DWORD dwSize);

BOOL HB_NET_StopVoiceCom(LONG lVoiceComHandle);

// 语音广播
BOOL HB_NET_ClientAudioStart();

BOOL HB_NET_ClientAudioStop();

LONG HB_NET_AddDev(LONG lUserID);

BOOL HB_NET_DelDev(LONG lVoiceHandle);

//////////////////////////////////////////////////////////////////////////
// 报警
#define COMM_ALARM                  0x1100  //报警信息
#define COMM_CONNECT                0x1200  //主机网络断开
#define MAX_DISKNUM                 16

typedef struct ST_HB_NET_ALARMOUT
{
    BYTE byAlarm[HB_MAX_CHANNUM];        //探头报警  0-无报价  1-有报警
    BYTE byVlost[HB_MAX_CHANNUM];       //视频丢失  ...
    BYTE byMotion[HB_MAX_CHANNUM];      //移动报警  ...
    BYTE byHide[HB_MAX_CHANNUM];        //遮挡报警  ...
    BYTE byDisk[MAX_DISKNUM];       //硬盘状态
}HB_NET_ALARMOUT,*LPHB_NET_ALARMOUT;

typedef struct ST_HB_NET_ALARM
{
    DWORD   dwSize;
    char *sDVRIP;
    LONG lUserID;
    char*   pBuffer;    //为LPHB_NET_ALARMOUT指针
    DWORD   dwDataSize;
    void*   pContext;
    DWORD dwRes;
}HB_NET_ALARM,*LPHB_NET_ALARM;
typedef void (CALLBACK* PHB_NET_ALARM_PROC)(LONG lCommand, LPHB_NET_ALARM lpAlarmInfo);

typedef struct ST_HB_NET_ALARMPARAM
{
    DWORD dwSize;
    PHB_NET_ALARM_PROC pfnCallback;
    void* pContext;
    DWORD dwRes;
}HB_NET_ALARMPARAM, *LPHB_NET_ALARMPARAM;

BOOL HB_NET_SetDevAlarmCallback(LPHB_NET_ALARMPARAM lpParam);

// 回调
//void HB_NET_SetDevMessCallBack();

// 消息
//void HB_NET_setDevMessage();

//////////////////////////////////////////////////////////////////////////
// 透明通道

typedef struct ST_HB_NET_SERIALCALLBACKDATA
{
    DWORD   dwSize;
    char*   pBuffer;
    DWORD   dwDataSize;
    void*   pContext;
    DWORD   dwRes;
}HB_NET_SERIALCALLBACKDATA, *LPHB_NET_SERIALCALLBACKDATA;

typedef void (CALLBACK* PHB_NET_SERIALDATA_PROC)(LONG lSerialHandle, 
                                                LPHB_NET_SERIALCALLBACKDATA lpData);

typedef struct ST_HB_NET_SERIALPARAM
{
    DWORD   dwSize;
    DWORD   dwOpType;                               // 0-设置本地 1-前端设备
    DWORD   dwSerialType;                           // 1:232 2:485
    DWORD   dwChnl;                                 // 通道号
    PHB_NET_SERIALDATA_PROC pfnCallback;
    void*   pContext;
    DWORD   dwRes;
}HB_NET_SERIALPARAM, *LPHB_NET_SERIALPARAM;

LONG HB_NET_SerialStart(LONG lUserID, LPHB_NET_SERIALPARAM lpParam);

BOOL HB_NET_SerialSend(LONG lSerialHandle, char* pSendData, DWORD dwSize);

BOOL HB_NET_Stop(LONG lSerialHandle);

//////////////////////////////////////////////////////////////////////////
// 智能设备

//一个点的坐标
typedef struct
{
    USHORT	x;                                      //横坐标
    USHORT	y;                                      //纵坐标
}HB_NET_IVA_POINT, *LPHB_NET_IVA_POINT;

// 客流统计参数
typedef struct ST_HB_NET_HDC_CTRLPARAM
{
    DWORD           dwWidth;                        // 处理视频的宽度，默认值352
    DWORD           dwHeight;                       // 处理视频高度，默认值288
    DWORD           objWidth;                       // 单个目标的宽度，单位为像素，默认值55
    HB_NET_IVA_POINT ptStart;                       // 检测线起点，默认值（5,216)
    HB_NET_IVA_POINT ptEnd;                         // 检测线终点，默认值(347,216)
    HB_NET_IVA_POINT ptDirection;                   // 检测线的方向，默认值(290, 205)
    DWORD           dwPassFrames;       // 初始化的单目标在合成图中的高度，
                                        // 即目标通过检测线的帧数，默认值15
    DWORD           dwMutiObjWidth;     // 三个以上目标并行时矩形框的宽度，默认值110
    DWORD           dwMutiObjWidthEdge; // 与dwMutiObjWidth有关，默认值25，
                                        // dwMutiObjWidthEdge=（dwMutiObjWidth / 2 - 5）/ 2
    DWORD           dwThreshBackDiff;   // 背景差阀值，默认值50，比较敏感
    DWORD           dwThreshFrameDiff;  // 帧间差阀值，默认值20
    BYTE            byStartPtLabel;     // 起点检测标记，0表示任何目标均计数，
                                        // 1表示小于阀值的目标不计数，默认值为0
    BYTE            byEndPtLable;       // 终点检测标记，0表示任何目标均计数，
                                        // 1表示小于阀值的目标不计数，默认值为0
    BYTE            byReserve[2];      // 保留字段
    DWORD           dwHalfObjW;         // 阀值，与前两项相关，宽度小于该阀值不计数，默认值为20
}HB_NET_HDC_CTRLPARAM, *LPHB_NET_HDC_CTRLPARAM;

/*****************************************************************

以下为四路智能的报警通信结构

*****************************************************************/
//矩形坐标
typedef struct
{
    int left;                                       // 矩形左坐标
    int top;                                        // 矩形上坐标
    int right;                                      // 矩形右坐标
    int bottom;                                     // 矩形下坐标
}HB_NET_IVA_RECT, *LPHB_NET_IVA_RECT;

//报警类型及位置信息
typedef struct
{
    int alarm_type;                                 // 类型,GUI_ATMI_ALARM_TYPE_E
    HB_NET_IVA_RECT position;                       // 坐标位置
}HB_NET_IVA_ALARM_POSITION, *LPHB_NET_IVA_ALARM_POSITION;

// 1.人脸通道报警结构体
typedef struct
{
    int alarm_num;                                  // 报警个数
    HB_NET_IVA_ALARM_POSITION alarm_area[10];       // 报警坐标值,一共有alarm_num个，后面的全为0
}HB_NET_IVA_FACE_ALARM, *LPHB_NET_IVA_FACE_ALARM;

// 2.面板通道报警结构体
typedef struct
{
    int alarm_num;                                  // 报警个数
    HB_NET_IVA_ALARM_POSITION alarm_area[10];       // 报警坐标值,一共有alarm_num个，后面的全为0
}HB_NET_IVA_PANEL_ALARM, *LPHB_NET_IVA_PANEL_ALARM;

// 3.加钞间检测输出信息
typedef struct
{
    int type;                                       // 是否有人闯入，0表示无，1表示有
}HB_NET_IVA_MONEY_ALARM, *LPHB_NET_IVA_MONEY_ALARM;

// 4.环境报警结构体,alarm_num所对应的区域在前，track_num所对应的区域紧跟在alarm_num区域后
typedef struct
{
    int alarm_num;                                  // 报警目标数量
    int track_num;                                  // 跟踪目标数量
    HB_NET_IVA_ALARM_POSITION envi_alarm_region[25];
}HB_NET_IVA_ENVI_ALARM, *LPHB_NET_IVA_ENVI_ALARM;

typedef enum
{
    HB_NET_IVA_FACE_BLOCK = 0,                      // 人脸遮挡
    HB_NET_IVA_FACE_NOSIGNAL,                       // 有脸通道视频丢失
    HB_NET_IVA_FACE_UNUSUAL,                        // 人脸异常
    HB_NET_IVA_FACE_NORMAL,                         // 人脸正常
    HB_NET_IVA_PANEL_BLOCK  = 40,                   // 面板遮挡
    HB_NET_IVA_PANEL_NOSIGNAL,                      // 面板通道视频丢失
    HB_NET_IVA_PANEL_PASTE,                         // 贴条
    HB_NET_IVA_PANEL_KEYBOARD,                      // 装假键盘
    HB_NET_IVA_PANEL_KEYMASK,                       // 破坏密码防护罩	
    HB_NET_IVA_PANEL_CARDREADER,                    // 破坏读卡器
    HB_NET_IVA_PANEL_TMIEOUT,                       // 超时报警
    HB_NET_IVA_ENTER,                               // 有人进入
    HB_NET_IVA_EXIT,                                // 人撤离
    HB_NET_IVA_MONEY_BLOCK = 80,                    // 加钞间视频遮挡
    HB_NET_IVA_MONEY_NOSIGNAL,                      // 加钞间通道视频丢失
    HB_NET_IVA_MONEY_UNUSUAL,                       // 加钞间异常,即有人闯入加钞间
    HB_NET_IVA_ENVI_BLOCK = 120,                    // 环境通道视频遮挡
    HB_NET_IVA_ENVI_NOSIGNAL,                       // 环境通道视频丢失
    HB_NET_IVA_ENVI_GATHER,                         // 多人聚集
    HB_NET_IVA_ENVI_MANYPEOPLEINREGION,             // 违规取款
    HB_NET_IVA_ENVI_LINGERING,                      // 人员徘徊
    HB_NET_IVA_ENVI_LIEDOWN,                        // 人员倒地
    HB_NET_IVA_ENVI_FORBIDREGION,                   // 非法进入警戒区
    HB_NET_IVA_ENVI_LEAVEOBJECT,                    // 物品遗留
}HB_NET_IVA_ALARM_TYPE_E;

//报警图片结构体
typedef struct
{
    DWORD pic_alarm_type;                           // HB_NET_IVA_ALARM_TYPE_E; 
    DWORD pic_format;                               // 图片格式CIF:0  D1:1
    DWORD pic_size;
}HB_NET_IVA_ALARM_PICINFO, *PHB_NET_IVA_ALARM_PICINFO;

typedef enum
{
    HBGK_HDCCOUNT_DIR1 = 0,	                        // 与标记方向相同
    HBGK_HDCCOUNT_DIR2		                        // 与标记方向相反
}HB_NET_HDCCOUNT_DIRECTION_E;

typedef struct ST_HB_NET_HDC_RESULT
{
    DWORD dwResultType;                             // 输出结果总类型
    DWORD dwSubType;                                // 输出结果子类型，表示人员流动统计的方向见HB_NET_HDCCOUNT_DIRECTION_E
    DWORD dwTrackNum;                               // 当前输出统计的ID编号(已统计人数)
    HB_NET_IVA_RECT rcPos;                          // 当前输出编号的外接矩形框
}HB_NET_HDC_RESULT, *LPHB_NET_HDC_RESULT;

// 系统时间定义
typedef struct 
{
    unsigned int sec : 6;
    unsigned int min : 6;
    unsigned int hour : 5;
    unsigned int day : 5;
    unsigned int month : 4;
    unsigned int year : 6;
} ASYSTIME, *LPASYSTIME;	

//报警时主机传给客户端的总结构体
typedef struct
{
    // int chn;    //通道号,每次报警后，根据通道号，去读取下面四个结构体中对应的那一个
    BYTE byChn;
    BYTE byRes1;
    BYTE byInfoType;        // 上传信息类型
    // 0, HB_NET_IVA_FACE_ALARM_S
    // 1, HB_NET_IVA_PANEL_ALARM_S
    // 2, HB_NET_IVA_MONEY_ALARM_S
    // 3, HB_NET_IVA_ENVI_ALARM_S
    // 4, PT_HDC_RESULT
    BYTE byRes2;

    union
    {
        HB_NET_IVA_FACE_ALARM   face;               // 1.人脸通道报警结构体
        HB_NET_IVA_PANEL_ALARM  panel;              // 2.面板通道报警结构体
        HB_NET_IVA_MONEY_ALARM  money;              // 3.加钞间通道报警结构体
        HB_NET_IVA_ENVI_ALARM   envi;               // 4.环境通道报警结构体
        HB_NET_HDC_RESULT       hdc;
    }info;

    HB_NET_IVA_ALARM_PICINFO pic;
    ASYSTIME time;                                  // 报警时间
}HB_NET_IVA_ALARM_INFO, *LPHB_NET_IVA_ALARM_INFO;
/*****************************************************************

以下为四路智能的设置或获取的界面结构

*****************************************************************/
//多边形表示结构体，带区域类型
typedef struct
{
    HB_NET_IVA_POINT    point[10];                  // 多边形顶点坐标
    int pointNum;                                   // 点的个数
    int regionType;                                 // 区域类型
}HB_NET_IVA_POLYGON, *LPHB_NET_IVA_POLYGON;

//矩形区域，带区域类型
typedef struct
{
    HB_NET_IVA_RECT region;                         // 矩形区域坐标
    int region_type;                                // 区域类型
}HB_NET_IVA_RECT_TYPE, *LPHB_NET_IVA_RECT_TYPE;

//人脸感兴趣区域以及该区域中人脸的大小
typedef struct
{
    HB_NET_IVA_RECT_TYPE roi;                       // 坐标
    int min_face;                                   // 最小尺寸
    int max_face;                                   // 最大尺寸
}HB_NET_IVA_FACE_ROI, *LPHB_NET_IVA_FACE_ROI;

// 1.人脸通道中所设置的区域
typedef struct
{
    int num;
    HB_NET_IVA_FACE_ROI face_roi[10];
}HB_NET_IVA_FACEROI_ALL, *LPHB_NET_IVA_FACEROI_ALL;

// 2.面板通道中所设置的区域
typedef struct
{
    int num;
    HB_NET_IVA_POLYGON atmi_panel_region[20];
}HB_NET_IVA_PANEL_REGION, *LPHB_NET_IVA_PANEL_REGION;

// 3.加钞间通道中所设置的区域及参数
typedef struct
{
    HB_NET_IVA_POLYGON pol_proc_region;             // 处理区域，默认4个点，包含全图
    HB_NET_IVA_RECT_TYPE no_process[10];            // 不处理区域
    int no_process_num;                             // 不处理区域个数 (0)
    int warn_interval;                              // 两次报警时间间隔，(100 秒)
}HB_NET_IVA_DISTRICTPARA, *LPHB_NET_IVA_DISTRICTPARA;	

// 4.场景通道中所设置的区域
typedef	struct
{
    HB_NET_IVA_POLYGON pol_proc_region;             // 图像中的处理区域，多边形表示

    /*用于ATM机前尾随取款检测的参数，标识ATM前人站立的区域*/
    HB_NET_IVA_POLYGON tail_region[10];             // Region in polygon.
    int tail_num;                                   // Region number. default: 0

    /*用于禁止区域进入报警，标识选定的禁止进入的区域*/
    HB_NET_IVA_POLYGON forbid_region[10];           // Region in polygon.
    int forbid_num;                                 // Region number.	default: 0

    HB_NET_IVA_POLYGON obnmp_height;                  // 目标（人）在图像中的高度，默认85
}HB_NET_IVA_SCENE_COMMONPARA, *LPHB_NET_IVA_SCENE_COMMONPARA;

// 5.环境通道设置的参数,以下以帧为单位的，我们在界面上做为秒，然后在内部再转化为帧数
typedef struct
{
    /*物品遗留算法相关参数*/
    int objlv_frames_th;                            // 物品遗留时间阈值(帧) (30)

    /*人员徘徊算法相关参数*/
    int mv_block_cnt;                               // 移动距离(20，0表示此规则无效)
    SHORT mv_stay_frames;                           // 场景中出现时间阈值(帧),存在总时间(0表示此规则无效)
    SHORT mv_stay_valid_frames;                     // ATM区域停留时间阈值(帧),ATM区域前停留时间(200, 0表示此规则无效)

    /*多人聚集算法相关参数*/
    SHORT gather_cnt;                               // 最多聚集人数(4)
    SHORT gather_interval_frames;                   // 报警间隔时间(帧)(1000 frames,约100秒)
    int gather_frame_cnt;                           // 多人聚集时间阈值(帧) (100)

    /*人员躺卧算法相关参数*/
    int liedown_frame_cnt;                          // 躺卧时间阈值(帧).(20 frames)

    /*尾随取款算法相关参数*/
    SHORT after_frame_cnt;                          // 可疑行为时间阈值(帧)(20 frames)
    SHORT after_interval_frames;                    // 报警间隔时间(帧)(1000 frames,约100秒)

    /*禁止进入算法相关参数*/
    SHORT forbid_frame_cnt;                         // 禁止站立时间阈值(帧)(20 frames)
    SHORT reserve;                                  // 保留
}HB_NET_IVA_SCENE_WARN_PARAM, *LPHB_NET_IVA_SCENE_WARN_PARAM;

// 1.人脸通道设置结构体
typedef struct
{
    SHORT face_unusual;                             // 是否打开异常人脸（戴口罩、蒙面等）检测功能，1 为打开，0 为关闭。默认为 0  
    SHORT output_oneface;                           // 设置人脸只输出一次与否，0为否，1为是，默认为1
    int fd_rate;                                    // 设置人脸检测跟踪间隔
    HB_NET_IVA_FACEROI_ALL face_roi;                // 人脸通道的区域及其它参数
}HB_NET_IVA_SET_FACE, *LPHB_NET_IVA_SET_FACE;

// 2.面板通道设置结构体
typedef struct
{
    int timeout_enable;                             // 超时时间
    HB_NET_IVA_PANEL_REGION panel_region;           // 面板通道区域及其它参数
}HB_NET_IVA_SET_PANEL, *LPHB_NET_IVA_SET_PANEL;

// 3.加钞间通道设置结构体
typedef struct
{
    HB_NET_IVA_DISTRICTPARA money_para;             // 加钞间通道区域及其它参数
}HB_NET_IVA_SET_MONEY, *LPHB_NET_IVA_SET_MONEY;

// 4.环境通道设置结构体
typedef struct
{
    HB_NET_IVA_SCENE_WARN_PARAM envi_para;          // 环境通道参数
    HB_NET_IVA_SCENE_COMMONPARA envi_region;        // 环境通道区域
}HB_NET_IVA_SET_ENVI, *LPHB_NET_IVA_SET_ENVI;

//客户端设置或获取到主机四路智能总的结构体
typedef struct ST_HB_NET_IVACFG
{
    // int chn;                                     // 通道号
    BYTE byChn;                                     // 通道号
    BYTE byRes1;                                    // 保留
    BYTE bySetInfoType;                             // 设置参数类型，
    // 0, HB_NET_IVA_SET_FACE_S;
    // 1, HB_NET_IVA_SET_PANEL_S;
    // 2, HB_NET_IVA_SET_MONEY_S;
    // 3, HB_NET_IVA_SET_ENVI_S;
    // 4, HB_NET_HDC_CTRLPARAM;
    BYTE byRes2;                                    // 保留

    int chn_attri;                                  // 通道属性(人脸、面板、加钞、环境)，目前未用，防止以后用
    SHORT channel_enable;                           // 通道开关，0关闭，1打开
    SHORT if_pic;                                   // 是否需要抓取图片
    SHORT enc_type;                                 // 抓取图片的格式
    SHORT email_linkage;                            // 联动email
    UINT sensor_num;                                // 探头输出,位表示
    UINT rec_linkage;                               // 联动录像，位表示

    union
    {
        HB_NET_IVA_SET_FACE  face_set_para;         // 人脸通道设置结构体
        HB_NET_IVA_SET_PANEL panel_set_para;        // 面板通道设置结构体
        HB_NET_IVA_SET_MONEY money_set_para;        // 加钞间通道设置结构体
        HB_NET_IVA_SET_ENVI  envi_set_para;         // 环境通道设置结构体
        HB_NET_HDC_CTRLPARAM    hdc;                // 人流统计参数设置
    }setInfo;
}HB_NET_IVACFG, *LPHB_NET_IVACFG;

typedef struct ST_HB_NET_IVACALLBACKDATA
{
    DWORD   dwSize;
    char*   pBuffer;                                // HB_NET_IVA_ALARM_INFO + pic
    DWORD   dwDataSize;
    void*   pContext;
    DWORD   dwRes;
}HB_NET_IVACALLBACKDATA, *LPHB_NET_IVACALLBACKDATA;

typedef void (CALLBACK* PHB_NET_IVADATA_PROC)(LONG lIVAHandle, LPHB_NET_IVACALLBACKDATA lpData);

typedef struct ST_HB_NET_IVAPARAM
{
    DWORD   dwSize;
    PHB_NET_IVADATA_PROC pfnCallback;
    void*   pContext;
    DWORD   dwRes;
}HB_NET_IVAPARAM, *LPHB_NET_IVAPARAM;

LONG HB_NET_IVAStart(LONG lUserID, LPHB_NET_IVAPARAM lpParam);

BOOL HB_NET_IVAStop(LONG lIVAHandle);

BOOL HB_NET_GetIVAConfig(LONG lIVAHandle, LPHB_NET_IVACFG lpCfg, DWORD* pBytesRet);

BOOL HB_NET_SetIVAConfig(LONG lIVAHandle, LPHB_NET_IVACFG lpCfg);

BOOL HB_NET_ResetIVAEnvi(LONG lIVAHandle);


// Add @2013/4/15 by WangZhen
typedef struct ST_HB_NET_ACTIVE_DEVICEINFO
{
    char sDVRID[HB_SERIALNO_LEN];  
    char sSerialNumber[HB_SERIALNO_LEN]; 
    BYTE byAlarmInPortNum;
    BYTE byAlarmOutPortNum; 
    BYTE byDiskNum;  
    BYTE byProtocol;             
    BYTE byChanNum;   
    BYTE byEncodeType; 
    BYTE reserve[26];    
    char sDvrName[HB_NAME_LEN];     
    char sChanName[HB_MAX_CHANNUM][HB_NAME_LEN];  
}HB_NET_ACTIVE_DEVICEINFO, *LPHB_NET_ACTIVE_DEVICEINFO;

typedef BOOL (CALLBACK* PHB_NET_LOGIN_PROC)(LONG lUserID, char* sDevIP, 
    LPHB_NET_ACTIVE_DEVICEINFO pDevInfo, void* pContext);

typedef struct ST_HB_NET_LISTENPARAM
{
    WORD                wPort;
    PHB_NET_LOGIN_PROC  pfnLogin;
    void*               pContext;
}HB_NET_LISTENPARAM, *LPHB_NET_LISTENPARAM;

BOOL HB_NET_StartListen(LPHB_NET_LISTENPARAM pListen);

typedef struct ST_HB_NET_ACTIVE_REALPLAYCON
{
    DWORD               dwSize;
    DWORD               dwMsgID;
    DWORD               dwChl;
    DWORD               dwStreamType;   // 0-主码流 1-子码流
    DWORD               dwLinkMode;     // 0-TCP 1-UDP
    DWORD               dwMultiCast;    // 是否多播
    DWORD               dwOSDScheme;    // osd字符编码格式 
    DWORD               dwMultiCastIP;  // 多播IP地址
    DWORD               dwPort;         // 多播端口
    PHB_NET_STREAMDATA_PROC pfnDataProc;
    void*               pContext;
    DWORD               dwReserver[4];
}HB_NET_ACTIVE_REALPLAYCON, *LPHB_NET_ACTIVE_REALPLAYCON;

LONG HB_NET_ActiveRealPlay(LONG lUserID, LPHB_NET_ACTIVE_REALPLAYCON pRealPlay);

typedef struct ST_HB_NET_ACTIVE_PLAYBACKCOND
{
    DWORD               dwSize;
    DWORD               dwMsgID;
    DWORD               dwChl;      // 通道号
    HB_NET_RECTYPE_E    dwFileType;     // 文件类型
    HB_NET_TIME         struStartTime;  // 下载时间段开始时间
    HB_NET_TIME         struStopTime;   // 结束时间
    PHB_NET_STREAMDATA_PROC     pfnDataProc;
    void*               pContext;
    DWORD               dwResever[4];
}HB_NET_ACTIVE_PLAYBACKCOND, *LPHB_NET_ACTIVE_PLAYBACKCOND;

LONG HB_NET_ActivePlayBack(LONG lUserID, LPHB_NET_ACTIVE_PLAYBACKCOND pPlayBack);

typedef struct ST_HB_NET_ACTIVE_FILEGETCOND
{
    DWORD               dwSize;
    DWORD               dwMsgID;
    DWORD               dwChl;      // 通道号
    HB_NET_RECTYPE_E    dwFileType;     // 文件类型
    HB_NET_TIME         struStartTime;  // 下载时间段开始时间
    HB_NET_TIME         struStopTime;   // 结束时间
    PHB_NET_STREAMDATA_PROC     pfnDataProc;
    void*               pContext;
    char*               pSaveFilePath;
    DWORD               dwResever[4];;
}HB_NET_ACTIVE_FILEGETCOND, *LPHB_NET_ACTIVE_FILEGETCOND;

LONG HB_NET_ActiveGetFile(LONG lUserID, LPHB_NET_ACTIVE_FILEGETCOND pGetFile);

typedef struct ST_HB_NET_ACTIVE_GETPICCOND
{
    DWORD               dwSize;
    DWORD               dwMsgID;
    DWORD               dwChl;
    DWORD               dwFormat;
    char*               pSaveFileName;
}HB_NET_ACTIVE_GETPICCOND, *LPHB_NET_ACTIVE_GETPICCOND;

BOOL HB_NET_ActiveGetPic(LONG lUserID, LPHB_NET_ACTIVE_GETPICCOND pGetPic);

typedef struct ST_HB_NET_ACTIVE_VOICECOMCOND
{
    DWORD               dwSize;
    DWORD               dwMsgID;
    PHB_NET_VOICEDATA_PROC  pfnDataProc;
    void*               pContext;
}HB_NET_ACTIVE_VOICECOMCOND, *LPHB_NET_ACTIVE_VOICECOMCOND;

LONG HB_NET_ActiveVoiceCom(LONG lUserID, LPHB_NET_ACTIVE_VOICECOMCOND pVoice);

#pragma pack()
#endif
