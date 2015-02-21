/*
 *	author:	zyt
 *	time:	begin in 2012/8/07
 */
#ifndef __RTSP_CONTEXT_H__
#define __RTSP_CONTEXT_H__

#include "rtsp_defs.h"
#include "rtsp_msg.h"
#include "rtsp_mem.h"


#ifndef likely
#define likely(x)   __builtin_expect((x),1)
#endif

#ifndef unlikely
#define unlikely(x) __builtin_expect((x),0)
#endif


typedef struct
{
	int read_ahead;

	char *rcv_buffer;				/* rtsp信令接收缓冲区 */
	size_t rcv_buffer_size; 		/* 缓冲区总长度 */
	size_t rcv_buffer_offset;		/* 有效数据起始 */
	size_t rcv_buffer_bytes;		/* 有效数据长度 */
}rtsp_context;


/* a structure for constructing RTSPMessages */
typedef struct		/* code from gstreamer */
{
	int state;
	RTSP_RESULT status;
	uint8 buffer[4096];
	uint32 offset;

	uint32 line;
	uint8 *body_data;	//所指内存大小为body_len + 1
	int body_len;
}rtsp_builder;


void build_init(rtsp_builder *builder);

void build_reset(rtsp_builder *builder);

rtsp_context *rtsp_ctx_new(size_t rb_size);

void rtsp_ctx_free(rtsp_context *rtsp_ctx);

RTSP_RESULT rtsp_ctx_recv(rtsp_context *rtsp_ctx, int rfd, rtsp_builder *builder, 
	rtsp_message *message, int *err);


#endif

