/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/

#ifndef __TINY_RAIN_RTP_PARSER_H__
#define __TINY_RAIN_RTP_PARSER_H__

#include "proto_parser.h"

BEGIN_NAMESPACE

#define RTP_MSG_MT		MT_02

int32_t register_rtp_msg();

proto_parser *alloc_rtp_parser( void );
void free_rtp_parser(proto_parser *parser);

END_NAMESPACE

#endif	//__TINY_RAIN_RTP_PARSER_H__
