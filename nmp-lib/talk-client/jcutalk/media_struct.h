
#ifndef __MEDIA_STRUCT_H__
#define __MEDIA_STRUCT_H__

#ifdef WIN32
typedef signed char int8_t;
typedef unsigned char   uint8_t;
typedef short  int16_t;
typedef unsigned short  uint16_t;
typedef int  int32_t;
typedef unsigned   uint32_t;
typedef long long  int64_t;
typedef unsigned long long   uint64_t;
#else
	#include "stdint.h"
#endif

enum 
{
    SAMPLE_8K, //8k
    SAMPLE_16K, //16k
    SAMPLE_32K, //32;
};

enum
{
    ENC_PCM,
    ENC_G711A,
    ENC_G711U,
    ENC_G726
};

#pragma pack(push, 1)

typedef struct _talk_audio_attr
{    
	uint8_t		samples_per_sec;	    //采样率		0--8k 1--16k 2-32k
	uint8_t		encode_type;		    //音频编码格式  0--pcm 1-g711a 2-g711u 3--g726
	uint8_t		audio_channel;	        //通道数		暂只支持1	
	uint8_t     audio_bits;		     	//位数			16bit
}talk_audio_attr_t;

typedef struct _talk_frame_hdr
{
    uint32_t    magic;                  //0x33222233  
    uint32_t    frame_num;              
    uint32_t    pts;
    uint32_t    frame_length;
    uint8_t     frame_data[0];          // audio data
}talk_frame_hdr_t;

typedef struct _talk_req
{
    uint32_t    magic;                  //0x33222233 
    uint8_t     user[64];				//admin
    uint8_t     psw[64];				//admin
    talk_audio_attr_t   attr;
    uint32_t    res;                    //响应 0: 成功 ~0: 失败
}talk_req_t;

typedef struct _proxy_talk_req
{
    uint32_t    magic;                  //0x33222233
    uint8_t     pu_id[32];              //pu_id
    uint32_t    channel;                //设备通道号
    talk_req_t  talk_req;
}proxy_talk_req_t;

typedef struct _media_info
{    
    uint8_t     pu_id[32];              //pu_id
    uint32_t    channel;                //设备通道号
    talk_audio_attr_t attr;             // 设备音频属性
}media_info_t;

#pragma pack(pop)

#define TALK_MAGIC   0x33222233
#define TALK_PORT    3322

#endif
