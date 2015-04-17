/*
 *          file: nmp_proxy_info.h
 *          description:声明代理服务器的一些基本宏和数据类型
 *
 *          hegui,heguijiss@gmail.com
 *          May 16th, 2013
 */

#ifndef __JPROXY_INFO_H__
#define __JPROXY_INFO_H__

#define MAX_USR_LEN                     32      //用户名长度
#define MAX_PSW_LEN                     32      //密码长度

#define MAX_HOST_LEN                    64
#define MAX_PUID_LEN                    32
#define MAX_IP_LEN                      16

#define DEFAULT_TTL                     15
#define DEFAULT_HEARTBEAT_TIMES         3
#define DEFAULT_HEARTBEAT_SECS          15
#define DEFAULT_MIN_HEART_SECS          3
#define DEFAULT_MAX_HEART_SECS          120

#define DEFAULT_CONNECTING_SECS         5
#define DEFAULT_DISCONNECTED_SECS       15
#define DEFAULT_DISCONNECTING_SECS      5
#define DEFAULT_REGISTERING_SECS        5

#define DEF_MAX_MEDIA_SIZE              1024

#define DEF_PROXY_TIME_ZOE              8

#define DEF_CONFIG_SERVICE_NAME         "Config Service"
#define DEF_PLATFORM_SERVICE_NAME       "Platform Service"

typedef enum
{
    SDK_HIK,
    SDK_DAH,
    SDK_HBN,
    SDK_BSM,
    SDK_JNY,
    SDK_HIE,
    SDK_XMT,
    SDK_TPS,
    MAX_SDK,
}SDK_TYPE_E;


enum
{
    ALARM_IN,
    ALARM_OUT
};

enum
{
    SWAP_PACK,
    SWAP_UNPACK,
};

enum
{
    CFG_SRV = 0x101,
    PLT_SRV = 0x102,
    SDK_SRV = 0x103,
};

/* 服务对象控制命令*/
enum
{
    CMD_STOP_ALL_STREAM,    /* 关闭所有打开的流句柄 */
};

typedef enum
{
    DEV_PRI_MIN = -1,
    DEV_PRI_PLT_SRV,
    DEV_PRI_PRX_SDK,
    DEV_PRI_MAX
}PRI_TYPE_E;        /* private data id */

typedef enum
{
    DISCONNECTED,
    CONNECTING,
    CONNECTED,//==UNREGISTERED
    REGISTERED,
    DISCONNECTING
}CONN_STATE_E;

typedef enum
{
    CTRL_CMD_LOGOUT   = 0x0,
    CTRL_CMD_LOGIN    = 0x1,
    CTRL_CMD_RESET    = 0x2,
    CTRL_CMD_REBOOT   = 0x3,
    CTRL_CMD_SHUTDOWN = 0x4,
    CTRL_CMD_FRMTDISK = 0x5,
    CTRL_CMD_SUBMIT   = 0x6,

    CTRL_CMD_COMPARE  = 0x10,

    CTRL_CMD_OBTAIN_HANDLER  = 0x20,
    CTRL_CMD_OBTAIN_STRM_OPT = 0x21,
    CTRL_CMD_OBTAIN_TALK_OPT = 0x22,
}CTRL_CMD_E;

typedef enum
{
    BROADCAST_EVENT = 0x100,
    CONTROL_DEVICE  = 0x101,
    RESOLVE_HOST    = 0x102,
    REBOOT_SERVER   = 0x103,
}PROXY_TASK_E;

typedef enum
{
    STATS_STREAM_NOINIT  =-1,
    STATS_STREAM_STOP    = 0,
    STATS_STREAM_PLAYING = 1,
    STATS_STREAM_PAUSE   = 2,
}stream_state_t;



typedef struct proxy_plt proxy_plt_t;

struct proxy_plt
{
//    char plt_name[MAX_USR_LEN];          /* 可能存在多平台 */
    char pu_id[MAX_PUID_LEN];
    int  pu_type;
    char cms_host[MAX_HOST_LEN];
    int  cms_port;
    char mds_host[MAX_HOST_LEN];
    int  mds_port;
//    int  rtsp_port;
    int  keep_alive_freq;   /* secs */
    int  protocol;          /* */
};

typedef struct proxy_sdk proxy_sdk_t;

struct proxy_sdk
{
    char sdk_name[MAX_USR_LEN];
    char username[MAX_USR_LEN];
    char password[MAX_PSW_LEN];
    char dev_host[MAX_HOST_LEN];
    int  dev_port;
};

typedef struct proxy_reg proxy_reg_t;

struct proxy_reg
{
    char pu_id[MAX_PUID_LEN];
    char cms_host[MAX_HOST_LEN];
    char dev_host[MAX_HOST_LEN];
    int  pu_type;
};

typedef struct proxy_config proxy_config_t;

struct proxy_config
{
    char host[MAX_HOST_LEN];
    int cmd_port;
    int rtsp_port;
};

#endif  /* __PROXY_INFO_H__ */

