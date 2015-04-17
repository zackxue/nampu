
#include <string.h>
#include <sys/time.h>

#include "nmp_sdk.h"

#include "stream_api.h"
#include "rtsp-server.h"
#include "standard_frame.h"

#include "nmp_hik_service.h"
#include "nmp_hik_channel.h"



static __inline__ int 
hik_get_start_time(const char *pri, NET_DVR_TIME *time)
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
hik_get_end_time(const char *pri, NET_DVR_TIME *time)
{
    int ret;
    JTime ts;

    ret = get_end_time(pri, &ts);
    if (!ret)
    {
        time->dwYear   = ts.year + J_SDK_DEF_BASE_YEAR;;
        time->dwMonth  = ts.month;
        time->dwDay    = ts.date;
        time->dwHour   = ts.hour;
        time->dwMinute = ts.minute;
        time->dwSecond = ts.second;
    }

    return ret;
}

static __inline__ int 
hik_time_compare(NET_DVR_TIME *time_base, NET_DVR_TIME *time)
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
hik_make_key_frame(LONG user_id, int channel, int level)
{
    if (0 == level)
    {
        if (FALSE == NET_DVR_MakeKeyFrame(user_id, channel))  // 强制关键帧只有在播数据才有效
        {
            show_warn("NET_DVR_MakeKeyFrame failed.\n");
        }
    }
    else if (0 < level)
    {
        if (FALSE == NET_DVR_MakeKeyFrameSub(user_id, channel))
        { 
            show_warn("NET_DVR_MakeKeyFrameSub failed.\n");
        }
    }
}

static __inline__ stream_info_t *
hik_find_record_stream_by_channel_and_time(stream_list_t *strm_list, 
    int channel, NET_DVR_TIME *start_time, NET_DVR_TIME *stop_time)
{
    nmp_list_t *list;
    hik_rec_strm_t *tmp_strm;
    stream_info_t *rec_strm = NULL;

    NMP_ASSERT(strm_list);

    list = nmp_list_first(strm_list->list);
    while (list)
    {
        tmp_strm = (hik_rec_strm_t*)nmp_list_data(list);
        if (!tmp_strm->strm_base.opened                          && 
            (channel == tmp_strm->strm_base.channel)             && 
            !hik_time_compare(&tmp_strm->start_time, start_time) &&
            !hik_time_compare(&tmp_strm->start_time, stop_time)) 
        {// 找到一个未使用过的录像流, 并返回
            rec_strm = (stream_info_t*)tmp_strm;
            break;
        }
        list = nmp_list_next(list);
    }

    return rec_strm;
}

static void 
hik_real_data_callback_v30(LONG handle, DWORD data_type, 
        BYTE *buffer, DWORD buf_size, void *user)
{//show_debug(">>>>>>>>>>>>>>>>>>>>>>%d:%d\n", data_type, buf_size);
    struct timeval ts;
    stream_info_t *strm_info;

    NMP_ASSERT(HIK_INVALID_HANDLE != handle);
    NMP_ASSERT(buffer && user);

    strm_info = (stream_info_t*)user;
    if (NET_DVR_SYSHEAD == data_type)
    {
        if (!strm_info->media_size)
        {
            memcpy(strm_info->media_info, buffer, buf_size);
            strm_info->media_size = buf_size;
            
            show_debug("\nsignal cond after real data callback.[size: %d]\n", 
                strm_info->media_size);
            proxy_cond_signal(((hik_real_strm_t*)strm_info)->cond);
        }
    }
    else
    {
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
            return ;
        }
    }

    return ;
}

static void 
hik_record_data_callback_v30(LONG handle, DWORD data_type,
        BYTE  *buffer, DWORD buf_size, DWORD user)
{//show_debug(">>>>>>>>>>>>>>>>>>>>>>%d:%d\n", data_type, buf_size);
    stream_info_t *strm_info;
    
    NMP_ASSERT(HIK_INVALID_HANDLE != handle);
    NMP_ASSERT(buffer && user);

    strm_info = (stream_info_t*)user;
    if (data_type == NET_DVR_SYSHEAD)
    {
        if (!strm_info->opened && !strm_info->media_size)
        {
            memcpy(strm_info->media_info, buffer, buf_size);
            strm_info->media_size = buf_size;
        }
    }
    else
    {
        while (1)
        {
            if (strm_info->opened)
            {
                if (!test_stream_blockable((gpointer)strm_info->opened, (gsize)buf_size))
                {
                    write_stream_data((gpointer)strm_info->opened, 
                        (gulong)0, (gulong)0, (guint8*)buffer, (gsize)buf_size, 
                        (guint8)data_type, (guint8*)NULL, (gsize)0, (guint8)0);
                    break;
                }
                else
                {
                    //show_debug("Sleep some times.\n");
                    usleep(RECORD_PACKET_SLEEP_TIME);
                }
            }
            else
                break;
        }
    }

    return ;
}

static int hik_get_download_percent(void *data)
{
    int percent;
    stream_info_t *strm_info;

    NMP_ASSERT(data);

    strm_info = (stream_info_t*)data;
    percent = NET_DVR_GetDownloadPos((LONG)strm_info->handle);

    switch (percent)
    {
        case 100:
            show_info(">>>>>>>>>>>>>>>>>>percent: %d\n", percent);
            write_stream_data((gpointer)strm_info->opened, 
                (gulong)0, (gulong)0, (guint8*)NULL, (gsize)0, 
                (guint8)15, (guint8*)NULL, (gsize)0, (guint8)0);
            nmp_del_timer(((hik_rec_strm_t*)strm_info)->pos_timer);
            ((hik_rec_strm_t*)strm_info)->pos_timer = NULL;
            break;
        case 200:
            show_info(">>>>>>>>>>>>>>>>>>percent: %d\n", percent);
            break;

        default:
            break;
    }

    return 0;
}

static __inline__ int 
hik_init_real_stream(hik_service_t *hik_srv, stream_info_t *strm_info)
{
    int ret = -1, offset = 0;
    int handle = HIK_INVALID_HANDLE;

    stream_info_t *real_strm;
    NET_DVR_CLIENTINFO client_info = {0};

    nmp_mutex_lock(hik_srv->rtsp.lock);
    real_strm = find_stream_by_channel_and_level(&hik_srv->rtsp.real_strm_list, 
                    strm_info->channel, strm_info->level);
    if (real_strm)
    {
        memcpy(strm_info->media_info, real_strm->media_info, real_strm->media_size);
        strm_info->media_size = real_strm->media_size;
        nmp_mutex_unlock(hik_srv->rtsp.lock);
        return 0;
    }
    nmp_mutex_unlock(hik_srv->rtsp.lock);

    if (J_SDK_NVR == hik_srv->owner->fastenings.dev_type)
        offset = HIK_NVR_CHANNEL_OFFSET;

    proxy_cond_new(&((hik_real_strm_t*)strm_info)->cond);

    client_info.lChannel     = strm_info->channel + offset;
    client_info.lLinkMode    = !strm_info->level ? 0 : 0x80000000;
    client_info.sMultiCastIP = NULL;

    handle = NET_DVR_RealPlay_V30((LONG)hik_get_user_id(&hik_srv->parm), 
                &client_info, hik_real_data_callback_v30, (void*)strm_info);
    if (HIK_INVALID_HANDLE != handle)
    {
        proxy_cond_timed_wait(((hik_real_strm_t*)strm_info)->cond, 
            MAX_INIT_WAITE_TIMES);
        NET_DVR_StopRealPlay(handle);

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
        show_warn("NET_DVR_RealPlay_V30(%d)\n", NET_DVR_GetLastError());
        ret = -1;
    }
    proxy_cond_free(((hik_real_strm_t*)strm_info)->cond);

    return ret;
}

static __inline__ int 
hik_init_record_stream(hik_service_t *hik_srv, stream_info_t *strm_info)
{
    int flag, ret = -1;
    int find, offset = 0;
    int handle = HIK_INVALID_HANDLE;

    HWND play_rect = {0};//!!!!must init.

    NET_DVR_FILECOND     find_cond;
    NET_DVR_FINDDATA_V30 find_data;

    memset(&find_cond, 0, sizeof(NET_DVR_FILECOND));
    memset(&find_data, 0, sizeof(NET_DVR_FINDDATA_V30));

    if (J_SDK_NVR == hik_srv->owner->fastenings.dev_type)
        offset = HIK_NVR_CHANNEL_OFFSET;

    find_cond.lChannel = strm_info->channel + offset;
    find_cond.dwFileType = 0xff;
    hik_get_start_time((const char*)strm_info->pri_data, &find_cond.struStartTime);
    hik_get_end_time((const char*)strm_info->pri_data, &find_cond.struStopTime);

    if (find_cond.struStopTime.dwYear > 2037)
        find_cond.struStopTime.dwYear = 2037;

    find = NET_DVR_FindFile_V30((LONG)hik_get_user_id(&hik_srv->parm), &find_cond);
    if(HIK_INVALID_HANDLE != find)
    {
        while (1)
        {
            flag = NET_DVR_FindNextFile_V30(find, &find_data);
            if (NET_DVR_ISFINDING == flag)
                continue;
            else if (NET_DVR_FILE_SUCCESS == flag)
            {
                handle = NET_DVR_PlayBackByName((LONG)hik_get_user_id(&hik_srv->parm), 
                            find_data.sFileName, play_rect);
                break;
            }
            else
            {
                switch (flag)
                {
                    case NET_DVR_FILE_NOFIND:
                        show_debug("NET_DVR_FILE_NOFIND\n");
                        break;
                    case NET_DVR_NOMOREFILE:
                        show_debug("NET_DVR_NOMOREFILE\n");
                        break;
                    case NET_DVR_FILE_EXCEPTION:
                        show_debug("NET_DVR_FILE_EXCEPTION\n");
                        break;
                }
                break;
            }
        }

        if (HIK_INVALID_HANDLE != handle)
        {
            if (NET_DVR_SetPlayDataCallBack(handle, 
                hik_record_data_callback_v30, (DWORD)strm_info))
            {
                if (NET_DVR_PlayBackControl(handle, NET_DVR_PLAYSTART, 0, NULL))
                {
                    NET_DVR_StopPlayBack(handle);
                    
                    if (strm_info->media_size)
                        ret = 0;
                    else
                        show_warn("stream header is not get.\n");
                }
                else
                {
                    show_warn("NET_DVR_PLAYSTART (%d)\n", NET_DVR_GetLastError());
                }
            }
            else
            {
                show_warn("NET_DVR_SetPlayDataCallBack (%d)\n", NET_DVR_GetLastError());
            }
        }
        else
        {
            show_warn("HIK_INVALID_HANDLE<<<<<\n");
        }
    }
    else
    {
        show_warn("NET_DVR_FindFile_V30(%d), FileType: %d\n", 
            NET_DVR_GetLastError(), find_cond.dwFileType);
    }

    return ret;
}
static __inline__ int 
hik_open_real_stream(hik_service_t *hik_srv, stream_info_t *strm_info)
{
    int ret = -1, offset = 0;
    int handle = HIK_INVALID_HANDLE;
    stream_info_t *real_strm;
    NET_DVR_CLIENTINFO client_info = {0};

    if (!find_stream_by_channel_and_level(&hik_srv->rtsp.real_strm_list, 
            strm_info->channel, strm_info->level))
    {
        proxy_cond_new(&((hik_real_strm_t*)strm_info)->cond);

        if (J_SDK_NVR == hik_srv->owner->fastenings.dev_type)
            offset = HIK_NVR_CHANNEL_OFFSET;

        client_info.lChannel     = strm_info->channel + offset;
        client_info.lLinkMode    = (strm_info->level) == 0 ? 0 : 0x80000000;
        client_info.sMultiCastIP = NULL;

        handle = NET_DVR_RealPlay_V30((LONG)hik_get_user_id(&hik_srv->parm), 
                    &client_info, hik_real_data_callback_v30, (void*)strm_info); 
        if (HIK_INVALID_HANDLE != handle)
        {
            proxy_cond_timed_wait(((hik_real_strm_t*)strm_info)->cond, MAX_INIT_WAITE_TIMES);
            NET_DVR_StopRealPlay(handle);

            if (strm_info->media_size)
            {
                ret = 0;
            }
            else
                show_warn("stream header is not get.\n");
        }
        else
            show_warn("NET_DVR_RealPlay_V30(%d)\n", NET_DVR_GetLastError());

        proxy_cond_free(((hik_real_strm_t*)strm_info)->cond);
    }
    else
    {
        show_warn("stream [channel:%d] [level:%d] already opened!!\n", 
            strm_info->channel, strm_info->level);
    }

    if (!ret)
    {
        real_strm = (stream_info_t*)nmp_new0(hik_real_strm_t, 1);
        real_strm->handle  = HIK_INVALID_HANDLE;
        real_strm->opened  = strm_info->opened;
        real_strm->type    = strm_info->type;
        real_strm->level   = strm_info->level;
        real_strm->channel = strm_info->channel;
        real_strm->state   = STATS_STREAM_STOP;
        real_strm->sdk_srv = (struct service*)hik_srv;//proxy_device_ref(hik_srv->owner);

        memcpy(real_strm->media_info, strm_info->media_info, strm_info->media_size);
        real_strm->media_size = strm_info->media_size;

        set_stream_user_data((gpointer)strm_info->opened, (void*)real_strm);
        add_one_stream(&hik_srv->rtsp.real_strm_list, real_strm);
    }

    return ret;
}

static __inline__ int 
hik_open_record_stream(hik_service_t *hik_srv, stream_info_t *strm_info)
{
    int flag = -1, ret = -1;
    int find, offset = 0;
    int handle = HIK_INVALID_HANDLE;
    stream_info_t *rec_strm;

    HWND play_rect          = {0};//!!!!must init.
    NET_DVR_TIME start_time = {0};
    NET_DVR_TIME stop_time  = {0};

    NET_DVR_FILECOND     find_cond;
    NET_DVR_FINDDATA_V30 find_data;

    memset(&find_cond, 0, sizeof(NET_DVR_FILECOND));
    memset(&find_data, 0, sizeof(NET_DVR_FINDDATA_V30));

    if (J_SDK_NVR == hik_srv->owner->fastenings.dev_type)
        offset = HIK_NVR_CHANNEL_OFFSET;

    hik_get_start_time((const char*)strm_info->pri_data, &start_time);
    hik_get_end_time((const char*)strm_info->pri_data, &stop_time);

    find_cond.lChannel = strm_info->channel + offset;
    find_cond.dwFileType = 0xff;
    memcpy(&find_cond.struStartTime, &start_time, sizeof(NET_DVR_TIME));
    memcpy(&find_cond.struStopTime, &stop_time, sizeof(NET_DVR_TIME));

    if (find_cond.struStopTime.dwYear > 2037)
        find_cond.struStopTime.dwYear = 2037;

    // 如果多个客户端打开相同的录像文件时，通道和时间都一样的，必须加其他字段来标识不一样的流
    if (!hik_find_record_stream_by_channel_and_time(
                    &hik_srv->rtsp.rec_strm_list, 
                    strm_info->channel, 
                    &start_time, &stop_time))
    {
        find = NET_DVR_FindFile_V30((LONG)hik_get_user_id(&hik_srv->parm), &find_cond);
        if(HIK_INVALID_HANDLE != find)
        {
            while (1)
            {
                flag = NET_DVR_FindNextFile_V30(find, &find_data);
                if (NET_DVR_ISFINDING == flag)
                    continue;
                else if (NET_DVR_FILE_SUCCESS == flag)
                {
                    handle = NET_DVR_PlayBackByName((LONG)hik_get_user_id(&hik_srv->parm), 
                                find_data.sFileName, play_rect);
                    break;
                }
                else//NET_DVR_FILE_NOFIND, NET_DVR_NOMOREFILE, NET_DVR_FILE_EXCEPTION
                    break;
            }

            if (HIK_INVALID_HANDLE != handle)
            {
                rec_strm = (stream_info_t*)nmp_new0(hik_rec_strm_t, 1);
                if (NET_DVR_SetPlayDataCallBack(handle, 
                        hik_record_data_callback_v30, (DWORD)rec_strm))
                {
                    ret = 0;
                }
                else
                    nmp_del(rec_strm, hik_rec_strm_t, 1);
            }
            else
                show_warn("NET_DVR_PlayBackByName(%d)\n", NET_DVR_GetLastError());
        }
        else
        {
            show_warn("NET_DVR_FindFile_V30(%d), FileType: %d\n", 
                NET_DVR_GetLastError(), find_cond.dwFileType);
        }
    }
    else
        show_warn("stream [channel:%d] [level:%d] already opened!!\n", 
            strm_info->channel, strm_info->level);

    if (!ret)
    {
        rec_strm->handle  = handle;
        rec_strm->opened  = strm_info->opened;
        rec_strm->type    = strm_info->type;
        rec_strm->level   = strm_info->level;
        rec_strm->channel = strm_info->channel;
        rec_strm->state   = STATS_STREAM_STOP;
        rec_strm->sdk_srv = (struct service*)hik_srv;

        memcpy(&((hik_rec_strm_t*)rec_strm)->start_time, 
            &start_time, sizeof(NET_DVR_TIME));
        memcpy(&((hik_rec_strm_t*)rec_strm)->stop_time, 
            &stop_time, sizeof(NET_DVR_TIME));

        ((hik_rec_strm_t*)rec_strm)->pos_timer = 
            nmp_set_timer(1000, hik_get_download_percent, rec_strm);

        set_stream_user_data((gpointer)rec_strm->opened, (void*)rec_strm);
        add_one_stream(&hik_srv->rtsp.rec_strm_list, rec_strm);
    }

    return ret;
}

static __inline__ int 
hik_process_stream(hik_service_t *hik_srv, stream_info_t *strm_info, stream_cmd_t cmd)
{
    int ret = -1;
    int handle, offset = 0;

    switch(cmd)
    {
    case CMD_STREAM_PLAY:
        if (STREAM_REAL == strm_info->type)
        {
            if (J_SDK_NVR == hik_srv->owner->fastenings.dev_type)
                offset = HIK_NVR_CHANNEL_OFFSET;

            if (STATS_STREAM_PLAYING != strm_info->state)
            {
                NET_DVR_CLIENTINFO client_info = {0};

                client_info.lChannel     = strm_info->channel + offset;
                client_info.lLinkMode    = strm_info->level == 0 ? 0 : 0x80000000;
                client_info.sMultiCastIP = NULL;

                handle = NET_DVR_RealPlay_V30((LONG)hik_get_user_id(&hik_srv->parm), 
                            &client_info, hik_real_data_callback_v30, (void*)strm_info);
                if (HIK_INVALID_HANDLE != handle)
                {
                    strm_info->state = STATS_STREAM_PLAYING;
                    strm_info->handle = handle;
                    ret = 0;
                }
                else
                    show_warn("NET_DVR_RealPlay_V30 (%d)\n", handle);
            }
            else
            {
                hik_make_key_frame((LONG)hik_get_user_id(&hik_srv->parm), 
                    strm_info->channel + offset, strm_info->level);
            }
        }
        else if (STREAM_VOD == strm_info->type || 
                 STREAM_DOWNLOAD == strm_info->type)
        {
            if (STATS_STREAM_PAUSE == strm_info->state && HIK_INVALID_HANDLE != strm_info->handle)
            {
                if (NET_DVR_PlayBackControl(strm_info->handle, NET_DVR_PLAYRESTART, 0, NULL))
                {
                    strm_info->state = STATS_STREAM_PLAYING;
                    ret = 0;
                }
                else
                    show_warn("NET_DVR_PLAYRESTART (%d)\n", NET_DVR_GetLastError());
            }
            else if (STATS_STREAM_STOP == strm_info->state && HIK_INVALID_HANDLE != strm_info->handle)
            {
                show_info("Open store stream...      \n");
                if (NET_DVR_PlayBackControl(strm_info->handle, NET_DVR_PLAYSTART, 0, NULL))
                {
                    strm_info->state = STATS_STREAM_PLAYING;
                    if (NET_DVR_PlayBackControl(strm_info->handle, NET_DVR_PLAYSTARTAUDIO, 0, NULL))// 打开声音
                    {
                        ret = 0;
                    }
                    else
                        show_warn("NET_DVR_PLAYSTARTAUDIO (%d)\n", NET_DVR_GetLastError());
                }
                else
                    show_warn("NET_DVR_PLAYSTART (%d)\n", NET_DVR_GetLastError());
            }
            else    // 连续PLAY两次，不响应
            {
                //
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
                HIK_INVALID_HANDLE != strm_info->handle)
            {
                if(NET_DVR_PlayBackControl(strm_info->handle, NET_DVR_PLAYPAUSE, 0, NULL))
                {
                    ret = 0;
                    strm_info->state = STATS_STREAM_PAUSE;
                }
                else
                {
                    show_warn("NET_DVR_PLAYPAUSE (%d)\n", NET_DVR_GetLastError());
                }
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

static int hik_stream_init(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    hik_service_t *hik_srv;
    hik_real_strm_t real_strm;
    hik_rec_strm_t  rec_strm;
    NMP_ASSERT(owner && strm_info);

    hik_srv = (hik_service_t*)owner;

    memset(&real_strm, 0, sizeof(real_strm));
    memset(&rec_strm, 0, sizeof(rec_strm));

    if (HIK_LOGOUT == hik_get_user_id(&hik_srv->parm))
    {
         show_warn("hik device is disconnected..\n");
    }
    else
    {
        strm_info->channel += 1;           // 转化为海康的通道号
        switch (strm_info->type)
        {
            case STREAM_REAL:               // 播放实时流
                memcpy(&real_strm, strm_info, sizeof(stream_info_t));
                
                ret = hik_init_real_stream(hik_srv, (stream_info_t*)&real_strm);
                memcpy(strm_info, &real_strm, sizeof(stream_info_t));
                strm_info->length = 0;

                break;
            case STREAM_VOD:                //2点播录像文件
            case STREAM_DOWNLOAD:           //3下载录像文件
                memcpy(&rec_strm, strm_info, sizeof(stream_info_t));
                
                ret = hik_init_record_stream(hik_srv, (stream_info_t*)&rec_strm);
                memcpy(strm_info, &rec_strm, sizeof(stream_info_t));
                
                break;
                
            default:
                break;
        }
    }

    return ret;
}
static int hik_stream_open(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    hik_service_t *hik_srv;
    NMP_ASSERT(owner && strm_info);

    hik_srv = (hik_service_t*)owner;

    if (HIK_LOGOUT == hik_get_user_id(&hik_srv->parm))
    {
         show_warn("hik device is disconnected..\n");
    }
    else
    {
        strm_info->channel += 1;           // 转化为海康的通道号
        nmp_mutex_lock(hik_srv->rtsp.lock);
        switch (strm_info->type)
        {
            case STREAM_REAL:               // 播放实时流
                ret = hik_open_real_stream(hik_srv, strm_info);
                break;
            case STREAM_VOD:                // 点播录像文件
            case STREAM_DOWNLOAD:           // 下载录像文件
                ret = hik_open_record_stream(hik_srv, strm_info);
                break;
                
            default:
                break;
        }
        nmp_mutex_unlock(hik_srv->rtsp.lock);
    }

    return ret;
}
static int hik_stream_play(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    hik_service_t *hik_srv;
    NMP_ASSERT(owner && strm_info);

    hik_srv = (hik_service_t*)owner;

    if (HIK_LOGOUT == hik_get_user_id(&hik_srv->parm))
    {
         show_warn("hik device is disconnected..\n");
    }
    else
    {
        nmp_mutex_lock(hik_srv->rtsp.lock);
        ret = hik_process_stream(hik_srv, strm_info, CMD_STREAM_PLAY);
        nmp_mutex_unlock(hik_srv->rtsp.lock);
    }

    return ret;
}
static int hik_stream_pause(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    hik_service_t *hik_srv;
    NMP_ASSERT(owner && strm_info);

    hik_srv = (hik_service_t*)owner;

    if (HIK_LOGOUT == hik_get_user_id(&hik_srv->parm))
    {
         show_warn("hik device is disconnected..\n");
    }
    else
    {
        nmp_mutex_lock(hik_srv->rtsp.lock);
        ret = hik_process_stream(hik_srv, strm_info, CMD_STREAM_PAUSE);
        nmp_mutex_unlock(hik_srv->rtsp.lock);
    }

    return ret;
}
static int hik_stream_close(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    int handle = HIK_INVALID_HANDLE;
    void *timer;
    hik_service_t *hik_srv;
    NMP_ASSERT(owner && strm_info);

    hik_srv = (hik_service_t*)owner;

    switch (strm_info->type)
    {
        case STREAM_REAL:
            nmp_mutex_lock(hik_srv->rtsp.lock);
            handle = strm_info->handle;
            strm_info->handle = HIK_INVALID_HANDLE;
            remove_one_stream(&hik_srv->rtsp.real_strm_list, strm_info);
            nmp_mutex_unlock(hik_srv->rtsp.lock);

            if (HIK_INVALID_HANDLE != handle)
            {
                if (NET_DVR_StopRealPlay(handle))
                {
                    ret = 0; 
                }
                else
                    show_warn("NET_DVR_StopRealPlay(%d)\n", 
                        NET_DVR_GetLastError());
            }

            memset(strm_info, 0, sizeof(hik_real_strm_t));
            nmp_del(strm_info, hik_real_strm_t, 1);
            break;

        case STREAM_VOD:
        case STREAM_DOWNLOAD:
            nmp_mutex_lock(hik_srv->rtsp.lock);
            handle = strm_info->handle;
            timer  = ((hik_rec_strm_t*)strm_info)->pos_timer;
            strm_info->handle = HIK_INVALID_HANDLE;
            ((hik_rec_strm_t*)strm_info)->pos_timer = NULL;
            remove_one_stream(&hik_srv->rtsp.rec_strm_list, strm_info);
            nmp_mutex_unlock(hik_srv->rtsp.lock);

            if (HIK_INVALID_HANDLE != handle)
            {
                printf("NET_DVR_GetDownloadPos(): %d<========\n", 
                    NET_DVR_GetDownloadPos((LONG)handle));

                if (timer)
                    nmp_del_timer(timer);

                if (NET_DVR_StopPlayBack(handle))
                    ret = 0;
                else
                    show_warn("NET_DVR_StopPlayBack(%d)\n", 
                        NET_DVR_GetLastError());
            }

            memset(strm_info, 0, sizeof(hik_rec_strm_t));
            nmp_del(strm_info, hik_rec_strm_t, 1);
            break;
    }

    return 0;
}
static int hik_stream_seek(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    hik_service_t *hik_srv;
    NMP_ASSERT(owner && strm_info);

    hik_srv = (hik_service_t*)owner;

    if (HIK_LOGOUT == hik_get_user_id(&hik_srv->parm))
    {
         show_warn("hik device is disconnected..\n");
    }
    else
    {
        nmp_mutex_lock(hik_srv->rtsp.lock);
        ret = hik_process_stream(hik_srv, strm_info, CMD_STREAM_SEEK);
        nmp_mutex_unlock(hik_srv->rtsp.lock);
    }

    return ret;
}
static int hik_stream_ctrl(void *owner, int channel, int level, int cmd, void *value)
{
    int ret = -1;
    hik_service_t *hik_srv;
    NMP_ASSERT(owner);

    hik_srv = (hik_service_t*)owner;

    if (HIK_LOGOUT == hik_get_user_id(&hik_srv->parm))
    {
        show_warn("hik device is disconnected.\n");
    }
    else
    {
        switch (cmd)
        {
            case DEF_STOP_ALL_STREAM_CMD:
                hik_cleanup_stream_info(&hik_srv->rtsp);
                ret = 0;
                break;

            default:
                break;
        }
    }

    return ret;
}

stream_operation_t hik_strm_opt = 
{
    hik_stream_init,
    hik_stream_open,
    hik_stream_play,
    hik_stream_pause,
    hik_stream_close,
    hik_stream_seek,
    hik_stream_ctrl,
};


