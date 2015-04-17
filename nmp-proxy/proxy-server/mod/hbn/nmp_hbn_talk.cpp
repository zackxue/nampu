
#include "HBNetSDK.h"

#include "alaw_encoder.h"

#include "nmp_hbn_talk.h"
#include "nmp_hbn_service.h"



#define MAX_ENCODE_LEN              (2 * 1024)

static int 
hbn_send_audio_data(talk_handle_t *hdl, char *buffer, int len, frame_t *hbn_frm)
{

    if(hbn_frm)
    {
        memset(hbn_frm, 0, sizeof(frame_t) + MAX_ENCODE_BUFFER_LEN);
        
        hbn_frm->magic = htonl(TALK_MAGIC);
        hbn_frm->frame_length = htonl(len);
        hbn_frm->pts = 0;
        hbn_frm->frame_num = 0;
        memcpy(hbn_frm->frame_data, buffer, len);
        send_talk_data(hdl, hbn_frm);
        return 0;
    }
    return -1;
}

static void 
hbn_audio_data_callback(long  lVoiceComHandle, LPHB_NET_VOICECOMCALLBACKDATA  lpData) 
{
	printf("----------------dwDataSize = %d, dwAudioFlag = %d\n", 
			(int)lpData->dwDataSize, (int)lpData->dwAudioFlag);
	if(lpData->dwDataSize < MAX_ENCODE_BUFFER_LEN)
	{
		talk_info_t *talk = (talk_info_t*)lpData->pContext;
		hbn_send_audio_data(talk->hdl, lpData->pBuffer, lpData->dwDataSize, talk->frm);
	}
}


static int 
hbn_talk_open(struct service *srv, talk_info_t *talk, 
        talk_handle_t *hdl, media_info_t *info)
{
    hbn_service_t *hbn_srv;

    NMP_ASSERT(srv && talk);


    hbn_srv = (hbn_service_t*)srv;
    talk->handle = hbn_get_user_id(&hbn_srv->parm);
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
	
    if (HBN_LOGOUT == hbn_get_user_id(&hbn_srv->parm))
    {
        show_warn("hbn device is disconnected.\n");
    }
    else
    {
		//  ¿ªÊ¼¶Ô½²
		LPHB_NET_VOICECOMPARAM  lpParam = (LPHB_NET_VOICECOMPARAM)nmp_alloc0(sizeof(HB_NET_VOICECOMPARAM));
		lpParam->dwSize = sizeof(HB_NET_VOICECOMPARAM);
		lpParam->pfnCallback = hbn_audio_data_callback;
		lpParam->pContext = talk;
		talk->pri_data = lpParam;
		talk->talk_handle = HB_NET_StartVoiceComMR(hbn_get_user_id(&hbn_srv->parm), lpParam); 
		if(-1 != talk->talk_handle)
		{
		    show_info("Start talk success! talk_handle = %d\n", (int)talk->talk_handle);
		}
		else
		{
		    show_debug("Start talk failed!, error = %d, user_id = %d\n", 
		        	(int)HB_NET_GetLastError(), (int )hbn_get_user_id(&hbn_srv->parm));
		    if(talk->frm)
		    {
		        nmp_dealloc(talk->frm, sizeof(frame_t) + MAX_ENCODE_BUFFER_LEN);
		        talk->frm = NULL;
		    }
			
			if(lpParam)
			{
				nmp_dealloc(lpParam, sizeof(HB_NET_VOICECOMPARAM));
				lpParam = NULL;
				talk->pri_data = NULL;
			}
		    return -1;
		}
    }

    return 0; 
}


static int hbn_talk_close(struct service *srv, talk_info_t *talk)
{
    int ret = -1;
    hbn_service_t *hbn_srv;

    NMP_ASSERT(srv && talk);
	
    if(talk->frm)
    {
        nmp_dealloc(talk->frm, sizeof(frame_t) + MAX_ENCODE_BUFFER_LEN);
        talk->frm = NULL;
    }

	if(talk->pri_data)
	{
		nmp_dealloc(talk->pri_data, sizeof(HB_NET_VOICECOMPARAM));
		talk->pri_data = NULL;
	}
	
    hbn_srv = (hbn_service_t*)srv;
    if (HB_NET_StopVoiceCom(talk->talk_handle))
    {
        ret = 0;
    }
    else
    {
        show_debug("------------HB_NET_StopVoiceCom (%d)\n", (int)HB_NET_GetLastError());
    }

    return ret;
}

static int hbn_talk_recv(struct service *srv, talk_info_t *talk, char *data, int len, media_info_t *info)
{
    int ret = -1;
    hbn_service_t *hbn_srv;
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
printf("====================hbn_talk_recv len = %d\n", in_data_len);
    hbn_srv = (hbn_service_t*)srv;	
	ret = HB_NET_VoiceComSendData(talk->talk_handle, (char *)frm->frame_data, in_data_len);
	if(ret == FALSE)
	{
		show_debug("HB_NET_VoiceComSendData!, error = %d, user_id = %d\n", 
		        (int)HB_NET_GetLastError(), (int )hbn_get_user_id(&hbn_srv->parm));
		return -1;
	}

    return 0;
}


struct talk_opt hbn_talk_opt = 
{
    hbn_talk_open,
    hbn_talk_close,
    hbn_talk_recv
};

