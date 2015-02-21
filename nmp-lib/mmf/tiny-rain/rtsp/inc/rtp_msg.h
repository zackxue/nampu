/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/


#ifndef __TINY_RAIN_RTP_MSG_H__
#define __TINY_RAIN_RTP_MSG_H__

#include "msg.h"
#include "h264_filter.h"

BEGIN_NAMESPACE

#define RTP_MTU			(9*1024)
#define RTP_MSG_MT		MT_02

int32_t register_rtp_msg( void );

static __inline__ msg*
rtp_mem_s_to_msg(rtp_mem_t *rm)
{
	return SET_MSG_TYPE(rm, RTP_MSG_MT);
}

static __inline__ rtp_mem_t*
rtp_msg_to_rtp_mem_s(msg *m)
{
	msg *_m =  GET_MSG_ADDR(m);
	BUG_ON(_m == m);
	return (rtp_mem_t*)_m;
}

END_NAMESPACE

#endif	//__TINY_RAIN_RTP_MSG_H__
