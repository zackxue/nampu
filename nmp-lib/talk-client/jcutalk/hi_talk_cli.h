#ifndef __hi_talk_cli_h__
#define __hi_talk_cli_h__

#include "media_struct.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef struct _HI_TALK_AUDIO_ATTR_
{    
	unsigned char		u8AudioSamples;	    //采样率		0--8k 1--16k 2-32k
	unsigned char		u8EncodeType;		//音频编码格式0--pcm 1-g711a 2-g711u 3--g726
	unsigned char		u8AudioChannels;	//通道数		暂只支持1	
	unsigned char		u8AudioBits;		//位数			16bit
}HI_TALK_AUDIO_ATTR, *LPHI_TALK_AUDIO_ATTR;

typedef struct _HI_TALK_FRMAE_HDR_
{
    unsigned long       u32Magic;           //0x33222233  
    unsigned long       u32FrameNo;             
    unsigned long       u32Pts;
    unsigned long       u32Len;
}HI_TALK_FRMAE_HDR, *LPHI_TALK_FRMAE_HDR;


typedef struct _HI_TALK_REQ_
{
    unsigned long   u32Magic;               //0x33222233 
    char            szUsr[64];				//admin
    char            szPsw[64];				//admin
    HI_TALK_AUDIO_ATTR  stAttr;
    long                s32Res;             //响应 0: 成功 ~0: 失败
}HI_TALK_REQ, *LPHI_TALK_REQ;

typedef struct _PROXY_TALK_REQ_
{
	unsigned long u32Magic;				//0x33222233
	char          szPuId[32];			//pd_id
	unsigned long channel;              // dev channel num
	HI_TALK_REQ   stTalkReq;
}PROXY_TALK_REQ_T, *LPPROXY_TALK_REQ_T;

#define HI_TALK_MAGIC   0x33222233
#define HI_TALK_PORT    "3322"


//////////////////////////////////////

typedef void hi_talk_cli_t;

typedef struct talk_parm_s {
    int (*recv)(hi_talk_cli_t *hdl, HI_TALK_FRMAE_HDR *frm, void *ctx);
    int (*start)(hi_talk_cli_t *hdl, void *ctx);
    int (*stop)(hi_talk_cli_t *hdl, void *ctx);
}talk_parm_t; 

int hi_talk_cli_init(talk_parm_t *parm);
int hi_talk_cli_uninit(void);

hi_talk_cli_t*
    hi_talk_cli_open(char *ip
                        , unsigned short port
                        , void *ctx, media_info_t *mi);                            
int hi_talk_cli_close(hi_talk_cli_t *hdl);
int hi_talk_cli_send(hi_talk_cli_t *hdl, HI_TALK_FRMAE_HDR *frm);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __hi_talk_cli_h__ */

