
#include <string.h>
#include <sys/time.h>

#include "nmp_sdk.h"

#include "stream_api.h"
#include "rtsp-server.h"
#include "standard_frame.h"

#include "j_hie_service.h"
#include "j_hie_channel.h"


static __inline__ int 
hie_get_start_time(const char *pri, TimeInfo *time)
{
    int ret;
    JTime ts;

    ret = get_start_time(pri, &ts);
    if (!ret)
    {
        time->wYear   = ts.year + J_SDK_DEF_BASE_YEAR;
        time->wMonth  = ts.month;
        time->wDay    = ts.date;
        time->wHour   = ts.hour;
        time->wMinute = ts.minute;
        time->wSecond = 0;//ts.second;
    }
    /*printf("start:%04d/%02d/%02d-%02d:%02d:%02d\n", 
        (int)time->wYear, (int)time->wMonth, (int)time->wDay, 
        (int)time->wHour, (int)time->wMinute, (int)time->wSecond);*/
    return ret;
}

static __inline__ int 
hie_get_end_time(const char *pri, TimeInfo *time)
{
    int ret;
    JTime ts;

    ret = get_end_time(pri, &ts);
    if (!ret)
    {
        time->wYear   = ts.year + J_SDK_DEF_BASE_YEAR;
        time->wMonth  = ts.month;
        time->wDay    = ts.date;
        time->wHour   = ts.hour;
        time->wMinute = ts.minute;
        time->wSecond = 0;//ts.second;
    }
    /*printf("end:%04d/%02d/%02d-%02d:%02d:%02d\n", 
        (int)time->wYear, (int)time->wMonth, (int)time->wDay, 
        (int)time->wHour, (int)time->wMinute, (int)time->wSecond);*/
    return ret;
}

static __inline__ int 
hie_time_compare(TimeInfo *time_base, TimeInfo *time)
{
    if (time_base->wYear   == time->wYear   && 
        time_base->wMonth  == time->wMonth  && 
        time_base->wDay    == time->wDay    && 
        time_base->wHour   == time->wHour   && 
        time_base->wMinute == time->wMinute && 
        time_base->wSecond == time->wSecond)
    {
        return 0;
    }

    return -1;
}

static __inline__ void 
hie_make_key_frame(HUSER user_id, int channel, int level)
{// 强制关键帧只有在播数据才有效
    int err;

    err = HieClient_ForceIFrame(user_id, channel);
    if (err)
    { 
        show_warn("HB_NET_MakeKeyFrame failed, err: %d.\n", err);
    }
}

static __inline__ stream_info_t *
hie_find_record_stream_by_channel_and_time(stream_list_t *strm_list, 
    int channel, TimeInfo *start_time, TimeInfo *stop_time)
{
    nmp_list_t *list;
    hie_rec_strm_t *tmp_strm;
    stream_info_t *rec_strm = NULL;

    NMP_ASSERT(strm_list);

    list = nmp_list_first(strm_list->list);
    while (list)
    {
        tmp_strm = (hie_rec_strm_t*)nmp_list_data(list);
        if (!tmp_strm->strm_base.opened                          && 
            (channel == tmp_strm->strm_base.channel)             && 
            !hie_time_compare(&tmp_strm->start_time, start_time) &&
            !hie_time_compare(&tmp_strm->start_time, stop_time)) 
        {// 找到一个未使用过的录像流, 并返回
            rec_strm = (stream_info_t*)tmp_strm;
            break;
        }
        list = nmp_list_next(list);
    }

    return rec_strm;
}

static int 
hie_real_data_callback(HSTREAM handle, const StreamMediaFrame *strm_data, 
    DWORD user_data)
{
    stream_info_t *strm_info;
    NMP_ASSERT(HIE_INVALID_HANDLE != handle);
    NMP_ASSERT(user_data);

    strm_info = (stream_info_t*)user_data;

    if (eFrameSysHeader == strm_data->dwFrameType)
    {
        if (!strm_info->media_info)
        {
            memcpy(strm_info->media_info, strm_data->cFrameBuffer.pBuffer, 
                strm_data->cFrameBuffer.dwBufLen);
            strm_info->media_size = strm_data->cFrameBuffer.dwBufLen;
        }
        return 0;
    }
    else
    {
        if (strm_info->opened)
        {
            show_debug("channel: %d, len: %d\n", strm_info->channel, 
                (int)strm_data->cFrameBuffer.dwBufLen);
            write_stream_data((gpointer)strm_info->opened, 
                (gulong)strm_info->ts, (gulong)DEF_TIMESTAMP, 
                (guint8*)strm_data->cFrameBuffer.pBuffer, 
                (gsize)strm_data->cFrameBuffer.dwBufLen, 
                (guint8)strm_data->dwFrameType, (guint8*)NULL, 
                (gsize)0, (guint8)0);
        }
    }

    return 0;
}

static int 
hie_record_data_callback(HSTREAM handle, const StreamMediaFrame *strm_data, 
    DWORD user_data)
{
    stream_info_t *strm_info;
    hie_rec_strm_t *rec_strm;

    NMP_ASSERT(HIE_INVALID_HANDLE != handle);
    NMP_ASSERT(strm_data);
    NMP_ASSERT(user_data);

    strm_info = (stream_info_t*)user_data;
    rec_strm  = (hie_rec_strm_t*)strm_info;

    if (eFrameSysHeader == strm_data->dwFrameType)
    {
        if (!strm_info->opened && !strm_info->media_size)
        {
            memcpy(strm_info->media_info, strm_data->cFrameBuffer.pBuffer, 
                strm_data->cFrameBuffer.dwBufLen);
            strm_info->media_size = strm_data->cFrameBuffer.dwBufLen;
        }
    }
    else
    {
        if (!strm_data->cFrameBuffer.pBuffer)
            return 0;

        while (1)
        {
            if (strm_info->opened && rec_strm->enable)
            {
                if (!test_stream_blockable((gpointer)strm_info->opened, 
                        (gsize)strm_data->cFrameBuffer.dwBufLen))
                {//printf("----------->%d\n", (int)strm_data->dwDataSize);
                    show_debug("channel: %d, len: %d\n", strm_info->channel, 
                        (int)strm_data->cFrameBuffer.dwBufLen);
                    write_stream_data((gpointer)strm_info->opened, 
                        (gulong)strm_info->ts, (gulong)DEF_TIMESTAMP, 
                        (guint8*)strm_data->cFrameBuffer.pBuffer, 
                        (gsize)strm_data->cFrameBuffer.dwBufLen, 
                        (guint8)strm_data->dwFrameType, (guint8*)NULL, 
                        (gsize)0, (guint8)0);
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

    return 0;
}

static __inline__ int 
hie_get_stream_header(hie_rtsp_t *rtsp, char *real_header, 
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
hie_set_stream_header(hie_rtsp_t *rtsp, char *real_header, 
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
hie_init_real_stream(hie_service_t *hie_srv, stream_info_t *strm_info)
{
    int ret, len, offset = 0;
    
    HSTREAM user_id, handle = HIE_INVALID_HANDLE;
    RealStreamPara rsp;               //实时流连接参数    

    if (J_SDK_NVR == hie_srv->owner->fastenings.dev_type)
        ;//offset = HB_NVR_CHANNEL_OFFSET;

    nmp_mutex_lock(hie_srv->rtsp.lock);
    len = hie_get_stream_header(&hie_srv->rtsp, strm_info->media_info, 
            sizeof(strm_info->media_info));
    nmp_mutex_unlock(hie_srv->rtsp.lock);
    if (len)
    {
        strm_info->media_size = len;
        return 0;
    }

    //配置实时流连接参数
    memset(&rsp, 0, sizeof(RealStreamPara));
    rsp.dwChannel = strm_info->channel + offset;                   //通道 1
    rsp.eMediaType = strm_info->level ? eAssistVideo : eMainVideoAndSound;  //主码流音视频
    rsp.eTransferMode = eGeneralTCP;         //普通 TCP 

    user_id = hie_get_user_id(&hie_srv->parm);

    //建立实时流连接
    if (!(ret = HieClient_RealStreamConnect(&handle, user_id, &rsp)))
    {
        //设置流媒体数据回调
        if (!(ret = HieClient_StreamMediaCB(handle, hie_real_data_callback, (DWORD)strm_info)))
        {
            //启动实时流
            if (!(ret = HieClient_StreamMediaControl(handle, eTaskStart)))
            {
                show_debug("HieClient_StreamMediaControl Success!\n");
                hie_set_stream_header(&hie_srv->rtsp, strm_info->media_info, 
                    strm_info->media_size);

                HieClient_StreamMediaCB(handle, NULL, 0);
                HieClient_StreamMediaControl(handle, eTaskStop);
            }
            else
            {
                HieClient_StreamMediaCB(handle, NULL, 0);
                show_debug("HieClient_StreamMediaControl failure, err: %d!!\n", ret);
            }
        }
        else
            show_debug("HieClient_StreamMediaCB failure, err: %d!!\n", ret);

        HieClient_RealStreamDisconnect(handle);
    }
    else
        show_debug("HieClient_RealStreamConnect failure, err: %d!!\n", ret);

   return ret;
}

static __inline__ int 
hie_init_record_stream(hie_service_t *hie_srv, stream_info_t *strm_info)
{
    int ret = -1, offset = 0;
    stream_info_t *rec_strm;

    HSTREAM user_id, handle = HIE_INVALID_HANDLE;

    TimeInfo start_time = {0};
    TimeInfo stop_time  = {0};

    HistoryStreamPara hsp;               //历史流连接参数

    nmp_mutex_lock(hie_srv->rtsp.lock);
    rec_strm = hie_find_record_stream_by_channel_and_time(
                &hie_srv->rtsp.rec_strm_list, 
                strm_info->channel, &start_time, &stop_time);
    if (rec_strm)
    {
        show_warn("stream [channel:%d] [level:%d] already opened!!\n", 
            strm_info->channel, strm_info->level);
        memcpy(strm_info->media_info, rec_strm->media_info, 
            rec_strm->media_size);
        strm_info->media_size = rec_strm->media_size;
        nmp_mutex_unlock(hie_srv->rtsp.lock);
        return 0;
    }
    nmp_mutex_unlock(hie_srv->rtsp.lock);

    if (J_SDK_NVR == hie_srv->owner->fastenings.dev_type)
        ;//offset = HB_NVR_CHANNEL_OFFSET;

    //配置历史流连接参数(HieClient_HistoryStreamCreate 仅支持单一录像类型或者所有录像类型)
    memset(&hsp, 0, sizeof(HistoryStreamPara));
    hsp.dwDiskGroup     = 1;               //盘组1
    hsp.dwChannel       = strm_info->channel + offset;              //通道16
    hsp.eTransferMode   = eGeneralTCP;     //普通的TCP 
    hsp.eStreamType     = eAllStreamMedia; //所有录像类型
    hsp.dwEnableEndTime = 1;               //结束时间有效

    hie_get_start_time((const char*)strm_info->pri_data, 
        &hsp.cBeginTime);
    hie_get_end_time((const char*)strm_info->pri_data, 
        &hsp.cEndTime);

    user_id = hie_get_user_id(&hie_srv->parm);

    //创建历史流通道(HieClient_HistoryStreamCreate 仅支持单一录像类型或者所有录像类型)
    if (!(ret = HieClient_HistoryStreamCreate(&handle, user_id, &hsp, 0)))
    {
        //设置流媒体数据回调
        if (!(ret = HieClient_StreamMediaCB(handle, hie_record_data_callback, 0)))
        {
            //启动历史流
            if (!(ret = HieClient_StreamMediaControl(handle, eTaskStart)))
            {
                show_debug("HieClient_StreamMediaControl Success!\n");
                //删除流媒体数据回调
                HieClient_StreamMediaCB(handle, NULL, 0);
                //停止历史流
                HieClient_StreamMediaControl(handle, eTaskStop);
            }
            else
            {
                //删除流媒体数据回调
                HieClient_StreamMediaCB(handle, NULL, 0);
                show_warn("HieClient_StreamMediaControl failure, err: %d!!\n", ret);
            }
        }
        else
            show_warn("HieClient_StreamMediaCB failure, err: %d!!\n", ret);
        //销毁历史流通道
        HieClient_HistoryStreamDestroy(handle);
    }
    else
        show_warn("HieClient_HistoryStreamCreate failure, err: %d!!\n", ret);

    return ret;
}

static __inline__ int 
hie_open_real_stream(hie_service_t *hie_srv, stream_info_t *strm_info)
{
    int ret = -1;
    stream_info_t *real_strm;

    strm_info->media_size = hie_get_stream_header(&hie_srv->rtsp, 
                                strm_info->media_info, 
                                sizeof(strm_info->media_info));
    if (strm_info->media_size)
    {
        real_strm = (stream_info_t*)nmp_new0(hie_real_strm_t, 1);
        real_strm->handle  = HIE_INVALID_HANDLE;
        real_strm->opened  = strm_info->opened;
        real_strm->type    = strm_info->type;
        real_strm->level   = strm_info->level;
        real_strm->channel = strm_info->channel;
        real_strm->state   = STATS_STREAM_STOP;
        real_strm->sdk_srv = (struct service*)hie_srv;

        memcpy(real_strm->media_info, strm_info->media_info, 
            strm_info->media_size);
        real_strm->media_size = strm_info->media_size;

        set_stream_user_data((gpointer)strm_info->opened, (void*)real_strm);
        add_one_stream(&hie_srv->rtsp.real_strm_list, real_strm);
        ret = 0;
    }

    return ret;
}

static __inline__ int 
hie_open_record_stream(hie_service_t *hie_srv, stream_info_t *strm_info)
{
    int ret = -1, offset = 0;
    stream_info_t *rec_strm;

    TimeInfo start_time = {0};
    TimeInfo stop_time  = {0};

    HistoryStreamPara hsp;               //历史流连接参数


    if (J_SDK_NVR == hie_srv->owner->fastenings.dev_type)
        ;//offset = HB_NVR_CHANNEL_OFFSET;

    //配置历史流连接参数(HieClient_HistoryStreamCreate 仅支持单一录像类型或者所有录像类型)
    memset(&hsp, 0, sizeof(HistoryStreamPara));
    hsp.dwDiskGroup     = 1;               //盘组1
    hsp.dwChannel       = strm_info->channel + offset;              //通道16
    hsp.eTransferMode   = eGeneralTCP;     //普通的TCP 
    hsp.eStreamType     = eAllStreamMedia; //所有录像类型
    hsp.dwEnableEndTime = 1;               //结束时间有效

    hie_get_start_time((const char*)strm_info->pri_data, 
        &hsp.cBeginTime);
    hie_get_end_time((const char*)strm_info->pri_data, 
        &hsp.cEndTime);

    if (!hie_find_record_stream_by_channel_and_time(&hie_srv->rtsp.rec_strm_list, 
            strm_info->channel, &start_time, &stop_time))
    {
        HSTREAM user_id, handle = HIE_INVALID_HANDLE;
        user_id = hie_get_user_id(&hie_srv->parm);

        //创建历史流通道(HieClient_HistoryStreamCreate 仅支持单一录像类型或者所有录像类型)
        if (!(ret = HieClient_HistoryStreamCreate(&handle, user_id, &hsp, 0)))
        {
            //设置流媒体数据回调
            if (!(ret = HieClient_StreamMediaCB(handle, hie_record_data_callback, 0)))
            {
                //启动历史流
                if (!(ret = HieClient_StreamMediaControl(handle, eTaskStart)))
                {
                    show_debug("HieClient_StreamMediaControl Success!\n");
                    //删除流媒体数据回调
                    HieClient_StreamMediaCB(handle, NULL, 0);
                    //停止历史流
                    HieClient_StreamMediaControl(handle, eTaskStop);
                }
                else
                {
                    //删除流媒体数据回调
                    HieClient_StreamMediaCB(handle, NULL, 0);
                    show_warn("HieClient_StreamMediaControl failure, err: %d!!\n", ret);
                }
            }
            else
                show_warn("HieClient_StreamMediaCB failure, err: %d!!\n", ret);
            //销毁历史流通道
            HieClient_HistoryStreamDestroy(handle);
        }
        else
            show_warn("HieClient_HistoryStreamCreate failure, err: %d!!\n", ret);
    }
    else
        show_warn("stream [channel:%d] [level:%d] already opened!!\n", 
            strm_info->channel, strm_info->level);

    if (!ret)
    {
        rec_strm = (stream_info_t*)nmp_new0(hie_rec_strm_t, 1);
        rec_strm->handle  = HIE_INVALID_HANDLE;
        rec_strm->opened  = strm_info->opened;
        rec_strm->type    = strm_info->type;
        rec_strm->level   = strm_info->level;
        rec_strm->channel = strm_info->channel;
        rec_strm->state   = STATS_STREAM_STOP;
        rec_strm->sdk_srv = (struct service*)hie_srv;

        memcpy(rec_strm->media_info, strm_info->media_info, 
            strm_info->media_size);
        rec_strm->media_size = strm_info->media_size;

        ((hie_rec_strm_t*)rec_strm)->enable = 0;

        memcpy(&((hie_rec_strm_t*)rec_strm)->start_time, 
            &start_time, sizeof(TimeInfo));
        memcpy(&((hie_rec_strm_t*)rec_strm)->stop_time, 
            &stop_time, sizeof(TimeInfo));

        set_stream_user_data((gpointer)rec_strm->opened, (void*)rec_strm);
        add_one_stream(&hie_srv->rtsp.rec_strm_list, rec_strm);
    }

    return ret;
}

static __inline__ int 
hie_process_stream(hie_service_t *hie_srv, stream_info_t *strm_info, 
    stream_cmd_t cmd, int type)
{
    int ret = -1, offset = 0, flag = -1;
    HSTREAM user_id, handle = HIE_INVALID_HANDLE;

    user_id = hie_get_user_id(&hie_srv->parm);

    switch(cmd)
    {
    case CMD_STREAM_PLAY:
        if (STREAM_REAL == type)
        {
            if (J_SDK_NVR == hie_srv->owner->fastenings.dev_type)
                ;//offset = HB_NVR_CHANNEL_OFFSET;

            RealStreamPara rsp;               //实时流连接参数    

            nmp_mutex_lock(hie_srv->rtsp.lock);
            if (STATS_STREAM_PLAYING != strm_info->state)
            {
                flag = 0;
                memset(&rsp, 0, sizeof(RealStreamPara));
                rsp.dwChannel = strm_info->channel + offset;                   //通道 1
                rsp.eMediaType = strm_info->level ? eAssistVideo : eMainVideoAndSound;  //主码流音视频
                rsp.eTransferMode = eGeneralTCP;         //普通 TCP 
            }
            nmp_mutex_unlock(hie_srv->rtsp.lock);

            if (!flag)
            {
                //建立实时流连接
                if (!(ret = HieClient_RealStreamConnect(&handle, user_id, &rsp)))
                {
                    //设置流媒体数据回调
                    if (!(ret = HieClient_StreamMediaCB(handle, hie_real_data_callback, (DWORD)strm_info)))
                    {
                        //启动实时流
                        if (!(ret = HieClient_StreamMediaControl(handle, eTaskStart)))
                        {
                            show_debug("HieClient_StreamMediaControl Success!\n");
                            hie_make_key_frame(user_id, strm_info->channel + 
                                offset, strm_info->level);

                            nmp_mutex_lock(hie_srv->rtsp.lock);
                            strm_info->state = STATS_STREAM_PLAYING;
                            strm_info->handle = (int)handle;
                            nmp_mutex_unlock(hie_srv->rtsp.lock);
                        }
                        else
                        {
                            HieClient_StreamMediaCB(handle, NULL, 0);
                            show_debug("HieClient_StreamMediaControl failure, err: %d!!\n", ret);
                        }
                    }
                    else
                        show_debug("HieClient_StreamMediaCB failure, err: %d!!\n", ret);

                    HieClient_RealStreamDisconnect(handle);
                }
                else
                    show_debug("HieClient_RealStreamConnect failure, err: %d!!\n", ret);
            }
            else
            {
                hie_make_key_frame(user_id, strm_info->channel + offset, 
                    strm_info->level);
            }
        }
        else if (STREAM_VOD == type || STREAM_DOWNLOAD == type)
        {
            HistoryStreamPara hsp;               //历史流连接参数

            nmp_mutex_lock(hie_srv->rtsp.lock);
            if (STATS_STREAM_PLAYING != strm_info->state)
            {
                flag = 0;
                //配置历史流连接参数(HieClient_HistoryStreamCreate 仅支持单一录像类型或者所有录像类型)
                memset(&hsp, 0, sizeof(HistoryStreamPara));
                hsp.dwDiskGroup     = 1;               //盘组1
                hsp.dwChannel       = strm_info->channel + offset; //通道16
                hsp.eTransferMode   = eGeneralTCP;     //普通的TCP 
                hsp.eStreamType     = eAllStreamMedia; //所有录像类型
                hsp.dwEnableEndTime = 1;               //结束时间有效

                memcpy(&hsp.cBeginTime, &((hie_rec_strm_t*)strm_info)
                    ->start_time, sizeof(TimeInfo));
                memcpy(&hsp.cEndTime, &((hie_rec_strm_t*)strm_info)
                    ->stop_time, sizeof(TimeInfo));
            }
            else    // 连续PLAY两次，不响应
            {
                //
            }
            nmp_mutex_unlock(hie_srv->rtsp.lock);

            if (!flag)
            {
                //创建历史流通道(HieClient_HistoryStreamCreate 仅支持单一录像类型或者所有录像类型)
                if (!(ret = HieClient_HistoryStreamCreate(&handle, user_id, &hsp, 0)))
                {
                    //设置流媒体数据回调
                    if (!(ret = HieClient_StreamMediaCB(handle, hie_record_data_callback, 0)))
                    {
                        //启动历史流
                        if (!(ret = HieClient_StreamMediaControl(handle, eTaskStart)))
                        {
                            show_debug("HieClient_StreamMediaControl Success!\n");
                            nmp_mutex_lock(hie_srv->rtsp.lock);
                            strm_info->handle = (int)handle;
                            strm_info->state = STATS_STREAM_PLAYING;
                            ((hie_rec_strm_t*)strm_info)->enable = 1;
                            nmp_mutex_unlock(hie_srv->rtsp.lock);
                        }
                        else
                        {
                            show_warn("HieClient_StreamMediaControl failure, err: %d!!\n", ret);
                        }
                    }
                    else
                        show_warn("HieClient_StreamMediaCB failure, err: %d!!\n", ret);
                }
                else
                    show_warn("HieClient_HistoryStreamCreate failure, err: %d!!\n", ret);
            }
        }
        break;

    case CMD_STREAM_PAUSE:
        if (STREAM_REAL == type)
        {
        }
        else if (STREAM_VOD == type || STREAM_DOWNLOAD == type)
        {
            nmp_mutex_lock(hie_srv->rtsp.lock);
            if (STATS_STREAM_PLAYING == strm_info->state && strm_info->handle)
            {
                strm_info->state = STATS_STREAM_PAUSE;
                ((hie_rec_strm_t*)strm_info)->enable = 0;
            }
            nmp_mutex_unlock(hie_srv->rtsp.lock);
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
hie_stream_init(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    hie_service_t *hie_srv;
    NMP_ASSERT(owner && strm_info);

    hie_srv = (hie_service_t*)owner;
    if (HIE_INVALID_HANDLE == hie_get_user_id(&hie_srv->parm))
    {
        show_warn("hie device is disconnected.\n");
    }
    else
    {
        switch (strm_info->type)
        {
            case STREAM_REAL://0 播放实时流
                ret = hie_init_real_stream(hie_srv, strm_info);
                strm_info->length = 0;
                break;
            case STREAM_VOD:      //2点播录像文件
            case STREAM_DOWNLOAD: //3下载录像文件
                ret = hie_init_record_stream(hie_srv, strm_info);
                strm_info->length = 0;
                break;

            default:
                break;
        }
    }

    return ret;
}
static int 
hie_stream_open(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    hie_service_t *hie_srv;
    NMP_ASSERT(owner && strm_info);

    hie_srv = (hie_service_t*)owner;

    if (HIE_INVALID_HANDLE == hie_get_user_id(&hie_srv->parm))
    {
         show_warn("hie device is disconnected..\n");
    }
    else
    {
        nmp_mutex_lock(hie_srv->rtsp.lock);
        switch (strm_info->type)
        {
            case STREAM_REAL:               // 播放实时流
                ret = hie_open_real_stream(hie_srv, strm_info);
                break;
            case STREAM_VOD:                // 点播录像文件
            case STREAM_DOWNLOAD:           // 下载录像文件
                //ret = hie_open_record_stream(hie_srv, strm_info);
                break;

            default:
                break;
        }
        nmp_mutex_unlock(hie_srv->rtsp.lock);
    }

    return ret;
}
static int 
hie_stream_play(void *owner, stream_info_t *strm_info)
{
    int type, ret = -1;
    hie_service_t *hie_srv;
    NMP_ASSERT(owner && strm_info);

    hie_srv = (hie_service_t*)owner;

    if (HIE_INVALID_HANDLE == hie_get_user_id(&hie_srv->parm))
    {
         show_warn("hie device is disconnected..\n");
    }
    else
    {
        nmp_mutex_lock(hie_srv->rtsp.lock);
        type = strm_info->type;
        nmp_mutex_unlock(hie_srv->rtsp.lock);

        //ret = hie_process_stream(hie_srv, strm_info, CMD_STREAM_PLAY, type);
    }

    return ret;
}
static int 
hie_stream_pause(void *owner, stream_info_t *strm_info)
{
    int type, ret = -1;
    hie_service_t *hie_srv;
    NMP_ASSERT(owner && strm_info);

    hie_srv = (hie_service_t*)owner;

    if (HIE_INVALID_HANDLE == hie_get_user_id(&hie_srv->parm))
    {
         show_warn("hie device is disconnected..\n");
    }
    else
    {
        nmp_mutex_lock(hie_srv->rtsp.lock);
        type = strm_info->type;
        nmp_mutex_unlock(hie_srv->rtsp.lock);
        
        //ret = hie_process_stream(hie_srv, strm_info, CMD_STREAM_PAUSE, type);
    }

    return ret;
}
static int 
hie_stream_close(void *owner, stream_info_t *strm_info)
{
    hie_service_t *hie_srv;
    HSTREAM handle = HIE_INVALID_HANDLE;
    NMP_ASSERT(owner && strm_info);

    hie_srv = (hie_service_t*)owner;

    switch (strm_info->type)
    {
        case STREAM_REAL:
            nmp_mutex_lock(hie_srv->rtsp.lock);
            handle = (HSTREAM)strm_info->handle;
            strm_info->handle = HIE_INVALID_HANDLE;
            remove_one_stream(&hie_srv->rtsp.real_strm_list, strm_info);
            nmp_mutex_unlock(hie_srv->rtsp.lock);

            if (HIE_INVALID_HANDLE != handle)
            {
                HieClient_StreamMediaCB(handle, NULL, 0);
                HieClient_StreamMediaControl(handle, eTaskStop);
                HieClient_RealStreamDisconnect(handle);
            }

            memset(strm_info, 0, sizeof(hie_real_strm_t));
            nmp_del(strm_info, hie_real_strm_t, 1);
            break;

        case STREAM_VOD:
        case STREAM_DOWNLOAD:
            nmp_mutex_lock(hie_srv->rtsp.lock);
            handle = (HSTREAM)strm_info->handle;
            strm_info->handle = HIE_INVALID_HANDLE;
            ((hie_rec_strm_t*)strm_info)->enable = 0;
            remove_one_stream(&hie_srv->rtsp.rec_strm_list, strm_info);
            nmp_mutex_unlock(hie_srv->rtsp.lock);

            if (HIE_INVALID_HANDLE != handle)
            {
                //删除流媒体数据回调
                HieClient_StreamMediaCB(handle, NULL, 0);
                //停止历史流
                HieClient_StreamMediaControl(handle, eTaskStop);
                //销毁历史流通道
                HieClient_HistoryStreamDestroy(handle);
            }

            memset(strm_info, 0, sizeof(hie_rec_strm_t));
            nmp_del(strm_info, hie_rec_strm_t, 1);
            break;
    }

    return 0;
}
static int 
hie_stream_seek(void *owner, stream_info_t *strm_info)
{
    int type, ret = -1;
    hie_service_t *hie_srv;
    NMP_ASSERT(owner && strm_info);

    hie_srv = (hie_service_t*)owner;

    if (HIE_INVALID_HANDLE == hie_get_user_id(&hie_srv->parm))
    {
         show_warn("hie device is disconnected..\n");
    }
    else
    {
        nmp_mutex_lock(hie_srv->rtsp.lock);
        type = strm_info->type;
        nmp_mutex_unlock(hie_srv->rtsp.lock);

        //ret = hie_process_stream(hie_srv, strm_info, CMD_STREAM_SEEK, type);
    }

    return ret;
}
static int 
hie_stream_ctrl(void *owner, int channel, int level, int cmd, void *value)
{
    int ret = -1;
    hie_service_t *hie_srv;
    NMP_ASSERT(owner);

    hie_srv = (hie_service_t*)owner;

    if (HIE_INVALID_HANDLE == hie_get_user_id(&hie_srv->parm))
    {
        show_warn("hie device is disconnected.\n");
    }
    else
    {
        switch (cmd)
        {
            case DEF_STOP_ALL_STREAM_CMD:
                hie_cleanup_stream_info(&hie_srv->rtsp);
                ret = 0;
                break;

            default:
                break;
        }
    }

    return ret;
}

const stream_operation_t hie_strm_opt = 
{
    hie_stream_init,
    hie_stream_open,
    hie_stream_play,
    hie_stream_pause,
    hie_stream_close,
    hie_stream_seek,
    hie_stream_ctrl,
};




