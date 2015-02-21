/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/

#ifndef __TINY_RAIN_RTSP_PARSER_H__
#define __TINY_RAIN_RTSP_PARSER_H__

#include "proto_parser.h"

BEGIN_NAMESPACE

#define RTSP_MSG_MT		MT_01

int32_t register_rtsp_msg();

proto_parser *alloc_rtsp_parser(uint32_t rb_size);
void free_rtsp_parser(proto_parser *parser);

END_NAMESPACE

#endif	//__TINY_RAIN_RTSP_PARSER_H__
