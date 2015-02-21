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

#define _GNU_SOURCE
#include <unistd.h>
#include <fcntl.h>
#include "mediaparser.h"
#include "demuxer.h"
#include "rtp.h"
#include "rtsp.h"
#include "rc_log.h"
#include "rtp_watch.h"

#define MAX_RND_PACKAGES	128
#define MAX_SND_BLOCKSIZE	(50*1024)
#define _RTP_MTU			(9*1024)
extern gint __set_fd_flags(gint fd, gint flgs);

static gboolean rtp_timer_cb(GEvent *ev, gint revents, void *user_data);
static gboolean rtp_lwrite_cb(GEvent *ev, gint revents, void *user_data);
static __inline__ void rtp_pause_transport(RTP_transport *transport);

typedef struct _RTP_writer RTP_writer;
struct _RTP_writer
{
	GEvent	base;
	RTP_session	*rtp_s;
};

typedef struct _RTP_packet RTP_packet;
struct _RTP_packet
{
#if (G_BYTE_ORDER == G_LITTLE_ENDIAN)
    guint8	csrc_len:4;
    guint8	extension:1;
    guint8	padding:1;
    guint8	version:2;
#elif (G_BYTE_ORDER == G_BIG_ENDIAN)
    guint8	version:2;
    guint8	padding:1;
    guint8	extension:1;
    guint8	csrc_len:4;
#else
# error Neither big nor little
#endif
    /* byte 1 */
#if (G_BYTE_ORDER == G_LITTLE_ENDIAN)
    guint8	payload:7;
    guint8	marker:1;     /* expect 1 */
#elif (G_BYTE_ORDER == G_BIG_ENDIAN)
    guint8	marker:1;
    guint8	payload:7;
#endif

    guint16	seq_no;
    guint32	timestamp;
    guint32	ssrc;	/* stream number is used here. */
    guint8	data[];	/**< Variable-sized data payload */
};

#define RTP_PKT_DATA(pkt)   (&((RTP_packet*)pkt)->data[0]  + ((RTP_packet*)pkt)->csrc_len)

typedef struct __j_rtp_extend_header j_rtp_extend_header_t;
struct __j_rtp_extend_header
{
#if G_BYTE_ORDER == G_BIG_ENDIAN
  guint8  version   : 2,          // 单独为RTP扩展部分设置版本号码
            frame_type: 4,          // 参考Jtype.h 中 j_frame_type_t 定义
            reserved  : 2;
#else
  guint8  reserved  : 2,
            frame_type: 4,
            version   : 2;
#endif
  guint8  unused;
  guint16 length;    // RTP 扩展包长度，不包括extend_header
};


typedef struct __j_rtp_extend_info j_rtp_extend_info_t;
struct __j_rtp_extend_info
{
  guint16 width;         // 视频宽度
  guint16 height;        // 视频高度
  guint32 frame_num;     // 帧号
  guint32 frame_length;  // 视频帧长度
};


#ifndef RTCP_SUPPORT

static __inline__ gboolean
rtcp_send_sr(RTP_session *session, rtcp_pkt_type type)
{
	return TRUE;
}

#else

static __inline__ gboolean
rtcp_send_sr(RTP_session *session, rtcp_pkt_type type)
{
	return TRUE;
}

#endif

static void
on_rtp_writer_destroy(GEvent *ev)
{
	gint fd = g_event_fd(ev);
	RTP_writer *w = (RTP_writer*)ev;

	rtp_session_unref(w->rtp_s);
	close(fd);
}

static void
on_rtp_periodic_destroy(GEvent *ev)
{
	RTP_writer *w = (RTP_writer*)ev;

	rtp_session_unref(w->rtp_s);
}

static __inline__ void
rtp_pause_transport(RTP_transport *transport)
{
	if (transport->rtp_writer)
	{
		g_event_remove(transport->rtp_writer);
		g_event_unref((GEvent*)transport->rtp_writer);
		transport->rtp_writer = NULL;
	}
}


void
rtp_transport_close(RTP_transport *transport)
{
	port_pair pair;

	switch (transport->trans_mode)
	{
	case RTP_OVER_TCP:
		g_event_remove((GEvent*)transport->passby);
		g_event_unref((GEvent*)transport->passby);
		/* fall through */
	case RTP_OVER_UDP:
	    pair.RTP = get_local_port(transport->rtp_sock);
	    pair.RTCP = get_local_port(transport->rtcp_sock);		
		put_port_pair(NULL, &pair);
		Sock_close(transport->rtp_sock);
		Sock_close(transport->rtcp_sock);
		break;

	case RTP_OVER_RTSP:
		g_event_unref((GEvent*)transport->passby);
		break;

	default:
		break;
	}
}


static __inline__ void
rtp_fill_pool_free(RTP_session *rtp_s)
{
    Resource *resource = rtp_s->track->parent;
    GThreadPool *tp;

	g_mutex_lock(resource->lock);
	resource->eor = TRUE;
	g_mutex_unlock(resource->lock);

	resource->eor = FALSE;

	tp = rtp_s->fill_pool;
	rtp_s->fill_pool = NULL;

	g_thread_pool_free(tp, TRUE, TRUE);
}


static void
rtp_session_fill_cb(gpointer unused_data, gpointer session_p)
{
    RTP_session *rtp_s = (RTP_session*)session_p;
    Resource *resource = rtp_s->track->parent;
    BufferQueue_Consumer *consumer = rtp_s->consumer;
    gulong unseen;

	unseen = bq_consumer_unseen(consumer);
	if (unseen >= MAX_RND_PACKAGES)
		return;

    while ((unseen = bq_consumer_unseen(consumer)) < 3*MAX_RND_PACKAGES)
    {
    	if (r_read(resource) != RESOURCE_OK)
            break;
    }
}


static __inline__ void
rtp_session_fill(RTP_session *session)
{
	if (!session->fill_pool)
		return;

    if (!session->track->parent->eor)
    {
        g_thread_pool_push(
        	session->fill_pool,
			GINT_TO_POINTER(-1),
			NULL
		);
	}
}


static __inline__ gint
stream_is_live_mode(RTP_session *rtp_s)
{
	g_assert(rtp_s && rtp_s->track);

	return rtp_s->track->properties.media_source == MS_live;
}


static void
on_producer_working(gint fd, gpointer user_data)
{
	write(fd, "x", 1);
}


static __inline__ void
rtp_session_resume_live(RTP_session *rtp_s)
{
	GEvent *ev;

	if (rtp_s->playing)
		return;

	if (pipe2(rtp_s->fd, O_NONBLOCK) < 0)
	{
		rc_log(
			RC_LOG_ERR,
			"[RTP] play live stream faild, pipe() < 0, err:%d.",
			errno
		);
		rtp_s->fd[0] = -1;
		rtp_s->fd[1] = -1;
		return;
	}

	if (G_UNLIKELY(rtp_s->transport.rtp_writer))	/* BUG */
	{
		rc_log(
			RC_LOG_ERR,
			"[RTP] play live stream faild, 'rtp_s->transport.rtp_writer' exists."
		);
		rtp_s->playing = TRUE;
		return;
	}

	ev = g_event_new(sizeof(RTP_writer), rtp_s->fd[0], EV_READ);
	g_event_set_timeout(ev, 100);
	g_event_set_callback(ev, rtp_lwrite_cb, rtp_s, on_rtp_writer_destroy);

	((RTP_writer*)ev)->rtp_s = rtp_s;
	rtp_s->transport.rtp_writer = ev;
	rtp_session_ref(rtp_s);

	bq_producer_add_fd(rtp_s->track->producer, rtp_s->fd[1], NULL,
		on_producer_working, NULL);
	g_scheduler_add(ev, LOOP_WEIGHT_VIDEO);

	rtp_s->playing = TRUE;
}


static __inline__ void
rtp_session_resume_stored(RTP_session *rtp_s)
{
	GEvent *periodic;

	if (rtp_s->playing)
		return;

	if (rtp_s->track && rtp_s->track->properties.media_type != MP_video)
	{//只创建一路audio是一个bug.
		rtp_s->track->parent->audio_rtp = rtp_s;
		return;
	}

	if (!rtp_s->fill_pool)
	{
	    rtp_s->fill_pool = g_thread_pool_new(rtp_session_fill_cb,
	    	rtp_s, 1, TRUE, NULL);
	    rtp_session_fill(rtp_s);
	}

	if (!rtp_s->transport.rtp_writer)
	{
		periodic = g_event_new(sizeof(RTP_writer), -1, 0);
		g_event_set_timeout(periodic, 1);
		g_event_set_callback(periodic, rtp_timer_cb, rtp_s, on_rtp_periodic_destroy);

		rtp_s->transport.rtp_writer = periodic;
		((RTP_writer*)periodic)->rtp_s = rtp_s;
		rtp_session_ref(rtp_s);
		g_scheduler_add(periodic, LOOP_WEIGHT_VIDEO);
	}

	rtp_s->playing = TRUE;
}


static void
rtp_session_resume(gpointer session_gen, gpointer range_gen)
{
    RTP_session *rtp_s = (RTP_session*)session_gen;
    RTSP_Range *range = (RTSP_Range*)range_gen;

    fnc_log(FNC_LOG_VERBOSE, "Resuming session %p\n", rtp_s);

    rtp_s->range = range;
    rtp_s->start_seq = 1 + rtp_s->seq_no;
    rtp_s->start_rtptime = g_random_int();
    rtp_s->send_time = 0.0;
    rtp_s->last_packet_send_time = time(NULL);

	if (stream_is_live_mode(rtp_s))
	{
		rtp_session_resume_live(rtp_s);
		return;
	}

	rtp_session_resume_stored(rtp_s);
}


void rtp_session_rtps_resume(GSList *sessions_list, RTSP_Range *range)
{
    g_slist_foreach(sessions_list, rtp_session_resume, range);
}


static __inline__ void
rtp_session_pause_live(RTP_session *rtp_s)
{
	rtp_s->playing = FALSE;
	if (rtp_s->fd[1] <= 0)
		return;

	bq_producer_remove_fd(rtp_s->track->producer, rtp_s->fd[1]);
	rtp_pause_transport(&rtp_s->transport);
	close(rtp_s->fd[1]);	
}


static __inline__ void
rtp_session_pause_stored(RTP_session *rtp_s)
{
	rtp_s->playing = FALSE;

	if (rtp_s->track && rtp_s->track->parent &&
		rtp_s->track->parent->audio_rtp == rtp_s)
	{
		rtp_s->track->parent->audio_rtp = NULL;
	}

    if (rtp_s->fill_pool)
    {
        rtp_fill_pool_free(rtp_s);
    }

	rtp_pause_transport(&rtp_s->transport);
}


static void
rtp_session_pause(gpointer session_gen, gpointer user_data)
{
    RTP_session *rtp_s = (RTP_session *)session_gen;

	if (!rtp_s->playing)
		return;

	if (stream_is_live_mode(rtp_s))
	{
		rtp_session_pause_live(rtp_s);
		return;
	}

	rtp_session_pause_stored(rtp_s);
}


void rtp_session_rtps_pause(GSList *sessions_list)
{
    g_slist_foreach(sessions_list, rtp_session_pause, NULL);
}


static __inline__ guint32
RTP_calc_rtptime(RTP_session *rtp_s, gint clock_rate,
	MParserBuffer *buffer)
{
    guint32 calc_rtptime;

	calc_rtptime = (buffer->timestamp - rtp_s->range->begin_time) * clock_rate;
    return rtp_s->start_rtptime + calc_rtptime;
}


static __inline__ gint
__rtp_pkt_send_ror(RTP_session *session, RTP_transport *transport, 
	RTP_packet *packet, gsize packet_size)
{
	GstRTSPMessage *msg;
	extern gint rtsp_client_send_message(RTSP_Client *client, GstRTSPMessage *msg);

	gst_rtsp_message_new_data(&msg, transport->rtp_ch);
	gst_rtsp_message_set_body(msg, (const guint8*)packet, packet_size);
	rtsp_client_send_message((RTSP_Client*)transport->passby, msg);
	gst_rtsp_message_free(msg);

	return 0;
}


static __inline__ gint
__rtp_pkt_send_rot(RTP_session *session, RTP_transport *transport, 
	RTP_packet *packet, gsize packet_size)
{
	GstRTSPMessage *msg;

	gst_rtsp_message_new_data(&msg, transport->rtp_ch);
	gst_rtsp_message_set_body(msg, (const guint8*)packet, packet_size);
	rtp_watch_write_message((RTP_watch*)transport->passby, msg);
	gst_rtsp_message_free(msg);

	return 0;
}


static __inline__ gint
__rtp_pkt_send(RTP_session *session, RTP_transport *transport, 
	RTP_packet *packet, gsize packet_size)
{
	switch (transport->trans_mode)
	{
	case RTP_OVER_UDP:
		return Sock_write(transport->rtp_sock, packet, packet_size,
			NULL, MSG_DONTWAIT | MSG_EOR);

	case RTP_OVER_TCP:
		return __rtp_pkt_send_rot(session, transport, packet, packet_size);

	case RTP_OVER_RTSP:
		return __rtp_pkt_send_ror(session, transport, packet, packet_size);
	}

	return -1;
}


static void
rtp_packet_send(RTP_session *session, MParserBuffer *buffer, time_t time_now)
{
    gsize packet_size = sizeof(RTP_packet) + buffer->data_size;
    RTP_packet *packet = g_malloc0(packet_size);
    Track *tr = session->track;
    guint32 timestamp = RTP_calc_rtptime(session,
                                         tr->properties.clock_rate,
                                         buffer);

    packet->version = 2;
    packet->padding = 0;
    packet->extension = 0;
    packet->csrc_len = 0;
    packet->marker = buffer->marker & 0x1;
    packet->payload = tr->properties.payload_type & 0x7f;
    packet->seq_no = htons(++session->seq_no);
    packet->timestamp = htonl(timestamp);
    packet->ssrc = htonl(session->ssrc);

    fnc_log(FNC_LOG_VERBOSE, "[RTP] Timestamp: %u", ntohl(timestamp));

    memcpy(packet->data, buffer->data, buffer->data_size);

	if (__rtp_pkt_send(session, &session->transport, 
		packet, packet_size) < 0)
	{
        fnc_log(FNC_LOG_DEBUG, "RTP Packet Lost\n");		
	}
    else
    {
        session->last_timestamp = buffer->timestamp;
        session->pkt_count++;
        session->octet_count += buffer->data_size;

        session->last_packet_send_time = time_now;//time(NULL);
    }

    g_free(packet);
}


static void
rtp_packet_send_eof(RTP_session *session, time_t time_now)
{
    gsize packet_size = sizeof(RTP_packet) + 32;
    RTP_packet *packet = g_malloc0(packet_size);
    Track *tr = session->track;
    guint32 timestamp = time_now + 1;
    j_rtp_extend_header_t *ext_hdr;

    packet->version = 2;
    packet->padding = 0;
    packet->extension = 1;
    packet->csrc_len = 0;
    packet->marker = 1;
    packet->payload = tr->properties.payload_type & 0x7f;
    packet->seq_no = htons(++session->seq_no);
    packet->timestamp = htonl(timestamp);
    packet->ssrc = htons(session->ssrc);

	ext_hdr = (j_rtp_extend_header_t*)(gchar*)RTP_PKT_DATA(packet);
	ext_hdr->frame_type = 15;//end frame.
	ext_hdr->version = 0x1;
	ext_hdr->length = htons(sizeof(j_rtp_extend_info_t)/4);

	__rtp_pkt_send(session, &session->transport, packet, packet_size);
	g_free(packet);
}


static gboolean
rtp_session_would_block(RTP_session *session)
{
	RTP_transport *transport = &session->transport;

	if (transport->trans_mode == RTP_OVER_RTSP)
		return rtsp_client_would_block((RTSP_Client*)transport->passby, _RTP_MTU);

	if (transport->trans_mode == RTP_OVER_TCP)
		return rtp_watch_would_block((RTP_watch*)transport->passby);

	return FALSE;
}


static void
rtp_packet_send_rtp(RTP_session *session, MParserBuffer *buffer, time_t time_now)
{
	if (__rtp_pkt_send(session, &session->transport, 
		(RTP_packet*)buffer->data, buffer->data_size) < 0)
	{
        fnc_log(FNC_LOG_DEBUG, "RTP Packet Lost\n");
	}
    else
    {
    	session->seq_no = ntohs(((RTP_packet*)buffer->data)->seq_no);
        session->last_timestamp = buffer->timestamp;
        session->pkt_count++;
        session->octet_count += buffer->data_size;
		session->ssrc = ntohs(((RTP_packet*)buffer->data)->ssrc);
        session->last_packet_send_time = time_now;//time(NULL);
    }
}


static gboolean
rtp_timer_cb(GEvent *ev, gint revents, void *user_data)
{
    RTP_session *session = (RTP_session*)user_data;
    Resource *resource = session->track->parent;
    MParserBuffer *buffer = NULL;
    ev_tstamp now, sleep_secs;
    gint to_send = MAX_SND_BLOCKSIZE;

	now = g_event_time_now_sync(ev);
	sleep_secs = .01;

#ifdef HAVE_METADATA
    if (session->metadata)
        cpd_send(session, now);
#endif

	if (!session->audio_rtp && resource->audio_rtp)
	{//Fixme, race against rtp_session_kill(resource->audio_rtp)
		session->audio_rtp = rtp_session_ref(resource->audio_rtp);
	}

	for (;;)
	{
		if (to_send <= 0)
		{
			break;
		}

	    if (bq_consumer_stopped(session->consumer))
	    {
	
	        fnc_log(FNC_LOG_INFO, "[rtp] Stream Finished");
			rtcp_send_sr(session, BYE);
	        return FALSE;
	    }

		if (rtp_session_would_block(session))
		{
			break;
		}

	    if (!(buffer = bq_consumer_get(session->consumer)))
	    {
	        if (resource->eor)
	        {
	            fnc_log(FNC_LOG_INFO, "[rtp] Stream Finished");
				rtp_packet_send_eof(session, (time_t)now);
				rtcp_send_sr(session, BYE);
				sleep_secs = 1;
				break;
	        }
	        sleep_secs = .005;
	        break;
	    }

		if (buffer->mtype != MP_video)
		{
			if (session->audio_rtp)
			{
				rtp_packet_send_rtp(session->audio_rtp, buffer, (time_t)now);
			}
		}
		else
		{
			rtp_packet_send_rtp(session, buffer, (time_t)now);
		}
		to_send -= buffer->data_size;

		if (!bq_consumer_move(session->consumer))
		{
			break;
		}
	}

	g_event_mod_timer_sync(ev, sleep_secs);
    rtp_session_fill(session);

	return TRUE;
}


static gboolean
rtp_lwrite_cb(GEvent *ev, gint revents, void *user_data)
{
#define XBUFLEN   256
	RTP_session *rtp_s = (RTP_session*)user_data;
	Resource *resource = rtp_s->track->parent;
	MParserBuffer *buffer;
	gchar x[XBUFLEN];
	gint n, fd = g_event_fd(ev);
	ev_tstamp now;

	while ( TRUE )
	{
		n = read(fd, x, XBUFLEN);
		if (n > 0)
			continue;
			
		if (n < 0 && errno == EAGAIN)
			break;

		rc_log(
			RC_LOG_WARN,
			"[rtp] writer stopped, something wrong with rtp->fd[0]."
		);

		rtcp_send_sr(rtp_s, BYE);
		return FALSE;
	}	

	if (G_UNLIKELY(bq_consumer_stopped(rtp_s->consumer)))
	{
	    rc_log(FNC_LOG_INFO, "[rtp] one live stream finished.");
		rtcp_send_sr(rtp_s, BYE);
	    return FALSE;
	}

	while ( TRUE )
	{	
		if (rtp_session_would_block(rtp_s))
		{
			break;
		}

		if (!(buffer = bq_consumer_get(rtp_s->consumer)))
		{
			if (resource->eor)
	        {
	            fnc_log(FNC_LOG_INFO, "[rtp] Stream Finished"); 
				rtp_packet_send_eof(rtp_s, (time_t)now);
				rtcp_send_sr(rtp_s, BYE);
	      		return FALSE;
	        }
			break;
		}

		if (buffer == LS_EOF)
		{
            fnc_log(FNC_LOG_INFO, "[rtp] Stream Finished"); 
			rtp_packet_send_eof(rtp_s, (time_t)now);
			rtcp_send_sr(rtp_s, BYE);
      		return FALSE;					
		}

		now = g_event_time_now_sync(ev);
		rtp_packet_send(rtp_s, buffer, (time_t)now);

		if (!bq_consumer_move(rtp_s->consumer))
			break;
	}

	return TRUE;
}


static gboolean
rtp_over_tcp_rw_cb(GEvent *ev, int revents, void *user_data)
{
	RTP_watch *w = (RTP_watch*)ev;

	return rtp_watch_write_pending(w);
}


void
rtp_over_tcp_prepare(RTP_transport *transport)
{
	transport->passby = (GEvent*)rtp_watch_new(Sock_fd(transport->rtp_sock), 0);
	g_event_set_callback((GEvent*)transport->passby, rtp_over_tcp_rw_cb,
		transport, rtp_watch_finalize);
	g_scheduler_add((GEvent*)transport->passby, LOOP_WEIGHT_VIDEO);
}


RTP_session *
rtp_session_new(RTSP_Client *client, RTSP_session *rtsp_s, RTP_transport *transport,
	GstRTSPUrl *uri, Track *tr)
{
    RTP_session *rtp_s = g_new0(RTP_session, 1);

	rtp_s->ref_count = 1;
	rtp_s->playing = FALSE;
    rtp_s->uri = gst_rtsp_url_get_request_uri(uri);

    memcpy(&rtp_s->transport, transport, sizeof(RTP_transport));
    memset(transport, 0, sizeof(RTP_transport));
    rtp_s->start_rtptime = g_random_int();
    rtp_s->start_seq = g_random_int_range(0, G_MAXUINT16);
    rtp_s->seq_no = rtp_s->start_seq - 1;

    rtp_s->track = tr;
    if (tr->properties.media_type == MP_video)
    	rtp_s->consumer = bq_consumer_new(tr->producer);

    rtp_s->ssrc = g_random_int();
    rtp_s->client = client;

#ifdef HAVE_METADATA
	rtp_s->metadata = rtsp_s->resource->metadata;
#endif

    rtsp_s->rtp_sessions = g_slist_append(rtsp_s->rtp_sessions, rtp_s);
    rtp_s->rtsp = rtsp_s;
    rtsp_session_ref(rtsp_s);

    return rtp_s;
}


void
rtp_session_kill(RTP_session *rtp_s)
{
    rtp_session_pause(rtp_s, NULL);
	if (rtp_s->consumer)
    	bq_consumer_kill(rtp_s->consumer);
}


static __inline__ void
rtp_session_finalize(RTP_session *rtp_s)
{
    rtp_transport_close(&rtp_s->transport);

    if (rtp_s->fill_pool)
        rtp_fill_pool_free(rtp_s);

	if (rtp_s->consumer)
    	bq_consumer_free(rtp_s->consumer);
	rtsp_session_unref(rtp_s->rtsp);

	if (rtp_s->audio_rtp)
		rtp_session_unref(rtp_s->audio_rtp);

    g_free(rtp_s->uri);
    g_free(rtp_s);
}


RTP_session *
rtp_session_ref(RTP_session *rtp_s)
{
	__OBJECT_REF(rtp_s);
}


void
rtp_session_unref(RTP_session *rtp_s)
{
	__OBJECT_UNREF(rtp_s, rtp_session);
}

//:~ End
