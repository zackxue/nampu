
#include "dhnetsdk.h"
#include "dhconfigsdk.h"

#include "alaw_encoder.h"

#include "nmp_dah_talk.h"
#include "nmp_dah_service.h"



#define MAX_ENCODE_LEN              (2 * 1024)


static int 
set_audio_attr(media_info_t *info, DHDEV_TALKDECODE_INFO *cur_talk_mode)
{
    NMP_ASSERT(info && cur_talk_mode);

    if(info->attr.encode_type == 0)
    {
        cur_talk_mode->encodeType = DH_TALK_PCM;
    }
    else if(info->attr.encode_type == 1)
    {
        cur_talk_mode->encodeType = DH_TALK_G711a;
    }
    else if(info->attr.encode_type == 2)
    {
        cur_talk_mode->encodeType = DH_TALK_G711u;
    }
    else
    {
        return -1;
    }

    if(info->attr.samples_per_sec == 0)
    {
        cur_talk_mode->dwSampleRate = 8000;
    }
    else if(info->attr.samples_per_sec == 1)
    {
        cur_talk_mode->dwSampleRate = 16000;
    }
    else if(info->attr.samples_per_sec == 2)
    {
        cur_talk_mode->dwSampleRate = 32000;
    }
    else
    {
        return -1;
    }
    
    cur_talk_mode->nAudioBit = info->attr.audio_bits;
    
    return 0;

}

static int 
dah_send_audio_data(talk_handle_t *hdl, char *buffer, int len, frame_t *dah_frm)
{

    if(dah_frm)
    {
        memset(dah_frm, 0, sizeof(frame_t) + MAX_ENCODE_BUFFER_LEN);
        
        dah_frm->magic = htonl(TALK_MAGIC);
        dah_frm->frame_length = htonl(len);
        dah_frm->pts = 0;
        dah_frm->frame_num = htonl(0);
        memcpy(dah_frm->frame_data, buffer, len);
        send_talk_data(hdl, dah_frm);
        return 0;
    }
    return -1;
}

static void 
dah_audio_data_callback(LLONG talk_handle, char *data_buf, DWORD buf_size, 
								BYTE audio_flag, LDWORD user)
{
    if(!user)
        return;
    talk_info_t *talk = (talk_info_t*)user;
    talk_handle_t *hdl = talk->hdl;
    if(1 == audio_flag)//设备发过来的数据
    {
        CLIENT_AudioDec(data_buf, buf_size);

        dah_send_audio_data(hdl, data_buf, buf_size, talk->frm);
    }
}

static int 
dah_talk_open(struct service *srv, talk_info_t *talk, 
        talk_handle_t *hdl, media_info_t *info)
{
    int ret = -1;
    dah_service_t *dah_srv;

    DH_AUDIO_FORMAT aft;
    DHDEV_TALKDECODE_INFO cur_talk_mode;

    NMP_ASSERT(srv && talk);

    memset(&aft, 0, sizeof(DH_AUDIO_FORMAT));
    memset(&cur_talk_mode, 0, sizeof(DHDEV_TALKDECODE_INFO));

    aft.byFormatTag = 0;
    aft.nChannels = 1;
    aft.nSamplesPerSec = 8000;
    aft.wBitsPerSample = 16;

    dah_srv = (dah_service_t*)srv;
    talk->handle = dah_get_user_id(&dah_srv->parm);
    if(talk->frm)
    {
        nmp_dealloc(talk->frm, sizeof(frame_t) + MAX_ENCODE_BUFFER_LEN);
        talk->frm = NULL;
    }

    talk->frm = (frame_t *)nmp_alloc0(sizeof(frame_t) + MAX_ENCODE_BUFFER_LEN);
    if(!talk->frm)
    {
        return -1;
    }
	
    if (DAH_LOGOUT == dah_get_user_id(&dah_srv->parm))
    {
        show_warn("dah device is disconnected.\n");
    }
    else
    {
        ret = CLIENT_InitAudioEncode(aft);
        if(ret != 0)
        {
            show_error("CLIENT_InitAudioEncode error ret = %d\n", ret);
            if(talk->frm)
            {
                nmp_dealloc(talk->frm, sizeof(frame_t) + MAX_ENCODE_BUFFER_LEN);
                talk->frm = NULL;
            }
            return -1;
        }

        //查询设备支持的语音对讲列表
        DHDEV_TALKFORMAT_LIST lstTalkEncode = {0};
        int nRetlen = 0;
        BOOL bSuccess = CLIENT_QueryDevState(dah_get_user_id(&dah_srv->parm), DH_DEVSTATE_TALK_ECTYPE, 
                (char*)&lstTalkEncode, sizeof(DHDEV_TALKFORMAT_LIST), &nRetlen, 2000);
        if (!(bSuccess&&nRetlen == sizeof(DHDEV_TALKFORMAT_LIST)))
        {
            show_error("query talk format failed, error = 0x%x\n", CLIENT_GetLastError());
            if(talk->frm)
            {
                nmp_dealloc(talk->frm, sizeof(frame_t) + MAX_ENCODE_BUFFER_LEN);
                talk->frm = NULL;
            }
            return -1;
        }

        ret = set_audio_attr(info, &cur_talk_mode);
        if(ret != 0)
        {
            if(talk->frm)
            {
                nmp_dealloc(talk->frm, sizeof(frame_t) + MAX_ENCODE_BUFFER_LEN);
                talk->frm = NULL;
            }
            return -1;
        }
        // 设置服务器模式语音对讲模式
        bSuccess = CLIENT_SetDeviceMode(dah_get_user_id(&dah_srv->parm), DH_TALK_ENCODE_TYPE, &cur_talk_mode);
        bSuccess = CLIENT_SetDeviceMode(dah_get_user_id(&dah_srv->parm), DH_TALK_SERVER_MODE, NULL);

        //  开始对讲
        talk->talk_handle = CLIENT_StartTalkEx(dah_get_user_id(&dah_srv->parm), 
            				(pfAudioDataCallBack)dah_audio_data_callback, (LDWORD)talk);
        if(0 != talk->talk_handle)
        {
            show_info("Start talk success! talk_handle = %d\n", (int)talk->talk_handle);
        }
        else
        {
            show_error("Start talk failed!, error = 0x%x, user_id = %d\n", 
                CLIENT_GetLastError(), dah_get_user_id(&dah_srv->parm));
            if(talk->frm)
            {
                nmp_dealloc(talk->frm, sizeof(frame_t) + MAX_ENCODE_BUFFER_LEN);
                talk->frm = NULL;
            }
            return -1;
        }
    }

    return 0; 
}


static int dah_talk_close(struct service *srv, talk_info_t *talk)
{
    int ret = -1;
    dah_service_t *dah_srv;

    NMP_ASSERT(srv && talk);
    if(talk->frm)
    {
        nmp_dealloc(talk->frm, sizeof(frame_t) + MAX_ENCODE_BUFFER_LEN);
        talk->frm = NULL;
    }

    dah_srv = (dah_service_t*)srv;
    if (CLIENT_StopTalkEx(talk->talk_handle))
    {
        CLIENT_ReleaseAudioEncode();
        ret = 0;
    }
    else
    {
        show_error("------------CLIENT_StopTalkEx(0x%x)\n", CLIENT_GetLastError());
    }

    return ret;
}

static int dah_talk_recv(struct service *srv, talk_info_t *talk, char *data, int len, media_info_t *info)
{
    int ret = -1;
    dah_service_t *dah_srv;
    int send_count = 0;
    unsigned char out_buffer[MAX_ENCODE_BUFFER_LEN];
    unsigned char in_buffer[MAX_ENCODE_LEN];
    unsigned int out_buffer_len = MAX_ENCODE_BUFFER_LEN;
    char pcm_data[MAX_ENCODE_BUFFER_LEN];
    int out_pcm_data_len = 0;
    unsigned int in_data_len = 0;
    frame_t *frm = NULL;

    NMP_ASSERT(srv && talk && data);

    frm = (talk_frame_hdr_t*)data;
    if(ntohl(frm->magic) != TALK_MAGIC)
    {
        show_warn("magic[0x%x] error", ntohl(frm->magic));
        return -1;
    }
    in_data_len = ntohl(frm->frame_length);
    memcpy(in_buffer, frm->frame_data, in_data_len);
    //memset(pcm_data, 0, MAX_ENCODE_BUFFER_LEN);

    if(info->attr.encode_type == 0)
    {
        memcpy(pcm_data, in_buffer, in_data_len);
        out_pcm_data_len = in_data_len;
    }
    else if(info->attr.encode_type == 1)
    {
        g711a_Decode2(in_buffer, pcm_data, (int)in_data_len, &out_pcm_data_len);
    }
    else if(info->attr.encode_type == 2)
    {
        g711u_Decode(in_buffer, pcm_data, (int)in_data_len, &out_pcm_data_len);
    }

    dah_srv = (dah_service_t*)srv;
    if( in_data_len <= MAX_ENCODE_LEN)
    {
        //memset(out_buffer, 0, MAX_ENCODE_BUFFER_LEN);
        ret = CLIENT_AudioEncode(0, (unsigned char*)pcm_data, 
                (unsigned int*)&out_pcm_data_len, out_buffer, &out_buffer_len);
        if(ret != 0)
        {
            show_warn("CLIENT_AudioEncode error ret = 0x%x in_data_len = %d out_buffer_len = %d\n",
                ret, in_data_len, out_buffer_len);
            return -1;  
        }
        send_count = CLIENT_TalkSendData(talk->talk_handle, (char*)out_buffer, out_buffer_len);
        if(send_count < 0)          
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }

    return ret;
}

struct talk_opt dah_talk_opt = 
{
    dah_talk_open,
    dah_talk_close,
    dah_talk_recv
};

