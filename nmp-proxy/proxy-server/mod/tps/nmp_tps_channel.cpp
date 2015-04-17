
#include <string.h>
#include <sys/time.h>

#include "nmp_sdk.h"

#include "stream_api.h"
#include "rtsp-server.h"
#include "standard_frame.h"

#include "nmp_tps_service.h"
#include "nmp_tps_channel.h"

#define TPS_STREAM_HEAD_INFO            "TPS-STREAM-HEADER"


/*static __inline__ int 
tps_get_start_time(const char *pri, H264_DVR_TIME *time)
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
}*/

/*static __inline__ int 
tps_get_end_time(const char *pri, H264_DVR_TIME *time)
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
}*/

/*static __inline__ int 
tps_time_compare(H264_DVR_TIME *time_base, H264_DVR_TIME *time)
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
}*/

static __inline__ void 
tps_make_key_frame(LONG user_id, int channel, int level)
{
    /*if (0 == level)
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
    }*/
}

/*static __inline__ stream_info_t *
tps_find_record_stream_by_channel_and_time(stream_list_t *strm_list, 
    int channel, H264_DVR_TIME *start_time, H264_DVR_TIME *stop_time)
{
    nmp_list_t *list;
    tps_rec_strm_t *tmp_strm;
    stream_info_t *rec_strm = NULL;

    NMP_ASSERT(strm_list);

    list = nmp_list_first(strm_list->list);
    while (list)
    {
        tmp_strm = (tps_rec_strm_t*)nmp_list_data(list);
        if (!tmp_strm->strm_base.opened                          && 
            (channel == tmp_strm->strm_base.channel)             && 
            !tps_time_compare(&tmp_strm->start_time, start_time) &&
            !tps_time_compare(&tmp_strm->start_time, stop_time)) 
        {// 找到一个未使用过的录像流, 并返回
            rec_strm = (stream_info_t*)tmp_strm;
            break;
        }
        list = nmp_list_next(list);
    }

    return rec_strm;
}*/

static int 
tps_real_data_callback(LONG handle, DWORD data_type, BYTE *buffer, 
    DWORD buf_size, FRAME_EXTDATA *ext_data)
{//show_debug("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<%d:%d\n", data_type, buf_size);
    stream_info_t *strm_info;

    NMP_ASSERT(TPS_INVALID_HANDLE != handle);
    NMP_ASSERT(buffer && 0 < buf_size);
    NMP_ASSERT(ext_data);

    strm_info = (stream_info_t*)ext_data->pUserData;
    switch (data_type)
    {
        case 0://表示视频
            if (strm_info->opened)
            {
                write_stream_data((gpointer)strm_info->opened, 
                    (gulong)ext_data->timestamp, (gulong)DEF_TIMESTAMP, 
                    (guint8*)buffer, (gsize)buf_size, 
                    (guint8)ext_data->bIsKey, (guint8*)NULL, (gsize)0, (guint8)0);
            }
            break;
        case 1://表示音频
            break;
        case 2://表示解码参数
            if (!strm_info->media_size)
            {
                memcpy(strm_info->media_info, buffer, buf_size);
                strm_info->media_size = buf_size;
                
                show_debug("\nsignal cond after real data callback.[size: %d]\n", 
                    strm_info->media_size);
                proxy_cond_signal(((tps_real_strm_t*)strm_info)->cond);
            }
            break;
        default:
            break;
    }

    return 0;
}

static int 
tps_record_data_callback(long handle, long data_type, 
        unsigned char *buffer, long buf_size, long user)
{//show_debug("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<%d:%d\n", data_type, buf_size);
    int flag = 0;
    stream_info_t *strm_info;
    tps_rec_strm_t *rec_strm;

    NMP_ASSERT(TPS_INVALID_HANDLE != handle);
    NMP_ASSERT(buffer && 0 < buf_size);
    NMP_ASSERT(user);

    strm_info = (stream_info_t*)user;
    rec_strm  = (tps_rec_strm_t*)strm_info;

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
tps_download_pos_callback(long handle, long total_size, 
        long download_size, long int user)
{//show_debug("----------------->%d:%d\n", total_size, download_size);
    stream_info_t *strm_info;
    tps_rec_strm_t *rec_strm;

    NMP_ASSERT(TPS_INVALID_HANDLE != handle);
    NMP_ASSERT(user);

    strm_info = (stream_info_t*)user;
    rec_strm = (tps_rec_strm_t*)user;

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
tps_init_real_stream(tps_service_t *tps_srv, stream_info_t *strm_info)
{
    int ret = -1, offset = 0;
    int handle = TPS_INVALID_HANDLE;

    stream_info_t *real_strm;
    USRE_VIDEOINFO video_info;

    memset(&video_info, 0, sizeof(USRE_VIDEOINFO));

    nmp_mutex_lock(tps_srv->rtsp.lock);
    real_strm = find_stream_by_channel_and_level(&tps_srv->rtsp.real_strm_list, 
                    strm_info->channel, strm_info->level);
    if (real_strm)
    {
        memcpy(strm_info->media_info, real_strm->media_info, real_strm->media_size);
        strm_info->media_size = real_strm->media_size;
        nmp_mutex_unlock(tps_srv->rtsp.lock);
        return 0;
    }
    nmp_mutex_unlock(tps_srv->rtsp.lock);

    proxy_cond_new(&((tps_real_strm_t*)strm_info)->cond);

    video_info.bIsTcp = 1;
    video_info.nVideoChannle = strm_info->channel;
    video_info.nVideoPort = 554;
    video_info.pUserData = strm_info;

    handle = IP_NET_DVR_RealPlay(
                    (LONG)tps_get_user_id(&tps_srv->parm), 
                    (IP_NET_DVR_CLIENTINFO*)NULL, 
                    (fRealDataCallBack)tps_real_data_callback, 
                    (USRE_VIDEOINFO*)&video_info, (BOOL)0);
    if (TPS_INVALID_HANDLE != handle)
    {
        proxy_cond_timed_wait(((tps_real_strm_t*)strm_info)->cond, 
            MAX_INIT_WAITE_TIMES);

        IP_NET_DVR_StopRealPlay((LONG)handle);

        if (strm_info->media_size)
        {
            ret = 0;
        }
        else
        {
            show_warn("stream header is not get.\n");
            ret = -1;
        }
    }
    else
    {
        show_warn("NET_DVR_RealPlay_V30() fail.\n");
        ret = -1;
    }
    proxy_cond_free(((tps_real_strm_t*)strm_info)->cond);

    return ret;
}

static __inline__ int 
tps_init_record_stream(tps_service_t *tps_srv, stream_info_t *strm_info)
{
    strcpy(strm_info->media_info, TPS_STREAM_HEAD_INFO);
    strm_info->media_size = strlen(TPS_STREAM_HEAD_INFO);
    return 0;
}

static __inline__ int 
tps_open_real_stream(tps_service_t *tps_srv, stream_info_t *strm_info)
{
    int ret = -1, offset = 0;
    int handle = TPS_INVALID_HANDLE;

    stream_info_t *real_strm;
    USRE_VIDEOINFO video_info;

    memset(&video_info, 0, sizeof(USRE_VIDEOINFO));

    if (!find_stream_by_channel_and_level(&tps_srv->rtsp.real_strm_list, 
            strm_info->channel, strm_info->level))
    {
        proxy_cond_new(&((tps_real_strm_t*)strm_info)->cond);

        video_info.bIsTcp = 1;
        video_info.nVideoChannle = strm_info->channel;
        video_info.nVideoPort = 554;
        video_info.pUserData = strm_info;

        handle = IP_NET_DVR_RealPlay((LONG)tps_get_user_id(&tps_srv->parm), 
                        (IP_NET_DVR_CLIENTINFO*)NULL, 
                        (fRealDataCallBack)tps_real_data_callback, 
                        (USRE_VIDEOINFO*)&video_info, (BOOL)0);
        if (TPS_INVALID_HANDLE != handle)
        {
            proxy_cond_timed_wait(((tps_real_strm_t*)strm_info)->cond, 
                MAX_INIT_WAITE_TIMES);
            IP_NET_DVR_StopRealPlay((LONG)handle);

            if (strm_info->media_size)
            {
                ret = 0;
            }
            else
                show_warn("stream header is not get.\n");
        }
        else
            show_warn("NET_DVR_RealPlay_V30() fail.\n");

        proxy_cond_free(((tps_real_strm_t*)strm_info)->cond);
    }
    else
    {
        show_warn("stream [channel:%d] [level:%d] already opened!!\n", 
            strm_info->channel, strm_info->level);
    }

    if (!ret)
    {
        real_strm = (stream_info_t*)nmp_new0(tps_real_strm_t, 1);
        real_strm->handle  = TPS_INVALID_HANDLE;
        real_strm->opened  = strm_info->opened;
        real_strm->type    = strm_info->type;
        real_strm->level   = strm_info->level;
        real_strm->channel = strm_info->channel;
        real_strm->state   = STATS_STREAM_STOP;
        real_strm->sdk_srv = (struct service*)tps_srv;

        memcpy(real_strm->media_info, strm_info->media_info, strm_info->media_size);
        real_strm->media_size = strm_info->media_size;

        set_stream_user_data((gpointer)strm_info->opened, (void*)real_strm);
        add_one_stream(&tps_srv->rtsp.real_strm_list, real_strm);
    }

    return ret;
}

static __inline__ int 
tps_open_record_stream(tps_service_t *tps_srv, stream_info_t *strm_info)
{
    int ret = -1;
    stream_info_t *rec_strm;

    /*H264_DVR_TIME start_time = {0};
    H264_DVR_TIME stop_time  = {0};

    if (0 != tps_get_start_time((const char*)strm_info->pri_data, &start_time))
        return -1;
    if (0 != tps_get_end_time((const char*)strm_info->pri_data, &stop_time))
        return -1;

    rec_strm = tps_find_record_stream_by_channel_and_time(&tps_srv->rtsp.rec_strm_list, 
                    strm_info->channel, &start_time, &stop_time);*/
    if (!rec_strm)
    {
        rec_strm = (struct stream_info*)nmp_new0(tps_rec_strm_t, 1);
        rec_strm->handle  = TPS_INVALID_HANDLE;
        rec_strm->opened  = strm_info->opened;
        rec_strm->type    = strm_info->type;
        rec_strm->level   = strm_info->level;
        rec_strm->channel = strm_info->channel;
        rec_strm->state   = STATS_STREAM_STOP;
        rec_strm->sdk_srv = (struct service*)tps_srv;

        strcpy(rec_strm->media_info, TPS_STREAM_HEAD_INFO);
        rec_strm->media_size = strlen(TPS_STREAM_HEAD_INFO);

        ((tps_rec_strm_t*)rec_strm)->enable = 0;

        /*memcpy(&((tps_rec_strm_t*)rec_strm)->start_time, 
            &start_time, sizeof(H264_DVR_TIME));
        memcpy(&((tps_rec_strm_t*)rec_strm)->stop_time, 
            &stop_time, sizeof(H264_DVR_TIME));*/

        set_stream_user_data((gpointer)rec_strm->opened, (void*)rec_strm);
        add_one_stream(&tps_srv->rtsp.rec_strm_list, rec_strm);
        ret = 0;
    }   
    else
        show_warn("stream [channel:%d] [level:%d] already opened!!\n", 
            strm_info->channel, strm_info->level);

    return ret;
}

static __inline__ int 
tps_process_stream(tps_service_t *tps_srv, stream_info_t *strm_info, stream_cmd_t cmd)
{
    int ret = 0;
    int handle = TPS_INVALID_HANDLE;
    int user_id = tps_get_user_id(&tps_srv->parm);

    USRE_VIDEOINFO video_info;

    switch(cmd)
    {
        case CMD_STREAM_PLAY:
            if (STREAM_REAL == strm_info->type)
            {
                if (STATS_STREAM_PLAYING != strm_info->state)
                {
                    memset(&video_info, 0, sizeof(USRE_VIDEOINFO));
                    video_info.bIsTcp = 1;
                    video_info.nVideoChannle = strm_info->channel;
                    video_info.nVideoPort = 554;
                    video_info.pUserData = strm_info;

                    handle = IP_NET_DVR_RealPlay((LONG)user_id, 
                                    (IP_NET_DVR_CLIENTINFO*)NULL, 
                                    (fRealDataCallBack)tps_real_data_callback, 
                                    (USRE_VIDEOINFO*)&video_info, (BOOL)0);
                    if (TPS_INVALID_HANDLE != handle)
                    {
                        ret = 0;
                        strm_info->handle = handle;
                        strm_info->state  = STATS_STREAM_PLAYING;
                    }
                    else
                    {
                        ret = -1;
                        show_warn("IP_NET_DVR_RealPlay() fail, ch: %d\n", 
                            strm_info->channel);
                    }
                }
                else
                {
                    tps_make_key_frame((LONG)user_id, 
                        strm_info->channel, strm_info->level);
                }
            }
            else if (STREAM_VOD == strm_info->type || 
                     STREAM_DOWNLOAD == strm_info->type)
            {
                if (STATS_STREAM_PAUSE == strm_info->state && 
                    TPS_INVALID_HANDLE != strm_info->handle)
                {
                    strm_info->state = STATS_STREAM_PLAYING;
                    ((tps_rec_strm_t*)strm_info)->enable = 1;
                }
                else if (STATS_STREAM_STOP == strm_info->state)
                {
                    show_info("Open record stream ...\n");
                    /*H264_DVR_FINDINFO find_info;
                    find_info.nChannelN0 = strm_info->channel;
                    find_info.nFileType = SDK_RECORD_REGULAR;
                    memcpy(&find_info.startTime, 
                        &((tps_rec_strm_t*)strm_info)->start_time, 
                        sizeof(H264_DVR_TIME));
                    memcpy(&find_info.endTime, 
                        &((tps_rec_strm_t*)strm_info)->stop_time, 
                        sizeof(H264_DVR_TIME));
                    handle = H264_DVR_PlayBackByTimeEx(
                                (long)user_id, 
                                &find_info, 
                                tps_record_data_callback, 
                                (long)strm_info, 
                                tps_download_pos_callback, 
                                (long)strm_info);*/
                    if (TPS_INVALID_HANDLE != handle)
                    {
                        strm_info->handle = handle;
                        strm_info->state = STATS_STREAM_PLAYING;
                        ((tps_rec_strm_t*)strm_info)->enable = 1;
                    }
                    else
                    {
                        ret = -1;
                        //show_warn("H264_DVR_PlayBackByTime(err: %d), ch: %d\n", 
                          //  (int)H264_DVR_GetLastError(), strm_info->channel);
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
                    TPS_INVALID_HANDLE != strm_info->handle)
                {
                    strm_info->state = STATS_STREAM_PAUSE;
                    ((tps_rec_strm_t*)strm_info)->enable = 0;
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

static int tps_stream_init(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    tps_service_t *tps_srv;
    NMP_ASSERT(owner && strm_info);

    tps_srv = (tps_service_t*)owner;
    if (TPS_LOGOUT == tps_get_user_id(&tps_srv->parm))
    {
        show_warn("tps device is disconnected.\n");
    }
    else
    {
        switch (strm_info->type)
        {
            case STREAM_REAL:       //0 播放实时流
                ret = tps_init_real_stream(tps_srv, strm_info);
                strm_info->length = 0;
                break;
            case STREAM_VOD:        //2点播录像文件
            case STREAM_DOWNLOAD:   //3下载录像文件
                ret = tps_init_record_stream(tps_srv, strm_info);
                strm_info->length = 0;
                break;

            default:
                break;
        }
    }

    return ret;
}
static int tps_stream_open(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    tps_service_t *tps_srv;
    NMP_ASSERT(owner && strm_info);

    tps_srv = (tps_service_t*)owner;

    if (TPS_LOGOUT == tps_get_user_id(&tps_srv->parm))
    {
         show_warn("tps device is disconnected..\n");
    }
    else
    {
        nmp_mutex_lock(tps_srv->rtsp.lock);
        switch (strm_info->type)
        {
            case STREAM_REAL:               // 播放实时流
                ret = tps_open_real_stream(tps_srv, strm_info);
                break;
            case STREAM_VOD:                // 点播录像文件
            case STREAM_DOWNLOAD:           // 下载录像文件
                ret = tps_open_record_stream(tps_srv, strm_info);
                break;

            default:
                break;
        }
        nmp_mutex_unlock(tps_srv->rtsp.lock);
    }

    return ret;
}
static int tps_stream_play(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    tps_service_t *tps_srv;
    NMP_ASSERT(owner && strm_info);

    tps_srv = (tps_service_t*)owner;

    if (TPS_LOGOUT == tps_get_user_id(&tps_srv->parm))
    {
         show_warn("tps device is disconnected..\n");
    }
    else
    {
        nmp_mutex_lock(tps_srv->rtsp.lock);
        ret = tps_process_stream(tps_srv, strm_info, CMD_STREAM_PLAY);
        nmp_mutex_unlock(tps_srv->rtsp.lock);
    }

    return ret;
}
static int tps_stream_pause(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    tps_service_t *tps_srv;
    NMP_ASSERT(owner && strm_info);

    tps_srv = (tps_service_t*)owner;

    if (TPS_LOGOUT == tps_get_user_id(&tps_srv->parm))
    {
         show_warn("tps device is disconnected..\n");
    }
    else
    {
        nmp_mutex_lock(tps_srv->rtsp.lock);
        ret = tps_process_stream(tps_srv, strm_info, CMD_STREAM_PAUSE);
        nmp_mutex_unlock(tps_srv->rtsp.lock);
    }

    return ret;
}
static int tps_stream_close(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    int handle = TPS_INVALID_HANDLE;
    tps_service_t *tps_srv;
    NMP_ASSERT(owner && strm_info);

    tps_srv = (tps_service_t*)owner;

    switch (strm_info->type)
    {
        case STREAM_REAL:
            nmp_mutex_lock(tps_srv->rtsp.lock);
            handle = strm_info->handle;
            strm_info->handle = TPS_INVALID_HANDLE;
            remove_one_stream(&tps_srv->rtsp.real_strm_list, strm_info);
            nmp_mutex_unlock(tps_srv->rtsp.lock);

            if (TPS_INVALID_HANDLE != handle)
            {
                if (!IP_NET_DVR_StopRealPlay((LONG)handle))
                    ret = 0;
                else
                    show_warn("IP_NET_DVR_StopRealPlay()\n");
            }

            memset(strm_info, 0, sizeof(tps_real_strm_t));
            nmp_del(strm_info, tps_real_strm_t, 1);
            break;

        case STREAM_VOD:
        case STREAM_DOWNLOAD:
            nmp_mutex_lock(tps_srv->rtsp.lock);
            handle = strm_info->handle;
            strm_info->handle = TPS_INVALID_HANDLE;
            remove_one_stream(&tps_srv->rtsp.rec_strm_list, strm_info);
            nmp_mutex_unlock(tps_srv->rtsp.lock);

            if (TPS_INVALID_HANDLE != handle)
            {
                ret = IP_NET_DVR_ControlPlay((LONG)tps_get_user_id(&tps_srv->parm), 
                        (LONG)ACTION_STOP, (LONG)NULL);
                switch (ret)
                {
                case ERR_NOT_REPLAY_MODE_ERROR: //当前不是处于回放模式
                    show_warn("IP_NET_DVR_ControlPlay() fail, err: ERR_NOT_REPLAY_MODE_ERROR\n");
                    break;
                case ERR_NOT_FIND_DEVICE:       //句柄错误
                    show_warn("IP_NET_DVR_ControlPlay() fail, err: ERR_NOT_FIND_DEVICE\n");
                    break;
                case ERR_DEV_NOT_CONNECTED:     //设备未连接
                    show_warn("IP_NET_DVR_ControlPlay() fail, err: ERR_DEV_NOT_CONNECTED\n");
                    break;
                case ERR_DEV_NOT_LOGIN:         //设备未登录成功
                    show_warn("IP_NET_DVR_ControlPlay() fail, err: ERR_DEV_NOT_LOGIN\n");
                    break;
                case ERR_OUTOFF_MEMORY:         //内存不足
                    show_warn("IP_NET_DVR_ControlPlay() fail, err: ERR_OUTOFF_MEMORY\n");
                    break;
                case ERR_PLAY_ACTION_ERROR:     //Action错误
                    show_warn("IP_NET_DVR_ControlPlay() fail, err: ERR_PLAY_ACTION_ERROR\n");
                    break;
                }
            }

            memset(strm_info, 0, sizeof(tps_rec_strm_t));
            nmp_del(strm_info, tps_rec_strm_t, 1);
            break;
    }

    return 0;
}
static int tps_stream_seek(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    tps_service_t *tps_srv;
    NMP_ASSERT(owner && strm_info);

    tps_srv = (tps_service_t*)owner;

    if (TPS_LOGOUT == tps_get_user_id(&tps_srv->parm))
    {
         show_warn("tps device is disconnected..\n");
    }
    else
    {
        nmp_mutex_lock(tps_srv->rtsp.lock);
        ret = tps_process_stream(tps_srv, strm_info, CMD_STREAM_SEEK);
        nmp_mutex_unlock(tps_srv->rtsp.lock);
    }

    return ret;
}
static int tps_stream_ctrl(void *owner, int channel, int level, int cmd, void *value)
{
    int ret = -1;
    tps_service_t *tps_srv;
    NMP_ASSERT(owner);

    tps_srv = (tps_service_t*)owner;

    if (TPS_LOGOUT == tps_get_user_id(&tps_srv->parm))
    {
        show_warn("tps device is disconnected.\n");
    }
    else
    {
        switch (cmd)
        {
            case DEF_STOP_ALL_STREAM_CMD:
                tps_cleanup_stream_info(&tps_srv->rtsp);
                ret = 0;
                break;
                
            default:
                break;
        }
    }

    return ret;
}

stream_operation_t tps_strm_opt = 
{
    tps_stream_init,
    tps_stream_open,
    tps_stream_play,
    tps_stream_pause,
    tps_stream_close,
    tps_stream_seek,
    tps_stream_ctrl,
};


