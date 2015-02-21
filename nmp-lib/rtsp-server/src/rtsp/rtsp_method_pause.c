/* *
 * This file is part of rtsp-server
 *
 * Copyright (C) 2012 fangyi <fangyi@szjxj.net>
 * See COPYING for more details
 *
 * rtsp-server is a stream transporting and controlling module using
 * RTP/RTCP/RTSP protocols, which is designed for jxj platform servers
 * such as mss, proxy-server etc.
 *
 * */

#include "rtsp.h"
#include "rc_log.h"
#include "rtp.h"

static __inline__ void
rtsp_do_pause(RTSP_session *rtsp_s)
{
    RTSP_Range *range;

	if (rtsp_s->cur_state != RTSP_SERVER_PLAYING)
		return;

	if (r_pause(rtsp_s->resource))
		return;

	range = g_queue_peek_head(rtsp_s->play_requests);
    range->begin_time = ev_time() -range->playback_time;
    range->playback_time = -0.1;

    rtp_session_rtps_pause(rtsp_s->rtp_sessions);
    rtsp_s->cur_state = RTSP_SERVER_READY;

	//@{TODO: Session timeout timer should be stopped ??}
}


static __inline__ void
rtsp_send_pause_response(RTSP_Client *client, RTSP_session *rtsp_s,
	RTSP_Ps *state)
{
	gst_rtsp_message_init_response(state->response, GST_RTSP_STS_OK,
		gst_rtsp_status_as_text(GST_RTSP_STS_OK), state->request);
	rtsp_client_send_response(client, rtsp_s, state->response);
}


void
rtsp_handle_pause_request(RTSP_Client *client, RTSP_Ps *state)
{
	GstRTSPResult res;
	gchar *session_id;
	RTSP_session *rtsp_s;

	res = gst_rtsp_message_get_header(state->request,
		GST_RTSP_HDR_SESSION, &session_id, 0);
	if (res != GST_RTSP_OK)
	{
		rtsp_client_send_generic_response(
			client,
			GST_RTSP_STS_SESSION_NOT_FOUND,
			state
		);
		return;
	}

	rtsp_s = rtsp_client_session_find(client, session_id);
	if (G_UNLIKELY(!rtsp_s))
	{
		rtsp_client_send_generic_response(
			client,
			GST_RTSP_STS_SESSION_NOT_FOUND,
			state
		);
		return;
	}	

	g_mutex_lock(rtsp_s->mutex);

	if (rtsp_s->killed)
		goto pause_end;

	rtsp_do_pause(rtsp_s);
	rtsp_send_pause_response(client, rtsp_s, state);

pause_end:
	g_mutex_unlock(rtsp_s->mutex);
	rtsp_session_unref(rtsp_s);
}

//:~ End
