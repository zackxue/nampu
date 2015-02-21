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

#include <math.h>
#include <string.h>
#include "rtsp.h"
#include "rc_log.h"

#include "media.h"
#include "rtp.h"
#include "mediautils.h"

#define PRECISION .0001
#define double_equal(v1, v2) (fabs((v1) - (v2)) < PRECISION)


static __inline__ void
rtsp_session_range_append(RTSP_session *rtsp_s, RTSP_Range *range)
{
    g_queue_push_tail(rtsp_s->play_requests, range);
}


static void
rtsp_range_free(gpointer element, gpointer user_data)
{
    g_slice_free(RTSP_Range, element);
}


void
rtsp_session_ranges_free(RTSP_session *rtsp_s)
{
    g_queue_foreach(rtsp_s->play_requests, rtsp_range_free, NULL);
    g_queue_clear(rtsp_s->play_requests);
}


static __inline__ gint
parse_rtsp_range_string(gchar *range_str, RTSP_Range *range)
{
	gchar *range_copy, *type, *start, *end;

	range_copy = g_strdup(range_str);
	type = strstr(range_copy, "npt=");
	if (type)
	{
		start = type + 4;
		end = strstr(start, "-");
		if (end)
		{
			*end = 0;
			++end;
		};

		if (start && *start)
			sscanf(start, "%lf", &range->begin_time);

		if (end && *end)
			sscanf(start, "%lf", &range->end_time);
	}

	g_free(range_copy);
	return 0;
}


static __inline__ GstRTSPStatusCode
rtsp_parse_play_range(RTSP_session *rtsp_s, RTSP_Ps *state)
{
	gchar *range_str = NULL;
	RTSP_Range *range;
	GstRTSPResult res;

	static RTSP_Range default_range =
	{
		.begin_time		= .0,
		.end_time		= -0.1,
		.playback_time	= -0.1
	};

	res = gst_rtsp_message_get_header(state->request,
		GST_RTSP_HDR_RANGE, &range_str, 0);
	if (res != GST_RTSP_OK)
	{
		range = g_queue_peek_head(rtsp_s->play_requests);
		if (range)
		{
			range->playback_time = ev_time();
			return GST_RTSP_STS_OK;
		}
	}

    range = g_slice_dup(RTSP_Range, &default_range);
	if (range_str)
	{
		if (parse_rtsp_range_string(range_str, range))
		{
			g_slice_free(RTSP_Range, range);
			return GST_RTSP_STS_INVALID_RANGE;
		}

		if (!rtsp_s->resource->info->seekable &&
			!double_equal(range->begin_time, .0) &&
			!double_equal(range->end_time, -0.1))
		{
		    g_slice_free(RTSP_Range, range);
		    return GST_RTSP_STS_HEADER_FIELD_NOT_VALID_FOR_RESOURCE;
		}
	}

	if (range->end_time < 0 ||
	    range->end_time > rtsp_s->resource->info->duration)
	{
	    range->end_time = rtsp_s->resource->info->duration;
	}

	if (range->playback_time < 0)
	    range->playback_time = ev_time();

	if (rtsp_s->cur_state != RTSP_SERVER_PLAYING)
	    rtsp_session_ranges_free(rtsp_s);

    rtsp_session_range_append(rtsp_s, range);

	rc_log(
		RC_LOG_VERBOSE,
	    "PLAY [%f]: %f %f %f\n", ev_time(),
	    range->begin_time, range->end_time, range->playback_time
	);

    return GST_RTSP_STS_OK;
}


static void
rtp_session_send_play_reply(gpointer element, gpointer user_data)
{
  GString *str = (GString *)user_data;
  RTP_session *p = (RTP_session *)element;
  Track *t = p->track;

  g_string_append_printf(str, "url=%s;seq=%u",
                         p->uri,
                         p->start_seq);

	if (t->properties.media_source != MS_live)
	{
		g_string_append_printf(
			str,
			";rtptime=%u",
			p->start_rtptime
		);
		
		g_string_append(str, ",");
	}
}


static __inline__ GstRTSPStatusCode
rtsp_do_play(RTSP_session *rtsp_s)
{
    RTSP_Range *range = g_queue_peek_head(rtsp_s->play_requests);

	if (rtsp_s->resource->info->seekable &&
	     r_seek(rtsp_s->resource, range->begin_time))
	{
	    return GST_RTSP_STS_INVALID_RANGE;
	}

    rtsp_s->cur_state = RTSP_SERVER_PLAYING;

	if (rtsp_s->rtp_sessions &&
	     ((RTP_session*)(rtsp_s->rtp_sessions->data))->multicast)
	{
	    return GST_RTSP_STS_OK;
	}

	rtsp_s->started = 1;

	rc_log(
		RC_LOG_VERBOSE,
		"[%f] resuming with parameters %f %f %f\n",
	    ev_time(),
	    range->begin_time, range->end_time, range->playback_time
	);

    rtp_session_rtps_resume(rtsp_s->rtp_sessions, range);

	if (r_play(rtsp_s->resource))
		return GST_RTSP_STS_INVALID_RANGE;

    return GST_RTSP_STS_OK;	
}


static __inline__ void
rtsp_send_play_response(RTSP_Client *client, RTSP_session *rtsp_s,
	RTSP_Ps *state)
{
	GString *range_str, *rtp_info;
	RTSP_Range *range;
	gchar *str;

	range = g_queue_peek_head(rtsp_s->play_requests);
	range_str = g_string_new("npt=");
	rtp_info = g_string_new("");

    if (range->begin_time >= 0)
        g_string_append_printf(range_str, "%f", range->begin_time);

    g_string_append(range_str, "-");

    if (range->end_time > 0)
      g_string_append_printf(range_str, "%f", range->end_time);

	gst_rtsp_message_init_response(state->response, GST_RTSP_STS_OK,
		gst_rtsp_status_as_text(GST_RTSP_STS_OK), state->request);

	str = g_string_free(range_str, FALSE);
	gst_rtsp_message_add_header(state->response, GST_RTSP_HDR_RANGE, str);
	g_free(str);

	g_slist_foreach(rtsp_s->rtp_sessions, rtp_session_send_play_reply, rtp_info);
	g_string_truncate(rtp_info, rtp_info->len-1);

	str = g_string_free(rtp_info, FALSE);
	gst_rtsp_message_add_header(state->response, GST_RTSP_HDR_RTP_INFO, str);
	g_free(str);

	rtsp_client_send_response(client, rtsp_s, state->response);
}


void
rtsp_handle_play_request(RTSP_Client *client, RTSP_Ps *state)
{
	GstRTSPStatusCode code;
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

    rtsp_s->session_timer = 0;
	g_mutex_lock(rtsp_s->mutex);

	if (G_UNLIKELY(rtsp_s->killed))
	{
		code = GST_RTSP_STS_METHOD_NOT_VALID_IN_THIS_STATE;
		goto play_end;
	}

	code = rtsp_parse_play_range(rtsp_s, state);
	if (G_UNLIKELY(code != GST_RTSP_STS_OK))
	{
		goto play_end;
	}

	code = rtsp_do_play(rtsp_s);
	if (G_UNLIKELY(code != GST_RTSP_STS_OK))
	{
		goto play_end;
	}

	rtsp_send_play_response(client, rtsp_s, state);

play_end:
	g_mutex_unlock(rtsp_s->mutex);
	rtsp_session_unref(rtsp_s);

	if (code != GST_RTSP_STS_OK)
	{
		rtsp_client_send_generic_response(
			client,
			code,
			state
		);		
	}
}

//:~ End
