/* *
 * This file is part of rtsp-client lib.
 * Copyright (C) 2012 by fangyi <ianfang.cn@gmail.com>
 *
 * */

#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <nmpevsched.h>
#include "loop_weight.h"
#include "fnc_log.h"
#include "rtsp_client.h"
#include "rtpparser.h"
#include "sock.h"
#include "rtsp.h"


#define RTP_BUFFER_LEN		(16*1024)

static __inline__ void rtp_session_unref(RTP_Session *rtp_s);
static __inline__ RTP_Session *rtp_session_ref(RTP_Session *rtp_s);


gint
rtp_parse_frame(RTP_Session *rtp_s, gchar *rtp_pkt, gsize rtp_size,
	RTP_Frame *fr)
{
	memset(fr, 0, sizeof(*fr));

	fr->stm = rtp_s->stm_type;
	fr->pt = RTP_PKT_PT(rtp_pkt);
	fr->ts = RTP_PKT_TS(rtp_pkt);

#if 0
	if (G_UNLIKELY(fr->pt != rtp_s->pt))
		return RTP_PKT_AGAIN;
#endif

	return rtp_s->parser->parse(rtp_s, rtp_pkt, rtp_size, fr);
}


static __inline__ gboolean
rtsp_recv_rtp_packets(RTSP_client_session *rtsp_s, RTP_Session *rtp_s,
	GEvent *ev)
{
	gchar rtp_pkt[RTP_BUFFER_LEN];
	gint rtp_size, err;

	for (;;)
	{
		rtp_size = recvfrom(g_event_fd(ev), rtp_pkt, RTP_BUFFER_LEN,
			MSG_DONTWAIT, NULL, NULL);
		if (rtp_size > 0)
		{
			RTP_Frame fr;
	
			err = rtp_parse_frame(rtp_s, rtp_pkt, rtp_size, &fr);
			if (err == RTP_PKT_OK || err == RTP_PKT_AGAIN)
			{
				if (err == RTP_PKT_OK)
				{
					INVOKE_DAT_HOOK_FRA(rtsp_s, &fr, NULL);
				}
				continue;
			}

			fnc_log(FNC_LOG_ERR,
				"[FC] Rtp parse failed, rtsp_s '%p', stop.", rtsp_s
			);

			g_event_remove_events_sync(ev, EV_READ|EV_WRITE);
			break;
		}
		else
		{
			err = -errno;
			if (err != -EAGAIN && err != -EINTR)
			{
				fnc_log(FNC_LOG_ERR,
					"[FC] Rtp recv failed, rtsp_s '%p', err:'%d', stop.", rtsp_s, err
				);
				g_event_remove_events_sync(ev, EV_READ|EV_WRITE);				
			}

			break;
		}
	}

	return TRUE;	
}


static __inline__ gboolean
rtp_recv_packets(RTP_Session *rtp_s, GEvent *ev)
{
	RTSP_client_session *rtsp_s = (RTSP_client_session*)rtp_s->rtsp_s;
	gboolean ret;

	if (rtsp_session_lock(rtsp_s))
		return FALSE;

	ret = rtsp_recv_rtp_packets(rtsp_s, rtp_s, ev);

	rtsp_session_unlock(rtsp_s);
	return ret;
}


static gboolean
rtp_packet_incoming_cb(GEvent *ev, int revents, void *user_data)
{
	RTP_Session *rtp_s = (RTP_Session*)user_data;

	return rtp_recv_packets(rtp_s, ev);	
}


#ifdef CONFIG_RTCP_SUPPORT
static gboolean
rtcp_packet_incoming_cb(GEvent *ev, int revents, void *user_data)
{
	gchar rtcp_pkt[RTP_BUFFER_LEN];

	for (;;)
	{
		if (recvfrom(g_event_fd(ev), rtcp_pkt, RTP_BUFFER_LEN,
			MSG_DONTWAIT, NULL, NULL) < 0)
		{
			if (errno != EAGAIN)
			{
				g_event_remove_events_sync(ev, EV_READ|EV_WRITE);
			}
			break;
		}
	}

	return TRUE;
}
#endif


static void
r_event_src_finalize(GEvent *ev)	/* on destroy notify */
{
	R_Event *r_ev = (R_Event*)ev;
	RTP_Session * rtp_s = r_ev->rtp_s;

	rtp_session_unref(rtp_s);
}


static __inline__ R_Event *
rtp_session_create_event(RTP_Session *rtp_s, enum R_Type type, gint sock)
{
	R_Event *ev = (R_Event*)g_event_new(sizeof(R_Event), sock, EV_READ);	/* init: readable */

	ev->type = type;
	ev->rtp_s = rtp_s;

	if (type == R_EV_RTP)
	{
		g_event_set_callback((GEvent*)ev , rtp_packet_incoming_cb, rtp_s,
			r_event_src_finalize);
	}
	else
	{
		g_event_set_callback((GEvent*)ev , rtcp_packet_incoming_cb, rtp_s,
			r_event_src_finalize);		
	}

	rtp_session_ref(rtp_s);
	return ev;
}


static __inline__ RTP_Session *
rtp_session_new(RTSP_client_session *rtsp_s)
{
	RTP_Session *rtp_s;
	gint err;

	rtp_s = g_new0(RTP_Session, 1);
	rtp_s->ref_count = 1;

	if (!rtsp_s->rtp_over_rtsp)
	{
		err = rtp_session_sock_init(rtp_s);
		if (err)
		{
			fnc_log(FNC_LOG_ERR, "[FC] Init RTP sock failed, err:%d", err);
			g_free(rtp_s);
			return NULL;
		}
	}
	else
	{
		rtp_s->interleaved = 1;
		rtp_s->rtp_port = rtsp_session_get_interleaved(rtsp_s);
#ifdef CONFIG_RTCP_SUPPORT
		rtp_s->rtcp_port = rtsp_session_get_interleaved(rtsp_s);
#endif
	}

	return rtp_s;
}


static __inline__ void
rtp_session_destroy_ssrc(RTP_Session *rtp_s)
{
	if (rtp_s->ssrc != &rtp_s->rtp_ssrc_init)
	{
		fnc_log(FNC_LOG_ERR, "[FC] rtp ssrc sanity check failed!");
		BUG();
	}

	return;
}


static __inline__ void
rtp_session_free(RTP_Session *rtp_s)
{
	if (rtp_s->parser)
	{
		rtp_s->parser->finalize(rtp_s);
		rtp_parser_release(rtp_s->parser);
	}

	if (rtp_s->rtsp_s)
		rtsp_client_session_unref((RTSP_client_session*)rtp_s->rtsp_s);

	if (rtp_s->ssrc)
		rtp_session_destroy_ssrc(rtp_s);

	if (!rtp_s->interleaved)
		rtp_session_sock_close(rtp_s);

	g_free(rtp_s);
}


static __inline__ RTP_Session *
rtp_session_create(RTSP_media *medium, const GstSDPMedia *m)
{
	RTSP_client_session *rtsp_s;
	gint err;
	RTP_Session *rtp_s;
	gchar *media;

	rtsp_s = (RTSP_client_session*)medium->rtsp_s;

	rtp_s = rtp_session_new(rtsp_s);
	if (!rtp_s)
	{
		return NULL;
	}

	rtp_s->m = (GstSDPMedia*)m;
	rtp_s->rtsp_s = rtsp_client_session_ref(rtsp_s);

#if 0
	const gchar *pt_value;
	pt_value = gst_sdp_media_get_attribute_val(m, "rtpmap");
	if (strstr(pt_value, "H264"))
	{
		rtp_s->pt = 96;
		rtp_s->parser = rtp_parser_create((gchar*)"H264");
		if (!rtp_s->parser)
		{
			fnc_log(FNC_LOG_ERR, "[FC] Init RTP parser failed, mime:'%s'", "H264");
			goto no_rtp_parser;
		}
	}
	else if (strstr(pt_value, "JPF-GENERIC"))
	{
		rtp_s->pt = 99;
		rtp_s->parser = rtp_parser_create((gchar*)"JPF-GENERIC");
		if (!rtp_s->parser)
		{
			fnc_log(FNC_LOG_ERR, "[FC] Init RTP parser failed, mime:'%s'", "JPF-GENERIC");
			goto no_rtp_parser;
		}		
	}
	else
	{
		goto no_rtp_parser;
	}
#else
	rtp_s->pt = 0;
	rtp_s->parser = rtp_parser_create((gchar*)"RTP-FAKE");
	if (!rtp_s->parser)
	{
		fnc_log(FNC_LOG_ERR, "[FC] Init RTP parser failed, mime:'%s'", "RTP");
		goto no_rtp_parser;
	}
#endif

	media = gst_sdp_media_get_media(m);
	if (!strcmp(media, "video"))
		rtp_s->stm_type = STM_VIDEO;
	else if (!strcmp(media, "audio"))
		rtp_s->stm_type = STM_AUDIO;
	else
		rtp_s->stm_type = STM_OTHER;

	err = rtp_s->parser->initialize(rtp_s, rtp_s->parser, rtp_s->pt);
	if (err)
	{
		rtp_parser_release(rtp_s->parser);
		goto no_rtp_parser;
	}

	rtp_s->ssrc = &rtp_s->rtp_ssrc_init;

	if (!rtp_s->interleaved)
	{
		rtp_s->rtp_src = rtp_session_create_event(rtp_s, R_EV_RTP, rtp_s->rtp_sock);
	
#ifdef CONFIG_RTCP_SUPPORT
		rtp_s->rtcp_src = rtp_session_create_event(rtp_s, R_EV_RTCP, rtp_s->rtcp_sock);
#endif
	}

	return rtp_s;

no_rtp_parser:
	rtp_session_free(rtp_s);
	return NULL;
}


static __inline__ RTP_Session *
rtp_session_ref(RTP_Session *rtp_s)
{
	REF_DEBUG_TEST(rtp_s);

	g_atomic_int_inc(&rtp_s->ref_count);
	return rtp_s;
}


static __inline__ void
rtp_session_unref(RTP_Session *rtp_s)
{
	REF_DEBUG_TEST(rtp_s);

	if (g_atomic_int_dec_and_test(&rtp_s->ref_count))
	{
		rtp_session_free(rtp_s);
	}
}


static __inline__ void
rtp_session_kill(RTP_Session *rtp_s)
{
#ifdef CONFIG_RTCP_SUPPORT
	if (rtp_s->rtcp_src)
	{
		g_scheduler_del((GEvent*)rtp_s->rtcp_src);
		g_event_unref((GEvent*)rtp_s->rtcp_src);
	}
#endif

	if (rtp_s->rtp_src)
	{
		g_scheduler_del((GEvent*)rtp_s->rtp_src);
		g_event_unref((GEvent*)rtp_s->rtp_src);
	}
}


static __inline__ void
rtp_session_kill_unref(RTP_Session *rtp_s)
{
	rtp_session_kill(rtp_s);
	rtp_session_unref(rtp_s);
}


static __inline__ void
rtp_session_attach_loop(RTP_Session *rtp_s)
{
	const gchar *media;
	gint w = LOOP_WEIGHT_AUDIO;

	if (rtp_s->interleaved)
		return;

	media = gst_sdp_media_get_media(rtp_s->m);
	if (media && strstr(media, "video"))	/* fix me */
	{
		w = LOOP_WEIGHT_VEDIO;
	}

	g_scheduler_add((GEvent*)rtp_s->rtp_src, w);

#ifdef CONFIG_RTCP_SUPPORT
	g_scheduler_add((GEvent*)rtp_s->rtcp_src, LOOP_WEIGHT_RTCP);
#endif
}


void
rtp_session_port(RTP_Session *rtp_s, gint *rtp, gint *rtcp)
{
	g_assert(rtp_s != NULL);

	if (rtp)
		*rtp = rtp_s->rtp_port;
	if (rtcp)
	{
#ifdef CONFIG_RTCP_SUPPORT
		*rtcp = rtp_s->rtcp_port;
#else
		*rtcp = -1;
#endif	
	}
}


RTP_Session *
rtsp_media_create_rtp_session(RTSP_media *medium, const GstSDPMedia *m)
{
	RTP_Session *rtp_s = NULL;
	g_assert(medium != NULL);

	/* We are in rtsp-watch func, medium->rtsp_s must be referenced and !killed */

	rtp_s = rtp_session_create(medium, m);
	if (rtp_s)
	{
		medium->rtp_sessions = g_list_append(medium->rtp_sessions, rtp_s);
		rtp_session_attach_loop(rtp_s);
	}

	return rtp_s;
}


static void
rtsp_media_unref_rtp(gpointer data, gpointer user_data)
{
	RTP_Session *rtp_s = (RTP_Session*)data;

	rtp_session_kill_unref(rtp_s);
}


static __inline__ void
rtsp_media_del_rtp_sessions(GList *rtp_s_list)
{
	g_list_foreach(rtp_s_list, rtsp_media_unref_rtp, NULL);
	g_list_free(rtp_s_list);
}


RTSP_media *rtsp_media_create( void )
{
	return g_new0(RTSP_media, 1);
}


void rtsp_media_free(RTSP_media *medium)
{
	g_assert(medium != NULL);

	if (medium->sdp)
	{
		gst_sdp_message_free(medium->sdp);
	}

	if (medium->rtp_sessions)
	{
		rtsp_media_del_rtp_sessions(medium->rtp_sessions);
	}
	
	if (medium->content_base)
	{
		g_free(medium->content_base);
	}

	g_free(medium);
}


typedef struct _InterleavedBuffer InterleavedBuffer;
struct _InterleavedBuffer
{
	guint8 ch;
	guint8 *buf;
	guint size;
	gint done;
};


static void
rtsp_media_rcv_rtp_interleaved(gpointer rtp, gpointer ibuffer)
{
	InterleavedBuffer *ib = (InterleavedBuffer*)ibuffer;
	RTP_Session *rtp_s = (RTP_Session*)rtp;
	RTSP_client_session *rtsp_s = (RTSP_client_session*)rtp_s->rtsp_s;
	gint err;

	if (ib->done)
		return;

	if (ib->ch == rtp_s->rtp_port)
	{
		RTP_Frame fr;

		err = rtp_parse_frame(rtp_s, (gchar*)ib->buf, ib->size, &fr);
		if (err == RTP_PKT_OK)
		{
			INVOKE_DAT_HOOK_FRA(rtsp_s, &fr, NULL);
		}
		ib->done = 1;
	}
}


gint
rtsp_session_media_interleaved_rcv(RTSP_media *media, guint8 channel,
	guint8* data, guint size)
{
	InterleavedBuffer ib;

	if (size >= RTP_BUFFER_LEN)
		return -EINVAL;

	ib.ch = channel;
	ib.buf = data;
	ib.size = size;
	ib.done = 0;

	g_list_foreach(media->rtp_sessions, rtsp_media_rcv_rtp_interleaved, &ib);
	return 0;
}


//:~ End
