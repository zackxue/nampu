#ifndef __HIK_SDKINFO_H__
#define __HIK_SDKINFO_H__

//#include "nmp_proxy_server.h"


#define HIK_STREAM_HEAD_SIZE        40
#define HIK_NVR_CHANNEL_OFFSET      32

#define HIK_INVALID_HANDLE          -1


typedef enum
{
    HIK_UNKNOWN=-2,
    HIK_LOGOUT =-1,
    HIK_LOGING = 0,
    HIK_LOGIN  = 1,
}HIK_STATE_E;

typedef struct hik_config hik_config_t;

struct hik_config
{
    int    command;
    int    channel;
    void  *buffer;
    size_t b_size;
    size_t returns;   //for get action
};

typedef struct hik_video_effect hik_video_effect_t;

struct hik_video_effect
{
    short bright;
    short contrast;
    short saturation;
    short hue;
};

typedef struct hik_ptz_ctrl hik_ptz_ctrl_t;

struct hik_ptz_ctrl
{
    int channel;
    int ptz_cmd;
    int stop;
};

typedef struct hik_format hik_format_t;

struct hik_format
{
    int handle;
    int disk_no;
    int disk_pos;
    int state;      //0-正在格式化；1-硬盘全部格式化完成；2-格式化当前硬盘出错，不能继续格式化此硬盘，本地和网络硬盘都会出现此错误；3-由于网络异常造成网络硬盘丢失而不能开始格式化当前硬盘
};

typedef struct hik_preset hik_preset_t;

struct hik_preset
{
    int channel;
    int preset_cmd;
    int preset_no;
};

typedef struct hik_cruise hik_cruise_t;

struct hik_cruise
{
    int channel;
    int crz_cmd;
    int crz_no;
    int input;
};

#endif  //__HIK_SDKINFO_H__


