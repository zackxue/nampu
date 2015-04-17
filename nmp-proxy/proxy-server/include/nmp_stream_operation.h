
#ifndef __STREAM_OPERATION_H__
#define __STREAM_OPERATION_H__

#include "nmp_sdk.h"
#include "nmp_proxy_info.h"


#define DEF_TIMESTAMP                   40          // 指定默认时间戳，25p/s
#define RECORD_PACKET_SLEEP_TIME        (200*1000)  // 200ms

#define DEF_PRI_START_TIME              "startTime="
#define DEF_PRI_END_TIME                "endTime="

#define DEF_STOP_ALL_STREAM_CMD         1


typedef enum 
{
    STREAM_REAL     = 0,
    STREAM_VOD      = 2,
    STREAM_DOWNLOAD = 3
}stream_type_t;

typedef enum 
{
    CMD_STREAM_PLAY = 100,
    CMD_STREAM_PAUSE,
    CMD_STREAM_SEEK,
    CMD_STREAM_STOP, 
    CMD_STREAM_CLOSE,
    CMD_STREAM_CTRL
}stream_cmd_t;

typedef struct stream_info stream_info_t;

struct stream_info
{
    int handle;                 /* 设备流句柄 */
    void *opened;               /* 转发流句柄 */

    stream_state_t state;       /* stream_state_t */
    stream_type_t  type;        /* stream_type_t */

    int channel;                /* 设备通道号 */
    int level;                  /* 码流等级 */

    char media_info[DEF_MAX_MEDIA_SIZE];
    size_t media_size;

    void *pri_data;
    double length;

    unsigned long ts;           /* 时间戳 */

    struct service *sdk_srv;    /* pointer back to service */
};

typedef struct stream_list stream_list_t;

struct stream_list
{
    //nmp_mutex_t *lock;  <--实际上无法保证数据安全,故舍弃;由拥有者进行数据安全把控
    nmp_list_t  *list;
};


typedef void (*stop_stream)(stream_info_t *strm_info);
typedef void (*stop_stream_cb)(stream_info_t *strm_info);

typedef struct stream_operation stream_operation_t;

struct stream_operation
{
    int (*stream_init) (void *owner, stream_info_t *strm_info);
    int (*stream_open) (void *owner, stream_info_t *strm_info);
    int (*stream_play) (void *owner, stream_info_t *strm_info);
    int (*stream_pause)(void *owner, stream_info_t *strm_info);
    int (*stream_close)(void *owner, stream_info_t *strm_info);
    int (*stream_seek) (void *owner, stream_info_t *strm_info);
    int (*stream_ctrl) (void *owner, int channel, int level, int cmd, void *value);
};

#ifdef __cplusplus
extern "C" {
#endif

int    get_start_time(const char *pri, JTime *ts);
int    get_end_time(const char *pri, JTime *ts);
double get_play_length(JTime *stop_ts, JTime *start_ts);

int get_stream_handle();
int set_stream_handle();
int get_stream_state();
int set_stream_state();

stream_list_t *
add_one_stream(stream_list_t *strm_list, stream_info_t *strm_info);
stream_list_t *
remove_one_stream(stream_list_t *strm_list, stream_info_t *strm_info);
void 
destory_stream_list(stream_list_t *strm_list, stop_stream_cb stop_strm_cb, size_t size);

stream_info_t *
find_stream_by_opened(stream_list_t *strm_list, void *opened);
stream_info_t *
find_stream_by_handle(stream_list_t *strm_list, int handle);
stream_info_t *
find_stream_by_channel(stream_list_t *strm_list, int channel);
stream_info_t *
find_stream_by_channel_and_level(stream_list_t *strm_list, int channel, int level);




#ifdef __cplusplus
    }
#endif


#endif  //__STREAM_OPERATION_H__

