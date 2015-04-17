#ifndef __TPS_SDKINFO_H__
#define __TPS_SDKINFO_H__

#define TPS_STREAM_HEAD_SIZE        40
#define TPS_NVR_CHANNEL_OFFSET      32

#define DEF_OUT_JSON_SIZE           (32*1024)

#define TPS_INVALID_HANDLE          0


typedef enum
{
    TPS_UNKNOWN=-2,
    TPS_OUTING =-1,
    TPS_LOGOUT = 0,
    TPS_LOGING = 1,
    TPS_LOGIN  = 2,
}TPS_STATE_E;

typedef struct tps_config tps_config_t;

struct tps_config
{
    int    command;
    int    channel;
    void  *buffer;
    size_t b_size;
    size_t returns;   //for get action
    int    waittime;
};

typedef struct tps_ptz_ctrl tps_ptz_ctrl_t;

struct tps_ptz_ctrl
{
    int channel;
    int ptz_cmd;
    int step;
    int stop;
};

typedef struct tps_query tps_query_t;

struct tps_query
{
    int channel;
    int file_type;
    //H264_DVR_TIME start;
    //H264_DVR_TIME end;
    char *buffer;
    int buf_size;
    int max_count;
    int filecount;
};

typedef struct tps_cruise tps_cruise_t;

struct tps_cruise
{
    int channel;
    int crz_cmd;
    int crz_no;
    int input;
};



#endif  //__TPS_SDKINFO_H__


