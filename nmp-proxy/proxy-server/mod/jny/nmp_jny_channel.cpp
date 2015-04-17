#include <string.h>
#include <sys/time.h>

#include "nmp_sdk.h"

#include "stream_api.h"
#include "rtsp-server.h"
#include "standard_frame.h"

#include "nmp_jny_service.h"
#include "nmp_jny_thread_proc.h"
#include "nmp_jny_channel.h"

#define DEBUG	1
#define MAX_INIT_WAITE_TIMES            3
#define MAX_STREAM_BUFFER_LEN			1024*1024
#if DEBUG

static int jny_set_video_parm(ST_AVFrameData* p_stAVFrameData, frame_data_info* send_data)
{
	frame_data_info *data = send_data;
	data->nBitRate = p_stAVFrameData->nBitRate;
	data->nCameraNo = p_stAVFrameData->nCameraNo;
	data->nDataLength = p_stAVFrameData->nDataLength;
	data->nDaylightSavingTime = p_stAVFrameData->nDaylightSavingTime;
	data->nEncoderType = p_stAVFrameData->nEncoderType;
	data->nESStreamType = p_stAVFrameData->nESStreamType;
	data->nFrameBufLen = p_stAVFrameData->nFrameBufLen;
	data->nFrameRate = p_stAVFrameData->nFrameRate;
	data->nFrameType = p_stAVFrameData->nFrameType;
	data->nImageFormatId = p_stAVFrameData->nImageFormatId;
	data->nImageHeight = p_stAVFrameData->nImageHeight;
	data->nImageWidth = p_stAVFrameData->nImageWidth;
	data->nSequenceId = p_stAVFrameData->nSequenceId;
	data->nStreamFormat = p_stAVFrameData->nStreamFormat;
	data->nStreamId = p_stAVFrameData->nStreamId;
	data->nTimeStamp = p_stAVFrameData->nTimeStamp;
	data->nTimezone = p_stAVFrameData->nTimezone;
	data->nVideoSystem = p_stAVFrameData->nVideoSystem;
	memcpy(data->pszData, p_stAVFrameData->pszData, p_stAVFrameData->nDataLength);
	return 0;
}

//static void *jny_real_stream_callback(void *data)
static long jny_real_stream_callback(long p_hHandle, ST_AVFrameData* p_stAVFrameData, void* pUserData)
{
	struct timeval ts;
	stream_info_t *strm_info;
	
    NMP_ASSERT(p_stAVFrameData);	

	strm_info = (stream_info_t *)pUserData;
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
		
		jny_set_video_parm(p_stAVFrameData, (frame_data_info *)strm_info->pri_data);
		//show_debug("nDataLength = %d\n", p_stAVFrameData->nDataLength);
        write_stream_data((gpointer)strm_info->opened, 
            (gulong)strm_info->ts, (gulong)DEF_TIMESTAMP, 
            (guint8*)strm_info->pri_data, (gsize)(p_stAVFrameData->nDataLength + sizeof(frame_data_info)), 
            (guint8)p_stAVFrameData->nESStreamType, (guint8*)NULL, (gsize)0, (guint8)0);
        return 0;
    }
	else
	{
		show_error("opened is NULL\n");
	}

    return -1;
}

static __inline__ int jny_init_real_stream(jny_service *jny_srv, stream_info_t *strm_info)
{
	int ret = 0;

	stream_info_t *real_strm;

	real_strm = find_stream_by_channel_and_level(&jny_srv->rtsp.real_strm_list, 
	                strm_info->channel, strm_info->level);

	if (real_strm)
	{
	    //memcpy(strm_info->media_info, real_strm->media_info, real_strm->media_size);
	    //strm_info->media_size = real_strm->media_size;
	    
	    ret = 0;
	}
	else
	{
		
	}

	return ret;
}
#endif

static int jny_stream_init(void *owner, stream_info_t *strm_info)
{
#if DEBUG
    int ret = -1;
    jny_service *jny_srv;
    jny_real_strm_info real_strm;
    //jny_rec_strm_info rec_strm;
    
    NMP_ASSERT(owner && strm_info);

    jny_srv = (jny_service *)owner;

    memset(&real_strm, 0, sizeof(real_strm));
    //memset(&rec_strm, 0, sizeof(rec_strm));

    if (JNY_LOGOUT == jny_get_user_id(&jny_srv->parm))
    {
         show_warn("jny device is disconnected..\n");
    }
    else
    {
    	strm_info->channel += 1;
        switch (strm_info->type)
        {
            case STREAM_REAL:				// 播放实时流
            	//Remote_Camera2_SetDefaultStreamId(jny_get_user_id(&jny_srv->parm), 1);
				//ret = Remote_Camera2_Open(jny_get_user_id(&jny_srv->parm), nCameraID);
				//if (ret != SN_SUCCESS)
				//{
				//	jny_print_error(ret);
				//	return -1;
				//}
    			memcpy(&real_strm, strm_info, sizeof(stream_info_t));
    			
    			ret = jny_init_real_stream(jny_srv, (stream_info_t*)&real_strm);
    			memcpy(strm_info, &real_strm, sizeof(stream_info_t));
    			strm_info->length = 0;
    			
                break;
    		case STREAM_VOD:      			//2点播录像文件
    		case STREAM_DOWNLOAD:			//3下载录像文件
    			
                break;
    			
            default:
                break;
        }
    }

    return ret;
#endif
	return 0;
}

#define MAX_STREAM_DATA_LEN 1024*1024
static __inline__ int jny_open_real_stream(jny_service *jny_srv, stream_info_t *strm_info)
{
    int ret = -1;
    stream_info_t *real_strm;

    real_strm = find_stream_by_channel_and_level(
                    &jny_srv->rtsp.real_strm_list, 
                    strm_info->channel, strm_info->level);
    if (!real_strm)
    {
		Remote_Camera2_SetDefaultStreamId(jny_get_user_id(&jny_srv->parm), 1);
		//Remote_Camera2_Open(jny_get_user_id(&jny_srv->parm), nCameraID);
	
        real_strm = (stream_info_t*)nmp_new0(jny_real_strm_info, 1);
        real_strm->handle  = jny_srv->parm.user_id;
        real_strm->opened  = strm_info->opened;
        real_strm->type    = strm_info->type;
        real_strm->level   = strm_info->level;
        real_strm->channel = strm_info->channel;
        real_strm->state   = STATS_STREAM_STOP;
        real_strm->sdk_srv = (struct service*)jny_srv;
        real_strm->pri_data = (void *)malloc(sizeof(frame_data_info) + MAX_STREAM_DATA_LEN);
        //memcpy(real_strm->media_info, strm_info->media_info, strm_info->media_size);
        //real_strm->media_size = strm_info->media_size;
        
        set_stream_user_data((void *)strm_info->opened, (void *)real_strm);
        add_one_stream(&jny_srv->rtsp.real_strm_list, real_strm);
        
        ret = 0;
    }
    else
    {
        show_error("stream [channel:%d] [level:%d] already opened!!\n", 
            strm_info->channel, strm_info->level);
    }

    return ret;
}


static __inline__ int jny_open_record_stream(jny_service *jny_srv, stream_info_t *strm_info)
{
    int ret = -1;
    stream_info_t *real_strm;

    real_strm = find_stream_by_channel_and_level(
                    &jny_srv->rtsp.real_strm_list, 
                    strm_info->channel, strm_info->level);
    if (!real_strm)
    {
		Remote_Camera2_SetDefaultStreamId(jny_get_user_id(&jny_srv->parm), 1);
		//Remote_Camera2_Open(jny_get_user_id(&jny_srv->parm), nCameraID);
	
        real_strm = (stream_info_t*)nmp_new0(jny_real_strm_info, 1);
        real_strm->handle  = jny_srv->parm.user_id;
        real_strm->opened  = strm_info->opened;
        real_strm->type    = strm_info->type;
        real_strm->level   = strm_info->level;
        real_strm->channel = strm_info->channel;
        real_strm->state   = STATS_STREAM_STOP;
        real_strm->sdk_srv = (struct service*)jny_srv;
        real_strm->pri_data = (void *)malloc(sizeof(frame_data_info) + MAX_STREAM_DATA_LEN);
        //memcpy(real_strm->media_info, strm_info->media_info, strm_info->media_size);
        //real_strm->media_size = strm_info->media_size;
        
        set_stream_user_data((void *)strm_info->opened, (void *)real_strm);
        add_one_stream(&jny_srv->rtsp.real_strm_list, real_strm);
        
        ret = 0;
    }
    else
    {
        show_error("stream [channel:%d] [level:%d] already opened!!\n", 
            strm_info->channel, strm_info->level);
    }

    return ret;
}


static int jny_stream_open(void *owner, stream_info_t *strm_info)
{
	int ret = -1;
    jny_service *jny_srv;

	NMP_ASSERT(owner && strm_info);

    jny_srv = (jny_service *)owner;

    if (JNY_LOGOUT == jny_get_user_id(&jny_srv->parm))
    {
         show_warn("jny device is disconnected..\n");
    }
    else
    {
        strm_info->channel += 1;
        switch (strm_info->type)
        {
            case STREAM_REAL:               // 播放实时流
                ret = jny_open_real_stream(jny_srv, strm_info);
                break;
            case STREAM_VOD:                // 点播录像文件
            case STREAM_DOWNLOAD:           // 下载录像文件
            
                break;
                
            default:
                break;
        }
    }

    return ret;
}

static __inline__ void jny_make_key_frame(long p_hHandle)
{

    Remote_Camera3_MakeKeyFrame(p_hHandle);

}


static __inline__ int jny_process_stream(jny_service *jny_srv, stream_info_t *strm_info, 
												stream_cmd_t cmd)
{
    int ret = -1;

    switch(cmd)
    {
    case CMD_STREAM_PLAY:
        if (STREAM_REAL == strm_info->type)
        {
            if (STATS_STREAM_PLAYING != strm_info->state)
            {
            	ret = Remote_Camera3_SetAVDateCallback(jny_get_user_id(&jny_srv->parm), jny_real_stream_callback, strm_info);
				if(ret != SN_SUCCESS)
				{
					//处理异常...
					jny_print_error(ret);
					return -1;
				}
				//ret = nmp_jny_stream_start((void *)strm_info, jny_real_stream_callback)
                //if (ret == 0)
                ret = Remote_Camera3_Open(jny_get_user_id(&jny_srv->parm), strm_info->channel);
				if(ret == SN_SUCCESS)
                {
                    strm_info->state = STATS_STREAM_PLAYING;
                    strm_info->handle = 0;
                    ret = 0;
                }
				else
				{
					show_error("nmp_jny_stream_start\n");
					ret = -1;
				}
    
            }
            else
            {
            	show_debug("------->jny_make_key_frame\n");
                jny_make_key_frame((long)jny_get_user_id(&jny_srv->parm));
            }
        }
        else if (STREAM_VOD == strm_info->type || 
                 STREAM_DOWNLOAD == strm_info->type)
        {
            if (STATS_STREAM_PAUSE == strm_info->state && -1 != strm_info->handle)
            {

            }
            else if (STATS_STREAM_STOP == strm_info->state && -1 != strm_info->handle)
            {

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
                -1 != strm_info->handle)
            {

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



static int jny_stream_play(void *owner, stream_info_t *strm_info)
{
    int ret = -1;
    jny_service *jny_srv;
	
    NMP_ASSERT(owner && strm_info);

    jny_srv = (jny_service*)owner;

    if (JNY_LOGOUT == jny_get_user_id(&jny_srv->parm))
    {
         show_warn("jny device is disconnected..\n");
    }
    else
    {
        ret = jny_process_stream(jny_srv, strm_info, CMD_STREAM_PLAY);
    }

    return ret;
}

static int jny_stream_pause(void *owner, stream_info_t *strm_info)
{
	return 0;
}

static int jny_stream_close(void *owner, stream_info_t *strm_info)
{
	int ret = -1;
	jny_service *jny_srv;
	NMP_ASSERT(owner && strm_info);

	jny_srv = (jny_service *)owner;

	switch (strm_info->type)
	{
	 case STREAM_REAL:
		 if (JNY_LOGOUT != jny_get_user_id(&jny_srv->parm))
		 {
		 	ret = Remote_Camera3_Close(jny_get_user_id(&jny_srv->parm));
			if (ret == SN_SUCCESS)
			{
			 ret = 0; 
			}
			else
			{
			 jny_print_error(ret);
			}
		 }
		 remove_one_stream(&jny_srv->rtsp.real_strm_list, strm_info);

		 memset(strm_info, 0, sizeof(jny_real_strm_info));
		 nmp_del(strm_info, jny_real_strm_info, 1);
		 break;

	 case STREAM_VOD:
	 case STREAM_DOWNLOAD:
		 if (JNY_INVALID_HANDLE != strm_info->handle)
		 {

		 }
		 break;
	}
	
	return ret;
}

static int jny_stream_seek(void *owner, stream_info_t *strm_info)
{
	return 0;
}

static int jny_stream_ctrl(void *owner, int channel, int level, int cmd, void *value)
{
	return 0;
}

stream_operation_t jny_strm_opt = 
{
    jny_stream_init,
    jny_stream_open,
    jny_stream_play,
    jny_stream_pause,
    jny_stream_close,
    jny_stream_seek,
    jny_stream_ctrl,
};

