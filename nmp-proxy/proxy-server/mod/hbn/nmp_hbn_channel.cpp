
#include <string.h>
#include <sys/time.h>

#include "nmp_sdk.h"

#include "stream_api.h"
#include "rtsp-server.h"
#include "standard_frame.h"

#include "nmp_hbn_service.h"
#include "nmp_hbn_channel.h"


static __inline__ int 
hbn_get_start_time(const char *pri, HB_NET_TIME *time)
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
        time->dwSecond = 0;//ts.second;
    }
    /*printf("start:%04d/%02d/%02d-%02d:%02d:%02d\n", 
        (int)time->dwYear, (int)time->dwMonth, (int)time->dwDay, 
        (int)time->dwHour, (int)time->dwMinute, (int)time->dwSecond);*/
    return ret;
}

static __inline__ int 
hbn_get_end_time(const char *pri, HB_NET_TIME *time)
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
        time->dwSecond = 0;//ts.second;
    }
    /*printf("end:%04d/%02d/%02d-%02d:%02d:%02d\n", 
        (int)time->dwYear, (int)time->dwMonth, (int)time->dwDay, 
        (int)time->dwHour, (int)time->dwMinute, (int)time->dwSecond);*/
    return ret;
}

static __inline__ int 
hbn_time_compare(HB_NET_TIME *time_base, HB_NET_TIME *time)
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
hbn_make_key_frame(LONG user_id, int channel, int level)
{
    if (0 == level)
    {
        if (FALSE == HB_NET_MakeKeyFrame(user_id, channel, 0))  // 强制关键帧只有在播数据才有效
        {
            show_warn("HB_NET_MakeKeyFrame failed.\n");
        }
    }
    else if (0 < level)
    {
        if (FALSE == HB_NET_MakeKeyFrame(user_id, channel, 1))
        { 
            show_warn("HB_NET_MakeKeyFrame failed.\n");
        }
    }
}

static __inline__ stream_info_t *
hbn_find_record_stream_by_channel_and_time(stream_list_t *strm_list, 
    int channel, HB_NET_TIME *start_time, HB_NET_TIME *stop_time)
{
    nmp_list_t *list;
    hbn_rec_strm_t *tmp_strm;
    stream_info_t *rec_strm = NULL;

    NMP_ASSERT(strm_list);

    list = nmp_list_first(strm_list->list);
    while (list)
    {
        tmp_strm = (hbn_rec_strm_t*)nmp_list_data(list);
        if (!tmp_strm->strm_base.opened                          && 
            (channel == tmp_strm->strm_base.channel)             && 
            !hbn_time_compare(&tmp_strm->start_time, start_time) &&
            !hbn_time_compare(&tmp_strm->start_time, stop_time)) 
        {// 找到一个未使用过的录像流, 并返回
            rec_strm = (stream_info_t*)tmp_strm;
            break;
        }
        list = nmp_list_next(list);
    }

    return rec_strm;
}

static void 
hbn_real_data_callback(long handle, HB_NET_STREAMCALLBACKDATA *strm_data)
{
    //struct timeval ts;
    stream_info_t *strm_info;
    NMP_ASSERT(HBN_INVALID_HANDLE != handle);
    NMP_ASSERT(strm_data);

    strm_info = (stream_info_t*)strm_data->pContext;

    if (HB_NET_SYSHEAD == strm_data->dwDataType)
    {
        if (!strm_info->media_size)
        {
            memcpy(strm_info->media_info, strm_data->pFrame, 
                strm_data->dwDataSize);
            strm_info->media_size = strm_data->dwDataSize;
        }
        return ;
    }
    else if (HB_NET_STREAMDATA == strm_data->dwDataType)
    {
        if (strm_info->opened)
        {
            /*if (0 == strm_info->ts)
            {
                gettimeofday(&ts, NULL);
                strm_info->ts = ts.tv_sec*1000 + ts.tv_usec/1000;
            }
            else
            {
                strm_info->ts += DEF_TIMESTAMP;
            }*/
            //show_debug("channel: %d, len: %d\n", strm_info->channel, (int)strm_data->dwDataSize);
            write_stream_data((gpointer)strm_info->opened, 
                (gulong)strm_info->ts, (gulong)DEF_TIMESTAMP, 
                (guint8*)strm_data->pFrame, (gsize)strm_data->dwDataSize, 
                (guint8)strm_data->dwDataType, (guint8*)NULL, (gsize)0, (guint8)0);
        }
    }
    else
        show_warn("Unknown data!!!!!\n");

    return ;
}

static void  
hbn_record_data_callback(long handle, HB_NET_STREAMCALLBACKDATA *strm_data)
{
    stream_info_t *strm_info;
    hbn_rec_strm_t *rec_strm;

    NMP_ASSERT(HBN_INVALID_HANDLE != handle);
    NMP_ASSERT(strm_data);

    strm_info = (stream_info_t*)strm_data->pContext;
    rec_strm  = (hbn_rec_strm_t*)strm_info;

    if (HB_NET_SYSHEAD == strm_data->dwDataType)
    {
        if (!strm_info->opened && !strm_info->media_size)
        {
            //printf("HB_NET_SYSHEAD: %s, %d<========================\n\n\n", 
              //  strm_data->pFrame, (int)strm_data->dwDataSize);
            memcpy(strm_info->media_info, strm_data->pFrame, 
                strm_data->dwDataSize);
            strm_info->media_size = strm_data->dwDataSize;
        }
    }
    else if (HB_NET_STREAMDATA == strm_data->dwDataType)
    {
        if (!strm_data->dwDataSize)
            return ;

        while (1)
        {
            if (strm_info->opened && rec_strm->enable)
            {
                if (!test_stream_blockable((gpointer)strm_info->opened, 
                        (gsize)strm_data->dwDataSize))
                {//printf("----------->%d\n", (int)strm_data->dwDataSize);
                    write_stream_data((gpointer)strm_info->opened, 
                        (gulong)0, (gulong)0, (guint8*)strm_data->pFrame, 
                        (gsize)strm_data->dwDataSize, (guint8)strm_data->dwDataType, 
                        (guint8*)NULL, (gsize)0, (guint8)0);
                    break;
                }
                else
                {
                    show_debug("Sleep some times.\n");
                    usleep(RECORD_PACKET_SLEEP_TIME);
                }
            }
            else
                break;
        }
    }

    return ;
}

static __inline__ int 
hbn_get_stream_header(hbn_rtsp_t *rtsp, char *real_header, 
    size_t header_size)
{
    int len = 0;
    if (rtsp->real_header.size)
    {
        if (header_size >= rtsp->real_header.size)
        {
            memcpy(real_header, rtsp->real_header.head, 
                rtsp->real_header.size);
            len = rtsp->real_header.size;
        }
    }

    return len;
}

static __inline__ int 
hbn_set_stream_header(hbn_rtsp_t *rtsp, char *real_header, 
    size_t header_size)
{
    nmp_mutex_lock(rtsp->lock);
    if (header_size <= sizeof(rtsp->real_header.head))
    {
        memcpy(rtsp->real_header.head, real_header, header_size);
        rtsp->real_header.size = header_size;
    }
    nmp_mutex_unlock(rtsp->lock);

    return 0;
}

static __inline__ int 
hbn_init_real_stream(hbn_service_t *hbn_srv, stream_info_t *strm_info)
{
    int ret, len, offset = 0;
    int handle = HBN_INVALID_HANDLE;
    HB_NET_CLIENTINFO client_info;

    //printf("S Iiiiiiiiiiiiinit, ch: %d\n", strm_info->channel);
    nmp_mutex_lock(hbn_srv->rtsp.lock);
    //printf("E Iiiiiiiiiiiiinit, ch: %d\n", strm_info->channel);
    len = hbn_get_stream_header(&hbn_srv->rtsp, strm_info->media_info, 
            sizeof(strm_info->media_info));
    nmp_mutex_unlock(hbn_srv->rtsp.lock);
    if (len)
    {
        strm_info->media_size = len;
        return 0;
    }

    if (J_SDK_NVR == hbn_srv->owner->fastenings.dev_type)
        ;//offset = HB_NVR_CHANNEL_OFFSET;

    memset(&client_info, 0, sizeof(HB_NET_CLIENTINFO));
    client_info.dwSize      = sizeof(HB_NET_CLIENTINFO);
    client_info.lChannel    = strm_info->channel + offset;
    client_info.lStreamType = strm_info->level;
    client_info.lLinkMode   = HB_NET_TCP;      //0: TCP, 1: UDP
    client_info.pContext    = strm_info;
    client_info.pfnCallback = hbn_real_data_callback;

    handle = HB_NET_RealPlay((LONG)hbn_get_user_id(&hbn_srv->parm), 
                &client_info);
    if (HBN_INVALID_HANDLE != handle)
    {
        HB_NET_StopRealPlay(handle);

        if (strm_info->media_size)
        {
            ret = 0;
            hbn_set_stream_header(&hbn_srv->rtsp, strm_info->media_info, 
                strm_info->media_size);
        }
        else
        {
            ret = -1;
            show_warn("stream header is not get.\n");
        }
    }
    else
    {
        ret = -1;
        show_warn("HB_NET_RealPlay(Error:%d)<-----------------\n", 
            (int)HB_NET_GetLastError());
    }

   return ret;
}

static __inline__ int 
hbn_init_record_stream(hbn_service_t *hbn_srv, stream_info_t *strm_info)
{
    int ret = -1, offset = 0;
    LONG user_id, handle = HBN_INVALID_HANDLE;
    stream_info_t *rec_strm;

    HB_NET_TIME start_time = {0};
    HB_NET_TIME stop_time  = {0};

    HB_NET_PLAYBACKCOND play_back_cond;

    nmp_mutex_lock(hbn_srv->rtsp.lock);
    rec_strm = hbn_find_record_stream_by_channel_and_time(
                &hbn_srv->rtsp.rec_strm_list, 
                strm_info->channel, &start_time, &stop_time);
    if (rec_strm)
    {
        show_warn("stream [channel:%d] [level:%d] already opened!!\n", 
            strm_info->channel, strm_info->level);
        memcpy(strm_info->media_info, rec_strm->media_info, 
            rec_strm->media_size);
        strm_info->media_size = rec_strm->media_size;
        nmp_mutex_unlock(hbn_srv->rtsp.lock);
        return 0;
    }
    nmp_mutex_unlock(hbn_srv->rtsp.lock);

    if (J_SDK_NVR == hbn_srv->owner->fastenings.dev_type)
        ;//offset = HB_NVR_CHANNEL_OFFSET;

    memset(&play_back_cond, 0, sizeof(HB_NET_PLAYBACKCOND));
    play_back_cond.dwSize      = sizeof(HB_NET_PLAYBACKCOND);
    play_back_cond.dwChannel   = strm_info->channel + offset;
    play_back_cond.pfnDataProc = hbn_record_data_callback;
    play_back_cond.pContext    = (void*)strm_info;
    play_back_cond.dwType      = 0xff;

    hbn_get_start_time((const char*)strm_info->pri_data, 
        &play_back_cond.struStartTime);
    hbn_get_end_time((const char*)strm_info->pri_data, 
        &play_back_cond.struStopTime);

    user_id = hbn_get_user_id(&hbn_srv->parm);
    handle = HB_NET_PlayBack(user_id, &play_back_cond);
    if (HBN_INVALID_HANDLE != handle)
    {
        HB_NET_StopPlayBack(handle);
        ret = 0;
    }
    else
        show_warn("HB_NET_PlayBack(%d) failure!!!!!!!\n", 
            (int)HB_NET_GetLastError());

    return ret;
}
static __inline__ int 
hbn_open_real_stream(hbn_service_t *hbn_srv, stream_info_t *strm_info)
{
    int ret = -1;
    stream_info_t *real_strm;

    strm_info->media_size = hbn_get_stream_header(&hbn_srv->rtsp, 
                                strm_info->media_info, 
                                sizeof(strm_info->media_info));
    if (strm_info->media_size)
    {
        real_strm = (stream_info_t*)nmp_new0(hbn_real_strm_t, 1);
        real_strm->handle  = HBN_INVALID_HANDLE;
        real_strm->opened  = strm_info->opened;
        real_strm->type    = strm_info->type;
        real_strm->level   = strm_info->level;
        real_strm->channel = strm_info->channel;
        real_strm->state   = STATS_STREAM_STOP;
        real_strm->sdk_srv = (struct service*)hbn_srv;

        memcpy(real_strm->media_info, strm_info->media_info, 
            strm_info->media_size);
        real_strm->media_size = strm_info->media_size;

        set_stream_user_data((gpointer)strm_info->opened, (void*)real_strm);
        add_one_stream(&hbn_srv->rtsp.real_strm_list, real_strm);
        ret = 0;
    }

    return ret;
}

static __inline__ int 
hbn_open_record_stream(hbn_service_t *hbn_srv, stream_info_t *strm_info)
{
    int ret = -1, offset = 0;
    LONG user_id, handle = HBN_INVALID_HANDLE;
    stream_info_t *rec_strm;

    HB_NET_TIME start_time = {0};
    HB_NET_TIME stop_time  = {0};

    HB_NET_PLAYBACKCOND play_back_cond;
    memset(&play_back_cond, 0, sizeof(HB_NET_PLAYBACKCOND));

    if (J_SDK_NVR == hbn_srv->owner->fastenings.dev_type)
        ;//offset = HB_NVR_CHANNEL_OFFSET;

    play_back_cond.dwSize      = sizeof(HB_NET_PLAYBACKCOND);
    play_back_cond.dwChannel   = strm_info->channel + offset;
    play_back_cond.pfnDataProc = hbn_record_data_callback;
    play_back_cond.pContext    = (void*)strm_info;
    play_back_cond.dwType      = 0xff;

    hbn_get_start_time((const char*)strm_info->pri_data, &start_time);
    hbn_get_end_time((const char*)strm_info->pri_data, &stop_time);
    memcpy(&play_back_cond.struStartTime, &start_time, sizeof(HB_NET_TIME));
    memcpy(&play_back_cond.struStopTime, &stop_time, sizeof(HB_NET_TIME));

    if (!hbn_find_record_stream_by_channel_and_time(&hbn_srv->rtsp.rec_strm_list, 
            strm_info->channel, &start_time, &stop_time))
    {
        user_id = hbn_get_user_id(&hbn_srv->parm);
        handle = HB_NET_PlayBack(user_id, &play_back_cond);
        if (HBN_INVALID_HANDLE != handle)
        {
            HB_NET_StopPlayBack(handle);
            ret = 0;
        }
        else
            show_warn("HB_NET_PlayBack(%d) failure!!!!!!!!!!\n", 
                (int)HB_NET_GetLastError());
    }
    else
        show_warn("stream [channel:%d] [level:%d] already opened!!\n", 
            strm_info->channel, strm_info->level);

    if (!ret)
    {
        rec_strm = (stream_info_t*)nmp_new0(hbn_rec_strm_t, 1);
        rec_strm->handle  = HBN_INVALID_HANDLE;
        rec_strm->opened  = strm_info->opened;
        rec_strm->type    = strm_info->type;
        rec_strm->level   = strm_info->level;
        rec_strm->channel = strm_info->channel;
        rec_strm->state   = STATS_STREAM_STOP;
        rec_strm->sdk_srv = (struct service*)hbn_srv;

        memcpy(rec_strm->media_info, strm_info->media_info, 
            strm_info->media_size);
        rec_strm->media_size = strm_info->media_size;

        ((hbn_rec_strm_t*)rec_strm)->enable = 0;

        memcpy(&((hbn_rec_strm_t*)rec_strm)->start_time, 
            &start_time, sizeof(HB_NET_TIME));
        memcpy(&((hbn_rec_strm_t*)rec_strm)->stop_time, 
            &stop_time, sizeof(HB_NET_TIME));

        set_stream_user_data((gpointer)rec_strm->opened, (void*)rec_strm);
        add_one_stream(&hbn_srv->rtsp.rec_strm_list, rec_strm);
    }

    return ret;
}

stream_info_t *
hbn_find_have_opened_stream(stream_list_t *strm_list, 
    int channel, int level)
{
    nmp_list_t *list;
    stream_info_t *strm_node;
    stream_info_t *strm_info = NULL;

    NMP_ASSERT(strm_list);

    list = nmp_list_first(strm_list->list);
    while (list)
    {
        strm_node = (stream_info_t*)nmp_list_data(list);
        if ((channel == strm_node->channel) && 
            (level == strm_node->level))
        {
            //show_debug("<<<<<<<<<<<<<<<<<<<<<<<<<<<<MMMMMMMMMMMMMMMMMMMMMM\n");
            if (HBN_INVALID_HANDLE != strm_node->handle)
            {
                strm_info = strm_node;
                break;
            }
        }

        list = nmp_list_next(list);
    }

    return strm_info;
}

static __inline__ int 
hbn_process_stream(hbn_service_t *hbn_srv, stream_info_t *strm_info, 
    stream_cmd_t cmd, int type)
{
    int ret = -1, offset = 0, flag = -1;
    int user_id, handle = HBN_INVALID_HANDLE;

    user_id = hbn_get_user_id(&hbn_srv->parm);

    switch(cmd)
    {
    case CMD_STREAM_PLAY:
        if (STREAM_REAL == type)
        {
            if (J_SDK_NVR == hbn_srv->owner->fastenings.dev_type)
                ;//offset = HB_NVR_CHANNEL_OFFSET;

            if (hbn_find_have_opened_stream(&hbn_srv->rtsp.real_strm_list, 
                strm_info->channel, strm_info->level))
            {
                show_debug("\n\n#######################"
                    "hbn_find_have_opened_stream####################33\n\n");
                ret = 0;
                break;
            }

            HB_NET_CLIENTINFO client_info;

            nmp_mutex_lock(hbn_srv->rtsp.lock);
            if (STATS_STREAM_PLAYING != strm_info->state)
            {
                flag = 0;
                client_info.dwSize      = sizeof(HB_NET_CLIENTINFO);
                client_info.lChannel    = strm_info->channel + offset;
                client_info.lStreamType = strm_info->level;
                client_info.lLinkMode   = HB_NET_TCP;      //0: TCP, 1: UDP
                client_info.pContext    = strm_info;
                client_info.pfnCallback = hbn_real_data_callback;
                memset(client_info.sMultiCastIP, 0, sizeof(HB_IP_LEN));
            }
            nmp_mutex_unlock(hbn_srv->rtsp.lock);

            if (!flag)
            {
                printf("S Playyyyyyyyyyyyyyyyy, ch: %d\n", strm_info->channel);
                handle = HB_NET_RealPlay((LONG)user_id, &client_info);
                printf("E Playyyyyyyyyyyyyyyyy, ch: %d\n", strm_info->channel);
                if (HBN_INVALID_HANDLE != handle)
                {
                    ret = 0;
                    hbn_make_key_frame((LONG)user_id, strm_info->channel + 
                        offset, strm_info->level);

                    nmp_mutex_lock(hbn_srv->rtsp.lock);
                    strm_info->state = STATS_STREAM_PLAYING;
                    strm_info->handle = handle;
                    nmp_mutex_unlock(hbn_srv->rtsp.lock);
                }
                else
                    show_warn("HB_NET_RealPlay(Error:%d)<-----------------\n", 
                        (int)HB_NET_GetLastError());
            }
            else
            {
                hbn_make_key_frame((LONG)user_id, strm_info->channel + offset, 
                    strm_info->level);
            }
        }
        else if (STREAM_VOD == type || STREAM_DOWNLOAD == type)
        {
            HB_NET_PLAYBACKCOND play_back_cond;

            nmp_mutex_lock(hbn_srv->rtsp.lock);
            if (STATS_STREAM_PLAYING != strm_info->state)
            {
                flag = 0;
                play_back_cond.dwSize      = sizeof(HB_NET_PLAYBACKCOND);
                play_back_cond.dwChannel   = strm_info->channel + offset;
                play_back_cond.pfnDataProc = hbn_record_data_callback;
                play_back_cond.pContext    = (void*)strm_info;
                play_back_cond.dwType      = 0xff;
                memcpy(&play_back_cond.struStartTime, 
                    &((hbn_rec_strm_t*)strm_info)->start_time, sizeof(HB_NET_TIME));
                memcpy(&play_back_cond.struStopTime, 
                    &((hbn_rec_strm_t*)strm_info)->stop_time, sizeof(HB_NET_TIME));
            }
            else    // 连续PLAY两次，不响应
            {
                //
            }
            nmp_mutex_unlock(hbn_srv->rtsp.lock);

            if (!flag)
            {
                handle = HB_NET_PlayBack((LONG)user_id, &play_back_cond);
                if (HBN_INVALID_HANDLE != handle)
                {
                    nmp_mutex_lock(hbn_srv->rtsp.lock);
                    strm_info->handle = handle;
                    strm_info->state = STATS_STREAM_PLAYING;
                    ((hbn_rec_strm_t*)strm_info)->enable = 1;
                    nmp_mutex_unlock(hbn_srv->rtsp.lock);
                }
            }
        }
        break;

    case CMD_STREAM_PAUSE:
        if (STREAM_REAL == type)
        {
        }
        else if (STREAM_VOD == type || STREAM_DOWNLOAD == type)
        {
            nmp_mutex_lock(hbn_srv->rtsp.lock);
            if (STATS_STREAM_PLAYING == strm_info->state && strm_info->handle)
            {
                strm_info->state = STATS_STREAM_PAUSE;
                ((hbn_rec_strm_t*)strm_info)->enable = 0;
            }
            nmp_mutex_unlock(hbn_srv->rtsp.lock);
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

static int 
hbn_stream_init(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    hbn_service_t *hbn_srv;
    NMP_ASSERT(owner && strm_info);

    hbn_srv = (hbn_service_t*)owner;
    if (HBN_LOGOUT == hbn_get_user_id(&hbn_srv->parm))
    {
        show_warn("hbn device is disconnected.\n");
    }
    else
    {
        switch (strm_info->type)
        {
            case STREAM_REAL://0 播放实时流
                ret = hbn_init_real_stream(hbn_srv, strm_info);
                strm_info->length = 0;
                break;
            case STREAM_VOD:      //2点播录像文件
            case STREAM_DOWNLOAD: //3下载录像文件
                ret = hbn_init_record_stream(hbn_srv, strm_info);
                strm_info->length = 0;
                break;

            default:
                break;
        }
    }

    return ret;
}
static int 
hbn_stream_open(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    hbn_service_t *hbn_srv;
    NMP_ASSERT(owner && strm_info);

    hbn_srv = (hbn_service_t*)owner;

    if (HBN_LOGOUT == hbn_get_user_id(&hbn_srv->parm))
    {
         show_warn("hbn device is disconnected..\n");
    }
    else
    {
        nmp_mutex_lock(hbn_srv->rtsp.lock);
        switch (strm_info->type)
        {
            case STREAM_REAL:               // 播放实时流
                ret = hbn_open_real_stream(hbn_srv, strm_info);
                break;
            case STREAM_VOD:                // 点播录像文件
            case STREAM_DOWNLOAD:           // 下载录像文件
                ret = hbn_open_record_stream(hbn_srv, strm_info);
                break;

            default:
                break;
        }
        nmp_mutex_unlock(hbn_srv->rtsp.lock);
    }

    return ret;
}
static int 
hbn_stream_play(void *owner, stream_info_t *strm_info)
{
    int type, ret = -1;
    hbn_service_t *hbn_srv;
    NMP_ASSERT(owner && strm_info);

    hbn_srv = (hbn_service_t*)owner;

    if (HBN_LOGOUT == hbn_get_user_id(&hbn_srv->parm))
    {
         show_warn("hbn device is disconnected..\n");
    }
    else
    {
        nmp_mutex_lock(hbn_srv->rtsp.lock);
        type = strm_info->type;
        nmp_mutex_unlock(hbn_srv->rtsp.lock);

        ret = hbn_process_stream(hbn_srv, strm_info, CMD_STREAM_PLAY, type);
    }

    return ret;
}
static int 
hbn_stream_pause(void *owner, stream_info_t *strm_info)
{
    int type, ret = -1;
    hbn_service_t *hbn_srv;
    NMP_ASSERT(owner && strm_info);

    hbn_srv = (hbn_service_t*)owner;

    if (HBN_LOGOUT == hbn_get_user_id(&hbn_srv->parm))
    {
         show_warn("hbn device is disconnected..\n");
    }
    else
    {
        nmp_mutex_lock(hbn_srv->rtsp.lock);
        type = strm_info->type;
        nmp_mutex_unlock(hbn_srv->rtsp.lock);
        
        ret = hbn_process_stream(hbn_srv, strm_info, CMD_STREAM_PAUSE, type);
    }

    return ret;
}
static int 
hbn_stream_close(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    int handle = HBN_INVALID_HANDLE;
    hbn_service_t *hbn_srv;
    NMP_ASSERT(owner && strm_info);

    hbn_srv = (hbn_service_t*)owner;

    switch (strm_info->type)
    {
        case STREAM_REAL:
            nmp_mutex_lock(hbn_srv->rtsp.lock);
            handle = strm_info->handle;
            strm_info->handle = HBN_INVALID_HANDLE;
            remove_one_stream(&hbn_srv->rtsp.real_strm_list, strm_info);
            nmp_mutex_unlock(hbn_srv->rtsp.lock);

            if (HBN_INVALID_HANDLE != handle)
            {
                printf("Before StopRealPlay<---------------\n");
                if (HB_NET_StopRealPlay(handle))
                {
                printf("After  StopRealPlay<---------------\n");
                    ret = 0;
                }
                else
                    show_warn("HB_NET_StopRealPlay(%d)\n", 
                        (int)HB_NET_GetLastError());
            }

            memset(strm_info, 0, sizeof(hbn_real_strm_t));
            nmp_del(strm_info, hbn_real_strm_t, 1);
            break;

        case STREAM_VOD:
        case STREAM_DOWNLOAD:
            nmp_mutex_lock(hbn_srv->rtsp.lock);
            handle = strm_info->handle;
            strm_info->handle = HBN_INVALID_HANDLE;
            ((hbn_rec_strm_t*)strm_info)->enable = 0;
            remove_one_stream(&hbn_srv->rtsp.rec_strm_list, strm_info);
            nmp_mutex_unlock(hbn_srv->rtsp.lock);

            if (HBN_INVALID_HANDLE != handle)
            {
                if (HB_NET_StopPlayBack(handle))
                    ret = 0;
                else
                    show_warn("HB_NET_StopPlayBack(%d)\n", 
                        (int)HB_NET_GetLastError());
            }

            memset(strm_info, 0, sizeof(hbn_rec_strm_t));
            nmp_del(strm_info, hbn_rec_strm_t, 1);
            break;
    }

    return 0;
}
static int 
hbn_stream_seek(void *owner, stream_info_t *strm_info)
{
    int type, ret = -1;
    hbn_service_t *hbn_srv;
    NMP_ASSERT(owner && strm_info);

    hbn_srv = (hbn_service_t*)owner;

    if (HBN_LOGOUT == hbn_get_user_id(&hbn_srv->parm))
    {
         show_warn("hbn device is disconnected..\n");
    }
    else
    {
        nmp_mutex_lock(hbn_srv->rtsp.lock);
        type = strm_info->type;
        nmp_mutex_unlock(hbn_srv->rtsp.lock);

        ret = hbn_process_stream(hbn_srv, strm_info, CMD_STREAM_SEEK, type);
    }

    return ret;
}
static int 
hbn_stream_ctrl(void *owner, int channel, int level, int cmd, void *value)
{
    int ret = -1;
    hbn_service_t *hbn_srv;
    NMP_ASSERT(owner);

    hbn_srv = (hbn_service_t*)owner;

    if (HBN_LOGOUT == hbn_get_user_id(&hbn_srv->parm))
    {
        show_warn("hbn device is disconnected.\n");
    }
    else
    {
        switch (cmd)
        {
            case DEF_STOP_ALL_STREAM_CMD:
                hbn_cleanup_stream_info(&hbn_srv->rtsp);
                ret = 0;
                break;
                
            default:
                break;
        }
    }

    return ret;
}

const stream_operation_t hbn_strm_opt = 
{
    hbn_stream_init,
    hbn_stream_open,
    hbn_stream_play,
    hbn_stream_pause,
    hbn_stream_close,
    hbn_stream_seek,
    hbn_stream_ctrl,
};



