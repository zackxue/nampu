
#ifndef _SN_STURCT_C_
#define _SN_STURCT_C_

#include "DomainConst.h"
#include "SNPlatOS.h"

//#pragma pack(push, 1)
/**********************************************************************/
//此处用于控制文件编译字节对齐，拷贝时两行注释间内容需一起拷贝，
//结束处的“#ifdef PRAGMA_PACK”部分也要一起拷贝，否则pragma pack入栈出栈不匹配
#if(PRAGMA_PACK_DEFINE != 10000)
	# error Not included "SNPlatOS.h".
#endif
 
#ifdef PRAGMA_PACK
	#ifdef WIN32  
		#pragma pack(push, PRAGMA_PACK_CHAR)
	#endif

	#ifndef WIN32  
		#ifndef _PACKED_1_  
		  
		#define _PACKED_1_ __attribute__((packed, aligned(PRAGMA_PACK_CHAR)))	// for gcc   
		#endif  
	#else  
		#ifndef _PACKED_1_  
		#define _PACKED_1_  
		#endif  
	#endif  
#else  
	#ifndef _PACKED_1_  
	#define _PACKED_1_  
	#endif  
#endif
/**********************************************************************/


namespace NVDC_STRUCT
{
	const int  CONST_MAX_RS484_NUM = 16;							//允许的最大RS485个数
	const int  CONST_MAX_PERIOD_SCHEDULE_RECORD_TIME_NUM = 3;		//允许的最大计划录像设定的单日时间段数
	const int  CONST_MAX_ALARM_OUT_NUM = 16;						//允许的最大报警输出通道个数	
	const int  CONST_MAX_ALARM_IN_NUM = 16;							//允许的最大报警输入通道个数
	const int  CONST_MAX_CAMERA_NUM = 16;							//允许的最大摄像头数目
	const int  CONST_MAX_PTZ_PRESET_NUM = 255;						//允许的最大云台预置点数目
	const int  CONST_MAX_DETECTION_NUM = 8;							//允许的最大侦测区域数
	const int  CONST_MAX_DISK_NUM = 16;								//允许的最大磁盘个数

	const int  CONST_MAX_PTZ_PROTOCOL_NUM = 8;						//允许的最大云台协议数
	const int  CONST_MAX_DEVICE_TYPE_NUM = 8;						//允许的最大的设备类型数
	const int  CONST_MAX_BITRATE_TYPE_NUM = 8;						//允许的最大比特率类型数目
	const int  CONST_MAX_DEVICE_IMAGE_FORMAT_NUM = 8;				//允许的设备支持的图像格式最大数目
	const int  CONST_MAX_DDNS_PROVIDER_NUM = 8;						//允许的最大的DDNS提供者数目
	const int  CONST_MAX_ALARM_TYPE_NUM = 8;						//允许的最大的报警类型数目
	const int  CONST_MAX_VIDEO_SYSTEM_NUM = 8;						//允许的最大的视频制式种类
	const int  CONST_MAX_FRAME_LENGTH = 1600*1200;					//一帧数据的最大值
	const int  CONST_MAX_ALARM_ACTION_STRATEGY = 5;					//报警行为策略最大个数
	const int  CONST_MAX_STREAM_NUM = 4*16;							//允许的设备最大流数目
	const int  CONST_MAX_ALARM_EVENT_NUM = 36;						//允许的最大报警事件数
	
	const int  CONST_MAX_AREA_NUM = 3;								//允许的最大区域数

	const int  CONST_MAX_FILE_SYSTEM_NUM = 6;						//允许的最大文件系统数目
	const int  CONST_MAX_DEVICE_VIDEO_ENCODER_NUM = 4;				//允许的最大视频编码器类型数目
	const int  CONST_MAX_AUDIO_TYPE_NUM = 16;						//允许的最大音频输入类型数
	const int  CONST_MAX_AUDIO_ENCODE_TYPE_NUM = 16;				//允许的最大音频编码类型数
	const int  CONST_MAX_AUDIO_DECODE_TYPE_NUM = 16;				//允许的最大音频解码类型数
	const int  CONST_MAX_SOURCE_RESOLUTION_NUM = 4;					//允许的最大源分辨率配置数目
	const int  CONST_MAX_CAMERA_STREAM_NUM = 6;						//允许的摄像头最大流数目
	const int  CONST_MAX_DISK_TYPE_NUM = 6;							//允许的最大磁盘类型数目

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	//Ip传输协议
	enum ENUM_TransferProtocol
	{
		UDP = 0X01,			//UDP协议
		TCP = 0X02,			//TCP协议
		RTP = 0X04			//RTP协议
	};

	//Ip协议版本
	enum ENUM_IPProtoVer	
	{
		IPPROTO_V4  = 1,	//IPv4协议

		IPPROTO_V6  = 2		//IPv6协议
	};

	//设备类型
	enum ENUM_DeviceType	
	{
		IPCAMERA	= 1,	//IP Camera
		DVR			= 2,	//DVR
		DVS			= 3,	//DVS
		IPDOME		= 4,	//IP Dome
		NVR			= 5		//NVR
	};

	//水印类型
	enum ENUM_OSDType
	{
		OSDTYPE_DEVICENAME	= 1,	//设备名
		OSDTYPE_CAMERAID	= 2,	//摄像机号
		OSDTYPE_CAMERANAME	= 3,	//摄像机名
		OSDTYPE_TIME		= 4,	//时间水印
		OSDTYPE_TEXT		= 5		//文字水印
	};

	//图像格式类型
	enum ENUM_ImageFormat
	{
		IMAGEFORMAT_D1					= 1,	//PAL 704*576,25frame； NTSC 704*480 30framre
		IMAGEFORMAT_LOW_BITRATE_D1		= 2,	//PAL 704*576 12.5frame；NTSC 704*480 15framre
		IMAGEFORMAT_CIF					= 4,	//PAL 352*288 25frame；NTSC 352*240 30framre
		IMAGEFORMAT_QCIF				= 5,	//PAL 176*144 25frame；NTSC 240*160 30framre
		IMAGEFORMAT_SXGA				= 6,	//NA 1280*960 1-22frame
		IMAGEFORMAT_QVGA				= 7,	//NA 320*240 1-22frame
		IMAGEFORMAT_1280_720			= 8,	//NA 1280*720 1-25frame
		IMAGEFORMAT_360_160				= 9,	//NA 320*160 1-25frame
		IMAGEFORMAT_640_360				= 10,	//NA 640*368 1-25frame
		IMAGEFORMAT_VGA					= 11,	//NA 640*480 1-25frame
		IMAGEFORMAT_UXGA				= 12,	//NA 1600*1200 1-25frame
		IMAGEFORMAT_1920_1080			= 13,	//NA 1920*1080 1-25frame
		IMAGEFORMAT_640_360_EX			= 14	//NA 640*360 1-25frame
	} ;

	//比特率类型
	enum ENUM_BitRateType
	{
		CBR_TYPE = 1,					//固定码率

		VBR_TYPE = 2					//不固定码率
	};

	//录像状态
	enum ENUM_RecordStatus
	{
		RECORDSTATUS_NO_RECORD	= 0,	//无录制
		RECORDSTATUS_RECORD		= 1		//有录制
	};

	//信号状态
	enum ENUM_SignalStatus
	{
		SIGNALSTATUS_HAS_SIGNAL	  = 0,	//信号正常
		SIGNALSTATUS_LOSS_SIGNAL  = 1	//信号丢失
	};

	//硬件状态
	enum ENUM_HardwareStatus
	{
		HARDWARESTATUS_NORMAL	  = 0,	//硬件正常
		HARDWARESTATUS_ABNORMAL	  = 1	//硬件错误
	};

	//磁盘状态
	enum ENUM_DiskStatus
	{
		DISKSTATUS_DORMANCY		  = 1,	//正常状态
		DISKSTATUS_ABNORMAL		  = 2,	//不正常状态
		DISKSTATUS_NOT_EXISTENT	  = 3,	//磁盘不存在
		DISKSTATUS_WRITE_PROTECT  = 4,	//磁盘写保护	
		DISKSTATUS_NOT_FORMAT	  = 5,	//磁盘未格式化
		DISKSTATUS_FORMATTING	  = 6	//磁盘正在格式化
	};

	//麦克风类型
	enum ENUM_ToneArmType
	{
		TONEARM_TYPE_INTERNAL				= 1,	//内置
		TONEARM_TYPE_EXTERNAL				= 2,	//外置
		TONEARM_TYPE_LINEIN					= 3,	//线输入
		TONEARM_TYPE_DIFF_LINEIN			= 4,	//差分线输入
		TONEARM_TYPE_DOUBLE_LINEIN			= 5		//双输入
	};

	//扬声器类型
	enum ENUM_LoudhailerType
	{
		LOUDHAILER_TYPE_IN		  = 1,	//内置
		LOUDHAILER_TYPE_OUT		  = 2	//外置
	};

	//485配置流控
	enum ENUM_FlowControl
	{
		FLOWCONTROL_NONE		  = 0,	//无
		FLOWCONTROL_SOFT		  = 1,	//软流控
		FLOWCONTROL_HARD		  = 2	//硬流控
	};

	//奇偶校验
	enum ENUM_COMParity
	{
		COM_PARITY_NONE				= 0,	//None，无校验
		COM_PARITY_ODD				= 1,	//Odd，奇校验
		COM_PARITY_EVEN				= 2,	//Even，偶校验
		COM_PARITY_MARK				= 3,	//Mark，标记校验
		COM_PARITY_SPACE			= 4		//Space，空格校验
	};

	//停止位
	enum ENUM_StopBits
	{
		STOPBITS_ONE				= 0,	//1
		STOPBITS_ONEHALF			= 1,	//1.5
		STOPBITS_TWO				= 2		//2
	};


	//报警有效信号
	enum ENUM_AlarmValidSignal
	{
		ALARM_VALID_SIGNAL_CLOSE  = 0,	//继电器闭合
		ALARM_VALID_SIGNAL_OPEN	  = 1	//继电器断开
	};

	//报警模式
	enum ENUM_AlarmMode
	{
		ALARMMODE_SWITCH		  = 1,	//开关模式
		ALARMMODE_MULTIPLE		  = 2	//方波模式
	};

	//适配器类型
	enum ENUM_AdapterType
	{
		ADAPTER_TYPE_LOCAL		  = 1,	//本地连接
		ADAPTER_TYPE_PPPOE		  = 2	//PPPoE连接
	};

	//DDNS域名提供者ID
	enum ENUM_ProviderId
	{
		PROVIDERID_3322			  = 1,	//3322.org
		PROVIDERID_DYNDNS		  = 2	//dyndns
	};

	//存储策略
	enum ENUM_StoragePolicy
	{
		STORAGE_POLICY_CYCLE	 = 1,	//循环存储
		STORAGE_POLICY_FULLSTOP  = 2	//磁盘满停止
	};

	//录制方式
	enum ENUM_RecordMode
	{
		RECORD_MODE_SIZE = 1,		  //按文件大小录制
		RECORD_MODE_TIME = 2		  //按时间长度录制
	};

	//录像格式类型
	enum ENUM_RecordFormatType	
	{
		RECORD_FORMAT_MPEG2_PS	= 1,			//PS格式
		RECORD_FORMAT_CUSTOM	= 2,			//自定义格式
		RECORD_FORMAT_MPEG2_TS	= 3				//TS格式
	};

	//录像规则
	enum ENUM_RuleType
	{
		RECORD_RULE_TYPE_ALWAYS = 1,			//总是录像
		RECORD_RULE_TYPE_SCHEDULE = 2			//计划录像
	};

	//报警输出标志
	enum ENUM_AlarmOutFlag
	{
		ALARMOUTFLAG_START		= 1,			//报警有效标志位，开始报警
		ALARMOUTFLAG_STOP		= 0				//报警无效标志位，停止报警
	};

	//报警输出状态
	enum ENUM_AlarmOutStatus
	{
		ALARMOUT_NO_ALARMOUT	= 0,			//无报警输出
		ALARMOUT_ALARMOUT		= 1				//有报警输出
	};

	//报警类型
	enum ENUM_AlarmType
	{
		TYPE_ALARMIO					= 1,	//I/O报警
		TYPE_MOTIONDETECT				= 2,	//运动检测报警
		TYPE_CAMERAMASK					= 3,	//摄像头遮挡报警
		TYPE_VIDEO_LOST					= 5,	//视频丢失报警
		TYPE_DISK_ERROR					= 6,	//磁盘读写异常
		TYPE_DISK_CONNECT_FAILED		= 7,	//网络磁盘连接失败
		TYPE_DISK_FULL					= 8,	//磁盘满
		TYPE_DISK_NOT_EXISTENT			= 9,	//磁盘不存在
		TYPE_DISK_THRESHOLD_ALARM		= 10,	//磁盘已用空间达到指定阀值 
		TYPE_DISK_NOT_FORMAT			= 11,	//磁盘未格式化
		TYPE_RECORD_STORAGE_FAILED		= 15    //录像存储失败
	};

	//扩展报警主类型
	enum  ENUM_AlarmMajorType
	{
		SECURITY_ALARM						= 1, //安全报警
		DISK_ALARM							= 4, //磁盘报警
		RECORD_ALARM						= 5  //录像报警。
	};

	//扩展安全报警子类型
	enum  ENUM_SecurityAlarmMinorType
	{
		SECURITYALARM_MINOR_TYPE_ALARMIO		= 1,	//I/O报警
		SECURITYALARM_MINOR_TYPE_MOTIONDETECT	= 2,	//运动检测报警
		SECURITYALARM_MINOR_TYPE_CAMERAMASK		= 3,	//摄像头遮挡报警
		SECURITYALARM_MINOR_TYPE_VIDEO_LOST		= 4		//视频丢失报警
	};

	//扩展磁盘报警子类型
	enum  ENUM_DiskAlarmMinorType
	{
		DISKALARM_MINOR_TYPE_DISK_OK			= 1,	//磁盘状态正常
		DISKALARM_MINOR_TYPE_DISK_ERROR			= 2,	//磁盘读写异常
		DISKALARM_MINOR_TYPE_CONNECT_FAILED		= 3,	//网络磁盘连接失败
		DISKALARM_MINOR_TYPE_DISK_FULL			= 4,	//磁盘满
		DISKALARM_MINOR_TYPE_NOT_EXISTENT		= 5,	//磁盘不存在
		DISKALARM_MINOR_TYPE_THRESHOLD_ALARM	= 6,	//磁盘已用空间达到指定阀值 
		DISKALARM_MINOR_TYPE_NOT_FORMAT			= 7,	//磁盘未格式化
		DISKALARM_MINOR_TYPE_DEVICE_NOSPACE		= 8		//设备存储空间不足
	};

	//扩展录像报警子类型
	enum ENUM_RecordAlarmMinorType
	{
		RECORDSOURCE_MINOR_TYPE_DEVICE_CONNECT_SUCCESS			= 1,		//数据源连接成功
		RECORDSOURCE_MINOR_TYPE_DEVICE_CONNECT_LOGIN_ERROR		= 2,		//数据源连接用户名密码错误
		RECORDSOURCE_MINOR_TYPE_DEVICE_CONNECT_NO_PRIVILEGE		= 3,		//数据源连接没有权限
		RECORDSOURCE_MINOR_TYPE_DEVICE_CONNECT_MAX_CONNECTION	= 4,		//数据源连接达到最大连接数
		RECORDSOURCE_MINOR_TYPE_AVDATA_MAX_SPEED				= 5,		//数据源达到最大限制速率
		RECORDSOURCE_MINOR_TYPE_LOGIN_USER_REPEATED				= 6,
		RECORDSOURCE_MINOR_TYPE_NOT_SUPPORT_VIDEO				= 7,
		RECORDSOURCE_MINOR_TYPE_CREATE_VIDEO_SESSION_FAILED		= 8,
		RECORDSTORAGE_MINOR_TYPE_STORAGE_FAILED					= 9
	};

	enum  ENUM_AlarmFlag
	{
		ALARM_FLAG_STOP = 0,						//报警停止
		ALARM_FLAG_START = 1						//报警开始
	};

	//图片格式
	enum ENUM_PhotoFormat
	{
		PHOTOFORMAT_JPEG	    = 1,			//jpge格式

		PHOTOFORMAT_BMP			= 2				//bmp格式
	};

	//抓拍状态
	enum ENUM_PhoteSnapShotFlag
	{
		PHOTESNAPSHOT_FLAG_NO		= 0,		//没配置抓拍
		PHOTESNAPSHOT_FLAG			= 1			//配置抓拍
	};

	//查询方式
	enum ENUM_RecordFileSelectMode
	{
		SELECT_MODE_ALL		= 0,				//查询所有
		SELECT_MODE_TYPE	= 1,				//按类型查询
		SELECT_MODE_TIME 	= 2					//按时间查询
	};

	//视频制式
	enum ENUM_VideoSystem
	{
		NTSC	= 0,	//PAL制式
		PAL		= 1,	//NTFS制式
		NA		= 2		//
	};

	//录像类型
	enum ENUM_RecordType
	{
		RECORD_TYPE_ALWAYS		= 1,	//总是录像
		RECORD_TYPE_SCHEDULE	= 2,	//计划录像
		RECORD_TYPE_ALARM		= 3		//报警录像
	};

	//打开模式
	enum ENUM_OpenMode
	{
		OPENMODE_SHARE		= 0,		//共享模式打开
		OPENMODE_EXCLUSIVE	= 1			//独占模式打开
	};

	//播放状态
	enum ENUM_PlayStatus
	{
		PLAY_STATUS_STOP = 1,       //停止
		PLAY_STATUS_PAUSE = 2,		//暂停
		PLAY_STATUS_PLAY = 3,		//播放
		PLAY_STATUS_FASTPLAY = 4,	//快进
		PLAY_STATUS_SLOWPLAY = 5,	//慢放
		PLAY_STATUS_FRAME_PLAY = 6, //帧进
		PLAY_STATUS_FRAME_BACK = 7  //帧退

	} ;

	//PTZ协议
	enum  ENUM_PTZProtocol
	{
		PROTOCOL_PELCO_D = 0,		//PELCO_D协议
		PROTOCOL_PELCO_P = 1		//PELCO_P协议
	};

	//录制状态
	enum ENUM_LiveRecordStatus
	{
		RECORD_RUNNING = 1,			//正在录制
		RECORD_STOP	   = 2			//停止录制
	};

	//编码格式
	enum ENUM_EncoderType
	{
		MPEG4		= 0,			//MPEG4编码
		H264		= 1,			//H264编码
		MJPEG		= 2,			//MJPEG编码
		SVC			= 3,			//SVC编码
		G7231		= 101,			//G7231编码
		G711_ALAW	= 102,			//G711A律编码
		G711_ULAW	= 103,			//G711U律编码
		G722		= 104,			//G722编码
		G726		= 105,			//G726编码
		G729		= 106,			//G729编码
		AMR			= 107,			//AMR编码
		RAW_PCM		= 108,			//PCM编码，即不编码
		UNKNOWN		= 0xFF			//未知编码

	};


	//流格式
	enum ENUM_StreamFormat
	{
		ES_STREAM		=   1,		//原始流	
		TS_STREAM		=   2		//TS混合流
	};

	enum ENUM_ESStreamType
	{
		VIDEO			=  1,		//视频流
		AUDIO			=  2		//音频流
	};

	//帧类型
	enum ENUM_FrameType
	{
		UnknownFrame	=	0, //未知帧类型
		IFrame			=	1, //I帧
		PFrame			=	2, //P帧
		BFrame			=	3  //B帧
	};

	//磁盘类型
	enum ENUM_DiskType
	{
		HardDisk	= 1,	//硬盘
		SDCard		= 2,	//SD卡
		FTP			= 3		//FTP
	};

	//PTZ守望类型
	enum ENUM_PTZ_Keeper_Type
	{
		PTZ_KEEPER_TYPE_PRESET		= 1,	//预置位
		PTZ_KEEPER_TYPE_SCAN		= 2,	//扫描
		PTZ_KEEPER_TYPE_AUTO_STUDY	= 3,	//自学习
		PTZ_KEEPER_TYPE_TOUR		= 4		//巡游
	};

	//RTSP传输协议
	enum RtspTransferProtocol
	{
		RTP_UDP		= 0x1,		//UDP
		RTSP_TUNNEL = 0x2,		//RTSP隧道
		HTTP_TUNNEL = 0x3		//HTTP隧道
	};
	///////////////////////////////////////////////////////////////////////////////////////////////////////


	//IP地址
	typedef struct _PACKED_1_ tagInetAddr
	{
		char			szHostIP[CONST_MAXLENGTH_IP + 1];		//IP地址（点分符形式）

		unsigned short	nPORT;									//端口号

		long			nIPProtoVer;							//IP协议版本（1：IPv4协议，2：IPv6协议）

	}ST_InetAddr;

	//设备信息
	typedef struct _PACKED_1_ tagDeviceInfo
	{
		ST_InetAddr		stInetAddr;											//设备的网络地址

		char			szUserID[CONST_MAXLENGTH_USERID + 1];				//登陆设备的用户ID

		char			szPassword[CONST_MAXLENGTH_PASSWORD + 1];			//登陆设备的密码

		char			szDeviceID[CONST_MAXLENGTH_DEVICEID + 1];			//设备ID

		char			szDeviceName[CONST_MAXLENGTH_DEVICENAME + 1];		//设备名称

		long			nDeviceType;										//设备类型

	}ST_DeviceInfo;

	//设备信息
	typedef struct _PACKED_1_ tagDeviceInfoEx
	{
		ST_InetAddr		stInetAddr;											//设备的网络地址

		char			szUserID[CONST_MAXLENGTH_USERID + 1];				//登陆设备的用户ID

		char			szPassword[CONST_MAXLENGTH_PASSWORD + 1];			//登陆设备的密码

		char			szDeviceID[CONST_MAXLENGTH_DEVICEID + 1];			//设备ID

		char			szDeviceName[CONST_MAXLENGTH_DEVICENAME + 1];		//设备名称

		long			nDeviceType;										//设备类型

		unsigned char	m_bRouterMappingEnableFlag;							//路由器映射标志
		char			m_szRouterAddr[CONST_MAXLENGTH_IP + 1];				//路由器地址，IP地址或域名
		unsigned short	m_nRouterMappingControlPort;						//控制映射端口
		unsigned short	m_nRouterMappingTCPAVPort;							//TCP音视频映射端口
		unsigned short	m_nRouterMappingRTSPPort;							//RTSP映射端口
		unsigned short	m_nRouterMappingRTPPort;							//RTP映射端口
		unsigned short	m_nRouterMappingRTCPPort;							//RTCP映射端口

	}ST_DeviceInfoEx;

	//云台协议
	typedef struct _PACKED_1_ tagPTZProtocol
	{
		long	nProtocolId;													//云台协议的ID号

		char	szProtocolName[CONST_MAXLENTH_PTZ_PROTOCOL_NAME + 1];			//云台协议的名字

	}ST_PTZProtocol;

	//设备类型
	typedef struct _PACKED_1_ tagDeviceType
	{
		long		nDeviceTypeId;										//设备类型id

		char		szDeviceTypeName[CONST_MAXLENGTH_DEVICETYPE_NAME];	//设备类型名称

	}ST_DeviceType;

	//比特率类型
	typedef struct _PACKED_1_ tagBitrateType
	{
		long		nBitrateTypeId;												//比特率类型id

		char		szBitrateTypeName[CONST_MAXLENGTH_BITRATETYPE_NAME + 1];	//比特率类型名称

	}ST_BitrateType;

	//域名提供者
	typedef struct _PACKED_1_ tagDDNSProvider
	{
		long		nProviderId;											//提供者id

		char		szProviderName[CONST_MAXLENGTH_DDNS_PROVIDER + 1];		//提供者名称

	}ST_DDNSProvider;

	//报警类型
	typedef struct _PACKED_1_ tagAlarmType
	{
		long		nAlarmTypeId;												//报警类型id

		char		szAlarmTypeName[CONST_MAXLENGTH_ALARM_TYPE_NAME + 1];		//报警类型名称
	}ST_AlarmType;

	//图像格式
	typedef struct _PACKED_1_ tagImageFormat
	{
		long		nImageFormatId;												//图像格式id

		char		szImageFormatName[CONST_MAXLENGTH_IMAGEFORMAT_NAME + 1];	//图像格式名称

	}ST_ImageFormat;

	//视频制式
	typedef struct _PACKED_1_ tagVideoSystem
	{
		long		nVideoSystemId;												//视频制式id

		char		szVideoSystemName[CONST_MAXLENGTH_VIDEOSYSTEM_NAME + 1];	//视频制式名称

	}ST_VideoSystem;

	//摄像头视频制式
	typedef struct _PACKED_1_ tagCameraVideoSystem
	{
		long			nCameraId;				//摄像头id

		ST_VideoSystem	stWebVideoSystem;		//视频制式

	}ST_CameraVideoSystem;

	//设备摄像头视频制式
	typedef struct _PACKED_1_ tagDeviceVideoSystem
	{
		long						nCameraNum;										//摄像头个数

		ST_CameraVideoSystem		stWebCameraVideoSystem[CONST_MAX_CAMERA_NUM];	//摄像头制式列表

	}ST_DeviceVideoSystem;

	//字典表数据结构
	typedef struct _PACKED_1_ tagDictionaryService
	{
		long					nWebPTZProtocolNum;		  	
		ST_PTZProtocol			stWebPTZProtocol[CONST_MAX_PTZ_PROTOCOL_NUM];			//云台协议

		long					nWebDeviceTypeNum;
		ST_DeviceType			stWebDeviceType[CONST_MAX_DEVICE_TYPE_NUM];				//设备类型

		long					nWebBitrateTypeNum;
		ST_BitrateType			stWebBitrateType[CONST_MAX_BITRATE_TYPE_NUM];			//比特率类型

		long					nWebDDNSProviderNum;							
		ST_DDNSProvider			stWebDDNSProvider[CONST_MAX_DDNS_PROVIDER_NUM];			//域名提供者

		long					nWebAlarmTypeNum;
		ST_AlarmType			stWebAlarmType[CONST_MAX_ALARM_TYPE_NUM];				//报警类型

		long					nWebImageFormatNum;
		ST_ImageFormat			stWebImageFormat[CONST_MAX_DEVICE_IMAGE_FORMAT_NUM];    //分辨率类型

		long					nWebVideoSystemNum;
		ST_VideoSystem			stWebVideoSystem[CONST_MAX_VIDEO_SYSTEM_NUM];			//视频制式

	}ST_DictionaryService;

	//编码能力参数信息
	typedef struct _PACKED_1_ tagEncoderAbility
	{
		long nImageFormatId;								//视频格式
		long nMaxFrameRate;									//最大码率
		long nMinFrameRate;									//最小码率
		long nMaxBitRate;									//最大比特率
		long nMinBitRate;									//最小比特率
		long nMaxQuality;									//最大视频质量值
		long nMinQuality;									//最小视频质量值
		long nBitRateTypeNum;								//比特路类型数目
		long nBitRateTypeList[CONST_MAX_BITRATE_TYPE_NUM];	//比特率类型列表

		long nVideoSystem;									//制式
		long nHeight;										//高度
		long nWidth;											//宽度
		long nEncoderType;									//编码类型

	}ST_EncoderAbility;

	//侦测能力
	typedef struct _PACKED_1_ tagMotionDetectionAbility
	{
		long			m_nImageFormatId;				//视频格式
		long			m_nMaxSensitivity;				//最大灵敏度
		long			m_nMinFrameInterval;			//最小帧间隔

	}ST_MotionDetectionAbility;

	//摄像头能力参数
	typedef struct _PACKED_1_ tagCameraAbility
	{
		long							nCameraId;														//摄像头ID

		unsigned char					bUserConfigurable;												//视频制是否由用户配置
		unsigned char					bMotionDetectionSupportFlag;									//运动检测标志,1表示支持,0表示不支持

		long							nImageFormatNum;												//编码能力参数
		ST_EncoderAbility				stEncoderAbility[CONST_MAX_DEVICE_IMAGE_FORMAT_NUM*2];

		long							nDetectionImageFormatNum;
		ST_MotionDetectionAbility		stMotionDetectionAbility[CONST_MAX_DEVICE_IMAGE_FORMAT_NUM*2];	//移动侦测能力参数

	}ST_CameraAbility;

	//视频能力信息
	typedef struct _PACKED_1_ tagVideoAbility
	{
		long					nCameraNum;											//摄像头个数
		ST_CameraAbility		stCameraAbility[CONST_MAX_CAMERA_NUM];				//摄像头能力

	}ST_VideoAbility;

	typedef struct _PACKED_1_ tagAudioAbility
	{
		unsigned char bInterPhoneFlag;
		long nAudioInTypeList[CONST_MAX_CAMERA_NUM];
		long nAudioEncodeTypeList[CONST_MAX_CAMERA_NUM];
		long nAudioDecodeTypeList[CONST_MAX_CAMERA_NUM];
	}ST_AudioAbility;

	//设备能力信息参数
	typedef struct _PACKED_1_ tagDeviceAbility
	{
		ST_VideoAbility	stVideoAbility;
		ST_AudioAbility stAudioAbility;

	}ST_DeviceAbility;

	typedef struct _PACKED_1_ tagPTZAbility
	{
		unsigned char			bInternalPTZFlag;											//内置PTZ标志	1表示支持内置PTZ，0表示不支持。

		int						nPTZProtocolNum;											//支持的PTZ协议数
		ST_PTZProtocol			stPTZProtocolList[CONST_MAX_PTZ_PROTOCOL_NUM];				//支持的PTZ协议列表

	}ST_PTZAbility;





	//文件系统
	typedef struct _PACKED_1_ tagFileSystem
	{
		int					nFileSystemType;

		char				szFileSystemName[CONST_MAXLENTH_DISK_FILE_SYSTEM_NAME];

	}ST_FileSystem;

	//文件系统能力
	typedef struct _PACKED_1_ tagFileSystemAbility
	{
		int							nDiskType;

		int							nFileSystemTypeNum;
		ST_FileSystem			stFileSystemList[CONST_MAX_FILE_SYSTEM_NUM];

	}ST_FileSystemAbility;


	//音频解码类型
	typedef struct _PACKED_1_ tagAudioDecodeType
	{
		int					nAudioDecodeTypeId;														//音频解码类型Id

		char				szAudioDecodeTypeName[CONST_MAXLENGTH_AUDIO_DECODE_TYPE_NAME];			//音频解码类型名

	}ST_AudioDecodeType;

	//音频编码类型
	typedef struct _PACKED_1_ tagEncodeType
	{
		int					nAudioEncodeTypeId;	   												//音频编码类型Id

		char				szAudioEncodeTypeName[CONST_MAXLENGTH_AUDIO_ENCODE_TYPE_NAME];		//音频编码类型名

	}ST_EncodeType;

	//音频输入类型
	typedef struct _PACKED_1_ tagAudioInType
	{
		int	 nAudioInTypeId;													//音频输入类型Id

		char szAudioInTypeName[CONST_MAXLENGTH_AUDIO_IN_TYPE_NAME];				//音频输入类型名

	}ST_AudioInType;

	//支持视频分辨率
	typedef struct _PACKED_1_ tagVideoResolution
	{
		int						nVideoHeight;													//高度
		int						nVideoWidth;													//宽度
		char					szResolutionName[CONST_MAXLENGTH_RESOLUTION_NAME];				//分辨率别名

	}ST_VideoResolution;
	

	//隐式遮蔽能力
	typedef struct _PACKED_1_ tagBlindAbility
	{
		int						nMaxBlindAreaNum;													//最大的遮挡区域数目
		int						nBlindArea;															//遮挡区域面积占源分辨率百分比
	}ST_BlindAbility;



	//视频分辨率能力
	typedef struct _PACKED_1_ tagVideoResolutionAbility
	{
		ST_VideoResolution		stVideoResolution;								//视频分辨率
		int						nMaxFrameRate;									//最大帧率
		int						nMinFrameRate;									//最小帧率
		int						nMaxBitRate;									//最大码率
		int						nMinBitRate;									//最小码率
		int						nMaxQuality;									//最大视频质量值
		int						nMinQuality;									//最小视频质量值
		int						nMaxIFrameInterval;								//最大I帧的步长
		int						nMinIFrameInterval;								//最小I帧的步长

		int						nBitRateTypeNum;								//比特路类型数目
		int						nBitRateTypeList[CONST_MAX_BITRATE_TYPE_NUM];	//比特率类型列表
	}ST_VideoResolutionAbility;

	//视频编码器
	typedef struct _PACKED_1_ tagVideoEncoderAbility
	{
		int									nEncoderType;																			//编码器类型
		char								szEncoderName[CONST_MAXLENGTH_ENCODER_NAME];											//编码器名称
		int									nVideoEncodeTotleAbility;																//视频编码总能力 

		int									nVideoResolutionAbilityNum;																
		ST_VideoResolutionAbility			stVideoResolutionAbilityList[CONST_MAX_DEVICE_IMAGE_FORMAT_NUM];						//视频分辨率能力

	}ST_VideoEncoderAbility;





	//流编码能力
	typedef struct _PACKED_1_ tagStreamEncodeAbility
	{
		int						nEncoderType;					//码流ID

		int						nVideoResolutionNum;			//分辨率数目

		ST_VideoResolution	stVideoResolutionList[CONST_MAX_DEVICE_IMAGE_FORMAT_NUM];

	}ST_StreamEncodeAbility;

	//摄像头流能力参数
	typedef struct _PACKED_1_ tagAVStreamAbility
	{
		int							nStreamId;													//码流ID

		int							nStreamEncodeAbilityNum;									//流编码能力数量

		ST_StreamEncodeAbility		stStreamEncodeAbility[CONST_MAX_DEVICE_VIDEO_ENCODER_NUM];	//码流编码能力列表

	}ST_AVStreamAbility;


	//音频扩展能力信息
	typedef struct _PACKED_1_ tagAudioExAbility
	{
		unsigned char			bInterPhoneFlag;											//音频对讲有效标志	1表示支持对讲，0表示不支持对讲功能。

		unsigned char			bAudioInFlag;												//音频输入有效标志	1表示支持音频输入，0表示不支持音频输入功能。

		int						nAudioInTypeNum;	
		ST_AudioInType			stAudioInTypeList[CONST_MAX_AUDIO_TYPE_NUM];				//音频输入类型列表

		int						nAudioEncodeTypeNum;
		ST_EncodeType			stAudioEncodeTypeList[CONST_MAX_AUDIO_ENCODE_TYPE_NUM];		//音频编码类型列表

		int						nAudioDecodeTypeNum;
		ST_AudioDecodeType		stAudioDecodeTypeList[CONST_MAX_AUDIO_DECODE_TYPE_NUM];		//音频解码类型列表

	}ST_AudioExAbility;

	//摄像头扩展能力参数
	typedef struct _PACKED_1_ tagCameraExAbility
	{
		int							nCameraId;														//摄像头编号
		ST_VideoResolution			stMaxVideoResolution;											//支持的最大分辨率

		int							nSourceResolutionNum;
		ST_VideoResolution			stSourceResolutionList[CONST_MAX_SOURCE_RESOLUTION_NUM];		//支持源分辨率列表

		int							nImageFormatNum;
		ST_VideoResolution			stVideoResolutionList[CONST_MAX_DEVICE_IMAGE_FORMAT_NUM];		//支持视频分辨率列表

		int							nAVStreamAbilityNum;
		ST_AVStreamAbility			stAVStreamAbilityList[CONST_MAX_CAMERA_STREAM_NUM];				//摄像机流

		ST_BlindAbility				stBlindAbility;

	}ST_CameraExAbility;

	//视频扩展能力信息
	typedef struct _PACKED_1_ tagVideoExAbility
	{
		int								nCameraNum;
		ST_CameraExAbility				stCameraExAbilityList[CONST_MAX_CAMERA_NUM];					//摄像机能力列表

		unsigned char					bIsVideoSystemConfigurable;										//视频制是否可配置

		int								nVideoEncoderNum;
		ST_VideoEncoderAbility			stVideoEncoderAbilityList[CONST_MAX_DEVICE_VIDEO_ENCODER_NUM];	//视频编码器列表

		int								nVideoSystemNum;
		ST_VideoSystem					stVideoSystem[CONST_MAX_VIDEO_SYSTEM_NUM];						//视频制式
	
	}ST_VideoExAbility;


	//设备扩展能力信息参数
	typedef struct _PACKED_1_ tagDeviceExAbility
	{
		ST_VideoExAbility				stVideoExAbility;												//视频能力

		ST_AudioExAbility				stAudioExAbility;												//音频能力

		ST_PTZAbility					stPTZAbility;													//PTZ能力

		int								nDiskTypeNum;													//磁盘类型个数
		ST_FileSystemAbility			stFileSystemAbilityList[CONST_MAX_DISK_TYPE_NUM];				//文件系统能力

	}ST_DeviceExAbility;













	//用户登陆信息
	typedef struct _PACKED_1_ tagLoginInfo
	{
		char	szUserName[CONST_MAXLENGTH_USERNAME + 1];							//登陆用户名
		char    szPassword[CONST_MAXLENGTH_USERPASSWORD + 1];						//登陆密码

	}ST_LoginInfo;

	//设备概要信息
	typedef struct _PACKED_1_ tagDeviceSummaryParam
	{
		char	szDeviceName[CONST_MAXLENGTH_DEVICENAME + 1];						//设备名称
		char	szDeviceId[CONST_MAXLENGTH_DEVICEID + 1];							//设备号

		char	szManufacturerId[CONST_MAXLENGTH_MANUFACTURERID + 1];				//设备型号
		char	szManufacturerName[CONST_MAXLENGTH_MANUFACTURERNAME + 1];			//生产厂家

		char	szProductModel[CONST_MAXLENGTH_PRODUCTMODEL + 1];					//产品模组
		char	szProductDescription[CONST_MAXLENGTH_PRODUCTDESCRIPTION + 1];		//产品描述
		char	szHardwareModel[CONST_MAXLENGTH_HARDWAREMODEL + 1];					//硬件模组

		char	szHardwareDescription[CONST_MAXLENGTH_HARDWAREDESCRIPTION + 1];		//硬件描述

		char	szMACAddress[CONST_MAXLENGTH_MACADDRESS + 1];						//MAC地址
		char	szBarCode[CONST_MAXLENGTH_BARCODE + 1];								//机器条形码
		char	szProductionTime[CONST_MAXLENGTH_PRODUCTIONTIME + 1];				//生产时间


		long	nDeviceType;														//设备类型
		long	nVideoSystem;														//编码帧模式

		long	nCameraNum;															//摄像头数
		long	nAlarmInNum;														//报警输入个数
		long	nAlarmOutNum;														//报警输出个数

		char	szHardwareVer[CONST_MAXLENGTH_HARDWAREVERSION_EXPAND + 1];					//版本信息
		char	szSoftwareVer[CONST_MAXLENGTH_SOFTWAREVERSION_EXPAND + 1];

		long	nRS485Num;															//RS485串口个数
		long	nRS232Num;															//RS232串口个数

	}ST_DeviceSummaryParam;

	//摄像头信息
	typedef struct _PACKED_1_ tagCameraInfoParam
	{
		long			nCameraId;												//摄像机的ID号

		char			szCameraName[CONST_MAXLENGTH_CAMERA_NAME + 1];			//摄像机的名字

		char			szCameraModel[CONST_MAXLENGTH_CAMERAMODEL + 1];			//摄像机的样式	

		long			nVideoSystem;											//视频制式

	}ST_CameraInfoParam;

	//所有摄像头信息
	typedef struct _PACKED_1_ tagAllCameraInfoParam
	{
		long					nCameraNum;											//摄像头数

		ST_CameraInfoParam		stWebCameraInfoParamList[CONST_MAX_CAMERA_NUM];		//摄像头信息

	}ST_AllCameraInfoParam;

	//cpu统计信息
	typedef struct _PACKED_1_ tagCpuStatistic
	{
		float				fCpuPercent;				//设备cpu百分比
		unsigned char		btCpuStatus;				//设备cpu工作状态

	}ST_CpuStatistic;

	//内存统计信息
	typedef struct _PACKED_1_ tagMemStatistic
	{
		long		nMemTotalSize;				//内存总大小，以KB为单位
		long		nMemFreeSize;				//内存剩余大小，以KB为单位

	}ST_MemStatistic;

	//磁盘统计信息
	typedef struct _PACKED_1_ tagDiskStatistic
	{
		long			nDiskId;				//磁盘编号

		long			nDiskTotalSize;			//磁盘总大小,以KB为单位

		long			nDiskFreeSize;			//磁盘剩余大小,以KB为单位

		unsigned char	btDiskStatus;			//磁盘状态

	}ST_DiskStatistic;

	//所有磁盘统计信息
	typedef struct _PACKED_1_ tagAllDiskStatistic
	{
		ST_DiskStatistic		stDiskStatisticList[CONST_MAX_DISK_NUM];			//磁盘信息

		long					nDiskNum;											//磁盘个数		

	}ST_AllDiskStatistic;

	//视频通道统计信息
	typedef struct _PACKED_1_ tagVideoChannelStatistic
	{
		long			nCameraId;					//摄像机编号

		unsigned char	btRecordStatus;				//录制状态

		unsigned char	btCaptureStatus;			//抓拍状态

		unsigned char	btSignalStatus;				//连接信号状态

		unsigned char	btHardwareStatus;			//硬件状态

		long			nBitRate;					//比特率

	}ST_VideoChannelStatistic;

	//报警输入统计信息
	typedef struct _PACKED_1_ tagAlarmInStatistic
	{
		long			nAlarmInId;					//报警输入通道id号

		unsigned char	btAlarmInStatus;			//报警输入通道状态

		long			nLastAlarmTime;				//最后报警输入时间

	}ST_AlarmInStatistic;

	//报警输出统计信息
	typedef struct _PACKED_1_ tagAlarmOutStatistic
	{
		long			nAlarmOutId;				//报警输出通道id号

		unsigned char	btAlarmOutStatus;			//报警输出通道状态

		long			nLastAlarmTime;				//最后报警时间

	}ST_AlarmOutStatistic;

	//设备运行状态
	typedef struct _PACKED_1_ tagDeviceWorkState
	{
		ST_CpuStatistic				stCpuStatistic;											//cpu统计信息

		ST_MemStatistic				stMemStatistic;											//内存统计信息

		ST_DiskStatistic			stDiskStatisticList[CONST_MAX_DISK_NUM];				//磁盘统计信息
		long						nDiskNum;

		ST_VideoChannelStatistic	stVideoChannelStatisticList[CONST_MAX_CAMERA_NUM];		//视频通道统计信息
		long						nVideoChannelNum;

		ST_AlarmInStatistic			stAlarmInStatisticList[CONST_MAX_ALARM_IN_NUM];			//报警输入统计信息
		long						nAlarmInNum;

		ST_AlarmOutStatistic		stAlarmOutStatisticList[CONST_MAX_ALARM_OUT_NUM];		//报警输出统计信息
		long						nAlarmOutNum;

	}ST_DeviceWorkState;

	//NTP参数
	typedef struct _PACKED_1_ tagNTPParam
	{
		unsigned char	bNTPEnableFlag;											//是否启用NTP的状态

		char			szNTPIp[CONST_MAXLENGTH_IP + 1];						//NTPIP地址

		unsigned short	nNTPPort;												//NTP的端口

		long			nRunTime;												//NTP较时时间

		long			nIPProtoVer;											//IP协议类型

	}ST_NTPParam;

	//DDNS参数
	typedef struct _PACKED_1_ tagDDNSParam
	{
		unsigned char	bDDNSEnableFlag;										//是否启用DDNS的状态

		long			nDDNSProviderId;										//提供者id

		char			szDDNSAccounts[CONST_MAXLENGTH_DDNS_ACCOUNTS + 1];		//DDNS账户

		char			szDDNSDomainName[CONST_MAXLENGTH_DDNS_DOMAINNAME + 1];	//DDNS域名	

		char			szDDNSPassword[CONST_MAXLENGTH_DDNS_PASSWORD + 1];		//DDNS密码
	}ST_DDNSParam;

	//PPPoE参数
	typedef struct _PACKED_1_ tagPPPoEParam
	{
		unsigned char	bPPPoEEnableFlag;										//是否启用PPPoE的状态

		char			szPPPoEUserName[CONST_MAXLENGTH_PPPOE_USERNAME + 1];	//PPPoE的用户名
		char			szPPPoEPassword[CONST_MAXLENGTH_PPPOE_PASSWORD + 1];	//PPPoE的密码

	}ST_PPPoEParam;

	//网络参数
	typedef struct _PACKED_1_ tagHostNetworkParam
	{
		char			szDeviceId[CONST_MAXLENGTH_DEVICEID + 1];				//DVR设备的设备ID

		long			nNetId;													//网口编号

		unsigned char	bAutoGetIpFlag;											//自动获取ip地址
		char			szLocalIp[CONST_MAXLENGTH_IP + 1];						//DVR设备本地IP地址
		char			szLocalSubnetMask[CONST_MAXLENGTH_IP + 1];				//DVR设备本地子网掩码
		char			szGateway[CONST_MAXLENGTH_IP + 1];						//DVR设备网关

		unsigned char	bAutoGetDNSFlag;										//自动获取DNS服务器地址
		char			szPrimaryDNSIp[CONST_MAXLENGTH_IP + 1];					//主DNS
		char			szSpareDNSIp[CONST_MAXLENGTH_IP + 1];					//备用DNS

		unsigned short	nControlPort;											//DVR设备网络控制端口
		unsigned short	nRegisterPort;											//DVR设备注册端口
		unsigned short	nHttpPort;												//DVR设备HTTP端口

		long			nIPProtoVer;											//IP协议类型

		long			nAdapterType;											//适配器类型

	}ST_HostNetworkParam;

	//设备端口参数
	typedef struct _PACKED_1_ tagDevicePortParam
	{
		char			szDeviceId[CONST_MAXLENGTH_DEVICEID + 1];				//DVR设备的设备ID

		unsigned short	m_nControlPort;											//网络视频设备的设备网络控制端口
		unsigned short	m_nTCPAVPort;											//网络视频设备的TCP音视频端口
		unsigned short	m_nRTSPPort;											//网络视频设备的RTSP端口
		unsigned short	m_nRTPPort;												//网络视频设备的RTP端口
		unsigned short	m_nRTCPPort;											//网络视频设备的RTCP端口
		unsigned short	m_nRegisterPort;										//网络视频设备的设备注册端口
		unsigned short	m_nHttpPort;											//网络视频设备的设备HTTP端口

		unsigned short	m_nReservePort1;										//保留使用
		unsigned short	m_nReservePort2;										
		unsigned short	m_nReservePort3;										
		unsigned short	m_nReservePort4;

	}ST_DevicePortParam;

	// PTZ参数
	typedef struct _PACKED_1_ tagPTZParam
	{
		unsigned char	    bPTZEnableFlag;										//是否启动云台

		long				nCameraId;											//云台对应的摄像机号

		long				nPTZDeviceId;										//云台设备地址

		long				nPTZProtocol;										//云台协议

		long				nComId;												//云台连接的串口id

		long				nBaudRate;											//波特率

		long				nDataBits;											//数据位

		long				nStopBits;											//停止位

		long				nParity;											//奇偶校验

	}ST_PTZParam;

	//所有的PTZ参数
	typedef struct _PACKED_1_ tagAllPTZParam
	{
		ST_PTZParam		stPTZParamList[CONST_MAX_CAMERA_NUM];	//PTZ参数

		long			nPTZParamNum;							//PTZ参数个数

	}ST_AllPTZParam;

	//RS485参数
	typedef struct _PACKED_1_ tagRS485Param
	{
		long		nComId;										//串口ID

		char		szComName[CONST_LENGTH_RS485_NAME + 1];		//串口自定义名称

		long		nBaudRate;									//波特率

		long		nDataBits;									//数据位

		long		nStopBits;									//停止位

		long		nParity;									//奇偶校验

		long		nFlowControl;								//流控

	}ST_RS485Param;

	//所有的RS485参数
	typedef struct _PACKED_1_ tagAllRS485Param
	{
		ST_RS485Param stRS485ParamList[CONST_MAX_RS484_NUM];	//RS485参数

		long		  nRs485Num;								//RS485个数

	}ST_AllRS485Param;

	//自动更新服务
	typedef struct _PACKED_1_ tagUpdateParam
	{
		unsigned char   bUpdateEnableFlag;										//启动自动更新服务

		char	    	szUpdateServerIP[CONST_MAXLENGTH_IP + 1];				//更新服务器IP地址

		unsigned short	nUpdateServerPort;										//更新服务器端口号

		long			nUpdatePeriod;											//升级周期时间

		long			nIPProtoVer;											//IP协议类型

	}ST_UpdateParam;

	//计划时间
	typedef struct _PACKED_1_ tagScheduleTime
	{
		long nStartTime;					//开始时间

		long nEndTime;						//结束时间

	}ST_ScheduleTime;

	typedef struct _PACKED_1_ tagScheduleWeek
	{
		long						nWeekDay;	

		int							nScheduleTimeNum;

		ST_ScheduleTime				stScheduleTime[CONST_MAX_PERIOD_SCHEDULE_RECORD_TIME_NUM];

	}ST_ScheduleWeek;

	//计划时间参数
	typedef struct _PACKED_1_ tagScheduleTimeParam
	{
		char						szDeviceId[CONST_MAXLENGTH_DEVICEID + 1];		//设备ID

		long						nCameraId;										//摄像机ID

		ST_ScheduleWeek				stScheduleWeekList[7];							//计划时间

	}ST_ScheduleTimeParam;

	//计划录像参数
	typedef struct _PACKED_1_ tagScheduleRecordParam
	{
		char						szDeviceId[CONST_MAXLENGTH_DEVICEID + 1];

		long						nCameraId;

		unsigned char			    nIsEnableRecord;										//开启录像标志 0:关闭录像；1开启录像

		unsigned char				nIsEnableRecordAudio;									//是否录制音频 0:不录制；1：录制

		long						nScheduleRecordType;									//录像规则

		ST_ScheduleTimeParam		stScheduleTimeParam;									//计划录像时间

	}ST_ScheduleRecordParam;

	//所有的计划录像参数
	typedef struct _PACKED_1_ tagAllScheduleRecordParam
	{
		ST_ScheduleRecordParam stScheduleRecordParamList[CONST_MAX_CAMERA_NUM];				//计划录像参数

		long				   nScheduleRecordNum;											//计划录像数目

	}ST_AllScheduleRecordParam;

	//本地录像磁盘写策略
	typedef struct _PACKED_1_ tagWritePolicy
	{
		long  nStoragePolicy;																//存储策略

		int	  nRecordLengthMode;															//录像类型, （1：按大小录制；2：按时间录制）

		long  nRecordFileMaxSize;															//录制文件最大尺寸

		long  nRecordFileTime;																//单个录像文件的时间大小, 以秒（S）为单位

		char  szFileNameSuffix[CONST_MAXLENGTH_FILENAME_SUFFIX + 1];						//录制文件后缀名

		long  nRecordFormatType;															//录像格式	1为MPEG2PS流，2为自定义流

	}ST_WritePolicy;

	//本地录像保存策略
	typedef struct _PACKED_1_ tagSavePolicy
	{
		long nSaveDays;																		//存储天

	}ST_SavePolicy;

	//本地录像空间策略
	typedef struct _PACKED_1_ tagSpacePolicy
	{
		long	  nCameraId;										//摄像机id

		long	  nMaxDiskSize;										//所能使用的最大磁盘空间

		float	  nScheduleRecordPercent;							//计划录像的所能使用的最大磁盘空间的百分比

		float	  nAlarmRecordPercent;								//报警录像所能使用的最大磁盘空间的百分比

	}ST_SpacePolicy;

	//本地录像存储参数
	typedef struct _PACKED_1_ tagRecordStorageParam
	{
		ST_WritePolicy		stWritePolicy;							//写策略

		ST_SavePolicy		stSavePolicy;							//保存策略

		long			    nCameraNum;								//摄像头数

		ST_SpacePolicy		stSpacePolicy[CONST_MAX_CAMERA_NUM];	//空间策略

	}ST_RecordStorageParam;

	//视频编码质量参数
	typedef struct _PACKED_1_ tagVideoEncodeQuality
	{
		long		nQuality;			//视频质量

		long		nVideoSystem;		//编码帧模式

		double		dFrameRate;			//编码帧速

		long	    nImageFormatId;		//分辨率

		long	    nBitRateType;		//码率类型

		long		nBitRate;			//比特率

		long		nQuant;				//Q值

		long		nIFrameInterval;	//I帧的步长

		long		nEncoderType;		//编码格式

		long		nCodingFlag;		//码流：1：主码流，0：子码流

	}ST_VideoEncodeQuality;

	//音频编码质量参数
	typedef struct _PACKED_1_ tagAudioEncodeQuality
	{
		unsigned short		nFormatTag;					//格式标记

		unsigned short		nChannels;					//声音通道数（声道）

		unsigned long		nSamplesPerSec;				//每秒采样数

		unsigned long		nAvgBytesPerSec;			//

		unsigned short		nBlockAlign;				//

		unsigned short		nBitsPerSample;				//每次采样的大小

		unsigned short		nCBSize;					//后面追加数据长度

		long				nEncodeType;				//编码格式

	}ST_AudioEncodeQuality;

	//录像质量参数
	typedef struct _PACKED_1_ tagRecordQualityParam
	{
		long				  nCameraId;							//摄像头id

		ST_VideoEncodeQuality stVideoVideoEncodeQuality;			//视频编码质量

		ST_AudioEncodeQuality stAudioEncodeQuality;					//音频编码质量

	}ST_RecordQualityParam;

	//所有的录像质量参数
	typedef struct _PACKED_1_ tagAllRecordQualityParam
	{
		ST_RecordQualityParam	  stRecordQualityParamList[CONST_MAX_CAMERA_NUM];				//录像质量

		long					  nRecordQualityNum;											//

	}ST_AllRecordQualityParam;

	//报警输出事件参数
	typedef struct _PACKED_1_ tagAlarmOutActionParam
	{
		char szDeviceId[CONST_MAXLENGTH_DEVICEID + 1];

		long nAlarmOutId;																		//报警输出端口的ID号

		long nAlarmOutFlag;																		//报警输出标志

		long nEventTypeId;																		//报警事件类型

	}ST_AlarmOutActionParam;

	//报警录像事件参数
	typedef struct _PACKED_1_ tagAlarmRecordActionParam
	{
		char			szDeviceId[CONST_MAXLENGTH_DEVICEID + 1];								//DVR设备的设备ID

		long			nCameraId;																//摄像机的ID号

		long			nPreRecordTime;															//预录时长

		long			nRecordTime;															//录制时长

		long			nEventTypeId;															//报警事件类型

		unsigned char   nIsEnableRecordAudio;													//是否录制音频

	}ST_AlarmRecordActionParam;

	//报警云台事件参数
	typedef struct _PACKED_1_ tagAlarmPTZActionParam
	{
		long					nEventTypeId;											//报警事件类型

		char					szDeviceId[CONST_MAXLENGTH_DEVICEID + 1];				//DVR设备的设备ID

		long					nCameraId;												//摄像机的ID号

		long					nComId;													//串口id号

		long					nPtzId;													//云台设备的ID号

		char					btPtzOperationCmd[CONST_MAXLENTH_PTZOPERATION_CMD];		//云台操作命令

		long					nPtzOperationCmdLen;									//云台操作命令长度

		long					nReserve;												//保留位

	}ST_AlarmPTZActionParam;

	//OSD信息参数
	typedef struct _PACKED_1_ tagOSDInfoParam
	{
		long				nOSDType;							//水印类型

		char				szInfo[CONST_MAXLENGTH_OSDINFO + 1];

		unsigned char		bOSDEnableFlag;						//是否显示水印（0：不显示，1：显示）

		long				nTopX;

		long				nTopY;

	}ST_OSDInfoParam;

	//视频OSD信息参数
	typedef struct _PACKED_1_ tagVideoOSDInfoParam
	{
		long					nCameraId;								//摄像机id

		long					nImageFormatID;							//图像格式(此参数已废弃)

		ST_OSDInfoParam			stOSDInfoParam[CONST_MAX_CAMERA_NUM];	//OSD信息参数

		long					nOSDInfoNum;							//OSD信息条目

	}ST_VideoOSDInfoParam;

	//视频OSD信息参数
	typedef struct _PACKED_1_ tagAllVideoOSDInfoParam
	{
		long							nVideoOSDInfoParamNum;							//OSD参数数目

		ST_VideoOSDInfoParam			stAlarmInParamList[CONST_MAX_CAMERA_NUM];		//OSD参数参数

	}ST_AllVideoOSDInfoParam;

	//报警覆盖事件参数
	typedef struct _PACKED_1_ tagAlarmOverlayActionParam
	{
		long nEventTypeId;										//报警事件类型

		char szDeviceId[CONST_MAXLENGTH_DEVICEID + 1];			//DVR设备的设备ID

		long nCameraId;											//摄像机ID号

		ST_OSDInfoParam stOSDInfoParam;							//水印信息类的对象

	}ST_AlarmOverlayActionParam;

	//报警IO参数
	typedef struct _PACKED_1_ tagAlarmIOEventParam
	{
		long								nEventId;																		//报警事件ID

		long								nPolicyId;																		//报警策略ID

		long								nAlarmInId;																		//报警输入端口号

		unsigned char						bAlarmIOEnableFlag;																//是否启动IO报警(0：不启动， 1：启动)

		ST_ScheduleTimeParam				stScheduleTimeParam;															//布防时间参数
		char								szDeviceId[CONST_MAXLENGTH_DEVICEID + 1];										//设备ID号

		long								nAlarmOutActionNum;																//报警输出事件个数
		ST_AlarmOutActionParam				stAlarmOutActionParam[CONST_MAX_ALARM_OUT_NUM];									//报警输出事件

		long								nAlarmRecordActionNum;															//报警录像事件个数
		ST_AlarmRecordActionParam			stAlarmRecordActionParam[CONST_MAX_CAMERA_NUM];									//报警录像事件

		long								nAlarmPTZActionNum;																//报警云台事件个数
		ST_AlarmPTZActionParam				stAlarmPTZActionParam[CONST_MAX_CAMERA_NUM];									//报警云台事件

		long								nAlarmOverlayNum;																//报警覆盖事件个数
		ST_AlarmOverlayActionParam			stAlarmOverlayActionParam[CONST_MAX_CAMERA_NUM];								//报警覆盖事件

	}ST_AlarmIOEventParam;

	//所有I/O报警事件参数
	typedef struct _PACKED_1_ tagAllAlarmIOEventParam
	{
		ST_AlarmIOEventParam stAlarmIOEventParamList[CONST_MAX_ALARM_IN_NUM];			//报警I/O事件

		long				 nAlarmIOEventNum;											//报警I/O事件数目

	}ST_AllAlarmIOEventParam;

	//侦测区域
	typedef struct _PACKED_1_ tagDetectionAreaParam
	{
		long		nTopX;								//侦测区域左上角 x 的坐标
		long		nTopY;								//侦测区域左上角 y 的坐标
		long		nWidth;								//侦测区域宽度
		long		nHeight;							//侦测区域高度

	}ST_DetectionAreaParam;

	//侦测参数
	typedef struct _PACKED_1_ tagMotionDetectionParam
	{
		long							nSensitivity;																	//灵敏度，此参数无效，保留使用

		long							nCheckBlockNum;																	//检测块数

		unsigned char					bToUpCheckFlag;																	//向左检测（1：检测，0：不检测）
		unsigned char					bToDownCheckFlag;																//向下检测（1：检测，0：不检测）
		unsigned char					bToLeftCheckFlag;																//向左检测（1：检测，0：不检测）
		unsigned char					bToRightCheckFlag;															//向右检测（1：检测，0：不检测）
		unsigned char					bAreaMaskFlag;																	//屏蔽标识（1：屏蔽，0：检测）

		long							nImageFormatId;																	//视频格式

		long							nFrameInterval;																	//帧间隔

		long							nDetectionAreaNum;																//侦测区域数目，百分比，指占总移动侦测区域的百分比，百分比越小，侦测灵敏度越高

		ST_DetectionAreaParam			stDetectionAreaList[CONST_MAX_DETECTION_NUM];									//移动侦测区域

	}ST_MotionDetectionParam;

	//侦测报警参数
	typedef struct _PACKED_1_ tagMotionDetectionEventParam
	{
		long							nEventId;																		//报警事件ID

		long							nPolicyId;																		//报警策略ID

		char							szDeviceId[CONST_MAXLENGTH_DEVICEID + 1];										//DVR设备的设备ID

		long							nCameraId;																		//DVR设备摄像头ID号

		unsigned char					bMotionDetectionEnableFlag;														//是否启动运动检测
		ST_ScheduleTimeParam			stScheduleTimeParam;															//布防时间参数

		ST_MotionDetectionParam			stMotionDetectionParam;															//侦测参数

		long							nAlarmOutActionNum;																//报警输出事件个数
		ST_AlarmOutActionParam			stAlarmOutActionParam[CONST_MAX_ALARM_OUT_NUM];									//报警输出事件

		long							nAlarmRecordActionNum;															//报警录像事件个数
		ST_AlarmRecordActionParam		stAlarmRecordActionParam[CONST_MAX_CAMERA_NUM];									//报警录像事件

		long							nAlarmPTZActionNum;																//报警云台事件个数
		ST_AlarmPTZActionParam			stAlarmPTZActionParam[CONST_MAX_CAMERA_NUM];									//报警云台事件

		long							nAlarmOverlayNum;																//报警覆盖事件个数
		ST_AlarmOverlayActionParam		stAlarmOverlayActionParam[CONST_MAX_CAMERA_NUM];								//报警覆盖事件


	}ST_MotionDetectionEventParam;

	//设备所有移动侦测报警事件
	typedef struct _PACKED_1_ tagAllMotionDetectionEventParam
	{
		ST_MotionDetectionEventParam stMotionDetectionEventParamList[CONST_MAX_CAMERA_NUM];				//移动侦测报警事件

		long						 nMotionDetectionEventNum;											//移动侦测报警事件条目

	}ST_AllMotionDetectionEventParam;

//////////////////

	//遮挡侦测参数
	typedef struct _PACKED_1_ tagOcclusionDetectionParam
	{
		int								nSensitivity;																//灵敏度

		int								nDetectionAreaNum;															//侦测区域数目
		ST_DetectionAreaParam			stDetectionAreaList[CONST_MAX_DETECTION_NUM];								//移动侦测区域

	}ST_OcclusionDetectionParam;

	//遮挡报警参数
	typedef struct _PACKED_1_ tagOcclusionDetectionEventParam
	{
		char							szDeviceId[CONST_MAXLENGTH_DEVICEID];										//设备ID

		int								nCameraId;																	//摄像头ID号

		unsigned char					bOcclusionDetectionEnableFlag;												//是否启动检测

		ST_ScheduleTimeParam			stScheduleTimeParam;														//布防时间参数

		ST_OcclusionDetectionParam		stOcclusionDetectionParam;													//侦测参数

		int								nAlarmOutActionNum;															//报警输出事件个数
		ST_AlarmOutActionParam			stAlarmOutActionParam[CONST_MAX_ALARM_OUT_NUM];								//报警输出事件

		int								nAlarmPTZActionNum;															//报警云台事件个数
		ST_AlarmPTZActionParam			stAlarmPTZActionParam[CONST_MAX_CAMERA_NUM];								//报警云台事件

	}ST_OcclusionDetectionEventParam;

	//设备所有遮挡侦测报警事件
	typedef struct _PACKED_1_ tagAllOcclusionDetectionEventParam
	{
		ST_OcclusionDetectionEventParam stOcclusionDetectionEventParamList[CONST_MAX_CAMERA_NUM];					//移动侦测报警事件

		long							nDetectionEventNum;															//移动侦测报警事件条目

	}ST_AllOcclusionDetectionEventParam;

	//视频丢失报警参数
	typedef struct _PACKED_1_ tagVideoLoseDetectionEventParam
	{
		char							szDeviceId[CONST_MAXLENGTH_DEVICEID];										//设备ID

		int								nCameraId;																	//摄像头ID号

		unsigned char					bVideoLoseDetectionEnableFlag;												//是否启动检测

		int								nAlarmOutActionNum;															//报警输出事件个数
		ST_AlarmOutActionParam			stAlarmOutActionParam[CONST_MAX_ALARM_OUT_NUM];								//报警输出事件

		int								nAlarmPTZActionNum;															//报警云台事件个数
		ST_AlarmPTZActionParam			stAlarmPTZActionParam[CONST_MAX_CAMERA_NUM];								//报警云台事件

	}ST_VideoLoseDetectionEventParam;

	//设备所有移动侦测报警事件
	typedef struct _PACKED_1_ tagAllVideoLoseDetectionEventParam
	{
		ST_VideoLoseDetectionEventParam stVideoLoseDetectionEventParamList[CONST_MAX_CAMERA_NUM];					//视频丢失报警事件

		long							nDetectionEventNum;															//视频丢失报警事件条目

	}ST_AllVideoLoseDetectionEventParam;


/////////////////////

	//报警I/O输入
	typedef struct _PACKED_1_ tagAlarmInParam
	{
		long  nAlarmInId;													//报警输入端口号

		char  szAlarmInputName[CONST_MAXLENGTH_ALARMINPUTNAME + 1];			//报警输入端口名

		long  nAlarmInValidLevel;											//报警输入的有效电平

	}ST_AlarmInParam;

	//报警I/O输出
	typedef struct _PACKED_1_ tagAlarmOutParam
	{

		long		nAlarmOutId;											//报警输出ID号

		char		szAlarmOutName[CONST_MAXLENGTH_ALARMOUTNAME + 1];		//报警输出名称

		long		nAlarmValidSignal;										//报警有效信号

		long		nAlarmMode;												//报警模式
		float		nAlarmOutFrequency;										//报警频率

		long		nAlarmTime;												//报警时长

		long		nActivatedStartTime;									//报警激活开始时间(从00：00到开始时间hh：mm的秒数)
		long		nActivatedEndTime;										//报警激活结束时间(从00：00到结束时间hh：mm的秒数)

	}ST_AlarmOutParam;

	//报警输入设备
	typedef struct _PACKED_1_ tagAllAlarmInParam
	{
		long					nAlarmInNum;									//报警输入设备数目

		ST_AlarmInParam			stAlarmInParamList[CONST_MAX_ALARM_IN_NUM];		//报警输入设备参数

	}ST_AllAlarmInParam;

	//报警输出设备
	typedef struct _PACKED_1_ tagAllAlarmOutParam
	{
		long					nAlarmOutNum;									//报警输出设备数目

		ST_AlarmOutParam		stAlarmOutParamList[CONST_MAX_ALARM_OUT_NUM];	//报警输出设备参数

	}ST_AllAlarmOutParam;

	//报警设备
	typedef struct _PACKED_1_ tagAlarmDeviceParam
	{
		long					nAlarmInNum;									//报警输入数目

		ST_AlarmInParam			stAlarmInParam[CONST_MAX_ALARM_IN_NUM];			//报警输入参数

		long					nAlarmOutNum;									//报警输入数目

		ST_AlarmOutParam		stAlarmOutParam[CONST_MAX_ALARM_OUT_NUM];		//报警输入参数		

	}ST_AlarmDeviceParam;

	//磁盘报警参数
	typedef struct _PACKED_1_ tagDiskAlarmParam
	{
		unsigned char		bDiskFullAlarmCheckFlag;		//是否检测硬盘满
		long				nMaxDiskPercent;				//硬盘报警高阀值（占用硬盘最大比率）
		long				nMinDiskPercent;				//硬盘报警低阀值（占用硬盘最低比率）

		unsigned char		bDiskErrorAlarmCheckFlag;		//是否检测硬盘出错

	}ST_DiskAlarmParam;

	//报警中心参数
	typedef struct _PACKED_1_ tagAlarmServiceParam
	{
		unsigned char		bAlarmCenterEnableFlag;										//是否启用报警中心(0：不启用， 1：启用)

		char				szAlarmCenterServerIP[CONST_MAXLENGTH_IP + 1];				//报警中心IP地址
		unsigned short		nAlarmCenterServerPort;										//报警中心的端口

		unsigned char		bAlarmEmailEnableFlag;										//是否启用报警邮件
		char				szAlarmEmailAddress[CONST_MAXLENTH_EMAIL_ADDRESS + 1];		//报警邮件地址

		long				nIPProtoVer;												//IP协议类型

	}ST_AlarmServiceParam;

	//报警参数
	typedef struct _PACKED_1_ tagAlarmParam
	{
		int						m_nAlarmInterval;			//报警间隔

	}ST_AlarmParam;

	//设备注册服务
	typedef struct _PACKED_1_ tagRegisterParam
	{
		unsigned char	bRegisterEnableFlag;								//是否启用注册(0：不启用， 1：启用)

		char			szRegisterSrvIP[CONST_MAXLENGTH_IP + 1];			//注册服务器IP地址,主机字节顺序

		unsigned short	nRegisterSrvPort;									//注册服务器的端口

		long			nIPProtoVer;										//IP协议类型

	}ST_RegisterParam;

	//拾音器参数配置
	typedef struct _PACKED_1_ tagToneArmParam
	{
		long			nCameraId;											//摄像机的ID号

		unsigned char	bToneArmEnableFlag;									//是否启用拾音器（0：不启用，1：启用）

		long			nToneArmType;										//拾音器类型

		long			nVolume;											//音量

	}ST_ToneArmParam;

	//所有拾音器参数
	typedef struct _PACKED_1_ tagAllToneArmParam 
	{
		ST_ToneArmParam  stToneArmParamList[CONST_MAX_CAMERA_NUM];

		long			 nToneArmNum;

	}ST_AllToneArmParam;

	//扬声器参数
	typedef struct _PACKED_1_ tagLoudhailerParam
	{
		long			nCameraId;										//摄像机的ID号

		unsigned char	bLoudhailerEnableFlag;							//是否启用扬声器（0：不启用，1：启用）

		long			nLoudhailerType;								//扬声器类型

		long			nVolume;										//音量

	}ST_LoudhailerParam;

	typedef struct _PACKED_1_ tagAllLoudhailerParam
	{
		ST_LoudhailerParam stLoudhailerParamList[CONST_MAX_CAMERA_NUM];

		long			   nLoudhailerNum;

	}ST_AllLoudhailerParam;

	//心跳参数配置
	typedef struct _PACKED_1_ tagHeartbeatParam
	{
		char			szHeartbeatServerIp[CONST_MAXLENGTH_IP + 1];	//心跳服务器IP

		unsigned short	nHeartbeatServerPort;							//心跳服务器端口

		long			nHeartbeatInterval;								//发送心跳间隔，以秒为单位

		long			nHeartbeatAckTimeout;							//单个心跳应答超时时间，以秒为单位

		long			nHeartbeatTimeoutCount;							//心跳超时总次数,超过这个次数重登录心跳服务器

		long			nIPProtoVer;									//IP协议类型

	}ST_HeartbeatParam;

	//FTP参数配置
	typedef struct _PACKED_1_ tagFTPParam
	{
		unsigned char	bFTPEnableFlag;												//是否启用FTP的状态

		char			szFTPServerAddr[CONST_MAXLENGTH_IP + 1];					//FTP服务器地址

		char			szFTPUserName[CONST_MAXLENGTH_FTP_USERNAME + 1];			//FTP账户

		char			szFTPPassword[CONST_MAXLENGTH_FTP_PASSWORD + 1];			//FTP密码

		char			szFTPWorkPath[CONST_MAXLENGTH_FTP_WORKPATH + 1];			//FTP工作路径	

		long			nIPProtoVer;												//IP协议类型

	}ST_FTPParam;

	//SMTP参数配置
	typedef struct _PACKED_1_ tagSMTPParam
	{
		unsigned char	bSMTPEnableFlag;										//SMTP启用标志(0：不启用，1：启用)

		unsigned char	bUseDefaultSMTPServerFlag;								//是否使用默认邮箱标识(0：不启用，1：启用)

		char			szSMTPServerAddr[CONST_MAXLENGTH_IP + 1];				//SMTP服务器地址

		unsigned short  nSMTPServerPort;										//SMTP服务器端口

		char			szSMTPUserName[CONST_MAXLENGTH_USERNAME + 1];			//账户

		char			szSMTPPassword[CONST_MAXLENGTH_USERPASSWORD + 1];		//密码

		char			szSenderEmailAddress[CONST_MAXLENGTH_EMAIL_ADDRESS + 1];//发件人地址	

		char			szRecipientEmailAddressList[16][CONST_MAXLENGTH_EMAIL_ADDRESS + 1];//发件人地址
		int				nRecipientEmailAddressCount;							//收件人个数

		int				nMailLanguage;											//邮件内容语言

		int				nAttachmentImageQuality;								//邮件附件的图像质量

	}ST_SMTPParam;

	//IP协议参数
	typedef struct _PACKED_1_ tagCommunicationParam
	{
		long			nIPProtoFlag;			//IP协议类型

	}ST_CommunicationParam;

	//IP协议参数
	typedef struct _PACKED_1_ tagBroadcastParam
	{
		unsigned char	bBroadcastEnableFlag;					//是否启用广播（0：不启用，1：启用）

		unsigned short	nBroadcastPort;							//广播端口

		long			nBroadcastInterval;						//发送广播间隔，以秒为单位

	}ST_BroadcastParam;


	//快照参数
	typedef struct _PACKED_1_ tagSnapshotParam
	{
		long			nCameraId;				//摄像机id

		unsigned char	bSnapshotEnableFlag;	//是否允许抓拍(0：不允许， 1：允许)

		long			nImageFormatId;			//分辨率

		long			nQuality;				//抓拍质量

		long			nShootTimes;			//每次触警抓拍次数

		long			nPhotoFormat;			//图片格式

	}ST_SnapshotParam;

	//所有摄像头快照参数
	typedef struct _PACKED_1_ tagAllSnapshotParam
	{
		ST_SnapshotParam stSnapshotParamList[CONST_MAX_CAMERA_NUM];

		long			 nCameraNum;

	}ST_AllSnapshotParam;

	//录像文件查询参数
	typedef struct _PACKED_1_ tagRecordFileSearchParam
	{
		char			szDeviceId[CONST_MAXLENGTH_DEVICEID + 1];			//设备ID

		char			szDeviceName[CONST_MAXLENGTH_DEVICENAME + 1];		//设备名称

		long			nCameraId;											//摄像机ID

		char			szCameraName[CONST_MAXLENGTH_CAMERA_NAME + 1];		//摄像机名称

		char			szDeviceIp[CONST_MAXLENGTH_IP + 1];					//设备IP

		char			szRecordFileName[CONST_MAXLENGTH_FILE_NAME + 1];	//录制文件名称

		unsigned long	nStartTime;											//录制文件开始时间

		unsigned long	nEndTime;											//录制文件结束时间

		long			nRecordType;										//录制类型

		long			nAlarmId;											//报警ID

		long			nAlarmType;											//报警类型

	}ST_RecordFileSearchParam;

	//录像文件参数
	typedef struct _PACKED_1_ tagRecordFile
	{
		long			nId;												//数据库表中的Id

		char			szDeviceId[CONST_MAXLENGTH_DEVICEID + 1];			//设备ID

		char			szDeviceName[CONST_MAXLENGTH_DEVICENAME + 1];		//设备名称

		long			nCameraId;											//摄像机编号

		char			szCameraName[CONST_MAXLENGTH_CAMERA_NAME + 1];		//摄像机名词

		char			szDeviceIp[CONST_MAXLENGTH_IP + 1];					//设备IP

		char			szRecordFileName[CONST_MAXLENGTH_FILE_NAME + 1];	//录制文件名称

		long			nRecordTime;										//录制时间

		long			nRecordType;										//录制文件类型

		long			nAlarmId;											//报警Id

		long			nAlarmType;											//报警类型

		long			nFileByteLength;									//录像文件大小，以字节为单位。

		long			nFileTimeLength;									//录像文件的播放时间长度，以S（秒）为单位

	}ST_RecordFile;

	//日志请求参数
	typedef struct _PACKED_1_ tagLogRequestParam
	{
		char			szDeviceId[CONST_MAXLENGTH_DEVICEID + 1];		//设备ID

		char			szDeviceIp[CONST_MAXLENGTH_IP + 1];				//设备IP

		long			nCameraId;										//摄像机ID号

		char			szUserName[CONST_MAXLENGTH_USERNAME + 1];		//用户名

		long			nSelectMode;									//查询方式

		long			nMajorType;										//主类型

		long			nMinorType;										//次类型

		unsigned long	nStartTime;										//开始时间

		unsigned long	nEndTime;										//结束时间

	}ST_LogRequestParam;

	//日志信息参数
	typedef struct _PACKED_1_ tagLogInfo
	{
		char			szDeviceId[CONST_MAXLENGTH_DEVICEID + 1];			//设备ID

		char			szDeviceIp[CONST_MAXLENGTH_IP + 1];					//设备IP

		long			nCameraId;											//摄像机ID号

		char			szUserName[CONST_MAXLENGTH_USERNAME + 1];			//用户名

		long			nMajorType;											//主类型

		long			nMinorType;											//次类型

		unsigned long	nLogTime;											//日志产生时间

		char			szLogData[CONST_MAXLENGTH_LOG_DATA + 1];			//日志信息

	}ST_LogInfo;

	typedef struct _PACKED_1_ tagAVFrameData
	{
		long					nStreamFormat;						//1表示原始流，2表示TS混合流

		long					nESStreamType;						//原始流类型，1表示视频，2表示音频

		long					nEncoderType;						//编码格式

		long					nCameraNo;							//摄像机号，表示数据来自第几路

		unsigned long			nSequenceId;						//数据帧序号

		long					nFrameType;							//数据帧类型,1表示I帧, 2表示P帧, 0表示未知类型

		INT64					nTimeStamp;							//数据采集时间戳，单位为毫秒

		char*					pszData;							//数据

		unsigned long			nDataLength;						//数据有效长度

		long					nFrameRate;							//帧率

		long					nBitRate;							//当前码率　

		long					nImageFormatId;						//当前格式

		long					nImageWidth;						//视频宽度

		long					nImageHeight;						//视频高度

		long					nVideoSystem;						//当前视频制式

		unsigned long			nFrameBufLen;						//当前缓冲长度

		long					nStreamId;							// 流ID
		long					nTimezone;							// 时区
		long					nDaylightSavingTime;				//夏令时

	}ST_AVFrameData;

	typedef struct _PACKED_1_ tagVideoInfo
	{
		unsigned long		nBitRate;								//比特率

		unsigned long		nBitErrorRate;

		unsigned long		nTimePerFrame;				

		unsigned long		nSize;

		unsigned long		nWidth;									//视频宽度

		unsigned long		nHeight;								//视频高度

		unsigned short		nPlanes;

		unsigned short		nBitCount;

		unsigned long		nCompression;

		unsigned long		nSizeImage;

		long				nXPelsPerMeter;

		long				nYPelsPerMeter;

		unsigned long		nClrUsed;

		unsigned long		nClrImportant;

		char				pszSPS_PPSData[512];

		long				nSPS_PPSDataLen;

	}ST_VideoInfo;

	typedef struct _PACKED_1_ tagAudioInfo
	{
		unsigned short		nFormatTag;			//格式标记

		unsigned short		nChannels;			//声音通道数（声道）

		unsigned long		nSamplesPerSec;		//每秒采样数

		unsigned long		nAvgBytesPerSec;	//

		unsigned short		nBlockAlign;		//

		unsigned short		nBitsPerSample;		//每次采样的大小

		unsigned short		nCBSize;			//后面追加数据长度

		long				nEncodeType;

	}ST_AudioInfo;

	typedef struct _PACKED_1_ tagAlarmData 
	{
		long		nAlarmSourceId;											//报警源ID号

		char		szAlarmSourceName[CONST_MAXLENGTH_ALARMSOURCENAME + 1];	//报警源名称

		long		nAlarmTime;												//报警时间

		long		nAlarmType;												//报警类型

		long		nEventType;												//环境类型

		long		nAlarmCode;												//报警代码

		long		nAlarmFlag;												//报警标志（0：报警产生，1：报警消除）

	}ST_AlarmData;

	typedef struct _PACKED_1_ tagAlarmInfo
	{
		char			szDeviceId[CONST_MAXLENGTH_DEVICEID + 1];				//设备id

		long			nDeviceType;											//设备类型

		char			szDeviceIp[CONST_MAXLENGTH_IP + 1];						//设备IP地址

		ST_AlarmData	stAlarmData;											//报警数据

	}ST_AlarmInfo;

	typedef struct _PACKED_1_ tagAlarmInfoEx
	{
		char			szDeviceId[CONST_MAXLENGTH_DEVICEID + 1];				//设备id

		long			nDeviceType;											//设备类型

		char			szDeviceIp[CONST_MAXLENGTH_IP + 1];						//设备IP地址

		int				nMajorType;												//报警主类型
		int				nMinorType;												//报警子类型
		char			szSourceId[CONST_MAXLENGTH_ALARM_SOURCE_ID + 1];		//源ID
		char			szSourceName[CONST_MAXLENGTH_ALARMSOURCENAME + 1];		//源名称
		int				nAlarmFlag;												//启动/结束标志,（1：报警产生，0：报警消除）
		unsigned long	nAlarmTime;												//时间
		char			szDescription[CONST_MAXLENGTH_ALARM_DESCRIPTION + 1];	//描述

	}ST_AlarmInfoEx;

	//报警行为类型
	typedef struct _PACKED_1_ tagAlarmActionStrategy
	{
		long					nAlarmActionStrategyNum;										//报警行为策略个数
		long					stAlarmActionStrategy[CONST_MAX_ALARM_ACTION_STRATEGY];				//报警行为策略

	}ST_AlarmActionStrategy;

	typedef struct _PACKED_1_ tagDeviceAccessParam
	{
		char			szLocalIp[CONST_MAXLENGTH_IP + 1];					//本地IP地址

		unsigned short	nControlPort;										//网络视频设备的设备网络控制端口
		unsigned short	nVideoTransferPort;									//网络视频设备的设备视频传输端口
		unsigned short	nAudioTransferPort;									//网络视频设备的设备音频传输端口
		unsigned short	nHttpPort;											//网络视频设备的设备HTTP端口

	}ST_DeviceAccessParam;

	typedef struct _PACKED_1_ tagNetVideoDeviceInfo
	{
		char			szDeviceId[CONST_MAXLENGTH_DEVICEID + 1];			//设备唯一标识

		int				nDeviceType;										//设备类型

		char			szLocalIp[CONST_MAXLENGTH_IP + 1];					//本地IP地址

		unsigned short	nControlPort;										//网络视频设备的设备网络控制端口

		unsigned short	nHttpPort;											//网络视频设备的设备HTTP端口

		char			szHardwareVer[CONST_MAXLENGTH_HARDWAREVERSION + 1];		//硬件版本信息
		char			szSoftwareVer[CONST_MAXLENGTH_SOFTWAREVERSION + 1];		//软件版本信息

		char			szManufacturerId[CONST_MAXLENGTH_MANUFACTURERID + 1];		//厂商ID
		char			szManufacturerName[CONST_MAXLENGTH_MANUFACTURERNAME + 1];	//厂商名称


	}ST_NetVideoDeviceInfo;

	typedef struct _PACKED_1_ tagRecordDirInfo
	{
		int nDiskType;													//目录磁盘类型
		char szDiskName[CONST_MAXLENGTH_FILE_NAME + 1];					//磁盘名称
		int nGroupId;													//磁盘所属盘组编号
		int nUsableSpace;												//磁盘可用空间 单位：M
		int nFreeSpace;													//磁盘剩余空间 单位：M
		int nAttribute;													//属性
		unsigned char bEnableFlag;										//启用标示
		int nAlarmThreshold;											//报警阈值
		int nStatus;													//磁盘状态	
		int	nFileSystemFormat;											//磁盘文件系统格式
	}ST_RecordDirInfo;

	typedef struct _PACKED_1_ tagRecordDirInfoList
	{
		ST_RecordDirInfo stRecordDirInfoList[CONST_MAX_DISK_NUM];			//磁盘信息列表
		int				 nRecordDirInfoNum;									//磁盘个数
	}ST_RecordDirInfoList;

	typedef struct _PACKED_1_ tagRecordQueryCondition
	{
		char			szDeviceId[CONST_MAXLENGTH_DEVICEID + 1];	//设备编号
		char			szDeviceIP[CONST_MAXLENGTH_IP + 1];			//设备IP
		int				nCameraId;									//设备通道号
		unsigned long	nRecordBeginTime;							//录像段开始时间
		unsigned long	nRecordEndTime;								//录像段结束时间
		unsigned char			bIsLockFile;								//录像锁标识
	}ST_RecordQueryCondition;

	typedef struct _PACKED_1_ tagRecordQueryResult
	{
		char			szDeviceId[CONST_MAXLENGTH_DEVICEID + 1];	//设备编号
		char			szDeviceIP[CONST_MAXLENGTH_IP + 1];			//设备IP
		int				nCameraId;									//设备通道号
		unsigned long	nRecordBeginTime;							//录像段开始时间
		unsigned long	nRecordEndTime;								//录像段结束时间
		unsigned char	bIsLockFile;								//录像锁标识
	}ST_RecordQueryResult;

	typedef struct _PACKED_1_ tagRecordInfo
	{
		char					szDeviceId[CONST_MAXLENGTH_DEVICEID + 1];	//设备编号
		char					szDeviceIP[CONST_MAXLENGTH_IP + 1];			//设备IP
		int						nCameraId;									//设备通道号
		unsigned long			nRecordBeginTime;							//录像段开始时间
		unsigned long			nRecordEndTime;								//录像段结束时间
		unsigned char			bIsLockFile;								//录像锁标识
	}ST_RecordInfo;

	//流配置参数
	typedef struct _PACKED_1_ tagAVStreamParam
	{
		int				nCameraId;												//摄像机ID
		int				nStreamId;												//码流ID
		char			szStreamName[CONST_MAXLENGTH_AVSREAM_NAME + 1];				//码流名
		int				nVideoHeight;											//高度
		int				nVideoWidth;											//宽度
		int				nFrameRate;												//帧率
		int				nBitRateType;											//码率类型
		int				nBitRate;												//码率
		int				nQuality;												//视频质量值
		int				nIFrameInterval;										//I帧步长
		int				nVideoEncoderType;										//视频编码类型
		int				nAudioEncoderType;										//音频编码类型
	}ST_AVStreamParam;

	//流配置参数
	typedef struct _PACKED_1_ tagAllAVStreamParam
	{
		int						nStreamNum;

		ST_AVStreamParam		stAVStreamParamList[CONST_MAX_STREAM_NUM];

	}ST_AllAVStreamParam;

	
	//报警事件
	typedef struct _PACKED_1_ tagAlarmEvent
	{
		int nAlarmType;

		int nAlarmId;

	}ST_AlarmEvent;

	//报警录像策略
	typedef struct _PACKED_1_ tagAlarmRecordPolicy
	{
		int								nAlarmPreRecordTime;

		int								nAlarmDelayRecordTime;

		unsigned char					bIsAlarmRecordFileLocked;

		unsigned char					bIsAlarmRecordOpened;

		int								nAlarmEventNum;

		ST_AlarmEvent					stAlarmEventList[CONST_MAX_ALARM_EVENT_NUM];

	}ST_AlarmRecordPolicy;

	//计划录像策略
	typedef struct _PACKED_1_ tagScheduleRecordPolicy
	{
		unsigned char					bIsScheduleRecordOpened;

		int								nScheduleRecordType;

		ST_ScheduleTimeParam			stScheduleTimeParam;

	}ST_ScheduleRecordPolicy;

	//录像策略
	typedef struct _PACKED_1_ tagRecordPolicyParam
	{
		int								nCameraId;				//摄像机编号

		int								nSaveDays;				//保存天数

		unsigned char					bCycleWriteFlag;		//循环写标志

		unsigned char					bIsRedundancy;			//是否冗余

		int								nDiskGroupId;			//存储盘组ID

		int								nStreamId;				//码流ID

		unsigned char					bIsRecordAudioOpened;	//是否录制音频

		ST_AlarmRecordPolicy			stAlarmRecordPolicy;	//报警录制策略

		ST_ScheduleRecordPolicy			stScheduleRecordPolicy;	//计划录像策略

	}ST_RecordPolicyParam;

	//所有的录像策略参数
	typedef struct _PACKED_1_ tagAllRecordPolicyParam
	{
		int								nRecordPolicyNum;

		ST_RecordPolicyParam			stRecordPolicyParam[CONST_MAX_CAMERA_NUM];
		
	}ST_AllRecordPolicyParam;

////////////////////////////////////////////////////////////////

	//侦测区域
	typedef struct _PACKED_1_ tagColorParam
	{
		unsigned long		nRed;						//红色值（0-255）
		unsigned long		nGreen;						//绿色值（0-255）
		unsigned long		nBlue;						//蓝色值（0-255）
		unsigned long		nAlpha;						//透明度（0-100），此参数预留

	}ST_ColorParam;

	//侦测区域
	typedef struct _PACKED_1_ tagAreaParam
	{
		long		nTopX;								//区域左上角 x 的坐标占总视频区宽度的百分比
		long		nTopY;								//区域左上角 y 的坐标占总视频区高度的百分比
		long		nWidth;								//区域宽度占视频区总宽度的百分比
		long		nHeight;							//区域高度占总视频区高度的百分比

	}ST_AreaParam;

	//设备隐私遮蔽参数
	typedef struct _PACKED_1_ tagBlindAreaParam
	{
		long					nCameraId;											//摄像机ID号

		unsigned char			bEnableFlag;										//是否启用遮蔽（1：启用，0：不启用）
		
		ST_ColorParam			stColorParam;										//颜色
		
		long					nAreaParamNum;										//侦测区域数目
		ST_AreaParam			stAreaParamList[CONST_MAX_AREA_NUM];				//移动侦测区域

	}ST_BlindAreaParam;

	//设备所有隐私遮蔽参数
	typedef struct _PACKED_1_ tagAllBlindAreaParam
	{
		long						nBlindAreaParamNum;									//隐私遮蔽参数条目
	
		ST_BlindAreaParam			stBlindAreaParamList[CONST_MAX_CAMERA_NUM];				//隐私遮蔽参数列表

	}ST_AllBlindAreaParam;

	//设备基本信息
	typedef struct _PACKED_1_ tagDeviceBaseInfo
	{
		char				szDeviceId[CONST_MAXLENGTH_DEVICEID + 1];								//设备唯一标识
		char				szDeviceName[CONST_MAXLENGTH_DEVICENAME + 1];							//设备名称
		int					nDeviceType;															//设备类型

		char				szDeviceIP[CONST_MAXLENGTH_IP + 1];										//设备IP地址
		unsigned short		nDevicePort;															//设备端口
		int					nIPProtoVer;															//IP协议类型

		char				szManufacturerId[CONST_MAXLENGTH_MANUFACTURERID + 1];					//设备型号
		char				szManufacturerName[CONST_MAXLENGTH_MANUFACTURERNAME + 1];				//设备生产厂家
		char				szProductModel[CONST_MAXLENGTH_PRODUCTMODEL + 1];						//产品模组
		char				szProductDescription[CONST_MAXLENGTH_PRODUCTDESCRIPTION + 1];			//产品描述
		char				szHardwareModel[CONST_MAXLENGTH_HARDWAREMODEL + 1];						//硬件模组
		char				szHardwareDescription[CONST_MAXLENGTH_HARDWAREDESCRIPTION + 1];			//硬件描述
		char				szMACAddress[CONST_MAXLENGTH_MACADDRESS + 1];							//MAC地址
		char				szBarCode[CONST_MAXLENGTH_BARCODE + 1];									//机器条形码
		char				szProductionTime[CONST_MAXLENGTH_PRODUCTIONTIME + 1];					//生产时间
		
		char				szHardwareVer[CONST_MAXLENGTH_HARDWAREVERSION_EXPAND + 1];				//版本信息
		char				szSoftwareVer[CONST_MAXLENGTH_SOFTWAREVERSION_EXPAND + 1];


		int					nCameraNum;																//通道数（摄像机数）
		int					nAlarmInNum;															//报警输入个数
		int					nAlarmOutNum;															//报警输出个数
		int					nRS485Num;																//RS485串口个数

		ST_CameraInfoParam	stWebCameraInfoParamList[CONST_MAX_CAMERA_NUM];							//摄像头信息

	}ST_DeviceBaseInfo;

	//设备状态信息
	typedef struct _PACKED_1_ tagDeviceStateInfo
	{
		char				szDeviceId[CONST_MAXLENGTH_DEVICEID + 1];							//设备唯一标识

		int					nDeviceType;														//设备类型

		long				nLastActiveTime;													//最后活动时间，精确到秒

		unsigned char		btOnlineState;														//在线状态（0：不在线，1：在线）

		ST_InetAddr			stInetAddr;															//设备的网络地址

		int					m_nSearchMode;														//在线更新方式，1：广播，2：主动搜索，3：在线搜索

	}ST_DeviceStateInfo;


	//外部接口
	typedef struct _PACKED_1_ tagExternInterface
	{
		int					nInterfaceType;
		ST_RS485Param		stRS485Device;

	}ST_ExternInterface;

	//PTZ键盘参数
	typedef struct _PACKED_1_ tagPTZKeyboardParam
	{
		unsigned char		bEnableFlag; //是否启动云台键盘(1：启动，0：不启动)

		ST_ExternInterface	stExternInterface;

	}ST_PTZKeyboardParam;

	//点钞机参数
	typedef struct _PACKED_1_ tagCashRegistersParam
	{
		unsigned char		bEnableFlag;	//是否启动点钞机(1：启动，0：不启动)
		int					nOSDTopX;		//列
		int					nOSDTopY;		//行
		int					nOSDLanguage;	//语言
		int					nModel;			//模式

		int					nOSDCameraIDList[CONST_MAX_CAMERA_NUM]; //通道ID
		int					nCameraNum;								//通道数

		ST_ExternInterface stExternInterface;					//外部接口

	}ST_CashRegistersParam;


	//点钞机参数
	typedef struct _PACKED_1_ tagTimeZoneParam
	{
		int				m_nTimeZone;												//时区

		bool			m_bDSTOpenFlag;												//夏令时开启标志

		int				m_nBeginMonth;												//夏令时开始月份
		int				m_nBeginWeekly;												//夏令时开始周（一月中的第几周）
		int				m_nBeginWeekDays;											//星期几
		unsigned int	m_nBeginTime;												//开始时间

		int				m_nEndMonth;												//夏令时结束月份
		int				m_nEndWeekly;												//夏令时结束周（一月中的第几周）
		int				m_nEndWeekDays;												//星期几
		unsigned int	m_nEndTime;													//结束时间

	}ST_TimeZoneParam;

	//快照参数
	typedef struct _PACKED_1_ tagRemoteSnapshotParam
	{
		int				nCameraID;													//通道号
		int				nQuality;													//质量
		int				nPhotoFormat;												//图像格式

	}ST_RemoteSnapshotParam;

}

/**********************************************************************/
#ifdef PRAGMA_PACK
#ifdef WIN32  
#pragma pack(pop)
#endif
 #endif
/**********************************************************************/
//#pragma pack(push, 1)

#endif
