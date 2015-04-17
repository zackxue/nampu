#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <sys/time.h>
#include <unistd.h>

#include "HCNetSDK.h"


#include "alaw_encoder.h"

#include "nmp_hik_talk.h"
#include "nmp_hik_service.h"


//#define MAX_ENCODE_BUFFER_LEN		(1 * 1024)

#define G722FRAMEDATALEN		80
#define G722PCMDATALEN			1280

/*********************talk operation****************************/
static int hik_send_audio_data(talk_handle_t *hdl, char *buffer, int len, frame_t *hik_frm)
{
	if(!hik_frm)
	{
		show_debug("hik_frm paramer is NULL\n");
		return -1;
	}
	//memset(hik_frm, 0, sizeof(frame_t) + MAX_ENCODE_BUFFER_LEN);
	
	hik_frm->magic = htonl(TALK_MAGIC);
	hik_frm->frame_length = htonl(len);
	hik_frm->pts = 0;
	hik_frm->frame_num = 0;

	memcpy(hik_frm->frame_data, buffer, len);

	send_talk_data(hdl, hik_frm);
	return 0;
}

static void hik_audio_data_callback(LONG talk_handle, char *recv_data_buf, 
										DWORD buf_size, BYTE audio_flag, void* user)
{
	struct talk_info *talk = (struct talk_info *)user;
	unsigned int decode_len = 0;
	unsigned char out_buff[MAX_ENCODE_BUFFER_LEN] = {0};
	int ret;
	if(!talk)
	{
		return;
	}

	talk_handle_t *hdl = talk->hdl;
	if(1 == audio_flag)//设备发过来的数据
	{		
		while(decode_len < buf_size)
		{
			ret = NET_DVR_DecodeG722Frame(talk->init_g722_decoder, (unsigned char *)recv_data_buf + decode_len, out_buff);
			if(!ret)
			{
				printf("***ret = %d****err = %d\n", ret, NET_DVR_GetLastError());
			}
			//printf("************************hik send data, audio_flag[%d], buf_size[%d]\n", audio_flag, buf_size);
		
			hik_send_audio_data(hdl, (char *)(unsigned char *)out_buff, G722PCMDATALEN, talk->frm);
			decode_len += G722FRAMEDATALEN;
		}
	}
	else
	{
		printf("------------------------hik_audio_data_callback, audio_flag[%d]\n", audio_flag);
	}
}


static int hik_get_audio_type(media_info_t *info, int *type)
{
	if(!info)
		return -1;
	switch(info->attr.encode_type)
	{
		case ENC_PCM:
			return -1;
			break;
		case ENC_G711A:
			*type = 2;
			break;
		case ENC_G711U:
			*type = 1;
		case ENC_G726:
			return -1;
			break;
	}
	return 0;
}


static int hik_talk_open(struct service *srv, struct talk_info *talk, 
							talk_handle_t *hdl, media_info_t *info)
{
    struct hik_service *hik_srv;
	long talk_hanle;
	
	NMP_ASSERT(srv && talk);

	if(talk->frm)
	{
		free(talk->frm);
		talk->frm = NULL;
	}
	talk->frm = (frame_t *)malloc(sizeof(frame_t) + MAX_ENCODE_BUFFER_LEN);
	if(!talk->frm)
	{
		return -1;
	}

	hik_srv = (hik_service_t *)srv;
	talk->handle = hik_get_user_id(&hik_srv->parm);
	
    if (HIK_LOGOUT == hik_get_user_id(&hik_srv->parm))
    {
        show_debug("dah device is disconnected.\n");
    }
	else
	{
		talk->init_g722_encoder = NET_DVR_InitG722Encoder();
		talk->init_g722_decoder = NET_DVR_InitG722Decoder();
		if(hik_srv->init_g722_decoder == NULL && hik_srv->init_g722_encoder)
		{		
			show_debug("NET_DVR_InitG722Encoder error[%d]\n", NET_DVR_GetLastError());
			if(talk->frm)
			{
				free(talk->frm);
				talk->frm = NULL;
			}
			return -1;
		}

		talk_hanle = NET_DVR_StartVoiceCom_MR_V30(talk->handle, info->channel + 1, 
													hik_audio_data_callback, (void *)talk);
    	if (talk_hanle < 0)
    	{
        	show_error("pyd---NET_DVR_StartVoiceCom_MR_V30 fail %d\n", NET_DVR_GetLastError());
			if(talk->frm)
			{
				free(talk->frm);
				talk->frm = NULL;
			}

        	return -1;
    	}
		show_info("**********hik talk->handle = %d, talk_handle = %ld\n", talk->handle, talk_hanle);
		talk->talk_handle = talk_hanle;
	}
    
    return 0; 
}


static int hik_talk_close(struct service *srv, struct talk_info *talk)
{
	struct hik_service *hik_srv;
	
	NMP_ASSERT(srv && talk);
	if(talk->frm)
	{
		free(talk->frm);
		talk->frm = NULL;
	}

	hik_srv = (hik_service_t *)srv;	
	
	if (NET_DVR_StopVoiceCom(talk->talk_handle) == FALSE)
	{
		show_error("pyd------------NET_DVR_StopVoiceCom fail[handle %ld] error[%d]!\n",
				talk->talk_handle, NET_DVR_GetLastError());
		return -1;
	}
	
	if(talk->init_g722_encoder)
	{
		NET_DVR_ReleaseG722Encoder(talk->init_g722_encoder);
		NET_DVR_ReleaseG722Decoder(talk->init_g722_decoder);
		talk->init_g722_encoder = NULL;
		talk->init_g722_decoder = NULL;
	}
	else
	{
		show_error("pyd-----------NET_DVR_ReleaseG722Decoder fail!\n");
		return -1;
	}
	
	return 0;
}



#define MAX_ENCODE_LEN	(2 * 1024)
static int hik_talk_recv(struct service *srv, struct talk_info *talk, char *data, 
							int len, media_info_t *info)
{
	int ret = -1;
	struct hik_service *hik_srv;
	unsigned char out_buffer[MAX_ENCODE_BUFFER_LEN];
	unsigned int in_data_len = 0;
	frame_t *frm = NULL;

	unsigned int itype;
	BOOL flag;
	NMP_ASSERT(srv && talk && data);
	
	frm = (talk_frame_hdr_t *)data;
	if(ntohl(frm->magic) != TALK_MAGIC)
	{
		show_warn("magic[0x%x] error", ntohl(frm->magic));
		return -1;
	}

	in_data_len = ntohl(frm->frame_length);

	if(info->attr.encode_type == 1)
	{
		//g711a_Decode2(in_buffer, pcm_data, (int)in_data_len, &out_pcm_data_len);
		itype = 1;
	}
	else if(info->attr.encode_type == 2)
	{
		//g711u_Decode(in_buffer, pcm_data, (int)in_data_len, &out_pcm_data_len);
		itype = 0;
	}

	hik_srv = (hik_service_t *)srv;

	flag = NET_DVR_EncodeG722Frame(talk->init_g722_encoder, frm->frame_data, out_buffer);
	if(flag == FALSE)
	{
		show_error("-------NET_DVR_EncodeG722Frame error[%d]\n", NET_DVR_GetLastError());
		
		return -1;	
	}

	ret = NET_DVR_VoiceComSendData(talk->talk_handle, (char *)out_buffer, G722FRAMEDATALEN);
	if(ret == FALSE)
	{
		show_error("----------NET_DVR_VoiceComSendData error[%d]\n", NET_DVR_GetLastError());
		return -1;
	}
	//usleep(10*1000);

	return ret;
}

struct talk_opt hik_talk_opt = 
{
	hik_talk_open,
	hik_talk_close,
	hik_talk_recv
};

