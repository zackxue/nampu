
#include <string.h>
#include <sys/time.h>

#include "nmp_sdk.h"

#include "stream_api.h"
#include "rtsp-server.h"
#include "standard_frame.h"

#include "nmp_xmt_service.h"
#include "nmp_xmt_channel.h"

#define XMT_STREAM_HEAD_INFO            "XMT-STREAM-HEADER"


static __inline__ int 
xmt_get_start_time(const char *pri, H264_DVR_TIME *time)
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
xmt_get_end_time(const char *pri, H264_DVR_TIME *time)
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
xmt_time_compare(H264_DVR_TIME *time_base, H264_DVR_TIME *time)
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
xmt_make_key_frame(LONG user_id, int channel, int level)
{
    if (0 == level)
    {
        if (FALSE == H264_DVR_MakeKeyFrame(user_id, channel, 0))  // 强制关键帧只有在播数据才有效
        {
            show_warn("H264_DVR_MakeKeyFrame failed.\n");
        }
    }
    else if (0 < level)
    {
        if (FALSE == H264_DVR_MakeKeyFrame(user_id, channel, 1))
        { 
            show_warn("H264_DVR_MakeKeyFrame failed.\n");
        }
    }
}

static __inline__ stream_info_t *
xmt_find_record_stream_by_channel_and_time(stream_list_t *strm_list, 
    int channel, H264_DVR_TIME *start_time, H264_DVR_TIME *stop_time)
{
    nmp_list_t *list;
    xmt_rec_strm_t *tmp_strm;
    stream_info_t *rec_strm = NULL;

    NMP_ASSERT(strm_list);

    list = nmp_list_first(strm_list->list);
    while (list)
    {
        tmp_strm = (xmt_rec_strm_t*)nmp_list_data(list);
        if (!tmp_strm->strm_base.opened                          && 
            (channel == tmp_strm->strm_base.channel)             && 
            !xmt_time_compare(&tmp_strm->start_time, start_time) &&
            !xmt_time_compare(&tmp_strm->start_time, stop_time)) 
        {// 找到一个未使用过的录像流, 并返回
            rec_strm = (stream_info_t*)tmp_strm;
            break;
        }
        list = nmp_list_next(list);
    }

    return rec_strm;
}

static int 
xmt_real_data_callback(long handle, long data_type, 
        unsigned char *buffer, long buf_size, long user)
{//show_debug("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<%d:%d\n", data_type, buf_size);
    struct timeval ts;
    stream_info_t *strm_info;

    NMP_ASSERT(XMT_INVALID_HANDLE != handle);
    NMP_ASSERT(buffer && 0 < buf_size);
    NMP_ASSERT(user);

    strm_info = (stream_info_t*)user;
    switch (data_type)
    {
        case 0:
        default://帧数据
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
    }
}

static int 
xmt_record_data_callback(long handle, long data_type, 
        unsigned char *buffer, long buf_size, long user)
{//show_debug("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<%d:%d\n", data_type, buf_size);
    int flag = 0;
    stream_info_t *strm_info;
    xmt_rec_strm_t *rec_strm;

    NMP_ASSERT(XMT_INVALID_HANDLE != handle);
    NMP_ASSERT(buffer && 0 < buf_size);
    NMP_ASSERT(user);

    strm_info = (stream_info_t*)user;
    rec_strm  = (xmt_rec_strm_t*)strm_info;

    switch (data_type)
    {
        case 0:
            if (strm_info->opened && rec_strm->enable)
            {
                if (!test_stream_blockable((gpointer)strm_info->opened, 
                    (gsize)buf_size))
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
xmt_download_pos_callback(long handle, long total_size, 
        long download_size, long int user)
{//show_debug("----------------->%d:%d\n", total_size, download_size);
    stream_info_t *strm_info;
    xmt_rec_strm_t *rec_strm;

    NMP_ASSERT(XMT_INVALID_HANDLE != handle);
    NMP_ASSERT(user);

    strm_info = (stream_info_t*)user;
    rec_strm = (xmt_rec_strm_t*)user;

    if (strm_info->opened && rec_strm->enable)
    {
        if (-1 == (int)download_size)
        {
            show_info("----------------->The end frame! total_size: %d\n", 
                (int)total_size);
            write_stream_data((gpointer)strm_info->opened, 
                (gulong)0, (gulong)0, (guint8*)NULL, (gsize)0, 
                (guint8)15, (guint8*)NULL, (gsize)0, (guint8)0);
        }
    }
}

static __inline__ int 
xmt_init_real_stream(xmt_service_t *xmt_srv, stream_info_t *strm_info)
{
    strcpy(strm_info->media_info, XMT_STREAM_HEAD_INFO);
    strm_info->media_size = strlen(XMT_STREAM_HEAD_INFO);
    return 0;
}

static __inline__ int 
xmt_init_record_stream(xmt_service_t *xmt_srv, stream_info_t *strm_info)
{
    strcpy(strm_info->media_info, XMT_STREAM_HEAD_INFO);
    strm_info->media_size = strlen(XMT_STREAM_HEAD_INFO);
    return 0;
}

static __inline__ int 
xmt_open_real_stream(xmt_service_t *xmt_srv, stream_info_t *strm_info)
{
    int ret = -1;
    stream_info_t *real_strm;

    real_strm = find_stream_by_channel_and_level(&xmt_srv->rtsp.real_strm_list, 
                    strm_info->channel, strm_info->level);
    if (!real_strm)
    {
        real_strm = (stream_info_t*)nmp_new0(struct xmt_real_strm_info, 1);
        real_strm->handle  = XMT_INVALID_HANDLE;
        real_strm->opened  = strm_info->opened;
        real_strm->type    = strm_info->type;
        real_strm->level   = strm_info->level;
        real_strm->channel = strm_info->channel;
        real_strm->state   = STATS_STREAM_STOP;
        real_strm->sdk_srv = (struct service*)xmt_srv;

        strcpy(real_strm->media_info, XMT_STREAM_HEAD_INFO);
        real_strm->media_size = strlen(XMT_STREAM_HEAD_INFO);

        set_stream_user_data((gpointer)real_strm->opened, (void*)real_strm);
        add_one_stream(&xmt_srv->rtsp.real_strm_list, real_strm);
        ret = 0;
    }
    else
        show_warn("stream [channel:%d] [level:%d] already opened!!\n", 
            strm_info->channel, strm_info->level);

    return ret;
}

static __inline__ int 
xmt_open_record_stream(xmt_service_t *xmt_srv, stream_info_t *strm_info)
{
    int ret = -1;
    stream_info_t *rec_strm;

    H264_DVR_TIME start_time = {0};
    H264_DVR_TIME stop_time  = {0};

    if (0 != xmt_get_start_time((const char*)strm_info->pri_data, &start_time))
        return -1;
    if (0 != xmt_get_end_time((const char*)strm_info->pri_data, &stop_time))
        return -1;

    rec_strm = xmt_find_record_stream_by_channel_and_time(&xmt_srv->rtsp.rec_strm_list, 
                    strm_info->channel, &start_time, &stop_time);
    if (!rec_strm)
    {
        rec_strm = (struct stream_info*)nmp_new0(xmt_rec_strm_t, 1);
        rec_strm->handle  = XMT_INVALID_HANDLE;
        rec_strm->opened  = strm_info->opened;
        rec_strm->type    = strm_info->type;
        rec_strm->level   = strm_info->level;
        rec_strm->channel = strm_info->channel;
        rec_strm->state   = STATS_STREAM_STOP;
        rec_strm->sdk_srv = (struct service*)xmt_srv;

        strcpy(rec_strm->media_info, XMT_STREAM_HEAD_INFO);
        rec_strm->media_size = strlen(XMT_STREAM_HEAD_INFO);

        ((xmt_rec_strm_t*)rec_strm)->enable = 0;

        memcpy(&((xmt_rec_strm_t*)rec_strm)->start_time, 
            &start_time, sizeof(H264_DVR_TIME));
        memcpy(&((xmt_rec_strm_t*)rec_strm)->stop_time, 
            &stop_time, sizeof(H264_DVR_TIME));

        set_stream_user_data((gpointer)rec_strm->opened, (void*)rec_strm);
        add_one_stream(&xmt_srv->rtsp.rec_strm_list, rec_strm);
        ret = 0;
    }   
    else
        show_warn("stream [channel:%d] [level:%d] already opened!!\n", 
            strm_info->channel, strm_info->level);

    return ret;
}

static __inline__ int 
xmt_process_stream(xmt_service_t *xmt_srv, stream_info_t *strm_info, stream_cmd_t cmd)
{
    int ret = 0;
    int handle = XMT_INVALID_HANDLE;
    int user_id = xmt_get_user_id(&xmt_srv->parm);

    switch(cmd)
    {
        case CMD_STREAM_PLAY:
            if (STREAM_REAL == strm_info->type)
            {
                if (STATS_STREAM_PLAYING != strm_info->state)
                {
                    H264_DVR_CLIENTINFO clt_info;
                    clt_info.nChannel = strm_info->channel;
                    clt_info.nStream = strm_info->level;
                    clt_info.nMode = 0; //0:TCP方式
                    clt_info.hWnd = NULL;
                    handle = H264_DVR_RealPlay((long)user_id, &clt_info);
                    if (XMT_INVALID_HANDLE != handle)
                    {
                        H264_DVR_SetRealDataCallBack((long)handle, 
                            xmt_real_data_callback, (long)strm_info);

                        ret = 0;
                        strm_info->handle = handle;
                        strm_info->state  = STATS_STREAM_PLAYING;
                    }
                    else
                    {
                        ret = -1;
                        show_warn("H264_DVR_RealPlay(handle: %d, err: %d), ch: %d\n", 
                            handle, (int)H264_DVR_GetLastError(), strm_info->channel);
                    }
                }
                else
                {
                    xmt_make_key_frame((LONG)user_id, 
                        strm_info->channel, strm_info->level);
                }
            }
            else if (STREAM_VOD == strm_info->type || 
                     STREAM_DOWNLOAD == strm_info->type)
            {
                if (STATS_STREAM_PAUSE == strm_info->state && 
                    XMT_INVALID_HANDLE != strm_info->handle)
                {
                    strm_info->state = STATS_STREAM_PLAYING;
                    ((xmt_rec_strm_t*)strm_info)->enable = 1;
                }
                else if (STATS_STREAM_STOP == strm_info->state)
                {
                    show_info("Open record stream ...\n");
                    H264_DVR_FINDINFO find_info;
                    find_info.nChannelN0 = strm_info->channel;
                    find_info.nFileType = SDK_RECORD_REGULAR;
                    memcpy(&find_info.startTime, 
                        &((xmt_rec_strm_t*)strm_info)->start_time, 
                        sizeof(H264_DVR_TIME));
                    memcpy(&find_info.endTime, 
                        &((xmt_rec_strm_t*)strm_info)->stop_time, 
                        sizeof(H264_DVR_TIME));
                    handle = H264_DVR_PlayBackByTimeEx(
                                (long)user_id, 
                                &find_info, 
                                xmt_record_data_callback, 
                                (long)strm_info, 
                                xmt_download_pos_callback, 
                                (long)strm_info);
                    if (XMT_INVALID_HANDLE != handle)
                    {
                        strm_info->handle = handle;
                        strm_info->state = STATS_STREAM_PLAYING;
                        ((xmt_rec_strm_t*)strm_info)->enable = 1;
                    }
                    else
                    {
                        ret = -1;
                        show_warn("H264_DVR_PlayBackByTime(err: %d), ch: %d\n", 
                            (int)H264_DVR_GetLastError(), strm_info->channel);
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
            else if (STREAM_VOD == strm_info->type || 
                     STREAM_DOWNLOAD == strm_info->type)
            {
                if (STATS_STREAM_PLAYING == strm_info->state && 
                    XMT_INVALID_HANDLE != strm_info->handle)
                {
                    strm_info->state = STATS_STREAM_PAUSE;
                    ((xmt_rec_strm_t*)strm_info)->enable = 0;
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

static int xmt_stream_init(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    xmt_service_t *xmt_srv;
    NMP_ASSERT(owner && strm_info);

    xmt_srv = (xmt_service_t*)owner;
    if (XMT_LOGOUT == xmt_get_user_id(&xmt_srv->parm))
    {
        show_warn("xmt device is disconnected.\n");
    }
    else
    {
        switch (strm_info->type)
        {
            case STREAM_REAL://0 播放实时流
                ret = xmt_init_real_stream(xmt_srv, strm_info);
                strm_info->length = 0;
                break;
            case STREAM_VOD:      //2点播录像文件
            case STREAM_DOWNLOAD: //3下载录像文件
                ret = xmt_init_record_stream(xmt_srv, strm_info);
                strm_info->length = 0;
                break;

            default:
                break;
        }
    }

    return ret;
}
static int xmt_stream_open(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    xmt_service_t *xmt_srv;
    NMP_ASSERT(owner && strm_info);

    xmt_srv = (xmt_service_t*)owner;

    if (XMT_LOGOUT == xmt_get_user_id(&xmt_srv->parm))
    {
         show_warn("xmt device is disconnected..\n");
    }
    else
    {
        nmp_mutex_lock(xmt_srv->rtsp.lock);
        switch (strm_info->type)
        {
            case STREAM_REAL:               // 播放实时流
                ret = xmt_open_real_stream(xmt_srv, strm_info);
                break;
            case STREAM_VOD:                // 点播录像文件
            case STREAM_DOWNLOAD:           // 下载录像文件
                ret = xmt_open_record_stream(xmt_srv, strm_info);
                break;

            default:
                break;
        }
        nmp_mutex_unlock(xmt_srv->rtsp.lock);
    }

    return ret;
}
static int xmt_stream_play(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    xmt_service_t *xmt_srv;
    NMP_ASSERT(owner && strm_info);

    xmt_srv = (xmt_service_t*)owner;

    if (XMT_LOGOUT == xmt_get_user_id(&xmt_srv->parm))
    {
         show_warn("xmt device is disconnected..\n");
    }
    else
    {
        nmp_mutex_lock(xmt_srv->rtsp.lock);
        ret = xmt_process_stream(xmt_srv, strm_info, CMD_STREAM_PLAY);
        nmp_mutex_unlock(xmt_srv->rtsp.lock);
    }

    return ret;
}
static int xmt_stream_pause(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    xmt_service_t *xmt_srv;
    NMP_ASSERT(owner && strm_info);

    xmt_srv = (xmt_service_t*)owner;

    if (XMT_LOGOUT == xmt_get_user_id(&xmt_srv->parm))
    {
         show_warn("xmt device is disconnected..\n");
    }
    else
    {
        nmp_mutex_lock(xmt_srv->rtsp.lock);
        ret = xmt_process_stream(xmt_srv, strm_info, CMD_STREAM_PAUSE);
        nmp_mutex_unlock(xmt_srv->rtsp.lock);
    }

    return ret;
}
static int xmt_stream_close(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    int handle = XMT_INVALID_HANDLE;
    xmt_service_t *xmt_srv;
    NMP_ASSERT(owner && strm_info);

    xmt_srv = (xmt_service_t*)owner;

    switch (strm_info->type)
    {
        case STREAM_REAL:
            nmp_mutex_lock(xmt_srv->rtsp.lock);
            handle = strm_info->handle;
            strm_info->handle = XMT_INVALID_HANDLE;
            remove_one_stream(&xmt_srv->rtsp.real_strm_list, strm_info);
            nmp_mutex_unlock(xmt_srv->rtsp.lock);

            if (XMT_INVALID_HANDLE != handle)
            {
                if (H264_DVR_StopRealPlay(handle))
                    ret = 0;
                else
                    show_warn("H264_DVR_StopRealPlay(%d)\n", 
                        (int)H264_DVR_GetLastError());
            }

            memset(strm_info, 0, sizeof(xmt_real_strm_t));
            nmp_del(strm_info, xmt_real_strm_t, 1);
            break;

        case STREAM_VOD:
        case STREAM_DOWNLOAD:
            nmp_mutex_lock(xmt_srv->rtsp.lock);
            handle = strm_info->handle;
            strm_info->handle = XMT_INVALID_HANDLE;
            remove_one_stream(&xmt_srv->rtsp.rec_strm_list, strm_info);
            nmp_mutex_unlock(xmt_srv->rtsp.lock);

            if (XMT_INVALID_HANDLE != handle)
            {
                if (H264_DVR_StopPlayBack(handle))
                    ret = 0;
                else
                    show_warn("H264_DVR_StopPlayBack(%d)\n", 
                        (int)H264_DVR_GetLastError());
            }

            memset(strm_info, 0, sizeof(xmt_rec_strm_t));
            nmp_del(strm_info, xmt_rec_strm_t, 1);
            break;
    }

    return 0;
}
static int xmt_stream_seek(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    xmt_service_t *xmt_srv;
    NMP_ASSERT(owner && strm_info);

    xmt_srv = (xmt_service_t*)owner;

    if (XMT_LOGOUT == xmt_get_user_id(&xmt_srv->parm))
    {
         show_warn("xmt device is disconnected..\n");
    }
    else
    {
        nmp_mutex_lock(xmt_srv->rtsp.lock);
        ret = xmt_process_stream(xmt_srv, strm_info, CMD_STREAM_SEEK);
        nmp_mutex_unlock(xmt_srv->rtsp.lock);
    }

    return ret;
}
static int xmt_stream_ctrl(void *owner, int channel, int level, int cmd, void *value)
{
    int ret = -1;
    xmt_service_t *xmt_srv;
    NMP_ASSERT(owner);

    xmt_srv = (xmt_service_t*)owner;

    if (XMT_LOGOUT == xmt_get_user_id(&xmt_srv->parm))
    {
        show_warn("xmt device is disconnected.\n");
    }
    else
    {
        switch (cmd)
        {
            case DEF_STOP_ALL_STREAM_CMD:
                xmt_cleanup_stream_info(&xmt_srv->rtsp);
                ret = 0;
                break;
                
            default:
                break;
        }
    }

    return ret;
}

stream_operation_t xmt_strm_opt = 
{
    xmt_stream_init,
    xmt_stream_open,
    xmt_stream_play,
    xmt_stream_pause,
    xmt_stream_close,
    xmt_stream_seek,
    xmt_stream_ctrl,
};



