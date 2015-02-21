/*
 * tiny-rain:
 * (c) Author:
 *
 * A simple multi-media server application framework.
*/

#ifndef __TINY_RAIN_RTSP_IMPL_H__
#define __TINY_RAIN_RTSP_IMPL_H__

#include "media.h"
#include "rtsp_client.h"
#include "rtsp_parse.h"

BEGIN_NAMESPACE

typedef struct __rtsp_ts_client rtsp_ts_client;	/* thread-safe client */
struct __rtsp_ts_client
{
	rtsp_client __super;
	int32_t killed;
	LOCK_T lock;
};

typedef struct __rtsp_wait_block rtsp_wait_block;
struct __rtsp_wait_block
{
	rtsp_client *rc;
	media *orig_media;
	rtsp_message *req_msg;
	int32_t i_track;
};

client *rtsp_impl_client_new(uint32_t factory, void *sock);
client *rtsp_impl_ts_client_new(uint32_t factory, void *sock);
int32_t rtsp_impl_ts_client_killed(rtsp_ts_client *rtc);

int32_t rtsp_impl_parse_url(rtsp_client *rc, const char *url,
	media_uri *mrl, int32_t *track, int32_t media_seq);

RTSP_STATUS_CODE rtsp_impl_trans_status_code(int32_t request, int32_t err);
int32_t rtsp_impl_queue_request(rtsp_client *rc, rtsp_message *req, int32_t *track,
	int32_t wt, pq_fun fun);

rtsp_message *rtsp_impl_new_generic_response(rtsp_message *req,
	RTSP_STATUS_CODE code);
int32_t rtsp_impl_send_message(rtsp_client *rc, rtsp_message *message);

rtsp_wait_block *rtsp_wait_block_new( void );
void rtsp_wait_block_free(rtsp_wait_block *rwb);

END_NAMESPACE

#endif	//__TINY_RAIN_RTSP_IMPL_H__
