/*
 * =====================================================================================
 *
 *       Filename:  sdk_struct.h
 *
 *    Description:  数据结构定义
 *
 *        Version:  1.0
 *        Created:  2011年05月26日 02时35分09秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  maohw (), maohongwei@gmail.com
 *        Company:  jxj
 *
 * =====================================================================================
 */
#ifndef __sdk_struct_h__
#define __sdk_struct_h__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>


#define MAX_VER_STR_LEN         32      //版本信息长度
#define MAX_SERIAL_NO_LEN       32      //序列号长度
#define MAX_DVR_NAME_LEN        32      //设备名称长度
#define MAX_CHANN_NAME_LEN      32      //通道名称长度
#define MAX_CHANN_NUM           32      //最大通道个数
#define MAX_USER_NAME_LEN       32      //用户名长度
#define MAX_PASSWD_LEN          32      //密码长度
#define MAX_ALARM_IN_NAME_LEN   32      //报警输入名称长度
#define MAX_WEEK_NUM            8       //星期天数
#define MAX_TIME_SEG_NUM        4       //时间段个数
#define MAX_ALARM_OUT_NUM       16      //最大报警输出个数
#define MAX_ALARM_IN_NUM        16      //最大报警输入个数
#define MAX_IP_ADDR_LEN         40      //IP地址长度
#define MAX_MAC_ADDR_LEN        20      //MAC地址长度
#define MAX_ETHERNET_NUM        4       //网卡个数
#define MAX_RECORD_ITEM_LEN     64      //录像记录名称长度
#define MAX_LOG_DESC_LEN        128     //日志记录内容长度
#define MAX_USER_NUM            16      //最大用户个数
#define MAX_PRESET_NUM          256     //最大云台预置位个数
#define MAX_CRUISE_NUM          256     //最大云台巡航个数
#define MAX_CRUISE_PRESET_NUM   16      //每条巡航中最大支持的预置位个数
#define MAX_CHANN_CONN_NUM      8       //每个通道最大支持的链接数
#define MAX_DISK_NUM            8       //最大磁盘个数
#define MAX_ALARM_DESC_LEN      64      //报警记录内容长度
#define MAX_SCREEN_NUM          4       //本地最大屏幕个数
#define MAX_WINDOWS_NUM         32      //本地预览视频最大窗口个数
#define MAX_PREVIEW_MODE        8       //最大预览布局个数
#define MAX_OVERLAY_NUM         4       //每个通道最大覆盖区域个数
#define MAX_MOTION_NUM          4       //每个通道最大移动侦测区域个数
#define MAX_OSD_STR_LEN         64      //OSD字符串长度
#define MAX_NTP_SETVER_STR_LEN  32      //NTP服务器地址长度
#define MAX_BACKUP_ITEM			16		//最大备份段数

#define MAX_VDA_SENSITIVE_LEVEL     (9)

/*----------- 消息定义 -----------*/

#define MAX_MSG_SIZE        (64*1024)
#define CHANN_TYPE_ALL      0xFA
#define PARAM_MASK_ALL		0xFFFF		// 恢复整机参数

#define DEV_TYPE_DVR		1			// dvr
#define DEV_TYPE_NVR		2			// nvr

typedef enum _SDK_MSG_DIR{
      SDK_MSG_RESP= 0x00
    , SDK_MSG_REQ = 0x01
}SDK_MSG_DIR_E;

typedef struct sdk_msg_s {
    uint16_t magic;         //保留，扩展(如设备类型)
    uint16_t version;       //版本号
#if 1
    union {
        struct {
            uint8_t src;    //源模块ID
            uint8_t dst;    //目标模块ID
        }__attribute__((packed)) id;
        uint16_t session_id;//用户会话ID(设备端分配)
    }__attribute__((packed));
    uint16_t sequence_id;   //命令序号
#endif
#if 1
    uint16_t msg_id;        //命令ID
    uint8_t  f_dir:1;       //（SDK_MSG_REQ, SDK_MSG_RESP)
    uint8_t  f_pkg:1;       //1:extend字段用作分包;
    uint8_t  f_res:6;       //保留
    uint8_t  ack;           //错误码
#endif
    uint32_t args;          //命令参数(设置、获取参数;或者是子命令类型[用户管理、升级、磁盘管理])
    uint32_t extend;        //扩展字段(8:pkg_total,8:pkg_num, 8:, 8:)
    uint32_t chann;         //通道号(CHANN_TYPE_ALL)
    uint32_t size;          //消息长度
    uint8_t  data[0];       //消息体
}sdk_msg_t;

/*----------- 参数方向 -----------*/

typedef enum _SDK_PARAM_DIR{
      SDK_PARAM_SET	= 0x01	    //设置参数
    , SDK_PARAM_GET        		//查询参数
}SDK_PARAM_DIR_E;


/*----------- 错误码定义 -----------*/

typedef enum _SDK_ERR_CODE {
      SDK_ERR_SUCCESS = 0x00        //成功
    , SDK_ERR_FAILURE               //失败
    , SDK_ERR_PASS                  //密码错误
    , SDK_ERR_USER_NOT_EXIST		//用户名不存在
    , SDK_ERR_ROOT_USER				//root用户不能删除
    , SDK_ERR_TOO_MANY_USER			//用户已满(最多可添加16个用户名)
    , SDK_ERR_USER_ALREADY_EXIST	//用户名已存在
    , SDK_ERR_NET_CFG				//配置网络参数失败
    , SDK_ERR_PERMIT                //权限错误
    , SDK_ERR_CHANN                 // 10 通道号错误
    , SDK_ERR_CONN_MAX              //超出最大链接数
    , SDK_ERR_CONN                  //链接错误
    , SDK_ERR_SEND                  //数据发送错误
    , SDK_ERR_RECV                  //数据接收错误
    , SDK_ERR_BUSY                  //设备正忙
    , SDK_ERR_DATA                  //数据格式错误
    , SDK_ERR_CMD                   //命令字错误
    , SDK_ERR_VER                   //协议版本错误
    , SDK_ERR_NO_DISK               //无磁盘
    , SDK_ERR_DISK_ERR              // 20 磁盘错误
    , SDK_ERR_RESOURCE              //资源不可用
    , SDK_ERR_FLASH                 //FLASH读写错误
    , SDK_ERR_SET_SYSTIME			//设置系统时间错误
    , SDK_ERR_SET_ENCODE_PARAM		//设置编码参数错误
    , SDK_ERR_SET_MD_PARAM			//设置移动侦测参数错误
    , SDK_ERR_SET_OSD_PARAM			//设置OSD参数错误
    , SDK_ERR_SET_OVERLAY_PARAM		//设置遮挡区域参数错误
    , SDK_ERR_SET_TV_PARAM			//设置边距大小错误
    , SDK_ERR_SET_IMAGE_ATTR		//设置图像属性错误
	, SDK_ERR_LOG_QUERY				//查询日志失败
	, SDK_ERR_LOG_EMPTY				//清空日志失败
	, SDK_ERR_LOG_BACKUP			//备份日志失败
	, SDK_ERR_SERIAL_NO				//串口号错误
	, SDK_ERR_SERIAL_OPEN			//打开串口失败
	, SDK_ERR_SERIAL_PARAM_SET		//设置串口参数失败
	, SDK_ERR_DISK_MOUNT			//磁盘挂载失败
	, SDK_ERR_UMOUNT				//磁盘卸载失败
	, SDK_ERR_BACKUP				//备份失败	
	, SDK_ERR_LOG_DEL				//删除日志失败
	
    , SDK_ERR_GET_DISK_PART_INOF	//获取磁盘分区信息错误
    , SDK_ERR_UPGRADING				//正在升级中
    , SDK_ERR_UPGRADE_CHECK_HEAD	//校验升级包头错误
    , SDK_ERR_UPGRADE_FILE_FIND		// 查找不到升级文件
    , SDK_ERR_UPGRADE				// 升级失败

	, SDK_ERR_NVR_NOT_LOGIN				// 前端设备还没登陆成功
  
}SDK_ERR_CODE_E;

/*----------- 告警类型定义 -----------*/

typedef enum _SDK_EVENT_TYPE{
	  SDK_EVENT_MANUAL_REC = 0x000	//手动录像事件
    , SDK_EVENT_ALARM_IN    		//输入告警事件
    , SDK_EVENT_MOTION              //移动侦测事件
    , SDK_EVENT_LOST                //视频丢失事件
    , SDK_EVENT_HIDE                //视频遮挡事件(目前不做)
    , SDK_EVENT_TIMER_REC           //定时录像事件
    , SDK_EVENT_HD_ERR              //磁盘错误事件
    , SDK_EVENT_HD_IS_EXIST			//录像盘是否存在事件 (pmsg->extend: 不存在/存在[0/1])

	/*8******* 专用于上报给其它模块的消息 *************/
	, SDK_EVENT_REC_MSG				//录像是否开启事件
	, SDK_EVENT_ALARM_IN_MSG
	, SDK_EVENT_MOTION_MSG
	, SDK_EVENT_LOST_MSG
	, SDK_EVENT_HIDE_MSG
	/******** ***********************/
    
    /*13*********add 20120601 for  NVR***************/
    , SDK_DEV_LOGIN				//登录
    , SDK_DEV_TIMEOUT           //登录超时
    , SDK_DEV_LOGOUT            //注销
	, SDK_STREAM_OPEN	//正在传送
	, SDK_STREAM_CLOSE  		//连接关闭
	
    /************add end******************/
	, SDK_EVENT_ALL
    , SDK_EVENT_BUTT
}SDK_EVENT_TYPE_E;


/*----------- 音视频类型定义 -----------*/


typedef enum _SDK_AUDIO_SAMPLE_RATE
{
	SDK_AUDIO_SAMPLE_R8K        = 0,   /* 8K Sample rate     */
	SDK_AUDIO_SAMPLE_R11_025K   = 1,   /* 11.025K Sample rate*/
	SDK_AUDIO_SAMPLE_R16K       = 2,   /* 16K Sample rate    */
	SDK_AUDIO_SAMPLE_R22050     = 3,   /* 22.050K Sample rate*/
	SDK_AUDIO_SAMPLE_R24K       = 4,   /* 24K Sample rate    */
	SDK_AUDIO_SAMPLE_R32K       = 5,   /* 32K Sample rate    */
	SDK_AUDIO_SAMPLE_R44_1K     = 6,   /* 44.1K Sample rate  */
	SDK_AUDIO_SAMPLE_R48K       = 7,   /* 48K Sample rate    */
    SDK_AUDIO_SAMPLE_BUTT,
}SDK_AUDIO_SAMPLE_RATE_E;

typedef enum _SDK_AUDIO_CODEC_FORMAT
{
	SDK_AUDIO_FORMAT_NULL	        = 0,   /*                    */
	SDK_AUDIO_FORMAT_G711A	        = 1,   /* G.711 A            */
	SDK_AUDIO_FORMAT_G711Mu	        = 2,   /* G.711 Mu           */
	SDK_AUDIO_FORMAT_ADPCM	        = 3,   /* ADPCM              */
	SDK_AUDIO_FORMAT_G726_16        = 4,   /* G.726              */
	SDK_AUDIO_FORMAT_G726_24        = 5,   /* G.726              */
	SDK_AUDIO_FORMAT_G726_32        = 6,   /* G.726              */
	SDK_AUDIO_FORMAT_G726_40        = 7,   /* G.726              */
	SDK_AUDIO_FORMAT_AMR	        = 8,   /* AMR encoder format */
	SDK_AUDIO_FORMAT_AMRDTX	        = 9,   /* AMR encoder formant and VAD1 enable */
	SDK_AUDIO_FORMAT_AAC	        = 10,  /* AAC encoder        */
	SDK_AUDIO_FORMAT_ADPCM_DVI4	    = 11,  /* ADPCM              */
	SDK_AUDIO_FORMAT_ADPCM_IMA	    = 12,  /* ADPCM              */
	SDK_AUDIO_FORMAT_MEDIA_G726_16  = 13,  /* G.726              */
	SDK_AUDIO_FORMAT_MEDIA_G726_24  = 14,  /* G.726              */
	SDK_AUDIO_FORMAT_MEDIA_G726_32  = 15,  /* G.726              */
	SDK_AUDIO_FORMAT_MEDIA_G726_40  = 16,  /* G.726              */
    SDK_AUDIO_FORMAT_BUTT,
}SDK_AUDIO_CODEC_FORMAT_E;

typedef enum _SDK_AUDIO_SAMPLE_WIDTH {
    SDK_AUDIO_SAMPLE_WIDTH_8  = 0,    /* 8bits */
    SDK_AUDIO_SAMPLE_WIDTH_16 = 2,    /* 16bits */
    SDK_AUDIO_SAMPLE_WIDTH_BUTT,
}SDK_AUDIO_SAMPLE_WIDTH_E;


/*
 * 音频帧信息(子结构体)
 */
typedef struct sdk_a_frame_info_s {
    uint8_t encode_type;        //编码类型 SDK_AUDIO_CODEC_FORMAT_E
    uint8_t samples;            //采样频率 SDK_AUDIO_SAMPLE_RATE_E
    uint8_t bits;               //位宽     SDK_AUDIO_SAMPLE_WIDTH_E
    uint8_t channels;           //通道数
}sdk_a_frame_info_t;


typedef enum _SDK_VIDEO_ENCODE_FORMAT {
    SDK_VIDEO_FORMAT_H264    = 0, //H.264
    SDK_VIDEO_FORMAT_MPEG4   = 1, //MPEG4
    SDK_VIDEO_FORMAT_MJPEG   = 2, //MJPEG
}SDK_VIDEO_ENCODE_FORMAT_E;


typedef enum _SDK_VIDEO_STANDARD {
    SDK_VIDEO_STANDARD_PAL  = 0, // PAL
    SDK_VIDEO_STANDARD_NTSC = 1, //NTSC
}SDK_VIDEO_STANDARD_E;

typedef enum _SDK_VIDEO_RESOLUTION
{
    SDK_VIDEO_RESOLUTION_QCIF = 0, // (176x144)
    SDK_VIDEO_RESOLUTION_CIF  = 1, // (352x288)
    SDK_VIDEO_RESOLUTION_HD1  = 2, // (704x288)
    SDK_VIDEO_RESOLUTION_D1   = 3, // (704x576)
    SDK_VIDEO_RESOLUTION_MD1  = 4,
    SDK_VIDEO_RESOLUTION_QVGA = 5, // (320x240)
    SDK_VIDEO_RESOLUTION_VGA  = 6, // (640x480）
    SDK_VIDEO_RESOLUTION_720p = 7, // (1280x720)
    SDK_VIDEO_RESOLUTION_1080p= 8, // (1920x1080)
    SDK_VIDEO_RESOLUTION_UXGA = 9, // (1600x1200)
    SDK_VIDEO_RESOLUTION_XGA  = 10,// (1024x768)
    SDK_VIDEO_RESOLUTION_SVGA = 11,// (800x600)
    SDK_VIDEO_RESOLUTION_SXGA = 12,// (1280x1024)
    SDK_VIDEO_RESOLUTION_QXGA = 13,// (2048x1536)
    
    SDK_VIDEO_RESOLUTION_960H = 14,// (960x576)
    
    SDK_VIDEO_RESOLUTION_BUTT      // ----------

}SDK_VIDEO_RESOLUTION_E;


typedef enum _SDK_CODEC_AO_DEV_E
{
    SDK_CODEC_COMM_AO_DEV = 0
    , SDK_CODEC_HDMI_AO_DEV = 1
    , SDK_CODEC_AO_DEV_BUTT
}SDK_CODEC_AO_DEV_E;
typedef enum _SDK_CODEC_AO_MOD_E
{
    SDK_CODEC_PREVIEW_AO_MOD = 0
    , SDK_CODEC_PLAYBACK_AO_MOD = 1
    , SDK_CODEC_TALK_AO_MOD = 2
    , SDK_CODEC_MUTE_AO_MOD = 3
}SDK_CODEC_AO_MOD_E;
/*
 * 视频帧信息(子结构体) 
 */
typedef struct sdk_v_frame_info_s {
    uint8_t encode_type;        //编码类型 SDK_VIDEO_ENCODE_FORMAT_E
    uint8_t standard;           //制式     0: PAL, 1: NTSC
    uint8_t resolution;         //分辨率   SDK_VIDEO_RESOLUTION_E
    uint8_t frame_rate;         //帧率     1-25/30 
}sdk_v_frame_info_t;



typedef enum _SDK_FRAME_TYPE {
    SDK_VIDEO_FRAME_I = 0x01,   //I帧
    SDK_VIDEO_FRAME_P = 0x02,   //P帧
    SDK_VIDEO_FRAME_B = 0x03,   //B帧
    SDK_AUDIO_FRAME_A = 0x04,   //音频帧A
}SDK_FRAME_TYPE_E;

/*
 * 音视频帧结构体定义
 */
typedef struct sdk_frame_s {
    uint16_t  magic;            //保留，扩展 
    uint8_t   res[1];           //保留
    uint8_t   frame_type;       //帧类型, 视频（I, P, B）, 音频（A）
    uint32_t  frame_size;       //帧长度
    uint32_t  frame_no;         //帧序号
    uint32_t  sec;              //帧时间（秒）
    uint32_t  usec;             //帧时间（微秒）
    uint64_t  pts;              //帧PTS
    union
    {
        sdk_v_frame_info_t video_info;//视频帧信息
        sdk_a_frame_info_t audio_info;//音频帧信息
    };
    uint8_t   data[0];          //帧数据
}sdk_frame_t;


/*----------- 配置参数结构定义 -----------*/


/*
 * 时间结构体定义
 */
typedef struct sdk_time_s{
    uint32_t year;
    uint32_t mon;
    uint32_t day;
    uint32_t hour;
    uint32_t min;
    uint32_t sec;
}sdk_time_t;

/*
 * 详细时间结构体定义
 */
 typedef struct sdk_date_s{
    sdk_time_t _time;
    uint8_t     wday;
    uint8_t     tzone;
    uint8_t     res[2];
}sdk_date_t;



/*
 * 区域结构体定义
 */
typedef struct sdk_rect_s{
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
}sdk_rect_t;

/*
 * 点结构体定义
 */
typedef struct sdk_point_s{
    uint16_t x;
    uint16_t y;
}sdk_point_t;

/*
 * 时间段结构体定义
 */
typedef struct sdk_sched_time_s{
	uint8_t enable;	    //激活, 当定时录像时表示录像类型;
	uint8_t res[3];
    uint8_t start_hour; //开始时间
	uint8_t start_min;
	uint8_t stop_hour;  //结束时间
	uint8_t stop_min;
}sdk_sched_time_t;


/*
 * 网络升级流程： SDK_UPGRAD_REQUEST => recv upg_pkg => SDK_UPGRAD_DISK_FILE => SDK_UPGRAD_PROGRESS;
 * GUI升级流程 :  SDK_UPGRAD_FIND_FILE => SDK_UPGRAD_DISK_FILE => SDK_UPGRAD_PROGRESS;
 */
typedef enum _SDK_UPGRAD_OP {
      SDK_UPGRAD_REQUEST   = 0x001  //升级请求				upgrade_packet_t
    , SDK_UPGRAD_DISK_FILE          //升级包在磁盘文件
    , SDK_UPGRAD_MEM_FILE           //升级包在内存
    , SDK_UPGRAD_PROGRESS           //升级进度
    , SDK_UPGRAD_FIND_FILE          //GUI查询升级包文件列表	upgrad_pkg_file_t
}SDK_UPGRAD_OP_E;

/*
 * GUI查询升级包文件列表
 */
typedef struct upgrad_pkg_file_s {
    uint8_t   device_type[16];//设备类型
    uint8_t   ver[16];        //版本
    uint8_t   date[16];       //日期
    uint32_t  size;           //大小
    uint8_t   name[32];       //名字 aa.upg
    uint8_t   file_path[64];  //升级文件全路经 /tmp/aa.upg
}upgrad_pkg_file_t;


typedef enum _SDK_USER_OP {
      SDK_USER_AAA = 0x001  //sdk_user_t
    , SDK_USER_ADD          //sdk_user_right_t
    , SDK_USER_DEL          //sdk_user_t
    , SDK_USER_MODI         //sdk_user_right_t
    , SDK_USER_QUERY        //sdk_user_t, sdk_user_right_t
}SDK_USER_OP_E;

typedef enum _SDK_DISK_OP {
      SDK_DISK_QUERY		= 0x001		//sdk_disk_t
    , SDK_DISK_FORMAT
    , SDK_DISK_PROGRESS
    , SDK_DISK_UMOUNT					//卸载U盘 chann字段表示"盘号"
}SDK_DISK_OP_E;


/*
 * 用户定义(用户名＋密码)
 */
typedef struct sdk_user_s {
    uint8_t user_name[MAX_USER_NAME_LEN];
    uint8_t user_pwd[MAX_PASSWD_LEN];
}sdk_user_t;


/*
 * 用户权限定义
 */
typedef struct sdk_user_right_s {

    sdk_user_t user;   
    uint32_t local_right;           //本地GUI权限
    /*bit.0: 本地控制云台*/
	/*bit.1: 本地手动录象*/
	/*bit.2: 本地回放*/
	/*bit.3: 本地设置参数*/
	/*bit.4: 本地查看状态、日志*/
	/*bit.5: 本地高级操作(升级，格式化，重启，关机)*/
    /*bit.6: 本地查看参数 */
    /*bit.7: 本地管理模拟和IP camera */
    /*bit.8: 本地备份 */
    /*bit.9: 本地关机/重启 */

    uint32_t remote_right;          //远程权限
    /*bit.0: 远程控制云台*/
	/*bit.1: 远程手动录象*/
	/*bit.2: 远程回放 */
	/*bit.3: 远程设置参数*/
	/*bit.4: 远程查看状态、日志*/
	/*bit.5: 远程高级操作(升级，格式化，重启，关机)*/
	/*bit.6: 远程发起语音对讲*/
	/*bit.7: 远程预览*/
	/*bit.8: 远程请求报警上传、报警输出*/
	/*bit.9: 远程控制，本地输出*/
	/*bit.10: 远程控制串口*/	
    /*bit.11: 远程查看参数 */
    /*bit.12: 远程管理模拟和IP camera */
    /*bit.13: 远程关机/重启 */

    uint8_t local_backup_right[MAX_CHANN_NUM];     //通道权限
    uint8_t net_preview_right[MAX_CHANN_NUM];
    
}sdk_user_right_t;


typedef enum _SDK_NETWORK_INTERFACE {
      SDK_IF_ETH0 = 0x0  //eth0
    , SDK_IF_ETH1		 //eth1
    , SDK_IF_PPP0		 //ppp0
    , SDK_IF_WIFI		 //wifi
}SDK_NET_IF_E;

/*
 * 网络配置（子结构体）
 */
typedef struct sdk_ethernet_s {
    
    uint8_t ip_addr[MAX_IP_ADDR_LEN];//IP ADDR
    uint8_t mask[MAX_IP_ADDR_LEN];   //IP MASK
    uint8_t gateway[MAX_IP_ADDR_LEN];//网关
    uint8_t mac[MAX_MAC_ADDR_LEN];   //MAC STRING
    uint8_t enable_dhcp;            //是否开启DHCP
    uint8_t if_type;                //网络接口1-10MBase-T 2-10MBase-T全双工 3-100MBase-TX 4-100M全双工 5-10M/100M自适应
    uint8_t res[2];                 //保留

}sdk_ethernet_t;

/*
 * 网络配置（子结构体）
 */
typedef struct sdk_pppoe_s {
    uint8_t enable;                    		//是否开启PPPOE
    uint8_t if_no;                      	//网络接口
    uint8_t res[2];                     	//保留
    
    uint8_t user[MAX_USER_NAME_LEN];    	//PPPOE 用户名
    uint8_t pass[MAX_PASSWD_LEN];       	//PPPOE 密码
    uint8_t pppoe_ip[MAX_IP_ADDR_LEN];  	//PPPOE IP ADDR
}sdk_pppoe_t;


/*
 * 网络配置
 */
typedef struct sdk_net_cfg_s {
    sdk_ethernet_t ethernet[MAX_ETHERNET_NUM];    //网络接口SDK_NET_IF_E
    sdk_pppoe_t    pppoe;                         //PPPOE配置
    
    uint8_t        def_if_no;                     //默认网络接口
    uint8_t        dns_auto_en;                   //自动获取DNS，开启DHCP时
    uint8_t        byte_res[2];                   //保留
    
    uint8_t        dns1[MAX_IP_ADDR_LEN];         //DNS1
    uint8_t        dns2[MAX_IP_ADDR_LEN];         //DNS2
    uint8_t        multicast[MAX_IP_ADDR_LEN];    //组播地址
    uint8_t        net_manager[MAX_IP_ADDR_LEN];  //管理服务器IP

    uint16_t       net_manager_port;              //管理服务器PORT
    uint16_t       http_port;                     //DVR HTTP PORT
    uint16_t       dvr_cmd_port;                  //DVR SERVICE PROT
    uint16_t       dvr_data_port;                 //保留

    uint8_t         res[64];                      //保留
    
}sdk_net_cfg_t;



/*
 * DVR常规配置参数
 */

typedef struct sdk_comm_cfg_s
{
    uint8_t 	dvr_name[MAX_DVR_NAME_LEN]; //DVR 名字
    uint32_t 	dvr_id;                    	//DVR ID,(遥控器)
    uint32_t 	recycle_record;            	//是否循环录像,0:不是; 1:是
    uint8_t 	language;					//语言0: 中文  1 :英语
    uint8_t  	video_mode;					//视频制式0:PAL  1 NTSC
    uint8_t  	out_device;					//输出设备  0: CVBS; 1: VGA-HDMI
    uint8_t  	resolution; 				//分辨率0 :1024 * 768    1 : 1280 * 720   2: 1280*1024  3: 1920*1080
    uint8_t 	standby_time; 				//菜单待机时间 1~60分钟  0: 表示不待机
    uint8_t  	boot_guide;					//开机向导 0 : 不启动向导  1 : 启动向导
    uint8_t  	reserve[2];
} sdk_comm_cfg_t;


/* 
 *
 * DVR系统配置参数
 */

typedef struct sdk_sys_cfg_s {
   
    /*---------- 以下信息不可更改 ----------*/
    uint8_t serial_no[MAX_SERIAL_NO_LEN];       //序列号
    uint8_t device_type[MAX_VER_STR_LEN];       //设备型号
    uint8_t software_ver[MAX_VER_STR_LEN];      //软件版本号
    uint8_t software_date[MAX_VER_STR_LEN];     //软件生成日期
    uint8_t panel_ver[MAX_VER_STR_LEN];         //前面板版本
    uint8_t hardware_ver[MAX_VER_STR_LEN];      //硬件版本

    uint32_t dev_type;                          //设备类型 (DEV_TYPE_DVR,DEV_TYPE_NVR)    J_DevType_E from j_sdk.h
    
    uint8_t ana_chan_num;                       //模拟通道个数
    uint8_t ip_chan_num;                        //数字通道数
    uint8_t dec_chan_num;                       //解码路数
    uint8_t stream_num;                            //保留
    
    uint8_t audio_in_num;                       //语音输入口的个数
    uint8_t audio_out_num;                      //语音输出口的个数
    uint8_t alarm_in_num;                       //报警输入个数
    uint8_t alarm_out_num;                      //报警输出个数

    uint8_t net_port_num;                       //网络口个数
    uint8_t rs232_num;                          //232串口个数
    uint8_t rs485_num;                          //485串口个数
    uint8_t usb_num;                            //USB口的个数
    
    uint8_t hdmi_num;                           //HDMI口的个数
    uint8_t vga_num;                            //VGA口的个数
    uint8_t cvbs_num;                           //cvbs口的个数
    uint8_t aux_out_num;                        //辅口的个数
    
    uint8_t disk_ctrl_num;                      //硬盘控制器个数
    uint8_t disk_num;                           //硬盘个数
    uint8_t res2[2];                            //保留
    
    uint32_t max_encode_res;                    //最大编码分辨率
    uint32_t max_display_res;                   //最大显示分辨率
    
    uint32_t dvr_bit;                           //DVR保留
    uint8_t  dvr_byte[4];                       //DVR保留
}sdk_sys_cfg_t;

/*
 * 平台信息
 *
 */

typedef struct sdk_platform_s {
    uint8_t pu_id[MAX_USER_NAME_LEN];           
    uint8_t cms_ip[MAX_IP_ADDR_LEN];            //平台mds ip
    uint8_t mds_ip[MAX_IP_ADDR_LEN];            //平台mds ip
    uint32_t cms_port;                          //平台cms端口号
    uint32_t mds_port;                          //平台mds端口号
    uint32_t protocol;                          //0:tcp   1:udp
    uint32_t plat_enable;                       //平台启用
    
}sdk_platform_t;


/*
 * 
 * 系统版本信息
 */

typedef struct sdk_version_s {
    uint8_t serial_no[MAX_SERIAL_NO_LEN];       /* 产品序列号 */
    uint8_t device_type[MAX_VER_STR_LEN];       /* 产品型号 */
	uint8_t software_ver[MAX_VER_STR_LEN];	    /* 版本号 */
	uint8_t software_date[MAX_VER_STR_LEN];	    /* 版本日期 */
    uint8_t panel_ver[MAX_VER_STR_LEN];         //前面板版本
    uint8_t hardware_ver[MAX_VER_STR_LEN];      //硬件版本
}sdk_version_t;


/*
 * 恢复默认参数
 */
typedef struct sdk_default_param {
   
    uint32_t param_mask;    //参数内容掩码
   /* bit.0: 编码参数 */
   /* bit.1: 云台参数 */
   /* bit.2: 录像参数 */
   /* bit.3: 网络参数 */
   /* bit.4: 通道参数 */
   /* bit.5: 视频属性 */
   /* bit.6: 串口参数 */
   /* bit.7: 遮挡区域参数 */
   /* bit.8: 移动侦测 */
   /* bit.9: 常规参数 */
   /* bit.10: osd参数 */
   /* bit.11: 用户管理 */
   /* bit.12: 报警输入参数*/
   /* bit.13: 视频遮挡侦测参数*/
   /* bit.14: 视频丢失参数*/
   /* bit.15: 边距调节参数*/
   /* bit.16: 巡回参数*/

   uint32_t res[1];
}sdk_default_param_t;


/*
 * 系统运行状态信息
 */
typedef enum _SDK_STATUS_OP {
      SDK_STATUS_DEVICE = 0x01  //设备运行状态
    , SDK_STATUS_DISK           //磁盘状态
    , SDK_STATUS_CHANN          //通道状态
    , SDK_STATUS_ALARM_IN       //输入报警状态
    , SDK_STATUS_ALARM_OUT      //输出报警状态
}SDK_STATUS_OP_E;

typedef struct sdk_chan_status_s {
    
    uint8_t chan_type;          //通道类型 0：本地，1：IP
    uint8_t record_state;       //录像状态,0-不录像,1-录像
    uint8_t signal_state;       //信号状态, 0:正常，1：信号丢失
    uint8_t encode_state;       //编码状态, 0:不编码，1：编码
    
    uint32_t bit_rate;          //实际码率
    uint32_t conn_num;          //通道连接数
    uint8_t  conn_ip[MAX_CHANN_CONN_NUM][MAX_IP_ADDR_LEN];//连接IP
}sdk_chan_status_t;

 
typedef struct sdk_disk_status_s {
    uint32_t volume;        //硬盘的容量
    uint32_t free_space;    //硬盘的剩余空间
    uint32_t disk_state;   //硬盘的状态,0-活动,1-休眠,2-不正常
}sdk_disk_status_t;

/*
 * 系统状态(后续拆分成 SDK_STATUS_OP_E)
 */
 
typedef struct sdk_status_s { 
    
	uint32_t            device_status;                      //设备状态，0-正常,1-CPU占用率太高,超过85%,2-硬件错误
	sdk_disk_status_t   disk_status[MAX_DISK_NUM];          //硬盘状态
	sdk_chan_status_t   chan_status[MAX_CHANN_NUM];         //通道状态
	uint8_t             alarm_in_status[MAX_ALARM_IN_NUM];  //报警端口的状态,0-没有报警,1-有报警
	uint8_t             alarm_out_status[MAX_ALARM_OUT_NUM];//报警输出端口的状态,0-没有输出,1-有报警输出
	
	uint8_t             local_display;                      //本地显示状态, 0:正常，1：不正常
	uint8_t             audio_chan_status;                  //语音通道的状态 0-未使用，1-使用中, 0xff无效
	uint8_t             res[2];
	
}sdk_status_t;




/*
 * 日志操作类型
 */
typedef enum _SDK_LOG_OP {
      SDK_LOG_QUERY = 0x01  // 查询 sdk_log_item_t, sdk_log_cond_t
    , SDK_LOG_EMPTY         // 清空(全部)
    , SDK_LOG_BACKUP        // 备份
    , SDK_LOG_DEL			// 删除(按时间段)
}SDK_LOG_OP_E;

/*
 * 日志类型定义（主类型，最大支持32种）
 */
typedef enum _LOG_MAJOR_TYPE {
      LOG_MAJOR_SYSTEM 		= 0x00000001  //系统日志
    , LOG_MAJOR_ALARM  		= 0x00000002  //告警日志
    , LOG_MAJOR_OPERATE		= 0x00000004  //操作日志
    , LOG_MAJOR_NETWORK		= 0x00000008  //网络日志
    , LOG_MAJOR_PARAM  		= 0x00000010  //参数日志
    , LOG_MAJOR_EXCEPTION	= 0x00000020  //异常日志
    
    , LOG_MAJOR_ALL    		= 0xffffffff  //所有日志
}LOG_MAJOR_TYPE_E;

/*
 * 系统日志子类型
 */
typedef enum _L_SYSTEM_MINOR_TYPE {
      L_SYSTEM_MINOR_STARTUP = 0x0001   //开机
    , L_SYSTEM_MINOR_SHUTDOWN           //关机
    , L_SYSTEM_MINOR_REBOOT				//重启
    
    , L_SYSTEM_MINOR_ALL     = 0xffff   //所有系统日志
}L_SYSTEM_MINOR_TYPE_E;

/*
 * 告警日志子类型
 */
typedef enum _L_ALARM_MINOR_TYPE {
      L_ALARM_MINOR_DI_START = 0x0001   //输入告警发生
    , L_ALARM_MINOR_DI_STOP             //输入告警停止
    , L_ALARM_MINOR_MD_START            //移动侦测告警发生
    , L_ALARM_MINOR_MD_STOP             //移动侦测告警停止
    , L_ALARM_MINOR_VL_START			//视频丢失告警发生
    , L_ALARM_MINOR_VL_STOP				//视频丢失告警停止
    
    , L_ALARM_MINOR_ALL		= 0xffff	//所有告警日志
}L_ALARM_MINOR_TYPE_E;

/*
 * 操作日志子类型
 */
 typedef enum _L_OPERATE_MINOR_TYPE {
      L_OPERATE_MINOR_LOGIN = 0x0001	// 登陆	
    , L_OPERATE_MINOR_LOGOUT 			// 注销
    , L_OPERATE_MINOR_USER_ADD			// 用户管理-增加
	, L_OPERATE_MINOR_USER_DEL			// 用户管理-删除
	, L_OPERATE_MINOR_USER_MODI			// 用户管理-修改
    , L_OPERATE_MINOR_SETTIME 			// 设置系统时间
    , L_OPERATE_MINOR_FORMAT_DISK 		// 格式化硬盘
    , L_OPERATE_MINOR_DEFAULT 			// 恢复默认
    , L_OPERATE_MINOR_UPGRADE 			// 升级
    , L_OPERATE_MINOR_PLAYBACK 			// 回放
    , L_OPERATE_MINOR_PTZ 				// 云台控制
    , L_OPERATE_MINOR_BACKUP 			// 备份
    , L_OPERATE_MINOR_RECORD_START 		// 启动录像
    , L_OPERATE_MINOR_RECORD_STOP 		// 停止录像
    , L_OPERATE_MINOR_CLR_ALARM 		// 清除报警
    , L_OPERATE_MINOR_TALKBACK_START 	// 对讲开始
    , L_OPERATE_MINOR_TALKBACK_STOP 	// 对讲结束
	, L_OPERATE_MINOR_LOG_DEL			// 删除日志
	, L_OPERATE_MINOR_LOG_EMPTY			// 清空日志
	, L_OPERATE_MINOR_LOG_BACKUP		// 备份日志
    , L_OPERATE_MINOR_MANUAL_RECORD_START 		// 手动启动录像
    , L_OPERATE_MINOR_MANUAL_RECORD_STOP 		// 手动停止录像	
    , L_OPERATE_MINOR_FORMAT_DISK_U 	// 格式化U盘

    , L_OPERATE_MINOR_ALL	= 0xffff	// 所有操作日志
}L_OPERATE_TYPE_E;

/*
 * 参数日志子类型
 */
typedef enum _L_PARAM_MINOR_TYPE {
      L_PARAM_MINOR_PARAM_NETWORK = 0x0001 	// 网络参数配置
    , L_PARAM_MINOR_PARAM_UART 				// 串口参数配置
    , L_PARAM_MINOR_PARAM_PTZ 				// 云台参数配置
    , L_PARAM_MINOR_PARAM_CHAN 				// 通道参数配置
    , L_PARAM_MINOR_PARAM_VENC 				// 编码参数配置
    , L_PARAM_MINOR_PARAM_TV 				// TV参数配置
	, L_PARAM_MINOR_PARAM_PIC_ATTR			// 图像属性配置
	, L_PARAM_MINOR_PARAM_RECORD			// 录像参数配置
	, L_PARAM_MINOR_PARAM_MOTION			// 移动侦测配置
	, L_PARAM_MINOR_PARAM_VIDEOLOST			// 视频丢失配置
	, L_PARAM_MINOR_PARAM_ALARMIN			// 输入告警配置
	, L_PARAM_MINOR_PARAM_POLL				// 轮巡配置
	, L_PARAM_MINOR_PARAM_PREVIEW			// 预览配置
	, L_PARAM_MINOR_PARAM_OVERLAY			// 遮挡区域配置
	, L_PARAM_MINOR_PARAM_OSD				// OSD配置
	, L_PARAM_MINOR_PARAM_COMM				// 常规配置

	, L_PRARM_MINOR_ALL	= 0xffff			// 所有参数日志
}L_PARAM_MINOR_TYPE_E;

/*
 * 异常日志子类型
 */
typedef enum _L_EXCEPTION_MINOR_TYPE {
      L_EXCEPTION_HD_FULL 		= 0x0001   	//硬盘满
    , L_EXCEPTION_HD_ERROR           		//硬盘错误
    , L_EXCEPTION_NET_DISCONNECT			//网络断开
    , L_EXCEPTION_IP_CONFLICT				//IP冲突
    
    , L_EXCEPTION_MINOR_ALL     = 0xffff   	//所有异常日志
}L_EXCEPTION_MINOR_TYPE_E;

/*
 * 日志记录定义
 */
typedef struct sdk_log_item_s {
    sdk_time_t  time;                       //时间
    uint32_t    major_type;                 //类型LOG_MAJOR_TYPE_E 
    uint32_t    minor_type;                 //0x0000:直接显示desc内容，否则通过L_XXX_MINOR_TYPE_E解析内容.
    uint32_t    args;                       /*子类型参数(args=0时不用解析,否则根据minor_type解析内容。
    										如果minor_type和通道号相关，args表示通道号;如果minor_type和告警相关，args表示告警输入号)*/
    uint8_t     user[MAX_USER_NAME_LEN];    //操作用户
    uint8_t     ip_addr[MAX_IP_ADDR_LEN];   //用户IP
    uint8_t     desc[MAX_LOG_DESC_LEN];     //日志内容(minor_type=0时有效)
}sdk_log_item_t;


/*
 * 日志查询条件
 */
typedef struct sdk_log_cond_s {
    sdk_time_t  begin_time;                 //开始时间
    sdk_time_t  end_time;                   //结束时间
    uint32_t    type;                       //类型
    uint8_t     user[MAX_USER_NAME_LEN];    //操作用户
    uint8_t     ip_addr[MAX_IP_ADDR_LEN];   //用户IP
}sdk_log_cond_t;



/*
 * 报警信息定义
 */
typedef struct sdk_alarm_info_s {
    sdk_time_t  time;                   //报警时间
    uint32_t    type;                   //报警类型
    uint32_t    args;                   //报警参数
    uint8_t     desc[MAX_ALARM_DESC_LEN];//报警内容
}sdk_alarm_info_t;


/*
 * 串口参数配置
 */
typedef struct sdk_serial_param_s {
    uint32_t baud_rate;             //串口波特率 115200, 57600, ...
    uint8_t  data_bit;              //数据位 5, 6, 7, 8 
    uint8_t  stop_bit;              //停止位 1, 2
    uint8_t  parity;                //校验 0:无，1:奇校验 2:偶校验 
    uint8_t  flow_ctl;              //流控
}sdk_serial_param_t;

/*
 * 串口用途配置
 */
typedef struct sdk_serial_func_cfg_s {
	uint8_t type;	// 0: normal, 1: ptz, 2...
	uint8_t res[3];
	sdk_serial_param_t serial_param;
}sdk_serial_func_cfg_t;


/*
 * 云台配置参数
 */
 /*
 *  _0: debug, _1: ptz/normal, _2: rs232, _3 mcu
 *  -------------------------, _2: mcu;
 *  0: 1
 *   
 */
typedef struct sdk_ptz_param_s {

    sdk_serial_param_t comm;                //串口参数，gui固定只配置串口1
    uint8_t          address;               //云台地址
    uint8_t          protocol;              //云台协议SDK_PTZ_PROTOCOL_E
    uint8_t          res[2];
    uint8_t          preset_set[MAX_PRESET_NUM];//设置预置位标志
    uint8_t          cruise_set[MAX_CRUISE_NUM];//设置巡航标志
}sdk_ptz_param_t;


/*
 *DVR实现巡航数据结构
 */
typedef struct sdk_cruise_param_s {
    uint8_t enable;                           //是否启动
    uint8_t res[3];                           //保留
    uint8_t preset[MAX_CRUISE_PRESET_NUM];    //预置点
    uint8_t speed[MAX_CRUISE_PRESET_NUM];     //移动速度
    uint8_t dwell_time[MAX_CRUISE_PRESET_NUM];//停留时间
}sdk_cruise_param_t;

//云台区域选择放大缩小(私有 快球专用)


/*
 * 
 * 预览配置(后继讨论修改)
 */
 

typedef enum _SDK_SCREE_SPLIT {
      SDK_SCREE_ONE  = 0x00
    , SDK_SCREE_FOUR
    , SDK_SCREE_NINE
    , SDK_SCREE_SIXTEEN
    , SDK_SCREE_THIRTY_TWO
    , SDK_SCREE_BUTT
}SDK_SCREE_SPLIT_E;


typedef struct sdk_preview_s {
    
    uint8_t preview_mode;   //预览模式,0-1画面,1-4画面,2-6画面,3-8画面,4-9画面,0xFF-最大画面
    uint8_t enable_audio;   //是否声音预览,0-不预览,1-预览
    uint16_t switch_time;   //切换时间(秒)
    uint8_t windows[MAX_PREVIEW_MODE][MAX_WINDOWS_NUM];//切换顺序,step[i]为 0xff表示不用

}sdk_preview_t;

/*
 * VO绑定定义
 */
typedef struct sdk_vo_binding_s {
    
    uint8_t mode;        //分屏模式,0-1画面,1-4画面,2-6画面,3-8画面,4-9画面,5-16画面,0xff:最大画面
    uint8_t win_num;     //窗口个数
    uint8_t res[2];      //保留
    uint8_t windows[MAX_WINDOWS_NUM];//窗口内容;

}sdk_vo_binding_t;


/*
 * 用于解码器屏幕控制
 */
typedef struct sdk_win_s {
    uint8_t valid;          //valid: 0表示无视频源
    uint8_t res[3];         //保留
    uint8_t url[256];       //视频源URL
}sdk_win_t;

typedef struct sdk_screen_s {
    uint8_t valid;                 //valid: 0表示显示屏被禁止
    uint8_t mode;                  //分屏模式,0:1画面,1:4画面,2:6画面,3:8画面,4:9画面,5:16画面,0xff:最大画面
    uint8_t res[2];                //保留
    sdk_win_t win[MAX_WINDOWS_NUM];//sdk_win_t
}sdk_screen_t;

/*
 *音频配置
 */
typedef struct sdk_audio_cfg_s {
   
    uint8_t encode_type;        //编码类型 1: G711A
    uint8_t samples;            //采样频率 0: 8000, 
    uint8_t bits;               //位宽     1: 16bit, 
    uint8_t channels;           //通道数
    
    uint8_t  bufsize;            //缓存大小,以帧为单位
    uint8_t  resample;           //0 无采样 2 2->1 4 4->1 6 6->1
    uint16_t sample_per_frm;     //每一帧采样点个数
}sdk_audio_cfg_t;


/*
 * GUI查询设备列表时使用
 */

typedef enum _MANUFACT_TYPE{
     MANUFACT_JXJ     = 0x00
    ,MANUFACT_SAE     = 0x01
    ,MANUFACT_ONVIF   = 0x02
}MANUFACT_TYPE_E;

typedef struct sdk_device_s {
    uint8_t   ip_addr[MAX_IP_ADDR_LEN];     //远程设备IP地址
    uint16_t  port;                         //远程设备port
    uint8_t   dns_en;                       //是否使用域名访问远程设备 1:使用，0:不使用
    uint8_t   device_type;                  //远程设备类型  0:IPC 1: DVR 2:NVR 3:DVS
    uint8_t   chann_num;                    //远程设备总通道数
    uint8_t   chann_stream_num;             //远程设备通道码流个数 1: 单码流 2: 双码流 3:三码流
    uint8_t   manufacturer;                 //设备厂家类型 MANUFACT_TYPE_E
    uint8_t   dev_res[1];                   //保留
    uint8_t   dns_path[64];                 //远程设备域名地址
    uint8_t   dst_id[32];                   //远程设备唯一标示符 字符串
    sdk_user_t user;                        //远程设备用户名+密码 
}sdk_device_t;

/*
 *
 * 通道配置（支持IP接入）
 */

typedef struct sdk_channel_s {
    uint8_t   enable;                        //是否启用
    uint8_t   is_local;                      //是否为本地视频源		1:本地; 0:远程
    //local vi info
    uint8_t   video_in;                      //本地视频源序号
    uint8_t   video_format;                  //本地视频源信号格式    0：PAL， 1：NTSC    
    //channel name
    uint8_t   name[MAX_CHANN_NAME_LEN];      //通道名称
    //net channel info
    sdk_user_t user;                        //远程设备用户名+密码 
    uint8_t  chann_no;                      //接入远程设备通道号
    uint8_t  chann_protocol;                //接入通道码流协议
    uint8_t  chann_audio_en:1;              //接入通道是否支持音频 1: 支持   0: 不支持
    uint8_t  chann_win_mode:2;              //预览码流模式 0: 自动, 1: 主码流, 2: 子码流
    uint8_t  chann_bit_res:5;               //保留
    uint8_t  chann_byte_res;                //保留
    
    //net device info
    sdk_device_t device;                    //远程设备信息
}sdk_channel_t;


/*
 * 通道编码参数(子结构体)
 */
typedef struct sdk_av_enc_s {
    uint8_t resolution;     //SDK_VIDEO_RESOLUTION_E
    uint8_t bitrate_type;	//码率类型 0--变码流;	1--定码流
    uint8_t pic_quilty;     //编码质量0-最好，1-次好，2-较好，3-一般，4-较差，5-差	
    uint8_t frame_rate;	    //编码帧率1-25(PAL)/30
    uint8_t gop;			//I 帧间隔1-200
    uint8_t video_enc;      //视频编码格式0--h264 1--MJPEG 2--JPEG	
    uint8_t audio_enc;      //音视编码格式0:G711A
    uint8_t mix_type;       //音视频流: 0, 视频流: 1.
    uint16_t bitrate;	    //编码码率 32kbps-16000kbps
    uint8_t level;          //编码等级，h264: 0--baseline, 1--main, 2--high;
    uint8_t h264_ref_mod; 
}sdk_av_enc_t;


/*
 * 通道编码参数
 */

typedef struct sdk_encode_s {
    sdk_av_enc_t main;          //主码流
    sdk_av_enc_t second;        //子码流
    sdk_av_enc_t res;           //保留
}sdk_encode_t;


/*
 *
 *视频覆盖区域设置
 */
 
typedef struct sdk_overlay_cfg_s {
    uint8_t   enable;                  //是否开启
    uint8_t   max_num;
    uint8_t   res[2];
    uint32_t   mask;                    //bit.0: area[0], bit.1: area[1]
    sdk_rect_t area[MAX_OVERLAY_NUM];   //区域坐标
}sdk_overlay_cfg_t;
typedef struct _sdk_codec_area_zoom_ch_info_s
{
    int is_enable;
    sdk_rect_t area;
}sdk_codec_ch_area_zoom_info_t;


/*
 *
 *TV设置
 */
typedef struct sdk_vo_sideSize_s
{
    int top_offset;
    int bottom_offset;
    int left_offset;
    int right_offset;
}sdk_vo_sideSize_t;

/*
 *
 *视频OSD区域(子结构体)
 */

typedef struct sdk_osd_info_s {

    sdk_point_t pos;            //位置
    uint8_t     valid;          // 当前区域是否有效
    uint8_t     font;           // 字体类型
    // [7 6 5 4 - 3 2 1 0]
    // bit[4-7]: display type, 0: time, 1: string, 2 .....
    // bit[0-3]: display format 
    //           sample time format:
    //           bit[2-3], 0: YYYYMMDD, 1: MMDDYYYY, 2: DDMMYYYY
    //           bit[1],   0: '-'       1: '/'
    //           bit[0],   0: 24hour    1: ampm
    uint8_t     format;         // 显示格式
    uint8_t     str_len;        // 字符串长度
    uint8_t     str[MAX_OSD_STR_LEN];//ascii+gb2312字符串
}sdk_osd_info_t;



/*
 *
 *视频OSD配置
 */
typedef struct sdk_osd_cfg_s {
    sdk_osd_info_t  time;        //OSD时间
    sdk_osd_info_t  chann_name;  //OSD通道名称
//    sdk_osd_info_t  vloss;      //保留
#if 0
    sdk_osd_info_t  res[1];      //保留
#else
    uint16_t max_width;
    uint16_t max_height;
    
    uint8_t  st_info_en;
    uint8_t  res1[3];
    
    uint8_t  res2[64];
#endif
}sdk_osd_cfg_t;



/*
 *
 *图像属性配置
 */
typedef struct sdk_image_attr_s {
    uint8_t brightness;         //亮度
    uint8_t contrast;           //对比度
    uint8_t saturation;         //饱和度
    uint8_t hue;                //色度
    uint8_t sharpness;          //锐度
    uint8_t res[3];
}sdk_image_attr_t;


/*
 * 抓拍结构体定义
 */
typedef struct sdk_snap_pic_s {
    uint16_t width;             //宽度
    uint16_t height;            //高度
    uint8_t  encode;            //编码格式,JPEG
    uint8_t  quilty;            //图像质量
    uint8_t  res[2];            //保留
}sdk_snap_pic_t;

typedef enum _SDK_DISK_STATUS {
	  SDK_DISK_STATUS_DISK_UNMOUNT = 0x00	// 未挂载
	, SDK_DISK_STATUS_DISK_NOMAL			// 正常
	, SDK_DISK_STATUS_DISK_USING			// 正在使用
	, SDK_DISK_STATUS_DISK_BUTT
}SDK_DISK_STATUS_E;

/*
 * 磁盘信息结构体
 */
typedef struct sdk_disk_s {
    uint32_t disk_no;       //磁盘号
    uint32_t disk_type;     //磁盘类型（0:SATA，1:USB，2:ISCSI; 3:NFS）
    uint32_t status;        //磁盘状态 SDK_DISK_STATUS_E
    uint32_t total_size;    //磁盘容量(MB)
    uint32_t free_size;     //可用容量(MB)
    uint32_t is_backup;     //是否备份磁盘 0:录像盘; 1:备份盘
    uint32_t is_raid;       //是否raid //属于哪个盘组
}sdk_disk_t;

//本地盘组信息配置

//网络硬盘结构配置


/*
 * 录像参数（子结构体）
 */
typedef struct sdk_record_sched_s {
    uint8_t is_allday;                              //是否全天录像
    uint8_t record_type;                            //录像类型
    uint8_t res[2];                                 //保留
    sdk_sched_time_t sched_time[MAX_TIME_SEG_NUM];  //布防时间段
    
}sdk_record_sched_t;

/*
 * 录像参数
 */
typedef struct sdk_record_cfg_s {
    uint8_t enable;                                //开启定时录像
    uint8_t res[3];                                 //保留
    sdk_record_sched_t record_sched[MAX_WEEK_NUM];  //布防时间段(0:星期日; 1:星期一,2:星期二,... ,6:星期六)
    
    uint32_t pre_record_time;                       //预录时间
    uint32_t record_duration_time;                  //录像保留时间
    
    uint8_t  enable_redundancy;                     //是否冗余备份
    uint8_t  enable_audio;                          //是否录制音频
    uint8_t  res2[2];                                //保留
    
}sdk_record_cfg_t;


/*
 * 录像记录定义
 */
typedef struct sdk_record_item_s {
    uint8_t item_name[MAX_RECORD_ITEM_LEN]; //记录名称
    uint32_t item_handle[8];                //sizeof(stor_fragment_t)
    sdk_time_t start_time;                  //开始时间
    sdk_time_t stop_time;                   //结束时间
    uint32_t item_size;                     //数据大小
    
    uint8_t is_locked;                      //是否锁定
    uint8_t record_type;                    //录像类型
    uint8_t res[2];                         //保留
    // card no;
}sdk_record_item_t;

/*
 * 录像查询条件
 */
typedef struct sdk_record_cond_s {
    uint8_t is_locked;              //锁定状态
    uint8_t record_type;            //录像类型
    uint8_t res[2];                 //保留
    sdk_time_t start_time;          //开始时间
    sdk_time_t stop_time;           //结束时间
    // card no;    
}sdk_record_cond_t;


/*
 *
 * 回放控制命令字
 */
typedef enum _SDK_PB_GROUP_CONTROL {
      SDK_PB_CONTROL_ONNE  = 0x00 	//
    , SDK_PB_CONTROL_PAUSE  		// 暂停
    , SDK_PB_CONTROL_SETP          	// 单帧进
    , SDK_PB_CONTROL_NORMAL        	// 正常
    , SDK_PB_CONTROL_DRAG          	// 拖拽
    , SDK_PB_CONTROL_FORWARD       	// 前进
    , SDK_PB_CONTROL_BACKWARD      	// 后退
    , SDK_PB_CONTROL_QUERY_TIME		// 查询当前所播放帧的时间，单位秒
    
    , SDK_PB_CONTROL_BUTT
}SDK_PB_GROUP_CONTROL_E;

/*
 *
 * 前进播放速度枚举
 */
typedef enum _SDK_PB_GROUP_CONTROL_FORWARD {
      SDK_PB_CONTROL_FW_NORMAL = 0x00  				// 正常
    , SDK_PB_CONTROL_FF_1X     						// 1倍快进 (FF-fast forward-向前快进)
    , SDK_PB_CONTROL_FF_2X    						// 2倍快进
    , SDK_PB_CONTROL_FF_4X         					// 4倍快进
    , SDK_PB_CONTROL_FF_8X							// 8倍快进
    , SDK_PB_CONTROL_FF_16X							// 16倍快进

    , SDK_PB_CONTROL_SF_1_2X						// 1倍慢放 (SF-slow forward-向前慢放)
    , SDK_PB_CONTROL_SF_1_4X						// 2倍慢放
    , SDK_PB_CONTROL_SF_1_8X						// 4倍慢放
    , SDK_PB_CONTROL_SF_1_16X						// 8倍慢放

}SDK_PB_GROUP_CONTROL_FORWARD_E;


/*
 * 回放通道组定义
 */
typedef struct sdk_pb_group_s {
    sdk_time_t start_time;             //开始时间
    sdk_time_t stop_time;              //结束时间
    uint32_t main_chann;               //主通道号
    uint8_t chann_mask[MAX_CHANN_NUM];//通道掩码（多路回放时）
}sdk_pb_group_t;



/* ---------------------------------------*/
// ntp
// ddns
// wifi
// email
// ftp
/* ---------------------------------------*/





/*
 *
 *设备信息（设备发现时用，如：搜索工具）
 */

typedef struct sdk_device_info_s {
    /*------ 设备配置信息 ------*/
    sdk_net_cfg_t   net_cfg;        //网络配置
    sdk_sys_cfg_t   sys_cfg;        //系统配置
    /*---- 系统运行时信息  ----*/
    //需要不断完善
}sdk_device_info_t;

/*----------------- 报警配置结构体定义 -----------------*/

typedef enum _SDK_PTZ_OP_TYPE {
	  SDK_PTZ_TYPE_NONE = 0x00
	, SDK_PTZ_TYPE_PRESET		// 预置位
	, SDK_PTZ_TYPE_CRUISE		// 巡航
	, SDK_PTZ_TYPE_BUTT
}SDK_PTZ_OP_TYPE_E;

/*
 *  报警联动定义
 */
typedef struct sdk_alarm_handle_s {

    uint8_t record_mask[MAX_CHANN_NUM];   //录像通道掩码
    uint8_t snap_mask[MAX_CHANN_NUM];     //抓拍通道掩码
    uint8_t alarm_out_mask[MAX_CHANN_NUM];//联动输出掩码      
    uint8_t ptz_type[MAX_CHANN_NUM];      //联动云台操作类型(预置位/巡航)	SDK_PTZ_OP_TYPE_E
    uint8_t ptz_param[MAX_CHANN_NUM];     //联动云台参数(预置位号，巡航号)
    uint8_t res_mask[MAX_CHANN_NUM];

    uint8_t  record_enable;
    uint8_t  record_time;                 //录像延时时长
    uint8_t  snap_enable;
    uint8_t  snap_interval;               //抓拍时间间隔(无效)
    
    uint8_t  snap_num;                    //连续抓拍张数
    uint8_t  beep_enable;                 //是否蜂鸣器
    uint8_t  beep_time;                   //蜂鸣时间
    uint8_t  ptz_enable;
    
    uint8_t  alarm_out_enable;
    uint8_t  alarm_out_time;              //联动输出时间
    uint8_t  res[2+4];                    //email, ftp, 3g;
}sdk_alarm_handle_t;

/*
 * 报警输入（IO）报警配置
 */
typedef struct sdk_alarm_in_cfg_s {
    uint8_t name[MAX_ALARM_IN_NAME_LEN];                        //报警输入名称
    uint8_t type;                                               //报警输入类型，1：常开(默认)，0：常闭
    uint8_t enable;                                             //允许联动
    uint8_t res[2];                                             //保留
    sdk_sched_time_t sched_time[MAX_WEEK_NUM][MAX_TIME_SEG_NUM];//布防时间段
    sdk_alarm_handle_t alarm_handle;                            //联动处理
}sdk_alarm_in_cfg_t;

/*
 * 视频移动侦测配置
 */
typedef enum _SDK_VDA_MOD_E {
      SDK_VDA_MD_MOD = 0
    , SDK_VDA_OD_MOD = 1
}SDK_VDA_MOD_E;
typedef struct sdk_vda_codec_cfg_s
{
    uint8_t enable;                                             //允许联动
    uint8_t sensitive;                                          //灵敏度(0[灵敏度最高]----6[最低])
    uint8_t mode;                                             	/* 0:MD 移动 1:OD 遮挡 */
    uint8_t res;
    uint32_t mask;												//按位
    sdk_rect_t area[MAX_MOTION_NUM];                            //区域
}sdk_vda_codec_cfg_t;

typedef struct sdk_motion_cfg_v2_s {
    sdk_vda_codec_cfg_t codec_vda_cfg;						    			
    sdk_sched_time_t sched_time[MAX_WEEK_NUM][MAX_TIME_SEG_NUM];//布防时间段
    sdk_alarm_handle_t alarm_handle;                            //联动处理
}sdk_motion_cfg_v2_t;

typedef struct sdk_motion_cfg_s {
    //uint8_t scope[18][22];                                    //需要确认
    uint8_t enable;                                             //允许联动
    uint8_t sensitive;                                          //灵敏度(0[灵敏度最高]----6[最低])
    uint8_t res[2];                                             //保留
    uint32_t mask;							// 按位
    sdk_rect_t area[MAX_MOTION_NUM];                            //区域
	sdk_sched_time_t sched_time[MAX_WEEK_NUM][MAX_TIME_SEG_NUM];//布防时间段
    sdk_alarm_handle_t alarm_handle;                            //联动处理
}sdk_motion_cfg_t;


/*
 * 视频遮挡侦测配置
 */
typedef struct sdk_hide_cfg_s {
    uint8_t enable;                                             //允许联动
	uint8_t sensitive;                                          //灵敏度(0[灵敏度最高]----6[最低])
	uint8_t res[2];                                             //保留
    uint32_t mask;												 // 按位
    sdk_rect_t area[MAX_MOTION_NUM];                            //区域
    sdk_sched_time_t sched_time[MAX_WEEK_NUM][MAX_TIME_SEG_NUM];//布防时间段
    sdk_alarm_handle_t alarm_handle;                            //联动处理
}sdk_hide_cfg_t;

/*
 * 视频丢失侦测配置
 */
typedef struct sdk_lost_cfg_s {
    uint8_t enable;                                             //允许联动
    uint8_t res[3];                                             //保留
    sdk_sched_time_t sched_time[MAX_WEEK_NUM][MAX_TIME_SEG_NUM];//布防时间段
    sdk_alarm_handle_t alarm_handle;                            //联动处理
}sdk_lost_cfg_t;


/*
 * 录像控制
 */
typedef struct sdk_manual_record_s {
    uint8_t manual_record[MAX_CHANN_NUM];	// 手动录像
    uint8_t stop_record[MAX_CHANN_NUM];		// 禁止录像
    uint8_t res[MAX_CHANN_NUM];				// 保留位
}sdk_manual_record_t;

/*
 * 手动开启(停止)报警输入检测
 */
typedef struct sdk_manual_alarmin_s {
    uint8_t enable_alarmin[MAX_ALARM_IN_NUM];	// 0:停止; 1:开启(默认全开启)
    uint8_t res[MAX_ALARM_IN_NUM];				// 保留位
}sdk_manual_alarmin_t;

/*
 * 手动开启(停止)报警输出
 */
typedef struct sdk_manual_alarmout_s {
    uint8_t enable_alarmout[MAX_ALARM_OUT_NUM];	// 0:停止(默认全停止); 1:开启
    uint8_t res[MAX_ALARM_OUT_NUM];				// 保留位
}sdk_manual_alarmout_t;


/*
 * 录像备份
 */
typedef enum _SDK_BACKUP_OP {
      SDK_BACKUP_START		= 0x001		//sdk_record_backup_t
    , SDK_BACKUP_PROGRESS				//pmsg->chan:备份进度
    , SDK_BACKUP_BUTT
}SDK_BACKUP_OP_E;

typedef struct sdk_record_handle_s {
	uint32_t  	item_handle[8];			//sizeof(stor_fragment_t)
	uint32_t 	item_size;				//段大小
	sdk_time_t 	start_time;          	//开始时间
    sdk_time_t 	end_time;           	//结束时间
    uint8_t  	res[4];					//保留
}sdk_record_handle_t;

typedef struct sdk_record_backup_s {
	sdk_record_handle_t item_arr[MAX_BACKUP_ITEM];
	uint8_t item_num;				//实际备份段数
	uint8_t record_type;            //录像类型
	uint8_t res[2];					//保留
}sdk_record_backup_t;


/*
 * 关闭系统
 */
typedef enum _SDK_CLOSE_SYS_OP {
      SDK_CLOSE_SYS_SHUTDOWN	= 0x001		//关机
    , SDK_CLOSE_SYS_REBOOT					//重启
    , SDK_COLSE_SYS_LOGOUT
    , SDK_CLOSE_SYS_BUTT
}SDK_CLOSE_SYS_OP_E;

/*
 * 音频控制
 */
typedef enum _SDK_AUDIO_CONTROL_OP {
      SDK_AUDIO_CONTROL_OPEN	= 0x001		//伴音
    , SDK_AUDIO_CONTROL_MUTE				//静音
    , SDK_AUDIO_CONTROL_TALK				//对讲
    , SDK_AUDIO_CONTROL_SPECIAL_CMD			//特殊命令(GUI通知主控进入回放界面，pmsg->extend:1进入; 0:退出)
    
    , SDK_AUDIO_CONTROL_BUTT
}SDK_AUDIO_CONTROL_OP_E;


/*
 * 域名解析
 */

#define 	MAX_DDNS_USER_LEN   	64				// DDNS用户名最大长度
#define 	MAX_DDNS_PWD_LEN    	32				// DDNS密码最大长度
#define 	MAX_DDNS_NAME_LEN   	128				// DDNS域名最大长度

typedef enum _SDK_DDNS_TYPE {
	  SDK_DDNS_TYPE_DYNDNS	= 0x00		// Dyndns
	, SDK_DDNS_TYPE_3322				// 3322
	, SDK_DDNS_TYPE_ORAY				// Oray
	
	, SDK_DDNS_TYPE_BUTT
}SDK_DDNS_TYPE_E;

typedef enum _SDK_DDNS_ORAY_USERTYPE {
	  SDK_ORAY_USERTYPE_COMMON	= 0x01	// 普通用户
	, SDK_ORAY_USERTYPE_EXPERT			// 专业用户

	, SDK_ORAY_USERTYPE_BUTT
}SDK_DDNS_ORAY_USERTYPE;

typedef struct sdk_ddns_cfg_s {
	uint8_t enable;						// 是否启用DDNS 0-否，1-是
	uint8_t type;						// DDNS类型，SDK_DDNS_TYPE_E
	uint8_t user_type;					// 用户类型，SDK_DDNS_ORAY_USERTYPE (仅花生壳有此选项)
	uint8_t domain_num;					// 解析出来的域名的个数
	uint8_t reserve[4];					// 保留
	
	uint32_t interval;					// 更新周期(单位:秒)
	uint32_t port;						// DDNS端口
	uint8_t server[MAX_DDNS_NAME_LEN];	// DDNS协议对应的服务器地址(IP和域名均可)
	uint8_t user[MAX_DDNS_USER_LEN];	// 用户名
	uint8_t pwd[MAX_DDNS_PWD_LEN];		// 密码
	uint8_t domain[MAX_DDNS_NAME_LEN];	// 用户申请的域名地址(目前仅dyndns用此参数,多个domain使用','隔开)
	uint8_t client_ip[MAX_IP_ADDR_LEN];	// 设备端IP
	uint8_t dn[16][MAX_DDNS_NAME_LEN]; 	// 服务器返回的域名
}sdk_ddns_cfg_t;






/* ---------------------------------*/

#ifdef __cplusplus
}
#endif

#endif //__sdk_struct_h__

