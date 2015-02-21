
#ifndef __J_XML_PACKET_H__
#define __J_XML_PACKET_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nmp_sdk.h"


typedef struct __Request
{
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char pu_or_gu_id[J_SDK_MAX_ID_LEN];

	int reserve;		//1. 获取串口信息: 串口号
						//2. 联动操作: 报警类型
}Request;

typedef struct __Result
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char pu_or_gu_id[J_SDK_MAX_ID_LEN];
	
	int reserve;		//1. 添加巡航路径: 巡航路径号
}Result;


//==================================================

typedef struct __PuGetCss
{
	JResultCode result;
	char css_ip[J_SDK_MAX_IP_LEN];
    int  css_port;
    char dev_code[J_SDK_MAX_ID_LEN];            //设备出厂编号
    char software_ver[J_SDK_MAX_VERSION_LEN];   //设备软件版本
}PuGetCssPacket;

typedef struct __PuRegisterCss
{
    char dev_code[J_SDK_MAX_ID_LEN];            //设备出厂编号
	char dev_ip[J_SDK_MAX_IP_LEN];
	char css_ip[J_SDK_MAX_IP_LEN];
    char software_ver[J_SDK_MAX_VERSION_LEN];   //设备软件版本
}PuRegisterCssPacket;

typedef struct __RegisterRequestPacket
{
	char pu_id[J_SDK_MAX_ID_LEN];				//上线设备的PUID号
	char dev_ip[J_SDK_MAX_IP_LEN];
	char cms_ip[J_SDK_MAX_IP_LEN];
	JPuType pu_type;
}RegisterRequestPacket;

typedef struct __RegisterResponsePacket
{
	JResultCode result;
	char pu_id[J_SDK_MAX_ID_LEN];
	char mds_ip[J_SDK_MAX_IP_LEN];
	int mds_port;
	int keep_alive;
}RegisterResponsePacket;

typedef struct __HeartBeatRequestPacket
{
	char pu_id[J_SDK_MAX_ID_LEN];
	char dev_ip[J_SDK_MAX_IP_LEN];
}HeartBeatRequestPacket;

typedef struct __HeartBeatResponsePacket
{
	JResultCode result;
	JTime server_time;
	int plt_type;	//1:云平台
}HeartBeatResponsePacket;


//##########################################################

typedef struct __MdsInfoPacket
{
	JResultCode result;
	char pu_id[J_SDK_MAX_ID_LEN];
	char cms_ip[J_SDK_MAX_IP_LEN];
	char mds_ip[J_SDK_MAX_IP_LEN];
	int mds_port;
}MdsInfoPacket;


typedef struct __ChangeDispatchPacket
{
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char pu_id[J_SDK_MAX_ID_LEN];
	
	char mds_ip[J_SDK_MAX_IP_LEN];
	int mds_port;
}ChangeDispatchPacket;

typedef struct __DeviceInfoPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char pu_id[J_SDK_MAX_ID_LEN];
	
	char manu_info[J_SDK_MAX_MANU_INFO_LEN];	//厂家信息
	char release_date[J_SDK_MAX_DATA_LEN];		//出厂日期
	char dev_version[J_SDK_MAX_VERSION_LEN];	//设备版本
	char hw_version[J_SDK_MAX_VERSION_LEN];		//硬件版本
	int pu_type;								//设备类型，详情见JPuType
	int sub_type;								//设备的机器类型，指设备的小类，详情见JPuSubType
	int di_num;									//设备输入接口数目
	int do_num;									//设备输出接口数目
	int channel_num;							//通道数目
	int rs485_num;								//485接口数目，若0，则无
	int rs232_num;								// RS22 接口数目，若0，则无
}DeviceInfoPacket;

typedef struct __DeviceNTPInfoPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char pu_id[J_SDK_MAX_ID_LEN];
	
	char ntp_server_ip[J_SDK_MAX_IP_LEN];		//NTP服务器
	int time_zone;								//时区
	int time_interval;							//对时间隔
	int ntp_enable;								//ntp开关；1：启动，0：不启动
	int dst_enable;								//夏令时；1：启动，0：不启动
	int reserve;								//保留字节
}DeviceNTPInfoPacket;

typedef struct __DeviceTimePacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char pu_id[J_SDK_MAX_ID_LEN];
	
	JTime time;									//时间
	int zone;									//时区
	int sync_enable;							//与平台服务器时间同步是否生效
	int set_flag;								//0: 都有效，1: 仅时间有效，2: 仅同步有效
}DeviceTimePacket;

typedef struct __PlatformInfoPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char pu_id[J_SDK_MAX_ID_LEN];
	
	char cms_ip[J_SDK_MAX_IP_LEN];				//平台cms ip
	char mds_ip[J_SDK_MAX_IP_LEN];				//平台mds ip
	int cms_port;								//平台cms端口号
	int mds_port;								//平台mds端口号
	int protocol;								//数据流的传送协议，详情见JProtocolType 
	int is_con_cms;								//是否连接平台，0: 连接平台
}PlatformInfoPacket;

typedef struct __NetworkInfoPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char pu_id[J_SDK_MAX_ID_LEN];
	
	JNetwork network[J_SDK_MAX_NETWORK_TYPE];	//网络接口信息
	char main_dns[J_SDK_MAX_DNS_LEN];			//主DNS服务器地址
	char backup_dns[J_SDK_MAX_DNS_LEN];			//备用DNS服务器地址
	int auto_dns_enable;						//是否启用自动获取dns
	int cur_network;							//eth0/wifi/3G
	int cmd_port;								//设备信令端口
	int data_port;								//设备数据端口
	int web_port;								//设备web端口
	int talk_port;                              //设备对讲端口
}NetworkInfoPacket;

typedef struct __PPPOEInfoPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char pu_id[J_SDK_MAX_ID_LEN];
	
	int type;									//PPPOE拨号的网络接口，详情见JNetworkType
	int enable;									//
	char ip[J_SDK_MAX_IP_LEN];					//
	char account[J_SDK_MAX_ACCOUNT_LEN];		//PPPOE拨号账号
	char passwd[J_SDK_MAX_PASSWD_LEN];			//PPPOE拨号密码
}PPPOEInfoPacket;

typedef struct __EncodeParameterPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char gu_id[J_SDK_MAX_ID_LEN];
	
	int level;					//码流类型：主(0)、从(1)、次(2)
	int frame_rate;				//帧率
	int i_frame_interval;		//I帧间隔
	int video_type;				//视频编码类型，详情见JAVCodeType
	int audio_type; 			//音频编码类型，详情见JAVCodeType
	int au_in_mode;				//音频输入类型，详情见JAudioInputType
	int audio_enble;			//音频是否打开
	int resolution;				//分辨率，详情见JResolution
	int qp_value;				//质量
	int code_rate;				//码率
	int frame_priority;			//是否帧率优先
	int format;					//制式，详情见JVideoFormat 
	int bit_rate;				//位率类型，详情见JBitRate
	int encode_level;			//编码类型，详情见JEncodeLevel
}EncodeParameterPacket;

typedef struct __DisplayParameterPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char gu_id[J_SDK_MAX_ID_LEN];
	
	int contrast;				//对比度
	int bright;					//亮度
	int hue;					//灰度
	int saturation;				//饱和度
	int sharpness;				//锐度
}DisplayParameterPacket;

typedef struct __RecordParameterPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char gu_id[J_SDK_MAX_ID_LEN];
	
	int level;					//码流类型：主(0)、从(1)、次(2)
	int pre_record;				//通道预录时长，单位：秒
	int auto_cover;				//通道录像是否自动覆盖

	JWeek week;
}RecordParameterPacket;

typedef struct __HideParameterPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char gu_id[J_SDK_MAX_ID_LEN];
	
	int hide_enable;			//是否启用遮挡
	int hide_color;				//遮挡时使用的颜色
	JArea hide_area;			//遮挡区域,数目为0即遮挡无效
	int max_width;				//最大宽度
	int max_height;				//最大高度
}HideParameterPacket;

typedef struct __SerialParameterPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char pu_id[J_SDK_MAX_ID_LEN];
	
	int serial_no;				//串口号
	int baud_rate;				//波特率
	int data_bit;				//数据位 
	int stop_bit;				//停止位
	int verify;					//校验
}SerialParameterPacket;

typedef struct __OSDParameterPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char gu_id[J_SDK_MAX_ID_LEN];
	
	int time_enable;					//是否显示时间；1：显示，0：隐藏
	int time_display_x;					//时间坐标X
	int time_display_y;					//时间坐标Y
	int time_display_color;				//时间显示颜色
	int text_enable;					//是否文本显示；1：显示，0：隐藏
	int text_display_x;					//文本坐标X
	int text_display_y;					//文本坐标Y
	int text_display_color;				//文本显示颜色
	int max_width;						//最大宽度
	int max_height;						//最大高度
	char text_data[J_SDK_MAX_TEXT_DATA_LEN];
	int stream_enable;					//是否显示码流信息；1：显示，0：隐藏
	
	int time_display_w;					//时间OSD 显示宽度
	int time_display_h;					//时间OSD 显示高度
	int text_display_w;					//文本OSD 显示宽度
	int text_display_h;					//文本OSD 显示高度

	uint32_t osd_invert_color;				//osd 反色；1：开启，0：关闭

	JOSDExtend ext_osd;
}OSDParameterPacket;

typedef struct __PTZParameterPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char gu_id[J_SDK_MAX_ID_LEN];
	
	int serial_no;						//串口号
	int ptz_addr;						//云台地址
	int protocol;						//
	int baud_rate;						//波特率
	int data_bit;						//数据位 
	int stop_bit;						//停止位
	int verify;							//校验
}PTZParameterPacket;

typedef struct __PTZControlPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char gu_id[J_SDK_MAX_ID_LEN];
	
	int action;							//云台操作，详情见JPTZDirection
	int param;							//各种操作参数值
}PTZControlPacket;

typedef struct __FTPParameterPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char pu_id[J_SDK_MAX_ID_LEN];
	
	char ftp_ip[J_SDK_MAX_IP_LEN];			//ftp服务器ip
	char ftp_usr[J_SDK_MAX_ACCOUNT_LEN];	//ftp服务器port
	char ftp_pwd[J_SDK_MAX_PASSWD_LEN];		//登陆用户名
	char ftp_path[J_SDK_MAX_PATH_LEN];		//登陆密码
	int ftp_port;							//Ftp上传路径
}FTPParameterPacket;

typedef struct __SMTPParameterPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char pu_id[J_SDK_MAX_ID_LEN];
	
	char mail_ip[J_SDK_MAX_IP_LEN];			//smtp 服务器地址
	char mail_addr[J_SDK_MAX_ADDR_LEN];		//发送邮件地址
	char mail_usr[J_SDK_MAX_ACCOUNT_LEN];	//发送邮件账号
	char mail_pwd[J_SDK_MAX_PASSWD_LEN];	//发送邮件密码
	char mail_rctp1[J_SDK_MAX_ADDR_LEN];	//接受邮件地址1
	char mail_rctp2[J_SDK_MAX_ADDR_LEN];	//接受邮件地址2
	char mail_rctp3[J_SDK_MAX_ADDR_LEN];	//接受邮件地址3
	int mail_port;							//smtp 服务器端口
	int ssl_enable;							//是否启用SSL
}SMTPParameterPacket;

typedef struct __UPNPParameterPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char pu_id[J_SDK_MAX_ID_LEN];
	
	char upnp_ip[J_SDK_MAX_IP_LEN];			//upnp 服务器IP
	int upnp_enable;						//upnp 开关
	int upnp_eth_no;						//0使用本机网卡映射 1 使用无线网卡映射
	int upnp_model;							//0固定映射 1自动映射
	int upnp_refresh_time;					//upnp 刷新时间(单位小时)
	int upnp_data_port;						//固定映射模式下, 数据端口的外网端口
	int upnp_web_port;						//固定映射模式下, Web端口的外网端口
	int upnp_data_port_result;				//数据端口的外网端口 0代表没有映射或不成功
	int upnp_web_port_result;				//Web端口的外网端口 0代表没有映射或不成功
	int upnp_cmd_port;                      //命令端口的外网端口 
    int upnp_talk_port;                     //对讲端口的外网端口 
    int upnp_cmd_port_result;               //命令端口的外网端口映射状态 0代表没有映射或不成功
    int upnp_talk_port_result;              //对讲端口的外网端口映射状态 0代表没有映射或不成功
}UPNPParameterPacket;

typedef struct __DeviceDiskInfoPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char pu_id[J_SDK_MAX_ID_LEN];
	
	int disk_num;
	JDiskInfo disk[J_SDK_MAX_DISK_NUMBER];
}DeviceDiskInfoPacket;	

typedef struct __FormatDiskPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char pu_id[J_SDK_MAX_ID_LEN];
	
	int disk_no;							//磁盘号
	int progress;							//格式化进度(0 - 100)
}FormatDiskPacket;

typedef struct __FormatProgressPacket
{
	char pu_id[J_SDK_MAX_ID_LEN];
	
	int disk_no;							//磁盘号
	int progress;							//格式化进度(0 - 100)
}FormatProgressPacket;

typedef struct __MoveAlarmPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char gu_id[J_SDK_MAX_ID_LEN];
	
	int move_enable;
	int sensitive_level;
	int detect_interval;
	int max_width;
	int max_height;
	JArea detect_area;
	JWeek week;
}MoveAlarmPacket;

typedef struct __LostAlarmPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char gu_id[J_SDK_MAX_ID_LEN];
	
	int lost_enable;
	int detect_interval;
	JWeek week;
}LostAlarmPacket;

typedef struct __HideAlarmPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char gu_id[J_SDK_MAX_ID_LEN];
	
	int hide_enable;
	int detect_interval;
	int sensitive_level;					//灵敏度字段，范围 0-4，值越低，越灵敏
	int max_width;
	int max_height;
	JArea detect_area;
	JWeek week;
}HideAlarmPacket;

typedef struct __IoAlarmPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char gu_id[J_SDK_MAX_ID_LEN];
	
	int io_type;					//详情见JIOType
	int alarm_enable;				//
	int detect_interval;			//
	JWeek week;						//
}IoAlarmPacket;

typedef struct __JointActionPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char gu_id[J_SDK_MAX_ID_LEN];
	
	int alarm_type;					//详情见JAlarmType
	JJoint joint;					//
}JointActionPacket;

typedef struct __SubmitAlarmPacket
{
	char pu_id[J_SDK_MAX_ID_LEN];
	
	char data[J_SDK_MAX_ALARM_DATA_LEN];
	int channel;
	int alarm_type;					//详见 JAlarmType
	int action_type;				//0：开始告警，1：结束告警
	JTime alarm_time;
}SubmitAlarmPacket;

typedef struct __MediaUrlPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char gu_id[J_SDK_MAX_ID_LEN];
	
	int level;						//码流类型
	int media_type;					//详见 JMediaType
	char ip[J_SDK_MAX_IP_LEN];		//
	char url[J_SDK_MAX_URL_LEN];	//
	char cu_ip[J_SDK_MAX_IP_LEN];	//客户端IP
}MediaUrlPacket;

typedef struct __StoreLogPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char gu_id[J_SDK_MAX_ID_LEN];

	int rec_type;
	int beg_node;
	int end_node;
	JTime beg_time;
	JTime end_time;
	
	int node_count;
	int total_count;
	int sess_id;
	Store store[J_SDK_MAX_STORE_LOG_SIZE];
}StoreLogPacket;

typedef struct __FirmwareUpgradePacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char pu_id[J_SDK_MAX_ID_LEN];

	int percent;
	int data_len;
	int file_len;
	int sess_id;
	char addr[J_SDK_MAX_IP_LEN];
	char data[J_SDK_MAX_UPGRADE_DATA_LEN];
}FirmwareUpgradePacket;

typedef struct __UpgradeProgressPacket
{
	char pu_id[J_SDK_MAX_ID_LEN];
	
	int percent;
}UpgradeProgressPacket;

typedef struct __ClientSimulatorConfigPacket
{
    int count;
    Channel channel[J_MAX_CHANNEL_INFO_SIZE];
}ClientSimulatorConfigPacket;

typedef struct __ChannelInfoPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char pu_id[J_SDK_MAX_ID_LEN];
	
	int ch_no;							//通道号（0-255）
	int ch_type;						//通道类型(本地、网络)
	int ch_status;						//通道状态（禁止、正常、正在链接）
	char ch_name[J_SDK_MAX_NAME_LEN];	//通道名称
	JRemoteChannelInfo rmt_ch_info;		//远程设备通道信息
	JRemoteDeviceInfo rmt_dev_info;		//远程设备信息
}ChannelInfoPacket;

typedef struct __PictureInfoPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char pu_id[J_SDK_MAX_ID_LEN];
	
	int mirror;					//镜像(图像左右调换) 1 镜像 0 不镜像
	int flip;					//翻转(图像上下调换) 1 翻转 0 不翻转
	int hz;						//电源频率 0 50Hz 1 60 Hz
	int awb_mode;				//白平衡模式 0 自动白平衡 1手动白平衡
	int awb_red;				//白平衡红色强度 0-255
	int awb_blue;				//白平衡蓝色强度 0-255
	int wdr;					//宽动态0 关闭 1-255为宽动态强度
	int iris_type;				//光圈类型 0 手动光圈 1 自动光圈
	int exp_compensation;		//曝光补偿 0-255
	int ae_mode;		 		//自动曝光模式（0-2）：0 噪声优先	  1 帧率优先	2 增益优先
}PictureInfoPacket;

typedef struct __WifiConfigPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char pu_id[J_SDK_MAX_ID_LEN];
	
	char essid[J_SDK_MAX_ID_LEN];	//服务区别号
	char pwd[J_SDK_MAX_PWD_LEN];	//密码
	int wifi_enable;				//0：断开，1：连接
	int encrypt_type;				//加密类型0:NONE  1:WPA  2:WPA2 3:WEP
	int auth_mode;					//认证方式0:NONE  1:EAP 2:PSK 3:OPEN 4:SHARED 
	int secret_key_type;			//密钥管理方式 0:NONE  1:AES 2:TKIP 只对应于加密类型为WPA/WPA2的情况
	int wifi_st;					//无线网卡连接状态 0 无连接 1 正在连接2 成功
}WifiConfigPacket;

typedef struct __WifiSearchResPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char pu_id[J_SDK_MAX_ID_LEN];
	
	int count;				//搜索到的wifi 个数
	JWifiApInfo wifi_ap[J_SDK_MAX_WIFI_AP_SIZE];	//
}WifiSearchResPacket;

typedef struct __NetworkStatusPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char pu_id[J_SDK_MAX_ID_LEN];
	
	int eth_st;				//有线网卡连接状态 0 无连接 1 正在连接2 成功
	int wifi_st;			//无线网卡连接状态0 无连接 1 正在连接2 成功
	int pppoe_st;			//pppoe拨号状态0 无连接 1 正在连接2 成功
}NetworkStatusPacket;

typedef struct __ControlDevicePacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char pu_id[J_SDK_MAX_ID_LEN];
	
	int command;			//控制命令类型
}ControlDevicePacket;

typedef struct __DdnsConfigPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char pu_id[J_SDK_MAX_ID_LEN];
	
	char ddns_account[J_SDK_MAX_NAME_LEN];		//DDNS 注册名
	char ddns_usr[J_SDK_MAX_NAME_LEN];			//用户名称
	char ddns_pwd[J_SDK_MAX_PWD_LEN];			//用户密码
	int ddns_open;								//DDNS开关
	int ddns_type;								//DDNS类型（0-dyndns 1-3322）
	int ddns_port;								//DDNS服务器端口
	int ddns_times;								//更新时间
}DdnsConfigPacket;

typedef struct __AvdConfigPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char pu_id[J_SDK_MAX_ID_LEN];
	
    int 		enable;                			//avd 使能
    JSegment	sched_time[J_SDK_MAX_SEG_SZIE]; //时间段
    JAvdRule	avd_rule[MAX_IVS_AVD_RULES];  	//视频诊断规则，index 下标取值见 JAvdType
}AvdConfigPacket;


//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
typedef struct __TransparentPacket
{
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char pu_id[J_SDK_MAX_ID_LEN];
	
	int type;
	int channel;
	int length;
	void *data;
}TransparentPacket;
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

#ifdef _USE_DECODER_PROTO_

typedef struct __DivisionModePacket
{
	JResultCode result;
	char pu_id[J_SDK_MAX_ID_LEN];
	JDivisionMode div_mode;
}DivisionModePacket;

typedef struct __ScreenStatePacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	int dis_channel;
	JScreenState scr_state;
}ScreenStatePacket;

typedef struct __ChangeDModePacket
{
	JResultCode result;
	JChangeDMode cd_mode;
}ChangeDModePacket;

typedef struct __FullScreenPacket
{
	JResultCode result;
	JScreen screen;
}FullScreenPacket;

typedef struct __TVWallPlayPacket
{
	JResultCode result;
	JTVWallPlay tv_play;
}TVWallPlayPacket;

typedef struct __ClearDivisionPacket
{
	JResultCode result;
	JScreen screen;
}ClearDivisionPacket;

# endif //_USE_DECODER_PROTO_

typedef struct __OperationLogPacket
{
	JResultCode result;
	JOperationLog opt_log;
}OperationLogPacket;

/*typedef struct __AlarmUploadCfgPacket
{
	JResultCode result;
	JAlarmUploadCfg au_cfg;
}AlarmUploadCfgPacket;*/

typedef struct __PPConfigPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char gu_id[J_SDK_MAX_ID_LEN];
	
	JPPConfig pp_cfg;
}PPConfigPacket;

typedef struct __PPSetPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char gu_id[J_SDK_MAX_ID_LEN];
	
	JPPSet pp_set;
}PPSetPacket;

typedef struct __CruiseConfigPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char gu_id[J_SDK_MAX_ID_LEN];
	
	JCruiseConfig crz_cfg;
}CruiseConfigPacket;

typedef struct __CruiseWayPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char gu_id[J_SDK_MAX_ID_LEN];
	
	JCruiseWay crz_way;
}CruiseWayPacket;

typedef struct __CruiseWaySetPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char gu_id[J_SDK_MAX_ID_LEN];
	
	JCruiseWaySet crz_set;
}CruiseWaySetPacket;

typedef struct __3DControlPacket
{
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char gu_id[J_SDK_MAX_ID_LEN];
	
	J3DControl _3d_ctr;
}_3DControlPacket;

typedef struct __IrcutCtrlPacket
{
    JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char gu_id[J_SDK_MAX_ID_LEN];
	
	JIrcutCtrl ir_ctrl;
}IrcutCtrlPacket;

typedef struct __LinkIOPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char gu_id[J_SDK_MAX_ID_LEN];
	
	JLinkIO link_io;
}LinkIOPacket;

typedef struct __LinkPresetPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char gu_id[J_SDK_MAX_ID_LEN];
	
	JLinkPreset link_preset;
}LinkPresetPacket;

typedef struct __ResolutionInfoPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char gu_id[J_SDK_MAX_ID_LEN];
    
    JResolutionInfo rsl_info;
}ResolutionInfoPacket;

typedef struct __IrcutControlPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char gu_id[J_SDK_MAX_ID_LEN];

    JIrcutControl ircut_ctrl;
}IrcutControlPacket;


typedef struct __ExtranetPortPacket
{
	JResultCode result;
	char session_id[J_SDK_MAX_ID_LEN];
	char domain_id[J_SDK_MAX_ID_LEN];
	char gu_id[J_SDK_MAX_ID_LEN];

    JExtranetPort extranet;
}ExtranetPortPacket;

typedef struct __HerdAnalysePacket
{
    JResultCode result;
    char session_id[J_SDK_MAX_ID_LEN];
    char domain_id[J_SDK_MAX_ID_LEN];
    char gu_id[J_SDK_MAX_ID_LEN];

    JHerdAnalyse herd_analyse;
}HerdAnalysePacket;

typedef struct __GrassPercentPacket
{
    JResultCode result;
    char session_id[J_SDK_MAX_ID_LEN];
    char domain_id[J_SDK_MAX_ID_LEN];
    char gu_id[J_SDK_MAX_ID_LEN];

    JGrassPercent grass;
}GrassPercentPacket;

typedef struct __P2PIdPacket
{
    JResultCode result;
    JP2PId p2p;
}P2PIdPacket;

typedef struct __P2PIdPacketReq
{
    JResultCode result;
    JP2PId p2p;
	int channel_count;
}P2PIdPacketReq;

#endif	//__J_XML_PACKET_H__

