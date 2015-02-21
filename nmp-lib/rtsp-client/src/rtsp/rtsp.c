/* *
 * This file is part of rtsp-client lib.
 * Copyright (C) 2012 by nampu
 *
 * */
 
#include <string.h>
#include <nmpevsched.h>
#include "loop_weight.h"
#include "media.h"
#include "rtsp.h"
#include "fnc_log.h"
#include "sock.h"
#include "rtsp_method.h"

#define RTSP_BUFFER_SIZE			4096
#define DEFAULT_SESSION_TIMEOUT		60

#define CONN_WAIT					3


static GstRTSPResult
rtsp_session_state_init(RTSP_client_session *rtsp_s, GstRTSPMethod method, 
	GstRTSPMessage *message)
{
	GstSDPMessage *sdp;
	guint8 *sdp_body;
	guint sdp_size, ntrack;
	RTSP_media *medium;
	gchar *content_base, *session_id;
	gchar *p_timeout;
	GstRTSPResult res;

	switch (method)
	{
	case GST_RTSP_DESCRIBE:
		if (rtsp_s->medium)
		{
			fnc_log(FNC_LOG_ERR, "[FC] Session %p already has medium binded.", rtsp_s);
			return GST_RTSP_ERROR;
		}

		if (gst_sdp_message_new(&sdp) != GST_SDP_OK)
		{
			fnc_log(FNC_LOG_ERR, "[FC] Create SDP message failed");	
			return GST_RTSP_ENOMEM;
		}

		gst_rtsp_message_get_body(message, &sdp_body, &sdp_size);	
		if (gst_sdp_message_parse_buffer(sdp_body, sdp_size, sdp) != GST_SDP_OK)
		{
			fnc_log(FNC_LOG_ERR,"[FC] Device DESCRIBE response, invalid SDP info");
			gst_sdp_message_free(sdp);
			return GST_RTSP_EPARSE;
		}

		ntrack = gst_sdp_message_medias_len(sdp);
		if (ntrack < 1)
		{
			fnc_log(FNC_LOG_ERR, "[FC] Media dosn't have stream!");
			gst_sdp_message_free(sdp);
			return GST_RTSP_ERROR;
		}

		printf("###################### Media has %d stream(s)!\n", ntrack );

		if (gst_rtsp_message_get_header(message, GST_RTSP_HDR_CONTENT_BASE,
			&content_base, 0) != GST_RTSP_OK)
		{
			fnc_log(FNC_LOG_ERR, "[FC] No 'content-base' in DESCRIBE response!");
			gst_sdp_message_free(sdp);
			return GST_RTSP_ERROR;	
		}

		medium = rtsp_media_create();
		medium->content_base = g_strdup(content_base);
		medium->ntrack = ntrack;
		medium->sdp = sdp;
		medium->request_itrack = 0;
		medium->rtsp_s = rtsp_s;
		rtsp_s->medium = medium;

		return rtsp_session_setup(rtsp_s);

	case GST_RTSP_SETUP:
		res = gst_rtsp_message_get_header(message, GST_RTSP_HDR_SESSION, &session_id, 0);
		if (res != GST_RTSP_OK)
		{
			fnc_log(FNC_LOG_ERR, "[FC] Server SETUP response, No 'Session:' field.");
			return GST_RTSP_ERROR;
		}

		if (!rtsp_s->session_id)
		{
			rtsp_s->session_id = g_strdup(session_id);
			strtok(rtsp_s->session_id, ";");
			rtsp_s->timeout = DEFAULT_SESSION_TIMEOUT;

			if ((p_timeout = (strstr(session_id, "timeout="))))
			{
				sscanf(p_timeout, "timeout=%d", &rtsp_s->timeout);
				if (rtsp_s->timeout <= 0)
					rtsp_s->timeout = DEFAULT_SESSION_TIMEOUT;
			}
		}
		else
		{
			if (strncmp(rtsp_s->session_id, session_id, strlen(rtsp_s->session_id)))
			{
				fnc_log(FNC_LOG_ERR, "[FC] SETUP response changed 'Session:' value.");
				return GST_RTSP_ERROR;				
			}
		}

		medium = rtsp_s->medium;
		if (medium->request_itrack >= medium->ntrack)
		{
			rtsp_s->state = RTSP_READY;
			if (rtsp_s->play_pending)
				return rtsp_session_play(rtsp_s);
			return GST_RTSP_OK;
		}
		else
		{
			return rtsp_session_setup(rtsp_s);
		}

		break;

	default:
		break;
	}

	return GST_RTSP_STS_INVALID;
}


static __inline__ gint
rtsp_session_interleaved_rcv(RTSP_client_session *rtsp_s, guint8 channel,
	guint8* data, guint size)
{
	if (rtsp_s->medium)
	{
		return rtsp_session_media_interleaved_rcv(rtsp_s->medium,
			channel, data, size);
	}
	return -EINVAL;
}


static GstRTSPResult
rtsp_session_parse_data(RTSP_client_session *rtsp_s, GstRTSPMessage *message)
{
	GstRTSPResult res;
	guint8 channel;
	guint8 *data;
	guint size;

	res = gst_rtsp_message_parse_data(message, &channel);
	if (res != GST_RTSP_OK)
		return res;

	gst_rtsp_message_steal_body(message, &data, &size);
	--size;

	if (data)
	{
		rtsp_session_interleaved_rcv(rtsp_s, channel, data, size);
		g_free(data);
	}

	return GST_RTSP_OK;
}


static GstRTSPResult
rtsp_session_state_ready(RTSP_client_session *rtsp_s, GstRTSPMethod method, 
	GstRTSPMessage *message)
{
	GstRTSPMsgType msg_type;
	g_assert(rtsp_s != NULL && message != NULL);

	msg_type = gst_rtsp_message_get_type(message);
	if (msg_type == GST_RTSP_MESSAGE_DATA)
	{
		return rtsp_session_parse_data(rtsp_s, message);
	}

	return GST_RTSP_OK;
}


static RTSP_state_fun rtsp_state_machine[STATES_NUM] =
{
	rtsp_session_state_init,
	rtsp_session_state_ready
};


GstRTSPResult
rtsp_sesseion_send_message(RTSP_client_session *rtsp_s,
	GstRTSPMessage *message)
{
	if (rtsp_s && rtsp_s->rtsp_watch)	/* no race with kill */
	{
		return gst_rtsp_watch_send_message_2(rtsp_s->rtsp_watch, message);
	}

	return GST_RTSP_EINVAL;
}


static GstRTSPResult
rtsp_session_handle_rtsp_message(RTSP_client_session *rtsp_s,
	GstRTSPMessage *message)
{
	GstRTSPStatusCode code;
	GstRTSPResult res;
	gchar *cseq;
	const gchar *reson;
	GstRTSPVersion version;
	GstRTSPMethod method;
	GstRTSPMsgType msg_type;
	g_assert(rtsp_s != NULL && message != NULL);

	msg_type = gst_rtsp_message_get_type(message);
	if (msg_type != GST_RTSP_MESSAGE_RESPONSE)
	{
		return rtsp_state_machine[rtsp_s->state](rtsp_s, GST_RTSP_INVALID, message);
	}

	res = gst_rtsp_message_parse_response(message, &code, &reson, &version);
	if (res != GST_RTSP_OK)
	{
		fnc_log(FNC_LOG_ERR, "[FC] Parse RTSP response failed.");
		return res;
	}

	if (code != GST_RTSP_STS_OK)
	{
		fnc_log(FNC_LOG_ERR, "[FC] RTSP response code != 200 OK.");
		return GST_RTSP_ERROR;
	}

	res = gst_rtsp_message_get_header(message, GST_RTSP_HDR_CSEQ, &cseq, 0);
	if (res != GST_RTSP_OK)
	{
		fnc_log(FNC_LOG_ERR, "[FC] Server RTSP response, No 'CSeq:' field.");
		return GST_RTSP_ERROR;
	}	

	if (atoi(cseq) != rtsp_s->wait_for.seq)
	{//fix me
		fnc_log(FNC_LOG_ERR, "[FC] Unexpected RTSP response.");
		return GST_RTSP_ERROR;		
	}

	method = rtsp_s->wait_for.method;

	return rtsp_state_machine[rtsp_s->state](rtsp_s, method, message);
}


static GstRTSPResult
rtsp_server_message_received(GstRtspWatch *watch, GstRTSPMessage *message,
	gpointer user_data)
{
	RTSP_client_session *rtsp_s = (RTSP_client_session*)user_data;

	g_mutex_lock(rtsp_s->s_mutex);
	if (!rtsp_s->killed)
	{
		rtsp_session_handle_rtsp_message(rtsp_s, message);
	}
	g_mutex_unlock(rtsp_s->s_mutex);

	return GST_RTSP_OK;
}


static GstRTSPResult
rtsp_session_watch_close(GstRtspWatch *watch, gpointer user_data)
{
//	RTSP_client_session *rtsp_s = (RTSP_client_session*)user_data;

	return GST_RTSP_OK;
}


static GstRTSPResult
rtsp_session_watch_error(GstRtspWatch *watch, GstRTSPResult result,
	gpointer user_data)
{
//	RTSP_client_session *rtsp_s = (RTSP_client_session*)user_data;

	return GST_RTSP_OK;
}


static GstRtspWatchFuncs rtsp_wath_funcs =
{
	.message_received	= rtsp_server_message_received,
	.closed				= rtsp_session_watch_close,
	.error				= rtsp_session_watch_error
};


static __inline__ void
rtsp_client_session_free(RTSP_client_session *rtsp_s)
{
	g_mutex_free(rtsp_s->s_mutex);

	if (rtsp_s->rtsp_sock > 0)
	{
		close(rtsp_s->rtsp_sock);
	}

	if (rtsp_s->url)
	{
		RTSP_Url_destroy(rtsp_s->url);
	}

	g_free(rtsp_s->session_id);
	g_free(rtsp_s);
}


RTSP_client_session *
rtsp_client_session_ref(RTSP_client_session *rtsp_s)
{
	REF_DEBUG_TEST(rtsp_s);

	g_atomic_int_inc(&rtsp_s->ref_count);
	return rtsp_s;
}


void
rtsp_client_session_unref(RTSP_client_session *rtsp_s)
{
	REF_DEBUG_TEST(rtsp_s);

	if (g_atomic_int_dec_and_test(&rtsp_s->ref_count))
	{
		rtsp_client_session_free(rtsp_s);
	}
}



static void
rtsp_session_watch_finalize(void *user_data)
{
	RTSP_client_session *rtsp_s = (RTSP_client_session*)user_data;
	rtsp_client_session_unref(rtsp_s);
}


static void
rtsp_session_timer_finalize(GEvent *ev)
{
	RTSP_client_session *rtsp_s = (RTSP_client_session*)g_event_u(ev);
	rtsp_client_session_unref(rtsp_s);
}


static __inline__ void
rtsp_session_keepalive(RTSP_client_session *rtsp_s)
{
	rtsp_session_options(rtsp_s);
}


static __inline__ gboolean
rtsp_session_1sec_elapse(RTSP_client_session *rtsp_s)
{
	if (++rtsp_s->ev_timer_counter >= rtsp_s->timeout)
	{
		rtsp_s->ev_timer_counter = 0;
		rtsp_session_keepalive(rtsp_s);
	}

	return TRUE;
}


static __inline__ gboolean
rtsp_conn_writable(RTSP_client_session *rtsp_s)
{
	gint err = 0;
	socklen_t len = sizeof(err);

	if (getsockopt(rtsp_s->rtsp_sock, SOL_SOCKET, SO_ERROR,
		&err, &len) < 0)
	{
		fnc_log(FNC_LOG_ERR,
			"[FC] rtsp_conn_writable()->getsockopt()"
		);
		return FALSE;
	}

	if (err)
	{
		fnc_log(FNC_LOG_ERR,
			"[FC] rtsp_conn_writable()->err:%d", err
		);
		return FALSE;
	}

	rtsp_s->rtsp_watch = gst_rtsp_watch_new_2(
										rtsp_s->rtsp_sock,
										&rtsp_wath_funcs,
										rtsp_client_session_ref(rtsp_s),
										rtsp_session_watch_finalize);
	rtsp_s->rtsp_sock = -1;

	g_event_remove_events_sync(rtsp_s->ev_timer, EV_READ|EV_WRITE);
	g_event_mod_timer_sync(rtsp_s->ev_timer, 1.);

	gst_rtsp_watch_attach_2(rtsp_s->rtsp_watch, NULL);
	
	rtsp_session_describe(rtsp_s);

	return TRUE;
}


static __inline__ gboolean
rtsp_conn_timeout(RTSP_client_session *rtsp_s)
{
	return FALSE;
}


static __inline__ gboolean
__rtsp_session_on_timer(RTSP_client_session *rtsp_s, gint revents)
{
	if (rtsp_s->rtsp_watch)
	{
		return rtsp_session_1sec_elapse(rtsp_s);
	}

	if (revents & EV_WRITE)
	{
		return rtsp_conn_writable(rtsp_s);
	}

	return rtsp_conn_timeout(rtsp_s);
}


static gboolean
rtsp_session_on_timer(GEvent *ev, gint revents, void *user_data)
{
	RTSP_client_session *rtsp_s = (RTSP_client_session*)user_data;
	gboolean ret = FALSE;

	g_mutex_lock(rtsp_s->s_mutex);
	if (!rtsp_s->killed)
	{
		ret = __rtsp_session_on_timer(rtsp_s, revents);
	}
	g_mutex_unlock(rtsp_s->s_mutex);

	return ret;
}


static __inline__ gint
__rtsp_session_connect_nowait(RTSP_client_session *rtsp_s)
{
	gint err;

	rtsp_s->rtsp_sock = unix_sock_bind(NULL, 0, L4_TCP);
	if (rtsp_s->rtsp_sock < 0)
		return rtsp_s->rtsp_sock;

	if (!(err = set_fd_flags(rtsp_s->rtsp_sock, O_NONBLOCK)))
	{
		err = unix_sock_connect(
			rtsp_s->rtsp_sock,
			rtsp_s->url->hostname,
			atoi(rtsp_s->url->port)
		);
		if (!err || err == -EINPROGRESS)
		{
			if (!err)
			{
				rtsp_s->ev_timer = g_event_new(sizeof(GEvent), -1, 0);
				g_event_set_timeout(rtsp_s->ev_timer, 1000);
				rtsp_s->rtsp_watch = gst_rtsp_watch_new_2(
													rtsp_s->rtsp_sock,
													&rtsp_wath_funcs,
													rtsp_client_session_ref(rtsp_s),
													rtsp_session_watch_finalize);
				rtsp_s->rtsp_sock = -1;
				gst_rtsp_watch_attach_2(rtsp_s->rtsp_watch, NULL);
				rtsp_session_describe(rtsp_s);
			}
			else
			{
				rtsp_s->ev_timer = g_event_new(sizeof(GEvent),
					rtsp_s->rtsp_sock, EV_WRITE);
				g_event_set_timeout(rtsp_s->ev_timer, CONN_WAIT * 1000);				
			}

			g_event_set_callback(
							rtsp_s->ev_timer,
							rtsp_session_on_timer,
							rtsp_client_session_ref(rtsp_s),
							rtsp_session_timer_finalize);
			g_scheduler_add(rtsp_s->ev_timer, 1);

			err = 0;
		}
	}

	return err;	
}


static __inline__ gint
rtsp_session_connect_nowait(RTSP_client_session *rtsp_s)
{
	gint ret = -EINVAL;

	if (rtsp_s->rtsp_sock > 0)
	{
		fnc_log(FNC_LOG_ERR,
			"[FC] rtsp_s->rtsp_sock already exist."
		);
		return -EINVAL;
	}

	g_mutex_lock(rtsp_s->s_mutex);
	if (!rtsp_s->killed)
	{
		ret = __rtsp_session_connect_nowait(rtsp_s);
	}
	g_mutex_unlock(rtsp_s->s_mutex);

	return ret;
}


RTSP_client_session *
rtsp_client_session_new( void )
{
	RTSP_client_session *rtsp_s;

	rtsp_s = g_new0(RTSP_client_session, 1);
	rtsp_s->ref_count = 1;
	rtsp_s->state = RTSP_INIT;
	rtsp_s->s_mutex = g_mutex_new();
	rtsp_s->timeout = DEFAULT_SESSION_TIMEOUT;

	return rtsp_s;
}


static __inline__ void
rtsp_session_kill(RTSP_client_session *rtsp_s)
{
	g_mutex_lock(rtsp_s->s_mutex);
	if (!rtsp_s->killed)
	{
		if (rtsp_s->ev_timer)
		{
			g_scheduler_del(rtsp_s->ev_timer);
			g_event_unref(rtsp_s->ev_timer);
			rtsp_s->ev_timer = NULL;
		}

		if (rtsp_s->rtsp_watch)
		{
			g_scheduler_del((GEvent*)rtsp_s->rtsp_watch);
			g_event_unref((GEvent*)rtsp_s->rtsp_watch);			
			rtsp_s->rtsp_watch = NULL;
		}

		if (rtsp_s->medium)
		{
			rtsp_media_free(rtsp_s->medium);
			rtsp_s->medium = NULL;
		}

		rtsp_s->killed = 1;
	}
	g_mutex_unlock(rtsp_s->s_mutex);
}


void rtsp_session_kill_sync(RTSP_client_session *rtsp_s)
{
	g_assert(rtsp_s != NULL);

	rtsp_session_kill(rtsp_s);
	rtsp_client_session_unref(rtsp_s);
}


void
rtsp_client_set_callback(RTSP_client_session *rtsp_s, Exp_Callback ecb,
	Data_Callback dcb, void *u)
{
	g_assert(rtsp_s != NULL);

	rtsp_s->usr_cb.u = u;
	rtsp_s->usr_cb.exp_cb = ecb;
	rtsp_s->usr_cb.dat_cb = dcb;
}


gint
rtsp_session_lock(RTSP_client_session *rtsp_s)
{
	g_assert(rtsp_s != NULL);

	g_mutex_lock(rtsp_s->s_mutex);
	if (G_UNLIKELY(rtsp_s->killed))
	{
		g_mutex_unlock(rtsp_s->s_mutex);
		return -1;
	}

	return 0;
}


void
rtsp_session_unlock(RTSP_client_session *rtsp_s)
{
	g_assert(rtsp_s != NULL);

	g_mutex_unlock(rtsp_s->s_mutex);
}


gint
rtsp_client_open_session(RTSP_client_session *rtsp_s, const gchar *url,
	L4_Proto l4)
{
	gint err = -EINVAL;
	g_assert(rtsp_s != NULL );

	if (rtsp_s->url)
	{
		fnc_log(FNC_LOG_ERR, "[FC] Session already opened.");
		return err;
	}

	if (!url || !*url)
	{
		fnc_log(FNC_LOG_ERR, "[FC] Invalid url while open session.");
		return err;
	}

	rtsp_s->url = RTSP_url_parse((gchar*)url);
	if (!RTSP_url_is_complete(rtsp_s->url))
	{/* RTSP_Url_destroy() while session freeing */
		fnc_log(FNC_LOG_ERR, "[FC] Invalid URL given.");
		return err;
	}

	rtsp_s->rtp_over_rtsp = l4 == L4_TCP ? 1 : 0;
	err = rtsp_session_connect_nowait(rtsp_s);
	if (err)
	{
		fnc_log(FNC_LOG_ERR, "[FC] Connect RTSP server %s:%s failed.",
			rtsp_s->url->hostname, rtsp_s->url->port);
		return err;
	}

	return 0;
}


static __inline__ gint
__rtsp_client_play_session(RTSP_client_session *rtsp_s,
	gdouble start, gdouble end)
{
	if (rtsp_s->state < RTSP_READY)
	{
		rtsp_s->play_pending = TRUE;
		return 0;
	}

	rtsp_session_play(rtsp_s);
	return 0;
}


gint rtsp_client_play_session(RTSP_client_session *rtsp_s,
	gdouble start, gdouble end)
{
	gint ret = -EINVAL;
	g_assert(rtsp_s != NULL);

	g_mutex_lock(rtsp_s->s_mutex);
	if (!rtsp_s->killed)
		ret = __rtsp_client_play_session(rtsp_s, start, end);
	g_mutex_unlock(rtsp_s->s_mutex);

	return ret;
}


gint rtsp_session_get_interleaved(RTSP_client_session *rtsp_s)
{
	gint ch;
	g_assert(rtsp_s != NULL);

	ch =  rtsp_s->interleaved;
	++rtsp_s->interleaved;

	return ch;
}


//:~ End
