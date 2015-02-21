/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/

#ifndef __TINY_RAIN_RTP_SINKER_H__
#define __TINY_RAIN_RTP_SINKER_H__

#include "network_sinker.h"
#include "rtsp_client.h"

BEGIN_NAMESPACE

typedef struct __rtp_sinker rtp_sinker;
struct __rtp_sinker
{
	network_sinker __super;
	int32_t trans_mode;
	rtsp_client *interleaved;
};

rtp_sinker *rtp_sinker_new(sinker_param *sp);

END_NAMESPACE

#endif	//__TINY_RAIN_RTP_SINKER_H__
