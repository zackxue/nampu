#ifndef __DAH_SDKINFO_H__
#define __DAH_SDKINFO_H__

#define DAH_STREAM_HEAD_SIZE        40
#define DAH_NVR_CHANNEL_OFFSET      32

#define DEF_OUT_JSON_SIZE           (32*1024)

#define DAH_INVALID_HANDLE          0


typedef enum
{
    DAH_UNKNOWN=-1,
    DAH_LOGOUT = 0,
    DAH_LOGING = 1,
    DAH_LOGIN  = 2,
}DAH_STATE_E;

typedef struct dah_config dah_config_t;

struct dah_config
{
    int    command;
    int    channel;
    void  *buffer;
    size_t b_size;
    size_t returns;   //for get action
    int    waittime;
};

typedef struct dah_new_config dah_new_config_t;

struct dah_new_config
{
    int    channel;
    char  *command;
    char   out_json[DEF_OUT_JSON_SIZE];
    char  *buffer;
    size_t b_size;
    int    waittime;
};

typedef struct dah_ptz_ctrl dah_ptz_ctrl_t;

struct dah_ptz_ctrl
{
    int channel;
    int ptz_cmd;
    int step;
    int stop;
};

typedef struct dah_query dah_query_t;

struct dah_query
{
    int channel;
    int file_type;
    NET_TIME start;
    NET_TIME end;
    char *card_id;
    char *buffer;
    int buf_size;
    int filecount;
    int waittime;
};

typedef struct dah_parse dah_parse_t;

struct dah_parse
{
    char *command;
    char *in_buf;
    char *out_buf;
    char *reserve;
};


#endif  //__DAH_SDKINFO_H__

