#ifndef __XMT_SDKINFO_H__
#define __XMT_SDKINFO_H__

#define XMT_STREAM_HEAD_SIZE        40
#define XMT_NVR_CHANNEL_OFFSET      32

#define DEF_OUT_JSON_SIZE           (32*1024)

#define XMT_INVALID_HANDLE          0


typedef enum
{
    XMT_UNKNOWN=-1,
    XMT_LOGOUT = 0,
    XMT_LOGING = 1,
    XMT_LOGIN  = 2,
}XMT_STATE_E;

typedef struct xmt_config xmt_config_t;

struct xmt_config
{
    int    command;
    int    channel;
    void  *buffer;
    size_t b_size;
    size_t returns;   //for get action
    int    waittime;
};

typedef struct xmt_ptz_ctrl xmt_ptz_ctrl_t;

struct xmt_ptz_ctrl
{
    int channel;
    int ptz_cmd;
    int step;
    int stop;
};

typedef struct xmt_query xmt_query_t;

struct xmt_query
{
    int channel;
    int file_type;
    H264_DVR_TIME start;
    H264_DVR_TIME end;
    char *buffer;
    int buf_size;
    int max_count;
    int filecount;
};

typedef struct xmt_cruise xmt_cruise_t;

struct xmt_cruise
{
    int channel;
    int crz_cmd;
    int crz_no;
    int input;
};



#endif  //__XMT_SDKINFO_H__

