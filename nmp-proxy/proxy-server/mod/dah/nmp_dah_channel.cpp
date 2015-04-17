
#include <string.h>
#include <sys/time.h>

#include "nmp_sdk.h"

#include "stream_api.h"
#include "rtsp-server.h"
#include "standard_frame.h"

#include "nmp_dah_service.h"
#include "nmp_dah_channel.h"

#define DAH_STREAM_HEAD_INFO            "DHAV"


static __inline__ int 
dah_get_start_time(const char *pri, NET_TIME *time)
{
    int ret;
    JTime ts;

    ret = get_start_time(pri, &ts);
    if (!ret)
    {
        time->dwYear   = ts.year + J_SDK_DEF_BASE_YEAR;
        time->dwMonth  = ts.month;
        time->dwDay    = ts.date;
        time->dwHour   = ts.hour;
        time->dwMinute = ts.minute;
        time->dwSecond = ts.second;
    }

    return ret;
}

static __inline__ int 
dah_get_end_time(const char *pri, NET_TIME *time)
{
    int ret;
    JTime ts;

    ret = get_end_time(pri, &ts);
    if (!ret)
    {
        time->dwYear   = ts.year + J_SDK_DEF_BASE_YEAR;
        time->dwMonth  = ts.month;
        time->dwDay    = ts.date;
        time->dwHour   = ts.hour;
        time->dwMinute = ts.minute;
        time->dwSecond = ts.second;
    }

    return ret;
}

static __inline__ int 
dah_time_compare(NET_TIME *time_base, NET_TIME *time)
{
    if (time_base->dwYear   == time->dwYear   && 
        time_base->dwMonth  == time->dwMonth  && 
        time_base->dwDay    == time->dwDay    && 
        time_base->dwHour   == time->dwHour   && 
        time_base->dwMinute == time->dwMinute && 
        time_base->dwSecond == time->dwSecond)
    {
        return 0;
    }

    return -1;
}

static __inline__ void 
dah_make_key_frame(LONG user_id, int channel, int level)
{
    if (0 == level)
    {
        if (FALSE == CLIENT_MakeKeyFrame(user_id, channel, 0))  // 强制关键帧只有在播数据才有效
        {
            show_warn("CLIENT_MakeKeyFrame failed.\n");
        }
    }
    else if (0 < level)
    {
        if (FALSE == CLIENT_MakeKeyFrame(user_id, channel, 1))
        { 
            show_warn("CLIENT_MakeKeyFrame failed.\n");
        }
    }
}

static __inline__ stream_info_t *
dah_find_record_stream_by_channel_and_time(stream_list_t *strm_list, 
    int channel, NET_TIME *start_time, NET_TIME *stop_time)
{
    nmp_list_t *list;
    dah_rec_strm_t *tmp_strm;
    stream_info_t *rec_strm = NULL;

    NMP_ASSERT(strm_list);

    list = nmp_list_first(strm_list->list);
    while (list)
    {
        tmp_strm = (dah_rec_strm_t*)nmp_list_data(list);
        if (!tmp_strm->strm_base.opened                          && 
            (channel == tmp_strm->strm_base.channel)             && 
            !dah_time_compare(&tmp_strm->start_time, start_time) &&
            !dah_time_compare(&tmp_strm->start_time, stop_time)) 
        {// 找到一个未使用过的录像流, 并返回
            rec_strm = (stream_info_t*)tmp_strm;
            break;
        }
        list = nmp_list_next(list);
    }

    return rec_strm;
}

static void 
dah_real_data_callback(long int handle, unsigned int data_type, 
        unsigned char *buffer, unsigned int buf_size, long int user)
{//show_debug("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<%d:%d\n", data_type, buf_size);
    struct timeval ts;
    stream_info_t *strm_info;

    NMP_ASSERT(DAH_INVALID_HANDLE != handle);
    NMP_ASSERT(buffer && 0 < buf_size);
    NMP_ASSERT(user);

    strm_info = (stream_info_t*)user;
    switch (data_type)
    {
        case 0://原始数据(与SaveRealData保存的数据一致)
            if (strm_info->opened)
            {
                if (0 == strm_info->ts)
                {
                    gettimeofday(&ts, NULL);
                    strm_info->ts = ts.tv_sec*1000 + ts.tv_usec/1000;
                }
                else
                {
                    strm_info->ts += DEF_TIMESTAMP;
                }

                write_stream_data((gpointer)strm_info->opened, 
                    (gulong)strm_info->ts, (gulong)DEF_TIMESTAMP, 
                    (guint8*)buffer, (gsize)buf_size, 
                    (guint8)data_type, (guint8*)NULL, (gsize)0, (guint8)0);
            }
            break;

        case 1://帧数据
            if (strm_info->opened)
            {
                if (0 == strm_info->ts)
                {
                    gettimeofday(&ts, NULL);
                    strm_info->ts = ts.tv_sec*1000 + ts.tv_usec/1000;
                }
                else
                {
                    strm_info->ts += DEF_TIMESTAMP;
                }

                write_stream_data((gpointer)strm_info->opened, 
                    (gulong)strm_info->ts, (gulong)DEF_TIMESTAMP, 
                    (guint8*)buffer, (gsize)buf_size, 
                    (guint8)data_type, (guint8*)NULL, (gsize)0, (guint8)0);
            }
            break;

        case 2://yuv数据
            break;

        case 3://pcm音频数据
            break;

        default:
            break;
    }
}

static int 
dah_record_data_callback(long int handle, unsigned int data_type, 
        unsigned char*buffer, unsigned int buf_size, long int user)
{//show_debug("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<%d:%d\n", data_type, buf_size);
    int flag = 0;
    stream_info_t *strm_info;
    dah_rec_strm_t *rec_strm;

    NMP_ASSERT(DAH_INVALID_HANDLE != handle);
    NMP_ASSERT(buffer && 0 < buf_size);
    NMP_ASSERT(user);

    strm_info = (stream_info_t*)user;
    rec_strm  = (dah_rec_strm_t*)strm_info;

    switch (data_type)
    {
        case 0:
            if (strm_info->opened && rec_strm->enable)
            {
                if (!test_stream_blockable((gpointer)strm_info->opened, (gsize)buf_size))
                {
                    flag = 1;
                    write_stream_data((gpointer)strm_info->opened, 
                        (gulong)0, (gulong)0, (guint8*)buffer, (gsize)buf_size, 
                        (guint8)1, (guint8*)NULL, (gsize)0, (guint8)0);
                }
                else
                {
                    //show_debug("Sleep some times.\n");
                    usleep(RECORD_PACKET_SLEEP_TIME);
                }
            }
            break;

        default:
            break;
    }

    return flag;
}

//进度回调函数
static void 
dah_download_pos_callback(long int handle, unsigned int total_size, 
        unsigned int download_size, long int user)
{//show_debug("----------------->%d:%d\n", total_size, download_size);
    stream_info_t *strm_info;
    dah_rec_strm_t *rec_strm;

    NMP_ASSERT(DAH_INVALID_HANDLE != handle);
    NMP_ASSERT(user);

    strm_info = (stream_info_t*)user;
    rec_strm = (dah_rec_strm_t*)user;

    if (strm_info->opened && rec_strm->enable)
    {
        if (-1 == (int)download_size)
        {
            show_info("----------------->The end frame! total_size: %d\n", total_size);
            write_stream_data((gpointer)strm_info->opened, 
                (gulong)0, (gulong)0, (guint8*)NULL, (gsize)0, 
                (guint8)15, (guint8*)NULL, (gsize)0, (guint8)0);
        }
    }
}

static __inline__ int 
dah_init_real_stream(dah_service_t *dah_srv, stream_info_t *strm_info)
{
    strcpy(strm_info->media_info, DAH_STREAM_HEAD_INFO);
    strm_info->media_size = strlen(DAH_STREAM_HEAD_INFO);
    return 0;
}

static __inline__ int 
dah_init_record_stream(dah_service_t *dah_srv, stream_info_t *strm_info)
{
    strcpy(strm_info->media_info, DAH_STREAM_HEAD_INFO);
    strm_info->media_size = strlen(DAH_STREAM_HEAD_INFO);
    return 0;
}
static __inline__ int 
dah_open_real_stream(dah_service_t *dah_srv, stream_info_t *strm_info)
{
    int ret = -1;
    stream_info_t *real_strm;

    real_strm = find_stream_by_channel_and_level(&dah_srv->rtsp.real_strm_list, 
                    strm_info->channel, strm_info->level);
    if (!real_strm)
    {
        real_strm = (stream_info_t*)nmp_new0(struct dah_real_strm_info, 1);
        real_strm->handle  = DAH_INVALID_HANDLE;
        real_strm->opened  = strm_info->opened;
        real_strm->type    = strm_info->type;
        real_strm->level   = strm_info->level;
        real_strm->channel = strm_info->channel;
        real_strm->state   = STATS_STREAM_STOP;
        real_strm->sdk_srv = (struct service*)dah_srv;

        strcpy(real_strm->media_info, DAH_STREAM_HEAD_INFO);
        real_strm->media_size = strlen(DAH_STREAM_HEAD_INFO);

        set_stream_user_data((gpointer)real_strm->opened, (void*)real_strm);
        add_one_stream(&dah_srv->rtsp.real_strm_list, real_strm);
        ret = 0;
    }
    else
        show_warn("stream [channel:%d] [level:%d] already opened!!\n", 
            strm_info->channel, strm_info->level);

    return ret;
}

static __inline__ int 
dah_open_record_stream(dah_service_t *dah_srv, stream_info_t *strm_info)
{
    int ret = -1;
    stream_info_t *rec_strm;

    NET_TIME start_time = {0};
    NET_TIME stop_time  = {0};

    if (0 != dah_get_start_time((const char*)strm_info->pri_data, &start_time))
        return -1;
    if (0 != dah_get_end_time((const char*)strm_info->pri_data, &stop_time))
        return -1;

    rec_strm = dah_find_record_stream_by_channel_and_time(&dah_srv->rtsp.rec_strm_list, 
                    strm_info->channel, &start_time, &stop_time);
    if (!rec_strm)
    {
        rec_strm = (struct stream_info*)nmp_new0(dah_rec_strm_t, 1);
        rec_strm->handle  = DAH_INVALID_HANDLE;
        rec_strm->opened  = strm_info->opened;
        rec_strm->type    = strm_info->type;
        rec_strm->level   = strm_info->level;
        rec_strm->channel = strm_info->channel;
        rec_strm->state   = STATS_STREAM_STOP;
        rec_strm->sdk_srv = (struct service*)dah_srv;

        strcpy(rec_strm->media_info, DAH_STREAM_HEAD_INFO);
        rec_strm->media_size = strlen(DAH_STREAM_HEAD_INFO);

        ((dah_rec_strm_t*)rec_strm)->enable = 0;

        memcpy(&((dah_rec_strm_t*)rec_strm)->start_time, 
            &start_time, sizeof(NET_TIME));
        memcpy(&((dah_rec_strm_t*)rec_strm)->stop_time, 
            &stop_time, sizeof(NET_TIME));

        set_stream_user_data((gpointer)rec_strm->opened, (void*)rec_strm);
        add_one_stream(&dah_srv->rtsp.rec_strm_list, rec_strm);
        ret = 0;
    }   
    else
        show_warn("stream [channel:%d] [level:%d] already opened!!\n", 
            strm_info->channel, strm_info->level);

    return ret;
}

static __inline__ int 
dah_process_stream(dah_service_t *dah_srv, stream_info_t *strm_info, stream_cmd_t cmd)
{
    int level, ret = 0;
    int handle = DAH_INVALID_HANDLE;

    switch(cmd)
    {
        case CMD_STREAM_PLAY:
            if (STREAM_REAL == strm_info->type)
            {
                if (STATS_STREAM_PLAYING != strm_info->state)
                {
                    switch (strm_info->level)
                    {
                        case 0:
                            level = DH_RType_Realplay_0;
                            break;
                        case 1:
                            level = DH_RType_Realplay_1;
                            break;
                        case 2:
                            level = DH_RType_Realplay_2;
                            break;
                    }
                    handle = CLIENT_RealPlayEx((long int)
                                dah_get_user_id(&dah_srv->parm), 
                                strm_info->channel, NULL, 
                                (DH_RealPlayType)level);
                    if (DAH_INVALID_HANDLE != handle)
                    {
                        CLIENT_SetRealDataCallBack((long int)handle, 
                            dah_real_data_callback, (long int)strm_info);

                        strm_info->handle = handle;
                        strm_info->state  = STATS_STREAM_PLAYING;
                    }
                    else
                    {
                        ret = -1;
                        show_warn("CLIENT_RealPlayEx(handle: %d, err: 0x%x), ch: %d, level: %d\n", 
                            handle, CLIENT_GetLastError(), strm_info->channel, strm_info->level);
                    }
                }
                else
                {
                    dah_make_key_frame((LONG)dah_get_user_id(&dah_srv->parm), 
                        strm_info->channel, strm_info->level);
                }
            }
            else if (STREAM_VOD == strm_info->type || STREAM_DOWNLOAD == strm_info->type)
            {
                if (STATS_STREAM_PAUSE == strm_info->state && DAH_INVALID_HANDLE != strm_info->handle)
                {
                    strm_info->state = STATS_STREAM_PLAYING;
                    ((dah_rec_strm_t*)strm_info)->enable = 1;
                }
                else if (STATS_STREAM_STOP == strm_info->state)
                {
                    show_info("Open record stream ...\n");
                    handle = CLIENT_PlayBackByTimeEx(
                                (long int)dah_get_user_id(&dah_srv->parm), 
                                (int)strm_info->channel, 
                                (NET_TIME*)&((dah_rec_strm_t*)strm_info)->start_time, 
                                (NET_TIME*)&((dah_rec_strm_t*)strm_info)->stop_time, 
                                (void*)NULL, 
                                dah_download_pos_callback, 
                                (long int)strm_info, 
                                dah_record_data_callback, 
                                (long int)strm_info);
                    if (DAH_INVALID_HANDLE != handle)
                    {
                        strm_info->handle = handle;
                        strm_info->state = STATS_STREAM_PLAYING;
                        ((dah_rec_strm_t*)strm_info)->enable = 1;
                    }
                    else
                    {
                        ret = -1;
                        show_warn("CLIENT_PlayBackByTime(err: 0x%x), ch: %d\n", 
                            CLIENT_GetLastError(), strm_info->channel);
                    }
                }
                else  // 连续PLAY两次，不响应
                {
                }
            }
            break;

        case CMD_STREAM_PAUSE:
            if (STREAM_REAL == strm_info->type)
            {
            }
            else if (STREAM_VOD == strm_info->type || STREAM_DOWNLOAD == strm_info->type)
            {
                if (STATS_STREAM_PLAYING == strm_info->state && DAH_INVALID_HANDLE != strm_info->handle)
                {
                    strm_info->state = STATS_STREAM_PAUSE;
                    ((dah_rec_strm_t*)strm_info)->enable = 0;
                }
            }
            break;

        case CMD_STREAM_SEEK:
            break;

        case CMD_STREAM_STOP:
            break;

        case CMD_STREAM_CLOSE:
            break;

        case CMD_STREAM_CTRL:
            break;

        default:
            break;
    }

    return ret;
}

static int dah_stream_init(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    dah_service_t *dah_srv;
    NMP_ASSERT(owner && strm_info);

    dah_srv = (dah_service_t*)owner;
    if (DAH_LOGOUT == dah_get_user_id(&dah_srv->parm))
    {
        show_warn("dah device is disconnected.\n");
    }
    else
    {
        switch (strm_info->type)
        {
            case STREAM_REAL://0 播放实时流
                ret = dah_init_real_stream(dah_srv, strm_info);
                strm_info->length = 0;
                break;
            case STREAM_VOD:      //2点播录像文件
            case STREAM_DOWNLOAD: //3下载录像文件
                ret = dah_init_record_stream(dah_srv, strm_info);
                strm_info->length = 0;
                break;

            default:
                break;
        }
    }

    return ret;
}
static int dah_stream_open(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    dah_service_t *dah_srv;
    NMP_ASSERT(owner && strm_info);

    dah_srv = (dah_service_t*)owner;

    if (DAH_LOGOUT == dah_get_user_id(&dah_srv->parm))
    {
         show_warn("dah device is disconnected..\n");
    }
    else
    {
        nmp_mutex_lock(dah_srv->rtsp.lock);
        switch (strm_info->type)
        {
            case STREAM_REAL:               // 播放实时流
                ret = dah_open_real_stream(dah_srv, strm_info);
                break;
            case STREAM_VOD:                // 点播录像文件
            case STREAM_DOWNLOAD:           // 下载录像文件
                ret = dah_open_record_stream(dah_srv, strm_info);
                break;

            default:
                break;
        }
        nmp_mutex_unlock(dah_srv->rtsp.lock);
    }

    return ret;
}
static int dah_stream_play(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    dah_service_t *dah_srv;
    NMP_ASSERT(owner && strm_info);

    dah_srv = (dah_service_t*)owner;

    if (DAH_LOGOUT == dah_get_user_id(&dah_srv->parm))
    {
         show_warn("dah device is disconnected..\n");
    }
    else
    {
        nmp_mutex_lock(dah_srv->rtsp.lock);
        ret = dah_process_stream(dah_srv, strm_info, CMD_STREAM_PLAY);
        nmp_mutex_unlock(dah_srv->rtsp.lock);
    }

    return ret;
}
static int dah_stream_pause(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    dah_service_t *dah_srv;
    NMP_ASSERT(owner && strm_info);

    dah_srv = (dah_service_t*)owner;

    if (DAH_LOGOUT == dah_get_user_id(&dah_srv->parm))
    {
         show_warn("dah device is disconnected..\n");
    }
    else
    {
        nmp_mutex_lock(dah_srv->rtsp.lock);
        ret = dah_process_stream(dah_srv, strm_info, CMD_STREAM_PAUSE);
        nmp_mutex_unlock(dah_srv->rtsp.lock);
    }

    return ret;
}
static int dah_stream_close(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    int handle = DAH_INVALID_HANDLE;
    dah_service_t *dah_srv;
    NMP_ASSERT(owner && strm_info);

    dah_srv = (dah_service_t*)owner;

    switch (strm_info->type)
    {
        case STREAM_REAL:
            nmp_mutex_lock(dah_srv->rtsp.lock);
            handle = strm_info->handle;
            strm_info->handle = DAH_INVALID_HANDLE;
            remove_one_stream(&dah_srv->rtsp.real_strm_list, strm_info);
            nmp_mutex_unlock(dah_srv->rtsp.lock);

            if (DAH_INVALID_HANDLE != handle)
            {
                if (CLIENT_StopRealPlayEx(handle))
                    ret = 0;
                else
                    show_warn("CLIENT_StopRealPlayEx(0x%x)\n", 
                        CLIENT_GetLastError());
            }

            memset(strm_info, 0, sizeof(dah_real_strm_t));
            nmp_del(strm_info, dah_real_strm_t, 1);
            break;

        case STREAM_VOD:
        case STREAM_DOWNLOAD:
            nmp_mutex_lock(dah_srv->rtsp.lock);
            handle = strm_info->handle;
            strm_info->handle = DAH_INVALID_HANDLE;
            remove_one_stream(&dah_srv->rtsp.rec_strm_list, strm_info);
            nmp_mutex_unlock(dah_srv->rtsp.lock);

            if (DAH_INVALID_HANDLE != handle)
            {
                if (CLIENT_StopPlayBack(handle))
                    ret = 0;
                else
                    show_warn("CLIENT_StopPlayBack(0x%x)\n", 
                        CLIENT_GetLastError());
            }

            memset(strm_info, 0, sizeof(dah_rec_strm_t));
            nmp_del(strm_info, dah_rec_strm_t, 1);
            break;
    }

    return 0;
}
static int dah_stream_seek(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    dah_service_t *dah_srv;
    NMP_ASSERT(owner && strm_info);

    dah_srv = (dah_service_t*)owner;

    if (DAH_LOGOUT == dah_get_user_id(&dah_srv->parm))
    {
         show_warn("dah device is disconnected..\n");
    }
    else
    {
        nmp_mutex_lock(dah_srv->rtsp.lock);
        ret = dah_process_stream(dah_srv, strm_info, CMD_STREAM_SEEK);
        nmp_mutex_unlock(dah_srv->rtsp.lock);
    }

    return ret;
}
static int dah_stream_ctrl(void *owner, int channel, int level, int cmd, void *value)
{
    int ret = -1;
    dah_service_t *dah_srv;
    NMP_ASSERT(owner);

    dah_srv = (dah_service_t*)owner;

    if (DAH_LOGOUT == dah_get_user_id(&dah_srv->parm))
    {
        show_warn("dah device is disconnected.\n");
    }
    else
    {
        switch (cmd)
        {
            case DEF_STOP_ALL_STREAM_CMD:
                dah_cleanup_stream_info(&dah_srv->rtsp);
                ret = 0;
                break;
                
            default:
                break;
        }
    }

    return ret;
}

stream_operation_t dah_strm_opt = 
{
    dah_stream_init,
    dah_stream_open,
    dah_stream_play,
    dah_stream_pause,
    dah_stream_close,
    dah_stream_seek,
    dah_stream_ctrl,
};


