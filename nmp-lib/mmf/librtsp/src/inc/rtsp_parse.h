/*
 *	author:	zyt
 *	time:	begin in 2012/8/14
 */
#ifndef __RTSP_PARSE_H__
#define __RTSP_PARSE_H__

#include "rtsp_ctx.h"
#include "rtsp_transport.h"
#include "rtsp_sdp.h"
#include "rtsp_mem.h"


typedef struct
{
	rtsp_context		*ctx;
	rtsp_builder		builder;
	rtsp_message		message;
	mbool			msg_complete;
}rtsp_parser;

rtsp_parser *rtsp_parser_new(size_t rb_size);

void rtsp_parse_free(rtsp_parser *parser);

RTSP_RESULT rtsp_parser_recv(rtsp_parser *parser, int fd, rtsp_message **msg, 
	int *err);


#endif 

