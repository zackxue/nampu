
#include <string.h>
#include <sys/time.h>

#include "nmp_sdk.h"

#include "stream_api.h"
#include "rtsp-server.h"
#include "standard_frame.h"

#include "nmp_bsm_service.h"
#include "nmp_bsm_channel.h"

#define BSM_STREAM_HEAD_INFO            "BSM-STREAM-HEAD"


static __inline__ void 
bsm_make_key_frame(int user_id, int channel, int level)
{// 强制关键帧只有在播数据才有效
    if (!HI_NET_DEV_MakeKeyFrame((HI_U32)user_id, (channel*10) + (level+1)))
    {
        show_warn("HI_NET_DEV_MakeKeyFrame failed.\n");
    }
}

static HI_S32 
bsm_real_stream_callback(HI_U32   handle,    /* 句柄 */
                                HI_U32   data_type, /* 数据类型，系统数据或音视频数据 */
                                HI_U8   *buffer,    /* 数据包含帧头 */
                                HI_U32   buf_len,   /* 数据长度 */
                                HI_VOID *user       /* 用户数据*/
                             )
{//show_debug("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<%d:%d\n", data_type, buf_len);
    struct timeval ts;
    stream_info_t *strm_info;

    NMP_ASSERT(handle);
    NMP_ASSERT(user);
    NMP_ASSERT(buffer && 0 < buf_len);

    strm_info = (stream_info_t*)user;

    switch (data_type)
    {
        case HI_NET_DEV_SYS_DATA://文件数据
            if (!strm_info->media_size)
            {
                memcpy(strm_info->media_info, buffer, buf_len);
                strm_info->media_size = buf_len;

                show_debug("Signal cond after real data callback.[size: %d]\n", strm_info->media_size);
                proxy_cond_signal(((bsm_real_strm_t*)strm_info)->cond);
            }
            break;

        case HI_NET_DEV_AV_DATA://音视频数据
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
                    (guint8*)buffer, (gsize)buf_len, 
                    (guint8)data_type, (guint8*)NULL, (gsize)0, (guint8)0);
            }
            break;

        default:
            break;
    }

    return HI_SUCCESS;
}

static __inline__ int 
bsm_init_real_stream(bsm_service_t *bsm_srv, stream_info_t *strm_info)
{
    strcpy(strm_info->media_info, BSM_STREAM_HEAD_INFO);
    strm_info->media_size = strlen(BSM_STREAM_HEAD_INFO);
    return 0;
}

static __inline__ int 
bsm_init_record_stream(bsm_service_t *bsm_srv, stream_info_t *strm_info)
{
    strcpy(strm_info->media_info, BSM_STREAM_HEAD_INFO);
    strm_info->media_size = strlen(BSM_STREAM_HEAD_INFO);
    return 0;
}
static __inline__ int 
bsm_open_real_stream(bsm_service_t *bsm_srv, stream_info_t *strm_info)
{
    int ret = -1;
    stream_info_t *real_strm;

    real_strm = find_stream_by_channel_and_level(&bsm_srv->rtsp.real_strm_list, 
                    strm_info->channel, strm_info->level);  
    if (!real_strm)
    {
        real_strm = (stream_info_t*)nmp_new0(bsm_real_strm_t, 1);
        real_strm->handle  = BSM_INVALID_HANDLE;
        real_strm->opened  = strm_info->opened;
        real_strm->type    = strm_info->type;
        real_strm->level   = strm_info->level;
        real_strm->channel = strm_info->channel;
        real_strm->state   = STATS_STREAM_STOP;
        real_strm->sdk_srv = (struct service*)bsm_srv;

        strcpy(real_strm->media_info, BSM_STREAM_HEAD_INFO);
        real_strm->media_size = strlen(BSM_STREAM_HEAD_INFO);

        set_stream_user_data((gpointer)real_strm->opened, (void*)real_strm);
        add_one_stream(&bsm_srv->rtsp.real_strm_list, real_strm);
        ret = 0;
    }
    else
        show_error("stream [channel:%d] [level:%d] already opened!!\n", 
            strm_info->channel, strm_info->level);

    return ret;
}

static __inline__ int 
bsm_open_record_stream(bsm_service_t *bsm_srv, stream_info_t *strm_info)
{
    int ret = -1;
#if 0
    stream_info_t *rec_strm;

    /*NET_TIME start_time = {0};
    NET_TIME stop_time  = {0};

    if (0 != bsm_get_start_time((const char*)strm_info->pri_data, &start_time))
        return -1;
    if (0 != bsm_get_end_time((const char*)strm_info->pri_data, &stop_time))
        return -1;

    rec_strm = bsm_find_record_stream_by_channel_and_time(&bsm_srv->rtsp.rec_strm_list, 
                    strm_info->channel, &start_time, &stop_time);*/
    if (!rec_strm)
    {
        rec_strm = (stream_info_t*)nmp_new0(bsm_rec_strm_t, 1);
        rec_strm->handle  = BSM_INVALID_HANDLE;
        rec_strm->opened  = strm_info->opened;
        rec_strm->type    = strm_info->type;
        rec_strm->level   = strm_info->level;
        rec_strm->channel = strm_info->channel;
        rec_strm->state   = STATS_STREAM_STOP;
        rec_strm->sdk_srv = (struct service*)bsm_srv;

        strcpy(rec_strm->media_info, BSM_STREAM_HEAD_INFO);
        rec_strm->media_size = strlen(BSM_STREAM_HEAD_INFO);

        ((bsm_rec_strm_t*)rec_strm)->enable = 0;
        //((bsm_rec_strm_t*)rec_strm)->start_time = start_time;
        //((bsm_rec_strm_t*)rec_strm)->stop_time = stop_time;

        set_stream_user_data((gpointer)rec_strm->opened, (void*)rec_strm);
        add_one_stream(&bsm_srv->rtsp.rec_strm_list, rec_strm);
        ret = 0;
    }   
    else
        show_error("stream [channel:%d] [level:%d] already opened!!\n", 
            strm_info->channel, strm_info->level);
    
#endif
    return ret;
}

static __inline__ int 
bsm_process_stream(bsm_service_t *bsm_srv, stream_info_t *strm_info, stream_cmd_t cmd)
{
    int ret = 0;
    int handle = BSM_INVALID_HANDLE;
    HI_S_STREAM_INFO hi_strm_info;

    switch(cmd)
    {
        case CMD_STREAM_PLAY:
            if (STREAM_REAL == strm_info->type)
            {
                if (STATS_STREAM_PLAYING != strm_info->state)
                {
                    hi_strm_info.u32Channel = strm_info->channel;
                    hi_strm_info.blFlag     = !strm_info->level ? HI_TRUE : HI_FALSE;
                    hi_strm_info.u32Mode    = HI_NET_DEV_STREAM_MODE_TCP;
                    hi_strm_info.u8Type     = HI_NET_DEV_STREAM_VIDEO_ONLY;

                    handle = bsm_get_user_id(&bsm_srv->parm);
                    HI_NET_DEV_SetStreamCallBack((HI_U32)handle, bsm_real_stream_callback, 
                        (HI_VOID*)strm_info);
                    if (HI_SUCCESS == HI_NET_DEV_StartStream((HI_U32)handle, &hi_strm_info))
                    {
                        strm_info->handle = handle;//<<<<<<<<<<<<<<<<<<<<<<<< todo lock
                        strm_info->state  = STATS_STREAM_PLAYING;
                    }
                    else
                    {
                        ret = -1;
                        show_warn("HI_NET_DEV_StartStream(handle: %d, ch: %d\n", 
                            handle, strm_info->channel);
                    }
                }
                else
                {
                    bsm_make_key_frame(bsm_get_user_id(&bsm_srv->parm), 
                        strm_info->channel, strm_info->level);
                }
            }
            break;

        case CMD_STREAM_PAUSE:
            if (STREAM_REAL == strm_info->type)
            {
            }
            /*else if (STREAM_VOD == strm_info->type || STREAM_DOWNLOAD == strm_info->type)
            {
                if (STATS_STREAM_PLAYING == strm_info->state && BSM_INVALID_HANDLE != strm_info->handle)
                {
                    strm_info->state = STATS_STREAM_PAUSE;
                    ((bsm_rec_strm_t*)strm_info)->enable = 0;
                }
            }*/
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

static int bsm_stream_init(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    bsm_service_t *bsm_srv;
    NMP_ASSERT(owner && strm_info);

    bsm_srv = (bsm_service_t*)owner;
    if (BSM_LOGOUT == bsm_get_user_id(&bsm_srv->parm))
    {
        show_warn("bsm device is disconnected.\n");
    }
    else
    {
        switch (strm_info->type)
        {
            case STREAM_REAL://0 播放实时流
                ret = bsm_init_real_stream(bsm_srv, strm_info);
                strm_info->length = 0;
                break;
            case STREAM_VOD:      //2点播录像文件
            case STREAM_DOWNLOAD: //3下载录像文件
                ret = bsm_init_record_stream(bsm_srv, strm_info);
                strm_info->length = 0;
                break;

            default:
                break;
        }
    }

    return ret;
}
static int bsm_stream_open(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    bsm_service_t *bsm_srv;
    NMP_ASSERT(owner && strm_info);

    bsm_srv = (bsm_service_t*)owner;

    if (BSM_LOGOUT == bsm_get_user_id(&bsm_srv->parm))
    {
         show_warn("bsm device is disconnected..\n");
    }
    else
    {
        nmp_mutex_lock(bsm_srv->rtsp.lock);
        switch (strm_info->type)
        {
            case STREAM_REAL:               // 播放实时流
                ret = bsm_open_real_stream(bsm_srv, strm_info);
                break;
            case STREAM_VOD:                // 点播录像文件
            case STREAM_DOWNLOAD:           // 下载录像文件
                ret = bsm_open_record_stream(bsm_srv, strm_info);
                break;
                
            default:
                break;
        }
        nmp_mutex_unlock(bsm_srv->rtsp.lock);
    }

    return ret;
}
static int bsm_stream_play(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    bsm_service_t *bsm_srv;
    NMP_ASSERT(owner && strm_info);

    bsm_srv = (bsm_service_t*)owner;

    if (BSM_LOGOUT == bsm_get_user_id(&bsm_srv->parm))
    {
         show_warn("bsm device is disconnected..\n");
    }
    else
    {
        nmp_mutex_lock(bsm_srv->rtsp.lock);
        ret = bsm_process_stream(bsm_srv, strm_info, CMD_STREAM_PLAY);
        nmp_mutex_unlock(bsm_srv->rtsp.lock);
    }

    return ret;
}
static int bsm_stream_pause(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    bsm_service_t *bsm_srv;
    NMP_ASSERT(owner && strm_info);

    bsm_srv = (bsm_service_t*)owner;

    if (BSM_LOGOUT == bsm_get_user_id(&bsm_srv->parm))
    {
         show_warn("bsm device is disconnected..\n");
    }
    else
    {
        nmp_mutex_lock(bsm_srv->rtsp.lock);
        ret = bsm_process_stream(bsm_srv, strm_info, CMD_STREAM_PAUSE);
        nmp_mutex_unlock(bsm_srv->rtsp.lock);
    }

    return ret;
}
static int bsm_stream_close(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    int handle = BSM_INVALID_HANDLE;
    bsm_service_t *bsm_srv;
    NMP_ASSERT(owner && strm_info);

    bsm_srv = (bsm_service_t*)owner;

    switch (strm_info->type)
    {
        case STREAM_REAL:
            nmp_mutex_lock(bsm_srv->rtsp.lock);
            handle = strm_info->handle;
            strm_info->handle = BSM_INVALID_HANDLE;
            remove_one_stream(&bsm_srv->rtsp.real_strm_list, strm_info);
            nmp_mutex_unlock(bsm_srv->rtsp.lock);

            if (BSM_INVALID_HANDLE != handle)
            {
                if (HI_SUCCESS == HI_NET_DEV_StopStream((HI_U32)handle))
                    ret = 0;
                else
                    show_warn("HI_NET_DEV_StopStream()\n");
            }

            memset(strm_info, 0, sizeof(bsm_real_strm_t));
            nmp_del(strm_info, bsm_real_strm_t, 1);
            break;

        case STREAM_VOD:
        case STREAM_DOWNLOAD:
            nmp_mutex_lock(bsm_srv->rtsp.lock);
            nmp_mutex_unlock(bsm_srv->rtsp.lock);
            break;
    }

    return 0;
}
static int bsm_stream_seek(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    bsm_service_t *bsm_srv;
    NMP_ASSERT(owner && strm_info);

    bsm_srv = (bsm_service_t*)owner;

    if (BSM_LOGOUT == bsm_get_user_id(&bsm_srv->parm))
    {
         show_warn("bsm device is disconnected..\n");
    }
    else
    {
        nmp_mutex_lock(bsm_srv->rtsp.lock);
        ret = bsm_process_stream(bsm_srv, strm_info, CMD_STREAM_SEEK);
        nmp_mutex_unlock(bsm_srv->rtsp.lock);
    }

    return ret;
}
static int bsm_stream_ctrl(void *owner, int channel, int level, int cmd, void *value)
{
    int ret = -1;
    bsm_service_t *bsm_srv;
    NMP_ASSERT(owner);

    bsm_srv = (bsm_service_t*)owner;

    if (BSM_LOGOUT == bsm_get_user_id(&bsm_srv->parm))
    {
        show_warn("bsm device is disconnected.\n");
    }
    else
    {
        switch (cmd)
        {
            case DEF_STOP_ALL_STREAM_CMD:
                bsm_cleanup_stream_info(&bsm_srv->rtsp);
                ret = 0;
                break;
                
            default:
                break;
        }
    }

    return ret;
}

stream_operation_t bsm_strm_opt = 
{
    bsm_stream_init,
    bsm_stream_open,
    bsm_stream_play,
    bsm_stream_pause,
    bsm_stream_close,
    bsm_stream_seek,
    bsm_stream_ctrl,
};


